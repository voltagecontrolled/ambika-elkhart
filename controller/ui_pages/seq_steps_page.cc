// Copyright 2011 Emilie Gillet.
//
// Sequencer mode: 3 pages of lockable params (Voice1 / Voice2 / Step) walked
// by the encoder. Pots write locks if any step button is held, otherwise
// write the track defaults. Step toggle suppressed if a pot moved while held.
//
// LCD layout matches the YAM 4-cells-per-row convention: 10 chars/cell,
// short_name left-justified at offset 1 (4 chars), value right-justified at
// offset 5 (4 chars), delimiters at 0/10/20/30 (outer edges skipped).

#include "controller/ui_pages/seq_steps_page.h"

#include "avrlib/string.h"
#include "common/patch.h"
#include "controller/display.h"
#include "controller/leds.h"
#include "controller/multi.h"
#include "controller/sequencer.h"
#include "controller/ui.h"

namespace ambika {

// Scale a 0..127 pot value to 0..max (inclusive).
static inline uint8_t ScalePot(uint8_t value, uint8_t max) {
  return (static_cast<uint16_t>(value) * (max + 1)) >> 7;
}

/* static */
uint8_t SeqStepsPage::step_lock_dirty_ = 0;

/* static */
uint8_t SeqStepsPage::cursor_ = 0;

// 2-char semitone names; index = semitone * 2.
static const prog_char kNoteNames[] PROGMEM =
  "C C#D D#E F F#G G#A A#B ";

// 4-char short_name per lockable param across the 24-slot space.
// Page 1 (cursor 0..7), Page 2 (8..15), Step (16..23).
// All lowercase by default; UpdateScreen uppercases the cursor's slot.
static const prog_char kAbbr[] PROGMEM =
  "notewav1pa1 blndrtiowav2pa2 fine"  // page 1
  "adec----fdec----pdec----noissub "  // page 2 (REL slots dead)
  "probssubreptratevel glidmintmdir"; // step page

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

// Encoder turn walks cursor across 24 lockable params; spills to the
// previous/next page (registry order) when stepping past the boundary.
/* static */
uint8_t SeqStepsPage::OnIncrement(int8_t increment) {
  int8_t next = static_cast<int8_t>(cursor_) + increment;
  if (next < 0) {
    cursor_ = 0;
    sequencer.mutable_global()->lock_page = 0;
    ui.ShowPageRelative(-1);
    return 1;
  }
  if (next >= 24) {
    cursor_ = 23;
    sequencer.mutable_global()->lock_page = 2;
    ui.ShowPageRelative(1);
    return 1;
  }
  cursor_ = next;
  sequencer.mutable_global()->lock_page = cursor_ >> 3;
  return 1;
}

// If any step button is held, write a lock for that step+param.
// Otherwise, write the track default.
//
// Note: switches_[] is indexed in reverse of the SwitchNumber enum —
// SR-index 0 corresponds to SWITCH_8, SR-index 7 to SWITCH_1
// (see Ui::Poll: `control = SWITCH_8 - i`). step_index (= SwitchNumber for
// SWITCH_1..8) = 7 - sr_index. inhibit_switch_ uses SR-index;
// step_lock_dirty_ and steps[] are step-index.
/* static */
uint8_t SeqStepsPage::OnPot(uint8_t index, uint8_t value) {
  if (index >= 8) return 0;
  uint8_t track = ui.state().active_part;
  SeqTrack* tr = sequencer.mutable_track(track);
  uint8_t page = sequencer.global().lock_page;
  uint8_t default_index = page * 8 + index;
  // Touching a knob brings the cursor to it.
  cursor_ = default_index;

  // Find a held step. Prefer fully-debounced holds (state==0); fall back to
  // any non-fully-released state for responsiveness on quick gestures.
  uint8_t held_sr = 0xff;
  for (uint8_t s = 0; s < 8; ++s) {
    if (ui.switch_held(s)) { held_sr = s; break; }
  }

  // Clamp pot value to valid range for params with constrained domains.
  uint8_t mapped = value;
  if (page == 0 && (index == kP1WAVE1 || index == kP1WAVE2)) {
    mapped = ScalePot(value, WAVEFORM_LAST - 1);
  } else if (page == 1 && index == kP2SUB) {
    // SUB knob doubles as sub-osc shape selector — but here we only edit
    // the level (kP2SUB) which is 0..255; voicecard wave shape lives in
    // config[kCfgWSUB] (not lockable). Pass through.
    mapped = value;
  }

  if (held_sr != 0xff) {
    uint8_t held_step = 7 - held_sr;
    SeqStep& step = tr->steps[held_step];
    uint8_t* slot = (page == 0) ? &step.page1[index]
                  : (page == 1) ? &step.page2[index]
                                : &step.steppage[index];
    *slot = mapped;
    step.lock_flags[default_index >> 3] |= (1 << (default_index & 7));
    // Mark this step dirty so the release doesn't toggle on/off.
    step_lock_dirty_ |= (1 << held_step);
    // Inhibit the next switch event for that step (SR-index in Poll).
    ui.inhibit_switch(1 << held_sr);
  } else {
    tr->defaults[default_index] = mapped;
  }
  return 1;
}

// Step toggle on release. Suppressed if a pot moved while this step was held.
/* static */
uint8_t SeqStepsPage::OnKey(uint8_t key) {
  if (key > SWITCH_8) return 0;
  if (step_lock_dirty_ & (1 << key)) {
    step_lock_dirty_ &= ~(1 << key);
    return 1;
  }
  uint8_t track = ui.state().active_part;
  SeqStep& s = sequencer.mutable_track(track)->steps[key];
  s.step_flags ^= kStepFlagOn;
  return 1;
}

// Write 3-char note name at buf: natural="C 4", sharp="C#4", sub-octave "C-".
static void WriteNoteName(char* buf, uint8_t note) {
  uint8_t semi = note % 12;
  int8_t  oct  = static_cast<int8_t>(note / 12) - 1;
  buf[0] = pgm_read_byte(kNoteNames + semi * 2);
  buf[1] = pgm_read_byte(kNoteNames + semi * 2 + 1);
  buf[2] = (oct < 0) ? '-' : ('0' + oct);
}

// Write a uint8 right-aligned into a 4-char field (last char left as space).
static void WriteU8Right(char* buf, uint8_t value) {
  buf[0] = ' ';
  UnsafeItoa<uint8_t>(value, 3, &buf[1]);
  AlignRight(&buf[1], 3);
}

/* static */
void SeqStepsPage::UpdateScreen() {
  uint8_t track = ui.state().active_part;
  const SeqTrack& tr = sequencer.track(track);
  uint8_t page = sequencer.global().lock_page;
  uint8_t cursor_in_page = (cursor_ >> 3) == page ? (cursor_ & 7) : 0xff;

  // If a step is held, show that step's resolved values (locks where set,
  // defaults elsewhere). Otherwise show the track defaults.
  uint8_t held_step = 0xff;
  for (uint8_t s = 0; s < 8; ++s) {
    if (ui.switch_held(s)) { held_step = 7 - s; break; }
  }

  for (uint8_t i = 0; i < 8; ++i) {
    uint8_t line = i < 4 ? 0 : 1;
    uint8_t row = (i & 3) * 10;
    char* buffer = display.line_buffer(line) + row;

    // Cell separators (skip outer edges).
    if (row != 0)                buffer[0]  = kDelimiter;
    if ((row + 10) != kLcdWidth) buffer[10] = kDelimiter;

    // Short name (4 chars) at offset 1.
    uint8_t abbr_off = page * 32 + i * 4;
    for (uint8_t c = 0; c < 4; ++c) {
      char ch = pgm_read_byte(kAbbr + abbr_off + c);
      // Uppercase the active knob's name.
      if (i == cursor_in_page && ch >= 'a' && ch <= 'z') {
        ch -= 0x20;
      }
      buffer[1 + c] = ch;
    }

    // Resolve value for display: locked-on-held-step → step's pageX[i];
    // otherwise track default.
    uint8_t lockable_index = page * 8 + i;
    uint8_t v;
    if (held_step != 0xff) {
      const SeqStep& step = tr.steps[held_step];
      uint8_t locked = step.lock_flags[lockable_index >> 3] &
                       (1 << (lockable_index & 7));
      if (locked) {
        v = (page == 0) ? step.page1[i]
          : (page == 1) ? step.page2[i]
                        : step.steppage[i];
      } else {
        v = tr.defaults[lockable_index];
      }
    } else {
      v = tr.defaults[lockable_index];
    }

    // Value (4 chars) at offset 5.
    if (page == 0 && i == kP1NOTE) {
      WriteNoteName(&buffer[5], v);
      buffer[8] = ' ';
    } else {
      WriteU8Right(&buffer[5], v);
    }
    buffer[9] = ' ';
  }
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
    leds.set_pixel(LED_1 + tr.shadow[kShdwLAST], 0xf0);
  }
}

}  // namespace ambika
