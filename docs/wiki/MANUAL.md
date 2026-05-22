# Elkhart — User Manual

*For Elkhart firmware v4.2, running on Mutable Instruments Ambika
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
| `S2` + turn                  | Step between settings pages (multi-page step)       |
| `S4` + turn                  | Jump to Sequencer mode                              |
| `S6` + turn                  | Jump to Per-track settings                          |
| `S7` + turn                  | Jump to Performance mixer                           |
| `S8` + turn                  | Jump to System page                                 |
| `S5` + turn CW               | Play / pause                                        |
| `S5` + turn CCW              | Stop (notes ring out per envelope release)          |
| `S5` + turn CCW × 2 (400 ms) | Panic — hard mute on every voice                    |

### Pages

v4.2 retires the button-equals-page model. `S1`–`S8` always
function as step buttons (see *Step buttons* below). Page navigation
is via the `Sn` + encoder combos listed above; `S2` + turn steps
through the patch pages (Oscillators, Filter, Envelopes, LFOs).

The active page groups are:

| Group                    | Reached by                                    |
|--------------------------|-----------------------------------------------|
| Oscillators / Mixer      | `S2` + turn until reached                     |
| Filter                   | `S2` + turn until reached                     |
| Envelopes + LFO          | `S2` + turn until reached                     |
| Sequencer mode           | `S4` + turn                                   |
| Per-track settings       | `S6` + turn                                   |
| Performance mixer        | `S7` + turn                                   |
| System (snapshots, info) | `S8` + turn                                   |

### Step buttons

`S1`–`S8` are step triggers for the active voice on **every page**
except System and Performance mixer.

| Gesture                                       | Effect                                                                       |
|-----------------------------------------------|------------------------------------------------------------------------------|
| Tap `Sn`                                      | Toggle step `n` on / off                                                     |
| Hold `Sn`                                     | "Peek" — show that step's locked values without toggling                     |
| Hold `Sn` + turn pot                          | Write a per-step lock for the pot's parameter (any lockable cell)            |
| Double-tap `Sn` within 300 ms                 | Clear every lock for step `n`                                                |

While holding a step button, the lower-left LCD character shows a
fullness gauge for the per-step lock pool (192 entries max). The
gauge updates at the moment the step is pressed; release and
re-press to see the count after adding more locks.

On the **System page** and **Performance mixer**, the buttons take
on page-specific roles (save/load/info/exit; voice mute/solo) — see
the respective sections.

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

## Page map

Every page reachable from the front panel, with the combo that gets
you there and the LCD cells visible on it. Detail on each cell lives
in the per-page sections below.

| Page                         | Reached by                                     | Cells (top row / bottom row)                                                  |
|------------------------------|------------------------------------------------|-------------------------------------------------------------------------------|
| Oscillators                  | `S2` + turn (Osc/Mixer group, page 1)          | `wave para rang tune` / `wave para rang tune`                                 |
| Mixer                        | `S2` + turn (Osc/Mixer group, page 2)          | `mix nois sub wave` / `xmod amnt fuzz crsh`                                   |
| Filter                       | `S2` + turn (Filter group)                     | `freq reso — mode` / `env2 — — —`                                             |
| Amp + Filter envelopes       | `S2` + turn (Envelopes group, page 1) / `S3`   | `rise fall curv amp` / `rise fall curv flt`                                   |
| Pitch envelope + voice LFO   | `S2` + turn (Envelopes group, page 2) / `S4`*  | `rise fall curv pitc` / `rate wave dest dept`                                 |
| Sequencer Step page          | `S4` + turn (Sequencer mode, lock page 1)      | `note vel vamt rate` / `subs prob glid sfx`                                   |
| Sequencer Voice 1 page       | encoder past Step page (lock page 2; INT only) | `nois w1 pa1 tun2` / `mix w2 pa2 fin2`                                        |
| Sequencer Voice 2 page       | encoder past Voice 1 page (lock page 3)        | `freq fdec famt adec` / `pdec pamt sub wave`                                  |
| MIDI CC page (EXT tracks)    | encoder past Step page when track is EXT       | `CC# CC# CC# CC#` / `val val val val` (slides through 8 slots)                |
| Per-track settings           | `S6` + turn                                    | `dirn rate rota leng` / `scal root mch mmod`                                  |
| Performance mixer            | `S7` + turn                                    | `v1 v2 v3 mode` / `v4 v5 v6 clr`                                              |
| System                       | `S8` + turn                                    | `Cur: Next: BPM CLK` / `save load info exit`                                  |

