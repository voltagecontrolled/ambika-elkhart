# Elkhart — User Manual

*For Elkhart firmware v4.0, running on Mutable Instruments Ambika
hardware.*

Elkhart turns the Ambika into a six-voice polymetric step sequencer.
Each of the six voices runs an independent synth with its own
pattern, step length, scale, direction, and length, so tracks drift
in and out of phase as they cycle. Per-step parameter locks let you
reshape any voice on a step-by-step basis — pitch, oscillator
algorithm, envelope decay, sub-osc shape, and twenty-one other
parameters can all be overridden on a single step.

The voice engine is a port of the YAM oscillator family: dual
oscillators with FM, eight-mode wavetables, vowel synthesis, a
state-variable filter, fuzz, and bit reduction. It's at home on
percussion — six independent voices, transient sub-osc shapes, and
tight envelope macros — but it covers melodic and textural ground
just as well.

## Hardware tour

Working from the front panel:

- **Display.** Two rows of forty characters. The top row usually
  shows parameter abbreviations; the bottom row shows their values.
- **Push encoder.** Turn to move between parameters or change pages.
  Click to drill into a focused edit, or to enter the substep editor
  in sequencer mode.
- **Eight pots.** Four above the display, four below. Pots edit the
  eight parameters currently shown; they're addressed left-to-right
  within each row.
- **Eight buttons (S1–S8).** Each button selects a page group. In
  sequencer mode the same eight buttons become step triggers for the
  active voice.
- **Audio outputs.** Six individual voice outputs plus a mix
  output. The six jacks are normalled 808-style: plugging a cable
  into an individual voice output removes that voice from the mix
  bus, so you can route a kick or hat to its own channel without
  manually muting it elsewhere.
- **MIDI in / out.** Standard 5-pin DIN on the back panel.
- **SD card slot.** Used for snapshots (save / load) and firmware
  flashing. Cards must be FAT16 or FAT32, with 8.3 filenames.

Throughout this manual, button references use the labels on the
panel — `S1` through `S8`. The encoder is just "the encoder."
Hold-button gestures (where you press and hold a button while
turning or clicking the encoder) are referred to as **combos**.

## Navigation

The encoder, eight buttons, and a handful of hold-button **combos**
move you between pages and voices. On most pages the eight buttons
are page selectors. On a few — sequencer mode and the performance
mixer — the buttons are repurposed and the combos are the only way
to jump pages.

### Combos

Hold the listed button and operate the encoder. Combos work from
any page, including pages that have taken over the buttons.

| Combo                        | Action                                              |
|------------------------------|-----------------------------------------------------|
| `S1` + turn                  | Select active voice (1–6)                           |
| `S2` or `S8` + turn          | Jump between settings pages                         |
| `S5` + turn CW               | Play / pause                                        |
| `S5` + turn CCW              | Stop (notes ring out per envelope release)          |
| `S5` + turn CCW × 2 (400 ms) | Panic — hard mute on every voice                    |
| `S7` + turn                  | Flip between Transport and Performance mixer        |

### Pages

Tap the listed button to jump to its page. Tap the same button
again to cycle to the next page in its group.

| Button | Page                                              |
|--------|---------------------------------------------------|
| `S1`   | Oscillators / Mixer (2 pages)                     |
| `S2`   | Filter                                            |
| `S3`   | Envelopes + LFO (2 pages: amp+filter, pitch+LFO)  |
| `S4`   | Shares the `S3` group (no dedicated page in v4.0) |
| `S5`   | Sequencer mode toggle                             |
| `S6`   | Per-track settings / Performance mixer (2 pages)  |
| `S7`   | Transport                                         |
| `S8`   | System (snapshots, firmware, info)                |

### Buttons as page selectors vs. button takeover

On most pages, `S1`–`S8` are page selectors as listed above. Two
pages take the buttons over for their own use:

| Page                        | Button role on that page                                              |
|-----------------------------|------------------------------------------------------------------------|
| **Sequencer mode (S5)**     | `S1`–`S8` are step triggers for the active voice                       |
| **Performance mixer (S6b)** | `S1`–`S6` toggle voice mutes/solos; `S7` cycles modes; `S8` unmutes all|

While buttons are taken over, **the combos table is the only way to
change pages or voices**. To return to normal page selection: leave
sequencer mode by tapping `S5` again, or leave the performance mixer
by using a combo (typically `S7` + turn back to Transport, or any of
the page-jump combos).

### Encoder

| Action                | Effect                                                                |
|-----------------------|-----------------------------------------------------------------------|
| Turn                  | Walk the cursor across parameters; cross page boundaries automatically|
| Click                 | Enter / exit focused edit on the highlighted parameter                |
| Click in sequencer mode (with a step held, cursor on `subs`) | Open substep editor |

### Pots

