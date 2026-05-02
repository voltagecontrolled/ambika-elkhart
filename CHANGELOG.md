# Elkhart Changelog

Fork of YAM (`bjoeri/ambika`) targeting a 6-voice polymetric percussive step sequencer
on the Michigan Synth Works Xena motherboard (ATmega644p) + SVF voicecards (ATmega328p).
Build requires avr-gcc 4.3.5 via `./build-squeeze.sh` from the repo root.

---

## [Unreleased]

### Phase 5 — Voice Parameter System & Envelope Redesign (2026-05-02)

**Flash result:** 43,224B (66.0% of 64KB), up ~1,810B from Phase 4.
**RAM:** 3,462B used, 634B free. kSystemVersion = 0x20 on both controller and voicecard.

**Architectural pivot:** LPG macro (LPGD/LPGA/LPGO) from SPEC_v2 was not implemented. Instead: three independent ADR+Curve envelopes with fixed routing, LFO1/2/3 removed from controller side (LFO4 voice-side only), `patch_mod[]` removed from SeqTrack (saves 252B), fixed routing table in PROGMEM replaces the per-track mod matrix shadow.

#### voicecard/envelope.h — ADR+Curve (no sustain)

- `stage_target_[DECAY]` and `stage_target_[SUSTAIN]` both set to 0; decay always falls to zero.
- `curve_` blends linear (`phase_>>8`) with exponential (`wav_res_env_expo`) via `U8Mix` for DECAY and RELEASE stages only. ATTACK stays exponential.
- Sustain byte in voicecard Patch struct repurposed as `curve_`.

#### controller/sequencer.h — SeqTrack restructuring

- `patch_mod[42]` removed (saves 252B across 6 tracks).
- `config[kCfgSIZE=29]`: voice-wide non-lockable settings (filter FREQ/RES/TYPE, osc ranges OSC1R/OSC2R/OSC2D, LFO4 LSHP/LFOS/LFO4D/LFO4A/LFOBYR, env ATK/CRV/DEPT × 3, E2DEPT, misc FUZZ/BITS/WSUB/FMOP/TRAK).
- `defaults[24]`: default lockable params (`page1[8]` + `page2[8]` + `steppage[8]`). SeqStep.page2[] reordered: E1DEC/E1REL/E2DEC/E2REL/E3DEC/E3REL/NOIS/SUB.
- `kP1XXX`, `kP2XXX`, `kCfgXXX` enums added for type-safe indexing.

#### controller/part.cc — Part rewired to SeqTrack

- `PatchAddrToSeqField()`: maps Patch byte offsets to SeqTrack fields. OSC page (0–7), Mixer (8–15), Filter (16–18, 22), EG ATK/DEC/CRV/REL × 3 (24–43), LFO4 (48–49), fixed routing amounts (58, 72, 73, 82), filter tracking (105), virtual EG depth (200–202).
- `GetValue()`: reads SeqTrack field via the map.
- `SetValue()`: translates virtual addresses (200→82, 201→22, 202→58) before voicecard write, then writes SeqTrack field.
- `Touch()`: sends `kDefaultMod[42]` from PROGMEM (fixed routing base), then iterates `kSyncAddresses[]` to push all voice config to voicecard.

#### Fixed mod routing (kDefaultMod PROGMEM)

- Slot 2: ENV3 → OSC_1_2_COARSE (E3DEPT, default 0)
- Slot 7: LFO4 → configurable dest/amount (LFO4D/LFO4A)
- Slot 10: ENV1 → VCA (E1DEPT, default 63 = full)
- Slot 11: VELOCITY → VCA
- Slot 12: PITCHBEND → coarse pitch
- ENV2→VCF: hardcoded in voicecard `voice.cc` `filter_env` path (E2DEPT)

#### controller/resources.h / resources.cc — manually added strings

- STR_RES_EG=382, STR_RES_DEPT=383, STR_RES_DEPTH=384, STR_RES_AMP=385, STR_RES_FLT=386, STR_RES_PCH=387

#### controller/parameter.h / parameter.cc — EG parameter system

