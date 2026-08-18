---
spec: 0058-plot-measurement-scaling
title: Plot measurement and scaling
status: in-progress
created: 2026-08-16
author: Claude (overnight run, unattended)
---

# Spec 0058 — Plot measurement and scaling

> **Phase 1 of 4 — the WHAT and the WHY.** Gate auto-approved for the unattended overnight run
> of 2026-08-16 (see `plan.md`); the maintainer re-reviews on the morning pass. Four
> independent tasks; each stands alone.

## Problem / Motivation

Four small, unrelated frictions in the time/line plot, all inherited from "guess, then hope":

1. **Tick spacing is guessed from a template string** (`"-8888.88"` plus a fixed 20 px gutter).
   Real labels are narrower or wider than the template (engineering suffixes, `us`, long
   decimals at deep zoom, wide fonts, other locales), so labels can collide or the axis can be
   sparser than it needs to be. Readout precision is guessed separately ("~3-4 significant
   digits") from the same range, so it can disagree with what the ticks show.
2. **Cursors report ΔX/ΔY but never 1/ΔX**, which is the number an oscilloscope user reaches for
   first on a time axis (period -> frequency).
3. **Y autoscale is continuous**: the range follows the data extent with 10 % padding and
   integer rounding, so a slowly drifting or breathing signal keeps re-scaling the axis by
   tiny amounts, ticks and grid shimmer, and the eye cannot hold a reference.
4. **The X axis has no ruler affordances**: no way to name a point of interest, no way to
   declare "this instant is t = 0" and read every other value relative to it, no pointer
   tracking marker.

## Goals

- Tick labels never collide at any zoom, font size, or locale, and the axis is as dense as the
  measured labels allow (X: width; Y: height); readout precision follows the tick unit.
- With two cursors on a time axis the readout also shows `1/ΔX` in SI-prefixed Hz.
- Y autoscale steps through a discrete 1-2-5 ladder with hysteresis so a signal hovering at a
  ladder boundary cannot make the axis oscillate; a user-configured range is untouched.
- The X axis gains named markers, a settable time-zero (labels and cursor readouts become
  relative), zero reset, and a hover marker; all persisted with the widget's settings through
  the existing debounced `widgetSettings` path.

## Non-Goals

- Changing the log-axis tick policy (`logInterval` stays as it is).
- A global command/palette entry or new icons for the ruler UX (in-widget interaction only).
- Touching MultiPlot's per-curve legend or FFT axes.
- Data-anchored markers (markers live in axis world coordinates, like the cursors do today).

## Requirements

1. **R1** — Tick period is the first candidate of the 1-2-5-per-decade sequence whose measured
   widest label plus a fixed gutter fits within one tick pitch; the search is bounded.
2. **R2** — Minor ticks: 4 subdivisions under a mantissa-2 period, 5 otherwise.
3. **R3** — Cursor readout precision derives from the chosen tick period, not from a separate
   estimate.
4. **R4** — With cursors A and B placed on a time axis and `ΔX != 0`, the readout shows
   `1/ΔX` formatted with SI prefixes (`1.25 kHz`, `250 mHz`); a Samples-mode axis or `ΔX == 0`
   shows no frequency.
5. **R5** — When the combined readout does not fit the label width, precision degrades and the
   hint text drops before the frequency or the units do.
6. **R6** — Data-derived Y limits are multiples of a ladder step chosen so the extent fits in a
   fixed number of divisions; the step only grows immediately and shrinks only when the extent
   comfortably fits the next smaller step (hysteresis).
7. **R7** — Zero sits on a division boundary whenever the data goes negative.
8. **R8** — Right-clicking the plot (outside cursor mode) offers: add a named marker at the
   click, set time zero at the click, reset time zero, toggle the hover marker, and remove an
   existing marker.
9. **R9** — With time zero set, X tick labels and cursor X readouts are relative to it (signed);
   ΔX is unchanged; the axis title says so.
10. **R10** — Markers, time zero and the hover toggle persist per widget via `widgetSettings`
    (memory + `setModified(true)`, no direct disk writes) and restore on load.

## Acceptance Criteria

- [ ] **AC1** (maintainer, running app) — at 5 zoom levels, 3 UI font sizes and one wide-glyph
      locale, no two X labels overlap; the axis has at least as many labels as before at the
      default font.
- [ ] **AC2** (maintainer) — cursor readout on a time axis shows `1/ΔX` with a sensible SI
      prefix; on a Samples axis it does not; with A and B at the same X it does not.
- [ ] **AC3** (maintainer) — a slowly breathing sine (amplitude drifting +/-10 %) keeps a stable
      Y range for long stretches and steps only at ladder boundaries, without flapping.
- [ ] **AC4** (maintainer) — markers, time zero and hover toggle survive a project reload.
- [ ] **AC5** (structural) — `code-verify.py --check` clean on touched files; every new
      user-visible string wrapped in `qsTr()`; no binding loops on plot open/zoom/pan.

## Constraints & Invariants

- Nothing on the frame path; all of this is draw-cadence (60 Hz) or interaction-cadence work.
- QML comment-sandwich convention; icons only through `Misc::IconRegistry` (none needed).
- The user-set-range path (`pltMin != pltMax`) is untouched.

## Open Questions

- Marker semantics on a time axis are "seconds ago" (they slide with the axis, like cursors);
  data-anchored markers would need the absolute timestamp, which the ring does not carry per
  slot. Left as a follow-up.
- Whether MultiPlot should share the ruler UX (it shares `PlotWidget`, so the affordance is
  present; persistence was wired for both).
