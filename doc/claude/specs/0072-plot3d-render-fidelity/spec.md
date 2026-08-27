---
spec: 0072-plot3d-render-fidelity
title: Plot3D Render Fidelity — Stereo Channel Isolation and Bounded Drawing
status: in-progress  # draft -> approved -> in-progress -> done | shelved
created: 2026-08-27
author: Alex Spataru
---

# Spec 0072 — Plot3D Render Fidelity

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

Spec 0071 moved the 3D plot off the CPU rasterizer and onto the GPU. The old renderer drew
the entire scene — background included — twice, into two images the size of the widget, and
then merged them one channel at a time: the red channel came from the left eye's image, green
and blue from the right eye's. Two properties fell out of that arrangement for free, and the
port lost both.

**The stereo channels stopped being independent.** Under the merge, a channel an eye did not
own kept whatever the *background* had there. The GPU port reproduced this by zeroing those
channels in the source colour instead and drawing the result as an ordinary blended stroke, so
each ghost now paints the unowned channels black rather than leaving them alone. On the default
light theme the minor grid's left ghost went from a pale cyan `rgb(183,240,241)` to a muddy
`rgb(183,146,146)`; the report was that "the 3D parallelogram makes blue look brown instead of
red". An interim correction is in the working tree — it fills the unowned channels from an
estimate of the background and carries luminance on the red channel — and it restores the grid
to within about three counts of the old output. It cannot go further, for two reasons that are
inherent to blending the two eyes on top of each other: the second ghost drawn always
attenuates the first, so the first eye tops out somewhere between a quarter and a half of the
contrast the merge produced (visibly dimmer trace ghosts, worst at small eye separations where
the two ghosts overlap almost completely); and the unowned channels are filled from a single
background estimate rather than the real gradient, which can leave a faint seam of up to about
9/255 on the darker themes.

**The trace stopped being bounded.** Every layer used to be an image the size of the widget, so
anything projecting past the edge was clipped by construction. The trace is now geometry with
no bounds culling and nothing clipping it, so a trace that projects off the widget is drawn
over its neighbours — sibling dashboard widgets, the dashboard toolbar, and the window chrome
above it. The grid and axes are not affected; they are culled during projection. The label and
camera-indicator overlays are not affected either; they are placed as bounded rectangles.

Both defects are visible on a normal dashboard, and the second one paints outside the widget
that owns the pixels, which no widget is allowed to do.

## Goals

- With stereo enabled, each eye's image reads at its full intended contrast, unchanged by
  whether the other eye's image happens to overlap it.
- With stereo enabled, the parts of the picture an eye does not own show the real background,
  exactly, with no tint, darkening or seam.
- Every dataset and theme colour produces a visible image for *both* eyes, on light and dark
  themes alike — including the cases the pre-0071 merge itself got wrong.
- The 3D plot never paints a pixel outside its own bounds, whatever the camera is doing.
- The non-stereo view is untouched, pixel for pixel.

## Non-Goals

- No change to the stereo geometry: eye separation, eye inversion, and the camera model stay
  exactly as they are. This spec is about which pixels get which colour, and where drawing
  stops — not about where the two views are placed.
- No new user-facing setting. The stereo colour treatment is not being made configurable.
- No change to any other widget's rendering. The sibling widget ported in the same commit
  places every one of its layers as a bounded rectangle derived from its own plot area, so it
  does not share the second defect; it is out of scope.
- Not a performance exercise. The port's throughput win is to be preserved, not extended.

## Requirements

1. **R1** — With stereo enabled, the channels an eye does not own are left at the background
   value. Sampling a pixel covered by only one eye's image must show that eye's channels
   changed and the other eye's channels equal to the background at that pixel, including
   across the background gradient.
2. **R2** — With stereo enabled, an eye's image contrast does not depend on whether the other
   eye's image overlaps it. Sweeping eye separation across its full range must not change how
   strongly either eye's image reads.
3. **R3** — Every dataset colour offered by every shipped theme produces a visible image for
   both eyes on both a light and a dark theme. Specifically, the two cases the previous
   renderer failed — a red axis on a light background and a blue trace on a dark one — must
   both be visible through the red lens.
4. **R4** — The stereo picture keeps the dataset's identity: the source colour remains
   recognisable through the cyan lens and in the plain on-screen view, rather than the plot
   going monochrome when stereo is switched on.
