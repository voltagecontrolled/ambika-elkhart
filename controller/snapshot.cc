#include "controller/snapshot.h"

#include <stddef.h>
#include <string.h>

#include "avrlib/op.h"
#include "avrlib/resources_manager.h"

#include "controller/migration_v41.h"
#include "controller/multi.h"
#include "controller/sequencer.h"
#include "controller/storage.h"

namespace ambika {

using namespace avrlib;

static const char kMagic[4] = { 'E', 'L', 'K', 'S' };
// v0x06 (Elkhart v4.4, issue #45): SeqStep shrinks 7→2 bytes (intrinsic
// per-step storage retired). LockPool capacity 192→240. v0x05 snapshots
// are migrated in-place: each step's diverged intrinsic byte becomes a
// pool entry; values matching the track default come back unlocked.
// v0x04 (Elkhart v4.3): adds a per-lock PROB pool serialized after the
// LockPool. v0x03 loads zero-fill the prob pool. v0x01 / v0x02 reach this
// via MigrationV41 — neither carries a prob pool either.
static const uint8_t kVersion = 0x06;
// v0x05 on-disk sizes (pre-shrink layout): SeqStep was 7 B, LockPool capacity 192.
static const uint8_t kV05SeqStepSize = 7;
static const uint16_t kV05LockPoolEntries = 192;
static const uint16_t kV05LockPoolRawSize = kV05LockPoolEntries * 3;  // = 576

// Persistent prefix = everything before shadow[]. offsetof is bulletproof
// against any compiler-side struct alignment changes.
static const uint16_t kTrackPersistentSize = offsetof(SeqTrack, shadow);

/* static */
void Snapshot::BuildPath(char* out, uint8_t slot) {
  // "/SNAPSHOT/NN.SNP"
  out[0] = '/';
  out[1] = 'S'; out[2] = 'N'; out[3] = 'A'; out[4] = 'P';
  out[5] = 'S'; out[6] = 'H'; out[7] = 'O'; out[8] = 'T';
  out[9] = '/';
  out[10] = '0' + (slot / 10);
  out[11] = '0' + (slot % 10);
  out[12] = '.';
  out[13] = 'S'; out[14] = 'N'; out[15] = 'P';
  out[16] = '\0';
}

/* static */
uint8_t Snapshot::SlotOccupied(uint8_t slot) {
  scoped_resource<SdCardSession> session;
  Storage::file_.Close();
  Storage::InvalidatePendingSysExTransfer();

  char path[20];
  BuildPath(path, slot);
  FilesystemStatus s = Storage::file_.Open(
      path, FA_READ | FA_OPEN_EXISTING, kFsInitTimeout);
  Storage::file_.Close();
  return (s == FS_OK) ? 1 : 0;
}

/* static */
FilesystemStatus Snapshot::Save(uint8_t slot) {
  // v4.4 layout guards. Any drift here breaks the save/load round-trip.
  STATIC_ASSERT(sizeof(SeqStep) == 2);
  STATIC_ASSERT(sizeof(LockEntry) == 3);
  STATIC_ASSERT(LockPool::kRawEntriesSize == 720);
  STATIC_ASSERT(sizeof(LockProbEntry) == 3);
  STATIC_ASSERT(LockProbPool::kRawEntriesSize == 96);
  STATIC_ASSERT(offsetof(SeqTrack, pattern)  == 16);
  STATIC_ASSERT(offsetof(SeqTrack, defaults) == 23);
  STATIC_ASSERT(offsetof(SeqTrack, config)   == 51);
  STATIC_ASSERT(offsetof(SeqTrack, shadow)   == 82);
  STATIC_ASSERT(sizeof(MultiData) == 61);

  // Stop transport so no step-fires queue voicecard SPI traffic during the
  // SD session. BeginSdCard's FlushBuffers waits for already-queued bytes
  // (the Release writes from Stop) to drain before taking the bus.
  sequencer.Stop();

  scoped_resource<SdCardSession> session;
  Storage::file_.Close();
  Storage::InvalidatePendingSysExTransfer();

  char path[20];
  BuildPath(path, slot);

  FilesystemStatus s = Storage::file_.Open(
      path, FA_WRITE | FA_CREATE_ALWAYS, kFsInitTimeout);
  if (s == FS_PATH_NOT_FOUND) {
    Storage::fs_.Mkdirs(path);
    s = Storage::file_.Open(
        path, FA_WRITE | FA_CREATE_ALWAYS, kFsInitTimeout);
  }
  if (s != FS_OK) {
    return s;
  }

  uint8_t checksum = 0;
  uint16_t written;
  uint8_t header[5] = { kMagic[0], kMagic[1], kMagic[2], kMagic[3], kVersion };
  if (Storage::file_.Write(header, 5, &written) != FS_OK || written != 5) {
    Storage::file_.Close();
    return FS_DISK_ERROR;
  }
  for (uint8_t i = 0; i < 5; ++i) checksum += header[i];

  for (uint8_t t = 0; t < kNumVoices; ++t) {
    const uint8_t* tp = reinterpret_cast<const uint8_t*>(sequencer.mutable_track(t));
    if (Storage::file_.Write(tp, kTrackPersistentSize, &written) != FS_OK
        || written != kTrackPersistentSize) {
      Storage::file_.Close();
      return FS_DISK_ERROR;
    }
    for (uint16_t i = 0; i < kTrackPersistentSize; ++i) checksum += tp[i];
  }

  // Lock pool: count byte + entries blob (issue #30 v4.2).
  {
    const LockPool& pool = sequencer.lock_pool();
    uint8_t pool_count = pool.count();
    if (Storage::file_.Write(&pool_count, 1, &written) != FS_OK || written != 1) {
      Storage::file_.Close();
      return FS_DISK_ERROR;
    }
    checksum += pool_count;
    const uint8_t* pe = pool.raw_entries();
    if (Storage::file_.Write(pe, LockPool::kRawEntriesSize, &written) != FS_OK
        || written != LockPool::kRawEntriesSize) {
      Storage::file_.Close();
      return FS_DISK_ERROR;
    }
    for (uint16_t i = 0; i < LockPool::kRawEntriesSize; ++i) checksum += pe[i];
  }

  // Per-lock PROB pool (v0x04, #38 slim): count + entries blob.
  {
    const LockProbPool& pool = sequencer.lock_prob_pool();
    uint8_t pool_count = pool.count();
    if (Storage::file_.Write(&pool_count, 1, &written) != FS_OK || written != 1) {
      Storage::file_.Close();
      return FS_DISK_ERROR;
    }
    checksum += pool_count;
    const uint8_t* pe = pool.raw_entries();
    if (Storage::file_.Write(pe, LockProbPool::kRawEntriesSize, &written) != FS_OK
        || written != LockProbPool::kRawEntriesSize) {
      Storage::file_.Close();
      return FS_DISK_ERROR;
    }
    for (uint16_t i = 0; i < LockProbPool::kRawEntriesSize; ++i) checksum += pe[i];
  }

  const uint8_t* mp = multi.raw_data();
  if (Storage::file_.Write(mp, sizeof(MultiData), &written) != FS_OK
      || written != sizeof(MultiData)) {
    Storage::file_.Close();
    return FS_DISK_ERROR;
  }
  for (uint8_t i = 0; i < sizeof(MultiData); ++i) checksum += mp[i];

  if (Storage::file_.Write(&checksum, 1, &written) != FS_OK || written != 1) {
    Storage::file_.Close();
    return FS_DISK_ERROR;
  }

  Storage::file_.Close();
  return FS_OK;
}

/* static */
FilesystemStatus Snapshot::Load(uint8_t slot) {
  // Voicecards and the SD card share one SPI bus, arbitrated by
  // SdCardSession. Any voicecard-bound write queued while the session is
  // active races the SD transfer through the Timer2 ISR
  // (voicecard_tx.SendBytes) and corrupts both. Keep the SD session
  // strictly bracketed around file I/O; sequencer.Reset() and multi.Touch()
  // (both queue voicecard writes) run after it ends.
  // Stop transport and kill voices before the SD session. Stops step-fires
  // from queueing voicecard SPI writes that would race the SD transfer, and
  // hard-mutes audio so envelopes from the prior patch don't bleed into the
  // loaded one. User must press Play to resume.
  sequencer.Panic();

  FilesystemStatus result = FS_OK;
  uint8_t snapshot_version = kVersion;  // populated inside the SD block
  {
    scoped_resource<SdCardSession> session;
    Storage::file_.Close();
    Storage::InvalidatePendingSysExTransfer();

    char path[20];
    BuildPath(path, slot);

    FilesystemStatus s = Storage::file_.Open(
        path, FA_READ | FA_OPEN_EXISTING, kFsInitTimeout);
    if (s != FS_OK) {
      return s;
    }

    uint8_t checksum = 0;
    uint16_t got;
    uint8_t header[5];
    if (Storage::file_.Read(header, 5, &got) != FS_OK || got != 5) {
      Storage::file_.Close();
      return FS_DISK_ERROR;
    }
    if (header[0] != kMagic[0] || header[1] != kMagic[1] ||
        header[2] != kMagic[2] || header[3] != kMagic[3]) {
      Storage::file_.Close();
      return FS_DISK_ERROR;
    }
    // v0x06 = native v4.4 (SeqStep shrunk to 2 B, pool capacity 240).
    // v0x05/v0x04/v0x03 = old layout (7-byte SeqStep, pool capacity 192).
    // v0x01 / v0x02 = v4.1-era dense-lock format (migration TU).
    if (header[4] != kVersion && header[4] != 0x05 && header[4] != 0x04 &&
        header[4] != 0x03 && header[4] != 0x02 && header[4] != 0x01) {
      Storage::file_.Close();
      return FS_DISK_ERROR;
    }
    snapshot_version = header[4];
    for (uint8_t i = 0; i < 5; ++i) checksum += header[i];

    if (snapshot_version == 0x01 || snapshot_version == 0x02) {
      // v4.1-era file: parse 337-byte SeqTrack blobs through migration TU.
      if (!MigrationV41::LoadAllTracks(&checksum)) {
        Storage::file_.Close();
        return FS_DISK_ERROR;
      }
    } else if (snapshot_version >= kVersion) {
      // Native v0x06 layout.
      for (uint8_t t = 0; t < kNumVoices; ++t) {
        uint8_t* tp = reinterpret_cast<uint8_t*>(sequencer.mutable_track(t));
        if (Storage::file_.Read(tp, kTrackPersistentSize, &got) != FS_OK
            || got != kTrackPersistentSize) {
          Storage::file_.Close();
          return FS_DISK_ERROR;
        }
        for (uint16_t i = 0; i < kTrackPersistentSize; ++i) checksum += tp[i];
      }
      uint8_t pool_count;
      if (Storage::file_.Read(&pool_count, 1, &got) != FS_OK || got != 1) {
        Storage::file_.Close();
        return FS_DISK_ERROR;
      }
      checksum += pool_count;
      uint8_t* pe = sequencer.mutable_lock_pool().mutable_raw_entries();
      if (Storage::file_.Read(pe, LockPool::kRawEntriesSize, &got) != FS_OK
          || got != LockPool::kRawEntriesSize) {
        Storage::file_.Close();
        return FS_DISK_ERROR;
      }
      for (uint16_t i = 0; i < LockPool::kRawEntriesSize; ++i) checksum += pe[i];
      sequencer.mutable_lock_pool().set_count(pool_count);

      uint8_t prob_count;
      if (Storage::file_.Read(&prob_count, 1, &got) != FS_OK || got != 1) {
        Storage::file_.Close();
        return FS_DISK_ERROR;
      }
      checksum += prob_count;
      uint8_t* qe = sequencer.mutable_lock_prob_pool().mutable_raw_entries();
      if (Storage::file_.Read(qe, LockProbPool::kRawEntriesSize, &got) != FS_OK
          || got != LockProbPool::kRawEntriesSize) {
        Storage::file_.Close();
        return FS_DISK_ERROR;
      }
      for (uint16_t i = 0; i < LockProbPool::kRawEntriesSize; ++i) checksum += qe[i];
      sequencer.mutable_lock_prob_pool().set_count(prob_count);
    } else {
      // v0x03 / v0x04 / v0x05 migration: SeqStep was 7 B with intrinsic
      // per-step storage. Read each track into a scratch buffer, transcribe
      // pattern/defaults/config into the new SeqTrack, stash each step's
      // intrinsic bytes in a side table, then — after the serialized
      // LockPool has been loaded — synthesize pool entries for any
      // intrinsic byte that diverges from the new track default. Values
      // matching the default come back as unlocked.
      const uint16_t kOldStepArea     = kV05SeqStepSize * 8;  // 56
      const uint16_t kOldPatternArea  = 7;
      const uint16_t kOldDefaultsArea = 28;
      const uint16_t kOldConfigArea   = (snapshot_version < 0x05) ? 29 : 31;
      const uint16_t kOldPersistentSize = kOldStepArea + kOldPatternArea +
                                          kOldDefaultsArea + kOldConfigArea;
      uint8_t buf[122];  // max old persistent size (v0x05 = 56+7+28+31)
      // Stash 5 intrinsic bytes per step per track: note, vel, prob, subs, glid.
      uint8_t old_intrinsics[kNumVoices][8][5];

      sequencer.mutable_lock_prob_pool().Init();

      for (uint8_t t = 0; t < kNumVoices; ++t) {
        if (Storage::file_.Read(buf, kOldPersistentSize, &got) != FS_OK
            || got != kOldPersistentSize) {
          Storage::file_.Close();
          return FS_DISK_ERROR;
        }
        for (uint16_t i = 0; i < kOldPersistentSize; ++i) checksum += buf[i];

        SeqTrack* dst = sequencer.mutable_track(t);
        memcpy(dst->pattern, &buf[kOldStepArea], kOldPatternArea);
        memcpy(dst->defaults,
               &buf[kOldStepArea + kOldPatternArea],
               kOldDefaultsArea);
        memcpy(dst->config,
               &buf[kOldStepArea + kOldPatternArea + kOldDefaultsArea],
               kOldConfigArea);
        for (uint8_t i = kOldConfigArea; i < kCfgSIZE; ++i) dst->config[i] = 0;

        for (uint8_t s = 0; s < 8; ++s) {
          const uint8_t* old_step = &buf[s * kV05SeqStepSize];
          // Old SeqStep layout: note, vel, prob, subs, glid, step_flags, substep_bits.
          old_intrinsics[t][s][0] = old_step[0];  // note
          old_intrinsics[t][s][1] = old_step[1];  // vel
          old_intrinsics[t][s][2] = old_step[2];  // prob
          old_intrinsics[t][s][3] = old_step[3];  // subs (packed nibbles)
          old_intrinsics[t][s][4] = old_step[4];  // glid
          dst->steps[s].step_flags   = old_step[5];
          dst->steps[s].substep_bits = old_step[6];
        }
      }

      // v0x05 serialized LockPool: 192 entries × 3 = 576 B preceded by a
      // count byte. Read directly into the front of the new pool buffer,
      // mark the tail slots [192..240) as free, then set_count.
      uint8_t pool_count;
      if (Storage::file_.Read(&pool_count, 1, &got) != FS_OK || got != 1) {
        Storage::file_.Close();
        return FS_DISK_ERROR;
      }
      checksum += pool_count;
      uint8_t* pe = sequencer.mutable_lock_pool().mutable_raw_entries();
      if (Storage::file_.Read(pe, kV05LockPoolRawSize, &got) != FS_OK
          || got != kV05LockPoolRawSize) {
        Storage::file_.Close();
        return FS_DISK_ERROR;
      }
      for (uint16_t i = 0; i < kV05LockPoolRawSize; ++i) checksum += pe[i];
      for (uint16_t i = kV05LockPoolEntries;
           i < LockPool::kRawEntriesSize / 3; ++i) {
        // LockEntry layout: ts, param, value. Mark param = kLockPoolFree.
        pe[i * 3 + 0] = 0;
        pe[i * 3 + 1] = kLockPoolFree;
        pe[i * 3 + 2] = 0;
      }
      sequencer.mutable_lock_pool().set_count(pool_count);

      // Per-lock PROB pool (only v0x04+).
      if (snapshot_version >= 0x04) {
        uint8_t prob_count;
        if (Storage::file_.Read(&prob_count, 1, &got) != FS_OK || got != 1) {
          Storage::file_.Close();
          return FS_DISK_ERROR;
        }
        checksum += prob_count;
        uint8_t* qe = sequencer.mutable_lock_prob_pool().mutable_raw_entries();
        if (Storage::file_.Read(qe, LockProbPool::kRawEntriesSize, &got) != FS_OK
            || got != LockProbPool::kRawEntriesSize) {
          Storage::file_.Close();
          return FS_DISK_ERROR;
        }
        for (uint16_t i = 0; i < LockProbPool::kRawEntriesSize; ++i) checksum += qe[i];
        sequencer.mutable_lock_prob_pool().set_count(prob_count);
      }

      // Synthesize intrinsic pool entries into the free tail slots. Each
      // diverged byte becomes one lock; on pool-full SetStepLock silently
      // drops the overflow.
      for (uint8_t t = 0; t < kNumVoices; ++t) {
        SeqTrack* dst = sequencer.mutable_track(t);
        uint8_t def_note = dst->defaults[0];
        uint8_t def_prob = dst->defaults[16 + 0];
        uint8_t def_ssub = dst->defaults[16 + 1];
        uint8_t def_rept = dst->defaults[16 + 2];
        uint8_t def_vel  = dst->defaults[16 + 4];
        uint8_t def_glid = dst->defaults[16 + 5];
        for (uint8_t s = 0; s < 8; ++s) {
          uint8_t old_note = old_intrinsics[t][s][0];
          uint8_t old_vel  = old_intrinsics[t][s][1];
          uint8_t old_prob = old_intrinsics[t][s][2];
          uint8_t old_subs = old_intrinsics[t][s][3];
          uint8_t old_glid = old_intrinsics[t][s][4];

          if (old_note != def_note) sequencer.SetStepLock(t, s, 0,  old_note);
          if (old_prob != def_prob) sequencer.SetStepLock(t, s, 16, old_prob);
          if (old_vel  != def_vel)  sequencer.SetStepLock(t, s, 20, old_vel);
          if (old_glid != def_glid) sequencer.SetStepLock(t, s, 21, old_glid);
          int8_t step_ssub = static_cast<int8_t>(old_subs & 0x0f) - 2;
          int8_t def_ssub_s = static_cast<int8_t>(def_ssub);
          if (step_ssub != def_ssub_s) {
            sequencer.SetStepLock(t, s, 17, static_cast<uint8_t>(step_ssub));
          }
          uint8_t step_rept = (old_subs >> 4) & 0x0f;
          if (step_rept != (def_rept & 0x0f)) {
            sequencer.SetStepLock(t, s, 18, step_rept);
          }
        }
      }
    }

    uint8_t* mp = multi.mutable_raw_data();
    // v0x01 had a 5-byte MultiData (no MIDI fields). Read that much,
    // zero the rest, and we'll fill MIDI defaults after the SD session.
    const uint16_t kMultiDataV1Size = 5;
    uint16_t multi_read_size = (snapshot_version == 0x01)
        ? kMultiDataV1Size : sizeof(MultiData);
    if (Storage::file_.Read(mp, multi_read_size, &got) != FS_OK
        || got != multi_read_size) {
      Storage::file_.Close();
      return FS_DISK_ERROR;
    }
    for (uint16_t i = 0; i < multi_read_size; ++i) checksum += mp[i];
    if (snapshot_version == 0x01) {
      for (uint16_t i = kMultiDataV1Size; i < sizeof(MultiData); ++i) mp[i] = 0;
    }

    uint8_t expected;
    if (Storage::file_.Read(&expected, 1, &got) != FS_OK || got != 1) {
      Storage::file_.Close();
      return FS_DISK_ERROR;
    }
    Storage::file_.Close();

    if (expected != checksum) {
      result = FS_DISK_ERROR;
    }
  }  // SD session ends here; SPI bus restored for voicecard traffic.

  if (result != FS_OK) {
    return result;
  }

  if (snapshot_version == 0x01) {
    // Populate v2 MIDI defaults for snapshots that pre-date the schema.
    // Mirrors the static init_settings in multi.cc.
    MultiData* d = multi.mutable_data();
    for (uint8_t i = 0; i < kNumVoices; ++i) {
      d->midi_channel[i] = i + 1;
      for (uint8_t c = 0; c < 8; ++c) {
        d->midi_cc_map[i][c] = 0xff;  // off / unassigned
      }
    }
    d->midi_only_mask = 0;
    d->midi_clock_mode = 2;  // OUT
  }

  // Discard transient playhead state and re-sync transport.
  sequencer.Reset();
  // Recompute BPM tick duration and re-push all voice params to voicecards.
  multi.Touch();
  // After the patch is pushed, kill voices one more time so any audio
  // transient from voicecard parameter changes (filter resets, env state
  // re-evaluation) doesn't leak as a click. User must press a key / hit
  // Play to hear the new patch.
  for (uint8_t v = 0; v < kNumVoices; ++v) {
    voicecard_tx.Kill(v);
  }
  return FS_OK;
}

}  // namespace ambika
