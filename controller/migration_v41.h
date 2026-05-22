// v4.1 → v4.2 snapshot migration (issue #30).
//
// Translates dense-lock v4.1 SeqTrack blobs (337 B/track, lock_flags +
// pageN[] storage) into the new intrinsic-SeqStep + LockPool layout. Lives
// in its own translation unit so it can be deleted cleanly in a future
// release once the user base has rolled forward.
//
// Consumed by snapshot.cc's Load path when the on-disk version byte is 0x02.

#ifndef CONTROLLER_MIGRATION_V41_H_
#define CONTROLLER_MIGRATION_V41_H_

#include "avrlib/base.h"

namespace ambika {

class MigrationV41 {
 public:
  // Stream-read 6 v4.1 SeqTrack blobs (337 B each) from Storage::file_,
  // accumulate every byte into *checksum, and populate sequencer.tracks_ +
  // sequencer.lock_pool_ in the new layout. Returns 1 on success, 0 on
  // disk error. Pool overflow during migration is non-fatal: extra locks
  // are dropped silently (caller may flash POOL FULL after).
  static uint8_t LoadAllTracks(uint8_t* checksum);
};

}  // namespace ambika

#endif  // CONTROLLER_MIGRATION_V41_H_
