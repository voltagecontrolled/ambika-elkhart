# Edit RATE in Raw Ticks

> Status: planned, not implemented. Earlier session drafted the design below,
> then aborted at the strip stage when persistence-format implications became
> clear. This document captures both the design and the gotchas to inform a
> future implementation pass.

## Motivation

Sequencer RATE — per-track `pattern[kPatCDIV]` and per-step
`steppage[kSPRATE]` — currently stores an index into a 15-entry table of
musical divisions:

```
kRateValues = {3, 4, 6, 8, 9, 12, 16, 18, 24, 32, 36, 48, 96, 144, 192}
```

These are step periods in MIDI ticks at 24 PPQN. Periods like 5, 7, 10, 11
are unreachable, so tracker-style swing is impossible: alternating `t7 / t5`
on 16ths gives ~58% swing; alternating `t8 / t4` gives a triplet feel; the
musical-only table forces a binary jump from straight (period 6) to triplet
(period 8) with nothing in between.

A prior session removed the PAGE_MULTI `swng` pot from the UI but left the
backing groove/swing state and resources orphaned. Wiring the swing template
back up was considered and rejected: at 24 PPQN, a "swing knob" has only
four distinct useful states between straight and triplet — the same
expressive range raw-tick provides directly, with none of the design
complications around polymeter, direction modes, and parameter-lock
interaction. Tracker-style raw-tick swing composes cleanly with everything
else the sequencer does; a global template does not.

## Approach: bit-7 escape on the existing RATE byte

A RATE byte does double duty based on its high bit:

| Byte range     | Meaning                                                       |
|----------------|---------------------------------------------------------------|
| `0x00`         | (per-step only) "inherit track rate" sentinel                 |
| `0x01..0x0F`   | preset index into `kRateValues[]` (existing semantics)        |
| `0x82..0xE0`   | raw ticks: `byte & 0x7F` is the period, clamped `[2, 96]`     |

`0x80`, `0x81`, and `0xE1..0xFF` are out-of-band. The minimum (2 ticks)
stays one short of the existing `period >= 3` assumption noted at
`sequencer.cc:612` but keeps gate-window math non-degenerate. The maximum
(96 ticks = 1 bar at 24 PPQN, matching the `1` preset) keeps the raw range
useful — `1d` (144) and `2B` (192) are reachable only via musical mode,
where sub-tick precision is meaningless anyway.

Saved patches load bit-identically: existing preset bytes have bit 7 clear,
so the encoding is fully backward-compatible at the byte level.

## UX: encoder-click mode toggle

Encoder click on a RATE cell flips bit 7:

- **Preset → raw:** resolve `period = kRateValues[idx]`, store
  `0x80 | period`. The display flips from `" 16 "` to `" t6 "`. Pot/encoder
  now edit in tick space.
- **Raw → preset:** linear-scan `kRateValues[]` for the nearest entry, store
  that idx. Round-trip is lossy for off-preset ticks — that's the intended
  "back to musical land" behavior.
- **Per-step inherit (`value == 0`):** no-op. Nothing to toggle until a
  concrete rate is set.

Encoder turn continues to walk the cursor (existing page behavior). Edit
paths in raw mode:

- **`seq_track_page.cc`** (cursor==1): pot edits the track RATE. Maps
  `0..127 → ticks 2..96` linearly with snap-on-cross (value doesn't update
  until pot physically sweeps through current). Encoder is cursor-only.
- **`seq_steps_page.cc`** (lockable==19): when a step is held, pot writes a
  per-step lock through the same raw-mode mapping. This is the path that
  makes per-step swing locks (`t7 / t5` alternating) reachable.

## Display

- **Preset:** existing `kRateLabels` lookup (`"  32"`, `" 16t"`, `" 8d "`).
- **Raw:** `" tNN"` formatted via `UnsafeItoa` + `AlignRight` (max value 96
  → max 2 digits). No new PROGMEM table.
