# Elkhart Control Map

Per-page truth table for pots, buttons, and encoder gestures across the
controller UI. Authoritative for what controls exist where. **Extracted
from `controller/ui.cc` `page_registry[]`, `controller/parameter.cc`
parameter table, and `controller/ui_pages/*.{h,cc}` handlers.**

This document is the source of truth for `docs/wiki/MANUAL.md` — when
the manual and this map disagree, the map wins (and the manual gets a
fix).

> Last extracted against master at `58a240d`. Re-extract whenever
> page_registry, parameter.cc, or any ui_page handler changes.
>
> **Switch numbering convention.** User-facing labels `S1..S8` and code
> identifiers `SWITCH_1..SWITCH_8` use the same numbering left → right
> (so `SWITCH_1 = S1`, `SWITCH_8 = S8`). The hardware shift-register
> *poll order* is reversed (`SR-index 0 = SWITCH_8`); that mapping is
> applied inside `Ui::Poll` via `control = SWITCH_8 - i` and shouldn't
> appear in feature-level docs.
>
> **Cell labels** are derived from `controller/resources.cc` `prog_char
> str_res_*` strings, truncated to the cell width (4 chars for normal
> cells, 2 chars for wave cells in S5b/S5c). Labels render lowercase by
> default and uppercase when the cursor is on that cell. Trailing spaces
> are part of the displayed label where shown (e.g. `mix `, `amp `,
> `vol `).

---

## Page registry — group → button mapping

From `default_most_recent_page_in_group[]` and `page_registry[]`:

| Button | Group | Pages in group (a → b cycle order)            | Handler                          |
|--------|-------|-----------------------------------------------|----------------------------------|
| `S1`   | 0     | `PAGE_OSCILLATORS` (S1a) → `PAGE_MIXER` (S1b) | `ParameterEditor`                |
| `S2`   | 1     | `PAGE_FILTER` (single page)                   | `ParameterEditor`                |
| `S3`   | 2     | `PAGE_ENV_LFO` (S3a) → `PAGE_VOICE_LFO` (S3b) | `ParameterEditor`                |
| `S4`   | 2     | (shares group 2 with S3)                      | `ParameterEditor`                |
| `S5`   | 4     | `PAGE_PART_SEQUENCER` (3 lock pages, encoder-cycled) | `SeqStepsPage`            |
| `S6`   | 5     | `PAGE_PART` (S6a) → `PAGE_SEQ_MIXER` (S6b)    | `SeqTrackPage` / `SeqMixerPage`  |
| `S7`   | 6     | `PAGE_MULTI` (single — `PAGE_MULTI_CLOCK` vestigial) | `MultiPage`               |
| `S8`   | 7,8   | `PAGE_OS_INFO` → `PAGE_SYSTEM_SETTINGS`       | `OsInfoPage` / `ParameterEditor` |

Vestigial registry entries (kept as ShowPageRelative wraparound bounds):
`PAGE_MULTI_CLOCK`, `PAGE_PERFORMANCE`, `PAGE_KNOB_ASSIGN`, `PAGE_LIBRARY`,
`PAGE_VERSION_MANAGER`, `PAGE_MODULATIONS`. All carry `0xff` pot data so
the encoder skips them.

---

## Global gestures

| Gesture                  | Effect                                                  |
|--------------------------|---------------------------------------------------------|
| Encoder turn             | Walk cursor through current page's parameters           |
| Encoder click            | Focused-edit on current parameter (ParameterEditor pages) |
| `S1` + encoder turn      | Voice select — cycle the active track                   |
| `S2` + encoder turn      | ×8 page-jump multiplier — skip whole page groups        |
| `S8` + encoder turn      | ×8 page-jump multiplier (same as `S2`)                  |
| `S8` hold                | System-wide SHIFT prefix (Copy / Swap / Paste / Snapshot in `Ui::Poll`) |
| `S5` (button-press)      | Toggle between sequencer mode and normal mode (symmetric) |

---

## S1a — `PAGE_OSCILLATORS` (Group 0, top half)

**Handler:** `ParameterEditor`  ·  **Param indices:** `{0, 1, 2, 3, 4, 5, 6, 7}`

Cell labels are the `short_name` resource string (`controller/resources.cc`)
truncated to the 4-char cell width by `Parameter::PrintName`.

