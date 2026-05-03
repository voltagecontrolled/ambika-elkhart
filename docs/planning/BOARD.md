# Elkhart Work Board

Kanban-style work tracker for **post-4.0** items. The v4.0 milestone
backlog lives on GitHub:
[milestone "ambika-elkhart 4.0"](https://github.com/voltagecontrolled/ambika-elkhart/milestone/1).
Items move out of the board into `CHANGELOG.md` when shipped — that's where
"Done" lives.

The "Phase N" framing in earlier `CHANGELOG.md` entries (Phase 2 – Phase 5)
is retired. New entries are topic-named and dated.

---

## Later (post-4.0)

- **Shadow playhead + hold modes.** Voltage Block (snap on release) /
  Elektron (no pause). 5 bytes/track already reserved in `SeqTrack.shadow`.

- **Track relationships beyond the v4.0 mod-matrix scope.** Issue #11
  delivers a basic mod-matrix surface for v4.0; the broader vision —
  up to 4 active inter-track relationships globally (transpose Osc 1 /
  Osc 2, clock, reset, accent) — is post-4.0.

- **Empirical UI re-tuning.** Knob assignments on Pages 1/2/3/6/7/8 are
  placeholder pending playing.

- **Reclaim remaining dead `kP2E3REL` slot.** Round 4 took two of the
  three dead REL slots for `tun2` / `fin2`. Round 5b consumed another
  for the new lockables. Final dead slot stays for a future lockable.

- **Linear FM via BLND ≥ 64 + RTIO ratio LUT.** Voicecard side: when
  `BLND ≥ 64`, use the upper half of the byte as FM depth (osc1 →
  osc2 phase modulation). RTIO drives a small ratio LUT
  (e.g. 0.25 / 0.5 / 0.75 / 1.0 / 1.5 / 2.0 / 3.0 / 4.0 in fixed
  point). Start with phase-mod (cheaper than true linear FM) and
  escalate only if percussion needs it. Will need its own planning
  doc — `voicecard/oscillator.cc` + `voice.cc` mixer changes, plus
  RTIO comment retitle in `sequencer.cc`.

- **Audio-rate voice LFO + pitch tracking.** Empirically the voice LFO
  already gets close to audio rate; SPEC had marked audio-rate as dropped
  but YAM's path may be cleaner than feared. Levers: `voice.cc` ~line 362
  (`U8U8Mul(patch_.voice_lfo_rate, 128)`) or extend
  `lut_res_lfo_increments`. Pitch tracking (`TRAK` in SPEC LFO section)
  becomes essential once it's audio-rate so the FM ratio stays musical
  across notes.

- **Toolchain sustainability.** CI builds depend on `debian/eol:squeeze`
  (digest-pinned) for avr-gcc 4.3.5. Squeeze is EOL — image hosting and
  Debian's archive could disappear. Future options: vendor `.deb` files
  into the repo, or migrate to a newer avr-gcc once we've confirmed it
  doesn't miscompile YAM's oscillator DSP.

---

## Done

See `CHANGELOG.md`. Existing Phase 2–5 entries stay as historical record.
