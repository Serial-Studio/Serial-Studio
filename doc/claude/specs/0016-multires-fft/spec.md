---
spec: 0016-multires-fft
title: Multi-resolution FFT display for the logarithmic frequency axis
status: shelved      # superseded by 0018-smooth-log-fft (2026-07-17): uniform latency beat
                     # multi-res low-band resolution; implementation removed same day
created: 2026-07-17
author: Alex Spataru
---

# Spec 0016 — Multi-resolution FFT display for the logarithmic frequency axis

> **Superseded by [spec 0018](../0018-smooth-log-fft/spec.md)** (2026-07-17). The
> multi-resolution analysis shipped, worked as specified, and was then replaced the same
> day: the low band's longer observation windows made it lag the high band (Δf = 1/T is
> a physical trade), and field testing showed uniform latency matters more than extra
> low-band resolution. Kept for the record — the requirements and the physics notes
> below explain why the trade exists.

## Problem / Motivation

Spec 0014 gave the FFT widget a logarithmic frequency axis, but the underlying analysis
still produces uniformly spaced frequency bins. On a log axis that uniformity inverts the
display's information density: at 44.1 kHz with a typical window, bin spacing is ~10 Hz,
so the region below ~200 Hz — which occupies roughly half of the log axis — is drawn from
only a couple dozen bins and renders as visibly straight, blocky line segments (confirmed
in the running app, 2026-07-17 screenshot), while the top decade collapses thousands of
bins into a few pixels. Audio and vibration work cares most about exactly that low region.

Professional audio tools (the maintainer's reference: a commercial DAW's EQ / spectrum view)
solve this with multi-resolution analysis: low frequencies are analyzed with longer time
windows (fine spectral detail), high frequencies with shorter ones, so the rendered
spectrum is smooth and evenly detailed across the whole log range. This must be real
resolution, not interpolation — two close low-frequency tones should resolve as two
peaks, which no amount of curve smoothing between sparse bins can do.

## Goals

- An FFT widget with the log frequency axis enabled renders a studio-analyzer-style spectrum:
  detail per octave is roughly even across the axis, with no blocky segment run at the
  low end and no loss of the envelope detail the top decades already have.
- Genuinely finer low-frequency resolution: closely spaced low-frequency components that
  today merge into one lump become distinguishable peaks.
- The rendered curve is smooth (no visible bin "staircase") at every widget size and zoom.
- The behavior is automatic — log frequency axis on = multi-resolution on. No new project
  keys, editor rows, or API fields; spec 0014's schema is final.

## Non-Goals

- **No change to the linear-axis FFT.** Log axis off = today's single-window pipeline,
  bit-for-bit behavior.
- **No Waterfall change.** The spectrogram keeps single-window rows (possible follow-up).
- **No new configuration surface.** Window size / sampling rate / window function keep
  their existing meanings; no "resolution" knob.
- **No exact constant-Q transform guarantee.** The goal is analyzer-like even *visual*
  detail from real multi-window analysis, not a mathematically constant-Q output.
- **No change to FFT-based alarm/export semantics** (none exist — the FFT is
  display-only today; keep it that way).

## Requirements

1. **R1** — With the log frequency axis enabled, the FFT widget analyzes low frequencies
   with longer effective time windows than high frequencies, such that displayed
   resolution per octave is approximately even across the axis.
2. **R2** — Two sinusoids one-third octave apart at 100 Hz (e.g. 100 Hz + 126 Hz) render
   as two distinct peaks at default settings where sampling rate and window permit —
   today they render as one.
3. **R3** — The transition between analysis regions introduces no artificial
   discontinuity: a swept-sine spectrum shows no step at band boundaries, and broadband
   noise shows nothing beyond the physically expected noise-density shelf (~6 dB per
   4x-finer stage — sine calibration, chosen 2026-07-17; flat-sine and flat-noise
   across different bin widths are mutually exclusive).
4. **R4** — Magnitude calibration is consistent across the axis: a constant-amplitude
   sine sweep traces the same dB level (within a small tolerance) at 50 Hz as at 5 kHz.