- `UNIT_EG_SELECT` added to `Unit` enum. `PrintValue` looks up `STR_RES_AMP + value` → "amp"/"flt"/"pch".
- `kNumParameters` bumped from 75 to 76.
- Param 24 changed from UNIT_INDEX to UNIT_EG_SELECT, short name STR_RES_EG.
- Param 75 (new): EG depth, PARAMETER_LEVEL_PATCH, offset 200, 3 instances, stride 1, indexed by PRM_UI_ACTIVE_ENV_LFO.

#### controller/ui.cc — PAGE_ENV_LFO layout

- Changed from `{ 24, 25, 26, 28, 27, 0xff, 0xff, 0xff }` to `{ 24, 0xff, 0xff, 75, 25, 26, 27, 28 }`.
- Top: EG selector ("amp"/"flt"/"pch"), empty, empty, EG depth.
- Bottom: attack, decay, curve, release.

#### Known issues

- **EEPROM incompatible:** SeqTrack layout changed. Force-reset required on first boot (hold button during power-on) after flashing v2.0.
- **LFO4 dest/amount (params 72/73) not on any UI page yet:** PatchAddrToSeqField maps them but no page displays them.
- **Step parameter locks not yet applied during playback:** lock_flags/page1/page2 fields exist but FireStep() doesn't apply them.

---

### Phase 4 — Transport UI (2026-05-01)

**Flash result:** 41,414B (63% of 64KB), up from 41,414B — +776B for MultiPage.
**RAM:** no change (MultiPage has no static data).

#### controller/ui_pages/multi_page.h + controller/ui_pages/multi_page.cc — new files

- `MultiPage`: custom event handler routed to `PAGE_MULTI`.
- `UpdateScreen`: line 0 shows `bpm NNN | stopped/playing/paused`; line 1 shows `play  paus  rst  | ... exit`.
- `OnKey`: SWITCH_1 → `sequencer.Play()`, SWITCH_2 → `sequencer.Pause()`, SWITCH_3 → `sequencer.Reset()`, SWITCH_8 → `ui.ShowPreviousPage()`. SWITCH_6 falls through to default group navigation (cycles to `PAGE_MULTI_CLOCK`).
- `UpdateLeds`: `LED_STATUS` bright when playing, dim when paused, off when stopped.

#### controller/ui.cc

- `PAGE_MULTI` now routes to `MultiPage::event_handlers_` instead of the all-0xff `ParameterEditor` stub.
- `default_most_recent_page_in_group[5]` changed from `PAGE_MULTI_CLOCK` to `PAGE_MULTI` so pressing SWITCH_6 lands on the transport page by default.

---

### Phase 3 — Sequencer data structures and clock core (2026-05-01)

**RAM result:** 3,436B used, 660B free (was 2,393B / 1,703B free).
**Flash result:** 40,638B (63% of 64KB), down slightly from Phase 2 (41,136B).

#### controller/sequencer.h + controller/sequencer.cc — new files

- `SeqStep` (29B): three parameter pages (page1/page2/steppage, 8B each), 24-bit lock bitfield, step_flags, substep_bits.
- `SeqTrack` (297B): 8 steps (232B) + pattern[8] + defaults[24] + config[20] + shadow[5] + mod[8].
- `SeqGlobal` (32B): transport state, hold_mode, swing, active_track, lock_page, held_step, 4-slot global mod matrix (Phase 9), reserved padding.
- `Sequencer` class: `Init()`, `Clock(ticks)`, `Play()`, `Pause()`, `Reset()`, `AdvanceStep()`, `FireStep()`.
- `Sequencer::Clock(1)` is called on every master tick from `Multi::Clock()`. Tracks advance their per-track tick counters independently, enabling polymetric clock divisions (CDIV). All four direction modes (Fwd/Rev/Pend/Rnd) implemented in `AdvanceStep()`. Pattern rotation (ROTA) applied at fire time.
- Phase 3 `FireStep()`: uses `defaults[kP1NOTE]` and `defaults[16+kSPVEL]` (no lock processing yet). Calls `voicecard_tx.Trigger()`. Lock processing and full snapshot send come in Phase 4.
- `sequencer.Init()` called from `Multi::Init()`.

#### controller/part.h / part.cc — stripped to MIDI routing stub

