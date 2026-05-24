// Copyright 2011 Emilie Gillet.
//
// Phase 3: Sequencer core implementation.

#include "controller/sequencer.h"

#include "avrlib/random.h"
#include "avrlib/string.h"
#include "controller/midi_dispatcher.h"
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
// Bit-7 escape: if a RATE byte has bit 7 set, byte & 0x7F is a raw tick
// period clamped to [2, 96], bypassing the preset table. Saved patches load
// bit-identically since existing preset bytes have bit 7 clear.
// Non-static: shared with seq_track_page.cc and seq_steps_page.cc via extern
// for the raw↔preset snap scan on encoder-click mode toggle.
extern const prog_uint8_t kRateValues[] PROGMEM = {
    3, 4, 6, 8, 9, 12, 16, 18, 24, 32, 36, 48, 96, 144, 192
};

// Resolves a 0-based preset byte (or a 0x80|period raw byte) to a tick period.
// Callers with 1-based per-step bytes must subtract 1 before passing.
static inline uint8_t RatePeriod(uint8_t byte) {
  if (byte & 0x80) {
    uint8_t p = byte & 0x7F;
    if (p < 2) p = 2;
    else if (p > 96) p = 96;
    return p;
  }
  if (byte >= 15) byte = 14;
  return pgm_read_byte(kRateValues + byte);
}

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
//  13 chr  {0..11}          — chromatic walk, all 12 semitones
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
  /* chr  */  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
};
static const prog_uint8_t kChordOffsets[] PROGMEM = {
  0, 1, 3, 6, 9, 12, 15, 18, 22, 26, 30, 34, 39
};
static const prog_uint8_t kChordSizes[] PROGMEM = {
  1, 2, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 12
};

// PROB cycle-phase slot table (v4.3, #6). Each byte:
//   bit 7    : neg ('!' variant)
//   bits 6..4: (X - 1)        — X is the offset within the cycle (1..8)
//   bits 3..1: (N - 1)        — N is the cycle length (1..8)
//   bit 0    : FILL marker — when set, the slot ignores X/N and gates on
//              ProbFillActive() instead.
//
// Slots 1..35 = positive iterative (X|N, fires when (loop % N) == X-1).
// Slots 36..53 = negative iterative (!X|N, fires when (loop % N) != X-1).
// Slots 54..55 = FILL / !FILL — gated by ProbFillActive() (stub returns 0
// until PERF page lands, so FILL never fires and !FILL always fires).
// Out-of-range slot or byte 0x80 → always fire (center dead zone).
//
// Packing (X-1) and (N-1) lets X and N each cover 1..8 in 3 bits without
// colliding with the neg bit — earlier (X << 4) encoding lost the 8:N row.
static const prog_uint8_t kProbCyclePhase[55] PROGMEM = {
  0x02, 0x12,                                            // 1:2, 2:2
  0x04, 0x14, 0x24,                                      // 1:3..3:3
  0x06, 0x16, 0x26, 0x36,                                // 1:4..4:4
  0x08, 0x18, 0x28, 0x38, 0x48,                          // 1:5..5:5
  0x0A, 0x1A, 0x2A, 0x3A, 0x4A, 0x5A,                    // 1:6..6:6
  0x0C, 0x1C, 0x2C, 0x3C, 0x4C, 0x5C, 0x6C,              // 1:7..7:7
  0x0E, 0x1E, 0x2E, 0x3E, 0x4E, 0x5E, 0x6E, 0x7E,        // 1:8..8:8
  0x84, 0x94, 0xA4,                                      // !1:3..!3:3
  0x86, 0x96, 0xA6, 0xB6,                                // !1:4..!4:4
  0x88, 0x98, 0xA8, 0xB8, 0xC8,                          // !1:5..!5:5
  0x8A, 0x9A, 0xAA, 0xBA, 0xCA, 0xDA,                    // !1:6..!6:6
  0x01, 0x81                                             // FILL, !FILL
};

// Global FILL state. Stubbed at 0 until PERF page provides the toggling
// gesture; FILL slots evaluate to "not firing" and !FILL slots to "always
// firing" in the meantime.
static uint8_t g_fill_active = 0;

uint8_t ProbRoll(uint8_t prob_byte, uint8_t loop_count) {
  if (!(prob_byte & 0x80)) {
    return ((Random::GetByte() & 0x7F) <= prob_byte) ? 1 : 0;
  }
  uint8_t slot = prob_byte & 0x7F;
  if (slot == 0 || slot > kProbCyclePhaseCount) return 1;  // always
  uint8_t entry = pgm_read_byte(&kProbCyclePhase[slot - 1]);
  uint8_t neg = entry & 0x80;
  uint8_t fires;
  if (entry & 0x01) {
    fires = g_fill_active;
  } else {
    uint8_t X = ((entry >> 4) & 0x07) + 1;  // 1..8
    uint8_t N = ((entry >> 1) & 0x07) + 1;  // 1..8
    fires = ((loop_count % N) == (X - 1)) ? 1 : 0;
  }
  return neg ? (fires ? 0 : 1) : fires;
}

