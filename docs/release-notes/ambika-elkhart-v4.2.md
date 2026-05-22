# ambika-elkhart v4.2

> **Draft — not yet tagged.** Issues #32, #6, and #31 are gating
> release. This file is the source-of-truth release-notes draft;
> finalize before tagging.

Major refactor on top of v4.1. The dense per-step lock storage that
was pushing controller RAM to 94.5% is replaced by a sparse global
lock pool, freeing ~720 B and enabling lockable patch pages. The
sequencer page collapses from three sub-pages to one (intrinsic
per-step fields only), the Transport page retires (BPM/CLK move to
the System page), and Save/Load swap their popup-dialog flow for
hold-to-confirm with LED feedback. v4.1 (and v4.0) snapshots
migrate transparently on load.

## Highlights

- **Sparse lock pool.** SeqStep shrinks from 34 B to 7 B (`note`,
  `vel`, `prob`, `subs`, `glid`, `step_flags`, `substep_bits`).
  Non-intrinsic locks move to a global 192-entry pool keyed by
  `(track, step, lock_index)`. Lookup is O(192) filtered by
  `(track, step)`, comfortably within the audio-rate budget on the
  644p.
- **Lockable patch pages.** Hold any step button on Oscillators,
  Mixer, Filter, Envelopes, or LFOs and turn a pot — that
  parameter's lock for the held step writes to the pool instead of
  the patch default. The display flips to show the held step's
  locked value while the button is held. Inhibit fires on pot
  motion so the step doesn't toggle on release.
- **Pool capacity gauge.** Lower-left LCD character renders pool
  fullness as a 5×8 vertical fill while any step button is held.
  Snapshot is taken at the moment the step is pressed (release and
  re-press to refresh). Glyph queues through the same OutputBuffer
  the BufferedDisplay uses, so the timer ISR consumes it in order —
  no interrupts-off window, no audio glitch.
- **Page registry rewrite.** Button-equals-page retires in favor of
  `Sn + encoder` shortcuts: `S2` steps through patch pages, `S4`
  jumps to Sequencer, `S6` to Track settings, `S7` to Performance
  mixer, `S8` to System. `S1`–`S8` function as step buttons on
  every page that doesn't take them over (System and Performance
  mixer keep their button-driven UI). Step LEDs render on every
  inheriting page (step-on/off + playhead).
- **Transport page retired.** BPM and CLK move to the System
  page top row as pots 3 / 4. `mrst` (master-reset period) moves
  to S5a in place of the redundant VAMT cell.
- **System page revamp.** Top row: `Cur:` / `Next:` / `BPM` / `CLK`
  (encoder picks Next slot; pots set BPM and CLK mode). Bottom row:
  `save` (S1) / `load` (S3) / `info` (S5) / `exit` (S7) — labels
  align with the 4-cell column layout. Save and Load use
  hold-to-confirm: 300 ms arm (LED fast-blink) → 900 ms fire. LED
  feedback after fire — green = success, red = fail. No more
  info / error dialogs.
- **Snapshot format `0x03`.** Adds the lock-pool payload to the
  save file. v0x01 / v0x02 snapshots auto-migrate on load: the
  migration TU streams the 337 B/track v4.1 layout, extracts
  intrinsics from track defaults (so unlocked-step pitches survive
  intact), seeds pool entries from `lock_flags`, and remaps the
  retired `kPatBPCH` slot. Pool overflow during migration drops
  extras silently.
- **Audio mute hardening on Load.** `Panic()` runs before the SD
  session; after `multi.Touch()` pushes the new patch parameters,
  an explicit `voicecard_tx.Kill()` pass silences any
  patch-change transients that would leak as a click. The current
  patch ends silently — press a key or hit play to hear the
  loaded patch.

## Under the hood

- **Controller RAM 94.5% → 76.9%** (−718 B). Headroom restored to
  safely above the SD-buffer overflow threshold; future feature
  work has runway again.
- **Controller flash 90.9% → 86.4%** (−2,934 B). Lock-pool
  refactor saves more than the migration TU + UI additions cost.
