# Elkhart Changelog

Fork of YAM (`bjoeri/ambika`) targeting a 6-voice polymetric percussive step sequencer
on the Michigan Synth Works Xena motherboard (ATmega644p) + SVF voicecards (ATmega328p).
Build requires avr-gcc 4.3.5 via `./build-squeeze.sh` from the repo root.

---

## [Unreleased]

> Going forward, entries are topic-named and dated; the "Phase N" framing
> below is retired. Historical Phase 2–5 entries kept verbatim. Current
> work tracker: `docs/planning/BOARD.md`.

### Sequencer foundation: hardware-pass fixes (2026-05-02)

**Flash result:** controller 45,578 B (69.5%), +366 B over the foundation
entry; voicecard 26,196 B (no functional change). **RAM:** controller
3,469 B (+7 B for `kShdwLAST` shadow byte ×6 tracks plus a few static
fields). `kSystemVersion` bumped to `0x23` on both sides.

Round-2 and round-3 fixes against initial hardware testing of the
foundation. Locks went from "not working" to "working" via the indexing
fix; the rest are layout, scaling, defaults, and UI-routing
corrections.

#### Lock authoring (round 2)

- **Indexing fix in `SeqStepsPage::OnPot`.** `switches_[]` is indexed
  reverse of the `SwitchNumber` enum (SR-index 0 = `SWITCH_8`, 7 =
  `SWITCH_1`, per `Ui::Poll`'s `control = SWITCH_8 - i`). The held-step
  detection was iterating SR-index but treating the result as a step
  index, so locks were authored on the wrong step. Fixed by deriving
  `held_step = 7 - sr_index`. `inhibit_switch_` continues to use SR-index
  (matches `Poll`'s release-event loop); `step_lock_dirty_` and
  `steps[]` are step-index.
- **`Ui::switch_held(i)` switched to bit-0 immediate detection** (most
  recent debounce sample low) instead of `low()` (8 consecutive lows).
  Removes the ~25 ms latency before a hold gesture registered, so quick
  press-and-turn is caught.
- **`Ui::inhibit_switch(mask)` added** as a public accessor so page
  handlers can suppress the next switch event for chord-style gestures.
  Existing `Poll` chord patterns now use `|=` instead of `=` to
  preserve concurrent inhibits.

#### LCD layout (round 2)

- Sequencer pages adopt the YAM 4-cells-per-row convention: 10 chars
  per cell, delimiters at 0/10/20/30 (skip outer edges), short_name
  left-justified at offset 1 (4 chars), value right-justified at
  offset 5 (4 chars).
- Active control's name uppercased (matches `ParameterEditor`).
- `>` cursor marker before the focused knob's label.
- `UpdateScreen` now reads from the held step's locked value when a
  step is being held + that param is locked; falls through to defaults
  otherwise. Provides feedback during lock authoring.

#### Chaselight (round 2)

- New `kShdwLAST` shadow byte tracks the actually-fired step.
  `Sequencer::Clock` writes `tr.shadow[kShdwLAST] = fired` before
  calling `FireStep`. `SeqStepsPage::UpdateLeds` reads that instead of
  computing `(next + len - 1) % len`, which was wrong in reverse and
  pendulum directions.
- `SeqTrack.shadow[]` size bumped from 5 to 6 (`kShdwSIZE`).

#### Page traversal (round 3)

- `UiPageNumber` enum reordered so `PAGE_PART_SEQUENCER` (S5) precedes
  `PAGE_PART` (S6) and `PAGE_PART_ARPEGGIATOR` (stub). Encoder
  navigation now walks S3 → S5 → S6 → S7 in left-to-right button
  order. `page_registry[]` array entries reordered to match (registry
  index = enum value).
- New `S2 + encoder` chord wired in `Ui::Poll`: `switches_.low(6)`
  multiplies `increment` by 8, giving a full-page (8-cursor) jump.
  Mirrors the existing `S8 + encoder` ×8 acceleration.
- `SeqStepsPage::OnIncrement` and `SeqTrackPage::OnIncrement` spill out
  to the previous/next page via `ui.ShowPageRelative(±1)` at cursor
  boundaries (cursor 23 / 7) so the encoder can leave the sequencer
  surface even though buttons are step triggers.

#### Pot scaling and label cleanup (round 3)

- All `kAbbr` strings lowercased by default; `UpdateScreen` continues
  to uppercase the cursor's slot.
- `E1RL` / `E2RL` / `E3RL` slot labels replaced with `----` (the slots
  are dead per `voice_envelopes.md`).
- Renamed `E1DC` / `E2DC` / `E3DC` → `adec` / `fdec` / `pdec`.
- **`HysteresisPotScanner<8, 0, 8, 7>` produces values 0..127** (not
  0..255). All seq-page pot scaling rewritten:
  - **WAVE1/WAVE2** (`SeqStepsPage::OnPot`) clamped to
    `0..WAVEFORM_LAST-1` (= 0..42). Unconstrained values past the enum
    were silencing voicecard oscillators (out-of-range index into
    dispatch tables → bad pointer / silent state requiring reboot).
  - **DIRN** `>>5` → 0..3.
  - **CDIV / ROTA** `>>4` → 0..7.
  - **LENG** `(>>4)+1` → 1..8 (was `(>>5)+1` capping at 4).
  - **ROOT** `(value*12)>>7` → 0..11.
  - **BPCH** raw value (already 0..127, MIDI range).
  - **OLEV** `<<1` → 0..254.
- `ScalePot(value, max)` helper added in `seq_steps_page.cc`.

#### Defaults (round 3)

- `kDefaultConfig` `E1CRV`/`E2CRV`/`E3CRV` changed from 192 → 64.
  Per `voice_envelopes.md` the curve range is 0..127 (0=linear,
  127=expo), so 192 was out of range. 64 is centered.

#### Documentation (round 3)

- `docs/planning/SPEC_v2.md` renamed to `docs/planning/SPEC.md`. The
  "v2" framing referred to the second design iteration (post-Carcosa)
  and is no longer load-bearing. Renamed via `git mv`; references in
  `README.md`, `CLAUDE.md`, `BOARD.md`, `voice_envelopes.md`,
  `sequencer.md`, `MANUAL.md`, and `CHANGELOG.md` updated.
- `docs/planning/sequencer.md` status table reorganized into Done /
  Open with a "Notes for next session" section pointing at the
  central code paths for the deferred work (focused-edit display,
  step-behavior parameters, mutate, hold modes).
- `docs/planning/BOARD.md` Now/Next reshaped — sequencer foundation
  shipped pending hardware verification; encoder-click focused-edit,
  WAVE strip-aware LUT, and signed-param display offsets surfaced as
  the next iteration's targets.

#### Known issues carried into the next session

- **Encoder click is a no-op on the seq pages.** No focused-edit
  display yet; full-row layout (`<page> | <full param name> <value>`)
  pending.
- **WAVE pot can still hit stripped CZ indices (6..14)** within the
  clamped 0..42 range. Those produce silence per Phase 2 strip but
  the byte is not invalid.
- **Signed/biased params** (FINE / OSC2D / LFO depth / curve as
  signed) render as raw bytes on seq pages — `64` instead of `0` for
  centered. Parameter system handles this on `PAGE_OSCILLATORS`;
  seq pages need their own renderer.
- **Step-behavior resolution** in `FireStep` is still a passthrough
  for note / velocity. PROB / REPT / RATE / SSUB / MINT / MDIR / GLID
  are stored but not acted on.

---

### Sequencer foundation: locks + track settings (2026-05-02)

**Flash result:** controller 45,212 B (69.0%), +1,834 B over the
envelope refactor; voicecard 26,196 B (79.9%), +114 B for the snapshot
handler. **RAM:** controller 3,462 B (+2 B), voicecard 1,049 B
(51.2%, +16 B for the expanded `arguments_` buffer).
`kSystemVersion` bumped to `0x22` on both sides.

Per-step parameter locks are now wired end-to-end. Every trigger
delivers a resolved 16-byte param snapshot (page1+page2) atomically to
the voicecard alongside note + velocity. Locks naturally don't bleed —
each trigger's snapshot fully describes the voice. Sequencer-mode UI
lands on S5 with three knob-pages cycled by the encoder, and per-track
settings (DIRN/CDIV/ROTA/LENG/SCAL/ROOT/BPCH/OLEV) get their own page on
S6. Transport relocates to S7. See `docs/planning/sequencer.md` for the
topic spec.

#### common/protocol.h

- New opcodes: `COMMAND_NOTE_ON_WITH_SNAPSHOT = 0x12` and
  `COMMAND_NOTE_ON_WITH_SNAPSHOT_LEGATO = 0x13`. Same `0xf0` family as
  existing note-on; bit 1 = snapshot variant; bit 0 = legato.

#### voicecard/voicecard_rx.h + voicecard_rx.cc

- `arguments_[]` expanded from 3 → 19 bytes to hold a 16-byte snapshot
  plus note MSB/LSB plus velocity. Other commands write into the same
  buffer head; no functional change.
- `Process()` dispatch adds an explicit branch: snapshot variants set
  `data_size_ = 19`; existing 0x10/0x11 keep `data_size_ = 3`.
- `DoLongCommand()`'s `case COMMAND_NOTE_ON` discriminates on
  `command_ & 0x02`. Snapshot path walks `kSnapshotAddrs[16]` PROGMEM
  and calls `voice.set_patch_data(addr, value)` for each non-`0xff`
  slot, then `voice.Trigger(...)` exactly as the legacy path.
- `kSnapshotAddrs[16]`: voicecard-side mirror of the controller's
  `PatchAddrToSeqField` map for the 16 lockable patch bytes. NOTE and
  the three dead REL slots are `0xff` (skipped).

#### controller/voicecard_tx.h + voicecard_tx.cc

- New `TriggerWithSnapshot(voice_id, note, velocity, legato, snapshot)`.
  Emits the new opcode, 16 snapshot bytes, note MSB/LSB, and velocity
  through the existing odd/even ring buffer infrastructure.
- Legacy `Trigger(...)` retained for MIDI input from `Part::NoteOn`
  (which has no snapshot context).

#### controller/sequencer.cc — `FireStep()`

- Replaced the 3-line stub. For each `i` in `[0..15]`, the resolver
  picks `step.pageX[i]` if `lock_flags` bit `i` is set, otherwise
  `defaults[i]`. Velocity follows the same pattern using lock bit
  `16+kSPVEL`. The resolved note is `snapshot[kP1NOTE]`.
- Mutate (MINT/MDIR), track transpose, and per-step rate are still
  out of scope; the note byte is just the lock-or-default value.

#### controller/ui_pages/seq_steps_page.h + seq_steps_page.cc

- Extended from a single-knob (note-only) toggle stub to a 3-page lock
  authoring UI. `cursor_` (static, 0..23) advances on encoder turn;
  `cursor_ >> 3` becomes `SeqGlobal.lock_page` (Voice1/Voice2/Step).
- `OnPot(index, value)`: if any step button is currently held
  (`Ui::switch_held(i)`), write a lock for that step+param and inhibit
  the next switch event for that step. Otherwise write the track
  default.
- `OnKey(key)`: tap toggles `step_flags & kStepFlagOn`. If a pot moved
  while held (tracked in `step_lock_dirty_`), the toggle is suppressed
  for that release.
- LCD: 8-column abbreviation row + value row, 2-char page tag at the
  right edge (`v1`/`v2`/`sp`), `>` cursor marker on the focused knob.

#### controller/ui_pages/seq_track_page.h + seq_track_page.cc — new files

- 8-pot custom UI mirroring `MultiPage` shape. Knobs map directly into
  `SeqTrack.pattern[8]`: DIRN/CDIV/ROTA/LENG on top, SCAL/ROOT/BPCH/OLEV
  on bottom. Pot ranges quantized per-knob; LCD displays named values
  for DIRN/CDIV/ROOT.

#### controller/ui.h

- New public accessors `Ui::switch_held(i)` and `Ui::inhibit_switch(mask)`
  so page handlers can read switch hold state and suppress the
  corresponding release event (used by the lock-edit gesture).

#### controller/ui.cc — page registry reshuffle

- `PAGE_PART_SEQUENCER`: group 5 → 4 (S5). Now the default landing for
  the Sequencer button.
- `PAGE_PART`: handler swapped from `ParameterEditor` →
  `SeqTrackPage::event_handlers_`. Stays on group 5 (S6).
- `PAGE_MULTI` + `PAGE_MULTI_CLOCK`: group 4 → 6 (S7). Chain
  `MULTI ↔ MULTI_CLOCK` for cycling.
- `default_most_recent_page_in_group[]` updated:
  - group 4 → `PAGE_PART_SEQUENCER`
  - group 5 → `PAGE_PART`
  - group 6 → `PAGE_MULTI`

#### docs/planning/

- New: `sequencer.md` — topic spec with protocol bytes, snapshot table,
  UI layout, and sub-project status table.
- `BOARD.md` — `Now` lane consolidated to "Sequencer foundation"; the
  retired locks/UI items moved out. Added `Later` items for per-voice
  defaults sub-page on S6 and reclaiming the dead REL slots.

#### Notes

- Hardware-flash and on-instrument verification still pending; controller
  + voicecard binaries built clean under the Squeeze toolchain.
- The `*REL` page-2 slots remain dead per `voice_envelopes.md` —
  voicecard ignores writes (snapshot table marks them `0xff`).

---

### Voice envelopes + LFO UI refactor (2026-05-02)

**Flash result:** controller 43,378 B (66.2%), +154 B over Phase 5; voicecard
26,082 B, –62 B from release-mod removal.
**RAM:** controller 3,460 B (–2 B). kSystemVersion bumped to `0x21` on both.

Each voice envelope is now parameterized by three bytes (`rise` / `fall` /
`curve`) plus a depth byte that lives in the modulation slot it drives.
`fall` sets both DECAY and RELEASE phase increments — the voicecard envelope
no longer reads a separate `release` byte. The page that previously selected
one of three envelopes via an active-EG knob is replaced by two pages that
expose all three envelopes plus the voice LFO directly.

Topic spec: `docs/planning/voice_envelopes.md`.

#### voicecard/envelope.h

- `Envelope::Update()` signature changed from `(attack, decay, curve, release)`
  to `(rise, fall, curve)`. `fall` drives both `stage_phase_increment_[DECAY]`
  and `stage_phase_increment_[RELEASE]`.
- DECAY / SUSTAIN targets stay pinned to 0 (Phase 5 model).

#### voicecard/voice.cc

- Envelope update loop drops the `release_mod` clip and the `.release` field
  read. Now reads `rise=patch_.env_lfo[i].attack`,
  `fall=patch_.env_lfo[i].decay`, `curve=patch_.env_lfo[i].sustain`.
- `Patch.env_lfo[i].release` byte left in struct but unread.

#### controller/parameter.cc + parameter.h

- `kNumParameters` 76 → 84.
- Repurposed in place: param 24 (was active-EG selector) → E1 rise; 25 → E1
  fall; 26 → E1 curv; 27 → E1 depth (virtual addr 200); 28 → E2 rise;
  75 → E2 fall.
- Appended params 76–83: E2 curv (34) / E2 depth (201) / E3 rise (40) / E3
  fall (41) / E3 curv (42) / E3 depth (202) / LFO dest (72,
  `UNIT_MODULATION_DESTINATION`) / LFO depth (73, `UNIT_INT8 -63..63`).
- All new params use `instance_count=1, instance_stride=0,
  indexed_by=0xff` — no more `PRM_UI_ACTIVE_ENV_LFO` indexing.
- Depth knob doubles as row label: short_name = `STR_RES_AMP` / `_FLT` /
  `_PCH` / `_DEPT`; full_name = `STR_RES_DEPTH` for all four.

#### controller/resources.cc + resources.h

- New strings (indices 388–392): `STR_RES_RISE` "rise", `STR_RES_FALL`
  "fall", `STR_RES_CURV` "curv", `STR_RES_DEST` "dest", `STR_RES_SHAP`
  "shap".
- `STR_RES_PCH` retitled "pch" → "pitc" (filling the 4-char LCD column;
  the EG selector that used the 3-char form is removed).

#### controller/part.cc

- `kSyncAddresses[]` drops 27, 35, 43 (envelope release bytes) — voicecard
  no longer reads them.

#### controller/ui.cc

- `PAGE_ENV_LFO` knob slots `{ 24, 25, 26, 27, 28, 75, 76, 77 }` —
  E1 rise/fall/curv/depth on top row, E2 rise/fall/curv/depth on bottom.
- `PAGE_VOICE_LFO` knob slots `{ 78, 79, 80, 81, 32, 33, 82, 83 }` —
  E3 rise/fall/curv/depth on top, LFO rate/shape/dest/depth on bottom.

#### docs/planning/

- New: `voice_envelopes.md` — topic spec for the 3-value envelope model.
- New: `BOARD.md` — Now / Next / Later kanban for ongoing work.

#### Notes

- No `SeqTrack` layout change; no EEPROM force-reset required.
- `kP2E1REL` / `kP2E2REL` / `kP2E3REL` slots in `defaults.page2` are dead
  bytes after this change. Will be reclaimed when the sequencer-mode UI
  lands.
- LFO destination + depth params (addresses 72/73) were defined in Phase 5
  but had no UI page; they're now on `PAGE_VOICE_LFO` bottom row.
- `UNIT_EG_SELECT` enum entry stays in `parameter.h` but no longer
  references any param — harmless until a future cleanup.

---

### Phase 5 — Voice Parameter System & Envelope Redesign (2026-05-02)

**Flash result:** 43,224B (66.0% of 64KB), up ~1,810B from Phase 4.
**RAM:** 3,462B used, 634B free. kSystemVersion = 0x20 on both controller and voicecard.

**Architectural pivot:** LPG macro (LPGD/LPGA/LPGO) from SPEC (then SPEC_v2) was not implemented. Instead: three independent ADR+Curve envelopes with fixed routing, LFO1/2/3 removed from controller side (LFO4 voice-side only), `patch_mod[]` removed from SeqTrack (saves 252B), fixed routing table in PROGMEM replaces the per-track mod matrix shadow.

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