uint8_t ProbCyclePhaseEntry(uint8_t slot) {
  if (slot == 0 || slot > kProbCyclePhaseCount) return 0;
  return pgm_read_byte(&kProbCyclePhase[slot - 1]);
}

void WriteProbByte(char* buf, uint8_t v) {
  buf[0] = ' '; buf[1] = ' '; buf[2] = ' '; buf[3] = ' ';
  if (!(v & 0x80)) {
    uint16_t pct = (static_cast<uint16_t>(v) * 100) / 127;
    if (pct > 100) pct = 100;
    if (pct >= 100) { buf[0] = '1'; buf[1] = '0'; buf[2] = '0'; }
    else if (pct >= 10) { buf[1] = '0' + (pct / 10); buf[2] = '0' + (pct % 10); }
    else { buf[2] = '0' + pct; }
    buf[3] = '%';
    return;
  }
  uint8_t entry = ProbCyclePhaseEntry(v & 0x7F);
  if (entry == 0) {
    buf[0] = ' '; buf[1] = '1'; buf[2] = '0'; buf[3] = '0';
    return;
  }
  uint8_t neg = entry & 0x80;
  if (entry & 0x01) {
    if (neg) { buf[0] = '!'; buf[1] = 'F'; buf[2] = 'I'; buf[3] = 'L'; }
    else     { buf[0] = 'F'; buf[1] = 'I'; buf[2] = 'L'; buf[3] = 'L'; }
    return;
  }
  uint8_t X = ((entry >> 4) & 0x07) + 1;
  uint8_t N = ((entry >> 1) & 0x07) + 1;
  uint8_t col = 0;
  if (neg) buf[col++] = '!';
  buf[col++] = '0' + X;
  buf[col++] = ':';
  buf[col++] = '0' + N;
}

uint8_t ProbEncodePot(uint8_t pot) {
  // Pot 0..62 → % bytes 0..124 (linear, top ≈ 98%).
  // Pot 63..64 → 0x80 (center dead zone, always fire).
  // Pot 65..119 → cycle-phase slots 1..55 (bytes 0x81..0xB7).
  // Pot 120..127 → clamp to last slot (0xB7 = !FILL).
  if (pot <= 62) return pot << 1;
  if (pot <= 64) return kProbAlways;
  uint8_t slot = pot - 64;          // 1..63
  if (slot > kProbCyclePhaseCount) slot = kProbCyclePhaseCount;
  return 0x80 | slot;
}

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
  WAVEFORM_FM,  // WAVE1 = FM
  0,    // PARA1
  0,    // BLND = 0 (Osc 1 only, no FM)
  0,    // RTIO = crossmod/FM amount (reserved for future linear-FM)
  0,    // WAVE2 = none
  0,    // PARA2
  0,    // RANG = OSC1 range (v4.4-WS1; slot 7 reclaimed from dead OSC1 detune)
};

// v4.4-WS1: defaults for the 20 newly lockable slots (li 28..47). Values
// mirror what kDefaultConfig was already shipping for these patch bytes so
// boot-time behavior is unchanged; subsequent track-default edits write
// here instead of the now-vestigial config[] bytes.
static const prog_uint8_t kDefaultExt[] PROGMEM = {
  0,    // 28 xmod    = mix_op = no FM/cross
  0,    // 29 fuzz    = off
  0,    // 30 crsh    = off (BITS)
  0,    // 31 reso    = no resonance
  0,    // 32 mode    = LP
  0,    // 33 E1 rise = fast attack
  64,   // 34 E1 curv = centered
  127,  // 35 E1 dept = full ENV1→VCA
  0,    // 36 E2 rise
  64,   // 37 E2 curv
  0,    // 38 E3 rise
  64,   // 39 E3 curv
  128,  // 40 LFO4 rate
  0,    // 41 LFO4 wave (triangle)
  0,    // 42 LFO4 dest
  0,    // 43 LFO4 dept
  128,  // 44 LFO5 rate
  0,    // 45 LFO5 wave (triangle)
  0,    // 46 LFO5 dest
  0,    // 47 LFO5 dept
};

static const prog_uint8_t kDefaultPage2[] PROGMEM = {
  40,   // E1DEC = env1 decay
  0,    // TUN2  = OSC2 coarse pitch (int8, 0 = no offset)
  40,   // E2DEC
  0,    // FIN2  = OSC2 detune (int8, 0 = no detune)
  40,   // E3DEC
  0,    // page2[5] = reserved (was E3REL)
  0,    // NOIS  = no noise
  0,    // SUB   = no sub-osc
};

