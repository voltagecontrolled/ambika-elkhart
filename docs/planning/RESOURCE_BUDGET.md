# Resource Budget — Open Issues

Projected flash and RAM impact of the open GitHub issues, against the
current v4.2 baseline. Estimates are deltas in bytes; **low** is the
optimistic minimum (clean implementation, code reuse maximized) and
**high** is the pessimistic ceiling (independent surfaces, generous
tables, no reuse). Negative numbers indicate a net reclaim.

Re-measure with `./toolchain/size-squeeze.sh controller|voicecard`
after each landed issue and update both this table and the baseline.

---

## Baseline (v4.2, 2026-05-22)

| Target     | Flash used | Flash free | RAM used | RAM free |
|------------|-----------:|-----------:|---------:|---------:|
| controller |    56634 B |     8902 B |   3151 B |    945 B |
| voicecard  |    26226 B |     6542 B |   1053 B |    995 B |

Voicecard flash is the tightest budget (80 % full); controller RAM
is the next-tightest (76 % full).

---

## Per-issue projection

Columns are signed byte deltas. **C-flash / C-RAM** = controller; **V-flash
/ V-RAM** = voicecard. Empty cells = no impact expected.

| #   | Title                                                | C-flash low | C-flash high | C-RAM low | C-RAM high | V-flash low | V-flash high | V-RAM low | V-RAM high | Notes |
|-----|------------------------------------------------------|------------:|-------------:|----------:|-----------:|------------:|-------------:|----------:|-----------:|-------|
| 6   | Iterative probability mode for part steps            |        +400 |         +900 |        +6 |        +10 |             |              |           |            | ~53-entry CW slot LUT, per-track cycle counter (6 B), global FILL state (1 B), pot-zone dispatcher. Counter reset on transport stop. |
| 8   | Mixer page cosmetic fixes                            |         +30 |         +100 |           |            |             |              |           |            | Label re-layout + LED-color polarity swap on `seq_mixer_page`. Pure draw-side change. |
| 11  | Mod matrix (4 slots × 2 pages on S4a/S4b)            |        +600 |        +1500 |       +32 |        +96 |        +200 |         +600 |       +24 |        +72 | New page + 4 slot records (dep/src/dst). V-side cost only if matrix is evaluated on voicecard; controller-side evaluation pushes already-resolved values via existing patch transport (cheaper voicecard, more controller traffic). |
| 12  | Clock-sync rates for LFO 4                           |        +100 |         +300 |           |         +2 |             |              |           |            | Extend LFO 4 rate field to include sync slots; reuses existing sync-rate table from LFO 1–3. |
| 18  | Wavefolder waveform                                  |         +10 |          +40 |           |            |        +200 |         +500 |           |            | Iterative quadratic fold; one wave-enum slot. **Risk:** voicecard already 80 % full — landing this may force dropping the reserved CZ filter-sim slots referenced in #18. |
| 19  | Performance: master transpose                        |         +60 |         +150 |        +1 |         +2 |             |              |           |            | Depends on #29 page existing. Bipolar `-12..+12` applied pre per-track quantize in `Sequencer::FireStep`. |
| 20  | Performance: master scale                            |        +100 |         +250 |        +1 |         +2 |             |              |           |            | Depends on #29. Reuses existing `kScaleMasks[]` palette + `---` sentinel; overrides per-track SCAL in fire path. |
| 25  | Track clear function on unused S6a pot               |        +200 |         +400 |        +2 |         +4 |             |              |           |            | Five clear modes (locks / steps / notes / voice / all) + 800 ms long-press-to-arm + tap-to-confirm state machine. |
| 29  | Performance page on S7b (frame: overrides + repeat)  |        +800 |        +1500 |        +8 |        +24 |             |              |           |            | New page registry + event handler, 8 override/offset pots, S1–S6 beat-repeat (1, 1/2, 1/4, 1/8, 1/16 bar) with quantize-to-16th punch-in, encoder scrub, S7 exclude-mask, S8 exit. State: repeat rate, exclude bitmask, scrub anchor. |
| 31  | Sub-step editor un-selects on button release         |         +20 |          +80 |           |            |             |              |           |            | Suppress the release event for the held step that opened the editor. Local fix in `seq_step_substeps_page` / step-button latch. |
| 32  | Retire S5b/S5c; relocate EXT-track CC# editing       |        -400 |         +200 |       -48 |          0 |             |              |           |            | Removes 24-cell walk, two abbr/lockable/patch-addr table rows, EXT-mode redirect, `lock_page` cycling. Net delta depends on chosen home for EXT CC# editor: long-press on Track-page MMOD = cheapest; dedicated `PAGE_EXT_MIDI` = ~+150 B PROGMEM. RAM win if `MultiData.midi_cc_map` (48 B) shrinks to global. |
|     | **Totals (worst case if all land)**                  |        +1920 |        +5420 |        +2 |       +140 |        +400 |        +1100 |       +24 |        +72 |       |

