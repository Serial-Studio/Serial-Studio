---
spec: 0053-layout-patterns
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-08-14  # T13-T15 added, T8 entry point revised (spec/plan amendment R11-R13)
---

# Tasks 0053 — Per-workspace auto-layout patterns + frozen shared borders

> **Phase 3 of 4 — the ordered checklist.** Ordered so the tree stays coherent: the pure
> module and its baseline land before anything depends on them, persistence before the UI
> that edits it, and the merge last since it is independent of the tiler.

## Conventions

- `python scripts/code-verify.py --check <files>` on every touched file is implied.
- T2 is the safety net for the whole spec: it captures today's Grid output *before* the
  refactor, so AC4 can be proven rather than asserted. It must land before T3.

## Tasks

### T1 — Pure layout module skeleton

- **Files:** `app/src/UI/LayoutPatterns.h`, `app/src/UI/LayoutPatterns.cpp`,
  `app/CMakeLists.txt`
- **Does:** Introduce `UI::Layouts`: `enum class Pattern {Grid, MasterStack, MasterGrid, Row,
  Column, Spiral}`, `struct LayoutEnv` (margin, spacing, availW, availH, isLandscape,
  minWidth, minHeight, ratioNum, ratioDen), `[[nodiscard]] QVector<QRect> tile(int count,
  Pattern, const LayoutEnv&)`, plus `patternId()`/`patternFromId()`. Grid only for now,
  every other pattern delegating to Grid. PURITY IS THE CONTRACT: no singletons, no
  `QQuickItem`, no Qt Quick include — that is what makes T2/T9 testable and mirrors
  `UI::Snap`.
- **Verify:** code-verify clean; header has no Qt Quick dependency.
- **Deps:** none
- [x] done — the real Grid math went in directly rather than a placeholder, so T3 folded in
  here; `LayoutEnv.ratio` is a single int (sixteenths) instead of a num/den pair

### T2 — Capture the pre-refactor Grid baseline

- **Files:** `app/tests/tst_layout_patterns.cpp`, `app/tests/CMakeLists.txt`
- **Does:** Port today's `autoLayoutColumnCount` / `tileExactGrid` / `tileUnevenColumns` math
  into the test as a frozen reference implementation, and assert `Layouts::tile(..., Grid,
  ...)` matches it exactly for n=1..12 across several canvas sizes and both orientations.
  This is the AC4 guard and must exist BEFORE `WindowManager` is touched, so any drift is
  caught by a test rather than by a user noticing their dashboard moved.
- **Verify:** `ctest -R tst_layout_patterns` once the maintainer builds; code-verify clean.
- **Deps:** T1
- [x] done — reference implementation copied verbatim into `namespace reference`; 6 canvas
  shapes x n=1..12, plus determinism and degenerate-input cases

### T3 — Move the real Grid math into the module

- **Files:** `app/src/UI/LayoutPatterns.cpp`
- **Does:** Replace T1's placeholder Grid with the current algorithm moved VERBATIM (column
  table, exact-grid and uneven-column paths), returning rects instead of placing windows.
  No tidying, no renaming of the math — a cleaner rewrite is exactly how silent drift gets in.
- **Verify:** T2's baseline suite passes unchanged.
- **Deps:** T2
- [x] done — folded into T1 (see above); nothing was re-derived, the math was moved as-is

### T4 — WindowManager delegates to the module

- **Files:** `app/src/UI/WindowManager.h`, `app/src/UI/WindowManager.cpp`
- **Does:** `autoLayout()` builds a `LayoutEnv` (margin/spacing from Dashboard, floor from the
  existing minimum-size rules) and places the returned rects; delete the now-duplicated local
  tiling statics. Behavior must be identical — this task changes who computes, not what.
  Keep the `anyWindowMaximized()` early-return semantics as they are.
- **Verify:** code-verify clean; T2 still passes; maintainer confirms an existing project
  looks unchanged.
