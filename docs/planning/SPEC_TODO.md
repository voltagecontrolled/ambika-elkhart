# SPEC TODO

> **OBSOLETE — Kept for reference only.** Design decisions captured here were folded into the prior Carcosa-based `SPEC.md`. Both predate the project's pivot to YAM. The current authoritative spec is `SPEC_v2.md`. Preserved as a source of pending-decision ideas that may apply to the YAM-based redesign.

Design decisions from 2026-04-28/29 session, not yet written into SPEC.md or manual.

---

## LPG Macro (replaces LPGD on sequencer)

Single bipolar parameter `LPG`, step-lockable. Replaces LPGD on Seq Voice Page 1 Bot 1.

- **CW:** sweeps vactrol character — internally derives `shape_fast` and `shape_slow` for both FG-A and FG-B, with filter curves sweeping wider/faster than amp curves
- **CCW:** same character sweep but with progressive attack slope added to both FG-A and FG-B. At full CCW: full swell/bow character — useful for comb filter excitation
- **Center:** neutral vactrol default

Two-slope decay model:
```c
float env_curve(float phase, float shape) {
    return powf(1.0f - phase, shape);
}

tail_mix = smoothstep(0.35f, 1.0f, d) * 0.35f;
vca = (1.0f - tail_mix) * fast_decay + tail_mix * slow_decay;
```
- `shape_fast` and `shape_slow` both derived from macro position at trigger time
- Filter tail attenuated relative to VCA tail (~0.6×) — highs die before body
- AVR implementation: compute derived shape values once on trigger, use precomputed lookup table in audio loop

Voice-level default position lives on Page 2 (non-lockable knob). Unlocked steps read the physical knob.

---

## Page 2 Layout Changes

DOFS and COFS absorbed into LPG macro — those slots freed. Proposed layout:

| Knob | Abbrev | Notes |
|------|--------|-------|
| Top 1 | `FREQ` | Cutoff |
| Top 2 | `RES ` | Resonance |
| Top 3 | `TYPE` | LP/BP/HP — pending hardware glitch test |
| Top 4 | `FDEP` | LPG depth (how much FG-B moves overall) |
| Bot 1 | `LPG ` | Voice default macro position |
| Bot 2 | `FFLR` | Filter floor — how closed the filter gets at rest. Determines ring brightness during comb filter decay. |
| Bot 3 | `DRIV` | Filter drive |
| Bot 4 | `BITS` | Bit reduction |

---

## Filter Floor (FFLR)

Minimum cutoff value FG-B cannot go below. Controls brightness of the resonator ring stage.
- Low FFLR: filter closes completely, dark ring
- High FFLR: filter stays partially open, bright harmonically rich ring-out
- Non-lockable voice setting. Works in concert with FDEP.

---

## Comb Filter Resonator

Pitch-tracked comb filter as insert effect after WC engine output.

- WC output is the exciter — can be complex FM transient or noise burst (NOIS full CW + low TIMB)
- Delay line length tracks pitch — resonant frequency matches fundamental
- Minimum tracked pitch sets delay buffer size (e.g. MIDI 48 ~196Hz → ~200 bytes)
- Feedback coefficient controls ring duration
- CCW LPG attack zone feeds sustained signal into resonator → bowed/swelled behavior
- New parameters TBD — where these live in the page layout not yet decided

---

## Velocity Routing

Velocity always offsets LPG macro position at trigger time — harder hits push toward brighter/faster vactrol character. This is physically accurate (vactrol responds to excitation amplitude).

**Open question:** whether user can additionally route velocity to other destinations (pitch, fold, FM depth) alongside the LPG offset, or whether LPG is the only velocity target.

- If additional routing kept: need to decide whether VDST stays configurable, and whether VDEC (velocity transient decay) is still needed or collapses into the LPG macro shape
- If LPG-only: VDST, VDEC, and FG-C (third envelope instance) all go away — significant simplification. Velocity sensitivity controlled by single `VSNS` parameter on 6b.

**Not decided yet — marinate.**

---

## SHAP CGRAM Glyphs

Waveform shapes for CGRAM (3 of 8 slots):
- Waveforms rotated 90° (time runs top-to-bottom) — sine as S-curve, triangle as zigzag, square as hard step
- Validated against Elektron Analog Rytm approach (their glyphs are 5×9, essentially same resolution as HD44780 5×8 CGRAM)
- Combo displayed as `[glyph]+[glyph]` using `+` as ASCII separator

---

## TYPE (Filter Mode) — Pending Hardware

Still need to test whether LP/BP/HP switching mid-note produces an audible click on SVF hardware. If it clicks, TYPE moves to 6b (non-lockable). If clean, stays on Page 2 as-is.

Separately under consideration: swap TYPE to 6b regardless, replace Page 2 Top 3 with FLDS (fold stages) as a step-lockable parameter. Defer until hardware confirmed and TIMB/FLDS interaction is understood.
