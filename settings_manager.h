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
  SettingsManager(const std::string& config_filename);

  bool UpdateProperty();

 private:
  std::string config_filename_;

  SettingsMap settings_;

  uint32_t serial_;

  DISALLOW_COPY_AND_ASSIGN(SettingsManager);
};

}  // namespace xsettingsd

#endif
