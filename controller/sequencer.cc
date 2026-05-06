// Copyright 2011 Emilie Gillet.
//
// Phase 3: Sequencer core implementation.

#include "controller/sequencer.h"

#include "avrlib/random.h"
#include "avrlib/string.h"
#include "controller/multi.h"
#include "controller/voicecard_tx.h"
#include "controller/ui_pages/seq_mixer_page.h"

namespace ambika {

/* extern */
Sequencer sequencer;

// Rate lookup: pattern[kPatCDIV] (track) indexes this table directly (0..14).
// Per-step kSPRATE uses 0 as the "inherit track" sentinel and 1..15 as direct
// picks (with rate-1 indexing into this table). Values are step periods in
// MIDI ticks at 24 PPQN (one Multi::Clock() = 1 tick).
// Labels (musical notation): 32, 16t, 16, 8t, 16d, 8, 4t, 8d, 4, 2t, 4d, 2,
//                            1, 1d, 2B.
static const prog_uint8_t kRateValues[] PROGMEM = {
    3, 4, 6, 8, 9, 12, 16, 18, 24, 32, 36, 48, 96, 144, 192
};

// MINT chord shapes (mutation step). Value 0 = off (no walk). Values 1..12
// index kChordOffsets / kChordSizes to slice into kChordIntervals. Each chord
// is a list of semitone offsets within an octave (0..11); the walk climbs by
// 12 semitones per cycle through the chord, capped at MOCT octaves.
//   1 oct  {0}             — pure octave climb
//   2 pwr  {0,7}           — root + 5
//   3 maj  {0,4,7}
//   4 min  {0,3,7}
//   5 sus2 {0,2,7}
//   6 sus4 {0,5,7}
//   7 dim  {0,3,6}
//   8  7   {0,4,7,10}      — dominant 7
//   9 m7   {0,3,7,10}
//  10 M7   {0,4,7,11}
//  11 7sus {0,5,7,10}
//  12 pent {0,3,5,7,10}    — minor pentatonic
static const prog_uint8_t kChordIntervals[] PROGMEM = {
  /* oct  */  0,
  /* pwr  */  0, 7,
  /* maj  */  0, 4, 7,
  /* min  */  0, 3, 7,
  /* sus2 */  0, 2, 7,
  /* sus4 */  0, 5, 7,
  /* dim  */  0, 3, 6,
  /*  7   */  0, 4, 7, 10,
  /* m7   */  0, 3, 7, 10,
  /* M7   */  0, 4, 7, 11,
  /* 7sus */  0, 5, 7, 10,
  /* pent */  0, 3, 5, 7, 10,
};
static const prog_uint8_t kChordOffsets[] PROGMEM = {
  0, 1, 3, 6, 9, 12, 15, 18, 22, 26, 30, 34
};
static const prog_uint8_t kChordSizes[] PROGMEM = {
  1, 2, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5
};

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
  0,    // RTIO = crossmod/FM amount (reserved for future linear-FM)
  0,    // WAVE2 = none
  0,    // PARA2
  0,    // FINE = OSC1 detune (0 = centered, int8_t)
};

static const prog_uint8_t kDefaultPage2[] PROGMEM = {
  40,   // E1DEC = env1 decay
  0,    // TUN2  = OSC2 coarse pitch (int8, 0 = no offset)
  40,   // E2DEC
  0,    // FIN2  = OSC2 detune (int8, 0 = no detune)
  40,   // E3DEC
  0,    // E3REL = dead slot
  0,    // NOIS  = no noise
  0,    // SUB   = no sub-osc
};

static const prog_uint8_t kDefaultPage3[] PROGMEM = {
  96,   // FREQ = cutoff at 3/4 open (matches kDefaultConfig[kCfgFREQ])
  64,   // FAMT = ENV2→VCF depth mid (matches kDefaultConfig[kCfgE2DEPT])
  0,    // PAMT = ENV3→pitch depth off
  0,    // WAVE = WAVEFORM_SUB_OSC_SQUARE_1
};