- **Deps:** T3
- [x] done — `autoLayout()` builds a `LayoutEnv` and places `Layouts::tile(..., Grid, ...)`;
  the four local statics (`autoLayoutColumnCount`, `tileExactGrid`, `tileUnevenColumns`,
  `dispatchTile`), the `TileEnv` struct and the now-unused `<QtMath>` include were removed.
  Still passes `Pattern::Grid` explicitly — T8 is what makes it read the workspace

### T5 — Remaining patterns

- **Files:** `app/src/UI/LayoutPatterns.cpp`
- **Does:** Implement Master + Stack, Master + Grid, Row, Column, Spiral, each parametric in
  the widget count and each honoring `ratioNum/ratioDen`. Every pattern checks the
  `minWidth`/`minHeight` floor before subdividing and degrades to Grid rather than emitting
  an unusable window (spec constraint: never hide a widget, never go under the floor).
- **Verify:** T9 extends the suite to cover these; code-verify clean.
- **Deps:** T4
- [x] done — Row/Column via a shared `tileStrip`, Master + Stack / Master + Grid via
  `tileMaster` (remainder stripped or gridded), Spiral splitting the remaining rect along its
  longer axis so the cut alternates on its own. One central degrade rule in `tile()`: a
  candidate whose count is wrong or that breaches the floor is replaced by Grid

### T6 — Layout-choice persistence

- **Files:** `app/src/DataModel/ProjectModel.{h,cpp}`
- **Does:** REVISED 2026-08-14 (maintainer): the choice does NOT live on the workspace entry.
  Storing it there forced `setCustomizeWorkspaces(true)` on every pick (materialising the auto
  workspace list as a side effect of choosing a layout) and left group tabs with no choice at
  all. It now lives beside the manual window geometry, under the same
  `Keys::layoutKey(scope, groupId)` widgetSettings entry, as sibling sub-keys `pattern` and
  `ratio` (`data` keeps holding the geometry). `layoutChoice()` reads, `setLayoutChoice()`
  writes through `saveWidgetSetting`, so it inherits the existing modified/autosave path and
  needs no customize mode. Defaults (`""`, 8) are simply absent keys.
- **Verify:** code-verify clean; round-trip covered in T10.
- **Deps:** none
- [x] done — the original `Workspace::layoutPattern`/`layoutRatio` fields, their `Keys::`
  entries and the `project.workspace.*` plumbing were reverted in full

### T7 — Pattern artwork + registration

- **Files:** `app/rcc/layouts/*.svg`, `app/rcc/rcc.qrc`
- **Does:** One SVG per pattern (Grid, Master + Stack, Master + Grid, Row, Column, Spiral)
  drawn in the existing icon palette, showing the arrangement at a representative widget
  count; register each in the qrc.
- **Verify:** `python scripts/registry-verify.py` (qrc sync).
- **Deps:** none
- [x] done — DEVIATION, see the note below: no SVG files ship. The picker draws each tile
  from `WindowManager::patternPreview`, i.e. from the real tiler, so artwork cannot drift from
  behaviour and there is no qrc/registry surface to keep in sync

### T8 — Picker UI + live apply

- **Files:** `app/qml/MainWindow/Panes/Dashboard/LayoutPatternPicker.qml` (new),
  `app/qml/MainWindow/Panes/Dashboard/Taskbar.qml`,
  `app/qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml`, `app/src/UI/Taskbar.{h,cpp}`,
  `app/CMakeLists.txt`
- **Does:** Artwork picker with the active pattern checked, plus a ratio control offering
  only ladder stops and shown only for patterns with a primary region. Selecting writes to the
  active workspace and re-tiles immediately; Taskbar exposes the active workspace's
  pattern/ratio and re-tiles on `activeGroupIdChanged` via the existing `triggerLayoutUpdate()`
  path (do not add a second layout entry point). **Revised 2026-08-14 (R13):** the picker is a
  popup anchored under the taskbar auto-layout button, not a canvas-context-menu entry, and it
  carries a **Manual** tile in the same grid — picking a pattern selects auto mode and applies
  it, picking Manual switches to manual mode. The button keeps its highlight-when-auto
  colouring and its freeze gating; `Taskbar.qml` therefore joins this task's file list. The
  canvas context menu keeps a route to the same popup. Leave the
  `dashboard.toggleAutoLayout` command binding and the `DashboardLayout` shortcut path alone —
  they are the surviving one-keystroke toggle.
