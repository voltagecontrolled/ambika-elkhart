// Per-step parameter lock pool (issue #30).

#include "controller/lock_pool.h"

namespace ambika {

void LockPool::Init() {
  ClearAll();
}

void LockPool::ClearAll() {
  for (uint8_t i = 0; i < kLockPoolCapacity; ++i) {
    entries_[i].param = kLockPoolFree;
  }
  count_ = 0;
}

uint8_t LockPool::FindFree() const {
  for (uint8_t i = 0; i < kLockPoolCapacity; ++i) {
    if (entries_[i].param == kLockPoolFree) return i;
  }
  return 0xff;
}

uint8_t LockPool::Find(uint8_t track, uint8_t step, uint8_t param) const {
  uint8_t ts = LockTsPack(track, step);
  for (uint8_t i = 0; i < kLockPoolCapacity; ++i) {
    if (entries_[i].param == param && entries_[i].ts == ts) {
      return i;
    }
  }
  return 0xff;
}

uint8_t LockPool::Set(
    uint8_t track, uint8_t step, uint8_t param, uint8_t value) {
  uint8_t i = Find(track, step, param);
  if (i != 0xff) {
    entries_[i].value = value;
    return 1;
  }
  i = FindFree();
  if (i == 0xff) return 0;  // pool full
  entries_[i].ts    = LockTsPack(track, step);
  entries_[i].param = param;
  entries_[i].value = value;
  ++count_;
  return 1;
}

void LockPool::Clear(uint8_t track, uint8_t step, uint8_t param) {
  uint8_t i = Find(track, step, param);
  if (i != 0xff) {
    entries_[i].param = kLockPoolFree;
    --count_;
  }
}

void LockPool::ClearStep(uint8_t track, uint8_t step) {
  uint8_t ts = LockTsPack(track, step);
  for (uint8_t i = 0; i < kLockPoolCapacity; ++i) {
    if (entries_[i].param != kLockPoolFree && entries_[i].ts == ts) {
      entries_[i].param = kLockPoolFree;
      --count_;
    }
  }
}

void LockPool::ClearTrack(uint8_t track) {
  for (uint8_t i = 0; i < kLockPoolCapacity; ++i) {
    if (entries_[i].param != kLockPoolFree &&
        LockTsTrack(entries_[i].ts) == track) {
      entries_[i].param = kLockPoolFree;
      --count_;
    }
  }
}

}  // namespace ambika
