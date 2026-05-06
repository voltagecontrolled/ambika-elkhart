# Elkhart — User Manual

*Targeting: ambika-elkhart-v4.0 (interim — describes current master.
Surfaces still landing for v4.0 are flagged in
[Pending v4.0 release](#pending-v40-release).)*

Elkhart is a 6-voice polymetric step sequencer + synthesizer firmware
for the Mutable Instruments Ambika hardware (Michigan Synth Works
Xena motherboard with SVF voicecards). Each of the six tracks runs an
independent synth voice with its own pattern, clock division, scale,
direction, and length; tracks drift in and out of phase as their
lengths and divisions interact. Per-step parameter locks let you
reshape any voice on a step-by-step basis — pitch, oscillator
algorithm, envelope decay, sub-osc shape, all 24 lockable parameters.

It's at home on percussion (six independent voices, transient sub-osc
shapes, tight envelope macros) but the YAM-derived voice engine —
dual oscillators with FM, eight-mode wavetables, vowel synthesis, an
SVF, fuzz, and bit-reduction — covers melodic and textural ground
equally well.

When this manual and `docs/planning/control_map.md` disagree, the
control map wins — it's extracted directly from `controller/ui.cc`
`page_registry[]`, `controller/parameter.cc`, and the
`controller/ui_pages/*.cc` handlers.

---

## Contents

1. [Firmware installation](#firmware-installation)
2. [Hardware](#hardware)
3. [Navigation](#navigation)
4. [Page reference](#page-reference)
5. [Sequencer mode](#sequencer-mode)
6. [Voice architecture](#voice-architecture)
7. [Pending v4.0 release](#pending-v40-release)
8. [Appendix — parameters by page](#appendix--parameters-by-page)

---

## Firmware installation

Elkhart ships as two binaries:

- `AMBIKA.BIN` — controller (motherboard, ATmega644p)
- `VOICE.BIN` — voicecard (ATmega328p), one binary, flashed
  independently to each of the six voice slots

**Controller and voicecards must run matching firmware versions.**
Master currently sets controller `kSystemVersion = 0x34` and
voicecard `kSystemVersion = 0x31` (`controller/controller.h:37`,
`voicecard/voicecard.h:35`). The OS Info page reports the running
version on each side; both must match the values shipped with the
v4.0 binaries.

### Prepare the SD card

Format the SD card to FAT16 or FAT32 with 8.3 filenames. Copy the
controller binary into the card root as `AMBIKA.BIN`. For each
voicecard slot you intend to flash, copy the voicecard binary as
`VOICE1.BIN` … `VOICE6.BIN`. The voicecard binary is identical for
every slot; only the filename differs.

### Flash the controller

1. Power off the Ambika.
2. Insert the SD card.
3. Hold **Button 8 (S8)** while powering on. The bootloader picks up
   `AMBIKA.BIN` from the SD card root and flashes the motherboard.
4. After boot, navigate to the OS Info page (`S8`) and confirm the
   controller version.

### Flash the voicecards

1. With the unit powered on, navigate to the OS Info page (`S8`).
2. The page lists the six voicecards and their reported versions. Use
   the encoder to highlight a voicecard slot.
3. Press **Button 4 (S4)** to flash the highlighted slot from
   `VOICE#.BIN`.
4. Repeat for each voicecard you want to update.

> **Mismatched versions can corrupt the per-step snapshot protocol** —
> step locks rely on a fixed 20-byte payload that both sides must
> agree on. If you see triggering anomalies after a partial flash,
> finish flashing all six voicecards.

---

## Hardware

- **Display:** 2 × 40 character LCD
- **Push encoder:** turn + click
- **Pots:** 4 above the LCD (top row) + 4 below (bottom row)
- **Buttons:** 8 buttons with LEDs, labeled `S1`..`S8`
- **Audio outputs:** 6 individual voice outs + 1 mix out (808-style
  normalling — plugging an individual out removes that voice from
  the mix)
- **Storage:** SD card (FAT16/FAT32, 8.3 filenames) for state
  snapshots and firmware flashing

This manual uses `S1`..`S8` for the eight buttons. Pots are
referenced as `top1`..`top4` and `bot1`..`bot4` — left-to-right
within each row.

---

## Navigation

### The eight pages

The eight buttons select page groups:

| Button | Group           | Pages in group                                |
|--------|-----------------|-----------------------------------------------|
| `S1`   | Oscillators     | S1a Oscillators, S1b Mixer                    |
| `S2`   | Filter          | S2 (single page)                              |
| `S3`   | Envelopes + LFO | S3a Amp+Filter env, S3b Pitch env + LFO       |
| `S4`   | (aliases S3)    | Same group as S3 — no dedicated S4 page in v4.0 |
| `S5`   | Sequencer mode  | S5a Step, S5b Voice 1, S5c Voice 2 (encoder-cycled) |
| `S6`   | Per-track       | S6a Track settings, S6b Performance mixer     |
| `S7`   | Transport       | Single page (PLAY/PAUS/RST/STOP, BPM, swing)  |
| `S8`   | System          | S8 OS Info / firmware flashing                |

Within a group, the **second tap of the same button** cycles to the
group's next page. Pressing a different button jumps groups.

A dedicated mod-matrix page is planned for v5.0; today the
`PAGE_MODULATIONS` registry slot is a `0xff` placeholder that the
encoder skips. Patch slot storage on S8a is slated for v4.0 — see
[Pending v4.0 release](#pending-v40-release).

### Encoder

- **Turn** — scrolls through the active page's parameters. On most
  pages the cursor advances one parameter per detent; on sequencer
  mode (S5) the encoder walks all 24 cells across the three lock
  pages and *spills* into the neighboring page group at the
  boundaries (cursor 0 → S3, cursor 23 → S6).
- **Click** — on `ParameterEditor` pages, enters focused-edit on the
  highlighted parameter (full name and value on row 2; encoder turn
  adjusts directly; click again to exit). On the sequencer step pages
  (S5a/b/c), encoder click is reserved for entering the substep
  editor when the cursor is on `subs` and a step is held; it does
  not enter focused-edit on other cells.

### Page-jump chord

Holding `S2` or `S8` while turning the encoder applies an ×8
multiplier — a fast page-jump shortcut to walk across page groups
without leaving the encoder. Single-detent turns without the chord
walk one cell at a time as usual.

### Global gestures

Two hold-button + encoder chords are available from any page,
including sequencer mode (where the encoder is normally consumed by
cursor walk).

**Hold `S5` + encoder = transport.** Reach play/pause/stop without
leaving the current page.

- Encoder click → toggle play/pause.
- Encoder CW → reset (release notes, zero playhead).
- Encoder CCW → stop. Notes ring out per envelope release.
- Encoder CCW twice within 400 ms → panic. Hard mute on every voice.

A plain `S5` press (no encoder activity) still toggles sequencer mode
as before — the chord only consumes the release when the encoder
actually fires.

**Hold `S7` + encoder turn = jump between Transport and Mixer pages.**
Cycles between `S7 — Transport` and `S6b — Performance mixer` from
any context.

### Pots

Pots always edit the eight parameters currently displayed. The
parameter abbreviations on row 1 of the LCD identify which pot drives
which value. On pages with fewer than 8 active parameters, unused
pots are inert.

### Voice select

Hold `S1` and turn the encoder to cycle the active track / voice. All
sequencer-mode editing — step toggles, knob writes, lock writes —
applies to the active voice's pattern.

### Sequencer mode toggle

`S5` enters **Sequencer Mode** (per-step parameter editing on the
active track). A second `S5` press exits back to normal mode. While
in sequencer mode, `S1`..`S8` are step triggers, not page-jump
shortcuts; encoder navigation is the way out (or `S5` again).

---

## Page reference

### S1a — Oscillators (Group 0, top half)

The voice's two oscillators. Each oscillator has the same four
controls:

```
top  wave | para | rang | tune     ← Osc 1
bot  wave | para | rang | tune     ← Osc 2
```

- **`wave`:** algorithm select. Palette spans modern PolyBLEP
  saw/PWM, sine, triangle, FM/FM-fb, dirty PWM, filtered noise, vowel
  synthesis, 16 wavetable banks, wavequence, the original
  pre-PolyBLEP saw, and several CZ filter-sim variants. Range
  `0..43`.
- **`para`:** algorithm-specific parameter — PWM amount for square,
  formant select for vowels, FM index for `fm`, etc. Range `0..127`.
- **`rang`:** coarse pitch offset, ±24 semitones.
- **`tune`:** fine detune, ±64 cents.

Knob writes here set the **track default** for that parameter (the
value used on unlocked steps). Per-step locks for the oscillator
parameters live on S5b (Voice 1 sequencer page).

### S1b — Mixer (Group 0, bottom half)

```
top  mix  | nois | sub  | wave     ← osc balance, noise, sub level, sub-osc shape
bot  xmod | amnt | fuzz | crsh     ← cross-mod, fuzz, bitcrush
```

- **`mix` (BLND):** bipolar oscillator interaction. Stored range is
  `0..63`. The full conceptual range is bipolar:
  - `0`–`63`: crossfade between Osc 1 and Osc 2 in the audio path
    (`0` = Osc 1 only, `63` = Osc 2 only).
  - `64`–`127`: Osc 2 leaves the audio path and FM-modulates Osc 1.
    Default builds clamp `mix` to `0..63` — the FM zone is part of
    the design but not exposed in the v4.0 default mix.
- **`nois`:** noise generator level into the audio path.
- **`sub`:** sub-oscillator / transient layer level.
- **`wave` (top4):** sub-osc shape, 11-shape palette (six traditional
  sub-bass shapes + five transient one-shots — see
  [Voice architecture](#voice-architecture)). Same control as `wave`
  on S5c bot4.
- **`xmod` / `amnt`:** cross-modulation operator and depth.
- **`fuzz`:** post-filter saturation/distortion.
- **`crsh`:** pre-DAC bit reduction. `0` = full resolution; the upper
  range adds digital grit.

`fuzz` and `crsh` live here, **not** on the Filter page.

### S2 — Filter

```
top  freq | reso | --   | mode
bot  env2 | --   | --   | --
```

Six of the eight pots are inert on this page.

- **`freq`:** SVF cutoff, `0..127`. Lockable per step on S5c.
- **`reso`:** resonance, `0..63`. High resonance at moderate cutoff
  produces ringing, near-self-oscillation tones.
- **`mode`:** filter mode select, `0..3` — **LP / BP / HP / Notch**.
  LP for body and warmth, HP for hats and cymbals, BP for
  nasal/metallic character, Notch for phaser-like rejection.
- **`env2`:** Env 2 → cutoff depth, `0..63`. Lockable on S5c as
  `famt`. The filter envelope modulates around the `freq` setting by
  this depth.

For pure passthrough (filter "off"): `freq` to max, `reso` to min.

Filter drive (`fuzz`) and bit reduction (`crsh`) live on **S1b
Mixer**.

### S3a — Amp + Filter envelopes (Group 2, top half)

Three independent ADR + Curve envelopes with fixed routing — Env 1
to VCA, Env 2 to filter cutoff, Env 3 to Osc 1 base pitch. There is
**no sustain stage**; `curv` controls the linear-to-exponential
shape blend of decay/release (`0` = linear, `127` = exponential).

```
top  rise | fall | curv | amp     ← Env 1 (VCA)
bot  rise | fall | curv | flt     ← Env 2 (VCF)
```

The depth knob doubles as the row label: `amp` and `flt` identify
the envelope.

### S3b — Pitch envelope + voice LFO (Group 2, bottom half; also S4)

```
top  rise | fall | curv | pitc    ← Env 3 (Pitch)
bot  rate | wave | dest | dept    ← LFO 4 (voice LFO)
```

LFO controls:

- **`rate`:** modulation rate (free-running in v4.0).
- **`wave`:** waveform — sine, triangle, square, ramp, S&H/random.
- **`dest`:** modulation destination (oscillator pitch, cutoff, FM
  depth, etc.).
- **`dept`:** modulation depth, signed `−63..+63`, so the LFO can
  modulate negatively.

Per-step lockable: the three `fall` bytes (Env 1 / Env 2 / Env 3)
participate in the lock system as `adec` / `fdec` / `pdec` on the
S5c sequencer page. The other envelope bytes are voice-wide.

### S4 — (no dedicated page in v4.0)

`S4` shares Group 2 with `S3` and routes to S3b. The
`PAGE_MODULATIONS` registry slot is a `0xff` placeholder; the
encoder skips it. A dedicated mod-matrix page is planned for v5.0.

### S5 — Sequencer mode

See [Sequencer mode](#sequencer-mode) below — the largest section of
this manual.

### S6a — Per-track settings

Per-track pattern configuration. Pots map directly into the active
track's `pattern[]`:

```
top  dirn | cdiv | rota | leng
bot  scal | root | ---- | vol
```

- **`dirn`:** direction — fwd / rev / pend (pendulum) / rnd.
- **`rate`:** track step length — the polymeter engine. 15 musical
  values selectable per track: `32` (32nd), `16t` (16th triplet),
  `16` (16th), `8t`, `16d`, `8`, `4t`, `8d`, `4` (quarter), `2t`,
  `4d`, `2` (half), `1` (whole), `1d`, `2B` (2 bars in 4/4). Two
  tracks at different rates drift in and out of phase as their
  patterns cycle. Default for new tracks is `16` (sixteenth note).
- **`rota`:** rotate pattern start point (0–7) without altering step
  data. Useful for shifting the downbeat of a fixed pattern.
- **`leng`:** pattern length in steps (1–8). Combined with `cdiv`,
  creates per-track polymeters.
- **`scal`:** scale quantizer — chro / maj / min / dor / mix / pMa
  (penta major) / pMi (penta minor) / blu (blues). All resolved step
  notes are quantized into the active scale.
- **`root`:** scale root, in semitones from C (0–11).
- **`vol`:** track volume — multiplicative scale on resolved step
  velocity. `255` is identity, `0` mutes the track.

The middle slot in the bottom row (`----`) is the retired BPCH cell.
The pot is inhibited and the cell renders `----`. A clear-function
on this slot is slated for v4.0; see
[Pending v4.0 release](#pending-v40-release).

The active track is selected by the `S1 + encoder` voice-select
chord — see [Voice select](#voice-select).

### S6b — Performance mixer

Live performance control for the six tracks: per-voice volume, mute,
audio mute, and solo, all reachable from a single page. Designed for
performance, not patch design — **state is transient** (cleared on
power-cycle, not saved with the patch slot).

```
Pots:  top1 top2 top3 top4         Buttons:  S1 S2 S3 S4 S5 S6 S7   S8
       bot1 bot2 bot3 bot4                   v1 v2 v3 v4 v5 v6 mode unmute
```

- **Pots `top1`..`top3` / `bot1`..`bot3`:** per-voice volume.
  **Pickup catch:** when entering the page, each pot must physically
  cross the stored volume value before it begins writing — prevents
  a destination jump from the resting pot position.
- **`S1`..`S6`:** toggle the active mode's bit for that voice.
- **`S7` tap:** cycle mode `MT-S → MT-A → SOLO → MT-S`. The `S7` LED
  encodes mode (off / dim red / bright red).
- **`S7` hold + `S1`..`S6` taps:** queue toggles. On `S7` release,
  the queued voices are XOR'd into the active mode's bits as a
  single batch. Queued voices blink red during the hold.
- **`S8` tap:** unmute-all — clear all three bit sets at once.

Three modes:

| Mode  | Skip future fires | Action on toggle of currently-sounding voice |
|-------|-------------------|-----------------------------------------------|
| MT-S  | yes               | none — current note's envelope completes      |
| MT-A  | yes               | instant audio cut (kill voice)                |
| SOLO  | non-solo'd voices | kill non-solo voices that just lost audibility|

The encoder walks an 8-cell cursor; spills to the neighboring page
at the boundary. Cosmetic LCD layout changes for the rightmost
column are slated for v4.0; see
[Pending v4.0 release](#pending-v40-release).

### S7 — Transport

Master clock and transport.

```
top  bpm  | swng | mrst | ----
bot  ---- | ---- | ---- | ----

Buttons:  S1   S2    S3   S4    S5  S6  S7  S8
          PLAY PAUS  RST  STOP  --  --  --  EXIT
```

- **`bpm` (top1):** master tempo, 40..240 BPM.
- **`swng` (top2):** groove amount (`PRM_MULTI_CLOCK_GROOVE_AMOUNT`).
  *Functional status under review — see
  [Pending v4.0 release](#pending-v40-release).*
- **`mrst` (top3):** master reset period. `off` (free-run) or
  `2..128` undivided steps. When the master tick counter reaches the
  threshold, all six tracks reset to step 0 — useful for keeping
  polymetric tracks at varying CDIVs from drifting open-ended.
  Default `off`.
- **`PLAY` / `PAUS` / `RST`:** standard transport.
- **`STOP` (S4):** single tap = `Pause` + `Reset`. **Double-tap
  within 300 ms = panic** — `Pause` + `Reset` + kill all six voices
  immediately. The S4 LED lights red while the second-tap window is
  open.
- **`EXIT` (S8):** return to the most recent non-system page.

LEDs: status LED bright while playing, dim while paused; `S1` (PLAY)
bright while playing; `S2` (PAUS) bright while paused; `S4` (STOP)
red flash during the double-tap window; `S8` (EXIT) always lit.

Both transport actions and the Transport ↔ Mixer page jump are also
available globally — see [Global gestures](#global-gestures).

### S8 — OS Info / firmware flashing

Read-only display showing controller + per-voicecard firmware
versions, and the gateway to per-card firmware flashing (see
[Firmware installation](#firmware-installation)).

Buttons:

- **`S1`:** upload controller firmware (if `/AMBIKA.BIN` is on SD).
- **`S4`:** upload voicecard firmware to the highlighted slot (if
  `/VOICE#.BIN` is on SD).
- **`S8`:** EXIT.

The encoder cycles the active voicecard slot for flashing. System
settings (`PAGE_SYSTEM_SETTINGS`) are reachable as the second page
in the S8 group.

A dedicated patch-slot save/load page (S8a) is slated for v4.0; see
[Pending v4.0 release](#pending-v40-release).

---

## Sequencer mode

Press **`S5`** to enter. Press **`S5`** again to exit.

In sequencer mode, the eight buttons (`S1`..`S8`) are step triggers
for the active track. The eight pots edit the lockable parameters of
the **active lock-page** (Step / Voice 1 / Voice 2). The encoder
walks across all 24 cells; the active page is `cursor >> 3`.

### Voice selection

Hold `S1` and turn the encoder to cycle the active track / voice.
All sequencer-mode editing — step toggles, knob writes, lock writes
— applies to the active voice's pattern.

### Step on/off, peek, and double-tap to clear

**Tap a step button** (≤ 250 ms) to toggle that step's main
`step_flags & on` bit. The `LED_1`..`LED_8` row lights dim for
steps that are on; the trailing playhead step lights bright while
transport is playing.

**Hold a step button** (> 250 ms) without turning a pot to
**peek** at its locks — the LCD reflects which parameters that step
overrides. Releasing a long-held step does **not** toggle the step,
so peeking is non-destructive.

**Double-tap a step** within 300 ms (two short taps on the same
step) to **clear all locks** for that step. The first tap's toggle
is undone, so the step's on/off state is unchanged — only the
`lock_flags` are cleared, reverting that step's parameters back to
the track defaults.

### Lock-page cycling

The encoder walks `cursor` from 0 to 23. The 4-character
abbreviations on row 1 of the LCD show the active page's eight
cells; the cursor `>` marker precedes the focused cell. The active
page is mirrored as a tag in the top-right (`v1` / `v2` / `sp`):

```
cursor 0..7   →  S5a  Step       (sp)
cursor 8..15  →  S5b  Voice 1    (v1)
cursor 16..23 →  S5c  Voice 2    (v2)
```

Crossing a page boundary flips the page automatically; encoder past
cursor 23 spills out to **S6** (track settings); past cursor 0 spills
out to **S3** (envelopes + LFO).

### Locking a parameter

**Hold a step button + turn a pot:** writes a lock for that pot's
parameter on that step. The dirty bit is set and the locked value
participates in the per-step snapshot.

**Encoder click while a step is held + cursor on `subs`** — and the
step has SSUB ≠ 0 or REPT ≠ 0 — enters the **substep editor** (see
below).

Locked parameters snap back to track defaults after the step fires —
every trigger ships a full parameter snapshot, with no carryover
between steps.

**Unlocked step parameters** read the **track default**, which is
itself live: turning a knob in sequencer mode while no step is held
writes the track default and is heard immediately on every unlocked
step. This tension between locked and unlocked steps is the main
performance dimension.

### S5a — Step page (cursor 0..7)

```
top  note | vel  | vamt | rate
bot  subs | prob | glid | gtim
```

| Cell   | Name                | Range / behavior |
|--------|---------------------|------------------|
| `note` | Step note           | 0..127, displayed as note name. Quantized to track scale. |
| `vel`  | Velocity            | 0..127. |
| `vamt` | Velocity → VCA depth| Voice-wide config (mod slot 11 amount); not lockable per step. |
| `rate` | Step rate           | Per-step rate override — replaces the track `rate` for this step (no relative math). `trk` = inherit track rate (no override). Other values are the same 15 musical entries available on S6a (`32` … `2B`). |
| `subs` | Sub-steps / mode    | Bipolar combined SSUB+REPT cell. CCW = repeats `8r`..`1r`, deadzone at center, CW = ratchets `1x`..`8x`. Display: `Nr` / `0` / `Nx` / `cus` (custom from substep editor). |
| `prob` | Probability         | 0..127 → 0..100% per-step fire probability. |
| `glid` | Glide               | Tie this step to the next — suppresses envelope retrigger on the next step (legato). |
| `gtim` | Glide time          | Voice-wide portamento time on glided notes; not lockable per step. |

A planned v4.0 change drops `glid` and renames `gtim` → lockable
`glid`; see [Pending v4.0 release](#pending-v40-release).

### S5b — Voice 1 page (cursor 8..15)

```
top  nois | w1   | pa1  | tun2
bot  mix  | w2   | pa2  | fin2
```

| Cell    | Name                  | Range / behavior |
|---------|-----------------------|------------------|
| `nois`  | Noise mix             | White noise level into the audio path. |
| `w1`    | Osc 1 wave            | Algorithm select; renders as 6-char waveform name. |
| `pa1`   | Osc 1 PARA            | Algorithm-specific parameter (PWM, formant, FM index, …). |
| `tun2`  | Osc 2 coarse tune     | Signed semitone offset on Osc 2. |
| `mix`   | BLND (osc interaction)| Bipolar `0..63` mix (FM zone gated in default builds). |
| `w2`    | Osc 2 wave            | Algorithm select. |
| `pa2`   | Osc 2 PARA            | Algorithm-specific parameter. |
| `fin2`  | Osc 2 fine tune       | Signed cents detune on Osc 2. |

### S5c — Voice 2 page (cursor 16..23)

```
top  freq | fdec | famt | adec
bot  pdec | pamt | sub  | wave
```

| Cell   | Name                       | Range / behavior |
|--------|----------------------------|------------------|
| `freq` | Filter cutoff base         | SVF cutoff. The filter envelope modulates around this. |
| `fdec` | Filter env (Env 2) decay   | Env 2 fall byte. |
| `famt` | Filter env depth           | Env 2 → cutoff depth. Signed. |
| `adec` | Amp env (Env 1) decay      | Env 1 fall byte. |
| `pdec` | Pitch env (Env 3) decay    | Env 3 fall byte. |
| `pamt` | Pitch env depth            | Env 3 → Osc 1 base pitch. Signed. |
| `sub`  | Sub-osc level              | Sub-osc / transient layer mix. |
| `wave` | Sub-osc shape              | 11-shape palette (see [Voice architecture](#voice-architecture)). |

### Substep editor

Hold a step button, scroll the cursor to `subs`, and click the
encoder. Two requirements: the cursor must be on `subs`, and the
held step must have either SSUB ≠ 0 or REPT ≠ 0 (otherwise the
click is a no-op).

The editor offers per-fire bit-level control over the gated fires of
the step. The interpretation depends on which mode you entered from:

#### Gated repeats (SSUB = -2 zone)

When entered from a step in the CCW (repeats) zone of `subs`,
`substep_bits` gates each of the `REPT + 1` period-boundary fires:

- **Bit 0** = does the initial step fire?
- **Bits 1..REPT** = does each of the subsequent period re-fires?

#### Gated ratchets (CW zone, `kStepFlagGated`)

When entered from a step in the CW (ratchets) zone, `substep_bits`
gates each of the `SSUB + 1` within-period sub-triggers — letting you
silence individual ratchet hits while preserving the ratchet timing
grid.

#### Editor controls

While the editor is active:

- **Pot 0 (`subs` slot):** count + mode. CCW → CW mirrors the S5a
  `subs` knob; switching across the deadzone toggles between
  gated-repeat and gated-ratchet modes. A pickup guard absorbs the
  first ADC reading on entry so the resting pot position doesn't
  overwrite the stored value.
- **Pot 1 (`mint` slot):** **MINT** — interval step size, 0..12.
  Labels: `off`, `m2`, `M2`, `m3`, `M3`, `P4`, `TT`, `P5`, `m6`, `M6`,
  `m7`, `M7`, `oct`.
- **Pot 2 (`mdir` slot):** **MDIR** — wave shape: `up` / `dn`
  (sawtooth), `ud` / `ud+` / `ud-` (triangle), `rnd` / `rnd+` / `rnd-`
  (random). See **Mutation** below.
- **Pot 3 (`moct` slot):** **MOCT** — range cap in octaves, 1..4.
- **`S1`..`S8`:** toggle individual `substep_bits` slots. Slots above
  the active count are inactive (button no-op, LED dark).
- All other pots are inert while the editor is active.

LCD layout while editing:

```
Line 0:  subs Nr | mint m3 | mdir up | moct 2
Line 1:  # # - # # # - -    (one slot per active position)
```

LEDs mirror `substep_bits`, masked to the active count.

On entry, `substep_bits` is trimmed to the active range. Stale
out-of-range bits from a prior session are cleared; if nothing
survives the trim, all active slots re-enable.

**Exit:** click the encoder again.

### Mutation (MINT + MDIR + MOCT)

When MINT is non-zero, every ratchet sub-trigger and repeat fire
after the first walks the step's pitch by integer multiples of MINT
semitones, bounded to ±`MOCT × 12` semitones from the base note. MDIR
selects the wave shape of the walk. The final pitch is clamped to
0..127 and re-quantized to the track's scale.

**MINT** sets the step size in semitones:

| Label | Semitones | Interval     |
|-------|-----------|--------------|
| `off` | —         | disabled     |
| `m2`  | 1         | minor 2nd    |
| `M2`  | 2         | major 2nd    |
| `m3`  | 3         | minor 3rd    |
| `M3`  | 4         | major 3rd    |
| `P4`  | 5         | perfect 4th  |
| `TT`  | 6         | tritone      |
| `P5`  | 7         | perfect 5th  |
| `m6`  | 8         | minor 6th    |
| `M6`  | 9         | major 6th    |
| `m7`  | 10        | minor 7th    |
| `M7`  | 11        | major 7th    |
| `oct` | 12        | octave       |

**MDIR** sets the wave shape of the walk:

| MDIR  | Shape    | Range                                    |
|-------|----------|------------------------------------------|
| `up`  | sawtooth | base → `+MOCT` oct, then wraps to base   |
| `dn`  | sawtooth | base → `−MOCT` oct, then wraps to base   |
| `ud`  | triangle | `±MOCT` oct, bipolar around base note    |
| `ud+` | triangle | base ↔ `+MOCT` oct (bounces off base)    |
| `ud-` | triangle | base ↔ `−MOCT` oct (bounces off base)    |
| `rnd` | random   | random MINT-multiple in `±MOCT` oct      |
| `rnd+`| random   | random MINT-multiple in 0..`+MOCT` oct   |
| `rnd-`| random   | random MINT-multiple in 0..`−MOCT` oct   |

All eight modes step in integer multiples of MINT, so the labelled
musical interval is preserved as the step size — `mint=oct, moct=4,
up` walks base, +1, +2, +3, +4 oct, then wraps. `rnd` differs from
`ud` only in choosing the next MINT-multiple at random instead of in
a deterministic wave; both are bounded by MOCT.

Mutation is resolved on the controller — the voicecard receives the
final computed pitch.

### Stateless triggering

Every step trigger sends a full 20-byte parameter snapshot — either
the step's locked values or the current track defaults — alongside
the note and velocity. There is no state carryover between steps.
What you see in the LCD on a step is exactly what fires on that
step, and only that step.

---

## Voice architecture

Each voicecard runs the same signal chain. All timbral variety comes
from parameter differences plus algorithm selection per oscillator.

### Signal chain

```
                       ┌────────────────┐
                       │  Sub-Osc /     │
                       │  Transient     │
                       │  (11 shapes)   │
                       └────────┬───────┘
                                │ SUB
                                ▼
┌────────┐                ┌──────────┐    ┌─────────┐    ┌──────────┐
│ Osc 2  │ FM (BLND≥64)  │  Osc 1   │    │  Mixer  │    │   SVF    │
│ (mod / │──────────────▶│(carrier) │───▶│ ±BLND   │───▶│ LP/BP/HP │──▶ Env1 (VCA) ──▶ OUT
│  mix)  │                └──────────┘    │ + SUB   │    │ + Notch  │
└────────┘                                 │ + NOIS  │    │ + fuzz   │
                                           └─────────┘    │ + crsh   │
                                                          └────┬─────┘
                                                               ▲
                                                               │
                                                          Env2 (Filter, FAMT-scaled)
                                                          (independent)

Env3 (Pitch) ──▶ Osc 1 base pitch (PAMT-scaled, independent)
LFO 4        ──▶ voice-config target (pitch / cutoff / FM depth / …)
```

### Three independent envelopes

Each voicecard runs **three independent envelopes** with fixed
routing. All three are parameterized identically: `rise` (attack),
`fall` (decay/release rate — there is no sustain), and `curv`
(linear-to-exponential blend). The depth knob lives in the modulation
slot the envelope drives:

| Envelope     | Drives          | Decay knob | Depth knob       |
|--------------|-----------------|------------|------------------|
| Env 1 (VCA)  | Output amplitude| `adec`     | `amp`            |
| Env 2 (VCF)  | SVF cutoff      | `fdec`     | `flt` (`famt`)   |
| Env 3 (Pitch)| Osc 1 base pitch| `pdec`     | `pitc` (`pamt`)  |

The three `fall` bytes are the only envelope parameters that
participate in the per-step lock system (as `adec` / `fdec` / `pdec`
on S5c).

### Dual oscillator (Osc 1 = carrier, Osc 2 = modulator)

Two oscillator slots, each independently selecting from YAM's wave
shape palette (PolyBLEP saw/PWM, sine, triangle, FM/FM-fb, dirty
PWM, filtered noise, vowel synthesis, 16 wavetable banks, wavequence,
the original pre-PolyBLEP saw, several CZ filter-sim variants). The
carrier (Osc 1) determines perceived pitch; the modulator (Osc 2) is
either mixed into the audio path or routed to FM-modulate Osc 1,
controlled by the bipolar `mix` (BLND) knob on S1b.

#### `mix` / BLND zones

- **Below center (0–63):** crossfade between Osc 1 and Osc 2 in the
  audio path.
- **At center (63→64):** acoustically silent transition — both
  endpoints are "Osc 1 only, no FM."
- **Above center (64–127):** Osc 2 modulates Osc 1's pitch (FM); Osc
  2 leaves the audio path. Default builds clamp `mix` to 0..63 — the
  FM zone is engineered but gated until linear-FM work lands.

At any setting you hear *either* a mix of Osc 1 and Osc 2 *or* Osc 1
being FM'd by Osc 2 — never both.

### Sub-oscillator and transient layer

The sub-oscillator slot doubles as a transient generator. Eleven
shapes total: six traditional sub-bass shapes (Square, Triangle,
Pulse — each in two octave-related variants) and five transient
one-shots (Click, Glitch, Blow, Metallic, Pop). Selecting a transient
shape produces a one-shot attack on note-on, mixed at the `sub` level
into the main path. Independent of Osc 1 / Osc 2 — you can layer a
Click transient under a vowel-and-FM voice without touching either
oscillator's wave selection.

---

## Pending v4.0 release

The following surfaces are scoped for the v4.0 milestone but have
not yet landed on master. They will be folded into this manual when
they ship; until then, this section is the canonical record of
what's outstanding.

| Issue | Surface | Status |
|-------|---------|--------|
| #8    | S6b mixer cosmetic fixes (S7/S8 cell layout, color convention) | Pending |
| #10   | S8a patch slot save/load page; OS Info moves to S8b | Pending |
| #18   | Wavefolder waveform added to the oscillator palette (`para` = fold depth); CZ filter-sim variants may be pulled to free flash | Pending |
| #23   | S5a layout change: drop legato `glid`, rename `gtim` → lockable `glid` on bot3, leave bot4 empty | Pending |
| #24   | S7 `swng` — fix or pull (currently appears not to affect the sequencer pattern) | Pending |
| #25   | S6a bot3 `clr` function: pot selects `clr locks` / `clr steps` / `clr notes` / `clr voice` / `clr all`; long-press S6 to arm, tap to confirm | Pending |

Deferred to v5.0 (do not expect in this manual): mod matrix (#11),
LFO 4 tempo sync (#12), encoder-click focused-edit on sequencer
pages (#15), master transpose (#19), master scale (#20), iterative
probability modes (#6).

---

## Appendix — parameters by page

`†` = also appears as a per-step lockable in sequencer mode.

### S1a Oscillators (voice config; all `†` items lockable on S5b)

| Abbrev | Name           | Notes |
|--------|----------------|-------|
| `wave`†| Osc 1 wave     | Algorithm select, 0..43 |
| `para`†| Osc 1 PARA     | Algorithm parameter, 0..127 |
| `rang` | Osc 1 range    | Coarse, ±24 semitones |
| `tune` | Osc 1 tune     | Fine, ±64 cents |
| `wave`†| Osc 2 wave     | Algorithm select |
| `para`†| Osc 2 PARA     | Algorithm parameter |
| `rang`†| Osc 2 range    | (= `tun2` on S5b — coarse semitones) |
| `tune`†| Osc 2 tune     | (= `fin2` on S5b — fine cents) |

### S1b Mixer (voice config; `†` items lockable)

| Abbrev | Name                | Notes |
|--------|---------------------|-------|
| `mix`† | Osc balance / BLND  | Bipolar 0..63; FM zone gated in default builds |
| `nois`†| Noise level         | 0..63 |
| `sub`† | Sub-osc level       | 0..63 |
| `wave`†| Sub-osc shape       | 11-shape palette (also reachable on S5c bot4) |
| `xmod` | Cross-mod operator  | Operator select |
| `amnt` | Cross-mod amount    | 0..63 |
| `fuzz` | Fuzz / saturation   | 0..63 |
| `crsh` | Bit reduction       | 0..31 |

### S2 Filter (voice config; `†` items lockable)

| Abbrev | Name                | Notes |
|--------|---------------------|-------|
| `freq`†| Cutoff              | 0..127 |
| `reso` | Resonance           | 0..63 |
| `mode` | Filter mode         | 0..3 — LP / BP / HP / Notch |
| `env2`†| Env 2 → cutoff depth| 0..63 (= `famt` on S5c) |

Six of the eight pots on this page are inert.

### S3a/S3b Envelopes + LFO (voice config; `fall` bytes are `†` lockable)

| Abbrev      | Name                        | Notes |
|-------------|-----------------------------|-------|
| `rise` (E1) | Env 1 attack                | |
| `fall` (E1)†| Env 1 decay/release (`adec`)| |
| `curv` (E1) | Env 1 curve blend           | 0 lin .. 127 expo |
| `amp`       | Env 1 → VCA depth           | |
| `rise` (E2) | Env 2 attack                | |
| `fall` (E2)†| Env 2 decay/release (`fdec`)| |
| `curv` (E2) | Env 2 curve blend           | |
| `flt`       | Env 2 → cutoff depth        | (= `famt` on S5c) |
| `rise` (E3) | Env 3 attack                | |
| `fall` (E3)†| Env 3 decay/release (`pdec`)| |
| `curv` (E3) | Env 3 curve blend           | |
| `pitc`      | Env 3 → pitch depth         | (= `pamt` on S5c) |
| `rate`      | LFO 4 rate                  | Free-running |
| `wave`      | LFO 4 waveform              | |
| `dest`      | LFO 4 destination           | |
| `dept`      | LFO 4 depth                 | Signed −63..+63 |

### S6a Per-track settings (track config)

| Abbrev | Name           | Notes |
|--------|----------------|-------|
| `dirn` | Direction      | fwd / rev / pend / rnd |
| `rate` | Track step length | 15 musical values: `32, 16t, 16, 8t, 16d, 8, 4t, 8d, 4, 2t, 4d, 2, 1, 1d, 2B` |
| `rota` | Rotate         | Pattern start offset, 0..7 |
| `leng` | Length         | Steps per cycle (1–8) |
| `scal` | Scale          | chro / maj / min / dor / mix / pMa / pMi / blu |
| `root` | Root           | Semitone offset (0–11) |
| `----` | (BPCH retired) | Inert; reserved for v4.0 `clr` function (#25) |
| `vol`  | Volume         | Velocity scale (0–255) |

### S6b Performance mixer (transient)

Per-voice volume + MT-S / MT-A / SOLO toggles. State not persisted.

### S7 Transport

| Abbrev | Name      | Notes |
|--------|-----------|-------|
| `bpm`  | Tempo     | Master BPM (40..240) |
| `swng` | Swing     | Groove amount (functional status under review — #24) |
| —      | PLAY/PAUS/RST/STOP/EXIT | Buttons (S1/S2/S3/S4/S8) |

### S5a Step page (per-step lockable except where noted)

| Abbrev | Name              | Notes |
|--------|-------------------|-------|
| `note` | Step note         | Quantized to track scale |
| `vel`  | Velocity          | 0..127 |
| `vamt` | Vel → VCA amount  | Voice-wide config (mod slot 11); not lockable |
| `rate` | Step rate         | Per-step rate override (`trk` = inherit track; `32`…`2B` direct, replaces track) |
| `subs` | Sub-steps / mode  | Bipolar SSUB + REPT cell |
| `prob` | Probability       | 0..127 → 0..100% |
| `glid` | Glide (legato)    | Tie to next step |
| `gtim` | Glide time        | Voice-wide portamento; not lockable |

### S5b Voice 1 page (per-step lockable)

| Abbrev | Name             | Notes |
|--------|------------------|-------|
| `nois` | Noise mix        | |
| `w1`   | Osc 1 wave       | Wave name |
| `pa1`  | Osc 1 PARA       | |
| `tun2` | Osc 2 coarse     | Signed |
| `mix`  | BLND             | Bipolar |
| `w2`   | Osc 2 wave       | Wave name |
| `pa2`  | Osc 2 PARA       | |
| `fin2` | Osc 2 fine       | Signed |

### S5c Voice 2 page (per-step lockable)

| Abbrev | Name                 | Notes |
|--------|----------------------|-------|
| `freq` | Filter cutoff        | |
| `fdec` | Env 2 decay          | |
| `famt` | Env 2 → cutoff depth | Signed |
| `adec` | Env 1 decay          | |
| `pdec` | Env 3 decay          | |
| `pamt` | Env 3 → pitch depth  | Signed |
| `sub`  | Sub-osc level        | |
| `wave` | Sub-osc shape        | 11-shape palette |

---

## License

GPLv3, inherited from upstream Mutable Instruments / YAM. Voice DSP
adapted from `bjoeri/ambika` (YAM); original Ambika firmware by
Émilie Gillet. Contains a variant of Peter Knight's Cantarino
formant synthesis algorithm.
