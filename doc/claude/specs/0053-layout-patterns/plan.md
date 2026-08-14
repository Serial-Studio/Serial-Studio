---
spec: 0053-layout-patterns
phase: plan
status: approved     # draft -> approved (gate before /ss-tasks)
updated: 2026-08-13
---

# Plan 0053 — Per-workspace auto-layout patterns + frozen shared borders

> **Phase 2 of 4 — the HOW.** Grounded in the current tiling code
> (`autoLayoutColumnCount` / `tileExactGrid` / `tileUnevenColumns` / `dispatchTile`), the
> `Workspace` struct in `Frame.h`, and the spec-0052 snap work.

## Approach (one paragraph)

Extract tiling into a new **pure** module, `UI::Layouts`, that answers
`tile(count, pattern, env) -> QVector<QRect>` with no Qt Quick dependency — the same shape
`UI::Snap` already has, and the thing that makes the spec's "no GUI required" acceptance
criterion possible. `WindowManager::autoLayout()` becomes a thin caller: build the env, ask
for rectangles, place windows. Today's algorithm moves verbatim into the `Grid` pattern so
existing projects render identically. The chosen pattern and its split ratio become two
optional `Workspace` fields, edited from a visual picker whose artwork is a bundled SVG per
pattern. The frozen shared-border merge is computed as per-window *edge flags* published to
QML — adjacency is geometry the WindowManager already knows — so borders change without any
stored geometry changing.

## Affected subsystems & files

| File | Change |
|------|--------|
| `app/src/UI/LayoutPatterns.h` (new) | `UI::Layouts`: `Pattern` enum, `LayoutEnv`, pure `tile()`, `patternId()`/`patternFromId()`, `ratioStops()` |
| `app/src/UI/LayoutPatterns.cpp` (new) | The six tiling functions; Grid holds the current math verbatim |
| `app/src/UI/WindowManager.h/.cpp` | `autoLayout()` delegates to `Layouts::tile()`; drop the local tiling statics; per-window merged-edge computation + publish; re-tile on workspace/pattern/ratio change |
| `app/src/DataModel/Frame.h` | `Workspace::layoutPattern` (QString) + `layoutRatio` (int sixteenths); `Keys::LayoutPattern`/`Keys::LayoutRatio`; serialize/read (omit at defaults) |
| `app/src/DataModel/Project/ProjectModelWorkspaces.cpp` | Getters/setters for the two fields, `setModified(true)` + undo scope like the other workspace mutators |
| `app/src/UI/Taskbar.{h,cpp}` | Expose active workspace's pattern/ratio to QML; re-tile on `activeGroupIdChanged` |
| `app/qml/MainWindow/Panes/Dashboard/LayoutPatternPicker.qml` (new) | Artwork grid + ratio control (ladder stops only) |
| `app/qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml` | Picker entry point (canvas context menu, beside "Tile Windows") |
| `app/qml/MainWindow/Panes/Dashboard/WidgetDelegate.qml` | Pass merged-edge flags to `MiniWindow` |
| `app/qml/Widgets/MiniWindow.qml` | Suppress the duplicated edge + square the corners on merged edges |
| `app/rcc/layouts/*.svg` + `app/rcc/rcc.qrc` | One artwork file per pattern, registered |
| `app/tests/tst_layout_patterns.cpp` + `app/tests/CMakeLists.txt` | Pure-geometry suite (AC2 + AC4 baselines) |
| `app/CMakeLists.txt` | New sources/headers/QML |

## Architecture & data flow

```
Workspace(layoutPattern, layoutRatio)      canvas resize / widget count change
            |                                          |
            v                                          v
   Taskbar (active workspace)  ---->  WindowManager::autoLayout()
                                              |
                            Layouts::tile(count, pattern, env) -> QVector<QRect>   [pure]
                                              |
                                        placeWindow(...)
                                              |
                                    computeMergedEdges()  --(flags)--> WidgetDelegate -> MiniWindow
```

`LayoutEnv` carries what the tiler needs and nothing else: `margin`, `spacing`, `availW`,
`availH`, `isLandscape`, `minWidth`, `minHeight`, `ratioNum`, `ratioDen`. Purity is the
contract — same inputs, same rectangles, no singletons, no `QQuickItem`.

**Patterns.** Grid = today's `autoLayoutColumnCount` + `tileExactGrid`/`tileUnevenColumns`,
moved unchanged. Master + Stack: primary occupies `ratio` of the long axis, remainder splits
the rest evenly. Master + Grid: as above but the remainder is gridded via the Grid tiler
(recursion into a sub-env). Row/Column: single strip. Spiral: alternate splitting the
remaining rect, each new widget taking `ratio` of it, last two share the final rect.
Every pattern degrades to Grid when a subdivision would breach `minWidth`/`minHeight`
(constraint: never emit unusable windows).

**Merge.** After placement, for each window compute which of its four edges coincides
(within the layout spacing) with a sibling edge, and publish a per-window bitmask. QML draws
a merged edge as no border + square corner. Gated on `frozen && titlebar hidden` for both
neighbours, evaluated in QML from live state so unfreezing reverts with no recompute.

