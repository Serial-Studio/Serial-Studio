---
spec: 0007-dashboard-freeze
title: Dashboard Freeze Mode
status: done         # draft -> approved -> in-progress -> done | shelved
created: 2026-07-14
author: Alex Spataru
---

# Spec 0007 — Dashboard Freeze Mode

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

Users coming from LabVIEW expect to be able to "finish" a dashboard: turn a working layout
into a clean operator front panel that shows only instruments and controls. Serial Studio
cannot do this today — every dashboard widget permanently displays desktop-window chrome
(a caption bar with pop-out/minimize/maximize/close buttons, an optional toolbar band, and
a drop shadow), and the layout stays draggable and resizable at all times. Even with manual
layout and the hidden-taskbar option, the result still reads as a window manager full of
little windows, not an instrument panel. This confuses users evaluating Serial Studio
against LabVIEW-style tools and undermines the "deploy a dashboard to an operator" story
that the Pro output widgets otherwise enable.

The deciding constraint: the frozen state must travel with the project file. A `.ssproj`
prepared by an engineer and handed to an operator has to open already frozen — if freezing
is a per-machine viewer setting, the feature does not deliver the finished-panel promise.

## Goals

- A single "Freeze" toggle converts the current dashboard into a chrome-free panel: every
  widget shows only its inner visualization or controls.
- A frozen project opens frozen on any machine with a valid license, with no extra steps.
- While frozen, the layout is inert: an operator cannot move, resize, close, or otherwise
  disturb the arrangement, only interact with the widgets' contents.
- Freeze composes with the existing manual-layout and hidden-taskbar options so that the
  combination produces a full LabVIEW-style kiosk view.
- Unfreezing restores the exact pre-freeze editing experience with nothing lost.

## Non-Goals

- **Output-widget appearance customization** (style dropdown, color overrides) — separate
  upcoming spec; freeze must not depend on it.
- **Hover-reveal or partial toolbars.** Frozen widgets hide their toolbars entirely;
  reintroducing operator functions (plot pause/zoom, sweep trigger) in a frozen dashboard
  is deliberately deferred.
- **Hiding the taskbar.** The existing taskbar-visibility setting stays independent;
  freeze does not touch it.
- **OS-level kiosk lockdown** (fullscreen enforcement, preventing app exit, hiding the
  main-window menu/toolbar). Freeze governs dashboard widget chrome and layout only.
- **Per-widget freeze granularity.** Freeze is dashboard-wide; individual widgets cannot
  be exempted.
- **Changes to external pop-out windows or the dashboard tools** (console, notification
  log, clock, stopwatch). They keep their current behavior; freeze only prevents creating
  new pop-outs while active (the affordance to do so is hidden with the chrome).

## Requirements

1. **R1** — The user can toggle Freeze on and off via three equivalent entry points: a
   button in the dashboard taskbar area, the keyboard shortcut `Ctrl+Shift+F`, and a
   main-menu item. All three reflect and control the same state.
2. **R2** — While frozen, every dashboard widget hides its caption bar (title and all
   window buttons), its toolbar band, and its window shadow; only the inner widget content
   remains visible, occupying the reclaimed space. *(Amendment 2026-07-14: DataGrid also
   hides its Title/Value header row while frozen.)*
3. **R3** — While frozen, the layout is locked: widgets cannot be dragged, resized,
   restacked, minimized, maximized, closed, or popped out, regardless of whether the
   dashboard is in automatic or manual layout mode.
4. **R4** — While frozen, widget content stays fully interactive: output controls send
   values, plots keep updating, tables scroll, and widgets that accept clicks or wheel
   input inside their content area keep accepting them.
5. **R5** — The frozen state is saved in the project file. Opening a project saved as
   frozen shows the frozen dashboard without the user re-toggling anything. Outside
   project-file mode (e.g. Quick Plot), the toggle works for the current session but is
   not persisted.
6. **R6** — Unfreezing restores the previous chrome and editing behavior exactly: captions,
   buttons, toolbars, shadows, and drag/resize return, and the widget arrangement is
   unchanged from the moment of freezing.
