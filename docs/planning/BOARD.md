# Elkhart Work Board

Kanban-style work tracker. Items move out of the board into `CHANGELOG.md`
when shipped — that's where "Done" lives.

The "Phase N" framing in earlier `CHANGELOG.md` entries (Phase 2 – Phase 5)
is retired. New entries are topic-named and dated.

---

## Now

- **Sequencer hardware verification, round 5a-1.** Controller
  kSystemVersion `0x31` (voicecards stay on `0x30` — code unchanged).
  Hardware-test bugfix bump on top of round 5a:
  - PROB rescaled 0..127 native (was 0..255 with unreachable ceiling),
    default 127 = always fires, displays as `0%`..`100%`.
  - VEL→VCA default depth 0 → 127 in `kDefaultMod` slot 11 (velocity
    was inert; this also unmasks VOL on S6).
  - BLND clamped to 0..63 (skip the dead linear-FM range that produced
    glitches).
  - SCAL labels: leading-space pattern so cell renders `SCAL pMi` not
    `SCALpMi`; `chro` shortened to `chr`.

  Round 5a (under `0x30`, still on the voicecards): env-depth range
  extended 0..63 → 0..127 unipolar (UI passthrough + voicecard VCA `<< 1`
  rescale + defaults bumped); S5 rebuilt and reordered so step-behavior
  is leftmost (default cursor=0 → NOTE): S5a `note vel glid rate / subs
  prob mint mdir`, S5b `nois w1 pa1 tun2 / mix w2 pa2 fin2`, S5c `freq
  fdec famt adec / pdec pamt sub wave`; wave abbrs `wav1`/`wav2` →
  `w1`/`w2`; `subs` merged bipolar cell on S5a; S6 drops BPCH, renames
  OLEV → VOL, adds 8-scale SCAL quantize; S7 `swng` realigned under top2;
  resolver wires PROB / GLID / VEL+VOL / SCAL.

---

## Next — Round 5b (sequencer execution + lockable expansion)

- **Lockable `freq` / `famt` / `pamt` / `wave` on the seq surface.** User
  needs these per-step lockable; today they're config-mapped (write to
  `tr.config[]`, not lockable). Mechanism options: extend `lock_flags`
  from 24 → 32 bits (one byte more), add a `page3[8]` to `SeqStep` for
  the 4 new lockables (8 bytes/step × 8 steps × 6 tracks = **192 B
  RAM**), extend `defaults[]` 24 → 28 (×6 = 24 B), extend `lock_flags`
  (1 B × 8 × 6 = 48 B) — total **~264 B RAM**, controller goes from
  84.7% to ~91% RAM. Voicecard side: extend `kSnapshotAddrs[]` from 16
  to 20 entries (add patch addrs 16 / 22 / 58 / 11), or repurpose dead
  `kP2E3REL` (lock idx 13) for one of them. Both-sides version bump.
  User has approved the RAM cost: "if things get tight, I'll bust out
  the chopping block then."

- **RATE / REPT / SSUB-ratchet execution in `Sequencer::Clock`.** Storage
  is in place from round 5a. RATE = per-step CDIV override (0 = use
  track CDIV, 1..7 = override). REPT = re-fire current step N times
  before advancing (uses existing `shadow[kShdwREPT]`). Positive SSUB =
  N evenly-spaced retriggers within the period; subdivides `period` by
  ratchet count.

- **Custom/Edit substep editor.** Entry: hold a step button + press
  encoder while cursor is on `subs` cell. While editing: step buttons
  1..8 toggle `substep_bits` for the held step; LEDs mirror the
  bitmap; encoder click again exits.

- **Mutate (MINT/MDIR) lives exclusively inside the substep editor.**
  Walking pitch only makes sense across substeps within a single primary
  step, so MINT/MDIR don't need top-level cells on S5a. Frees two cells
  on S5a (mint, mdir) for the vel-mod controls.

- **New S6b page: portamento + vel mod settings.** Adds a second sub-page
  in group 5 for non-track-pattern voice-config knobs that don't fit
  S5b/S5c. Top row would carry portamento (kCfgSMTH, patch addr 19)
  alongside `vdst` (velocity destination) and `vamt` (velocity depth).
  Frees the `vel` cell on S5a top2 to be paired with vdst/vamt
  configurability — natural location for them is the row alongside vel
  on S5a once mint/mdir migrate to the substep editor.

- **Encoder-click focused-edit display on sequencer pages.** Today click
  is a no-op (except as a substep-edit trigger once that lands). Needs
  full-row layout: `<page name> | <full param name> <value>`,
  mirroring the `ParameterEditor` convention. Requires a full-name
  table for the lockable params and a focused-edit state machine in
  `SeqStepsPage`.

- **WAVE strip-aware LUT.** Pot currently scales 0..127 → 0..42, but
  CZ filter-sim indices 6..14 are stripped per Phase 2 and produce
  silence. Build a contiguous valid-set lookup so the pot only ever
  selects audible waveforms.

- **Hold-step semantics polish.** First pass uses "if any step button is
  held when a pot moves, write a lock for that step; release suppresses
  toggle." Catalyst-style long-press detection and double-tap-to-clear
  are not yet implemented.

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
