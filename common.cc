// Copyright 2009 Daniel Erat <dan@erat.org>
// All rights reserved.

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

int GetPadding(int length, int increment) {
  return (increment - (length % increment)) % increment;
  // From xsettings-common.h in Owen Taylor's reference implementation --
  // "n" is length and "m" is increment, I think.  This produces results
  // that don't seem like padding, though: when "n" is 2 and "m" is 4, it
  // produces 4.
  //return ((n + m - 1) & (~(m - 1)));
}

const char* kProgName = "xsettingsd";

}  // namespace xsettingsd