The eight pots always edit the eight parameters currently displayed
on screen, addressed left-to-right within each row. Pots that
correspond to inactive cells on the current page are inert.

## Oscillators and Mixer (`S1`)

The `S1` group has two pages — Oscillators and Mixer — for the
voice's two-oscillator core, sub-oscillator, noise, and the post-
oscillator character controls.

### Oscillators page

Each oscillator has the same four controls: waveform algorithm,
algorithm parameter, coarse range, and fine tune.

```
wave saw  | para  64 | rang  +0 | tune   +0      ← Osc 1
wave fm   | para  90 | rang +12 | tune  -05      ← Osc 2
```

| Cell   | Range            | Notes                                                                 |
|--------|------------------|-----------------------------------------------------------------------|
| `wave` | 44 algorithms    | Selects the oscillator algorithm. Lockable per step.                  |
| `para` | 0–127            | Algorithm-specific parameter (PWM amount, formant, FM index, …). Lockable per step. |
| `rang` | ±24 semitones    | Coarse pitch offset. On Osc 2, lockable per step.                     |
| `tune` | ±64 cents        | Fine detune. On Osc 2, lockable per step.                             |

Knob writes here set the **track default** for that parameter — the
value used on every step that doesn't have a per-step lock for it.
Per-step locks for the lockable cells are written from the
sequencer-mode pages.

#### Wave palette

The 44 algorithms span several families. The `para` knob means
something different in each family:

- **Analog-style.** Saw and PWM (modern PolyBLEP), sine, triangle.
  `para` controls pulse width on PWM.
- **FM.** A two-operator FM voice and an FM-with-feedback variant.
  `para` is FM index.
- **Vowel synthesis.** Formant-based vowel sounds. `para` selects
  the vowel.
- **Wavetables.** Sixteen wavetable banks plus a wavequence mode.
  `para` scans through the table.
- **Filtered noise.** Coloured noise sources. `para` shapes the
  filter.
- **Period-grit.** A "dirty PWM" with intentional aliasing, the
  pre-PolyBLEP saw kept for character, and several CZ-style filter-
  simulation variants. `para` shapes the timbral character of each.

### Mixer page

This page balances the two oscillators against each other and adds
noise, sub-osc, cross-modulation, and the post-oscillator character
controls.

```
mix   32 | nois  16 | sub   48 | wave  squ1
xmod env | amnt  20 | fuzz  18 | crsh   4
```

| Cell   | Range          | Notes                                                                                                |
|--------|----------------|------------------------------------------------------------------------------------------------------|
| `mix`  | 0–63           | Crossfade between Osc 1 and Osc 2 in the audio path (0 = Osc 1 only, 63 = Osc 2 only). Lockable per step. |
| `nois` | 0–63           | Noise generator level into the audio path. Lockable per step.                                        |
| `sub`  | 0–63           | Sub-oscillator / transient layer level. Lockable per step.                                           |
| `wave` | 11 shapes      | Sub-oscillator shape (see *Sub-oscillator and transient layer*). Lockable per step.                  |
| `xmod` | operator list  | Cross-modulation operator (which oscillator parameter cross-mods into which).                        |
| `amnt` | 0–63           | Cross-modulation depth.                                                                              |
| `fuzz` | 0–63           | Saturation / distortion driven into the filter. Adds harmonic warmth and grit; `0` is clean.         |
| `crsh` | 0–31           | Sample-rate reduction driven into the filter. `0` runs at full rate; higher values progressively downsample for digital crunch and aliasing artifacts. |

`fuzz` and `crsh` live on this page rather than on the Filter page;
both run **before** the filter, so the filter shapes their character
rather than just colouring a clean voice.

### Sub-oscillator and transient layer

The `sub` slot doubles as a sub-bass oscillator and a one-shot
percussive layer. Eleven shapes share the slot — pick a tonal one
for low-end reinforcement, or a transient one to layer a drum-like
attack underneath the main voice without touching either oscillator.

The shape is set by the `wave` cell on the Mixer page; level is set
by `sub` on the same page. Both are lockable per step, so a single
voice can fire a `click` transient on one step and a `squ2` sub on
the next.

| Group       | Shape    | Character                                              |
|-------------|----------|--------------------------------------------------------|
| Tonal sub   | `squ1`   | Square, one octave below                               |
|             | `tri1`   | Triangle, one octave below                             |
|             | `pul1`   | Pulse, one octave below                                |
|             | `squ2`   | Square, two octaves below                              |
|             | `tri2`   | Triangle, two octaves below                            |
|             | `pul2`   | Pulse, two octaves below                               |
| Transient   | `click`  | Short, broadband attack. Hat- and rim-shaped.          |
|             | `glitch` | Pitched digital chirp.                                 |
|             | `blow`   | Soft noise burst, breath-like.                         |
|             | `metal`  | Short FM-flavoured ping.                               |
|             | `pop`    | Pitched body thump.                                    |