| Slot  | Idx | Label  | short_name str         | Name           | Range           | Notes |
|-------|-----|--------|------------------------|----------------|-----------------|-------|
| top1  | 0   | `wave` | `str_res_waveform`     | Osc 1 waveform | 0..43 enum      | CZ filter-sim variants 6..14 strip pending |
| top2  | 1   | `para` | `str_res_parameter`    | Osc 1 parameter| 0..127          | Algorithm-dependent (PWM, formant, FM index, fold depth, …) |
| top3  | 2   | `rang` | `str_res_range`        | Osc 1 range    | -24..+24 semis  | Coarse pitch offset |
| top4  | 3   | `tune` | `str_res_tune`         | Osc 1 detune   | -64..+64 cents  | Fine pitch offset |
| bot1  | 4   | `wave` | `str_res_waveform`     | Osc 2 waveform | 0..43 enum      | |
| bot2  | 5   | `para` | `str_res_parameter`    | Osc 2 parameter| 0..127          | |
| bot3  | 6   | `rang` | `str_res_range`        | Osc 2 range    | -24..+24 semis  | |
| bot4  | 7   | `tune` | `str_res_tune`         | Osc 2 detune   | -64..+64 cents  | |

## S1b — `PAGE_MIXER` (Group 0, bottom half)

**Handler:** `ParameterEditor`  ·  **Param indices:** `{8, 13, 12, 11, 9, 10, 14, 15}`

| Slot  | Idx | Label  | short_name str         | Name              | Range  | Notes |
|-------|-----|--------|------------------------|-------------------|--------|-------|
| top1  | 8   | `mix ` | `str_res_mix`          | Osc balance / BLND| 0..63  | BLND ≥ 64 FM zone clamped in default builds |
| top2  | 13  | `nois` | `str_res_noise`        | Noise level       | 0..63  | |
| top3  | 12  | `sub ` | `str_res_sub_osc_`     | Sub-osc level     | 0..63  | "sub osc." truncated |
| top4  | 11  | `wave` | `str_res_waveform`     | Sub-osc waveform  | 0..10  | 6 traditional + 5 transient |
| bot1  | 9   | `xmod` | `str_res_xmod`         | X-mod operator    | 0..N   | Cross-mod type select |
| bot2  | 10  | `amnt` | `str_res_amnt`         | X-mod amount      | 0..63  | |
| bot3  | 14  | `fuzz` | `str_res_fuzz`         | Fuzz / distortion | 0..63  | |
| bot4  | 15  | `crsh` | `str_res_crsh`         | Crush / bitcrush  | 0..31  | |

## S2 — `PAGE_FILTER` (Group 1)

**Handler:** `ParameterEditor`  ·  **Param indices:** `{16, 17, 0xff, 18, 22, 0xff, 0xff, 0xff}`

| Slot  | Idx  | Label  | short_name str        | Name           | Range   | Notes |
|-------|------|--------|-----------------------|----------------|---------|-------|
| top1  | 16   | `freq` | `str_res_frequency`   | Cutoff         | 0..127  | Lockable per-step on S5c |
| top2  | 17   | `reso` | `str_res_resonance`   | Resonance      | 0..63   | |
| top3  | —    | —      | —                     | (unused)       | —       | |
| top4  | 18   | `mode` | `str_res_mode`        | Filter mode    | 0..3    | LP / BP / HP / Notch |
| bot1  | 22   | `env2` | `str_res_env2Tvcf`    | Env 2 → cutoff | 0..63   | "env2~vcf" truncated. Lockable on S5c as `famt` |
| bot2  | —    | —      | —                     | (unused)       | —       | |
| bot3  | —    | —      | —                     | (unused)       | —       | |
| bot4  | —    | —      | —                     | (unused)       | —       | |

`DRIV` and `BITS` referenced in the previous MANUAL are **not present on
this page** in the current registry — they live on S1b (`fuzz` / `crsh`).
MANUAL needs correction.

## S3a — `PAGE_ENV_LFO` (Group 2, top half)

**Handler:** `ParameterEditor`  ·  **Param indices:** `{24, 25, 26, 27, 28, 75, 76, 77}`

