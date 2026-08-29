---
spec: 0017-fft-ballistics
title: FFT display ballistics (fast attack, exponential release)
status: done         # approved 2026-07-17
created: 2026-07-17
author: Alex Spataru
---

# Spec 0017 — FFT display ballistics

> Compact spec for a small display feature; scope decisions were taken during review and
> are recorded in the Decisions section.

## Problem / Motivation

Without ballistics the FFT display draws the raw spectrum of the moment: the high band
flickers at full rate while the low band steps between analysis frames, and the two
regions visibly update at different rhythms. Professional audio analyzers never draw raw
frames — every displayed bin has instant attack and a slow
exponential release, which unifies the perceived motion of the whole spectrum and is a
large part of why those analyzers feel realtime.

## Requirements

1. **R1** — An FFT dataset gains a **Peak Ballistics** option (project editor checkbox)
   and a **Release (ms)** value (default 300, sane clamp): with ballistics on, each
   displayed bin rises instantly to a higher fresh value and decays exponentially
   toward a lower one with the configured release time.
2. **R2** — Attack is instant (peaks never under-read); only the decay is smoothed.
3. **R3** — Off by default; absent keys read as off; untouched projects serialize and
   render byte-identically (0014/0016 additive-schema discipline).
4. **R4** — Works on both the linear and log frequency axis (independent of `fftLogX`),
   at any UI refresh rate (decay is wall-clock-based, not tick-count-based).
5. **R5** — Persists in the project file and is settable via `project.dataset.update`
   (`fftBallistics`, `fftBallisticsRelease`).
6. **R6** — Display-only: analysis, ingest, and exports untouched; no allocation at
   steady state.

## Acceptance Criteria

- [x] **AC1** — Maintainer: with ballistics on, music renders as one cohesive, smoothly
      decaying spectrum (no low/high rhythm mismatch); peaks still hit full level.
- [x] **AC2** — Maintainer: toggling the checkbox off restores today's raw display.
- [x] **AC3** — pytest round-trip: both fields persist via API + save/load; absent keys
      read off/300.
- [x] **AC4** — `--benchmark-hotpath` unchanged (nothing off the widget draw layer).

## Decisions (2026-07-17)

Off by default; the editor exposes the checkbox and release time only (attack is fixed
instant — a nonzero attack would make displayed peaks under-read); applies to the FFT
widget only.
