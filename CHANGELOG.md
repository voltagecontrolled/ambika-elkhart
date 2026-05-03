# Elkhart Changelog

Fork of YAM (`bjoeri/ambika`) targeting a 6-voice polymetric percussive step sequencer
on the Michigan Synth Works Xena motherboard (ATmega644p) + SVF voicecards (ATmega328p).
Build requires avr-gcc 4.3.5 via `./build-squeeze.sh` from the repo root.

---

## [Unreleased]

> Going forward, entries are topic-named and dated; the "Phase N" framing
> below is retired. Historical Phase 2–5 entries kept verbatim. Current
> work tracker: `docs/planning/BOARD.md`.

### Sequencer round 5a-1: hardware-test bugfix bump (2026-05-02)

**Flash result:** controller 47,082 B (71.8%, +116 B over 0x30); voicecard
unchanged. RAM unchanged. `kSystemVersion` 0x30 → 0x31 on the **controller
only** — voicecard code didn't change, so existing 0x30 voicecards stay.

Findings from the first hardware-test pass on round 5a:

- **PROB ceiling was unreachable.** Pot wrote 0..127 but the resolver
  short-circuit was `if (prob < 255)`, so 127 = ~50% skip rate. Storage
  rescaled to 0..127 native, default `kDefaultStepPage[kSPPROB] = 127`,
  resolver compares `(rand & 0x7F) > prob`. 127 = always fires, 0 = never.
  Display now renders as `0%`..`100%` (`prob * 100 / 127`).
- **VEL was inert.** `kDefaultMod` slot 11 (VELOCITY → VCA) had `amount = 0`
  — the routing was wired but at zero depth, so velocity had no audible
  effect. Bumped to `127` (full sensitivity matching the round-5a env-depth
  rescale). This also unmasks `VOL` on S6 — `(vel * VOL) >> 8` had nothing
  to attenuate before.
- **BLND glitched in upper half.** The reserved-for-future-FM range
  (BLND ≥ 64) produced undefined output. UI pot now clamps `value >> 1`,
  so 0..127 pot → 0..63 crossfade-only. Half-resolution but no glitch zone.
- **`SCAL pMi` cosmetic.** Scale labels now use a leading-space pattern
  (`" chr"`, `" maj"`, `" pMi"` …) so the cell renders `SCAL pMi` not
  `SCALpMi`. `chro` shortened to `chr`.

Voicecards stay at 0x30 — only `firmware/latest/AMBIKA.BIN` needs flashing.

### Sequencer round 5a: env-depth rescale + S5/S6/S7 layout + step-behavior wiring (2026-05-02)

**Flash result:** controller 46,966 B (71.7%, +984 B over 0x25);
voicecard 26,194 B (79.9%, ~unchanged). **RAM:** controller 3,469 B
(no change), voicecard 1,049 B (no change). `kSystemVersion` bumped
to `0x30` on both sides.

Round 5a is the first half of the sequencer round-5 effort: ship the
layout / cosmetic / env-depth changes plus the cheap resolver wiring
(PROB / GLID / VEL+VOL / SCAL). The rest of round 5 (RATE / REPT /
SSUB ratchet execution, Mutate, the Custom/Edit substep editor)
moves to round 5b.

#### Envelope-depth range extended 0..63 → 0..127, unipolar

The old bipolar -63..+63 mapping was too narrow for percussive use —
in particular building a kick wanted more headroom on the VCA and
filter envelopes. Both `famt` (filter-env depth) and `pamt`
(pitch-env depth) on S5b now pass the pot value through unmodified
(0..127, unsigned), and display unsigned. Storage stays as `int8_t`
so the existing `S8U8Mul` / `S8S8Mul` matrix paths are unchanged.

The VCA-amount path in `voicecard/voice.cc` `ProcessModulationMatrix`
needed adjusting because it special-cased "amount == 63 → full
mod, skip the U8Mix": `<< 2` → `<< 1`, `!= 63` → `!= 127`. Net
effect: amount=127 still means full pass-through (matches the old
amount=63 behavior at full depth); intermediate values now have
finer granularity. The two additive paths (filter_env via patch
addr 22, ENV3→pitch via mod slot 2 amount) needed no voicecard
changes — they already accepted the full int8 range.

Defaults: `kCfgE1DEPT` 63 → 127, `kCfgE2DEPT` 63 → 64
(`kCfgE3DEPT` stays 0).

#### Sequencer-mode page rebind (S5a / S5b / S5c)

The three sequencer-mode lock pages were rebuilt and reordered so the
step-behavior surface is leftmost — default `cursor_=0` now lands on
NOTE, the most foundational sequencer knob. Voice 1 (osc/mix) follows
on S5b and Voice 2 (filter/env/sub) on S5c. The encoder still walks
cursor 0..23 left-to-right; spilling past S5c lands on S6 (track
settings), and each transition zooms outward (per-step → per-voice
tone → per-track pattern).

