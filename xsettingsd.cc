// Copyright 2009 Daniel Erat <dan@erat.org>
// All rights reserved.

#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <getopt.h>
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
  static const char* kUsage =
      "Usage: xsettingsd [OPTION] ...\n"
      "\n"
      "Daemon implementing the XSETTINGS spec to control settings for X11\n"
      "applications.\n"
      "\n"
      "Options: -c, --config=FILE    config file (default is ~/.xsettingsd)\n"
      "         -h, --help           print this help message\n"
      "         -s, --screen=SCREEN  screen to use (default is all)\n";

  int screen = -1;
  string config_file = "";

  struct option options[] = {
    { "config", 1, NULL, 'c', },
    { "help", 0, NULL, 'h', },
    { "screen", 1, NULL, 's', },
    { NULL, 0, NULL, 0 },
  };

  opterr = 0;
  while (true) {
    int ch = getopt_long(argc, argv, "c:hs:", options, NULL);
    if (ch == -1) {
      break;
    } else if (ch == 'c') {
      config_file = optarg;
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

  // Use the default config file if one wasn't supplied via a flag.
  if (config_file.empty()) {
    const char* home_dir = getenv("HOME");
    if (!home_dir) {
      fprintf(stderr, "%s: $HOME undefined; use --config=FILE\n", kProgName);
      return 1;
    }
    config_file = StringPrintf("%s/.xsettingsd", home_dir);
  }

  SettingsManager manager(config_file);
  if (!manager.LoadConfig())
    return 1;
  if (!manager.InitX11(screen, true))
    return 1;

  signal(SIGHUP, HandleSignal);

  manager.RunEventLoop();
  return 0;
}
