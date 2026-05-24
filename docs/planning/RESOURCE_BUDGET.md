# Resource Budget — Open Issues

Projected flash and RAM impact of the open GitHub issues, against the
current v4.2 baseline. Estimates are deltas in bytes; **low** is the
optimistic minimum (clean implementation, code reuse maximized) and
**high** is the pessimistic ceiling (independent surfaces, generous
tables, no reuse). Negative numbers indicate a net reclaim.

Re-measure with `./toolchain/size-squeeze.sh controller|voicecard`
after each landed issue and update both this table and the baseline.

---

## Baseline (v4.4 WIP, 2026-05-22, page reorg + LFO 5)

| Target     | Flash used | Flash free | RAM used | RAM free |
|------------|-----------:|-----------:|---------:|---------:|
| controller |    60156 B |     5380 B |   3300 B |    796 B |
| voicecard  |    26750 B |     6018 B |   1062 B |    986 B |

### v4.4 update (2026-05-23, wavefolder + sn16 sine bank)

| Target     | Flash used | Flash free | RAM used | RAM free |
|------------|-----------:|-----------:|---------:|---------:|
| controller |    60178 B |     5358 B |   3300 B |    796 B |
| voicecard  |    26642 B |     6126 B |   1068 B |    980 B |

- **Wavefolder** added a ~200-byte iterative-reflection loop in `Voice::ProcessBlock` plus ~32 B for the parameter row + lock wiring on the controller.
- **CZ filter-sim removal** dropped 3 render methods + the `wav_res_cz_phase_reset` LUT — ~530 B of voicecard flash reclaimed.
- **sn16 windowed sine bank** added a single `RenderSin16Bit` (~130 B) — net voicecard reclaim −108 B.

### v4.4 update (2026-05-24, mod-matrix removal + groovebox MIDI trim)

| Target     | Flash used | Flash free | RAM used | RAM free |
|------------|-----------:|-----------:|---------:|---------:|
| controller |    59066 B |     6470 B |   3342 B |    754 B |
| voicecard  |    26608 B |     6160 B |   1077 B |    971 B |

Session entry point: controller **61232 B** (93.8% full, 208 B from cap), voicecard 26664 B. Net session reclaim: controller **−2166 B**, voicecard **−56 B**.

- **Orphaned `wav_res_lfo_waveforms` deleted** from controller. The 2 KB PROGMEM table of Emilie's 16 custom MI LFO shapes (sine harmonics, gurgles, bat/folded, spiky, LP-filtered, stepped) has been orphaned on the controller since before the YAM fork — stock voicecard's table was already a 2-byte stub. Nothing on either side ever read the controller copy. Removed via `controller/resources/waveforms.py` → empty `waveforms = []`. Matching 2-byte voicecard stub also dropped. Net controller reclaim: −1904 B.
- **Mod matrix retired on voicecard**. `ProcessModulationMatrix()` (14-slot iteration) replaced with `ApplyModulations()` — four hardcoded direct paths (LFO 5 → user dest, LFO 4 → user dest, ENV 1 → VCA, VEL → VCA). Modifier loop in `LoadSources` deleted. `kDefaultMod` PROGMEM table and its `Part::Touch()` ship-loop deleted on the controller. Patch struct `modulation[14]` + `modifier[4]` retained as anonymous padding to preserve SD-card snapshot wire format (no `kVersion` bump).
- **Groovebox MIDI trim** (no Aftertouch / PitchBend / mod wheel CC). `Part::PitchBend`, `Part::Aftertouch`, `Part::ControlChange` mod-wheel case removed; matching `Multi::*` and `MidiDispatcher::*` handlers removed — parser falls through to empty `midi::MidiDevice` base impls. Dead `MidiDispatcher::SendPitchBend` removed (zero callers, predated cleanup). MIDI Thru / Chain / Full passthrough unaffected (`RawByte` and `RawMidiData` paths independent). Net for this trim: controller −182 B, voicecard −58 B.
- **`kSystemVersion` deliberately unbumped** per mid-sprint policy.

### v4.4 delta from v4.3 (full LFO 4 + LFO 5 stack)

| Target     |   Δ Flash |  Δ RAM |
|------------|----------:|-------:|
| controller |  +1004 B  | +38 B  |
| voicecard  |   +524 B  |  +9 B  |

