// Copyright 2011 Emilie Gillet.
//
// Step grid: 8 buttons toggle steps; S1+encoder selects track;
// encoder navigates pages; one pot per lockable param (pot0=note).

#include "controller/ui_pages/seq_steps_page.h"

#include "avrlib/string.h"
#include "controller/display.h"
#include "controller/leds.h"
#include "controller/multi.h"
#include "controller/sequencer.h"
#include "controller/ui.h"

namespace ambika {

// 2-char semitone names; index = semitone * 2.
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
  if (index != 0) return 0;
  uint8_t track = ui.state().active_part;
  // C2 (36) to B6 (95): 60-note range across pot sweep
  uint8_t note = 36 + static_cast<uint8_t>(
      (static_cast<uint16_t>(value) * 60) >> 8);
  sequencer.mutable_track(track)->defaults[kP1NOTE] = note;
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

// Write 3-char note name at buf: natural="C 4", sharp="C#4".
static void WriteNoteName(char* buf, uint8_t note) {
  uint8_t semi = note % 12;
  uint8_t oct  = note / 12 - 1;  // standard MIDI: C4=60, octave=4
  buf[0] = pgm_read_byte(kNoteNames + semi * 2);
  buf[1] = pgm_read_byte(kNoteNames + semi * 2 + 1);
  buf[2] = '0' + oct;
}

/* static */
void SeqStepsPage::UpdateScreen() {
  uint8_t track = ui.state().active_part;
  const SeqTrack& tr = sequencer.track(track);

  // Line 0: param name labels (one per column, col 0 = "note").
  char* buf = display.line_buffer(0) + 1;
  buf[14] = kDelimiter;
  memcpy_P(&buf[0], PSTR("note "), 5);

  // Line 1: param values (col 0 = current track note).
  buf = display.line_buffer(1) + 1;
  buf[14] = kDelimiter;
  WriteNoteName(&buf[0], tr.defaults[kP1NOTE]);
  buf[3] = ' ';
  buf[4] = ' ';
}

/* static */
void SeqStepsPage::UpdateLeds() {
  UiPage::UpdateLeds();
  uint8_t track = ui.state().active_part;
  const SeqTrack& tr = sequencer.track(track);
  uint8_t transport = sequencer.global().transport;
  if (transport == kSeqPlaying) {
    leds.set_pixel(LED_STATUS, 0xf0);
  } else if (transport == kSeqPaused) {
    leds.set_pixel(LED_STATUS, 0x0f);
  }
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
