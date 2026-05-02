# Ambika Percussive Synth — Firmware Spec (Elkhart, YAM-based)

## Project Overview

Fork of YAM (`bjoeri/ambika`, "Yet Another Mutation") to create a **6-voice polymetric percussive synthesizer** with per-step parameter locks, an LPG-coupled amp+filter envelope plus an independent pitch envelope, an FM-capable dual oscillator, and a transient layer. The design philosophy prioritizes **constrained playability over feature depth** — inspired by instruments like Fors Dyad, where limited but well-chosen parameters create a more engaging instrument than exhaustive control.

A prior iteration of this spec was Carcosa-based and committed to a West Coast complex oscillator. After hardware listening, that direction was abandoned in favor of YAM's broader oscillator palette plus a future wavefolder addition. The sequencer, parameter-lock system, polymetric track relationships, transport, and envelope architecture are the original contribution; the voice engine is YAM with targeted modifications.

### Hardware Platform

- **Motherboard:** ATmega644p (64KB flash, 4KB RAM, 20MHz)
  - Handles UI, sequencer, MIDI, SD card storage, SPI bus master
- **Voice cards (×6):** ATmega328p (32KB flash, 2KB RAM each)
  - Each runs its own oscillator → filter → envelope signal chain independently
  - Connected to motherboard via SPI
- **Display:** 2×40 character LCD
- **Controls:** Push encoder (clickable), 8 potentiometers (4 above LCD, 4 below), 8 buttons with LEDs
- **Audio:** Per-voice individual outputs + mixed output (808-style normalling: plugging an individual out removes that voice from the mix)
- **Filter type:** SVF (2-pole multimode state variable filter) on all voice cards
- **Storage:** SD card (FAT16/FAT32, 8.3 filenames)

### Design Principles

1. **Constraint breeds playability.** Every parameter must earn its panel/step presence. No deep menus, no hidden features.
2. **The instrument is the interaction between voices.** Polymetric relationships, track-to-track modulation, and the tension between locked and unlocked steps are the primary expressive dimensions.
3. **No dedicated voice types for specific drums.** YAM's oscillator algorithms + the SVF + the sub-osc transient layer cover kicks through hats without wasting flash on specialized engines.
4. **Every step is stateless.** Locked parameters snap back to voice defaults after the step fires. No state leaks between steps. What you see (lock indicator) is exactly what you get on that step and only that step.

---

## Voice Architecture

Each voice card runs the same signal chain. All timbral variety comes from parameter differences plus algorithm selection per oscillator.

### Signal Chain

```
                        ┌────────────────────┐
                        │  Sub-Osc /         │
                        │  Transient Layer   │
                        │  (11 shapes)       │
                        └──────────┬─────────┘
                                   │ SUB level
                                   ▼
┌─────────┐                  ┌──────────┐    ┌─────────┐    ┌──────────┐
│  Osc 2  │ FM (BLND ≥ 64) │  Osc 1   │    │  Mixer  │    │   SVF    │
│ (mod /  │───────────────▶│(carrier) │───▶│ ±BLND   │───▶│ LP/BP/HP │──▶ Env1 (VCA) ──▶ OUT
│  mix)   │                 └──────────┘    │ + SUB   │    │ + DRIV   │
└─────────┘                                  │ + NOIS  │    └────┬─────┘
                                             └─────────┘         ▲
                                                                 │
                                                            Env2 (Filter)
                                                            LPGA-scaled
                                                            (LPGD × LPGO)

           Env3 (Pitch) ──▶ Osc 1 base pitch (PITA-scaled, PITD time)
           LFO          ──▶ voice-config target (non-lockable in v1)
```

### Dual Oscillator (Osc 1 = carrier, Osc 2 = modulator)

Two oscillator slots, each independently selecting from YAM's wave shape palette. The carrier (Osc 1) determines perceived pitch; the modulator (Osc 2) is either mixed into the audio path or routes its output to FM-modulate Osc 1, controlled by a single bipolar `BLND` knob.

- **`NOTE` (Base Note):** The carrier's pitch in semitones. Drives perceived pitch.
- **`WAVE` (per oscillator):** Selects the algorithm/wave shape from YAM's palette.
- **`PARA` (per oscillator):** Algorithm-specific parameter. Meaning depends on selected wave: PWM amount for square, formant select for vowel, FM index for `fm`, fold depth for the future wavefolder shape, etc. The LCD second line shows the algorithm-specific name when actively editing.
- **`RTIO` (Osc 2 ratio):** Frequency ratio of Osc 2 relative to Osc 1's pitch. Drawn from `lut_res_fm_frequency_ratios` (DX-style: 0.125, 0.25, 0.5, 0.707, π/4, 1.0, √2, π/2, 7/4, 2, 9/4, 11/4, 2√2, 3, π, 4, 5, 8, …).
- **`FINE` (Osc 2 cents):** Fine pitch offset on the modulator (±100 cents). Slight non-integer detune produces FM beating/shimmer without altering perceived pitch.
- **`BLND` (Oscillator Interaction):** Bipolar single-byte control:
  - `0`–`63`: Crossfade between Osc 1 and Osc 2 in the audio path (`0` = Osc 1 only, `63` = Osc 2 only). No FM.
  - `64`–`127`: Osc 2 modulates Osc 1 (Osc 2 leaves the audio path). FM depth scales from `0` at `64` to maximum at `127`.
  - The `63→64` transition is acoustically silent (both endpoints = "Osc 1 only, no FM"). LCD second line shows "Mix" below 64, "FM Depth" at 64+.

