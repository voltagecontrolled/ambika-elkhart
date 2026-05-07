# ambika-elkhart v4.0

First public release of elkhart — polymetric 6-track groovebox firmware for
the Ambika, with parameter locks, per-step mutation, and probability.
Targets the Michigan Synth Works Xena motherboard + SVF voicecards. Fork of
YAM (`bjoeri/ambika`).

## Highlights

- **Per-step parameter locks across three sub-pages.** 28 lockable parameters
  spanning step behavior (probability, rate, repeat, glide, mutation), voice
  oscillators (note, wave, parameter, blend, ratio, fine), and modulation
  (envelope decays/amounts, noise, sub-osc).
- **Substep editor.** Hold-step + encoder click on `subs` enters a per-step
  substep editor. Two zones: CCW = gated REPT period fires (8r..1r), CW =
  gated within-period ratchets (1x..8x). Bit toggles via S1–S8.
- **MINT / MDIR pitch walk.** Per-step mutation interval (0–24 semitones,
  named labels) with direction up / dn / ud (ping-pong) / rnd. Walks across
  repeats and ratchets.
- **Performance mixer (S6b).** Per-voice volume with pickup/catch, three
  modes: Mute-Skip, Mute-Active (kill on toggle), Solo.
- **Transport page (S7a).** STOP single-tap = pause + reset; double-tap
  within 300 ms = panic kill all six voices. Master Reset (`mrst`)
  re-aligns all track playheads to the downbeat.
- **Transport gestures from any screen.** Hold-S7 + encoder turns transport
  into a global mixer/transport shortcut; dedicated S7 mixer shortcut for
  fast access during performance.
- **Polymetric clock divisions.** Independent CDIV per track; tracks line up
  on the downbeat after Reset regardless of CDIV. RATE and CDIV display as
  human-readable ratios (`1/4`, `1/3`, `1/2`, `2/3`, `3/4`, `1/1`, `3/2`,
  `2/1`) instead of raw indices.
- **Patch saving (S8a).** Save and load patches directly from the
  performance page; firmware upgrade moves to S8b.
- **Hold-step polish.** Long-press to enter step edit, double-tap to clear
  a locked step, consistent across all three lock sub-pages.
- **Probability scoping.** Probability gates only step fires, not the
  substeps within a triggered step — clearer rhythmic behavior.
- **Track scale + substep mutations.** Track scale quantization now applies
  to mutated pitches generated within substeps.

## Compatibility

- **Hardware:** Mutable Instruments Ambika with Michigan Synth Works Xena
  motherboard (ATmega644p) and SVF voicecards (ATmega328p).
- **Storage:** SD card (FAT16/FAT32, 8.3 filenames).
- **Voicecard count:** designed for 6 voices.

## Firmware version pair

This release ships with:

- Controller `kSystemVersion = 0x34`
- Voicecard `kSystemVersion = 0x31`

The OS Info page reports these — both must match what's listed here after
flashing. Mismatched versions can corrupt the per-step snapshot protocol.

## Flashing

Two binaries are attached to this release:

- `AMBIKA.BIN` — controller (motherboard) firmware
- `VOICE.BIN` — voicecard firmware (copy to `VOICE1.BIN` … `VOICE6.BIN` on the
  SD card root, one per voice slot you want to flash)

See the [Wiki — Firmware Installation](https://github.com/voltagecontrolled/ambika-elkhart/wiki)
section of the user manual for the full flash procedure.

`SHA256SUMS` is attached for integrity verification.

## Known limitations

- Track relationships (cross-track transpose, clock, reset, accent) beyond
  the v4.0 scope are deferred to v5.0.
- Shadow playhead + Voltage Block / Elektron hold modes (`LTCK`) deferred.
- Audio-rate voice LFO + pitch tracking deferred.
- Some Page 1 / 2 / 3 knob assignments are placeholder pending empirical
  retuning on hardware.

## Acknowledgements

- **YAM** (`bjoeri/ambika`) — the upstream fork point. Voice-engine DSP
  (oscillators, sub/transient layer, SVF) is YAM's work.
- **Mutable Instruments / Émilie Gillet** — the original Ambika firmware
  and hardware design.
- Developed with AI assistance (Claude Code).

## License

GPLv3, inherited from upstream Mutable Instruments / YAM.
