---
spec: 0053-layout-patterns
phase: plan
status: approved     # draft -> approved (gate before /ss-tasks)
updated: 2026-08-14  # amendment section added (R11-R13); amendment itself awaits approval
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
| Pattern storage | `Workspace` fields / `widgetSettings` / QSettings | **`widgetSettings`, beside the manual geometry** (revised 2026-08-14). Workspace fields looked right but coupled the choice to customize mode — picking a layout would materialise the auto workspace list — and gave group tabs no choice at all. The `layout:<scope>:<gid>` entry is already the per-view layout record; the pattern belongs in it. It no-ops outside ProjectFile, so WindowManager holds the live value and the store is persistence only |
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

---

# Amendment — 2026-08-14 (R11-R13)

> Covers the spec amendment of the same date: constant manual-layout metrics (R11/R12) and the
> pattern gallery on the taskbar auto-layout button (R13). Grounded in the current manual-mode
> code (`applyManualAnchors`, `scaledManualGeometry`, `weldManualSeams`, `storeManualGeometry`,
> `serializeLayout`) and `Taskbar.qml`'s auto-layout button.

## Approach (one paragraph)

Manual rescaling stops being "scale each rectangle, then repair the seams" and becomes a single
**structure-preserving transform** of the whole layout: derive the per-axis *seam lines* the
layout is built on, map them into a gap-free coordinate space, scale that space, then re-inflate
with the configured spacing applied as a constant. Gaps and canvas-flush edges are therefore
outputs of the model, not survivors of a repair, and the fixed pixel tolerance no longer has to
absorb a spacing that has itself been scaled. The transform lands in the existing pure
`UI::Layouts` module beside `tile()`, so it is verifiable without a GUI (AC9/AC10), and
`WindowManager` calls it from the three places that currently rescale by hand. Two correctness
fixes ride along: the reference layout is re-snapshotted for *every* window when one is
committed (today a single commit re-bases the shared reference size, stranding the others), and
the rescale always reads that stored reference — never geometry it just produced. For R13, the
picker planned for the canvas context menu becomes a popup anchored to the taskbar auto-layout
button, with Manual as one of its entries.

## The rescale, precisely

Per axis, independently, given the reference rectangles, the reference canvas extent `R`, the
new extent `N`, and the spacing `s`:

1. **Normalize.** Each rectangle contributes a *leading* edge (`left`, minus `s` when `left > 0`)
   and a *trailing* edge (`left + width`). Subtracting `s` up front is what makes a seam's two
   sides land on the same value instead of `s` apart.
2. **Cluster** those values together with `0` and `R`, at the existing 6 px tolerance, into seam
   lines; a cluster touching a canvas bound adopts that bound.
3. **Classify.** A seam is *gap-consuming* when it is interior and holds both a leading and a
   trailing edge — a real join between two widgets, as opposed to a lone floating edge.
4. **Deflate.** Seam `i` maps to `pos - g(i) * s`, where `g(i)` counts the gap-consuming seams
   before it. The gap-free span is `R - k*s` for `k` gap-consuming seams; the new one is `N - k*s`.
5. **Scale** the deflated positions by `(N - k*s) / (R - k*s)`, then **re-inflate** by adding
   `g(i) * s` back. The outermost seams land exactly on `0` and `N`, so outer edges stay flush.
6. **Rebuild.** `left = pos(leading seam) + (left_ref > 0 ? s : 0)`, `right = pos(trailing seam)`.
   Every join therefore measures exactly `s`, at any canvas size, for any sign of `s`.
7. **Degrade** (spec constraint): if `N - k*s` is not positive the canvas cannot hold the gaps —
   reduce the effective `s` toward `0` until it is, and only then clamp to the minimum window
   size. Never overlap, never go below the floor.

Purity falls out: same rectangles + same two extents + same spacing = same output, and applying
it twice is a no-op, which is what kills the drift class (R12).

## Affected subsystems & files

| File | Change |
|------|--------|
| `app/src/UI/LayoutPatterns.h` | Add `rescaleManual(const QVector<QRect>&, QSize refCanvas, QSize newCanvas, int spacing)`; pure, no Qt Quick |
| `app/src/UI/LayoutPatterns.cpp` | The seam model: normalize, cluster, classify, deflate/scale/inflate, degrade |
| `app/src/UI/WindowManager.cpp` | `applyManualAnchors`, `applySavedGeometries`, `preloadPendingGeometries` all route through `rescaleManual`; `weldManualSeams` + `scaledManualGeometry` + `anchoredGeometry` + `manualMarginsForGeometry` deleted; `commitManualGeometry` re-snapshots every window; re-apply on manual spacing change |
| `app/src/UI/WindowManager.h` | Drop `weldManualSeams` + `m_manualMargins`; add the snapshot-all helper |
| `app/qml/MainWindow/Panes/Dashboard/Taskbar.qml` | Auto-layout button opens the gallery popup instead of toggling |
| `app/qml/MainWindow/Panes/Dashboard/LayoutPatternPicker.qml` (new, was T8) | Popup form; gains a Manual entry alongside the pattern tiles |
| `app/qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml` | Context-menu route to the same popup; re-apply layout on `layoutSpacingChanged` in manual mode too |
| `app/tests/tst_layout_patterns.cpp` | Rescale suite (AC9/AC10/AC11-math) |

## Architecture & data flow

