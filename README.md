# Elkhart

Custom firmware for the Mutable Instruments Ambika, targeting a **6-voice
polymetric percussive step sequencer** on the Michigan Synth Works Xena
motherboard (ATmega644p) + SVF voicecards (ATmega328p).

Fork of YAM (`bjoeri/ambika`, "Yet Another Mutation") — uses YAM's oscillator
DSP as the voice engine. The sequencer, parameter-lock system, fixed mod
routing, and ADR+Curve envelope are the original contribution.

## Status

Iterating on hardware. v2.0 builds and runs; basic transport and step
triggering work; many features from the spec are stubbed pending future
phases. See `CHANGELOG.md` for the as-built record and known issues.

## Documentation

- **`CHANGELOG.md`** — authoritative as-built architecture, phase history
- **`docs/planning/SPEC_v2.md`** — original design intent (⚠️ diverged from
  implementation; see banner at top of file)
- **`docs/wiki/MANUAL.md`** — user-facing manual draft

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
