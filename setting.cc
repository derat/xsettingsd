// Copyright 2009 Daniel Erat <dan@erat.org>
// All rights reserved.

#include "setting.h"

#include "data_writer.h"

using std::string;

namespace xsettingsd {

bool Setting::operator==(const Setting& other) const {
  if (other.type_ != type_)
    return false;
  return EqualsImpl(other);
}

bool Setting::Write(const string& name, DataWriter* writer) const {
  if (!writer->WriteInt8(type_))                       return false;
  if (!writer->WriteZeros(1))                          return false;
  if (!writer->WriteInt16(name.size()))                return false;
  if (!writer->WriteBytes(name.data(), name.size()))   return false;
  if (!writer->WriteZeros(GetPadding(name.size(), 4))) return false;
  if (!writer->WriteInt32(serial_))                    return false;

  return WriteBody(writer);
}

void Setting::UpdateSerial(const Setting* prev, uint32_t serial) {
  if (prev && operator==(*prev))
    serial_ = prev->serial_;
  else
    serial_ = serial;
}

bool IntegerSetting::WriteBody(DataWriter* writer) const {
  return writer->WriteInt32(value_);
}

bool IntegerSetting::EqualsImpl(const Setting& other) const {
  const IntegerSetting* cast_other =
      dynamic_cast<const IntegerSetting*>(&other);
  if (!cast_other)
    return false;
  return (cast_other->value_ == value_);
}

bool StringSetting::WriteBody(DataWriter* writer) const {
  if (!writer->WriteInt32(value_.size()))                return false;
  if (!writer->WriteBytes(value_.data(), value_.size())) return false;
  if (!writer->WriteZeros(GetPadding(value_.size(), 4))) return false;
  return true;
}

bool StringSetting::EqualsImpl(const Setting& other) const {
  const StringSetting* cast_other = dynamic_cast<const StringSetting*>(&other);
  if (!cast_other)
    return false;
  return (cast_other->value_ == value_);
}

bool ColorSetting::WriteBody(DataWriter* writer) const {
  // Note that XSETTINGS asks for RBG-order, not RGB.
  if (!writer->WriteInt16(red_))   return false;
  if (!writer->WriteInt16(blue_))  return false;
  if (!writer->WriteInt16(green_)) return false;
  if (!writer->WriteInt16(alpha_)) return false;
  return true;
}

bool ColorSetting::EqualsImpl(const Setting& other) const {
  const ColorSetting* cast_other = dynamic_cast<const ColorSetting*>(&other);
  if (!cast_other)
    return false;
  return (cast_other->red_ == red_ &&
          cast_other->green_ == green_ &&
          cast_other->blue_ == blue_ &&
          cast_other->alpha_ == alpha_);
}

SettingsMap::~SettingsMap() {
  for (Map::iterator it = map_.begin(); it != map_.end(); ++it) {
    delete it->second;
  }
  map_.clear();
}

const Setting* SettingsMap::GetSetting(const std::string& name) const {
  Map::const_iterator it = map_.find(name);
  if (it == map_.end())
    return NULL;
  return it->second;
}

}  // namespace xsettingsd
