// Copyright 2011 Emilie Gillet.
//
// Phase 3: Sequencer data structures.
// SeqStep (29B), SeqTrack (297B × 6 = 1,782B), SeqGlobal (32B).

#ifndef CONTROLLER_SEQUENCER_H_
#define CONTROLLER_SEQUENCER_H_

#include "avrlib/base.h"
#include "controller/controller.h"

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

// ---- page2[] indices (Voice Page 2 — Envelope DEC/REL + mixer) ----
static const uint8_t kP2E1DEC = 0;
static const uint8_t kP2E1REL = 1;
static const uint8_t kP2E2DEC = 2;
static const uint8_t kP2E2REL = 3;
static const uint8_t kP2E3DEC = 4;
static const uint8_t kP2E3REL = 5;
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
static const uint8_t kSPMDIR  = 7;

static const uint8_t kNumLockableParams = 24;
static const uint8_t kNumStepsPerTrack  = 8;

// bit 0 of step_flags: trigger enabled
static const uint8_t kStepFlagOn = 0x01;

// SeqStep — 29 bytes.
// lock_flags bit N: lockable param N is overridden for this step.
//   N = page_index*8 + param_index, where page_index: 0=page1, 1=page2, 2=steppage.
// SSUB: uint8_t, interpreted as int8_t. -2=Edit, -1=Custom, 0=normal, +1..+8=ratchets.
struct SeqStep {
  uint8_t page1[8];       // NOTE, WAVE1, PARA1, BLND, RTIO, WAVE2, PARA2, FINE
  uint8_t page2[8];       // LPGD, LPGA, LPGO, NOIS, PITD, PITA, WAVE_sub, SUB
  uint8_t steppage[8];    // PROB, SSUB, REPT, RATE, VEL, GLID, MINT, MDIR
  uint8_t lock_flags[3];  // 24-bit lock bitfield (one bit per lockable param)
  uint8_t step_flags;     // bit 0: on
  uint8_t substep_bits;   // 8-bit sub-step bitfield (SSUB = -1 or -2)
};

// ---- pattern[] indices ----
static const uint8_t kPatDIRN = 0;
static const uint8_t kPatCDIV = 1;
static const uint8_t kPatROTA = 2;
static const uint8_t kPatLENG = 3;
static const uint8_t kPatSCAL = 4;
static const uint8_t kPatROOT = 5;
static const uint8_t kPatBPCH = 6;
static const uint8_t kPatOLEV = 7;

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
static const uint8_t kCfgLFOR  = 9;   // LFO4 reset mode
static const uint8_t kCfgTRAK  = 10;  // filter KB tracking
static const uint8_t kCfgE1ATK = 11;  // Env1 attack
static const uint8_t kCfgE2ATK = 12;  // Env2 attack
static const uint8_t kCfgE3ATK = 13;  // Env3 attack
static const uint8_t kCfgE1CRV = 14;  // Env1 decay curve (0=linear, 255=expo)
static const uint8_t kCfgE2CRV = 15;  // Env2 decay curve
static const uint8_t kCfgE3CRV = 16;  // Env3 decay curve
static const uint8_t kCfgPHSE  = 17;  // oscillator phase reset on trigger
static const uint8_t kCfgSMTH  = 18;  // portamento / smoothing
// config[19]: reserved
static const uint8_t kCfgOSC1R = 20;  // osc1 range
static const uint8_t kCfgOSC2R = 21;  // osc2 range
static const uint8_t kCfgOSC2D = 22;  // osc2 detune
static const uint8_t kCfgFMOP  = 23;  // FM/crossmod operator mode
static const uint8_t kCfgFUZZ  = 24;  // fuzz
static const uint8_t kCfgE1DEPT = 25; // ENV1→VCA depth (mod slot 10 amount)
static const uint8_t kCfgE2DEPT = 26; // ENV2→VCF depth (filter_env)
static const uint8_t kCfgE3DEPT = 27; // ENV3→pitch depth (mod slot 2 amount)
static const uint8_t kCfgWSUB  = 28;  // sub-osc waveform shape

static const uint8_t kCfgSIZE  = 29;

// ---- shadow[] indices (transient playhead state; zeroed on Reset/load) ----
static const uint8_t kShdwSTEP = 0;  // next step index to fire (0–7)
static const uint8_t kShdwTICK = 1;  // ticks elapsed within current CDIV period
static const uint8_t kShdwREPT = 2;  // step repeats remaining (Phase 5)
static const uint8_t kShdwSSUB = 3;  // sub-step position (Phase 5)
static const uint8_t kShdwDIR  = 4;  // pendulum direction: 0=fwd, 1=rev

// SeqTrack — 232+8+24+kCfgSIZE+5 bytes per track.
// defaults[N]: default value for lockable param N.
//   defaults[0..7] = page1, [8..15] = page2 (E1DEC/REL/E2DEC/REL/E3DEC/REL/NOIS/SUB), [16..23] = steppage.
// config[kCfgSIZE]: voice config (filter, LFO4, env ATK/CRV/DEPT, osc, mixer).
// shadow[5]: transient playhead; zeroed on Reset.
// Mod matrix routing is fixed; only amounts (at Patch bytes 58/72/73/82) live in config[].
struct SeqTrack {
  SeqStep steps[8];          // 232 bytes
  uint8_t pattern[8];        // DIRN, CDIV, ROTA, LENG, SCAL, ROOT, BPCH, OLEV
  uint8_t defaults[24];      // default value per lockable param
  uint8_t config[kCfgSIZE];  // voice config
  uint8_t shadow[5];         // transient playhead state
};

struct SeqGlobal {
  uint8_t transport;     // kSeqStopped / kSeqPlaying / kSeqPaused
  uint8_t hold_mode;     // 0=Voltage Block, 1=Elektron
  uint8_t swing;
  uint8_t active_track;  // 0–5
  uint8_t lock_page;     // 0=Voice1, 1=Voice2, 2=Step
  uint8_t held_step;     // 0xff=none; step index during parameter lock edit
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

  SeqTrack* mutable_track(uint8_t i) { return &tracks_[i]; }
  const SeqTrack& track(uint8_t i) const { return tracks_[i]; }
  SeqGlobal* mutable_global() { return &global_; }
  const SeqGlobal& global() const { return global_; }

 private:
  void AdvanceStep(uint8_t t);
  void FireStep(uint8_t t, uint8_t step_index);

  SeqTrack  tracks_[kNumVoices];  // 6 × 297 = 1,782 bytes
  SeqGlobal global_;               // 32 bytes
};

extern Sequencer sequencer;
extern const uint8_t kDefaultMod[42];

}  // namespace ambika

#endif  // CONTROLLER_SEQUENCER_H_
