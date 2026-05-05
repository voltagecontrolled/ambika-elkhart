# Personality Oscillators + Modulation Plumbing

> Adds SID and OPL character oscillators to the existing voicecard dispatch,
> introduces one-shot LFO waveforms so the voice LFO can act as a fourth
> envelope-like source, and proposes default modulation routings that exercise
> the new shapes. The voice structure, modmatrix, filter, envelopes,
> controller UI, and patch format are unchanged.

## Intent

The voicecard's existing YAM oscillator stack covers analog-flavored timbres
well but does not produce convincing SID or OPL character. This spec adds six
new oscillator shapes — four SID-flavored, two OPL-flavored — as new entries
in the existing shape dispatch, each driven by the single PARA knob already
present on every YAM oscillator. No engine replacement, no new patch fields,
no new UI pages.

The shapes target specific recognizable artifacts rather than chip-accurate
emulation: aliasing, combined-waveform bit-AND blends, LFSR-clocked noise,
inharmonic 2-op FM, and PM with a non-sine modulator. Authentic reSID /
Nuked-OPL3 fidelity is explicitly out of scope.

## Per-card cycle budget

ATmega328p @ 20 MHz, 32 KB flash, 2 KB SRAM. Output sample rate is set by
Timer2 in `voicecard/voicecard.cc` (~39 kHz), giving a budget of roughly 510
CPU cycles per output sample. The existing `Voice::ProcessBlock` pipeline
(`voicecard/voice.cc` line 440) — pitch lookup, modulation matrix, envelope
update, mixer, filter CV — consumes roughly half. The oscillator render proc
itself gets ~200–300 cycles per sample.

The render dispatch is already duplicated/specialized per shape
(`voicecard/oscillator.h` line 18) for performance. New shapes follow the
same pattern.

## Oscillator shape catalog

| Shape | Cycles/sample (est.) | PARA semantics | Character |
|-------|---------------------|---------------|-----------|
| `WAVEFORM_SID_COMBO` | 20–40 | Combo selector (saw+tri, saw+pulse, tri+pulse, all-three) | Bit-AND blend of two existing renders; sparse harmonics, "vocal" character |
| `WAVEFORM_SID_SQUARE` | 15–25 | Pulse width | Hard-edged pulse, deliberately *not* PolyBLEP-anti-aliased |
| `WAVEFORM_SID_NOISE` | 30–60 | LFSR clock divider | 23-bit LFSR clocked at a divided phase rate; tonal "metalness" varies with PARA |
| `WAVEFORM_SID_TRIANGLE` | 25–45 | Bit-shift / fold amount | XOR-MSB triangle in the SID idiom; PARA selects how many lower bits are folded |
| `WAVEFORM_OPL_FM` | 130–200 | Modulator-to-carrier ratio | 2-op phase-modulation with quarter-sine LUT; bell / metallic when ratio is inharmonic |
| `WAVEFORM_OPL_REED` | 100–170 | Modulation index | PM with a hard-edged (square / saw) modulator; reedy, rubbery |

The SID shapes deliberately do not anti-alias. Aliasing is the design
intent — smoothing erases the character.

## Implementation pattern