- **Inherit** (`seq_steps_page.cc` only): existing `" trk"`.

## Implementation outline

**`controller/sequencer.cc`**

Add a `static inline uint8_t RatePeriod(uint8_t byte)` helper near
`kRateValues[]`. Returns ticks: if `byte & 0x80` →
`clamp(byte & 0x7F, 2, 96)`; else → `pgm_read_byte(kRateValues + min(byte, 14))`.

Update two call sites:

- Fire-time resolve (`sequencer.cc:268-271`): for the per-step byte, if
  `rate == 0` fall back to `RatePeriod(tr.pattern[kPatCDIV])`; else
  `RatePeriod(rate)`. The existing `rate ? (rate-1) : ...` -1-indexing
  trick collapses into the helper — pass through with bit 7 preserved.
- Reset pre-charge (`sequencer.cc:644-647`): replace the clamp +
  `pgm_read_byte` pair with `period = RatePeriod(tracks_[t].pattern[kPatCDIV])`.

**`controller/ui_pages/seq_track_page.cc`**

- Add custom `OnClick`: if `cursor_ == 1`, toggle bit 7 on `tr->pattern[1]`
  per the rules above; otherwise delegate to `UiPage::OnClick`. Register
  in `event_handlers_`.
- Extend `OnPot` case 1: if `tr->pattern[1] & 0x80`, map value →
  `0x80 | (2 + ((value * 94) >> 7))` capped at 96, with snap-on-cross gate.
  Else existing preset-index mapping.
- Extend `UpdateScreen` case 1: if `v & 0x80`, render `" tNN"`; else
  existing `kRateLabels` lookup.

**`controller/ui_pages/seq_steps_page.cc`**

- Add (or extend) `OnClick`: if cursored cell is `lockable == 19` and
  stored value is non-zero, toggle bit 7 on the appropriate slot
  (`tr.defaults[16+kSPRATE]` when no step held, `step.steppage[kSPRATE]`
  when held). No-op when value == 0.
- Extend `OnPot` `lockable == 19` branch: same raw-mode mapping with
  snap-on-cross.
- Replace `uint8_t r = v & 15;` at the render site (currently corrupts
  raw-tick bytes) with explicit bit-7 branch.

### Reuse

- `kRateValues[]` and `kRateLabels[]` — unchanged.
- `UnsafeItoa<uint8_t>(...)` and `AlignRight(...)` from `avrlib/string.h`.
- `UiPage::OnClick` default — delegate from custom override for non-RATE
  cursors.
- `parameter.cc:is_snapped` — pattern to follow for snap-on-cross gating.
  May need a small page-local "snapped" flag since RATE doesn't route
  through the parameter system on these pages.

## Cost estimate

Raw-tick feature alone: ~+250–400 B flash, 0 B RAM.

Combined with a full groove/swing strip (see below): ~−100 to −300 B flash
(net reclaim), −3 B RAM.

Baseline measured before the aborted attempt: 58458 B flash (89%, 7078 B
free), 3867 B RAM (94%, 229 B free).

## Why the combined plan was aborted

The earlier session attempted to fold a "finish the groove/swing strip"
into the same commit. The strip turned out to be more invasive than
expected, for reasons not visible until verification:

1. **`MultiData` field order is a serialized contract.** `MultiData`
   contains `clock_groove_template` and `clock_groove_amount` at byte
   offsets 1 and 2. The struct is dumped to SD via `snapshot.cc:107` (full
   `sizeof(MultiData)` raw write) and to EEPROM via `storage.cc:97`.
   Removing those two fields shifts every following field by 2 bytes,
   which breaks every saved patch and every snapshot file on the SD card.
   `snapshot.cc:22` already carries a `kMultiDataV1Size = 5` constant and
   a v1→v2 migration; finishing the strip means cutting a snapshot v3 and
   handling the v1/v2→v3 migration in the load path. Out of scope for a
   raw-tick feature commit.

