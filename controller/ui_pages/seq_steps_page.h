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

  DISALLOW_COPY_AND_ASSIGN(SeqStepsPage);
};

}  // namespace ambika

#endif  // CONTROLLER_UI_PAGES_SEQ_STEPS_PAGE_H_
