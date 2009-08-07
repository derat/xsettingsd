// Copyright 2009 Daniel Erat <dan@erat.org>
// All rights reserved.

#include <cstdlib>
#include <string>

#include "common.h"
#include "config_parser.h"
#include "settings_manager.h"

using namespace xsettingsd;
using std::string;

int main(int argc, char** argv) {
  const char* home_dir = getenv("HOME");
  if (!home_dir) {
    fprintf(stderr, "%s: $HOME undefined\n", kProgName);
    return 1;
  }
  string filename = StringPrintf("%s/.xsettingsd", home_dir);

  SettingsManager manager;
  if (!manager.LoadConfig(filename))
    return 1;

  return 0;
}