2. **Resources renumbering is mechanical but error-prone.** Strings 321–326
   (`swing/shuffle/push/lag/human/monkey`) and the 6 `lut_res_groove_*`
   LUTs are referenced positionally from `str_table[]` and `lut_table[]`
   in `resources.cc`. Removing them requires renumbering 66 `STR_RES_*`
   defines (327–392 → 321–386) in `resources.h` and reordering both
   tables. No raw numeric references exist outside `resources.h/.cc`
   (verified — all consumers use the named `STR_RES_*` macros), so the
   change is safe but the diff is large.

3. **Python source-of-truth not runnable locally.** `resources.h/.cc` are
   generated by `avrlib/tools/resources_compiler.py` from
   `controller/resources/strings.py` and `lookup_tables.py`. The compiler
   is Python 2.5 era and requires `numpy`, which is not available in this
   environment. CLAUDE.md forbids local tool installation — the AVR build
   itself uses Squeeze Docker. Either the python sources and the generated
   C++ are edited in lockstep by hand, or the regeneration step is moved
   inside a separate Docker image. Neither is appropriate to bundle with
   a raw-tick feature commit.

4. **Verified safe, just deferred:** `SeqGlobal` is RAM-only — never
   serialized — so removing `SeqGlobal.swing` is free of save-format
   implications. That single byte could be reclaimed in isolation if
   desired, but it's not load-bearing for the raw-tick feature.

## Recommended next-session shape

Two separate commits, in order:

1. **Raw-tick RATE escape only.** Touches `sequencer.cc`,
   `seq_track_page.cc`, `seq_steps_page.cc`. No persistence-format change,
   no resources changes, no groove/swing removal. Ships the feature.
   Estimated: +250–400 B flash, 0 B RAM.

2. **Dedicated groove/swing strip + snapshot v3.** Separate session,
   focused on:
   - `MultiData` field removal + `snapshot.cc` v3 layout and v1/v2→v3
     migration.
   - `resources.h/.cc` renumbering, with python sources updated to match.
     Either run the regeneration tool inside Docker, or commit hand-edited
     C++ alongside python source changes and note the drift.
   - `SeqGlobal.swing`, `multi.clock_groove_*`, `UNIT_GROOVE_TEMPLATE`,
     `PRM_MULTI_CLOCK_GROOVE_*`, parameter descriptors, the 6 groove LUTs,
     and the 6 groove strings.
   - Estimated reclaim: ~400–600 B flash, ~3 B RAM.

## Open questions for the implementation pass

- Does `seq_steps_page.cc` already register a custom `OnClick`, or is it
  using the `UiPage` default? Determines whether the click toggle hooks
  cleanly in.
- Does `parameter.cc:is_snapped` extend to cells that don't route through
  the parameter system, or is a small ad-hoc "snapped" flag needed in
  page-local state? RATE on these two pages is edited directly via
  `pattern[]` / `steppage[]` writes, not through `Parameter::Scale`.

## Hardware test checklist

- Existing preset behavior unchanged across saved patches.
- Track-level RATE: encoder-click flips `" 16 "` ↔ `" t6 "`; pot in raw
  mode sweeps tick values with snap-on-cross.
- Per-step RATE: click on `" trk"` (inherit) is a no-op; click on a
  concrete rate flips musical ↔ raw; clicking back snaps to nearest
  preset.
- Swung 16ths: track CDIV=16, step 0 RATE=`t7`, step 1 RATE=`t5`, etc.
  Audible swing at the expected ratio; no drift across many bars.
- Polymeter: tracks at different CDIVs, one with raw-tick locks, stay
  independent and don't desync the others.
- MRST mid-pattern: tracks with raw-tick rates re-align cleanly (step 0
  fires at master tick 0 + period − 1, matching `sequencer.cc:644-647`).
- Save/load round-trip with bit-7 rates present.
- Voicecard image untouched — no rebuild required.

---

## Reference: original plan file