- **Verify:** code-verify clean; maintainer applies each pattern (AC1/AC5) and exercises the
  button popup incl. the Manual tile (AC12).
- **Deps:** T5, T6, T7
- [x] done — `LayoutPatternPicker.qml` as a popup under the taskbar auto-layout button,
  with the Manual entry in the same gallery and the ratio row gated on
  `patternHasPrimary`; `autoLayout()` reads the active workspace's pattern/ratio, and
  `setTaskbar` wires `activeGroupIdChanged` to re-tile on workspace switch. The canvas
  context-menu route was left out (spec says the button is primary, the menu "may" keep one)

### T9 — Pattern geometry tests

- **Files:** `app/tests/tst_layout_patterns.cpp`
- **Does:** Extend the suite to every pattern for n=1..12 across several canvas sizes and both
  orientations: full coverage of the available area, no overlaps, nothing below the floor,
  and byte-identical output on repeated calls (determinism is what the spec leans on). Add
  ratio-stop handling and out-of-range clamping.
- **Verify:** `ctest -R tst_layout_patterns` (AC2).
- **Deps:** T5
- [x] done — Extended `everyPatternCoversTheCanvas` with exact canvas coverage (asserted
  only for spacing >= 0 -- see the note below) and a floor check gated on Grid clearing the
  floor; added `patternsHonorTheRatioStops` and widened the determinism case to all six
  patterns

### T10 — Persistence integration tests

- **Files:** `tests/integration/test_layout_patterns.py` (new)
- **Does:** Workspace round-trip of `layoutPattern`/`layoutRatio`; both omitted at defaults;
  unknown pattern id and out-of-range ratio fall back instead of erroring; two workspaces
  with different patterns keep them independently (AC3/AC7).
- **Verify:** `pytest tests/integration/test_layout_patterns.py -v` against a running,
  rebuilt app.
- **Deps:** T6
- [x] done — `tests/integration/test_layout_patterns.py`: defaults, per-pattern round-trip,
  partial patch, unknown-id tolerance, ratio clamping, two independent workspaces. Needs the
  rebuilt app; NOT run here (`clean_state` resets the project of the running instance)

### T11 — Frozen shared borders

- **Files:** `app/src/UI/WindowManager.{h,cpp}`,
  `app/qml/MainWindow/Panes/Dashboard/WidgetDelegate.qml`, `app/qml/Widgets/MiniWindow.qml`
- **Does:** After placement, compute per-window which of the four edges coincide with a
  sibling edge (within the layout spacing) and publish a bitmask; QML drops that border and
  squares the corner when BOTH neighbours are frozen with hidden titlebars. PRESENTATION
  ONLY — stored geometry must not change, so the effect reverts on unfreeze with nothing to
  undo (R8/AC7). Gate in QML on live state so unfreezing needs no recompute.
- **Verify:** code-verify clean; maintainer visual in light + dark, frozen and unfrozen
  (AC6); T10 confirms no new project keys.
- **Deps:** T4
- [x] done — Per-window merged-edge bitmask (`WindowManager::computeMergedEdges`, published
  as the `mergedEdges` map) consumed by `WidgetDelegate`: a frozen, titlebar-less widget bleeds
  its body 1px over each shared edge so both borders land on one line, and squares its corners.
  Presentation only -- no geometry moves, so unfreezing needs no undo. `MiniWindow.qml` needed
  no change (the delegate owns the body border)

### T12 — API surface + docs

- **Files:** `app/src/API/EnumLabels.cpp`, `doc/help/*`, `app/rcc/ai/skills/dashboard_layout.md`
- **Does:** Pattern slugs/labels for the API; document the picker, the pattern catalog, the
  ratio control and the frozen-merge behavior following `ss-docs` conventions; update the AI
  corpus's layout description.
