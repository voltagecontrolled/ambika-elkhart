# Sequencer

Topic spec for the elkhart 6-track polymetric step sequencer with per-step
parameter locks. Authoritative for sub-project status; cross-references
`voice_envelopes.md` for the envelope/LFO surface and `sequencer.h` for the
data structure layouts.

---

## Overview

Six independent tracks run in parallel, each driving one voicecard. Each
track has 8 steps (`SeqStep`), a per-track pattern config (`pattern[8]`),
voice defaults (`defaults[24]`) for the 24 lockable params, voice-wide
non-lockable settings (`config[29]`), and a transient playhead
(`shadow[5]`). Tracks advance independently — `pattern[kPatCDIV]` divides
the master tick to produce polymeter.

Every trigger is **stateless**: the controller resolves locks against
defaults into a 20-byte snapshot and ships it atomically alongside the
note/velocity. The voicecard applies the snapshot, then triggers the
note. There is no state carryover between steps.

---

## SPI snapshot protocol

New opcodes (in `common/protocol.h`):

```
COMMAND_NOTE_ON_WITH_SNAPSHOT          = 0x12
COMMAND_NOTE_ON_WITH_SNAPSHOT_LEGATO   = 0x13
```

Packet layout (25 bytes total):

```
byte 0       command (0x12 normal, 0x13 legato)
bytes 1..20  snapshot[20] = page1[0..7] || page2[0..7] || page3[0..3]
                page1: NOTE (skipped voicecard-side), WAVE1, PARA1, BLND,
                       RTIO, WAVE2, PARA2, FINE
                page2: E1DEC, TUN2, E2DEC, FIN2, E3DEC, E3REL_dead, NOIS, SUB
                       (TUN2 / FIN2 reclaimed dead E1REL / E2REL in round 4)
                page3: FREQ (patch 16), FAMT (patch 22), PAMT (patch 58),
                       WAVE (patch 11)  — added round 5b
bytes 21,22  note MSB, note LSB
byte 23      velocity << 1
```

The 20-byte snapshot omits `steppage[]` — step-behavior params
(PROB, SSUB, REPT, RATE, VEL, GLID, MINT, MDIR) are resolved on the
controller and never reach the voicecard.

### Voicecard handler

`voicecard/voicecard_rx.h` — when `command_ == 0x12 || 0x13`,
`Process()` collects 23 args. `DoLongCommand()` then walks
`kSnapshotAddrs[20]` (PROGMEM, in `voicecard_rx.cc`):

| snapshot[i] | source field    | patch addr |
|:-----------:|-----------------|:----------:|
| 0           | NOTE            | 0xff (skip — note is in args[20..21]) |
| 1           | WAVE1           | 0          |
| 2           | PARA1           | 1          |
| 3           | BLND            | 8          |
| 4           | RTIO            | 10         |
| 5           | WAVE2           | 4          |
| 6           | PARA2           | 5          |
| 7           | FINE            | 3          |
| 8           | E1DEC           | 25         |
| 9           | TUN2            | 6          |
| 10          | E2DEC           | 33         |
| 11          | FIN2            | 7          |
| 12          | E3DEC           | 41         |
| 13          | E3REL (dead)    | 0xff       |
| 14          | NOIS            | 13         |
| 15          | SUB             | 12         |
| 16          | FREQ            | 16         |
| 17          | FAMT            | 22         |
| 18          | PAMT            | 58         |
| 19          | WAVE (sub-osc)  | 11         |

`set_patch_data(addr, value)` is called for each non-`0xff` slot, then
`voice.Trigger(note, velocity, legato)` runs.

This mirrors `controller/part.cc` `PatchAddrToSeqField()`. Keep them in
sync — adding a new lockable param means updating both.

### Controller resolver

`Sequencer::FireStep()` in `controller/sequencer.cc`:

