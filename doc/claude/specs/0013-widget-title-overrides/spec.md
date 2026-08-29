---
spec: 0013-widget-title-overrides
title: Per-widget title overrides and freeze-titlebar visibility
status: done          # closed 2026-08-20
created: 2026-07-16
author: Alex Spataru
---

# Spec 0013 — Per-widget title overrides and freeze-titlebar visibility

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

Every title a dashboard widget displays — the window caption, the freeze-mode panel
header, the painted title strip inside instrument widgets (Bar, Gauge, Meter) — reads the
canonical dataset or group title from the project, live. The only way to change what a
widget window is *called* is to rename the dataset or group itself, and that rename leaks
everywhere the canonical title is identity: CSV/MDF4 column headers, API responses,
transform-script lookups, the project editor tree. Users who want a display name for the
operator ("Chamber Pressure") that differs from the instrumentation name ("PT-01-RAW")
have no way to get one.

Freeze mode (spec 0007) has a related, hardcoded gap. When a dashboard freezes, the
window caption hides and a panel-style freeze title bar takes its place — except on Bar,
Gauge, and Meter, which opt out at the widget-type level because they paint their own
titles. The user cannot influence either behavior: a frozen gauge can never show the
panel header, a frozen plot can never hide it, and neither can display anything but the
canonical title. On dense instrument panels (the use case freeze mode exists for), a
per-widget choice of "titlebar or no titlebar, and what it says" is the difference
between a readable operator screen and a cluttered one.

## Goals

- A user can give any dashboard widget a display title that differs from the canonical
  dataset/group title, without touching the dataset or group itself.
- A user can show or hide the freeze-mode title bar per widget — including on Bar,
  Gauge, and Meter, where it is currently hardcoded off.
- Overrides and visibility choices persist in the project file and survive save, close,
  and reload, keyed by stable identity (dataset/group `uniqueId`), so they survive
  group/dataset reorders.
- Both are editable from the workspace editor in the Project Editor and from the widget
  window itself on the dashboard.

## Non-Goals

- **Not a rename of the underlying dataset or group.** The canonical title remains the
  identity used by exports (CSV, MDF4, Session DB), API responses, transform scripts,
  console prints, and the project editor. The override is presentation-only.
- **Not per-workspace.** One override map per project; a widget shown in three
  workspaces displays the same title everywhere (scoping is per widget kind, never
  per workspace).
- **Not a change to freeze-mode semantics.** What freeze locks (input, chrome, layout)
  is untouched; this only governs the freeze header's visibility and text.
- **Not a new tier gate.** No Pro/Trial gating on overrides or visibility toggles; the
  freeze header itself remains reachable only inside freeze mode, which is already
  Pro-gated.
- **Not styling.** No per-widget fonts, colors, or header heights — visibility and text
  only.

## Requirements

