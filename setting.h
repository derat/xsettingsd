#ifndef __XSETTINGSD_SETTING_H__
#define __XSETTINGSD_SETTING_H__

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

  Setting(const std::string& name, Type type)
      : name_(name),
        type_(type),
        serial_(0) {
  }
  virtual ~Setting() {}

  const std::string& name() const { return name_; }

  bool Write(DataWriter* writer) const;

 protected:
  // Swiped from xsettings-common.h in Owen Taylor's reference
  // implementation.
  static int GetPadding(int n, int m) {
    return ((n + m - 1) & (~(m - 1)));
  }

 private:
  virtual bool WriteBody(DataWriter* writer) const = 0;

  std::string name_;

  Type type_;

  uint32 serial_;
};

class IntegerSetting : public Setting {
 public:
  IntegerSetting(const std::string& name, int32 value)
      : Setting(name, TYPE_INTEGER),
        value_(value) {
  }

 private:
  bool WriteBody(DataWriter* writer) const;

  int32 value_;
};

class StringSetting : public Setting {
 public:
  StringSetting(const std::string& name, const std::string& value)
      : Setting(name, TYPE_STRING),
        value_(value) {
  }

 private:
  bool WriteBody(DataWriter* writer) const;

  std::string value_;
};

class ColorSetting : public Setting {
 public:
  ColorSetting(const std::string& name,
               uint16 red,
               uint16 blue,
               uint16 green,
               uint16 alpha)
      : Setting(name, TYPE_COLOR),
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

}  // namespace xsettingsd

#endif
