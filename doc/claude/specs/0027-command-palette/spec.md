---
spec: 0027-command-palette
title: Generic Command Palette (contexts, unified search, workspace switching)
status: done         # draft -> approved -> in-progress -> done | shelved
created: 2026-07-21
author: Alex Spataru
---

# Spec 0027 — Generic Command Palette

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

Serial Studio now has three separate "find something and go there" surfaces that have
drifted apart: the full-screen workspace switcher (Ctrl+K), the taskbar search box, and
the Start menu's searchable action list. Each keeps its own item list and its own
activation rules, which has already produced real user-facing bugs observed on the BADAQ
project (a large real project with many groups, widgets, and custom workspaces):

- Selecting a searched widget that belongs to no custom workspace silently activates a
  group-level pseudo-workspace that the user never created — confusing next to their
  curated workspace list.
- Pressing Enter to open the first search result stopped working; Escape did not close
  the switcher until recently for the same structural reason (keyboard events swallowed
  by an inner control).
- The switcher's Tools section lists only 4 tools while the Start menu offers 12
  (Sessions, File Transmission, Preferences, AI Assistant, Help Center, etc. are
  missing) — two hand-maintained tool lists that disagree.
- Search results show only a widget's name with no folder/group path, so identically
  named widgets (e.g. eight "Channel N" rows across six RTAM groups) are
  indistinguishable.
- The switcher dialog does not scale to small windows: labels clip and the fixed-size
  grid overflows.

Every future navigation surface (project editor, main window) would today mean a fourth
and fifth copy of this logic. The deciding constraint: **one shared, context-driven
palette model must serve every surface, so item lists and activation behavior cannot
drift again.**

## Goals

- One palette component, launchable in multiple contexts (dashboard/workspace switcher
  today; main window and project editor ready to adopt it), presenting
  context-appropriate browse categories and search sections.
- A single tools/actions list shared with the Start menu — one place to add a tool, and
  it appears in the Start menu, the palette, and taskbar search alike.
- Search results that a user of a large project can tell apart at a glance (path
  subtitles) and open confidently (no surprise pseudo-workspaces).
- Full keyboard operation: type, arrows, Enter, Escape — regardless of which child
  control has focus.
- A dialog that remains usable and un-clipped at small window sizes.
- Taskbar search backed by the same model as the palette, so both surfaces return the
  same items with the same behavior.

## Non-Goals

- No redesign of the Start menu itself; it remains the tools list's home.
- No change to workspace/folder persistence, project format, or workspace CRUD rules.
- No fuzzy-matching/scoring engine; matching stays simple substring, case-insensitive.
- No re-introduction of the removed folder context menu (New/Rename Folder).
- No new global shortcuts beyond those already wired (Ctrl+K et al.).

## Requirements

1. **R1 — Context-driven palette.** The palette is one reusable component configured by
   a launch context. The dashboard context reproduces today's workspace-switcher
   behavior (folder browse, breadcrumbs, search); a main-window context (tools/actions
   only, no workspace browse) ships in the same implementation as proof, and further
   contexts (project editor) can be launched with a different category/section set
   without duplicating the palette itself.
2. **R2 — Root browse categories.** At the root browse level (no query), the dashboard
   context shows a "Workspaces" category (folders, workspaces, Add Workspace cell) and a
   "Tools" category, each with an uppercase section header and separator, matching the
   Settings dialog's category-label style.
3. **R3 — Single tools model.** The palette's Tools items come from the same list the
   Start menu search uses, with identical visibility rules (build tier, runtime mode).
   Sessions, File Transmission, Preferences, AI Assistant, Help Center, Console,
   Notifications, Clock, Stopwatch, Auto Layout, Full Screen, and Add External Window
   appear in the palette exactly when they appear in Start menu search.
4. **R4 — Search sections.** With a query, results group into Folders, Workspaces,
   Groups, Widgets, and Tools sections (a section is hidden when empty). Groups and
   datasets both surface as widget-type entries when they have a widget — multiple
   entries from one group are expected and correct.
5. **R5 — Path subtitles.** Every search result renders a primary label (item name) and
   a small subtitle (reduced opacity) with its location: folder path for workspaces and
   folders, group name for group/dataset widgets. Both labels elide with a protective
   maximum width; no text overflows its cell at any window size.
