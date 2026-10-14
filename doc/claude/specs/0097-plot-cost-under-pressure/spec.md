---
spec: 0097-plot-cost-under-pressure
title: Plots pay for what arrives and what they draw
status: in-progress  # approved 2026-10-13; implementation started 2026-10-13
created: 2026-10-13
author: Alex Spataru
---

# Spec 0097 — Plots pay for what arrives and what they draw

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

Plotting is the one thing Serial Studio must do well, and today it scales badly with plot
count. Measured on 2026-10-13 against the maintainer's running HEAD build (db2debec4, macOS,
HDR on). The setup was N single-curve time plots fed by one TCP frame-lane source at 1 kHz, a
10 s time range and a 60 Hz display tick. Physical-footprint growth from connect, after 15 s
of streaming:

| Time plots | Footprint growth | Per plot |
|-----------:|-----------------:|---------:|
| 1          | +18 MB           | 18 MB    |
| 10         | +112 MB          | 11 MB    |
| 30         | +471 MB          | 16 MB    |

The data that arrives per curve over the retained window is about 200 KB
(12.5 s × 1 kHz × 16 B). The rest comes from three behaviors:

1. **Every time-plot curve reserves history for a hypothetical 1.024 MHz source.** That is
   262,144 (time, value) slots, 4 MB, plus a 0.27 MB envelope pyramid, whatever the source
   delivers. The ring only ever grows, and only once it is full, so a slow source
   never gives the memory back.
2. **Oscilloscope sweep storage is reserved for every time-plot curve whether sweep is on or
   not.** That is two more rings of the same size, 8 MB per curve, for a feature that is off
   by default.
3. **All of it is locked into physical memory when it is allocated.** The OS can neither
   leave untouched capacity uncommitted nor compress it. A 10 Hz curve costs the same
   12.3 MB of wired RAM as a 1 MHz one.

The maintainer's field project (BADAQ: 4 sources, 27 time plots, 4 FFTs, 91 data grids)
therefore takes about 330 MB of locked plot memory at connect. That comes on top of an idle
footprint of about 800 MB, which is a separate problem (see Non-Goals). This is the "10
plots blow past 1 GB" report.

**The GUI thread had no idle time at all.** A 10 s sample at 30 plots / 1 kHz showed three
costs:

- **The GUI thread spent 53% of its time blocked on the render thread.** Qt's threaded
  render loop holds the GUI thread while the render thread syncs, and also while it finishes
  the previous frame. The render thread was 58% busy:
  - Scene-graph sync took 31%. Of that, building plot-curve stroke geometry was about 19% of
    wall time and area-fill geometry about 8%.
  - The render pass took 25%.
- **Plot downsampling, the data side of the draw, was only about 5%.** The data model
  holds up; the geometry does not.
- **Stroke geometry grows with data density, not plot width.** Dense data reaches the
  renderer as a per-column min/max envelope, and that envelope reverses direction at almost
  every point. Every reversal emits a full round-join fan, and every vertex re-derives its
  color from scratch.
- **The console took 31% of the GUI thread** formatting about 220 KB/s of incoming text
  into the scrollback of a console nobody was looking at.

The one principle this spec enforces: **a plot pays for the data that arrives and
the pixels it draws, never for the worst case it could theoretically receive, and
it never gives up a real sample to save memory.** That rules out any fix that buys memory
with resolution. Audio must stay sample-exact, and every plot must stay zoomable down to
its actual data.

## Goals

- A time-plot curve's memory is on the order of the data its source delivers over
  the window. A 1 kHz curve at a 10 s range costs well under 1 MB, not 12.3 MB.
- Every curve keeps every actual sample of its window, up to the existing per-curve ceiling.
  Zooming any plot, audio included, shows the real samples, never a reconstruction. Nothing
  that is exact today becomes less exact.
- A feature that is switched off allocates nothing.
- Dashboard display memory is ordinary pageable memory.
- Per-tick render work for a curve is bounded by its on-screen width, the same for dense
  data as for sparse.
