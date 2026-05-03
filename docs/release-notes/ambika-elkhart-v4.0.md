# ambika-elkhart v4.0

> **Draft.** First public release of elkhart — a 6-voice polymetric percussive
> step sequencer firmware for the Mutable Instruments Ambika hardware (Michigan
> Synth Works Xena motherboard + SVF voicecards). Fork of YAM (`bjoeri/ambika`).
>
> Items still tracked on the [v4.0 milestone](https://github.com/voltagecontrolled/ambika-elkhart/milestone/1)
> will fill in the **Highlights** and **Known limitations** sections as they
> close.

## Highlights

<!-- Seed list — finalize once milestone closes. -->

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
- **Transport STOP + panic (S7a).** Single-tap = pause + reset; double-tap
  within 300 ms = panic kill all six voices.
- **Polymetric clock divisions.** Independent CDIV per track; tracks line up
  on the downbeat after Reset regardless of CDIV.

<!-- Add as 4.0 issues close: patch saving (#10), mod matrix (#11), LFO4 clock
sync (#12), iterative probability modes (#6), mixer cosmetic fixes (#8),
master reset (#9), transport S7a enhancements (#2), RATE/CDIV ratio display
(#14), encoder-click focused-edit (#15), hold-step polish (#16), wavefolder
(#18). -->

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

<!-- Seed list — refine as 4.0 milestone work resolves. -->

- Track relationships (cross-track transpose, clock, reset, accent) beyond
  the v4.0 mod-matrix scope are still pending.
- Shadow playhead + Voltage Block / Elektron hold modes (`LTCK`) deferred.
- Audio-rate voice LFO + pitch tracking deferred.
- Some Page 1 / 2 / 3 knob assignments are placeholder pending empirical
  retuning on hardware.

## Acknowledgements

- **YAM** (`bjoeri/ambika`) — the upstream fork point. Voice-engine DSP
  (oscillators, sub/transient layer, SVF) is YAM's work.
- **Mutable Instruments / Émilie Gillet** — the original Ambika firmware
  and hardware design.
- AI-assisted development credited in `README.md`.

## License

GPLv3, inherited from upstream Mutable Instruments / YAM.
