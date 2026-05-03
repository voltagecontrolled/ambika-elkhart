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

/* static */
bool SeqStepsPage::editing_substeps_ = false;

/* static */
uint8_t SeqStepsPage::substep_step_ = 0;

/* static */
uint8_t SeqStepsPage::substep_count_ = 8;

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
  "notevel vamtratesubsprobglidgtim"  // page 1 = S5a (step behavior)
  "noisw1  pa1 tun2mix w2  pa2 fin2"  // page 2 = S5b (voice 1: osc / mix)
  "freqfdecfamtadecpdecpamtsub wave"; // page 3 = S5c (voice 2: filter/env/sub)

// Per-cell target. 0..27 = lockable param index (writes to tr.defaults[N]
// or step.{page1|page2|steppage|page3}[N%8]; lock_flags bit N marks per-step lock).
// 0xfe = merged SSUB+REPT cell (`subs` on S5a) — special-cased in OnPot
//        and UpdateScreen; writes to both kSPSSUB and kSPREPT with mutex.
//
// Lockable indices: page1 0..7, page2 8..15, steppage 16..23, page3 24..27.
// tun2 / fin2 reclaim the dead E1REL / E2REL slots (lockable 9 / 11).
// freq / famt / pamt / wave are now lockable (24..27) instead of config-mapped.
static const prog_uint8_t kCellLockable[24] PROGMEM = {
  // S5a: note, vel, vamt(cfg), rate | subs(merged), prob, glid, gtim(cfg)
  // vamt/gtim are config-mapped (0xff); MINT/MDIR now live only in substep editor.
  0,    20,   0xff, 19,
  0xfe, 16,   21,   0xff,
  // S5b: nois, w1, pa1, tun2 | mix(blnd), w2, pa2, fin2
  14,   1,    2,    9,
  3,    5,    6,    11,
  // S5c: freq, fdec, famt, adec | pdec, pamt, sub, wave  (all lockable now)
  24,   10,   25,   8,
  12,   26,   15,   27,
};

// Patch address for config-mapped cells (0xff for lockable cells).
// vamt = vel→VCA amount  → patch addr 85 (mod slot 11 amount)
// gtim = portamento time → virtual addr 203 (→ VOICECARD_DATA_PART offset 6)
// Page3 cells use kPage3PatchAddrs for their live-feedback path.
static const prog_uint8_t kCellPatchAddr[24] PROGMEM = {
  0xff, 0xff, 85,   0xff, 0xff, 0xff, 0xff, 203,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
};

