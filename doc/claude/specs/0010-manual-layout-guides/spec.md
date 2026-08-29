---
spec: 0010-manual-layout-guides
title: Figma-style alignment guides and small-window support in manual layout mode
status: done         # draft -> approved -> in-progress -> done | shelved
created: 2026-07-15
author: Alex Spataru
---

# Spec 0010 — Figma-style alignment guides and small-window support in manual layout mode

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

Manual layout mode (auto-layout off) is where final dashboard layouts are composed — the
freeze feature (spec 0007) exists precisely so a hand-built layout can be locked and shipped
to operators. But the mode offers no precision tools: while dragging, the only aid is an
Aero-style snap that offers half/quarter-canvas rectangles near the canvas edges, and there
is no way to align a window to its neighbors — no edge or center snapping between windows,
no equal-spacing help, no readout of position or size. Lining up a row of gauges today means
eyeballing pixel offsets, which produces visibly ragged layouts exactly where polish matters
most.

Separately, every dashboard window enforces a 356×320 minimum. Instrument-panel layouts
want small tiles — a compact LED cluster, a narrow bar, a small numeric readout — and the
current floor makes dense panels impossible; widgets already scale their own content down
(title strips hide below 96 px width, toolbars hide below a height threshold), so the floor
is the delegate's, not the widgets'.

## Goals

- A user arranging windows in manual mode can align any window edge or center to any other
  window's edge or center, to the canvas edges, and to the canvas centerlines, with visual
  guide lines confirming the alignment at the moment it happens — for both move and resize.
- A user building rows or columns of windows gets equal-spacing assistance: when the gap
  being created matches a neighboring gap, the drag snaps to it and the matching gaps are
  indicated.
- A user can match one window's width or height to another's during resize, with a visual
  cue when the sizes are equal.
- A user always knows the exact geometry of the window being manipulated via an on-canvas
  x/y/w×h readout in pixels.
- A user who wants a fixed rhythm can enable an optional grid and have moves/resizes snap
  to it.
- A user can place a window completely freely: holding a modifier suspends all snapping for
  the duration of the gesture.
- Windows in manual mode can be resized down to 48×48 px, small enough for dense instrument
  tiles while still showing the caption buttons.

## Non-Goals

- No change to auto-layout mode: its packing, sizing, and drag-to-reorder behavior stay as
  they are, including its existing minimum-size assumptions.
- No change to external pop-out windows (their 356×320 minimum and OS-managed behavior stay).
- The half/quarter-canvas Aero snap is removed from manual mode, not redesigned or hidden
  behind a modifier — manual mode becomes pure freeform plus smart guides.
- No persistent user-placed ruler guides (Figma's draggable guide lines) and no multi-select
  / group alignment operations; guides apply to the single window being manipulated.
- No redesign of widget content rendering at small sizes: widgets keep their existing
  degradation behavior (hiding titles, toolbars, labels) and may look minimal at 48×48;
  that is accepted.
- No change to frozen mode: manipulation stays fully disabled when frozen.

## Requirements

1. **R1 — Edge/center snap on move.** While dragging a window in manual mode, the drag
   position snaps, within a small threshold, to alignments between the dragged window's
   edges/centers and (a) other visible windows' edges/centers, (b) the canvas edges, and
   (c) the canvas horizontal/vertical centerlines. When a snap holds, a guide line renders
   along the full extent of the alignment.
2. **R2 — Edge/center snap on resize.** While resizing, the moving edge(s) snap to the same
   target set, with the same guide-line feedback. Non-moving edges never shift.
3. **R3 — Equal-spacing snap.** While dragging, when the gap between the dragged window and
   a neighbor can match an existing gap between that neighbor and the next window along the
   same axis, the position snaps to equalize the gaps and the matched gaps are visually
   indicated.
4. **R4 — Size-match snap.** While resizing, when the in-progress width or height comes
   within the threshold of another visible window's width or height, the size snaps to match
   and the matching window is visually indicated.
5. **R5 — Geometry readout.** During any manual move or resize, a compact badge shows the
   window's current x, y, width, and height in canvas pixels, updating live and disappearing
   when the gesture ends.
6. **R6 — Optional grid.** A user-facing toggle enables a rendered background grid in manual
   mode with a configurable cell size; while enabled, moves and resizes snap to grid lines.
   Grid snap and smart guides compose, with smart guides winning when both are in range.
   The preference persists across sessions.
7. **R7 — Snap bypass.** Holding Alt during a move or resize suspends all snapping (guides,
   spacing, size-match, grid) for that gesture; releasing Alt mid-gesture restores it.
8. **R8 — Aero snap removed.** The half/quarter-canvas snap rectangles no longer appear in
   manual mode; dropping a window near a canvas edge leaves it exactly where the (possibly
   snapped) drag put it.
9. **R9 — 48×48 manual minimum.** In manual mode, windows can be resized down to 48×48 px
   and no smaller; caption buttons remain visible and clickable at that size. Auto-layout
   mode and external windows keep today's minimums, and switching a small-sized layout back
   to auto-layout must not break packing.
10. **R10 — Guides are manual-mode only.** No guide, badge, grid, or snap behavior appears
    in auto-layout mode or while frozen, and none of it appears when merely hovering (only
    during an active move/resize gesture).
11. **R11 — Layout persistence unchanged in shape.** Saved manual geometries, including
    sub-356×320 sizes, persist and restore exactly like today's layouts (per project and
    canvas-size bucket), and older saved layouts load unchanged.