Cumulative additions versus v4.3:
- **LFO 4**: clock sync (2-zone rate field), three one-shot shapes (1exp / 1lin / 1tri), retrigger gating, sync-rate `set_phase(0)` special-case for 1/96.
- **LFO 5**: second per-voice LFO mirroring LFO 4 (sync + one-shots + retrigger), routed via `MOD_SRC_LFO_2` and pre-configured mod-slot 6 (configurable dest/amount).
- **Trigger toggles**: `osc_phase_reset` and per-LFO `lfo4_retrigger` / `lfo5_retrigger` patch bytes flipped by encoder click on the OSC1 SHAPE / LFO4 SHAPE / LFO5 SHAPE cells, with ~500 ms transient `rst on / off` overlay.
- **Transport double-Stop** → broadcast `COMMAND_LFO_RESET`.
- **Sync rate ordering reversed** so the value-14↔15 boundary puts the slowest sync rate (1/1) adjacent to the slowest free-run rate.
- **Page layout reorg**: filter envelope moved from env page to filter page (bottom row); pitch envelope occupies the freed env-page bottom row; voice LFO page now has LFO 4 (top) and LFO 5 (bottom), each with rate / wave / dest / dept.
- **Cleanup**: relabelled dead `MOD_SRC_LFO_1`/`LFO_3` to `lfo.` (kept enum slots for patch compat); removed `kNumLfos` constant; snapshot version bumped to `0x05` (config slots widened from 29 to 31).

Controller RAM growth dominated by `kCfgSIZE` 29 → 31 (× 6 voices = 12 B) plus added parameter table entries.

## Previous baseline (v4.4 WIP, 2026-05-22, LFO 4 sync + one-shots only)

| Target     | Flash used | Flash free | RAM used | RAM free |
|------------|-----------:|-----------:|---------:|---------:|
| controller |    59568 B |     5968 B |   3283 B |    813 B |
| voicecard  |    26562 B |     6206 B |   1054 B |    994 B |

### v4.4-mid delta from v4.3 (LFO 4 sync + one-shot shapes + trigger toggles)

| Target     |   Δ Flash |  Δ RAM |
|------------|----------:|-------:|
| controller |   +416 B  |  +21 B |
| voicecard  |   +336 B  |   +1 B |

Controller: sync rate table, clock-tick dispatch loop, `osc_phase_reset`
and `lfo_retrigger` patch-byte plumbing (`Part::SetValue`, `Touch()`
sync list), encoder-click toggle handler on OSC1 shape + voice LFO
shape cells, transient "rst on / rst off" overlay (~300 ms), double-Stop
detection broadcasting `COMMAND_LFO_RESET`. RAM growth = `feedback_*`
static state + the 30-entry `lfo_phase_increment_per_clock_tick[]`
PROGMEM. Voicecard: one-shot render branch, sync-rate decoding, `tick()`
+ `LfoTick()` + `LfoReset()` paths, conditional osc_1.Reset and LFO
retrigger in `Voice::Trigger`. SPI: 6 single-byte ticks per MIDI clock
while transport runs, plus 6-byte broadcast on slow double-Stop.

Controller: `lfo_phase_increment_per_clock_tick[15]` table never landed
(voicecard owns the sync rate lookup); only multi.cc clock-dispatch loop
and `TickLfo()` inline send add bytes. Voicecard: one-shot render branch
(EXP/LIN/TRI), reordered LfoWave enum, sync-zone rate decoding, +1 B
`one_shot_done_` state in Lfo. SPI: 6 single-byte ticks per MIDI clock
when transport is running.

## Previous baseline (v4.3, 2026-05-22)

| Target     | Flash used | Flash free | RAM used | RAM free |
|------------|-----------:|-----------:|---------:|---------:|
| controller |    59152 B |     6384 B |   3262 B |    834 B |
| voicecard  |    26226 B |     6542 B |   1053 B |    995 B |

### v4.3 delta from v4.2

| Target     |   Δ Flash |  Δ RAM |
|------------|----------:|-------:|
| controller |  +2518 B  | +111 B |
| voicecard  |        0  |     0  |