// Voicecard patch addrs for the 4 page3 lockables (indexed by kP3*).
static const prog_uint8_t kPage3PatchAddrs[4] PROGMEM = {
  16, 22, 58, 11,   // FREQ, FAMT, PAMT, WAVE
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

// Map a 0..127 pot value to a valid osc waveform index, skipping the
// 9 stripped CZ resonant variants (indices 6..14). Valid set: 0..5, 15..42
// = 34 entries. Pot maps 0..127 → 0..33, then +9 offset for indices ≥ 6.
static uint8_t MapWaveform(uint8_t value) {
  uint8_t idx = ScalePot(value, 33);
  return (idx <= 5) ? idx : idx + 9;
}

// Sub-osc waveform cell — 4-char text name from STR_RES_SQU1.
static inline uint8_t IsSubWaveCell(uint8_t lockable) {
  return lockable == 27;   // kP3WAVE (24 + kP3WAVE=3)
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

// Encoder click: enter substep editor when on subs cell with step held;
// exit substep editor on any second click.
/* static */
uint8_t SeqStepsPage::OnClick() {
  if (editing_substeps_) {
    editing_substeps_ = false;
    return 1;
  }
  if (cursor_ == 4) {
    for (uint8_t s = 0; s < 8; ++s) {
      if (ui.switch_held(s)) {
        substep_step_ = 7 - s;
        editing_substeps_ = true;
        SeqTrack* tr = sequencer.mutable_track(ui.state().active_part);
        SeqStep& step = tr->steps[substep_step_];
        // Set SSUB=-2 (Edit) so substep_bits drives playback.
        step.steppage[kSPSSUB] = static_cast<uint8_t>(static_cast<int8_t>(-2));
        step.lock_flags[2] |= (1 << kSPSSUB);
        // Read substep count from REPT lock or default.
        uint8_t rept_v = (step.lock_flags[2] & (1 << kSPREPT))
            ? step.steppage[kSPREPT]
            : tr->defaults[16 + kSPREPT];
        substep_count_ = rept_v ? rept_v : 8;
        // Auto-enable bits for all active slots when none are set yet.
        if (step.substep_bits == 0) {
          step.substep_bits = (substep_count_ < 8)
              ? static_cast<uint8_t>((1 << substep_count_) - 1)
              : 0xff;
        }
        return 1;
      }
    }
  }
  return 0;
}

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

  // Substep editor: pot 4 = substep count, pots 6/7 = MINT/MDIR, rest swallowed.
  if (editing_substeps_) {
    SeqTrack* tr = sequencer.mutable_track(track);
    SeqStep& step = tr->steps[substep_step_];
    if (index == 4) {
      // Substep count 1..8 stored in REPT (0 = unconstrained = 8).
      uint8_t cnt = ScalePot(value, 7) + 1;  // 1..8
      if (cnt > substep_count_) {
        // Auto-enable newly exposed slots.
        for (uint8_t b = substep_count_; b < cnt; ++b) step.substep_bits |= (1 << b);
      } else if (cnt < substep_count_) {
        // Mask out slots beyond new count.
        if (cnt < 8) step.substep_bits &= static_cast<uint8_t>((1 << cnt) - 1);
      }
      substep_count_ = cnt;
      step.steppage[kSPREPT] = cnt;
      step.lock_flags[2] |= (1 << kSPREPT);
      return 1;
    }
    if (index < 6) return 0;
    uint8_t param_idx = (index == 6) ? kSPMINT : kSPMDIR;
    uint8_t mapped = (param_idx == kSPMDIR) ? ScalePot(value, 1) : value;
    step.steppage[param_idx] = mapped;
    step.lock_flags[2] |= (1 << param_idx);
    return 1;
  }

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

  // Merged subs cell — deadzone at 12 o'clock, CCW=repeats 8r..1r, CW=ratchets 1x..8x.
  // 0..55: 8 bands of 7 → 8r..1r. 56..71: deadzone (normal). 72..127: 1x..8x.
  if (lockable == kSubsMergedSentinel) {
    int8_t  ssub_v = 0;
    uint8_t rept_v = 0;
    if (value < 56) {
      rept_v = 8 - (value / 7);
      if (rept_v < 1) rept_v = 1;
    } else if (value > 71) {
      uint8_t r = (value - 72) / 7 + 1;
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
    // WAVE1 / WAVE2: skip CZ resonant variants (indices 6..14 = silence).
    mapped = MapWaveform(value);
  } else if (lockable == 27) {
    // Sub-osc waveform (kP3WAVE).
    mapped = ScalePot(value, WAVEFORM_SUB_OSC_LAST - 1);
  } else if (lockable == 9) {
    // tun2 — Osc2 coarse pitch, UNIT_INT8 -24..+24.
    mapped = MapPotInt8(value, -24, 24);
  } else if (lockable == 11) {
    // fin2 — Osc2 detune, UNIT_INT8 -64..+64.
    mapped = MapPotInt8(value, -64, 64);
  } else if (lockable == 21) {
    // GLID — binary on/off.
    mapped = ScalePot(value, 1);
  } else if (lockable == 3) {
    // BLND (mix) — clamp to crossfade range. Upper half (≥ 64) was reserved
    // for future linear-FM and isn't implemented; the dead range produced
    // glitchy output. 0..127 pot → 0..63 crossfade only.
    mapped = value >> 1;
  }
  // freq / famt / pamt pass through 0..127 (matches PAGE_FILTER pot semantics
  // and the round-5 unipolar env-depth range).

  // Config-mapped cells (smth, vamt) push directly to the voicecard.
  if (lockable == 0xff) {
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
                  : (buf_page == 2) ? &step.steppage[buf_idx]
                                    : &step.page3[buf_idx];
    *slot = mapped;
    step.lock_flags[lockable >> 3] |= (1 << (lockable & 7));
    step_lock_dirty_ |= (1 << held_step);
    ui.inhibit_switch(1 << held_sr);
  } else {
    tr->defaults[lockable] = mapped;
    // Page3 lockables also push live to the voicecard so filter/wave respond
    // immediately when adjusting the default (same as config-mapped cells did).
    if (lockable >= 24 && lockable <= 27) {
      uint8_t p3_addr = pgm_read_byte(&kPage3PatchAddrs[lockable - 24]);
      multi.mutable_part(track)->SetValue(p3_addr, mapped, 0);
    }
  }
  return 1;
}

// Step toggle on release. Suppressed if a pot moved while this step was held.
// In substep editor mode, toggles substep_bits instead of step_flags.
/* static */
uint8_t SeqStepsPage::OnKey(uint8_t key) {
  if (key > SWITCH_8) return 0;
  uint8_t track = ui.state().active_part;
  if (editing_substeps_) {
    if (key >= substep_count_) return 1;  // inactive slot — swallow but don't toggle
    SeqStep& s = sequencer.mutable_track(track)->steps[substep_step_];
    s.substep_bits ^= (1 << key);
    return 1;
  }
  if (step_lock_dirty_ & (1 << key)) {
    step_lock_dirty_ &= ~(1 << key);
    return 1;
  }
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

  if (editing_substeps_) {
    const SeqStep& step = tr.steps[substep_step_];
    char* line0 = display.line_buffer(0);
    char* line1 = display.line_buffer(1);
    for (uint8_t i = 0; i < kLcdWidth; ++i) { line0[i] = ' '; line1[i] = ' '; }
    // subs cell (offset 0..9): show substep count
    memcpy_P(&line0[1], PSTR("subs"), 4);
    WriteU8Right(&line0[5], substep_count_);
    line0[9] = ' ';
    // MINT cell (offset 10..19)
    line0[10] = kDelimiter;
    memcpy_P(&line0[11], PSTR("MINT"), 4);
    WriteU8Right(&line0[15], step.steppage[kSPMINT]);
    line0[19] = ' ';
    // MDIR cell (offset 20..29)
    line0[20] = kDelimiter;
    memcpy_P(&line0[21], PSTR("MDIR"), 4);
    WriteU8Right(&line0[25], step.steppage[kSPMDIR]);
    line0[29] = ' ';
    // Substep bit pattern on line 1 (8 slots × 4 chars = 32 chars).
    // Slots >= substep_count_ are shown blank (not interactive).
    uint8_t bits = step.substep_bits;
    for (uint8_t b = 0; b < 8; ++b) {
      uint8_t pos = b * 4;
      line1[pos]     = ' ';
      line1[pos + 1] = ' ';
      line1[pos + 2] = (b < substep_count_) ? ((bits & (1 << b)) ? '#' : '-') : ' ';
      line1[pos + 3] = ' ';
    }
    return;
  }

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
      if (rept_v > 0) {
        buffer[6] = '0' + (rept_v > 9 ? 9 : rept_v);
        buffer[8] = 'r';
      } else if (ssub_v > 0) {
        buffer[6] = '0' + (ssub_v > 9 ? 9 : ssub_v);
        buffer[8] = 'x';
      } else if (ssub_v == -2) {
        memcpy_P(&buffer[5], PSTR("edit"), 4);
      } else {
        buffer[8] = '0';
      }
      buffer[9] = ' ';
      continue;
    }

    // Resolve value for display.
    uint8_t v;
    if (lockable == 0xff) {
      // Config-mapped cell: read live value from config via GetValue.
      v = multi.part(track).GetValue(patch_addr);
    } else if (held_step != 0xff &&
        (tr.steps[held_step].lock_flags[lockable >> 3] & (1 << (lockable & 7)))) {
      const SeqStep& step = tr.steps[held_step];
      uint8_t buf_page = lockable >> 3;
      uint8_t buf_idx  = lockable & 7;
      v = (buf_page == 0) ? step.page1[buf_idx]
        : (buf_page == 1) ? step.page2[buf_idx]
        : (buf_page == 2) ? step.steppage[buf_idx]
                          : step.page3[buf_idx];
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
    } else if (IsSubWaveCell(lockable)) {
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
  if (editing_substeps_) {
    uint8_t bits = tr.steps[substep_step_].substep_bits;
    for (uint8_t i = 0; i < kNumStepsPerTrack; ++i) {
      if (i < substep_count_ && (bits & (1 << i))) {
        leds.set_pixel(LED_1 + i, 0x0f);
      }
    }
    return;
  }
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
