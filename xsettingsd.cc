// Copyright 2009 Daniel Erat <dan@erat.org>
// All rights reserved.

#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unistd.h>

#include "common.h"
#include "config_parser.h"
#include "settings_manager.h"

using namespace xsettingsd;
using std::string;

void HandleSignal(int signum) {
}

int main(int argc, char** argv) {
  const char* home_dir = getenv("HOME");
  if (!home_dir) {
    fprintf(stderr, "%s: $HOME undefined\n", kProgName);
    return 1;
  }

  SettingsManager manager(StringPrintf("%s/.xsettingsd", home_dir));
  if (!manager.LoadConfig())
    return 1;
  if (!manager.InitX11(true))
    return 1;

  signal(SIGHUP, HandleSignal);

  manager.RunEventLoop();
  return 0;
}
