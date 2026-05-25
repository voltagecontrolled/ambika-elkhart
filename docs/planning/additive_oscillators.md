# Additive oscillators (HRM family)

4-partial additive percussion oscillators added in v4.4-WS2. Two user-facing shapes:
- `hrm1` — RANGE drives spread, PARA detunes partials 1-3
- `hrm2` — RANGE drives spread, PARA cascades partial-to-partial PM

Each runs 4 parallel sine partials sharing the same fundamental phase increment, scaled per-partial. Implemented as two separate render functions (`RenderHrm1` / `RenderHrm2`) in `voicecard/oscillator.cc` with no per-sample mode branching.

## Spread (RANGE)

RANGE drives bidirectional partial spread for both variants:
- `RANGE = 0` → harmonic series (1, 2, 3, 4)
- `RANGE > 0` → sweeps toward octave stack (1, 2, 4, 8)
- `RANGE < 0` → sweeps toward sub-octave stack (1, ½, ¼, ⅛)

Per-partial delta tables (8.8 fixed-point) drive the sweep:
- Up: `{0, 0, 256, 1024}` — additions to harmonic 256/512/768/1024 to reach octave 256/512/1024/2048
- Down: `{0, 384, 704, 992}` — subtractions from harmonic to reach sub-octave 256/128/64/32

`fm_parameter_ = osc.range + 36` carries RANGE into the render function (convention shared with FM family). RANGE→pitch contribution suppressed via the `voice.cc:475` gate (same gate as FM family).

## PARA character

**HRM1 — percentage detune.** Per-partial detune coefficients `{0, +20, -28, +32}` × PARA, scaled relative to the current ratio so the cent-amount stays consistent regardless of spread. At PARA=127 partials 1-3 drift roughly ±150-200 cents from their nominal positions, producing chorus/swarm character.

**HRM2 — cascade PM.** Partial 0 phase-modulates partial 1; partial 1 PM-s partial 2; partial 2 PM-s partial 3. Uses unsigned PM math (`s_n * pm_depth` rather than `(s_n - 128) * pm_depth`) — same convention as `RenderFm` — for cycle savings. Sonically near-identical to signed PM with a constant phase offset on the modulated partials.

## Storage

Per-oscillator partial phases stored in `AdditiveState { uint16_t phase[4]; }` (8 B) within the shared `OscillatorState` union. Union size unchanged at 17 B (still dominated by `VowelSynthesizerState`).

## Snapshot migration

v0x08 (pre-HRM) → v0x09 (3-HRM testing layout) → v0x0a (current 2-HRM) chained migration in `controller/snapshot.cc`:
- `ShiftHrmInsertWaveform(v)` — v0x08 → v0x09, `+3` for `v >= 15`
- `ShiftHrm3DropWaveform(v)` — v0x09 → v0x0a, `-1` for `v >= 16`

v0x08 snapshots compose both shifts (net `+2` for `v >= 15`). v0x09 test snapshots get only the second shift. v0x0a snapshots load native.

## Known limitation: cross-osc CPU degradation

HRM oscillators must be used solo per voice. Pairing with any other shape on the same voice causes audible degradation in both oscillators' output — sounds like sample loss / aliasing.

Investigation done:
- Estimated per-sample CPU for HRM2 cascade PM: ~90 cycles. Voicecard budget at 40 kHz / 20 MHz is ~500 cycles per sample. Total ProcessBlock estimated ~10-15k cycles per 1 ms block, well under the 20k budget. Math suggests we're within budget.
- Originally suspected per-sample mode branching in a single `RenderAdditive` function. Splitting into separate `RenderHrm1` / `RenderHrm2` functions eliminated all per-sample branching. **Cross-osc issue persists.** Branching wasn't the cause.
- Switched HRM2 from signed to unsigned PM math (cheaper MUL instead of MULSU). Issue persists.
- No shared mutable state identified between `Oscillator` instances. `fn_table_` is `static const PROGMEM`. `data_` is per-instance.
- Inspected sync state passing (`sync_state_` / `dummy_sync_state_`) — only matters for `OP_SYNC` mix op, not default sum.

Possible remaining hypotheses (untested):
- Real CPU budget overrun that estimates underestimate
- Some shared resource (SPI bus, audio output ISR) that HRM interacts with differently
- Memory aliasing in the AdditiveState union not yet spotted

To make progress: add GPIO timing instrumentation around `Voice::ProcessBlock` and scope on the voicecard, or progressively strip `RenderHrm2` to find a minimum reproducer.

For now, documented as a limitation: HRM shapes are intended for solo-osc patches. The pre-filter wavefolder (`fold`) is the recommended companion — sine partials fold cleanly into rich harmonics.

## Rejected design alternatives

The path to the current 2-variant design ran through several rejected approaches:

**3-variant version with `hrm1` = sine→saw→triangle waveshape morph.** Tried first. PARA morphed all 4 partials' waveshape from sine through saw to triangle. Failed: the naive saw waveform (`p >> 8`) isn't bandlimited, so partial 3 at 4× fundamental aliased its upper harmonics back as inharmonic content. Sounded "detuned" even at clean settings. Lesson: any non-sine partial shape needs bandlimiting to avoid this.

**`hrm1` PARA = per-partial phase offset.** Designed to shift partial 1/2/3's LUT lookup positions by progressive offsets. Failed: the partial phase accumulators free-run from random initial values (inherited from union memory at note-on), so adding a constant offset to one partial's lookup just produces another random-looking phase relationship. Effect was inaudible. Would require resetting partial phases on note-on to make phase offset meaningful — possible but requires plumbing not currently present.

**`hrm1` PARA = drawbar-style sequential builder.** Per-partial amplitude weights faded in over PARA segments (0-31 partial 1, 32-63 partial 2, 64-95 partial 3). Sonically interesting but the per-sample weighted-sum branch added enough CPU to tip HRM2 over its timing budget, breaking the cascade-PM variant. Reverted.

**Boring sine-sum `hrm1` with no PARA effect.** Dropped — it was a less-versatile clone of `hrm2` with detune at zero.

## Open follow-ups

- Cross-osc CPU investigation (above). Tracked as a documented limitation rather than an open issue since reproducing requires hardware testing.
- Potential 3rd HRM variant if a meaningful per-sample-cheap PARA character emerges. Architecture supports it; a slot was reserved during testing.
- Resetting partial phases on note-on would unlock phase-spread as a real character control if anyone revisits.
