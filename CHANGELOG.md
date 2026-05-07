# Elkhart Changelog

Fork of YAM (`bjoeri/ambika`) targeting a 6-voice polymetric percussive step sequencer
on the Michigan Synth Works Xena motherboard (ATmega644p) + SVF voicecards (ATmega328p).
Build requires avr-gcc 4.3.5 via `./build-squeeze.sh` from the repo root.

---

## [Unreleased]

> Going forward, entries are topic-named and dated; the "Phase N" framing
> below is retired. Historical Phase 2–5 entries kept verbatim. Current
> work tracker: `docs/planning/BOARD.md`.

### Save/Load snapshots to SD card (2026-05-06)

**Flash:** controller 60,004 B (91.6%, +3.4 KB from previous entry).
**RAM:** 3,822 B (93.3%, +6 B for SystemPage statics). Controller only.

Resolves issue #10. New `SystemPage` (Save/Load/Info menu) reachable from
S8 captures the full instrument state — all 6 SeqTracks (steps, pattern,
defaults, config; transient `shadow` is excluded) plus MultiData — to a
fixed-size 2,033-byte file at `/SNAPSHOT/NN.SNP`, slot 00–63. Magic
`ELKS` + version + 8-bit checksum; round-trip verified by checksum on
load. Bypasses the broken legacy `Storage::Save()` RIFF path entirely
(`Part::raw_patch_data()` returns NULL post-Phase 3, and there is no
reverse marshaller from legacy `Patch` bytes back into `SeqTrack`).

UI shows `Cur: NN` (slot the running state was last loaded from / saved
to) and `New: NN *` (encoder-hovered slot, `*` if occupied). Save with
overwrite-confirm dialog; Load on empty slot reports "empty". `Cur:` is
RAM-only — `--` again at boot.

- `controller/snapshot.{h,cc}` — codec.
- `controller/ui_pages/system_page.{h,cc}` — UI page with internal
  MENU/SAVE/LOAD modes, dialog wiring.
- `controller/storage.h` — `friend class Snapshot;` for `file_` / `fs_`
  access.
- `controller/ui.cc` — `PAGE_LIBRARY` registry slot repurposed to point
  at `SystemPage`; `default_most_recent_page_in_group[7]` (S8 entry)
  retargeted from `PAGE_OS_INFO` to `PAGE_LIBRARY`. `OsInfoPage` still
  reachable via S7 from the System menu.

#### SPI bus contention

Voicecards and SD card share one SPI bus, arbitrated by
`scoped_resource<SdCardSession>`. The Timer2 ISR fires
`voicecard_tx.SendBytes()` unconditionally (no `sd_card_busy_` check),
so any voicecard write queued during an SD session either clobbers the
SD card's chip-select mid-transfer or is dispatched onto a bus that's
mode-configured for the SD card — corrupting both. Two consequences for
this feature:

- `Snapshot::Load` holds `SdCardSession` in an explicit inner block,
  so `sequencer.Reset()` and `multi.Touch()` (both queue voicecard SPI
  traffic) run after the session ends, not inside it.
- `Save` calls `sequencer.Stop()` and `Load` calls `sequencer.Panic()`
  before opening the file. `BeginSdCard`'s `FlushBuffers` then waits
  for the queued `Release`/`Kill` writes to drain before reconfiguring
  SPI. Transport stays stopped after the operation; user presses Play
  to resume.

The clean fix would be to make `voicecard_tx.SendBytes()` short-circuit
when `sd_card_busy_` is set, but that lives in a hot ISR path used by
every voicecard-bound write in the firmware and is out of scope for
this initial save/load landing. See `docs/planning/saveload.md` for
full design notes.

### Per-step glid + sfx step-FX + S5 transport refinement (2026-05-05)

**Flash:** controller 56,644 B (86.4%, +584 B). **RAM:** 3,816 B (93.2%,
unchanged — sfx packs into existing `step_flags`). Controller only.

Three GitHub issues land together on the S5a step page and the global
S5 transport chord. They share `controller/sequencer.{h,cc}` and
`controller/ui_pages/seq_steps_page.cc`.

#### S5a glid/gtim consolidation (#23)

