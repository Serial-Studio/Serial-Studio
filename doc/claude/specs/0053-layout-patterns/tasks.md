---
spec: 0053-layout-patterns
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-08-13
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
- [ ] done

### T6 — Workspace persistence

- **Files:** `app/src/DataModel/Frame.h`,
  `app/src/DataModel/Project/ProjectModelWorkspaces.cpp`
- **Does:** `Keys::LayoutPattern` + `Keys::LayoutRatio`; `Workspace::layoutPattern` (QString)
  and `layoutRatio` (int sixteenths, default 8); serialize omitting both at defaults so an
  untouched project stays byte-identical; read clamps the ratio to 1..15 and falls back to
  Grid on an unknown pattern id rather than failing the parse. Mutators open an undo scope
  and `setModified(true)` like the other workspace edits.
- **Verify:** code-verify clean; round-trip covered in T10.
- **Deps:** none
- [ ] done

### T7 — Pattern artwork + registration

- **Files:** `app/rcc/layouts/*.svg`, `app/rcc/rcc.qrc`
- **Does:** One SVG per pattern (Grid, Master + Stack, Master + Grid, Row, Column, Spiral)
  drawn in the existing icon palette, showing the arrangement at a representative widget
  count; register each in the qrc.
- **Verify:** `python scripts/registry-verify.py` (qrc sync).
- **Deps:** none
- [ ] done

### T8 — Picker UI + live apply

- **Files:** `app/qml/MainWindow/Panes/Dashboard/LayoutPatternPicker.qml` (new),
  `app/qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml`, `app/src/UI/Taskbar.{h,cpp}`,
  `app/CMakeLists.txt`
- **Does:** Artwork picker with the active pattern checked, plus a ratio control offering
  only ladder stops and shown only for patterns with a primary region. Entry point beside
  "Tile Windows" in the canvas context menu. Selecting writes to the active workspace and
  re-tiles immediately; Taskbar exposes the active workspace's pattern/ratio and re-tiles on
  `activeGroupIdChanged` via the existing `triggerLayoutUpdate()` path (do not add a second
  layout entry point).
- **Verify:** code-verify clean; maintainer applies each pattern (AC1/AC5).
- **Deps:** T5, T6, T7
- [ ] done

### T9 — Pattern geometry tests

- **Files:** `app/tests/tst_layout_patterns.cpp`
- **Does:** Extend the suite to every pattern for n=1..12 across several canvas sizes and both
  orientations: full coverage of the available area, no overlaps, nothing below the floor,
  and byte-identical output on repeated calls (determinism is what the spec leans on). Add
  ratio-stop handling and out-of-range clamping.
- **Verify:** `ctest -R tst_layout_patterns` (AC2).
- **Deps:** T5
- [ ] done

### T10 — Persistence integration tests

- **Files:** `tests/integration/test_layout_patterns.py` (new)
- **Does:** Workspace round-trip of `layoutPattern`/`layoutRatio`; both omitted at defaults;
  unknown pattern id and out-of-range ratio fall back instead of erroring; two workspaces
  with different patterns keep them independently (AC3/AC7).
- **Verify:** `pytest tests/integration/test_layout_patterns.py -v` against a running,
  rebuilt app.
- **Deps:** T6
- [ ] done

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
- [ ] done

### T12 — API surface + docs

- **Files:** `app/src/API/EnumLabels.cpp`, `doc/help/*`, `app/rcc/ai/skills/dashboard_layout.md`
- **Does:** Pattern slugs/labels for the API; document the picker, the pattern catalog, the
  ratio control and the frozen-merge behavior following `ss-docs` conventions; update the AI
  corpus's layout description.
- **Verify:** `scripts/documentation-verify.py`; sanitize-commit rebuilds the search index.
- **Deps:** T8, T11
- [ ] done

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` met, or explicitly handed to the maintainer
      (AC1/AC5/AC6 visual + the rebuild-dependent ctest/pytest runs).
- [ ] `python scripts/code-verify.py --check` clean on all changed files.
- [ ] `qt-cpp-review` run on the C++ diff; findings addressed or noted.
- [ ] AC4 proven by T2's baseline suite, not by inspection.
- [ ] `pytest` targets listed for the maintainer (T10).
- [ ] `python scripts/sanitize-commit.py` run; `registry-verify.py` clean.
- [ ] Diff is *what was asked, and only that* — manual mode's snapping, welding and spacing
      behavior unchanged (AC8).
- [ ] `spec.md` status set to `done`.