```cpp
// page1 + page2 (indices 0..15)
for (i = 0..15) {
  bool locked = step.lock_flags[i>>3] bit (i&7) is set;
  snapshot[i] = locked ? step.page1or2[i] : tr.defaults[i];
}
// page3 (indices 16..19)
for (i = 0..3) {
  bool locked = step.lock_flags[3] bit i is set;
  snapshot[16+i] = locked ? step.page3[i] : tr.defaults[24+i];
}
note     = snapshot[kP1NOTE];
velocity = ResolveStepByte(tr, step_index, kSPVEL) scaled by pattern[kPatVOL];
glid     = ResolveStepByte(tr, step_index, kSPGLID);
voicecard_tx.TriggerWithSnapshot(t, note<<7, velocity, glid?1:0, snapshot);
```

### SPI bandwidth

25 bytes per voice per trigger ≈ 100 µs at 2 MHz SPI. Six-voice worst case
≈ 600 µs. Fits within the trigger window at 240 BPM.

---

## Data structure recap

See `controller/sequencer.h` for authoritative definitions.

- **`SeqStep`** (34 B): `page1[8]` + `page2[8]` + `steppage[8]` + `page3[4]`
  + `lock_flags[4]` (32-bit bitfield: bits 0..7 = page1, 8..15 = page2,
  16..23 = steppage, 24..27 = page3, bits 28..31 reserved) + `step_flags`
  (bit 0 = trigger on/off) + `substep_bits`.
- **`SeqTrack`** (343 B): 8 × `SeqStep` (272 B) + `pattern[8]` (DIRN, CDIV,
  ROTA, LENG, SCAL, ROOT, BPCH, VOL) + `defaults[28]` + `config[29]` +
  `shadow[6]`.
  - `defaults[0..7]` = page1 defaults; `[8..15]` = page2; `[16..23]` = steppage;
    `[24..27]` = page3 (FREQ, FAMT, PAMT, WAVE).
- **`SeqGlobal`** (≥6 B): transport, hold_mode, swing, active_track,
  lock_page, held_step.

Index enums: `kP1*`, `kP2*`, `kP3*`, `kSP*`, `kCfg*`, `kPat*`, `kShdw*`.

---

## Sequencer Mode UI (S5 / `PAGE_PART_SEQUENCER`)

`controller/ui_pages/seq_steps_page.cc`. Buttons 1–8 are step triggers;
8 pots are the active page's lockable knobs.

- **Encoder turn** advances `cursor_` (0..23). The active page is
  `cursor_ >> 3` and is mirrored into `SeqGlobal.lock_page`. Crossing 8
  flips Voice1→Voice2; crossing 16 flips →Step. Stepping past either end
  spills out via `ui.ShowPageRelative(±1)` — encoder past cursor 23 lands
  on S6 (track settings), past cursor 0 lands on S3 (envelopes).
- **Pot turn**:
  - If any step button is currently held (`Ui::switch_held(i)`): write the
    lock value into that step's `pageX[]` slot, set `lock_flags` bit, and
    inhibit the next switch event for that step (so release doesn't toggle
    on/off). The dirty bit is also cleared on the next OnKey for that step.
  - Otherwise: write the track default (`defaults[]`).
- **Step toggle**: tap a step button to flip `step_flags & kStepFlagOn`.
  If a pot moved while held, the toggle is suppressed for that release.
- **`SWITCH_8 + encoder`** (existing framework convention): selects voice
  via `state_.active_part`. Note: SPEC names "S1+encoder=voice select"
  but the as-built handler is on SWITCH_8.
- **`SWITCH_1 + encoder`** (existing framework convention): ×8 encoder
  acceleration — effectively a full-page jump.

LCD layout:
- Line 0: 4-char abbreviations × 8 columns. Page tag at columns 36–37
  (`v1` / `v2` / `sp`). Cursor marker `>` precedes the focused knob's
  label on the active page.
- Line 1: 8 values aligned to the abbreviation columns. NOTE renders as
  3-char note name; everything else is right-aligned uint8 (with named
  enums for DIRN/CDIV/ROOT on the track page).