\* `S4` is shared with the Sequencer mode entry combo. Tapping `S4`
toggles sequencer mode; `S4` + turn (held) walks through Envelope
pages.

Lock-vs-default behaviour:

- **Patch pages** (Oscillators / Mixer / Filter / Envelopes): turning a
  pot writes the **track default**. Holding any step button while
  turning writes a **per-step lock** for the parameter under that pot,
  if the cell is lockable.
- **Sequencer Step / Voice 1 / Voice 2 pages**: every cell is per-step
  lockable. Without a step held, the pot writes the track default;
  with a step held, it writes a per-step lock.
- **Per-track settings**: track-wide. No per-step locks; values fire
  for every step on that voice.
- **Performance mixer + System**: page-local state; no track defaults
  or step locks involved.

### LCD mockups

```
Oscillators                       Mixer
wave saw  | para  64 | rang  +0 | tune   +0     mix   32 | nois  16 | sub   48 | wave  squ1
wave fm   | para  90 | rang +12 | tune  -05     xmod env | amnt  20 | fuzz  18 | crsh   4

Filter                            Amp + Filter envelopes
freq  64 | reso  18 | --       | mode  LP       rise   2 | fall  64 | curv  90 | amp  127
env2  20 | --       | --       | --             rise   8 | fall  72 | curv  40 | flt   48

Pitch envelope + voice LFO        Per-track settings
rise   0 | fall  20 | curv 100 | pitc  +24      dirn fwd  | rate 16   | rota   0 | leng   8
rate  48 | wave tri | dest pit | dept  +32      scal min  | root   0  | mch    1 | mmod INT

Sequencer Step page               Sequencer Voice 1 page (INT tracks)
note C 3 | vel  100 | vamt  64 | rate trk       nois  16 | w1   saw | pa1  64 | tun2 +07
subs   0 | prob 127 | glid   0 | sfx none       mix   32 | w2   fm  | pa2  90 | fin2 -05

Sequencer Voice 2 page            Performance mixer
freq  64 | fdec  72 | famt +20 | adec  64       v1 192 | v2 220 | v3 180 | mode MT-A
pdec  20 | pamt +24 | sub   48 | wave squ1      v4 255 | v5 200 | v6 128 | clr  unmt

MIDI CC page (EXT tracks)         System
CC#  20 | CC#  74 | CC#  off| CC#  16          Cur: 04 | Next: 12*| BPM 120 | CLK OUT
val 127 | val  45 | val   0 | val  90          save     load     info     exit
```

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
| `pitc` | −63..+63         | Env 3 → Osc 1 pitch depth (bipolar, default 0). Same byte as `pamt` on the voice page / step locks — edits in either place agree. Range is roughly ±5 octaves at full deflection, plenty for kick-drum pitch drops or upward chirps. |
| `rate` | 0–127            | LFO speed. Free-running — there is no tempo sync in this version.       |
| `wave` | sine / tri / square / ramp / S&H | LFO waveform.                                          |
| `dest` | destination list | What the LFO modulates (pitch, cutoff, FM depth, etc.).                 |
| `dept` | −63..+63         | LFO depth, signed — negative values invert the modulation.              |

## Per-track settings (`S6`)

The `S6` group's first page configures the active voice's
pattern-level behaviour: direction, step length, rotation, scale,
root, MIDI channel, and INT/EXT mode. Use the `S1` + encoder combo
to pick which voice you're editing.

```
dirn fwd  | rate 16   | rota   0 | leng   8
scal min  | root   0  | mch    1 | mmod INT
```

| Cell   | Range / values                                                 | Notes                                                                                                            |
|--------|----------------------------------------------------------------|------------------------------------------------------------------------------------------------------------------|
| `dirn` | `fwd` / `rev` / `pend` / `rnd`                                 | Playback direction. `pend` ping-pongs end-to-end; `rnd` jumps to a random step each tick.                        |
| `rate` | 15 musical step lengths (see table below)                      | The track's step length. Different rates on different tracks is what produces polymetric drift.                  |
| `rota` | 0–7                                                            | Rotates the pattern's start point without altering step data — useful for shifting which step lands on the downbeat. |
| `leng` | 1–8                                                            | Pattern length in steps. Combined with `rate`, drives polymetric cycle length.                                   |
| `scal` | `chro` / `maj` / `min` / `dor` / `mix` / `pMa` / `pMi` / `blu` | Quantises every step's note into the chosen scale.                                                               |
| `root` | 0–11                                                           | Scale root, in semitones from C.                                                                                 |
| `mch`  | 1–16                                                           | MIDI channel for the track's sequencer-out notes and (on EXT) MIDI CC. Defaults to the track index.              |
| `mmod` | `INT` / `EXT`                                                  | INT (default) drives the internal voicecard. EXT silences the voicecard and routes the track to MIDI out — sequencer notes + configurable CCs only. See *MIDI sequencing* below. |

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

