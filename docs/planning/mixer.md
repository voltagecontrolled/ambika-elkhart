# Performance Mixer (S6b)

Topic spec for `SeqMixerPage` — the live performance mixer on slot S6b.
Implementation: `controller/ui_pages/seq_mixer_page.{h,cc}`. Resolves
issue #3.

## Goals

Live-performance control surface for the six tracks: per-voice volume,
sequencer mute, audio mute, and solo, all reachable simultaneously from a
single page. Designed for percussive performance, not patch design — state
is transient (cleared on power-cycle, not saved with the patch slot).

## Layout

```
Pots:  top1 top2 top3 [unused]      Buttons:  S1 S2 S3 S4 S5 S6 S7 S8
       bot1 bot2 bot3 [unused]                v1 v2 v3 v4 v5 v6 mode unmt
```

- Pots top1–3 / bot1–3 → per-voice volume → `SeqTrack.pattern[kPatVOL]`.
- S1–S6 → toggle the **active mode's** bit for that voice.
- S7 tap → cycle mode (MT-S → MT-A → SOLO → MT-S).
- S7 hold → shift-gate (see below).
- S8 → unmute-all (clears all three bit sets).
- Pots top4 / bot4 → unused (reserved for per-voice send/pan).
- Encoder → cursor across 8 cells; spills to neighboring page at boundary.

## Modes

Three independent per-voice bit sets:

| Mode  | Bit semantics            | On bit 0→1 toggle of sounding voice         |
|-------|--------------------------|----------------------------------------------|
| MT-S  | sequencer mute           | none — current note's envelope completes     |
| MT-A  | audio mute               | `voicecard_tx.Kill(v)` — instant audio cut   |
| SOLO  | solo (any-set inverts)   | non-solo voices that lost audibility get Kill|

Audibility rule (in `Sequencer::FireStep`):

```
audible(i) = !smut[i] && !amut[i] && (solo == 0 || solo[i])
```

If `!audible(i)`, `FireStep` early-returns before
`voicecard_tx.TriggerWithSnapshot`.

## Pickup-catch volume

On page entry pots may be physically far from the stored VOL. Each pot
arms a per-voice "caught" flag — the first OnPot event records the
physical position; subsequent events only write once the pot crosses the
stored value (in 7-bit pot space). Same idea as the substep editor's
pot-0 pickup guard.

## Shift-gate (S7 hold)

S7 doubles as mode-cycle (tap) and group-toggle gate (hold):

- While S7 is held, S1–S6 presses XOR into `pending_toggle_` instead of
  applying live. Queued voices' LEDs blink red.
- On S7 release: if `pending_toggle_ != 0`, XOR it into the active mode's
  bit set (with the appropriate immediate-action pass for MT-A/SOLO),
  and **don't cycle mode**. If pending is empty, cycle mode normally.

This puts batched group changes on a single button without colliding
with `Ui::Poll`'s system-wide SHIFT semantics on S8.

## Why not S8 hold?

S8 is the system-wide SHIFT prefix in `Ui::Poll` — when S8 is held,
`switches_.low(0)` is true and any other key release is rewritten to
`SWITCH_SHIFT_*` (which falls through to `UiPage::OnKey` storage
operations: Copy / Swap / Paste / Snapshot). Worse, S8's own release is
suppressed via `inhibit_switch_` once any S1–7 fires while S8 is down.
The original S8-hold-gate plan was unsalvageable; S7 has none of these
special semantics.

## State scope

All state is **transient file-static** in `seq_mixer_page.cc`:

- `smut_bits_`, `amut_bits_`, `solo_bits_` — 6-bit masks
- `mode_` — 0/1/2 = MT-S / MT-A / SOLO
- `pot_caught_` (6-bit) + `pot_entry_[6]` — pickup state
- `pending_toggle_` — shift-gate queue

No `SeqTrack` struct change, no snapshot byte, no protocol bump. Mute/solo
do not survive power-cycle by design (performance state, not patch state).

## Sequencer integration

A single early-return in `Sequencer::FireStep`:

```c
if (SeqMixerPage::skip_mask() & (1 << t)) return;
```

`skip_mask()` returns `(smut | amut) | (solo ? ~solo & 0x3f : 0)`.

## Future work

- Pots top4 / bot4 are unused; reserved for per-voice send (effects bus)
  or pan if those become relevant later.
- Encoder click is a no-op on this page; could host a "save mute scene"
  or similar if scene-recall becomes desirable.
- Mute persistence is intentionally off; if patch slots get a separate
  "performance state" slice, mute/solo could live there.
