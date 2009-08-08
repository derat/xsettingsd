#include <cassert>
#include <cstdio>
#include <cstring>
#include <string>
#include <X11/Xlib.h>

using std::min;
using std::string;

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

int main(int argc, char** argv) {
  static const int kBufferSize = 2 << 15;
  char buffer[kBufferSize];

  size_t data_size = 0;
  if (!GetData(buffer, kBufferSize, &data_size))
    return 1;

  return 0;
}
