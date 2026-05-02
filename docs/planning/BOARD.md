# Elkhart Work Board

Kanban-style work tracker. Items move out of the board into `CHANGELOG.md`
when shipped — that's where "Done" lives.

The "Phase N" framing in earlier `CHANGELOG.md` entries (Phase 2 – Phase 5)
is retired. New entries are topic-named and dated.

---

## Now

- **Voice envelopes + LFO UI refactor** —
  3-value envelope (rise / fall / curve) on voicecard, two clean pages on
  controller. See `voice_envelopes.md`. Builds clean; pending hardware test.

---

## Next

- **Apply step parameter locks at `FireStep()`.**
  The `lock_flags` / `page1` / `page2` / `steppage` fields on `SeqStep` are
  populated, but `Sequencer::FireStep()` ignores them and sends only
  `defaults[kP1NOTE]` + a velocity. Wire up: for each lockable param, send
  the lock value if the bit is set, else the default. This is the keystone
  for the entire per-step expressivity story.

- **Sequencer Mode UI.** Button 5 toggle, three-page knob remap (Voice 1 /
  Voice 2 / Step), hold-step-to-edit, knob-turn-writes-lock, double-tap
  clears. Until this lands, locks can't be authored from the panel.

- **Step behavior parameters.** PROB, VEL, REPT, RATE, SSUB ratchets,
  MINT/MDIR mutate, GLID. Most are cheap; SSUB Custom/Edit modes and Mutate
  pitch-walk are the bigger pieces.

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

---

## Done

See `CHANGELOG.md`. Existing Phase 2–5 entries stay as historical record.