### Wave Shape Palette (Osc 1 / Osc 2, post-strip)

The full YAM palette is defined in `common/patch.h`. Phase 2 (Weight Reduction) strips the CZ phase-distortion filter-sim variants (9 shapes) to free flash for a future wavefolder-bearing shape. Working set:

- `none` (silence — useful for "Osc 2 silent" without setting BLND to `0`)
- `polyblep_saw`, `polyblep_pwm`, `polyblep_csaw` — modern band-limited classics
- `triangle`, `sine` — fundamentals
- `cz_saw` — kept (no filter-sim suffix)
- `quad_saw_pad`, `quad_pwm` — chorused/detuned multi-osc wavetables
- `fm`, `fm_fb` — internal 2-op FM and FM-with-feedback
- `dirty_pwm` — gritty PWM
- `filtered_noise` — noise through resonator
- `vowel`, `vowel_2` — formant synthesis (`vowel_2` candidate for cut later)
- `wavetable_1`–`wavetable_16` — 16 wavetable banks
- `wavequence` — wavetable sequencing
- `old_saw` — the original (pre-PolyBLEP) saw, preserved

**Cut in Phase 2:** `cz_saw_lp/pk/bp/hp`, `cz_pls_lp/pk/bp/hp`, `cz_tri_lp` — 9 shapes total. Phase distortion filter sims that the user dislikes; their flash is reclaimed.

### Sub-Oscillator (and Transient Layer)

YAM's sub-oscillator is repurposed as a dual-function layer providing both traditional sub-bass reinforcement *and* transient hits. The 11-shape palette (`WAVEFORM_SUB_OSC_*` in `common/patch.h`):

- **Traditional sub:** `Square 1`, `Square 2`, `Triangle 1`, `Triangle 2`, `Pulse 1`, `Pulse 2`
- **Transient:** `Click`, `Glitch`, `Blow`, `Metallic`, `Pop` (rendered by `voicecard/transient_generator.h`, triggered on note-on, fixed-shape one-shots)

Two parameters expose this layer (both per-step lockable on Voice Page 2):

- **`WAVE` (sub):** Selects the sub-osc shape. Switching between traditional and transient shapes per step is a primary percussion-expressiveness tool.
- **`SUB`:** Sub-osc level mixed into the main signal path.

### Envelopes — LPG-Coupled Amp+Filter, Independent Pitch

Three envelope instances internally, but **Env1 (VCA) and Env2 (Filter) are coupled through an LPG macro**. Env3 (Pitch) is fully independent. Re-introduces the LPG concept dropped from earlier YAM-spec drafts, motivated by the realization that lockable filter cutoff is unnecessary if the LPG mechanic provides per-step filter variation through depth and decay-personality controls.

| Env | Fixed Routing | Driven By |
|-----|---------------|-----------|
| `Env1` (VCA) | Output amplitude | `LPGD` (anchor decay) |
| `Env2` (Filter) | SVF cutoff modulation | `LPGD` × `LPGO` (offset from Env1), depth = `LPGA` |
| `Env3` (Pitch) | Osc 1 base pitch | `PITD` (decay), `PITA` (amount, bipolar) |

The three lockable LPG controls (Voice Page 2 top row, slots 1–3):

- **`LPGD` (LPG Decay):** Shared anchor decay time. Drives Env1 directly; drives Env2 with `LPGO`-derived offset. Short = pluck. Long = pad-like.
- **`LPGA` (LPG Amount):** How strongly Env2 modulates the filter cutoff. At `0`, the filter is static at the voice-config `FREQ` (env has no effect). At max, full LPG sweep. The Env1 amplitude path is unaffected by `LPGA` — it's purely the filter-side scaling.
- **`LPGO` (LPG Offset):** Bipolar macro defining the personality of the LPG spread.
  - **Center:** typical vactrol-like — filter decays slightly faster than amp.
  - **CCW:** more plucky — filter closes much faster than amp; highs vanish well before body fades.
  - **CW:** looser/resonant tail — filter holds longer than amp.
  - At extremes, `LPGO` may also shift envelope curve and dial in a small amount of filter Q to enhance the personality (Phase 8 implementation detail).

VCA env amount is implicit (always-on: amplitude env always shapes output level). Filter env amount is `LPGA`. There is no separate lockable `VCAD` or `FLTD` — both decays are derived from `LPGD` plus `LPGO`'s offset relationship.

In YAM stock, the envelopes are numbered with VCA on Env3. Phase 8 (Voice Engine Refinement) renumbers so VCA = Env1 — which matches the user mental model and the LPG macro's structure (Env1 is the anchor, Env2 is offset from it).

### Filter Section

The SVF provides simultaneous LP/BP/HP outputs; mode selects which output passes through.

- **`FREQ` (Cutoff):** Base filter frequency. Voice-config. The LPG sweeps relative to this value; per-step variation comes from `LPGA` and `LPGO`, not from per-step `FREQ`.
- **`RES` (Resonance):** SVF resonance. Voice-config. May receive a small offset from `LPGO` at extreme settings (Phase 8 implementation detail).
- **`TYPE` (Mode):** LP/BP/HP. Voice-config (per Phase 1 open question — switching this mid-note may glitch).
- **`DRIV` (Drive):** Voice-config. Gain stage before SVF — adds saturation/grit at high levels.
- **`BITS` (Bit Reduction):** Voice-config (TBD: candidate for lockable Page 3 if added later). Bit depth reduction pre-DAC.

