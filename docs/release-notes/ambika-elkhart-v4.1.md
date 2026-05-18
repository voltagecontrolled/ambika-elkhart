# ambika-elkhart v4.1

Maintenance release on top of v4.0. Adds per-track MIDI sequencing with a
clock coordinator, fixes a handful of timing and velocity bugs, recovers
~3.1 KB of controller flash from dead YAM-inherited code, and tightens the
S5 / S6 cell layout. Snapshot format bumped v1 → v2 with automatic on-load
migration; v4.0 saves are read transparently.

## Highlights

- **Per-track MIDI INT / EXT mode.** The `mmod` cell on `S6a` (replacing
  the retired `vol` cell — per-voice volume lives on the mixer) selects
  whether a track plays through its voicecard or routes purely as a
  MIDI-only output lane. Switching INT → EXT releases any sounding voice
  cleanly. Saved with the snapshot, so a song carries its routing.
- **Per-track MIDI channel.** The `mch` cell on `S6a` (replacing the
  retired `BPCH` cell) sets the MIDI channel 1–16, defaulting to the
  track index. Sequencer note-on / note-off and all CC emits use this
  channel.
- **CC sequencer on EXT tracks.** `S5b` / `S5c` repurpose into a MIDI CC
  sequencer view when the track is in EXT mode: 4 slots per page (8 total
  per track), top-row pot picks the CC# (1–127 or fully-CCW `off`),
  bottom-row pot edits the lockable value with normal step-lock
  semantics. Slots default to `off` so a fresh EXT track is silent until
  opted in.
- **VAMT and GLID as standard CCs on EXT.** VAMT (`S5a` cell 2) → CC 1
  (Mod Wheel), GLID (cell 6) → CC 5 (Portamento Time). VAMT promoted to
  lockable index 4 so Mod Wheel can be sequenced per-step.
- **Clock coordinator.** The transport page's old `swng` cell is replaced
  by a 4-state `clk` (`INT` / `EXT` / `OUT` / `THR`) covering all
  combinations of clock source × clock-out enabled. Internal-timer or
  inbound-`0xF8` advance, with clock bytes gated on the way out.
- **`mrst` on transport pot 1.** Master Reset moves from cell 2 to cell 1
  after the `swng` removal. Unchanged behavior, more direct reach.
- **Velocity scaling fix.** `VOL = 255` now produces exact identity
  (was off by one). VOL=0 still mutes; smooth across the range.
- **TICK pre-charge fix.** Sequencer fires step 0 inline at the cycle
  boundary with a full-period gate after Play / Reset; previously the
  first step's gate window was truncated by one tick.
- **`multi.Start` / `multi.Stop` on transport.** Explicit lifecycle calls
  from `Play` / `Pause` / `Stop` so multi-side state stays consistent
  with the transport.
- **BPM pot remap to 60..185.** Every integer BPM in range corresponds to
  a stable detent (previous mapping skipped many values).
- **Full-width cell values on S5 / S6.** Custom sequencer / track / mixer
  pages now use the same 4-char value field as the stock parameter
  pages, with the recovered column going to longer labels (`w1` → `wav1`,
  `pa1` → `prm1`, etc.).
- **System page residual-glyph fix.** `UpdateScreen` clears both line
  buffers on entry so leftovers from the prior page don't bleed into
  Load / Save's unused positions.

## Under the hood

- **~3.1 KB controller flash recovered** by removing dead YAM-inherited
  code paths: `voice_allocator` (the polyphonic dispatcher elkhart
  doesn't use), legacy RIFF storage paths, the SysEx dump handler, and
  the edit-popup overlay.
- **Snapshot format v2.** Adds per-track MIDI channel, EXT mode mask,
  clock mode, and CC map. v1 snapshots auto-migrate on first load with
  sensible defaults (per-track MCH = track index, MMOD = INT, CC map =
  off).
- **Legacy `MIDI_OUT_SEQUENCER` clock-byte gate dropped.** Clock bytes
  always go out unconditionally at the OnStart/OnStop/OnClock layer; the
  new transport `clk` cell handles whether they actually transmit.

## Compatibility

- **Hardware:** Mutable Instruments Ambika with Michigan Synth Works Xena
  motherboard (ATmega644p) and SVF voicecards (ATmega328p).
- **Storage:** SD card (FAT16/FAT32, 8.3 filenames).
- **Voicecard count:** designed for 6 voices.
- **v4.0 saves:** read transparently. The snapshot loader detects v1 blobs
  and migrates them on the fly; first re-save commits v2 format.

## Firmware version pair

This release ships with:

- Controller `kSystemVersion = 0x41`
- Voicecard `kSystemVersion = 0x40` (unchanged from v4.0; voicecard
  binaries are byte-identical to the v4.0 release)

The OS Info page reports these. Mismatched versions can corrupt the
per-step snapshot protocol — if the OS Info page shows anything other
than this pair, re-flash.

## Flashing

A single zip is attached to this release:

- `ambika-elkhart-v4.1-firmware.zip` — contains `AMBIKA.BIN` (controller)
  and `VOICE1.BIN` … `VOICE6.BIN` (voicecards, one per slot). The six
  voicecard files are byte-identical; unzip the whole archive onto the SD
  card root.

See the [Wiki — Firmware Installation](https://github.com/voltagecontrolled/ambika-elkhart/wiki)
section of the user manual for the full flash procedure.

`SHA256SUMS` is attached separately for integrity verification of the
unzipped binaries.

## Known limitations

- RAM headroom is tight (94.5% used). Heavy SD card activity at this
  ceiling can produce buffer issues. The v4.2 milestone addresses this
  with a ground-up lock-pool refactor and page reorganization.
- Track relationships (cross-track transpose, clock, reset, accent)
  beyond the v4.0 scope are deferred to v5.0.
- Shadow playhead + Voltage Block / Elektron hold modes (`LTCK`)
  deferred.
- Audio-rate voice LFO + pitch tracking deferred.

## Acknowledgements

- **YAM** (`bjoeri/ambika`) — the upstream fork point. Voice-engine DSP
  (oscillators, sub/transient layer, SVF) is YAM's work.
- **Mutable Instruments / Émilie Gillet** — the original Ambika firmware
  and hardware design.
- Developed with AI assistance (Claude Code).

## License

GPLv3, inherited from upstream Mutable Instruments / YAM.