Transient shapes fire once at note-on and decay quickly on their
own envelope, independent of Env 1; raising `sub` mixes them louder
into the voice.

## Filter (`S2`)

A single page drives the state-variable filter. Only four of the
eight pot positions are active; the other four are inert.

```
freq  64 | reso  18 | --       | mode  LP
env2  20 | --       | --       | --
```

| Cell   | Range                | Notes                                                                                                                   |
|--------|----------------------|-------------------------------------------------------------------------------------------------------------------------|
| `freq` | 0–127                | Cutoff frequency. Lockable per step.                                                                                    |
| `reso` | 0–63                 | Resonance. High resonance near a strong harmonic produces ringing, near-self-oscillation tones.                         |
| `mode` | LP / BP / HP / Notch | Filter response. **LP** for body and warmth, **HP** for hats and air, **BP** for nasal / metallic character, **Notch** for phaser-like rejection. |
| `env2` | 0–63                 | Filter envelope depth — how far Env 2 sweeps the cutoff around the `freq` setting. Lockable per step (as `famt`).       |

For an effectively open / passthrough sound, set `freq` to maximum
and `reso` to `0`.

The filter envelope itself (rise, fall, curve) is Env 2, edited on
the Envelopes page.

## Envelopes and LFO (`S3`)

Each voice has **three independent envelopes** with fixed routing,
and one LFO. Every envelope is parameterised the same way: an
**attack** rate (`rise`), a **decay/release** rate (`fall`), a
**curve** blend, and a **depth** sent to the envelope's destination.

There is no sustain stage. After the attack completes, each envelope
falls immediately at its `fall` rate. `curv` blends the fall shape
from linear (`0`) to exponential (`127`).

| Envelope | Drives                | Lockable per step |
|----------|-----------------------|-------------------|
| Env 1    | Output amplitude (VCA)| `fall` (as `adec`)|
| Env 2    | Filter cutoff         | `fall` (as `fdec`)|
| Env 3    | Osc 1 base pitch      | `fall` (as `pdec`)|

The `S3` group has two pages: amp + filter envelopes, then pitch
envelope + voice LFO. `S4` shares the same group; tapping `S4`
lands on the pitch + LFO page.

### Amp + Filter envelopes

```
rise   2 | fall  64 | curv  90 | amp  127      ← Env 1 (VCA)
rise   8 | fall  72 | curv  40 | flt   48      ← Env 2 (Filter)
```

The depth knob doubles as the row label so you can tell which
envelope you're editing at a glance: `amp` = Env 1, `flt` = Env 2.

| Cell   | Range                  | Notes                                                                       |
|--------|------------------------|-----------------------------------------------------------------------------|
| `rise` | 0–127                  | Attack rate. Low = snappy; high = slow swell.                               |
| `fall` | 0–127                  | Decay/release rate. Lockable per step.                                      |
| `curv` | 0–127                  | Linear-to-exponential blend on the fall stage.                              |
| `amp` / `flt` | 0–63 (signed for `flt`) | Depth into the envelope's destination. `flt` is the same control as `env2` on the Filter page. |

### Pitch envelope + voice LFO

```
rise   0 | fall  20 | curv 100 | pitc  +24     ← Env 3 (Pitch)
rate  48 | wave tri | dest pit | dept  +32     ← LFO
```

| Cell   | Range            | Notes                                                                   |
|--------|------------------|-------------------------------------------------------------------------|
| `rise` | 0–127            | Env 3 attack.                                                           |
| `fall` | 0–127            | Env 3 decay/release. Lockable per step (as `pdec`).                     |
| `curv` | 0–127            | Env 3 fall curve blend.                                                 |
| `pitc` | signed           | Env 3 → Osc 1 pitch depth. Lockable per step (as `pamt`). Useful for percussive pitch drops or upward "kick" attacks. |
| `rate` | 0–127            | LFO speed. Free-running — there is no tempo sync in this version.       |
| `wave` | sine / tri / square / ramp / S&H | LFO waveform.                                          |
| `dest` | destination list | What the LFO modulates (pitch, cutoff, FM depth, etc.).                 |
| `dept` | −63..+63         | LFO depth, signed — negative values invert the modulation.              |

## Per-track settings (`S6a`)

The `S6` group's first page configures the active voice's
pattern-level behaviour: direction, step length, rotation, scale,
root, and volume. Use the `S1` + encoder combo to pick which voice
you're editing.

```
dirn fwd  | rate 16   | rota   0 | leng   8
scal min  | root   0  | ----     | vol  255
```

