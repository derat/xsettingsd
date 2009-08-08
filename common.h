// Copyright 2009 Daniel Erat <dan@erat.org>
// All rights reserved.

#ifndef __XSETTINGSD_COMMON_H__
#define __XSETTINGSD_COMMON_H__

#include <string>

namespace xsettingsd {

#define DISALLOW_COPY_AND_ASSIGN(class_name) \
  class_name(const class_name&); \
  void operator=(const class_name&)

std::string StringPrintf(const char* format, ...);

bool IsLittleEndian();

extern const char* kProgName;

}  // namespace xsettingsd

#endif
