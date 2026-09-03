---
spec: 0075-review-remediation
package: WP-E (dashboard, widgets, QML)
tasks: WPE-T1 .. WPE-T11
status: complete (two named tests deferred, see below)
---

# Handoff — WP-E

All eleven tasks are ticked in `tasks.md`. `code-verify.py --check` is clean over the whole diff
(two pre-existing `cxx-tu-too-long` advisories on `Dashboard.cpp` and `Terminal.cpp`, both smaller
or unchanged versus master); `--tu-census --check`, `--singleton-census --check`,
`claim-verify.py` and `registry-verify.py` all pass with no new findings.

## Binding invariants named before the edits

- Dashboard ingest is GUI-thread hotpath: per-block work stays O(pixels + fftSize + datasets)
  except the deliberate per-sample line/sweep feeds, which are additionally bounded by the ring
  capacity.
- Push tables hold indexes and value pointers resolved at configure time, never ring pointers,
  and share the `m_layoutValid` staleness contract; `clearPushTables()` drops them on reset.
- `m_plotClocks` and `m_plotDisplayTimeSec` are ONE state, cleared only through
  `resetPlotClocks()`, which stayed in the facade.
- A uniform-grid block continues from the previous block's span (`advancePlotClock` with
  `blockSpanSec`), never from the smoothed cadence; time rings stay rate-sized.
- Render-thread work lives only in `updatePaintNode`; rasterization stays in `updatePolish`.
- `ThemeManager` constructs pre-root, so nothing new is reached from its constructor.
- `Dataset::value` strings propagate only to `string_targets`.
- WPE-T1 is a pure move: no behaviour change, benchmark checkpoint.

## Files changed

### New

| File | Role |
|------|------|
| `app/src/UI/Dashboard/DashboardIngest.{h,cpp}` | The dashboard's block-ingest sub-object: `applyBlock*`, `feed*`, `advancePlotClock`, `foldExtremes`, `update*Series`, and every push table (build + clear). Binds Dashboard state by reference (`UI::IngestBindings`) and calls the facade back through `UI::IngestHost`. |
| `app/src/UI/Widgets/Waterfall/WaterfallColorMap.{h,cpp}` | The eight color maps plus the 256-entry LUT bake, lifted out of `Waterfall.cpp` so the spectrogram indexes a table and the maps are unit-testable. |
| `app/src/UI/Widgets/Waterfall/WaterfallTiles.{h,cpp}` | Pure decomposition of the visible spectrogram span into per-band quads (ring seam + band boundaries). |
| `app/tests/tst_dashboard_ingest.cpp` | Uniform-grid lane, ring-capacity bound, GPS per block, plot-clock continuation, stale-generation drop, column mismatch. |
| `app/tests/tst_colormap_lut.cpp` | LUT is a bake of the continuous map, ends are the extremes, out-of-range clamps. |
| `app/tests/tst_waterfall_tiles.cpp` | Quads tile the plot exactly across the seam and the band boundaries. |
| `app/tests/tst_terminal_selection.cpp` | Selection clamp, front erase reporting dropped rows, colour rows trimmed in lockstep. |
| `app/tests/tst_theme_property_map.cpp` | Shipped key set, no-op republish notifies nothing, per-key notify, dropped key empties. |
| `tests/integration/test_dashboard_lanes.py` | Samples-axis plot fed through `dashboard.tailFrames`, survives a point-count change; audio-gated stream-lane case. |

### Modified

