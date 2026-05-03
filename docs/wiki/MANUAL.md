# Elkhart — User Manual

*Last updated for: ambika-elkhart-v4.0*

Elkhart is a 6-voice polymetric step sequencer + synthesizer firmware for
the Mutable Instruments Ambika hardware (Michigan Synth Works Xena
motherboard with SVF voicecards). Each of the six tracks runs an
independent synth voice with its own pattern, clock division, scale,
direction, and length; tracks drift in and out of phase as their lengths
and divisions interact. Per-step parameter locks let you reshape any
voice on a step-by-step basis — pitch, oscillator algorithm, envelope
decay, sub-osc shape, all 28 of them.

It's at home on percussion (six independent voices, transient sub-osc
shapes, tight envelope macros) but the YAM-derived voice engine — dual
oscillators with FM, eight-mode wavetables, vowel synthesis, an SVF with
drive and bit-reduction, a wavefolder — covers melodic and textural
ground equally well.

> **About this manual:** sections marked with `<!-- VERIFY: ... -->`
> document v4.0 behavior that has been spec'd but not yet hardware-walked
> at time of writing. The verification pass before tag-time will resolve
> these markers against the shipping firmware.

---

## Contents

1. [Firmware installation](#firmware-installation)
2. [Hardware](#hardware)
3. [Navigation](#navigation)
4. [Page reference](#page-reference)
5. [Sequencer mode](#sequencer-mode)
6. [Voice architecture](#voice-architecture)
7. [Appendix — parameters by page](#appendix--parameters-by-page)

---

## Firmware installation

Elkhart ships as two binaries:

- `AMBIKA.BIN` — controller (motherboard, ATmega644p)
- `VOICE.BIN` — voicecard (ATmega328p), one binary, flashed independently
  to each of the six voice slots

**Controller and voicecards must run matching firmware versions.** v4.0
ships with controller `kSystemVersion = 0x34` and voicecard
`kSystemVersion = 0x31`. The OS Info page reports the running version on
each side; both must match these values after flashing.

### Prepare the SD card

Format the SD card to FAT16 or FAT32 with 8.3 filenames. Copy the
binaries into the card root, naming them as follows:

| File on SD card | Source binary  | Flashed to                |
|-----------------|----------------|---------------------------|
| `AMBIKA.BIN`    | `AMBIKA.BIN`   | Controller (motherboard)  |
| `VOICE1.BIN`    | `VOICE.BIN`    | Voicecard slot 1          |
| `VOICE2.BIN`    | `VOICE.BIN`    | Voicecard slot 2          |
| …               | …              | …                         |
| `VOICE6.BIN`    | `VOICE.BIN`    | Voicecard slot 6          |

You only need `VOICE#.BIN` files for the slots you intend to flash.

### Flash the controller

1. Power off the Ambika.
2. Insert the SD card.
3. Hold **Button 8 (S8)** while powering on. The bootloader will pick up
   `AMBIKA.BIN` from the SD card root and flash the motherboard.
4. After the boot completes, navigate to the OS Info page (`S8`) and
   confirm the controller version reads `0x34`.

### Flash the voicecards

1. With the unit powered on, navigate to the OS Info / firmware-upgrade
   page. <!-- VERIFY: per issue #10, firmware upgrade moves to S8b at v4.0; patch save lives on S8a. Confirm exact navigation gesture before publishing. -->
2. The page lists the six voicecards and their reported versions. Use
   the encoder to highlight a voicecard slot.
3. Press **Button 4 (S4)** to flash the highlighted slot from
   `VOICE#.BIN`.
4. After flash, the version readout for that slot should update to
   `0x31`. Repeat for each voicecard you want to update.

> **Mismatched versions can corrupt the per-step snapshot protocol** —
> step locks rely on a fixed 20-byte payload that both sides must agree
> on. If you see triggering anomalies after a partial flash, finish
> flashing all six voicecards.

---

## Hardware

- **Display:** 2 × 40 character LCD
- **Push encoder:** turn + click
- **Pots:** 4 above the LCD (top row) + 4 below (bottom row)
- **Buttons:** 8 buttons with LEDs, labeled `S1`..`S8`
- **Audio outputs:** 6 individual voice outs + 1 mix out (808-style
  normalling — plugging an individual out removes that voice from the
  mix)
- **Storage:** SD card (FAT16/FAT32, 8.3 filenames) for state snapshots,
  patch slots, and firmware flashing

This manual uses `S1`..`S8` for the eight buttons. Pots are referenced
as `top1`..`top4` and `bot1`..`bot4` — left-to-right within each row.

---

## Navigation

### The eight pages

The eight buttons select the eight top-level page groups:

| Button | Group           | Pages in group                                |
|--------|-----------------|-----------------------------------------------|
| `S1`   | Oscillators     | S1a Oscillators, S1b Mixer                    |
| `S2`   | Filter          | S2 (single page)                              |
| `S3`   | Envelopes + LFO | S3a Amp+Filter env, S3b Pitch env + LFO       |
| `S4`   | Mod matrix      | <!-- VERIFY: S4 = mod matrix surface for issue #11. Confirm whether it lives on its own page or shares with S3. --> |
| `S5`   | Sequencer mode  | S5a Step, S5b Voice 1, S5c Voice 2 (encoder)  |
| `S6`   | Per-track       | S6a Track settings, S6b Performance mixer     |
| `S7`   | Transport       | Single page (PLAY/PAUSE/STOP/RESET, BPM, swing)|
| `S8`   | Patch + system  | S8a Patch slots, S8b Firmware / system <!-- VERIFY: per issue #10 --> |

Within a group, the **second tap of the same button** cycles to the
group's next page. Pressing a different button jumps groups.

### Encoder

- **Turn** — scrolls through the active page's parameters. On most
  pages the cursor advances one parameter per detent; on sequencer
  mode (S5) the encoder walks through all 24 cells across the three
  lock pages and *spills* into the neighboring page group at the
  boundaries (cursor 0 → S3, cursor 23 → S6).
- **Click** — enters focused-edit mode on the highlighted parameter.
  The full parameter name and value are shown on row 2; the encoder
  turn directly adjusts the value. Click again to exit.
  <!-- VERIFY: per issue #15, encoder click on sequencer pages also enters focused-edit. Substep editor entry on `subs` cell is the existing exception. -->

### Page-jump chord

Holding `S2` or `S8` while turning the encoder applies an ×8
multiplier — a fast page-jump shortcut to walk across page groups
without leaving the encoder. Single-detent turns without the chord
walk one cell at a time as usual.

### Pots

Pots always edit the eight parameters currently displayed. The
parameter abbreviations on row 1 of the LCD identify which pot drives
which value. On pages with fewer than 8 active parameters, unused pots
are inert.

### Sequencer mode toggle

`S5` enters **Sequencer Mode** (per-step parameter editing on the
active track). A second `S5` press exits back to normal mode. While
in sequencer mode, `S1`..`S8` are step triggers, not page-jump
shortcuts; encoder navigation is the way out (or `S5` again).

---

## Page reference

### S1 — Oscillators (S1a) / Mixer (S1b)

The voice's two oscillators and their mix into the audio path. Voice
defaults for the per-step lockable osc parameters live here as well —
`NOTE`, `WAVE1`, `PARA1`, `BLND`, `RTIO`, `WAVE2`, `PARA2`, `FINE` are
also lockable from S5b (Voice 1 sequencer page). Turning the
corresponding knob on S1a while no step is held in S5 sets the **track
default** for that parameter.

- **`WAVE1` / `WAVE2`:** algorithm select for each oscillator. Palette
  spans modern PolyBLEP saw/PWM, sine, triangle, FM/FM-fb, dirty PWM,
  filtered noise, vowel synthesis, 16 wavetable banks, wavequence,
  the original pre-PolyBLEP saw, and a wavefolder.
  <!-- VERIFY: wavefolder availability tied to issue #18 close + voicecard flash budget. If the build pulls the CZ filter-sim variants to make room, note that. -->
- **`PARA1` / `PARA2`:** algorithm-specific parameter — PWM amount for
  square, formant select for vowels, FM index for `fm`, fold depth for
  the wavefolder, etc.
- **`BLND`:** bipolar oscillator interaction.
  - `0`–`63`: crossfade between Osc 1 and Osc 2 in the audio path
    (`0` = Osc 1 only, `63` = Osc 2 only). No FM.
  - `64`–`127`: Osc 2 is removed from the audio path and routed as an
    FM modulator into Osc 1. FM depth scales from `0` at `64` to max
    at `127`. The `63→64` transition is acoustically silent (both
    endpoints are "Osc 1 only, no FM").
- **`RTIO`:** Osc 2 frequency ratio relative to Osc 1, drawn from a
  DX-style ratio LUT (0.125, 0.25, 0.5, … 1, √2, π/2, 2, 3, π, 4, 5,
  8, …).
- **`FINE`:** ±100-cent detune on Osc 2.
- **`TUN2`:** semitone-coarse pitch offset on Osc 2 (lockable, signed).
- **`NOTE`:** carrier base note; usually driven from the sequencer
  rather than this page.

The S1b Mixer sub-page exposes the sub-osc and noise mix levels that
also appear on the sequencer S5c page — handy for setting voice-wide
defaults.

<!-- VERIFY: exact knob layout on S1a/S1b to be re-walked against controller/ui.cc page_registry entries before publishing. -->

### S2 — Filter

The state-variable filter and bit-reduction stage.

- **`FREQ`:** SVF cutoff. The filter envelope (Env 2) modulates around
  this point.
- **`RES`:** resonance. High resonance at moderate cutoff produces
  ringing, near-self-oscillation tones.
- **`TYPE`:** LP / BP / HP output select. LP for body and warmth, HP
  for hats and cymbals, BP for nasal/metallic character.
- **`DRIV`:** filter drive — gain into the SVF. At high levels adds
  saturation and grit; interacts with resonance.
- **`BITS`:** pre-DAC bit reduction. `0` = full resolution, max =
  1-bit (extreme digital grit). The filter then shapes the added
  harmonics.

For pure passthrough (filter "off"): `FREQ` to max, `RES` to min.

`FREQ`, `FAMT` (Env 2 → cutoff depth), `PAMT` (Env 3 → pitch depth),
and the sub-osc `WAVE` are also lockable per step from S5c.

### S3 — Envelopes + LFO

Three independent ADR + Curve envelopes with fixed routing, plus one
voice LFO. Each envelope has three control bytes (`rise`, `fall`,
`curv`) plus a depth byte that lives in the modulation slot it drives.
There is **no sustain stage** — `curv` controls the linear-to-exponential
shape blend of decay/release (`0` = linear, `127` = exponential).

| Envelope     | Routing                            | Depth          |
|--------------|-----------------------------------|----------------|
| Env 1 (VCA)  | Output amplitude                  | `amp`          |
| Env 2 (VCF)  | SVF cutoff                        | `flt`          |
| Env 3 (Pitch)| Osc 1 base pitch (coarse, ±)      | `pitc`         |

#### S3a — Amp + Filter envelopes

```
top  rise | fall | curv | amp     ← Env 1 (VCA)
bot  rise | fall | curv | flt     ← Env 2 (VCF)
```

The depth knob doubles as the row label: `amp` and `flt` identify the
envelope.

#### S3b — Pitch envelope + voice LFO

```
top  rise | fall | curv | pitc    ← Env 3 (Pitch)
bot  rate | shap | dest | dept    ← LFO 4 (voice LFO)
```

LFO controls:

- **`rate`:** modulation rate.
- **`shap`:** waveform — sine, triangle, square, ramp, S&H/random.
- **`dest`:** modulation destination (oscillator pitch, cutoff, FM
  depth, etc.).
- **`dept`:** modulation depth, signed (`-63..+63`) so the LFO can
  modulate negatively.

Per-step lockable: only the three `fall` bytes (Env 1 / Env 2 / Env 3)
participate in the lock system as `adec` / `fdec` / `pdec` on the S5c
sequencer page. The other envelope bytes are voice-wide.

LFO 4 (voice LFO) gains tempo-synced rate division at v4.0 — when sync
is engaged, `rate` selects a clock-relative division instead of a free
running speed. <!-- VERIFY: per issue #12, exact UI surface for LFO sync (knob position, label, available divisions). -->

### S4 — Mod matrix

A small modulation matrix with up to four active assignments. Each
slot picks a source, destination, and amount.

<!-- VERIFY: per issue #11, the mod matrix is being scoped down from YAM's full matrix. Document the v4.0 surface (sources / destinations / slot count / UI) once the implementation lands. The PAGE_MODULATIONS registry entry exists but is currently `0xff` placeholder. -->

### S5 — Sequencer mode

See [Sequencer mode](#sequencer-mode) below — the largest section of
this manual.

### S6a — Per-track settings

Per-track pattern configuration. Pots map directly into the active
track's `pattern[]`:

```
top  DIRN | CDIV | ROTA | LENG
bot  SCAL | ROOT | ---- | VOL
```

- **`DIRN`:** direction — fwd / rev / pend (pendulum) / rnd.
- **`CDIV`:** clock division — the polymeter engine. Values are stored
  as ratios: `1/4`, `1/3`, `1/2`, `2/3`, `3/4`, `1/1`, `3/2`, `2/1`.
  Two tracks at different ratios drift in and out of phase as their
  patterns cycle. <!-- VERIFY: ratio labels per issue #14. Until that ships, raw indices `1, 2, 3, 4, 6, 8, 12, 16` may still be displayed. -->
- **`ROTA`:** rotate pattern start point (0–7) without altering step
  data. Useful for shifting the downbeat of a fixed pattern.
- **`LENG`:** pattern length in steps (1–8). Combined with `CDIV`,
  creates per-track polymeters.
- **`SCAL`:** scale quantizer — chro / maj / min / dor / mix / pMa
  (penta major) / pMi (penta minor) / blu (blues). All resolved step
  notes are quantized into the active scale.
- **`ROOT`:** scale root, in semitones from C (0–11).
- **`VOL`:** track volume — multiplicative scale on resolved step
  velocity. `255` is identity, `0` mutes the track.

The middle slot in the bottom row (`----`) is reserved for a future
track-transpose control. The slot is inert today.

The active track is selected by `VSEL` — see Voice selection below.

### S6b — Performance mixer

Live performance control for the six tracks: per-voice volume, mute,
audio mute, and solo, all reachable from a single page. Designed for
performance, not patch design — **state is transient** (cleared on
power-cycle, not saved with the patch slot).

```
Pots:  top1 top2 top3 top4         Buttons:  S1 S2 S3 S4 S5 S6 S7   S8
       bot1 bot2 bot3 bot4                   v1 v2 v3 v4 v5 v6 mode unmute
```

- **Pots `top1`..`top3` / `bot1`..`bot3`:** per-voice volume. **Pickup
  catch:** when entering the page, each pot must physically cross the
  stored volume value before it begins writing — prevents a
  destination jump from the resting pot position.
- **`S1`..`S6`:** toggle the active mode's bit for that voice.
- **`S7` tap:** cycle mode `MT-S → MT-A → SOLO → MT-S`. The `S7` LED
  encodes mode (off / dim red / bright red).
- **`S7` hold + `S1`..`S6` taps:** queue toggles. On `S7` release, the
  queued voices are XOR'd into the active mode's bits as a single
  batch. Queued voices blink red during the hold.
- **`S8` tap:** unmute-all — clear all three bit sets at once.

Three modes:

| Mode  | Skip future fires | Action on toggle of currently-sounding voice |
|-------|-------------------|-----------------------------------------------|
| MT-S  | yes               | none — current note's envelope completes      |
| MT-A  | yes               | instant audio cut (kill voice)                |
| SOLO  | non-solo'd voices | kill non-solo voices that just lost audibility|

The encoder walks an 8-cell cursor; spills to the neighboring page at
the boundary.

### S7 — Transport

Master clock, transport, and the panic / master-reset buttons.

```
top  bpm  | swng | ---- | ----
bot  ---- | ---- | ---- | ----

Buttons:  S1   S2    S3   S4    S5  S6  S7  S8
          PLAY PAUS  RST  STOP  --  --  --  mrst
```

<!-- VERIFY: button slot assignments and S7b page (if any) per issue #2. -->

- **`bpm` (top1):** master tempo in BPM.
- **`swng` (top2):** swing — delays even-numbered steps to add
  push/pull feel to the groove.
- **`PLAY` / `PAUS` / `RST`:** standard transport.
- **`STOP` (S4):** single tap = `Pause` + `Reset`. **Double-tap within
  300 ms = panic** — `Pause` + `Reset` + kill all six voices
  immediately. The S4 LED lights red while the second-tap window is
  open.
- **`mrst` Master Reset (S8):** all sequencer state back to defaults
  (excluding patch slots). <!-- VERIFY: exact behavior + confirm prompt UX per issue #9. -->

### S8a — Patch slots

A simple slot-based patch storage system. The whole instrument state —
all six tracks of pattern data, locks, voice defaults, voice config,
mod matrix, global settings — saves as a single file per slot. There
are no kits/parts/multis abstractions: one slot = one full state.

```
top  name | ---- | ---- | ----
bot  ---- | ---- | ---- | ----

Buttons:  S1    S2 S3 S4 S5  S6      S7 S8
          save  -- -- -- --  update  -- exit
```

<!-- VERIFY: per issue #10. Layout, naming pot, slot-cycle gesture, and save/update/exit semantics confirmed against shipping firmware. -->

- **Slot list:** the encoder cycles between numbered slots (1, 2, …).
  An asterisk (`*`) after the slot name indicates a written slot.
- **Pot `top1` `name`:** spell the slot name.
- **`S1` `save`:** write current state into the highlighted slot. If
  the slot is already written, prompts to confirm overwrite.
- **`S6` `update`:** save the current state into the **already-loaded**
  slot — overwrites without re-naming.
- **`S8` `exit`:** leave the page.

Action buttons render green; `exit` renders red, by Ambika
convention.

A future revision will add a copy/paste sub-menu for moving voices or
sequence patterns between tracks.

### S8b — Firmware / system

The OS Info page — version readouts for the controller and each
voicecard, and the gateway to per-card firmware flashing (see
[Firmware installation](#firmware-installation) above). System
settings (MIDI channel, MIDI thru, etc.) also live in this page
group.

---

## Sequencer mode

Press **`S5`** to enter. Press **`S5`** again to exit.

In sequencer mode, the eight buttons (`S1`..`S8`) are step triggers
for the active track. The eight pots edit the lockable parameters of
the **active lock-page** (Step / Voice 1 / Voice 2). The encoder
walks across all 24 cells; the active page is `cursor >> 3`.

### Voice selection

Hold `S8` and turn the encoder to cycle the active track / voice. All
sequencer-mode editing — step toggles, knob writes, lock writes —
applies to the active voice's pattern.

### Step on/off

**Tap a step button** (without holding it long enough for a hold
gesture) to toggle that step's main `step_flags & on` bit. The
`LED_1`..`LED_8` row lights dim for steps that are on; the trailing
playhead step lights bright while transport is playing.

### Lock-page cycling

The encoder walks `cursor` from 0 to 23. The 4-character abbreviations
on row 1 of the LCD show the active page's eight cells; the cursor `>`
marker precedes the focused cell. The active page is mirrored as a tag
in the top-right (`v1` / `v2` / `sp`):

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

**Held step + pot turn while focus is on `subs`:** standard lock write
on `subs` (count + mode).

**Encoder click while a step is held + cursor on `subs`** — and the
step has SSUB ≠ 0 or REPT ≠ 0 — enters the **substep editor** (see
below).

**Double-tap a step button while holding it:** clears all locks for
that step back to track defaults. <!-- VERIFY: per issue #16 hold-step polish. Today the gesture is "any held step + pot turn writes lock"; long-press detection + double-tap clear are part of the v4.0 polish. -->

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
| `vamt` | Velocity → VCA depth| Voice-wide config (mod slot 11 amount). Determines how strongly velocity scales VCA depth. |
| `rate` | Step rate           | Per-step CDIV override — fires faster / slower than the track CDIV. <!-- VERIFY: ratio labels per issue #14. --> |
| `subs` | Sub-steps / mode    | Bipolar combined SSUB+REPT cell. CCW = repeats `8r`..`1r`, deadzone at center, CW = ratchets `1x`..`8x`. Display: `Nr` / `0` / `Nx` / `cus` (custom from substep editor). |
| `prob` | Probability         | 0..100% — step fires or is silently skipped. Affects only the main step; ratchets and REPT re-fires inherit the probability outcome. |
| `glid` | Glide               | Tie this step to the next — suppresses envelope retrigger on the next step (legato). |
| `gtim` | Glide time          | Voice-wide portamento time on glided notes. |

**Iterative probability modes:** `prob` supports modes beyond raw
percentage at v4.0 — e.g., "1-of-N", "first / last / not-first /
not-last". <!-- VERIFY: per issue #6, full mode list + UI. -->

### S5b — Voice 1 page (cursor 8..15)

```
top  nois | w1   | pa1  | tun2
bot  mix  | w2   | pa2  | fin2
```

| Cell    | Name                 | Range / behavior |
|---------|----------------------|------------------|
| `nois`  | Noise mix            | White noise level into the audio path. |
| `w1`    | Osc 1 wave           | Algorithm select; renders as 6-char waveform name. |
| `pa1`   | Osc 1 PARA           | Algorithm-specific parameter (PWM, formant, FM index, fold depth, …). |
| `tun2`  | Osc 2 coarse tune    | Signed semitone offset on Osc 2 (lockable; reclaimed dead E1 release slot). |
| `mix`   | BLND (osc interaction)| Bipolar 0..63 mix / 64..127 FM (clamped to 0..63 in default builds; 64+ pending linear-FM work). |
| `w2`    | Osc 2 wave           | Algorithm select. |
| `pa2`   | Osc 2 PARA           | Algorithm-specific parameter. |
| `fin2`  | Osc 2 fine tune      | Signed cents detune on Osc 2 (lockable). |

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
| `pamt` | Pitch env depth            | Env 3 → Osc 1 base pitch. Signed; positive sweeps up to base, negative sweeps down to base. |
| `sub`  | Sub-osc level              | Sub-osc / transient layer mix into the audio path. |
| `wave` | Sub-osc shape              | 11-shape palette: 6 traditional sub-bass shapes (Square 1/2, Triangle 1/2, Pulse 1/2) + 5 transient one-shots (Click, Glitch, Blow, Metallic, Pop). Switching between traditional and transient shapes per step is a primary percussion-expressive tool. |

### Substep editor

Hold a step button, scroll the cursor to `subs`, and click the
encoder. Two requirements: the cursor must be on `subs`, and the held
step must have either SSUB ≠ 0 or REPT ≠ 0 (otherwise the click is a
no-op).

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
- **Pot 1 (`mint` slot):** **MINT** — semitone walk per sub-trigger,
  0..24. Labels: `off`, `m2`..`M7`, `8va`, `8m2`..`8va2`.
- **Pot 2 (`mdir` slot):** **MDIR** — direction: `up` / `dn` / `ud`
  (ping-pong) / `rnd`.
- **`S1`..`S8`:** toggle individual `substep_bits` slots. Slots above
  the active count are inactive (button no-op, LED dark).
- All other pots are inert while the editor is active.

LCD layout while editing:

```
Line 0:  subs Nr | MINT m3 | MDIR up
Line 1:  # # - # # # - -    (one slot per active position)
```

LEDs mirror `substep_bits`, masked to the active count.

On entry, `substep_bits` is trimmed to the active range. Stale
out-of-range bits from a prior session are cleared; if nothing
survives the trim, all active slots re-enable.

**Exit:** click the encoder again.

### Mutation (MINT + MDIR)

When MINT is non-zero, repeats and ratchet sub-triggers walk the step
pitch by `MINT` semitones per fire. Direction is set by MDIR:

| MDIR | Label | Effect                                        |
|------|-------|-----------------------------------------------|
| 0    | `up`  | `+sub_idx × MINT`                             |
| 1    | `dn`  | `−sub_idx × MINT`                             |
| 2    | `ud`  | Odd `sub_idx`: `+MINT`; even: `−MINT`         |
| 3    | `rnd` | Random `±MINT` offset per fire                |

The note is clamped to 0..127 and re-quantized to the track's scale
after the offset is applied. Mutation is resolved on the controller —
the voicecard receives the final computed pitch.

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
│  mix)  │                └──────────┘    │ + SUB   │    │ + DRIV   │
└────────┘                                 │ + NOIS  │    └────┬─────┘
                                           └─────────┘         ▲
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

| Envelope     | Drives          | Decay knob | Depth knob |
|--------------|-----------------|------------|------------|
| Env 1 (VCA)  | Output amplitude| `adec`     | `amp`      |
| Env 2 (VCF)  | SVF cutoff      | `fdec`     | `flt` (`famt`) |
| Env 3 (Pitch)| Osc 1 base pitch| `pdec`     | `pitc` (`pamt`) |

The three `fall` bytes are the only envelope parameters that
participate in the per-step lock system (as `adec` / `fdec` / `pdec`
on S5c).

### Dual oscillator (Osc 1 = carrier, Osc 2 = modulator)

Two oscillator slots, each independently selecting from YAM's wave
shape palette plus elkhart's wavefolder. The carrier (Osc 1)
determines perceived pitch; the modulator (Osc 2) is either mixed
into the audio path or routed to FM-modulate Osc 1, controlled by the
single bipolar `BLND` knob.

#### `BLND` zones

- **Below center (0–63):** crossfade between Osc 1 and Osc 2 in the
  audio path.
- **At center (63→64):** acoustically silent transition — both
  endpoints are "Osc 1 only, no FM."
- **Above center (64–127):** Osc 2 modulates Osc 1's pitch (FM). Osc
  2 leaves the audio path. FM amount sweeps from 0 at 64 to maximum
  at 127. <!-- VERIFY: linear-FM mode availability tied to BLND ≥ 64 work; current default builds clamp BLND to 0..63. Confirm before publishing. -->

At any setting you hear *either* a mix of Osc 1 and Osc 2 *or* Osc 1
being FM'd by Osc 2 — never both. This constraint matches the natural
FM design pattern of operator-as-modulator-only.

### Sub-oscillator and transient layer

The sub-oscillator slot doubles as a transient generator. Eleven
shapes total: six traditional sub-bass shapes (Square, Triangle, Pulse
— each in two octave-related variants) and five transient one-shots
(Click, Glitch, Blow, Metallic, Pop). Selecting a transient shape
produces a one-shot attack on note-on, mixed at the `SUB` level into
the main path. Independent of Osc 1 / Osc 2 — you can layer a Click
transient under a vowel-and-FM voice without touching either
oscillator's wave selection.

### Wavefolder

Elkhart adds a wavefolder waveform alongside YAM's palette: a
Carcosa-style iterative quadratic. Selecting wavefolder for `WAVE1`
or `WAVE2` exposes fold depth on the `PARA` knob — at low depths it
behaves like a soft saturator on the input wave; at high depths it
reflects into harmonic-rich folded territory, useful for textural
leads, metallic strikes, and noise-adjacent percussion.

<!-- VERIFY: presence in the v4.0 build per issue #18. If voicecard flash budget required pulling the CZ filter-sim variants to make room, note the palette change here. -->

---

## Appendix — parameters by page

`†` = also appears as a per-step lockable in sequencer mode.

### S1a Oscillators (voice config; `†` items also lockable on S5b)

| Abbrev | Name           | Notes |
|--------|----------------|-------|
| `WAVE1`†| Osc 1 wave    | Algorithm select |
| `PARA1`†| Osc 1 PARA    | Algorithm parameter |
| `BLND` †| Osc interaction| Bipolar mix / FM |
| `RTIO` †| Osc 2 ratio   | DX-style ratio LUT |
| `WAVE2`†| Osc 2 wave    | Algorithm select |
| `PARA2`†| Osc 2 PARA    | Algorithm parameter |
| `FINE` †| Osc 2 cents   | ±100-cent detune |
| `TUN2` †| Osc 2 coarse  | Signed semitone offset |

### S2 Filter (voice config; some `†` lockable)

| Abbrev | Name           | Notes |
|--------|----------------|-------|
| `FREQ` †| Cutoff        | SVF base frequency |
| `RES`   | Resonance     | SVF resonance |
| `TYPE`  | Filter mode   | LP / BP / HP |
| `DRIV`  | Drive         | Saturation gain into SVF |
| `BITS`  | Bit reduction | Pre-DAC bit depth reduction |
| `FAMT` †| Env 2 → cutoff depth | Signed |
| `PAMT` †| Env 3 → pitch depth  | Signed (lives on S5c — exposed here for cross-reference) |

### S3a/S3b Envelopes + LFO (voice config; `fall`/decay bytes are `†` lockable)

| Abbrev | Name              | Notes |
|--------|-------------------|-------|
| `rise` (E1) | Env 1 attack | |
| `fall` (E1)†| Env 1 decay/release rate (`adec` lockable) | |
| `curv` (E1) | Env 1 curve blend | 0 lin .. 127 expo |
| `amp`  | Env 1 → VCA depth | |
| `rise` (E2) | Env 2 attack | |
| `fall` (E2)†| Env 2 decay/release (`fdec`) | |
| `curv` (E2) | Env 2 curve blend | |
| `flt`  | Env 2 → cutoff depth | (= `famt` on S5c) |
| `rise` (E3) | Env 3 attack | |
| `fall` (E3)†| Env 3 decay/release (`pdec`) | |
| `curv` (E3) | Env 3 curve blend | |
| `pitc` | Env 3 → pitch depth | (= `pamt` on S5c) |
| `rate` | LFO 4 rate (sync-aware) | |
| `shap` | LFO 4 waveform | |
| `dest` | LFO 4 destination | |
| `dept` | LFO 4 depth | Signed -63..+63 |

### S4 Mod matrix

<!-- VERIFY: parameter list pending issue #11 implementation. -->

### S6a Per-track settings (track config)

| Abbrev | Name           | Notes |
|--------|----------------|-------|
| `DIRN` | Direction      | fwd / rev / pend / rnd |
| `CDIV` | Clock division | Ratio: 1/4..2/1 |
| `ROTA` | Rotate         | Pattern start offset |
| `LENG` | Length         | Steps per cycle (1–8) |
| `SCAL` | Scale          | chro / maj / min / dor / mix / pMa / pMi / blu |
| `ROOT` | Root           | Semitone offset (0–11) |
| `VOL`  | Volume         | Velocity scale (0–255) |

### S6b Performance mixer (transient)

Per-voice volume + MT-S / MT-A / SOLO toggles. State not persisted.

### S7 Transport

| Abbrev | Name      | Notes |
|--------|-----------|-------|
| `bpm`  | Tempo     | Master BPM |
| `swng` | Swing     | Groove amount |
| —      | PLAY/PAUS/RST/STOP/mrst | Buttons (S1/S2/S3/S4/S8) |

### S5a Step page (per-step lockable)

| Abbrev | Name              | Notes |
|--------|-------------------|-------|
| `note` | Step note         | Quantized to track scale |
| `vel`  | Velocity          | 0..127 |
| `vamt` | Vel → VCA amount  | Voice-wide config (mod slot 11) |
| `rate` | Step rate         | Per-step CDIV override |
| `subs` | Sub-steps / mode  | Bipolar SSUB + REPT cell |
| `prob` | Probability       | 0..100% + iterative modes |
| `glid` | Glide             | Tie to next step |
| `gtim` | Glide time        | Voice-wide portamento |

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

| Abbrev | Name              | Notes |
|--------|-------------------|-------|
| `freq` | Filter cutoff     | |
| `fdec` | Env 2 decay       | |
| `famt` | Env 2 → cutoff depth | Signed |
| `adec` | Env 1 decay       | |
| `pdec` | Env 3 decay       | |
| `pamt` | Env 3 → pitch depth | Signed |
| `sub`  | Sub-osc level     | |
| `wave` | Sub-osc shape     | 11-shape palette |

---

## License

GPLv3, inherited from upstream Mutable Instruments / YAM. Voice DSP
adapted from `bjoeri/ambika` (YAM); original Ambika firmware by Émilie
Gillet. Contains a variant of Peter Knight's Cantarino formant
synthesis algorithm.