NOIS moved up to S5b top1 (was on the old voice-2 page); FREQ (filter
cutoff, config-mapped to patch addr 16) moved into S5c top1 alongside
the filter/env params; the rest of S5c reshuffled to group filter /
env / sub cleanly. Sub-osc `wave`, `famt`, `pamt` remain config-mapped
(write through `Part::SetValue`).

```
S5a  top  note vel  glid rate
     bot  subs prob mint mdir

S5b  top  nois w1   pa1  tun2
     bot  mix  w2   pa2  fin2

S5c  top  freq fdec famt adec
     bot  pdec pamt sub  wave
```

`subs` (S5a bot1) is the first merged-cell on the seq surface: a
single bipolar knob writing to either `kSPSSUB` or `kSPREPT` (with
mutex zeroing). Pot bands: 0..7 = Edit (-2), 8..15 = Cust (-1),
16..62 = repeats 7..1, 63..71 = normal (deadzone), 72..127 =
ratchets +1..+8. Display glyphs: `Edit` / `Cust` / `N r` / `0` /
`N x`. UI/storage land in this round; ratchet/repeat/Custom-edit
*execution* is round 5b.

#### Cosmetic: `wav1`/`wav2` → `w1`/`w2`

The 4-char abbreviations on the wave cells ate two columns the wave
name needed. Wave cells now render the abbr at 2 chars (positions
1..2) and grow the value field to 6 chars (positions 3..8).
`triangl`, `square`, `polysaw` etc. fit without the truncation that
made it impossible to tell apart `polys` from `polyt` on the prior
4-char field.

#### Track-page rework (S6)

```
top  dirn cdiv rota leng
bot  scal root ----  vol
```

- `kPatBPCH` slot retired — `defaults[kP1NOTE]` is the de-facto
  per-track base pitch. The slot is kept reserved (cell renders
  `----`, pot is inhibited); a future signed track-transpose will
  live on the Performance page.
- `kPatOLEV` renamed to `kPatVOL` (pot 0..127 → 0..255). Wired in
  `Sequencer::FireStep` as a multiplicative scale on the resolved
  velocity: `velocity = (vel * VOL) >> 8`. VOL=255 = identity,
  VOL=0 = silent track.
- `kPatSCAL` knob now indexes an 8-entry scale LUT (chromatic /
  major / minor / dorian / mixolydian / pentaMaj / pentaMin /
  blues). The labels render on the cell (`chro` / `maj ` / `min `
  / etc.). `QuantizeToScale` snaps the resolved note down to the
  nearest scale-allowed semitone using `kPatROOT` as the
  reference; chromatic is a no-op pass-through.

#### Step-behavior resolver (`Sequencer::FireStep`)

- **PROB**: `if (prob < 255 && Random::GetByte() > prob) return;`
  before the snapshot is even built. 255 = always fire.
- **VEL + VOL**: lock-or-default `kSPVEL`, then multiply by
  `pattern[kPatVOL]` (>> 8).
- **SCAL**: `QuantizeToScale(note, scal & 7, root)` after the note
  is resolved from the lock/default.
- **GLID**: lock-or-default `kSPGLID`; any non-zero value sets the
  legato bit on `TriggerWithSnapshot` (0x12 → 0x13). The voicecard
  side already honored the legato bit (round 3 work).

A small `ResolveStepByte(track, step_idx, kSP*)` helper centralizes
the lock-or-default lookup so adding the remaining step-behavior
params in round 5b is a one-liner per param.

Deferred to round 5b: `kSPRATE` per-step subdivision override,
`kSPREPT` repeats, positive `kSPSSUB` ratchets, the Custom/Edit
substep editor (encoder-click + held-step on `subs` cell), and
Mutate (`kSPMINT` / `kSPMDIR`) — likely consolidated into the
substep editor since walking pitch only makes sense across
ratchets.

#### Transport (S7) alignment

`swng` was rendered at columns 16..23, but its pot is top2 — which
sits over columns 10..19. Fixed by rebuilding line 0 around the
canonical 4-cells-per-row convention: `bpm` in cell 0 (cols 1..7),
`swng` in cell 1 (cols 11..17). Line 1 (play / paus / rst / exit)
unchanged.



**Flash result:** controller 45,982 B (70.2%, +404 B over 0x23);
voicecard 26,196 B (no size change). **RAM:** controller 3,469 B
(no change), voicecard 1,049 B (no change). `kSystemVersion` bumped
to `0x25` on both sides — voicecard joined the bump because the
snapshot table now writes to addresses 6 / 7.

A round of UI iteration on top of the sequencer foundation: the S5
page-1/2 cells were rebuilt around per-cell descriptor tables so a
single cell can target either a lockable param or a config byte.
Page 1 now exposes osc 2 coarse / fine tuning as proper per-step
locks; page 2 surfaces the env depth amounts (`famt` / `pamt`) and
sub-osc shape (`wave`) inline. The S7 group collapsed from two pages
to one, with the groove-amount knob (`swng`) folded onto the
transport page.

#### Defaults

