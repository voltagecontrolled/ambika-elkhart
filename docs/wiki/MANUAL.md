# Elkhart — User Manual

*Draft 0.2 — based on SPEC_v2 (YAM-based), pre-hardware*

---

## Firmware Installation

<!-- TODO: Document accurate controller and voicecard flash procedure once verified on hardware. Key points to cover: SD card prep, S8+Power for controller, voicecard flash menu navigation, S4 to trigger per-card flash, version confirmation. Note: controller and voicecards must run matching firmware versions. -->

---

## Overview

Elkhart is a 6-voice percussive synthesizer with a polymetric step sequencer. Each voice is independent — its own oscillator stack, filter, and envelopes — and each track has its own pattern that runs at its own length and clock division. The primary expressive dimensions are the tension between locked and unlocked steps and the rhythmic relationships that emerge when six independent step lengths and clock divisions interact.

Each voice runs the same signal chain: **two oscillators** drawn from a wide algorithm palette (sines, vowels, FM, wavetables, more), with an interaction control that crossfades or FMs them; a **sub-oscillator and transient layer** providing both traditional sub-bass reinforcement and transient hits (clicks, pops, blows, glitches, metallic); a **multimode SVF filter** with drive and bit-reduction; an **LPG-coupled amp+filter envelope** controlled by three macro knobs (decay, depth, personality offset); and an **independent pitch envelope** for transients and pitch sweeps.

There are no dedicated drum voice types. The same voice that makes a plonky kick makes a metallic hi-hat via different oscillator algorithm, ratio, filter mode, envelope decay, and transient choice.

---

## Hardware

- **2×40 LCD**
- **Push encoder** (turn + click)
- **8 knobs:** Top row (4) + Bottom row (4)
- **8 buttons with LEDs:** Labeled 1–8
- **6 individual audio outputs** + **mix output** (plugging an individual out removes that voice from the mix)
- **SD card** for state snapshot storage

---

## Navigation

- **Encoder turn:** Scrolls through the full parameter list continuously — all pages flow into each other without stopping at boundaries.
- **Encoder push:** Enters focused-edit mode on the highlighted parameter. The full parameter name is shown on the second LCD line and the encoder turn directly adjusts the value. Push again to exit.
- **Knobs 1–8:** Always edit the 8 parameters currently displayed.
- **Buttons 1–8:** Context-dependent based on the current page:
  - **On most normal-mode pages:** quick-jump shortcut to that page (Button 1 → Oscillator, Button 2 → Filter, etc.).
  - **On the Master / Transport page (Button 7):** buttons 4–6 act as PLAY / PAUSE / RESET. Page-jump shortcuts are not available while on Master — use the encoder to scroll back to other pages.
  - **In Sequencer Mode:** buttons are step triggers. Page shortcuts are not available — use the encoder to navigate.
- **Button 5:** Toggles Sequencer Mode (symmetric — second press exits).

The currently selected **voice** is set by `VSEL` — accessible on Page 1 Top 1 and Page 7 Top 1 (placeholder pending final UI). All edits apply to the selected voice.

---

## Normal Mode

### Page 1 — Oscillator

The dual oscillator. Two oscillators with full algorithm-palette selection on each, plus a single bipolar `BLND` knob that controls whether they're mixed in parallel or one FMs the other.

