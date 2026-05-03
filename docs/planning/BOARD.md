# Elkhart Work Board

Kanban-style work tracker. Items move out of the board into `CHANGELOG.md`
when shipped — that's where "Done" lives.

The "Phase N" framing in earlier `CHANGELOG.md` entries (Phase 2 – Phase 5)
is retired. New entries are topic-named and dated.

---

## Now

- **Sequencer hardware verification, round 5b + substep editor overhaul.**
  Controller `0x32`, voicecards `0x31` — both sides must be reflashed
  (snapshot protocol extended from 16 → 20 bytes). Items to verify:
  - `freq` / `famt` / `pamt` / `wave` per-step locks: hold step, turn
    those knobs, confirm per-step snapshot variation on hardware.
  - `gtim` (portamento): audible glide on held notes.
  - `vamt` (velocity → VCA): velocity-sensitive amplitude.
  - RATE per-step override: step fires faster or slower than track CDIV.
  - Subs pot: CCW = repeats 8r..1r, deadzone at 12 o'clock, CW = 1x..8x.
  - Substep editor — gated repeats: enter via hold-step + encoder click
    on `subs` (step must have REPT > 0). Toggle bits on step buttons,
    verify LED pattern, exit, verify REPT-gated playback.
  - Substep editor — gated ratchets: enter from a step with SSUB > 0.
    Count pot CW adds ratchet slots; toggling bits silences individual
    within-period fires.
  - Substep editor — MINT/MDIR: set MINT to a named interval (e.g. `P5`),
    MDIR to `up`; verify audible pitch walk across repeat/ratchet fires.

---

## Next

- **RATE / CDIV ratio display.** User wants ratios not raw indices: `1/4`,
  `1/3`, `1/2`, `2/3`, `3/4`, `1/1`, `3/2`, `2/1`. Current
  `kCDivValues[] = {1,2,3,4,6,8,12,16}` with `kNumTicksPerStep = 6`
  makes `1/4` and `3/4` non-integer (1.5 and 4.5 ticks). Fix: change
  base unit to 12 (lcm of 3 and 4), store periods directly
  `{3,4,6,8,9,12,18,24}`. Display labels in `seq_track_page.cc` and
  `seq_steps_page.cc`.

- **New S6b page: portamento + vel-mod settings.** `gtim` and `vamt` are
  reachable on S5a but a dedicated settings sub-page in group 5 would
  surface them alongside any future per-voice config that doesn't belong
  on the lockable pages. Design TBD pending hardware testing of current
  placement.

- **Encoder-click focused-edit display on sequencer pages.** Click is a
  no-op outside the substep editor. Needs full-row layout:
  `<page name> | <full param name> <value>`, mirroring the
  `ParameterEditor` convention. Requires a full-name table for the
  28 lockable params and a focused-edit state machine in `SeqStepsPage`.

- **Hold-step semantics polish.** First pass: any held step + pot turn
  writes lock. Catalyst-style long-press detection and
  double-tap-to-clear are not yet implemented.

- **Round 5c: slot-based patch storage.** Numbered slots + save button,
  no kits/patches abstraction. `tracks_[6]` + `global_` raw dump per
  slot. `PAGE_LIBRARY` enum slot is the registry home. Voice copy/paste
  UX TBD.

---

## Next — Round 5c (patch storage)

- **Slot-based machine-state save/load.** Design constraints from user:
  numbered slots + save button. No kits/patches abstraction —
  one slot = entire `tracks_[6]` + `global_` snapshot (~1.8 KB).
  Implementation surface:
  - `Storage` class + FAT/SD backend already exist (`controller/storage.h`,
    used today only by os_info_page / card_info_page).
  - `PAGE_LIBRARY` enum slot exists in the page registry (currently
    aliased to OS Info) — natural home for the new `SeqStoragePage`.
  - File format: 4-byte header (`magic[2] + kSystemVersion + reserved`) +
    raw struct dump. Header lets the loader detect-and-refuse stale
    saves after struct-size changes (round 5b adds `kShdwMUT` bytes).
  - Empty-slot indication via file-exists probe per slot when the
    page renders.
  - **Voice copy/paste between voices.** `tracks_[dst] = tracks_[src]`
    is a 297 B memcpy. UX TBD when in front of the hardware — chord
    on S6 vs. dedicated clipboard page.

---

## Later

- **Shadow playhead + hold modes.** Voltage Block (snap on release) /
  Elektron (no pause). 5 bytes/track already reserved in `SeqTrack.shadow`.

- **Track relationships / mod matrix.** Up to 4 active inter-track
  relationships globally (transpose Osc 1 / Osc 2, clock, reset, accent).

- **Wavefolder waveform.** Carcosa-style iterative quadratic, `PARA` =
  fold depth. Slot reserved by the Phase 2 strip.

- **Empirical UI re-tuning.** Knob assignments on Pages 1/2/3/6/7/8 are
  placeholder pending playing.

- **Reclaim remaining dead `kP2E3REL` slot.** Round 4 took two of the
  three dead REL slots for `tun2` / `fin2`. Round 5b will likely
  consume this for one of the new lockables (freq / famt / pamt / wave)
  — see Round 5b above. If round 5b grows snapshot independently, the
  slot stays for later.

- **Linear FM via BLND ≥ 64 + RTIO ratio LUT.** Voicecard side: when
  `BLND ≥ 64`, use the upper half of the byte as FM depth (osc1 →
  osc2 phase modulation). RTIO drives a small ratio LUT
  (e.g. 0.25 / 0.5 / 0.75 / 1.0 / 1.5 / 2.0 / 3.0 / 4.0 in fixed
  point). Start with phase-mod (cheaper than true linear FM) and
  escalate only if percussion needs it. Will need its own planning
  doc — `voicecard/oscillator.cc` + `voice.cc` mixer changes, plus
  RTIO comment retitle in `sequencer.cc`.

- **Refresh `docs/wiki/MANUAL.md`.** Currently behind on multiple
  fronts: Page 2 still describes the never-shipped LPGD/LPGA/LPGO
  macro envelopes; Page 1 still references RTIO/FINE as visible
  cells (now retired from S5 round 4). Needs a pass once the layout
  stabilises.

- **Audio-rate voice LFO + pitch tracking.** Empirically the voice LFO
  already gets close to audio rate; SPEC had marked audio-rate as dropped
  but YAM's path may be cleaner than feared. Levers: `voice.cc` ~line 362
  (`U8U8Mul(patch_.voice_lfo_rate, 128)`) or extend
  `lut_res_lfo_increments`. Pitch tracking (`TRAK` in SPEC LFO section)
  becomes essential once it's audio-rate so the FM ratio stays musical
  across notes.

---

## Done

See `CHANGELOG.md`. Existing Phase 2–5 entries stay as historical record.
