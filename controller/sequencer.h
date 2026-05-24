// Copyright 2011 Emilie Gillet.
//
// v4.2 (issue #30): sparse lock-pool storage.
// v4.4 (issue #45): SeqStep loses its intrinsic-byte cache. All 28 lockables
// — including NOTE / PROB / SSUB / REPT / VEL / GLID — live in the global
// LockPool keyed by (track, step, lock_index). SeqStep keeps only the
// step_flags + substep_bits trigger state.

#ifndef CONTROLLER_SEQUENCER_H_
#define CONTROLLER_SEQUENCER_H_

#include "avrlib/base.h"
#include "controller/controller.h"
#include "controller/lock_pool.h"
#include "controller/lock_prob_pool.h"

namespace ambika {

// ---- page1[] indices (Voice Page 1 — Oscillators) ----
static const uint8_t kP1NOTE  = 0;
static const uint8_t kP1WAVE1 = 1;
static const uint8_t kP1PARA1 = 2;
static const uint8_t kP1BLND  = 3;
static const uint8_t kP1RTIO  = 4;
static const uint8_t kP1WAVE2 = 5;
static const uint8_t kP1PARA2 = 6;
static const uint8_t kP1FINE  = 7;

// ---- page2[] indices ----
// Slots 1 / 3 reclaimed from the dead E1REL / E2REL bytes for lockable
// OSC2 coarse / detune. Slot 5 is anonymous reserved space (originally
// E3REL; release_mod was dropped voicecard-side at protocol 0x21).
static const uint8_t kP2E1DEC = 0;
static const uint8_t kP2TUN2  = 1;   // OSC2 coarse pitch (patch addr 6, int8)
static const uint8_t kP2E2DEC = 2;
static const uint8_t kP2FIN2  = 3;   // OSC2 detune       (patch addr 7, int8)
static const uint8_t kP2E3DEC = 4;
static const uint8_t kP2FOLD  = 5;   // pre-filter wavefolder (patch addr 111)
static const uint8_t kP2NOIS  = 6;
static const uint8_t kP2SUB   = 7;

// ---- steppage[] indices (Step Page — Behavior) ----
static const uint8_t kSPPROB  = 0;
static const uint8_t kSPSSUB  = 1;
static const uint8_t kSPREPT  = 2;
static const uint8_t kSPRATE  = 3;
static const uint8_t kSPVEL   = 4;
static const uint8_t kSPGLID  = 5;
static const uint8_t kSPMINT  = 6;
// kSPMDIR byte packs two fields:
//   bits 0..2: MDIR (0..7) — wave shape (up/dn/ud/ud+/ud-/rnd/rnd+/rnd-)
//   bits 3..4: MOCT (0..3) — range cap, decoded as 1..4 octaves
static const uint8_t kSPMDIR  = 7;
inline uint8_t MdirOf(uint8_t b) { return b & 0x07; }
inline uint8_t MoctOf(uint8_t b) { return ((b >> 3) & 0x03) + 1; }
inline uint8_t PackMdirMoct(uint8_t mdir, uint8_t moct_one_based) {
  return (mdir & 0x07) | (((moct_one_based - 1) & 0x03) << 3);
}

// ---- page3[] indices (Voice Page 3 — Filter / Pitch / Sub) ----
// lock indices 24..27, snapshot bytes 16..19.
static const uint8_t kP3FREQ  = 0;  // filter cutoff     (patch addr 16)
static const uint8_t kP3FAMT  = 1;  // ENV2→VCF depth    (patch addr 22)
static const uint8_t kP3PAMT  = 2;  // ENV3→pitch depth  (patch addr 58)
static const uint8_t kP3WAVE  = 3;  // sub-osc waveform  (patch addr 11)

// v4.4 WS1+: grew from 28 to 48 to cover every patch-page lockable cell.
// Slots 0..27 retain their v4.4-mid meaning; slots 28..47 add the
// previously config-only patch params (osc1 rang reclaiming the dead
// slot 7; xmod/fuzz/crsh/reso/mode; E1/E2/E3 rise+curv and E1 depth;
// LFO4/LFO5 × rate/wave/dest/dept).
static const uint8_t kNumLockableParams = 48;
static const uint8_t kNumStepsPerTrack  = 8;

// bit 0 of step_flags: trigger enabled
static const uint8_t kStepFlagOn     = 0x01;
// bit 1 of step_flags: substep gating active (SSUB>0 ratchets gated by substep_bits)
static const uint8_t kStepFlagGated  = 0x02;
// bits 2..5 of step_flags: SMOD (step modifier) nibble — 0..13. See kSmod*
// constants below. Bits 6..7 reserved.
static const uint8_t kStepFlagSmodMask = 0x3c;

// SMOD values (4 bits). Read/written via StepSmod / SetStepSmod.
static const uint8_t kSmodNone = 0;   // fire normally
static const uint8_t kSmodSkip = 1;   // do not fire; advance and re-evaluate
static const uint8_t kSmodFwd  = 2;   // sticky: set track DIRN to forward
static const uint8_t kSmodRev  = 3;   // sticky: set track DIRN to reverse
static const uint8_t kSmodDir  = 4;   // transient: toggle pendulum shadow dir
static const uint8_t kSmodRjmp = 5;   // jump to a random step
static const uint8_t kSmodJmp1 = 6;   // jump to step 1..8 (kSmodJmp1+N-1)
static const uint8_t kSmodJmp8 = 13;
static const uint8_t kSmodESkp = 14;  // explicit skip: skipped on normal iter; fires only when summoned by a jump SMOD
static const uint8_t kSmodCount = 15;

// PROB byte encoding (v4.3, bipolar).
//   bit 7 clear : low 7 bits = % roll, 0..127 → 0..100% (v4.2-compatible).
//   bit 7 set   : low 7 bits = slot index into kProbCyclePhase[] (1..55),
//                 with slot 0 (byte 0x80) and any out-of-range slot meaning
//                 "always fire" (center dead zone).
static const uint8_t kProbAlways = 0x80;
static const uint8_t kProbCyclePhaseCount = 55;

// Evaluate a PROB byte against the per-track loop counter. Returns 0/1.
uint8_t ProbRoll(uint8_t prob_byte, uint8_t loop_count);

// Encode a 0..127 pot value into a PROB byte using the bipolar pot layout.
uint8_t ProbEncodePot(uint8_t pot);

// Decode a cycle-phase slot index (1..kProbCyclePhaseCount) to its packed
// entry byte ((neg << 7) | (X << 4) | N). Returns 0 for out-of-range slot.
uint8_t ProbCyclePhaseEntry(uint8_t slot);

// Render a bipolar PROB byte into a 4-char field at buf[0..3]:
// "NN%" / "100" / "X:N" / "!X:N" / "FILL" / "!FIL". Shared by step PROB (#6)
// and per-lock PROB drill-in (#38 slim + WS1 patch pages).
void WriteProbByte(char* buf, uint8_t v);

// SeqStep — 2 bytes (v4.4). Holds only trigger-gate state. All 28 lockable
// parameters (including NOTE / PROB / SSUB / REPT / VEL / GLID) resolve at
// fire time from tr.defaults[li] plus any LockPool entry keyed by
// (track, step, li).
struct SeqStep {
  uint8_t step_flags;     // bit 0=on, bit 1=gated, bits 2..5=SMOD nibble
  uint8_t substep_bits;   // 8-bit sub-step bitfield (SSUB = -1 or -2)
};

// SSUB lock byte (lock_index 17) holds the signed -2..+8 value bit-cast to
// uint8_t; REPT lock byte (lock_index 18) holds 0..15 directly. The track
// default in tr.defaults[16 + kSPSSUB] uses the same int8-cast encoding.
inline int8_t SsubOf(uint8_t b) {
  return static_cast<int8_t>(b);
}
inline uint8_t ReptOf(uint8_t b) {
  return b & 0x0f;
}

inline uint8_t StepSmod(const SeqStep& s) {
  return (s.step_flags >> 2) & 0x0f;
}
inline void SetStepSmod(SeqStep& s, uint8_t v) {
  s.step_flags = (s.step_flags & ~kStepFlagSmodMask) | ((v & 0x0f) << 2);
}

// ---- pattern[] indices ----
// v4.2: kPatBPCH slot retired (was index 6 in v4.1, unused since round 5).
// pattern[] shrinks from 8 to 7 bytes; kPatVOL moves from 7 → 6.
static const uint8_t kPatDIRN = 0;
static const uint8_t kPatCDIV = 1;
static const uint8_t kPatROTA = 2;
static const uint8_t kPatLENG = 3;
static const uint8_t kPatSCAL = 4;
static const uint8_t kPatROOT = 5;
static const uint8_t kPatVOL  = 6;   // track velocity scale

// DIRN values
static const uint8_t kDirnFwd  = 0;
static const uint8_t kDirnRev  = 1;
static const uint8_t kDirnPend = 2;
static const uint8_t kDirnRnd  = 3;

// ---- config[] indices (voice config — non-lockable) ----
static const uint8_t kCfgFREQ  = 0;   // filter cutoff
static const uint8_t kCfgRES   = 1;   // filter resonance
static const uint8_t kCfgTYPE  = 2;   // filter mode (LP/BP/HP)
static const uint8_t kCfgDRIV  = 3;   // drive
static const uint8_t kCfgBITS  = 4;   // bit reduction
static const uint8_t kCfgLSHP  = 5;   // LFO4 shape
static const uint8_t kCfgLFO4D = 6;   // LFO4 destination
static const uint8_t kCfgLFOS  = 7;   // LFO4 rate
static const uint8_t kCfgLFO4A = 8;   // LFO4 amount
static const uint8_t kCfgLFOR  = 9;   // LFO4 retrigger
static const uint8_t kCfgTRAK  = 10;  // filter KB tracking
static const uint8_t kCfgE1ATK = 11;  // Env1 attack
static const uint8_t kCfgE2ATK = 12;  // Env2 attack
static const uint8_t kCfgE3ATK = 13;  // Env3 attack
static const uint8_t kCfgE1CRV = 14;  // Env1 decay curve (0=linear, 255=expo)
static const uint8_t kCfgE2CRV = 15;  // Env2 decay curve
static const uint8_t kCfgE3CRV = 16;  // Env3 decay curve
static const uint8_t kCfgPHSE  = 17;  // oscillator phase reset on trigger
static const uint8_t kCfgSMTH   = 18;  // portamento / smoothing
static const uint8_t kCfgVELAMT = 19;  // velocity → VCA amount (mod slot 11)
static const uint8_t kCfgOSC1R = 20;  // osc1 range
static const uint8_t kCfgL5SH  = 21;  // LFO5 shape (was kCfgOSC2R — unused)
static const uint8_t kCfgL5FR  = 22;  // LFO5 rate  (was kCfgOSC2D — unused)
static const uint8_t kCfgFMOP  = 23;  // FM/crossmod operator mode
static const uint8_t kCfgFUZZ  = 24;  // fuzz
static const uint8_t kCfgE1DEPT = 25; // ENV1→VCA depth (mod slot 10 amount)
static const uint8_t kCfgE2DEPT = 26; // ENV2→VCF depth (filter_env)
static const uint8_t kCfgL5RT  = 27;  // LFO5 retrigger (was kCfgE3DEPT — unused)
static const uint8_t kCfgWSUB  = 28;  // sub-osc waveform shape
static const uint8_t kCfgL5D   = 29;  // LFO5 destination (mod slot 6)
static const uint8_t kCfgL5A   = 30;  // LFO5 amount      (mod slot 6)

static const uint8_t kCfgSIZE  = 31;

// ---- shadow[] indices (transient playhead state; zeroed on Reset/load) ----
static const uint8_t kShdwSTEP = 0;  // next step index to fire (0–7)
static const uint8_t kShdwTICK = 1;  // ticks elapsed within current CDIV period
static const uint8_t kShdwREPT = 2;  // step repeats remaining (Phase 5)
static const uint8_t kShdwSSUB = 3;  // sub-step position (Phase 5)
static const uint8_t kShdwDIR  = 4;  // pendulum direction: 0=fwd, 1=rev
static const uint8_t kShdwLAST = 5;  // most-recently-fired step (for chaselight LED)
static const uint8_t kShdwPROB = 6;  // PROB roll outcome for current main step (1=fire-allowed)
static const uint8_t kShdwLOOP = 7;  // per-track loop counter, increments at pattern wrap
static const uint8_t kShdwSubs = 8;  // 1 = substep machinery active this fire (SUBS PROB gate)
static const uint8_t kShdwSIZE = 9;

// SeqTrack — v4.4-WS1: 16 + 7 + 48 + kCfgSIZE + kShdwSIZE = 111 bytes/track.
// defaults[N]: fallback value for lockable param N when no pool entry exists.
//   defaults[0..7]   = page1 (NOTE..FINE)        — defaults[0] (NOTE) unused; NOTE is intrinsic
//                      defaults[7] = osc1 RANGE (v4.4-WS1; reclaimed from dead OSC1_DETUNE slot)
//   defaults[8..15]  = page2 (E1DEC, TUN2, E2DEC, FIN2, E3DEC, FOLD, NOIS, SUB)
//   defaults[16..23] = steppage (PROB, SSUB, REPT, RATE, VEL, GLID, MINT, MDIR)
//                      defaults[16,17,18,20,21] (PROB/SSUB/REPT/VEL/GLID) unused — intrinsic
//   defaults[24..27] = page3 (FREQ, FAMT, PAMT, WAVE)
//   defaults[28..47] = WS1 extensions (xmod, fuzz, crsh, reso, mode,
//                      E1 rise/curv/depth, E2 rise/curv, E3 rise/curv,
//                      LFO4 rate/wave/dest/dept, LFO5 rate/wave/dest/dept)
// config[kCfgSIZE]: residual voice config — bytes for the params migrated to
//   defaults[28..47] are still allocated for ABI continuity but no longer
//   written via PatchAddrToSeqField; they're vestigial pending cleanup.
// shadow[]: transient playhead; zeroed on Reset.
struct SeqTrack {
  SeqStep steps[8];          // 16 bytes (8 × 2)
  uint8_t pattern[7];        // DIRN, CDIV, ROTA, LENG, SCAL, ROOT, VOL
  uint8_t defaults[48];      // default value per lockable param (48 slots, v4.4-WS1)
  uint8_t config[kCfgSIZE];  // voice config (vestigial bytes pending cleanup)
  uint8_t shadow[kShdwSIZE]; // transient playhead state
};

struct SeqGlobal {
  uint8_t transport;     // kSeqStopped / kSeqPlaying / kSeqPaused
  uint8_t hold_mode;     // 0=Voltage Block, 1=Elektron
  uint8_t swing;
  uint8_t active_track;  // 0–5
  uint8_t lock_page;     // 0=Voice1, 1=Voice2, 2=Step
  uint8_t held_step;     // 0xff=none; step index during parameter lock edit
  uint16_t master_tick;  // ticks since last Reset; gates mrst trigger
};

static const uint8_t kSeqStopped = 0;
static const uint8_t kSeqPlaying = 1;
static const uint8_t kSeqPaused  = 2;

class Sequencer {
 public:
  Sequencer() {}
  void Init();
  void Clock(uint8_t ticks);
  void Play();
  void Pause();
  void Reset();
  void Stop();
  void Panic();