| Knob | Name | Description |
|------|------|-------------|
| Top 1 | `NOTE` Base Note | Base pitch of Osc 1 (the carrier) in semitones. Drives the perceived pitch of the voice. Lockable per step — different notes per step build melodic patterns. |
| Top 2 | `WAVE` Osc 1 Wave | Selects the Osc 1 algorithm/wave shape from the palette. Sine, triangle, polyblep saw/pwm, vowel, fm, fm_fb, wavetables, and more. |
| Top 3 | `PARA` Osc 1 Parameter | Algorithm-specific timbral parameter for Osc 1. Behavior depends on the selected wave shape — PWM amount for square, formant select for vowel, modulation index for fm, etc. The LCD second line shows the algorithm-specific name when adjusting. |
| Top 4 | `BLND` Oscillator Interaction | Bipolar. **0–63:** Crossfade between Osc 1 and Osc 2 in the audio path (0 = Osc 1 only, 63 = Osc 2 only). No FM. **64–127:** Osc 2 modulates Osc 1's pitch (Osc 2 leaves the audio path). FM depth scales from 0 at 64 up to maximum at 127. The 63→64 transition is acoustically silent. |
| Bot 1 | `RTIO` Osc 2 Ratio | Frequency ratio of Osc 2 relative to Osc 1. DX-style ratio table — 0.125, 0.25, 0.5, 0.707, 1.0, √2, 7/4, 2, 9/4, π, 4, 5, 8, etc. The single most important timbral knob in FM mode. |
| Bot 2 | `WAVE` Osc 2 Wave | Selects the Osc 2 algorithm/wave shape — same palette as Osc 1. |
| Bot 3 | `PARA` Osc 2 Parameter | Algorithm-specific timbral parameter for Osc 2. |
| Bot 4 | `FINE` Osc 2 Fine Tune | ±100 cents detune on Osc 2. Slight non-integer detune produces FM beating and shimmer without altering perceived pitch. |

### Page 2 — Filter

The state-variable filter and bit-reduction stage. The filter envelope is on Page 3.

| Knob | Name | Description |
|------|------|-------------|
| Top 1 | `FREQ` Cutoff Freq | Base filter frequency. The filter envelope (Env2) modulates around this point. |
| Top 2 | `RES ` Resonance | SVF resonance. High resonance at moderate cutoff produces ringing tones. |
| Top 3 | `TYPE` Filter Mode | LP / BP / HP output select. LP for body and warmth, HP for hats and cymbals, BP for nasal/metallic character. |
| Top 4 | `DRIV` Filter Drive | Gain into the SVF. At high levels adds saturation and grit. Interacts with resonance — drive + high resonance gets aggressive. |
| Bot 1 | `BITS` Bit Reduction | Reduces the digital signal's bit depth before the SVF and DAC. 0 = full resolution. Max = 1-bit (extreme digital grit). The filter then shapes the added harmonics. |

Knob mappings on this page are placeholder until empirical UI tuning. Filter bypass: `FREQ` to max and `RES` to min.

### Page 3 — Modulation

Envelope amounts and curves, plus LFO settings. These are voice-level; the per-step decay times for each envelope live on Sequencer Voice Page 2.

| Knob | Name | Description |
|------|------|-------------|
| Top 1 | `VCAA` VCA Env Amount | Amplitude envelope (Env1) overall depth. Voice-level. (Placeholder slot — VCA env is generally always-on.) |
| Top 2 | — | TBD |
| Top 3 | — | TBD |
| Top 4 | — | TBD |
| Bot 1 | `LSHP` LFO Shape | Sine, triangle, square, ramp, or random/S&H. |
| Bot 2 | `LFOR` LFO Reset | Free-running, per-step (resets each trigger), or per-pattern (resets on pattern start). |
| Bot 3 | `LFOA` LFO Amount | Modulation depth. |
| Bot 4 | `LFOD` LFO Destination | LFO target: pitch, cutoff, fold (when applicable), or FM depth. |

The exact knob layout on Page 3 is settled empirically during Phase 7 (UI). The LFO is voice-level only in v1 — audio-rate operation is not supported. If empirical use reveals a need for per-step LFO control, `LFOD` / `LFOS` / `LFOA` are candidates for elevation to a future third lockable page.

### Page 4 — Track Relationships

Defines how the six tracks interact with each other. Up to 4 active relationships globally.

The LCD shows a list of active relationships:
```
[Src] Track 1 ──▶ [Dst] Track 3   Type: TransOsc1   Amt: +7
[Src] Track 2 ──▶ [Dst] Track 5   Type: Clock       Amt: ---
```

The encoder scrolls through relationship slots. Knobs set source, destination, type, and amount.