The session that produced this design also wrote a detailed implementation
plan at `~/.claude/plans/all-the-swing-resilient-pond.md` (machine-local,
not checked in). Its full contents follow for reference. Most of it is
captured above; the differences are (a) the original plan included the
groove/swing strip in scope, and (b) the original plan predated discovery
of the `MultiData` persistence issue described above.

````markdown
# Raw-tick RATE escape + finish groove/swing strip

## Context

Elkhart's sequencer RATE (per-track `pattern[kPatCDIV]` and per-step
`steppage[kSPRATE]`) currently stores an index into a 15-entry table of
musical divisions (`kRateValues = {3,4,6,8,9,12,16,18,24,32,36,48,96,144,192}`
ticks at 24 PPQN). The user cannot pick periods like 5, 7, 10, 11 ticks —
exactly the values needed to dial in swing tracker-style (e.g. 7/5
alternating 16ths ≈ 58% swing).

A prior cleanup session removed the PAGE_MULTI `swng` pot/display and moved
MRST into that slot, but the underlying swing/groove state, parameter
descriptors, and 6 PROGMEM groove templates are still present and orphaned.
The decision (this session) is: don't wire up the swing template — at 24
PPQN it has only 4 distinct useful states between straight and triplet, the
same range raw-tick already gives the user. Strip the residual instead and
let users program swing tracker-style by editing two step RATEs per voice.
Per-track, per-step raw-tick swing composes cleanly with polymeter,
direction modes, and parameter locks; a global swing template does not.

**Intended outcome:** one byte of storage per RATE cell does double duty —
bit 7 clear = preset index (existing semantics, saved patches load
bit-identically); bit 7 set = `byte & 0x7F` raw MIDI ticks. Encoder-click
on the RATE cell toggles between modes. All vestigial groove/swing code is
removed. Net cost: ~−100 to −300 B flash, −3 B RAM.

## Scope

Two coordinated changes in one plan:

1. **Finish the groove/swing strip** (cosmetic UI strip already happened;
   this kills the backing state and resources).
2. **Add raw-tick RATE escape** with bit-7 flag, encoder-click mode toggle,
   render/pot/encoder support on both `seq_track_page.cc` (cursor==1) and
   `seq_steps_page.cc` (lockable==19).

## Design

### Encoding

A RATE byte is one of three things:

- `0x00` — only valid for per-step `kSPRATE`: "inherit track rate"
  (existing sentinel).
- `0x01 .. 0x0F` — preset index. Per-step uses 1..15 (resolves to
  `kRateValues[idx-1]`); per-track uses 0..14 directly. Existing semantics.
- `0x82 .. 0xE0` — raw ticks. `byte & 0x7F` is the period in ticks,
  clamped to `[2, 96]`. (96 ticks = 1 bar at 24 PPQN, matching the `1`
  preset; anything longer is preset territory — `1d`=144 and `2B`=192 are
  reachable only via musical mode.)

`0x80`, `0x81`, and `0xE1..0xFF` are out-of-band. Period < 2 violates the
`period >= 3` assumption noted at `sequencer.cc:612` for gate-window math;
clamping to 2 stays one short but keeps gate-windows non-degenerate.
Period=1 would fire every `Multi::Clock()` and break gate timing.

### Mode toggle UX

Encoder click on the RATE cell:

- Bit 7 clear, value > 0: convert preset → raw. Resolve
  `period = kRateValues[idx-or-idx-1]`, store `0x80 | period`. Display
  flips from e.g. `" 16 "` to `" t6 "`. User can then nudge with
  encoder/pot.
- Bit 7 set: convert raw → preset. Snap to nearest entry in
  `kRateValues[]` via linear scan, store that idx. Round-trip is lossy for
  off-preset ticks; that's the intended "back to musical land" behavior.
- Per-step inherit (`value == 0`): no-op. Nothing to toggle; user must
  first set a concrete rate.

