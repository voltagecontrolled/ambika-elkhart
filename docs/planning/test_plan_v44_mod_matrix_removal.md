# Test plan — v4.4 mod-matrix removal (controller + voicecard slot 1)

Build staged in `firmware/latest/`:

- `AMBIKA.BIN` — controller, 59,248 B (was 61,392 B; -2,144 B, 90.4% full)
- `VOICE1.BIN` — voicecard, 26,608 B (was 26,662 B; -54 B, 81.2% full)
- `VOICE2.BIN`..`VOICE6.BIN` — unchanged from prior build (for A/B comparison against slot 1)

`kSystemVersion` was deliberately not bumped, so the OS Info page will show identical version bytes across all slots — distinguish slot 1 from 2-6 by behavior, not by version display.

## What changed

1. Orphaned controller-side `wav_res_lfo_waveforms` PROGMEM table removed (2 KB).
2. Voicecard `ProcessModulationMatrix()` (14-slot iteration over `patch_.modulation[]`) replaced with `ApplyModulations()` — five hardcoded direct paths:
   - LFO 5 → user-selected destination, amount from L5A
   - LFO 4 → user-selected destination, amount from L4A
   - ENV 1 → VCA (multiplicative), amount from E1DEPT
   - VELOCITY → VCA (multiplicative), amount from byte 85
   - MIDI pitch bend → OSC_1_2_COARSE, amount from byte 88 (`patch_.modulation[12].amount`) — added after initial hardware test confirmed pitch bend was running through the now-deleted matrix slot 12
3. Voicecard modifier feature (4 computed sources via OP enum) removed entirely — the `LoadSources()` modifier loop is gone.
4. Controller `kDefaultMod` PROGMEM table + the `Part::Touch()` for-loop that shipped 42 bytes of mod-matrix defaults are gone.
5. Patch struct unchanged on disk (`modulation[14]` + `modifier[4]` retained as anonymous padding, preserving 112 B size + all byte offsets).

## What did NOT change

- Snapshot file format (`kVersion` still 0x07).
- All migration paths (v0x01..v0x06 still load via existing branches).
- Controller-side mod-matrix UI parameters (`PRM_PATCH_MOD_SOURCE` / `DESTINATION` / `AMOUNT`, `PRM_PATCH_MOD_OPERAND*`, `PRM_PATCH_MOD_OPERATOR`) — these still exist as orphans in `parameters[]` but are unreachable since `PAGE_MODULATIONS` was retired in v4.2. Tracked by issue #47.
- LFO 1/3 string remnants — still in resources, blocked by issue #47.

## Pre-flight

- [ ] Save current snapshot from device to SD card via `M -> SAV` (insurance).
- [ ] Note current track defaults for one expressive patch (LFO 4 dest/depth, LFO 5 dest/depth, E1 depth, velocity response) — use these as the A/B reference between slot 1 and slots 2-6.

## Golden path

Flash slot 1 only. Leave slots 2-6 on the prior firmware.

- [ ] Boot — all six voices come up (`port N device 1.0` or similar on every slot in OS Info).
- [ ] Load any existing v0x07 snapshot — voices play; patch struct round-trips fine (modulation[] / modifier[] padding is silently ignored).
- [ ] Play a held note on slot 1 — VCA opens via ENV 1 (E1DEPT default = 127). If voice is silent, ENV→VCA is broken — this is the highest-impact failure mode.
- [ ] Vary velocity hard vs. soft on slot 1 — VCA scales (byte 85 = VELOCITY→VCA amount, default 127). If velocity is inert on slot 1 but works on slots 2-6, the VEL→VCA path is broken.

## LFO 4 routing

- [ ] On the voice LFO page, slot 1 patch: LFO 4 dest = PARAMETER_1, depth > 0 — hear OSC1 PARAM1 wobble.
- [ ] Sweep LFO 4 depth pot from 0 → max — modulation depth scales smoothly.
- [ ] Sweep LFO 4 rate pot — modulation rate changes smoothly across both synced (0..14) and free-run (15..142) zones.
- [ ] Change LFO 4 dest to CUTOFF — modulation moves to filter cutoff.
- [ ] Change LFO 4 dest to VCA — tremolo (multiplicative, not additive — verify the volume modulation feels right, not just additive bias).
- [ ] Click LFO 4 SHAPE cell — retrigger toggle still works (`rst on`/`rst off` overlay appears).

## LFO 5 routing

- [ ] Repeat the LFO 4 sequence above for LFO 5 (`patch_.modulation[6]` path). Same expected behavior.
- [ ] One-shot shapes (`1exp`, `1lin`, `1tri`) still fire and hold at zero — these are LFO 4/5 envelopes-by-design.

## Sequencer / locks regression

- [ ] On a patch with at least one per-step lock on a value-locked cell (VEL, NOTE, FREQ, anything), play the sequence — locks fire correctly on slot 1, sound identical to slot 2.
- [ ] Drill-in PROB on a sequencer-step lock (`hold step + click encoder + turn pot` from the patch-page cell) — gesture still works.
- [ ] Long-press encoder on a lockable cell — `clr locks` overlay fires, all step locks for that param clear.
- [ ] Substep editor (chord walk on loop, ratchet) — fires correctly.

## Filter behavior

- [ ] ENV 2 → cutoff via the FAMT/FLT cell — sweeps cutoff normally.
- [ ] Filter LFO cell (`filter_lfo` field) — still modulates cutoff (this uses `modulation_sources_[MOD_SRC_LFO_2]` inside `UpdateDestinations` — overlaps with LFO 5 source but is a dedicated path).
- [ ] Resonance, mode, drive (BITS/CRSH/FUZZ/FOLD) — all behave normally.

## MIDI

- [ ] Note on/off → voice plays / releases normally.
- [ ] Pitch bend wheel → pitch bends on slot 1 voice. Reads `patch_.modulation[12].amount` (preserved from saved snapshots). First build of this plan had pitch bend broken because the plan incorrectly claimed pitch bend was folded into `pitch_value_`; second build restored it.
- [ ] Mod wheel — was previously routed through mod matrix slot 13's wheel-scaling special case; with the matrix gone, mod wheel now has no effect. **Expected regression**; flag if a patch relied on it.
- [ ] Aftertouch — was previously a mod-matrix source; with the matrix gone, aftertouch has no effect. **Expected regression**.

## Known expected differences vs. slots 2-6

- Mod wheel and aftertouch are no longer routable. Any patch that used them via the mod matrix UI will sound inert on slot 1. Both routings can come back later as direct paths if needed.
- The mod matrix UI on the controller is still navigable (orphan parameter entries) but its edits will have no audible effect on slot 1. Pots / encoder changes there will silently no-op. Issue #47 deletes the UI.
- The modifier UI is similarly orphan / inert.

## Roll-back

- Restore prior `firmware/reference/AMBIKA.BIN` and `firmware/reference/VOICE1.BIN` to the SD card and re-flash.
- No EEPROM or SD-card data format changed, so rollback is clean.

## Sign-off

- [ ] Golden path passes on slot 1.
- [ ] LFO 4/5 routing all-clear on slot 1, matches slot 2 sonically (modulo any patches that exercised mod wheel / aftertouch / non-default mod-matrix routings).
- [ ] No silent voices, no stuck notes, no LCD freezes during a 10-minute jam.

If all green, flash voicecards 2-6 with `VOICE1.BIN` (rename per slot) and ship as the v4.4 baseline.