| Slot  | Idx | Label  | short_name str    | Name           | Range  | Notes |
|-------|-----|--------|-------------------|----------------|--------|-------|
| top1  | 24  | `rise` | `str_res_rise`    | Env 1 rise     | 0..127 | Attack rate |
| top2  | 25  | `fall` | `str_res_fall`    | Env 1 fall     | 0..127 | Decay/release rate (= `adec` lockable on S5c) |
| top3  | 26  | `curv` | `str_res_curv`    | Env 1 curve    | 0..127 | 0=linear, 127=expo |
| top4  | 27  | `amp ` | `str_res_amp`     | Env 1 → VCA    | 0..127 | Depth (virtual addr 200). 3-char string + space pad |
| bot1  | 28  | `rise` | `str_res_rise`    | Env 2 rise     | 0..127 | |
| bot2  | 75  | `fall` | `str_res_fall`    | Env 2 fall     | 0..127 | (= `fdec` lockable on S5c) |
| bot3  | 76  | `curv` | `str_res_curv`    | Env 2 curve    | 0..127 | |
| bot4  | 77  | `flt ` | `str_res_flt`     | Env 2 → cutoff | 0..127 | Depth (virtual addr 201; same value as `famt` on S5c) |

## S3b — `PAGE_VOICE_LFO` (Group 2, bottom half / also reachable via S4)

**Handler:** `ParameterEditor`  ·  **Param indices:** `{78, 79, 80, 81, 32, 33, 82, 83}`

