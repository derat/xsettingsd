#include <map>
#include <string>

using namespace std;

typedef int int32;
typedef unsigned short uint16;

class SettingsManager {
 public:

 private:
  class Setting {
   public:
    Setting(const string& name)
        : name_(name) {
    }
    virtual ~Setting() {}

    const string& name() const { return name_; }

    virtual bool AppendToBuffer(char* buffer,
                                size_t buf_len) = 0;

   private:
    string name_;
  };

  class IntegerSetting : public Setting {
   public:
    IntegerSetting(const string& name, int32 value)
        : Setting(name),
          value_(value) {}

    bool AppendToBuffer(char* buffer, size_t buf_len) {
      return true;
    }

   private:
    int32 value_;
  };

  class StringSetting : public Setting {
   public:
    StringSetting(const string& name, string value)
        : Setting(name),
          value_(value) {}

    bool AppendToBuffer(char* buffer, size_t buf_len) {
      return true;
    }

   private:
    string value_;
  };

  class ColorSetting : public Setting {
   public:
    ColorSetting(const string& name,
                 uint16 red,
                 uint16 blue,
                 uint16 green,
                 uint16 alpha)
        : Setting(name),
          red_(red),
          blue_(blue),
          green_(green),
          alpha_(alpha) {
    }

    bool AppendToBuffer(char* buffer, size_t buf_len) {
      return true;
    }

   private:
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