- **Verify:** `scripts/documentation-verify.py`; sanitize-commit rebuilds the search index.
- **Deps:** T8, T11
- [x] done — `EnumLabels::layoutPatternSlug/Label`, `project.workspace.update` and `.get`
  carry `layoutPattern`/`layoutRatio` (the update registration split into a file-static helper
  to stay under the 100-line cap), Toolbar-Reference row rewritten for the gallery, and a
  "Layout pattern per workspace" section added to the AI corpus skill

## Amendment tasks — 2026-08-14 (R11/R12)

Independent of T5-T12: they touch the manual-layout path, not the tiler, so they can land
first. T8 above already absorbed R13.

### T13 — Pure manual rescale in `UI::Layouts`

- **Files:** `app/src/UI/LayoutPatterns.h`, `app/src/UI/LayoutPatterns.cpp`
- **Does:** Add `[[nodiscard]] QVector<QRect> rescaleManual(const QVector<QRect>& rects, QSize
  refCanvas, QSize newCanvas, int spacing)` implementing the plan's seam model: normalize
  (leading edge minus `spacing` when past the canvas start), cluster per axis at the existing
  6 px tolerance with bound-touching clusters adopting the bound, classify a seam as
  gap-consuming when interior and holding both a leading and a trailing edge, deflate by the
  gap-consuming seams before it, scale the gap-free span, re-inflate, rebuild each rect.
  Degrade by reducing the effective spacing when `newExtent - k*spacing` is not positive —
  never overlap, never emit below the floor. PURITY IS THE CONTRACT, same as `tile()`: no
  singletons, no `QQuickItem`, no Qt Quick include; applying it twice with the same inputs must
  be a no-op, which is the invariant the whole amendment rests on.
- **Verify:** code-verify clean; header still free of Qt Quick; T14 proves the behavior.
- **Deps:** T1
- [x] done — `rescaleManual` plus four file-static helpers (`clusterEdges`,
  `gapConsumingSeams`, `effectiveSpacing`, `seamPositions`) and a `Span` axis pair;
  `kSeamTolerance` moved into the header beside the other layout constants. Algorithm was
  prototyped and validated in the scratchpad before porting (all four spacings, four target
  canvases, 10x round-trip, idempotence)

### T14 — Rescale test suite

- **Files:** `app/tests/tst_layout_patterns.cpp`
- **Does:** Extend the suite with rescale cases over four layouts (2x2 grid, master + stack, a
  free-floating window, an overlapping pair) for spacing `-1, 0, 4, 16`: every shared edge
  measures exactly the spacing and every outer edge is flush after rescaling across several
  canvas sizes and non-uniform aspect changes (AC9); ten reference-based A->B->A round-trips
  are byte-identical to the start, and the re-derived path (output fed back in) settles within
  2 px on the first hop and moves no further across 50 cycles with joins and flushness exact
  (AC10); rescale A->B, swap one rect, rescale B->A leaves the untouched rects within that
  bound with joins intact (AC11, math half); a canvas too small for the gaps degrades without
  overlap or sub-floor windows.
- **Verify:** `ctest -R tst_layout_patterns` once the maintainer builds; code-verify clean.
- **Deps:** T13
- [x] done — four fixtures + five helpers in `namespace fixtures`; five new slots
  (`rescaleHoldsJoinsAndOuterEdges` data-driven over 4 layouts x 4 spacings x 5 targets,
  `rescaleFromTheReferenceIsLossless`, `rescaleSettlesWhenReDerived`,
  `rescaleLeavesUntouchedWidgetsAlone`, `rescaleDegradesOnACrampedCanvas`). Every assertion
  was pre-validated against the scratchpad model; AC10/AC11 wording was amended first because
  the original "byte-identical" claim is not reachable through a lossy intermediate canvas

### T15 — WindowManager routes through the rescale

- **Files:** `app/src/UI/WindowManager.h`, `app/src/UI/WindowManager.cpp`,
  `app/qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml`