Knob layout (round 5b — `kSystemVersion 0x32`):

```
S5a / Step (cursor 0..7):
  top  note  vel   vamt  rate
  bot  subs  prob  glid  gtim

S5b / Voice 1 (cursor 8..15):
  top  nois  w1    pa1   tun2
  bot  mix   w2    pa2   fin2

S5c / Voice 2 (cursor 16..23):
  top  freq  fdec  famt  adec
  bot  pdec  pamt  sub   wave
```

`vamt` (cursor 2, config-mapped, patch addr 85): velocity → VCA mod slot 11
amount. Backed by `kCfgVELAMT = 19`.

`gtim` (cursor 7, config-mapped, virtual addr 203): portamento/glide time.
Routes to `VOICECARD_DATA_PART` offset 6 in `Part::SetValue` and `Touch` —
not to the Patch struct. (Patch addr 19 was wrong; portamento lives in the
voicecard `Part` struct.)

`subs` (cursor 4) is the merged SSUB+REPT bipolar cell. Center deadzone at
12 o'clock; CCW = repeats 8r..1r; CW = 1x..8x ratchets. Display: `Nr` /
`0` / `Nx` / `cus` (for unconstrained custom pattern). `mint`/`mdir`
removed from top-level cells — they live exclusively inside the substep
editor.

**Substep editor** (round 5b): entered by holding any step button and
clicking the encoder while cursor is on `subs`. Only enterable when that
step has SSUB ≠ 0 or REPT ≠ 0 — no-op for plain steps. Exit: second encoder
click.

Two gated modes:
- **Gated repeats** (SSUB=-2): `substep_bits` gates each of the REPT+1
  period-boundary fires. Bit 0 = initial fire; bits 1..REPT = re-fires.
  Entered from repeats (CCW zone) or plain REPT > 0 steps.
- **Gated ratchets** (`kStepFlagGated` + SSUB > 0): `substep_bits` gates
  each of the SSUB+1 within-period sub-triggers. Entered from ratchet steps
  (CW zone).

While editing:
- **Pot 0** (top-row, under `subs` label): count + mode. CCW/deadzone/CW
  mirrors S5a subs pot. Changing CCW↔CW switches between gated-repeat and
  gated-ratchet modes. A pickup guard (`substep_pot0_entry_`) absorbs the
  first ADC reading on entry so the stored value isn't overwritten.
- **Pot 1** (top-row `mint`): MINT — semitone walk per sub-trigger, 0..24.
  Labels: `off`, `m2`..`M7`, `8va`, `8m2`..`8va2`.
- **Pot 2** (top-row `mdir`): MDIR — direction; `up`/`dn`/`ud`/`rnd`.
- Step buttons toggle individual `substep_bits` slots; slots ≥ substep_count_
  are inactive (button is a no-op, LED dark).
- Screen line 0: `subs Nr` (or `Nx`/`cus`) | `MINT m3 ` | `MDIR up `.
  Line 1: `#`/`-` per active slot, blank for inactive.
- LEDs mirror `substep_bits` masked to `substep_count_`.

On entry, `substep_bits` is trimmed to the active range; if nothing survives
the trim (stale out-of-range bits from a prior session), all slots re-enable.

`FireStep(t, step_index, sub_idx)`: initial fire passes `sub_idx=0`; REPT
re-fires and ratchet sub-triggers pass the fire index (1-based). MINT × sub_idx
semitones are added to the base note, direction set by MDIR.

S5c `freq`/`famt`/`pamt`/`wave` are now fully per-step lockable (round 5b);
previously they were config-mapped.

Wave cells (`w1`, `w2`) use a 2-char abbr + 6-char value layout (offsets
1..2 and 3..8) so longer waveform names render without truncation.