7. **R7** — Freeze is a Pro/Trial feature, gated identically to other Pro features: on
   lower tiers the toggle is visible but disabled with the standard upsell affordance,
   and activating it is impossible.
8. **R8** — A frozen project opened without a qualifying license opens unfrozen, but the
   saved frozen flag survives a load/save round-trip — an unlicensed user editing and
   saving the project does not strip the frozen state.
9. **R9** — If the license activates after a frozen project has loaded (late or offline
   activation), the dashboard becomes frozen without requiring a project reload or app
   restart.
10. **R10** — Freeze state is orthogonal to taskbar visibility: either can be toggled
    without affecting the other, and enabling both yields the full kiosk view.
11. **R11** — While frozen, the taskbar shows a passive frozen indicator positioned next
    to the layout options, so the state is legible whenever the taskbar is visible.
12. **R12** — Freezing captures the dashboard exactly as the user arranged it, including
    a maximized widget: freezing while a widget is maximized keeps it maximized, and that
    presentation is what persists and restores with the project.
13. **R13** *(amendment, maintainer-approved 2026-07-14)* — Widget toolbars share one
    central component and scroll horizontally when the widget is too narrow to show every
    button, instead of hiding entirely; the freeze gate lives in that component, not in
    each widget.

## Acceptance Criteria

- [x] **AC1** — In-app: toggling Freeze from each of the three entry points (button,
  shortcut, menu) flips the same state; all three stay in sync. (Maintainer observation.)
- [x] **AC2** — In-app: with a dashboard containing at least one toolbar widget (e.g. a
  plot), one plain widget, and one output panel, freezing hides all captions, buttons,
  toolbars, and shadows; content fills the reclaimed space; unfreezing restores them with
  the arrangement intact. (Maintainer observation.)
- [x] **AC3** — In-app: while frozen, attempts to drag, resize, or restack widgets in both
  auto and manual layout modes have no effect, while clicking an output button still
  transmits its payload. (Maintainer observation; transmit verified against a live
  connection or the transmit test dialog.)
- [x] **AC4** — Project round-trip: saving a frozen project and reopening it yields a
  frozen dashboard; the saved JSON contains the frozen flag. Verifiable via a `pytest`
  integration test using the API server's project save/load commands once the flag is
  exposed, plus direct inspection of the `.ssproj`.
- [x] **AC5** — Licensing: on an unlicensed build/tier the toggle is disabled with the
  standard upsell; a frozen `.ssproj` opens unfrozen; saving it back preserves the frozen
  flag in the JSON. (Maintainer observation + JSON inspection.)
- [x] **AC6** — Late activation: load a frozen project unlicensed, then activate (offline
  activation path included) — the dashboard freezes without reloading the project.
  (Maintainer observation.)
- [x] **AC7** — The `--benchmark-hotpath` CI gate passes unchanged — freeze is view-layer
  only and must not add per-frame work to the data path.

## Constraints & Invariants

- **No hotpath impact.** The change must not touch frame ingest, dashboard data
  propagation, or anything measured by the 256 kHz benchmark gate; freeze is purely
  presentation and input-handling state.
- **License-gated derived state must re-derive on activation change.** Freeze applied at
  project load is derived from license state; per the composition-root rules, it must
  also respond to a later activation (R9) or offline-activated machines ship a degraded
  view — the exact failure mode that bit Plot3D in July 2026.
- **Unknown-key tolerance.** Older Serial Studio versions opening a project containing the
  new frozen flag must load it normally (existing behavior for unknown project keys must
  be preserved, not assumed).
- **The frozen flag must never be silently dropped** by load/save cycles on any tier (R8);
  losing it is data loss in the same category as the migration/workspace rules.
- **No new dependencies**; works on all supported platforms and in light/dark themes.
- **Existing persistence split stays intact**: project-file state for project-scoped
  settings, application settings for machine-scoped ones — freeze is project-scoped.

## Open Questions

All resolved with the maintainer on 2026-07-14:

- Frozen indicator: yes — passive indicator in the taskbar next to the layout options (R11).
- Shortcut: `Ctrl+Shift+F` (R1); free in the current shortcut map, fits the existing
  `Ctrl+Shift+` family.
- Maximized widget at freeze time: capture as-is, no restore-first (R12).
