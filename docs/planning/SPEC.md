# Ambika Percussive Synth — Firmware Spec

> **OBSOLETE — Kept for reference only.** This document describes the prior Carcosa-based design and predates the project's pivot to YAM as the fork base. The current authoritative spec is `SPEC_v2.md`. Preserved as source material for design ideas, parameter naming, and structural patterns that may inform replacement work.

## Project Overview

Fork of the Mutable Instruments Ambika firmware to create a **6-voice polymetric percussive synthesizer** with west coast–inspired voices, parameter-lock sequencing, and inter-track modulation. The design philosophy prioritizes **constrained playability over feature depth** — inspired by instruments like Fors Dyad, where limited but well-chosen parameters create a more engaging instrument than exhaustive control.

### Hardware Platform

- **Motherboard:** ATmega644p (64KB flash, 4KB RAM, 20MHz)
  - Handles UI, sequencer, MIDI, SD card storage, SPI bus master
- **Voice cards (x6):** ATmega328p (32KB flash, 2KB RAM each)
  - Each runs its own oscillator → filter → FG signal chain independently
  - Connected to motherboard via SPI
- **Display:** 2x40 character LCD
- **Controls:** Push encoder (clickable), 8 potentiometers (4 above LCD, 4 below), 8 buttons with LEDs
- **Audio:** Per-voice individual outputs + mixed output (808-style normalling: plugging an individual out removes that voice from the mix)
- **Filter type:** SVF (2-pole multimode state variable filter) on all voice cards
- **Storage:** SD card (FAT16/FAT32, 8.3 filenames)

### Design Principles

1. **Constraint breeds playability.** Every parameter must earn its panel/step presence. No deep menus, no hidden features.
2. **The instrument is the interaction between voices.** Polymetric relationships, track-to-track modulation, and the tension between locked and unlocked steps are the primary expressive dimensions.
3. **West coast architecture, percussion focus.** Complex oscillator → multimode SVF (used as LPG or HPF or both) → function generator. The same voice that makes plonky bass makes metallic hats via high FM ratio + HPF + fast decay.
4. **No dedicated voice types for specific drums.** The SVF's multimode output, the noise source, and the complex oscillator's range cover kicks through hats without wasting flash on specialized engines. If a second voice algorithm fits in flash (e.g., modal/Rings-style), it's a bonus, not a requirement.
5. **Every step is stateless.** Locked parameters snap back to voice defaults after the step fires. No state leaks between steps. What you see (lock indicator) is exactly what you get on that step and only that step.

---

## Voice Architecture

Each voice card runs the same signal chain. All timbral variety comes from parameter differences, not separate engines.

### Signal Chain

```
           ┌──────────┐     ┌──────────┐
           │  Noise   │     │   LFO    │
           │  (LFSR)  │     │ (audio-  │
           └────┬─────┘     │  rate    │
                │           │ capable) │
                │ Noise Mix └────┬─────┘
                ▼                │ Target: Pitch / Cutoff / Fold / FM
                                 │
┌──────────┐  FM   ┌──────────┐  │   ┌───────┐   ┌─────────┐   ┌─────────┐
│  Osc 1   │──────▶│  Osc 2   │──┼──▶│ Drive │──▶│   SVF   │──▶│   Mix   │
│(Modulator)│      │(Carrier) │  │   │ (gain)│   │ LP/BP/HP│   │  Output │
└──────────┘      │+ Wavefold│  │   └───────┘   └─────────┘   └────┬────┘
                   └──────────┘  │                    ▲              │
                                 │                    │              ▼
                          LFO modulates          FG-B (filter)   ┌──────┐
                          target param           Decay offset    │ FG-A │
                                                 Curve offset    │(VCA) │
                                                      │          └──────┘
                                                      │              │
                                                 Coupled via         │
                                                 LPG Depth          OUT
```

### Dual Function Generator (LPG Emulation)

The LPG behavior is achieved through two function generators running in parallel from shared controls, with configurable offsets:

**FG-A (VCA):** Controls output amplitude. Uses the performer-facing Rise, Fall, and Curve parameters directly.

**FG-B (Filter):** Controls filter cutoff. Cloned from FG-A with two offsets applied:

- **Decay Offset (`DOFS`):** Makes FG-B's fall time shorter or longer than FG-A. Negative = filter closes faster than VCA = highs die before body = classic LPG character. **Default: negative (vactrol-like).**
- **Curve Offset (`COFS`):** Shifts FG-B's response shape relative to FG-A. Pushes the filter's curve toward steeper exponential decay while the VCA stays gentler. **Default: slight positive offset (steeper filter curve).**

Both FGs use a two-stage decay curve: fast initial slope followed by a slower residual tail. This mimics the vactrol's nonlinear photoresistor response. FG-B with a shorter decay and steeper curve hits its inflection point earlier than FG-A, naturally creating the spectral evolution where highs disappear before the sound's body fades.

```
Per audio sample:
  if (fg_level > INFLECTION_POINT):
      fg_level -= fast_decay_rate
  else:
      fg_level -= slow_decay_rate

FG-A output → VCA amplitude (direct)
FG-B output → filter cutoff (scaled by LPG Depth)
```

**LPG Depth (`FDEP`)** controls the coupling: at 0, FG-B doesn't reach the filter (VCA-only, filter is static). At max, full LPG behavior. The offsets only matter when LPG Depth is non-zero.

The performer sees one envelope (decay knob on the sequencer, rise/fall/curve on the FG page). The offsets are set-and-forget on the VCF page — dial in how the LPG feels once, then perform with decay and LPG depth.

**Implementation note:** FG-A and FG-B are two instances of the same envelope code (which already exists in Carcosa with a loop/cycling mode suitable for function generator behavior). FG-B's fall rate and curve are derived from FG-A's values plus the offsets, computed once per trigger. No new state machine needed.

### Complex Oscillator

Two oscillators with FM, noise injection, and wavefolding — inspired by Buchla-style complex oscillator topology:

