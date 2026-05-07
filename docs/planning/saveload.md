# Save / Load (Snapshots)

Persistent storage of the full instrument state — patch + sequencer +
multi — to SD card, addressable by slot. Issue #10.

## Why a new format

The Phase 3 refactor folded all per-voice state into `SeqTrack` and left
the legacy `Storage::Save()` RIFF path non-functional: `Part::raw_patch_data()`
returns NULL, so the existing patch-save code writes RIFF chunks of size
zero, and there is no reverse marshaller from legacy `Patch` bytes back
into `SeqTrack`. Repairing the RIFF path would require a forward+reverse
marshaller pair AND would silently drop elkhart-specific state (BITS/CRSH,
8-step pattern, MUT8/SSUB, SeqTrack `config` bytes).

Snapshots bypass RIFF entirely and serialize `SeqTrack` directly. The
codec is decoupled from the legacy `Storage::Save/Load` path.

## On-disk format

Path: `/SNAPSHOT/NN.SNP` where `NN` is a zero-padded slot 00–63.

| offset | size | field        |
|-------:|-----:|--------------|
|     0  |   4  | magic `ELKS` |
|     4  |   1  | version      |
|     5  | 2022 | tracks (6 × 337 B persistent SeqTrack data) |
|  2027  |   5  | MultiData    |
|  2032  |   1  | 8-bit checksum (sum of all preceding bytes) |

Total file = **2,033 B**, fixed.

Per-track persistent slice = 337 B = `steps[8]` (272) + `pattern[8]`
+ `defaults[28]` + `config[29]`. The trailing `shadow[7]` (transient
playhead state) is **not** serialized. `offsetof(SeqTrack, shadow)` is
asserted at compile time to lock down the boundary.

`MultiData` is the 5-byte clock/groove/master-reset block also persisted
to EEPROM at boot. `SeqGlobal` (transport, swing, hold mode, …) is not
saved — these are session-level UI state, not patch data.

## UI

`SystemPage` (registered at `PAGE_LIBRARY`'s slot, reachable via S8 from
the top-level page) has three modes: menu, save-slot picker, load-slot
picker.

Display labels: **Cur:** is the slot the running state was last loaded
from or saved to (`--` if undefined since boot). **New:** is the slot the
encoder is hovering on. The slot picker marks occupied slots with `*`.

A successful Save or Load updates `Cur:` to the operated slot. `Cur:` is
RAM-only — undefined again at boot.

Switch layout (menu): `S1 save`, `S4 load`, `S7 info` (opens `OsInfoPage`),
`S8 exit`. Save-on-occupied-slot prompts a `DIALOG_CONFIRM` overwrite
dialog; load-on-empty-slot shows `DIALOG_INFO "empty"`.

## Voicecard SPI bus contention

The voicecards and SD card share one SPI bus, arbitrated by
`scoped_resource<SdCardSession>`. While that resource is held, SPI is
configured for SD card I/O. The Timer2 ISR fires at audio rate and calls
`voicecard_tx.SendBytes()` unconditionally — `SendBytes` does **not**
check `sd_card_busy_`. So any voicecard write queued during the SD session
either clobbers the SD card's chip-select mid-transfer (corrupting the
file) or is dispatched onto a bus that's mode-configured for the SD card
(corrupting voicecard internal state).

Two consequences:

1. `Snapshot::Load` must not call any voicecard-touching code while the
   `SdCardSession` is live. `sequencer.Reset()` and `multi.Touch()` —
   both of which queue `Release`/`WriteData` calls per voice — run after
   the session ends, not inside it. The session is held in an explicit
   inner block to make this lifetime obvious.

2. The sequencer must not fire steps during the SD transfer. Step-fires
   queue `TriggerWithSnapshot` traffic via the same path. `Save` calls
   `sequencer.Stop()` and `Load` calls `sequencer.Panic()` before opening
   the file. `BeginSdCard`'s `FlushBuffers` then waits for the queued
   `Release`/`Kill` writes to drain before reconfiguring SPI.

After Load, transport is stopped — press Play to resume.

## Live-use limitation

Because Save and Load auto-stop transport, they are not safe to use mid-
performance. The cleanest fix would be to make `voicecard_tx.SendBytes()`
short-circuit when `sd_card_busy_` is set, removing the need to stop the
transport at all. That change lives in a hot ISR path used by every
voicecard-bound write in the firmware and is out of scope for the initial
save/load feature.

## Memory and code budget

- 2,033 B per slot on SD card. 64 slots = ~130 KB of SD use at full
  occupancy.
- No EEPROM use beyond what was already there.
- Flash growth from feature: ~3 KB (snapshot codec + `SystemPage`).
- RAM growth: 6 B (SystemPage's `cur_slot_` / `new_slot_` /
  `pending_slot_` / `mode_` statics).

## Files

- `controller/snapshot.{h,cc}` — codec
- `controller/ui_pages/system_page.{h,cc}` — UI
- `controller/storage.h` — `friend class Snapshot;` for `file_` / `fs_`
  access
- `controller/ui.cc` — registry repurposes `PAGE_LIBRARY` slot to point
  at `SystemPage`; S8 `default_most_recent_page_in_group[7]` retargeted
  to `PAGE_LIBRARY`
