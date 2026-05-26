// EXT-track MIDI CC editing page (issue #32).
//
// Replaces the retired S5b/S5c per-step CC surface with a dedicated page,
// reachable only when the active track's MMOD is EXT. 4 slots per track.
// Top row = CC# (per-slot enable bit toggled by encoder click); bot row =
// CC value (pool-backed per-step lockable via kCcSlotLockBase+slot).

#ifndef CONTROLLER_UI_PAGES_EXT_CC_PAGE_H_
#define CONTROLLER_UI_PAGES_EXT_CC_PAGE_H_

#include "controller/ui_pages/ui_page.h"

namespace ambika {

class ExtCcPage : public UiPage {
 public:
  ExtCcPage() {}

  static uint8_t OnIncrement(int8_t increment);
  static uint8_t OnClick();
  static uint8_t OnPot(uint8_t index, uint8_t value);
  static uint8_t OnKey(uint8_t key);
  static void UpdateScreen();
  static void UpdateLeds();

  static const prog_EventHandlers event_handlers_;

 private:
  // Cursor across the 8 cells (4 CC# top, 4 VAL bot). Encoder turn advances.
  static uint8_t cursor_;

  // Step-button gesture state — mirrors SeqStepsPage so peek (long-press
  // suppresses toggle) and double-tap-clear (within kStepDoubleTapMs)
  // behave the same way on this page.
  static uint8_t last_tap_step_;
  static uint16_t last_tap_ms_;

  DISALLOW_COPY_AND_ASSIGN(ExtCcPage);
};

}  // namespace ambika

#endif  // CONTROLLER_UI_PAGES_EXT_CC_PAGE_H_