## Acceptance Criteria

- [x] **AC1** — In-app: drag a window slowly past another window's left edge, top edge, and
  horizontal center; each alignment produces a visible snap plus guide line, and the dropped
  geometry differs by 0 px from the target alignment (verify via the R5 badge).
- [x] **AC2** — In-app: resize a window's right edge toward a neighbor's right edge; it
  snaps flush, and the left edge does not move during the entire gesture.
- [x] **AC3** — In-app: with three windows in a row, drag the third until its gap matches
  the first pair's gap; the spacing indicator appears and the two gaps are pixel-identical
  after drop.
- [x] **AC4** — In-app: resize a window until its width matches a sibling's; the size-match
  cue appears and both widths read identical in the badge.
- [x] **AC5** — In-app: the badge appears on gesture start, tracks live, matches the final
  saved geometry, and disappears on release.
- [x] **AC6** — In-app: enable the grid at a chosen cell size; moves land on grid lines;
  disable it and freeform placement returns; the toggle state survives an app restart.
- [x] **AC7** — In-app: holding Alt while dragging through a known snap alignment produces
  no snap and no guides; releasing Alt mid-drag re-enables them within the same gesture.
- [x] **AC8** — In-app: dragging a window against all four canvas edges never shows the old
  half/quarter-canvas snap rectangle in manual mode.
- [x] **AC9** — In-app: a window resizes down to exactly 48×48 and stops; close, minimize,
  and maximize buttons remain visible and respond to clicks at that size; a 48×48 window
  survives save/reload of the layout at the same geometry.
- [x] **AC10** — In-app: toggle to auto-layout with several sub-356×320 windows present;
  packing succeeds with no overlapping or zero-sized windows; toggling back restores the
  saved manual geometry (existing round-trip behavior).
- [x] **AC11** — In-app: with freeze enabled, no gesture, guide, badge, or grid interaction
  is possible; with auto-layout enabled, none of the new visuals ever appear.
- [x] **AC12** — Regression: `--benchmark-hotpath` gates unchanged (the feature is
  interaction-time UI only; nothing runs on the frame path).

## Constraints & Invariants

- **Deciding constraint: precision without stickiness.** Snapping must make pixel-exact
  alignment the easy default while keeping ordinary dragging fluid — small thresholds,
  instant visual feedback, and a reliable bypass. A snap system that fights the user is
  worse than none.
- Guide computation and rendering run only during an active gesture at interaction rate;
  nothing is added to the telemetry frame path, and dashboards with many windows (30+) must
  not stutter while dragging.
- Frozen-mode input gating stays absolute: freeze continues to abort/refuse every
  manipulation path, unchanged by any new gesture logic.
- Manual-geometry persistence keeps its existing storage shape (per canvas-size bucket, per
  project); old layouts load byte-compatibly, and layouts saved with small windows must not
  crash or mis-restore on older builds' 356×320 floor beyond clamping up.
- The caption bar, its buttons, and hit targets must remain usable at 48 px width — whatever
  chrome cannot fit must collapse gracefully rather than overflow or become unclickable.
- Guide/grid visuals follow the active theme (no hardcoded colors) and must be legible over
  user-set background images.
- Free feature: no Pro/tier gating on any part of it.
- RTL locales: guides and badge must render correctly with `LayoutMirroring` in play.

## Open Questions

- Grid default cell size and where the toggle lives (dashboard toolbar vs. Settings →
  Dashboard) — plan proposes, maintainer picks.
- Whether the equal-spacing snap should also work on resize (Figma limits it to move;
  proposed scope follows Figma).
