// v4.1 → v4.2 snapshot migration (issue #30).

#include "controller/migration_v41.h"

#include "controller/sequencer.h"
#include "controller/storage.h"

namespace ambika {

// v4.1 byte offsets within a single 34-byte SeqStep blob:
//   page1[0..7]    src[0..7]   — NOTE, WAVE1, PARA1, BLND, RTIO, WAVE2, PARA2, FINE
//   page2[0..7]    src[8..15]  — E1DEC, TUN2, E2DEC, FIN2, E3DEC, _resv_, NOIS, SUB
//   steppage[0..7] src[16..23] — PROB, SSUB, REPT, RATE, VEL, GLID, MINT, MDIR
//   page3[0..3]    src[24..27] — FREQ, FAMT, PAMT, WAVE
//   lock_flags[4]  src[28..31]
//   step_flags     src[32]
//   substep_bits   src[33]
// So byte src[i] for i in 0..27 is exactly the value for lock_index i.
static const uint16_t kV41StepSize = 34;
static const uint16_t kV41PatternSize = 8;
static const uint16_t kV41DefaultsSize = 28;
static const uint16_t kV41ConfigSize = 29;
static const uint16_t kV41TrackSize =
    kV41StepSize * 8 + kV41PatternSize + kV41DefaultsSize + kV41ConfigSize;
// = 272 + 8 + 28 + 29 = 337.

// Total tail size per track after the 8 steps = pattern + defaults + config.
static const uint16_t kV41TailSize =
    kV41PatternSize + kV41DefaultsSize + kV41ConfigSize;

// Bit positions inside our scratch lock_bits[s] byte. Track which v4.1
// intrinsics were locked in each step so we can apply the track default
// post-hoc for unlocked ones.
static const uint8_t kBitNote = 0x01;
static const uint8_t kBitProb = 0x02;
static const uint8_t kBitSsub = 0x04;
static const uint8_t kBitRept = 0x08;
static const uint8_t kBitVel  = 0x10;
static const uint8_t kBitGlid = 0x20;

/* static */
uint8_t MigrationV41::LoadAllTracks(uint8_t* checksum) {
  // Reset the pool — we're rebuilding it from the per-step lock_flags.
  sequencer.mutable_lock_pool().Init();

  // Stream-read in two scratch buffers (34 B + 65 B = 99 B max on stack).
  uint8_t step_buf[kV41StepSize];
  uint8_t tail_buf[kV41TailSize];
  uint8_t lock_bits[8];   // per-step lock bitmap for intrinsics
  uint16_t got;

  for (uint8_t t = 0; t < kNumVoices; ++t) {
    SeqTrack* dst = sequencer.mutable_track(t);

    // ---- 8 steps × 34 bytes ----
    for (uint8_t s = 0; s < 8; ++s) {
      if (Storage::file_.Read(step_buf, kV41StepSize, &got) != FS_OK
          || got != kV41StepSize) {
        return 0;
      }
      for (uint8_t i = 0; i < kV41StepSize; ++i) *checksum += step_buf[i];

      const uint8_t* src = step_buf;
      SeqStep& step = dst->steps[s];

      // Intrinsics — provisionally store the page-byte value. If the
      // lock_flags bit was NOT set in v4.1, playback used the track
      // default; we'll overwrite below once the tail (defaults) is read.
      step.note          = src[0];               // page1[kP1NOTE]
      step.prob          = src[16 + 0];          // steppage[kSPPROB]
      int8_t  ssub_signed = static_cast<int8_t>(src[16 + 1]);  // SSUB
      uint8_t rept        = src[16 + 2];                       // REPT
      step.subs          = PackSubs(ssub_signed, rept);
      step.vel           = src[16 + 4];          // steppage[kSPVEL]
      step.glid          = src[16 + 5];          // steppage[kSPGLID]
      step.step_flags    = src[32];
      step.substep_bits  = src[33];

      // Pack the lock bits for this step's intrinsics. v4.1 lock_flags
      // byte layout: byte 0 = locks 0..7 (page1), byte 2 = locks 16..23
      // (steppage). NOTE=0, PROB=16, SSUB=17, REPT=18, VEL=20, GLID=21.
      uint8_t lf0 = src[28];
      uint8_t lf2 = src[30];
      lock_bits[s] = 0;
      if (lf0 & 0x01) lock_bits[s] |= kBitNote;
      if (lf2 & 0x01) lock_bits[s] |= kBitProb;
      if (lf2 & 0x02) lock_bits[s] |= kBitSsub;
      if (lf2 & 0x04) lock_bits[s] |= kBitRept;
      if (lf2 & 0x10) lock_bits[s] |= kBitVel;
      if (lf2 & 0x20) lock_bits[s] |= kBitGlid;

      // Walk lock_flags and emit pool entries for non-intrinsic lock indices.
      // lock_flags bytes at src[28..31]; bit b of byte k covers lock_index
      // k*8 + b.
      for (uint8_t li = 0; li < 28; ++li) {
        if (IsIntrinsicLock(li)) continue;
        uint8_t bit = src[28 + (li >> 3)] & (1 << (li & 7));
        if (!bit) continue;
        // SetStepLock returns 0 if pool is full — we silently drop.
        sequencer.SetStepLock(t, s, li, src[li]);
      }
    }

    // ---- Tail: pattern (8) + defaults (28) + config (29) = 65 bytes ----
    if (Storage::file_.Read(tail_buf, kV41TailSize, &got) != FS_OK
        || got != kV41TailSize) {
      return 0;
    }
    for (uint16_t i = 0; i < kV41TailSize; ++i) *checksum += tail_buf[i];

    // Pattern: v4.1 had 8 bytes, v4.2 has 7 (kPatBPCH at slot 6 retired).
    // Map src[0..5] → dst->pattern[0..5], src[7] (OLEV/VOL) → dst->pattern[6].
    const uint8_t* pat = tail_buf;
    for (uint8_t i = 0; i < 6; ++i) dst->pattern[i] = pat[i];
    dst->pattern[6] = pat[7];  // VOL

    // Defaults: 28 bytes, layout identical.
    const uint8_t* defs = pat + kV41PatternSize;
    for (uint8_t i = 0; i < 28; ++i) dst->defaults[i] = defs[i];

    // Config: 29 bytes, layout identical.
    const uint8_t* cfg = defs + kV41DefaultsSize;
    for (uint8_t i = 0; i < kCfgSIZE; ++i) dst->config[i] = cfg[i];

    // Second pass: for any step whose v4.1 intrinsic lock bit was clear,
    // overwrite the provisional page-byte value with the track default
    // (which is what v4.1 playback would have used).
    uint8_t def_note = defs[0];
    uint8_t def_prob = defs[16 + 0];
    uint8_t def_ssub = defs[16 + 1];
    uint8_t def_rept = defs[16 + 2];
    uint8_t def_vel  = defs[16 + 4];
    uint8_t def_glid = defs[16 + 5];
    for (uint8_t s = 0; s < 8; ++s) {
      SeqStep& step = dst->steps[s];
      uint8_t bits = lock_bits[s];
      if (!(bits & kBitNote)) step.note = def_note;
      if (!(bits & kBitProb)) step.prob = def_prob;
      if (!(bits & kBitVel))  step.vel  = def_vel;
      if (!(bits & kBitGlid)) step.glid = def_glid;
      if (!(bits & kBitSsub) || !(bits & kBitRept)) {
        int8_t  ssub = (bits & kBitSsub)
            ? SsubOf(step.subs)
            : static_cast<int8_t>(def_ssub);
        uint8_t rept = (bits & kBitRept) ? ReptOf(step.subs) : def_rept;
        step.subs = PackSubs(ssub, rept);
      }
    }
  }
  return 1;
}

}  // namespace ambika