The filter envelope's amount and timing are controlled per step via `LPGA` and `LPGO` on Voice Page 2 — there is no separate `FLTA` knob.

### LFO

A single LFO per voice. **Audio-rate operation is dropped** in v1 — YAM's signal path is too lo-fi to gain-stage cleanly under audio-rate modulation. LFO settings are voice-config only:

- `LSHP`: shape (sine, triangle, square, ramp, random/S&H)
- `LFOD`: destination (pitch, cutoff, fold, FM depth)
- `LFOS`: speed
- `LFOA`: amount
- `LFOR`: reset behavior (free-run / per-step / per-pattern)
- `TRAK`: pitch tracking (on/off or amount)

If empirical playing reveals LFO demand at the per-step level, `LFOD/LFOS/LFOA` are the natural candidates for elevation to a third lockable page later.

### Noise

Independent noise generator (LFSR), mixed into the main signal path. Per-step lockable amount via `NOIS`.

### Pitch Envelope

Env3 modulates Osc 1's base pitch. `PITA` is bipolar: positive sweeps pitch upward at trigger, decaying toward base; negative sweeps below and rises to base. `PITD` is the decay time. Replaces the prior `BEND`/`TIME` parameters.

---

## UI Architecture

### Navigation Model

Two modes of operation, switched via Button 5:

```
NORMAL MODE (default):
  Buttons 1–8     = page select (quick-jump to first param of that page)
                    Exception: on the Voice/Parts page (Page 6), buttons act as
                    transport (PLAY/PAUS/RST) instead of page-select. To reach
                    other pages from this page, use the encoder.
  Knobs           = edit the 8 parameters currently displayed on the LCD
  Encoder turn    = scroll continuously through ALL parameters on ALL pages
                    (each page is exactly 8 params; encoder flows between pages
                     without stopping at page boundaries)
  Encoder push    = enter "focused edit" mode on the highlighted parameter
                    (full-name and value displayed prominently; encoder turn
                     directly adjusts the value; push again exits)

SEQUENCER MODE (enter via Button 5):
  Buttons 1–8     = step triggers
  Knobs           = lockable parameters (three pages cycled by encoder turn)
  Encoder turn    = cycle Voice Page 1 / Voice Page 2 / Step Page
  Encoder push    = enter focused edit on highlighted param (same semantics)
  Button 5        = exit sequencer mode (symmetric — toggles mode)
```

### LCD Abbreviation Convention

The 2×40 LCD shows 4-character abbreviations per parameter in the overview (8 params across the display). The full parameter name appears on the second line when actively adjusting with the encoder. On pages where buttons act contextually (Master), the bottom row of the LCD shows pipe-separated button labels instead of bottom-knob parameter names.

---

## Normal Mode — Button Assignments

| Button | Hardware Label | Function |
|--------|----------------|----------|
| 1 | OSC / Mixer | **Oscillator config** — non-lockable oscillator voice settings (FM type, portamento, phase reset, tuning offsets) |
| 2 | VCF | **Filter config** — cutoff, resonance, mode, drive, bit reduction, LPG range adjustments |
| 3 | ENV / LFO | **Envelope + LFO config** — attack times, curves, and loop mode for Env1/2/3; LFO settings |
| 4 | MOD MATRIX | **Track Relationships** |
| 5 | KeyBed/Arp/Seq | **Toggle Sequencer Mode** (symmetric) |
| 6 | Voice / Parts | **Voice + Transport** — voice select, track pattern settings, BPM, swing, hold mode; transport buttons (PLAY/PAUS/RST while on this page) |
| 7 | Perf | **Performance** — sequencer tricks, beat repeat, mute/solo (placeholder) |
| 8 | Load / Save | **Load / Save** — state snapshot save/load, global settings, firmware update |

Normal Mode pages 1–3 expose **non-lockable** voice config settings. The lockable parameters (NOTE, WAVE, PARA, BLND, RTIO, FINE, LPGD, LPGA, LPGO, NOIS, PITD, PITA, SUB, WAVE_sub) are accessible only in Sequencer Mode — voice defaults for these are the current knob positions in Sequencer Mode when no step is held.

Knob mappings on Pages 1, 2, 3, 6, 7, and 8 are placeholder pending empirical tuning during Phase 7 (UI). The lockable params and the transport state machine are firm.

### Page 6 — Voice / Parts + Transport (placeholder layout)

The voice configuration and transport page. Top knobs configure global timing and per-track pattern settings for the selected voice; the bottom button row carries transport controls while on this page.

- **Top knobs (placeholder):** `VSEL` (Voice Select, 1–6) | `BPM` | `SWNG` (Swing) | `LTCK` (Hold Mode: Voltage Block / Elektron)
- **Bottom knobs (placeholder):** `CDIV` | `LENG` | `DIRN` | `ROTA` — primary track pattern settings. `SCAL` / `ROOT` / `BPCH` / `OLEV` accessible via encoder from this page; exact placement TBD.
- **Bottom row = transport glyphs** (pipe-separated on LCD), centered on Buttons 4-5-6:
  - `      |▶  |‖  |⏮  |     ` — symbols rendered via CGRAM custom characters (slots 0–2):