RAM: 96 B per-lock PROB pool + 1 B count + 6 B per-track loop
counter (`kShdwLOOP`) + 6 B per-track SUBS gate (`kShdwSubs`) +
~2 B drill-in UI state. Flash: bipolar PROB table + eval; per-lock
PROB pool struct + serdes; drill-in UI on step page + EXT-track
CC view; SUBS PROB plumbing; substep editor layout rework; MINT
`chr` chord; snapshot `0x03` → `0x04` migration scaffolding.

### Previous baseline (v4.2, 2026-05-21)

| Target     | Flash used | Flash free | RAM used | RAM free |
|------------|-----------:|-----------:|---------:|---------:|
| controller |    56634 B |     8902 B |   3151 B |    945 B |
| voicecard  |    26226 B |     6542 B |   1053 B |    995 B |

---

## Per-issue projection

Columns are signed byte deltas. **C-flash / C-RAM** = controller; **V-flash
/ V-RAM** = voicecard. Empty cells = no impact expected.

| #   | Title                                                | C-flash low | C-flash high | C-RAM low | C-RAM high | V-flash low | V-flash high | V-RAM low | V-RAM high | Notes |
|-----|------------------------------------------------------|------------:|-------------:|----------:|-----------:|------------:|-------------:|----------:|-----------:|-------|
| 6   | Iterative probability mode for part steps            |        +400 |         +900 |        +6 |        +10 |             |              |           |            | ~53-entry CW slot LUT, per-track cycle counter (6 B), global FILL state (1 B), pot-zone dispatcher. Counter reset on transport stop. |
| 8   | Mixer page cosmetic fixes                            |         +30 |         +100 |           |            |             |              |           |            | Label re-layout + LED-color polarity swap on `seq_mixer_page`. Pure draw-side change. |
| 12  | Clock-sync rates for LFO 4                           |        +100 |         +300 |           |         +2 |             |              |           |            | Extend LFO 4 rate field to include sync slots; reuses existing sync-rate table from LFO 1–3. *(Landed in v4.4.)* |
| 18  | Wavefolder waveform                                  |         +10 |          +40 |           |            |        +200 |         +500 |           |            | Iterative quadratic fold; one wave-enum slot. **Risk:** voicecard already 80 % full — landing this may force dropping the reserved CZ filter-sim slots referenced in #18. |
| 19  | Performance: master transpose                        |         +60 |         +150 |        +1 |         +2 |             |              |           |            | Depends on #29 page existing. Bipolar `-12..+12` applied pre per-track quantize in `Sequencer::FireStep`. |
| 20  | Performance: master scale                            |        +100 |         +250 |        +1 |         +2 |             |              |           |            | Depends on #29. Reuses existing `kScaleMasks[]` palette + `---` sentinel; overrides per-track SCAL in fire path. |
| 25  | Track clear function on unused S6a pot               |        +200 |         +400 |        +2 |         +4 |             |              |           |            | Five clear modes (locks / steps / notes / voice / all) + 800 ms long-press-to-arm + tap-to-confirm state machine. |
| 29  | Performance page on S7b (frame: overrides + repeat)  |        +800 |        +1500 |        +8 |        +24 |             |              |           |            | New page registry + event handler, 8 override/offset pots, S1–S6 beat-repeat (1, 1/2, 1/4, 1/8, 1/16 bar) with quantize-to-16th punch-in, encoder scrub, S7 exclude-mask, S8 exit. State: repeat rate, exclude bitmask, scrub anchor. |
| 31  | Sub-step editor un-selects on button release         |         +20 |          +80 |           |            |             |              |           |            | Suppress the release event for the held step that opened the editor. Local fix in `seq_step_substeps_page` / step-button latch. |
| 32  | Retire S5b/S5c; relocate EXT-track CC# editing       |        -400 |         +200 |       -48 |          0 |             |              |           |            | Removes 24-cell walk, two abbr/lockable/patch-addr table rows, EXT-mode redirect, `lock_page` cycling. Net delta depends on chosen home for EXT CC# editor: long-press on Track-page MMOD = cheapest; dedicated `PAGE_EXT_MIDI` = ~+150 B PROGMEM. RAM win if `MultiData.midi_cc_map` (48 B) shrinks to global. |
| TBD | Per-track loop counter (`kShdwLOOP`)                 |         +30 |          +80 |        +6 |         +6 |             |              |           |            | Add `kShdwLOOP` to `SeqTrack.shadow[]`, increment on pattern wrap. Transient, no snapshot impact. Shared infra for chord mutation + randomness submenu. |
| TBD | Chord mutation across pattern loops                  |        +150 |         +300 |         0 |          0 |             |              |           |            | When `SSUB=0` and chord shape set, advance chord-walk on loop wrap instead of substep. Uses `kShdwLOOP`. |
| TBD | `kSmodSfx` skip-unless-jumped step modifier          |        +100 |         +250 |         0 |          0 |             |              |           |            | New SMOD value (slot 14 of 16). Step is skipped during normal iter, fires when a jump SMOD lands on it. |
| TBD | Per-lock randomness submenu + `RandPool`             |        +830 |        +1370 |      +225 |       +242 |             |              |           |            | New `RandPool` (32 × 6 B entries + count byte), 8-entry walk-state pool (32 B), drill-in editor with shared `RenderPercent` path + 4-entry DIR string table, four fields per packet (`PROB`, `RND`, `DIR`, `RNGE`). Drill-in gated by `kRandomizableParams` PROGMEM bitmask (~15 of 28 lockables). Held-step label inversion: first letter for lockable cells, whole label for randomizable cells. Subsumes #6 — closes it. Snapshot bumps 0x03 → 0x04 with migration. |
|     | **Totals (worst case if all land)**                  |        +3030 |        +7420 |      +233 |       +388 |        +400 |        +1100 |       +24 |        +72 |       |