### Raw-tick `rate` (tracker swing)

The 15 musical-rate presets are evenly distributed but skip a few
periods that make tracker-style swing possible — 5, 7, 10, 11 ticks
at 24 PPQN. To reach them, the `rate` cell has a raw-tick mode.

**Click the encoder on the `rate` cell** (either the track `rate` on
`S6`, or the per-step `rate` on the Step lock page) to toggle between
preset and raw-tick mode. The display flips from `" 16 "` to `" t6 "`:
the leading `t` marks raw mode, and the number is the period in MIDI
ticks (24 PPQN, so `t6` = `1/16`, `t12` = `1/8`, etc).

In raw mode the pot edits the period directly, `2..96` ticks
(`96` = one bar). It uses snap-on-cross — sweep the pot through the
stored value before edits commit, so cursoring onto the cell doesn't
jump the rate. Click again to flip back to the preset table; the byte
snaps to the nearest entry.

The per-step `rate` inherit sentinel (`trk`) cannot be toggled until a
concrete rate is set.

**Recipe — tracker swing on 16ths.** Set the track `rate` to `16`
(period 6). On the Step lock page, hold step 0 and click on its `rate`
cell to flip to raw mode, then turn the pot to `t7`. Repeat on step 1
with `t5`. Alternating `t7 / t5` averages back to 6 ticks but with a
~58% swing feel. Any pair that sums to twice the underlying preset
period stays in sync with the master clock; further-apart values
(`t8 / t4` ≈ triplet feel) work too.

### Polymeter, briefly

Two tracks at the same `rate` and `leng` lock to the same downbeat
forever. Set them to different `rate` values, or different `leng`
values, and they cycle at different absolute lengths — drifting in
and out of phase. Eight short patterns at six different rates is
the core gesture of this instrument.