5. **R5** — With the log axis disabled, the analysis path and output are unchanged from
   today (single window, existing smoothing).
6. **R6** — The multi-resolution path adds no perceptible UI sluggishness at the default
   refresh rate with a 44.1 kHz audio source; total analysis cost stays within a small
   multiple (≤ ~3x) of the current single-FFT cost.
7. **R7** — The existing FFT configuration (window size, sampling rate, window function,
   dB range) keeps working: window size continues to bound latency/history, and the
   configured window function applies to all analysis regions.
8. **R8** — Zoom, pan, crosshairs, and cursor readouts on the log axis keep working over
   the multi-resolution spectrum (readouts in true Hz/dB, unchanged from 0014).
9. **R9** — The low band feels realtime: the finest analysis stage is bounded by
   response time (~0.4 s observation target — resolution and response are the same
   physical knob, Δf = 1/T), adapting the extension to the window size and sampling
   rate instead of a fixed multiple (added 2026-07-17 after field testing showed a
   fixed 16x extension felt sluggish on audio-sized windows).
10. **R10** — The log axis starts at the finest analysis stage's first bin — the
    closest a log axis gets to 0 Hz — so nothing the analysis captures is cropped
    (decided 2026-07-17, superseding an interim ~100 Hz default lower bound).

## Acceptance Criteria

- [x] **AC1** — Maintainer check, running app, audio source at 44.1 kHz: music or
      broadband audio shows a smooth, detailed low end (no straight-segment runs below
      200 Hz) comparable in character to the reference analyzer screenshot.
- [x] **AC2** — Maintainer check: a two-tone test signal (100 Hz + 126 Hz) shows two
      peaks with log axis on; toggling log off reproduces today's single-lump rendering
      (proving the linear path is untouched).
- [x] **AC3** — Maintainer check: white noise renders as a continuous spectrum whose
      only level changes are the expected smooth noise-density shelves at the two
      seams (no abrupt one-bin jumps); a swept sine shows no seam at all. Crossfade
      fallback available if a shelf reads as abrupt.
- [x] **AC4** — Maintainer check: a fixed-amplitude sine swept 50 Hz → 5 kHz holds a
      steady peak dB level across the sweep (no per-band gain jumps).
- [x] **AC5** — `--benchmark-hotpath` passes at its existing gates (analysis runs at UI
      cadence, ingest untouched — the gate proves no regression).
- [x] **AC6** — Existing projects with linear-axis FFTs render identically; a project
      with `fftLogX: true` needs no re-editing to get the new rendering.
- [x] **AC7** — Maintainer check: CPU usage of a dashboard with one log-axis FFT widget
      at default refresh stays in the same order as before the change (Task Manager
      sanity check, no fan-spin regression).
- [x] **AC8** — Maintainer check: the low band visibly tracks program-material changes
      within about half a second — no multi-second lag relative to the high band.
- [x] **AC9** — Maintainer check: the log axis starts at the lowest resolvable bin
      (nothing cropped at the low end); the axis-range dialog can still narrow the view.

## Constraints & Invariants

- **Display-side only.** Ingest, rings, and the 256 kHz hotpath gate are untouched; all
  added analysis work runs at UI draw cadence in the widget layer, like 0014.
- **Zero schema/API delta.** No new project keys, editor items, or API fields; the
  feature keys off the existing `fftLogX` flag.
- **History extension is passive and bounded.** Longer low-band windows come from a
  bounded, configure-time extension of the existing FFT sample ring (approved at plan
  gate 2026-07-17); the per-sample ingest code path must remain byte-identical — no new
  ingest-path logic, counters, or per-frame work.
- **Deterministic memory.** Any additional analysis buffers are sized at configure time
  and allocation-free at steady state (same discipline as the 0014 scratch ring).
- **No new dependencies** (the existing FFT library is sufficient).
- **QuickPlot audio FFTs** (auto-generated datasets) get the improvement automatically
  when their log axis is enabled, with no special-casing.

## Open Questions

- None — activation (automatic with the log axis), fidelity (true multi-resolution, not
  interpolation-only), and Waterfall exclusion were all decided 2026-07-17.
