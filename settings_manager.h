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

class DataWriter;

// SettingsManager is the central class responsible for loading and parsing
// configs (via ConfigParser), storing them (in the form of Setting
// objects), and setting them as properties on X11 windows.
class SettingsManager {
 public:
  SettingsManager(const std::string& config_filename);
  ~SettingsManager();

  // Load settings from 'config_filename_', updating 'settings_' and
  // 'serial_' if successful.  If the load was unsuccessful, false is
  // returned and an error is printed to stderr.
  bool LoadConfig();

  // Connect to the X server, create windows, updates their properties, and
  // take the selections.  A negative screen value will attempt to take the
  // manager selection on all screens.  Returns false if someone else
  // already has a selection unless 'replace_existing_manager' is set.
  bool InitX11(int screen, bool replace_existing_manager);

  // Wait for events from the X server, destroying our windows and exiting
  // if we see someone else take a selection.
  void RunEventLoop();

 private:
  // Destroy all windows in 'windows_'.
  void DestroyWindows();

  // Create and initialize a window.
  Window CreateWindow(int screen);

  // Write the currently-loaded property to the passed-in buffer.
  bool WriteProperty(DataWriter* writer);

  // Update the settings property on the passed-in window.
  void SetPropertyOnWindow(Window win, const char* data, size_t size);

  // Manage XSETTINGS for a particular screen.
  bool ManageScreen(int screen, Window win, bool replace_existing_manager);

  // File from which we load settings.
  std::string config_filename_;

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
