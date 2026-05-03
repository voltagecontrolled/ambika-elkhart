// Copyright 2011 Emilie Gillet.
//
// Transport control page — PLAY / PAUS / RST / STOP.
// STOP (S4): single tap = Pause + Reset; double tap (<300ms) = panic
// (Pause + Reset + Kill all voices). Groovebox-typical.

#include "controller/ui_pages/multi_page.h"

#include "avrlib/string.h"
#include "avrlib/time.h"
#include "controller/display.h"
#include "controller/leds.h"
#include "controller/multi.h"
#include "controller/sequencer.h"
#include "controller/ui.h"
#include "controller/voicecard_tx.h"

namespace ambika {

static uint32_t last_stop_tap_ = 0;
static const uint16_t kStopDoubleTapWindowMs = 300;

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
  // Clamp to ±1 — S2/S8 + encoder arrives as ±8, but ShowPageRelative
  // walks pages one at a time (skipping all-0xff entries), so the raw
  // multiplied value would jump 8 registry slots in one click.
  ui.ShowPageRelative(increment > 0 ? 1 : -1);
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
  if (index == 1) {
    multi.SetValue(PRM_MULTI_CLOCK_GROOVE_AMOUNT, value);
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
    case SWITCH_4: {
      uint32_t now = milliseconds();
      // Pause() toggles paused↔playing; only call it when actually playing.
      if (sequencer.global().transport == kSeqPlaying) {
        sequencer.Pause();
      }
      sequencer.Reset();
      if (last_stop_tap_ != 0 &&
          (now - last_stop_tap_) < kStopDoubleTapWindowMs) {
        for (uint8_t v = 0; v < kNumVoices; ++v) {
          voicecard_tx.Kill(v);
        }
        last_stop_tap_ = 0;
      } else {
        last_stop_tap_ = now ? now : 1;
      }
      return 1;
    }
    case SWITCH_8:
      ui.ShowPageRelative(-1);
      return 1;
  }
  return 0;
}

/* static */
void MultiPage::UpdateScreen() {
  // Canonical 4-cells-per-row layout (cell width 10, abbr at +1, value at +5).
  // bpm sits in cell 0 (under top1 pot); swng in cell 1 (under top2 pot).
  char* buffer = display.line_buffer(0);
  memcpy_P(&buffer[1], PSTR("bpm "), 4);
  UnsafeItoa<uint8_t>(multi.data().clock_bpm, 3, &buffer[5]);
  AlignRight(&buffer[5], 3);
  buffer[10] = kDelimiter;
  memcpy_P(&buffer[11], PSTR("swng"), 4);
  UnsafeItoa<uint8_t>(multi.data().clock_groove_amount, 3, &buffer[15]);
  AlignRight(&buffer[15], 3);

  buffer = display.line_buffer(1) + 1;
  memcpy_P(&buffer[0],  PSTR("play "), 5);
  memcpy_P(&buffer[5],  PSTR("paus "), 5);
  memcpy_P(&buffer[10], PSTR("rst  "), 5);
  memcpy_P(&buffer[15], PSTR("stop"), 4);
  buffer[19] = kDelimiter;
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
  if (last_stop_tap_ != 0 &&
      (milliseconds() - last_stop_tap_) < kStopDoubleTapWindowMs) {
    leds.set_pixel(LED_4, 0x0f);
  }
  leds.set_pixel(LED_8, 0xf0);
}

}  // namespace ambika