- **Osc 1 (Modulator):** Frequency set by base pitch. Waveform is one of three shapes: sine, triangle, or square — selected via `SHAP`.
- **Osc 2 (Carrier):** Output oscillator. Frequency derived from Osc 1 × Ratio, plus optional semitone offset (±36). Receives FM from Osc 1. Waveform is one of three shapes: sine, triangle, or square — selected via `SHAP`.
- **Wave Shapes (`SHAP`):** Selects the waveform tap for both oscillators simultaneously. Nine combinations from {sine, triangle, square} × {sine, triangle, square}, displayed as modulator/carrier (e.g., S/T = sine modulator, triangle carrier). These are output taps — both shapes always exist in the oscillator; `SHAP` selects which ones are routed forward.
- **Ratio (`A:B`):** Frequency ratio between Osc 1 and Osc 2. Integer and non-integer ratios. Primary timbral control. **Implementation:** use `lut_res_fm_frequency_ratios` (25-entry table already in voicecard resources: 0.125, 0.25, 0.5, sqrt(2)/2, pi/4, 1.0, sqrt(2), pi/2, 7/4, 2, 9/4, 11/4, 2*sqrt(2), 3, pi, 4, 5, 8, and others — DX-style set covering sub-harmonic through high harmonic and key inharmonic ratios).
- **Detune (`DET`):** Bipolar pitch offset applied to Osc 1. Detuning the modulator changes the FM beating pattern / timbral flutter without shifting the perceived pitch of the carrier.
- **FM Depth (`FM`):** Modulation index applied to Osc 1 → Osc 2.
- **Noise (`NOIS`):** Bipolar. Center = no noise. CW = noise blended into carrier output post-fold (adds noise body — snare, clap, breath). CCW = noise blended into the FM source path (noisy FM — gritty, textural, pitched-but-dirty).
- **Timbre (`TIMB`):** Carrier-only timbral modifier. Behavior depends on the carrier waveform set by `SHAP`: sine or triangle carrier = wavefolder depth (iterative reflection, adds harmonics); square carrier = pulse width (narrows duty cycle from 50% square toward thin pulse).
- **Osc 2 Pitch (`PIT2`):** Semitone offset applied to Osc 2, ±36 semitones (6 octaves). For melodic sequences or large pitch jumps within a pattern.

#### Noise Generator

Linear-feedback shift register (LFSR), a few bytes of code. Available in two injection points via `NOIS`: into the FM source path (CCW) or directly into the carrier output post-fold (CW). Spectrally flat white noise character, distinct from audio-rate S&H (which is stepped/aliased and grittier).

> **Implementation note:** Carcosa's `westcoast.h` is the starting point. Its fold algorithm (iterative reflection, quadratic gain curve), FM path (16-bit sine via `InterpolateSine16`), post-fold color filter (2-pole IIR), and `env_to_fold` are all reusable. The modulator needs a separate audio output path added (currently FM-source-only), integer-only ratio logic replaced with `lut_res_fm_frequency_ratios`, noise injection added at both the FM mix point and post-fold output, and `TIMB` wired to pulse width when carrier is square. `InterpolateSine16` and `wav_res_sine16` are currently defined in `fm4op.h`; move them to a shared header before stripping FM4OP.

### Filter Section

#### SVF Filter (as LPG / HPF / Timbral Shaping)

The SVF provides simultaneous lowpass, bandpass, and highpass outputs. Filter mode selects which output passes through.

- **Cutoff (`FREQ`):** Base filter frequency.
- **Resonance (`RES`):** SVF resonance.
- **Filter Mode (`TYPE`):** LP / BP / HP output select. All three outputs exist simultaneously — switching is glitch-free. LP for LPG-style plonks, HP for hats/cymbals, BP for nasal/metallic tones.
- **LPG Depth (`FDEP`):** How strongly FG-B modulates the filter cutoff. At 0, filter is static and only FG-A controls amplitude. At max, full LPG behavior.
- **Drive (`DRIV`):** Gain stage before the SVF input. At low levels, clean. At high levels, pushes the filter into soft clipping/saturation. Adds grit and thickness. Interacts with resonance — drive + high Q gets aggressive.
- **Bit Reduction (`BITS`):** Reduces the bit depth of the oscillator output before it reaches the DAC and analog filter. At 0 = off (full resolution). At max = 1-bit (extreme digital grit). Adds harmonic content that the SVF then shapes. Per-step lockable.

> **Note:** A true dry/wet crossfade between pre- and post-filter signal is not implementable. The signal path is linear — oscillators → digital mixer → DAC → analog SVF — with no pre-filter tap available in the digital domain. Filter bypass is a shortcut preset: cutoff = max, resonance = min.

#### Decay Offset and Curve Offset

Set on the VCF page (Button 2). Voice-level settings, not per-step lockable.

- **Decay Offset (`DOFS`):** Bipolar. Adjusts FG-B's fall time relative to FG-A. Default: negative (vactrol-like, filter closes faster).
- **Curve Offset (`COFS`):** Bipolar. Adjusts FG-B's curve shape relative to FG-A. Default: slight positive (steeper filter decay curve).

### Function Generator (Envelope)

The performer-facing envelope. Controls both FG-A (VCA) and FG-B (filter, via offsets) simultaneously.

- **Rise (`RISE`):** Attack time. For percussion, typically near-zero.
- **Fall / Decay (`FALL`):** Primary envelope time. The most important per-step parameter for percussion (exposed as `LPGD` on sequencer page 1).
- **Curve (`CURV`):** Response shape for FG-A. Linear → logarithmic → exponential. FG-B derives its curve from this value plus the curve offset.
- **Loop (`LOOP`):** On/off. When on, FG cycles (rise → fall → rise...) for drone/LFO behavior. Not step-lockable — voice-level setting.

### Velocity Function Generator (FG-C)

A third envelope instance, triggered automatically on each note-on. Creates a velocity-responsive transient on a configurable destination — the percussive equivalent of a vactrol responding to signal level.

- **Attack:** Instant. Fires immediately on trigger; no rise time.
- **Peak:** `velocity × VAMT / 127`. At full velocity, peak equals VAMT. Softer hits scale proportionally.
- **Fall:** Set by `VDEC` — independent from FG-A. A slow body decay can still have a fast filter ping.
- **Destination (`VDST`):** Pitch, cutoff, fold, FM depth. Modulates only the destination parameter; all others are unaffected.
- **Curve:** Borrows FG-A's `CURV` setting — keeps the velocity transient tonally consistent with the main envelope without requiring a separate param slot.

At VAMT = 0 or velocity = 0, FG-C produces no output. When active, harder hits create a brighter transient that decays naturally. `VDST` and `VAMT` are voice config settings on Page 6b — not per-step lockable. `VDEC` is per-step lockable on Sequencer Voice Page 2, with the Page 6b value serving as the voice default for unlocked steps.

