// Copyright 2009 Daniel Erat <dan@erat.org>
// All rights reserved.

#include <string>

#include "config_parser.h"
#include "settings_manager.h"

using namespace xsettingsd;
using std::string;

int main(int argc, char** argv) {
  SettingsManager manager("bogus_file");
  manager.UpdateProperty();

  return 0;
}
