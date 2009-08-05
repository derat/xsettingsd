// Copyright 2009 Daniel Erat <dan@erat.org>
// All rights reserved.

#include "settings_manager.h"

#include "data_writer.h"
#include "setting.h"

using std::make_pair;
using std::map;
using std::string;

namespace xsettingsd {

SettingsManager::SettingsManager()
    : serial_(0) {
}

#if 0
bool SettingsManager::TakeSelection(bool replace) {
  Atom atom = XInternAtom(display_, "_XSETTINGS_S0");

  // TODO: Select events for someone taking the selection.

  XGrabServer(display_);
  Window prev_win = XGetSelectionOwner(display_, atom);
  if (prev_win != None && !replace) {
    XUngrabServer(display_);
    return false;
  }

  if (prev_win)
    XSelectInput(display_, prev_win, StructureNotifyMask);
  XSetSelectionOwner(display_, atom, win, CurrentTime);
  XUngrabServer(display_);

  if (prev_win) {
    XEvent event;
    while (true) {
      XWindowEvent(display_, prev_win, StructureNotifyMask, &event);
      if (event.type == StructureNotify)
        break;
    }
  }

  if (XGetSelectionOwner(display_, atom) != win)
    return false;

  // TODO: Send message to root window.
}
#endif

bool SettingsManager::UpdateProperty() {
  static const int kBufferSize = 8192;
  char buffer[kBufferSize];
  DataWriter writer(buffer, sizeof(buffer));

  // FIXME: First field is supposed to be byte-order.
  if (!writer.WriteInt8(0))                       return false;
  if (!writer.WriteZeros(3))                      return false;
  if (!writer.WriteInt32(serial_))                return false;
  if (!writer.WriteInt32(settings_.map().size())) return false;

  for (SettingsMap::Map::const_iterator it = settings_.map().begin();
       it != settings_.map().end(); ++it) {
    if (!it->second->Write(it->first, &writer))
      return false;
  }

  return true;
}

}  // namespace xsettingsd