1. **R1 — Title override map, two scopes.** The project persists display-title
   overrides at two scopes: *widget-level* (a specific widget of a specific
   dataset/group — e.g. only the FFT of a dataset) and *entity-level* (every widget of
   a dataset/group at once). Resolution: widget-level wins, else entity-level, else
   canonical title (today's behavior). An empty-string override is equivalent to
   absent. *(Amended 2026-07-16: a dataset shown as Plot + FFT + Waterfall must
   support individual per-widget titles; the original single dataset-uid map applied
   one title to all of them.)*
2. **R2 — Display surfaces honor the override.** Wherever a widget displays its own
   name, the override (when present) replaces the canonical title: the widget window
   caption, the freeze-mode panel header, the painted title strip and in-face title of
   Bar/Gauge/Meter, taskbar entries, dashboard widget search, and external (popped-out)
   widget window titles.
3. **R3 — Non-display surfaces ignore the override.** CSV/MDF4/Session exports, API
   command responses, transform-script and data-table lookups, console output, and the
   project editor tree continue to show canonical titles, unaffected by any override.
4. **R4 — Per-widget freeze-title mode.** Each dashboard widget carries a persisted
   freeze-title mode with only explicit values: **title bar** (panel header shown; the
   default for every widget type except Bar/Gauge/Meter), **painted** (title on the
   instrument face; only offered for Bar/Gauge/Meter and their default), and **hidden**
   (no title anywhere while frozen). A frozen Gauge can thus show the panel header, and
   a frozen Plot can hide its title. There is no "auto" value anywhere — UI and API
   always present the resolved mode, and selecting a widget's own default simply removes
   its stored entry. *(Amended from a boolean on plan approval, and from a tri-state
   with "auto" during implementation, both 2026-07-16 — "auto" made the control
   unpredictable per widget type.)*
5. **R5 — Name shown at most once.** For Bar, Gauge, and Meter, the mode also governs
   the widget's own painted title while frozen: **hidden** produces a fully label-free
   instrument face, **title bar** suppresses the painted title, and in every mode the
   (possibly overridden) name appears at most once — never both the panel header and
   the painted strip for the same widget.
6. **R6 — Workspace editor editing.** The workspace editor view in the Project Editor
   exposes, per widget row: an editable display-title field (blank = use canonical,
   shown as placeholder) and a freeze-titlebar visibility toggle. Edits follow the
   existing workspace-editing lifecycle (staged modification, saved with the project,
   restored on load).
7. **R7 — On-widget editing.** Every dashboard widget window carries a visible menu
   button at the left edge of its caption (replacing the external-window button and the
   earlier hidden right-click menu). Its menu hosts rename, the freeze-title mode
   options, and "Open in External Window" (absorbing the removed button); top-level
   entries carry mono icons from `app/rcc/icons/buttons`, the checkable mode options
   stay icon-free. Edits persist to the project
   identically to R6. *(Amended during implementation, 2026-07-16: discoverable button
   instead of right-click; pop-out moved into the menu.)*
8. **R8 — Live update.** Changing an override or visibility flag updates every affected
   display surface immediately — no project reload, no dashboard rebuild visible to the
   user, and no interruption of streaming data.
9. **R9 — Identity survival.** Overrides keyed by `uniqueId` survive group/dataset
   reorders, moves between groups, and project save/load cycles. Deleting the dataset
   or group orphans its entry harmlessly (no crash, no misapplied title on a different
   item); re-import that assigns fresh `uniqueId`s is allowed to drop overrides.
10. **R10 — API parity.** The `project.*` API exposes read and write access to the
    override map and visibility flags, so the AI assistant and external tooling can do
    what the UI does; mutations follow the established safety conventions for
    project-mutating commands.

## Acceptance Criteria

- [x] **AC1** (R1, R9) — Integration test: set an override via the API, save, reload;
  the map round-trips and survives reordering groups/datasets. Deleting the target
  dataset leaves the project loadable and the entry inert.
- [x] **AC2** (R2) — In-app observation: with an override on one gauge dataset, the
  window caption, freeze header (when enabled), painted gauge title, taskbar entry,
  and popped-out window all show the override; a second widget on the same dataset
  shows it too.
- [x] **AC3** (R3) — Integration test: with overrides set, CSV export headers and
  `project.dataset.get`/live-data API titles still return canonical titles.
- [x] **AC4** (R4, R5) — In-app observation: defaults match today (plot shows freeze
  header, gauge doesn't); flipping each toggle inverts it, and no widget ever shows
  two titles at once in freeze mode.
- [x] **AC5** (R6) — In-app observation: workspace editor rows show the editable title
  and visibility toggle; edits mark the project modified, save, and restore.
- [x] **AC6** (R7, R8) — In-app observation: renaming from the widget window updates
  all surfaces immediately while a data stream is running, and persists after reload.
- [x] **AC7** (R10) — Integration test: the new/extended API commands read and mutate
  overrides and visibility flags; static test confirms safety-tier registration for
  any new mutating command.
- [x] **AC8** (hotpath) — `--benchmark-hotpath` gates unchanged: title resolution must
  not add per-frame work.

## Constraints & Invariants

- **No per-frame cost.** Title resolution happens at reconfigure/binding time, never on
  the frame path; the 256 kHz benchmark gates must not regress.
- **Canonical title stays canonical.** Nothing keyed by title (exports, script lookups,
  aliases, API identity) may observe the override — R3 is the contract that keeps
  existing consumer tooling working.
- **Stable identity only.** Overrides key on `uniqueId` (dataset or group), never on
  positional `groupId`/`datasetId` or on title strings.
- **Unfreeze-safe.** Visibility flags only manifest in freeze mode; normal-mode chrome
  (caption, toolbar policy) is unchanged, and unfreezing must always restore today's
  normal-mode appearance regardless of flag state.
- **Works in both dashboard layout modes** (auto-layout and manual/freeform) and for
  external widget windows.
- **Loader tolerance.** Older projects without the map load unchanged; a project saved
  with overrides opens in an older Serial Studio without crashing (unknown keys
  ignored).
- **Mode scope.** Overrides live in the project file, so they apply in ProjectFile mode;
  QuickPlot/DeviceDefined dashboards (regenerated frames) are out of scope unless an
  entry happens to match a synthesized `uniqueId`.

## Open Questions

- Should the on-widget rename affordance (R7) be caption double-click, a context-menu
  entry, or a toolbar button? (UX shape — decide in `/ss-plan` with a mockup; the
  requirement only fixes that some on-widget path exists.)
- ~~Hidden freeze header on non-instrument widgets~~ — **resolved 2026-07-16**: hidden
  means hidden; the widget shows no title anywhere in freeze mode.
- ~~Visibility toggle scope~~ — **resolved 2026-07-16**: universal; every widget row
  gets the same toggle, whether or not it paints its own title.