6. **R6 — Widget activation without pseudo-workspaces.** Activating a widget result that
   is contained in a custom workspace switches to that workspace and reveals/highlights
   the widget. Activating a widget contained in no custom workspace creates a new empty
   external window (the same creation action as the Start menu's "Add External Window")
   showing **only that widget** — not the widget's whole group. The main dashboard never
   switches to a group-level pseudo-workspace from palette/search activation.
7. **R7 — Keyboard completeness.** While the palette is open: Escape closes it, Enter
   activates the highlighted item (or the first item when none is highlighted), and
   arrow keys move the highlight — all independent of which inner control has keyboard
   focus. The taskbar search popup honors the same Enter/Escape/arrow behavior.
8. **R8 — Responsive layout.** At small window sizes the palette shrinks gracefully:
   the dialog fits inside the window with margins, content reflows or scrolls, headers
   and hint text remain readable or collapse, and no control clips or overlaps.
9. **R9 — Taskbar search parity.** The taskbar search box produces the same result set
   (same sections, same items, same activation behavior including R6) as the palette's
   search for the same query.
10. **R10 — Browse-mode parity.** Folder navigation (drill-in, breadcrumb jumps, Back,
    slide animation) and the Add Workspace cell keep working as today inside the
    dashboard context.
11. **R11 — Presentation split.** Browse mode renders grid cells (as today); search
    results render as denser list rows (icon + primary name + path subtitle), which
    remain legible and elided at small window sizes.
12. **R12 — Main-window context ships now.** A tools/actions-only palette context is
    reachable from the main window (outside the dashboard) in this implementation, as
    the second consumer proving the component is generic.

## Acceptance Criteria

- [x] **AC1 (R1)** — Maintainer opens the palette on the dashboard: browse, folders,
      search all behave as before the refactor; a second context can be instantiated
      with a different section set (verified in code review by the absence of
      dashboard-specific assumptions in the shared component). *Code review: grep confirms
      `CommandPalette.qml` has no `Cpp_UI_Dashboard`/`taskBar`/workspace symbols — all data
      flows via the injected `model`; the main-window `PaletteModel` (browse off) is the
      second context. Runtime browse/folder/search behavior on maintainer list.*
- [x] **AC2 (R2)** — In BADAQ at root: "WORKSPACES" and "TOOLS" uppercase headers with
      separators are visible; Add Workspace cell present.
- [x] **AC3 (R3)** — On a commercial build (non-runtime mode), palette Tools shows the
      full Start-menu action set; on a GPL build, gated items (Sessions, File
      Transmission, AI Assistant, Notifications) are absent from both surfaces
      identically. *Code review: the tools item array (name/icon/visible/run) is defined
      once, in `ToolActions.qml`; StartMenu, palette, and taskbar search all read it via
      `ToolActions.items()`; tier/runtime gating stays in each item's `visible:` with no
      consumer-side filter beyond the query match. Two-build visibility parity on
      maintainer list.*
- [x] **AC4 (R4/R5)** — Searching "Channel" in BADAQ yields multiple identically named
      widget entries distinguishable by group-name subtitles; searching a group's name
      yields both its group widget entry and its dataset widget entries.
- [x] **AC5 (R6)** — Clicking a searched widget not in any custom workspace opens a new
      external window containing only that widget (not its sibling group widgets); the
      main dashboard's active workspace is unchanged and no group-level workspace
      appears in the switcher or taskbar.
- [x] **AC6 (R6)** — Clicking a searched widget that is in a custom workspace switches
      to that workspace and highlights the widget.
- [x] **AC7 (R7)** — With the palette open and after mouse-hovering results (focus
      anywhere), Escape closes, Enter opens first/highlighted item, arrows move the
      highlight. Same checks pass in the taskbar search popup.
- [x] **AC8 (R8)** — Maintainer resizes the main window down to ~800x500: palette stays
      inside the window, all labels elide instead of clipping, grid reflows, scrolling
      reaches every item.
- [x] **AC9 (R9)** — The same query typed in the taskbar search and in the palette
      returns the same items; activating the same widget entry from either surface has
      the same outcome.
- [x] **AC10 (R10)** — Folder drill-in/out, breadcrumb jump, and Add Workspace still
      work in the dashboard context after the refactor.
- [x] **AC11 (R1/R11)** — The main-window context opens outside the dashboard, lists
      tools/actions only (no workspace browse), and activates them with the same
      keyboard behavior as the dashboard context.

## Constraints & Invariants

- No hotpath involvement: the palette rebuilds its model on open/keystroke only; no
  per-frame work, no connections to frame/data signals.
- Existing keyboard shortcuts (Ctrl+K toggle, workspace cycling) keep working.
- Pro/GPL and runtime-mode gating of tools must be inherited from the single tools
  model, never re-encoded in the palette.
- Workspace IDs >= 1000 are user workspaces; the palette must not create, rename, or
  delete workspaces beyond the existing Add Workspace prompt flow.
- External-window creation must reuse the existing Start-menu action path (including
  persistence/reconciliation of external windows in the project file); no parallel
  window-spawning mechanism.
- QML-only UI changes plus the minimal C++ needed for workspace-membership lookup and
  external-window seeding; no new dependencies.
- All user-visible strings translatable; RTL layouts must not regress.
- Style contract: `scripts/code-verify.py` clean on all touched files.

## Open Questions

All resolved with the maintainer (2026-07-21):

- **Q1 — Search result presentation.** RESOLVED → grid cells for browse, denser list
  rows for search results (now R11).
- **Q2 — Seeded external window contents.** RESOLVED → the external window shows only
  the seeded widget, not its group (now in R6). Plan-phase finding: the app already has
  a per-widget floating window mechanism (the same one dashboard tools and widget
  pop-outs use), with per-project persistence — R6 reuses it directly, so the
  "empty dashboard window + seed" phrasing in early discussion is superseded and no
  new window mode is needed.
- **Q3 — Second context.** RESOLVED → ship a main-window tools/actions context in the
  same implementation (now R12).