| Relationship | Description |
|--------------|-------------|
| **Transpose Osc 1** | Source track's current pitch transposes destination's Osc 1 (the carrier). |
| **Transpose Osc 2** | Same, but Osc 2 only — allows cross-track detuning and beating. |
| **Clock** | Source track's triggers advance the destination's playhead. Destination only steps when source fires. |
| **Reset** | Source track's step 1 resets destination to step 1. |
| **Accent** | When source and destination fire simultaneously, destination's velocity is boosted. |

### Page 5

Pressing Button 5 enters Sequencer Mode. Pressing Button 5 again returns to Normal Mode. See the Sequencer Mode section below.

### Page 6 — Track Settings

Pattern, scale, and timing settings for the selected track. The original Mutable Ambika "voice parts" hierarchy and the 6a/6b split are gone — track settings now live on a single page.

| Knob | Name | Description |
|------|------|-------------|
| Top 1 | `SCAL` Scale | Scale quantization. Used by the Mutate `Scale` interval and pitch resolution. |
| Top 2 | `ROOT` Root Note | Root note for scale-relative operations. |
| Top 3 | `BPCH` Base Pitch | Default pitch for unlocked NOTE; starting point for Mutate. |
| Top 4 | `OLEV` Output Level | Voice output level in the mix. |
| Bot 1 | `DIRN` Direction | Forward / Reverse / Pendulum / Random. |
| Bot 2 | `CDIV` Clock Div | Per-track clock division: 1, 2, 3, 4, 6, 8, 12, 16. **The polymeter engine.** Two tracks at different divisions drift in and out of phase as their patterns play. |
| Bot 3 | `ROTA` Rotate | Shifts the pattern start point (0–7) without altering step data. |
| Bot 4 | `LENG` Pat Length | Pattern length 1–8 steps. Combined with `CDIV`, creates polymeters. |

Voice-config items not on the Sequencer pages or Page 1/2/3 (filter mode, drive, bits, env curves, LFO settings, smoothing, phase sync, etc.) live here or on Page 3 — exact placement TBD during Phase 7.

### Page 7 — Master / Transport

The transport page. Tempo and groove on the top row of knobs; transport buttons on the bottom row. **This is the only page where buttons act as transport** — leaving the page returns the buttons to their normal page-jump function.

#### Top Row — Knobs

| Knob | Name | Description |
|------|------|-------------|
| Top 1 | `BPM ` Tempo | Master tempo. |
| Top 2 | `GROO` Groove Type | Groove preset (TBD: may collapse into single SWING knob). |
| Top 3 | `AMNT` Groove Amount | Groove depth. |
| Top 4 | `LTCK` Hold Mode | Voltage Block / Elektron — the global step-hold behavior (see Step Hold Modes). |

#### Bottom Row — Transport Buttons

The bottom row of the LCD shows pipe-separated button labels. Buttons 4-5-6 act as transport while on this page.

```
                |PLAY|PAUS|RST |
```

- **PLAY (Button 4):** Starts playback from the current position. If already playing, no-op. From paused, resumes.
- **PAUSE (Button 5):** Pauses playback, preserving position. From paused, resumes (same as Play). When stopped, no-op.
- **RESET (Button 6):** All track playheads jump to step 1. Transport state is preserved — if playing, stays playing; if stopped, stays stopped.

#### LED Feedback

- **PLAY LED:** Blinks during playback. Off when stopped or paused.
- **PAUSE LED:** Blinks while paused. Off otherwise.
- **RESET LED:** Brief flash on press.

Slot 4 of the bottom row may carry `MRST` (master reset count, an automatic playhead-reset cycle) or other voice-config — TBD. Currently `MRST` is the strongest candidate.

A dedicated **Stop** button (full halt of all voices, distinct from Pause) is reserved for a future revision once empirical use confirms it's needed. To fully stop with current controls: press Pause, then Reset.

### Page 8 — Load / Save

**State snapshot model:** the entire instrument state — all 6 tracks of pattern data, locks, voice defaults, voice config, mod matrix, global settings — is saved as a single file. There are no separate "patches", "programs", "parts", or "multis" — that hierarchy is replaced by one all-inclusive snapshot.

