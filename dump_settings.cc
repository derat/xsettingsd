#include <cassert>
#include <cstdio>
#include <cstring>
#include <stdint.h>
#include <string>
#include <X11/Xlib.h>

#include "common.h"
#include "setting.h"

using std::min;
using std::string;

namespace xsettingsd {

bool GetData(char* buffer, size_t buffer_size, size_t* data_size) {
  assert(data_size);

  Display* display = XOpenDisplay(NULL);
  if (!display) {
    fprintf(stderr, "Couldn't open display\n");
    return false;
  }

  static const char* kSelName = "_XSETTINGS_S0";
  Atom sel_atom = XInternAtom(display, kSelName, False);
  Window win = XGetSelectionOwner(display, sel_atom);
  if (win == None) {
    fprintf(stderr, "No current owner for %s selection\n", kSelName);
    return false;
  }

  static const char* kPropName = "_XSETTINGS_SETTINGS";
  Atom prop_atom = XInternAtom(display, kPropName, False);
  Atom type_ret = None;
  int format_ret = 0;
  unsigned long num_items_ret = 0;
  unsigned long rem_bytes_ret = 0;
  unsigned char* prop_ret = NULL;
  int retval = XGetWindowProperty(display,
                                  win,
                                  prop_atom,
                                  0,                // offset
                                  buffer_size / 4,  // length (32-bit multiples)
                                  False,            // delete
                                  AnyPropertyType,  // type
                                  &type_ret,        // actual type
                                  &format_ret,      // actual format
                                  &num_items_ret,   // actual num items
                                  &rem_bytes_ret,   // remaining bytes
                                  &prop_ret);       // property
  if (retval != Success) {
    fprintf(stderr, "XGetWindowProperty() returned %d\n", retval);
    return false;
  }
  if (num_items_ret == 0) {
    fprintf(stderr, "Property %s doesn't exist on 0x%x\n",
            kPropName, static_cast<unsigned int>(win));
    return false;
  }
  if (rem_bytes_ret > 0) {
    fprintf(stderr, "Property %s on 0x%x is more than %d bytes (%lu remain)\n",
            kPropName, static_cast<unsigned int>(win), buffer_size,
            rem_bytes_ret);
    XFree(prop_ret);
    return false;
  }
  if (format_ret != 8) {
    fprintf(stderr, "Got unexpected format %d\n", format_ret);
    XFree(prop_ret);
    return false;
  }

  *data_size = min(buffer_size, static_cast<size_t>(num_items_ret));
  memcpy(buffer, prop_ret, *data_size);
  return true;
}

class DataReader {
 public:
  DataReader(const char* data, size_t data_size)
      : data_(data),
        data_size_(data_size),
        current_pos_(0) {
  }

  bool Read(string* str, size_t size) {
    if (current_pos_ + size > data_size_)
      return false;
    if (str)
      str->assign(data_ + current_pos_, size);
    current_pos_ += size;
    return true;
  }

 private:
  const char* data_;
  size_t data_size_;
  size_t current_pos_;
};

bool DumpSetting(DataReader* reader) {
  string type_str;
  if (!reader->Read(&type_str, 1)) {
    fprintf(stderr, "Unable to read setting type\n");
    return false;
  }
  Setting::Type type = static_cast<Setting::Type>(type_str[0]);

  if (type != Setting::TYPE_INTEGER && type != Setting::TYPE_STRING) {
    // TODO: Handle colors.
    fprintf(stderr, "Got setting with unhandled type %d\n", type);
    return false;
  }

  if (!reader->Read(NULL, 1)) {
    fprintf(stderr, "Unable to read 1-byte setting padding\n");
    return false;
  }

  string name_size_str;
  if (!reader->Read(&name_size_str, 2)) {
    fprintf(stderr, "Unable to read setting name size\n");
    return false;
  }
  uint16_t name_size = *reinterpret_cast<const uint16_t*>(name_size_str.data());

  string name;
  if (!reader->Read(&name, name_size)) {
    fprintf(stderr, "Unable to read %u-byte setting name\n", name_size);
    return false;
  }

  size_t name_padding = GetPadding(name_size, 4);
  if (!reader->Read(NULL, name_padding)) {
    fprintf(stderr, "Unable to read %d-byte setting name padding\n",
            name_padding);
    return false;
  }

  if (!reader->Read(NULL, 4)) {
    fprintf(stderr, "Unable to read setting serial number\n");
    return false;
  }

  if (type == Setting::TYPE_INTEGER) {
    string value_str;
    if (!reader->Read(&value_str, 4)) {
      fprintf(stderr, "Unable to read integer setting value\n");
      return false;
    }
    int32_t value = *reinterpret_cast<const int32_t*>(value_str.data());
    printf("%s %d\n", name.c_str(), value);

  } else if (type == Setting::TYPE_STRING) {
    string value_size_str;
    if (!reader->Read(&value_size_str, 4)) {
      fprintf(stderr, "Unable to read string setting value size\n");
      return false;
    }
    uint32_t value_size =
        *reinterpret_cast<const uint32_t*>(value_size_str.data());

    string value;
    if (!reader->Read(&value, value_size)) {
      fprintf(stderr, "Unable to read %u-byte string setting value\n",
              value_size);
      return false;
    }

    size_t value_padding = GetPadding(value_size, 4);
    if (!reader->Read(NULL, value_padding)) {
      fprintf(stderr, "Unable to read %d-byte string setting value padding\n",
              value_padding);
      return false;
    }

    string escaped_value;
    for (size_t i = 0; i < value.size(); ++i) {
      char ch = value.c_str()[i];
      switch (ch) {
        case '\n':
          escaped_value.append("\\n");
          break;
        case '"':
          escaped_value.append("\\\"");
          break;
        default:
          escaped_value.push_back(ch);
      }
    }
    printf("%s \"%s\"\n", name.c_str(), escaped_value.c_str());

  } else {
    assert(false);
  }

  return true;
}

bool DumpSettings(DataReader* reader) {
  int byte_order = IsLittleEndian() ? LSBFirst : MSBFirst;

  // Read 1-byte byte order.
  string byte_order_str;
  if (!reader->Read(&byte_order_str, 1)) {
    fprintf(stderr, "Couldn't read byte order\n");
    return false;
  }
  if (byte_order_str[0] != byte_order) {
    // TODO: Handle properties written in other order.
    fprintf(stderr, "Property byte order (%d) differed from our own (%d)\n",
            byte_order_str[0], byte_order);
    return false;
  }

  // Read 3 bytes of padding and 4-byte serial.
  if (!reader->Read(NULL, 7)) {
    fprintf(stderr, "Unable to read header\n");
    return false;
  }

  string num_settings_str;
  if (!reader->Read(&num_settings_str, 4)) {
    fprintf(stderr, "Unable to read number of settings\n");
    return false;
  }
  uint32_t num_settings =
      *reinterpret_cast<const uint32_t*>(num_settings_str.data());

  for (uint32_t i = 0; i < num_settings; ++i) {
    if (!DumpSetting(reader))
      return false;
  }

  return true;
}

}  // namespace xsettingsd

int main(int argc, char** argv) {
  static const size_t kBufferSize = 2 << 15;
  char buffer[kBufferSize];

  size_t data_size = 0;
  if (!xsettingsd::GetData(buffer, kBufferSize, &data_size))
    return 1;
  assert(data_size <= kBufferSize);

  xsettingsd::DataReader reader(buffer, data_size);
  if (!xsettingsd::DumpSettings(&reader))
    return false;

  return 0;
}