| File | Change |
|------|--------|
| `app/src/UI/Dashboard.{h,cpp}` | Ingest extracted (2724 -> 1856 lines); `Dashboard` privately implements `IngestHost`; `rebuildLineSeriesPreservingState()` shared by `setPoints`/`setPlotTimeRange` (F4); `rebuildPushTables()` after a post-layout title edit (F10); dead static `FrameBuilder::instance()` re-resolve dropped (F11). |
| `app/src/UI/Widgets/Waterfall.{h,cpp}` | 256-entry colour LUT; spectrogram drawn as per-band textured quads with per-band dirty flags; overlay re-rasterized only when a visible marker readout moved (F7); spurious `historySizeChanged` on an FFT-size change removed (F15). |
| `app/src/UI/Widgets/Waterfall/WaterfallOverlay.{h,cpp}` | `updateMarkerStates` returns whether the drawn readout changed. |
| `app/src/UI/Widgets/Terminal.{h,cpp}` | `clear()` drops the selection; `copy()` clamps rows and columns; both ANSI erase overrides drain buffer events and clamp the selection; per-paint segment/run text reuses two scratch buffers (F2, F18). |
| `app/src/UI/Widgets/Terminal/TerminalBuffer.{h,cpp}` | `clampPoint()` (static, testable); `eraseRowsBefore` routes through `trimFront` so it reports dropped rows; `eraseRowsAfter` trims colour rows regardless of the ANSI flag and pulls the cursor back (F2, F17). |
| `app/src/UI/Widgets/Terminal/AnsiStateMachine.cpp` | CSI parameter list capped at 32 (F17). |
| `app/src/UI/Widgets/ExtensionData.{h,cpp}` | Per-tick pass updates volatile fields only through `buildVolatileRow` (no widget-map walk, no `QVariantList` per row); rows rebuilt on `widgetCountChanged` (F6). |
| `app/src/UI/Widgets/FFTPlot.{h,cpp}` | `markerValuesChanged` fires only when a marker's state or its one-decimal peak moved (F15). |
| `app/src/UI/Widgets/MultiPlot.{h,cpp}` | `m_drawOrders` removed (never read) (F15). |
| `app/src/UI/WindowManager.cpp`, `WindowManager/WindowGeometry.{h,cpp}` | Resize applies the clamped rectangle; the clamp is now `WindowGeometry::clampResizeToCanvas` (F19). |
| `app/src/Misc/ThemeManager.{h,cpp}` | `colors` is a `QQmlPropertyMap*`; `Misc::syncColorMap()` (header-inline) republishes per key (G2). |
| `app/qml/Widgets/Dashboard/ConsoleAnnotations.qml` | Lanes reassigned only when window, geometry or class set changed (G1). |
| `app/qml/ProjectEditor/Views/TableDelegate.qml` | Separator canvas repaints on `themeChanged` (G3). |
| `app/qml/Widgets/Dashboard/ValueFormat.js`, `Compass.qml` | One `formatValue` round-trip per sample; the range ends are memoized / constant (G10). |
| `app/src/Licensing/MonotonicClock.cpp` | The floor is persisted at most once a minute; in between the cached floor still catches a rewind (K10). |
| `app/src/Licensing/Trial.{h,cpp}` | `daysRemaining()` cached against the current date and invalidated on every expiry move; `trialEnabled()` reads it (K10). |
| `app/CMakeLists.txt`, `app/tests/CMakeLists.txt` | New sources and five new suites, appended contiguously. |
| `app/tests/tst_window_geometry.cpp` | Two `clampResizeToCanvas` cases. |

## Tests added

`tst_dashboard_ingest`, `tst_colormap_lut`, `tst_waterfall_tiles`, `tst_terminal_selection`,
`tst_theme_property_map` (all registered in `app/tests/CMakeLists.txt`), two cases appended to
`tst_window_geometry`, and `tests/integration/test_dashboard_lanes.py`.