- `kDefaultPage1[kP1FINE] 64 → 0`. Osc 1 detune now sits at the int8
  center; the previous "64" rendered as "+64 cents" via UNIT_INT8.
- `kDefaultPage1[kP1RTIO]` stays at `7` (1.0 ratio). Comment retitled
  to flag the byte as *reserved for future linear FM* — the slot will
  drive the osc2:osc1 ratio LUT when the BLND≥64 FM mode lands.
- `kDefaultPage2[kP2TUN2] = 0`, `kDefaultPage2[kP2FIN2] = 0` in the
  reclaimed slots (centered int8).

#### Lockable space reshuffle (`controller/sequencer.h`)

- `kP2E1REL` (idx 1, lock idx 9) → `kP2TUN2` (osc 2 coarse, patch
  addr 6). Voicecard ignored E1REL writes (release_mod removed in
  0x21), so the slot was dead.
- `kP2E2REL` (idx 3, lock idx 11) → `kP2FIN2` (osc 2 detune, patch
  addr 7). Same justification.
- `kP2E3REL` (idx 5, lock idx 13) remains dead pending future use.
- `kCfgOSC2R` / `kCfgOSC2D` enum slots stay (removing them cascades
  through every other reference) but are now unused — the bytes live
  in `defaults.page2` for lockability.
- `controller/part.cc` `PatchAddrToSeqField` cases 6 / 7 now route to
  `defaults[8 + kP2TUN2/FIN2]`. Cases 27 / 35 (E1REL / E2REL release
  bytes) are dropped — they had no live consumers; case 43 retained
  for legacy CC paths to land somewhere.
- `voicecard/voicecard_rx.cc` `kSnapshotAddrs[9] = 6`, `[11] = 7`.
  Per-step locks now reach the voicecard via the snapshot path
  exactly like the existing page-2 lockables.

#### S5 sequencer mode (`controller/ui_pages/seq_steps_page.cc`)

- Rewritten around two PROGMEM cell-descriptor tables —
  `kCellLockable[24]` (lockable index per cell, `0xff` = config) and
  `kCellPatchAddr[24]` (patch address for config-mapped cells).
  Encoding lets a single cell target either a per-step lockable
  byte or a voice-wide config byte; config cells push through
  `Part::SetValue` so changes reach the voicecard immediately.
- New page-1 layout (8 cells):
  ```
  top  note  WAV1  PA1   tun2
  bot  mix   WAV2  PA2   fin2
  ```
  All eight are lockable. RTIO + FINE retired from this surface but
  still reachable on S1a / S1b.
- New page-2 layout:
  ```
  top  fdec  famt  pdec  pamt
  bot  adec  nois  sub   wave
  ```
  `famt` / `pamt` / `wave` are config-mapped (filter-env depth at
  patch addr 22, pitch-env depth at 58, sub-osc shape at 11). Dead
  `*REL` cells gone from the UI.
- Page-3 (step behavior) layout unchanged.

#### Display polish

- WAVE1 / WAVE2 / sub-WAVE cells render the waveform name string
  instead of the raw byte (uses `STR_RES_NONE` / `STR_RES_SQU1` base
  + value, parallels `Parameter::PrintValue`).
- Signed display for `tun2` / `fin2` (lockable, lock indices 9 / 11)
  and `famt` / `pamt` (config). NOTE cell value shifted +1 column so
  its octave digit right-aligns with neighbouring values.
- New helpers: `MapPotInt8(value, min, max)` for bipolar pot scaling,
  `WriteI8Right` for signed-display formatting, and
  `IsSignedLockable(lockable)` so the dispatch doesn't conflate
  lockable tun2/fin2 with config-mapped famt/pamt.

#### S7 transport collapse (`controller/ui_pages/multi_page.cc` + `controller/ui.cc`)

- `MultiPage::OnPot` index 1 now drives `PRM_MULTI_CLOCK_GROOVE_AMOUNT`.
  Line 0 reads `bpm xxx | swng xxx`.
- `PAGE_MULTI` `next_page` set to itself (single-page group). Pressing
  S7 a second time is a no-op rather than walking to clock params.
- `PAGE_MULTI_CLOCK` registry entry retained (still needed as the
  upper bound used by `ShowPageRelative`'s wraparound) but its data
  cells are now all `0xff` so the encoder skips it.

#### Notes for next session

- Hardware verification of 0x25 still pending. The snapshot semantic
  change (addrs 6 / 7 became active) means a stale 0x23 voicecard
  would silently ignore the per-step tun2/fin2 locks.
- Linear FM via `BLND ≥ 64` + RTIO ratio LUT is queued for a separate
  planning phase (likely phase-mod-first to keep voicecard CPU sane,
  with a 16-entry ratio LUT at 0.25/0.5/.../4.0).
- `docs/wiki/MANUAL.md` is now significantly behind the implementation
  (mentions LPGD/LPGA/LPGO macro envelopes that never shipped, plus
  the older Page 1 layout with RTIO / FINE). Refresh tracked in
  `BOARD.md` Later.

---

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