```c
// CGRAM slot 0 — Play ▶
{0x00, 0x10, 0x18, 0x1C, 0x1C, 0x18, 0x10, 0x00}
// . . . . .
// # . . . .
// # # . . .
// # # # . .
// # # # . .
// # # . . .
// # . . . .
// . . . . .

// CGRAM slot 1 — Pause ‖
{0x00, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x1B, 0x00}
// . . . . .
// # # . # #
// # # . # #
// # # . # #
// # # . # #
// # # . # #
// # # . # #
// . . . . .

// CGRAM slot 2 — Reset |◀  (vertical bar + gap + left-pointing triangle)
{0x00, 0x11, 0x13, 0x17, 0x17, 0x13, 0x11, 0x00}
// . . . . .
// # . . . #
// # . . # #
// # . # # #
// # . # # #
// # . . # #
// # . . . #
// . . . . .
```

  - Uses 3 of 8 available CGRAM slots; 5 slots remain for lock indicators, waveform thumbnails, etc.
  - Transport buttons override the central button-press meanings *while on this page only*.
- **Transport buttons are modal: only active on Page 6.** Other pages have no transport access; navigation to other pages uses the encoder turn or pressing a different page button.
- **LED feedback:**
  - PLAY LED: blinks during playback, off when stopped or paused.
  - PAUS LED: blinks when paused, off otherwise.
  - RST LED: momentary flash on press.

#### Transport State Machine

| From | Press PLAY | Press PAUS | Press RST |
|------|-----------|-----------|-----------|
| Stopped | Playing | (no-op) | (no-op) |
| Playing | (no-op) | Paused | Playing, all playheads → step 1 |
| Paused | Playing (resume from current position) | Playing (resume) | Paused, all playheads → step 1 |

A dedicated **Stop** button (full halt of all voices, distinct from Pause) is reserved for a future revision once empirical use confirms it's needed.

### Page 7 — Perf (placeholder)

Performance and sequencer tricks. Placeholder pending empirical use. Candidate features:

- **Beat repeat:** Loop a sub-section of the active pattern for fills or stutter effects.
- **Mute / solo:** Toggle individual track output without stopping the sequencer.
- **Stutter / gate reduction:** Real-time gate length shortening for chop effects.

Knob assignments and final feature set TBD in Phase 10.

---

## Sequencer Mode

Entered by pressing Button 5. Exited by pressing Button 5 again.

### Controls in Sequencer Mode

- **Buttons 1–8:** Step triggers. Tap = toggle on/off. Hold = enter parameter lock edit for that step.
- **Knobs 1–8:** Lockable parameters. Three pages, cycled by encoder turn.
- **Encoder turn:** Cycle Voice Page 1 / Voice Page 2 / Step Page.
- **Encoder push:** Focused edit on highlighted param.

### Sequencer Voice Page 1 — Oscillators (Lockable)

The primary timbre page — algorithm select, pitch relationship, interaction.

| Knob | Abbrev | Parameter | Notes |
|------|--------|-----------|-------|
| Top 1 | `NOTE` | Base Note | Carrier (Osc 1) pitch in semitones. Drives perceived pitch. |
| Top 2 | `WAVE` | Osc 1 Wave Shape | Algorithm select for the carrier |
| Top 3 | `PARA` | Osc 1 Parameter | Algorithm-specific (PWM / formant / FM index / fold depth / etc.) |
| Top 4 | `BLND` | Osc Interaction | Bipolar: 0–63 mix Osc1↔Osc2, 64+ Osc2 FMs Osc1 |
| Bot 1 | `RTIO` | Osc 2 Ratio | Frequency ratio Osc2:Osc1 (lut_res_fm_frequency_ratios) |
| Bot 2 | `WAVE` | Osc 2 Wave Shape | Algorithm select for the modulator |
| Bot 3 | `PARA` | Osc 2 Parameter | Algorithm-specific |
| Bot 4 | `FINE` | Osc 2 Fine Tune | ±100 cents detune. Beating/shimmer source. |

### Sequencer Voice Page 2 — Modulation (Lockable)

LPG envelope macro, pitch envelope, noise, sub-osc.

| Knob | Abbrev | Parameter | Notes |
|------|--------|-----------|-------|
| Top 1 | `LPGD` | LPG Decay | Anchor decay time. Drives Env1 directly; drives Env2 with `LPGO`-derived offset. |
| Top 2 | `LPGA` | LPG Amount | Filter env depth. 0 = filter static at FREQ. |
| Top 3 | `LPGO` | LPG Offset | Bipolar personality macro: CCW = plucky (filter much faster than amp), center = vactrol-like, CW = looser/resonant tail. |
| Top 4 | `NOIS` | Noise Amount | White noise mixed into main path |
| Bot 1 | `PITD` | Pitch Env Decay | Env3 fall time |
| Bot 2 | `PITA` | Pitch Env Amount | Env3 → carrier pitch (bipolar) |
| Bot 3 | `WAVE` | Sub-Osc Wave Shape | 11-shape palette: 6 traditional + 5 transient |
| Bot 4 | `SUB ` | Sub-Osc Level | Mix amount of sub layer |

### Sequencer Step Page — Behavior (Lockable)

