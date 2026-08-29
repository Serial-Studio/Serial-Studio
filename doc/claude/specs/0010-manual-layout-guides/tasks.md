---
spec: 0010-manual-layout-guides
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-07-15
---

# Tasks 0010 — Figma-style alignment guides and small-window support in manual layout mode

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

### T1 — SnapGuides resolver: types + move snap

- **Files:** `app/src/UI/SnapGuides.h` (new), `app/src/UI/SnapGuides.cpp` (new),
  `app/CMakeLists.txt`
- **Does:** Creates the pure value types (`SnapInput`, `SnapResult`, `GuideLine`,
  `SpacingIndicator`) and `resolveMoveSnap()` + `snapToGrid()`: edge/center candidates vs
  siblings + canvas edges/centerlines, nearest-within-6px per axis, edges beat centers,
  smart guide beats grid, guides spanning to the farthest aligned sibling; equal-gap
  detection for move. Pure functions, no WindowManager dependency, fixed loop bounds,
  reserve()d result vectors, canvas-clamped candidates only. Registers the new TU in
  CMake next to `WindowManager.cpp`.
- **Verify:** `python scripts/code-verify.py --check app/src/UI/SnapGuides.h
  app/src/UI/SnapGuides.cpp`; read-back of candidate/priority logic against plan
  "Resolver semantics".
- **Deps:** none
- [x] done

### T2 — SnapGuides resolver: resize snap + size match

- **Files:** `app/src/UI/SnapGuides.h`, `app/src/UI/SnapGuides.cpp`
- **Does:** Adds `resolveResizeSnap(..., ResizeEdge)`: adjusts only the moving edge(s) —
  the resolver must never emit a change to a non-moving edge (spec R2) — and adds sibling
  width/height size-match candidates reporting the matched sibling rect. Equal-spacing
  stays move-only per spec.
- **Verify:** `python scripts/code-verify.py --check` on both files; read-back: for each
  of the 8 `ResizeEdge` values, confirm the fixed edge set is preserved.
- **Deps:** T1
- [x] done

### T3 — WindowManager: publish surface + grid settings

- **Files:** `app/src/UI/WindowManager.h`, `app/src/UI/WindowManager.cpp`
- **Does:** Adds the notify properties (`alignmentGuides`, `spacingIndicators`,
  `sizeMatchRect` + visible, `manualGestureActive`, `manualGestureGeometry`,
  `gridEnabled`, `gridSize`) with guarded setters (skip emit when unchanged — the
  guide-churn mitigation), members for the published visuals + `m_snapSiblings`, and
  QSettings persistence (`WindowManager_GridEnabled`, `WindowManager_GridSize`, defaults
  false/16) read in the ctor next to the wallpaper key. No behavior change to gestures
  yet.
- **Verify:** `python scripts/code-verify.py --check app/src/UI/WindowManager.h
  app/src/UI/WindowManager.cpp`; header follows the Q_OBJECT → Q_PROPERTY → signals
  order; every setter has the guard return.
- **Deps:** none
- [x] done

### T4 — WindowManager: wire snapping into move/resize gestures

- **Files:** `app/src/UI/WindowManager.cpp`, `app/src/UI/WindowManager.h`
- **Does:** `startManualPress` snapshots `m_snapSiblings` (visible `"normal"` windows
  minus target); `handleDragMove` manual branch and `handleResizeMove` call the resolvers
  (skipping when `event->modifiers()` has Alt — bypass clears visuals), apply the snapped
  rect, then run the existing canvas clamps; publish visuals; `mouseReleaseEvent`,
  `setFrozen(true)`, and `clear()` reset all new visuals. Binding invariants named at
  edit time: (1) frozen/auto-layout early-outs stay the first statements — no new event
  entry points; (2) snapping runs before the existing `qBound`/edge clamps; (3) commit
  path (`commitManualGeometry`) is unchanged.
- **Verify:** `python scripts/code-verify.py --check` on both files; read-back of every
  gesture entry/exit path confirming visuals can never survive a gesture (release,
  freeze, clear all reset them).
- **Deps:** T1, T2, T3
- [x] done

### T5 — WindowManager: remove manual Aero snap + tear-off, lower manual floors

- **Files:** `app/src/UI/WindowManager.cpp`, `app/src/UI/WindowManager.h`
- **Does:** Deletes `computeSnapRect`, `updateManualSnapIndicator`, `applyManualSnap` and
  their call sites (`liftSnapBottom` + `snapIndicator` stay — auto-mode swap indicator
  uses them); removes the near-fullsize tear-off block in `handleDragMove` (with Aero
  gone it would shrink a fullscreen window to the new 48-px implicit size mid-drag);
  lowers `constrainWindows`' 100×80 fallback to 48×48 in manual mode (auto mode
  unaffected) so saved sub-100px windows reload unclamped (spec R9/R11).
- **Verify:** `python scripts/code-verify.py --check` on both files; grep confirms zero
  remaining references to the deleted functions and that the auto-mode
  `findOverlapTarget`/`tryReorderDraggedWindow`/`snapIndicator` path is untouched.
- **Deps:** T4
- [x] done

### T6 — WidgetDelegate: mode-dependent 48×48 minimum

- **Files:** `app/qml/MainWindow/Panes/Dashboard/WidgetDelegate.qml`
- **Does:** `minimumWidth/Height` become
  `windowManager.autoLayoutEnabled ? 356 : 48` (320/48 for height). Binding invariant:
  `implicitWidth/Height` (which C++ reads as the resize floor) must keep following these
  properties; `width/height` initial bindings unchanged.
