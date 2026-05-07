// elkhart: full-state snapshot to SD card.
//
// Persists all 6 SeqTracks (steps + pattern + defaults + config; shadow
// is excluded) plus MultiData. Bypasses the broken Storage::Save() RIFF
// path and writes a fixed-layout flat file per slot.

#ifndef CONTROLLER_SNAPSHOT_H_
#define CONTROLLER_SNAPSHOT_H_

#include "avrlib/filesystem/filesystem.h"

namespace ambika {

using avrlib::FilesystemStatus;

class Snapshot {
 public:
  static const uint8_t kNumSlots = 64;

  static FilesystemStatus Save(uint8_t slot);
  static FilesystemStatus Load(uint8_t slot);
  static uint8_t SlotOccupied(uint8_t slot);

 private:
  static void BuildPath(char* out, uint8_t slot);
};

}  // namespace ambika

#endif  // CONTROLLER_SNAPSHOT_H_