| Cell   | Range / values                                                 | Notes                                                                                                            |
|--------|----------------------------------------------------------------|------------------------------------------------------------------------------------------------------------------|
| `dirn` | `fwd` / `rev` / `pend` / `rnd`                                 | Playback direction. `pend` ping-pongs end-to-end; `rnd` jumps to a random step each tick.                        |
| `rate` | 15 musical step lengths (see table below)                      | The track's step length. Different rates on different tracks is what produces polymetric drift.                  |
| `rota` | 0–7                                                            | Rotates the pattern's start point without altering step data — useful for shifting which step lands on the downbeat. |
| `leng` | 1–8                                                            | Pattern length in steps. Combined with `rate`, drives polymetric cycle length.                                   |
| `scal` | `chro` / `maj` / `min` / `dor` / `mix` / `pMa` / `pMi` / `blu` | Quantises every step's note into the chosen scale.                                                               |
| `root` | 0–11                                                           | Scale root, in semitones from C.                                                                                 |
| `----` | inert                                                          | Pot is disabled and the cell renders as `----` in this version.                                                  |
| `vol`  | 0–255                                                          | Per-track volume. Multiplies into each step's velocity. `255` is unity; `0` mutes the track.                     |

### `rate` values

The 15 musical step lengths are evenly spread from 32nd-note
triplets to two-bar steps. Read the suffixes as: no suffix =
straight, `t` = triplet, `d` = dotted, `B` = bars (in 4/4).

| Value | Meaning             |
|-------|---------------------|
| `32`  | 32nd note           |
| `16t` | 16th-note triplet   |
| `16`  | 16th note (default) |
| `8t`  | 8th-note triplet    |
| `16d` | dotted 16th         |
| `8`   | 8th note            |
| `4t`  | quarter-note triplet|
| `8d`  | dotted 8th          |
| `4`   | quarter note        |
| `2t`  | half-note triplet   |
| `4d`  | dotted quarter      |
| `2`   | half note           |
| `1`   | whole note          |
| `1d`  | dotted whole        |
| `2B`  | two bars            |

### Polymeter, briefly

Two tracks at the same `rate` and `leng` lock to the same downbeat
forever. Set them to different `rate` values, or different `leng`
values, and they cycle at different absolute lengths — drifting in
and out of phase. Eight short patterns at six different rates is
the core gesture of this instrument.

## Performance mixer (`S6b`)

A single-page live mixer for the six voices: per-voice volume,
mute, audio-mute, and solo, all reachable from the panel without
diving into menus. Designed for performance, not patch design —
**state is transient and cleared at power-cycle**, not saved with
the snapshot.

```
v1 192 | v2 220 | v3 180 | mode MT-A
v4 255 | v5 200 | v6 128 | clr  unmt
```

### Volumes (pots)

The first six pots set per-voice volume. **Pickup catch:** on entry
to the page, each pot has to physically cross its stored value
before it starts writing — this stops a resting pot position from
yanking a voice's level.

### Buttons

`S1`–`S8` are repurposed on this page; the pages-by-button table in
*Navigation* does not apply here. Use a navigation combo to leave.

| Button       | Action                                                                                |
|--------------|---------------------------------------------------------------------------------------|
| `S1`–`S6`    | Toggle mute or solo for that voice — which one depends on the active mode.            |
| `S7` tap     | Cycle the active mode: `MT-S` → `MT-A` → `SOLO` → `MT-S`. The `S7` LED encodes mode (off / dim red / bright red). |
| `S7` hold + `S1`–`S6` taps | Queue toggles. Queued voices light up green while `S7` is held; on release, all queued voices flip together as a single batch. |
| `S8` tap     | Unmute-all. Clears every bit on every mode at once.                                   |

### The three modes

| Mode   | Stops future fires?       | What happens to a voice already sounding when toggled |
|--------|---------------------------|--------------------------------------------------------|
| `MT-S` | yes                       | nothing — the current note's envelope rings out        |
| `MT-A` | yes                       | instant audio cut                                      |
| `SOLO` | yes (for non-solo voices) | non-solo voices that just lost audibility are killed   |

Use `MT-S` for musical drop-outs (the tail rings out), `MT-A` for a
hard duck, and `SOLO` for isolating a voice without manually
muting the other five.

### Encoder

The encoder walks an 8-cell cursor across the page and spills into
the neighbouring page at the boundaries.

## Transport (`S7`)

A single page for the master clock and transport controls.

```
bpm 120 | swng   0 | mrst off
play paus rst  stop                exit
```

| Cell   | Range / values            | Notes                                                                                                                                       |
|--------|---------------------------|---------------------------------------------------------------------------------------------------------------------------------------------|
| `bpm`  | 40–240                    | Master tempo.                                                                                                                               |
| `swng` | 0–127                     | Swing / groove depth. **Note:** swing handling is limited in this version and may not affect timing as expected.                            |
| `mrst` | `off` / 2–128             | Master reset period, in undivided steps. When set, all six tracks snap back to step 0 each time the master tick counter reaches this number — useful for keeping polymetric tracks at different rates from drifting open-ended. Default `off` (free-run). |