static const prog_uint8_t kDefaultStepPage[] PROGMEM = {
  127,  // PROB = always fire (range 0..127 = 0%..100%)
  0,    // SSUB = normal (no ratchet)
  0,    // REPT = no repeat
  0,    // RATE = 0 → " trk" sentinel (inherit track rate)
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
  127,  // VELAMT = full velocity→VCA (mod slot 11 amount)
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

// Resolve a step-page (steppage[]) byte: lock if set, otherwise track default.
// step_param is a kSP* index in [0..7].
static inline uint8_t ResolveStepByte(
    const SeqTrack& tr, uint8_t step_index, uint8_t step_param) {
  const SeqStep& step = tr.steps[step_index];
  return (step.lock_flags[2] & (1 << step_param))
      ? step.steppage[step_param]
      : tr.defaults[16 + step_param];
}

static const prog_uint8_t kDefaultPattern[] PROGMEM = {
  kDirnFwd,  // DIRN = forward
  2,          // RATE index 2 → " 16" = 16th note per step
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
      memcpy_P(step.page3,    kDefaultPage3,    4);
      step.lock_flags[0] = 0;
      step.lock_flags[1] = 0;
      step.lock_flags[2] = 0;
      step.lock_flags[3] = 0;
      step.step_flags    = 0;
      step.substep_bits  = 0;
    }
    memcpy_P(tr.pattern,       kDefaultPattern,  8);
    memcpy_P(&tr.defaults[0],  kDefaultPage1,    8);
    memcpy_P(&tr.defaults[8],  kDefaultPage2,    8);
    memcpy_P(&tr.defaults[16], kDefaultStepPage, 8);
    memcpy_P(&tr.defaults[24], kDefaultPage3,    4);
    memcpy_P(tr.config,        kDefaultConfig,   kCfgSIZE);
    memset(tr.shadow, 0, kShdwSIZE);
  }
  global_.transport    = kSeqStopped;
  global_.hold_mode    = 0;
  global_.swing        = 0;
  global_.active_track = 0;
  global_.lock_page    = 0;
  global_.held_step    = 0xff;
  global_.master_tick  = 0;
}