I could not run ctest (no build dir, and building is the maintainer's). Every suite was written
against the headers it links and uses only the accessors those headers declare.

## Not done, and why

1. **`tst_extension_data_rows` (WPE-T7).** `ExtensionRowsModel` lives in `ExtensionData.cpp`
   beside a `QQuickItem` bound to `UI::Dashboard`, so the unit tier cannot link it without the
   whole application. Landing this test needs `ExtensionRowsModel` split into its own file pair
   first — which is also the one-class-one-file-pair fix, and WP-I owns that area (F14). The F6
   defect itself is fixed and is observable through the row-model's own `updateRow` contract.
2. **Shared `datasetWidgets()` helper (WPE-T7).** `DataGrid::datasetWidgets` and
   `ExtensionData::datasetWidgets` are still twins. Deduplicating them needs a new shared TU that
   reaches the dashboard widget map; F14 (`datasetWidgets` + `formatValue` duplication) is listed
   under WP-I, and doing it here would put two packages in the same new file. The per-tick cost
   that made it a defect is gone: only a rebuild calls it now.
3. **`Dashboard.cpp` is 1856 lines**, still over the 1500 cap. WPE-T1 only moved the ingest;
   R12.8's remaining Dashboard split is WP-I's `WPI-T5`/`WPI-T6` work. The aggregate TU census
   *shrank* (3968 -> 3150 excess lines), so no gate regresses.

## Patches for the coordinator

### 1. Re-seed the two census baselines (both shrank, neither blocks CI)

```
python3 scripts/code-verify.py --tu-census --accept
python3 scripts/code-verify.py --singleton-census --accept
```

### 2. `scripts/code-verify.py` — hotpath assert whitelist (WP0 owns the file)

`DashboardIngest.cpp` is now a hotpath TU but cannot use `SS_ASSERT_HOTPATH`. I used
`SS_ASSERT(block != nullptr, return)` at the one site instead (block rate, not per sample, and
strictly safer than the compiled-out check it replaced). If WP0 wants the macro available there:

```diff
--- a/scripts/code-verify.py
+++ b/scripts/code-verify.py
@@ _HOTPATH_ASSERT_ALLOWED = (
     "app/src/UI/Dashboard.h",
     "app/src/UI/Dashboard.cpp",
+    "app/src/UI/Dashboard/DashboardIngest.h",
+    "app/src/UI/Dashboard/DashboardIngest.cpp",
 )
```

### 3. WP-F handoff hunk

`UI/Dashboard/DashboardTools.cpp` was not touched, as instructed. WP-F's
`configureActions`-on-`actionsChanged` hunk applies cleanly on top of this branch.

### 4. WP-H note for `tst_monotonic_clock`

`MonotonicClock::nowFloored(settings, crypt)` is unchanged and still writes on every call — the
throttle lives in `now()`. A write-count case must therefore drive `now()` (two calls inside one
minute must leave `licensing/lastSeen` untouched), not `nowFloored`.

### 5. WP-J doc updates this diff earns

- `dashboard.md` "Dashboard Ingest — Pre-resolved Push Tables": the tables and the ingest path now
  live in `UI::DashboardIngest`; `StreamTargets` gained `yLinePushIndexes`, `xLinePushIndexes` and
  `multiSampleIndexes`, and the uniform-grid lane feeds the sample rings and GPS.
- `dashboard.md` line 52 ("`applyBlock` is O(pixels + fftSize + datasets) per block, never
  O(samples)") is now doubly wrong and should say what the per-sample feeds are and how they are
  bounded (F8 already flagged the first half).
- `dashboard.md` "Waterfall / Spectrogram": the history image is uploaded as 64-row texture bands.
- `code-style.md` QML section: the canvas theme-repaint hook (`Connections { target:
  Cpp_ThemeManager; function onThemeChanged() }`) and the fact that `Cpp_ThemeManager.colors` is a
  `QQmlPropertyMap` (bracket syntax unchanged, per-key notify) — G11/R13.3.

## Invariants I found that the plan did not state

- **`Dashboard::refreshDisplayTitles()` mutates the widget buckets after the push tables point
  into them.** `WidgetMapBuilder::rebuildDatasetReferences()`'s own doxygen says every such
  mutation must re-resolve, and that path did not. An implicitly-shared bucket detaching on the
  title write would move every `Dataset` the value, FFT, GPS, 3D, line and multiplot tables
  address — not only `m_datasetReferences`. `rebuildPushTables()` re-resolves all of them.
- **`applyBlockColumn`'s `m_datasetReferences` write was redundant with `applyBlockValues`.**
  Both wrote `isNumeric`/`numericValue` into the same target set (`ValuePush::targets` is built
  from `m_datasetReferences`), the first per column with the last sample, the second per block.
  Removing the column-side copy is what makes the F9 string fix possible without losing a write.
- **GPS is a per-block consumer, not a per-column one.** Feeding it from `applyBlockColumn` would
  have tripled its fix rate (lat, lon and alt are three columns of one block), so it advances once
  per block after `applyBlockValues`, exactly where `updatePlot3DSeries` already did.
- **A band can appear at most twice in one frame.** The two physical runs the ring seam produces
  are disjoint, so one alias node per band is provably enough for the second quad.
- **Waterfall marker chips render the peak to one decimal**, which is why "the overlay changed"
  is `state != previous || round(peak * 10) != previous`, not a raw float compare — a raw compare
  would have re-rasterized on every tick anyway and the fix would have been a no-op.

## Counterfactual self-check

**Which rule does this diff most risk violating?** "No allocation on the dashboard path", via
WPE-T2: `feedSampleRings` adds a per-sample loop to `applyBlockColumn`, which the dashboard doc
describes as never O(samples).

**Evidence that it does not:** the loop performs stores into a pre-sized `DSP::FixedQueue` and
allocates nothing; it is bounded twice — by the block's sample cap (`kStreamBlockSampleCap`, 4096)
and by `push_ring_tail`'s ring-capacity clamp, so a 4096-sample block into a 1000-point plot costs
1000 stores, not 4096. The lane it joins already walks every sample twice (`appendDecimated` into
the time ring, `sweep.advance` per sample), so the per-block shape is unchanged; the targets are
resolved once per layout into index vectors, so there is no per-sample lookup. The counterweight
in the same task removes work from the same function: the per-column `QString::number` heap
allocation is gone (F9), and `applyBlockColumn` no longer walks `m_datasetReferences` at all.

**Second-closest risk:** the `m_plotClocks` / `m_plotDisplayTimeSec` one-state rule, since
`advancePlotClock` moved into the sub-object while `resetPlotClocks()` stayed in the facade. The
sub-object binds both by reference and writes only through `advancePlotClock`; the only clear
path is still `Dashboard::resetPlotClocks()`, and `reconfigureDashboard` still saves and restores
the pair together (`grep m_plotDisplayTimeSec` shows three sites, all in the facade, all paired
with `m_plotClocks`).

**Third:** `--benchmark-hotpath` was not run (the maintainer builds). WPE-T1 is a mechanical move
with one behavioural delta (a release-evaluated null check per block), so
`HOTPATH_DASHBOARD_INGEST_COST` should be flat; WPE-T2 and T3 move in opposite directions and
should be compared against the T1 checkpoint, as the plan's risk table asks.
