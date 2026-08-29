---
spec: 0018-smooth-log-fft
title: Smooth log FFT — single window + smooth interpolated rendering
status: done         # approved 2026-07-17; supersedes 0016
created: 2026-07-17
author: Alex Spataru
---

# Spec 0018 — Smooth interpolated log FFT rendering

> Compact combined spec/plan/tasks: records the decision that superseded
> [spec 0016](../0016-multires-fft/spec.md) and the implementation that replaced it.

## Problem / Decision

Spec 0016's multi-resolution analysis delivered real low-frequency resolution, but the
low band inherently lagged the high band (Δf = 1/T — longer observation is the price of
finer bins), and neither the 0.4 s time budget nor the 0017 ballistics could remove the
*attack* latency differential, which field testing kept surfacing. Investigating the
reference tools settled it: professional DAW spectrum analyzers do **not** do
multi-resolution — they run one moderate FFT (uniform latency everywhere) and render
the sparse low bins as a smooth interpolated curve in log space, plus ballistics.
Decision (2026-07-17): follow that recipe — accept single-FFT low-frequency resolution,
make it *draw* smooth, keep the feel uniform and realtime.

## What was done

1. **Removed** the 0016 machinery: decimation cascade, FIR taps, stage layout/stitch,
   raw/stage scratch buffers, `resyncLogModeConfig`, and the Dashboard ring extension
   (log and linear FFTs share one code path again; ring capacity = clamped
   `fftSamples`). The hostile-input clamp from the 0016 review **stays**
   (`qBound(1, fftSamples, kMaxFftRingSamples)` in `configureFftSeries`).
2. **Added** smooth log rendering: `FFTPlot::buildLogRenderCurve` fits a
   Fritsch-Carlson monotone cubic (PCHIP — never overshoots, peaks stay honest)
   through the bins in log-x space and resamples it onto a uniform 2048-point log
   grid; `rebuildLogBinTable` caches per-bin log-x positions at ctor/plan rebuild.
   Pipeline per tick: `computeBinSpectrum` (dB + boxcar + optional 0017 ballistics,
   per bin) → `emitLinearSpectrum` (linear axis, unchanged shape) or
   `buildLogRenderCurve` (log axis).
3. Axis starts at the first FFT bin — the closest a log axis gets to 0 Hz, so nothing
   the analysis captures is cropped; linear-axis rendering is unchanged; the 0017
   ballistics apply per bin upstream of both paths.
4. Related follow-up (same day, outside this spec's scope): the FFT window-size ceiling
   was raised from 16384 to 262144 in the project editor and the widget clamp — with
   the resolution/response trade now an explicit user knob, large windows are a
   deliberate deep-analysis choice (Δf ≈ 0.17 Hz at 44.1 kHz over a ~6 s window).

## Acceptance Criteria

- [x] **AC1** — Maintainer: log-axis FFT feels uniformly realtime — no low/high update
      differential; low decades render as smooth interpolated hills, no angular
      segments.
- [x] **AC2** — Maintainer: peaks read the same dB as before (monotone interpolation,
      no overshoot); linear-axis FFT unchanged.
- [x] **AC3** — Existing pytest suite still green (no schema/API delta in this spec;
      0014/0017 round-trip tests cover the persisted fields).
- [x] **AC4** — `--benchmark-hotpath` unchanged (widget draw layer only; the Dashboard
      diff vs master is now just the input clamp).

## Files

`app/src/UI/Widgets/FFTPlot.h/.cpp` (rewrite of the log path),
`app/src/UI/Dashboard.cpp` (ring extension reverted, clamp kept),
`doc/claude/architecture/dashboard.md` (section rewritten), spec 0016 marked shelved.
