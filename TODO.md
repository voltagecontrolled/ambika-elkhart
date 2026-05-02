# Elkhart TODO

See `docs/planning/SPEC_v2.md` for the authoritative feature spec and parameter definitions.

---

## Active / next up

### Phase 3 — Sequencer foundation

RAM headroom after Phase 2: **1,703 B free** (4,096 B total, 2,393 B used).

- [ ] Design `SeqStep` struct: per-step note, velocity, param locks (compact; target ≤ 8 B/step × 8 steps × 6 voices = 384 B)
- [ ] Add `SeqPattern` (8 steps, length field) to controller state
- [ ] Drive step advancement from `Multi::Clock()` at `kNumTicksPerStep` (currently 6 ticks)
- [ ] `Multi::Start()`/`Stop()` reset step counters correctly — already in place
- [ ] Per-voice step length (polymetric): each voice has its own `step_count_` and wraps at its own pattern length
- [ ] Wire step note + velocity into `Part::NoteOn()` on clock tick
- [ ] Apply param locks at step trigger time; restore voice defaults after step fires (stateless step invariant)
- [ ] EEPROM/SD persistence for patterns

### Phase 3 — Sequencer UI page

- [ ] New `controller/ui_pages/sequencer.cc/.h` replacing the `PAGE_PART_SEQUENCER` stub
- [ ] Step display on 2×40 LCD: active step indicator, lock state per-step
- [ ] Encoder scrolls through steps; click toggles step on/off
- [ ] Knobs set note/velocity for active step
- [ ] Shift+knob: param lock for that step

---

## Voicecard work

### CZ oscillator strip (Phase 2 wave palette)

- [ ] Remove 9 CZ phase-distortion filter-sim variants from voicecard:
  `cz_saw_lp`, `cz_saw_pk`, `cz_saw_bp`, `cz_saw_hp`,
  `cz_pls_lp`, `cz_pls_pk`, `cz_pls_bp`, `cz_pls_hp`, `cz_tri_lp`
- [ ] Reclaim freed flash for wavefolder shape (deferred until sequencer lands)
- [ ] Update `common/patch.h` `OscillatorAlgorithm` enum — remove those 9 constants,
  renumber remaining entries, verify resource table and parameter display strings match

### LPG-coupled envelope macro

- [ ] Implement `LPG` macro on voicecard: derives `shape_fast`/`shape_slow` for both
  FG-A (VCA env) and FG-B (filter env) from single bipolar byte (see `SPEC_v2.md`)
- [ ] CW: vactrol-style character sweep; CCW: adds progressive attack slope
- [ ] Two-slope decay model: `tail_mix` blends fast/slow decay curves
- [ ] Compute derived shape values once on trigger; use PROGMEM lookup table in audio loop
- [ ] Filter tail attenuated relative to VCA tail (~0.6×) — highs die before body

### Pitch envelope (Env 3)

- [ ] Env 3 routes to Osc 1 base pitch (not modulation routing table) — hardwired
- [ ] `PITA` (pitch attack/depth amount) and `PITD` (pitch decay time) on Voice Page 2
- [ ] Verify existing YAM env 3 infrastructure on voicecard; determine if new routing
  code needed or if modulation table entry covers it

### Filter floor (FFLR)

- [ ] Add non-lockable `FFLR` parameter: minimum cutoff FG-B cannot close below
- [ ] Relevant for comb filter resonator ring brightness

---

## Pending hardware validation

- [ ] **Filter mode switching**: test whether LP/BP/HP switching mid-note clicks on SVF
  hardware. If it does, `TYPE` moves to a non-lockable voice setting page.
- [ ] **PolyBLEP saw vs. old_saw**: listening comparison on hardware — confirm
  `old_saw` is worth keeping in palette
- [ ] **Voicecard note trigger** (`note << 7` encoding): confirm correct pitch with
  elkhart controller + elkhart voicecard end-to-end (not yet tested)
- [ ] **MIDI ch1–6 routing**: smoke test that notes on each channel trigger the correct
  voice independently

---

## Known issues / technical debt

### Build hygiene

- `common/patch.h` was missing `#include <avr/pgmspace.h>` — fixed in Phase 2
  (`e22c4c2`). Root cause: relied on transitive include from headers stripped in Phase 2.
  Watch for this pattern in other headers if more includes are removed.

- `controller/parameter.cc` was missing `#include "avrlib/random.h"` — fixed in
  Phase 2. `Random::GetByte()` was reachable only via transitive include from
  old `part.h` → `common/lfo.h` etc.

- `midi::kAllSoundOff` and `midi::kAllNotesOff` do not exist as constants in
  `midi/midi.h` — replaced with raw CC values `0x78` (120) and `0x7b` (123) in
  `controller/part.cc`. If the midi library is updated, re-evaluate whether named
  constants should be added there.

### Voice allocator stub

- `controller/voice_allocator.cc` still compiles and links (it's in the PACKAGES list).
  It is currently dead code — nothing calls it — but it contributes ~N bytes of flash.
  Evaluate removing it from PACKAGES once no lingering references exist.

### PAGE_MULTI placeholder

- `PAGE_MULTI` in `ui.cc` is a fully-dead stub (all `0xff` params). It exists to
  maintain the page-group numbering expected by the switch/button dispatch. If the
  sequencer page absorbs group 4, this entry can be removed.

### Legacy PartParameter enum values

- `PartParameter` enum (offsets 49–57 in the combined Patch+PartData address space)
  still refers to arp/sequencer fields. These offsets are valid in the current 16-byte
  `PartData` struct. If `PartData` is ever reorganized, the enum and `resources.cc`
  parameter display table must be updated in lockstep.

### BPCH hardcoded to middle C

- Ch 10 drum map (`multi.cc`: `parts_[note - 36].NoteOn(60, velocity)`) uses middle C
  for all drum hits. A future enhancement could make the base pitch configurable per
  voice or derived from the voice's stored `NOTE` parameter.
