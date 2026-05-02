// Copyright 2011 Emilie Gillet.
//
// Phase 4: Transport control page — PLAY / PAUS / RST.

#include "controller/ui_pages/multi_page.h"

#include "avrlib/string.h"
#include "controller/display.h"
#include "controller/leds.h"
#include "controller/multi.h"
#include "controller/sequencer.h"
#include "controller/ui.h"

namespace ambika {

/* static */
const prog_EventHandlers MultiPage::event_handlers_ PROGMEM = {
  OnInit,
  SetActiveControl,
  OnIncrement,
  OnClick,
  OnPot,
  OnKey,
  NULL,
  OnIdle,
  UpdateScreen,
  UpdateLeds,
  OnDialogClosed,
};

/* static */
uint8_t MultiPage::OnIncrement(int8_t increment) {
  ui.ShowPageRelative(increment);
  return 1;
}

/* static */
uint8_t MultiPage::OnPot(uint8_t index, uint8_t value) {
  if (index == 0) {
    uint8_t bpm = 40 + static_cast<uint8_t>(
        (static_cast<uint16_t>(value < 127 ? value : 128) * 200) >> 7);
    multi.SetValue(PRM_MULTI_CLOCK_BPM, bpm);
    return 1;
  }
  return 0;
}

/* static */
uint8_t MultiPage::OnKey(uint8_t key) {
  switch (key) {
    case SWITCH_1:
      sequencer.Play();
      return 1;
    case SWITCH_2:
      sequencer.Pause();
      return 1;
    case SWITCH_3:
      sequencer.Reset();
      return 1;
    case SWITCH_8:
      ui.ShowPageRelative(-1);
      return 1;
  }
  return 0;
}

/* static */
void MultiPage::UpdateScreen() {
  char* buffer = display.line_buffer(0) + 1;
  memcpy_P(&buffer[0], PSTR("bpm "), 4);
  UnsafeItoa<uint8_t>(multi.data().clock_bpm, 3, &buffer[4]);
  AlignRight(&buffer[4], 3);
  buffer[14] = kDelimiter;

  buffer = display.line_buffer(1) + 1;
  memcpy_P(&buffer[0],  PSTR("play "), 5);
  memcpy_P(&buffer[5],  PSTR("paus "), 5);
  memcpy_P(&buffer[10], PSTR("rst "), 4);
  buffer[14] = kDelimiter;
  memcpy_P(&buffer[35], PSTR("exit"), 4);
}

/* static */
void MultiPage::UpdateLeds() {
  UiPage::UpdateLeds();
  uint8_t transport = sequencer.global().transport;
  if (transport == kSeqPlaying) {
    leds.set_pixel(LED_STATUS, 0xf0);
    leds.set_pixel(LED_1, 0xf0);
  } else if (transport == kSeqPaused) {
    leds.set_pixel(LED_STATUS, 0x0f);
    leds.set_pixel(LED_2, 0x0f);
  }
  leds.set_pixel(LED_8, 0xf0);
}

}  // namespace ambika