static const prog_uint8_t kDefaultPage3[] PROGMEM = {
  127,  // FREQ = cutoff fully open / filter inactive (matches kDefaultConfig[kCfgFREQ])
  0,    // FAMT = ENV2→VCF depth off (matches kDefaultConfig[kCfgE2DEPT])
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
  127,  // FREQ = cutoff fully open / filter inactive (mirrors kDefaultPage3 FREQ)
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
  0,    // L5SH (was OSC2R) — LFO5 shape = TRIANGLE
  128,  // L5FR (was OSC2D) — LFO5 rate = medium free-run (mirrors LFO4)
  0,    // FMOP = no FM
  0,    // FUZZ = no fuzz
  127,  // E1DEPT = ENV1→VCA full depth (round 5: 0..127 unipolar)
  0,    // E2DEPT = ENV2→VCF depth off (mirrors kDefaultPage3 FAMT)
  0,    // L5RT (was E3DEPT) — LFO5 retrigger off
  0,    // WSUB = WAVEFORM_SUB_OSC_SQUARE_1
  0,    // L5D = LFO5 destination (mod slot 6 dest)
  0,    // L5A = LFO5 amount (mod slot 6 amount)
};

uint8_t Sequencer::StepLockedValue(
    uint8_t t, uint8_t s, uint8_t lock_index) const {
  return lock_pool_.Get(t, s, lock_index, tracks_[t].defaults[lock_index]);
}

uint8_t Sequencer::StepIsLocked(
    uint8_t t, uint8_t s, uint8_t lock_index) const {
  return (lock_pool_.Find(t, s, lock_index) != 0xff) ? 1 : 0;
}

uint8_t Sequencer::SetStepLock(
    uint8_t t, uint8_t s, uint8_t lock_index, uint8_t value) {
  return lock_pool_.Set(t, s, lock_index, value);
}

void Sequencer::ClearStepLock(
    uint8_t t, uint8_t s, uint8_t lock_index) {
  lock_pool_.Clear(t, s, lock_index);
  lock_prob_pool_.Clear(t, s, lock_index);
}

// Parameter-table id → sequencer lock_index 0..47 (0xff = not lockable).
// v4.4-WS1: expanded to 77 entries to cover every patch-page cell that has
// a per-step DSP byte. Slot 7 reclaims OSC1 RANGE; slots 28..47 host the
// formerly config-only params (xmod/fuzz/crsh/reso/mode, full env shape,
// LFO4/5 controls). Mapping fixes for E2/E3 fall + depth at params 64/66/68/70
// route patch-page cells to existing slots 10/25/12/26 (FAMT/PAMT/fdec/pdec).
static const prog_uint8_t kParamLockMap[] PROGMEM = {
  /* 0  OSC1_SHAPE   */ 1,
  /* 1  OSC1_PWM     */ 2,
  /* 2  OSC1_RANGE   */ 7,        // WS1: reclaims dead OSC1_DETUNE slot
  /* 3  OSC1_DETUNE  */ 0xff,     // WS0 soft-dropped; no UI cell
  /* 4  OSC2_SHAPE   */ 5,
  /* 5  OSC2_PWM     */ 6,
  /* 6  OSC2_RANGE   */ 9,
  /* 7  OSC2_DETUNE  */ 11,
  /* 8  MIX_BALANCE  */ 3,
  /* 9  MIX_OPERATOR */ 28,       // WS1: xmod
  /* 10 MIX_PARAMETER*/ 4,
  /* 11 MIX_SUB_SHAPE*/ 27,
  /* 12 MIX_SUB_LEVEL*/ 15,
  /* 13 MIX_NOISE_LV */ 14,
  /* 14 MIX_FUZZ     */ 29,       // WS1
  /* 15 MIX_CRUSH    */ 30,       // WS1
  /* 16 FILTER1_CUT  */ 24,
  /* 17 FILTER1_RES  */ 31,       // WS1
  /* 18 FILTER1_MODE */ 32,       // WS1
  /* 19 FILTER2_CUT  */ 0xff,
  /* 20 FILTER2_RES  */ 0xff,
  /* 21 FILTER2_MODE */ 0xff,
  /* 22 FILTER1_ENV  */ 25,       // orphaned param (no cell); shares slot 25 with param 66
  /* 23 FILTER1_LFO  */ 0xff,
  /* 24 E1 rise      */ 33,       // WS1
  /* 25 E1 fall      */ 8,
  /* 26 E1 curv      */ 34,       // WS1
  /* 27 E1 depth     */ 35,       // WS1
  /* 28 E2 rise      */ 36,       // WS1
  /* 29 LFO_SYNC     */ 0xff,
  /* 30 LFO_RATE     */ 0xff,
  /* 31 LFO_SHAPE    */ 0xff,
  /* 32 VOICE_LFO_RATE  */ 40,    // WS1: LFO4 rate
  /* 33 VOICE_LFO_SHAPE */ 41,    // WS1: LFO4 wave
  /* 34 UI_ACTIVE_MOD */ 0xff,
  /* 35 MOD_SOURCE   */ 0xff,
  /* 36 MOD_DEST     */ 0xff,
  /* 37 MOD_AMOUNT   */ 0xff,
  /* 38 UI_ACTIVE_MODIFIER */ 0xff,
  /* 39 MOD_OPERAND1 */ 0xff,
  /* 40 MOD_OPERAND2 */ 0xff,
  /* 41 MOD_OPERATOR */ 0xff,
  /* 42..63 — PART / MULTI / SYSTEM / filter velo/kbt — not lockable */
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,    /* 42..48 PART */
  0xff, 0xff, 0xff,                            /* 49..51 PART seq lengths */
  0xff, 0xff, 0xff,                            /* 52..54 MULTI */
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,    /* 55..61 SYSTEM */
  0xff, 0xff,                                  /* 62..63 filter velo/kbt */
  /* 64 E2 fall      */ 10,       // mapping fix: patch-page cell → existing slot
  /* 65 E2 curv      */ 37,       // WS1
  /* 66 E2 depth     */ 25,       // mapping fix: shares slot with FILTER1_ENV/22
  /* 67 E3 rise      */ 38,       // WS1
  /* 68 E3 fall      */ 12,       // mapping fix
  /* 69 E3 curv      */ 39,       // WS1
  /* 70 E3 depth     */ 26,       // mapping fix
  /* 71 LFO4 dest    */ 42,       // WS1
  /* 72 LFO4 dept    */ 43,       // WS1
  /* 73 LFO5 rate    */ 44,       // WS1
  /* 74 LFO5 wave    */ 45,       // WS1
  /* 75 LFO5 dest    */ 46,       // WS1
  /* 76 LFO5 dept    */ 47,       // WS1
};
static const uint8_t kParamLockMapSize =
    sizeof(kParamLockMap) / sizeof(kParamLockMap[0]);

