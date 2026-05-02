# Elkhart — Phase 1 Evaluation

> **OBSOLETE — Kept for reference only.** This evaluation analyzed the Carcosa codebase before the project's pivot to YAM as the fork base. Findings about Carcosa-specific code structure, flash budgets anchored to West Coast oscillator measurements, and patch_data alias details no longer apply directly. Methodology, RAM/flash analysis approach, and general codebase reconnaissance patterns remain useful as templates for re-running the evaluation against YAM. The current authoritative spec is `SPEC_v2.md`.

Static analysis findings from pre-hardware codebase study (2026-04-27). Hardware (Michigan Synth Works Xena + SVF voicecards) arrives 2026-04-29. Items marked **[NEEDS HARDWARE]** require physical validation.

---

## Build System

**Toolchain:** avr-gcc 9.5.0 (Homebrew), avrdude 8.1. Both at `/usr/local/bin/`.

**Commands:**
```
make -f voicecard/makefile     # ATmega328p voice cards
make -f controller/makefile    # ATmega644p motherboard
```

`AVRLIB_TOOLS_PATH = /usr/local/bin/` is now set at the top of both sub-makefiles. No override needed on the command line.

**Submodules:** `.gitmodules` updated from deprecated `git://` to `git@github.com:` SSH URLs. `avrlib` cloned at pinned commit. `tools` pinned commit was orphaned upstream (force-push); resolved by fetching at `origin/master` HEAD (c95507f). Both submodules populated.

---

## Baseline Size Measurements (Carcosa v2.06, avr-gcc 9.5.0)

| Target | Device | Flash used | Flash total | % | RAM used | RAM total | % |
|--------|--------|-----------|-------------|---|----------|-----------|---|
| Voicecard | ATmega328p | 31,738 B | 32,768 B | 96.9% | 1,603 B | 2,048 B | 78.3% |
| Controller | ATmega644p | 61,242 B | 65,536 B | 93.4% | 3,706 B | 4,096 B | 90.5% |

These are within ~0.5% of the numbers in CARCOSA.md — the difference is compiler version (9.5.0 vs. whatever produced the documented figures).

---

## Voicecard Flash Analysis

### Per-object breakdown (pre-link, includes dead code)

| Object | Flash (text) | RAM (bss) | Notes |
|--------|-------------|-----------|-------|
| `resources.o` | 18,765 B | 0 | PROGMEM tables — see below |
| `voice.o` | 9,434 B | 1,251 B | All synthesis engines + envelopes |
| `oscillator.o` | 1,768 B | 0 | Classic oscillator waveform generation |
| `voicecard.o` | 1,194 B | 399 B | Main loop, SPI comm |
| `string.o` | 182 B | 0 | |
| `time.o` | 90 B | 5 B | |
| others | ~305 B | ~— | serial, adc, random, etc. |

### The `waveform_table` anchor effect

`resources.cc` ends with a `waveform_table[]` pointer array referencing all 30 waveform arrays. Because `oscillator.cc` looks up waveforms by index (`waveform_table[index]`), the linker's `--gc-sections` cannot eliminate any individual entry — referencing the table pulls in everything, including `wav_res_waves` (the raw wavetable binary: **10,320 bytes**).

The linker flag `--gc-sections` is active, but operates at the granularity of sections, not pointer-table entries.

**West Coast does not use `waveform_table`.** `westcoast.h` references `wav_res_sine` directly (line 207). Once `oscillator.cc` is stripped, `waveform_table` becomes dead, and `--gc-sections` eliminates all 30 arrays.

### Projected voicecard flash headroom after Phase 2 strip

Stripping `fm4op.h`, `karplus_strong.h`, and the classic oscillator (`oscillator.cc`):

| Freed | Approx size |
|-------|------------|
| `wav_res_waves` (wavetable binary) | 10,320 B |
| Band-limited square/saw/triangle tables (~14 × 257 B) | ~3,600 B |
| Formant sine/square + vowel data | ~600 B |
| FM4OP + KS engine code in `voice.o` | ~4,000–6,000 B |
| **Total** | **~18–20 KB** |

This brings voicecard flash from 31,738 B to an estimated **~12–14 KB used**, leaving **~18–20 KB of headroom**. The SPEC's earlier estimate of ~6.4 KB was based on the `voice.o` code delta alone; the `waveform_table` anchor effect makes the actual savings 3× larger.

