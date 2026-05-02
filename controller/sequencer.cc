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

static const prog_uint8_t kDefaultPage1[] PROGMEM = {
  60,   // NOTE = middle C
  1,    // WAVE1 = polyblep_saw (index 1 in waveform enum)
  0,    // PARA1
  0,    // BLND = 0 (Osc 1 only, no FM)
  7,    // RTIO = index 7 ≈ ratio 1.0
  0,    // WAVE2 = none
  0,    // PARA2
  64,   // FINE = center (0 cents detune)
};

static const prog_uint8_t kDefaultPage2[] PROGMEM = {
  128,  // LPGD = medium decay
  0,    // LPGA = no filter envelope
  64,   // LPGO = center (vactrol-like spread)
  0,    // NOIS = no noise
  64,   // PITD = medium pitch env decay
  64,   // PITA = center (no pitch env amount)
  0,    // WAVE_sub = none
  0,    // SUB = no sub-osc
};

static const prog_uint8_t kDefaultStepPage[] PROGMEM = {
  255,  // PROB = always fire
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
  0,    // LSHP = triangle LFO
  0,    // LFOD = destination: pitch
  128,  // LFOS = medium speed
  0,    // LFOA = no LFO amount
  0,    // LFOR = free-run
  0,    // TRAK = no pitch tracking
  0,    // E1ATK = fast Env1 attack
  0,    // E2ATK = fast Env2 attack
  0,    // E3ATK = fast Env3 attack
  0,    // E1CRV = linear curve
  0,    // E2CRV = linear curve
  0,    // E3CRV = linear curve
  0,    // PHSE = no phase reset
  0,    // SMTH = no portamento
  0,    // reserved
  0,    // OSC1R = osc1 range 0
  0,    // OSC2R = osc2 range 0
  0,    // OSC2D = osc2 detune center
  0,    // FMOP = no FM crossmod
  0,    // FUZZ = no fuzz
  20,   // E1SUS = Env1 sustain (matches init_patch)
  60,   // E1REL = Env1 release
  20,   // E2SUS = Env2 sustain
  60,   // E2REL = Env2 release
  20,   // E3SUS = Env3 sustain
  60,   // E3REL = Env3 release
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
    memset(tr.shadow, 0, 5);
    memset(tr.mod,    kModNone, 8);
  }
  global_.transport    = kSeqStopped;
  global_.hold_mode    = 0;
  global_.swing        = 0;
  global_.active_track = 0;
  global_.lock_page    = 0;
  global_.held_step    = 0xff;
  memset(global_.mod,       kModNone, 16);
  memset(global_._reserved, 0,        10);
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

void Sequencer::FireStep(uint8_t t, uint8_t step_index) {
  SeqTrack& tr = tracks_[t];
  // Phase 3: no lock processing yet — always use track defaults.
  uint8_t note     = tr.defaults[kP1NOTE];
  uint8_t velocity = tr.defaults[16 + kSPVEL];
  voicecard_tx.Trigger(t, static_cast<uint16_t>(note) << 7, velocity, 0);
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