Step-level firing, timing, and pitch-walk parameters. Resolved on the motherboard; the voice card receives only the final pitch and parameter snapshot.

| Knob | Abbrev | Parameter | Notes |
|------|--------|-----------|-------|
| Top 1 | `PROB` | Probability | 0–100%. Step fires or is skipped. |
| Top 2 | `SSUB` | Sub-steps / Ratchets | Bipolar. CW (+1..+8) = ratchets. Center = normal. CCW –1 = Custom (plays sub-step bitfield). CCW –2 = Edit (buttons become sub-step toggles). |
| Top 3 | `REPT` | Step Repeat | Times step replays before advancing |
| Top 4 | `RATE` | Step Rate | Per-step timing multiplier: half / normal / double / triplet |
| Bot 1 | `VEL ` | Velocity | Amplitude emphasis |
| Bot 2 | `GLID` | Glide | Tie to next step — affects envelope retrigger |
| Bot 3 | `MINT` | Mutate Interval | Off, Scale, Min3, Min5, Min7, Min9, Maj3, Maj5, Maj7, Maj9, Oct |
| Bot 4 | `MDIR` | Mutate Direction | Up / Down / UpDn / Rnd. Active when MINT ≠ Off. |

### Step Interaction Model

- **Tap a step button:** Toggle step on/off.
- **Hold a step button:** Enter parameter lock edit. LCD shows current lock values for whichever page is active. Turn any knob to write a lock for that parameter on that step.
- **Lock clear:** Double-tap a held step to clear all locks for that step back to defaults.
- **Ratchets:** While holding a step, turn `SSUB` clockwise (+1..+8) to set a ratchet count.
- **Custom sub-step playback:** Turn `SSUB` one click CCW (value –1 = Custom). Plays back the stored sub-step bitfield without entering edit mode.
- **Sub-step edit:** Turn `SSUB` two clicks CCW (value –2 = Edit). Buttons 1–8 become sub-step toggles. Release the parent step button; sub-steps remain editable. Tap any button to toggle. Moving `SSUB` back to –1 exits edit mode.

### Stateless Step Behavior

**Locked parameters snap back to the voice default value after the step.** A locked step does not affect subsequent unlocked steps. Every trigger sends a full parameter snapshot to the voice card — either the step's locks or the voice defaults (current knob positions for unlocked params).

### Mutate (`MINT` + `MDIR`)

Pitch-walk feature applied across a step's repeats and sub-steps. Resolved entirely on the motherboard; voice card receives only the final computed `NOTE` in the trigger snapshot.

- `MINT`: 11 values (Off, Scale, Min3, Min5, Min7, Min9, Maj3, Maj5, Maj7, Maj9, Oct). Off disables.
- `MDIR`: 4 values (Up, Down, UpDn alternating, Rnd). Active when MINT ≠ Off.
- `Scale` interval respects the track's `SCAL` and `ROOT`; other intervals are chromatic.
- Walk hits ceiling/floor → sawtooth reset to base.
- When both `REPT` and `SSUB` are active: repeats mutate first; sub-steps within a repeat walk from that repeat's pitch.

### Unlocked Steps Read Live Knobs

Unlocked steps read current physical knob positions in real time. Sweeping a knob during playback affects all unlocked steps immediately; locked steps are pinned and unaffected. This tension between locked and unlocked steps is the primary performance dimension.

### Pattern Settings (per track, Voice/Parts page — Button 6)

| Parameter | Abbrev | Range |
|-----------|--------|-------|
| Direction | `DIRN` | Fwd / Rev / Pendulum / Random |
| Clock Div | `CDIV` | 1, 2, 3, 4, 6, 8, 12, 16 — the polymeter engine |
| Rotate | `ROTA` | 0–7 |
| Length | `LENG` | 1–8 |
| Scale | `SCAL` | Scale quantize for Mutate's Scale interval |
| Root | `ROOT` | Root note |
| Base Pitch | `BPCH` | Base pitch for unlocked NOTE / starting point for Mutate |
| Output Level | `OLEV` | Track output level |

### Step Hold Modes (LTCK on Voice/Parts page — Button 6)

Configurable on Voice/Parts page (Button 6), top slot 4 (LTCK). Global setting affecting all tracks.

#### Voltage Block Mode (Snap)

- Hold step → playhead **pauses** on that step. Held step fires repeatedly so edits are heard.
- A **shadow playhead** continues advancing silently, maintaining timing.
- Release step → playhead **snaps to shadow position**. No timing disruption.

#### Elektron Mode (No Pause)

- Hold step → playhead **keeps running**. Pattern doesn't pause.
- Knob movements write to the held step's lock storage.
- Heard the next time the step comes around.

#### Shadow Playhead Implementation

Runs in both modes (5 bytes per track × 6 tracks = 30 bytes total). In Elektron mode it's not read on release.

---

## Track Relationship Matrix

Defines how tracks interact. Maximum 4 active relationships globally (CPU and SPI bound).

| Type | Description |
|------|-------------|
| Transpose Osc 1 | Source track's pitch transposes destination's Osc 1 (carrier) |
| Transpose Osc 2 | Same, modulator only — detuning across tracks |
| Clock | Source's triggers clock destination |
| Reset | Source's step 1 resets destination to step 1 |
| Accent | Simultaneous-fire boosts destination's velocity |

---

## Memory Architecture

### Per-Step Storage (on Motherboard)