### Encoder turn / pot on RATE cell

- Preset mode: existing behavior. `seq_track_page.cc` pot maps 0..127 →
  0..14 (existing); `seq_steps_page.cc` pot maps `value >> 3` → 0..15
  (existing).
- Raw mode:
  - Pot maps 0..127 → ticks 2..96 linearly: `2 + ((value * 94) >> 7)` ≈
    0.74 ticks per pot position. Covers the entire useful raw-tick range,
    with hysteresis pot resolution overlap being benign.
  - Pot uses **snap-on-cross**: the value doesn't update until the pot
    physically sweeps through the current value. Standard Ambika feel;
    see `parameter.cc:is_snapped` for the existing snap mechanism (extend
    the same pattern here — track a per-cell "snapped" flag transient).
  - Encoder turn on `seq_track_page.cc` continues to walk the cursor (no
    value editing — matches current page behavior at lines 62-76).
    Tick-precise editing on the track page comes from the pot only.
  - On `seq_steps_page.cc`, when a step is held (`held_sr != 0xff`) and
    the cursored cell is RATE: pot writes via the same raw-mode mapping
    into the lock slot at `seq_steps_page.cc:494-497`. This is the path
    that makes per-step swing locks (e.g. `t7 / t5` alternating)
    reachable.

### Display

Add a small renderer used by both pages:

- Preset → existing `kRateLabels` lookup (`"  32"`, `" 16t"`, `" 8d "`,
  etc.).
- Raw → format as `" tN "` or `" tNN"` (max value 96 → max 2 digits) via
  existing `UnsafeItoa` + `AlignRight` in `avrlib/string.h`. No new
  PROGMEM table.
- Per-step inherit (only on `seq_steps_page.cc`) → existing `" trk"`.

## Files to modify

### Raw-tick feature

**`controller/sequencer.cc`**

- Add a `static inline uint8_t RatePeriod(uint8_t byte)` helper near
  `kRateValues[]` at line 25. Returns ticks: if `byte & 0x80` →
  `clamp(byte & 0x7F, 2, 96)`; else →
  `pgm_read_byte(kRateValues + min(byte, 14))`. Document the bit-7 escape
  in the comment block at lines 19-24.
- Update the fire-time site at lines 268-271: replace the `cdiv_idx` +
  `pgm_read_byte` pair with calls to `RatePeriod()`. For the step byte: if
  `rate == 0` → fall back to `RatePeriod(tr.pattern[kPatCDIV])`; else
  `RatePeriod(rate)` (passing through with bit 7 preserved). The current
  `rate ? (rate-1) : ...` -1-indexing trick collapses into the helper.
- Update the Reset pre-charge at lines 644-647: replace the clamp-and-
  pgm_read with `period = RatePeriod(tracks_[t].pattern[kPatCDIV])`. Drop
  the `if (cdiv_idx >= 15) cdiv_idx = 14;` clamp (the helper handles it).

**`controller/ui_pages/seq_track_page.cc`** (track-level RATE at cursor==1)

- Add a custom `OnClick` static method. If `cursor_ == 1`, read
  `tr->pattern[1]`, toggle bit 7 per the rules above, write back, return 1.
  Otherwise delegate to `UiPage::OnClick` (need to inspect ui_page.cc:72
  default — the default toggles `edit_mode_`; preserve that on non-RATE
  cursors).
- Wire `OnClick` into `event_handlers_` at line 51 (currently registers
  the default `OnClick`).
- `OnIncrement` is **unchanged** — `seq_track_page.cc` encoder is
  cursor-walk-only (lines 62-76 confirm), so tick editing on the track
  page comes via the pot only.
- Extend `OnPot` case 1 (line 91): if `tr->pattern[1]` has bit 7 set, map
  `value` → raw ticks: `0x80 | (2 + ((value * 94) >> 7))`, capped at 96,
  with snap-on-cross gating. Else existing preset-index mapping.