- Data arriving for a console nobody is viewing costs the GUI thread close to nothing, and a
  visible console's cost is bounded per tick.
- The regressions cannot quietly return. The memory contract is checked in CI, and plot
  history size is observable at runtime.

## Non-Goals

- **Reducing the ~800 MB idle footprint.** Measured with a trivial project loaded and
  nothing connected: about 400 MB of dirty heap, 167 MB of window drawables and 157 MB of
  GPU-owned memory. That needs its own investigation and spec.
- **The cost of HDR's per-window FP16 composition layer.** It is opt-in and not
  plot-specific.
- **Changing what a sample costs to store** (for example implicit timestamps for
  uniform-rate sources, or narrower value types). This is a follow-up candidate. This spec
  stops paying for samples that never arrive; it does not compress the ones that do.
- **Plot types sized by point count rather than by time:** FFT, waterfall, GPS, 3D,
  dataset-X and Samples-axis plots. They are small and unaffected.
- **Any rate cap, sample skipping or per-view reduction on the ingest path.** These are
  forbidden by the hotpath rules and are not needed.
- **A user-facing memory setting.** The bounds are internal (decided 2026-10-13).
- **Visual redesign.** Plots must look the same (R11).

## Requirements

### Plot memory

1. **R1 — Rate-true sizing.** A time-plot curve's history is sized from the rate its source
   delivers in the current session, never from an assumed maximum.
   - Frame-lane and stream-lane sources both follow this rule.
   - A curve whose source speeds up mid-session grows to the new rate.
   - A curve whose source slows down keeps its retained history and does not shrink
     mid-session. Its footprint resets at the next connect or layout rebuild.
2. **R2 — Zoom to actual data.** For every curve whose rate × window fits under the
   per-curve ceiling (R3), the retained history holds every sample the source delivered
   over the full window plus headroom, from the first sample onward, with its exact value
   and timestamp.
   - "From the first sample onward" means there is no coarse start-up stretch before the
     ring has learned the rate.
   - This holds on both lanes, for audio at its native rate, and for frame-lane sources
     with timestamp jitter.
   - Every curve that is exact today stays exact.
3. **R3 — Per-curve ceiling unchanged.** Exact retention is guaranteed up to today's
   per-curve ceiling of 4,194,304 samples. Above it, history decimates with min/max
   preserved, exactly as today.
4. **R4 — Off costs nothing.** A time plot with sweep/trigger off holds no sweep storage.
   - Enabling sweep allocates it at the curve's rate-true size.
   - Disabling sweep releases it.
   - Sweep behavior while on is unchanged: triggering, timebase, retained segments, and
     configuration surviving a time-range change.
5. **R5 — No overshoot.** In steady state, a curve's allocated history storage is at most
   1.25× what its retained window needs. Today's power-of-two rounding can double it.
6. **R6 — Pageable.** No dashboard display memory is locked into physical memory. The
   acquisition pipeline's own buffers keep their current treatment.
7. **R7 — Safety rail, never silent.** Total time-plot history has a hard ceiling of 512 MB
   that only binds in configurations no realistic project reaches. For scale, 8 channels at
   192 kHz or about 50 channels of 48 kHz audio fit exactly at the default range.
   - When the rail binds, a curve whose growth would cross it keeps its current size and
     decimates from then on, only down to the min/max-preserving decimation R3 describes.
     No curve ever shrinks to make room, because shrinking would discard history.
     *(Amended 2026-10-13 at plan time: "fastest curves first" needed shrinking.)*
   - Every affected plot shows a visible "decimated" indication.
   - Precision is never lost silently.
8. **R8 — Observable.** The app reports on demand, through the API and at no per-frame
   cost:
   - the total bytes held by plot history;
   - each curve's retained rate and whether it is exact;
   - whether the rail is binding.

### Plot render cost