void Sequencer::Clock(uint8_t ticks) {
  if (global_.transport != kSeqPlaying) return;

  // Master Reset: if mrst != 0, reset all tracks every (mrst + 1)
  // undivided steps. Stored value k → period of (k + 1) steps; k = 0 = off.
  uint8_t mrst = multi.data().master_reset_steps;
  if (mrst != 0) {
    global_.master_tick += ticks;
    uint16_t threshold =
        static_cast<uint16_t>(mrst + 1) * kNumTicksPerStep;
    if (global_.master_tick >= threshold) {
      Reset();
      return;
    }
  }

  for (uint8_t t = 0; t < kNumVoices; ++t) {
    SeqTrack& tr = tracks_[t];

    // RATE: per-step rate override for the currently-playing step.
    // 0 = inherit track; 1..15 = direct pick from kRateValues[rate-1].
    uint8_t rate = ResolveStepByte(tr, tr.shadow[kShdwLAST], kSPRATE);
    uint8_t cdiv_idx = rate ? (rate - 1) : tr.pattern[kPatCDIV];
    if (cdiv_idx >= 15) cdiv_idx = 14;
    uint8_t period = pgm_read_byte(kRateValues + cdiv_idx);

    tr.shadow[kShdwTICK] += ticks;

    // SSUB ratchet: fire sub-triggers between period boundaries.
    // Only active when a step has been fired (kShdwLAST is valid post-reset).
    // Gate on kShdwPROB so substeps follow the main-step probability decision.
    uint8_t cur = tr.shadow[kShdwLAST];
    int8_t ssub = static_cast<int8_t>(ResolveStepByte(tr, cur, kSPSSUB));
    if (tr.shadow[kShdwTICK] < period) {
      if (ssub > 0 && tr.shadow[kShdwPROB]) {
        // Ratchets: N+1 evenly-spaced fires per period. Slot 0 = main fire.
        uint8_t sub_period = period / (static_cast<uint8_t>(ssub) + 1);
        if (sub_period == 0) sub_period = 1;
        uint8_t slot_now  = tr.shadow[kShdwTICK] / sub_period;
        uint8_t slot_prev = (tr.shadow[kShdwTICK] - ticks) / sub_period;
        if (slot_now != slot_prev && slot_now > 0) {
          voicecard_tx.Release(t);
          if (tr.steps[cur].step_flags & kStepFlagOn) {
            // kStepFlagGated: gate each ratchet slot by substep_bits.
            if (!(tr.steps[cur].step_flags & kStepFlagGated) ||
                (slot_now < 8 && (tr.steps[cur].substep_bits & (1 << slot_now)))) {
              FireStep(t, cur, slot_now);
            }
          }
        }
      }
      // SSUB=-2: custom repeat pattern — fires happen at period boundaries (REPT
      // path below), each gated by substep_bits. No within-period sub-triggers.
    }

    if (tr.shadow[kShdwTICK] >= period) {
      tr.shadow[kShdwTICK] -= period;
      uint8_t len = tr.pattern[kPatLENG];
      if (len == 0) len = 1;

      if (tr.shadow[kShdwREPT] > 0) {
        // REPT: re-fire the last-fired step, no advance.
        // PROB decision is carried from the main fire (kShdwPROB).
        uint8_t last = tr.shadow[kShdwLAST];
        uint8_t rept_total = ResolveStepByte(tr, last, kSPREPT);
        tr.shadow[kShdwREPT]--;
        uint8_t repeat_idx = rept_total - tr.shadow[kShdwREPT];
        voicecard_tx.Release(t);
        if (tr.shadow[kShdwPROB] && (tr.steps[last].step_flags & kStepFlagOn)) {
          int8_t ssub_l = static_cast<int8_t>(ResolveStepByte(tr, last, kSPSSUB));
          if (ssub_l != -2) {
            FireStep(t, last, repeat_idx);
          } else {
            // Custom pattern: gate this repeat by substep_bits.
            if (repeat_idx < 8 && (tr.steps[last].substep_bits & (1 << repeat_idx))) {
              FireStep(t, last, repeat_idx);
            }
          }
        }
        if (tr.shadow[kShdwREPT] == 0) {
          AdvanceStep(t);
        }
      } else {
        uint8_t step  = tr.shadow[kShdwSTEP];
        uint8_t fired = (step + tr.pattern[kPatROTA]) % len;

        // PROB roll first — gates both fire AND SMOD. Ratchets/repeats
        // downstream inherit this decision via kShdwPROB.
        uint8_t prob = ResolveStepByte(tr, fired, kSPPROB);
        tr.shadow[kShdwPROB] =
            ((Random::GetByte() & 0x7F) <= prob) ? 1 : 0;

        if (tr.shadow[kShdwPROB]) {
          // SMOD dispatch. skip = bounded re-advance loop; fwd/rev/dir
          // mutate kPatDIRN (sticky); rjmp/jmp[N] reseat the playhead
          // before firing. Only applied when PROB passes.
          uint8_t fire_now = 1;
          uint8_t guard;
          for (guard = 0; guard < len; ++guard) {
            uint8_t smod = StepSmod(tr.steps[fired]);
            if (smod == kSmodSkip) {
              AdvanceStep(t);
              fired = (tr.shadow[kShdwSTEP] + tr.pattern[kPatROTA]) % len;
              continue;
            }
            if (smod == kSmodFwd) {
              tr.pattern[kPatDIRN] = kDirnFwd;
            } else if (smod == kSmodRev) {
              tr.pattern[kPatDIRN] = kDirnRev;
            } else if (smod == kSmodDir) {
              // Sticky toggle Fwd <-> Rev. From Pend/Rnd, set Rev so the
              // toggle has somewhere to go on the next dir step.
              tr.pattern[kPatDIRN] =
                  (tr.pattern[kPatDIRN] == kDirnRev) ? kDirnFwd : kDirnRev;
            } else if (smod == kSmodRjmp) {
              uint8_t target = Random::GetByte() % len;
              tr.shadow[kShdwSTEP] = target;
              fired = (target + tr.pattern[kPatROTA]) % len;
            } else if (smod >= kSmodJmp1 && smod <= kSmodJmp8) {
              uint8_t target = smod - kSmodJmp1;
              if (target >= len) target = len - 1;
              tr.shadow[kShdwSTEP] = target;
              fired = (target + tr.pattern[kPatROTA]) % len;
            }
            break;
          }
          if (guard >= len) fire_now = 0;  // every step is skip — silent

          voicecard_tx.Release(t);
          tr.shadow[kShdwLAST] = fired;

          if (fire_now && (tr.steps[fired].step_flags & kStepFlagOn)) {
            int8_t ssub_f = static_cast<int8_t>(ResolveStepByte(tr, fired, kSPSSUB));
            // SSUB=-2: gate initial fire on bit 0 of substep_bits.
            if (ssub_f != -2 || (tr.steps[fired].substep_bits & 0x01)) {
              FireStep(t, fired, 0);
            }
          }
          if (fire_now) {
            uint8_t rept = ResolveStepByte(tr, fired, kSPREPT);
            tr.shadow[kShdwREPT] = rept;
            if (rept == 0) {
              AdvanceStep(t);
            }
          } else {
            tr.shadow[kShdwREPT] = 0;  // already advanced len times in loop
          }
        } else {
          // PROB failed — no fire, no SMOD. Just advance normally.
          voicecard_tx.Release(t);
          tr.shadow[kShdwLAST] = fired;
          tr.shadow[kShdwREPT] = 0;
          AdvanceStep(t);
        }
      }
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

void Sequencer::FireStep(uint8_t t, uint8_t step_index, uint8_t sub_idx) {
  // Performance mixer gate: skip triggers for muted/non-solo'd voices.
  if (SeqMixerPage::skip_mask() & (1 << t)) return;

  SeqTrack& tr = tracks_[t];
  const SeqStep& step = tr.steps[step_index];

  // PROB rolled once per main step in Clock(); ratchets/repeats are gated
  // there on tr.shadow[kShdwPROB], so by the time FireStep runs the decision
  // has already been made and we always fire.

  // Resolve 20-byte snapshot: page1[8] || page2[8] || page3[4].
  uint8_t snapshot[20];
  for (uint8_t i = 0; i < 16; ++i) {
    uint8_t locked = step.lock_flags[i >> 3] & (1 << (i & 7));
    const uint8_t* src = locked
        ? (i < 8 ? &step.page1[i] : &step.page2[i - 8])
        : &tr.defaults[i];
    snapshot[i] = *src;
  }
  for (uint8_t i = 0; i < 4; ++i) {
    snapshot[16 + i] = (step.lock_flags[3] & (1 << i))
        ? step.page3[i]
        : tr.defaults[24 + i];
  }

  // Note: lock-or-default at kP1NOTE, then quantize by track scale + root.
  uint8_t note = snapshot[kP1NOTE];
  note = QuantizeToScale(note, tr.pattern[kPatSCAL] & 7, tr.pattern[kPatROOT]);

  // MINT/MDIR/MOCT: arpeggiator-style walk per sub-trigger index.
  // MINT = chord shape (0=off, 1..12 indexes kChordIntervals),
  // MOCT = range cap in octaves (1..4),
  // MDIR = wave shape: 0=up/1=dn (sawtooth, wrap to root),
  //                    2=ud bipolar / 3=ud+ above / 4=ud- below (triangle),
  //                    5=rnd bipolar / 6=rnd+ above / 7=rnd- below (random).
  // The walk visits chord tones in interval order, climbing by 12 semitones
  // each time it cycles through the chord, capped at MOCT octaves above/below.
  if (sub_idx > 0) {
    uint8_t mint = ResolveStepByte(tr, step_index, kSPMINT);
    if (mint > 0 && mint <= 12) {
      uint8_t mdir_byte = ResolveStepByte(tr, step_index, kSPMDIR);
      uint8_t mdir = MdirOf(mdir_byte);
      uint8_t moct = MoctOf(mdir_byte);
      uint8_t chord_idx = mint - 1;
      uint8_t chord_size = pgm_read_byte(&kChordSizes[chord_idx]);
      uint8_t chord_offset = pgm_read_byte(&kChordOffsets[chord_idx]);
      uint8_t N = chord_size * moct;  // walk has positions 0..N (N+1 total)
      if (N == 0) N = 1;
      int16_t step_count = 0;
      switch (mdir) {
        default:
        case 0: {  // up sawtooth
          step_count = sub_idx % (N + 1);
          break;
        }
        case 1: {  // dn sawtooth
          step_count = -static_cast<int16_t>(sub_idx % (N + 1));
          break;
        }
        case 2: {  // ud bipolar triangle, period 4N
          uint8_t period = N << 2;
          uint8_t phase = sub_idx % period;
          if (phase <= N) step_count = phase;
          else if (phase <= 3 * N) step_count = static_cast<int16_t>(2 * N) - phase;
          else step_count = static_cast<int16_t>(phase) - static_cast<int16_t>(period);
          break;
        }
        case 3: {  // ud+ unipolar above, period 2N
          uint8_t period = N << 1;
          uint8_t phase = sub_idx % period;
          step_count = (phase <= N) ? phase : static_cast<int16_t>(2 * N) - phase;
          break;
        }
        case 4: {  // ud- unipolar below, period 2N
          uint8_t period = N << 1;
          uint8_t phase = sub_idx % period;
          int16_t s = (phase <= N) ? phase : static_cast<int16_t>(2 * N) - phase;
          step_count = -s;
          break;
        }
        case 5: {  // rnd bipolar
          step_count = static_cast<int16_t>(Random::GetByte() % (2 * N + 1)) - N;
          break;
        }
        case 6: {  // rnd+ unipolar above
          step_count = static_cast<int16_t>(Random::GetByte() % (N + 1));
          break;
        }
        case 7: {  // rnd- unipolar below
          step_count = -static_cast<int16_t>(Random::GetByte() % (N + 1));
          break;
        }
      }
      // Chord-aware delta: |step_count| picks a chord-tone position
      // (apos % size = which chord tone, apos / size = which octave above root).
      uint8_t apos = step_count < 0
          ? static_cast<uint8_t>(-step_count)
          : static_cast<uint8_t>(step_count);
      uint8_t interval = pgm_read_byte(
          &kChordIntervals[chord_offset + (apos % chord_size)]);
      int16_t delta = static_cast<int16_t>(interval)
          + static_cast<int16_t>(apos / chord_size) * 12;
      if (step_count < 0) delta = -delta;
      int16_t new_note = static_cast<int16_t>(note) + delta;
      if (new_note < 0) new_note = 0;
      if (new_note > 127) new_note = 127;
      note = static_cast<uint8_t>(new_note);
      note = QuantizeToScale(note, tr.pattern[kPatSCAL] & 7, tr.pattern[kPatROOT]);
    }
  }

  // Velocity: lock-or-default, then scale by track VOL (255 = identity).
  uint8_t velocity = ResolveStepByte(tr, step_index, kSPVEL);
  velocity = (static_cast<uint16_t>(velocity) * tr.pattern[kPatVOL]) >> 8;

  // GLID: per-step portamento time. Pushed to the voicecard part struct
  // (offset 6 = portamento) before the trigger so the slide uses this
  // step's value. Note: this leaves the voicecard's portamento at the
  // last step's glid for any interleaved MIDI/keyboard notes — acceptable
  // since step playback is the dominant path here.
  uint8_t glid = ResolveStepByte(tr, step_index, kSPGLID);
  voicecard_tx.WriteData(t, VOICECARD_DATA_PART, 6, glid);

  voicecard_tx.TriggerWithSnapshot(
      t, static_cast<uint16_t>(note) << 7, velocity, 0, snapshot);
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
    tracks_[t].shadow[kShdwREPT] = 0;
    tracks_[t].shadow[kShdwSSUB] = 0;
    tracks_[t].shadow[kShdwDIR]  = 0;
    tracks_[t].shadow[kShdwLAST] = 0;
    tracks_[t].shadow[kShdwPROB] = 0;
    // Pre-charge TICK so the first Clock() call after Play() crosses period
    // and fires step 0 immediately. Without this, tracks with CDIV>1 wait
    // their full period before firing and start off-grid relative to CDIV=1.
    uint8_t cdiv_idx = tracks_[t].pattern[kPatCDIV];
    if (cdiv_idx >= 15) cdiv_idx = 14;
    uint8_t period = pgm_read_byte(kRateValues + cdiv_idx);
    tracks_[t].shadow[kShdwTICK] = period;
  }
  global_.master_tick = 0;
}

void Sequencer::Stop() {
  Reset();
  global_.transport = kSeqStopped;
}

void Sequencer::Panic() {
  for (uint8_t t = 0; t < kNumVoices; ++t) {
    voicecard_tx.Kill(t);
  }
  global_.transport = kSeqStopped;
}

}  // namespace ambika