## Hotpath & threading impact

- **Touches the hotpath?** No. Layout runs on gesture, canvas resize, workspace switch,
  widget-count change and pattern/ratio edits — never per frame, never on the parse path.
  No `--benchmark-hotpath` exposure; the gate is unaffected.
- **New cross-thread signal/slot?** No. Everything is GUI thread: `WindowManager`,
  `Taskbar`, `ProjectModel` and QML.
- **New input to a cached hotpath flag?** No.
- **Timestamp ownership** — untouched.

## Data model & persistence

- `Keys::LayoutPattern("layoutPattern")`, `Keys::LayoutRatio("layoutRatio")` on `Workspace`.
- `layoutPattern` omitted when empty (= Grid), `layoutRatio` omitted when 8 (= 1/2), so an
  untouched project serializes byte-identically to today (AC4/AC7).
- Ratio stored as **integer sixteenths** (1..15): ladder-native, trivially validated,
  reduces for display via the existing `Snap::fractionLabel` vocabulary. Unknown pattern ids
  and out-of-range ratios fall back to the defaults on load rather than failing the parse.
- No writer-version bump; both keys additive.

## API / SDK surface

Workspace read/write already flows through the project handlers; the two fields ride along
as additional optional keys. `EnumLabels` gains pattern slugs so the API can name them.
No new commands; no commercial gating.

## QML / UI

`LayoutPatternPicker.qml`: a grid of pattern tiles (artwork + name, current one checked) plus
a ratio selector shown only for patterns that have a primary region, offering ladder stops.
Reached from the canvas context menu next to the existing "Tile Windows" entry. Selecting a
pattern writes through to the workspace and re-tiles immediately.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Tiling shape | pure `tile()` returning rects / keep placing items in-place / hybrid | **Pure** — the only version that satisfies AC2 without a GUI, and it mirrors `UI::Snap` |
| Module home | new `UI::Layouts` / extend `UI::Snap` / private to WindowManager | **New module** — Snap resolves gestures, Layouts arranges; merging them would blur two clean responsibilities |
| Pattern storage | `Workspace` fields / `widgetSettings` / QSettings | **Workspace fields** — it is per-workspace document state by definition; widgetSettings is per-widget and no-ops outside ProjectFile |
| Ratio encoding | int sixteenths / "5/8" string / double | **Int sixteenths** — ladder-native, exactly representable, validates as a range |
| Grid fidelity | move math verbatim / re-derive cleanly | **Verbatim** — AC4 demands pixel-identical output; a tidier rewrite risks silent drift |
| Merge mechanism | published edge flags / QML neighbour queries / overlap windows by 1 px | **Edge flags** — presentation-only by construction, and WindowManager already knows the geometry; overlapping would mutate stored geometry, violating R8 |
| Surplus/degenerate cases | degrade to Grid / allow tiny windows / hide widgets | **Degrade** — never emit a window under the floor, never hide a widget |

## Risks & mitigations

- **AC4 regression (existing dashboards shift).** Highest risk. Mitigated by moving the Grid
  math verbatim and by a ctest that pins Grid output against a baseline captured from the
  current algorithm for n=1..12 across several canvas sizes and both orientations.
- **Spiral/master recursion producing sub-minimum windows** on small canvases — every
  pattern checks the floor before subdividing and falls back to Grid.
- **Merge flicker** during freeze/unfreeze or drag: flags recompute only on placement, and
  QML gates on live `frozen`/titlebar state, so no recompute storm.
- **Workspace switch races** the widget rebuild: re-tile on `activeGroupIdChanged` *after*
  the widget map settles, reusing the existing `triggerLayoutUpdate()` path rather than a new
  one.
- **Silent drift between the ladder used by snapping and by ratios** — both read
  `UI::Snap`'s denominators; no second copy of the ladder.

## Test & verification plan

- **Unit (ctest, pure — runnable once the maintainer builds):** `tst_layout_patterns.cpp`
  - AC2: for every pattern, n=1..12, several canvas sizes/orientations — assert full
    coverage, no overlaps, no window below the floor, deterministic repeat calls.
  - AC4: Grid output matches the captured baseline of today's algorithm exactly.
  - Ratio handling: stops honored; out-of-range clamps.
- **Integration (pytest, maintainer runs with the app up):** workspace round-trip of
  `layoutPattern`/`layoutRatio`; absence-at-defaults; unknown pattern id falls back.
- **Maintainer visual:** picker artwork legibility, each pattern applied, ratio re-tiles
  live, frozen shared borders in light + dark themes, unfreeze reverts (AC1/AC5/AC6).
- **Hotpath:** not touched; no benchmark run required beyond the standard pre-commit gate.
- **Static:** `code-verify.py --check`, `registry-verify.py` (qrc/artwork), `qt-cpp-review`
  on the C++ diff, `sanitize-commit.py` before handoff.
