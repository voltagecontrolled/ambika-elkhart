// Copyright 2011 Emilie Gillet.
//
// Sequencer mode: 8 step buttons + 8 lockable knobs across 3 pages
// (Voice1 / Voice2 / Step). Encoder turn cycles pages.

#ifndef CONTROLLER_UI_PAGES_SEQ_STEPS_PAGE_H_
#define CONTROLLER_UI_PAGES_SEQ_STEPS_PAGE_H_

#include "controller/ui_pages/ui_page.h"

namespace ambika {

class SeqStepsPage : public UiPage {
 public:
  SeqStepsPage() { }

  static uint8_t OnIncrement(int8_t increment);
  static uint8_t OnClick();
  static uint8_t OnPot(uint8_t index, uint8_t value);
  static uint8_t OnKey(uint8_t key);
  static void UpdateScreen();
  static void UpdateLeds();

  static const prog_EventHandlers event_handlers_;

 private:
  // Bit per step: set when a pot moved while that step was held — the
  // release that follows skips the on/off toggle (it was a lock-edit).
  static uint8_t step_lock_dirty_;

  // Cursor across all 24 lockable params (0..23). Encoder turns advance this
  // by 1; the active page is derived as cursor_ / 8.
  static uint8_t cursor_;

  // Substep editor state: entered via encoder click while on `subs` cell
  // with a step button held. While active, step buttons toggle substep_bits
  // and pots 1/2/3 control MINT/MDIR/MOCT for the target step.
  static bool editing_substeps_;
  static uint8_t substep_step_;
  static uint8_t substep_count_;      // active slot count for current edit session
  static uint8_t substep_pot0_entry_; // 0xff = first pot0 event not yet seen; pickup guard

  // Most-recent tapped step (0xff = none) and its release ms, for
  // double-tap-to-clear. A second tap on the same step within the window
  // clears all lock_flags and cancels the first tap's toggle.
  static uint8_t last_tap_step_;
  static uint16_t last_tap_ms_;

  DISALLOW_COPY_AND_ASSIGN(SeqStepsPage);
};

}  // namespace ambika

#endif  // CONTROLLER_UI_PAGES_SEQ_STEPS_PAGE_H_
