#ifndef __XSETTINGSD_SETTINGS_MANAGER_H__
#define __XSETTINGSD_SETTINGS_MANAGER_H__

#include <map>
#include <string>

#include "common.h"

namespace xsettingsd {

class Setting;

class SettingsManager {
 public:
  SettingsManager();
  ~SettingsManager();

  bool UpdateProperty();

 private:
  std::map<std::string, Setting*> settings_;

  uint32 serial_;
};

}  // namespace xsettingsd

#endif