Cells use a per-(page,cell) descriptor (`kCellLockable[]` +
`kCellPatchAddr[]`) so a single cell can target either a per-step
lockable byte or a voice-wide config byte. On Page 1 every cell is
lockable; `tun2` / `fin2` (lock indices 9 / 11) reclaim the dead
E1REL / E2REL slots in `defaults.page2`. On Page 2, `famt` / `pamt` /
`wave` are config-mapped — turning the knob writes through
`Part::SetValue` (patch addrs 22 / 58 / 11) so the voicecard sees
the change immediately; everything else on the page is lockable.

WAVE1 / WAVE2 / sub-`wave` cells render the waveform name string
(parallels S1a). Signed cells (`tun2` / `fin2` / `famt` / `pamt`)
display int8 values; the NOTE cell value is right-aligned (shifted
+1 col vs round 3).

Only `kP2E3REL` (lock idx 13) remains as a dead reserved slot in
`defaults.page2`.

LEDs: `LED_1..LED_8` lit dim for `step_flags & kStepFlagOn`; the trailing
playhead step is lit bright when transport is playing.

---

## Per-track Settings (S6 / `PAGE_PART`)

`controller/ui_pages/seq_track_page.cc`. 8 pots map directly into the
active track's `pattern[8]`. Encoder turn cycles to the next page in the
group.

```
top  DIRN CDIV ROTA LENG
bot  SCAL ROOT ---- VOL
```

Pot ranges (round 5a):
- DIRN: 0..3 (pot >> 5) — fwd / rev / pend / rnd
- CDIV: 0..7 (pot >> 4) — index into `kCDivValues[] = {1,2,3,4,6,8,12,16}`
- ROTA: 0..7 (pot >> 4)
- LENG: 1..8 ((pot >> 4) + 1)
- SCAL: 0..7 (pot >> 4) — index into `kScaleMasks[]`
        (chromatic / major / minor / dorian / mixolydian / pentaMaj /
         pentaMin / blues)
- ROOT: 0..11 (pot * 12 / 128) — semitone offset from C
- BPCH: retired (pot inhibited; `----` cell). Slot reserved for a
        future track-transpose on the Performance page.
- VOL: 0..255 (pot << 1) — multiplicative scale on resolved velocity in
       `FireStep`. VOL=255 is identity, VOL=0 mutes the track.

`PAGE_PART_ARPEGGIATOR` remains a stub in group 5 for future per-voice
default knob pages.

---

## Transport (S7 / `PAGE_MULTI`)

`MultiPage` (group 6, single page after round 4). PLAY/PAUS/RST on
`SWITCH_1`/`SWITCH_2`/`SWITCH_3`. Pot 0 sets BPM; pot 1 sets `swng`
(groove amount, `PRM_MULTI_CLOCK_GROOVE_AMOUNT`). Line 0 reads
`bpm xxx | swng xxx`.

`PAGE_MULTI_CLOCK` is now a vestigial registry entry — its data is
all `0xff` so the encoder skips it, but it stays in the registry
because `Ui::ShowPageRelative` uses it as the wraparound bound.
Pressing S7 a second time stays on `PAGE_MULTI`. Beat-repeat /
mute-solo / stutter is reserved for a future page in this group.

---

## Sub-project status

`kSystemVersion 0x32` on the controller, `0x31` on the voicecards.
Round 5b bumped both sides because the snapshot protocol extended from
16 → 20 bytes (voicecard `kSnapshotAddrs[20]`, controller `TriggerWithSnapshot`
loop). The substep overhaul and MINT/MDIR work that followed are controller-only
(no further protocol change).

### Done

