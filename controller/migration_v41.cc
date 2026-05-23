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

/* static */
uint8_t MigrationV41::LoadAllTracks(uint8_t* checksum) {
  // Reset the pools — we'll rebuild lock_pool from per-step lock_flags.
  // v4.1 predates the per-lock PROB pool, so it just zero-fills.
  sequencer.mutable_lock_pool().Init();
  sequencer.mutable_lock_prob_pool().Init();

  // Stream-read in two scratch buffers.
  uint8_t step_buf[kV41StepSize];
  uint8_t tail_buf[kV41TailSize];
  // Stash each step's raw page bytes so we can emit pool entries for locked
  // intrinsics after the tail (defaults) has been read.
  uint8_t step_page_bytes[8][6];   // [step][NOTE,PROB,SSUB,REPT,VEL,GLID]
  uint8_t step_lock_bits[8];       // bit per intrinsic, see ordering below
  uint16_t got;

  // Map ordering for step_page_bytes / step_lock_bits.
  //   bit 0 = NOTE  (li 0,  src[0])
  //   bit 1 = PROB  (li 16, src[16+0])
  //   bit 2 = SSUB  (li 17, src[16+1])
  //   bit 3 = REPT  (li 18, src[16+2])
  //   bit 4 = VEL   (li 20, src[16+4])
  //   bit 5 = GLID  (li 21, src[16+5])

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

      step_page_bytes[s][0] = src[0];        // NOTE
      step_page_bytes[s][1] = src[16 + 0];   // PROB
      step_page_bytes[s][2] = src[16 + 1];   // SSUB (raw signed -2..+8)
      step_page_bytes[s][3] = src[16 + 2];   // REPT
      step_page_bytes[s][4] = src[16 + 4];   // VEL
      step_page_bytes[s][5] = src[16 + 5];   // GLID
      step.step_flags    = src[32];
      step.substep_bits  = src[33];

      uint8_t lf0 = src[28];
      uint8_t lf2 = src[30];
      uint8_t bits = 0;
      if (lf0 & 0x01) bits |= 0x01;  // NOTE
      if (lf2 & 0x01) bits |= 0x02;  // PROB
      if (lf2 & 0x02) bits |= 0x04;  // SSUB
      if (lf2 & 0x04) bits |= 0x08;  // REPT
      if (lf2 & 0x10) bits |= 0x10;  // VEL
      if (lf2 & 0x20) bits |= 0x20;  // GLID
      step_lock_bits[s] = bits;

      // Pool entries for non-intrinsic lock indices. lock_flags bytes at
      // src[28..31]; bit b of byte k covers lock_index k*8 + b. NOTE/PROB/
      // SSUB/REPT/VEL/GLID are now also pool-backed, but for those we emit
      // below after defaults are known so we can dedup against the default.
      for (uint8_t li = 0; li < 28; ++li) {
        if (li == 0 || li == 16 || li == 17 || li == 18 ||
            li == 20 || li == 21) continue;
        uint8_t bit = src[28 + (li >> 3)] & (1 << (li & 7));
        if (!bit) continue;
        sequencer.SetStepLock(t, s, li, src[li]);
      }
    }

    // ---- Tail: pattern (8) + defaults (28) + config (29) = 65 bytes ----
    if (Storage::file_.Read(tail_buf, kV41TailSize, &got) != FS_OK
        || got != kV41TailSize) {
      return 0;
    }
    for (uint16_t i = 0; i < kV41TailSize; ++i) *checksum += tail_buf[i];

    const uint8_t* pat = tail_buf;
    for (uint8_t i = 0; i < 6; ++i) dst->pattern[i] = pat[i];
    dst->pattern[6] = pat[7];  // VOL

    const uint8_t* defs = pat + kV41PatternSize;
    for (uint8_t i = 0; i < 28; ++i) dst->defaults[i] = defs[i];

    const uint8_t* cfg = defs + kV41DefaultsSize;
    for (uint8_t i = 0; i < kV41ConfigSize; ++i) dst->config[i] = cfg[i];
    for (uint8_t i = kV41ConfigSize; i < kCfgSIZE; ++i) dst->config[i] = 0;

    // Emit pool entries for locked intrinsics. Where the locked value
    // matches the track default we still emit so playback exactly mirrors
    // v4.1 — the original patch authored a lock, and the pool entry is
    // what now distinguishes "locked to this value" from "follows default".
    for (uint8_t s = 0; s < 8; ++s) {
      uint8_t bits = step_lock_bits[s];
      if (bits & 0x01) sequencer.SetStepLock(t, s, 0,  step_page_bytes[s][0]);
      if (bits & 0x02) sequencer.SetStepLock(t, s, 16, step_page_bytes[s][1]);
      if (bits & 0x04) sequencer.SetStepLock(t, s, 17, step_page_bytes[s][2]);
      if (bits & 0x08) sequencer.SetStepLock(t, s, 18, step_page_bytes[s][3]);
      if (bits & 0x10) sequencer.SetStepLock(t, s, 20, step_page_bytes[s][4]);
      if (bits & 0x20) sequencer.SetStepLock(t, s, 21, step_page_bytes[s][5]);
    }
  }
  return 1;
}

}  // namespace ambika