Tables retained after strip (needed by West Coast + our additions):
- `wav_res_sine` (257 B) — West Coast sub-oscillator and sine LFO shape
- `wav_res_sine16` (512 B in `voice.cc`) — `InterpolateSine16` for FM
- `lut_res_oscillator_increments` (512 B) — pitch → phase increment
- `lut_res_lfo_increments` (256 B) — LFO rates
- `lut_res_env_portamento_increments` (256 B) — envelope rates
- `lut_res_vca_linearization` (512 B) — SVF voicecard VCA curve
- `lut_res_fm_frequency_ratios` (50 B) — A:B ratio lookup (25 entries × 2 B)
- `wav_res_distortion` (256 B) — fuzz/drive table, potentially useful
- `wav_res_env_expo` (257 B) — envelope curve

---

## Controller Flash and RAM Analysis

Controller flash (61,242 B) is the baseline for a complete replacement. The entire controller application layer (Multi, Part, UI pages, voice allocation) is being replaced by the elkhart sequencer. The numbers below characterize what's being discarded.

### Controller flash by object (top consumers)

| Object | Flash | RAM (bss) | Notes |
|--------|-------|-----------|-------|
| `ff.o` | 13,489 B | 4 B | FatFS — kept |
| `resources.o` | 8,426 B | 0 | Parameter strings, scale/arp tables |
| `part.o` | 6,671 B | 19 B | Part engine — replaced |
| `storage.o` | 4,514 B | 154 B | SD storage — largely kept |
| `parameter.o` | 4,472 B | 33 B | Parameter table — replaced |
| `ui.o` | 3,716 B | 454 B | UI dispatch — replaced |
| `controller.o` | 2,934 B | 353 B | Main loop — replaced |
| `library.o` | 2,367 B | 184 B | Patch library UI — replaced |
| `mmc.o` | 2,002 B | 3 B | SD card hardware — kept |
| `fm_page.o` | 1,690 B | 118 B | FM4OP UI page — gone |
| `env_page.o` | 1,473 B | 118 B | Envelope UI page — replaced |
| `randomizer.o` | 1,275 B | 114 B | Randomizer page — gone |
| `ks_page.o` | 1,213 B | 118 B | KS UI page — gone |
| `wc_page.o` | 1,201 B | 118 B | West Coast UI page — replaced |
| `multi.o` | 1,150 B | 1,993 B | Multi engine — replaced |

`multi.o`'s 1,993 B BSS is `Part parts_[6]` — the 6 Part objects' combined state.

### Strippable categories (controller)

| Category | Flash | RAM | Notes |
|----------|-------|-----|-------|
| Voice allocator (`voice_allocator.o` + `voice_assigner.o`) | ~1,120 B | ~240 B | Fixed 1:1 part→voice replaces entirely |
| Arpeggiator (in `part.cc`) | ~2,000 B | ~42 B | Our sequencer replaces this |
| Controller-side LFO management | ~500 B | ~200 B | LFOs move to voice card |
| Engine UI pages (fm/ks/wc/knob-assigner/randomizer/perf) | ~6,425 B | ~110 B | All replaced by our UI |
| Groove templates (6 tables × 16 × 2 B) | ~192 B | ~32 B | Single SWNG param replaces |
| Raga/scale tables (26 of 32 tables × 24 B) | ~624 B | 0 | Keep ~6 Western scales |
| Key range + note stack machinery | ~100 B | ~186 B | Not needed |
| **Total** | **~11 KB** | **~810 B** | Before parameter table shrink |

`parameter.o` (4,472 B) likely halves once FM/KS/WC/arp parameter definitions are removed.

**Net controller headroom estimate:** ~17 KB of flash available for the sequencer before writing a line. RAM: from 390 B headroom today to ~1,200 B — enough for the 2,590 B sequencer data model (which replaces 3,706 B of Carcosa state).

---

## LFO Independence from Classic Oscillator

The voicecard makefile already sets `DISABLE_WAVETABLE_LFOS`. The `Lfo::Render()` switch in `common/lfo.h` compiles to four pure-arithmetic shapes when this flag is set:

- `LFO_WAVEFORM_RAMP` — `phase_ >> 8`
- `LFO_WAVEFORM_TRIANGLE` — bit arithmetic on phase
- `LFO_WAVEFORM_SQUARE` — sign bit of phase
- `LFO_WAVEFORM_S_H` — `Random::GetByte()` on loop

None of these touch `waveform_table` or any PROGMEM table. Stripping `oscillator.cc` has no effect on LFO functionality.

The only shape we lose is **sine** — previously accessible via the disabled wavetable path. Adding a sine LFO case costs one `InterpolateSample(wav_res_sine, phase_ >> 8)` line; `wav_res_sine` is already retained for the West Coast sub-oscillator. Zero incremental flash cost.

