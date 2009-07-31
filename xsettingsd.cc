#include <cassert>
#include <cstring>
#include <strings.h>

#include "xsettingsd.h"

using namespace std;

DataWriter::DataWriter(char* buffer, size_t buf_len)
    : buffer_(buffer),
      buf_len_(buf_len),
      bytes_written_(0) {
}

bool DataWriter::WriteBytes(const char* data, size_t bytes_to_write) {
  if (bytes_to_write > buf_len_ - bytes_written_)
    return false;

  memcpy(buffer_ + bytes_written_,
         data,
         min(bytes_to_write, buf_len_ - bytes_written_));
  bytes_written_ += bytes_to_write;
  return true;
}

bool DataWriter::WriteInt8(int8 num) {
  if (sizeof(int8) > buf_len_ - bytes_written_)
    return false;

  *(reinterpret_cast<int8*>(buffer_ + bytes_written_)) = num;
  return true;
}

bool DataWriter::WriteInt16(int16 num) {
  if (sizeof(int16) > buf_len_ - bytes_written_)
    return false;

  *(reinterpret_cast<int16*>(buffer_ + bytes_written_)) = num;
  return true;
}

bool DataWriter::WriteInt32(int32 num) {
  if (sizeof(int32) > buf_len_ - bytes_written_)
    return false;

  *(reinterpret_cast<int32*>(buffer_ + bytes_written_)) = num;
  return true;
}

bool DataWriter::WriteZeros(size_t bytes_to_write) {
  if (bytes_to_write > buf_len_ - bytes_written_)
    return false;

  bzero(buffer_ + bytes_written_,
        min(bytes_to_write, buf_len_ - bytes_written_));
  bytes_written_ += bytes_to_write;
  return true;
}


SettingsManager::SettingsManager()
    : serial_(0) {
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


bool SettingsManager::Setting::Write(DataWriter* writer) const {
  if (!writer->WriteInt8(type_))                        return false;
  if (!writer->WriteZeros(1))                           return false;
  if (!writer->WriteInt16(name_.size()))                return false;
  if (!writer->WriteBytes(name_.data(), name_.size()))  return false;
  if (!writer->WriteZeros(GetPadding(name_.size(), 4))) return false;
  if (!writer->WriteInt32(serial_))                     return false;

  return WriteBody(writer);
}


bool SettingsManager::IntegerSetting::WriteBody(DataWriter* writer) const {
  return writer->WriteInt32(value_);
}


bool SettingsManager::StringSetting::WriteBody(DataWriter* writer) const {
  if (!writer->WriteInt32(value_.size()))                return false;
  if (!writer->WriteBytes(value_.data(), value_.size())) return false;
  if (!writer->WriteZeros(GetPadding(value_.size(), 4))) return false;
  return true;
}


bool SettingsManager::ColorSetting::WriteBody(DataWriter* writer) const {
  if (!writer->WriteInt16(red_))   return false;
  if (!writer->WriteInt16(blue_))  return false;
  if (!writer->WriteInt16(green_)) return false;
  if (!writer->WriteInt16(alpha_)) return false;
  return true;
}


int main(int argc, char** argv) {
  return 0;
}
