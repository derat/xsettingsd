// Copyright 2009 Daniel Erat <dan@erat.org>
// All rights reserved.

#ifndef __XSETTINGSD_COMMON_H__
#define __XSETTINGSD_COMMON_H__

#include <string>
#include <vector>

namespace xsettingsd {

#define DISALLOW_COPY_AND_ASSIGN(class_name) \
  class_name(const class_name&); \
  void operator=(const class_name&)

std::string StringPrintf(const char* format, ...);

// Splits |str| along |delim|. Repeated occurrences of |delim| in |str| will
// result in empty strings in the output. An empty |delim| will result in |str|
// being returned unsplit. An empty |str| will result in an empty vector.
std::vector<std::string> SplitString(const std::string& str,
                                     const std::string& delim);

bool IsLittleEndian();

int GetPadding(int length, int increment);

// Returns $HOME/.xsettingsd followed by all of the config file locations
// specified by the XDG Base Directory Specification
// (http://standards.freedesktop.org/basedir-spec/basedir-spec-latest.html).
std::vector<std::string> GetDefaultConfigFilePaths();

extern const char* kProgName;

}  // namespace xsettingsd

#endif
