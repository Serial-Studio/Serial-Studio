---
spec: 0027-command-palette
phase: plan
status: approved     # draft -> approved (gate before /ss-tasks)
updated: 2026-07-21
---

# Plan 0027 — Generic Command Palette

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Gate: do not start `/ss-tasks` until a human marks this
> `approved`.

## Approach (one paragraph)

Split the current `WorkspaceSwitcherOverlay` into a non-visual controller
(`PaletteModel.qml`: section building, search, activation policy) and a generic UI shell
(`CommandPalette.qml`: dialog chrome, breadcrumbs, grid/list rendering, keyboard). A
context is just a `PaletteModel` configuration (which taskbar, which capabilities), so
the dashboard context reproduces today's switcher, a main-window tools-only context
ships as the second consumer, and the taskbar search popup renders the same model's
search sections — parity by construction. The Start menu's inline tools array moves
into a shared `ToolActions.qml` component (single tools model). Orphan-widget
activation reuses the **existing single-widget floating window** mechanism
(`DashboardCanvas.openExternalWidgetWindow`, already persisted per-project) instead of
inventing an empty-dashboard-window seeding flow; the only new C++ is
`Taskbar::workspaceContainingWidget()` for the membership lookup.

## Affected subsystems & files

| File | Change |
|------|--------|
| `app/qml/Widgets/CommandPalette.qml` | NEW — generic palette UI: panel, header/search, breadcrumb bar, grid (browse) + list rows (search), keyboard handling, responsive rules. No dashboard assumptions; driven by a `PaletteModel`. |
| `app/qml/MainWindow/Panes/Dashboard/PaletteModel.qml` | NEW — non-visual controller (`QtObject`): properties `taskBar`, `toolActions`, `capabilities` (workspaces browse on/off); functions `browseSections()`, `searchSections(query)`, `activate(node)` implementing R6; folder flatten helpers moved from the overlay. |
| `app/qml/MainWindow/Panes/Dashboard/ToolActions.qml` | NEW — the single tools/actions list (moved verbatim from `StartMenu.searchableItems`), parameterized by optional refs (`taskBar`, host callbacks); items with missing refs are filtered out by their existing `visible:` guards. |
| `app/qml/MainWindow/Panes/Dashboard/StartMenu.qml` | `searchableItems()` delegates to a `ToolActions` instance (list defined once, R3). |
| `app/qml/MainWindow/Panes/Dashboard/WorkspaceSwitcherOverlay.qml` | DELETED — replaced by `CommandPalette` + dashboard-context `PaletteModel` instantiated in `DashboardLayout`. |
| `app/qml/MainWindow/Panes/Dashboard/DashboardLayout.qml` | Instantiate `PaletteModel` (dashboard context) + `CommandPalette`; pass the model to `Taskbar`; keep `openWorkspaceSwitcher()` entry point; `openWidgetWindow()` already exists (orphan path target). |
| `app/qml/MainWindow/Panes/Dashboard/Taskbar.qml` | Search popup consumes `PaletteModel.searchSections()` (sectioned list rows with subtitles); `triggerSearchEntry` replaced by `model.activate(node)`; remove the local combined-results builder. |
| `app/qml/MainWindow/MainWindow.qml` | Ctrl+K routes to the dashboard palette when the dashboard is available, else opens a main-window tools-only palette (`PaletteModel` with `taskBar: null` + `CommandPalette`) — R12 without a new shortcut. |
| `app/qml/Widgets/SearchField.qml` | Additive `accepted` signal (forwarded from the inner field) so Enter works focus-independently (R7). |
| `app/src/UI/Taskbar.h` / `.cpp` | NEW `Q_INVOKABLE [[nodiscard]] int workspaceContainingWidget(int windowId) const` — resolves workspace refs to windowIds via the same resolution `populateTaskbarFromWorkspace` uses; prefers the active workspace, else first match in workspace order, else -1. (`navigateToWidget(...allowAddToWorkspace)` from the pre-spec session is kept.) |
| `app/CMakeLists.txt` | Register the three new QML files (QML file list, ~line 532 block); drop the deleted overlay entry. |

