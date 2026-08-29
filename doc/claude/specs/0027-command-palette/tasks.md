---
spec: 0027-command-palette
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-07-21
---

# Tasks 0027 — Generic Command Palette

> **Phase 3 of 4 — the ordered checklist.** Decompose [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable*. `/ss-implement` works this list top to
> bottom and keeps the status boxes current. (Gate batched by maintainer's one-shot
> instruction, 2026-07-21.)

## Conventions

- One task = one focused, reviewable change.
- **Verify** = `python3 scripts/code-verify.py --check <files>` plus the stated read-back.
- **Deps** lists task IDs that must land first.

## Tasks

### T1 — C++ workspace-membership lookup

- **Files:** `app/src/UI/Taskbar.h`, `app/src/UI/Taskbar.cpp`
- **Does:** Adds `Q_INVOKABLE [[nodiscard]] int workspaceContainingWidget(int windowId)
  const`: prefers the active workspace, else first match in workspace order, else -1.
  **Binding invariants:** factor the per-ref "workspace ref → windowId" resolution out of
  `populateTaskbarFromWorkspace` into one shared private helper (no parallel copy); user
  workspaces are `id >= WorkspaceIds::AutoStart` (1000); GUI-thread only, no signals added.
- **Verify:** code-verify clean; read-back confirms `populateTaskbarFromWorkspace` and the
  new invokable call the same helper.
- **Deps:** none
- [x] done

### T2 — Extract ToolActions.qml (single tools model)

- **Files:** `app/qml/MainWindow/Panes/Dashboard/ToolActions.qml` (new),
  `app/qml/MainWindow/Panes/Dashboard/StartMenu.qml`, `app/CMakeLists.txt`
- **Does:** Moves the `searchableItems` item array verbatim into a non-visual
  `ToolActions` component with optional refs (`taskBar`, host callbacks — items whose
  refs are missing stay hidden via their `visible:` guards); `StartMenu.searchableItems`
  delegates to an instance; registers the new file in the QML list (~line 532 block).
  **Binding invariant:** tier/runtime gating lives only in the item `visible:` rules —
  no consumer-side filters beyond query match (R3).
- **Verify:** code-verify clean; grep shows exactly one definition of the tools array;
  Start-menu search behavior unchanged by read-back.
- **Deps:** none
- [x] done

### T3 — PaletteModel.qml (sections + activation policy)

- **Files:** `app/qml/MainWindow/Panes/Dashboard/PaletteModel.qml` (new),
  `app/CMakeLists.txt`
- **Does:** Non-visual controller with `taskBar`, `toolActions`, browse capability flag;
  `browseSections()` (root: "Workspaces" category incl. Add Workspace cell + "Tools"
  category; sub-levels headerless), `searchSections(query)` (Folders / Workspaces /
  Groups / Widgets / Tools; title + subtitle split: workspace/folder subtitle = parent
  path, widget/group subtitle = `groupName`), and `activate(node)` implementing R6:
  member widget → `selectWorkspaceById` + `navigateToWidget(id, gid, false)`; orphan
  widget → host `openWidgetWindow(windowId)`; never `setActiveGroupId(groupId)`.
  Flatten helpers move here from the overlay. **Binding invariants:** pull-only model
  (no frame/data signal subscriptions — spec hotpath constraint); empty workspace tree
  (QuickPlot/ConsoleOnly) degrades to Tools-only without errors.
- **Verify:** code-verify clean; read-back against R2/R4/R5/R6 section shapes.
- **Deps:** T1, T2
- [x] done

### T4 — CommandPalette.qml + SearchField.accepted

- **Files:** `app/qml/Widgets/CommandPalette.qml` (new),
  `app/qml/Widgets/SearchField.qml`, `app/CMakeLists.txt`
- **Does:** Generic palette UI from the overlay's chrome: header (icon/title/search/
  close), breadcrumb bar, grid cells for browse, **list rows for search** (icon +
  elided title + elided 0.7-opacity subtitle, shared single delegate), uppercase
  section headers, slide animation, focus-independent Esc `Shortcut`, Enter via a new
  additive `accepted` signal on `SearchField` (forwarded from the inner field), arrow
  nav (±1 list mode, ±columns grid mode). Responsive: panel `min(680, w-32)` /
  `min(480, h-32)`, search-width floor `Math.max(120, ...)`, title/hint hidden below
  width thresholds, breadcrumbs elide, Flow reflows to 1 column. **Binding
  invariants:** no dashboard/workspace assumptions in this file (all data via the
  model, R1); every label elided with a protective max width (R5/R8);
  `SearchField` change is additive only (11 existing consumers unaffected).
