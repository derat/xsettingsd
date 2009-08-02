#include "setting.h"

#include "data_writer.h"

namespace xsettingsd {

bool Setting::Write(DataWriter* writer) const {
  if (!writer->WriteInt8(type_))                        return false;
  if (!writer->WriteZeros(1))                           return false;
  if (!writer->WriteInt16(name_.size()))                return false;
  if (!writer->WriteBytes(name_.data(), name_.size()))  return false;
  if (!writer->WriteZeros(GetPadding(name_.size(), 4))) return false;
  if (!writer->WriteInt32(serial_))                     return false;

  return WriteBody(writer);
}

bool IntegerSetting::WriteBody(DataWriter* writer) const {
  return writer->WriteInt32(value_);
}

bool StringSetting::WriteBody(DataWriter* writer) const {
  if (!writer->WriteInt32(value_.size()))                return false;
  if (!writer->WriteBytes(value_.data(), value_.size())) return false;
  if (!writer->WriteZeros(GetPadding(value_.size(), 4))) return false;
  return true;
}

bool ColorSetting::WriteBody(DataWriter* writer) const {
  if (!writer->WriteInt16(red_))   return false;
  if (!writer->WriteInt16(blue_))  return false;
  if (!writer->WriteInt16(green_)) return false;
  if (!writer->WriteInt16(alpha_)) return false;
  return true;
}

}  // namespace xsettingsd
