// Copyright 2011 Emilie Gillet.
//
// Step grid: 8 buttons toggle steps; S1+encoder selects track;
// encoder navigates pages; pots set per-step note; LEDs green=on red=chaselight.

#include "controller/ui_pages/seq_steps_page.h"

#include "avrlib/string.h"
#include "controller/display.h"
#include "controller/leds.h"
#include "controller/multi.h"
#include "controller/sequencer.h"
#include "controller/ui.h"

namespace ambika {

// 2-char names for 12 semitones: "C ", "C#", "D ", "D#", "E ", "F ",
//                                 "F#", "G ", "G#", "A ", "A#", "B "
static const prog_char kNoteNames[] PROGMEM =
  "C C#D D#E F F#G G#A A#B ";

/* static */
const prog_EventHandlers SeqStepsPage::event_handlers_ PROGMEM = {
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
uint8_t SeqStepsPage::OnIncrement(int8_t increment) {
  ui.ShowPageRelative(increment);
  return 1;
}

/* static */
uint8_t SeqStepsPage::OnPot(uint8_t index, uint8_t value) {
  uint8_t track = ui.state().active_part;
  sequencer.mutable_track(track)->steps[index].page1[kP1NOTE] = value >> 1;
  return 1;
}

/* static */
uint8_t SeqStepsPage::OnKey(uint8_t key) {
  if (key > SWITCH_8) return 0;
  uint8_t track = ui.state().active_part;
  SeqStep& s = sequencer.mutable_track(track)->steps[key];
  s.step_flags ^= kStepFlagOn;
  return 1;
}

/* static */
void SeqStepsPage::UpdateScreen() {
  uint8_t track = ui.state().active_part;
  const SeqTrack& tr = sequencer.track(track);

  // Line 0: per-step note names (3 chars centered in each 5-char column).
  // kStepPositions center: 2, 7, 12, [14=delim], 17, 22, 27, 32, 37
  static const uint8_t kCenters[8] = { 2, 7, 12, 17, 22, 27, 32, 37 };
  char* buf = display.line_buffer(0) + 1;
  buf[14] = kDelimiter;
  for (uint8_t i = 0; i < kNumStepsPerTrack; ++i) {
    uint8_t note = tr.steps[i].page1[kP1NOTE];
    uint8_t semi = note % 12;
    uint8_t oct  = note / 12;
    uint8_t pos  = kCenters[i];
    buf[pos - 1] = pgm_read_byte(kNoteNames + semi * 2);
    buf[pos]     = pgm_read_byte(kNoteNames + semi * 2 + 1);
    buf[pos + 1] = '0' + oct;
  }

  // Line 1: reserved for additional lockable params.
  display.line_buffer(1)[15] = kDelimiter;
}

/* static */
void SeqStepsPage::UpdateLeds() {
  uint8_t track = ui.state().active_part;
  const SeqTrack& tr = sequencer.track(track);
  uint8_t transport = sequencer.global().transport;
  if (transport == kSeqPlaying) {
    leds.set_pixel(LED_STATUS, 0xf0);
  } else if (transport == kSeqPaused) {
    leds.set_pixel(LED_STATUS, 0x0f);
  }
  // Green for ON steps; chaselight overrides with red.
  for (uint8_t i = 0; i < kNumStepsPerTrack; ++i) {
    if (tr.steps[i].step_flags & kStepFlagOn) {
      leds.set_pixel(LED_1 + i, 0x0f);
    }
  }
  if (transport == kSeqPlaying) {
    uint8_t len  = tr.pattern[kPatLENG];
    if (len == 0) len = 1;
    uint8_t next = tr.shadow[kShdwSTEP];
    uint8_t last = (next + len - 1) % len;
    leds.set_pixel(LED_1 + last, 0xf0);
  }
}

}  // namespace ambika
