---
spec: 0057-cascading-envelope-rings
title: Cascading envelope rings for plot history
status: done          # closed 2026-08-20
created: 2026-08-16
author: Claude (overnight run, unattended)
---

# Spec 0057 — Cascading envelope rings for plot history

> **Phase 1 of 4 — the WHAT and the WHY.** Gate auto-approved for the unattended overnight run
> of 2026-08-16 (see `plan.md`); the maintainer re-reviews on the morning pass.

## Problem / Motivation

A time-axis plot keeps its history in one bounded `(time, value)` ring that decimates on ingest
onto a single absolute time grid (min/max envelope pair per grid cell). Two things follow from
"one grid":

1. **Render cost scales with the ring, not the screen.** Every draw walks every retained sample
   inside the visible window and re-buckets it into pixel columns. A stream-lane ring sized for
   44.1 kHz over a 12.5 s window holds ~550k slots (up to 4M at the byte ceiling); a 1000 px
   plot at 60 Hz walks all of them 60 times a second, per curve. That is why the ring ceilings
   and the drain budget exist, and why wide windows on dense sources feel heavy.
2. **The grid resolution has to be guessed from the source rate.** Level of detail retained is
   fixed at ingest, so a grid coarser than the source loses detail forever, and a grid finer
   than the memory budget allows cannot span the window. `streamRingCapacity`,
   `growTimeRing`/`resizeCapacity` and `kMaxRateSizedRingSamples` all exist to steer that guess
   at layout time or on the display tick.

The idea adapted here (from oscilloscope/logic-analyzer viewers) is a pyramid of min/max
envelope levels, each 16x coarser than the one below, appended incrementally, with the renderer
choosing the coarsest level whose cells are still finer than one pixel and reading only the
visible span. Serial Studio runs for days, so every level must be a bounded ring, never an
unbounded append log.

## Goals

- Rendering a wide window reads O(pixels) envelope cells instead of O(samples in window), for
  both `Plot` and `MultiPlot` time-axis draws, frame lane and stream lane alike.
- Full-detail history is preserved exactly as today at level 0; zooming into a narrow window
  still reads level 0 and shows the same detail the current build shows.
- Memory for the coarse levels is bounded and small: with level k holding
  `ceil(cells0 / 16^k) + 1` cells of 32 bytes (two time-ordered extremes) over a level-0 ring
  of `C0` slots (16 bytes each, `cells0 = C0 / 2` saturated cells), the coarse levels sum to
  `32 * (C0/2) * (1/16 + 1/256 + ...) = 16 * C0 / 15`, i.e. **1.067x** level 0's bytes for any
  level count. Verified: C0 = 262144 gives 4.19 MB + 0.28 MB (5 levels), C0 = 4M gives
  67.1 MB + 4.47 MB (6 levels); C0 = 1024 gives 3 levels at 1.070x. Levels stop when a level
  would hold fewer than two real cells, capped at ten levels.
- Append is incremental and allocation-free after construction: a completed level-0 cell folds
  into every coarser open cell by min/max; no rescan, no per-sample virtual call.
- Wraparound at any level keeps the levels mutually consistent: a coarse cell's min/max always
  equals the min/max of the level-0 samples its span covers (samples that were ever appended),
  and a coarse level never claims history older than what it holds.

## Non-Goals

- Making level 0 unbounded or larger than today's ceilings (memory budget unchanged).
- Replacing the renderer's per-pixel min/max aggregation (`dsAccumulateBuckets` and friends
  stay; only what they read changes).
- Touching the sweep/trigger rings (`SweepEngine` keeps plain `TimeRing`s: sweeps are short
  windows rendered through `downsampleWindowAbsolute`).
- Changing the sample-axis, dataset-X, FFT, GPS or 3D rings.
- Removing the level-0 sizing/growth machinery. It keeps sizing level 0 for memory; the
  pyramid removes the render-cost reason for guessing, not the memory reason (decision recorded
  in `plan.md`).
