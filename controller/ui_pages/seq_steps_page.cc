// Copyright 2011 Emilie Gillet.
//
// Phase 4: Step grid — 8 buttons toggle steps on/off; encoder selects track.

#include "controller/ui_pages/seq_steps_page.h"

#include "avrlib/string.h"
#include "controller/display.h"
#include "controller/leds.h"
#include "controller/multi.h"
#include "controller/sequencer.h"

namespace ambika {

static const prog_char kDirnNames[] PROGMEM = "fwd rev pnd rnd ";

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
  SeqGlobal& g = *sequencer.mutable_global();
  int8_t t = static_cast<int8_t>(g.active_track) + increment;
  if (t < 0) t = 0;
  if (t >= kNumVoices) t = kNumVoices - 1;
  g.active_track = static_cast<uint8_t>(t);
  ui.mutable_state()->active_part = g.active_track;
  return 1;
}

/* static */
uint8_t SeqStepsPage::OnKey(uint8_t key) {
  if (key > SWITCH_8) return 0;
  uint8_t step = SWITCH_8 - key;  // SWITCH_1=step7, SWITCH_8=step0
  // Invert: SWITCH_1 = step 0, SWITCH_8 = step 7
  step = key;  // SWITCH_1=0, SWITCH_2=1, ..., SWITCH_8=7
  uint8_t track = sequencer.global().active_track;
  SeqStep& s = sequencer.mutable_track(track)->steps[step];
  s.step_flags ^= kStepFlagOn;
  return 1;
}

/* static */
void SeqStepsPage::UpdateScreen() {
  uint8_t track = sequencer.global().active_track;
  const SeqTrack& tr = sequencer.track(track);
  uint8_t transport = sequencer.global().transport;

  char* buffer = display.line_buffer(0) + 1;
  buffer[0] = 't';
  buffer[1] = '1' + track;
  buffer[2] = ' ';
  // Direction name (3 chars)
  uint8_t dirn = tr.pattern[kPatDIRN];
  if (dirn > kDirnRnd) dirn = kDirnFwd;
  memcpy_P(&buffer[3], kDirnNames + (dirn * 4), 3);
  buffer[6] = ' ';
  // Pattern length
  UnsafeItoa<uint8_t>(tr.pattern[kPatLENG], 2, &buffer[7]);
  AlignRight(&buffer[7], 2);
  buffer[14] = kDelimiter;
  if (transport == kSeqPlaying) {
    memcpy_P(&buffer[15], PSTR("playing"), 7);
  } else if (transport == kSeqPaused) {
    memcpy_P(&buffer[15], PSTR("paused "), 7);
  } else {
    memcpy_P(&buffer[15], PSTR("stopped"), 7);
  }

  // Line 1: step indicators — each step gets 5 chars centered around position
  // SWITCH_1=buf[2], SWITCH_2=buf[7], SWITCH_3=buf[12], SWITCH_4=buf[17]...
  // SWITCH_5=buf[22], SWITCH_6=buf[27], SWITCH_7=buf[32], SWITCH_8=buf[37]
  buffer = display.line_buffer(1) + 1;
  uint8_t playhead = tr.shadow[kShdwSTEP];
  uint8_t len = tr.pattern[kPatLENG];
  if (len == 0) len = 1;
  static const uint8_t kStepPositions[8] = { 2, 7, 12, 17, 22, 27, 32, 37 };
  for (uint8_t i = 0; i < kNumStepsPerTrack; ++i) {
    uint8_t pos = kStepPositions[i];
    uint8_t on = tr.steps[i].step_flags & kStepFlagOn;
    uint8_t active = (transport == kSeqPlaying) && (i == playhead) && (i < len);
    if (active) {
      buffer[pos] = '>';
    } else if (on) {
      buffer[pos] = '*';
    } else {
      buffer[pos] = '.';
    }
  }
  buffer[14] = kDelimiter;
}

/* static */
void SeqStepsPage::UpdateLeds() {
  uint8_t track = sequencer.global().active_track;
  const SeqTrack& tr = sequencer.track(track);
  uint8_t transport = sequencer.global().transport;
  if (transport == kSeqPlaying) {
    leds.set_pixel(LED_STATUS, 0xf0);
  } else if (transport == kSeqPaused) {
    leds.set_pixel(LED_STATUS, 0x0f);
  }
  // Button LEDs: bright if step ON, off if OFF
  for (uint8_t i = 0; i < kNumStepsPerTrack; ++i) {
    if (tr.steps[i].step_flags & kStepFlagOn) {
      leds.set_pixel(LED_1 + (7 - i), 0xf0);
    }
  }
}

}  // namespace ambika
