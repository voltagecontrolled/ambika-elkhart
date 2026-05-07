# Elkhart

Polymetric 6-track groovebox firmware for the Ambika — parameter locks,
per-step mutation, probability.

Targets the Michigan Synth Works Xena motherboard (ATmega644p) + SVF
voicecards (ATmega328p). Fork of YAM (`bjoeri/ambika`, "Yet Another
Mutation") — uses YAM's oscillator DSP as the voice engine. The
sequencer, parameter-lock system, fixed mod routing, and ADR+Curve
envelope are the original contribution.

## Status

First public release: v4.0. Iterating on hardware. See `CHANGELOG.md` for
the as-built record and `docs/release-notes/` for what's in each release.

## Documentation

- **`CHANGELOG.md`** — authoritative as-built architecture, phase history
- **`docs/planning/SPEC.md`** — original design intent (⚠️ diverged from
  implementation; see banner at top of file)
- **`docs/planning/sequencer.md`** — topic spec for the per-step parameter
  lock system + sequencer mode UI
- **`docs/planning/voice_envelopes.md`** — topic spec for the as-built
  3-envelope voice architecture
- **`docs/planning/BOARD.md`** — Now / Next / Later work board
- **`docs/wiki/MANUAL.md`** — user-facing manual draft

## Releases

Tagged releases are published on the [Releases](https://github.com/voltagecontrolled/ambika-elkhart/releases)
tab — each ships an `AMBIKA.BIN` (controller), a `VOICE.BIN` (voicecard),
and a `SHA256SUMS` file. The user manual lives on the
[Wiki](https://github.com/voltagecontrolled/ambika-elkhart/wiki) and is
synced from `docs/wiki/MANUAL.md` at every change to that file.

Release notes are drafted under `docs/release-notes/` before tag-time;
the release workflow uses the matching file as the GitHub Release body.

## Hardware platform

- **Motherboard:** ATmega644p — 64 KB flash, 4 KB RAM, 20 MHz
- **Voicecards (×6):** ATmega328p — 32 KB flash, 2 KB RAM
- **Display:** 2×40 character LCD
- **Controls:** push encoder, 8 pots (4 above + 4 below LCD), 8 buttons w/ LEDs
- **Audio:** 6× individual outs + mix out (808-style normalling)
- **Storage:** SD card (FAT16/32, 8.3 names)

## License

GPLv3, inherited from upstream Mutable Instruments / YAM. Contains a variant
of Peter Knight's Cantarino formant-synthesis algorithm.

Original developer (upstream): Emilie Gillet (emilie.o.gillet@gmail.com).

Developed with AI assistance (Claude Code).
