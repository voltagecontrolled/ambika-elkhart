// Copyright 2011 Emilie Gillet.
//
// Phase 4: Transport control page (PLAY / PAUS / RST).

#ifndef CONTROLLER_UI_PAGES_MULTI_PAGE_H_
#define CONTROLLER_UI_PAGES_MULTI_PAGE_H_

#include "controller/ui_pages/ui_page.h"

namespace ambika {

class MultiPage : public UiPage {
 public:
  MultiPage() { }

  static uint8_t OnKey(uint8_t key);
  static void UpdateScreen();
  static void UpdateLeds();

  static const prog_EventHandlers event_handlers_;

  DISALLOW_COPY_AND_ASSIGN(MultiPage);
};

}  // namespace ambika

#endif  // CONTROLLER_UI_PAGES_MULTI_PAGE_H_
