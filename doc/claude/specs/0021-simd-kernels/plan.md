---
spec: 0021-simd-kernels
title: Portable SIMD kernels
status: done          # closed 2026-08-25
created: 2026-07-19
author: Alex Spataru
---

# 0021 — Portable SIMD kernels (plan)

Status: approved-by-commission (user requested identify -> plan -> implement in one autonomous
pass, 2026-07-19). Identification ran as an 8-agent audit of `app/src`; this plan is the
synthesis. No spec.md: the WHAT/WHY is one sentence — vectorize the bulk loops the compiler
provably cannot, identically on x86-64-v2 (SSE4.2 baseline) and arm64 (NEON), with zero
tolerance for value drift.

## Constraints (binding)

- No fast-math, no reassociation: every kernel is per-lane identical to its scalar loop.
  Allowed op classes: byte compares/masks, integer ops, IEEE min/max compares, per-lane
  add/mul in the scalar's exact order, per-lane f64->f32 converts. The only documented
  divergence class: `min(-0,+0)` sign selection, which no caller observes.
- ISA floor: x86-64-v2 (SSE2..SSE4.2, SSSE3 pshufb OK) — matches `cmake/Optimization.cmake`;
  armv8-a NEON (incl. float64x2). Anything else (AVX2, PCLMUL/PMULL) is out: no runtime
  dispatch in this pass.
- One home for all kernels: `app/src/DSPSimd.h`, `namespace DSP`, detection macros
  `SS_SIMD_X86` / `SS_SIMD_NEON` in the HotpathOptimization.h cascade style, scalar fallback
  always present and always the reference semantics.
- Hotpath rules apply to the span-lane site (TextTemplates): allocation-free, bounded loops,
  x86 codegen path preserved (same instruction sequence family), NEON added.

## Kernels (DSPSimd.h)

| Kernel | Semantics | Consumers |
|---|---|---|
| `simdForEachByteMatch(data, len, needle, fn)` | ascending positions of `needle`; `fn` false aborts | TextTemplates `splitSpansSingleByte` (replaces local SSE2 block, adds NEON) |
| `simdMinMaxF64(p, n, lo, hi)` | seeded `p[0]`, predicate `v<acc` / `v>acc` (NaN-sticky like scalar) | ReportData `appendBucketSamples` (+ scalar first-index recovery), DSP `dsColumnYBounds` |
| `simdFiniteMinMaxStrided2(pts, n, lane, lo, hi)` | in-out accumulate over QPointF lane, skips non-finite | Plot `updateDataExtremes` / `computeMinMaxValues`, MultiPlot `scanCurvesForRange`, PlotCurve `runVisible` |
| `simdWindowedComplexFill(ring, front, mask, n, offset, scale, win, out2)` | `v = isfinite(r) ? f32((r+offset)*scale) : 0f; out2[2i]=v*win[i]; out2[2i+1]=0` — ring split to 2 contiguous spans | FFTPlot + Waterfall `updateData` FFT staging |
| `simdRingsToPoints(xraw, xfront, xmask, yraw, yfront, ymask, n, out)` | dual-ring gather -> QPointF interleave | Plot `updateData` XY mode, non-log branch only |
| `simdFindAnyByte(p, len, needles, count)` | first index in byte set (<=8 needles) | ZMODEM `zdleEncode` (bulk-copy clean runs) |
| `simdHexPairs16(p, out32)` + `simdPrintableMask16(p)` | 16 bytes -> 32 UTF-16 lowercase hex chars (pshufb/tbl); printable bitmask `0x20<=b<0x7F` | Console `hexadecimalStr` full rows (scratch row + one append; `std::isprint` -> range test, matching `plainTextStr`) |

## Call-site edits

Disjoint per-file changes, each preserving byte/bit-identical output (Console: identical
except `isprint` locale dependence, deliberately aligned to the sibling function's range
test). `Plot3D.cpp` additionally gets `SS_RESTRICT` on the three anaglyph scanline pointers
(auto-vec enabler, no helper). `DSP.h`: `dsColumnYBounds` loop -> two `simdMinMaxF64` calls
(identity pre-fill makes the `cnt` guard redundant; `any == (ymin <= ymax)`).

## Deliberately NOT in this pass (identified, deferred)

- `IO/Checksum.cpp` CRC family: right fix is Sarwate tables (scalar), not SIMD; hotpath —
  separate change with benchmark gate.
- `PainterContext` box blur: O(radius) inner loop wants the O(1) sliding-window fix first;
  SIMD second.
- `Plot3D::screenProjection` 4x4 batch transform: needs visual verification the session
  cannot provide; float accumulation-order matching required.
- Waterfall colormap LUT, FFT `log10` vectorization (non-exact), Terminal/plainTextStr scans,
  CircularBuffer multi-pattern scan, BinaryTemplates byte->string (allocation-bound),
  Dashboard `plotData3D` split-copy (non-SIMD micro-fix).

## Verification

Read-back diff review per site (exactness argument named per kernel), adversarial Sonnet
review pass, `python scripts/code-verify.py --check` on every touched file. No builds here;
the 256 kHz gate re-runs in CI (`--benchmark-hotpath`), and the TextTemplates edit keeps the
x86 fast path structurally unchanged so the gate risk is the NEON lane only.

## Outcome (verified 2026-08-25)

Every planned consumer got a kernel; the names drifted during implementation, so read
`app/src/DSPSimd.h` as the authority, not the table above.

| Planned | Shipped as | Consumer |
|---|---|---|
| `simdForEachByteMatch` | same | TextTemplates |
| `simdMinMaxF64` | same (+ `simdMinF64` / `simdMaxF64` split out) | ReportData, `DSP::dsColumnYBounds` |
| `simdRingsToPoints` | same | Plot XY |
| `simdFindAnyByte` | same | ZMODEM `zdleEncode` |
| `simdFiniteMinMaxStrided2` | `simdFiniteMinMaxPointF` | Plot, PlotCurve, MultiPlot |
| `simdWindowedComplexFill` | `simdWindowedRealFill` | FFTPlot, Waterfall |
| `simdHexPairs16` + `simdPrintableMask16` | not built | — |

The Console path took a different shape than planned: `simdAsciiDots16` (Console `Handler.cpp`)
and `simdWidenAscii` (`DataModel/Frame.h`) replaced the hex-pair/printable-mask pair. Two kernels
landed that the plan never listed — `simdPowerSpectrumDb` (FFTPlot, Waterfall) and
`simdDeinterleaveToF64` (`IO::StreamWorker`) — both from the spec-0051 stream lane, which did not
exist when this plan was written.

The deferred list above still stands: none of it was picked up.

