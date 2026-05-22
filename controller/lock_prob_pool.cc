#include "controller/lock_prob_pool.h"

#include "controller/lock_pool.h"  // LockTsPack

namespace ambika {

void LockProbPool::Init() {
  ClearAll();
}

void LockProbPool::ClearAll() {
  for (uint8_t i = 0; i < kLockProbPoolCapacity; ++i) {
    entries_[i].param = kLockProbPoolFree;
  }
  count_ = 0;
}

uint8_t LockProbPool::FindFree() const {
  for (uint8_t i = 0; i < kLockProbPoolCapacity; ++i) {
    if (entries_[i].param == kLockProbPoolFree) return i;
  }
  return 0xff;
}

uint8_t LockProbPool::Find(uint8_t track, uint8_t step, uint8_t param) const {
  uint8_t ts = LockTsPack(track, step);
  for (uint8_t i = 0; i < kLockProbPoolCapacity; ++i) {
    if (entries_[i].param == param && entries_[i].ts == ts) {
      return i;
    }
  }
  return 0xff;
}

uint8_t LockProbPool::Set(
    uint8_t track, uint8_t step, uint8_t param, uint8_t prob) {
  uint8_t i = Find(track, step, param);
  if (i != 0xff) {
    entries_[i].prob = prob;
    return 1;
  }
  i = FindFree();
  if (i == 0xff) return 0;
  entries_[i].ts    = LockTsPack(track, step);
  entries_[i].param = param;
  entries_[i].prob  = prob;
  ++count_;
  return 1;
}

void LockProbPool::Clear(uint8_t track, uint8_t step, uint8_t param) {
  uint8_t i = Find(track, step, param);
  if (i != 0xff) {
    entries_[i].param = kLockProbPoolFree;
    --count_;
  }
}

void LockProbPool::ClearStep(uint8_t track, uint8_t step) {
  uint8_t ts = LockTsPack(track, step);
  for (uint8_t i = 0; i < kLockProbPoolCapacity; ++i) {
    if (entries_[i].param != kLockProbPoolFree && entries_[i].ts == ts) {
      entries_[i].param = kLockProbPoolFree;
      --count_;
    }
  }
}

void LockProbPool::ClearTrack(uint8_t track) {
  for (uint8_t i = 0; i < kLockProbPoolCapacity; ++i) {
    if (entries_[i].param != kLockProbPoolFree &&
        LockTsTrack(entries_[i].ts) == track) {
      entries_[i].param = kLockProbPoolFree;
      --count_;
    }
  }
}

}  // namespace ambika
