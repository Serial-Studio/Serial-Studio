---
spec: 0053-layout-patterns
title: Per-workspace auto-layout patterns + frozen shared borders
status: done          # closed 2026-08-20
created: 2026-08-13
amended: 2026-08-14  # R11-R13 / AC9-AC12: constant manual metrics + picker on the auto-layout button
author: Alex Spataru
---

# Spec 0053 — Per-workspace auto-layout patterns + frozen shared borders

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

Auto layout ships exactly one arrangement: equal-width columns, with the column count read
from a fixed table keyed on widget count. Every dashboard therefore looks the same, and every
widget is granted equal visual weight regardless of importance. A test cell watching one EGT
plot alongside six supporting readouts cannot express that hierarchy.

The only escape today is manual mode, which asks the user to place and size every window by
hand. That is a lot of work for a common intent ("one big plot, the rest around it"), and the
result is fragile: a manual layout is a set of absolute rectangles, so it has to be rescaled
and re-welded on every canvas resize (spec 0052 follow-ups added fraction snapping and seam
welding precisely to keep those layouts intact). Manual mode is the right tool for genuinely
bespoke arrangements; it should not be the price of wanting a master-and-stack dashboard.

Separately, a frozen dashboard built from titlebar-less widgets still draws two borders at
every seam — each widget renders its own edge — so a layout intended to read as one
continuous instrument panel reads as a grid of separate boxes.

**Amendment, 2026-08-14 — two gaps found while building this spec.**

*Manual layouts do not survive a window resize.* The rescale treats a manual layout as a set
of proportional rectangles, so the canvas margin and the gap between two widgets are scaled by
the same factor as the widgets themselves: with a 4 px spacing, doubling the dashboard width
turns every 4 px gap into 8 px, and halving it closes the gaps entirely. The seam repair that
runs afterwards only recognizes gaps below a fixed pixel tolerance, so a larger spacing
setting is never repaired at all, and when it does fire it re-centers the seam — moving it a
fraction of the spacing each time and trading pixels between the two neighbours. Because the
resulting geometry is what gets saved, that shift compounds across resizes and sessions until
the arrangement no longer resembles what the user built. Committing a move or resize of a
single widget after the dashboard has been resized also re-bases the reference size shared by
*all* widgets, so the untouched ones jump back to a stale arrangement on the next resize.
Margin and spacing are pixel settings; the user expects them to look the same at every window
size.

*The pattern picker is not discoverable.* The picker was scoped into the canvas context menu,
which is where users do not look. The taskbar auto-layout button is the control they already
associate with "how is this dashboard arranged", so that is where the choice belongs.

## Goals

- A user picks a named layout pattern per workspace from a visual picker and the dashboard
  re-tiles into it, with no manual placement.
- Every pattern works for any widget count: adding or removing a widget re-flows the pattern
  rather than breaking it or leaving holes.
- Patterns that have a primary/secondary split expose that split as a per-workspace ratio,
  chosen from the same wrench-fraction vocabulary the manual snapping already uses.
- A frozen dashboard whose widgets hide their titlebars reads as one continuous surface:
  adjacent widgets share a single border instead of drawing one each.
- Existing projects keep their current appearance: the default pattern reproduces today's
  equal-grid behavior exactly.
- Manual mode remains available for arrangements no pattern expresses, and a manual layout
  keeps its look at any dashboard size.
- The arrangement choice — pattern, split ratio, or manual — is reachable from the control the
  user already uses to change the layout.

## Non-Goals

- Not a replacement for manual mode, and not a migration away from it. The spec-0052
  fraction-snapping and seam-welding work stays load-bearing.
- No user-authored patterns in this iteration: the catalog is the set the application ships.
  (The picker artwork is per-pattern, so adding one later stays cheap.)
- No per-widget cell assignment or pinning ("always put this widget in the master cell") —
  patterns consume the existing widget order.
- No change to widget rendering, contents, or the dashboard data path.
- No change to how workspaces themselves are created, ordered, or persisted beyond the two
  new per-workspace values.
- The shared-border merge is presentation only; it must not alter stored geometry.

## Requirements