## Performance mixer (`S7`)

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
subs  1x | prob 100 | glid   0 | sfx none
```

| Cell   | Range / values                                                                  | Notes                                                                                                                                                                                  |
|--------|---------------------------------------------------------------------------------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `note` | 0–127, shown as note name                                                       | Step note. Quantised to the track's scale and root.                                                                                                                                    |
| `vel`  | 0–127                                                                           | Step velocity. Supports per-lock probability — see below.                                                                                                                              |
| `vamt` | 0–127                                                                           | Velocity → VCA depth on INT tracks. **Lockable per step** so it can also drive MIDI Mod Wheel (CC 1) on EXT tracks. On INT, only the live-tweak value reaches the voicecard today — step locks are stored but don't override the voicecard amount at fire time.  |
| `rate` | `trk`, preset (`32`…`2B`), or raw-tick (`t2`…`t96`)                             | Per-step rate override. `trk` inherits the track rate. Encoder click toggles between preset and raw-tick modes — see *Raw-tick `rate` (tracker swing)* above.                          |
| `subs` | `1x..8x` (ratchets), centre `1x` (no substeps), `1r..8r` (repeats)              | Bipolar substep cell. Displayed as total fires. `1x` = main step only. CW values pack multiple sub-triggers into the step's own period (e.g. `4x` = main + 3 ratchets = 4 fires). CCW values re-fire the step on successive pattern periods (`4r` = main + 3 re-fires = 4 fires). Per-step only — shows `----` when no step is held. The substep editor extends both modes — see the next section. |
| `prob` | bipolar: `0..99%` (CCW), `100` (centre), `1:2..8:8`, `!1:3..!6:6`, `FILL`, `!FIL` (CW) | Probability that the step fires. **Centre (`100`)** = always fires. **CCW** = random roll, dialled in as a percentage. **CW** = iterative cycle-phase. `X:N` fires on loop X of every N pattern wraps (e.g. `2:4` = fires every 4th loop, on loop 2 of 4); `!X:N` fires every loop except X of N. `FILL` / `!FIL` are reserved for a forthcoming fill button. Each track maintains its own loop counter. PROB also gates whether `sfx` takes effect. |
| `glid` | 0–127                                                                           | Per-step glide / portamento time. `0` = no glide. On EXT tracks, the locked value also emits as CC 5 (Portamento Time) on the track's MIDI channel. Supports per-lock probability — see below. |
| `sfx`  | `none`, `skip`, `fwd`, `rev`, `dir`, `rjmp`, `jmp1`–`jmp8`, `eskp`              | Per-step modifier. No track default — shows `----` when no step is held. Gated by `prob`. `skip` advances without firing; `fwd`/`rev` set track direction sticky; `dir` toggles direction; `rjmp` jumps to a random step; `jmp1`–`jmp8` jump to that absolute step; `eskp` (explicit-skip) is skipped during normal play and only fires when another step's jump SMOD lands on it. Supports per-lock probability — see below. |

### Voice 1 page

Oscillator-side per-step locks. **On EXT tracks**, this page is
replaced with a MIDI CC sequencer view — see *MIDI sequencing*
below.

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

Filter, envelope, and sub layers per-step. **On EXT tracks**, this
page is replaced with a MIDI CC sequencer view — see *MIDI
sequencing* below.

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
| `pamt` | −63..+63         | Pitch envelope depth (bipolar, default 0). Same byte as `pitc` on the envelope page. ~±5 octaves at full deflection. |
| `sub`  | 0–63             | Sub-oscillator level.                                                  |
| `wave` | 11 shapes        | Sub-oscillator shape (`squ1`, `tri1`, `pul1`, `squ2`, `tri2`, `pul2`, `click`, `glitch`, `blow`, `metal`, `pop`). |

### Per-lock probability

Any per-step lock can carry its own probability gate. When the gate passes the lock applies; when it fails, the step plays through with that parameter's track default for the loop. Each gate uses the same bipolar PROB encoding as the step's main `prob` cell — random % roll on the CCW side, "always" at centre, or iterative `X:N` / `!X:N` patterns on the CW side. Gates roll independently of step PROB, so a step can fire on every other loop *and* have its FREQ override apply on every third loop.

#### How to set a per-lock PROB

1. Turn the encoder so the cursor sits on the cell whose lock you want to gate.
2. Hold the step button.
3. Click the encoder. The cell flips from its normal value display to a PROB readout.
4. Turn that cell's pot to dial the PROB (`50%`, `1:3`, `!2:4`, etc.).
5. Release the step (or click again) to exit PROB-edit mode.

The cell's label renders **UPPERCASE** any time that step has a non-default PROB attached, as a "this lock is gated" indicator.

**Order matters.** Cursor first, then hold the step. Holding the step first and turning the encoder doesn't move the cursor — step buttons act as encoder modifiers for shortcut handling, so the turn is consumed elsewhere.

#### What can be gated

| Where | What gets gated on roll fail |
|-------|------------------------------|
| Step page cells: `vel`, `glid`, `sfx`        | Intrinsic field reverts to the track default. `sfx` becomes `none` (step fires normally, no jump / no skip / no direction change). |
| Voice 1 / Voice 2 page cells (synth params)  | Parameter override doesn't apply; the synth uses the track default. |
| EXT-track CC slots (S5b / S5c)               | Locked CC value doesn't send; the track default CC value sends instead. |
| Substep editor — see next section            | Ratchets / repeats / chord walk all suppressed for that loop; main step still fires (subject to step PROB). |

Cells that **don't** support per-lock PROB: `note` (intrinsic with no overlay path), `rate` (encoder click is reserved for raw-tick toggle), `prob` (it's already the gate), `subs` (gated separately inside the substep editor). The four cells excluded above don't get drill-in mode — click is a no-op for them.

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

The editor opens for any held step. Steps without ratchets or repeats (`subs = 1x`) open the editor in a single-slot view so you can still reach the chord-walk controls and the SUBS PROB cell.

Click the encoder again, or release the step, to exit. The encoder is modal inside the editor — turning it doesn't navigate away.

### Two modes — repeats vs. ratchets

Which mode the editor is in follows the step's current `subs` value:

- **Ratchets** (CW side, `Nx`). Sub-triggers packed inside the step's own period. `4x` = main hit + 3 ratchets = 4 fires total, evenly spaced.
- **Repeats** (CCW side, `Nr`). Re-fires on successive pattern periods. `4r` = main hit + 3 re-fires across the next 3 periods = 4 fires total.
- **Centre (`1x`)**. No substep behaviour — single fire per visit. Chord-walk (`mint`) still works in this mode; it advances once per pattern loop instead of per sub-trigger.

You can flip modes from inside the editor by sweeping pot 1 across its centre deadzone.

### LCD layout

```
 subs  4x | mint  maj | mdir   up | moct   2
 prob  50 |           |           |