| Slot  | Idx | Label  | short_name str    | Name              | Range    | Notes |
|-------|-----|--------|-------------------|-------------------|----------|-------|
| top1  | 78  | `rise` | `str_res_rise`    | Env 3 rise        | 0..127   | |
| top2  | 79  | `fall` | `str_res_fall`    | Env 3 fall        | 0..127   | (= `pdec` lockable on S5c) |
| top3  | 80  | `curv` | `str_res_curv`    | Env 3 curve       | 0..127   | |
| top4  | 81  | `pitc` | `str_res_pch`     | Env 3 → pitch     | 0..127   | Depth (virtual addr 202; = `pamt` on S5c). String literal is `"pitc"` despite resource enum being `pch` |
| bot1  | 32  | `rate` | `str_res_rate`    | LFO 4 rate        | 0..127   | Tempo-sync extension pending (#12) |
| bot2  | 33  | `wave` | `str_res_waveform`| LFO 4 waveform    | 0..N enum| Sine/tri/sqr/ramp/S&H |
| bot3  | 82  | `dest` | `str_res_dest`    | LFO 4 destination | 0..N enum| |
| bot4  | 83  | `dept` | `str_res_dept`    | LFO 4 depth       | -63..+63 | Signed |

## S4 — Mod matrix (planned; currently `PAGE_MODULATIONS` placeholder)

`PAGE_MODULATIONS` exists in the registry with all-`0xff` pot data — the
encoder skips it. Layout for v4.0 is being scoped on issue #11.

---

## S5a — `PAGE_PART_SEQUENCER` Step page (cursor 0..7)

**Handler:** `SeqStepsPage`  ·  Source: `controller/ui_pages/seq_steps_page.cc`

| Slot  | Cursor | Abbrev | Name                  | Lockable? | Notes |
|-------|--------|--------|-----------------------|-----------|-------|
| top1  | 0      | `note` | Step note             | yes       | Quantized to track scale |
| top2  | 1      | `vel`  | Step velocity         | yes       | 0..127 |
| top3  | 2      | `vamt` | Velocity → VCA amount | config    | Patch addr 85, mod slot 11 amount; voice-wide |
| top4  | 3      | `rate` | Step rate             | yes       | Per-step CDIV override |
| bot1  | 4      | `subs` | Sub-steps / mode      | yes       | Bipolar SSUB+REPT cell. CCW = repeats `8r..1r`, deadzone, CW = ratchets `1x..8x`. Display: `Nr` / `0` / `Nx` / `cus` |
| bot2  | 5      | `prob` | Probability           | yes       | 0..127 → 0%..100% |
| bot3  | 6      | `glid` | Per-step glide time   | yes       | 0..127 portamento time pushed to voicecard part offset 6 each fire (replaced binary legato gate) |
| bot4  | 7      | `sfx ` | Step modifier (SMOD)  | per-step  | Per-step only, no track default. Packed into `step_flags` bits 2..5. Values: `none/skip/fwd/rev/dir/rjmp/jmp1..jmp8`. Gated by PROB roll. Render `----` when no step held |

## S5b — `PAGE_PART_SEQUENCER` Voice 1 page (cursor 8..15)

| Slot  | Cursor | Abbrev | Name             | Lockable? | Notes |
|-------|--------|--------|------------------|-----------|-------|
| top1  | 8      | `nois` | Noise mix        | yes       | |
| top2  | 9      | `w1`   | Osc 1 wave       | yes       | 2-char abbr + 6-char value (long wave names) |
| top3  | 10     | `pa1`  | Osc 1 PARA       | yes       | |
| top4  | 11     | `tun2` | Osc 2 coarse tune| yes       | Signed; reclaims dead E1 release slot |
| bot1  | 12     | `mix`  | BLND             | yes       | Bipolar |
| bot2  | 13     | `w2`   | Osc 2 wave       | yes       | |
| bot3  | 14     | `pa2`  | Osc 2 PARA       | yes       | |
| bot4  | 15     | `fin2` | Osc 2 fine tune  | yes       | Signed; reclaims dead E2 release slot |

## S5c — `PAGE_PART_SEQUENCER` Voice 2 page (cursor 16..23)

| Slot  | Cursor | Abbrev | Name              | Lockable? | Notes |
|-------|--------|--------|-------------------|-----------|-------|
| top1  | 16     | `freq` | Filter cutoff     | yes (round 5b) | |
| top2  | 17     | `fdec` | Env 2 fall        | yes       | |
| top3  | 18     | `famt` | Env 2 → cutoff    | yes (round 5b) | Signed |
| top4  | 19     | `adec` | Env 1 fall        | yes       | |
| bot1  | 20     | `pdec` | Env 3 fall        | yes       | |
| bot2  | 21     | `pamt` | Env 3 → pitch     | yes (round 5b) | Signed |
| bot3  | 22     | `sub`  | Sub-osc level     | yes       | |
| bot4  | 23     | `wave` | Sub-osc shape     | yes (round 5b) | 11-shape palette |

### S5 cursor + lock-edit semantics

- **Encoder turn** advances `cursor_` 0..23. Active page = `cursor_ >> 3`.
- **Crossing 8** flips Voice1; **crossing 16** flips Voice2.
- **Encoder past 23** spills out via `ShowPageRelative(+1)` → S6.
- **Encoder past 0** spills out via `ShowPageRelative(-1)` → S3.
- **Pot turn while no step held:** writes track default (`tr.defaults[]`).
- **Pot turn while step held:** writes lock value into that step's
  `pageX[]` slot, sets `lock_flags` bit, inhibits next switch event.
- **Step tap:** toggle `step_flags & kStepFlagOn` (suppressed if pot
  moved while held).
- **`S1`+encoder, `S2`+encoder:** ×8 multiplier. `SeqStepsPage::OnIncrement`
  short-circuits when `|increment| >= 8` and pages out immediately.

### S5 substep editor

**Entry:** hold any step button + click encoder while cursor is on `subs`.
Only enterable when that step has SSUB ≠ 0 or REPT ≠ 0 (no-op for plain
steps). **Exit:** second encoder click.

| Pot   | Function in editor                                 |
|-------|----------------------------------------------------|
| pot 0 | Count + mode (mirrors S5a `subs` knob; pickup guard absorbs first ADC reading) |
| pot 1 | MINT — semitone walk per sub-trigger, 0..24 (`off`, `m2`..`M7`, `8va`, `8m2`..`8va2`) |
| pot 2 | MDIR — direction: `up` / `dn` / `ud` / `rnd`       |
| 3..7  | Inert                                              |

Step buttons toggle `substep_bits[i]`. Slots ≥ active count are inactive
(button no-op, LED dark). On entry, `substep_bits` is masked to active
range and re-enabled if nothing survives the trim.

---

## S6a — `PAGE_PART` Per-track settings

**Handler:** `SeqTrackPage`  ·  All pots write into the active track's `pattern[]`.

Labels from `kAbbr[] PROGMEM = "dirncdivrotalengscalroot----vol "` (lowercase by default, uppercased on cursor).

| Slot  | Label  | Name           | Range  | Mapping                  |
|-------|--------|----------------|--------|--------------------------|
| top1  | `dirn` | Direction      | 0..3   | `pot >> 5`. Values from `kDirnLabels`: ` fwd`, ` rev`, `pend`, `rnd ` |
| top2  | `cdiv` | Clock division | 0..7   | `pot >> 4` → index into `kCDivValues[] = {1,2,3,4,6,8,12,16}` (pending #14 ratio rework) |
| top3  | `rota` | Rotate         | 0..7   | `pot >> 4` |
| top4  | `leng` | Pattern length | 1..8   | `(pot >> 4) + 1` |
| bot1  | `scal` | Scale          | 0..7   | `pot >> 4` → `kScaleMasks[]`. Labels: ` chr`, ` maj`, ` min`, ` dor`, ` mix`, ` pMa`, ` pMi`, ` blu` |
| bot2  | `root` | Scale root     | 0..11  | `pot * 12 / 128` → ` C  `..` B  ` |
| bot3  | `----` | (BPCH retired) | —      | Pot inhibited; cell renders `----`. Reserved for track transpose. |
| bot4  | `vol ` | Track volume   | 0..255 | `pot << 1`. Multiplicative scale on resolved velocity. |

**Encoder:** walks 8 cells; spills to neighboring page at boundaries.

## S6b — `PAGE_SEQ_MIXER` Performance mixer

**Handler:** `SeqMixerPage`  ·  Source: `controller/ui_pages/seq_mixer_page.{h,cc}`

| Slot  | Cell idx | Label  | Function                                       |
|-------|----------|--------|------------------------------------------------|
| top1  | 0        | `v1`   | V1 volume (pickup-catch → `pattern[kPatVOL]`, value `>> 1`) |
| top2  | 1        | `v2`   | V2 volume                                      |
| top3  | 2        | `v3`   | V3 volume                                      |
| top4  | 3        | `mode` | Mode display: `MT-S` / `MT-A` / `SOLO` (read-only — S7 cycles) |
| bot1  | 4        | `v4`   | V4 volume                                      |
| bot2  | 5        | `v5`   | V5 volume                                      |
| bot3  | 6        | `v6`   | V6 volume                                      |
| bot4  | 7        | `clr`  | Hint cell — renders `clr  unmt` (read-only — S8 unmutes all) |

| Button | No `S7` hold                  | `S7` held                          |
|--------|-------------------------------|------------------------------------|
| `S1`–`S6` | Toggle active mode bit for voice 1..6 | Queue toggle into `pending_toggle_` (LEDs blink red) |
| `S7`   | Cycle mode `MT-S → MT-A → SOLO → MT-S` | If `pending_toggle_ != 0`: XOR queued bits into active mode (and **don't** cycle); else: cycle |
| `S8`   | Clear all bit sets (unmute-all) | (S8 is system-wide SHIFT — held interactions go to `Ui::Poll`) |

Modes:

| Mode  | Bit semantics  | On bit 0→1 toggle of sounding voice |
|-------|----------------|--------------------------------------|
| MT-S  | sequencer mute | none — current note's envelope completes |
| MT-A  | audio mute     | `voicecard_tx.Kill(v)` — instant cut |
| SOLO  | solo mask      | non-solo voices that lost audibility get `Kill` |

LEDs: V1..V6 lit = audible in current mode (or solo'd in SOLO); blink red
during S7-hold queue. S7 LED encodes mode (off / dim red / bright red).

State is **transient file-static** — not saved with patch slot.

## S7 — `PAGE_MULTI` Transport

**Handler:** `MultiPage`  ·  Source: `controller/ui_pages/multi_page.{h,cc}`

| Slot  | Abbrev | Function                              |
|-------|--------|---------------------------------------|
| top1  | `bpm`  | Master tempo (40..240 BPM)            |
| top2  | `swng` | Groove amount (`PRM_MULTI_CLOCK_GROOVE_AMOUNT`) |
| top3  | —      | Unused (candidate slot for #9 `mrst`, #19 master transpose, or #20 master scale) |
| top4  | —      | Unused                                |
| bot1..4| —     | Unused                                |

| Button | Function                                                  |
|--------|-----------------------------------------------------------|
| `S1`   | PLAY                                                      |
| `S2`   | PAUS (toggle pause ↔ resume)                              |
| `S3`   | RST  (all playheads → step 0; transport state preserved)  |
| `S4`   | STOP — single tap = `Pause` + `Reset`. Double-tap < 300 ms = panic (`Pause` + `Reset` + `Kill` all 6 voices). LED red during second-tap window. |
| `S5`–`S7`| Unused (candidates for #21 mixer shortcut, #22 jump gesture, etc.) |
| `S8`   | EXIT — return to most recent non-system page              |

LEDs:

- Status LED: bright while playing, dim while paused.
- `S1` (PLAY): bright while playing.
- `S2` (PAUS): bright while paused.
- `S4` (STOP): red flash during double-tap window.
- `S8` (EXIT): always lit.

Display:

```
Line 0:  bpm XXX | swng XXX
Line 1:  play  paus  rst   stop                    exit
```

## S8 — `PAGE_OS_INFO` (and `PAGE_SYSTEM_SETTINGS`)

**Handler:** `OsInfoPage`  ·  Source: `controller/ui_pages/os_info_page.{h,cc}`

Read-only display showing controller + per-voicecard firmware versions.
Patch saving (issue #10) will move firmware-upgrade to S8b and add a
`SeqStoragePage` on S8a.

| Button | Function (current)                                     |
|--------|--------------------------------------------------------|
| `S1`   | Upload controller firmware (if `/AMBIKA.BIN` present)  |
| `S4`   | Upload voicecard firmware (if `/VOICE#.BIN` present for selected slot) |
| `S8`   | EXIT                                                   |

**Encoder:** cycles `active_control_` 0..5 to select voicecard slot for
flashing.

**`PAGE_SYSTEM_SETTINGS`** (`ParameterEditor`, indices `{66, 67, 71, 72, 68, 69, 0xff, 70}`):

| Slot  | Idx | Label  | short_name str          | Name                     | Range  |
|-------|-----|--------|-------------------------|--------------------------|--------|
| top1  | 66  | `inpt` | `str_res_inpt_filter`   | MIDI input filter        | 0..15  |
| top2  | 67  | `outp` | `str_res_outp_mode`     | MIDI output mode         | 0..2   |
| top3  | 71  | `leds` | `str_res_leds`          | Voicecard LED enable     | 0..1   |
| top4  | 72  | `swap` | `str_res_swap_colors`   | Swap LED colors          | 0..1   |
| bot1  | 68  | `help` | `str_res_help`          | Show help text           | 0..1   |
| bot2  | 69  | `snap` | `str_res_snap`          | Snap to knob on load     | 0..1   |
| bot3  | —   | —      | —                       | (unused)                 | —      |
| bot4  | 70  | `auto` | `str_res_auto_backup`   | Auto backup to SD        | 0..1   |

---

## Cell display convention

All ParameterEditor + sequencer pages share a 4-cell × 2-row layout, ten
characters per cell:

```
Row 0:  [cell 0  abbr|val] | [cell 1  abbr|val] | [cell 2  abbr|val] | [cell 3  abbr|val]
Row 1:  [cell 4  abbr|val] | [cell 5  abbr|val] | [cell 6  abbr|val] | [cell 7  abbr|val]
```

- 4-char abbreviation (lowercase by default; uppercased on cursor).
- 4-char value (right-aligned for numeric, labeled for enums).
- Wave cells (S5b/S5c) use 2-char abbr + 6-char value to fit long wave
  names.

Cursor is shown by uppercasing the focused cell's abbreviation — there is
no separate cursor glyph on most pages (the sequencer page uses `>` as
documented in `docs/planning/sequencer.md`).

---

## Known drift between MANUAL and code (snapshot — fixes pending #17)

These are the divergences the verification pass will need to address:

1. **S2 Filter** — MANUAL documents `DRIV` and `BITS` on S2. Both live
   on **S1b Mixer** (`fuzz` / `crsh`). MANUAL wrong.
2. **S2 Filter** — MANUAL documents `TYPE` selecting LP/BP/HP. Code
   exposes 0..3 (LP/BP/HP/Notch). MANUAL incomplete.
3. **MANUAL S5b** — references `WAVE1`, `WAVE2`, etc. Code uses 2-char
   abbreviations `w1`/`w2`. Style choice — adopt code abbreviations.
4. **MANUAL S6a** — describes BPCH as a control. Retired in round 5a;
   pot inhibited. MANUAL needs update.
5. **MANUAL S7 mrst** — placed on `S8` button. Issue #9 says pot top2-or-3.
   Decision: **follow #9**.
6. **MANUAL S8 patch slots** — pre-emptively documents issue #10 layout
   that hasn't shipped. Mark as planned, not as-built.
7. **MANUAL S5a `vamt` / `gtim`** — described as voice-wide config in the
   manual (correct), but the lock semantics need clarification: lock
   indices are `0xff` (not lockable per-step despite living on S5a).

---

## Re-extraction process

When the controller UI changes:

1. Re-read `controller/ui.cc` `page_registry[]` and re-derive any moved
   parameter indices.
2. For ParameterEditor pages, dereference indices through
   `controller/parameter.cc` `parameters[]` for abbrev / range.
3. For custom-handler pages, re-read the matching
   `controller/ui_pages/*.cc` `OnPot` / `OnKey` / `OnIncrement` /
   `OnClick` methods.
4. Update this doc.
5. Open a follow-up issue to align `docs/wiki/MANUAL.md`.
