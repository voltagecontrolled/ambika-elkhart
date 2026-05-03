// Copyright 2011 Emilie Gillet.
//
// Sequencer mode: 3 pages of lockable params (Voice1 / Voice2 / Step) walked
// by the encoder. Pots write locks if any step button is held, otherwise
// write the track defaults. Step toggle suppressed if a pot moved while held.
//
// Cells are described by a per-(page,cell) descriptor. Most cells back a
// lockable param (writes go to tr.defaults[N] or step.pageX[i] when a step
// is held). A few cells on page 2 back voice config (e.g. filter env amount):
// those cells are not lockable — turning the knob writes tr.config[] and
// pushes the new value to the voicecard immediately via Part::SetValue.
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
#include "controller/resources.h"
#include "controller/sequencer.h"
#include "controller/ui.h"

namespace ambika {

// Scale a 0..127 pot value to 0..max (inclusive).
static inline uint8_t ScalePot(uint8_t value, uint8_t max) {
  return (static_cast<uint16_t>(value) * (max + 1)) >> 7;
}

// Map a 0..127 pot value to a signed int8_t range [min..max], stored as the
// underlying uint8_t (two's-complement). Mid pot ≈ 0; pot endpoints clamp.
static uint8_t MapPotInt8(uint8_t value, int8_t min, int8_t max) {
  int16_t range = static_cast<int16_t>(max) - min + 1;
  int16_t scaled = (static_cast<int16_t>(value) * range) >> 7;
  int16_t signed_val = scaled + min;
  if (signed_val > max) signed_val = max;
  if (signed_val < min) signed_val = min;
  return static_cast<uint8_t>(static_cast<int8_t>(signed_val));
}

/* static */
uint8_t SeqStepsPage::step_lock_dirty_ = 0;

/* static */
uint8_t SeqStepsPage::cursor_ = 0;

// 2-char semitone names; index = semitone * 2.
static const prog_char kNoteNames[] PROGMEM =
  "C C#D D#E F F#G G#A A#B ";

// 4-char short_name per cell (8 cells × 3 pages × 4 chars = 96 bytes).
// Lowercase by default; UpdateScreen uppercases the cursor's slot.
// `w1  ` / `w2  ` carry trailing spaces — wave cells render abbr at the
// 2-char width and grow the value field to 6 chars (handled in UpdateScreen).
//
// Page order (round 5a-2): step-behavior is the leftmost page so the
// default cursor=0 lands on NOTE — the most foundational sequencer knob.
// Voice 1 / Voice 2 follow.
static const prog_char kAbbr[] PROGMEM =
  "notevel glidratesubsprobmintmdir"  // page 1 = S5a (step behavior)
  "noisw1  pa1 tun2mix w2  pa2 fin2"  // page 2 = S5b (voice 1: osc / mix)
  "freqfdecfamtadecpdecpamtsub wave"; // page 3 = S5c (voice 2: filter/env/sub)

// Per-cell target. 0..23 = lockable param index (writes to tr.defaults[N]
// or step.{page1|page2|steppage}[N%8]; lock_flags bit N marks per-step lock).
// 0xff = config-mapped cell — see kCellPatchAddr below.
// 0xfe = merged SSUB+REPT cell (`subs` on S5a) — special-cased in OnPot
//        and UpdateScreen; writes to both kSPSSUB and kSPREPT with mutex.
//
// Lockable indices: page1 0..7, page2 8..15, steppage 16..23 (storage-side;
// the UI page order below is independent of the storage page order).
// tun2 / fin2 reclaim the dead E1REL / E2REL slots (lockable 9 / 11).
static const prog_uint8_t kCellLockable[24] PROGMEM = {
  // S5a: note, vel, glid, rate | subs(merged), prob, mint, mdir
  0,    20,   21,   19,
  0xfe, 16,   22,   23,
  // S5b: nois, w1, pa1, tun2 | mix(blnd), w2, pa2, fin2
  14,   1,    2,    9,
  3,    5,    6,    11,
  // S5c: freq*, fdec, famt*, adec | pdec, pamt*, sub, wave*  (* = config)
  0xff, 10,   0xff, 8,
  12,   0xff, 15,   0xff,
};

// Patch address for config-mapped cells (delivered to the voicecard via
// Part::SetValue, which also mirrors into tr.config[]). 0xff for lockable.
//   freq = filter cutoff     → patch addr 16 (kCfgFREQ)
//   famt = filter env depth  → patch addr 22 (FILTER1_ENV)
//   pamt = pitch env depth   → patch addr 58 (mod slot 2)
//   wave = sub-osc waveform  → patch addr 11 (MIX_SUB_SHAPE)
static const prog_uint8_t kCellPatchAddr[24] PROGMEM = {
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  16,   0xff, 22,   0xff, 0xff, 58,   0xff, 11,
};

// Sentinel for the merged SSUB+REPT `subs` cell on S5c (cursor 20).
static const uint8_t kSubsMergedSentinel = 0xfe;

// Lockable indices that store an int8_t (centered at 0).
static inline uint8_t IsSignedLockable(uint8_t lockable) {
  return lockable == 9 || lockable == 11;   // kP2TUN2 / kP2FIN2 (8 + 1, 8 + 3)
}

// Wave cells use a 2-char abbr + 6-char value layout instead of the
// standard 4/4. Lockable 1 (kP1WAVE1) and 5 (kP1WAVE2) on Page 1.
static inline uint8_t IsWaveCell(uint8_t lockable) {
  return lockable == 1 || lockable == 5;
}

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

// Encoder turn walks cursor across 24 cells; spills to the previous/next
// page (registry order) when stepping past the boundary.
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

// If any step button is held, write a lock for that step+param (lockable
// cells only). Otherwise, write the track default. Config-mapped cells
// always write through Part::SetValue (no per-step locking).
//
// switches_[] is indexed in reverse of the SwitchNumber enum — SR-index 0
// corresponds to SWITCH_8, SR-index 7 to SWITCH_1 (see Ui::Poll: `control =
// SWITCH_8 - i`). step_index (= SwitchNumber for SWITCH_1..8) = 7 - sr_index.
// inhibit_switch_ uses SR-index; step_lock_dirty_ and steps[] are step-index.
/* static */
uint8_t SeqStepsPage::OnPot(uint8_t index, uint8_t value) {
  if (index >= 8) return 0;
  uint8_t track = ui.state().active_part;
  uint8_t page = sequencer.global().lock_page;
  uint8_t cell = page * 8 + index;
  // Touching a knob brings the cursor to it.
  cursor_ = cell;

  uint8_t lockable = pgm_read_byte(&kCellLockable[cell]);
  uint8_t patch_addr = pgm_read_byte(&kCellPatchAddr[cell]);

  SeqTrack* tr = sequencer.mutable_track(track);

  // Find a held step (SR-index 0..7 corresponds to SWITCH_8..SWITCH_1; the
  // step index = 7 - sr). 0xff = no step held.
  uint8_t held_sr = 0xff;
  for (uint8_t s = 0; s < 8; ++s) {
    if (ui.switch_held(s)) { held_sr = s; break; }
  }

  // Merged subs cell (S5c bot1) — writes both kSPSSUB and kSPREPT with mutex.
  // Pot bands: 0..7=Edit (-2), 8..15=Cust (-1), 16..62=reps 7..1, 63..71=norm,
  // 72..127=ratchets +1..+8.
  if (lockable == kSubsMergedSentinel) {
    int8_t  ssub_v = 0;
    uint8_t rept_v = 0;
    if (value < 8) {
      ssub_v = -2;
    } else if (value < 16) {
      ssub_v = -1;
    } else if (value <= 62) {
      rept_v = ((62 - value) / 7) + 1;       // 1..7 reps
      if (rept_v > 7) rept_v = 7;
    } else if (value <= 71) {
      // center deadzone — both 0
    } else {
      uint8_t r = ((value - 72) / 7) + 1;    // 1..8 ratchets
      if (r > 8) r = 8;
      ssub_v = static_cast<int8_t>(r);
    }
    uint8_t ssub_byte = static_cast<uint8_t>(ssub_v);
    if (held_sr != 0xff) {
      uint8_t held_step = 7 - held_sr;
      SeqStep& step = tr->steps[held_step];
      step.steppage[kSPSSUB] = ssub_byte;
      step.steppage[kSPREPT] = rept_v;
      step.lock_flags[2] |= (1 << kSPSSUB) | (1 << kSPREPT);
      step_lock_dirty_ |= (1 << held_step);
      ui.inhibit_switch(1 << held_sr);
    } else {
      tr->defaults[16 + kSPSSUB] = ssub_byte;
      tr->defaults[16 + kSPREPT] = rept_v;
    }
    return 1;
  }

  // Per-cell pot scaling.
  uint8_t mapped = value;
  if (lockable == 1 || lockable == 5) {
    // WAVE1 / WAVE2: full waveform enum range.
    mapped = ScalePot(value, WAVEFORM_LAST - 1);
  } else if (patch_addr == 11) {
    // Sub-osc waveform.
    mapped = ScalePot(value, WAVEFORM_SUB_OSC_LAST - 1);
  } else if (lockable == 9) {
    // tun2 — Osc2 coarse pitch, UNIT_INT8 -24..+24.
    mapped = MapPotInt8(value, -24, 24);
  } else if (lockable == 11) {
    // fin2 — Osc2 detune, UNIT_INT8 -64..+64.
    mapped = MapPotInt8(value, -64, 64);
  } else if (lockable == 3) {
    // BLND (mix) — clamp to crossfade range. Upper half (≥ 64) was reserved
    // for future linear-FM and isn't implemented; the dead range produced
    // glitchy output. 0..127 pot → 0..63 crossfade only.
    mapped = value >> 1;
  }
  // freq / famt / pamt pass through 0..127 (matches PAGE_FILTER pot semantics
  // and the round-5 unipolar env-depth range).

  if (lockable == 0xff) {
    // Config cell — push through Part::SetValue (writes voicecard +
    // mirrors into tr.config[]).
    multi.mutable_part(track)->SetValue(patch_addr, mapped, 0);
    return 1;
  }

  if (held_sr != 0xff) {
    uint8_t held_step = 7 - held_sr;
    SeqStep& step = tr->steps[held_step];
    uint8_t buf_page = lockable >> 3;
    uint8_t buf_idx  = lockable & 7;
    uint8_t* slot = (buf_page == 0) ? &step.page1[buf_idx]
                  : (buf_page == 1) ? &step.page2[buf_idx]
                                    : &step.steppage[buf_idx];
    *slot = mapped;
    step.lock_flags[lockable >> 3] |= (1 << (lockable & 7));
    step_lock_dirty_ |= (1 << held_step);
    ui.inhibit_switch(1 << held_sr);
  } else {
    tr->defaults[lockable] = mapped;
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

// Write a signed int8 (reinterpreted from uint8 storage) right-aligned into
// a 4-char field.
static void WriteI8Right(char* buf, uint8_t value) {
  int16_t v = static_cast<int8_t>(value);
  UnsafeItoa<int16_t>(v, 4, buf);
  AlignRight(buf, 4);
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
    uint8_t cell_global = page * 8 + i;
    uint8_t lockable = pgm_read_byte(&kCellLockable[cell_global]);
    uint8_t patch_addr = pgm_read_byte(&kCellPatchAddr[cell_global]);

    uint8_t line = i < 4 ? 0 : 1;
    uint8_t row = (i & 3) * 10;
    char* buffer = display.line_buffer(line) + row;

    // Cell separators (skip outer edges).
    if (row != 0)                buffer[0]  = kDelimiter;
    if ((row + 10) != kLcdWidth) buffer[10] = kDelimiter;

    uint8_t is_wave = IsWaveCell(lockable);
    // Short name. Wave cells use 2-char abbr (positions 1..2) and grow the
    // value field to 6 chars; everything else uses the standard 4/4.
    uint8_t abbr_off = cell_global * 4;
    uint8_t abbr_len = is_wave ? 2 : 4;
    for (uint8_t c = 0; c < abbr_len; ++c) {
      char ch = pgm_read_byte(kAbbr + abbr_off + c);
      if (i == cursor_in_page && ch >= 'a' && ch <= 'z') {
        ch -= 0x20;
      }
      buffer[1 + c] = ch;
    }
    // Pad position 3..4 with spaces for wave cells (value field starts at 3).
    if (is_wave) {
      buffer[3] = ' ';
      buffer[4] = ' ';
    }

    // Merged subs cell — read both kSPSSUB and kSPREPT (lock or default)
    // and render the combined glyph.
    if (lockable == kSubsMergedSentinel) {
      int8_t  ssub_v;
      uint8_t rept_v;
      if (held_step != 0xff &&
          (tr.steps[held_step].lock_flags[2] & (1 << kSPSSUB))) {
        ssub_v = static_cast<int8_t>(tr.steps[held_step].steppage[kSPSSUB]);
      } else {
        ssub_v = static_cast<int8_t>(tr.defaults[16 + kSPSSUB]);
      }
      if (held_step != 0xff &&
          (tr.steps[held_step].lock_flags[2] & (1 << kSPREPT))) {
        rept_v = tr.steps[held_step].steppage[kSPREPT];
      } else {
        rept_v = tr.defaults[16 + kSPREPT];
      }
      buffer[5] = ' ';
      buffer[6] = ' ';
      buffer[7] = ' ';
      buffer[8] = ' ';
      if (ssub_v == -2) {
        memcpy_P(&buffer[5], PSTR("Edit"), 4);
      } else if (ssub_v == -1) {
        memcpy_P(&buffer[5], PSTR("Cust"), 4);
      } else if (rept_v > 0) {
        buffer[6] = '0' + (rept_v > 9 ? 9 : rept_v);
        buffer[8] = 'r';
      } else if (ssub_v > 0) {
        buffer[6] = '0' + (ssub_v > 9 ? 9 : ssub_v);
        buffer[8] = 'x';
      } else {
        buffer[8] = '0';
      }
      buffer[9] = ' ';
      continue;
    }

    // Resolve value for display.
    uint8_t v;
    if (lockable == 0xff) {
      v = multi.part(track).GetValue(patch_addr);
    } else if (held_step != 0xff &&
               (tr.steps[held_step].lock_flags[lockable >> 3] &
                (1 << (lockable & 7)))) {
      const SeqStep& step = tr.steps[held_step];
      uint8_t buf_page = lockable >> 3;
      uint8_t buf_idx  = lockable & 7;
      v = (buf_page == 0) ? step.page1[buf_idx]
        : (buf_page == 1) ? step.page2[buf_idx]
                          : step.steppage[buf_idx];
    } else {
      v = tr.defaults[lockable];
    }

    if (lockable == 0) {
      // NOTE — render as 3-char note name, right-aligned in the 4-char field.
      buffer[5] = ' ';
      WriteNoteName(&buffer[6], v);
    } else if (is_wave) {
      // Wave cells — 6-char value at offset 3..8.
      for (uint8_t k = 3; k <= 8; ++k) buffer[k] = ' ';
      ResourcesManager::LoadStringResource(STR_RES_NONE + v, &buffer[3], 6);
      AlignRight(&buffer[3], 6);
    } else if (patch_addr == 11) {
      // Sub-osc waveform — 4-char text name.
      for (uint8_t k = 5; k <= 8; ++k) buffer[k] = ' ';
      ResourcesManager::LoadStringResource(STR_RES_SQU1 + v, &buffer[5], 4);
      AlignRight(&buffer[5], 4);
    } else if (IsSignedLockable(lockable)) {
      WriteI8Right(&buffer[5], v);
    } else if (lockable == 16) {
      // PROB — render as percentage. Storage 0..127 → display 0%..100%.
      uint16_t pct = (static_cast<uint16_t>(v) * 100) / 127;
      if (pct > 100) pct = 100;
      buffer[5] = ' ';
      buffer[6] = ' ';
      buffer[7] = ' ';
      if (pct >= 100) {
        buffer[5] = '1'; buffer[6] = '0'; buffer[7] = '0';
      } else if (pct >= 10) {
        buffer[6] = '0' + (pct / 10);
        buffer[7] = '0' + (pct % 10);
      } else {
        buffer[7] = '0' + pct;
      }
      buffer[8] = '%';
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
