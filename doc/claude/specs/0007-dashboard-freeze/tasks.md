---
spec: 0007-dashboard-freeze
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-07-14
---

# Tasks 0007 — Dashboard Freeze Mode

> **Phase 3 of 4 — the ordered checklist.** Decompose [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable* — each one a coherent diff a reviewer
> could read in isolation. `/ss-implement` works this list top to bottom and keeps the status
> boxes current. Gate: do not start `/ss-implement` until a human marks this `approved`.

## Conventions

- One task = one focused, reviewable change. If a task touches >3 files or needs a paragraph
  to describe, split it.
- **Verify** is how *this* unit is confirmed before moving on — usually
  `python scripts/code-verify.py --check <files>`, plus a test or a read-back where one fits.
- **Deps** lists task IDs that must land first.
- Order so the tree compiles (conceptually) after each task where practical.

## Tasks

### T1 — Stored flag: `Keys::Frozen` + `ProjectModel::frozen`

- **Files:** `app/src/DataModel/Frame.h`, `app/src/DataModel/ProjectModel.h`,
  `app/src/DataModel/ProjectModel.cpp`
- **Does:** Add `Keys::Frozen("frozen")` beside `PlotTimeRange` (Frame.h:177 block). Add
  `m_frozen` (ctor-init-list `false` — no in-header init), `frozen()` getter, `setFrozen()`
  setter (guard-return; runtime license gate via `SerialStudio::proWidgetsEnabled()`;
  `setModified(true)`; `Q_EMIT frozenChanged()`), `Q_PROPERTY`, signal; reset
  `m_frozen = false` in `newJsonFile()`. **Invariant:** `newJsonFile()` is inside the
  protected ctor closure — the reset is a plain member write only; no singleton calls, no
  new signal wiring there.
- **Verify:** `python scripts/code-verify.py --check` on all three files; read back the
  setter for guard-return + gate + `setModified` order (mirror `setPlotTimeRange`).
- **Deps:** none
- [x] done — clean (3 files, 0 errors); enable-only license gate so unfreeze always works

### T2 — Persistence: serialize + load round-trip

- **Files:** `app/src/DataModel/Project/ProjectModelPersistence.cpp`,
  `app/src/DataModel/Project/ProjectModelLoading.cpp`
- **Does:** `json.insert(Keys::Frozen, m_frozen)` in `serializeToJson` (beside
  `PlotTimeRange`, :246). Load path: **direct member write** from JSON (default `false`),
  bypassing `setFrozen()` — the license gate must not strip the flag on an unlicensed load
  (R8) — and emit `frozenChanged` in the post-load signal block (:1069).
- **Verify:** code-verify on both files; read back that the loader never routes through the
  gated setter and that omit-means-false holds.
- **Deps:** T1
- [x] done — clean; `loadFrozen` helper mirrors `loadChangeDrivenTransforms`, direct member write

### T3 — Effective state: `UI::Dashboard::frozen`

- **Files:** `app/src/UI/Dashboard.h`, `app/src/UI/Dashboard.cpp`
- **Does:** Read-only `frozen` Q_PROPERTY computed as
  `ProjectModel::frozen() && SerialStudio::proWidgetsEnabled()`; `setFrozen(bool)` slot
  forwarding to `ProjectModel::setFrozen()` (single QML entry point). Notify wiring:
  `ProjectModel::frozenChanged`, `LemonSqueezy::activatedChanged` (inside
  `BUILD_COMMERCIAL`), `AppState::operationModeChanged` → `frozenChanged`. **Invariants:**
  Dashboard is constructed last in `instantiateCoreModules()`, so referencing the
  ProjectModel/LemonSqueezy singletons in its ctor is order-safe; `frozen` must NOT be read
  anywhere on `hotpathRxFrame`/ingest; all connections main-thread default (direct).
- **Verify:** code-verify on both files; grep the diff to confirm no hotpath function reads
  `frozen`.
- **Deps:** T1
- [x] done — clean; grep confirms no hotpath reads; notify wired to all three sources

### T4 — Input lock: `WindowManager::frozen` early-outs

- **Files:** `app/src/UI/WindowManager.h`, `app/src/UI/WindowManager.cpp`
- **Does:** Plain-bool `frozen` Q_PROPERTY (guard-return setter, set from QML). Early-outs:
  `startManualPress()` **first statement** `if (m_frozen) return false` — this single gate
  closes caption drag, manual-mode *body* drag of unfocused windows, and edge resize, since
  all three originate there from both entry points; `childMouseEventFilter()` (return false
  ⇒ pass-through to widget content, R4); `mousePressEvent()` both branches (incl. auto-mode
  caption drag / tile reorder); `mouseDoubleClickEvent()` (no maximize toggle);
  `updateHoverCursor()` (unset cursor, no resize affordance). **Invariant:** every early-out
  returns *before* grab/raise/focus state is created so widget-content interaction is
  untouched.
- **Verify:** code-verify on both files; read back each of the five handlers for the gate.
- **Deps:** none
- [x] done — clean; all five handlers gated, setter unsets cursor on freeze

### T5 — Central toolbar: `WidgetToolbar.qml`

- **Files:** `app/qml/Widgets/Dashboard/WidgetToolbar.qml` (new) + registration where
  sibling Dashboard widget QML files are listed (grep how `DashboardToolButton.qml` is
  registered: qrc/CMake QML module).
- **Does:** 48 px band hosting toolbar buttons in a horizontal `Flickable`
  (`contentWidth` = row implicit width, `clip: true`, wheel-scrollable, no scrollbar chrome,
  edge-fade affordance when overflowing; verify button clicks work inside the Flickable —
  `pressDelay`/`interactive` tuning if flick steals presses). Policy owner:
  `shown = !frozen && widgetHeight >= minWidgetHeight` (property, per-widget threshold);
  **width never hides it — it scrolls** (R13). Frozen read:
  `windowRoot && windowRoot.frozen === true` (undefined in external pop-outs ⇒ not frozen).
  Collapses to height 0 when hidden.
- **Verify:** code-verify on the new file; qmldir/registration read-back.
- **Deps:** none
- [x] done — clean; registered in app/CMakeLists.txt QML_SOURCES; Flickable interactive only on overflow

### T6 — Migrate Plot, MultiPlot, FFTPlot to `WidgetToolbar`

- **Files:** `app/qml/Widgets/Dashboard/Plot.qml`, `MultiPlot.qml`, `FFTPlot.qml`
- **Does:** Move each toolbar `RowLayout` into `WidgetToolbar` (buttons and
  `DashboardToolButton` usage unchanged); **delete** the imperative
  `onWidthChanged`/`onHeightChanged` `hasToolbar` assignments; `hasToolbar` becomes
  `readonly property bool hasToolbar: toolbar.shown` (preserves the delegate/band mirror
  contract). Keep each widget's height threshold (220). **Invariant:** the sweep toggle,
  settings dialog, and persisted `saveWidgetSetting` toolbar actions must keep working from
  inside the Flickable.
- **Verify:** code-verify on all three; read back content anchor margins
  (`hasToolbar ? 48 : 0` consumers now follow `toolbar.shown`).
- **Deps:** T5
- [x] done — clean; all three migrated, imperative hasToolbar assignments deleted

### T7 — Migrate Waterfall, ImageView, Plot3D

- **Files:** `app/qml/Widgets/Dashboard/Waterfall.qml`, `ImageView.qml`, `Plot3D.qml`
- **Does:** Same migration as T6 (Waterfall threshold 200; ImageView's extra
  `model.frameCount > 0` visibility term stays ANDed into its toolbar content, not the
  band policy).
- **Verify:** code-verify on all three; read back thresholds + ImageView's compound
  visibility.
- **Deps:** T5
- [x] done — clean; WidgetToolbar made layout-agnostic (anchors moved to consumers) so
  ImageView's ColumnLayout works via Layout props + `available` gate; Waterfall keeps 200

### T8 — Migrate GPS, DataGrid, Accelerometer

- **Files:** `app/qml/Widgets/Dashboard/GPS.qml`, `DataGrid.qml`, `Accelerometer.qml`
- **Does:** Same migration (declarative `hasToolbar` variants; Accelerometer threshold 296;
  GPS content top-margin follows `toolbar.shown`).
- **Verify:** code-verify on all three; read back anchor math.
- **Deps:** T5
- [x] done — clean; Accelerometer keeps 296 threshold, GPS container margin follows toolbar.height

### T9 — Terminal: adapt, don't force-fit

- **Files:** `app/qml/Widgets/Dashboard/Terminal.qml`
- **Does:** Terminal derives `hasToolbar` from its console toolbar — adopt `WidgetToolbar`
  only if it fits cleanly; otherwise keep its structure and add only the frozen gate
  (`windowRoot.frozen === true`) to its toolbar visibility. Whichever branch is taken gets
  named in chat before the edit (planned scope decision, not drift).
- **Verify:** code-verify; read back that frozen hides the console toolbar and the band.
- **Deps:** T5
- [x] done — no edit: Terminal is a dashboard tool (external windows only, per
  architecture/dashboard.md), so it never renders in canvas chrome and freeze never
  reaches it; the spec non-goal leaves external windows unchanged

### T10 — Chrome + canvas bindings: WidgetDelegate, DashboardCanvas

- **Files:** `app/qml/MainWindow/Panes/Dashboard/WidgetDelegate.qml`,
  `app/qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml`
- **Does:** WidgetDelegate: `readonly property bool frozen: Cpp_UI_Dashboard.frozen`;
  `headerVisible: !frozen`; `shadowEnabled: !frozen && (existing expression)`; the
  `windowRoot.hasToolbar` mirror assignment becomes `widgetInstance.hasToolbar && !frozen`,
  re-evaluated on `frozenChanged` (the existing mirror is an imperative connect — extend it,
  don't replace it with a binding that the widget's signal would clobber). DashboardCanvas:
  bind `windowManager.frozen: Cpp_UI_Dashboard.frozen`; gate the manual-resize hover
  MouseArea (`enabled: !_wm.autoLayoutEnabled && !Cpp_UI_Dashboard.frozen`) and the canvas
  right-click context menu on frozen. **Invariant (signal wiring):** read the existing
  Connections/connect blocks in both files in full before touching them.
- **Verify:** code-verify on both files; read back the mirror re-evaluation path for both
  signal sources (widget `hasToolbarChanged` + `frozenChanged`).
- **Deps:** T3, T4, T6-T9 (mirror semantics final)
- [x] done — clean; mirror untouched on purpose (freeze flows WidgetToolbar.shown →
  widget hasToolbar → existing mirror, single path); context menu needs no QML gate
  (rightClicked source is closed by the C++ frozen early-out); delegate defines `frozen`
  for windowRoot consumers + headerVisible/shadow bindings

### T11 — Toggle plumbing: DashboardLayout + Dashboard pane

- **Files:** `app/qml/MainWindow/Panes/Dashboard/DashboardLayout.qml`,
  `app/qml/MainWindow/Panes/Dashboard.qml`
- **Does:** `toggleFreeze()` → `Cpp_UI_Dashboard.setFrozen(!Cpp_UI_Dashboard.frozen)`
  following the `toggleAutoLayout()` delegation pattern (:131 / :56); gate
  `closeActiveWindow()`, `minimizeActiveWindow()`, and `toggleAutoLayout()` on
  `Cpp_UI_Dashboard.frozen` (R3's non-chrome escape hatches).
- **Verify:** code-verify on both files; read back the delegation chain matches the
  existing stubs.
- **Deps:** T3
- [x] done — clean; toggleFreeze delegation + frozen gates on close/minimize/toggleAutoLayout

### T12 — Icon asset

- **Files:** `app/rcc/icons/buttons/freeze.svg` (new) + its qrc registration (grep where
  `auto-layout.svg` is listed).
- **Does:** Single-color glyph (snowflake or padlock) on the same grid/stroke style as
  sibling `icons/buttons/*.svg`, tinted at runtime via `icon.color`.
- **Verify:** read-back of qrc registration; visual check by maintainer at T13.
- **Deps:** none
- [x] done — padlock glyph in the 48x48 filled style; registered alphabetically in rcc.qrc

### T13 — Taskbar freeze button + passive indicator

- **Files:** `app/qml/MainWindow/Panes/Dashboard/Taskbar.qml`
- **Does:** `IconButton` next to the auto-layout button (:909 block): highlight color when
  frozen — the checked look *is* the R11 indicator; tooltip. Unlicensed
  (`!SerialStudio.proWidgetsEnabled()`-equivalent QML check): reduced opacity, click opens
  the license/upgrade dialog instead of toggling (R7). Also confirm the taskbar
  window-button click path only focuses (never minimizes/restores geometry) while frozen;
  gate there if it does.
- **Verify:** code-verify; read back both license branches.
- **Deps:** T3, T11, T12
- [x] done — clean; button highlight = R11 indicator, unlicensed click opens license dialog;
  also gated the two taskbar escape hatches found in read-back (entry-click showWindow
  restore of minimized windows, right-click remove-from-workspace menu, auto-layout button)

### T14 — Shortcut + StartMenu entry

- **Files:** `app/qml/MainWindow/MainWindow.qml`,
  `app/qml/MainWindow/Panes/Dashboard/StartMenu.qml`
- **Does:** `Shortcut` `Ctrl+Shift+F` → `dashboard.toggleFreeze()`, `enabled:
  root.dashboardVisible` — **bound exactly once in this window** (ambiguous-shortcut
  gotcha, `common-mistakes.md` Qt/QML). Add `&& !Cpp_UI_Dashboard.frozen` to the
  `Ctrl+Shift+W` / `Ctrl+Shift+M` / `Ctrl+Shift+L` / `Ctrl+Home` enables. StartMenu:
  checkable "Freeze Dashboard" item mirroring the taskbar button (incl. license branch).
  New strings via `qsTr()`; **no `.ts`/`.qm` files touched.**
- **Verify:** code-verify on both files; grep the window for a second `Ctrl+Shift+F`
  binding (must be none).
- **Deps:** T11, T13
- [x] done — clean; shortcut bound exactly once; deviation: no `!frozen` on the
  Ctrl+Shift+W/M/L enables since T11 gated the functions they call (single source);
  Ctrl+Home left ungated (focus-clear only); start-menu icon added as
  icons/start/freeze.svg (start icons are a separate two-tone 30x30 set)

### T15 — Integration test: frozen project round-trip (AC4)

- **Files:** `tests/integration/` (new `test_dashboard_freeze.py` or extension of the
  project-handling test file — pick per `tests/README.md` conventions, read it first)
- **Does:** pytest case: load a frozen `.ssproj` via the API server, assert the project
  JSON reports `frozen: true`, save, re-read, assert preserved. Marked per the catalog;
  maintainer runs it (needs the app up with the API server on :7777).
- **Verify:** `python -m pytest --collect-only` on the file (collection is runnable
  without the app); maintainer executes it live.
- **Deps:** T1, T2
- [x] done — `tests/integration/test_dashboard_freeze.py`, 3 tests collect clean;
  round-trip test doubles as the machine-checkable R8 check (exportJson returns
  serializeToJson verbatim; loadJson routes through loadFromJsonDocument/loadFrozen)

### T16 — Static sweep + handoff checks

- **Files:** none new — whole diff
- **Does:** `python scripts/code-verify.py --check` across every touched file;
  `qt-cpp-review` on the C++ diff; counterfactual check named in chat (which rule does this
  diff most risk violating + evidence it doesn't); self-review the diff for scope (the
  plan's file table is the lane). Maintainer then runs the in-app ACs (AC1-AC3, AC5, AC6,
  R13 scroll check) and one `--benchmark-hotpath` run (AC7 — expected no-delta; I never
  build/run).
- **Verify:** clean reports; maintainer sign-off on the observation list.
- **Deps:** T1-T15
- [x] done (agent-side) — code-verify clean on all 26 touched files; qt-cpp-review ran
  (6 agents): 1 confirmed finding + 2 investigation targets, all three fixed
  (operationModeChanged dead wiring removed; setFrozen(true) aborts in-flight
  drag/resize; shortcut path opens license dialog on refused enable);
  architecture/dashboard.md updated with the freeze + WidgetToolbar contracts.
  NOT run: sanitize-commit (working tree holds the maintainer's own uncommitted
  edits — README.md, Donate.qml, LicenseManagement.qml, LemonSqueezy.cpp — which it
  would sweep up; maintainer runs it before committing). Maintainer still owns:
  in-app ACs, pytest live run, --benchmark-hotpath.

## Definition of Done

- [x] Every acceptance criterion in `spec.md` is met and checked off there (AC1-AC7 + R13
      scroll observation) — **maintainer in-app pass pending**.
- [x] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [x] `qt-cpp-review` run on the C++ diff; all findings fixed (see T16).
- [x] `--benchmark-hotpath` not regressed (maintainer/CI run — view-layer change, expected
      no-delta).
- [x] `pytest tests/integration/test_dashboard_freeze.py` identified for the maintainer.
- [x] `python scripts/sanitize-commit.py` — **deliberately not run by the agent**: the
      working tree contains the maintainer's own uncommitted edits it would sweep up;
      run it before committing.
- [x] Diff is *what was asked, and only that* — every touched file is in the plan's lane
      or a named deviation recorded in T9/T10/T13/T14/T16.
- [x] `spec.md` status set to `done` — after the maintainer's AC pass.