**Implementation note:** FG-C is a third instance of the same envelope code on the voice card. The motherboard sends `VDST`, `VAMT`, `VDEC`, and `velocity` in the trigger snapshot; the voice card computes the peak scaling locally once per trigger.

### Pitch Envelope ("Bend")

A simple dedicated pitch transient, essential for percussion:

- **Bend Level (`BEND`):** Depth of pitch sweep (bipolar — positive or negative).
- **Bend Time (`TIME`):** Duration of pitch sweep. Fast = kick thump, slow = tom sweep, very fast = click transient.

### LFO (Modulation Source / Third Operator)

A single LFO per voice that spans from sub-audio modulation rates to audio rate. At low speeds it provides traditional modulation (vibrato, filter sweep, tremolo). At audio rate it becomes a third oscillator — effectively a third FM operator or, with random/S&H waveshape, a noise source.

**Lockable parameters (per-step via sequencer page 2):**
- **LFO Destination (`LFOD`):** Target (pitch, cutoff, fold, FM depth, sample rate reduction). Determines the synthesis technique at audio rate. Sample rate reduction (`CRSH`) skips DAC updates to reduce effective sample rate — at audio-rate LFO speeds this creates aliased textures; at slow speeds it steps the sample rate down in discrete jumps.
- **LFO Speed (`LFOS`):** Speed within the current range setting. Usable knob resolution because the range switch handles the coarse selection.
- **LFO Amount (`LFOA`):** Modulation depth. Primary gesture for bringing LFO character in/out per step.

**Non-lockable parameters (voice-level settings, FG/Modulation page):**
- **LFO Shape (`LSHP`):** Sine, triangle, square, ramp, random/S&H. At audio rate, shape determines the timbral character of the third operator.
- **LFO Reset (`LFOR`):** Free-running, reset on step trigger, reset on pattern start.
- **LFO Range (`RANG`):** Low / Mid / High. Shifts the speed range so the LFOS knob has usable resolution in each band. Low = sub-Hz to ~20Hz, Mid = ~1Hz to ~200Hz, High = ~20Hz to 2kHz+.
- **LFO Pitch Tracking (`TRAK`):** On/off (or amount). When on, LFO frequency scales with note pitch, maintaining a consistent harmonic ratio to the fundamental — essential for the "third operator" use case. Without tracking, the LFO creates different harmonic relationships at different pitches.

---

## UI Architecture

### Navigation Model

Two modes of operation, switched via Button 5 (Seq):

```
NORMAL MODE (default):
  Buttons 1–8 = page select (quick-jump to first param of that page)
  Knobs = edit the 8 parameters currently displayed on the LCD
  Encoder turn = scroll continuously through ALL parameters on ALL pages
                 (each page is exactly 8 params; encoder flows between pages
                  without stopping at page boundaries)
  Encoder push = confirm / secondary action (context-dependent)

SEQUENCER MODE (enter via Button 5):
  Buttons 1–8 = step triggers
  Knobs = lockable parameters (three pages, cycled by encoder turn)
  Encoder push = exit sequencer mode
```

Each normal-mode page is exactly 8 parameters mapped to the 8 knobs. Button presses are quick-jump shortcuts; the encoder is the universal access method that scrolls the full parameter list continuously.

### LCD Abbreviation Convention

The 2x40 LCD shows 4-character abbreviations per parameter in the overview (8 params across the display). The full parameter name appears on the second line when actively adjusting with the encoder. All abbreviations are confirmed below; marked (draft) where subject to change during implementation.

---

## Normal Mode — Button Assignments

Each button selects a parameter page. Each page displays up to 8 parameters on the 2x40 LCD, edited by the 4 top + 4 bottom knobs.

| Button | Label (Original) | New Function |
|--------|-------------------|--------------|
| 1 | Osc/Mixer | **Oscillator** |
| 2 | VCF | **Filter / LPG** |
| 3 | Env/LFO | **FG / Modulation** |
| 4 | Mod Matrix | **Track Relationships** |
| 5 | Seq | **Enter Sequencer Mode** |
| 6 | Voice/Parts | **Track Settings (6a)** — press again for **Voice Config (6b)** |
| 7 | Perf | **Performance / Master** |
| 8 | Load/Save | **Load / Save** |

### Page 1 — Oscillator (Button 1)

| Knob | Abbrev | Parameter | Notes |
|------|--------|-----------|-------|
| Top 1 | `VSEL` | Voice Select | Active voice (1–6). Same control as Page 7. |
| Top 2 | `SHAP` | Wave Shapes | Modulator/carrier waveform tap combo. 9 options: S/S, S/T, S/Q, T/S, T/T, T/Q, Q/S, Q/T, Q/Q (S=sine, T=triangle, Q=square) |
| Top 3 | `A:B ` | Osc Ratio | Frequency ratio between oscillators |
| Top 4 | `FM  ` | FM Depth | Modulation index Osc1→Osc2 |
| Bot 1 | `NOIS` | Noise | Bipolar. Center = off. CW = noise into carrier post-fold. CCW = noise into FM path. |
| Bot 2 | `TIMB` | Timbre | Carrier timbral modifier. Sine/triangle carrier = wavefold depth. Square carrier = pulse width. |
| Bot 3 | `BEND` | Bend Level | Pitch sweep depth (bipolar) |
| Bot 4 | `TIME` | Bend Time | Pitch sweep duration |

### Page 2 — Filter / LPG (Button 2)

| Knob | Abbrev | Parameter | Notes |
|------|--------|-----------|-------|
| Top 1 | `FREQ` | Cutoff Freq | Base filter frequency |
| Top 2 | `RES ` | Resonance | SVF resonance |
| Top 3 | `TYPE` | Filter Mode | LP / BP / HP output select |
| Top 4 | `FDEP` | LPG EG Depth | FG-B → cutoff coupling amount |
| Bot 1 | `DOFS` | LPG Dcy Offset | FG-B decay relative to FG-A. Default: negative (vactrol-like) |
| Bot 2 | `COFS` | LPG Crv Offset | FG-B curve relative to FG-A. Default: slight positive |
| Bot 3 | `DRIV` | Filter Drive | Gain into SVF. Adds saturation/grit at high levels |
| Bot 4 | `BITS` | Bit Reduction | Bit depth reduction pre-DAC. 0 = off, max = 1-bit |

### Page 3 — FG / Modulation (Button 3)

