// Copyright 2009 Daniel Erat <dan@erat.org>
// All rights reserved.

#include "settings_manager.h"

#include <cassert>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/types.h>
#include <unistd.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

#include "config_parser.h"
#include "data_writer.h"
#include "setting.h"

using std::make_pair;
using std::map;
using std::max;
using std::string;
using std::vector;

namespace xsettingsd {

// Arbitrarily big number.
static const int kMaxPropertySize = (2 << 15);

SettingsManager::SettingsManager(const string& config_filename)
    : config_filename_(config_filename),
      serial_(0),
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

bool SettingsManager::LoadConfig() {
  ConfigParser parser(new ConfigParser::FileCharStream(config_filename_));
  SettingsMap new_settings;
  if (!parser.Parse(&new_settings, &settings_, serial_ + 1)) {
    fprintf(stderr, "%s: Unable to parse %s: %s\n",
            kProgName, config_filename_.c_str(), parser.FormatError().c_str());
    return false;
  }
  serial_++;
  fprintf(stderr, "%s: Loaded %d setting%s from %s\n",
          kProgName, new_settings.map().size(),
          (new_settings.map().size() == 1) ? "" : "s",
          config_filename_.c_str());
  settings_.swap(&new_settings);
  return true;
}

bool SettingsManager::InitX11(int screen, bool replace_existing_manager) {
  assert(!display_);
  display_ = XOpenDisplay(NULL);
  if (!display_) {
    fprintf(stderr, "%s: Unable to open connection to X server\n", kProgName);
    return false;
  }

  prop_atom_ = XInternAtom(display_, "_XSETTINGS_SETTINGS", False);

  char data[kMaxPropertySize];
  DataWriter writer(data, kMaxPropertySize);
  if (!WriteProperty(&writer))
    return false;

  int min_screen = 0;
  int max_screen = ScreenCount(display_) - 1;
  if (screen >= 0)
    min_screen = max_screen = screen;

  for (screen = min_screen; screen <= max_screen; ++screen) {
    Window win = None;
    Time timestamp = 0;
    if (!CreateWindow(screen, &win, &timestamp)) {
      fprintf(stderr, "%s: Unable to create window on screen %d\n",
              kProgName, screen);
      return false;
    }
    fprintf(stderr, "%s: Created window 0x%x on screen %d with timestamp %lu\n",
            kProgName, static_cast<unsigned int>(win), screen, timestamp);

    SetPropertyOnWindow(win, data, writer.bytes_written());

    if (!ManageScreen(screen, win, timestamp, replace_existing_manager))
      return false;

    windows_.push_back(win);
  }

  return true;
}

void SettingsManager::RunEventLoop() {
  int x11_fd = XConnectionNumber(display_);
  // TODO: Need to also use XAddConnectionWatch()?

  while (true) {
    // Rather than blocking in XNextEvent(), we just read all the available
    // events here.  We block in select() instead, so that we'll get EINTR
    // if a SIGHUP came in to ask us to reload the config.
    while (XPending(display_)) {
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
      }
    }

    // TODO: There's a small race condition here, in that SIGHUP can come
    // in while we're outside of the select() call, but it's probably not
    // worth trying to work around.

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(x11_fd, &fds);
    if (select(x11_fd + 1, &fds, NULL, NULL, NULL) == -1) {
      if (errno != EINTR) {
        fprintf(stderr, "%s: select() failed: %s\n",
                kProgName, strerror(errno));
        return;
      }

      fprintf(stderr, "%s: Reloading configuration\n", kProgName);
      if (!LoadConfig())
        continue;

      char data[kMaxPropertySize];
      DataWriter writer(data, kMaxPropertySize);
      if (!WriteProperty(&writer))
        continue;

      for (vector<Window>::const_iterator it = windows_.begin();
           it != windows_.end(); ++it) {
        SetPropertyOnWindow(*it, data, writer.bytes_written());
      }
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

bool SettingsManager::CreateWindow(int screen,
                                   Window* win_out,
                                   Time* timestamp_out) {
  assert(win_out);
  assert(timestamp_out);

  if (screen < 0 || screen >= ScreenCount(display_))
    return false;

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
  if (win == None)
    return false;
  *win_out = win;

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

  // Grab a timestamp from our final property change; we'll need it later
  // when announcing that we've taken the manager selection.
  pid_t pid = getpid();
  XSelectInput(display_, win, PropertyChangeMask);
  XChangeProperty(display_,
                  win,
                  XInternAtom(display_, "_NET_WM_PID", False),  // property
                  XA_CARDINAL,  // type
                  32,           // format (bits per element)
                  PropModeReplace,
                  reinterpret_cast<const unsigned char*>(&pid), // value
                  1);           // num elements
  XSelectInput(display_, win, NoEventMask);

  XEvent event;
  while (true) {
    XWindowEvent(display_, win, PropertyChangeMask, &event);
    if (event.type == PropertyNotify) {
      *timestamp_out = event.xproperty.time;
      break;
    }
  }

  return true;
}

bool SettingsManager::WriteProperty(DataWriter* writer) {
  assert(writer);

  int byte_order = IsLittleEndian() ? LSBFirst : MSBFirst;
  if (!writer->WriteInt8(byte_order))              return false;
  if (!writer->WriteZeros(3))                      return false;
  if (!writer->WriteInt32(serial_))                return false;
  if (!writer->WriteInt32(settings_.map().size())) return false;

  for (SettingsMap::Map::const_iterator it = settings_.map().begin();
       it != settings_.map().end(); ++it) {
    if (!it->second->Write(it->first, writer))
      return false;
  }
  return true;
}

void SettingsManager::SetPropertyOnWindow(
    Window win, const char* data, size_t size) {
  XChangeProperty(display_,
                  win,
                  prop_atom_,  // property
                  prop_atom_,  // type
                  8,           // format (bits per element)
                  PropModeReplace,
                  reinterpret_cast<const unsigned char*>(data),
                  size);
}

bool SettingsManager::ManageScreen(int screen,
                                   Window win,
                                   Time timestamp,
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
  ev.xclient.data.l[0] = timestamp;
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