Grep-confirmed touch-points: overlay is instantiated only in `DashboardLayout.qml`
(`_switcherOverlay`); Ctrl+K binding at `MainWindow.qml:508`; taskbar search consumers
are internal to `Taskbar.qml`; `openExternalWidgetWindow` exists at
`DashboardCanvas.qml:76` with per-widget dedupe + persistence; external windows reach it
through `DashboardLayout.openWidgetWindow` → `widgetWindowRequested` →
`_mainLayout.openWidgetWindow` (`Dashboard.qml:439`).

## Architecture & data flow

- **Model build (pull, on demand):** `PaletteModel.browseSections()` reads
  `taskBar.workspaceTree()`; `searchSections(q)` reads `workspaceTree()` +
  `taskBar.searchResults` (C++ getter, filter set via `taskBar.searchFilter`) +
  `toolActions.items(q)`. Rebuilt on open and per keystroke only.
- **Search entry shape:** widget/group entries carry `windowId`, `groupId`,
  `groupName` (subtitle), `isGroupWidget`; dataset widgets are child entries of their
  group in `Taskbar::searchResults()` — multiple rows per group is native behavior.
  Workspaces/folders carry `fullPath` built by the flatten helpers (subtitle = parent
  path, title = own name — split into two labels, no longer concatenated into one).
- **Activation policy (`PaletteModel.activate`, R6):**
  - tool → `run()`; folder → palette drill-in (from the taskbar popup: open the
    palette at that folder); workspace → `taskBar.selectWorkspaceById(id)`.
  - widget → `const ws = taskBar.workspaceContainingWidget(windowId)`;
    `ws >= 1000` → `selectWorkspaceById(ws)` then `navigateToWidget(windowId, groupId,
    false)` (reveal + highlight; the add-branch is disabled and the group fallback is
    unreachable because membership was pre-checked); else →
    palette host's `openWidgetWindow(windowId)` (single-widget floating window; the
    dashboard's active workspace is untouched).
- **Contexts:** dashboard = `{taskBar, toolActions(with taskBar refs), workspaces
  browse on}`; main window = `{taskBar: null, toolActions(global refs only), browse
  off}` — the palette shows only the Tools category and search matches tools only.
- **Threads:** everything is main-thread QML + main-thread `UI::Taskbar`. The
  `buildTreeModel`/`groupsChanged` queued-connection convention is unaffected — the
  palette *pulls* `workspaceTree()` when opened; it never subscribes to model-rebuild
  signals.

## Hotpath & threading impact

- **Touches the hotpath?** No. No connection to frame/data signals; model rebuilds
  happen on open and per keystroke (spec constraint). No `Dashboard` ingest/draw code
  is touched.
- **New cross-thread signal/slot?** No. All new code is GUI-thread.
- **New input to a cached hotpath flag?** No.
- **Timestamp ownership** — untouched; no export/report involvement.

## Data model & persistence

None. No project-JSON, `widgetSettings`, or `Keys::` changes. Orphan-widget windows
reuse the existing `externalWindows` widgetSettings entry maintained by
`saveExternalWindowStates()` (already excludes tools); workspace CRUD paths unchanged.

## API / SDK surface

None. `workspaceContainingWidget` is a QML-facing invokable on `UI::Taskbar`, not an
API handler.

## QML / UI

- **Browse (grid, R2/R10/R11):** grid cells as today; root shows two categories with
  uppercase headers + separator rule — "Workspaces" (folders, workspaces, Add
  Workspace cell) and "Tools" (from `ToolActions`). Sub-folder levels keep the
  headerless grid. Breadcrumbs/Back/slide preserved (moved, not rewritten).
- **Search (list rows, R5/R11):** one column of rows — icon (16-20 px), primary label
  (elided, `Layout.fillWidth`), subtitle (elided, opacity ~0.7,
  `customUiFont(0.8)`), grouped under the same uppercase section headers. Keyboard
  highlight = row index in the flat list (existing `displayNodes`/`currentIndex`
  scheme; `move()` steps ±1 in list mode, ±columns only in grid mode).
- **Responsive (R8):** panel `width: min(680, root.width - 32)`, `height:
  min(480, root.height - 32)`; header search width clamped with a floor
  (`Math.max(120, ...)`) and the header title hidden below a width threshold; hint
  text hidden below a threshold; breadcrumb labels elide; grid `Flow` reflows (min 1
  column); everything scrolls in the existing `Flickable`.
- **Taskbar popup:** same row delegate as palette search — one delegate definition
  shared, not two copies.