- Extend `UpdateScreen` case 1 (lines 146-151): if `v & 0x80`, format
  `" tNN"` via `UnsafeItoa` + `AlignRight`; else existing `kRateLabels`
  lookup. Clamp the preset path the same way (existing
  `if (i >= 15) i = 14;`).

**`controller/ui_pages/seq_steps_page.cc`** (per-step RATE at lockable==19)

- Add a custom `OnClick` static method (or hook into whatever the existing
  OnClick is — need to check current handler). If the cursored cell is
  `lockable == 19` and the stored value is non-zero (concrete rate),
  toggle bit 7 in the appropriate slot (track-default
  `tr.defaults[16+kSPRATE]` when no step held, or `step.steppage[kSPRATE]`
  when held). If value is 0 (inherit), no-op.
- Extend `OnPot` `lockable == 19` branch at lines 505-507: if current
  stored byte has bit 7 set, map pot to raw ticks via
  `0x80 | (2 + ((value * 94) >> 7))` capped at 96 (snap-on-cross); else
  existing `value >> 3` preset path.
- Extend the render at lines 890-900: replace `uint8_t r = v & 15;` (which
  corrupts raw-tick bytes) with explicit bit-7 branch. `v == 0` →
  `" trk"`; `v & 0x80` → `" tNN"` via `UnsafeItoa` + `AlignRight`; else
  preset label via `kRateLabels[(r-1)*4]`.

### Strip groove/swing residue

**`controller/sequencer.h`** — remove line 184 (`uint8_t swing;` field in
`SeqGlobal`).

**`controller/sequencer.cc`** — remove line 237 (`global_.swing = 0;` in
Reset/Init).

**`controller/multi.h`** — remove lines 26-27 (`clock_groove_template`,
`clock_groove_amount` fields) and lines 46-47
(`PRM_MULTI_CLOCK_GROOVE_TEMPLATE/_AMOUNT` enum entries). Renumber
subsequent enum entries if needed (check whether they're explicit values
or sequential — preserve any explicit numbering that other code depends
on).

**`controller/multi.cc`** — remove lines 38-39 (initializer zeros for the
two fields). Update line 188's condition
`if (address <= PRM_MULTI_CLOCK_GROOVE_AMOUNT)` to use the next remaining
param (likely `PRM_MULTI_CLOCK_BPM` or similar — read the enum order to
pick correctly).

**`controller/parameter.h`** — remove line 50 (`UNIT_GROOVE_TEMPLATE` enum
entry).

**`controller/parameter.cc`** — remove line 55
(`STR_RES_SWING, // UNIT_GROOVE_TEMPLATE` from the `unit_default_strings`
table — verify table name and that removing this entry doesn't shift
indices used elsewhere; if the table is indexed by `UNIT_*` enum value,
the removal must accompany the enum removal). Remove the two parameter
descriptors at lines 741-753 (PRM_MULTI_CLOCK_GROOVE_TEMPLATE and
PRM_MULTI_CLOCK_GROOVE_AMOUNT). Renumber the comment line `// 63`, `// 64`
if other parts of code or notes reference these comment numbers.

**`controller/resources.h`** — remove lines 79-84 (6 `lut_res_groove_*`
externs), line 142 (`STR_RES_GROOVE`), line 408 (`STR_RES_SWING`), lines
548-559 (12 `LUT_RES_GROOVE_*` defines). Renumber `LUT_RES_*` and
`STR_RES_*` defines if downstream code expects specific indices into the
pointer tables. Most likely they're just symbolic — verify by grepping
for raw numeric references.

**`controller/resources.cc`** — remove lines 82 (`str_res_groove`), 324-325
(`str_res_swing`, `str_res_shuffle`), 453 (`str_res_groove` pointer
entry), 719 (`str_res_swing` pointer entry), 936-956 (6 groove LUT
definitions), 997-1002 (6 LUT pointer-table entries). Also any other
strings referenced only by the dead groove descriptors: `str_res_push`,
`str_res_lag`, `str_res_human`, `str_res_monkey` — grep to confirm zero
non-groove uses, then remove their definitions and pointer-table entries
too.