  SeqTrack* mutable_track(uint8_t i) { return &tracks_[i]; }
  const SeqTrack& track(uint8_t i) const { return tracks_[i]; }
  SeqGlobal* mutable_global() { return &global_; }
  const SeqGlobal& global() const { return global_; }

  LockPool& mutable_lock_pool() { return lock_pool_; }
  const LockPool& lock_pool() const { return lock_pool_; }

  LockProbPool& mutable_lock_prob_pool() { return lock_prob_pool_; }
  const LockProbPool& lock_prob_pool() const { return lock_prob_pool_; }

  // Resolve a lockable param's value at (track, step).
  // For intrinsic locks (NOTE/VEL/PROB/SSUB/REPT/GLID): reads from SeqStep.
  // For pool-backed locks: pool entry if present, else tracks_[t].defaults[lock_index].
  uint8_t StepLockedValue(
      uint8_t track, uint8_t step, uint8_t lock_index) const;

  // Returns 1 if (track, step, lock_index) has a non-default value held.
  // Intrinsic locks: always "held" (storage is dedicated). Pool: 1 if entry exists.
  uint8_t StepIsLocked(
      uint8_t track, uint8_t step, uint8_t lock_index) const;

  // Write a per-step lock value. Intrinsic locks always write to SeqStep.
  // Pool-backed locks: allocate/update a pool entry. Returns 1 on success,
  // 0 on pool-full (the caller should flash POOL FULL).
  uint8_t SetStepLock(
      uint8_t track, uint8_t step, uint8_t lock_index, uint8_t value);

