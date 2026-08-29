---
spec: 0071-gpu-plot3d-waterfall
title: GPU Rendering for the 3D Plot and Waterfall
status: done         # retro-closed 2026-08-29: work landed, no regressions since; maintainer confirmed
created: 2026-08-26
author: Alex Spataru
---

# Spec 0071 — GPU Rendering for the 3D Plot and Waterfall

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

Placing a 3D plot or a waterfall on a dashboard visibly slows the whole application. The
maintainer reports both widgets as the ones that "really slow down" Serial Studio, and the
slowdown is global rather than local to the widget: because these widgets rasterize on the
GUI thread, the cost lands on the same thread that services the display tick, the toolbar,
and every other widget on the dashboard. One heavy widget therefore degrades the entire
window.

The cause is structural, not incidental. Both widgets rebuild full-screen CPU rasters on
every display tick. The 3D plot allocates and clears a separate screen-sized image for each
of its four layers — data, grid, background, camera indicator — projects every data point on
the CPU, strokes the trace one antialiased line segment at a time with a colour change per
segment (which defeats the raster engine's batching), then composites all four images back
together; with stereo enabled it does all of that twice and adds a full-frame per-pixel merge
pass. The waterfall keeps its history in a ring-ordered image and performs a smooth-scaled
blit of that whole image on every tick, so the spectrogram is resampled in software each
frame even when only one row changed.

The project has already solved this exact class of problem once. Plot, FFT, and MultiPlot
curves used to stroke through a CPU path-triangulation route that stalled on audio-rate data,
and were moved to a purpose-built GPU curve renderer that now carries them. The 3D plot's
trace is the same shape of work — a coloured line strip — and the waterfall's history is the
same shape as a texture. Neither widget needs a general-purpose CPU rasterizer to do what it
actually does.

## Goals

- Adding a 3D plot or a waterfall to a dashboard no longer measurably degrades the frame rate
  or responsiveness of unrelated widgets in the same window.
- Both widgets stop performing full-screen CPU rasterization and full-screen image compositing
  on the display tick.
- The waterfall's cost per tick scales with the data that actually changed (a new row), not
  with the on-screen size of the widget.
- The 3D plot's cost per tick scales with the number of plotted points, not with the widget
  area times the number of layers.
- Both widgets keep their current visual result and their current interactive behaviour, so
  the change is invisible to users except as speed.

## Non-Goals

- The console/terminal and the map/GPS widgets are **out of scope** for this spec. Their
  bottleneck is text and vector rasterization rather than full-screen compositing, and they
  are addressed by a separate follow-up spec.
- The embedded code editors and the custom title bar are **not** being moved off their
  current painting path. Their repaint rate is user-input driven and does not stall anything.
- No new external dependency, and no bump of the project's pinned Qt version, is in scope.
  The follow-up spec for the console and map may require both; this one must not.
- No visual redesign. No new features, options, or settings for either widget. Colour
  schemes, axes, markers, cursors, grids, camera behaviour, and stereo remain as they are.
- Restoring rendering under the Software fallback backend is not a goal (see Constraints).

## Requirements

1. **R1** — With a live data source running, a dashboard containing a 3D plot holds the same
   frame rate and input responsiveness as an equivalent dashboard without one, within the
   normal run-to-run variation of the machine.
2. **R2** — The same holds for a dashboard containing a waterfall.
3. **R3** — A dashboard containing both a 3D plot and a waterfall simultaneously remains
   interactive: dragging, resizing, and toolbar interaction do not stutter.
4. **R4** — The 3D plot renders the same scene as before: trace with its colour gradient,
   grid, background gradient, and camera indicator, in the same draw order, with the same
   depth ordering behaviour as the camera rotates past its current threshold.
5. **R5** — All existing 3D plot camera interactions — orbit, pan, zoom, auto-centre,
   auto-scale, and the fit behaviour — continue to work with unchanged feel.
6. **R6** — 3D plot stereo/anaglyph rendering continues to produce a red-cyan image with the
   existing eye-separation and inverted-eye controls behaving as they do today. The result
   must be visually equivalent; it is not required to be pixel-identical to the current
   output.
7. **R7** — The waterfall renders the same spectrogram content, with its history scrolling in
   the same direction and its oldest-row wrap-around producing no visible seam or tear.
8. **R8** — Waterfall axes, markers, marker chips, the hover cursor readout, Campbell-mode row
   placement, and logarithmic-X resampling all continue to behave as they do today.
9. **R9** — Both widgets continue to follow the dashboard's plot time range and frame-rate
   settings, and continue to freeze when the dashboard is paused.
10. **R10** — Resizing either widget, and moving the window between displays with different
    device pixel ratios, produces correctly scaled output with no stale or blurred frame.
11. **R11** — Neither widget allocates per frame in its steady state; buffers are resized only
    when the data extent or the widget geometry actually changes.
12. **R12** — Both widgets remain Pro-gated exactly as they are today, including when a
    license is activated or deactivated while the dashboard is open.

## Acceptance Criteria

- [x] **AC1** — A `sample` capture of the running application, taken while a dashboard with a
      3D plot and a waterfall is receiving live data, shows no full-screen rasterization or
      image-composition frames attributable to either widget on the GUI thread. A matching
      "before" capture is recorded first so the two can be compared directly.
- [x] **AC2** — Maintainer observation in the running app: with a live source, a dashboard
      carrying a 3D plot and a waterfall feels as responsive as one without them, and the
      widgets themselves animate smoothly.
- [x] **AC3** — Side-by-side screenshots of each widget before and after, at the same window
      size, data, and theme, are visually equivalent for R4, R7 and R8. Stereo mode is checked
      the same way against R6's visual-equivalence bar.
- [x] **AC4** — Manual interaction pass in the running app covering R5, R8, R9 and R10:
      orbit/pan/zoom the 3D plot, hover and click waterfall markers, toggle pause, resize both
      widgets, and drag the window between displays of different pixel ratios.
- [x] **AC5** — The existing `--benchmark-hotpath` gate still passes at its configured
      thresholds, confirming the acquisition pipeline was not disturbed.
- [x] **AC6** — `scripts/code-verify.py --check` reports no new errors, and the translation-unit
      and singleton census gates do not regress.
- [x] **AC7** — A Pro license is activated and deactivated with a dashboard open; both widgets
      appear and disappear correctly, with no crash and no stale rendering (R12).

## Constraints & Invariants

- **Frame construction runs on the render thread while the GUI thread is blocked.** Every value
  either widget reads from the dashboard or the project model must be captured on the GUI
  thread before the frame is built. No dashboard model may be read directly from render-thread
  code.
- **No steady-state allocation while rendering.** Buffers grow only on a real change of data
  extent or widget geometry.
- **Repaints stay driven by the existing display tick.** No new timer, no new per-frame signal,
  and no change to the tick cadence.
- **The acquisition pipeline is untouched.** This is a presentation-layer change only; no
  hotpath, threading, or publication behaviour changes.
- **Reuse the rendering approach already proven in the project for GPU curves.** Do not
  introduce a parallel or competing rendering mechanism, and do not add a dependency.
- **The Software fallback backend keeps its current, known limitation.** Custom GPU geometry
  does not render under it, exactly as plot curves already do not. Both widgets may degrade
  there; no duplicate CPU rendering path will be maintained to cover it. This gap is
  pre-existing and is explicitly accepted, not introduced by this work.
- **Both widgets are Pro features** and stay gated as they are.
- **No change to project file format, API surface, or persisted settings.**

## Open Questions

- The "before" performance capture (AC1) has not been taken yet — it needs the application
  running with a live source. Whether the 3D plot and the waterfall contribute comparably, or
  one dominates, will decide which is ported first and is worth knowing before `/ss-plan`
  fixes an order.
- The waterfall's ring buffer already carries a flagged latent issue noted in the dashboard
  architecture documentation. Whether that is fixed as part of this work or deliberately left
  alone should be decided at plan time rather than absorbed silently into the port.
- Accepting the Software-backend gap makes it wider than it is today. Whether the application
  should warn users who select that backend that some widgets will not render is a separate,
  small question worth answering, but not part of this spec.
