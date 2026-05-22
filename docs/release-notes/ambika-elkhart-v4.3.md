# ambika-elkhart v4.3

Probability becomes a first-class field across the sequencer. Every step's PROB cell can now express either a percentage roll or a polymetric "fire on cycle X of N" pattern in the same byte. Every overlaid parameter lock, the SMOD modifier, the substep machinery, and the intrinsic VEL / GLID fields can each carry an independent PROB gate. The result: one 8-step pattern can host firing probabilities that alternate, retrigger every third loop, walk a chord chromatically, and conditionally apply per-parameter overrides — all composed multiplicatively from a small number of cells. v4.2 snapshots load unchanged.

## Highlights

- **Bipolar PROB cell.** A step's PROB pot is now bipolar. CCW = standard random % roll (0..100%). Center = always fire (`100`). CW = cycle-phase iterative — pot positions display as `1:2`, `2:2`, `1:3`..`8:8`, `!1:3`..`!6:6`, `FILL`, `!FIL`. `X:N` fires on loop X of every N pattern wraps; `!X:N` fires every loop except X of N. Each track maintains its own loop counter that advances on natural pattern wraps and on every jump SMOD. A single PROB byte replaces what used to need separate "probability" and "iterative" modes.

- **`chr` chord shape.** New `MINT = chr` walks all 12 semitones — useful as the chord source for melodic patterns where any tone is fair game.

- **Chord walk on pattern loop.** With `SSUB = 0` and `MINT > 0`, the chord-walk eval now advances by the per-track loop counter instead of by sub-trigger. Each pattern loop plays the next chord tone at the step. Octave-walk (`MOCT`) and direction (`MDIR`) behavior preserved. Set up a chromatic walk on a single step and let it climb across loops.

- **`eskp` SMOD.** New step modifier `eskp` (explicit-skip). Steps set to `eskp` are skipped during normal iteration but fire when another step's `jmp[N]` or `rjmp` reseats the playhead onto them. Used to hide variation hits inside a pattern that only play when explicitly summoned.

- **Per-lock probability.** Hold a step, cursor to a pool-backed cell (any synth-param lockable, plus the intrinsic VEL / GLID / SMOD / SUBS cells), click the encoder, and the cell flips to PROB-edit mode. The pot now writes a bipolar PROB byte attached to *that* lock for *that* step. On roll fail at fire time, the lock doesn't apply that loop and the track default plays through. Cells with a non-default PROB render their label uppercase as a "this lock is gated" indicator. Composes multiplicatively with step PROB: a step might fire on alternating loops while its FREQ override applies on a separate three-loop cycle, the velocity humanizes only every fourth loop, and the jump only fires every third visit.

- **Per-lock PROB on MIDI CC.** EXT-track CC slots (S5b / S5c) honor per-lock PROB the same way — drill into a slot, set a PROB, and that CC override only sends when the roll passes (otherwise the track default CC value sends instead). Compose polymeter directly into outbound MIDI.

- **SUBS probability + substep editor rework.** The substep editor adds a SUBS PROB cell (pot 4, top-left of the bottom pot row). On roll fail, ratchets / repeats / chord-walk are suppressed for that loop while the main step still fires (subject to step PROB). The editor's layout matches the rest of the page: 4 cells per line, no trailing gaps. Substep on/off state moves entirely to the S1..S8 LEDs (green = active and firing, red = active and suppressed, dark = inactive slot) — the textual indicator row is gone. Encoder is modal inside the editor.

- **SUBS labels show total fires.** `SSUB=0, REPT=0` displays `1x` (1 fire, just the main step). `SSUB=1` displays `2x` (main + 1 ratchet = 2 fires). Range `1x..8x` for ratchets / `1x..8r` for repeats. The pot caps at 8 total fires so the editor's 8 slot indicators always match.

- **Substep `bit 0` correctly gates the main fire.** In ratchet mode the first slot's LED now controls whether the main step fires at all, matching what the editor's UI implies. Previously the first slot was visual-only for ratchets.

## Drill-in gesture order

The encoder click that enters PROB-edit mode needs the cursor to be on the target cell *before* you hold the step. Sequence:

1. Turn the encoder to put the cursor on the cell you want to gate.
2. Hold the step button.
3. Click the encoder. The cell switches to PROB display.
4. Turn that cell's pot to set the PROB.
5. Release the step (or click again) to exit.

Holding the step first and then turning the encoder won't move the cursor — step buttons act as encoder modifiers and consume the turn for shortcut handling.

## What didn't make v4.3

The continuous-parameter drift fields from the original randomness-submenu issue (RND / DIR / RNGE / WALK across ~15 params) are deferred. A beat-synced sample-and-hold LFO routed through the mod matrix (coming in v4.4) covers that use case more musically and at lower flash cost than a per-step UI for it. v4.3 ships the parts that the mod matrix can't replace: cycle-phase step PROB, conditional lock overlays, and PROB on the intrinsic / SMOD / SUBS gates.

`FILL` / `!FIL` PROB slots are stubbed pending the PERF page. `FILL` always evaluates to "not firing"; `!FIL` always evaluates to "firing." Both will become useful once a momentary fill button lands.

## How to flash

Copy `firmware/latest/AMBIKA.BIN` to the root of the Ambika SD card and follow the standard OS update procedure. Voicecard binary is unchanged from v4.2 — no need to reflash voicecards.

## Resource usage

Controller: **90.3%** flash, **79.6%** RAM. Up from 86.4% / 76.9% in v4.2. The increase covers the 96 B per-lock PROB pool + 6 B per-track loop counter + 6 B per-track SUBS gate, plus the bipolar PROB table and drill-in UI.

Snapshot format bumps to `0x04`. v4.2 (`0x03`) snapshots load unchanged with the new per-lock PROB pool zero-filled; existing locks behave exactly as before.

## Known limitations

- `FILL` / `!FIL` cycle-phase slots are stubbed until the PERF page exists. Setting them now produces always-off / always-on respectively — not the intended momentary-fill behavior.
- Per-lock PROB drill-in is available only on the step page and the EXT-track CC view. The patch-page parameter editor doesn't render PROB drill-in (the prob byte still gates fires correctly, but the patch page won't show it).
- `RATE` cell click is reserved for raw-tick-mode toggle and doesn't support per-lock PROB drill-in.

## Acknowledgements

- **YAM** (`bjoeri/ambika`) — upstream fork point.
- **Mutable Instruments / Émilie Gillet** — original Ambika firmware and hardware.
- **avrlib** (`pichenettes/avril`) — vendored, GPLv3.
- Developed with AI assistance (Claude Code).

## License

GPLv3, inherited from upstream Mutable Instruments / YAM.
