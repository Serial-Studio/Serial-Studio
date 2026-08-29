---
spec: 0061-sweep-segment-retention
title: Sweep segment retention
status: done          # closed 2026-08-20
created: 2026-08-16
author: Claude (overnight run, unattended; planned + implemented 2026-08-17 on maintainer request)
---

# Spec 0061 — Sweep segment retention

> **Phase 1 of 4 — the WHAT and the WHY.** Written 2026-08-16 as specify-only; on 2026-08-17 the
> maintainer asked for B to be planned and implemented — see `plan.md` for the decisions taken
> on the open questions and the deviations recorded there.

## Problem / Motivation

`DSP::SweepEngine` keeps exactly two rings per curve: `back` (the sweep being captured) and
`front` (the last completed sweep). `completeSweep()` swaps them, so every triggered sweep but
the last is discarded the moment the next one completes. That is the right default for a live
scope display, but it throws away exactly what a user triggering on a rare event wants: "show
me the last N events", "overlay them", "compare this glitch to the one 30 s ago", "save the
captured events with the session". A Session Database recording captures the continuous
stream, not the sweep segments the trigger carved out of it, so the trigger's work is lost on
replay too.

## Goals

- Retain the last N completed sweeps per plot as first-class records (times relative to their
  own `t0`, plus the absolute trigger instant), N configurable, memory bounded.
- Overlay/compare in the plot: persistence-style overlay of the last N (older = dimmer),
  step-through single segments, and a "reference" pin that freezes one segment for comparison.
- Persist retained segments with a Session Database recording and restore them on playback.
- Clear separation of storage from the live front/back rings so the hotpath is untouched.

## Non-Goals

- Changing trigger detection, holdoff, Auto/Normal/Single semantics.
- Unbounded retention or per-segment file export (CSV of one segment is a cheap follow-up).
- Averaging/envelope-across-segments math modes (a natural second slice; not here).

## Requirements

1. **R1** — On `completeSweep()`, the finished `front` ring's contents are copied into a
   segment record `{triggerAbsSec, sweepWindowSec, times[], values[]}` per curve and appended
   to a per-plot ring of `N` segments (default 8, max 64); the copy is one memcpy-shaped walk
   at completion time, never per sample.
2. **R2** — Memory bound per plot = `N * curves * frontCapacity * 16 B`; the setting clamps N
   so a plot never exceeds a fixed byte budget (`kMaxSegmentBytes`, e.g. 32 MB); the UI shows
   the resulting N.
3. **R3** — Plot toolbar (sweep mode only): overlay toggle (last N, dimming by age), a segment
   stepper (newest ... oldest, shows trigger time-ago), a "pin as reference" toggle that keeps
   one segment drawn in a distinct colour across new captures, and "clear segments".
4. **R4** — Segments survive a Time-Range rebuild and a layout rebuild like the ring snapshots
   do; `clearPlotData` and `resetData` drop them.
5. **R5** — With the Session Database recording, retained segments persist (per plot, per
   curve: trigger instant, window, samples) and reload on session playback so the same
   overlay/stepper works on a replayed session.
6. **R6** — Everything is per-widget `widgetSettings` (N, overlay on/off, pinned index) via the
   existing debounced path.

## Acceptance Criteria

- [x] **AC1** — Normal trigger on a 1 Hz pulse: after 12 s the stepper shows 8 segments (N=8),
      the oldest 4 s ago; overlay draws 8 traces with age dimming.
- [x] **AC2** — Memory: N clamps automatically on a 4M-slot stream ring so the segment store
      stays under `kMaxSegmentBytes`, and the UI reflects the clamp.
- [x] **AC3** — Pin segment 3, keep triggering: the pinned trace stays while others rotate.
- [x] **AC4** — Record a session with 5 retained segments, replay it: the same 5 segments are
      available in the stepper.
- [x] **AC5** — `--benchmark-hotpath` unchanged (nothing per sample changed).

## Constraints & Invariants

- The copy in R1 happens on the GUI thread inside the display-tick drain (where the sweep
  engines already live); no allocation per sample; segment rings pre-sized on configure.
- No new mutex; single writer as the sweep engines today.
- Session DB writes go through the existing worker (never on the GUI thread); schema bump per
  the schema-v2 discipline (surrogate keys, no `INSERT OR IGNORE`).

## Overlap with spec 0057 (cascading envelope rings)

A completed sweep is a short window (`activeWindow()`, typically ms to a few s) rendered
through `downsampleWindowAbsolute` at full detail; the pyramid's win is wide windows, so
segments do **not** need coarse levels and should stay plain `TimeRing`-shaped storage. Two
things they could share: (1) the *storage type* for a segment could be a `TimeRing` (level 0
only) copied out of `front`, so no new container is invented; (2) 0057's `EnvelopeRing` is
what the *continuous* history uses, and a future "extract a segment from history around a
marker" feature would read level 0 of that ring. Recommendation: separate storage, shared
types; do not put segments inside the envelope pyramid.

## Open Questions for Alex

- Default N and the byte budget: 8 / 32 MB per plot are guesses.
- Should overlay use age dimming or a colour ramp (persistence-phosphor look vs. discrete)?
- Session DB: store segments as their own table (`sweep_segments`) or as a JSON blob under
  `project_metadata`? A table is queryable and export-friendly; a blob is one migration.
- Multiplot: retain per group (all curves at once, same trigger) — assumed yes.

## Addendum 2026-08-17 — UI reduced to retention

Maintainer review of the shipped UI: the overlay toggle is redundant (retained sweeps should
always draw) and the pin is not readable as a concept; the only control worth keeping is how
many sweeps to keep. The floating segment bar also sat on top of the data.

- **R3 is superseded**: the plot toolbar carries a single retention pill (`< N >`, doubling
  ladder 0-1-2-4-8-16-32-64, showing the memory-clamped count). No stepper, no pin, no overlay
  toggle, no clear button.
- **R6 is reduced** to `sweepRetention` only.
- **AC1/AC3 are void** (they exercised the stepper and the pin). AC2 now reads off the pill.
- `Widgets::Plot` keeps `sweepRetention`, `sweepSegmentCount`, `sweepSegmentCapacity` and
  `drawSegment()`; the stepper/pin/overlay/clear members and slots were deleted, along with
  `Dashboard::clearPlotSweepSegments`.
- `Widgets::MultiPlot` loses the segment surface entirely: it never drew retained segments, so
  retention there had no observable effect. Restoring it means implementing a per-curve
  `drawSegment()` first (`Dashboard::setMultiplotSweepRetention` /
  `clearMultiplotSweepSegments` were deleted with it).
