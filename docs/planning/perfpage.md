# Performance Macros + Beat-Repeat / Loop

Issue: #29.

A two-surface live performance feature:

- **S7a** — session setup additions: master scale, master transpose, master
  reset.
- **S7b** — live performance surface combining (a) seven user-assignable
  global parameter offset slots and (b) sequencer beat-repeat / loop /
  scrub gestures.

Status: design draft. Implementation has not started.

---

## S7a additions (session setup)

| Slot | Param | Type | Notes |
|------|-------|------|-------|
| P1   | SCAL  | override (linear, 0 = sentinel) | Master scale; survives all panic resets. |
| P2   | TRSP  | override (signed, semitones) | Master transpose (TBD: candidate to remain on S7b as offset; see open questions). |
| —    | RESET | button gesture | Master reset (existing). Clears all S7b live offsets and pickup state. |

These are **session-level** harmonic / global decisions, not performance
gestures. They persist with the patch and are not affected by the S7b
panic-clear gesture.

---

## S7b — live performance surface

Two coexisting surfaces on one page:

1. **Seven assignable parameter offset slots** (P1–P7, eighth slot TBD).
   User binds any sound parameter from the lockable param table to each
   slot. Pots edit a bipolar offset added on top of the patch / lock
   value at render time.
2. **Sequencer loop / beat-repeat / scrub** on switches S1–S8.

Layout follows S6b (performance mixer) — row of pots above the LCD with
4-char abbreviations for the currently-bound param per slot, value
readout below.

### Offset surface (pots P1–P7)

- Pickable params: same set as the S5a/S5b lockable param table. The
  range, value semantics, and 4-char abbreviation come from that table —
  no new metadata to maintain.
- Offset is bipolar; clamp vs. wrap behavior is per-param. **Wrap** for
  enum-type params (waveforms, etc.) for max impact; clamp for scalar
  params with hard endpoints.
- Live offset values are session-state — never persisted; always reset
  to 0 on patch load.

### Picker gesture

- **Hold S7 + twist pot N on S7b** = scrolls slot N's binding through
  the available param list.
- Release S7 commits.
- Hold-S7 only means something while on S7b. There is no cross-page
  binding gesture.

### Pickup mode

- Engaged on every offset reset (patch load, tap-S8 panic, hold-S8
  binding clear).
- Per-pot. Pot is inert until its physical position passes through 0,
  at which point it picks up and tracks normally.
- Prevents jump artifacts when offsets reset to 0 with pots parked at
  arbitrary positions.

### Sequencer gestures (S1–S8)

| Switch | Alone | + Hold S6 | + Hold S7 |
|--------|-------|-----------|-----------|
| S1     | beat-repeat 1 bar (96 ticks)     | 2/3 bar (64 ticks)   | exclude track 1 (toggle) |
| S2     | beat-repeat 1/2 bar (48 ticks)   | 1/3 bar (32 ticks)   | exclude track 2 (toggle) |
| S3     | beat-repeat 1/4 bar (24 ticks)   | 1/6 bar (16 ticks)   | exclude track 3 (toggle) |
| S4     | beat-repeat 1/8 bar (12 ticks)   | 1/12 bar (8 ticks)   | exclude track 4 (toggle) |
| S5     | beat-repeat 1/16 bar (6 ticks)   | 1/24 bar (4 ticks)   | exclude track 5 (toggle) |
| S6     | (modifier — illuminates triplet armed indicator)        | —        | exclude track 6 (toggle) |
| S7     | (modifier — picker / track-exclusion mode)              | —        | — |
| S8     | tap = exit + clear all live offsets | —     | hold = exit + keep offsets |

**Beat-repeat / loop semantics:**

- Pressing S1–S5 punches in a loop window of the indicated length,
  starting at the **next clean musical division** equal to the chosen
  rate, measured against the master clock. (Snap-forward, not
  snap-nearest — avoids stumble.)
- The window is **master-clock relative**. A track at CDIV=1/2 with
  LAST=8 (16-bar pattern) loops only the bar-sized slice falling inside
  the window. Each track's playhead loops within its own pattern,
  scoped to that master-clock window.
- All five rates divide 96 ticks evenly; binary and triplet ladders
  remain phase-coherent with the bar at every rate. Switching rates
  mid-loop never throws the grid off.
- **Hold S6** during the gesture maps S1–S5 to triplet variants (see
  table). Triplet "armed" indicator lights while S6 is held; release
  reverts to binary.
- On switch release, the master clock is treated as having continued
  underneath. Each track's playhead snaps to **where it would have
  been** had the loop not happened (no time freeze).

