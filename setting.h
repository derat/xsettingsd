#ifndef __XSETTINGSD_SETTING_H__
#define __XSETTINGSD_SETTING_H__

#include <map>
#include <stdint.h>
#include <string>

#include "common.h"

namespace xsettingsd {

class DataWriter;

class Setting {
 public:
  enum Type {
    TYPE_INTEGER = 0,
    TYPE_STRING  = 1,
    TYPE_COLOR   = 2,
  };

  explicit Setting(Type type)
      : type_(type),
        serial_(0) {
  }
  virtual ~Setting() {}

  bool operator==(const Setting& other) const;

  // Write this setting (using the passed-in setting name) in the format
  // described in the XSETTINGS spec.
  bool Write(const std::string& name, DataWriter* writer) const;

  // Update this setting's serial number based on the previous version of
  // the setting.  (If the setting changed, we'll increment the serial;
  // otherwise we use the same serial as before.)
  void UpdateSerial(Setting& prev);

 protected:
  // Swiped from xsettings-common.h in Owen Taylor's reference
  // implementation.
  static int GetPadding(int n, int m) {
    return ((n + m - 1) & (~(m - 1)));
  }

 private:
  // Write type-specific data.
  virtual bool WriteBody(DataWriter* writer) const = 0;

  // Cast 'other' to this setting's type and compare it.
  virtual bool EqualsImpl(const Setting& other) const = 0;

  Type type_;

  // Incremented when the setting's value changes.
  uint32_t serial_;

  DISALLOW_COPY_AND_ASSIGN(Setting);
};

class IntegerSetting : public Setting {
 public:
  explicit IntegerSetting(int32_t value)
      : Setting(TYPE_INTEGER),
        value_(value) {
  }

 private:
  bool WriteBody(DataWriter* writer) const;
  bool EqualsImpl(const Setting& other) const;

  int32_t value_;

  DISALLOW_COPY_AND_ASSIGN(IntegerSetting);
};

class StringSetting : public Setting {
 public:
  explicit StringSetting(const std::string& value)
      : Setting(TYPE_STRING),
        value_(value) {
  }

 private:
  bool WriteBody(DataWriter* writer) const;
  bool EqualsImpl(const Setting& other) const;

  std::string value_;

  DISALLOW_COPY_AND_ASSIGN(StringSetting);
};

class ColorSetting : public Setting {
 public:
  ColorSetting(uint16_t red,
               uint16_t blue,
               uint16_t green,
               uint16_t alpha)
      : Setting(TYPE_COLOR),
        red_(red),
        blue_(blue),
        green_(green),
        alpha_(alpha) {
  }

 private:
  bool WriteBody(DataWriter* writer) const;
  bool EqualsImpl(const Setting& other) const;

  uint16_t red_;
  uint16_t blue_;
  uint16_t green_;
  uint16_t alpha_;

  DISALLOW_COPY_AND_ASSIGN(ColorSetting);
};

// A simple wrapper around a string-to-Setting map.
// Handles deleting the Setting objects in its d'tor.
class SettingsMap {
 public:
  SettingsMap() {}
  ~SettingsMap();

  typedef std::map<std::string, Setting*> Map;
  const Map& map() const { return map_; }
  Map* mutable_map() { return &map_; }

  // Update settings' serial numbers based on the previous map.
  void SetSerials(const SettingsMap& prev_map);

 private:
  Map map_;

  DISALLOW_COPY_AND_ASSIGN(SettingsMap);
};

}  // namespace xsettingsd

#endif