1. **R1 — Pattern catalog.** The application ships a fixed set of named layout patterns:
   Grid (today's behavior), Master + Stack, Master + Grid, Row, Column, and Spiral. Each has
   a human-readable name and its own artwork for the picker.
2. **R2 — Any widget count.** Every pattern produces a valid, gap-free tiling for any widget
   count from 1 upward. No widget is ever hidden, zero-sized, or overlapped by the pattern.
3. **R3 — Per-workspace selection.** The chosen pattern is a property of the workspace and
   persists in the project file. Switching workspaces applies that workspace's pattern.
4. **R4 — Default preserves today.** A workspace with no stored pattern renders exactly as
   the current auto layout does, so opening an existing project changes nothing on screen.
5. **R5 — Visual picker.** The user chooses the pattern from a picker showing each pattern's
   artwork, so the arrangement is recognizable without applying it.
6. **R6 — Tunable split ratio.** Patterns with a primary region (Master + Stack, Master +
   Grid, Spiral) expose a split ratio stored per workspace. The offered values are wrench
   ladder fractions; the default is 1/2.
7. **R7 — Frozen shared borders.** While the dashboard is frozen, two adjacent widgets that
   both hide their titlebar render their shared edge as a single border rather than two.
8. **R8 — Merge is presentation-only and reversible.** Unfreezing, or a titlebar becoming
   visible again, restores normal borders. No merge state is written to the project.
9. **R9 — Patterns are the default path.** New workspaces start in pattern mode; manual mode
   stays reachable and behaves exactly as it does today when selected.
10. **R10 — Free tier.** Patterns, the picker, the ratio control, and the shared-border merge
    are available in GPL builds.
11. **R11 — Layout metrics are absolute.** The canvas margin and the inter-widget spacing are
    pixel settings in both layout modes. Resizing the dashboard changes neither: the widgets
    absorb the size change, the gaps do not scale, grow, shrink, or close. A layout whose
    widgets sat flush against each other stays flush; one built at the configured spacing keeps
    exactly that spacing.
12. **R12 — Resizing is lossless.** Resizing the dashboard away from a size and back restores
    the layout that was there, and repeated resizes introduce no cumulative drift — geometry is
    always re-derived from what the user authored, never from the previous rescale's output.
    Editing one widget after a resize leaves every other widget where it is. Saving and
    reloading at a
    different dashboard size preserves the arrangement's structure — which edges are shared,
    which are flush with the canvas — not merely the approximate rectangles.
13. **R13 — The layout choice lives on the auto-layout button.** The taskbar auto-layout button
    opens a gallery of the shipped patterns plus a Manual entry, with the active choice marked
    and the split ratio offered for patterns that have a primary region. Choosing a pattern
    applies it immediately; choosing Manual switches to manual mode. The canvas context menu
    may keep a route to the same gallery, but the button is the primary one.

## Acceptance Criteria

- [x] **AC1** — Each shipped pattern can be selected for a workspace and the dashboard
      re-tiles accordingly (maintainer check against a workspace with several widgets).
- [x] **AC2** — For every pattern, adding and removing widgets across a 1..12 range produces
      a tiling with no gaps, no overlaps, and no widget smaller than the layout floor
      (automated check over the tiling function; no GUI required).
- [x] **AC3** — The selected pattern and ratio survive save/reload of the project, and
      switching between two workspaces with different patterns applies each correctly
      (pytest project round-trip + maintainer check).
- [x] **AC4** — An existing project created before this change renders identically before and
      after (maintainer visual comparison against the current build).
- [x] **AC5** — Changing the split ratio re-tiles immediately and offers only ladder
      fractions (maintainer check).
- [x] **AC6** — With the dashboard frozen and titlebars hidden, adjacent widgets show one
      shared border; unfreezing or restoring a titlebar returns two (maintainer visual check
      in light and dark themes).
- [x] **AC7** — The project file gains no merge-related keys, and a project saved while
      frozen-and-merged reloads identical to one saved unfrozen (round-trip test).
- [x] **AC8** — Manual mode still snaps, welds seams and honors its spacing setting exactly
      as before (existing spec-0052 behavior, re-checked).
- [x] **AC9** — For a manual layout of several adjacent widgets rescaled across a range of
      canvas sizes (including non-uniform aspect changes), every shared edge measures exactly
      the configured spacing and every outer edge sits flush with the canvas, for spacing
      values of -1, 0, 4 and 16 (automated check over the rescale function; no GUI required).
- [x] **AC10** — Rescaling a manual layout from its stored reference to another canvas size and
      back returns geometry identical to the start, ten times over. Where a layout is instead
      re-derived from an intermediate size — a save taken there, or an edit committed there —
      integer rounding may move an edge by at most 2 px on the first hop; repeating that cycle
      50 times must not move it further, and joins and outer-edge flushness stay exact
      throughout (automated).
- [x] **AC11** — After the dashboard is resized, moving or resizing one widget leaves every
      other widget's geometry unchanged at commit time; returning to the earlier size leaves
      them within the AC10 bound of their earlier geometry, joins and flushness exact. The
      commit re-bases the reference to the current canvas by design — that size is what the
      user just authored against (automated plus maintainer check).
- [x] **AC12** — Clicking the taskbar auto-layout button opens the gallery with the active
      choice marked; each pattern and the Manual entry apply on selection, and the ratio
      control appears only for patterns with a primary region (maintainer check).

## Constraints & Invariants

- **Presentation only for the merge.** Stored window geometry must not change when widgets
  merge borders; the effect must survive freeze/unfreeze cycles without drift.
- **Additive persistence.** The two new per-workspace values are optional; their absence
  means "default pattern, default ratio", and older builds must ignore them harmlessly.
- **No hotpath impact.** Layout runs on gesture, resize, workspace switch and widget-count
  change — never per frame. The 256 kHz gate must be unaffected.
- **Tiling stays deterministic and pure.** Given a widget count, canvas size, pattern and
  ratio, the resulting rectangles must be reproducible — this is what makes AC2 testable
  without a GUI.
- Both layout modes read the one shared margin and spacing pair; neither mode may reintroduce
  a private copy of either value.
- Rescaling a manual layout must be a pure function of the stored layout and the two canvas
  sizes: applying it twice with the same inputs yields the same output. Nothing may be derived
  from the geometry it just produced.
- A canvas too small to hold the configured gaps degrades by reducing them, never by
  overlapping widgets or driving one below the minimum size.
- Must honor the existing minimum window size; a pattern that would violate it on a small
  canvas degrades (fewer subdivisions) rather than producing unusable windows.
- No new dependency; artwork ships as bundled resources like the existing icon sets.

## Open Questions

- None blocking. Resolved with the maintainer on 2026-08-13: patterns are parametric in the
  widget count (not fixed cell sets); the SVG is picker artwork only, with geometry declared
  in code/manifest; the catalog is Grid, Master + Stack, Master + Grid, Row, Column, Spiral;
  the split ratio is per-workspace and restricted to wrench ladder stops; the shared-border
  merge is automatic while frozen for titlebar-less neighbours; manual mode is kept with
  patterns as the default path.
- Resolved with the maintainer on 2026-08-14 (amendment): the auto-layout button opens the
  gallery popup rather than keeping its one-click auto/manual toggle — Manual is one of the
  entries in the gallery.
