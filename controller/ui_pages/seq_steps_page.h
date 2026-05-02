// Copyright 2011 Emilie Gillet.
//
// Phase 4: Step grid page — 8-button step toggle with track selector.

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

  DISALLOW_COPY_AND_ASSIGN(SeqStepsPage);
};

}  // namespace ambika

#endif  // CONTROLLER_UI_PAGES_SEQ_STEPS_PAGE_H_
