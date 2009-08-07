// Copyright 2009 Daniel Erat <dan@erat.org>
// All rights reserved.

#ifndef __XSETTINGSD_SETTINGS_MANAGER_H__
#define __XSETTINGSD_SETTINGS_MANAGER_H__

#include <stdint.h>
#include <string>

#include "common.h"
#include "setting.h"

namespace xsettingsd {

class Setting;

class SettingsManager {
 public:
  SettingsManager();

  // Load settings from 'filename'.  If the load was unsuccessful, false is
  // returned and an error is printed to stderr.
  bool LoadConfig(const std::string& filename);

  bool UpdateProperty();

 private:
  // Currently-loaded settings.
  SettingsMap settings_;

  // Current serial number.
  uint32_t serial_;

  DISALLOW_COPY_AND_ASSIGN(SettingsManager);
};

}  // namespace xsettingsd

#endif
