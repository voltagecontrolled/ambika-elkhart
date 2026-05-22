// Per-step parameter lock pool (issue #30).
//
// Replaces dense per-step lock storage. Stores up to 192 sparse
// (track, step, param_id, value) tuples in a global array.
// Lookup is O(192) worst case, filtered by (track, step). On the 644p
// audio-rate eval path, current measurements put this within budget; a
// per-track linked-list head can be added later if profiling shows the
// scan is hot.
//
// Param IDs are Parameter table indices (0..kNumParameters-1), matching
// what the UI cell descriptors and ParameterManager already use.

#ifndef CONTROLLER_LOCK_POOL_H_
#define CONTROLLER_LOCK_POOL_H_

#include "avrlib/base.h"

namespace ambika {

static const uint8_t kLockPoolCapacity = 192;
static const uint8_t kLockPoolFree     = 0xff;  // param_id sentinel = empty slot

struct LockEntry {
  // byte 0: track in bits 0..2, step in bits 3..5; bits 6..7 reserved
  uint8_t ts;
  uint8_t param;   // Parameter table index; 0xff = free slot
  uint8_t value;
};

inline uint8_t LockTsPack(uint8_t track, uint8_t step) {
  return (track & 0x07) | ((step & 0x07) << 3);
}
inline uint8_t LockTsTrack(uint8_t ts) { return ts & 0x07; }
inline uint8_t LockTsStep (uint8_t ts) { return (ts >> 3) & 0x07; }

class LockPool {
 public:
  LockPool() {}
  void Init();

  // Lookup: returns pool slot index (0..kLockPoolCapacity-1) if found,
  // or 0xff if no lock for (track, step, param).
  uint8_t Find(uint8_t track, uint8_t step, uint8_t param) const;

  // Returns locked value if found, else default_value.
  inline uint8_t Get(
      uint8_t track, uint8_t step, uint8_t param, uint8_t default_value) const {
    uint8_t i = Find(track, step, param);
    return (i == 0xff) ? default_value : entries_[i].value;
  }

  // Write a lock. If (track, step, param) already exists, overwrites value.
  // Otherwise allocates a new slot. Returns 1 on success, 0 if pool is full.
  uint8_t Set(uint8_t track, uint8_t step, uint8_t param, uint8_t value);

  // Clear a single lock. No-op if not present.
  void Clear(uint8_t track, uint8_t step, uint8_t param);

  // Clear every lock belonging to a (track, step) pair. Used when a step's
  // intrinsic NOTE/etc are reset, and on track-clear.
  void ClearStep(uint8_t track, uint8_t step);

  // Clear every lock on a track (track-clear UX).
  void ClearTrack(uint8_t track);

  // Wipe the entire pool.
  void ClearAll();

  inline uint8_t count()    const { return count_; }
  inline uint8_t capacity() const { return kLockPoolCapacity; }

  // Raw access for snapshot save/load. The serialized pool is [count_ byte]
  // followed by [kRawEntriesSize bytes] of LockEntry array data.
  static const uint16_t kRawEntriesSize =
      static_cast<uint16_t>(sizeof(LockEntry)) * kLockPoolCapacity;
  inline const uint8_t* raw_entries() const {
    return reinterpret_cast<const uint8_t*>(entries_);
  }
  inline uint8_t* mutable_raw_entries() {
    return reinterpret_cast<uint8_t*>(entries_);
  }
  inline void set_count(uint8_t c) { count_ = c; }

  // Iteration over pool slots — used by hot eval paths that resolve many
  // lockable values for a single (track, step) in one pass.
  inline const LockEntry& entry(uint8_t i) const { return entries_[i]; }

 private:
  // Returns index of first free slot, or 0xff if none.
  uint8_t FindFree() const;

  LockEntry entries_[kLockPoolCapacity];
  uint8_t   count_;
};

}  // namespace ambika

#endif  // CONTROLLER_LOCK_POOL_H_