- **Verify:** code-verify clean; grep confirms no `Cpp_UI_Dashboard`/workspace symbol
  references inside `CommandPalette.qml`.
- **Deps:** T3
- [x] done

### T5 — Dashboard context wiring + overlay removal

- **Files:** `app/qml/MainWindow/Panes/Dashboard/DashboardLayout.qml`,
  `app/qml/MainWindow/Panes/Dashboard/WorkspaceSwitcherOverlay.qml` (delete),
  `app/CMakeLists.txt`
- **Does:** Instantiates the dashboard-context `PaletteModel` + `CommandPalette` in
  `DashboardLayout` (keeps `openWorkspaceSwitcher()` entry point and Ctrl+K path;
  orphan activation host hook = existing `openWidgetWindow`); deletes the overlay file
  and its CMake entry. **Binding invariants:** do NOT touch
  `WorkspaceSwitcherOverlay.qml.autosave` (maintainer's file); grep sweep must show
  zero remaining `WorkspaceSwitcherOverlay` references.
- **Verify:** code-verify clean; `grep -r WorkspaceSwitcherOverlay app/` returns only
  the `.autosave` file.
- **Deps:** T4
- [x] done

### T6 — Taskbar search popup on the shared model

- **Files:** `app/qml/MainWindow/Panes/Dashboard/Taskbar.qml`
- **Does:** Search popup renders `PaletteModel.searchSections()` (sectioned list rows
  with subtitles, same delegate as T4); activation via `model.activate(node)`; removes
  `combinedSearchResults`/`triggerSearchEntry` builder; folder rows open the palette
  drilled into that folder. **Binding invariants:** `isBusy` composition stays correct
  (popup/search focus still gate taskbar autohide); `dismissSearch()`/
  `searchDismissed` flow preserved; existing field-level key handlers kept plus the
  `accepted` path (R7/R9).
- **Verify:** code-verify clean; read-back: no result-list building left in
  `Taskbar.qml`.
- **Deps:** T5
- [x] done

### T7 — Main-window tools context (R12)

- **Files:** `app/qml/MainWindow/MainWindow.qml`
- **Does:** Ctrl+K routes contextually: dashboard available → existing
  `dashboard.openWorkspaceSwitcher()`; otherwise toggles a main-window `CommandPalette`
  fed by a `PaletteModel` with `taskBar: null` + a `ToolActions` limited to global
  refs (browse off, Tools only). **Binding invariant:** no new shortcut sequences
  (spec non-goal); reuse the binding at `MainWindow.qml:508`.
- **Verify:** code-verify clean; read-back of the Ctrl+K handler branch.
- **Deps:** T4
- [x] done

### T8 — Final sweep & handoff

- **Files:** all touched files (read-only pass), `doc/claude/specs/0027-command-palette/*`
- **Does:** Full `code-verify --check` across the diff; `qt-cpp-review` on
  `Taskbar.h/.cpp`; grep sweeps from plan Risks (single tools definition, no overlay
  refs, palette free of dashboard symbols); counterfactual check named in chat
  (most-at-risk rule + evidence); checks off spec ACs that are code-verifiable and
  lists the maintainer-observation ACs (AC2, AC4-AC11) for runtime confirmation;
  updates tasks/spec status.
- **Verify:** all boxes below checkable except maintainer-runtime items, which are
  explicitly handed off.
- **Deps:** T1-T7
- [x] done

## Definition of Done

- [x] Every acceptance criterion in `spec.md` is met and checked off there (runtime ACs
      explicitly handed to the maintainer).
- [x] `python3 scripts/code-verify.py --check` is clean on all changed files (no new
      errors). *(Re-verified 2026-08-14 against current HEAD: 0 errors across
      PaletteModel.qml, CommandPalette.qml, SearchField.qml, DashboardLayout.qml,
      Taskbar.qml, MainWindow.qml, Taskbar.h/.cpp.)*
- [x] `qt-cpp-review` run on the C++ diff; findings addressed or noted.
- [x] Hotpath untouched (plan states none) — no benchmark run required; confirm no
      frame-signal subscriptions crept in. *(Re-verified 2026-08-14: grep for
      frame/dashboardTick signals in PaletteModel.qml/CommandPalette.qml — none.)*
- [x] No pytest targets apply (QML UI); maintainer observation list delivered.
      *(Re-verified 2026-08-14: no `tests/` files reference the palette; spec.md's
      unchecked AC2/AC4-AC11 stand as the maintainer observation list.)*
- [x] `python3 scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [x] Diff is *what was asked, and only that* — no scope creep; foreign files
      (`WorkspaceSwitcherOverlay.qml.autosave`) untouched.
- [x] `spec.md` status set to `done`. *(Re-verified 2026-08-14.)*
