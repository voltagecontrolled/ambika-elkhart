# Elkhart Changelog

Fork of YAM (`bjoeri/ambika`) targeting a 6-voice polymetric percussive step sequencer
on the Michigan Synth Works Xena motherboard (ATmega644p) + SVF voicecards (ATmega328p).
Build requires avr-gcc 4.3.5 via `./build-squeeze.sh` from the repo root.

---

## [Unreleased]

### Phase 2 — RAM gutting and MIDI routing (2026-05-01)

**Commit:** `e22c4c2`

**Motivation:** Baseline controller had only 241 B of free RAM (3,855/4,096 used, 94%).
No sequencer code could land without first clearing the overhead from YAM's voice
allocator, arpeggiator, groove table, and dead UI pages.

**RAM result:** 2,393 B used, 1,703 B free (was 241 B free).
**Flash result:** 41,136 B (63% of 64 KB), down from 59,370 B (91%).

#### controller/part.h / part.cc — stripped Part class

- `PartData` reduced from 84 B to 16 B: removed `sequence_data[64]` and `padding[4]`
- `Part` class stripped to `Patch` (112 B) + `PartData` (16 B) + `voice_id_` + `flags_`
- Removed: `NoteStack`, `VoiceAllocator`, LFO state, arp/sequencer state
- `Init(uint8_t voice_id)` — new signature; no longer queries voice allocator
- `NoteOn`: applies octave + tuning from `PartData`, then calls `voicecard_tx.Trigger()`
  directly (note encoding: `midi_note << 7`, matching `VoicecardProtocolTx::Trigger`)
- `NoteOff`: calls `voicecard_tx.Release(voice_id_)`
- `lfo_value()` stub returns 0 (LFO refresh runs on voicecard, not controller)
- Kept all enum types (`ArpeggiatorDirection`, `ArpSequencerMode`, `PolyphonyMode`,
  `PartParameter`) for `parameter.cc` / resources compat; offsets 8–15 of `PartData`
  still correspond to those parameter definitions

#### controller/multi.h / multi.cc — stripped Multi class

- `MultiData` reduced from 56 B to 4 B: only clock params (`bpm`, `groove_template`,
  `groove_amount`, `latch`)
- `PRM_MULTI_CLOCK_BPM` moved from offset 24 → 0 to match new struct layout
- Removed: `PartMapping`, `KnobAssignment`, `tick_duration_table_[16]` (32 B),
  `lfo_refresh_counter_`, `lfo_refresh_cycle_`, `idle_ticks_`
- Groove table replaced with single `ComputeTickDuration()` (integer formula)
- `NoteOn`: hardwired MIDI ch 0–5 → voices 0–5; ch 9 drum map: notes 36–41 →
  voices 0–5 at middle C (MIDI note 60)
- `OmniModeOff/On`, `MonoModeOn`, `PolyModeOn` are no-op stubs

#### controller/midi_dispatcher.h — ProgramChange

- Removed `#include "controller/ui_pages/library.h"` (file deleted)
- `ProgramChange`: replaced Library-based handler with direct `StorageLocation` load
  (bank = `current_bank_`, slot = program number, for channel < kNumParts)

#### controller/storage.cc — sequence data guards

- `object_size(STORAGE_OBJECT_SEQUENCE)` returns 0
- `object_data` / `mutable_object_data` for SEQUENCE return NULL
- `RIFFWriteObject`: guarded with `if (size > 0 && data)` before writing
- `TouchObject`: SEQUENCE case is a no-op

#### controller/ui.cc — dead page removal

- Removed includes: `knob_assigner`, `library`, `performance_page`,
  `sequence_editor`, `version_manager`, `voice_assigner`
- `PAGE_MULTI`: placeholder (all `0xff` params); voice assignment removed
- `PAGE_MULTI_CLOCK`: active clock params (offsets 62–65); links to itself
- `PAGE_PART_SEQUENCER`, `PAGE_PERFORMANCE`, `PAGE_KNOB_ASSIGN`: `0xff` stubs
- `PAGE_LIBRARY`, `PAGE_VERSION_MANAGER`: routed to `OsInfoPage` handlers
- `default_most_recent_page_in_group`: group 5 → `PAGE_MULTI_CLOCK`,
  group 7 → `PAGE_OS_INFO`

#### controller/ui_pages/parameter_editor.cc — knob assignment removal

- Removed `knob_assignment[]` branches from `parameter_index()`, `part_index()`,
  `instance_index()` (dead code: no live page injects `0xf0`–`0xf7` parameter IDs)

#### Deleted files

- `controller/ui_pages/sequence_editor.cc/.h`
- `controller/ui_pages/library.cc/.h`
- `controller/ui_pages/knob_assigner.cc/.h`
- `controller/ui_pages/voice_assigner.cc/.h`
- `controller/ui_pages/performance_page.cc/.h`
- `controller/ui_pages/version_manager.cc/.h`

#### common/patch.h — build fix

- Added `#include <avr/pgmspace.h>` — `patch.h` uses `PROGMEM` in its typedef but
  relied on transitive inclusion from headers we removed. Now self-contained.

---

### Earlier work (doc and spec commits, pre-Phase 2)

- `69981fc` — Fix Windows path separator in `controller/resources.cc` include
- `9338bf6` — Add MIDI input routing spec (ch1–6 dedicated, ch10 drum map)
- `2c7b4f6` — Add CGRAM transport glyph bitmaps to Page 6 spec
- `2a35043` — Remap normal-mode pages to match hardware button labels
- `a6bc0f5` — Reintroduce LPG-coupled envelope macro on Voice Page 2
- `af654d9` — Move wiki out of planning folder
- `1169169` — Add elkhart planning docs and YAM-based v2 spec