- `Patch patch_` and `PartData data_` removed from `Part` class (saves 128B × 6 = 768B).
- `Part` is now 2 bytes (`voice_id_` + `flags_`): pure MIDI→voicecard routing layer.
- `NoteOn` calls `voicecard_tx.Trigger()` directly with raw note (no octave/tuning since data_ gone).
- `GetValue()` returns 0; `SetValue()` is a no-op; `TouchPatch()`/`Touch()` are stubs. Parameter editor pages display 0 for voice params until Phase 7 rewires them.
- `raw_patch_data()`/`raw_data()` return `NULL`; storage guards against null before writing.
- `PartData` struct definition retained in `part.h` for `sizeof()` references in `storage.cc`.

#### controller/multi.cc — sequencer wiring

- `sequencer.Init()` called in `Multi::Init()` after parts init.
- `sequencer.Clock(1)` called in `Multi::Clock()` on every master tick.
- Removed `parts_[i].TouchPatch()` and `parts_[i].Touch()` on load — now stubs anyway.

#### controller/storage.cc — stub EEPROM Part data

- `WriteMultiToEeprom()` and `LoadMultiFromEeprom()` now only persist `MultiData` (BPM/clock settings). Previously wrote 6 × (Patch + PartData) blocks which would now dereference NULL. Old EEPROM content checksums as invalid → falls through to `InitSettings(DEFAULT)`, which is correct behavior after this layout change.

#### Known issues introduced or exposed by this phase

- **Parameter editor pages show 0 for voice parameters:** `Part::GetValue()` is a stub returning 0. Pages OSC, FILTER, ENV_LFO, MOD show 0s. Edits via knobs are no-ops. Intentional until Phase 7 rewires these pages to `SeqTrack.config[]` and `SeqTrack.defaults[]`.
- **Voicecards receive no initial patch on boot:** `Part::TouchPatch()` is now a no-op. Voicecards boot with their own defaults. First sequencer trigger will fire but voice timbre is whatever the voicecard defaults to until `TRIGGER_WITH_SNAPSHOT` is implemented in Phase 4.
- **MIDI note input ignores octave/tuning offsets:** `Part::NoteOn()` sends raw note without `PartData.octave` offset (data_ removed). MIDI ch 1–6 triggers are note-accurate but no longer shifted by per-part octave settings.
- **660B RAM free:** Tight but sufficient for Phases 4–8. Mod matrix slots (Phase 9) are pre-allocated in SeqTrack.mod[8] and SeqGlobal.mod[16]. No further large data structures expected before Phase 9.

---

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

#### Known issues introduced or exposed by this phase

- **`common/patch.h` missing pgmspace** (fixed): any header that uses `PROGMEM` in a
  typedef and relies solely on `avrlib/base.h` will break if its transitive include
  chain is shortened. Check for this pattern before stripping more includes.
- **`avrlib/random.h` not in `part.cc`** (fixed): `Random::GetByte()` was only
  reachable via the old `part.h` → `common/lfo.h` transitive chain. Now explicit.
- **`midi::kAllSoundOff` / `midi::kAllNotesOff` don't exist** (fixed): `midi/midi.h`
  has no named constants for CC 120/123. Replaced with raw values `0x78` / `0x7b` in
  `part.cc`. If the MIDI library grows these constants, consolidate.
- **`controller/voice_allocator.cc` is dead code**: still in PACKAGES and links, but
  nothing calls it. Safe to remove from PACKAGES once confirmed no references remain.
- **`PAGE_MULTI` is a fully-dead stub**: exists only to hold group-numbering. Can be
  collapsed once the sequencer page absorbs its slot.
- **Ch 10 base pitch hardcoded to middle C** (`multi.cc:101`): drum map fires all
  voices at MIDI note 60. Should eventually derive from the voice's stored `NOTE`.

---

### Earlier work (doc and spec commits, pre-Phase 2)

- `69981fc` — Fix Windows path separator in `controller/resources.cc` include
- `9338bf6` — Add MIDI input routing spec (ch1–6 dedicated, ch10 drum map)
- `2c7b4f6` — Add CGRAM transport glyph bitmaps to Page 6 spec
- `2a35043` — Remap normal-mode pages to match hardware button labels
- `a6bc0f5` — Reintroduce LPG-coupled envelope macro on Voice Page 2
- `af654d9` — Move wiki out of planning folder
- `1169169` — Add elkhart planning docs and YAM-based v2 spec