The legato `glid` gate on bot3 had no audible effect (steps don't tie),
and `gtim` (portamento time) on bot4 was part-global, not per-step
lockable. Resolved by repurposing the existing `kSPGLID` slot
(steppage[5], lockable index 21) to carry 0..127 portamento time and
freeing bot4 entirely. Pot scaling drops the binary clamp; `FireStep`
pushes the resolved per-step value to the voicecard part struct
(offset 6, the existing portamento address) immediately before the
trigger and passes `legato=0` to `TriggerWithSnapshot`.

- `controller/sequencer.cc` — `FireStep` no longer derives a legato
  flag from `glid`; instead writes `glid` via
  `voicecard_tx.WriteData(t, VOICECARD_DATA_PART, 6, glid)`.
- `controller/ui_pages/seq_steps_page.cc` — abbrev row drops `gtim`,
  `kCellPatchAddr[7]` cleared to `0xff`, GLID binary clamp removed,
  empty-cell guards added in `OnPot` and `UpdateScreen`.

No migration: existing patterns lose the legato semantic (which was
inaudible anyway) and gain per-step slide. Default `glid` stays 0
(no glide), so silent patterns play the same.

#### `sfx` step modifier on S5a bot4 (#28)

New per-step modifier on bot4 (cell sentinel `0xfd`), packed into
`step_flags` bits 2..5 — RAM-neutral, rides existing storage
round-trip. Values: `none, skip, fwd, rev, dir, rjmp, jmp1..jmp8` (14
total, 4 bits). Cell label is `sfx ` (3 + space) so values render with
a gap; only meaningful per-step (no track default), shown as `----`
when no step is held.

Dispatched in `Sequencer::Clock` after the PROB roll — `sfx` only
applies when the step's probability passes. `skip` runs a bounded
re-advance loop (silent clock if every step is skip). `fwd`/`rev`
write `kPatDIRN` (sticky, visible in the direction UI). `dir` toggles
`kPatDIRN` between Fwd↔Rev (sticky toggle, since the original
transient `kShdwDIR` flip was a no-op outside pendulum mode).
`rjmp`/`jmp1..jmp8` reseat `kShdwSTEP` before firing.

- `controller/sequencer.h` — `kSmodNone..kSmodJmp8` constants,
  `kStepFlagSmodMask`, `StepSmod`/`SetStepSmod` accessors.
- `controller/sequencer.cc` — SMOD dispatch block in `Clock`.
- `controller/ui_pages/seq_steps_page.cc` — `kSmodCellSentinel` +
  `kSmodLabels` PROGMEM table; `OnPot` and `UpdateScreen` branches.

Persistence is automatic — the SMOD nibble lives inside an existing
serialized byte. Non-touched bits (`kStepFlagOn`, `kStepFlagGated`)
are preserved by all step_flags writes.

#### S5 transport: play/pause moves from click to encoder CW (#27 follow-up)

The transport chord landed in #27 used encoder click for play/pause,
which collided with the SUBS substep editor on step 5 (also entered
via S5 + encoder click). Resolved by moving play/pause to encoder CW
and dropping the Reset gesture entirely; encoder click is no longer
inhibited so it falls through to the page handler.

Final mapping under hold-S5: **CW** = toggle play/pause; **CCW** =
Stop, double-CCW within 400 ms = Panic; **click** = passes through
(SUBS editor on step 5).

- `controller/ui.cc` — restructured the hold-S5 chord to act on
  `increment != 0` only; CW-branch handles play/pause toggle, CCW
  branch unchanged. `transport_ccw_*` fields retained for the panic
  window.

### MINT becomes chord-shape selector (2026-05-05)

**Flash:** controller 56,060 B (85.5%, +140 B). **RAM:** 3,816 B (93.2%,
unchanged — chord tables are PROGMEM). Controller only.

`MINT` was a single semitone interval (0..12 = `off`, `m2`..`M7`, `oct`)
that the MDIR walk multiplied to produce sub-trigger pitch offsets.
It now selects a chord shape; the walk visits chord tones in interval
order, advancing by an octave each time it cycles through the chord,
capped at MOCT octaves.

New labels (0..12): `off`, `oct`, `pwr`, `maj`, `min`, `sus2`, `sus4`,
`dim`, `7`, `m7`, `M7`, `7sus`, `pent`. The old single-interval shapes
(`m2`, `M2`, `m3`, `M3`, `P4`, `TT`, `P5`, `m6`, `M6`, plus single-interval
`m7`/`M7`) are gone — most were redundant with the chord set, and the
fifth/octave survive inside `pwr` (`{0, 7}`) and `oct` (`{0}`).

- `controller/sequencer.cc` — added `kChordIntervals` (39 B),
  `kChordOffsets` (12 B), `kChordSizes` (12 B) PROGMEM tables.
  Walk math at `FireStep` no longer computes `delta = step_count *
  mint`; instead, `|step_count|` indexes a chord tone via
  `apos % chord_size` and octave-shifts via `apos / chord_size`,
  with sign re-applied. `N = chord_size * MOCT` (was
  `cap_semi / mint`). MDIR up/dn/ud/ud+/ud-/rnd/rnd+/rnd- semantics
  are unchanged.
- `controller/ui_pages/seq_steps_page.cc` — `kMintNames` relabeled.
- `docs/wiki/MANUAL.md` — Mutation section rewritten.
- `docs/planning/sequencer.md` — added divergence banner; existing
  MINT prose left as historical context.

No migration: existing patterns with non-zero MINT will play
different pitches. Re-enter MINT to taste.

### Transport gestures + step-button polish + mrst Master Reset (2026-05-05)

**Flash:** controller 55,920 B (85.3%, +850 B). **RAM:** 3,816 B (93.2%,
+40 B). `kSystemVersion` bumped controller `0x36` → `0x37`. Controller only.

Four GitHub issues land together. They cluster on the global `Ui::Poll`
chord layer, the `SeqStepsPage` step-button event handler, and the
Transport (`PAGE_MULTI`) page.

#### Hold-S5 + encoder = global transport chord (#27)

From any page: hold S5 and use the encoder to control transport without
leaving the current screen.

- Encoder click → toggle play/pause.
- Encoder CW → Reset (release notes, zero playhead).
- Encoder CCW → Stop (Reset + transport=stopped). Notes ring out per
  envelope release.
- Encoder CCW twice within 400 ms → Panic. Hard mute via
  `voicecard_tx.Kill()` on every voice.

S5's release event is suppressed only when the chord actually fires, so
a plain S5 press still toggles steps in sequencer mode. New
`Sequencer::Stop()` and `Sequencer::Panic()` helpers.

#### Hold-S7 + encoder = cycle Transport ↔ Mixer page (#22)

Hold S7 and turn the encoder to jump between `PAGE_MULTI` and
`PAGE_SEQ_MIXER` from any context. `display.Clear()` before `ShowPage`
so cells the new page doesn't paint don't carry stale content from the
previous page.

#### Step-button hold semantics polish (#16)

Holding a step button to "peek" at its locks no longer toggles the step
on release. New per-switch hold-duration tracking lives in `Ui::Poll`
(updated on debounced `lowered`/`raised` edges); pages query
`Ui::last_hold_ms(sr_index)` in `OnKey` to distinguish tap (≤ 250 ms)
from hold.

Bonus: two short taps on the same step within 300 ms clear all
`lock_flags` for that step and undo the first tap's toggle — no on/off
state change, locks gone. Removed the now-redundant `step_lock_dirty_`
early-return that was eating the first tap of every double-tap (the
`Ui::inhibit_switch` set by `OnPot` already suppresses lock-edit
release events).

#### `mrst` Master Reset on Transport pot 2 (#9)

New `mrst` cell on Transport. Range: `off | 2..128` undivided steps.
When the master tick counter reaches the threshold, `Sequencer::Reset()`
fires and zeroes all tracks back to step 0 — useful for keeping
polymetric tracks at varying CDIVs from drifting into open-ended
chaos. Default `off` (free-run). New `SeqGlobal::master_tick`
(uint16_t) accumulates ticks in `Sequencer::Clock()` and is zeroed in
`Sequencer::Reset()`. Encoded value `k` maps to a period of `(k + 1)`
steps for `k ≥ 1`, so storage 0..127 covers off + 2..128.

### Sequencer rate redesign: musical-notation labels with override semantics (2026-05-05)

**Flash:** controller 55,076 B (84.0%, +108 B). **RAM:** 3,776 B (92.2%,
unchanged). `kSystemVersion` bumped controller `0x35` → `0x36`. Controller
only.

The S6a track-level `cdiv` cell is renamed to `rate` and the per-step S5a
`rate` cell now shares the same value table. Previous raw-index display
(`1..16`) is replaced with 15 musical-notation entries:

`32, 16t, 16, 8t, 16d, 8, 4t, 8d, 4, 2t, 4d, 2, 1, 1d, 2B`

Tick periods at 24 PPQN: `{3, 4, 6, 8, 9, 12, 16, 18, 24, 32, 36, 48, 96, 144, 192}`.

#### Override semantics

Per-step `rate` is a 16-entry field. Index `0` is a ` trk` sentinel — the
step inherits whatever the track is currently set to. Indices `1..15` index
directly into the 15-entry table (offset by one). When non-zero, the
per-step value **replaces** the track value for that step (no relative
math, no scaling). This resolves the prior confusion where ratio-style
labels under absolute-beat semantics didn't compose intuitively across
different track CDIVs.

#### controller/sequencer.cc

- `kCDivValues[]` renamed to `kRateValues[]`; values changed to the 15
  musical entries above.
- `Sequencer::Clock()` and `Reset()` adjusted to the new 1-based indexing
  for per-step overrides; defensive clamp `cdiv_idx >= 15 → 14`.
- Default track `rate = 2` (= ` 16` sixteenth note); per-step default
  stays `0` (= ` trk` inherit).

#### controller/ui_pages/seq_track_page.cc

- `kAbbr` slot renamed `cdiv` → `rate`.
- `kCDivLabels` renamed to `kRateLabels`; 15-entry × 4-char PROGMEM table.
- Pot scaling `(value * 15) >> 7` for 0..14 indexing.
- RATE per-step lock field widened from 3 to 4 bits.

#### controller/ui_pages/seq_steps_page.cc

- Extern decl renamed to `kRateLabels`.
- S5a `rate` cell render: `r == 0` → ` trk`; else `kRateLabels[r - 1]`
  with clamp.

#### docs/wiki/MANUAL.md, docs/planning/v4.0_sprint.md

- Updated S6a / S5a `rate` cell descriptions and parameter tables.
- v4.0 sprint plan #14 entry rewritten to reflect final shipped scope.

### MINT/MDIR/MOCT redesign: arpeggiator-style mutation walk (2026-05-03)

**Flash:** controller 54,968 B (83.9%, +508 B). **RAM:** 3,776 B (92.2%,
unchanged). `kSystemVersion` bumped controller `0x34` → `0x35`. Controller only.

The substep mutation feature is overhauled to separate step-size from range
cap and to match arpeggiator semantics. Old behavior (surfaced during
pre-release manual review) had two problems:

1. **Naming.** `8va` / `8va2` / `8m2..8M7` used musical notation that
   overloaded "8" to mean both "one octave" and "octave-plus-interval".
2. **`rnd` semantic mismatch.** `up`/`dn`/`ud` walked in MINT-multiples, but
   `rnd` picked any random semitone offset within ±MINT — so `mint=8va, rnd`
   produced chromatic chaos within ±1 octave instead of octave-flavored
   randomness.

The redesign splits MINT and MOCT and gives MDIR eight wave shapes:

- **MINT** (0..12): step size in semitones. Labels: `off`, `m2`..`M7`, `oct`.
- **MOCT** (1..4): range cap in octaves. New control on substep editor pot 3.
- **MDIR** (0..7): wave shape. `up`/`dn` (sawtooth, wrap to base);
  `ud`/`ud+`/`ud-` (triangle: bipolar / above-base / below-base);
  `rnd`/`rnd+`/`rnd-` (random pick of MINT-multiple in ±/+/− MOCT octaves).

All eight modes step in integer multiples of MINT, so the labelled musical
interval is preserved as the walk's grid. After the offset the note is
clamped 0..127 and re-quantized to the track scale.

#### controller/sequencer.h — bit-packing helpers

- `kSPMDIR` byte now packs MDIR (bits 0..2) + MOCT (bits 3..4). Old saved
  state (high bits zero) decodes as MOCT=1, preserving the prior MDIR
  values 0..3 as `up`/`dn`/`ud`/`rnd`.
- New inlines: `MdirOf(b)`, `MoctOf(b)` (returns 1..4), `PackMdirMoct(mdir, moct)`.

#### controller/sequencer.cc — arpeggiator walk

`Sequencer::FireStep()` mutation block (`sub_idx > 0` branch) replaced with
8-mode switch. Per mode, with `N = MOCT × 12 / MINT`:

- `up`/`dn`: `delta = ±(sub_idx % (N+1)) × MINT` — sawtooth, wraps to base.
- `ud`: triangle, period `4N`, bipolar through base.
- `ud+`/`ud-`: triangle, period `2N`, bounces off base on the chosen side.
- `rnd`: `Random::GetByte() % (2N+1) - N` then × MINT (bipolar).
- `rnd+`/`rnd-`: `±Random::GetByte() % (N+1) × MINT` (one-sided).

#### controller/ui_pages/seq_steps_page.cc — UI

- `kMintNames` collapsed from 25 entries to 13 (`off`, `m2`..`M7`, `oct`).
- `kMdirNames` expanded from 4 entries to 8 (`up`, `dn`, `ud`, `ud+`, `ud-`,
  `rnd`, `rnd+`, `rnd-`).
- Substep editor pot routing: pot 1 = MINT (scale 12), pot 2 = MDIR (scale 7,
  preserves packed MOCT bits), pot 3 = MOCT (scale 3, preserves packed MDIR
  bits, displayed +1 as 1..4).
- LCD substep-editor row extended from 3 cells to 4: `subs Nr | mint m3 |
  mdir up | moct 2`.

#### docs/wiki/MANUAL.md

Substep editor controls + Mutation section rewritten to describe the new
8-mode walk and MOCT control.

### MANUAL.md interim refresh against current master (2026-05-03)

Doc-only. Interim pass on `docs/wiki/MANUAL.md` realigning page-by-page
descriptions with the actual `controller/` source and
`docs/planning/control_map.md`. A final hardware-walked pass at v4.0
tag-time (issue #17) will fold in the v4.0-pending surfaces listed in
the new "Pending v4.0 release" section.

- **S1a Oscillators rewritten** to actual `wave / para / rang / tune`
  per oscillator. Dropped false `BLND / RTIO / FINE / NOTE` claims (they
  live on S1b or S5b, or are sequencer-driven).
- **S1b Mixer added as a real subsection** (`mix / nois / sub / wave`
  top, `xmod / amnt / fuzz / crsh` bot). `fuzz` and `crsh` are
  documented here, not on S2.
- **S2 Filter rewritten** to `freq / reso / mode / env2` with six pots
  inert. Filter mode corrected to four values (LP / BP / HP / Notch),
  not three. `DRIV` / `BITS` removed (cross-referenced to S1b).
- **S3b LFO label** corrected `shap` → `wave`.
- **S4 mod-matrix section deleted**; S4 aliases S3 in v4.0.
- **S6a** kept honest with current code: `----` placeholder still
  inert, "reserved for transpose" line removed.
- **S7 Transport** corrected: removed false `mrst` on `S8` claim
  (`mrst` is not yet implemented); buttons now match `multi_page.cc`.
- **S8** described as the OS Info / firmware page it currently is;
  premature `S8a` patch-slot section removed.
- **v5.0-deferred features stripped** from VERIFY markers: mod
  matrix (#11), LFO sync (#12), iterative probability (#6),
  double-tap-clear (#16), encoder-click focused-edit (#15),
  master transpose / scale (#19/#20).
- **New "Pending v4.0 release" section** consolidates #8, #9, #10,
  #14, #18, #22, #23, #24, #25 with explicit pending status.
- **Tagline** no longer claims a wavefolder ships (it's pending #18).

No firmware changes. `kSystemVersion` unchanged.

### Bug-fix bundle: GH #1, #4, #5, #7 (2026-05-03)

**Flash:** controller 54,460 B (83.1%, +174 B). **RAM:** 3,776 B (92.2%,
+6 B — 1 byte × 6 tracks for the new `kShdwPROB` shadow slot).
`kSystemVersion` bumped controller `0x33` → `0x34`. Controller only.

- **#1 — CDIV>1 misalignment after `rst`+`play`:** `Sequencer::Reset()`
  now resets `kShdwLAST` and pre-charges `kShdwTICK` to each track's own
  `period = kNumTicksPerStep × cdiv`, so every track fires step 0 on the
  very first clock pulse after Play (instead of waiting one period of its
  own CDIV). All tracks line up on the same downbeat regardless of CDIV.
- **#4 — substep mutations ignored track scale:** the MUT note is now
  re-passed through `QuantizeToScale` after the MINT/MDIR offset is
  applied, matching the base-note behavior.
- **#5 — probability rolled per substep:** the PROB roll moved out of
  `FireStep()` and up into `Sequencer::Clock()` at the main-step decision.
  The outcome is cached in a new `kShdwPROB` shadow slot; ratchet substeps
  and REPT repeats inherit it. PROB is now all-or-nothing per main step.
- **#7 — page-jump gesture absorbed by sequencer page:** `S2/S8 + encoder`
  (×8 multiplier in `Ui::Poll`) was being absorbed by `SeqStepsPage`'s 24-
  cell cursor walk — up to three modifier clicks moved cursor through the
  three lock pages before paging out, making the page feel "sticky" when
  arriving from outside the sequencer. `SeqStepsPage::OnIncrement` now
  short-circuits when `|increment| >= 8` and pages out immediately,
  leaving unmodified encoder turns to walk the cells normally.
  Additionally `MultiPage::OnIncrement` was passing the raw multiplied
  increment to `ShowPageRelative`, so CCW from page 7a (transport) jumped
  8 registry slots to 2b (osc mix) instead of stepping back to 6b
  (mixer); now clamped to ±1 like other pages. S2 and S8 are both
  retained.

### Performance mixer (S6b) + transport STOP / panic (2026-05-03)

**Flash:** controller 54,286 B (82.8%, +1,914 B). **RAM:** 3,770 B (92.0%,
+33 B). Controller only — no SPI protocol change. `kSystemVersion` bumped
controller `0x32` → `0x33`; voicecards stay `0x31`. No voicecard reflash.

#### Performance mixer page (S6b, `SeqMixerPage`)

Resolves issue #3. New page on the S6b slot (was the unreachable
`PAGE_PART_ARPEGGIATOR` stub). Surface:

- Pots **top1–3 / bot1–3**: per-voice volume (writes `SeqTrack.pattern[kPatVOL]`).
  Pickup/catch on entry — pot must cross stored value before writing.
- **S1–S6**: toggle the active mode's bit for that voice.
- **S7 tap**: cycle mode `MT-S → MT-A → SOLO → MT-S`.
- **S7 hold + S1–S6 taps**: queue toggles; on S7 release, XOR them into
  the active mode's bits as a single batch. Queued voices blink red during
  the hold.
- **S8**: clear all three bit sets (unmute-all). Tap-only — S8 is the
  system-wide SHIFT prefix in `Ui::Poll`, so hold-gate had to move to S7.
- Encoder walks 8-cell cursor; spills to neighboring page at boundary.

Three modes, per-voice bit sets:

| Mode  | Skip future fires | Action on toggle of currently-sounding voice |
|-------|---|---|
| MT-S  | yes | none — current note's envelope completes naturally |
| MT-A  | yes | `voicecard_tx.Kill(v)` — instant cut |
| SOLO  | non-solo'd voices | `Kill()` non-solo voices that just lost audibility |

State is **transient** (file-static; cleared on power-cycle). No struct
change, no snapshot bump, no patch save. `Sequencer::FireStep` early-returns
on `SeqMixerPage::skip_mask() & (1 << track)`.

LED scheme: MT-S/MT-A lit = audible (bit clear); SOLO lit = solo'd. S7 LED
encodes mode (off / dim red / bright red = MT-S / MT-A / SOLO).

See `docs/planning/mixer.md` for the topic spec.

#### Transport STOP + panic (S6a `MultiPage`)

New `S4 = stop` button on the transport page. Single tap = `Pause()` +
`Reset()`. Double tap within 300 ms = `Pause()` + `Reset()` + `Kill()` all
six voices (panic). LED_4 lights red while the second-tap window is open.

`Sequencer::Pause()` is a toggle (paused → playing on second call), so
the handler guards with `if (transport == kSeqPlaying)` before calling
Pause to keep STOP an absorbing transition.

#### Page registry rename

`PAGE_PART_ARPEGGIATOR` → `PAGE_SEQ_MIXER` in `controller/ui.h`. Group 5
cycle now: `PAGE_PART` (S6a) ↔ `PAGE_SEQ_MIXER` (S6b).

---

### Sequencer substep editor overhaul: gated ratchets, MINT/MDIR, UX fixes (2026-05-03)

**Flash:** controller 52,372 B (79.9%, +498 B). **RAM:** 3,737 B (91.2%,
unchanged). Controller only — no SPI protocol change. `kSystemVersion` stays
`0x32` / voicecards `0x31`. No voicecard reflash required.

#### SSUB=-2 rearchitected: gates REPT period fires, not within-period triggers

The original substep editor ran within-period sub-triggers for SSUB=-2 (the
same mechanism as SSUB>0 ratchets), which caused repeats to behave as ratchets
on editor entry. Rearchitected so SSUB=-2 gates **period-boundary REPT fires**:

- Bit 0 of `substep_bits` = does the initial step fire happen?
- Bits 1..REPT = does each of the REPT subsequent period re-fires happen?

`Clock()` REPT path: when `ssub_l == -2`, checks
`substep_bits & (1 << repeat_idx)` before calling `FireStep`. Initial fire
path checks `substep_bits & 0x01`. No within-period sub-triggers occur for
SSUB=-2.

#### Gated ratchets (`kStepFlagGated`)

New `kStepFlagGated = 0x02` bit in `step_flags`. When set alongside SSUB > 0,
the `Clock()` ratchet sub-trigger path checks
`substep_bits & (1 << slot_now)` before firing — extending substep pattern
editing to within-period ratchets.

The substep editor's count pot (CW zone) sets SSUB=N, REPT=0, and
`kStepFlagGated`. The CCW zone sets SSUB=-2, REPT=N, and clears the flag.
Editor entry from a ratchet step (SSUB > 0) sets `kStepFlagGated` directly
without migrating to REPT.

#### Substep editor: revised entry, pot layout, and UX

**Entry guard**: encoder click on `subs` with a step held is a no-op when
that step has SSUB=0 and REPT=0 (no sub-activity configured). Entry no longer
force-migrates ratchets to REPT.

**Pot layout** moved from physical positions (4/6/7) to the top-row positions
matching the editor's screen labels (0/1/2):
- Pot 0 (`subs` knob): count + mode. Mirrors S5a subs knob —
  CCW 0..55 = repeats 8r..1r, 56..71 = deadzone, CW 72..127 = ratchets 1x..8x.
- Pot 1 (`mint` position): MINT, 0..24 semitones.
- Pot 2 (`mdir` position): MDIR, 0..3 direction.
- All other pots swallowed while editor is active.

**Pot pickup guard**: `substep_pot0_entry_` (0xff sentinel) set on editor
entry. The first pot-0 event records the physical position and returns without
writing, preventing the resting pot position from overwriting the stored value.

**Substep bits sanitization**: on entry, `substep_bits` is masked to the
active range (`(1 << substep_count_) - 1`). Bits above the range are cleared.
If no bits survive the mask, all active slots are re-enabled. Previously
auto-enable only fired when `substep_bits == 0` entire, leaving stale
out-of-range bits able to silently mute slots from a prior session.

**Display**: editor `subs` cell now renders the same `Nr` / `Nx` / `cus`
glyph as S5a (REPT with 'r' suffix, or SSUB with 'x') instead of the raw
fire count (`substep_count_`).

#### MINT/MDIR execution in `FireStep`

`FireStep` gains a `sub_idx` parameter (0 = initial fire, 1..N = repeat or
ratchet fires). When `sub_idx > 0` and `MINT > 0`, the resolved base note
is offset by `sub_idx × MINT` semitones, direction controlled by MDIR:

| MDIR | label | effect |
|------|-------|--------|
| 0 | `up`  | `+sub_idx × MINT` |
| 1 | `dn`  | `−sub_idx × MINT` |
| 2 | `ud`  | odd sub_idx: `+MINT`; even: `−MINT` (ping-pong ±mint from base) |
| 3 | `rnd` | random `±MINT` offset from base per fire |

Note is clamped to 0..127 after offset. All three `FireStep` call sites pass
`sub_idx`: initial fire = 0, REPT re-fires = `repeat_idx` (1-based), ratchet
sub-triggers = `slot_now`.

#### MINT/MDIR display

- **MINT**: `kMintNames[]` (PROGMEM, 25 × 4 chars) covers 0..24. Entries 0..12
  use standard interval abbreviations (`off`, `m2`..`M7`, `8va`); 13..24
  prefix with `8` (`8m2`..`8M7`, `8va2`). Pot clamped to `ScalePot(value, 24)`.
- **MDIR**: `kMdirNames[]` (PROGMEM, 4 × 4 chars): ` up `, ` dn `, ` ud `,
  ` rnd`. Pot mapped `ScalePot(value, 3)` → 0..3 (was 0..1 binary).

---

### Sequencer round 5b: lockable page3, substep editor, execution wiring (2026-05-02)

**Flash result:** controller 50,864 B (77.6%, +3,782 B over 0x31); voicecard
26,216 B (79.9%, +20 B for extended snapshot handler). **RAM:** controller
3,737 B (91.2%, +268 B for page3 lockable extension). `kSystemVersion` 0x31
→ `0x32` on the controller; voicecard `0x30` → `0x31` (snapshot protocol
changed — both sides must be reflashed).

#### Lockable `freq` / `famt` / `pamt` / `wave` — data + protocol

Added `page3[4]` to `SeqStep` and extended `lock_flags[3]` → `lock_flags[4]`.
`SeqStep` is now 34 B (was 29 B); `SeqTrack.defaults[]` grows from 24 to 28
entries. New lockable indices 24–27 map to `kP3FREQ`, `kP3FAMT`, `kP3PAMT`,
`kP3WAVE`.

`Part::PatchAddrToSeqField` cases 16 / 22 / 58 / 11 now map to
`tr.defaults[24+kP3*]` instead of `tr.config[]`; the config slots remain but
are no longer the canonical storage for these four params.

`TriggerWithSnapshot` snapshot grows from 16 → 20 bytes:
```
snapshot[0..7]  = page1[8]
snapshot[8..15] = page2[8]
snapshot[16..19]= page3[4]  (FREQ=16, FAMT=22, PAMT=58, WAVE=11)
```
`voicecard_rx.cc` `kSnapshotAddrs[20]` extended with the four new patch addrs;
`DoLongCommand` loop bound 16 → 20. Total SPI packet: 25 bytes.

`Part::Touch` iterates the expanded `kSyncAddresses[]` which now includes all
page3 addresses; voicecard is fully re-synced on voice init.

#### S5a layout redesign

```
S5a  top  note  vel   vamt  rate
     bot  subs  prob  glid  gtim
```

`mint` and `mdir` removed from the top-level cells — they live exclusively
inside the substep editor now. Their two slots are taken by:
- `vamt` (cell top3, config-mapped, patch addr 85): velocity→VCA mod slot 11
  amount. Backed by `kCfgVELAMT = 19` in `tr.config[]`.
- `gtim` (cell bot4, config-mapped, virtual addr 203): portamento/glide time.
  Virtual addr 203 routes to `VOICECARD_DATA_PART` offset 6 in `Part::SetValue`
  and `Part::Touch` — not to the Patch struct (which does not contain
  portamento). The previous `case 19` mapping was wrong (`filter[1].cutoff`).

Config-mapped cell display fix: `UpdateScreen` now reads config-mapped cells
via `multi.part(track).GetValue(patch_addr)` rather than indexing
`tr.defaults[0xff]` (out-of-bounds read).

#### RATE / REPT / SSUB execution in `Sequencer::Clock`

- **RATE**: `ResolveStepByte(tr, kShdwLAST, kSPRATE)` on each tick; 0 = use
  track CDIV, 1..7 = override with that kCDivValues index.
- **REPT**: `shadow[kShdwREPT]` counts down from the resolved REPT value. The
  step re-fires once per period before advancing; `AdvanceStep` runs when the
  counter reaches 0.
- **SSUB ratchets** (SSUB > 0): N+1 evenly-spaced fires per period. Fixed
  `sub_period = 0` guard: `if (sub_period == 0) sub_period = 1;` so counts
  above the period length still fire.

#### Substep editor (Custom/Edit mode — SSUB = −2)

Entry: hold any step button + encoder click while cursor is on the `subs` cell
(cursor 4). Exit: second encoder click.

SSUB = −2 mode gates each REPT repeat fire by `substep_bits` rather than
firing within-period sub-triggers:
- Bit 0 of `substep_bits` = does the initial fire at the period boundary happen?
- Bits 1..REPT = does each of the REPT subsequent period fires happen?

On entry the editor:
- Migrates a ratchet count (SSUB > 0) to REPT so playback semantics are
  consistent and the subs cell displays "Nr" on exit rather than "edit".
- Reads `substep_count_ = REPT + 1` (total fires including initial).
- Auto-enables bits 0..substep_count_−1 if `substep_bits` is all-zero.

While editing:
- Pot 4 (the physical subs knob): adjusts `substep_count_` (2..8 total fires),
  stored as `REPT = count − 1`. Expanding the count auto-enables new slots;
  shrinking masks them off.
- Pots 6/7: write MINT/MDIR for the held step.
- Pots 0–3, 5: swallowed.
- Step buttons toggle individual `substep_bits` slots; slots ≥ substep_count_
  are gated (button press is a no-op, LED dark, screen shows blank not '−').
- Screen line 0: `subs N | MINT XX | MDIR X`. Line 1: '#'/'-' per active
  slot, blank for inactive.
- LEDs mirror `substep_bits` masked to `substep_count_`.

#### Subs pot redesign

Old pot had Edit (−2) and Cust (−1) in the CCW bands; new design:
- 0..55 (CCW half): 8 bands → repeats 8r..1r (REPT = 8 down to 1).
- 56..71: deadzone — normal, both fields zero.
- 72..127 (CW half): 1x..8x ratchets (SSUB = 1..8).

Edit mode (SSUB = −2) is no longer reachable from the subs pot; it is set
exclusively by entering the substep editor.

#### Bug fixes

- **Envelope depth range**: `parameter.cc` params 27 / 77 / 81 (E1/E2/E3
  depth) had `max = 63`; bumped to 127 to match the round-5a unipolar rescale.
- **RTIO default**: `kDefaultPage1[kP1RTIO]` 7 → 0. The value surfaces as
  crossmod amount on S5b; 7 was audibly wrong.
- **Substep slot_period = 0**: with `kNumTicksPerStep = 6`, `period >> 3 = 0`,
  blocking all substep fires. Now uses `period / sub_count` where
  `sub_count = REPT ? REPT : 8`, with `if (slot_period == 0) slot_period = 1`.

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
