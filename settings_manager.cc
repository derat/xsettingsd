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
using std::vector;

namespace xsettingsd {

SettingsManager::SettingsManager()
    : serial_(0),
      display_(NULL),
      prop_atom_(None) {
}

SettingsManager::~SettingsManager() {
  if (display_) {
    if (!windows_.empty())
      DestroyWindows();
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

  prop_atom_ = XInternAtom(display_, "_XSETTINGS_SETTINGS", False);

  for (int screen = 0; screen < ScreenCount(display_); ++screen) {
    Window win = CreateWindow(screen);
    if (win == None) {
      fprintf(stderr, "%s: Unable to create window on screen %d\n",
              kProgName, screen);
      return false;
    }
    fprintf(stderr, "%s: Created window 0x%x on screen %d\n",
            kProgName, static_cast<unsigned int>(win), screen);

    if (!UpdateProperty(win)) {
      fprintf(stderr, "%s: Unable to update settings property on window\n",
              kProgName);
      return false;
    }

    if (!ManageScreen(screen, win, replace_existing_manager))
      return false;

    windows_.push_back(win);
  }

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
        // If someone else took the selection, that's our sign to leave.
        fprintf(stderr, "%s: 0x%x took a selection from us; exiting\n",
                kProgName,
                static_cast<unsigned int>(event.xselectionclear.window));
        DestroyWindows();
        return;
      }
      default:
        fprintf(stderr, "%s: Ignoring event of type %d\n",
                kProgName, event.type);
        break;
    }
  }
}

void SettingsManager::DestroyWindows() {
  assert(display_);
  for (vector<Window>::iterator it = windows_.begin();
       it != windows_.end(); ++it) {
    XDestroyWindow(display_, *it);
  }
  windows_.clear();
}

Window SettingsManager::CreateWindow(int screen) {
  XSetWindowAttributes attr;
  attr.override_redirect = True;
  Window win = XCreateWindow(display_,
                             RootWindow(display_, screen),  // parent
                             -1, -1,                        // x, y
                             1, 1,                          // width, height
                             0,                             // border_width
                             CopyFromParent,                // depth
                             InputOutput,                   // class
                             CopyFromParent,                // visual
                             CWOverrideRedirect,            // attr_mask
                             &attr);
  if (win == None) {
    fprintf(stderr, "%s: Unable to create window on screen %d\n",
            kProgName, screen);
    return false;
  }

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
                  prop_atom_,  // property
                  prop_atom_,  // type
                  8,           // format (bits per element)
                  PropModeReplace,
                  reinterpret_cast<unsigned char*>(buffer),
                  writer.bytes_written());
  return true;
}

bool SettingsManager::ManageScreen(int screen,
                                   Window win,
                                   bool replace_existing_manager) {
  assert(display_);
  assert(win != None);
  assert(screen < ScreenCount(display_));

  Window root = RootWindow(display_, screen);

  string sel_atom_name = StringPrintf("_XSETTINGS_S%d", screen);
  Atom sel_atom = XInternAtom(display_, sel_atom_name.c_str(), False);

  XGrabServer(display_);
  Window prev_win = XGetSelectionOwner(display_, sel_atom);
  fprintf(stderr, "%s: Selection %s is owned by 0x%x\n",
          kProgName, sel_atom_name.c_str(),
          static_cast<unsigned int>(prev_win));
  if (prev_win != None && !replace_existing_manager) {
    fprintf(stderr, "%s: Someone else already owns the %s selection "
            "and we weren't asked to replace them\n",
            kProgName, sel_atom_name.c_str());
    XUngrabServer(display_);
    return false;
  }

  if (prev_win)
    XSelectInput(display_, prev_win, StructureNotifyMask);
  XSetSelectionOwner(display_, sel_atom, win, CurrentTime);
  fprintf(stderr, "%s: Took ownership of selection %s\n",
          kProgName, sel_atom_name.c_str());
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
  if (XGetSelectionOwner(display_, sel_atom) != win) {
    fprintf(stderr, "%s: Someone else took ownership of the %s selection\n",
            kProgName, sel_atom_name.c_str());
    return false;
  }

  XEvent ev;
  ev.xclient.type = ClientMessage;
  ev.xclient.window = root;
  ev.xclient.message_type = XInternAtom(display_, "MANAGER", False);
  ev.xclient.format = 32;
  ev.xclient.data.l[0] = CurrentTime;  // FIXME
  ev.xclient.data.l[1] = sel_atom;
  ev.xclient.data.l[2] = win;
  ev.xclient.data.l[3] = 0;
  XSendEvent(display_,
             root,
             False,                // propagate
             StructureNotifyMask,  // event_mask
             &ev);

  return true;
}

}  // namespace xsettingsd