The remaining pot positions are inert.

### Buttons

`S1`–`S4` and `S8` are repurposed on this page for transport.

| Button       | Action                                                                                       |
|--------------|----------------------------------------------------------------------------------------------|
| `S1`         | Play.                                                                                        |
| `S2`         | Pause (notes ring out per the envelope release).                                             |
| `S3`         | Reset — return every track to step 0 without changing transport state.                       |
| `S4` tap     | Stop. Equivalent to pause + reset.                                                           |
| `S4` double-tap (within 300 ms) | **Panic.** Pause + reset + immediate hard mute on every voice. The `S4` LED lights red while the double-tap window is open. |
| `S8`         | Exit — return to the most recent non-system page.                                            |

LED behaviour: the status LED is bright while playing, dim while
paused. `S1` is bright while playing; `S2` is bright while paused;
`S8` stays lit as the exit affordance.

### From any other page

Most transport actions are also available globally via the
hold-`S5` + encoder combo (see *Navigation*) — play / pause, stop,
and panic without leaving the current page.

## Sequencer mode

Tap `S5` to enter sequencer mode for the active voice. Tap `S5`
again to exit. (Use the `S1` + encoder combo to switch which voice
is active.)

In sequencer mode:

- The eight buttons become **step triggers** for the active voice.
- The eight pots edit the **lockable parameters** of the lock page
  the cursor is currently on.
- The encoder walks across the lockable cells (24 in total, spread
  over three lock pages — covered in the next section).

The page-jump combos and global transport combos still work
normally; the cell-selector tables in *Navigation* listing `S1`–`S8`
as page selectors do not apply while sequencer mode is active.

### Steps — tap, hold, double-tap

| Gesture                     | Effect                                                                                  |
|-----------------------------|-----------------------------------------------------------------------------------------|
| Tap (≤ 250 ms)              | Toggle the step on or off.                                                              |
| Hold (> 250 ms), no pot     | **Peek.** The LCD shows that step's locked values. Releasing does **not** toggle the step — peeking is non-destructive. |
| Double-tap (within 300 ms)  | **Clear all locks** for that step. The first tap's toggle is undone, so the step's on/off state is unchanged — only the locks are cleared, returning the step to track defaults. |
| Hold + turn a pot           | Write a per-step lock for that pot's parameter. (Covered in detail in the lock-pages section.) |

Step LEDs: each `S1`–`S8` LED lights **green** for steps that are
on, and lights **red** as the playhead passes over that step
while transport is running.

### Track defaults vs. per-step locks

Each parameter on a step reads from one of two places:

- **Track default** — set by turning a pot in sequencer mode with
  no step held. Heard immediately on every unlocked step.
- **Per-step lock** — written by holding a step and turning a pot.
  Overrides the track default for that one step only.

After a step fires, every parameter snaps back to the track default
for the next step. There is no parameter carryover between steps;
what you see on a step is exactly what fires when that step is
played, and only on that step.

This is the main performance dimension of the sequencer: a single
voice can be a stable timbre on most steps and surprise on a few,
or it can be reshaped step-by-step by dialing track defaults
underneath a fixed set of locks.

## Sequencer mode — the three lock pages

The encoder walks across all 24 lockable cells, eight per page, in
this order: **Step → Voice 1 → Voice 2**. The active page flips
automatically as the cursor crosses each eight-cell boundary; the
parameter abbreviations on the top row of the LCD show which page
you're currently on. Turning past the last cell spills out into
the Envelopes group; turning before the first cell spills out into
Per-track settings.