uint8_t ParamIdToLockIndex(uint8_t param_id, uint8_t /*instance*/) {
  // FOLD lives at parameters[] array index 77 (last entry), past the
  // contiguous map. Reclaim the page2 reserved slot (lock_index 13).
  if (param_id == 77) return 13;
  if (param_id >= kParamLockMapSize) return 0xff;
  return pgm_read_byte(&kParamLockMap[param_id]);
}

void SeedExtendedDefaults(SeqTrack& tr) {
  memcpy_P(&tr.defaults[28], kDefaultExt, 20);
}

void Sequencer::ClearTrackLocksForParam(uint8_t t, uint8_t lock_index) {
  for (uint8_t s = 0; s < kNumStepsPerTrack; ++s) {
    lock_pool_.Clear(t, s, lock_index);
    lock_prob_pool_.Clear(t, s, lock_index);
  }
}

void Sequencer::ClearAllStepLocks(uint8_t t, uint8_t s) {
  lock_pool_.ClearStep(t, s);
  lock_prob_pool_.ClearStep(t, s);
  // Also reset per-step editable surfaces that don't live in the pool:
  // SMOD (step_flags bits 2..5) and the substep_bits bitfield. Preserve
  // bit 0 (step on/off) — the user's first tap of the double-tap already
  // toggled it; the second tap is the clear gesture, not a force-off.
  SeqStep& step = tracks_[t].steps[s];
  step.step_flags &= kStepFlagOn;
  step.substep_bits = 0;
}


static const prog_uint8_t kDefaultPattern[] PROGMEM = {
  kDirnFwd,  // DIRN = forward
  2,          // RATE index 2 → " 16" = 16th note per step
  0,          // ROTA = no rotation
  8,          // LENG = 8 steps
  0,          // SCAL = chromatic
  0,          // ROOT = C
  255,        // VOL = full velocity scale
};