```
Voice Page 1 (oscillators):  8 bytes
Voice Page 2 (modulation):   8 bytes
Step Page (behavior):        8 bytes
  └─ SSUB byte encodes: signed (–2=Edit, –1=Custom, 0=Normal, +1..+8=Ratchets)
Lock flags:                  3 bytes (bitfield: 24 lockable params)
Step flags:                  1 byte  (bit 0: trigger on/off, 1–7 reserved)
Sub-step bitfield:           1 byte  (8 bits = 8 sub-steps; meaningful when SSUB ∈ {–1, –2})
                            ─────
Total per step:             29 bytes
```

### Per-Track Storage

```
8 steps × 29 bytes:          232 bytes
Pattern settings:              8 bytes (DIRN, CDIV, ROTA, LENG, SCAL, ROOT, BPCH, OLEV)
Voice defaults:              ~24 bytes (one byte per lockable, current knob values)
Voice config:                ~20 bytes (LFO config, env amounts/curves, filter cutoff/res/mode/drive/bits, PHSE, SMTH, etc.)
Shadow playhead state:         5 bytes
Mod relationship slots (×2):   8 bytes (source, dest, type, amount × 2)
                             ─────
Total per track:           ~297 bytes
```

### Motherboard RAM Budget (ATmega644p — 4,096 bytes)

```
6 tracks × 297 bytes:       1,782 bytes
Global settings:               32 bytes (tempo, swing, master reset, hold mode, etc.)
Mod matrix (4 relationships): 16 bytes
UI state:                      64 bytes
SPI TX/RX buffers:            128 bytes
LCD frame buffer:              80 bytes
Stack:                        512 bytes
                             ─────
Estimated total:           2,614 bytes of 4,096 available (~64% used, ~36% headroom)
```

> **Phase 1 must profile actual RAM after stripping YAM's stock controller — Multi/Patch/Program/Part hierarchy + voice parts UI + arpeggiator + sequencer.** YAM's stock controller currently consumes most of available RAM; that reclaim is the prerequisite for the sequencer fitting.

### Voice Card RAM (ATmega328p — 2,048 bytes)

Voice cards store no pattern data. Only synthesis state.

```
Oscillator state (Osc1+Osc2):   ~64 bytes
Sub-osc / transient state:        8 bytes
Noise LFSR state:                 4 bytes
LFO state:                       16 bytes
Filter state:                    32 bytes
Drive/saturation state:           4 bytes
Env1 (VCA), Env2 (Filter), Env3 (Pitch): ~48 bytes
Current parameter snapshot:      24 bytes
Smoothing interpolation:         24 bytes
SPI RX buffer:                   32 bytes
Audio output buffer:             64 bytes
Stack + overhead:               512 bytes
                                ─────
Estimated total:               ~832 bytes of 2,048 available (~60% headroom)
```

### Voice Card Flash Budget (ATmega328p — 32,768 bytes)

YAM voicecard built under the Squeeze toolchain measures **26,144 bytes** with the full algorithm zoo + sequencer + arpeggiator. Stripping and adding:

```
Stock YAM voicecard:                                     26,144 bytes
- Strip CZ filter-sim variants (×9):                        est. -1,500 bytes
- Strip wavequence (if not used):                           est.   -300 bytes
- Strip vowel_2 (if dropped):                               est.   -150 bytes
- Add wavefolder waveform (Phase 8):                        est.   +800 bytes
- Refactor envelopes (3 instances + LPG macro coupling):    est.   +400 bytes
- Add NOTE byte to SPI snapshot path:                       est.   +100 bytes
- Add transient layer routing (already infrastructure):     est.    +50 bytes
                                                           ────────
Estimated total:                                         ~25,444 bytes
Available headroom:                                       ~7,300 bytes
```

These are coarse estimates pending Phase 1 profiling. The key signal: starting from YAM (26K) plus targeted strips/additions stays well under 32K, unlike the prior elkhart Carcosa-based snapshot which approached the flash ceiling.

---

## SPI Communication Protocol

### Parameter Snapshot (Motherboard → Voice Card)

Every trigger sends a full snapshot. No delta tracking, no state carryover. Locked params use lock values; unlocked params use current knob positions. Mutate, sub-step scheduling, repeat counting, and pitch resolution all happen on the motherboard.