**v1:** one save slot. Press to save, press to load.

**Future:** multiple slots with a picker UI.

---

## Sequencer Mode

Press **Button 5** to enter. Press **Button 5** again to exit back to Normal Mode.

### Controls

- **Buttons 1–8:** Step toggles. Tap to turn a step on or off. Hold to enter lock edit for that step.
- **Knobs 1–8:** Edit lockable parameters for the current sub-page.
- **Encoder turn:** Scroll through parameters. The three sequencer sub-pages are continuous — the encoder flows through them without stopping at boundaries.
- **Encoder push:** Focused edit on the highlighted parameter — full name and value displayed prominently; encoder turn directly adjusts; push again to exit.

### Sequencer Voice Page 1 — Oscillators

The primary timbre page — algorithm select, pitch relationship, interaction. Identical layout to Page 1 of normal mode but at sequencer scale (locks per step rather than voice defaults).

| Knob | Name | Description |
|------|------|-------------|
| Top 1 | `NOTE` Base Note | Carrier (Osc 1) pitch in semitones. |
| Top 2 | `WAVE` Osc 1 Wave | Osc 1 algorithm. |
| Top 3 | `PARA` Osc 1 Parameter | Algorithm-specific knob for Osc 1. |
| Top 4 | `BLND` Oscillator Interaction | Bipolar mix-or-FM. |
| Bot 1 | `RTIO` Osc 2 Ratio | Frequency ratio of Osc 2 relative to Osc 1. |
| Bot 2 | `WAVE` Osc 2 Wave | Osc 2 algorithm. |
| Bot 3 | `PARA` Osc 2 Parameter | Algorithm-specific knob for Osc 2. |
| Bot 4 | `FINE` Osc 2 Fine Tune | ±100 cents detune on the modulator. |

### Sequencer Voice Page 2 — Modulation

Envelope decays/amounts, noise, sub-osc.

| Knob | Name | Description |
|------|------|-------------|
| Top 1 | `LPGD` LPG Decay | Anchor decay time. Drives the VCA envelope (Env1) directly and the filter envelope (Env2) with `LPGO`-derived offset. The primary "decay" knob for percussion. |
| Top 2 | `LPGA` LPG Amount | How strongly the LPG opens/closes the filter. At 0, filter is static at the voice-config `FREQ` value. At max, full LPG sweep. |
| Top 3 | `LPGO` LPG Offset | Bipolar personality macro. Center = vactrol-like (filter slightly faster than amp). CCW = plucky (filter much faster). CW = looser/resonant tail (filter holds longer). |
| Top 4 | `NOIS` Noise Amount | White noise mixed into the main signal path. |
| Bot 1 | `PITD` Pitch Env Decay | Env3 fall time. |
| Bot 2 | `PITA` Pitch Env Amount | Env3 → carrier pitch (bipolar — positive sweeps up to base, negative sweeps down to base). |
| Bot 3 | `WAVE` Sub-Osc Wave | Sub-oscillator shape. 11 options: Square 1/2, Triangle 1/2, Pulse 1/2 (traditional sub) plus Click, Glitch, Blow, Metallic, Pop (transient hits). |
| Bot 4 | `SUB ` Sub-Osc Level | Mix amount of the sub layer. |

### Sequencer Step Page — Step Behavior

Controls *when*, *whether*, and *how* the step fires. Processed by the sequencer on the motherboard; only the resolved pitch and parameters reach the voice card.

| Knob | Name | Description |
|------|------|-------------|
| Top 1 | `PROB` Probability | 0–100%. The step fires or is silently skipped. |
| Top 2 | `SSUB` Sub-steps / Ratchets | Bipolar from center. Clockwise (+1 to +8) = ratchet count. Center (0) = normal. Counter-clockwise –1 = Custom sub-step playback. CCW –2 = Sub-step edit mode. See below. |
| Top 3 | `REPT` Step Repeat | How many times the step replays before the playhead advances. |
| Top 4 | `RATE` Step Rate | Per-step timing: half / normal / double / triplet. |
| Bot 1 | `VEL ` Velocity | Step velocity / accent level. |
| Bot 2 | `GLID` Glide | Ties this step to the next — suppresses envelope retrigger, producing a legato effect. |
| Bot 3 | `MINT` Mutate Interval | Pitch interval to walk across repeats and sub-steps. Off, Scale, Min3, Min5, Min7, Min9, Maj3, Maj5, Maj7, Maj9, Oct. |
| Bot 4 | `MDIR` Mutate Direction | Walk direction: Up / Down / UpDn (alternating) / Random. Only active when `MINT` is not Off. |

