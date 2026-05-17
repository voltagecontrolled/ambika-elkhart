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
    // Pot is 7-bit (128 stable positions). Map to 60..185 (126 values) so
    // every integer BPM in range is reachable — wider ranges leave holes.
    uint8_t bpm = 60 + static_cast<uint8_t>(
        (static_cast<uint16_t>(value) * 126) >> 7);
    multi.SetValue(PRM_MULTI_CLOCK_BPM, bpm);
    return 1;
  }
  if (index == 1) {
    // mrst moved from cell 2 → cell 1 after swng was dropped.
    multi.SetValue(PRM_MULTI_MASTER_RESET, value);
    return 1;
  }
  if (index == 2) {
    // CLK 4-state: 0=INT, 1=EXT, 2=OUT, 3=THR. Snap pot to 32-wide bands.
    multi.mutable_data()->midi_clock_mode = value >> 5;
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
  // bpm cell 0 (pot0), mrst cell 1 (pot1, moved from cell 2 after swng was
  // dropped), clk cell 2 (pot2) cycles INT/EXT/OUT/THR.
  char* buffer = display.line_buffer(0);
  memcpy_P(&buffer[1], PSTR("bpm "), 4);
  UnsafeItoa<uint8_t>(multi.data().clock_bpm, 3, &buffer[5]);
  AlignRight(&buffer[5], 3);
  buffer[10] = kDelimiter;
  memcpy_P(&buffer[11], PSTR("mrst"), 4);
  {
    uint8_t mrst = multi.data().master_reset_steps;
    if (mrst == 0) {
      memcpy_P(&buffer[15], PSTR(" off"), 4);
    } else {
      buffer[15] = ' ';
      UnsafeItoa<uint8_t>(mrst + 1, 3, &buffer[16]);
      AlignRight(&buffer[16], 3);
    }
  }
  buffer[20] = kDelimiter;
  memcpy_P(&buffer[21], PSTR("clk "), 4);
  {
    static const char kClockModeLabels[4][4] PROGMEM = {
      {' ', 'I', 'N', 'T'},
      {' ', 'E', 'X', 'T'},
      {' ', 'O', 'U', 'T'},
      {' ', 'T', 'H', 'R'},
    };
    uint8_t m = multi.data().midi_clock_mode & 3;
    memcpy_P(&buffer[25], kClockModeLabels[m], 4);
  }

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