```

`S1`–`S8` LEDs show the substep slot state:

- **Green** — slot is active and firing.
- **Red** — slot is active but currently disabled.
- **Dark** — slot is inactive (above the active count).

Press a button to toggle its slot between green and red.

### Editor controls

| Pot | Cell label | Function                                                                                                  |
|-----|------------|-----------------------------------------------------------------------------------------------------------|
| 1   | `subs`     | Count + mode. Same encoding as the Step-page `subs` knob; sweeping across centre toggles between repeats, off, and ratchets. |
| 2   | `mint`     | **MINT** — chord-walk shape (see table below).                                                            |
| 3   | `mdir`     | **MDIR** — chord-walk direction (see table below).                                                        |
| 4   | `moct`     | **MOCT** — octave cap on the chord walk, 1–4.                                                             |
| 5   | `prob`     | **SUBS PROB** — bipolar PROB (`50%`, `1:3`, `!2:4`, etc.). On roll fail, ratchets / repeats / chord walk are suppressed for that loop; the main step still fires. |
| 6–8 | —          | Inert.                                                                                                    |

When the editor opens, slot gates above the active count are cleared. If no gates remain, every active slot is re-enabled so the step still fires.

### Mutation (MINT + MDIR + MOCT)

When MINT is set to anything but `off`, the step's pitch walks through the tones of the chosen chord. The walk timing depends on the step's `subs` mode:

- **Ratchets / repeats (`Nx` / `Nr`)**: each sub-fire after the main hit advances one step through the walk.
- **No substeps (`1x`)**: the walk advances **once per pattern loop** — each time this track wraps around, the step plays the next chord tone.

Each time the walk cycles through the chord, it climbs by an octave; MOCT caps the maximum distance from the base note. MDIR sets the shape of the walk. The final pitch is clamped to MIDI range and re-quantised to the track's scale.

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
| `chr`  | {0..11}              | chromatic (all 12 semitones) |

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

## System page (`S8`)

The System page consolidates snapshots, master clock, and OS info.

```
Cur: 04 | Next: 12*| BPM 120 | CLK OUT
save     load     info     exit
```

**Top row** (read-only / pot-edited):

| Cell    | Source  | Notes                                                        |
|---------|---------|--------------------------------------------------------------|
| `Cur:`  | RAM     | Slot the running state was last loaded from / saved to.      |
| `Next:` | Encoder | Target slot for save / load. `*` marks occupied slots.       |
| `BPM`   | Pot 3   | Master tempo (40–240).                                       |
| `CLK`   | Pot 4   | Clock mode: INT / EXT / OUT / THR.                           |

**Bottom row** (hold-to-confirm buttons):

| Button | Action                                                                                  |
|--------|-----------------------------------------------------------------------------------------|
| `S1`   | **Save** to `Next` slot. Empty slot fires on tap; occupied slot requires hold.          |
| `S3`   | **Load** from `Next` slot. Always hold-to-confirm.                                      |
| `S5`   | **Info** — opens the OS Info / firmware page.                                           |
| `S7`   | **Exit** — return to the previous page.                                                 |

### Clock modes

| Mode  | Clock source                  | Clock out                            | When to use                                                          |
|-------|-------------------------------|--------------------------------------|----------------------------------------------------------------------|
| `INT` | Internal (from `BPM`)         | Suppressed                           | Standalone — no MIDI clock leaves the box.                           |
| `EXT` | External (inbound MIDI clock) | Suppressed                           | Slave to a master clock; don't echo it back out.                     |
| `OUT` | Internal (from `BPM`)         | Sends `0xF8` stream at internal BPM  | Master to other gear.                                                |
| `THR` | External (inbound MIDI clock) | Forwards inbound clock to MIDI out   | Pass-through — slaved to a master while re-clocking downstream gear. |

### Hold-to-confirm

Save (on occupied slots) and Load both use a hold-to-confirm flow:

- **0 → 300 ms held**: nothing visible.
- **300 → 900 ms held**: button LED fast-blinks ("armed"). Release at
  this point cancels.
- **≥ 900 ms held**: action fires. LED gives 2-blink feedback —
  **green** on success, **red** on failure (load fail / save error).

Save on an empty slot fires on tap (no hold needed). Load on an
empty slot reports as a failure (red feedback) — there's nothing to
load.

### What's saved

A snapshot captures the full pattern-and-voice state for all six
voices, plus the master clock settings (BPM, CLK mode, master reset
period) and the per-step lock pool. It does **not** include:

- Performance-mixer state (mutes, solos) — these are deliberately
  transient.
- Transport state (play / pause). After a load, transport is
  stopped; press play to start.
- The `Cur` slot number itself — `Cur` is RAM-only, undefined
  again at next power-on.

v4.2 snapshots use file format `0x03`. Older snapshots (`0x01`,
`0x02`) are migrated on load via a one-shot translation that
converts the v4.1 dense lock format into the new lock pool and
remaps the retired `kPatBPCH` slot.

### Live-use caveat

Save and Load both interrupt sounding voices. After Load, transport is
stopped and voices are silent — press play to hear the loaded patch.

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

Standard 5-pin DIN MIDI in and out are on the back panel.

### MIDI in

| Source                | Effect                                                                              |
|-----------------------|-------------------------------------------------------------------------------------|
| Clock (`0xF8`)        | Advances the sequencer when `clk` (System page) is `EXT` or `THR`.                  |
| Start/Stop/Continue   | Drives transport when slaved to external clock.                                     |
| Notes on ch 10        | Notes 36–41 trigger voices 1–6 (fixed General-MIDI-style drum map).                 |

### MIDI out

Sequencer note events leave the box on the track's configured MIDI
channel (set via `mch` on the Per-track settings page). Clock and transport bytes follow
the `clk` mode on the System page. CC and Mod Wheel emissions
are described under *MIDI sequencing* below.

### MIDI sequencing (EXT mode)

Each track has an INT/EXT mode toggle (the `mmod` cell on the Per-track settings page).

- **INT** (default): the track drives its internal voicecard as
  usual. Sequencer notes are also echoed to MIDI out on the track's
  channel — useful for layering external gear with the internal
  voice.
- **EXT**: the voicecard is bypassed entirely. The track becomes a
  MIDI-only output: sequencer notes, plus a configurable set of
  CC values driven by step locks. Use this when you want a track to
  drive an external synth or drum machine instead of the local
  voice.

The track's MIDI channel (`mch`) and EXT mode (`mmod`) both live
on the Per-track settings page and are saved with the snapshot.

#### What an EXT track sends

| Source                         | MIDI message                          |
|--------------------------------|---------------------------------------|
| Step note + velocity           | Note On / Note Off on `mch`           |
| `vamt` cell (Step page)        | CC 1 (Mod Wheel) on `mch`             |
| `glid` cell (Step page)        | CC 5 (Portamento Time) on `mch`       |
| 8 configurable slots           | User-assigned CC on `mch`             |

`vamt` and `glid` are fixed-purpose on EXT — they map directly to
the matching standard MIDI controllers so external synths reach
them with no setup. The other eight slots are configurable: each
slot picks its own CC number, so you can target whichever filter,
envelope, or modulation control the receiving gear expects.

#### MIDI CC page (EXT tracks)

When the active track is EXT, the Voice 1 and Voice 2 lock pages flip
from their normal synth-param layout to a MIDI CC sequencer view. Four
slots per page, eight per track total. Each slot occupies one column
of the LCD, with two stacked rows:

```
CC#  20 | CC#  74 | CC#  off| CC#  16    ← top row: configurable CC#
val 127 | val  45 | val   0 | val  90    ← bottom row: lockable value
```

- **Top row (pots 0–3): the CC number.** Twist the pot to assign a
  CC (1–127). All the way CCW reads `off` and disables emission
  for that slot. Default is `off` on every slot — a fresh EXT
  track is silent on CC until you opt each slot in.
- **Bottom row (pots 4–7): the value sent.** This is a normal
  lockable cell: hold a step button and turn the pot to author a
  per-step lock, or turn without a step held to set the track
  default. Unlocked steps "snap back" to the default on every step.

Step locks on these cells fire the assigned CC at step time, just
like an INT track's synth params get locked. Channel comes from
`mch`. Live pot moves emit the CC immediately so you can perform
with the knob in real time.

#### Switching INT ↔ EXT

Flipping `mmod` silences the voicecard immediately on INT → EXT;
EXT → INT resumes firing on the next step.

### Connecting

- **MIDI In** receives clock from a master sequencer or DAW. Set
  `clk` to `EXT` (for receive-only) or `THR` (to also forward
  clock downstream).
- **MIDI Out** sends note + CC events for every active track plus
  clock when `clk` is `OUT` or `THR`. Hook external synths or
  drum machines to their respective tracks' channels and let
  Elkhart sequence them.

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