5. **R5** — With stereo disabled, the rendered output is identical to the current build
   *wherever the current build stayed inside the widget's own bounds*. R6 necessarily changes
   the one place it did not: pixels the current build painted outside those bounds are gone.
   The two requirements are in direct conflict there and R6 wins, because a widget drawing over
   the window chrome is the worse defect.
6. **R6** — Nothing the 3D plot draws appears outside the widget's own bounds, at any camera
   angle, zoom level, world scale or eye separation, both embedded in the dashboard and in a
   popped-out widget window.
7. **R7** — If the rendering path required for R1 and R2 cannot be established at runtime, the
   widget falls back to the interim colour treatment already in the tree rather than rendering
   nothing. Stereo continues to work, at the reduced contrast that treatment implies.
8. **R8** — No allocation is added to the per-tick render path, and the widget's per-tick
   render cost does not regress.

## Acceptance Criteria

- [ ] **AC1** (R1, R3, R4) — A C++ unit under the `ctest` tier feeds each shipped theme's
      dataset, axis and grid colours through the per-eye colour derivation and asserts, for
      each: the owned channels carry the expected value, the unowned channels are marked as
      not written, and the contrast against that theme's background clears a stated minimum
      for both eyes. The two historical failure cases are named fixtures in that unit.
- [ ] **AC2** (R2) — Maintainer observation in the running app: with a project open and stereo
      enabled, sweep eye separation from minimum to maximum and confirm neither ghost fades as
      the two converge. Before this change the first-drawn ghost visibly weakens as separation
      drops.
- [ ] **AC3** (R1) — Maintainer observation on the default light theme: the minor grid's two
      ghosts read as pale cyan and pale red against the background, with no brown or muddy
      cast, matching the pre-0071 screenshots.
- [ ] **AC4** (R5) — Maintainer observation with stereo off, on both a light and a dark theme:
      grid, axes, trace, labels and camera indicator are indistinguishable from the current
      build. Frame the camera so nothing projects past the widget edge — otherwise this fails
      on the R6 clip, which is the fix working, not a regression.
- [ ] **AC5** (R6) — Maintainer observation: with a 3D plot on a dashboard alongside other
      widgets, zoom and orbit until the trace would leave the widget, and confirm nothing is
      drawn over the neighbouring widgets, the dashboard toolbar or the window titlebar.
      Repeat in a popped-out widget window against that window's own chrome.
- [ ] **AC6** (R7) — A C++ unit or in-app self-test asserts that when the preferred rendering
      path is unavailable, the widget still produces stereo output through the fallback rather
      than an empty scene.
- [ ] **AC7** (R8) — `scripts/code-verify.py` is clean, the translation-unit and singleton
      censuses are at or below baseline, and the maintainer confirms no visible change in
      dashboard smoothness with a 3D plot live.

## Constraints & Invariants

- **The non-stereo path is the common path and must stay bit-identical.** Stereo is a niche
  mode; nothing done for it may cost or change anything when it is off.
- **No new build dependency and no new install step.** Whatever the rendering path needs must
  already be present in the Qt the project builds against today and in CI.
- **The widget's translation unit is at its size ceiling.** It currently sits exactly at the
  1500-line limit the linter enforces, so new code has to move out into its own translation
  unit rather than accrete onto the existing one.
- **No allocation on the per-tick render path**, consistent with the rest of the dashboard.
- **Pro-gated feature.** The 3D plot is Pro; nothing here may change the gating or make a
  GPL-only build behave differently.
- **Fail soft, never blank.** A rendering path that cannot be established must degrade to a
  worse-looking picture, never to an empty widget (R7).

## Open Questions

- The interim colour treatment currently in the working tree is uncommitted. Should it land as
  its own commit first — so the visible defect is fixed for users immediately and this spec
  becomes a follow-up improvement — or be folded into the single commit that implements this
  spec? Landing it first is the safer sequencing if this work slips, and it is the fallback
  path R7 depends on either way.
- R6 can be satisfied either by bounding the whole widget's drawing or by culling the trace
  during projection the way the grid already is. The two differ in cost and in what happens to
  a trace that only partly leaves the view. Deferred to `/ss-plan`.
- R2 asks that contrast not depend on overlap. Worth confirming during the plan whether that
  holds where a ghost crosses the label and camera-indicator overlays, which are drawn as
  bounded images rather than as stroked geometry and so may not participate in the same
  channel discipline.