| Knob | Abbrev | Parameter | Notes |
|------|--------|-----------|-------|
| Top 1 | `RISE` | FG Rise | Attack time |
| Top 2 | `FALL` | FG Fall | Decay time (same value as LPGD in sequencer — voice default) |
| Top 3 | `CURV` | FG Curve | Response shape (linear → log → exp) |
| Top 4 | `LOOP` | FG Loop | On/off. Cycling FG for drones |
| Bot 1 | `LSHP` | LFO Shape | Sine, tri, square, ramp, random/S&H |
| Bot 2 | `LFOR` | LFO Reset | Free-run / per-step / per-pattern |
| Bot 3 | `RANG` | LFO Range | Low / Mid / High frequency range |
| Bot 4 | `TRAK` | LFO Tracking | LFO freq scales with pitch (on/off or amount) |

### Page 4 — Track Relationships / Mod Matrix (Button 4)

Defines how tracks interact with each other. Displayed as a list or grid on the 2x40 LCD. Encoder navigates cells, knobs set type and amount. Maximum of 4 active relationships total to limit CPU and SPI overhead.

| Relationship Type | Description |
|-------------------|-------------|
| **Transpose Osc 1** | Source track's current pitch transposes destination's Osc 1 |
| **Transpose Osc 2** | Same, but Osc 2 only — allows detuning/beating across tracks |
| **Clock** | Source track's triggers clock the destination (dest only advances when source fires) |
| **Reset** | Source track's step 1 resets destination to step 1 |
| **Accent** | Source track's triggers boost destination's velocity when both fire simultaneously |

### Page 5 — Sequencer Mode (Button 5)

Described in full in the Sequencer Mode section below.

### Page 6a — Track Settings (Button 6)

Pattern and playback settings for the currently selected track. Not step-lockable.

| Knob | Abbrev | Parameter | Notes |
|------|--------|-----------|-------|
| Top 1 | `SCAL` | Scale | Scale quantize |
| Top 2 | `ROOT` | Root Note | Root / transpose |
| Top 3 | `BPCH` | Base Pitch | Base pitch for this voice |
| Top 4 | `OLEV` | Output Level | Voice output level |
| Bot 1 | `DIRN` | Direction | Fwd / Rev / Pendulum / Random |
| Bot 2 | `CDIV` | Clock Div | Per-track clock division (1,2,3,4,6,8,12,16) |
| Bot 3 | `ROTA` | Rotate | Shift pattern start (0–7) |
| Bot 4 | `LENG` | Pat Length | Pattern length (1–8) |

### Page 6b — Voice Config (Button 6 × 2)

Voice-level behavior settings. Not step-lockable. Encoder flows into this page naturally after 6a; Button 6 pressed again quick-jumps here.

| Knob | Abbrev | Parameter | Notes |
|------|--------|-----------|-------|
| Top 1 | `PHSE` | Phase Sync | Free / Sync. Sync resets oscillator phase on each trigger — consistent percussive onset. Free = running phase, more organic. |
| Top 2 | `SMTH` | Smoothing | Slew rate applied to continuous parameters between steps. 0 = instant snap. Higher = slower slew. Discrete params (SHAP, TYPE) always snap. |
| Top 3 | `VDST` | Velocity FG Dest | Destination for velocity-triggered transient: pitch, cutoff, fold, FM depth. |
| Top 4 | `VAMT` | Velocity FG Amt | Peak level of velocity FG at full velocity. 0 = off (FG-C disabled). |
| Bot 1 | `VDEC` | Velocity FG Decay | Decay time for FG-C. Voice default — overridden per step on Seq Voice Page 2. Independent from FG-A fall. |
| Bot 2 | `FLDS` | Fold Stages | Number of cascade fold passes (1–6). Sets the character of the folder: 1 = clean single-pass (Buchla-style), higher = denser harmonic cascade (Serge-style). Voice config only; not step-lockable. |
| Bot 3–4 | — | TBD | Reserved |

### Page 7 — Performance / Master (Button 7)

| Knob | Abbrev | Parameter | Notes |
|------|--------|-----------|-------|
| Top 1 | `VSEL` | Voice Select | Select active voice (1–6). Also on Page 1 Top 1. |
| Top 2 | `BPM ` | Tempo | Master tempo |
| Top 3 | `MRST` | Master Reset | All tracks reset after N steps |
| Top 4 | `SWNG` | Swing | Swing amount |
| Bot 1 | `HOLD` | Step Hold Mode | Voltage Block / Elektron (see below) |
| Bot 2–4 | | TBD | Mute states, scene select, etc. |

### Page 8 — Load / Save (Button 8)

Patch and pattern storage to SD card. Behavior TBD — may keep stock Ambika save/load structure and extend it for pattern data, or redesign. The SD card format needs to store both voice settings and sequencer patterns.

---

## Sequencer Mode

Entered by pressing Button 5 (Seq). Exited by pressing encoder push.

### Controls in Sequencer Mode

- **Buttons 1–8:** Step triggers. Tap = toggle on/off. Hold = enter parameter lock edit for that step.
- **Knobs 1–8:** Lockable parameters. Three pages, cycled by encoder turn.
- **Encoder turn:** Cycle through Voice Page 1, Voice Page 2, and Step Page.
- **Encoder push:** Exit sequencer mode, return to normal button navigation.

### Sequencer Voice Page 1 — Core Timbre (Lockable)

The primary performance page. Controls what the step sounds like — the most commonly tweaked parameters.

| Knob | Abbrev | Parameter | Notes |
|------|--------|-----------|-------|
| Top 1 | `A:B ` | Osc Ratio | Frequency ratio between oscillators |
| Top 2 | `DET ` | Detune (Osc1) | Bipolar offset on modulator. Changes FM flutter, not perceived pitch. |
| Top 3 | `FM  ` | FM Depth | Modulation index Osc1→Osc2 |
| Top 4 | `TIMB` | Timbre | Carrier modifier: wavefold (sine/tri carrier) or pulse width (square carrier) |
| Bot 1 | `LPGD` | LPG Decay | FG fall time. Most common per-step variation. |
| Bot 2 | `NOIS` | Noise | Bipolar. Center = off. CW = post-fold mix. CCW = FM path injection. |
| Bot 3 | `BEND` | Bend Level | Pitch sweep depth (bipolar) |
| Bot 4 | `TIME` | Bend Time | Pitch sweep duration |

### Sequencer Voice Page 2 — Extended Timbre (Lockable)

