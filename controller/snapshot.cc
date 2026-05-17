#include "controller/snapshot.h"

#include <stddef.h>
#include <string.h>

#include "avrlib/op.h"
#include "avrlib/resources_manager.h"

#include "controller/multi.h"
#include "controller/sequencer.h"
#include "controller/storage.h"

namespace ambika {

using namespace avrlib;

static const char kMagic[4] = { 'E', 'L', 'K', 'S' };
static const uint8_t kVersion = 0x02;
// v1 layout had a 5-byte MultiData (bpm, groove_template, groove_amount,
// clock_latch, master_reset_steps). v2 appends MIDI per-track config. Load
// path falls back to v1 reads + defaults if the file is older.
static const uint16_t kMultiDataV1Size = 5;

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
  STATIC_ASSERT(sizeof(SeqStep) == 34);
  STATIC_ASSERT(offsetof(SeqTrack, pattern)  == 272);
  STATIC_ASSERT(offsetof(SeqTrack, defaults) == 280);
  STATIC_ASSERT(offsetof(SeqTrack, config)   == 308);
  STATIC_ASSERT(offsetof(SeqTrack, shadow)   == 337);
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
    // Accept v1 and v2. v1 has a 5-byte MultiData; v2 appends MIDI fields.
    if (header[4] != kVersion && header[4] != 0x01) {
      Storage::file_.Close();
      return FS_DISK_ERROR;
    }
    snapshot_version = header[4];
    for (uint8_t i = 0; i < 5; ++i) checksum += header[i];

    for (uint8_t t = 0; t < kNumVoices; ++t) {
      uint8_t* tp = reinterpret_cast<uint8_t*>(sequencer.mutable_track(t));
      if (Storage::file_.Read(tp, kTrackPersistentSize, &got) != FS_OK
          || got != kTrackPersistentSize) {
        Storage::file_.Close();
        return FS_DISK_ERROR;
      }
      for (uint16_t i = 0; i < kTrackPersistentSize; ++i) checksum += tp[i];
    }

    uint8_t* mp = multi.mutable_raw_data();
    uint16_t multi_read_size = (snapshot_version == 0x01)
        ? kMultiDataV1Size : sizeof(MultiData);
    if (Storage::file_.Read(mp, multi_read_size, &got) != FS_OK
        || got != multi_read_size) {
      Storage::file_.Close();
      return FS_DISK_ERROR;
    }
    for (uint16_t i = 0; i < multi_read_size; ++i) checksum += mp[i];
    if (snapshot_version == 0x01) {
      // v1 file: zero the appended MIDI region; populate defaults below if
      // checksum validates. v2 defaults come from InitSettings on first re-save.
      uint8_t* tail = mp + kMultiDataV1Size;
      for (uint16_t i = 0; i < sizeof(MultiData) - kMultiDataV1Size; ++i) {
        tail[i] = 0;
      }
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
  return FS_OK;
}

}  // namespace ambika
