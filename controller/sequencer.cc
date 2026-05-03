// Copyright 2011 Emilie Gillet.
//
// Phase 3: Sequencer core implementation.

#include "controller/sequencer.h"

#include "avrlib/random.h"
#include "avrlib/string.h"
#include "controller/multi.h"
#include "controller/voicecard_tx.h"

namespace ambika {

/* extern */
Sequencer sequencer;

// CDIV lookup: pattern[kPatCDIV] indexes this table (PROGMEM).
static const prog_uint8_t kCDivValues[] PROGMEM = { 1, 2, 3, 4, 6, 8, 12, 16 };

// 12-bit scale masks (bit i = semitone i above ROOT is allowed).
//   chro = chromatic (all 12)
//   maj  = ionian       0,2,4,5,7,9,11
//   min  = aeolian      0,2,3,5,7,8,10
//   dor  = dorian       0,2,3,5,7,9,10
//   mix  = mixolydian   0,2,4,5,7,9,10
//   pMa  = penta major  0,2,4,7,9
//   pMi  = penta minor  0,3,5,7,10
//   blu  = blues        0,3,5,6,7,10
static const prog_uint16_t kScaleMasks[] PROGMEM = {
  0x0fff, 0x0ab5, 0x05ad, 0x06ad,
  0x06b5, 0x0295, 0x04a9, 0x04d1,
};

// Walk down to the nearest semitone allowed by the scale relative to root.
// note 0..127, scale_idx 0..7, root 0..11. note=0..11 is C0; ROOT shifts the
// allowed-set within each octave.
static uint8_t QuantizeToScale(uint8_t note, uint8_t scale_idx, uint8_t root) {
  if (scale_idx == 0) return note;  // chromatic — no-op
  uint16_t mask = pgm_read_word(&kScaleMasks[scale_idx & 7]);
  // Offset from root within the 12-tone octave.
  int8_t offset = static_cast<int8_t>(note % 12) - static_cast<int8_t>(root % 12);
  if (offset < 0) offset += 12;
  for (uint8_t i = 0; i < 12; ++i) {
    int8_t test = offset - static_cast<int8_t>(i);
    if (test < 0) test += 12;
    if (mask & (1U << test)) {
      // Step down by i semitones (or wrap into the octave below if it crosses).
      return (note >= i) ? note - i : 0;
    }
  }
  return note;
}

static const prog_uint8_t kDefaultPage1[] PROGMEM = {
  60,   // NOTE = middle C
  1,    // WAVE1 = polyblep_saw (index 1 in waveform enum)
  0,    // PARA1
  0,    // BLND = 0 (Osc 1 only, no FM)
  7,    // RTIO = osc2:osc1 ratio (index 7 ≈ 1.0; reserved for future linear-FM)
  0,    // WAVE2 = none
  0,    // PARA2
  0,    // FINE = OSC1 detune (0 = centered, int8_t)
};

static const prog_uint8_t kDefaultPage2[] PROGMEM = {
  40,   // E1DEC = env1 decay (matches init_patch)
  60,   // E1REL = env1 release
  40,   // E2DEC
  60,   // E2REL
  40,   // E3DEC
  60,   // E3REL
  0,    // NOIS = no noise
  0,    // SUB = no sub-osc
};

static const prog_uint8_t kDefaultStepPage[] PROGMEM = {
  127,  // PROB = always fire (range 0..127 = 0%..100%)
  0,    // SSUB = normal (no ratchet)
  0,    // REPT = no repeat
  0,    // RATE = normal (1×)
  100,  // VEL
  0,    // GLID = no glide
  0,    // MINT = off (no mutate)
  0,    // MDIR = up
};

static const prog_uint8_t kDefaultConfig[] PROGMEM = {
  96,   // FREQ = cutoff at 3/4 open
  0,    // RES = no resonance
  0,    // TYPE = LP mode
  0,    // DRIV = no drive
  0,    // BITS = no bit reduction
  0,    // LSHP = triangle LFO4 shape
  0,    // LFO4D = destination: PARAMETER_1
  128,  // LFOS = medium LFO4 rate
  0,    // LFO4A = no LFO4 amount
  0,    // LFOR = free-run
  0,    // TRAK = no KB tracking
  0,    // E1ATK = fast attack
  0,    // E2ATK = fast attack
  0,    // E3ATK = fast attack
  64,   // E1CRV = centered (range 0=linear .. 127=expo per voice_envelopes.md)
  64,   // E2CRV
  64,   // E3CRV
  0,    // PHSE = no phase reset
  0,    // SMTH = no portamento
  0,    // reserved
  0,    // OSC1R = 0
  0,    // OSC2R = 0
  0,    // OSC2D = center
  0,    // FMOP = no FM
  0,    // FUZZ = no fuzz
  127,  // E1DEPT = ENV1→VCA full depth (round 5: 0..127 unipolar)
  64,   // E2DEPT = ENV2→VCF depth (round 5 mid; can push higher now)
  0,    // E3DEPT = ENV3→pitch depth (default off)
  0,    // WSUB = WAVEFORM_SUB_OSC_SQUARE_1
};

// Fixed mod routing sent by Part::Touch() — amounts at bytes 58/72/73/82 come from config[].
const prog_uint8_t kDefaultMod[42] PROGMEM = {
  0, 0, 0,    // slot  0: cleared
  0, 0, 0,    // slot  1: cleared
  2, 4, 0,    // slot  2: ENV_3 → OSC_1_2_COARSE (pitch depth, amount from E3DEPT)
  0, 0, 0,    // slot  3: cleared
  0, 0, 0,    // slot  4: cleared
  0, 0, 0,    // slot  5: cleared
  0, 0, 0,    // slot  6: cleared
  6, 0, 0,    // slot  7: LFO_4 → dest/amount from LFO4D/LFO4A
  11, 0, 0,   // slot  8: SEQ_1 → PARAMETER_1
  12, 1, 0,   // slot  9: SEQ_2 → PARAMETER_2
  0, 18, 63,  // slot 10: ENV_1 → VCA (amount from E1DEPT)
  14, 18, 127, // slot 11: VELOCITY → VCA (round 5a-1: was 0 — vel was inert)
  16, 4, 0,   // slot 12: PITCH_BEND → OSC_1_2_COARSE
  0, 0, 0,    // slot 13: cleared
};

static const prog_uint8_t kDefaultPattern[] PROGMEM = {
  kDirnFwd,  // DIRN = forward
  0,          // CDIV index 0 → ÷1 (normal rate)
  0,          // ROTA = no rotation
  8,          // LENG = 8 steps
  0,          // SCAL = chromatic
  0,          // ROOT = C
  60,         // BPCH = middle C
  255,        // OLEV = full output level
};

void Sequencer::Init() {
  for (uint8_t t = 0; t < kNumVoices; ++t) {
    SeqTrack& tr = tracks_[t];
    for (uint8_t s = 0; s < kNumStepsPerTrack; ++s) {
      SeqStep& step = tr.steps[s];
      memcpy_P(step.page1,    kDefaultPage1,    8);
      memcpy_P(step.page2,    kDefaultPage2,    8);
      memcpy_P(step.steppage, kDefaultStepPage, 8);
      step.lock_flags[0] = 0;
      step.lock_flags[1] = 0;
      step.lock_flags[2] = 0;
      step.step_flags    = 0;
      step.substep_bits  = 0;
    }
    memcpy_P(tr.pattern,       kDefaultPattern,  8);
    memcpy_P(&tr.defaults[0],  kDefaultPage1,    8);
    memcpy_P(&tr.defaults[8],  kDefaultPage2,    8);
    memcpy_P(&tr.defaults[16], kDefaultStepPage, 8);
    memcpy_P(tr.config,        kDefaultConfig,   kCfgSIZE);
    memset(tr.shadow, 0, kShdwSIZE);
  }
  global_.transport    = kSeqStopped;
  global_.hold_mode    = 0;
  global_.swing        = 0;
  global_.active_track = 0;
  global_.lock_page    = 0;
  global_.held_step    = 0xff;
}

void Sequencer::Clock(uint8_t ticks) {
  if (global_.transport != kSeqPlaying) return;

  for (uint8_t t = 0; t < kNumVoices; ++t) {
    SeqTrack& tr   = tracks_[t];
    uint8_t cdiv   = pgm_read_byte(kCDivValues + tr.pattern[kPatCDIV]);
    uint8_t period = kNumTicksPerStep * cdiv;

    tr.shadow[kShdwTICK] += ticks;
    if (tr.shadow[kShdwTICK] >= period) {
      tr.shadow[kShdwTICK] -= period;
      uint8_t len   = tr.pattern[kPatLENG];
      if (len == 0) len = 1;
      uint8_t step  = tr.shadow[kShdwSTEP];
      uint8_t fired = (step + tr.pattern[kPatROTA]) % len;
      voicecard_tx.Release(t);
      tr.shadow[kShdwLAST] = fired;
      if (tr.steps[fired].step_flags & kStepFlagOn) {
        FireStep(t, fired);
      }
      AdvanceStep(t);
    }
  }
}

void Sequencer::AdvanceStep(uint8_t t) {
  SeqTrack& tr = tracks_[t];
  uint8_t len  = tr.pattern[kPatLENG];
  if (len == 0) len = 1;
  uint8_t step = tr.shadow[kShdwSTEP];

  switch (tr.pattern[kPatDIRN]) {
    default:
    case kDirnFwd:
      step = (step + 1 >= len) ? 0 : step + 1;
      break;
    case kDirnRev:
      step = (step == 0) ? len - 1 : step - 1;
      break;
    case kDirnPend:
      if (tr.shadow[kShdwDIR] == 0) {
        ++step;
        if (step >= len) {
          tr.shadow[kShdwDIR] = 1;
          step = (len > 1) ? len - 2 : 0;
        }
      } else {
        if (step == 0) {
          tr.shadow[kShdwDIR] = 0;
          step = (len > 1) ? 1 : 0;
        } else {
          --step;
        }
      }
      break;
    case kDirnRnd:
      step = Random::GetByte() % len;
      break;
  }

  tr.shadow[kShdwSTEP] = step;
}

// Resolve a step-page (steppage[]) byte: lock if set, otherwise track default.
// step_param is a kSP* index in [0..7].
static inline uint8_t ResolveStepByte(
    const SeqTrack& tr, uint8_t step_index, uint8_t step_param) {
  const SeqStep& step = tr.steps[step_index];
  return (step.lock_flags[2] & (1 << step_param))
      ? step.steppage[step_param]
      : tr.defaults[16 + step_param];
}

void Sequencer::FireStep(uint8_t t, uint8_t step_index) {
  SeqTrack& tr = tracks_[t];
  const SeqStep& step = tr.steps[step_index];

  // PROB — uint8_t 0..127 (matches pot range). 127 = always fire, 0 = never.
  // Compare 7-bit random to prob: rand 0..127 > prob ↔ skip probability.
  uint8_t prob = ResolveStepByte(tr, step_index, kSPPROB);
  if ((Random::GetByte() & 0x7F) > prob) return;

  // Resolve 16-byte snapshot: page1[8] || page2[8].
  uint8_t snapshot[16];
  for (uint8_t i = 0; i < 16; ++i) {
    uint8_t locked = step.lock_flags[i >> 3] & (1 << (i & 7));
    const uint8_t* src = locked
        ? (i < 8 ? &step.page1[i] : &step.page2[i - 8])
        : &tr.defaults[i];
    snapshot[i] = *src;
  }

  // Note: lock-or-default at kP1NOTE, then quantize by track scale + root.
  uint8_t note = snapshot[kP1NOTE];
  note = QuantizeToScale(note, tr.pattern[kPatSCAL] & 7, tr.pattern[kPatROOT]);

  // Velocity: lock-or-default, then scale by track VOL (255 = identity).
  uint8_t velocity = ResolveStepByte(tr, step_index, kSPVEL);
  velocity = (static_cast<uint16_t>(velocity) * tr.pattern[kPatVOL]) >> 8;

  // GLID: any non-zero value flips the SPI command bit (0x12 → 0x13).
  uint8_t glid = ResolveStepByte(tr, step_index, kSPGLID);
  uint8_t legato = glid ? 1 : 0;

  voicecard_tx.TriggerWithSnapshot(
      t, static_cast<uint16_t>(note) << 7, velocity, legato, snapshot);
}

void Sequencer::Play() {
  if (global_.transport == kSeqStopped) {
    Reset();
  }
  global_.transport = kSeqPlaying;
}

void Sequencer::Pause() {
  if (global_.transport == kSeqPlaying) {
    global_.transport = kSeqPaused;
    for (uint8_t t = 0; t < kNumVoices; ++t) {
      voicecard_tx.Release(t);
    }
  } else if (global_.transport == kSeqPaused) {
    global_.transport = kSeqPlaying;
  }
}

void Sequencer::Reset() {
  for (uint8_t t = 0; t < kNumVoices; ++t) {
    voicecard_tx.Release(t);
    tracks_[t].shadow[kShdwSTEP] = 0;
    tracks_[t].shadow[kShdwTICK] = 0;
    tracks_[t].shadow[kShdwREPT] = 0;
    tracks_[t].shadow[kShdwSSUB] = 0;
    tracks_[t].shadow[kShdwDIR]  = 0;
  }
}

}  // namespace ambika