### Locking Parameters

**Hold a step button** to enter lock edit mode for that step. The LCD shows current lock values for the active sub-page. **Turn any knob** to write a lock for that parameter on that step. Locked parameters are shown on the LCD. Release the step button to exit lock edit.

**Clearing locks:** Double-tap while holding a step to clear all locks for that step back to voice defaults.

Locked parameters snap back to voice defaults after the step fires. Locking a step does not affect other steps. Every trigger sends a full parameter snapshot — either the step's locked values or the current knob positions, with no carryover between steps.

**Unlocked steps** always reflect the current physical knob position in real time. Turning a knob while a pattern plays immediately affects all unlocked steps. This tension between locked and unlocked steps is the main performance dimension.

### Sub-steps (SSUB)

`SSUB` is a bipolar knob with three zones:

- **CW (+1 to +8):** The step fires as N evenly-spaced ratchets within its time slot.
- **Center (0):** Normal — step fires once.
- **CCW –1 (Custom):** Plays back a stored sub-step pattern. 8 bits representing 8 sub-step positions on or off.
- **CCW –2 (Edit):** Enters sub-step edit mode. Buttons 1–8 represent the 8 sub-steps. Tap any button to toggle that sub-step on or off. Release the parent step button — sub-steps remain editable. Turn `SSUB` back to –1 to exit edit mode and play back the pattern.

All sub-step hits share the same parameter snapshot. There are no per-sub-step locks.

### Mutation (MINT + MDIR)

Mutation applies a pitch interval walk across a step's repeats and/or sub-steps. `MINT` selects the interval; `MDIR` selects the direction.

When both `REPT` and `SSUB` are active, repeats mutate first, and sub-steps within each repeat continue the walk from the repeat's pitch:

```
step pitch = base + mutate(repeat_index) + mutate(substep_index)
```

When the walk reaches the top or bottom of the playable range, it resets to base pitch and restarts — sawtooth reset, not a hard clamp.

Mutation is resolved entirely on the motherboard. The voice card only receives the final computed pitch.

---

## Step Hold Modes

Set on Page 7 (`LTCK`). Global — applies to all tracks.

### Voltage Block Mode

Hold a step → the playhead **pauses** on that step. The held step fires repeatedly so you hear your edits in real time. A shadow playhead continues in the background, maintaining timing relative to other tracks. Release the step → the playhead snaps to the shadow position. You may have silently missed some steps while holding, but pattern phase is preserved. Good for live performance.

### Elektron Mode

Hold a step → the playhead **keeps running**. Knob turns write lock values to the held step's storage. You hear the result the next time that step comes around. Good for studio programming.

---

## Voice Architecture

### Signal Chain

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
                                                          Env2 (Filter, LPGA-scaled)
                                                          (LPGD × LPGO)