- A digital/boolean timeline widget or an edge-extraction mipmap (out of scope tonight).

## Requirements

1. **R1** — A time-axis plot ring is a pyramid: level 0 behaves exactly like today's decimating
   ring; each level k >= 1 stores time-ordered `{t0, v0, t1, v1}` extreme pairs per cell, each
   cell covering `16^k` level-0 grid cells, aligned to the level-0 grid.
2. **R2** — Appending a sample costs what it costs today plus one branch on the common path;
   the fold into coarser levels runs only when a level-0 cell completes and touches at most one
   open cell per level.
3. **R3** — Coarse levels never allocate after construction; level 0 keeps its existing
   grow/resize behaviour and the coarse levels are rebuilt from level 0's contents when it is
   resized.
4. **R4** — Given a visible time span and a pixel width, the ring returns the coarsest level
   whose cell span is at or below one pixel of time and which still covers the requested span;
   otherwise it falls to a finer level, level 0 being the ground truth that always qualifies.
5. **R5** — Both plot widgets and every other reader of the ring (the API's tail-frame
   accessor) keep working; the API keeps reading level 0 samples.
6. **R6** — Non-finite times or values are rejected before touching any level, exactly as level
   0 rejects them today.
7. **R7** — Snapshot/restore across layout rebuilds and time-range changes preserves history as
   today: same-shape rings move, different-shape rings replay through the pyramid append.

## Acceptance Criteria

- [x] **AC1** (ctest, `tst_envelope_ring`) — appending a known ramp yields, at every level, cells
      whose min/max equal a brute-force min/max over the covered level-0 grid range.
- [x] **AC2** (ctest) — level selection returns the expected level for a given seconds-per-pixel,
      falls back to level 0 for narrow windows, and falls to a finer level when the coarse level
      does not cover the requested span.
- [x] **AC3** (ctest) — wraparound at ring capacity keeps every level's cells consistent with the
      samples still covered, and a coarse level's oldest cell is never older than the level-0
      grid it summarises claims.
- [x] **AC4** (ctest) — NaN/inf time or value is rejected at every level; nothing is appended.
- [x] **AC5** (maintainer, running app) — a 44.1 kHz stream-lane source (Audio) on a 120 s time
      range: the wide view draws from a coarse level (CPU of the GUI thread visibly lower than
      the current build on the same project; sample with `top`/Instruments), and zooming into
      any 1 s slice, including one 60 s old, shows the level-0 detail the ring retained. Detail
      density in the narrow view is unchanged from the current build (level 0 is unchanged);
      the win is the wide-view cost.
- [x] **AC6** (maintainer) — `--benchmark-hotpath` gated tiers unchanged within noise: nothing on
      the pipeline thread changed. The ungated `lua+dashboard` phase may move slightly (one
      extra branch per appended sample on the GUI thread).
- [x] **AC7** (structural) — `python3 scripts/code-verify.py --check` clean on every touched file.

## Constraints & Invariants

- The frame pipeline (pipeline thread) is untouched: rings live in `UI::Dashboard`, GUI thread,
  single writer, no mutex — the same discipline as the current ring.
- No allocation on the append path (levels sized once; level 0 grows only through the existing
  display-tick growth path).
- `SS_ASSERT_HOTPATH` only where the condition restates a guard that provably already ran; the
  append path uses the plain `SS_ASSERT` the current ring uses (this is GUI-side code).
- Existing render pipeline invariants stay: newest sample rebased to 0, absolute column lattice,
  visible-window binary search, per-column first/min/max/last emission.
- No new dependency; header-only addition to `app/src/DSP.h`.

## Open Questions

- Should the freed render budget be spent on a larger level-0 ceiling (more detail per curve)
  or left as headroom? Left unchanged tonight; this is a maintainer decision.
- Level 0's grow-on-saturation path is kept as-is (upward only). With the render cost decoupled
  from level-0 density, a simpler policy (size level 0 once from a fixed byte budget) becomes
  possible; recorded in `plan.md` as the rejected-for-tonight alternative.