Secondary timbral controls. Waveshapes, pitch offset, filter details, and LFO performance controls.

| Knob | Abbrev | Parameter | Notes |
|------|--------|-----------|-------|
| Top 1 | `SHAP` | Wave Shapes | Modulator/carrier waveform tap combo (9 options) |
| Top 2 | `PIT2` | Osc2 Pitch | Semitone offset ±36. For melodic sequences or large pitch jumps. |
| Top 3 | — | TBD | Reserved |
| Top 4 | `VDEC` | Velocity FG Decay | Decay time of the velocity-triggered transient (FG-C). Per-step lockable — shorter on some steps, longer on others. |
| Bot 1 | `LFOD` | LFO Destination | Target: pitch, cutoff, fold, FM depth |
| Bot 2 | `LFOS` | LFO Speed | Speed within current range setting |
| Bot 3 | `LFOA` | LFO Amount | Modulation depth. Primary gesture for LFO character per step. |
| Bot 4 | `CRSH` | Sample Rate Reduction | Skips DAC updates to reduce effective sample rate. 0 = off (39kHz full rate). |

### Sequencer Step Page — Behavior (Lockable)

Controls *when, whether, and how* the step fires. Evaluated by the motherboard sequencer, not sent to voice cards (except final resolved pitch which is included in the trigger snapshot).

| Knob | Abbrev | Parameter | Notes |
|------|--------|-----------|-------|
| Top 1 | `PROB` | Probability | 0–100%. Step fires or is skipped. |
| Top 2 | `SSUB` | Sub-steps / Ratchets | Bipolar from center. CW (+1 to +8) = ratchet count (evenly spaced automatic subdivisions). Center (0) = normal step. CCW –1 = Custom (plays stored sub-step bitfield). CCW –2 = Edit (buttons 1–8 become sub-step toggles). Mutation (MINT/MDIR) applies to all subdivision hits regardless of mode. |
| Top 3 | `REPT` | Step Repeat | Times step replays before advancing |
| Top 4 | `RATE` | Step Rate | Per-step timing multiplier: half / normal / double / triplet |
| Bot 1 | `VEL ` | Velocity | Amplitude emphasis |
| Bot 2 | `GLID` | Glide | Tie to next step — affects FG retrigger |
| Bot 3 | `MINT` | Mutate Interval | Pitch interval for mutation walk: Off, Scale, Min3, Min5, Min7, Min9, Maj3, Maj5, Maj7, Maj9, Oct. Off = no mutation. |
| Bot 4 | `MDIR` | Mutate Direction | Up / Down / UpDn (alternating) / Rnd. Only active when MINT ≠ Off. |

### Step Interaction Model

- **Tap a step button:** Toggle step on/off.
- **Hold a step button:** Enter parameter lock edit. LCD shows current lock values for whichever page (voice 1 / voice 2 / step) is active. Turn any knob to write a lock for that parameter on that step.
- **Lock clear:** Double-tap a held step to clear all locks for that step back to defaults.
- **Ratchets:** While holding a step, turn `SSUB` clockwise (values +1 to +8) to set a ratchet count. The step fires that many evenly spaced times within its duration.
- **Custom sub-step playback:** Turn `SSUB` one click CCW (value –1 = Custom). The step plays back the stored sub-step bitfield without entering edit mode.
- **Sub-step edit:** Turn `SSUB` two clicks CCW (value –2 = Edit). The view switches — buttons 1–8 represent the 8 sub-steps of that step. Release the parent step button; sub-steps remain editable. Tap any button to toggle that sub-step on/off. Moving `SSUB` back to –1 (Custom) exits edit mode and plays back the pattern. All active sub-steps fire with the same parameter snapshot (no per-sub-step locks).

### Parameter Lock Behavior — Stateless Steps

**Locked parameters snap back to the voice default value after the step.** A locked step does not affect subsequent unlocked steps. Every trigger sends a full parameter snapshot to the voice card — either the step's locks or the voice defaults (current knob positions for unlocked params).

This means:
- Step 3 has Ratio locked to 3.5 and Decay locked to short
- Step 4 has no locks
- When step 3 fires: Ratio = 3.5, Decay = short (locked values)
- When step 4 fires: Ratio = whatever the knob is currently set to, Decay = whatever the knob is currently set to
- No carryover from step 3 to step 4

### Mutate (`MINT` + `MDIR`)

Applies a pitch interval walk across a step's repeats and/or sub-steps, turning a single step into an automatic melodic sequence without per-sub-step locks. Split across two independent knobs:

- **`MINT` (Mutate Interval):** Selects the interval to walk. 11 values: Off, Scale (one scale degree), Min3, Min5, Min7, Min9, Maj3, Maj5, Maj7, Maj9, Oct. Off disables mutation entirely. Stored as 1 byte.
- **`MDIR` (Mutate Direction):** Selects the walk direction. 4 values: Up, Down, UpDn (alternating), Rnd (random each hit). Only active when MINT ≠ Off. Stored as 1 byte.

Total storage unchanged from the previous combined MUT8 byte — MINT + MDIR together occupy the same 2 bytes formerly used by MTIM + MUT8.

**Subdivision modes:** Mutation applies to all subdivision hits — ratchets (SSUB +1 to +8), custom sub-steps (SSUB –1/–2), and repeats (REPT). The motherboard increments the mutation index on each hit regardless of which mode generated it.

**When both `REPT` and `SSUB` are active:** Repeats mutate first. Each repeat starts at `base_pitch + mutate(repeat_index)`. Sub-steps within that repeat walk from that repeat's pitch using the same interval.

```
substep_pitch = base_pitch + mutate(repeat_index) + mutate(substep_index)
```

**Ceiling/floor behavior:** When the walk hits the top or bottom of the playable note range, it resets to base pitch and restarts the walk. Sawtooth reset, not hard clamp.

**Implementation note:** `MINT` and `MDIR` are resolved entirely on the motherboard. The voice card receives only the final computed pitch in the trigger snapshot — it has no knowledge of mutation. The `Scale` interval respects the track's `SCAL` and `ROOT` settings; all other intervals are chromatic.

### Unlocked Step Behavior

Unlocked steps (or unlocked parameters on partially-locked steps) use the **current physical knob positions in real time**. Sweeping a knob while the pattern plays affects all unlocked steps immediately. Steps with locks are pinned and unaffected. This tension between locked and unlocked steps is the primary performance dimension.