```
m_manualGeometries + (m_manualCanvasWidth/Height)      <- snapshot of what the user built
            |
            |  canvas resize / project load / spacing change
            v
   Layouts::rescaleManual(rects, refCanvas, newCanvas, spacing)   [pure]
            |
            v
   place windows -> constrainWindows()      (reference untouched: no feedback loop)

   user finishes a move/resize -> snapshot ALL normal windows at the current canvas
```

The invariant the drift bugs violated, stated once: **the reference set is written only by a
user gesture (or a project load), never by a rescale.**

## Hotpath & threading impact

- **Touches the hotpath?** No. Manual rescale runs on canvas resize, project load, gesture
  commit and spacing edits — never per frame, never on the parse path. `--benchmark-hotpath`
  unaffected.
- **New cross-thread signal/slot?** No. GUI thread throughout (`WindowManager`, QML).
- **New input to a cached hotpath flag?** No.
- **Timestamp ownership** — untouched.

## Data model & persistence

No format change. `serializeLayout` keeps writing absolute rectangles plus the canvas size it
wrote them at; the seam structure is re-derived from those rectangles on load, which is exactly
what R12 asks for. This works *because* the saved geometry stops being drift-contaminated —
today's saved rectangles are the welded output of a previous rescale. Layouts saved by current
builds load fine: they are re-clustered like any other, and whatever seams they express become
the model.

## QML / UI

The auto-layout button opens `LayoutPatternPicker` as a popup anchored under it: pattern tiles
with the active one checked, a **Manual** tile in the same grid, and the ratio row shown only for
patterns with a primary region. Picking a pattern selects auto mode and applies it; picking
Manual switches to manual. The button keeps its highlight-when-auto colouring so mode is still
readable at a glance. The existing `dashboard.toggleAutoLayout` command binding and the
`DashboardLayout` shortcut path are untouched, so the one-keystroke toggle survives for anyone
using it. Freeze gating on the button is unchanged.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Rescale model | seam-grid / fraction-anchored edges stored per window / patch the existing weld | **Seam-grid** — gap constancy becomes structural instead of a repair; the fraction variant needs new persistence keys and a migration, the weld patch fails once `spacing * scale` outgrows any fixed tolerance |
| Module home | extend `UI::Layouts` / new `UI::LayoutRescale` / keep it private in `WindowManager` | **Extend `UI::Layouts`** — same responsibility (arranging widgets), reuses the existing ctest target; private-to-WindowManager would fail AC9/AC10's "no GUI required" |
| Seam detection input | reference geometry / live geometry | **Reference** — deriving from output the rescale just produced is precisely the feedback loop that compounds drift today |
| Commit scope | snapshot every window / snapshot only the edited one | **Every window** — the reference canvas size is shared, so a partial snapshot leaves the rest interpreted against the wrong size |
| Old weld path | delete / keep as a post-pass | **Delete** — an idempotent transform followed by a non-idempotent repair is the current bug with extra steps |
| Button behaviour | popup gallery / toggle + chevron / toggle + right-click | **Popup gallery** (maintainer decision, 2026-08-14) — one control, discoverable; the toggle survives via the existing command binding |

## Risks & mitigations

- **Bespoke overlapping layouts.** The seam model assumes edges cluster meaningfully. Windows
  that overlap or float freely still map through it (each edge simply gets its own seam), but
  their relative overlap now scales in gap-free space. Covered by an explicit overlap case in
  the rescale suite.
- **Tolerance collisions.** Two intentionally-distinct edges within 6 px merge into one seam.
  This is today's behavior, and normalizing the spacing away *before* clustering makes it
  strictly less likely than now. Tolerance stays at 6.
- **Existing saved layouts carry accumulated drift.** They load as-is; the first user gesture
  re-snapshots them cleanly. Called out for the maintainer rather than auto-corrected — silently
  rewriting someone's saved geometry is not ours to do.
- **Manual mode's minimum size (48 px) versus the degrade path** — degrade reduces spacing first
  and only then hits `constrainWindows`, so a small canvas loses gaps before it loses widgets.
- **Popup replacing a one-click toggle** — the command binding and shortcut keep the fast path;
  worth a maintainer sanity check that the popup does not feel heavy for a mode flip.

## Test & verification plan

- **Unit (ctest, pure):** `tst_layout_patterns.cpp`
  - AC9: 2x2 grid, master+stack, a floating window and an overlapping pair, each rescaled across
    several canvas sizes and non-uniform aspect changes, for spacing `-1, 0, 4, 16` — assert
    every shared edge measures exactly the spacing and every outer edge is flush.
  - AC10: from a fixed reference, rescale to B and back to A ten times — byte-identical to the
    start. Then the re-derived path (output fed back in, which is what a save or a commit taken
    at B produces): assert it settles within 2 px on the first hop and moves no further across
    50 cycles, joins and flushness exact throughout. Measured in the scratchpad prototype
    before the port: 0-2 px, stable from the first cycle, against today's weld walking
    `spacing/2` on every single resize.
  - AC11 (math half): rescale A→B, replace one rectangle, rescale B→A — assert the untouched
    rectangles return within the AC10 bound with their joins intact.
  - Degrade: canvas too small for `k` gaps yields no overlap and nothing under the floor.
- **Maintainer visual:** drag the dashboard window across sizes with a manual layout and confirm
  gaps and the canvas inset never change (AC9); move one widget after a resize and confirm the
  others hold (AC11, glue half); the gallery popup opens from the auto-layout button, marks the
  active choice, and Manual works (AC12).
- **Hotpath:** not touched; no benchmark run required beyond the standard pre-commit gate.
- **Static:** `code-verify.py --check`, `qt-cpp-review` on the C++ diff, `sanitize-commit.py`.