- **Verify:** `python scripts/code-verify.py --check app/qml/MainWindow/Panes/Dashboard/
  WidgetDelegate.qml`; read-back that no other consumer of `minimumWidth` in the file
  assumes 356.
- **Deps:** T5 (C++ floors must accept 48 first, or a mid-sequence run inflates windows)
- [x] done

### T7 — MiniWindow: progressive caption collapse

- **Files:** `app/qml/Widgets/MiniWindow.qml`
- **Does:** Width-driven collapse ladder: `< 200` hide external-window button, `< 148`
  hide title, `< 120` hide minimize, `< 88` hide maximize; close never hides. Binding
  invariant: `externControlWidth` / `windowControlsWidth` must keep tracking the
  *visible* controls (the C++ caption hit-test reads them), so hiding must zero the
  measured MouseArea widths, not just opacity.
- **Verify:** `python scripts/code-verify.py --check app/qml/Widgets/MiniWindow.qml`;
  read-back: at width 48 the drag region is `48 - closeButtonWidth` and stays > 0.
- **Deps:** none (independent of C++ tasks; only matters visually below 200 px)
- [x] done

### T8 — Theme keys for guides + grid

- **Files:** `app/rcc/themes/default.json`, `app/rcc/themes/fluent-dark.json`,
  `app/rcc/themes/fluent-light.json`
- **Does:** Adds `alignment_guide` and `canvas_grid` to all three themes (accent-family
  guide color per theme; low-contrast grid line legible over wallpapers). Missing keys
  fail silent (black), so all three land in one diff.
- **Verify:** JSON parses (`python -c "import json,glob;
  [json.load(open(p)) for p in glob.glob('app/rcc/themes/*.json')]"`); grep shows both
  keys present in all three files.
- **Deps:** none
- [x] done

### T9 — DashboardCanvas: guide/badge/size-match overlay

- **Files:** `app/qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml`
- **Does:** Overlay `Item` at `z: _wm.zCounter + 9999`, visible only when
  `_wm.manualGestureActive && !_wm.autoLayoutEnabled && !_wm.frozen` (spec R10):
  guide-line Repeater (1-px rects, `alignment_guide`), spacing indicators with centered
  px labels, size-match highlight rect, and the geometry badge (`x, y · w × h` from
  `manualGestureGeometry`, anchored above the gesture rect, clamped into the canvas).
  Canvas coordinates throughout — no `LayoutMirroring` dependence.
- **Verify:** `python scripts/code-verify.py --check app/qml/MainWindow/Panes/Dashboard/
  DashboardCanvas.qml`; read-back of the visibility predicate against R10.
- **Deps:** T3, T4, T8
- [x] done

### T10 — DashboardCanvas: grid rendering + context-menu controls

- **Files:** `app/qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml`
- **Does:** QML `Canvas` above the wallpaper and under the windows, painting
  `canvas_grid` lines every `gridSize` px, visible only in manual mode with
  `gridEnabled`; `requestPaint()` only on size/gridSize/theme change (no per-frame
  repaints). Context menu gains checkable "Show Grid" and a "Grid Size" submenu
  (8/16/24/32/64) bound to `_wm.gridEnabled`/`_wm.gridSize`.
- **Verify:** `python scripts/code-verify.py --check` on the file; read-back: no binding
  ties `requestPaint` to gesture or data properties.
- **Deps:** T3, T8
- [x] done

### T11 — Docs: dashboard architecture note

- **Files:** `doc/claude/architecture/dashboard.md`, `CLAUDE.md` (only if the one-line
  layout note fits an existing bullet; otherwise dashboard.md alone)
- **Does:** Short subsection: manual-mode snapping lives in `UI::SnapGuides` (pure
  resolver, WindowManager publishes visuals), Aero snap is auto-mode-only, manual floor
  is 48×48 via `WidgetDelegate`'s mode-dependent minimum, grid prefs in QSettings.
- **Verify:** read-back against the shipped code; no stale claims (spec 0007 section
  untouched).
- **Deps:** T5, T6, T10
- [x] done

### T12 — Self-review + static gate

- **Files:** none new — the whole diff
- **Does:** Re-reads the full diff for scope (only planned files touched), runs
  `qt-cpp-review` on `SnapGuides.*`/`WindowManager.*`, then
  `python scripts/sanitize-commit.py`. Counterfactual check named at handoff: the rule
  this diff most risks is the frozen-mode input gate — evidence: no new event entry
  points, all new logic behind existing `m_frozen`/`autoLayoutEnabled()` early-outs,
  `setFrozen` clears the new visuals.
- **Verify:** `.code-report` shows no new errors; sanitize run clean; diff-vs-plan file
  list matches.
- **Deps:** T1-T11
- [x] done

## Definition of Done

- [x] Every acceptance criterion in `spec.md` is met and checked off there (AC1-AC11
      maintainer-observed in-app; AC12 via one `--benchmark-hotpath` run, pass/fail only).
- [x] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [x] `qt-cpp-review` run on the C++ diff; findings addressed or noted.
- [x] `--benchmark-hotpath` not regressed (formality — no hotpath code touched).
- [x] No pytest surface applies (canvas gestures unreachable via API); noted in plan.
- [x] `python scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [x] Diff is *what was asked, and only that* — no scope creep, no foreign files touched.
- [x] `spec.md` status set to `done`.