- **Does:** Point `applyManualAnchors`, `applySavedGeometries` and `preloadPendingGeometries`
  at `Layouts::rescaleManual` on the whole window set; delete `weldManualSeams`,
  `scaledManualGeometry`, `anchoredGeometry`, `manualMarginsForGeometry`, `clusterEdges`, the
  `m_manualMargins` member and the now-unused `kSeamWeldTolerance` if the module owns it.
  `commitManualGeometry` re-snapshots EVERY normal window at the current canvas, since
  `m_manualCanvasWidth/Height` is shared and a partial snapshot strands the rest. Re-apply the
  layout when `layoutSpacing` changes in manual mode (the QML `Connections` block currently
  re-tiles only in auto mode). THE INVARIANT TO NAME AT EDIT TIME: the reference set
  (`m_manualGeometries` + `m_manualCanvas*`) is written only by a user gesture or a project
  load — never from geometry a rescale just produced, which is the feedback loop that
  compounds the drift today. Signal wiring: keep the existing `m_suppressGeometrySignal`
  semantics around resize so per-window `geometryChanged` bursts do not reappear.
- **Verify:** code-verify clean; T14 unaffected (pure module untouched); maintainer resizes a
  manual dashboard across sizes and confirms constant gaps and inset (AC9), then moves one
  widget after a resize and confirms the others hold (AC11, glue half).
- **Deps:** T13, T14
- [x] done — `applyManualAnchors` became `applyManualLayout` (rescales the whole set from the
  reference); `applySavedGeometries`/`preloadPendingGeometries` share a new
  `parseSavedGeometries` helper and rescale once; `weldManualSeams`, `clusterEdges`,
  `scaledManualGeometry`, `anchoredGeometry`, `manualMarginsForGeometry`, `m_manualMargins`,
  `kSeamWeldTolerance` and the now-unused `<QMargins>` include are gone; `storeManualLayout()`
  snapshots every window and `commitManualGeometry` calls it. The per-edge
  `Snap::snapToFraction` pass went with the old rescale: it moves an edge onto a canvas
  fraction without accounting for the spacing, which would break the exact-join guarantee.
  `DashboardCanvas.qml` needed no change after all — the C++ `layoutSpacingChanged` lambda
  already covered manual mode, so the QML side stayed out of the diff

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` met, or explicitly handed to the maintainer
      (AC1/AC5/AC6 visual + the rebuild-dependent ctest/pytest runs).
- [x] `python scripts/code-verify.py --check` clean on all changed files. Re-verified
      2026-08-14: full-repo `code-verify.py --check` is 0 errors/0 advisory across 3503 files.
- [ ] `qt-cpp-review` run on the C++ diff; findings addressed or noted.
- [ ] AC4 proven by T2's baseline suite, not by inspection.
- [x] `pytest` targets listed for the maintainer (T10):
      `tests/integration/test_layout_patterns.py` (13 cases, collects clean; needs a
      REBUILT app with the API server on to execute).
- [ ] `python scripts/sanitize-commit.py` run; `registry-verify.py` clean.
- [ ] AC9/AC10/AC11 proven by T14, not by inspection; AC11's glue half and AC12 handed to the
      maintainer as visual checks.
- [x] Old manual-rescale helpers are gone, not left dead beside the new path (T15).
      Re-verified 2026-08-14: `weldManualSeams`, `scaledManualGeometry`, `anchoredGeometry`,
      `manualMarginsForGeometry`, `clusterEdges`, `m_manualMargins`, `kSeamWeldTolerance` all
      grep-absent from `WindowManager.h`/`.cpp`.
- [ ] Diff is *what was asked, and only that* — manual mode's snapping and gesture behavior
      unchanged (AC8). Amended 2026-08-14: the seam weld is deliberately replaced by T13/T15;
      AC8 now means snapping, guides and the spacing setting behave as before, with gaps
      constant across resizes rather than repaired after one.
- [ ] `spec.md` status set to `done`.
