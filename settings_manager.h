// Copyright 2009 Daniel Erat <dan@erat.org>
// All rights reserved.

#ifndef __XSETTINGSD_SETTINGS_MANAGER_H__
#define __XSETTINGSD_SETTINGS_MANAGER_H__

#include <stdint.h>
#include <string>

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

  // Connect to the X server, create our window, and take the selection.
  // Returns false if someone else already has the selection unless
  // 'replace_existing_manager' is set.
  bool InitX11(bool replace_existing_manager);

  // Wait for events from the X server, exiting if we see someone else take
  // the selection.
  void RunEventLoop();

 private:
  // Create and initialize a window.
  Window CreateWindow();

  // Update the settings property on the passed-in window.
  bool UpdateProperty(Window win);

  // Currently-loaded settings.
  SettingsMap settings_;

  // Current serial number.
  uint32_t serial_;

  Display* display_;
  Window root_;
  Atom sel_atom_;
  Atom prop_atom_;
  Window win_;

  DISALLOW_COPY_AND_ASSIGN(SettingsManager);
};

}  // namespace xsettingsd

#endif