| Surface                                              |
|------------------------------------------------------|
| `TRIGGER_WITH_SNAPSHOT` SPI command (`0x12`/`0x13`)  |
| Voicecard handler + `kSnapshotAddrs[16]` table       |
| Controller `TriggerWithSnapshot()` TX                |
| `Sequencer::FireStep()` lock resolution              |
| Sequencer Mode UI (3 lock pages on S5, encoder cycles cursor 0..23) |
| Per-track settings page (S6) — DIRN/CDIV/ROTA/LENG/SCAL/ROOT/BPCH/OLEV |
| Transport relocation S5 → S7                         |
| Encoder spill at lock-page boundary (cursor → previous/next page) |
| Page registry order so encoder walks S3 → S5 → S6 → S7 in button order |
| S2 + encoder = ×8 page-jump chord                    |
| Chaselight tracks the actually-fired step (kShdwLAST) — handles reverse / pendulum |
| WAVE/SUB pot ranges clamped to enum bounds (no more voicecard hangs) |
| Track-page pot scaling fixed for the 0..127 pot range |
| Lowercase labels by default; uppercased on cursor    |
| `E1RL`/`E2RL`/`E3RL` slot labels replaced with `----` |
| Renamed `E1DC`/`E2DC`/`E3DC` → `adec`/`fdec`/`pdec`  |
| Curve defaults 192 → 64 (range is 0..127)            |
| Lock-edit indexing fix (SR-index → step-index conversion in `OnPot`) |
| Page 1/2 cell-descriptor refactor (`kCellLockable` + `kCellPatchAddr` so a cell can be either lockable or config-mapped) |
| Lockable `tun2` / `fin2` via reclaimed `kP2E1REL` / `kP2E2REL` slots (lock indices 9 / 11; voicecard `kSnapshotAddrs[9]=6, [11]=7`) |
| Signed display + bipolar pot scaling for `tun2` / `fin2` (lockable) and `famt` / `pamt` (config) |
| Waveform name strings on WAVE1 / WAVE2 / sub-WAVE cells (parallels S1a) |
| `mix` on Page 1 bot1 (BLND lock confirmed audible on hardware) |
| `famt` / `pamt` / `wave` (env depths + sub-osc shape) surfaced inline on Page 2 (config-mapped) |
| S7 collapsed to single-page `PAGE_MULTI` with `swng` knob; `PAGE_MULTI_CLOCK` retired to vestigial all-`0xff` entry |
| Defaults: `kP1FINE 64 → 0`; `kP1RTIO` retitled (reserved for future linear FM) |
| Round 5a: env-depth range extended 0..63 → 0..127 unipolar (UI passthrough + voicecard VCA `<< 1` rescale) |
| Round 5a: S5a/b/c rebuilt — NOTE → S5c, NOIS → S5a, FREQ → S5b (config-mapped cutoff) |
| Round 5a: `wav1`/`wav2` → `w1`/`w2` cosmetic (2-char abbr + 6-char value) |
| Round 5a: `subs` merged SSUB+REPT bipolar cell on S5c (UI/storage; execution round 5b) |
| Round 5a: BPCH retired (pattern slot reserved); OLEV → VOL wired as velocity scale |
| Round 5a: SCAL quantize wired against `kScaleMasks[8]` LUT (chro/maj/min/dor/mix/pMa/pMi/blu) |
| Round 5a: `Sequencer::FireStep` resolves PROB / GLID / VEL+VOL / SCAL |
| Round 5a: S7 transport `swng` realigned under top2 pot |
| Round 5a-1: PROB rescaled 0..127 native, default 127, displays as `0%`..`100%` |
| Round 5a-1: VEL→VCA default depth 0 → 127 (was inert; also unmasks VOL) |
| Round 5a-1: BLND clamped 0..63 to skip the dead linear-FM range |
| Round 5a-1: SCAL labels leading-space pattern (`SCAL pMi` not `SCALpMi`) |
| Round 5b: lockable `freq` / `famt` / `pamt` / `wave` — `page3[4]` + `lock_flags[4]`, snapshot 16→20 bytes, both-sides reflash (0x32/0x31) |
| Round 5b: `Part::PatchAddrToSeqField` cases 16/22/58/11 → `tr.defaults[24+kP3*]` (previously `tr.config[]`) |
| Round 5b: RATE per-step CDIV override in `Clock()` |
| Round 5b: REPT period re-fire with `shadow[kShdwREPT]` countdown |
| Round 5b: SSUB ratchets — N+1 evenly-spaced sub-triggers per period, `sub_period=0` guard |
| Round 5b: S5a layout — `note`/`vel`/`vamt`/`rate` top, `subs`/`prob`/`glid`/`gtim` bot |
| Round 5b: `vamt` config-mapped (patch addr 85, `kCfgVELAMT=19`, mod slot 11 amount) |
| Round 5b: `gtim` portamento via virtual addr 203 → `VOICECARD_DATA_PART` offset 6 (was wrong `filter[1].cutoff` at addr 19) |
| Round 5b: substep editor — SSUB=-2 gates REPT period fires via `substep_bits`; pots 0/1/2; pot pickup guard; bits sanitization on entry |
| Round 5b: gated ratchets — `kStepFlagGated` + `substep_bits` gate in `Clock()` SSUB>0 path |
| Round 5b: `FireStep(sub_idx)` — MINT/MDIR note walk; up/dn/ud/rnd modes; MINT interval names 0..24; MDIR 4-mode pot |