- **Vendored avrlib.** Upstream `pichenettes/avril` is archived,
  so the submodule has been removed and avrlib's files
  committed directly into the outer repo. Future avrlib edits
  diff and version like any other source. Carries the original
  GPLv3 license headers.
- **CGRAM gauge queued via OutputBuffer.** Added
  `Hd44780Lcd::QueueCustomCharMap` (queues
  `SET_CGRAM_ADDR` / 8 data bytes / `SET_DDRAM_ADDR` restore via
  the same buffer DDRAM writes use) and
  `BufferedDisplay::Invalidate` (resets `scan_position_last_write_`
  so the next Tick re-issues `MoveCursor`). Replaces the original
  `SlowWrite` + `cli/sei` + `Flush` busy-wait, which produced
  ~1 ms IRQ-off windows that audibly clicked and held off the
  LED-refresh ISR.
- **SeqTrack `pattern[]` shrinks 8 → 7.** Retired the dead
  `kPatBPCH` slot.
- **`kLockableParams` bitmap removed** (placeholder from early
  v4.2 work); superseded by `ParamIdToLockIndex` LUT consulted
  by ParameterEditor.
- **SD-busy icon no longer flashes on Settings pot turns.**
  `SystemPage::UpdateScreen` was calling `Snapshot::SlotOccupied`
  every redraw (opening a fresh `SdCardSession` each time).
  Cached now; re-queried only on encoder slot change or after
  a successful save.

## Compatibility

- **Hardware:** Mutable Instruments Ambika with Michigan Synth
  Works Xena motherboard (ATmega644p) and SVF voicecards
  (ATmega328p).
- **Storage:** SD card (FAT16/FAT32, 8.3 filenames).
- **Voicecard count:** designed for 6 voices.
- **v4.0 / v4.1 saves:** read transparently. The snapshot loader
  detects v0x01 / v0x02 blobs and migrates them on the fly; first
  re-save commits the new v0x03 format. Pool overflow during
  migration is reported as a silent drop — patches with extreme
  per-step lock density may lose a few locks (cap = 192).

## Firmware version pair

This release ships with:

- Controller `kSystemVersion = 0x42`
- Voicecard `kSystemVersion = 0x40` (unchanged from v4.0 / v4.1;
  voicecard binaries are byte-identical to those releases)

The OS Info page reports these. Mismatched versions can corrupt
the per-step snapshot protocol — if the OS Info page shows
anything other than this pair, re-flash.

## Flashing

A single zip is attached to this release:

- `ambika-elkhart-v4.2-firmware.zip` — contains `AMBIKA.BIN`
  (controller) and `VOICE1.BIN` … `VOICE6.BIN` (voicecards, one
  per slot). The six voicecard files are byte-identical; unzip
  the whole archive onto the SD card root.

See the [Wiki — Firmware Installation](https://github.com/voltagecontrolled/ambika-elkhart/wiki)
section of the user manual for the full flash procedure.

`SHA256SUMS` is attached separately for integrity verification of
the unzipped binaries.

## Known limitations

- Sequencer pages S5b / S5c remain in the firmware (now-redundant
  with patch-page lock-edit, but still hosting EXT-track CC#
  configuration). Retirement and a new home for the CC# editor
  are tracked as issue #32.
- `MultiData.clock_groove_*` bytes remain reserved despite being
  unused since the transport-page swing removal. Reclaiming them
  would shift Parameter table IDs that downstream pages
  reference; the 2 B savings isn't worth the cascade.
- Lockable parameter coverage on patch pages is 15 of the 28
  lockable slots. The remaining slots (e.g. RTIO, FINE, env
  attack/curve) are reachable only via the sequencer page until
  the `Parameter ID → lock_index` LUT is expanded.

## Acknowledgements

- **YAM** (`bjoeri/ambika`) — the upstream fork point. Voice-engine
  DSP (oscillators, sub/transient layer, SVF) is YAM's work.
- **Mutable Instruments / Émilie Gillet** — the original Ambika
  firmware and hardware design.
- **avrlib** (`pichenettes/avril`) — vendored in v4.2 since the
  upstream repo is archived. Carries the original GPLv3 licensing.
- Developed with AI assistance (Claude Code).

## License

GPLv3, inherited from upstream Mutable Instruments / YAM.