To write a lock on any of these cells: hold the step button and
turn the pot for that cell. To peek at what a step has locked, just
hold the step (don't touch a pot).

### Step page

Per-step performance and timing controls.

```
note C 3 | vel  100 | vamt  64 | rate trk
subs   0 | prob 127 | glid   0 | sfx none
```

| Cell   | Range / values                                                                  | Notes                                                                                                                                                                                  |
|--------|---------------------------------------------------------------------------------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `note` | 0–127, shown as note name                                                       | Step note. Quantised to the track's scale and root.                                                                                                                                    |
| `vel`  | 0–127                                                                           | Step velocity.                                                                                                                                                                         |
| `vamt` | 0–127                                                                           | Velocity → VCA depth. **Voice-wide** — not lockable per step (set this once for the voice).                                                                                            |
| `rate` | `trk` or any value from the per-track `rate` list (`32`…`2B`)                   | Per-step rate override. `trk` inherits the track rate; any other value replaces it on this step only.                                                                                  |
| `subs` | repeats (CCW), `0` (centre), ratchets (CW)                                      | Bipolar substep cell. CCW values fire the step on subsequent periods; CW values pack multiple sub-triggers into the step's own period. The substep editor extends both modes — see the next section. |
| `prob` | 0–127 (≈ 0–100 %)                                                               | Probability that the step fires. Also gates whether `sfx` takes effect.                                                                                                                |
| `glid` | 0–127                                                                           | Per-step glide / portamento time. `0` = no glide.                                                                                                                                      |
| `sfx`  | `none`, `skip`, `fwd`, `rev`, `dir`, `rjmp`, `jmp1`–`jmp8`                      | Per-step modifier. No track default — shows `----` when no step is held. Gated by `prob`. `skip` advances without firing; `fwd`/`rev` set track direction sticky; `dir` toggles direction; `rjmp` jumps to a random step; `jmp1`–`jmp8` jump to that absolute step. |

### Voice 1 page

Oscillator-side per-step locks.

```
nois  16 | w1   saw | pa1  64 | tun2 +07
mix   32 | w2   fm  | pa2  90 | fin2 -05
```

| Cell   | Range                                | Notes                                                       |
|--------|--------------------------------------|-------------------------------------------------------------|
| `nois` | 0–63                                 | Noise level.                                                |
| `w1`   | 44 algorithms, shown as wave name    | Osc 1 algorithm.                                            |
| `pa1`  | 0–127                                | Osc 1 algorithm parameter.                                  |
| `tun2` | signed semitones                     | Osc 2 coarse tune (the same control as `rang` for Osc 2).   |
| `mix`  | 0–63                                 | Oscillator balance.                                         |
| `w2`   | 44 algorithms                        | Osc 2 algorithm.                                            |
| `pa2`  | 0–127                                | Osc 2 algorithm parameter.                                  |
| `fin2` | signed cents                         | Osc 2 fine tune (the same control as `tune` for Osc 2).     |

### Voice 2 page

Filter, envelope, and sub layers per-step.

```
freq  64 | fdec  72 | famt +20 | adec  64
pdec  20 | pamt +24 | sub   48 | wave squ1
```

| Cell   | Range            | Notes                                                                  |
|--------|------------------|------------------------------------------------------------------------|
| `freq` | 0–127            | Filter cutoff base. The filter envelope sweeps around this.            |
| `fdec` | 0–127            | Filter envelope (Env 2) decay rate.                                    |
| `famt` | signed           | Filter envelope depth (the same control as `flt` / `env2`).            |
| `adec` | 0–127            | Amp envelope (Env 1) decay rate.                                       |
| `pdec` | 0–127            | Pitch envelope (Env 3) decay rate.                                     |
| `pamt` | signed           | Pitch envelope depth (the same control as `pitc`).                     |
| `sub`  | 0–63             | Sub-oscillator level.                                                  |
| `wave` | 11 shapes        | Sub-oscillator shape (`squ1`, `tri1`, `pul1`, `squ2`, `tri2`, `pul2`, `click`, `glitch`, `blow`, `metal`, `pop`). |

## Sequencer mode — substep editor

The `subs` cell on the Step page sets a coarse repeat or ratchet
count for a step. The **substep editor** lets you go further: gate
each individual repeat or ratchet hit on or off, and optionally
mutate the pitch of each fire through a chord shape.

### Entering and exiting

1. Walk the encoder until the cursor is on the `subs` cell.
2. Hold the step button you want to edit.
3. Click the encoder.

(Navigating to `subs` first matters — otherwise holding a step
while the cursor is somewhere else can trip a shortcut on a
neighbouring cell.)

The editor opens only if the held step has a non-zero `subs` value
(either side of centre). With the cursor anywhere else, or with
`subs` at `0`, the click is ignored.

Click the encoder again to exit.

### Two modes — repeats vs. ratchets

Which mode the editor opens in is determined by which side of the
`subs` deadzone the step is currently on:

- **Repeats** (CCW side, `Nr`). Gates each fire on successive
  pattern periods. Slot 1 is the original step; slots 2..N are
  the subsequent period re-fires.
- **Ratchets** (CW side, `Nx`). Gates each sub-trigger packed
  inside the step's own period. Slots 1..N are the ratchet hits
  in order, on the timing grid.

You can flip modes from inside the editor by sweeping pot 1 across
the centre deadzone (see *Editor controls* below).

### LCD layout

```
subs  4r | mint maj | mdir  up | moct  2
# # - # # # - -
```

Top row: the four editor controls. Bottom row: one slot per active
fire position, `#` if that fire is enabled and `-` if muted. The
`S1`–`S8` LEDs mirror the bottom row; buttons above the active
count are dark and inert.

### Editor controls

While the editor is active, only the first four pots are live. The
remaining pots are inert.

| Pot | Cell label | Function                                                                                                  |
|-----|------------|-----------------------------------------------------------------------------------------------------------|
| 1   | `subs`     | Count + mode. Mirrors the Step-page `subs` knob; sweeping across the centre deadzone toggles between repeats and ratchets. |
| 2   | `mint`     | **MINT** — mutation chord shape (see table below).                                                        |
| 3   | `mdir`     | **MDIR** — mutation walk shape (see table below).                                                         |
| 4   | `moct`     | **MOCT** — octave cap on the mutation walk, 1–4.                                                          |
| 5–8 | —          | Inert.                                                                                                    |

`S1`–`S8` toggle the gate on each active fire slot. Buttons above
the active count are inert.

When the editor opens, gates that fall outside the new active count
are cleared. If no gates remain, every active slot is re-enabled so
the step still fires.

### Mutation (MINT + MDIR + MOCT)

When MINT is set to anything but `off`, every fire after the first
walks the step's pitch through the tones of the chosen chord. Each
time the walk cycles through the chord, it climbs by an octave;
MOCT caps the maximum distance from the base note. MDIR sets the
shape of the walk. The final pitch is clamped to MIDI range and
re-quantised to the track's scale.

#### MINT — chord shapes

| Label  | Intervals            | Notes              |
|--------|----------------------|--------------------|
| `off`  | —                    | mutation disabled  |
| `oct`  | {0}                  | pure octave climb  |
| `pwr`  | {0, 7}               | root + fifth       |
| `maj`  | {0, 4, 7}            | major triad        |
| `min`  | {0, 3, 7}            | minor triad        |
| `sus2` | {0, 2, 7}            | suspended 2nd      |
| `sus4` | {0, 5, 7}            | suspended 4th      |
| `dim`  | {0, 3, 6}            | diminished         |
| `7`    | {0, 4, 7, 10}        | dominant 7         |
| `m7`   | {0, 3, 7, 10}        | minor 7            |
| `M7`   | {0, 4, 7, 11}        | major 7            |
| `7sus` | {0, 5, 7, 10}        | 7sus4              |
| `pent` | {0, 3, 5, 7, 10}     | minor pentatonic   |

#### MDIR — walk shapes

| Label  | Shape    | Range                                                |
|--------|----------|------------------------------------------------------|
| `up`   | sawtooth | base → +MOCT octaves, wraps back to base             |
| `dn`   | sawtooth | base → −MOCT octaves, wraps back to base             |
| `ud`   | triangle | bipolar ±MOCT around base                            |
| `ud+`  | triangle | base ↔ +MOCT (bounces off base)                      |
| `ud-`  | triangle | base ↔ −MOCT (bounces off base)                      |
| `rnd`  | random   | random chord-tone position within ±MOCT              |
| `rnd+` | random   | random chord-tone position within 0..+MOCT           |
| `rnd-` | random   | random chord-tone position within 0..−MOCT           |

Example: `mint = maj`, `moct = 1`, `mdir = up` walks base → +M3 →
+P5 → +octave, then wraps back to base. `mint = oct`, `moct = 4`,
`mdir = up` walks base, +1 oct, +2, +3, +4, then wraps. `rnd*`
shapes pick chord-tone positions at random instead of stepping
through them, but stay bounded by MOCT.

## Save / Load (snapshots)

The system page (`S8`) holds save / load. State is stored on the
SD card as numbered slots, 0–63.

The page has three modes: a top-level menu, a save-slot picker,
and a load-slot picker.

### Menu

```
system          Cur: 04
save     load     info exit
```

`Cur` shows the slot the running state was last loaded from or
saved to (`--` if nothing has been loaded or saved since the unit
was powered on).

| Button | Action                                                          |
|--------|-----------------------------------------------------------------|
| `S1`   | Enter save-slot picker.                                         |
| `S4`   | Enter load-slot picker.                                         |
| `S7`   | Open the OS Info / firmware page.                               |
| `S8`   | Exit back to the previous page.                                 |

### Save-slot picker

```
save  Cur: 04 | New: 12*
                              ok   back
```

The encoder selects the target slot. `New` is the slot you're
hovering on. A trailing `*` marks slots that already contain a
snapshot.

| Button | Action                                                          |
|--------|-----------------------------------------------------------------|
| `S7`   | Confirm save. If the slot is occupied, an overwrite-confirm dialog appears before the save runs. |
| `S8`   | Back to the menu without saving.                                |

After a successful save the menu reappears with `Cur` updated to
the newly saved slot and a brief `saved` confirmation.

### Load-slot picker

```
load  Cur: 04 | New: 09*
                              ok   back
```

Same layout as save — `*` marks occupied slots, encoder picks
target.

| Button | Action                                                          |
|--------|-----------------------------------------------------------------|
| `S7`   | Confirm load. Loading an empty slot shows an `empty slot` info dialog instead of loading. |
| `S8`   | Back to the menu without loading.                               |

After a successful load the menu reappears with `Cur` updated and
a brief `loaded` confirmation.

### What's saved

A snapshot captures the full pattern-and-voice state for all six
voices, plus the master clock settings (BPM, swing depth, master
reset period). It does **not** include:

- Performance-mixer state (mutes, solos) — these are deliberately
  transient.
- Transport state (play / pause). After a load, transport is
  stopped; press play to start.
- The `Cur` slot number itself — `Cur` is RAM-only, undefined
  again at next power-on.

### Live-use caveat

Save and Load each stop the transport before touching the SD card.
Use them between performances rather than during one — there is no
silent "save while playing" path in this version.

## Firmware update

Elkhart ships as seven binaries — one controller and one per voice
slot. Copy them all to the SD card root.

| Filename     | Contents                              |
|--------------|---------------------------------------|
| `AMBIKA.BIN` | Controller firmware                   |
| `VOICE1.BIN` | Voice slot 1 firmware                 |
| `VOICE2.BIN` | Voice slot 2 firmware                 |
| `VOICE3.BIN` | Voice slot 3 firmware                 |
| `VOICE4.BIN` | Voice slot 4 firmware                 |
| `VOICE5.BIN` | Voice slot 5 firmware                 |
| `VOICE6.BIN` | Voice slot 6 firmware                 |

**Controller and all six voicecards must run matching versions.**
The OS Info page reports the running version on each side; when
they agree the unit behaves as designed, and when they don't you
may see triggering or timing oddities until the mismatch is
resolved.

### Prepare the SD card

Format the card as FAT16 or FAT32 with 8.3 filenames, then copy the
seven release binaries to the card root.

### Flash the controller

There are two paths.

**Runtime upload (preferred):**

1. With the unit powered on and the SD card inserted, open the OS
   Info page: press `S8` to open the system page, then `S7` to
   enter OS Info.
2. Press `S1` to upload the controller from `/AMBIKA.BIN`.

**Bootloader recovery (use if the running controller can't reach
OS Info):**

1. Power the unit off.
2. Insert the SD card.
3. Hold `S8` while powering on. The bootloader picks up
   `AMBIKA.BIN` and flashes the controller.
4. Once boot completes, open OS Info to confirm the new running
   version.

### Flash the voicecards

1. With the unit powered on and the SD card inserted, open the OS
   Info page (`S8` → `S7`).
2. The page lists the six voicecard slots and their currently
   reported versions. Turn the encoder to highlight the slot you
   want to flash.
3. Press `S4` to upload `/VOICE#.BIN` to the highlighted slot.
4. Repeat for each slot you want to update.

When all six voicecards report the same version as the controller,
you're done.

### Reading the running version

The OS Info page shows the controller version and the running
version of each of the six voicecards in a single view. Use this
to confirm a flash succeeded, and to compare against what's on
your SD card before flashing.

## MIDI

Standard 5-pin DIN MIDI in and out are on the back panel. They
connect Elkhart to the rest of your rig.

### What MIDI does in this version

| Direction | Behaviour                                                                                   |
|-----------|---------------------------------------------------------------------------------------------|
| In        | Receives MIDI clock and transport (start / continue / stop) for external sync.              |
| In        | On channel 10, notes 36–41 trigger voices 1–6 (a fixed General-MIDI-style drum map).        |
| Out       | Sends MIDI clock, transport messages, and note events while the sequencer is running.       |

### What it doesn't do

This version of Elkhart ships **without a detailed MIDI configurator**.
There is no per-track channel selector, no CC routing UI, and no
MIDI learn. The behaviour described above is the whole MIDI surface
you can rely on at v4.0.

A more configurable MIDI implementation — per-voice channels, CC
mappings, and learn — is on the roadmap for a later release. Until
then, MIDI is best treated as a clock and transport bridge plus a
basic drum-map note input.

### Connecting

- **MIDI In** receives clock from a master sequencer or DAW. Send
  start / stop / continue from the master to drive Elkhart's
  transport.
- **MIDI Out** sends clock and notes from Elkhart, so external
  drum machines or synths can follow along, and so individual step
  notes can play hardware connected downstream.

## License and credits

Elkhart is released under the **GNU General Public License v3.0**,
inherited from upstream Mutable Instruments and YAM.

- The original Ambika firmware is by Émilie Gillet (Mutable
  Instruments).
- Elkhart's voice DSP is adapted from the YAM fork
  (`bjoeri/ambika`).
- The vowel-synthesis oscillator is a variant of Peter Knight's
  Cantarino formant algorithm.

For the full license text, see `LICENSE` in the source repository.