9. **R9 — Density-independent geometry.** The geometry a curve draws per tick is bounded
   by a small constant per on-screen pixel column, whatever the data density.
   - A segment costs the same geometry at any turn angle, so a dense min/max envelope
     (which reverses at almost every point) pays no more per segment than a straight line.
   - The area fill stays bounded per column.
   *(Amended 2026-10-13 at plan time: the per-column bound replaces a dense-vs-sparse ratio
   that compared inputs of different sizes.)*
10. **R10 — The GUI thread breathes under pressure.** In Scenario P (below), each thread
    meets its target:

    | Thread | Measure | Target | Today |
    |---|---|---|---|
    | Render | Scene-graph sync, which blocks the GUI thread | ≤ 10% of wall | 31% |
    | GUI | Blocked on the render thread | ≤ 25% of wall | 53% |
    | GUI | Idle | ≥ 40% of wall | ≈ 0% |

11. **R11 — Visual parity.** Every look is preserved, with only differences a side-by-side
    comparison cannot find:
    - stroke width and antialiasing;
    - per-curve color, including HDR boost;
    - interpolation modes (linear, ZOH, stem) and NaN gaps;
    - area fill (bipolar, negative and FFT floors) and log axes;
    - sweep with retained segments;
    - zoom and pan;
    - light and dark themes.

### Console ingest

12. **R12 — A hidden console is nearly free.** While no console view is on screen,
    incoming data costs the GUI thread no per-character formatting. In Scenario P the
    console's GUI-thread share is ≤ 3% of wall (today 31%).
13. **R13 — No gap on show.** Opening the console shows the most recent output up to its
    scrollback limit, rendered as it would have been had it been visible all along.
    - Plain text is exact.
    - ANSI colors and VT100 effects are exact within the retained window. Escape state from
      before that window (an SGR color, cursor addressing, a clear) may be lost.
    *(Amended 2026-10-13 at plan time: the bounded form, resolving the open question.)*
14. **R14 — A visible console is bounded.** With the console on screen, its per-tick cost
    does not grow with the incoming rate beyond what one scrollback can display. In
    Scenario P with the console visible, its GUI-thread share is ≤ 10% of wall.

### Durability

15. **R15 — Gated, not remembered.** The memory contract (R1–R5) is enforced by checks
    that run in CI. A future change that reserves for an assumed rate, allocates for an off
    feature, or pins display memory fails a check, not a code review.

**Scenario P** (the reference pressure scenario) is the run measured above:

- 30 single-curve time plots, plus a variant with 4 multiplots of 8 curves;
- one TCP frame-lane source at 1 kHz carrying all values as CSV text;
- 10 s time range and a 60 Hz display tick;
- the Console pane present but not shown;
- the maintainer's Mac, with HDR both on and off.

**Scenario A** is the audio loopback rig with two 48 kHz channels and a 10 s range.

## Acceptance Criteria

- [ ] **AC1 (R1, R2, R3, R5) — unit tier.** Rates of 10 Hz, 1 kHz, 10 kHz, 48 kHz and
  192 kHz, with windows of 1 s, 10 s and 300 s, on both lanes, each pass two checks:
  - After a full window of synthetic samples (frame-lane cases with ±20% timestamp
    jitter), the retained history equals the input sample-for-sample, from the first
    sample, wherever rate × window is under the ceiling. Above it, it matches today's
    min/max decimation.
  - Allocated bytes are ≤ 1.25× the retained need.
- [ ] **AC2 (R1) — unit tier.** A rate step up mid-run grows the ring with no lost samples.
  A step down keeps history and does not shrink.
- [ ] **AC3 (R4) — unit tier.** Sweep off holds zero sweep bytes. Turning it on allocates
  sweep storage, and turning it off releases it. Today's sweep tests pass unchanged.
- [x] **AC4 (R6, R15) — CI check.** A structural check fails if any dashboard display
  storage requests pinned memory.
- [ ] **AC5 (R7, R8) — unit + API.** A synthetic configuration over 512 MB stays under the
  rail: the curves whose growth was refused decimate, and the API reports the rail as
  binding and those curves as inexact. A normal project reports every curve exact.
