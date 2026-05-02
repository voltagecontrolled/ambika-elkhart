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
defaults into a 16-byte snapshot and ships it atomically alongside the
note/velocity. The voicecard applies the snapshot, then triggers the
note. There is no state carryover between steps.

---

## SPI snapshot protocol

New opcodes (in `common/protocol.h`):

```
COMMAND_NOTE_ON_WITH_SNAPSHOT          = 0x12
COMMAND_NOTE_ON_WITH_SNAPSHOT_LEGATO   = 0x13
```

Packet layout (21 bytes total):

```
byte 0       command (0x12 normal, 0x13 legato)
bytes 1..16  snapshot[16] = page1[0..7] || page2[0..7]
                page1: NOTE (skipped voicecard-side), WAVE1, PARA1, BLND,
                       RTIO, WAVE2, PARA2, FINE
                page2: E1DEC, E1REL_dead, E2DEC, E2REL_dead, E3DEC,
                       E3REL_dead, NOIS, SUB
bytes 17,18  note MSB, note LSB
byte 19      velocity << 1
```

The 16-byte snapshot omits `steppage[]` — those step-behavior params
(PROB, SSUB, REPT, RATE, VEL, GLID, MINT, MDIR) are resolved on the
controller and never reach the voicecard.

### Voicecard handler

`voicecard/voicecard_rx.h` — when `command_ == 0x12 || 0x13`,
`Process()` collects 19 args. `DoLongCommand()` then walks
`kSnapshotAddrs[16]` (PROGMEM, in `voicecard_rx.cc`):

| snapshot[i] | source field    | patch addr |
|:-----------:|-----------------|:----------:|
| 0           | NOTE            | 0xff (skip — note is in args[16..17]) |
| 1           | WAVE1           | 0          |
| 2           | PARA1           | 1          |
| 3           | BLND            | 8          |
| 4           | RTIO            | 10         |
| 5           | WAVE2           | 4          |
| 6           | PARA2           | 5          |
| 7           | FINE            | 3          |
| 8           | E1DEC           | 25         |
| 9           | E1REL (dead)    | 0xff       |
| 10          | E2DEC           | 33         |
| 11          | E2REL (dead)    | 0xff       |
| 12          | E3DEC           | 41         |
| 13          | E3REL (dead)    | 0xff       |
| 14          | NOIS            | 13         |
| 15          | SUB             | 12         |

`set_patch_data(addr, value)` is called for each non-`0xff` slot, then
`voice.Trigger(note, velocity, legato)` runs.

This mirrors `controller/part.cc` `PatchAddrToSeqField()`. Keep them in
sync — adding a new lockable param means updating both.

### Controller resolver

`Sequencer::FireStep()` in `controller/sequencer.cc`:

```cpp
for (i = 0..15) {
  bool locked = step.lock_flags bit i is set;
  snapshot[i] = locked ? step.pageX[i] : tr.defaults[i];
}
note     = snapshot[kP1NOTE];   // resolved from page1 lock or default
velocity = (lock bit 16+kSPVEL set) ? step.steppage[kSPVEL]
                                    : tr.defaults[16+kSPVEL];
voicecard_tx.TriggerWithSnapshot(t, note<<7, velocity, 0, snapshot);
```

GLID/legato is currently hardcoded to 0; it becomes the `command_ & 1`
bit when the GLID step-behavior parameter ships.

Mutate (MINT/MDIR), track transpose, and per-step rate (RATE) are all
unimplemented — `note` is the raw lock-or-default value for now.

### SPI bandwidth

21 bytes per voice per trigger ≈ 84 µs at 2 MHz SPI. Six-voice worst case
≈ 500 µs. Fits comfortably in the trigger window even at 240 BPM/8×
ratchet (~7.8 ms between triggers).

---

## Data structure recap

See `controller/sequencer.h` for authoritative definitions.

- **`SeqStep`** (29 B): `page1[8]` + `page2[8]` + `steppage[8]` + `lock_flags[3]`
  (24-bit bitfield, bit `N = page_index*8 + param_index`) + `step_flags`
  (bit 0 = trigger on/off) + `substep_bits` (8 ratchets).