---

## Headroom check

Applying the high column against current free space:

| Target           | Free now | High delta | Free after | Margin |
|------------------|---------:|-----------:|-----------:|-------:|
| controller flash |   8902 B |    +5420 B |     3482 B |  ~39 % of current free retained |
| controller RAM   |    945 B |     +140 B |      805 B |  comfortable |
| voicecard flash  |   6542 B |    +1100 B |     5442 B |  comfortable |
| voicecard RAM    |    995 B |      +72 B |      923 B |  comfortable |

All targets stay inside their budgets in the worst case. The pressure
points worth watching:

- **#11 mod matrix** dominates the controller-flash growth; if the four
  slots inflate (e.g. per-step modulation lanes added later) the high
  estimate is the one most likely to be exceeded.
- **#18 wavefolder** is the only issue that meaningfully grows voicecard
  flash. Land it with the CZ filter-sim slots still on the chopping
  block as a fallback if the build runs hot.
- **#29 performance page** is the single largest controller-flash
  consumer; co-landing #19 and #20 on top of it is cheap because the
  page infrastructure is amortized.

---

## Stub-build measurements (2026-05-22)

For the two heaviest issues, I built minimal stubs (page class with own
`event_handlers_`, registry entry, real event-handler bodies for pot
dispatch / screen render / LED paint, and concrete state storage) and
measured the actual delta. These pin down the *frame* — the fixed
overhead — and leave only the feature-specific logic as the unknown.

| Issue | Stub flash Δ | Stub RAM Δ | What the stub covers | Realistic ceiling on top |
|-------|-------------:|-----------:|----------------------|--------------------------|
| #29 Performance page | **+788 B** |  **+16 B** | 1 registry entry, own `event_handlers_`, OnInit/OnIncrement/OnClick/OnPot (8 pots routed to override + 6 offsets), OnKey (S1–S6 stash repeat rate, S7 toggle exclude mask, S8 exit), UpdateScreen rendering 8 values, UpdateLeds painting 6+2 LEDs, 11 bytes of state. | Beat-repeat punch-in + rate→ticks quantize-to-16th state machine, encoder scrub wired into playhead, override-scale fold into `FireStep` quantize (issue #20), transpose offset (#19), per-pot routing to patch params. Estimate **+400 to +900 B** on top → **#29 full ~1200–1700 B flash, ~25–40 B RAM**. |
| #11 Mod matrix       | **+806 B** |  **+18 B** | 2 registry entries (S4a + S4b sharing one handler set), own `event_handlers_`, OnIncrement cursor, OnPot routing 4 controls × 2 slots per page, UpdateScreen rendering 2 slot rows per page, `Evaluate(track)` walking 4 slots and writing dep-scaled value (wired into `FireStep` so it isn't DCE'd), 13 bytes of state. | Real src lookup table (LFO/env/velocity/CC/etc.), real dst dispatch into patch + voicecard params, abbr strings for src/dst names on screen, second page's screen variant. Estimate **+300 to +700 B** on top → **#11 full ~1100–1500 B flash, ~20–30 B RAM**. |

Conclusion: both fit comfortably. Even if the realistic-ceiling estimates
land at their high ends and both ship together, the combined hit is
roughly **2400–3200 B controller flash** against 8902 B free — leaving
5700+ B headroom. Neither is on the edge of feasibility.

Net update to the per-issue table above: the **C-flash high** column was
optimistic for #29 (had +1500 B; realistic is closer to +1700 B at the
ceiling) but pessimistic for #11 (had +1500 B; stub + ceiling lands
nearer +1500 B *total*, not on top of a separate frame). Both stay well
inside budget.

---

## Re-measurement protocol

After each issue lands:

1. `./toolchain/build-squeeze.sh controller/makefile`
2. `./toolchain/build-squeeze.sh voicecard/makefile`
3. `./toolchain/size-squeeze.sh controller && ./toolchain/size-squeeze.sh voicecard`
4. Update the baseline row above to the new used / free numbers.
5. Strike the issue's row through (or move into a "Landed" section)
   with the actual delta in parentheses next to the projection.
