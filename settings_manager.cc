// Copyright 2009 Daniel Erat <dan@erat.org>
// All rights reserved.

#include "settings_manager.h"

#include <cassert>
#include <cstring>
#include <sys/types.h>
#include <unistd.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

#include "config_parser.h"
#include "data_writer.h"
#include "setting.h"

using std::make_pair;
using std::map;
using std::string;

namespace xsettingsd {

SettingsManager::SettingsManager()
    : serial_(0),
      display_(NULL),
      root_(None),
      atom_(None),
      win_(None) {
}

SettingsManager::~SettingsManager() {
  if (display_) {
    XCloseDisplay(display_);
    display_ = NULL;
  }
}

bool SettingsManager::LoadConfig(const string& filename) {
  ConfigParser parser(new ConfigParser::FileCharStream(filename));
  SettingsMap new_settings;
  if (!parser.Parse(&new_settings, &settings_, serial_ + 1)) {
    fprintf(stderr, "%s: Unable to parse %s: %s\n",
            kProgName, filename.c_str(), parser.FormatError().c_str());
    return false;
  }
  serial_++;
  settings_.swap(&new_settings);
  return true;
}

bool SettingsManager::InitX11(bool replace_existing_manager) {
  assert(!display_);
  display_ = XOpenDisplay(NULL);
  if (!display_) {
    fprintf(stderr, "%s: Unable to open connection to X server\n", kProgName);
    return false;
  }
  root_ = DefaultRootWindow(display_);

  // TODO: Handle multiple screens.
  atom_ = XInternAtom(display_, "_XSETTINGS_S0", False);

  XGrabServer(display_);
  Window prev_win = XGetSelectionOwner(display_, atom_);
  fprintf(stderr, "Selection is owned by 0x%x\n",
          static_cast<unsigned int>(prev_win));
  if (prev_win != None && !replace_existing_manager) {
    fprintf(stderr, "%s: Someone else already owns the _XSETTINGS_S0 selection "
            "and we weren't asked to replace them\n", kProgName);
    XUngrabServer(display_);
    return false;
  }

  win_ = CreateWindow();
  if (win_ == None) {
    fprintf(stderr, "Unable to create window\n");
    return false;
  }
  fprintf(stderr, "Created window 0x%x\n", static_cast<unsigned int>(win_));

  if (!UpdateProperty(win_)) {
    fprintf(stderr, "Unable to update settings property on window\n");
    return false;
  }

  if (prev_win)
    XSelectInput(display_, prev_win, StructureNotifyMask);
  XSetSelectionOwner(display_, atom_, win_, CurrentTime);
  fprintf(stderr, "Took ownership of selection\n");
  XUngrabServer(display_);

  if (prev_win) {
    // Wait for the previous owner to go away.
    XEvent event;
    while (true) {
      XWindowEvent(display_, prev_win, StructureNotifyMask, &event);
      if (event.type == DestroyNotify)
        break;
    }
  }

  // Make sure that no one else took the selection while we were waiting.
  if (XGetSelectionOwner(display_, atom_) != win_) {
    fprintf(stderr, "%s: Someone else took ownership of the _XSETTINGS_S0 "
            "selection\n", kProgName);
    return false;
  }

  XEvent ev;
  ev.xclient.type = ClientMessage;
  ev.xclient.window = root_;
  ev.xclient.message_type = XInternAtom(display_, "MANAGER", False);
  ev.xclient.format = 32;
  ev.xclient.data.l[0] = CurrentTime;  // FIXME
  ev.xclient.data.l[1] = atom_;
  ev.xclient.data.l[2] = win_;
  ev.xclient.data.l[3] = 0;
  XSendEvent(display_,
             root_,
             False,                // propagate
             StructureNotifyMask,  // event_mask
             &ev);

  return true;
}

void SettingsManager::RunEventLoop() {
  while (true) {
    XEvent event;
    XNextEvent(display_, &event);

    switch (event.type) {
      case MappingNotify:
        // Doesn't really mean anything to us, but might as well handle it.
        XRefreshKeyboardMapping(&(event.xmapping));
        break;
      case SelectionClear: {
        const XSelectionClearEvent& e = event.xselectionclear;
        if (e.selection == atom_ && e.window == win_) {
          // If someone else took the selection, that's our sign to leave.
          XDestroyWindow(display_, win_);
          win_ = None;
          return;
        }
        fprintf(stderr, "%s: Ignoring SelectionClear event with atom 0x%x "
                "and window 0x%x\n", kProgName,
                static_cast<unsigned int>(e.selection),
                static_cast<unsigned int>(e.window));
        break;
      }
      default:
        fprintf(stderr, "%s: Ignoring event of type %d\n",
                kProgName, event.type);
        break;
    }
  }
}

Window SettingsManager::CreateWindow() {
  XSetWindowAttributes attr;
  attr.override_redirect = True;
  Window win = XCreateWindow(display_,
                             root_,               // parent
                             -1, -1,              // x, y
                             1, 1,                // width, height
                             0,                   // border_width
                             CopyFromParent,      // depth
                             InputOutput,         // class
                             CopyFromParent,      // visual
                             CWOverrideRedirect,  // attr_mask
                             &attr);

  // This sets a few properties for us, including WM_CLIENT_MACHINE.
  XSetWMProperties(display_,
                   win,
                   NULL,   // window_name
                   NULL,   // icon_name
                   NULL,   // argv
                   0,      // argc
                   NULL,   // normal_hints
                   NULL,   // wm_hints
                   NULL);  // class_hints

  XStoreName(display_, win, kProgName);
  XChangeProperty(display_,
                  win,
                  XInternAtom(display_, "_NET_WM_NAME", False),  // property
                  XInternAtom(display_, "UTF8_STRING", False),   // type
                  8,  // format (bits per element)
                  PropModeReplace,
                  reinterpret_cast<const unsigned char*>(kProgName),
                  strlen(kProgName));

  pid_t pid = getpid();
  XChangeProperty(display_,
                  win,
                  XInternAtom(display_, "_NET_WM_PID", False),  // property
                  XA_CARDINAL,  // type
                  32,           // format (bits per element)
                  PropModeReplace,
                  reinterpret_cast<const unsigned char*>(&pid), // value
                  1);           // num elements

  return win;
}

bool SettingsManager::UpdateProperty(Window win) {
  static const int kBufferSize = 8192;
  char buffer[kBufferSize];
  DataWriter writer(buffer, sizeof(buffer));

  int byte_order = IsLittleEndian() ? LSBFirst : MSBFirst;

  if (!writer.WriteInt8(byte_order))              return false;
  if (!writer.WriteZeros(3))                      return false;
  if (!writer.WriteInt32(serial_))                return false;
  if (!writer.WriteInt32(settings_.map().size())) return false;

  for (SettingsMap::Map::const_iterator it = settings_.map().begin();
       it != settings_.map().end(); ++it) {
    if (!it->second->Write(it->first, &writer))
      return false;
  }

  XChangeProperty(display_,
                  win,
                  atom_,  // property
                  atom_,  // type
                  8,      // format (bits per element)
                  PropModeReplace,
                  reinterpret_cast<unsigned char*>(buffer),
                  writer.bytes_written());
  return true;
}

}  // namespace xsettingsd