**`controller/resources/strings.py`** — remove line 78 (`groove`), line
363 (`swing`), and any of `shuffle`, `push`, `lag`, `human`, `monkey`
confirmed unused elsewhere. These are the sources `resources.cc/h` are
regenerated from.

**`controller/resources/lookup_tables.py`** — remove lines 315-345 (the
`Groove templates` section: `ConvertGrooveTemplate` def and the 6 LUT
registrations).

**`controller/ui.cc`** — line 104 comment references "groove amount aka
swng" — update or remove the comment.

### Critical care points

1. **`STR_RES_MONKEY - STR_RES_SWING` arithmetic at `parameter.cc:744`.**
   This abused the string-resource numbering as a count of consecutive
   entries. Once those string entries are gone, the indices shift.
   **Verify no other code uses this pattern** before reordering /
   removing strings.
2. **Renumbering shock.** `STR_RES_*` and `LUT_RES_*` defines may be
   referenced in `resources.cc`'s pointer tables by position (the order
   of entries in the `str_table[]` and `lut_table[]` arrays must match
   the define values). Removing entries means either (a) renumber
   everything below and reorder both tables, or (b) leave the
   defines/slots in place as `STR_RES_RESERVED_N` placeholders pointing
   to a dummy string. Option (b) is safer and is what `resources.h:755`
   already does ("was PRM_MULTI_CLOCK_LATCH — removed, unused").
   **Prefer leaving renumbering for the resources-regeneration sweep**;
   this plan should just delete the unused content and leave defines as
   orphan placeholders unless the python regeneration is run.
3. **Toolchain regeneration.** `resources.h`, `resources.cc` are
   generated by the python scripts in `controller/resources/`. If the
   python sources are edited, the C++ outputs should be regenerated to
   match. If not regenerated, edit the C++ outputs by hand and accept
   they'll be out of sync with the python sources until next regen. Need
   to confirm which approach is in use in this repo (the python files
   have last-modified dates that may pre-date manual C++ edits).
4. **`Multi::SetValue` recompute trigger** at `multi.cc:188`:
   `if (address <= PRM_MULTI_CLOCK_GROOVE_AMOUNT)`. After removing those
   enum values, this needs to point at whatever's now the last
   clock-related param to preserve the "recompute tick duration when a
   clock param changes" behavior. Read the surrounding enum to identify
   the right new boundary.
