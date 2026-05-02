# Elkhart Work Board

Kanban-style work tracker. Items move out of the board into `CHANGELOG.md`
when shipped — that's where "Done" lives.

The "Phase N" framing in earlier `CHANGELOG.md` entries (Phase 2 – Phase 5)
is retired. New entries are topic-named and dated.

---

## Now

- **Sequencer hardware verification, round 3.** kSystemVersion `0x23` —
  page reorder (S3 → S5 → S6 → S7), S2+encoder page jump, lowercase labels,
  WAVE clamp, track-page pot scaling, curve defaults. Locks confirmed
  working in round 2; round-3 changes need hardware retest. See
  `sequencer.md` for the open verification checklist.

---

## Next

- **Encoder-click focused-edit display on sequencer pages.** Today click
  is a no-op. Needs full-row layout: `<page name> | <full param name>
  <value>`, mirroring the `ParameterEditor` convention. Requires a
  full-name table for the 24 lockable params and a focused-edit state
  machine in `SeqStepsPage`.

- **WAVE strip-aware LUT.** Pot currently scales 0..127 → 0..42, but
  CZ filter-sim indices 6..14 are stripped per Phase 2 and produce
  silence. Build a contiguous valid-set lookup so the pot only ever
  selects audible waveforms.

- **Display offsets for signed params.** `FINE` / `OSC2D` / `LFO depth`
  store as `int8_t` with bias 64. The seq pages render the raw byte
  ("64" for "no detune"). The Parameter system handles this via
  `UNIT_INT8`; the seq pages need their own offset-aware renderer
  (or to route through Parameter for display).

- **Step behavior parameters.** PROB, VEL (currently sent but not
  lockable in resolver), REPT, RATE, SSUB ratchets, MINT/MDIR mutate,
  GLID. Most are cheap; SSUB Custom/Edit modes and Mutate pitch-walk are
  the bigger pieces.

- **Hold-step semantics polish.** First pass uses "if any step button is
  held when a pot moves, write a lock for that step; release suppresses
  toggle." Catalyst-style long-press detection and double-tap-to-clear
  are not yet implemented.

---

## Later

- **Shadow playhead + hold modes.** Voltage Block (snap on release) /
  Elektron (no pause). 5 bytes/track already reserved in `SeqTrack.shadow`.

- **Track relationships / mod matrix.** Up to 4 active inter-track
  relationships globally (transpose Osc 1 / Osc 2, clock, reset, accent).

- **Storage multi-slot.** Slot picker on Page 8; today there's only a
  single state snapshot.

- **Wavefolder waveform.** Carcosa-style iterative quadratic, `PARA` =
  fold depth. Slot reserved by the Phase 2 strip.

- **Empirical UI re-tuning.** Knob assignments on Pages 1/2/3/6/7/8 are
  placeholder pending playing.

- **Per-voice defaults sub-page on S6.** Currently S6 shows track-pattern
  settings only; a second page in group 5 reserved (`PAGE_PART_ARPEGGIATOR`
  stub) for per-voice default-knob editing once layout settles.

- **Reclaim dead `kP2*REL` slots.** 3 bytes per track × 6 tracks = 18 B
  reserved in `SeqTrack.defaults.page2`. Repurpose for filter locks
  (FREQ/RES/DRIV) or FM placeholders when empirical demand surfaces.

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
