#include <cassert>
#include <cstring>
#include <map>
#include <string>
#include <strings.h>

using namespace std;

// FIXME: Fix this for 64-bit.
typedef char  int8;
typedef short int16;
typedef int   int32;
typedef unsigned short uint16;


class DataWriter {
 public:
  DataWriter(char* buffer, size_t buf_len)
      : buffer_(buffer),
        buf_len_(buf_len),
        bytes_written_(0) {
  }

  size_t bytes_written() const { return bytes_written_; }

  bool WriteBytes(const char* data, size_t bytes_to_write) {
    if (bytes_to_write > buf_len_ - bytes_written_)
      return false;

    memcpy(buffer_ + bytes_written_,
           data,
           min(bytes_to_write, buf_len_ - bytes_written_));
    bytes_written_ += bytes_to_write;
    return true;
  }

  bool WriteInt8(int8 num) {
    if (sizeof(int8) > buf_len_ - bytes_written_)
      return false;

    *(reinterpret_cast<int8*>(buffer_ + bytes_written_)) = num;
    return true;
  }

  bool WriteInt16(int16 num) {
    if (sizeof(int16) > buf_len_ - bytes_written_)
      return false;

    *(reinterpret_cast<int16*>(buffer_ + bytes_written_)) = num;
    return true;
  }

  bool WriteInt32(int32 num) {
    if (sizeof(int32) > buf_len_ - bytes_written_)
      return false;

    *(reinterpret_cast<int32*>(buffer_ + bytes_written_)) = num;
    return true;
  }

  bool WriteZeros(size_t bytes_to_write) {
    if (bytes_to_write > buf_len_ - bytes_written_)
      return false;

    bzero(buffer_ + bytes_written_,
          min(bytes_to_write, buf_len_ - bytes_written_));
    bytes_written_ += bytes_to_write;
    return true;
  }

 private:
  char* buffer_;  // not owned

  size_t buf_len_;

  size_t bytes_written_;
};


class SettingsManager {
 public:

 private:
  class Setting {
   public:
    enum Type {
      TYPE_INTEGER = 0,
      TYPE_STRING  = 1,
      TYPE_COLOR   = 2,
    };

    Setting(const string& name, Type type)
        : name_(name),
          type_(type),
          serial_(0) {
    }
    virtual ~Setting() {}

    const string& name() const { return name_; }

    bool WriteToBuffer(DataWriter* writer) {
      if (!writer->WriteInt8(type_))                        return false;
      if (!writer->WriteZeros(1))                           return false;
      if (!writer->WriteInt16(name_.size()))                return false;
      if (!writer->WriteBytes(name_.data(), name_.size()))  return false;
      if (!writer->WriteZeros(GetPadding(name_.size(), 4))) return false;
      if (!writer->WriteInt32(serial_))                     return false;

      return WriteBodyToBuffer(writer);
    }

   protected:
    // Swiped from xsettings-common.h in Owen Taylor's reference
    // implementation.
    static int GetPadding(int n, int m) {
      return ((n + m - 1) & (~(m - 1)));
    }

   private:
    virtual bool WriteBodyToBuffer(DataWriter* writer) = 0;

    string name_;

    Type type_;

    int serial_;
  };

  class IntegerSetting : public Setting {
   public:
    IntegerSetting(const string& name, int32 value)
        : Setting(name, TYPE_INTEGER),
          value_(value) {}

   private:
    bool WriteBodyToBuffer(DataWriter* writer) {
      return writer->WriteInt32(value_);
    }

    int32 value_;
  };

  class StringSetting : public Setting {
   public:
    StringSetting(const string& name, string value)
        : Setting(name, TYPE_STRING),
          value_(value) {}

   private:
    bool WriteBodyToBuffer(DataWriter* writer) {
      if (!writer->WriteInt32(value_.size()))                return false;
      if (!writer->WriteBytes(value_.data(), value_.size())) return false;
      if (!writer->WriteZeros(GetPadding(value_.size(), 4))) return false;
      return true;
    }

    string value_;
  };

  class ColorSetting : public Setting {
   public:
    ColorSetting(const string& name,
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
    bool WriteBodyToBuffer(DataWriter* writer) {
      if (!writer->WriteInt16(red_))   return false;
      if (!writer->WriteInt16(blue_))  return false;
      if (!writer->WriteInt16(green_)) return false;
      if (!writer->WriteInt16(alpha_)) return false;
      return true;
    }

    uint16 red_;
    uint16 blue_;
    uint16 green_;
    uint16 alpha_;
  };

  map<string, Setting*> settings_;
};


int main(int argc, char** argv) {
  return 0;
}