---

## West Coast Oscillator Assessment

### What works

- **Fold algorithm** (`westcoast.h:Fold()`): iterative reflection at ±127 with quadratic gain curve. Algorithmically correct model of an analog wavefolder. 1× gain at `fold=0`, 33× at `fold=127` (~16 folds maximum).
- **FM quality**: `InterpolateSine16` uses a 512-entry 16-bit sine table with linear interpolation. Significantly better resolution than the 8-bit `wav_res_sine`. FM sounds clean.
- **`env_to_fold`**: sweeping fold depth from an envelope is the core Buchla pluck technique. Implemented correctly and wired to `MOD_SRC_ENV_1`.
- **Post-fold color filter**: 2-pole IIR LP (12dB/oct). Necessary at 39kHz to prevent harsh aliasing from high fold depths. Coefficient `lp_coeff = 16 + (color << 1)` gives min-16 to ensure fundamental always passes.
- **Symmetry + bias**: additive DC offsets into the fold point. Mechanically correct for shifting even/odd harmonic balance.
- **Self-sync**: `sync_phase_` runs faster than carrier and resets `phase_` on wrap. Produces metallic complex tones. Appropriate for percussion.

### Gaps relative to Brenso / DPO

- **Single audio path.** The modulator is FM-source-only — no independent audio output. Both reference instruments are true complex oscillators with two mixable signal paths. Our elkhart spec's dual-oscillator architecture (Osc1=modulator, Osc2=carrier, both with audio) addresses this.
- **No through-zero FM.** Phase increment never goes negative — at high FM depths produces "pinched" harmonic distortion rather than clean TZFM sidebands. Acceptable for percussion at moderate depths.
- **Integer-only FM ratios.** Ratio logic: `increment >> shift` (sub-harmonics) or `increment × integer` (harmonics). No fractional inharmonic ratios. Cuts off the most interesting FM space (beating near-integer ratios). **Fix:** wire `lut_res_fm_frequency_ratios` to the ratio parameter (table already exists in voicecard resources, currently used only by FM4OP).
- **FM modulator is sine-only.** Shaping the modulator waveform (square, triangle) produces qualitatively different sideband distributions. Unimplemented. Our WAV1 parameter addresses this.
- **`fold_stages` is a dead parameter.** Accepted in `Render()` signature (line 104) but never referenced in the render loop. Resolved: implement cascade folding (1–6 passes), expose as `FLDS` on Page 6b Bot 2 (voice config, non-lockable).

### Concrete dependency: `InterpolateSine16`

`InterpolateSine16()` and `wav_res_sine16` are defined in `fm4op.h` (lines 29–43 and voice.cc line 90). `westcoast.h` `#include`s `fm4op.h` solely for this function. **Before stripping FM4OP in Phase 2, move `InterpolateSine16` and `wav_res_sine16` to a shared header** (e.g., `voicecard/osc_utils.h`) so `westcoast.h` no longer depends on the FM4OP file.

---

## Open Questions

### Answered by static analysis

| # | Question | Answer |
|---|----------|--------|
| 4 | Build system / toolchain | avr-gcc 9.5.0 at `/usr/local/bin/`, `make -f voicecard/makefile` / `make -f controller/makefile` |
| 7 | Noise mix injection point | `westcoast.h` FM is computed as `fm_mod = (mod_val - 32768) * fm_depth >> 8` at line 159. Noise injection is a crossfade at this point: `fm_source = lerp(mod_val, noise_val, nois_param)` before the depth multiply. No structural change to the oscillator needed. |

### Partially answered

| # | Question | Status |
|---|----------|--------|
| 2 | West Coast oscillator flash size | voice.o is 9,434 B with all engines; WC in isolation estimated 3–5 KB after strip. **Confirm with Phase 1 profiling.** |
| 3 | Motherboard RAM after controller strip | Our sequencer model estimated at 2,590 B (37% headroom). Controller-side analysis supports this but actual usage needs profiling. **Confirm with Phase 1 profiling.** |

### Still open — requires hardware

| # | Question |
|---|----------|
| 1 | Filter mode (TYPE) switching glitch — does LP/BP/HP switching mid-note click on SVF hardware? Determines whether TYPE is per-step lockable. |
| 5 | SD card storage format — how are patches stored in Carcosa? Can we extend for pattern data or must we redesign? |
| 6 | Timer/interrupt structure — which timers are used for audio rate vs. UI vs. MIDI? Where does sequencer tick live? |
| 8 | CRSH parameter range — what crush divisor values give musically useful sample rate steps at 39kHz? |
