// Copyright 2009 Daniel Erat <dan@erat.org>
// All rights reserved.

#ifndef __XSETTINGSD_SETTINGS_MANAGER_H__
#define __XSETTINGSD_SETTINGS_MANAGER_H__

#include <stdint.h>
#include <string>
#include <vector>

#include <X11/Xlib.h>

#include "common.h"
#include "setting.h"

namespace xsettingsd {

class Setting;

class SettingsManager {
 public:
  SettingsManager();
  ~SettingsManager();

  // Load settings from 'filename'.  If the load was unsuccessful, false is
  // returned and an error is printed to stderr.
  bool LoadConfig(const std::string& filename);

  // Connect to the X server, create windows, updates their properties, and
  // take the selections.  Returns false if someone else already has a
  // selection unless 'replace_existing_manager' is set.
  bool InitX11(bool replace_existing_manager);

  // Wait for events from the X server, destroying our windows and exiting
  // if we see someone else take a selection.
  void RunEventLoop();

 private:
  // Destroy all windows in 'windows_'.
  void DestroyWindows();

  // Create and initialize a window.
  Window CreateWindow(int screen);

  // Update the settings property on the passed-in window.
  bool UpdateProperty(Window win);

  // Manage XSETTINGS for a particular screen.
  bool ManageScreen(int screen, Window win, bool replace_existing_manager);

  // Currently-loaded settings.
  SettingsMap settings_;

  // Current serial number.
  uint32_t serial_;

  // Connection to the X server.
  Display* display_;

  // Atom representing "_XSETTINGS_SETTINGS".
  Atom prop_atom_;

  // Windows that we've created to hold settings properties (one per
  // screen).
  std::vector<Window> windows_;

  DISALLOW_COPY_AND_ASSIGN(SettingsManager);
};

}  // namespace xsettingsd

#endif