- **Keyboard (R7):** `CommandPalette` keeps the focus-independent `Shortcut` for Esc;
  Enter via the new `SearchField.accepted` signal; arrows via existing `Keys`
  handlers. The taskbar popup keeps its field-level handlers and gains the same
  accepted-signal path.
- **Theme/RTL:** colors via `Cpp_ThemeManager.colors`; all strings `qsTr`; reuse the
  overlay/taskbar LayoutMirroring conventions. No ComboBoxes introduced (no restore
  race).

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Where the model lives | Palette-owns-model / **shared PaletteModel component** / C++ model in Taskbar | **PaletteModel.qml** — R9 parity and R1 contexts fall out structurally; C++ can't own activation anyway (tools call QML `app.show*`). |
| Orphan widget window | Empty external dashboard window + seed / **existing single-widget `openExternalWidgetWindow`** | **Existing mechanism** — spec intent ("window shows only that widget") satisfied with zero new persistence; the empty-window+seed idea predates discovering this mechanism (spec Q2 note amended). |
| Tools list sharing | Copy list into palette / palette reads a StartMenu instance / **extract ToolActions.qml** | **Extract** — a StartMenu instance doesn't exist outside the dashboard (R12); one definition serves all three consumers. |
| Main-window entry (R12) | New shortcut / menu item / **Ctrl+K contextual fallback** | **Ctrl+K fallback** — honors the no-new-shortcuts non-goal; dashboard available → switcher context, else tools context. |
| Search presentation | Grid everywhere / **grid browse + list search** | Maintainer-decided (spec Q1 → R11). |

## Risks & mitigations

- **Workspace-ref resolution drift** (`workspaceContainingWidget` vs
  `populateTaskbarFromWorkspace`): both must resolve stored refs to windowIds
  identically. Mitigate by factoring the per-ref resolution into one private helper
  used by both — never a parallel copy.
- **Silent-breakage classes** (`common-mistakes.md`): mode boundary — the palette
  reads `workspaceTree()`, which already branches on `operationMode`; the palette must
  handle an empty tree (QuickPlot/ConsoleOnly) by showing Tools only, not erroring.
  ComboBox restore race — N/A. macOS file-dialog reentrancy — N/A.
- **External-window layouts:** each external window hosts its own `DashboardLayout`
  (own taskBar); palette + model stay per-layout as the overlay is today, and
  orphan-widget activation inside an external window routes through the existing
  `widgetWindowRequested` relay to the main canvas (already wired).
- **Enter/Escape regressions elsewhere:** `SearchField` gains an additive `accepted`
  signal only — the 11 existing consumers see no behavior change unless they connect.
- **Tools gating drift:** gating lives only in `ToolActions` item `visible:` rules
  (build tier, runtime mode, ref availability); the palette and Start menu must not
  add their own filters beyond the query match.
- **Deleted-file references:** grep for `WorkspaceSwitcherOverlay` after removal
  (QML, CMake, docs) — currently only `DashboardLayout.qml` + `app/CMakeLists.txt`.
  The stray `WorkspaceSwitcherOverlay.qml.autosave` in the tree is the maintainer's
  file — do not touch it.

## Test & verification plan

No pytest coverage exists for QML UI surfaces; verification is static + maintainer
observation, mapped per AC:

- **Static (implementer runs):** `python3 scripts/code-verify.py --check` on every
  touched file; `qt-cpp-review` on the `Taskbar.h/.cpp` change; grep sweeps: no
  remaining `WorkspaceSwitcherOverlay` references, `searchableItems` list defined
  once (AC3 by review), `CommandPalette.qml` free of dashboard/workspace assumptions
  outside the model (AC1 by review).
- **Maintainer observations (app):** AC2 (root headers in the field project), AC4 ("Channel"
  search shows group-name subtitles; a group-name search shows group + dataset rows),
  AC5 (orphan widget → single-widget window, active workspace unchanged), AC6
  (member widget → workspace switch + highlight), AC7 (Esc/Enter/arrows in palette +
  taskbar popup), AC8 (~800x500 resize), AC9 (same query both surfaces, same
  outcomes), AC10 (folder drill/breadcrumb/Add Workspace), AC11 (Ctrl+K outside the
  dashboard → tools palette).
- **Hotpath:** not touched; no `--benchmark-hotpath` run required
  (`workspaceContainingWidget` is on-demand GUI-thread).
- **Commit:** `python3 scripts/sanitize-commit.py` before commit (maintainer-gated).