- **`SeqTrack`** (298 B): 8 × `SeqStep` (232 B) + `pattern[8]` (DIRN, CDIV,
  ROTA, LENG, SCAL, ROOT, BPCH, OLEV) + `defaults[24]` + `config[29]` +
  `shadow[6]` (the 6th byte added in round 2 = `kShdwLAST`, the
  most-recently-fired step for chaselight).
- **`SeqGlobal`** (≥6 B): transport, hold_mode, swing, active_track,
  lock_page, held_step.

Index enums: `kP1*`, `kP2*`, `kSP*`, `kCfg*`, `kPat*`, `kShdw*`.

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

Knob layout:

```
Voice 1 (cursor 0..7):
  top  note WAV1 PA1  BLND
  bot  RTIO WAV2 PA2  FINE

Voice 2 (cursor 8..15):
  top  E1DC E1RL E2DC E2RL
  bot  E3DC E3RL NOIS sub

Step (cursor 16..23):
  top  PROB SSUB REPT RATE
  bot  vel  GLID MINT MDIR
```

The three `*RL` slots (E1REL/E2REL/E3REL) are reserved per
`voice_envelopes.md` — `fall` drives both decay and release, so the
voicecard ignores writes to these patch addresses. The slots remain in
the layout for future repurposing.

LEDs: `LED_1..LED_8` lit dim for `step_flags & kStepFlagOn`; the trailing
playhead step is lit bright when transport is playing.

---

## Per-track Settings (S6 / `PAGE_PART`)

`controller/ui_pages/seq_track_page.cc`. 8 pots map directly into the
active track's `pattern[8]`. Encoder turn cycles to the next page in the
group.

```
top  DIRN CDIV ROTA LENG
bot  SCAL ROOT BPCH OLEV
```

Pot ranges:
- DIRN: 0..3 (pot >> 6) — fwd / rev / pend / rnd
- CDIV: 0..7 (pot >> 5) — index into `kCDivValues[] = {1,2,3,4,6,8,12,16}`
- ROTA: 0..7 (pot >> 5)
- LENG: 1..8 ((pot >> 5) + 1)
- SCAL: 0..255 raw (mapping TBD)
- ROOT: 0..11 (pot * 12 / 256)
- BPCH: 0..127 (pot >> 1)
- OLEV: 0..255 raw

`PAGE_PART_ARPEGGIATOR` remains a stub in group 5 for future per-voice
default knob pages.

---

## Transport (S7 / `PAGE_MULTI`)

`MultiPage` relocated from group 4 → group 6. `PAGE_MULTI_CLOCK` (BPM/swing
knobs) chained alongside it. PLAY/PAUS/RST on `SWITCH_1`/`SWITCH_2`/`SWITCH_3`
unchanged. Beat-repeat / mute-solo / stutter is reserved for a future page
in this group.

---

## Sub-project status

`kSystemVersion 0x23` (controller + voicecard).

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

### Open / next iteration

| Surface                                              | Notes |
|------------------------------------------------------|-------|
| Hardware verification of round 3                     | Pending user retest |
| Encoder-click focused-edit display                   | No-op today; needs full-row `page \| name value` layout + 24-entry full-name table |
| WAVE strip-aware LUT                                 | Pot reaches CZ stripped indices (6..14) → silence; build contiguous valid-set lookup |
| Display offset for signed params (FINE / OSC2D / LFO depth) | Seq pages render raw byte; need `int8_t` bias rendering |
| Step behavior — PROB, REPT, RATE                     | Storage exists; resolver doesn't act on them |
| Step behavior — VEL                                  | Sent in trigger; not yet lockable in resolver |
| Step behavior — SSUB ratchets (Custom + Edit modes)  | Not yet |
| Step behavior — MINT/MDIR mutate (pitch walk)        | Not yet — needs note resolver pass |
| Step behavior — GLID legato                          | `command_ & 1` bit hardcoded to 0 in `FireStep` |
| Shadow playhead + Voltage-Block / Elektron hold modes | 5 bytes/track reserved, not yet wired |
| Hold-step semantics polish                           | Today: any held step + pot turn writes lock. No long-press / double-tap-clear yet |
| Per-voice defaults sub-page on S6                    | `PAGE_PART_ARPEGGIATOR` reserved as the slot |
| Reclaim dead `kP2*REL` slots                         | 18 B reserved across 6 tracks; awaiting use case |
| Track relationships / mod matrix                     | 4 active globally; not started |
| Storage multi-slot                                   | Single state snapshot only |
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