void Sequencer::Init() {
  lock_pool_.Init();
  lock_prob_pool_.Init();
  for (uint8_t t = 0; t < kNumVoices; ++t) {
    SeqTrack& tr = tracks_[t];
    memcpy_P(tr.pattern,       kDefaultPattern,  7);
    memcpy_P(&tr.defaults[0],  kDefaultPage1,    8);
    memcpy_P(&tr.defaults[8],  kDefaultPage2,    8);
    memcpy_P(&tr.defaults[16], kDefaultStepPage, 8);
    memcpy_P(&tr.defaults[24], kDefaultPage3,    4);
    SeedExtendedDefaults(tr);
    memcpy_P(tr.config,        kDefaultConfig,   kCfgSIZE);
    memset(tr.shadow, 0, kShdwSIZE);

    for (uint8_t s = 0; s < kNumStepsPerTrack; ++s) {
      SeqStep& step = tr.steps[s];
      step.step_flags   = 0;
      step.substep_bits = 0;
    }
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
      // Fall through: the per-track loop runs on this same tick so step 0
      // fires inline at the cycle boundary. With TICK pre-charged to
      // period-1, the +=1 below lands TICK == period and fires step 0 with
      // a full-period gate window.
    }
  }

  for (uint8_t t = 0; t < kNumVoices; ++t) {
    SeqTrack& tr = tracks_[t];

    // RATE: per-step rate override for the currently-playing step.
    // 0 = inherit track; 1..15 = preset (rate-1 into kRateValues); bit-7 set
    // = raw tick period.
    uint8_t rate = StepLockedValue(t, tr.shadow[kShdwLAST], 16 + kSPRATE);
    uint8_t cdiv_byte;
    if (rate == 0) {
      cdiv_byte = tr.pattern[kPatCDIV];
    } else if (rate & 0x80) {
      cdiv_byte = rate;
    } else {
      cdiv_byte = rate - 1;
    }
    uint8_t period = RatePeriod(cdiv_byte);

    tr.shadow[kShdwTICK] += ticks;

    // SSUB ratchet: fire sub-triggers between period boundaries.
    // Only active when a step has been fired (kShdwLAST is valid post-reset).
    // Gate on kShdwPROB so substeps follow the main-step probability decision.
    uint8_t cur = tr.shadow[kShdwLAST];
    int8_t ssub = SsubOf(StepLockedValue(t, cur, 17));
    if (tr.shadow[kShdwTICK] < period) {
      if (ssub > 0 && tr.shadow[kShdwPROB] && tr.shadow[kShdwSubs]) {
        // Ratchets: N+1 evenly-spaced fires per period. Slot 0 = main fire.
        uint8_t sub_period = period / (static_cast<uint8_t>(ssub) + 1);
        if (sub_period == 0) sub_period = 1;
        uint8_t slot_now  = tr.shadow[kShdwTICK] / sub_period;
        uint8_t slot_prev = (tr.shadow[kShdwTICK] - ticks) / sub_period;
        if (slot_now != slot_prev && slot_now > 0) {
          voicecard_tx.Release(t);
          midi_dispatcher.SequencerNoteOff(t);
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
        uint8_t rept_total = ReptOf(StepLockedValue(t, last, 18));
        tr.shadow[kShdwREPT]--;
        uint8_t repeat_idx = rept_total - tr.shadow[kShdwREPT];
        voicecard_tx.Release(t);
        midi_dispatcher.SequencerNoteOff(t);
        if (tr.shadow[kShdwPROB] && (tr.steps[last].step_flags & kStepFlagOn)) {
          int8_t ssub_l = SsubOf(StepLockedValue(t, last, 17));
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
        tr.shadow[kShdwPROB] =
            ProbRoll(StepLockedValue(t, fired, 16), tr.shadow[kShdwLOOP]);

        // SUBS PROB (v4.3, #38): a second gate attached to the SUBS cell.
        // On roll fail the main step still fires (subject to kShdwPROB) but
        // ratchets/repeats/chord-walk are suppressed for this loop.
        {
          uint8_t pi = lock_prob_pool_.Find(t, fired, kProbKeySubs);
          tr.shadow[kShdwSubs] =
              (pi == 0xff) ||
              ProbRoll(lock_prob_pool_.entry(pi).prob, tr.shadow[kShdwLOOP]);
        }

        if (tr.shadow[kShdwPROB]) {
          // SMOD dispatch. skip = bounded re-advance loop; fwd/rev/dir
          // mutate kPatDIRN (sticky); rjmp/jmp[N] reseat the playhead
          // before firing. Only applied when PROB passes.
          uint8_t fire_now = 1;
          uint8_t jumped = 0;
          uint8_t pre_jump_fired = 0;
          uint8_t guard;
          for (guard = 0; guard < len; ++guard) {
            uint8_t smod = StepSmod(tr.steps[fired]);
            // Per-step SMOD PROB gate (v4.3, #38). If a prob entry exists
            // for this step under key kProbKeySmod, roll it; on fail, treat
            // the step as if its SMOD were None — fire normally, no jump,
            // no skip, no direction mutation.
            if (smod != kSmodNone) {
              uint8_t pi = lock_prob_pool_.Find(t, fired, kProbKeySmod);
              if (pi != 0xff &&
                  !ProbRoll(lock_prob_pool_.entry(pi).prob, tr.shadow[kShdwLOOP])) {
                smod = kSmodNone;
              }
            }
            if (smod == kSmodSkip || smod == kSmodESkp) {
              // ESkp (explicit skip) steps are skipped during normal
              // iteration; they only fire when another step's jump SMOD
              // reseats fired onto them.
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
              pre_jump_fired = fired;
              tr.shadow[kShdwSTEP] = target;
              fired = (target + tr.pattern[kPatROTA]) % len;
              jumped = 1;
            } else if (smod >= kSmodJmp1 && smod <= kSmodJmp8) {
              uint8_t target = smod - kSmodJmp1;
              if (target >= len) target = len - 1;
              pre_jump_fired = fired;
              tr.shadow[kShdwSTEP] = target;
              fired = (target + tr.pattern[kPatROTA]) % len;
              jumped = 1;
            }
            break;
          }
          if (guard >= len) fire_now = 0;  // every step is skip — silent

          voicecard_tx.Release(t);
          midi_dispatcher.SequencerNoteOff(t);
          tr.shadow[kShdwLAST] = fired;

          if (fire_now && (tr.steps[fired].step_flags & kStepFlagOn)) {
            int8_t ssub_f = SsubOf(StepLockedValue(t, fired, 17));
            // Bit 0 of substep_bits gates the main fire whenever the step is
            // in a substep mode — ratchets (kStepFlagGated) or repeats
            // (SSUB=-2). Only applies when SUBS PROB passes; if SUBS is
            // gated out for this loop, the step fires as if SSUB=0.
            uint8_t gated = tr.shadow[kShdwSubs] &&
                            ((ssub_f == -2) ||
                             (tr.steps[fired].step_flags & kStepFlagGated));
            if (!gated || (tr.steps[fired].substep_bits & 0x01)) {
              FireStep(t, fired, 0);
            }
          }
          // Bump kShdwLOOP only when the jump prevents natural wrap from
          // firing this pass — i.e. backward jumps in Fwd, forward jumps in
          // Rev, direction-aware in Pend. Forward jumps in Fwd (and the
          // mirror in Rev) still reach the end of the pattern and trigger
          // the wrap bump in AdvanceStep — bumping here too would
          // double-count, causing iterative gates (e.g. 4:4 on a jmp step
          // with REPT on the target) to drift after the first jump cycle.
          // Rnd: AdvanceStep never sets looped, so the manual bump is the
          // only source of LOOP advancement — keep it unconditional there.
          if (jumped) {
            uint8_t dir = tr.pattern[kPatDIRN];
            uint8_t bump = 1;
            if (dir == kDirnFwd) {
              bump = (fired < pre_jump_fired) ? 1 : 0;
            } else if (dir == kDirnRev) {
              bump = (fired > pre_jump_fired) ? 1 : 0;
            } else if (dir == kDirnPend) {
              bump = (tr.shadow[kShdwDIR] == 0)
                  ? ((fired < pre_jump_fired) ? 1 : 0)
                  : ((fired > pre_jump_fired) ? 1 : 0);
            }
            if (bump) ++tr.shadow[kShdwLOOP];
          }
          if (fire_now) {
            // SUBS PROB suppresses repeats this loop — set REPT to 0 so the
            // step doesn't re-fire even though the intrinsic REPT field is
            // non-zero.
            uint8_t rept = tr.shadow[kShdwSubs]
                ? ReptOf(StepLockedValue(t, fired, 18)) : 0;
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
          midi_dispatcher.SequencerNoteOff(t);
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
  uint8_t looped = 0;

  switch (tr.pattern[kPatDIRN]) {
    default:
    case kDirnFwd:
      if (step + 1 >= len) { step = 0; looped = 1; }
      else                 { step = step + 1; }
      break;
    case kDirnRev:
      if (step == 0) { step = len - 1; looped = 1; }
      else           { step = step - 1; }
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
          looped = 1;  // full out-and-back cycle complete
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
  if (looped) ++tr.shadow[kShdwLOOP];
}

void Sequencer::FireStep(uint8_t t, uint8_t step_index, uint8_t sub_idx) {
  // Performance mixer gate: skip triggers for muted/non-solo'd voices.
  if (SeqMixerPage::skip_mask() & (1 << t)) return;

  SeqTrack& tr = tracks_[t];

  // PROB rolled once per main step in Clock(); ratchets/repeats are gated
  // there on tr.shadow[kShdwPROB], so by the time FireStep runs the decision
  // has already been made and we always fire.

  // Resolve 40-byte snapshot (v4.4-WS1): page1[8] || page2[8] || page3[4] ||
  // ext[20]. Seed with track defaults, then iterate the lock pool ONCE and
  // overlay any pool entries matching (t, step_index). NOTE (li 0) flows
  // through the same path as every other lock. Intrinsic step-page slots
  // (16..23 PROB/SSUB/REPT/RATE/VEL/GLID/MINT/MDIR) live outside the
  // snapshot; their fire-time effects are dispatched separately below.
  uint8_t snapshot[40];
  for (uint8_t i = 0; i < 16; ++i) snapshot[i]       = tr.defaults[i];
  for (uint8_t i = 0; i <  4; ++i) snapshot[16 + i]  = tr.defaults[24 + i];
  for (uint8_t i = 0; i < 20; ++i) snapshot[20 + i]  = tr.defaults[28 + i];
  {
    uint8_t ts = LockTsPack(t, step_index);
    uint8_t loop = tr.shadow[kShdwLOOP];
    for (uint8_t i = 0; i < kLockPoolCapacity; ++i) {
      const LockEntry& e = lock_pool_.entry(i);
      if (e.param == kLockPoolFree || e.ts != ts) continue;
      uint8_t li = e.param;
      // Per-lock PROB gate (v4.3, #38 slim). If a prob entry exists for
      // this (t, step, param), roll it; on fail, skip the overlay so the
      // snapshot retains the track default for that param this loop.
      uint8_t pi = lock_prob_pool_.Find(t, step_index, li);
      if (pi != 0xff) {
        if (!ProbRoll(lock_prob_pool_.entry(pi).prob, loop)) continue;
      }
      if (li < 16) {
        snapshot[li] = e.value;
      } else if (li >= 24 && li < 28) {
        snapshot[16 + (li - 24)] = e.value;
      } else if (li >= 28 && li < 48) {
        snapshot[20 + (li - 28)] = e.value;  // [20..39] covers li 28..47
      }
    }
  }

  // Note: intrinsic from step, then quantize by track scale + root.
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
  //
  // SSUB=0 case: no substeps to drive the walk, so use the per-track loop
  // counter instead — each pattern wrap advances the chord-tone position.
  uint8_t walk_idx = sub_idx;
  if (walk_idx == 0 && SsubOf(StepLockedValue(t, step_index, 17)) == 0 &&
      tr.shadow[kShdwSubs]) {
    walk_idx = tr.shadow[kShdwLOOP];
  }
  if (walk_idx > 0) {
    uint8_t mint = StepLockedValue(t, step_index, 16 + kSPMINT);
    if (mint > 0 && mint <= 13) {
      uint8_t mdir_byte = StepLockedValue(t, step_index, 16 + kSPMDIR);
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
          step_count = walk_idx % (N + 1);
          break;
        }
        case 1: {  // dn sawtooth
          step_count = -static_cast<int16_t>(walk_idx % (N + 1));
          break;
        }
        case 2: {  // ud bipolar triangle, period 4N
          uint8_t period = N << 2;
          uint8_t phase = walk_idx % period;
          if (phase <= N) step_count = phase;
          else if (phase <= 3 * N) step_count = static_cast<int16_t>(2 * N) - phase;
          else step_count = static_cast<int16_t>(phase) - static_cast<int16_t>(period);
          break;
        }
        case 3: {  // ud+ unipolar above, period 2N
          uint8_t period = N << 1;
          uint8_t phase = walk_idx % period;
          step_count = (phase <= N) ? phase : static_cast<int16_t>(2 * N) - phase;
          break;
        }
        case 4: {  // ud- unipolar below, period 2N
          uint8_t period = N << 1;
          uint8_t phase = walk_idx % period;
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
  // (v * (VOL+1)) >> 8 so VOL=255 produces true identity; VOL=0 still silent.
  // Per-lock PROB gate (v4.3, #38): on roll fail, revert to track default
  // so per-step VEL only applies probabilistically.
  uint8_t velocity = StepLockedValue(t, step_index, 20);
  {
    uint8_t pi = lock_prob_pool_.Find(t, step_index, 20);
    if (pi != 0xff &&
        !ProbRoll(lock_prob_pool_.entry(pi).prob, tr.shadow[kShdwLOOP])) {
      velocity = tr.defaults[16 + kSPVEL];
    }
  }
  velocity = (static_cast<uint16_t>(velocity) *
              (tr.pattern[kPatVOL] + 1)) >> 8;

  // GLID: per-step portamento time. Pushed to the voicecard part struct
  // (offset 6 = portamento) before the trigger so the slide uses this
  // step's value. Note: this leaves the voicecard's portamento at the
  // last step's glid for any interleaved MIDI/keyboard notes — acceptable
  // since step playback is the dominant path here.
  // Per-lock PROB gate (v4.3, #38): on roll fail, revert to track default.
  uint8_t glid = StepLockedValue(t, step_index, 21);
  {
    uint8_t pi = lock_prob_pool_.Find(t, step_index, 21);
    if (pi != 0xff &&
        !ProbRoll(lock_prob_pool_.entry(pi).prob, tr.shadow[kShdwLOOP])) {
      glid = tr.defaults[16 + kSPGLID];
    }
  }
  uint8_t track_ext = multi.track_is_ext(t);
  uint8_t channel = (multi.track_channel(t) - 1) & 0x0f;

  if (!track_ext) {
    voicecard_tx.WriteData(t, VOICECARD_DATA_PART, 6, glid);
    voicecard_tx.TriggerWithSnapshot(
        t, static_cast<uint16_t>(note) << 7, velocity, 0, snapshot);
  }
  midi_dispatcher.SequencerNoteOn(t, channel, note, velocity);

  if (track_ext) {
    // Emit resolved (lock-or-default) values every step so external gear
    // sees the same "snap back to default on unlocked step" behavior the
    // internal synth gets from a full snapshot push. No dedup state.
    //
    // VAMT (lockable 4) → CC 1 (Mod Wheel). snapshot[4] already resolved.
    midi_dispatcher.SendVamtCc(channel, snapshot[4] >> 1);
    // GLID (lockable 21) → CC 5 (Portamento Time). Intrinsic, read above.
    midi_dispatcher.SendGlidCc(channel, glid);
    // EXT slots — 4 on S5b, 4 on S5c. Lockable indices match cells 0..3 of
    // each page. snapshot already holds lock-or-default for these.
    static const prog_uint8_t kExtSlotLockable[8] PROGMEM = {
      14, 1, 2, 9,    // S5b slots 0..3
      24, 10, 25, 8,  // S5c slots 0..3
    };
    for (uint8_t slot = 0; slot < 8; ++slot) {
      uint8_t lockable = pgm_read_byte(&kExtSlotLockable[slot]);
      uint8_t snap_idx = (lockable < 16) ? lockable : (lockable - 8);
      midi_dispatcher.SendSlotCc(channel,
                                 multi.cc_for_slot(t, slot),
                                 snapshot[snap_idx] >> 1);
    }
  }
}

void Sequencer::Play() {
  if (global_.transport == kSeqStopped) {
    Reset();
  }
  global_.transport = kSeqPlaying;
  multi.Start();
}

void Sequencer::Pause() {
  if (global_.transport == kSeqPlaying) {
    global_.transport = kSeqPaused;
    for (uint8_t t = 0; t < kNumVoices; ++t) {
      voicecard_tx.Release(t);
      midi_dispatcher.SequencerNoteOff(t);
    }
    multi.Stop();
  } else if (global_.transport == kSeqPaused) {
    global_.transport = kSeqPlaying;
    multi.Start();
  }
}

void Sequencer::Reset() {
  for (uint8_t t = 0; t < kNumVoices; ++t) {
    voicecard_tx.Release(t);
    midi_dispatcher.SequencerNoteOff(t);
    tracks_[t].shadow[kShdwSTEP] = 0;
    tracks_[t].shadow[kShdwREPT] = 0;
    tracks_[t].shadow[kShdwSSUB] = 0;
    tracks_[t].shadow[kShdwDIR]  = 0;
    tracks_[t].shadow[kShdwLAST] = 0;
    tracks_[t].shadow[kShdwPROB] = 0;
    tracks_[t].shadow[kShdwLOOP] = 0;
    tracks_[t].shadow[kShdwSubs] = 1;
    // Pre-charge TICK so the first Clock() call after Play()/Reset() lands
    // TICK exactly at period and fires step 0 with a full-period gate window
    // (matching every subsequent step). Pre-charging to period (instead of
    // period-1) would cross the threshold by one tick on first increment and
    // truncate step 0's gate. period >= 2 (raw escape clamps to 2; preset
    // entries are all >= 3).
    uint8_t period = RatePeriod(tracks_[t].pattern[kPatCDIV]);
    tracks_[t].shadow[kShdwTICK] = period - 1;
  }
  global_.master_tick = 0;
}

void Sequencer::Stop() {
  // Second Stop while already stopped: broadcast LFO phase reset to every
  // voice. Lets the user align synced LFOs to song-position-0 deliberately
  // without affecting first-time Stop semantics.
  if (global_.transport == kSeqStopped) {
    for (uint8_t t = 0; t < kNumVoices; ++t) {
      voicecard_tx.ResetLfo(t);
    }
    return;
  }
  Reset();
  global_.transport = kSeqStopped;
  multi.Stop();
}

void Sequencer::Panic() {
  for (uint8_t t = 0; t < kNumVoices; ++t) {
    voicecard_tx.Kill(t);
    midi_dispatcher.SequencerNoteOff(t);
  }
  global_.transport = kSeqStopped;
}

}  // namespace ambika