Env3 (Pitch) ──▶ Osc 1 base pitch (PITA-scaled, PITD time) — independent
LFO          ──▶ voice-config target
```

### Envelopes — LPG-Coupled Amp+Filter, Independent Pitch

Three envelopes, but **Env1 (VCA) and Env2 (Filter) are coupled through an LPG macro**. Env3 (Pitch) is fully independent.

- **Env1 (VCA):** Shapes the output amplitude. Always-on. Decay time is set by `LPGD`.
- **Env2 (Filter):** Modulates the SVF cutoff. Decay time is `LPGD` modified by `LPGO`'s offset; depth is `LPGA`. Setting `LPGA` to 0 makes the filter static at the cutoff knob value. Setting `LPGA` high makes Env2 sweep the filter.
- **Env3 (Pitch):** Modulates Osc 1's base pitch. Decay is `PITD`; depth is `PITA` (bipolar — positive sweeps up to base, negative sweeps down to base). Independent of the LPG macro. Used for kick thump, tom sweep, click attack.

The three LPG macro knobs:

- **`LPGD` (LPG Decay):** Anchor decay time. The most-tweaked per-step envelope knob.
- **`LPGA` (LPG Amount):** Filter env depth. 0 = filter is static; max = full sweep.
- **`LPGO` (LPG Offset):** Bipolar personality macro:
  - **Center:** typical vactrol-like — filter decays slightly faster than amp.
  - **CCW:** more plucky — filter closes much faster than amp; highs vanish well before body fades.
  - **CW:** looser/resonant tail — filter holds longer than amp.
  - At extremes, `LPGO` may also shift envelope curve and dial in a small filter Q to enhance the personality.

Envelope curves are baked-in per envelope and tuned for the LPG behavior at center `LPGO`. There is no separate VCA decay or filter decay knob — the macro structure is intentional: one decay (LPGD), one personality knob (LPGO), one amount (LPGA).

### The BLND Knob

A single bipolar knob that subsumes oscillator routing:

- **Below center (0–63):** Crossfade between the two oscillators in the audio path. At 0, only Osc 1 is heard; at 63, only Osc 2.
- **At center (63→64):** Acoustically silent transition. Both endpoints are "Osc 1 only, no FM."
- **Above center (64–127):** Osc 2 modulates Osc 1's pitch (FM). Osc 2 leaves the audio path. FM amount sweeps from 0 at 64 to maximum at 127.

This means: at any setting, you hear *either* a mix of Osc 1 and Osc 2 OR Osc 1 being FM'd by Osc 2 — never both. The constraint simplifies operating mental model and matches the natural FM design pattern of operator-as-modulator-only.

### Sub-Oscillator and Transients

The sub-oscillator slot doubles as a transient generator. Eleven shapes total: six traditional sub-bass shapes (square, triangle, pulse — each in two octave-related variants) and five transient one-shots (Click, Glitch, Blow, Metallic, Pop). Selecting a transient shape produces a one-shot attack on note-on, mixed at the `SUB` level into the main path. This is independent of Osc 1 / Osc 2 — you can layer a Click transient under a vowel-and-FM voice without touching either oscillator's wave selection.

---

## Appendix — Parameters by Page

`†` = also appears as a per-step lockable in Sequencer Mode

### Page 1 — Oscillator (Voice Defaults)

| Abbrev | Name | Notes |
|--------|------|-------|
| `NOTE` † | Base Note | Carrier pitch in semitones |
| `WAVE` † | Osc 1 Wave | Algorithm select for Osc 1 |
| `PARA` † | Osc 1 Parameter | Algorithm-specific timbral knob (Osc 1) |
| `BLND` † | Oscillator Interaction | Bipolar 0–63 mix / 64+ FM |
| `RTIO` † | Osc 2 Ratio | Frequency ratio Osc 2 : Osc 1 |
| `WAVE` † | Osc 2 Wave | Algorithm select for Osc 2 |
| `PARA` † | Osc 2 Parameter | Algorithm-specific timbral knob (Osc 2) |
| `FINE` † | Osc 2 Fine Tune | ±100 cents detune on Osc 2 |

### Page 2 — Filter

| Abbrev | Name | Notes |
|--------|------|-------|
| `FREQ` | Cutoff Freq | SVF base frequency |
| `RES ` | Resonance | SVF resonance |
| `TYPE` | Filter Mode | LP / BP / HP |
| `DRIV` | Filter Drive | Saturation gain into SVF |
| `BITS` | Bit Reduction | Pre-DAC bit depth reduction |

(Knob layout placeholder — final assignment in Phase 7.)

### Page 3 — Modulation

LFO and envelope amount/curve voice-level settings. Final knob layout TBD in Phase 7.

| Abbrev | Name | Notes |
|--------|------|-------|
| `LSHP` | LFO Shape | Sine / triangle / square / ramp / random |
| `LFOR` | LFO Reset | Free / per-step / per-pattern |
| `LFOA` | LFO Amount | Modulation depth |
| `LFOD` | LFO Destination | Pitch / cutoff / fold / FM |
| `LFOS` | LFO Speed | (TBD location) |
| `RANG` | LFO Range | (TBD whether retained) |
| `TRAK` | LFO Tracking | Pitch tracking on/off (TBD location) |

### Page 6 — Track Settings

| Abbrev | Name | Notes |
|--------|------|-------|
| `SCAL` | Scale | Scale quantization |
| `ROOT` | Root Note | Root for scale-relative ops |
| `BPCH` | Base Pitch | Default pitch / Mutate base |
| `OLEV` | Output Level | Voice mix level |
| `DIRN` | Direction | Fwd / Rev / Pendulum / Random |
| `CDIV` | Clock Div | 1, 2, 3, 4, 6, 8, 12, 16 |
| `ROTA` | Rotate | Pattern start offset (0–7) |
| `LENG` | Pat Length | Steps per cycle (1–8) |

### Page 7 — Master / Transport

| Slot | Abbrev | Name | Notes |
|------|--------|------|-------|
| Top 1 | `BPM ` | Tempo | Master tempo |
| Top 2 | `GROO` | Groove Type | (May collapse with AMNT into single SWING) |
| Top 3 | `AMNT` | Groove Amount | |
| Top 4 | `LTCK` | Hold Mode | Voltage Block / Elektron |
| Btn 4 | `PLAY` | Play | LED blinks while playing |
| Btn 5 | `PAUS` | Pause | LED blinks while paused |
| Btn 6 | `RST ` | Reset | All playheads → step 1 |
| Bot 4 | `MRST` | Master Reset | (Placeholder — global cycle length) |

### Sequencer Voice Page 1 — Oscillators (Lockable)

| Abbrev | Name | Voice Default From |
|--------|------|--------------------|
| `NOTE` | Base Note | Page 1 Top 1 |
| `WAVE` | Osc 1 Wave | Page 1 Top 2 |
| `PARA` | Osc 1 Parameter | Page 1 Top 3 |
| `BLND` | Oscillator Interaction | Page 1 Top 4 |
| `RTIO` | Osc 2 Ratio | Page 1 Bot 1 |
| `WAVE` | Osc 2 Wave | Page 1 Bot 2 |
| `PARA` | Osc 2 Parameter | Page 1 Bot 3 |
| `FINE` | Osc 2 Fine Tune | Page 1 Bot 4 |

### Sequencer Voice Page 2 — Modulation (Lockable)

| Abbrev | Name | Voice Default From |
|--------|------|--------------------|
| `LPGD` | LPG Decay | Page 3 (TBD slot) |
| `LPGA` | LPG Amount | Page 3 (TBD slot) |
| `LPGO` | LPG Offset | Page 3 (TBD slot) |
| `NOIS` | Noise Amount | Page 1 (TBD location) or Page 3 |
| `PITD` | Pitch Env Decay | Page 3 (TBD slot) |
| `PITA` | Pitch Env Amount | Page 3 (TBD slot) |
| `WAVE` | Sub-Osc Wave | Page 1 / Page 3 (TBD) |
| `SUB ` | Sub-Osc Level | Page 1 / Page 3 (TBD) |

### Sequencer Step Page — Step Behavior (Lockable)

| Abbrev | Name | Notes |
|--------|------|-------|
| `PROB` | Probability | 0–100% fire chance |
| `SSUB` | Sub-steps / Ratchets | +8→+1 ratchets, 0 normal, –1 custom, –2 edit |
| `REPT` | Step Repeat | Replay count before advance |
| `RATE` | Step Rate | ½ / ×1 / ×2 / triplet |
| `VEL ` | Velocity | Step velocity |
| `GLID` | Glide | Tie to next step |
| `MINT` | Mutate Interval | Pitch walk interval |
| `MDIR` | Mutate Direction | Up / Down / UpDn / Random |
