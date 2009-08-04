#include <string>

#include "config_parser.h"
#include "settings_manager.h"

using namespace xsettingsd;
using std::string;

int main(int argc, char** argv) {
  SettingsManager manager;
  manager.UpdateProperty();

  return 0;
}
