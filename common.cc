// Copyright 2009 Daniel Erat <dan@erat.org>
// All rights reserved.

#include "common.h"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>

using std::string;
using std::vector;

namespace xsettingsd {

string StringPrintf(const char* format, ...) {
  char buffer[1024];
  va_list argp;
  va_start(argp, format);
  vsnprintf(buffer, sizeof(buffer), format, argp);
  va_end(argp);
  return string(buffer);
}

vector<string> SplitString(const string& str, const string& delim) {
  if (str.empty())
    return vector<string>();
  if (delim.empty())
    return vector<string>(1, str);

  vector<string> parts;
  size_t start = 0;
  while (start <= str.size()) {
    if (start == str.size()) {
      parts.push_back(string());
      break;
    }
    size_t next = str.find(delim, start);
    if (next == string::npos) {
      parts.push_back(str.substr(start, str.size() - start));
      break;
    }
    parts.push_back(str.substr(start, next - start));
    start = next + delim.size();
  }
  return parts;
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

vector<string> GetDefaultConfigFilePaths() {
  vector<string> paths;

  // Try ~/.xsettingsd first.
  const char* home_dir = getenv("HOME");
  if (home_dir)
    paths.push_back(StringPrintf("%s/.xsettingsd", home_dir));

  // Next look under $XDG_CONFIG_HOME, or in $HOME/.config if $XDG_CONFIG_HOME
  // is unset.
  vector<string> xdg_dirs;
  const char* xdg_config_home = getenv("XDG_CONFIG_HOME");
  if (xdg_config_home && xdg_config_home[0] != '\0')
    xdg_dirs.push_back(xdg_config_home);
  else if (home_dir)
    xdg_dirs.push_back(StringPrintf("%s/.config", home_dir));

  // Finally split the colon-delimited $XDG_CONFIG_DIRS variable.
  const char* xdg_config_dirs = getenv("XDG_CONFIG_DIRS");
  if (xdg_config_dirs) {
    vector<string> split_dirs = SplitString(xdg_config_dirs, ":");
    xdg_dirs.insert(xdg_dirs.end(), split_dirs.begin(), split_dirs.end());
  } else {
    xdg_dirs.push_back("/etc");
  }

  for (size_t i = 0; i < xdg_dirs.size(); ++i) {
    paths.push_back(StringPrintf("%s/xsettingsd/xsettingsd.conf",
                                 xdg_dirs[i].c_str()));
  }

  return paths;
}

const char* kProgName = "xsettingsd";

}  // namespace xsettingsd