  // Clear a per-step lock. For intrinsic locks: resets the field to the track
  // default (defaults[lock_index]). For pool-backed: removes the pool entry.
  void ClearStepLock(uint8_t track, uint8_t step, uint8_t lock_index);

  // Clear every per-step lock for (track, step). Resets intrinsic fields to
  // their track defaults and removes every pool entry for the (track, step)
  // pair. Used by the double-tap-clear UX.
  void ClearAllStepLocks(uint8_t track, uint8_t step);

  // Clear (track, *, lock_index) — every step's lock+prob entry for one
  // lock_index on one track. Used by the encoder-long-press clear (#42).
  void ClearTrackLocksForParam(uint8_t track, uint8_t lock_index);

 private:
  void AdvanceStep(uint8_t t);
  void FireStep(uint8_t t, uint8_t step_index, uint8_t sub_idx);

  SeqTrack     tracks_[kNumVoices];
  SeqGlobal    global_;
  LockPool     lock_pool_;
  LockProbPool lock_prob_pool_;
};

extern Sequencer sequencer;
extern const uint8_t kDefaultMod[42];

// Map a (Parameter table id, instance_index) to a sequencer lock_index 0..47.
// Returns 0xff if the parameter is not lockable. `instance` is used only for
// per-EG envelope parameters (PATCH_ENV_DECAY across env 0/1/2 → locks 8/10/12).
uint8_t ParamIdToLockIndex(uint8_t param_id, uint8_t instance);

// Seed defaults[28..47] from PROGMEM. Called by Init() and by the v0x06
// snapshot-load migration path to populate the new WS1 lockable slots.
void SeedExtendedDefaults(SeqTrack& tr);

}  // namespace ambika

#endif  // CONTROLLER_SEQUENCER_H_