**Track exclusion (hold S7 during loop):**

- While S7 is held in beat-repeat mode, S1–S6 illuminate green and
  represent the six voice tracks.
- Tapping S1–S6 toggles whether that track participates in the active
  beat-repeat loop. Excluded tracks continue normal playback uninterrupted.
- Exclusion bitmap **persists with the patch**. State is not reset on
  page exit.

**Scrub:**

- While holding any of S1–S5, the encoder scrubs the active loop's
  playhead position within its window. Encoder press releases scrub
  and resumes normal looping at the scrubbed position.

### Reset rules

| Gesture | Effect |
|---------|--------|
| Tap S8 (panic) | Clear all live offsets to 0; engage pickup on every pot. Bindings preserved. Track exclusion preserved. Exit page. |
| Hold S8 | Exit page; keep offsets and bindings as-is. |
| Master reset (S7a) | Same as tap-S8: clear live offsets, engage pickup. |
| Patch load | Live offsets always 0. Bindings + exclusion bitmap loaded from patch. Pickup engaged on every pot. |

There is **no carve-out** for the offset reset rule — every S7b slot
clears uniformly. Override-type params (SCAL, master TRSP) live on S7a
and are not affected.

---

## Persistence

Per-patch additions:

| Field | Bytes | Notes |
|-------|------:|-------|
| S7b slot bindings | 7 | One byte per slot — index into lockable param table. |
| Track exclusion bitmap | 1 | Six bits used (one per track). |
| **Total** | **8** | Added to patch struct. |

Live offset values, pickup flags, beat-repeat state, scrub state — all
session RAM, never persisted.

---

## RAM cost (live)

| State | Bytes |
|-------|------:|
| 7 slot bindings × `{param_id, current_offset}` | 14 |
| Pickup mode flags (8 pots, 1 bit each) | 1 |
| Track exclusion bitmap (live mirror) | 1 |
| Beat-repeat: active flag, length enum, captured loop-start tick | ~6 |
| Triplet-mode flag (S6 held) | <1 |
| Scrub: active flag, suppressed-advance bookkeeping | ~3 |
| **Total** | **~25** |

Headroom per `reference/optimization.md` is ~274 bytes; this fits with
margin for any of Tier-2 cleanups deferred.

## Flash cost (estimate)

- Page implementation + LCD render: ~500–800 bytes.
- Beat-repeat / loop / scrub logic in `sequencer.cc`: ~800–1500 bytes.
  This is the most uncertain estimate — the surgery interacts with tick
  advancement, snap-forward boundary detection, and per-track scoping
  of the master-clock loop window.
- Total estimate: ~1.5–2.5 KB. Controller currently 91.6% flash with
  ~5.4 KB free.

## Implementation notes

- Offset application path: `Part::SetParameter` (or equivalent in the
  voice render fan-out) — add active S7b offsets after lock resolution,
  before send to voicecard.
- Lockable param table reuse: the picker walks the same indices used by
  `seq_steps_page.cc` lock writes. Range / step / abbreviation come
  for free.
- Snap-forward: needs the current master-clock tick + the rate's tick
  count; loop-start = `((current_tick / rate) + 1) * rate`.
- Per-track loop-window scoping: each excluded track ignores the loop;
  each included track's playhead is constrained to `[loop_start_tick,
  loop_start_tick + window_length)` mod its own pattern length.
- Storage version bump required for the 8 new patch bytes — coordinate
  with the saveload path.

## Open questions

1. **TRSP location.** Move fully to S7a (session-level), or keep on S7b
   as a default-bound offset slot? Master TRSP and per-S7b-offset TRSP
   are not the same thing; if both are wanted, they need different
   names.
2. **Slot count.** Seven slots or eight? S7b layout can fit eight pots;
   the eighth could be left as TBD or filled with a default-bound param.
3. **Loop-window edge case.** What happens if a track's CDIV makes its
   step interval *longer* than the loop window (e.g. CDIV=1/4 with S5's
   1/16 bar window)? Loop a sub-step slice, or skip looping that track?
4. **Picker UX during hold.** Does the LCD show the *list* of available
   params during hold-S7+twist (scrolling display), or just the new
   selection as the pot moves? Latter is simpler.
5. **Default bindings on first boot.** Suggested: FREQ, RESO, ADEC, FDEC,
   WAV1, PRM1, OSC2 wave (or PRM2). Confirm after the eighth-slot
   decision.
6. **Tap-S8 vs page navigation.** Existing S8 behavior on other pages
   needs to coexist; S7b's tap-S8-clear must not conflict with whatever
   global S8 role exists.
