// Copyright 2009 Daniel Erat <dan@erat.org>
// All rights reserved.

#include <cassert>
#include <cstdio>
#include <cstring>
#include <getopt.h>
#include <stdint.h>
#include <string>
#include <unistd.h>
#include <X11/Xlib.h>

#include "common.h"
#include "data_reader.h"
#include "setting.h"

using std::min;
using std::string;

namespace xsettingsd {

bool GetData(int screen, char* buffer, size_t buffer_size, size_t* data_size) {
  assert(data_size);

  Display* display = XOpenDisplay(NULL);
  if (!display) {
    fprintf(stderr, "Couldn't open display\n");
    return false;
  }

  string sel_name = StringPrintf("_XSETTINGS_S%d", screen);
  Atom sel_atom = XInternAtom(display, sel_name.c_str(), False);
  Window win = XGetSelectionOwner(display, sel_atom);
  if (win == None) {
    fprintf(stderr, "No current owner for %s selection\n", sel_name.c_str());
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
    fprintf(stderr, "Property %s on 0x%x is more than %zu bytes (%lu remain)\n",
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

bool DumpSetting(DataReader* reader) {
  int8_t type_byte = 0;
  if (!reader->ReadInt8(&type_byte)) {
    fprintf(stderr, "Unable to read setting type\n");
    return false;
  }
  Setting::Type type = static_cast<Setting::Type>(type_byte);

  if (type != Setting::TYPE_INTEGER &&
      type != Setting::TYPE_STRING &&
      type != Setting::TYPE_COLOR) {
    fprintf(stderr, "Got setting with unhandled type %d\n", type);
    return false;
  }

  if (!reader->ReadBytes(NULL, 1)) {
    fprintf(stderr, "Unable to read 1-byte setting padding\n");
    return false;
  }

  uint16_t name_size = 0;
  if (!reader->ReadInt16(reinterpret_cast<int16_t*>(&name_size))) {
    fprintf(stderr, "Unable to read setting name size\n");
    return false;
  }

  string name;
  if (!reader->ReadBytes(&name, name_size)) {
    fprintf(stderr, "Unable to read %u-byte setting name\n", name_size);
    return false;
  }

  size_t name_padding = GetPadding(name_size, 4);
  if (!reader->ReadBytes(NULL, name_padding)) {
    fprintf(stderr, "Unable to read %zu-byte setting name padding\n",
            name_padding);
    return false;
  }

  if (!reader->ReadBytes(NULL, 4)) {
    fprintf(stderr, "Unable to read setting serial number\n");
    return false;
  }

  if (type == Setting::TYPE_INTEGER) {
    int32_t value = 0;
    if (!reader->ReadInt32(&value)) {
      fprintf(stderr, "Unable to read integer setting value\n");
      return false;
    }
    printf("%s %d\n", name.c_str(), value);

  } else if (type == Setting::TYPE_STRING) {
    uint32_t value_size = 0;
    if (!reader->ReadInt32(reinterpret_cast<int32_t*>(&value_size))) {
      fprintf(stderr, "Unable to read string setting value size\n");
      return false;
    }

    string value;
    if (!reader->ReadBytes(&value, value_size)) {
      fprintf(stderr, "Unable to read %u-byte string setting value\n",
              value_size);
      return false;
    }

    size_t value_padding = GetPadding(value_size, 4);
    if (!reader->ReadBytes(NULL, value_padding)) {
      fprintf(stderr, "Unable to read %zu-byte string setting value padding\n",
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

  } else if (type == Setting::TYPE_COLOR) {
    uint16_t red = 0, blue = 0, green = 0, alpha = 0;
    if (!reader->ReadInt16(reinterpret_cast<int16_t*>(&red)) ||
        !reader->ReadInt16(reinterpret_cast<int16_t*>(&blue)) ||
        !reader->ReadInt16(reinterpret_cast<int16_t*>(&green)) ||
        !reader->ReadInt16(reinterpret_cast<int16_t*>(&alpha))) {
      fprintf(stderr, "Unable to read color values\n");
      return false;
    }
    // Note that unlike the spec, our config uses RGB-order, not RBG.
    printf("%s (%u, %u, %u, %u)\n", name.c_str(), red, green, blue, alpha);

  } else {
    assert(false);
  }

  return true;
}

bool DumpSettings(DataReader* reader) {
  int byte_order = IsLittleEndian() ? LSBFirst : MSBFirst;

  // Read 1-byte byte order.
  int8_t prop_byte_order = 0;
  if (!reader->ReadInt8(&prop_byte_order)) {
    fprintf(stderr, "Couldn't read byte order\n");
    return false;
  }
  if (prop_byte_order != byte_order) {
    reader->set_reverse_bytes(true);
  }

  // Read 3 bytes of padding and 4-byte serial.
  if (!reader->ReadBytes(NULL, 7)) {
    fprintf(stderr, "Unable to read header\n");
    return false;
  }

  uint32_t num_settings = 0;
  if (!reader->ReadInt32(reinterpret_cast<int32_t*>(&num_settings))) {
    fprintf(stderr, "Unable to read number of settings\n");
    return false;
  }

  for (uint32_t i = 0; i < num_settings; ++i) {
    if (!DumpSetting(reader))
      return false;
  }

  return true;
}

}  // namespace xsettingsd

int main(int argc, char** argv) {
  static const char* kUsage =
      "Usage: dump_xsettings [OPTION] ...\n"
      "\n"
      "Dump current XSETTINGS values in xsettingd's format.\n"
      "\n"
      "Options: -h, --help           print this help message\n"
      "         -s, --screen=SCREEN  screen to use (default is 0)\n";

  int screen = 0;

  struct option options[] = {
    { "help", 0, NULL, 'h', },
    { "screen", 1, NULL, 's', },
    { NULL, 0, NULL, 0 },
  };

  opterr = 0;
  while (true) {
    int ch = getopt_long(argc, argv, "hs:", options, NULL);
    if (ch == -1) {
      break;
    } else if (ch == 'h' || ch == '?') {
      fprintf(stderr, "%s", kUsage);
      return 1;
    } else if (ch == 's') {
      char* endptr = NULL;
      screen = strtol(optarg, &endptr, 10);
      if (optarg[0] == '\0' || endptr[0] != '\0' || screen < 0) {
        fprintf(stderr, "Invalid screen \"%s\"\n", optarg);
        return 1;
      }
    }
  }

  static const size_t kBufferSize = 2 << 15;
  char buffer[kBufferSize];

  size_t data_size = 0;
  if (!xsettingsd::GetData(screen, buffer, kBufferSize, &data_size))
    return 1;
  assert(data_size <= kBufferSize);

  xsettingsd::DataReader reader(buffer, data_size);
  if (!xsettingsd::DumpSettings(&reader))
    return 1;

  return 0;
}