A new `TRIGGER_WITH_SNAPSHOT` command is added to the SPI protocol (extends YAM's existing protocol):

```
Trigger message:
  - Command byte:        1 byte  (TRIGGER_WITH_SNAPSHOT = 0x12)
  - Note (final pitch):  1 byte  (resolved by motherboard incl. mutate + track transpose)
  - Voice Page 1 params: 8 bytes (NOTE_default, WAVE1, PARA1, BLND, RTIO, WAVE2, PARA2, FINE)
  - Voice Page 2 params: 8 bytes (LPGD, LPGA, LPGO, NOIS, PITD, PITA, WAVE_sub, SUB)
  - Voice config bytes: ~10 bytes (filter cutoff/res/mode, drive, BITS, LFO config, env curves if dynamic, ...)
  - Velocity:            1 byte
  Total:                ~29 bytes per trigger
```

> **On the duplicate `NOTE` slot.** Voice Page 1 Top 1 has `NOTE` as a per-step lockable input value; the resolved final pitch (after Mutate, after track transposition relationships) is sent as a separate top-of-message byte. The voice card uses the resolved byte; the lockable `NOTE` value is the input to the resolver, not the final pitch.

### Timing Analysis

At SPI clock 2 MHz: 29 bytes ≈ 116 µs per voice card.
Worst case (6 voices simultaneous trigger): ≈ 696 µs total.
Fastest possible trigger rate (240 BPM, 8× ratchet): one trigger every ≈ 7.8 ms.
**SPI bandwidth is not a constraint.** Worst case <9% of available time.

### Continuous Updates (Live Knob Tweaks)

When unlocked steps are playing and the user turns a knob, the motherboard sends a parameter update to the active voice card between triggers. Reuses the existing CC-style parameter update mechanism in YAM.

### Pre-Phase 8 Compatibility Mode

Through Phases 3–7, voicecards still run **stock YAM** with only the Phase 2 strip applied. The snapshot format above is the **target**; during sequencer development the controller maps our parameter names to YAM's internal voice struct fields where they don't yet match natively. Phase 8 (Voice Engine Refinement) closes that gap by reshaping the voicecard side to understand the parameter set directly.

---

## Storage

### State Snapshot Model

The Mutable/Ambika hierarchy of Multi → Patch → Program → Part is **abandoned**. A single "state snapshot" persists the entire instrument at once: all 6 tracks of pattern + locks + voice defaults + voice config + mod matrix + global settings. One file = one complete instrument state.

```
Estimated snapshot size:
  6 tracks × 297 bytes:      1,782 bytes
  Global settings:              32 bytes
  Mod matrix (4 slots):         16 bytes
  Header + version + checksum: ~16 bytes
                               ─────
  Total:                     ~1,846 bytes
```

This easily fits in a single SD card file with room for forward-compatibility metadata.

### v1: Single Slot

One save action, one load action on Page 8. The currently-loaded state is in motherboard RAM; pressing Save writes it to a fixed file (e.g., `STATE.BIN`); pressing Load reads it back. No slot picker, no patch browser.

### Future: Multiple Slots

Phase 10 expands this with a slot picker. Each slot is a separate file (`STATE01.BIN`, `STATE02.BIN`, …). UI on Page 8 lets the user select source/destination slot before save/load. Stretch goal: simple naming.

---

## Open Questions for Phase 1 Evaluation

1. **Filter mode (TYPE) switching glitch:** Switching LP/BP/HP via 3 GPIO lines per audio block — does mid-note switching produce an audible click? If yes, `TYPE` stays voice-config; if not, candidate for elevation to a third lockable page.
2. **YAM oscillator strip flash savings:** Profile actual flash cost of each algorithm in isolation to confirm strip estimates and decide what else can go.
3. **Motherboard RAM after Weight Reduction:** YAM's stock controller currently fills most of RAM (Multi/Patch/Program/Part state, voice parts UI, arpeggiator, sequencer). Profile actual usage after the strip.
4. **YAM envelope code reusability:** Can three envelope instances (Env1/VCA + Env2/Filter coupled via LPG macro, Env3/Pitch independent) be derived from YAM's envelope code, or does it need fresh implementation? Includes the macro logic that maps `LPGD`/`LPGO` onto Env1 and Env2 decay rates plus optional curve and Q dialing at `LPGO` extremes.
5. **YAM LFO capabilities:** Range, shape, destination set. Verify what's already supported before re-implementing.
6. **YAM noise generator:** Is there a separate LFSR available, or is noise only embedded in specific oscillator algorithms (`filtered_noise`)?
7. **YAM SPI protocol extensibility:** Confirm `TRIGGER_WITH_SNAPSHOT` adds cleanly.
8. **Wavefolder feasibility:** Can a Carcosa-style iterative-quadratic wavefolder be added as a wave shape that fits the budget, sounds good, and uses PARA as fold depth?
9. **CRSH (sample rate reduction) keep/drop:** YAM has the algorithm; does it have a place in this design or is it cut?
10. **Sub-osc duplicate strip:** Are duplicate octave variants (Square 1/2, Triangle 1/2, Pulse 1/2) all needed, or can some be dropped?
11. **SD card storage format:** What primitives does YAM provide? Trivially extendable for the single-state-snapshot file?
12. **Timer/interrupt structure:** What timers are used for audio rate vs. UI scanning vs. MIDI on the motherboard? Where does the sequencer tick live and at what priority? Is shadow playhead implementable without contention?

---

## Development Phases

### Phase 1: Evaluation & Proof of Concept
- **Build pipeline ✓** — Squeeze Docker toolchain hardware-verified 2026-04-30.
- Profile YAM voicecard flash cost per algorithm.
- Profile YAM controller RAM usage.
- Sketch SPI snapshot PoC: motherboard sends a fixed 29-byte test message, voicecard receives and applies.
- Document findings in `EVALUATION.md`.

### Phase 2: Weight Reduction
**Subtractive phase. Goal: get controller and voicecard down to a stable baseline that frees space for the sequencer.**

Controller-side strips:
- YAM's existing sequencer code
- YAM's arpeggiator code
- Voice/Parts configuration page (Button 6 in stock)
- Multi/Patch/Program/Part hierarchy and all associated state
- Keyboard range/split/transpose/per-part MIDI channel routing
- Patch storage UIs and chrome
- Performance modes / chord memory / anything else not serving the percussion-sequencer concept

Voicecard-side strips:
- CZ filter-sim variants: `cz_saw_lp/pk/bp/hp`, `cz_pls_lp/pk/bp/hp`, `cz_tri_lp` (9 shapes)
- Other algorithms confirmed unwanted (TBD pending Phase 1 listening: `vowel_2`, `wavequence`, others)

Voicecard-side additions:
- `TRIGGER_WITH_SNAPSHOT` SPI handler that accepts our snapshot format; initially translates to YAM's internal voice struct.

Storage scaffolding:
- Single state snapshot save/load on Page 8 (one file, no UI complexity).

Validation:
- Trimmed voicecard still sounds correct on hardware (regression check).
- Trimmed controller boots, accepts MIDI, accepts SPI snapshot, RAM/flash measurements meet targets.

### Phase 3: Sequencer Core
- Implement step sequencer on motherboard: 6 independent tracks, 8 steps each.
- Per-track pattern length (1–8) and clock division (polymeters).
- Direction modes (Fwd / Rev / Pendulum / Random).
- Pattern rotate (0–7).
- Full parameter snapshot via `TRIGGER_WITH_SNAPSHOT` on every trigger.
- Stateless step semantics (locks snap back to defaults).
- Master page transport: Play/Pause/Reset buttons, LED feedback, state machine.

### Phase 4: Parameter Locks
- Three-page lock system (Voice Page 1 / Voice Page 2 / Step Page) cycled by encoder turn.
- Hold-step-to-edit interaction.
- Lock clear (double-tap held step).
- Unlocked steps read live knob positions.
- Lock flag display on LCD.

### Phase 5: Step Behavior Features
- Probability (`PROB`)
- `SSUB`: bipolar — CW = ratchets (1–8), 0 = normal, –1 = Custom playback, –2 = Edit mode (buttons → sub-steps)
- Step repeat (`REPT`)
- Mutate (`MINT` + `MDIR`) — pitch interval walk across repeats and sub-steps, sawtooth reset
- Per-step rate multiplier (`RATE`)
- Velocity / accent (`VEL`)
- Glide (`GLID`)

### Phase 6: Dual Playhead & Hold Modes
- Shadow playhead implementation (5 bytes/track).
- Voltage Block mode (pause + snap on release).
- Elektron mode (no pause; edit-in-place).
- Global mode toggle on Master page (`LTCK`).

### Phase 7: UI
- Remap normal-mode pages with final knob assignments (Pages 1, 2, 3, 6, 7, 8).
- Voice/Parts page (Page 6) final layout: VSEL, BPM, SWING, LTCK on top; CDIV, LENG, DIRN, ROTA on bottom; transport button centering.
- Encoder focused-edit mode implementation.
- Full LCD layout — 4-char abbreviations on line 1, full names on line 2 when adjusting; pipe-separated button labels on Voice/Parts page.

### Phase 8: Voice Engine Refinement
**Voice engine modifications happen against a known-working sequencer.**

- Refactor envelope code to three fixed-routing instances; renumber so VCA = Env1.
- Implement LPG macro: `LPGD` drives Env1 decay; Env2 decay is `LPGD` × f(`LPGO`); `LPGA` scales Env2's contribution to filter cutoff. At `LPGO` extremes, optionally shift envelope curves and dial in a small filter Q offset (tune ranges empirically).
- Wire pitch envelope (Env3) to Osc 1 base pitch — independent of the LPG macro.
- Surface sub-osc as a layer addressable independently of Osc 1/Osc 2 selection (transient generator already in code).
- Reshape voicecard parameter struct to natively understand the elkhart snapshot format (drops the translation layer used in Phases 3–7).
- Optional: add wavefolder-bearing wave shape (Carcosa `westcoast.h` port) — `PARA` controls fold depth.
- Optional: replace YAM's `fm` algorithm internals with 16-bit sine interpolation (`InterpolateSine16` + `wav_res_sine16`) for cleaner FM.

### Phase 9: Track Relationships
- Mod matrix implementation (transpose Osc 1 / transpose Osc 2 / clock / reset / accent).
- Cross-track state evaluation in sequencer tick.
- Mod matrix UI (Button 4).
- Limit to 4 active relationships globally; validate CPU impact.

### Phase 10: Storage Expansion & Polish
- Multi-slot snapshot save/load (slot picker UI on Page 8).
- Optional: simple naming.
- MIDI sync (external clock in/out — interaction with internal Play/Stop transport).
- Performance optimizations if RAM / CPU is tight.
- Empirical reassessment: does usage call for a third lockable page, dedicated Stop button, tap tempo, etc.?

---

## References

- **Mutable Instruments Ambika repo (upstream):** `https://github.com/pichenettes/ambika`
- **YAM (immediate fork base):** `https://github.com/bjoeri/ambika`
- **Ambika manual:** `https://pichenettes.github.io/mutable-instruments-diy-archive/ambika/manual/`
- **Ambika technical notes:** `https://pichenettes.github.io/mutable-instruments-diy-archive/ambika/technotes/`
- **Michigan Synth Works Xena (SVF version):** Target hardware (Ambika clone with SVF voicecards)
- **Fors Dyad / Para:** Design reference for constrained playability, voice parameter set, modulation smoothing
- **Catalyst firmware fork:** `https://github.com/voltagecontrolled/catalyst-firmware` — reference for ratchets, step repeat, parameter lock UX patterns