New shapes plug into the existing dispatch at `voicecard/oscillator.h` line
18 ("Oscillators. Note that the code of each oscillator is duplicated /
specialized, for a noticeable performance boost"). Each shape:

- Reuses the 24-bit phase accumulator path in `voicecard/oscillator.cc`.
- Uses the fixed-point primitives in `avrlib/op.h` (`U8U8MulShift8`,
  `S8S8Mul`, `S8U8MulShift8`).
- For `WAVEFORM_OPL_FM` and `WAVEFORM_OPL_REED`: requires a 256 B PROGMEM
  quarter-sine LUT. **Open question:** verify whether `voicecard/resources.cc`
  already exposes a sine table reusable here before adding one.

The SID combo shape composes two existing render paths (saw / triangle /
pulse) and bit-ANDs the upper bits of their outputs; it does not need a new
oscillator math primitive.

## One-shot LFO waveforms

The existing voice LFO (`MOD_SRC_LFO_4`, voicecard-resident) is a continuous
free-running source. With three real envelopes (amp, filter, pitch) already
committed, FM and SID patches need additional envelope-shaped sources for
modulation index sweeps, ratio sweeps, and per-note timbre transients.
Repurposing the voice LFO via one-shot waveforms gives a fourth
envelope-like source with no new patch fields and no new UI.

### New `LfoWave` enum entries

Append to `LfoWave` in `common/patch.h` line 136 (insert before
`LFO_WAVEFORM_LAST`):

| Enum | Shape | End-of-cycle behavior |
|------|-------|----------------------|
| `LFO_WAVEFORM_ONESHOT_RAMP` | Linear 0→max | Clamp at max |
| `LFO_WAVEFORM_ONESHOT_EXP` | Exponential decay max→0 | Clamp at 0 |
| `LFO_WAVEFORM_ONESHOT_AD` | Linear up then exponential down | Clamp at 0 |

### Render and clamp

`Lfo::Render()` in `common/lfo.h` line 43 detects the one-shot range and
clamps `phase_` at end-of-cycle rather than letting the natural 16-bit wrap
re-trigger the curve. The class already exposes `set_phase()` (line 89) and
tracks `looped_` (line 107); both are sufficient for one-shot use.

### Per-note retrigger

The voice LFO's phase is currently never reset on note-on. Add a
`voice_lfo_.set_phase(0)` call inside `Voice::Trigger` (`voicecard/voice.cc`
line 156) so one-shot LFOs restart per note like envelopes do. The reset is
unconditional — for non-one-shot waveforms it produces a phase-aligned
restart, which is benign and arguably an improvement.

### Per-voice vs. per-part

Only `MOD_SRC_LFO_4` is voice-local on the voicecard. `MOD_SRC_LFO_1..3` are
controller-rendered per-part and shipped via `WriteLfo` snapshots; they do
not retrigger per-note. Per-part one-shot LFOs would require controller-side
note-on hooks and are out of scope for this spec.

### Modmatrix coupling

`Voice::ProcessModulationMatrix` (`voicecard/voice.cc` line 270) AC-couples
LFO sources by treating 128 as center. This is correct for bipolar LFO
shapes but wrong for envelope-shaped one-shots, which are unipolar 0→max.
Add a special case: when the source is `MOD_SRC_LFO_4` *and* the patch's
`lfo[3].shape` is in the one-shot range, bypass the AC-coupling subtract.
Cost: ~3 cycles per matrix slot that uses `MOD_SRC_LFO_4`.

## Default modulation routings

Elkhart's init patch (`voicecard/voice.cc` lines 63–105) populates 7 of the
14 modulation slots and leaves 7 empty. The proposed defaults below make the
new oscillators usable out of the box without changing user-saved patches.

Currently active in init (for reference):

| Slot | Source | Destination | Amount |
|------|--------|-------------|--------|
| 2 | `ENV_3` | `OSC_1_2_COARSE` | 0 |
| 7 | `LFO_4` | `PARAMETER_1` | 0 |
| 8 | `SEQ_1` | `PARAMETER_1` | 0 |
| 9 | `SEQ_2` | `PARAMETER_2` | 0 |
| 10 | `ENV_1` | `VCA` | 63 |
| 11 | `VELOCITY` | `VCA` | 0 |
| 12 | `PITCH_BEND` | `OSC_1_2_COARSE` | 0 |

Proposed additions to populate the empty slots:

| Slot | Source | Destination | Rationale |
|------|--------|-------------|-----------|
| 0 | `ENV_2` | `PARAMETER_2` | Envelope-driven timbre (FM modulation index, SID combo blend on osc2) |
| 1 | `ENV_3` | `OSC_1_2_FINE` | Per-note pitch micro-sweep / FM ratio glide |
| 3 | `VELOCITY` | `PARAMETER_1` | Touch-sensitive brightness — the OPL feel |
| 4 | `LFO_4` | `PARAMETER_2` | One-shot LFO acts as second timbre envelope when shape is set to a one-shot waveform |

Slots 5, 6, and 13 remain empty for user customization, preserving headroom
for per-patch routings.

All `MOD_DST_*` values referenced above already exist in `common/patch.h`
lines 208–234. No enum, code, or protocol changes are required for this
piece — it is a data-only edit of `init_patch`.

## Resource cost summary

Voicecard flash:

- Six new render procs: ~1–2 KB combined (the inline-specialized pattern in
  `voicecard/oscillator.h` line 18 trades flash for cycles).
- Quarter-sine LUT for OPL shapes: 256 B if not already present in
  `voicecard/resources.cc`.
- One-shot LFO branch in `Lfo::Render`: ~30–60 B in `common/lfo.h`.

Track flash with `./toolchain/size-squeeze.sh voicecard` after each shape
lands. Voicecard flash is the binding constraint for this work.

Voicecard SRAM:

- LFSR state: 3 B per voice.
- FM operator phase: 3 B per operator (one operator beyond the existing osc
  phase pair).
- One-shot LFO state: zero — reuses existing `Lfo` instance fields.

Controller flash: optional — short-name strings for the new shapes in
`controller/resources.h` so the encoder readout shows e.g. "SID combo"
instead of a numeric index.

## Risks

- **2-op FM is the only shape near the cycle ceiling.** Mitigation: drop the
  quarter-sine LUT to 128 entries with cruder interpolation. Quantization
  noise is on-aesthetic for the OPL idiom.
- **Voicecard flash is tight.** If something has to give, `WAVEFORM_OPL_REED`
  is the closest in character to `WAVEFORM_OPL_FM` at low PARA and is the
  correct first cut.
- **Aliasing is intentional** for `WAVEFORM_SID_SQUARE`, `WAVEFORM_SID_NOISE`,
  and `WAVEFORM_SID_TRIANGLE`. Do not anti-alias them.
- **One-shot modmatrix coupling** must land or the user model breaks: an
  envelope-shaped source biased by 128 produces a wrong-direction modulation
  signal. The shape-aware bypass in `ProcessModulationMatrix` is mandatory,
  not optional.
- **Per-note LFO retrigger affects all voice-LFO uses,** not just one-shot
  shapes. Continuous LFOs will phase-reset on every note-on. This is
  audibly different from current behavior. Consider gating the reset on
  `lfo[3].shape >= LFO_WAVEFORM_ONESHOT_RAMP` if free-running continuous
  LFOs across notes is desirable.

## Out of scope

- Per-operator OPL envelope shapes. The existing 3 envelopes plus the
  one-shot voice LFO cover the modulation routings this spec anticipates.
- Authentic reSID or Nuked-OPL3 fidelity. The shapes target *recognizable
  artifacts*, not bit-exact emulation.
- Controller-side per-note retrigger for `LFO_1..3`. Achievable but invasive;
  defer until the +1 voice-local one-shot source is shown to be insufficient.
- Generic mix-and-match voicecard manifest system (where different cards
  expose different parameter sets and labels). Substantially larger scope;
  would warrant its own spec.

## Open questions

- Does `voicecard/resources.cc` already include a sine table reusable for
  the OPL shapes, or does this work add one?
- Should the SID combo selector (which waveforms get bit-ANDed) be exposed
  via PARA as proposed, or fixed at compile time with PARA repurposed for
  pulse width on the pulse-bearing variants?
- For `WAVEFORM_OPL_FM`, is PARA = ratio with modulation index from
  velocity the right binding, or should PARA = modulation index with ratio
  selected from a small fixed table? Ratio-as-PARA gives more sonic range
  but is harder to tune; index-as-PARA is more predictable but flatter.
- Should the per-note LFO phase reset be unconditional (simpler, may
  surprise users with continuous LFOs) or gated on the shape being a
  one-shot?
