# Voice Envelopes + LFO

> Supersedes the LPGD/LPGA/LPGO macro envelope sections in `SPEC.md`. The
> macro design was abandoned during the Phase 5 envelope rework; this document
> captures the as-built voice envelope architecture and the current UI layout.

## Model: 3-value envelope, 3 of them

Each voice card runs three independent envelopes with fixed routing. Each
envelope is parameterized by **three bytes** plus a depth byte that lives in
the modulation slot it drives.

| Envelope | Routing | Depth byte (mod slot amount) |
|----------|---------|------------------------------|
| Env 1 — VCA | Output amplitude | Slot 10 (`ENV1 → VCA`) |
| Env 2 — VCF | SVF cutoff | Voicecard `filter_env` (E2DEPT) |
| Env 3 — Pitch | Osc 1 base pitch (coarse) | Slot 2 (`ENV3 → OSC_1_2_COARSE`) |

The voicecard's `Envelope::Update(rise, fall, curve)` runs ATTACK → DECAY →
RELEASE phase increments. `DECAY` and `RELEASE` share the same phase
increment (both fall to 0), so a single `fall` byte controls the rate at
which both stages drop. `curve` blends linear and exponential decay/release
shape (0 = linear, 127 = expo).

There is **no sustain stage**. The byte in the Patch struct historically
named `sustain` is repurposed as `curve`. The byte historically named
`release` is no longer read by the voicecard.

## UI: two pages, eight knobs each

`PAGE_ENV_LFO` — Amp + Filter envelopes:

```
top  rise | fall | curv | amp     ← E1 (VCA)
bot  rise | fall | curv | flt     ← E2 (VCF)
```

`PAGE_VOICE_LFO` — Pitch envelope + voice LFO:

```
top  rise | fall | curv | pitc    ← E3 (Pitch)
bot  rate | shap | dest | dept    ← LFO4 (voice LFO)
```

The depth knob (column 4) doubles as the row label — `amp` / `flt` / `pitc`
identify the envelope, `dept` is the LFO depth. The encoder-grab full-name
readout reads "depth" for all four.

## Parameter table layout

Patch byte addresses (controller-side, used by `Part::SetValue` /
`PatchAddrToSeqField`):

| Param | Patch addr | SeqTrack field |
|-------|-----------|----------------|
| E1 rise | 24 | `config[kCfgE1ATK]` |
| E1 fall | 25 | `defaults[8 + kP2E1DEC]` |
| E1 curv | 26 | `config[kCfgE1CRV]` |
| E1 depth | 200 (→ 82) | `config[kCfgE1DEPT]` |
| E2 rise | 32 | `config[kCfgE2ATK]` |
| E2 fall | 33 | `defaults[8 + kP2E2DEC]` |
| E2 curv | 34 | `config[kCfgE2CRV]` |
| E2 depth | 201 (→ 22) | `config[kCfgE2DEPT]` |
| E3 rise | 40 | `config[kCfgE3ATK]` |
| E3 fall | 41 | `defaults[8 + kP2E3DEC]` |
| E3 curv | 42 | `config[kCfgE3CRV]` |
| E3 depth | 202 (→ 58) | `config[kCfgE3DEPT]` |
| LFO rate | 49 | `config[kCfgLFOS]` |
| LFO shape | 48 | `config[kCfgLSHP]` |
| LFO dest | 72 | `config[kCfgLFO4D]` |
| LFO depth | 73 | `config[kCfgLFO4A]` |

The `(→ NN)` arrows on depth params show the virtual-address translation in
`Part::SetValue` — when the controller writes to virtual address 200/201/202,
it actually transmits to mod-slot byte 82/22/58 on the voicecard.

Lockable per-step: only the three fall bytes (addresses 25 / 33 / 41) live
in `defaults.page2` and can carry per-step overrides via the lock system.
The other envelope bytes are voice-wide config.

## Future tightening

- The `kP2E1REL / kP2E2REL / kP2E3REL` slots in `SeqTrack.defaults.page2`
  remain reserved as dead space (3 bytes per track × 6 tracks = 18 B).
  Reclaim them when the sequencer mode UI lands and we want more lockable
  step parameters.
- `Patch.env_lfo[i].release` byte on the voicecard is unread but still
  occupies the struct. Drop only if voicecard flash gets tight.
- LFO depth uses `UNIT_INT8 -63..63` to allow inverted modulation. If MIDI
  CC mapping is ever added for these new params, populate the `midi_cc` /
  `midi_cc_map` / `midi_nrpn_map` fields.
