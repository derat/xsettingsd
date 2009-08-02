#include "settings_manager.h"

#include "data_writer.h"
#include "setting.h"

using std::make_pair;
using std::map;
using std::string;

namespace xsettingsd {

SettingsManager::SettingsManager()
    : serial_(0) {
  // FIXME: Just for testing.
  settings_.insert(make_pair("int", new IntegerSetting("int", 5)));
  settings_.insert(make_pair("str", new StringSetting("str", "value")));
  settings_.insert(make_pair("col", new ColorSetting("col", 1, 2, 3, 4)));
}

SettingsManager::~SettingsManager() {
  for (map<string, Setting*>::iterator it = settings_.begin();
       it != settings_.end(); ++it) {
    delete it->second;
  }
  settings_.clear();
}

bool SettingsManager::UpdateProperty() {
  static const int kBufferSize = 8192;
  char buffer[kBufferSize];
  DataWriter writer(buffer, sizeof(buffer));

  // FIXME: First field is supposed to be byte-order.
  if (!writer.WriteInt8(0))                 return false;
  if (!writer.WriteZeros(3))                return false;
  if (!writer.WriteInt32(serial_))          return false;
  if (!writer.WriteInt32(settings_.size())) return false;

  for (map<string, Setting*>::const_iterator it = settings_.begin();
       it != settings_.end(); ++it) {
    if (!it->second->Write(&writer))
      return false;
  }

  return true;
}

}  // namespace xsettingsd
