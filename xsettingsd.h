#include <map>
#include <string>

using std::map;
using std::string;


// FIXME: Fix this for 64-bit.
typedef char  int8;
typedef short int16;
typedef int   int32;
typedef unsigned short uint16;
typedef unsigned int   uint32;


class DataWriter {
 public:
  DataWriter(char* buffer, size_t buf_len);

  size_t bytes_written() const { return bytes_written_; }

  bool WriteBytes(const char* data, size_t bytes_to_write);
  bool WriteInt8(int8 num);
  bool WriteInt16(int16 num);
  bool WriteInt32(int32 num);
  bool WriteZeros(size_t bytes_to_write);

 private:
  char* buffer_;  // not owned

  size_t buf_len_;

  size_t bytes_written_;
};


class SettingsManager {
 public:
  SettingsManager();

  bool UpdateProperty();

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

    bool Write(DataWriter* writer) const;

   protected:
    // Swiped from xsettings-common.h in Owen Taylor's reference
    // implementation.
    static int GetPadding(int n, int m) {
      return ((n + m - 1) & (~(m - 1)));
    }

   private:
    virtual bool WriteBody(DataWriter* writer) const = 0;

    string name_;

    Type type_;

    uint32 serial_;
  };

  class IntegerSetting : public Setting {
   public:
    IntegerSetting(const string& name, int32 value)
        : Setting(name, TYPE_INTEGER),
          value_(value) {
    }

   private:
    bool WriteBody(DataWriter* writer) const;

    int32 value_;
  };

  class StringSetting : public Setting {
   public:
    StringSetting(const string& name, string value)
        : Setting(name, TYPE_STRING),
          value_(value) {
    }

   private:
    bool WriteBody(DataWriter* writer) const;

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
    bool WriteBody(DataWriter* writer) const;

    uint16 red_;
    uint16 blue_;
    uint16 green_;
    uint16 alpha_;
  };

  map<string, Setting*> settings_;

  uint32 serial_;
};