5. **Snapshot/storage compatibility.** Patches saved before the strip
   include `SeqGlobal.swing` in their byte stream. `snapshot.cc:22` uses
   `offsetof(SeqTrack, shadow)` for SeqTrack persistence — that doesn't
   touch SeqGlobal. Check whether `SeqGlobal` is persisted anywhere
   (likely not — global state usually isn't). If it is, removing `swing`
   is a save-format break and old files won't load. **Verify before
   removing the field.**

### Functions/utilities to reuse

- `kRateValues[]` at `controller/sequencer.cc:25` — the preset table,
  unchanged.
- `kRateLabels[]` at `controller/ui_pages/seq_track_page.cc:30` (extern'd
  from `seq_steps_page.cc:33`) — the preset display labels, unchanged.
- `UnsafeItoa<uint8_t>(...)` and `AlignRight(...)` from
  `avrlib/string.h` — already used in both pages for numeric rendering;
  reuse for `"tNN"` formatting.
- `UiPage::OnClick` default at `controller/ui_pages/ui_page.cc:72` —
  preserve its `edit_mode_` toggle for non-RATE cursors. Override
  pattern: check cursor, handle RATE specially, else
  `return UiPage::OnClick();`.
- `ResolveStepByte(...)` at `controller/sequencer.cc:192` — unchanged;
  the bit-7 escape lives in the new `RatePeriod()` helper downstream of
  `ResolveStepByte`.

## Verification

End-to-end test plan, ordered to catch regressions early:

1. **Baseline flash/RAM measurement.**
   `./toolchain/size-squeeze.sh controller` before any changes. Record
   numbers.
2. **Strip-only build.** Apply the groove/swing strip in isolation first
   (no raw-tick changes yet). Build controller image:
   `touch controller/sequencer.cc && ./toolchain/build-squeeze.sh controller/makefile`.
   Confirm clean compile (no dangling references).
   `./toolchain/size-squeeze.sh controller` and verify flash reclaim of
   ~400–600 B and RAM reduction of 3 B.
3. **Patch-save round-trip.** Boot stripped firmware on hardware, save
   current patch to SD slot, power-cycle, reload. Confirm no corruption.
   (Only meaningful if `SeqGlobal` is actually persisted; verify in step
   "Critical care points" #5.)
4. **Raw-tick build.** Apply raw-tick changes on top. Build, size-check.
   Net should land at ~−100 to −300 B flash vs original baseline, −3 B
   RAM.
5. **Hardware functional checks:**
   - **Existing preset behavior unchanged.** Load a saved patch with
     various track-level RATEs and per-step RATE locks. Confirm playback
     timing is identical to pre-change.
   - **Track-level mode toggle.** Navigate to seq_track_page, cursor onto
     RATE cell (cursor==1). Click encoder. Display flips from e.g.
     `" 16 "` to `" t6 "`. Click again — flips back to nearest preset
     (`" 16 "`). Turn pot in raw mode — display shows `" t2 "` ..
     `"t32 "` linearly.
   - **Per-step mode toggle.** Hold a step on seq_steps_page,
     encoder-walk to RATE cell. With value `" trk"` (inherit), click
     should no-op. Set a concrete rate, then click — flips to `t<n>`.
     Click again — back to preset.
   - **Swung 16ths.** On a forward track with CDIV=16 (period 6), set
     step 0 RATE to `t7`, step 1 RATE to `t5`, etc. Confirm audible
     swing at the expected ratio. Verify it stays in sync with master
     clock over many bars (no drift).
   - **Polymeter regression.** Multiple tracks at different CDIVs, one
     with raw-tick RATEs. Confirm tracks remain independent and the
     modified track doesn't desync the others.
   - **Reset alignment.** Press MRST mid-pattern. Confirm tracks with
     raw-tick rates re-align cleanly with track step 0 firing at master
     tick 0 + period − 1 (matches `sequencer.cc:644-647` invariant).
   - **Save/load round-trip with raw-tick rates.** Patch with bit-7
     rates saved, reloaded, plays identically.
6. **Voicecard untouched.** No voicecard rebuild needed; confirm by not
   modifying any `voicecard/` source.

## Open questions to confirm during implementation

- Does `seq_steps_page.cc` have an existing `OnClick` that needs
  extending vs replacing? (Inspect `event_handlers_` table; if it
  registers the default `UiPage::OnClick`, replace with a custom one
  that checks `lockable == 19` and falls back to the default for other
  cells.)
- Is `SeqGlobal` persisted to SD or EEPROM? Determines whether
  `SeqGlobal.swing` removal is save-format-breaking. (`snapshot.cc`
  traces `kTrackPersistentSize = offsetof(SeqTrack, shadow)` for tracks;
  check for any separate SeqGlobal save path.)
- Are the python `resources/` sources still the source of truth, or has
  the C++ output drifted from them? Determines whether to regenerate or
  hand-edit.
- Does `parameter.cc:is_snapped` machinery extend cleanly to the RATE
  cell in raw mode, or does the snap-on-cross need a per-cell ad-hoc
  flag? (If `is_snapped` is gated on `unit`, RATE doesn't currently use
  the parameter system on these pages — may need a small page-local
  snapshot of "last pot value" or a per-cell `snapped` bit in the
  page's static state.)
````
