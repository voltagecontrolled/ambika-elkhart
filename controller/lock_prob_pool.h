// Per-lock probability pool (v4.3, issue #38 slim).
//
// Sparse parallel pool: each entry attaches a PROB byte (bipolar encoding,
// shared with step PROB / issue #6) to a (track, step, param) lock. At fire
// time, after the main lock pool has overlaid the snapshot, each overlaid
// lock that has a matching prob-pool entry rolls the byte via ProbRoll().
// On fail, the snapshot slot reverts to the track default so the lock
// effectively does not apply that loop.
//
// Capacity 32 (96 B + 1 count byte). Lookup is O(32). Snapshot v0x04 adds
// this pool after the existing LockPool blob; v0x03 loads zero-fill it.
//
// Independent gating semantic: composes freely with step PROB (#6) — both
// roll separately, each loop.

#ifndef CONTROLLER_LOCK_PROB_POOL_H_
#define CONTROLLER_LOCK_PROB_POOL_H_

#include "avrlib/base.h"

namespace ambika {

static const uint8_t kLockProbPoolCapacity = 32;
static const uint8_t kLockProbPoolFree     = 0xff;  // param sentinel = empty slot

// Synthetic prob-pool keys for intrinsic fields that aren't in the regular
// Parameter table. These reuse the (track, step, param) addressing of the
// pool to attach a PROB byte to non-pool-backed step state.
//   28 = SMOD nibble (in step_flags). Drill-in via the SFX cell on the
//        step page; fire-time gate suppresses the SMOD effect on roll fail.
//   29 = SUBS machinery (ratchets/repeats/chord walk). Drill-in via the
//        SUBS cell; on roll fail the step still fires (subject to step
//        PROB) but ratchets/repeats/chord-walk are suppressed for that loop.
static const uint8_t kProbKeySmod = 28;
static const uint8_t kProbKeySubs = 29;

struct LockProbEntry {
  uint8_t ts;     // track in bits 0..2, step in bits 3..5 (LockTsPack-compatible)
  uint8_t param;  // Parameter table index; 0xff = free slot
  uint8_t prob;   // bipolar PROB byte (see sequencer.h)
};

class LockProbPool {
 public:
  LockProbPool() {}
  void Init();

  uint8_t Find(uint8_t track, uint8_t step, uint8_t param) const;

  inline uint8_t GetProb(
      uint8_t track, uint8_t step, uint8_t param) const {
    uint8_t i = Find(track, step, param);
    return (i == 0xff) ? 0x80 : entries_[i].prob;  // 0x80 = always-fire default
  }

  // Returns 1 on success, 0 if pool full.
  uint8_t Set(uint8_t track, uint8_t step, uint8_t param, uint8_t prob);

  void Clear(uint8_t track, uint8_t step, uint8_t param);
  void ClearStep(uint8_t track, uint8_t step);
  void ClearTrack(uint8_t track);
  void ClearAll();

  inline uint8_t count()    const { return count_; }
  inline uint8_t capacity() const { return kLockProbPoolCapacity; }

  static const uint16_t kRawEntriesSize =
      static_cast<uint16_t>(sizeof(LockProbEntry)) * kLockProbPoolCapacity;
  inline const uint8_t* raw_entries() const {
    return reinterpret_cast<const uint8_t*>(entries_);
  }
  inline uint8_t* mutable_raw_entries() {
    return reinterpret_cast<uint8_t*>(entries_);
  }
  inline void set_count(uint8_t c) { count_ = c; }

  inline const LockProbEntry& entry(uint8_t i) const { return entries_[i]; }

 private:
  uint8_t FindFree() const;

  LockProbEntry entries_[kLockProbPoolCapacity];
  uint8_t       count_;
};

}  // namespace ambika

#endif  // CONTROLLER_LOCK_PROB_POOL_H_