- [ ] **AC6 (end-to-end memory) — performance test against the running app (Scenario P).**
  Footprint growth from connect is ≤ 120 MB at 30 plots (today 471) and ≤ 45 MB at 10
  plots (today 112). The plot-history bytes reported by R8 are ≤ 15 MB at 30 plots.
  BADAQ's plot-history bytes at connect are reported and recorded in the spec report.
- [ ] **AC7 (R2, audio) — Scenario A.** After 15 s of streaming, each channel's retained
  history holds rate × window (+ headroom) samples, and the narrowest zoom shows individual
  samples matching the loopback signal.
- [ ] **AC8 (R9) — unit tier.** A stroke segment emits the same vertex and index count at
  every turn angle (straight, 90°, 180°). A dense min/max envelope at W columns emits
  ≤ 8·W stroke vertices, and the fill stays ≤ its current per-column bound.
- [ ] **AC9 (R10, R12, R14) — profile run.** In Scenario P under `sample`:
  - Render-thread sync is ≤ 10% of wall.
  - The GUI thread is blocked ≤ 25% and idle ≥ 40%.
  - Console share is ≤ 3% hidden and ≤ 10% visible.
  - The multiplot variant meets the same numbers.
  - The before/after profiles are attached to the spec report.
- [ ] **AC10 (R11) — maintainer visual check.** A side-by-side of the R11 scene list,
  before and after, with HDR on and off and in light and dark themes.
- [ ] **AC11 (R13) — unit tier + maintainer observation.** Stream a known ANSI-colored text pattern for 10 s
  with the console hidden, then show it. The pattern colors each line with its own SGR
  codes. The visible scrollback matches the pattern's tail exactly, colors included.
  *(Amended 2026-10-13 at task time: the API has no view of terminal contents, so the
  check is a ctest buffer equivalence plus the maintainer's in-app observation.)*
- [ ] **AC12 (no regression).** `--benchmark-hotpath` passes all nine gates at the default
  `--min-fps`. The `dashboard ingest costs` ratio is no worse than the db2debec4 baseline.
  `ctest` and `pytest tests/integration/` are as green as before.

## Constraints & Invariants

- **Deciding constraint: no fix may trade a real sample for memory.** Memory comes back by
  not reserving for data that never arrives, never by retaining less of the data that did
  (R2, R3). The rail (R7) is the only exception, and it is never silent.
- The 256 kHz hotpath gate holds.
- The dashboard's per-sample ingest stays allocation-free.
- History growth never runs inside the per-sample loop. It runs at block granularity, a
  bounded number of times per ring per session (logarithmic), and keeps retained samples
  exactly. *(Amended 2026-10-13 at plan time: "display-tick rate" could wrap away samples at
  low UI refresh rates.)*
- The time-ring rules hold:
  - capacity derives from a rate, never a bare sample count;
  - a ring's clock never rewinds;
  - a rebuild never seeds a ring.
- The envelope pyramid's (spec 0057) contract holds: wide windows read O(pixels).
- Must work across all of these:
  - frame lane and stream lane (spec 0055);
  - QuickPlot and ProjectFile modes;
  - replay of CSV, MDF4 and Historian sessions;
  - the remote dashboard viewer (spec 0040).
- The `dashboard.tailFrames` API semantics are preserved.
- Sweep (Pro) behaves identically while enabled.
- Pro gating is unchanged and trial parity holds.
- The HDR rendering path keeps its contract (spec 0089). The software renderer still draws.
- Diagnostics are pulled at most at 1 Hz, never pushed per frame (specs 0033/0035).
- No new user-facing setting.
- Supported on macOS, Windows and Linux.

## Open Questions

- ~~**R13 exactness.**~~ Resolved at plan time: the bounded form (see R13).
- **R10 targets.** These are set from one machine. Should the gate also be stated per
  plot (for example render-thread µs per curve per tick), so it carries to slower Windows
  and Linux hardware?