> **Note:** `DET` (Seq Voice Page 1) and `PIT2` (Seq Voice Page 2) have no corresponding normal-mode knob. Their stored voice default is used for unlocked steps — both naturally default to 0 (no detune, no osc2 offset) and are only meaningfully set via per-step locks. `LFOD`, `LFOS`, `LFOA`, and `CRSH` are similar: no normal-mode knob, voice default applies to unlocked steps. This is intentional — these are step-sculpting tools, not live-sweep params.

### Pattern Settings

Per-track pattern parameters, set on the Track Settings page (Button 6):

| Parameter | Abbrev | Range | Description |
|-----------|--------|-------|-------------|
| Direction | `DIRN` | Fwd / Rev / Pendulum / Random | Playhead movement pattern |
| Clock Div | `CDIV` | 1, 2, 3, 4, 6, 8, 12, 16 | Per-track clock division — the polymeter engine |
| Rotate | `ROTA` | 0–7 | Shift pattern start point without changing step data |
| Length | `LENG` | 1–8 | Per-track pattern length. Combined with Clock Div, creates polymeters |

### Step Hold Modes

Configurable on the Performance/Master page (Button 7). Global setting, not per-track.

#### Voltage Block Mode (Snap)

- Hold step → playhead **pauses** on that step. The held step fires repeatedly so you can hear your edits in real time.
- A **shadow playhead** continues advancing silently in the background, maintaining correct timing relative to other tracks.
- Release step → playhead **snaps to the shadow position**. No timing disruption. You may have silently missed some steps while editing, but the pattern phase is preserved.
- Best for: live performance, auditioning locks in context.

#### Elektron Mode (No Pause)

- Hold step → playhead **keeps running normally**. The pattern doesn't pause.
- Knob movements write lock values to the held step's storage.
- You hear the result the **next time that step comes around**.
- No shadow playhead needed. Hold-step is purely a UI state that routes knob changes to a specific step's lock data.
- Best for: studio editing, programming patterns without interrupting playback.

### Shadow Playhead Implementation

The shadow playhead runs in both modes (the overhead is negligible — 5 bytes per track). In Elektron mode it simply isn't read on release. Per track:

```
Active playhead position:      1 byte
Shadow playhead position:      1 byte
Shadow clock div accumulator:  2 bytes
Edit state (held step index):  1 byte
Total:                         5 bytes × 6 tracks = 30 bytes
```

---

## Track Relationship Matrix

### Mod Matrix UI (Button 4)

The 2x40 LCD displays relationships as a list:

```
Line 1: [Src] Track 1 ──▶ [Dst] Track 3  Type: TransOsc1  Amt: +7
Line 2: [Src] Track 2 ──▶ [Dst] Track 5  Type: Clock      Amt: ---
```

Encoder scrolls through relationship slots (4 max). Knobs set source, destination, type, and amount. Keeping the max at 4 active relationships limits CPU cost — each relationship requires cross-track state evaluation per sequencer tick.

---

## Memory Architecture

### Per-Step Storage (on Motherboard)

All step data lives on the motherboard. Voice cards receive parameter snapshots per trigger and store no pattern data.

```
Voice Params Page 1:       8 bytes (one byte per lockable parameter)
Voice Params Page 2:       8 bytes (one byte per lockable parameter)
Step Params:               8 bytes (one byte per step behavior parameter)
  └─ SSUB byte encodes:    signed byte: –2=Edit, –1=Custom, 0=Normal, +1..+8=Ratchets
Lock Flags:                3 bytes (bitfield: which of 24 params have locks)
Step Flags:                1 byte  (bit 0: trigger on/off, bits 1-7: reserved)
Sub-step bitfield:         1 byte  (8 bits = 8 sub-steps on/off; only meaningful
                                    when SSUB = –1 or –2)
                           ─────
Total per step:           30 bytes
```

### Per-Track Storage

```
8 steps × 30 bytes:           240 bytes (step data)
Pattern settings:               4 bytes (direction, clock div, rotate, length)
Voice defaults:               ~20 bytes (base values for all lockable voice params)
Voice config:                 ~16 bytes (LFO shape, LFO reset, LFO range, LFO tracking,
                                         FG curve, FG loop, LPG decay offset, LPG curve offset,
                                         filter drive, phase sync, smoothing, scale, root,
                                         VDST, VAMT, VDEC)
Shadow playhead state:          5 bytes
Mod relationships (×2 max):     8 bytes (source, dest, type, amount × 2)
                               ─────
Total per track:             ~293 bytes
```

### Motherboard RAM Budget (ATmega644p — 4,096 bytes)

```
6 tracks × 293 bytes:       1,758 bytes
Global settings:                32 bytes (tempo, swing, master reset, hold mode, etc.)
Mod matrix (4 relationships):   16 bytes
UI state:                       64 bytes (current page, selected track, edit state, etc.)
SPI TX/RX buffers:            128 bytes
LCD frame buffer:              80 bytes (2×40 chars)
Stack:                        512 bytes
                             ─────────
Estimated total:            2,590 bytes of 4,096 available
Headroom:                  ~1,506 bytes (~37%)
```

> **Note:** Carcosa firmware (the starting point) measures 93% controller RAM usage (3,828 / 4,096 bytes) with its own data structures. We replace those wholesale with the sequencer structures above. The 38% headroom estimate is based on our own data model, not Carcosa's. Phase 1 must profile actual usage after stripping Carcosa's controller data and substituting ours.
>
> Mitigation strategies if needed:
> - **Reduce max active mod relationships from 2 to 1 per track** (saves ~48 bytes)
> - **Move voice defaults/config to voice card RAM** — voice cards have substantial RAM headroom
> - **Use SD card as swap** — keep only the active pattern in RAM, load others on demand

### Voice Card RAM (ATmega328p — 2,048 bytes)

Voice cards store no pattern data. They only maintain current synthesis state:

```
Oscillator state:            64 bytes (two oscs, phase accumulators, wavetable pointers)
Noise LFSR state:             4 bytes
LFO state:                   16 bytes (phase, rate, shape lookup, target routing)
Filter state:                32 bytes (SVF state variables, coefficients)
Drive/saturation state:       4 bytes
FG-A state (VCA):            16 bytes (phase, rate, curve, inflection)
FG-B state (Filter):        16 bytes (same structure, offset-derived rates)
FG-C state (Velocity FG):   12 bytes (phase, rate, peak level, destination routing)
Pitch envelope state:         8 bytes
Current parameter set:       24 bytes (last received snapshot from motherboard)
Smoothing interpolation:     24 bytes (current interpolated values for continuous params)
SPI RX buffer:               32 bytes
Audio output buffer:         64 bytes
Stack + overhead:           512 bytes
                           ─────────
Estimated total:           ~828 bytes of 2,048 available
Headroom:                 1,220 bytes (~60%)
```

