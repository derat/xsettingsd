#include "settings_manager.h"

using namespace xsettingsd;

int main(int argc, char** argv) {
  SettingsManager manager;
  manager.UpdateProperty();
  return 0;
}