### Open / next iteration

| Surface                                              | Notes |
|------------------------------------------------------|-------|
| Hardware verification of round 5b                    | Pending; controller `0x32` + all voicecards `0x31` must be reflashed (snapshot protocol changed). Substep editor, gated ratchets, MINT/MDIR walk all untested on hardware |
| Round 5b: S6b page (portamento + vel-mod settings)   | New sub-page in group 5: portamento (`kCfgSMTH`, virtual addr 203) + `vdst` (vel destination) + `vamt` (vel amount). `PAGE_PART_ARPEGGIATOR` stub is the natural slot |
| RATE/CDIV ratio display                              | User wants ratios (`1/4`, `1/3`, `1/2` … `2/1`) not raw indices. Requires `kNumTicksPerStep=12`, new period table `{3,4,6,8,9,12,18,24}`, display labels in `seq_track_page.cc` and `seq_steps_page.cc` |
| Encoder-click focused-edit display                   | Click outside substep editor is a no-op; needs full-row `page \| name value` layout + PROGMEM full-name table for 28 lockable params |
| Hold-step semantics polish                           | Any held step + pot turn writes lock today. Long-press detection + double-tap-to-clear not yet implemented |
| Round 5c: slot-based patch storage                   | Numbered slots + save button, no kits/patches abstraction. `tracks_[6]` + `global_` raw dump (~1.8 KB) per slot. `PAGE_LIBRARY` enum slot is the registry home. Voice copy/paste UX TBD |
| WAVE strip-aware LUT                                 | Pot reaches CZ stripped indices (6..14) → silence; build contiguous valid-set lookup |
| Shadow playhead + Voltage-Block / Elektron hold modes | 5 bytes/track reserved, not yet wired |
| Linear FM via BLND ≥ 64 + RTIO ratio LUT             | Voicecard side; queued for separate planning phase |
| Track relationships / mod matrix                     | 4 active globally; not started |
| Mutate-aware note resolver                           | `FireStep` passes raw `kP1NOTE` |

### Notes for next session

- **`FireStep` resolver is the central place for step-behavior work.** PROB
  becomes a probabilistic skip; REPT a small loop around `voicecard_tx`;
  RATE multiplies the per-track tick period; MINT/MDIR transforms `note`
  before the snapshot is built; GLID flips bit 0 of the SPI command.
- **The `ParameterEditor` focused-edit pattern** (lines 162–186 of
  `parameter_editor.cc`) is the reference for the click-to-edit display
  on the seq pages. Both layouts use the same 19/20 column split.
- **`Ui::switch_held(i)` is bit-0 immediate**, not full-debounce. It
  reads the most recent ADC sample's bit, so the gesture catches even
  quick press+turn. Keep this in mind when adding new chord patterns.
- **Page traversal is enum-order driven.** Adding new pages requires
  inserting them at the right enum slot AND placing the registry entry
  at the matching array index — see `ui.h` `UiPageNumber` ordering.
