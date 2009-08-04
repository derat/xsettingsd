#ifndef __XSETTINGSD_SETTING_H__
#define __XSETTINGSD_SETTING_H__

#include <map>
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

  Setting(Type type)
      : type_(type),
        serial_(0) {
  }
  virtual ~Setting() {}

  bool Write(const std::string& name, DataWriter* writer) const;

 protected:
  // Swiped from xsettings-common.h in Owen Taylor's reference
  // implementation.
  static int GetPadding(int n, int m) {
    return ((n + m - 1) & (~(m - 1)));
  }

 private:
  virtual bool WriteBody(DataWriter* writer) const = 0;

  Type type_;

  uint32 serial_;
};

class IntegerSetting : public Setting {
 public:
  IntegerSetting(int32 value)
      : Setting(TYPE_INTEGER),
        value_(value) {
  }

 private:
  bool WriteBody(DataWriter* writer) const;

  int32 value_;
};

class StringSetting : public Setting {
 public:
  StringSetting(const std::string& value)
      : Setting(TYPE_STRING),
        value_(value) {
  }

 private:
  bool WriteBody(DataWriter* writer) const;

  std::string value_;
};

class ColorSetting : public Setting {
 public:
  ColorSetting(uint16 red,
               uint16 blue,
               uint16 green,
               uint16 alpha)
      : Setting(TYPE_COLOR),
        red_(red),
        blue_(blue),
        green_(green),
        alpha_(alpha) {
  }

 private:
  bool WriteBody(DataWriter* writer) const;

  uint16 red_;
  uint16 blue_;
  uint16 green_;
  uint16 alpha_;
};

class SettingsMap {
 public:
  ~SettingsMap();

  typedef std::map<std::string, Setting*> Map;
  const Map& map() const { return map_; }
  Map* mutable_map() { return &map_; }

 private:
  Map map_;
};

}  // namespace xsettingsd

#endif