### Voice Card Flash Budget (ATmega328p — 32,768 bytes)

Carcosa measures 92% flash usage (29,806 / 32,256 bytes) with FM4OP + Karplus-Strong + West Coast all present. We strip FM4OP and Karplus-Strong, keeping West Coast as the oscillator base. This frees an estimated 5–8KB before we add our custom code.

```
Carcosa baseline (West Coast only, stripped):  ~22KB (estimated after removing FM4OP + KS)
SVF filter interface + drive:                  ~0.5KB (analog filter; DAC PWM control only)
FG-A + FG-B (dual envelope, shared code):      ~1.5KB (extend existing envelope code)
Pitch envelope:                                ~0.5KB
Noise LFSR:                                    ~0.2KB
LFO (audio-rate + range switching + tracking): ~1.5KB
Bit reduction (BITS):                          ~0.1KB (already in Carcosa as OP_BITS)
Sample rate reduction (CRSH):                  ~0.1KB (already in Carcosa as crush param)
                                               ─────
Estimated total:                              ~26.4KB
Available:                                     ~6.4KB headroom
```

> **Note:** West Coast oscillator baseline size needs profiling in Phase 1 — this is the largest unknown. If it's larger than expected, FM4OP and Karplus-Strong removal still gives ~6KB back to work with. A simplified modal percussion engine (~4–6KB) remains a stretch goal contingent on Phase 1 flash profiling.

---

## SPI Communication Protocol

### Parameter Snapshot (Motherboard → Voice Card)

Every trigger sends a full snapshot. No delta tracking, no state carryover. Locked params use lock values; unlocked params use current knob positions. `MUT8` pitch resolution, sub-step scheduling, and repeat counting are all handled on the motherboard — the voice card receives only the final computed pitch and parameter values.

A new `TRIGGER_WITH_SNAPSHOT` command is added to the SPI protocol (extends the existing Carcosa protocol):

```
Trigger message:
  - Command byte:      1 byte  (TRIGGER_WITH_SNAPSHOT = 0x12)
  - Voice params P1:   8 bytes (ratio, detune, FM, fold, LPG decay, noise, bend level, bend time)
  - Voice params P2:   8 bytes (wav1, wav2, osc2 pitch, vel FG decay, LFO dest, LFO speed, LFO amt, sample rate reduction)
  - Voice config:      9 bytes (LPG depth, drive, bits, LFO shape, LFO range, LFO tracking, VDST, VAMT, VDEC)
  - Velocity:          1 byte  (from step params or default)
  Total:              27 bytes per trigger
```

### Timing Analysis

At SPI clock 2MHz: 27 bytes = 108µs per voice card.
Worst case (6 voices simultaneous trigger): 648µs total.
Fastest possible trigger rate (240 BPM, 8x ratchet): one trigger every ~7.8ms.
**SPI bandwidth is not a constraint.** Even worst-case is <8% of available time.

### Continuous Updates (Live Knob Tweaks)

When unlocked steps are playing and the user turns a knob, the motherboard sends a parameter update to the active voice card between triggers. This reuses the stock Ambika's CC-style parameter update mechanism.

---

## Percussion Recipes

The west coast voice covers the full percussion spectrum without dedicated engines:

| Sound | SHAP | A:B | FM | NOIS | TIMB | TYPE | LPGD | BEND | LFO |
|-------|------|-----|-----|------|------|------|------|------|-----|
| **Kick** | S/S | 1:1 | Low | 0 | Low | LP | Long | Down, fast | Off |
| **Snare** | S/S | 2.3:1 | Mid | CW mid | Mid | BP | Medium | Down, fast | Off |
| **Closed HH** | S/S | 7.1:1 | High | CW mid | High | HP | Very short | None | Off |
| **Open HH** | S/S | 7.1:1 | High | CW mid | High | HP | Medium | None | Off |
| **Tom** | S/S | 1.5:1 | Low | 0 | Low | LP | Medium | Down, slow | Off |
| **Rim** | S/Q | 3:1 | High | 0 | Mid (PW) | BP | Very short | Up, fast | Off |
| **Bell** | S/S | 5:1 | Mid | 0 | Low | BP | Long | None | Off |
| **Clap** | S/S | 1:1 | Low | CW | Mid | BP | Medium | None | Off |
| **Metallic** | T/S | 3.7:1 | High | CCW low | High | BP | Medium | None | Off |
| **Shimmer** | S/S | 2:1 | Mid | 0 | Low | LP | Long | None | Slow→pitch |
| **Grit** | S/S | 1:1 | High | CCW | High | BP | Medium | None | Audio→fold |
| **3-op FM** | S/S | 1.5:1 | Mid | 0 | Low | LP | Long | None | Audio sine→pitch |
| **Driven** | S/S | 1:1 | Mid | 0 | Mid | LP+DRIV | Medium | Down, fast | Off |
| **Parallel** | Q/S | 2:1 | High | 0 | Mid (PW) | HP+BITS | Short | None | Off |

---

## Optional: Second Voice Engine (Modal Percussion)

### Concept

A simplified modal/physical-modeling percussion engine inspired by Mutable Instruments Plaits/Rings. Provides struck-surface sounds: bells, marimbas, wood blocks, membranes. Complements the west coast voice's metallic/FM character with more natural acoustic resonances.

### Feasibility

The Plaits modal voice was designed for STM32F (Cortex-M4), significantly more powerful than the ATmega328p. A reduced version (fewer resonant modes, lower sample rate, simpler exciter) might fit in the ~15KB of available flash. **This is a stretch goal — evaluate after the primary engine is working and profiled.**

### Voice Engine Selection

If implemented, voice engine is selectable per track on the Track Settings page (Button 6). The lockable voice params would have different meanings depending on the engine, but the sequencer doesn't care — it stores bytes per step regardless.

---

## Open Questions for Phase 1 Evaluation

Codebase evaluation of Carcosa has answered many questions. Remaining unknowns for Phase 1:

