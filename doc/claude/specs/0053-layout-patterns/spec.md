---
spec: 0053-layout-patterns
title: Per-workspace auto-layout patterns + frozen shared borders
status: in-progress  # draft -> approved -> in-progress -> done | shelved
created: 2026-08-13
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
- Manual mode remains available and unchanged for arrangements no pattern expresses.

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

## Acceptance Criteria

- [ ] **AC1** — Each shipped pattern can be selected for a workspace and the dashboard
      re-tiles accordingly (maintainer check against a workspace with several widgets).
- [ ] **AC2** — For every pattern, adding and removing widgets across a 1..12 range produces
      a tiling with no gaps, no overlaps, and no widget smaller than the layout floor
      (automated check over the tiling function; no GUI required).
- [ ] **AC3** — The selected pattern and ratio survive save/reload of the project, and
      switching between two workspaces with different patterns applies each correctly
      (pytest project round-trip + maintainer check).
- [ ] **AC4** — An existing project created before this change renders identically before and
      after (maintainer visual comparison against the current build).
- [ ] **AC5** — Changing the split ratio re-tiles immediately and offers only ladder
      fractions (maintainer check).
- [ ] **AC6** — With the dashboard frozen and titlebars hidden, adjacent widgets show one
      shared border; unfreezing or restoring a titlebar returns two (maintainer visual check
      in light and dark themes).
- [ ] **AC7** — The project file gains no merge-related keys, and a project saved while
      frozen-and-merged reloads identical to one saved unfrozen (round-trip test).
- [ ] **AC8** — Manual mode still snaps, welds seams and honors its spacing setting exactly
      as before (existing spec-0052 behavior, re-checked).

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
- Layouts must respect the existing auto-layout margin and spacing settings, and the manual
  layout spacing setting stays independent of them.
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