---

## Headroom check

Applying the high column against current free space:

| Target           | Free now | High delta | Free after | Margin |
|------------------|---------:|-----------:|-----------:|-------:|
| controller flash |   8902 B |    +7420 B |     1482 B |  tight — see notes |
| controller RAM   |    945 B |     +388 B |      557 B |  comfortable |
| voicecard flash  |   6542 B |    +1100 B |     5442 B |  comfortable |
| voicecard RAM    |    995 B |      +72 B |      923 B |  comfortable |

All targets stay inside their budgets in the worst case. The pressure
points worth watching:

- **Controller flash** is now the tightest target if every projected
  issue lands at its high estimate. The randomness submenu (`RandPool`
  + drill-in UI) and #29/#11 together account for most of the growth.
  Realistic ceilings (see stub-build section) trim ~1500 B off the
  worst case, putting actual landing closer to ~6200 B used, ~2700 B
  free. Re-measure as each lands.
- **#11 mod matrix** dominates among the existing issues; if the four
  slots inflate (e.g. per-step modulation lanes added later) the high
  estimate is the one most likely to be exceeded.
- **#18 wavefolder** is the only issue that meaningfully grows voicecard
  flash. Land it with the CZ filter-sim slots still on the chopping
  block as a fallback if the build runs hot.
- **#29 performance page** is the single largest controller-flash
  consumer among the existing issues; co-landing #19 and #20 on top of
  it is cheap because the page infrastructure is amortized.
- **Randomness submenu** is the single largest RAM consumer in the new
  batch (+193..210 B); its 32-entry `RandPool` capacity is the main
  tunable if pressure shows up — drop to 16 for −96 B.

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

---

## Cleanup possibilities (unprojected)

Captured during the 2026-05-24 mod-matrix cleanup session. These are *not* gated on any planned feature — they're standing reclamation candidates ordered by hunch (high-confidence-quick-win → speculative-but-large). Estimates are rough; verify before committing.

### Quick wins (high confidence, low risk)

**`VOICECARD_DATA_MODULATION` SPI command — likely unused after 2026-05-24.** All three controller-side callers (`Part::PitchBend`, `Part::Aftertouch`, `Part::ControlChange` mod-wheel case) were deleted. If nothing else sends the command, the voicecard's RX dispatch case + `voice.set_modulation_source` glue is dead. Estimate: **~50-150 B voicecard**, ~10 B controller. Verify by grep, then drop the RX case + the SPI command enum slot if also unreferenced.

**`MOD_SRC_*` init writes in `Voice::ResetAllControllers`.** Defensive inits for `PITCH_BEND` / `AFTERTOUCH` / `WHEEL` / `WHEEL_2` / `EXPRESSION` and the `CONSTANT_*` ladder — none of these slots have consumers post-mod-matrix-removal. Estimate: **~20-40 B voicecard**. Cosmetic; safe to drop.

