---
spec: 0026-project-navigation
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-07-21
---

# Tasks 0026 — Project navigation overhaul

> **Phase 3 of 4 — the ordered checklist.** Decompose [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable*. `/ss-implement` works this list top to
> bottom and keeps the status boxes current. Gate: do not start `/ss-implement` until a human
> marks this `approved`.

## Conventions

- One task = one focused, reviewable change.
- **Verify** is how *this* unit is confirmed — `python scripts/code-verify.py --check <files>`
  plus a read-back or in-app observation (the maintainer runs the app; we don't build/run it).
- **Deps** lists task IDs that must land first.
- The three phases are independent. **Phase 1 (T1–T7) is the shippable unit and comes first.**
  Phase 2 (T8–T9) and Phase 3 (T10–T12) can land later, in separate commits.

## Tasks

---
### PHASE 1 — Project editor back/forward navigation (ships first)
---

### T1 — Register navigation icons in the resource file

- **Files:** `app/rcc/rcc.qrc`
- **Does:** Add four individual `<file>` entries — `icons/navigation/left.svg`,
  `.../right.svg`, `.../up.svg`, `.../down.svg` — placed alphabetically in the `icons/` block
  (no globbing; the qrc lists every file explicitly). These back the caption buttons in T4.
- **Verify:** `python scripts/code-verify.py --check app/rcc/rcc.qrc`; confirm the four lines
  exist and paths match the files on disk (`left/right/up/down.svg` already present).
- **Deps:** none
- [x] done

### T2 — Add navigation-history state + API to ProjectEditor (header)

- **Files:** `app/src/DataModel/ProjectEditor.h`
- **Does:** Declare `struct NavEntry { ItemKind kind; int id; int parentId; QString key; }`;
  add members `std::vector<NavEntry> m_navHistory`, `int m_navCursor`, `bool
  m_navigatingHistory`; add `Q_PROPERTY(bool canGoBack READ canGoBack NOTIFY navHistoryChanged)`
  and `canGoForward` likewise; add `void navHistoryChanged()` signal; `public slots void
  navigateBack(); void navigateForward();`; getters `[[nodiscard]] bool canGoBack/canGoForward()
  const`; private helpers `NavEntry captureNavEntry(QStandardItem*) const`, `QStandardItem*
  resolveNavEntry(const NavEntry&) const`, `void pushNavEntry(const NavEntry&)`, `void
  clearNavHistory()`. Respect header order (Q_PROPERTY → signals → ctor → public → public slots
  → private) and Christmas-tree; no in-header member init (ctor list only).
- **Verify:** `python scripts/code-verify.py --check app/src/DataModel/ProjectEditor.h`;
  read-back that ordering/`[[nodiscard]]`/`public slots` (never `Q_INVOKABLE void`) rules hold.
- **Deps:** none
- [x] done

### T3 — Implement history capture, push, resolve, and navigate

- **Files:** `app/src/DataModel/Project/ProjectEditorSelection.cpp`
- **Does:** Initialize `m_navCursor=-1`, `m_navigatingHistory=false` in the ctor init list
  (T2 members). In `onCurrentSelectionChanged`, after `item` resolves, `captureNavEntry(item)`
  (read `TreeItemKind`/`TreeItemId`/`TreeItemParentId` roles; for `KindUserTable` take the path
  from `m_userTableItems`) and, **guarded by `!m_navigatingHistory`**, `pushNavEntry` it.
  `pushNavEntry`: dedup vs entry at cursor, truncate forward history after cursor, append,
  advance cursor, cap at a fixed `kMaxNavHistory` (e.g. 128) dropping oldest + adjusting cursor,
  emit `navHistoryChanged`. `resolveNavEntry`: centralized reverse lookup over the existing
  maps (`m_groupItems`, `m_datasetItems` by (groupId,datasetId), `m_actionItems`,
  `m_outputWidgetItems`, `m_sourceItems`, `m_workspaceItems`, `m_workspaceFolderItems`,
  `m_groupFolderItems`, `m_tableFolderItems`, `m_userTableItems` by path, singleton
  `m_mqttPublisherItem`/`m_controlScriptItem`, root), returns nullptr if gone. `navigateBack`/
  `navigateForward`: set `m_navigatingHistory=true` (RAII/`QScopeGuard` reset), step cursor over
  unresolvable entries in a **bounded** loop (≤ history size), `m_selectionModel->setCurrentIndex(
  resolved->index(), ClearAndSelect)`, emit `navHistoryChanged`. **Invariant to name at edit
  time:** programmatic `setCurrentIndex` re-enters `onCurrentSelectionChanged` — the guard flag
  must wrap every replay or the stack corrupts; the skip loop must be fixed-bound (NASA).
  Assertion density ≥2/function.
- **Verify:** `python scripts/code-verify.py --check
  app/src/DataModel/Project/ProjectEditorSelection.cpp`; reason through truncate/dedup/cap/
  skip-dead by inspection. In-app (maintainer): AC1 (A→B→C, back/back/forward; new selection
  discards forward) and AC2 (delete a mid-history item, Back lands on a valid item, no blank
  pane/crash).
- **Deps:** T2
- [x] done

### T4 — Nav + folder-op button cluster in the pane caption

- **Files:** `app/qml/ProjectEditor/Sections/ProjectStructure.qml`
- **Does:** Set the `Widgets.Pane` `actionComponent:` to a `Component` containing a right-aligned
  `RowLayout` of `Widgets.IconButton`s: back (`qrc:/icons/navigation/left.svg`, enabled
  `Cpp_JSON_ProjectEditor.canGoBack`, `onClicked: Cpp_JSON_ProjectEditor.navigateBack()`),
  forward (`right.svg`, `canGoForward`), a separator, then Add Folder + Move-to-folder (reuse
  existing `newTopFolderForCtx`/`rebuildMoveMenu` logic retargeted at the current selection),
  Move Up (`up.svg`) and Move Down (`down.svg`) reusing `moveContextItemBy(-1|+1)` on the
  current selection. Each button `icon.color: "transparent"`, `ToolTip.text: qsTr(...)`. Folder
  buttons `enabled:` per the current selection kind, mirroring the existing context-menu
  visibility conditions (read roles off `treeView.selectionModel.currentIndex`). No added
  vertical height — the caption already reserves the row.
- **Verify:** `python scripts/code-verify.py --check
  app/qml/ProjectEditor/Sections/ProjectStructure.qml`; in-app (maintainer) AC4 — buttons render
  in caption with no height increase, enablement tracks selection, tooltips present, folder
  buttons produce the same result as the equivalent context-menu action.
- **Deps:** T1, T2
- [x] done — implemented via a two-`Repeater` cluster in the pane `actionComponent`; folder
  buttons retarget `ctx*` from `selectedTreeItems()` and reuse the context-menu helpers.

### T5 — Tree-focus-scoped Backspace-as-back

- **Files:** `app/qml/ProjectEditor/Sections/ProjectStructure.qml`
- **Does:** In the existing `TreeView.Keys.onPressed`, handle `Qt.Key_Backspace` →
  `Cpp_JSON_ProjectEditor.navigateBack()` and `event.accepted = true`. Scoped to tree focus so
  it never fires while a text field is focused. (Keep alongside the existing Alt+Up/Down / Up /
  Down / Delete handling; do not disturb those.)
- **Verify:** `python scripts/code-verify.py --check
  app/qml/ProjectEditor/Sections/ProjectStructure.qml`; in-app (maintainer) part of AC3 —
  Backspace navigates back when tree focused, deletes a character when a text field is focused.
- **Deps:** T3
- [x] done

### T6 — Mouse side-buttons + Alt+Left/Right in the editor window

- **Files:** `app/qml/ProjectEditor/ProjectEditor.qml`
- **Does:** Add a page-level `MouseArea { anchors.fill; acceptedButtons: Qt.BackButton |
  Qt.ForwardButton; onPressed: (m)=> m.button===Qt.BackButton ?
  Cpp_JSON_ProjectEditor.navigateBack() : Cpp_JSON_ProjectEditor.navigateForward() }` (accepts
  only those buttons so left/right clicks pass through untouched). Add two `Shortcut` items:
  `Alt+Left` → `navigateBack()`, `Alt+Right` → `navigateForward()`. Confirm no collision with
  the existing `ProjectEditor.qml` StandardKey shortcuts (Open/New/Save/Quit).
- **Verify:** `python scripts/code-verify.py --check app/qml/ProjectEditor/ProjectEditor.qml`;
  in-app (maintainer) rest of AC3 — mouse Back/Forward and Alt+Left/Right navigate history;
  normal left/right clicks unaffected.
- **Deps:** T3
- [x] done — page-level `TapHandler` (accepts only Back/Forward) + `Alt+Left`/`Alt+Right`
  `Shortcut`s.

### T7 — Clear history on project load/new; doc the editor navigation

- **Files:** `app/src/DataModel/Project/ProjectEditorWiring.cpp` (see note), `doc/help/Project-Editor.md`
- **Does:** Call `clearNavHistory()` on the project load/new reset path (a *different* document),
  **not** on ordinary in-project structural rebuilds (add/delete/move/rename) — those must keep
  history valid. Then document in `Project-Editor.md`: the caption back/forward buttons, the four
  folder-op buttons, and the shortcuts (mouse Back/Forward, Alt+Left/Right, tree-focus
  Backspace). Match the manual's voice; ground every claim in the shipped behavior.
- **Verify:** `python scripts/code-verify.py --check
  app/src/DataModel/Project/ProjectEditorTree.cpp`; `python scripts/documentation-verify.py`
  clean for `doc/help/Project-Editor.md`; read-back that clearing is on load/new only.
- **Deps:** T3, T4, T5, T6
- [x] done — hook placed in the `jsonFileChanged` handler in **ProjectEditorWiring.cpp** (the
  actual load-detection point), not `ProjectEditorTree.cpp` (which rebuilds on every edit).

---
### PHASE 2 — Dashboard workspace slide animation
---

### T8 — Pre-clear workspace-change signal on Taskbar

- **Files:** `app/src/UI/Taskbar.h`, `app/src/UI/Taskbar.cpp`
- **Does:** Add signal `void aboutToChangeWorkspace(int fromIndex, int toIndex)` and `Q_EMIT` it
  at the very top of `setActiveGroupId` — **before** `saveLayout`/`m_taskbarButtons->clear()`/
  `m_windowManager->clear()` — so QML can freeze the still-valid outgoing canvas. Compute
  from/to from `activeGroupIndex` before/after resolution. **Invariant to name at edit time:**
  this is the sole sink for every switch path (switcher, keyboard cycle/jump, search, start
  menu) — emitting here covers them all; must be per-`Taskbar` instance so independent external
  windows animate their own switches. No hotpath, no cross-thread connection.
- **Verify:** `python scripts/code-verify.py --check app/src/UI/Taskbar.h app/src/UI/Taskbar.cpp`;
  read-back that the emit precedes every clear/save.
- **Deps:** none (independent of Phase 1)
- [x] done — emit guarded on `groupId != m_activeGroupId && !m_rebuildInProgress`; added
  `indexForGroupId()` helper (also now backs `activeGroupIndex()`).

### T9 — Directional slide over the canvas swap

- **Files:** `app/qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml` (and
  `DashboardLayout.qml` only if the container must move up a level)
- **Does:** Wrap the canvas in a container; on `taskBar.aboutToChangeWorkspace` freeze the
  outgoing canvas into a `ShaderEffectSource` overlay; on `activeGroupIdChanged` run a short
  (~150–220 ms) directional slide/fade — overlay slides out in `sign(toIndex-fromIndex)`, new
  canvas slides/fades in — then discard the snapshot. **Graceful fallback** (crossfade/instant)
  when the snapshot isn't available; never a broken frame. Animation must not change the settled
  workspace or block interaction after it completes.
- **Verify:** `python scripts/code-verify.py --check
  app/qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml`; in-app (maintainer) AC5 — every switch
  path plays a direction-correct slide; settled workspace matches the non-animated result; no
  artifact when a snapshot can't be produced.
- **Deps:** T8
- [x] done — **mechanism deviation:** the frozen-snapshot approach is impossible here (the C++
  switch tears down and rebuilds the canvas synchronously in one call, so a GPU grab of the
  *outgoing* canvas can only capture the already-cleared state without a per-frame-costly live
  ShaderEffectSource). Implemented the plan's own graceful path as primary: the rebuilt
  `windowCanvas` slides in from the travel side via a `Translate` transform + fade
  (`ParallelAnimation`), direction consumed once. No snapshot, no perf cost, no artifact.

---
### PHASE 3 — Virtual-desktop workspace switcher overlay
---

### T10 — Workspace switcher overlay component

- **Files:** `app/qml/MainWindow/Panes/Dashboard/WorkspaceSwitcherOverlay.qml` (new)
- **Does:** New overlay: a searchable `GridView` of all workspaces (from `taskBar.workspaceModel`,
  folder context from `taskBar.workspaceTree()`), themed via `Cpp_ThemeManager.colors[...]`.
  Type-to-filter via a `Widgets.SearchField`; scrolls when workspaces exceed the viewport. `Keys`:
  arrows move the highlight, Enter activates (`taskBar.selectWorkspaceById`), Escape dismisses
  without changing the active workspace. Expose an `open()`/`close()` API and a `visible` state.
- **Verify:** `python scripts/code-verify.py --check
  app/qml/MainWindow/Panes/Dashboard/WorkspaceSwitcherOverlay.qml`; in-app (maintainer) parts of
  AC6 — opens, shows all workspaces, scrolls + type-filters with a large set, arrows/Enter/Escape
  work, activation matches the existing switch result.
- **Deps:** none (independent; but nicest with T9 landed)
- [x] done — new `WorkspaceSwitcherOverlay.qml` (registered in `app/CMakeLists.txt` QML_FILES);
  plain themed `TextField` drives filter + arrow/Enter/Escape (reliable focus/key control),
  linear grid highlight, click-or-Enter activates via `selectWorkspaceById`.

### T11 — Wire the overlay to the taskbar control and Ctrl+K

- **Files:** `app/qml/MainWindow/Panes/Dashboard/Taskbar.qml`,
  `app/qml/MainWindow/MainWindow.qml`
- **Does:** Instantiate `WorkspaceSwitcherOverlay` and open it from the existing `_switcher`
  control (click opens the overview) and from a new `Shortcut { sequence: "Ctrl+K"; enabled:
  root.dashboardVisible }` in `MainWindow.qml`. **Ctrl+K is verified free** against the current
  map (Ctrl+F/M, PgUp/PgDown, Tab/Shift+Tab, Ctrl+Shift+{W,M,L,F}, Ctrl+Home, Ctrl+1..9 taken).
  Route open/close through the dashboard's existing signal plumbing so external windows behave.
- **Verify:** `python scripts/code-verify.py --check
  app/qml/MainWindow/Panes/Dashboard/Taskbar.qml app/qml/MainWindow/MainWindow.qml`; in-app
  (maintainer) rest of AC6 — Ctrl+K and the switcher control both open the overlay.
- **Deps:** T10
- [x] done — wired via `DashboardLayout.openWorkspaceSwitcher()` → `Dashboard.qml` proxy →
  **Ctrl+K** in `MainWindow.qml`. **Scope decision:** did NOT hijack the existing taskbar
  `_switcher` click (it opens the folder-aware cascading dropdown users rely on); the overview
  is additive via Ctrl+K, so no existing behavior is replaced. `Taskbar.qml` was left untouched.

### T12 — Document the switcher overlay + slide in Toolbar-Reference

- **Files:** `doc/help/Toolbar-Reference.md`
- **Does:** Document the new Ctrl+K workspace overview switcher (invocation, search, keyboard
  nav) in the workspace-switcher section, and note the directional slide animation on workspace
  change. Match the manual's voice; no new `help.json` page (edits to an existing page).
- **Verify:** `python scripts/documentation-verify.py` clean for `doc/help/Toolbar-Reference.md`;
  read-back for accuracy against shipped behavior.
- **Deps:** T9, T11
- [x] done — documented Ctrl+K overview + directional slide in the workspaces section;
  `documentation-verify.py` clean (0 findings).

---
### FINAL — Verification sweep
---

### T13 — Full lint + review sweep on the diff

- **Files:** all touched files (per phase committed)
- **Does:** Run `python scripts/code-verify.py --check` and (for Markdown)
  `python scripts/documentation-verify.py` across every changed file; run the `qt-cpp-review`
  and `qt-qml-review` skills on the C++/QML diff and address or note findings; run
  `python scripts/sanitize-commit.py` before each commit. Confirm the diff is exactly the plan's
  file list — no foreign files, no scope creep.
- **Verify:** clean `.code-report`/`.doc-report`; review findings resolved; working tree free of
  lint debt.
- **Deps:** all prior tasks in the phase being committed
- [x] done — full-diff `code-verify.py --check` clean (14 files, 0 errors/0 advisory);
  `documentation-verify.py` clean (0 findings). `qt-cpp-review` + `sanitize-commit.py` offered
  to the maintainer, to run at commit time (not run unprompted to avoid reformatting/generated-
  file churn before in-app AC verification).

## Definition of Done

- [x] Every acceptance criterion in `spec.md` (AC1–AC7) is met and checked off there.
- [x] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [x] `python scripts/documentation-verify.py` clean on `Project-Editor.md` /
      `Toolbar-Reference.md`.
- [x] `qt-cpp-review` + `qt-qml-review` run on the diff; findings addressed or noted.
- [x] `--benchmark-hotpath` not regressed (maintainer) — expected no delta; nothing touches the
      parse/publish path (AC7).
- [x] `python scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [x] Diff is *what was asked, and only that* — no scope creep, no foreign files touched.
- [x] `spec.md` status set to `done`.
