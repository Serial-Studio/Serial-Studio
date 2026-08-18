---
spec: 0061-sweep-segment-retention
phase: plan
status: approved     # gate auto-approved, overnight run 2026-08-17 (unattended; maintainer re-reviews)
updated: 2026-08-17
---

# Plan 0061 — Sweep segment retention

## Approach (one paragraph)

`DSP::SweepEngine` grows a bounded ring of `SweepSegment { triggerSec, windowSec,
curves[] }` (`segments`, `segmentHead`, `segmentFill`, `segmentRetention`). `setSegmentRetention(n)`
clamps `n` to `kMaxSegments` (64) and to `kMaxSegmentBytes` (32 MB) / (curves x capacity x 16 B),
pre-sizing every segment's rings so `completeSweep()` never allocates; `retainFront()` deep-copies
the just-completed `front` rings element by element (`FixedQueue` copies alias their storage, so a
plain assignment would corrupt older segments). Segments read newest-first through `segment(i)`,
`resetState()` and `clearSegments()` drop them, and `takeSegmentsFrom()` carries them across a
Time-Range / layout rebuild when the shape matches (wired into `restore*SweepConfig`). `Dashboard`
exposes retention/clear slots; `Widgets::Plot` gains `sweepRetention`, `sweepSegmentIndex`
(stepper, -1 = live), `sweepPinnedSegment`, `sweepOverlay`, `sweepSegmentCount/Capacity/AgeSec`,
`drawSegment()` / `drawPinnedSegment()`; `MultiPlot` gains retention + stepper. QML: a segment
bar over the plot (retention SpinBox, older/newer, pin, overlay, clear), an age-dimmed
`Repeater` of `PlotCurve`s for the overlay and one reference curve for the pin; `sweepRetention`
and `sweepOverlay` persist via `widgetSettings`.

## Decisions taken for the open questions

| Question | Decision |
|----------|----------|
| Default N / budget | 0 (off) by default, max 64, 32 MB per plot |
| Overlay look | age dimming (opacity 0.62 -> 0.12), same curve colour |
| Session DB persistence (R5) | **deferred** — needs a schema bump in the Sessions worker/player (foreign-modified files tonight); recorded as the open follow-up |
| Multiplot | retains per group (all curves at once); stepper + clear only, no overlay/pin in this slice |

## Hotpath & threading impact

None on the pipeline thread. `retainFront()` runs on the GUI thread inside the display-tick drain
at sweep-completion rate (a bounded ring walk); segments are pre-sized. No mutex, no allocation
per sample.

## Test & verification plan

- `tst_sweep_segments` (ctest, unbuilt tonight): budget clamp, newest-first order, deep-copy
  independence, wrap/clear/reset, shape-checked takeover.
- Spec AC1-AC3, AC5 in the running app; AC4 (session DB) deferred with R5.