### Discrete trade-offs

**FatFs trim** (`avrlib/third_party/ff/ffconf.h`). Setting `_USE_MKFS=0` drops on-device SD-card format (currently exposed via Card Info page → FORMAT action). Setting `_FS_MINIMIZE=1` additionally drops `f_mkdir` (used by `snapshot.cc:104` `Storage::fs_.Mkdirs(path)`), `f_unlink`, `f_truncate`, `f_rename`, `f_stat`, `f_getfree` (the last one is consumed by the Card Info page's free-space display). Estimate: **~1-2 KB controller** total. Requires removing the FORMAT call site and either pre-creating the snapshot directory tree or working around `Mkdirs` deletion. Free-space display is a separate UX call.

**Parameter table index shift (issue #47, already filed).** Deletes orphan `parameters[]` entries 34..41 (the retired mod-matrix UI parameters), shifts every subsequent ID down by 8, cascades through `kParamLockMap`, hardcoded IDs in `parameter_editor.cc`, MIDI CC/NRPN maps. Unblocks deletion of `UNIT_MODULATION_SOURCE` / `UNIT_MODIFIER` format cases, the now-orphan source/destination/modifier strings (including `lfo 1` / `lfo 3` long labels), and the `MOD_SRC_PITCH_BEND` / `MOD_SRC_WHEEL` / `MOD_SRC_AFTERTOUCH` enum slots. Estimate: **~500-900 B controller**. Mechanical but invasive; needs hardware verification on every patch-page cell.

### Speculative (uncertain magnitude)

**C++ static initializers — `_GLOBAL__I_*` stubs.** `avr-nm` shows ~16 translation units each emitting a ~156 B `_GLOBAL__I_*` static-init stub from non-trivial constructors (event-handler tables, `lcd`, `leds`, `multi`, `sequencer`, page registries). If the offending constructors can be made trivial (BSS zero-init + explicit `Init()` calls), libgcc's per-TU init machinery drops. **Theoretical ceiling ~2.5 KB** controller; realistic recovery probably half that. Needs per-TU disassembly to identify which globals carry the cost.

**`SeqStepsPage` size reduction.** Largest single page in the binary: `UpdateScreen` 2410 B + `OnPot` 2088 B + `OnClick` 998 B = **~5.5 KB combined**. WS1's lockable expansion ballooned the per-cell switch dispatch. PROGMEM dispatch tables or merged display/edit paths could shave **~300-800 B** without behavior change. Big refactor; high test surface.

### Investigative (probably small individually, may add up)

**`controller/voicecard_tx` command audit.** Every `VOICECARD_DATA_*` we no longer send is dead bytes both sides of the SPI wire. After 2026-05-24's deletions, survey which commands have zero remaining callers and prune.

**MIDI Out helpers in `midi_dispatcher.h`.** `SendCc`, `SendSlotCc`, `SendVamtCc`, `SendGlidCc`, `Send3`. Audit each for callers — `SendPitchBend` was already dead before today; sibling helpers might be too.

**`modulation_sources_[]` and `modulation_destinations_[]` array sizing on the voicecard.** `kNumModulationSources = 32` but ~12 slots have zero readers post-mod-matrix removal (LFO_1, LFO_3, OP_1..OP_4, AFTERTOUCH, WHEEL, WHEEL_2, EXPRESSION, PITCH_BEND, the CONSTANT_* ladder). Pruning is gated on the parameter-table shift (issue #47) since the enum slots are addressed by index. Per-slot reclaim is small (1 B RAM + tiny init code) but they accumulate.

**Voicecard's `init_patch` table.** 112 bytes of hardcoded PROGMEM defaults; mod-matrix slots now write to bytes no consumer reads. Zeroing them out is cosmetic (no save) but worth doing for clarity if any of the surrounding cleanup lands.

### Out of bounds

DSP path (oscillators, envelopes, filter), `avrlib/` proper (shared with hypothetical other forks), and audited resource tables (most are load-bearing). Risk/reward is bad in those areas; leave them alone unless a specific feature ask forces a revisit.