1. **Filter mode (TYPE) switching glitch:** Switching LP/BP/HP is done via 3 GPIO lines on the voice card PCB, sent per audio block. Does switching mid-note produce an audible click or glitch on the hardware? If yes, `TYPE` must be a voice-level setting, not a per-step lockable parameter.
2. **West Coast oscillator flash size:** Profile the actual flash cost of `westcoast.h` in isolation after stripping FM4OP and Karplus-Strong. This is the largest unknown in the voice card flash budget.
3. **Actual motherboard RAM after controller strip:** Carcosa controller uses 3,828 / 4,096 bytes with its data structures. After replacing those with our sequencer model, profile actual usage. Our 38% headroom estimate needs validation.
4. **Build system:** Toolchain (avr-gcc version, make targets), flash procedure. Can individual voice cards be flashed separately from the motherboard?
5. **SD card storage format:** How are patches stored in Carcosa? Can we extend the format for pattern/sequence data, or must we redesign?
6. **Timer/interrupt structure:** What timers are used for audio rate vs. UI scanning vs. MIDI on the motherboard? Where does the sequencer tick live and at what priority?
7. **Noise mix injection point:** Carcosa's West Coast oscillator has an FM path. Can noise be injected as a crossfadeable FM source within `westcoast.h`, or does it require restructuring the oscillator?
8. **CRSH parameter range:** At 39kHz, what crush divisor values give musically useful sample rate reductions? Map the 0–255 byte range to useful frequency steps (e.g. 39k, 19.5k, 13k, 9.75k, ...) rather than linear divisor.

---

## Development Phases

### Phase 1: Evaluation & Proof of Concept
- Clone repo, build stock firmware, understand architecture
- Answer all open questions above, write findings to EVALUATION.md
- **Critical: profile actual motherboard RAM usage** — 7% headroom estimate needs validation
- Identify the complex oscillator fork (if any) and evaluate
- Implement a minimal FG (decay-only) replacing one envelope on one voice card
- Verify full-snapshot SPI approach works (send 27 bytes per trigger, voice card applies them)

### Phase 2: Voice Engine
- Strip FM4OP and Karplus-Strong from Carcosa voicecard; keep West Coast as oscillator base
- Add noise LFSR and noise mix injection into West Coast FM path
- Extend existing envelope code to dual FG (FG-A for VCA, FG-B for filter with decay/curve offsets)
- Implement FG-C (Velocity FG) — third envelope instance; instant attack, velocity-scaled peak, independent fall (`VDEC`), configurable destination (`VDST`/`VAMT`)
- Implement two-stage decay curve (fast initial + slow tail) for vactrol emulation
- Implement pitch envelope (bend)
- Implement LPG behavior (FG-B → filter cutoff coupling via LPG Depth)
- Implement filter drive (gain + saturation before DAC send)
- Expose `BITS` (bit reduction) as a standalone parameter (refactor from Carcosa's OP_BITS)
- Expose `CRSH` (sample rate reduction) as a standalone parameter (already in Carcosa as crush)
- Implement LFO with audio-rate capability, range switching, and pitch tracking
- Test on a single voice card, verify percussion recipes
- Default the LPG offsets to vactrol-like values and validate the feel

### Phase 3: Sequencer Core
- Implement step sequencer on motherboard (6 independent tracks, 8 steps max)
- Per-track pattern length (1–8) and clock division (polymeters)
- Direction modes (forward, reverse, pendulum, random)
- Pattern rotate (0–7)
- Full parameter snapshot sent via TRIGGER_WITH_SNAPSHOT on every trigger
- Stateless step behavior (locks snap back to defaults)

### Phase 4: Parameter Locks
- Three-page lock system (voice page 1 / voice page 2 / step params)
- Hold-step-to-edit interaction
- Lock clear (double-tap)
- Unlocked steps read live knob positions
- Lock flag display on LCD

### Phase 5: Step Behavior Features
- Probability
- SSUB: bipolar knob — CW = ratchets (1–8), center = normal, CCW –1 = custom sub-step playback, CCW –2 = sub-step edit mode (buttons become sub-step toggles)
- Step repeat (REPT)
- Mutate: MINT (interval select) + MDIR (direction) — pitch interval walk across repeats then sub-steps, sawtooth reset at ceiling/floor
- Per-step rate multiplier (RATE): half / normal / double / triplet
- Velocity/accent (VEL)
- Glide — FG retrigger suppression (GLID)

### Phase 6: UI
- Remap button pages to new parameter layout
- Implement sequencer mode (buttons as steps, encoder page cycle across 3 lock pages)
- Implement all normal-mode pages with abbreviations
- Track Settings page 6a (pattern/playback) and 6b (voice config: PHSE, SMTH, VDST, VAMT, VDEC)
- FG/Modulation page with LFO shape, reset, range, tracking
- VCF page with LPG offsets, drive, bit reduction
- Performance/Master page with voice select, tempo, step hold mode
- LCD layout for all pages — 4-char abbreviations on line 1, full names on line 2 when adjusting

### Phase 7: Dual Playhead & Step Hold Modes
- Shadow playhead implementation
- Voltage Block mode (pause + snap)
- Elektron mode (no pause, edit-in-place)
- Global mode toggle on Performance page

### Phase 8: Track Relationships
- Mod matrix implementation (transpose osc1/osc2, clock, reset, accent)
- Cross-track state evaluation in sequencer tick
- Mod matrix UI (Button 4)
- Limit to 4 active relationships, validate CPU impact

### Phase 9: Storage & Polish
- SD card save/load for patterns and voice settings
- Patch management UI on Load/Save page
- Performance optimizations (especially if RAM is tight)
- MIDI sync (external clock in/out)
- Optional: modal percussion engine evaluation and implementation

---

## References

- **Ambika firmware repo:** `https://github.com/pichenettes/ambika`
- **Ambika manual:** `https://pichenettes.github.io/mutable-instruments-diy-archive/ambika/manual/`
- **Ambika technical notes:** `https://pichenettes.github.io/mutable-instruments-diy-archive/ambika/technotes/`
- **Plaits source (modal voice):** `https://github.com/pichenettes/eurorack` (plaits/ directory)
- **Michigan Synth Works Xena (SVF version):** Target hardware (Ambika clone with SVF voicecards)
- **Fors Dyad / Para:** Design reference for constrained playability, voice parameter set, inter-voice FM, phase sync, modulation smoothing
- **Catalyst firmware fork:** `https://github.com/voltagecontrolled/catalyst-firmware` — reference for ratchets, step repeat, parameter lock UX patterns
