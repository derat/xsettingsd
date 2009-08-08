#include "common.h"

#include <cstdarg>
#include <cstdio>

using std::string;

namespace xsettingsd {

std::string StringPrintf(const char* format, ...) {
  char buffer[1024];
  va_list argp;
  va_start(argp, format);
  vsnprintf(buffer, sizeof(buffer), format, argp);
  va_end(argp);
  return std::string(buffer);
}

bool IsLittleEndian() {
  int i = 1;
  return reinterpret_cast<char*>(&i)[0];
}

const char* kProgName = "xsettingsd";

}  // namespace xsettingsd
