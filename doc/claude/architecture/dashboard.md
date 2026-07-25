# Architecture — Dashboard, Widgets & Plot Internals

> Part of the architecture corpus ([index](../architecture.md)). Read this file in full
> before touching `UI::Dashboard`, any widget, the plot/time-ring/sweep/waterfall render
> paths, alarms, workspaces, or output widgets. Dashboard ingest is hotpath — read
> [dataflow.md](dataflow.md) too, and let the `ss-hotpath` skill fire.

## Dashboard Ingest — Pre-resolved Push Tables

`Dashboard::hotpathRxFrame` does no per-frame container lookups; everything is resolved at
reconfigure and the per-frame walk is pointer-only.

- **Value propagation** (`m_valuePushes`, built by `buildValuePushes` per source in row-major
  group/dataset order from `m_datasetReferences`): `updateDashboardData` walks it positionally
  and validates each entry's `uniqueId` against the incoming dataset (mismatch or unmapped UID →
  `handleMissingDataset`, the same reconfigure-and-retry-once semantics the old per-dataset
  `QHash::find` provided).
- **String values are written only where observable.** Numeric datasets copy `Dataset::value`
  (a QString COW bump per target) only into `stringTargets`: DataGrid-group copies and the
  `m_lastFrame` copies (`dashboard.getData` serializes that frame, incl. `Keys::Value`).
  Non-numeric datasets write the string to every target. **A new widget that displays
  `Dataset::value` must be registered in `buildValuePushes`' `string_targets` set** or its
  tiles silently read stale strings.
- **FFT / waterfall / GPS / 3D mirror the line-plot push tables** (`m_fftPushes`,
  `m_waterfallPushes`, `m_gpsPushes`, `m_plot3DPushes`): raw `sourceId` / value / buffer
  pointers resolved in the matching `configure*` (second pass, after the buffers stop growing),
  dropped by `clearPushTables()` on reset, and sharing the `m_layoutValid` staleness contract
  with `LinePush`. GPS keeps the per-axis `isNumeric` gate via pointer (`GpsPush::Field`).
- **3D plots ingest into `DSP::FixedQueue<QVector3D>` rings** (`m_plot3DRings`, O(1)
  overwrite — the old `erase(begin())` was an O(points) memmove per frame); `plotData3D()`
  materializes the ordered snapshot (`m_plotData3D`, mutable) at read/render cadence. A live
  `points()` change is absorbed by an `[[unlikely]]` `ring->resize()` in `updatePlot3DSeries`.
- **Benchmark**: `runAndReport` adds a same-project isolation pass — `lua+dashboard(off)` runs
  the all-widget project with `dashboardIngest=false` (Dashboard early-returns) and prints
  `dashboard ingest costs N.NNx` / `HOTPATH_DASHBOARD_INGEST_COST`. Optimize against that
  number; the historical `dashboard costs N.NNx` line compares two different projects.

## Alarm Bands — Central Tracking in `UI::AlarmMonitor`

Alarm-band *notifications* are dataset-level, not widget-level. `UI::AlarmMonitor` (singleton,
wired in `ModuleManager::setupCrossModuleConnections`) rebuilds per-dataset trackers from
`Dashboard::datasets()` on `widgetCountChanged` / `dataReset` and evaluates them on `updated()`
(UI rate, not hotpath). Trackers resolve datasets by `uniqueId` on every pass — never cache
`Dataset*` across signals; `resetData(true)` emits `updated()` *before* `widgetCountChanged`,
so cached pointers would dangle. Consequences:

- Notifications fire even when the dataset's widget is hidden, popped out, or `hideOnDashboard`.
- `Bar` / `Gauge` / `Meter` / `LEDPanel` are display-only band consumers; do not re-add
  per-widget `NotificationCenter` posts (that double-fires when a dataset is both a band
  widget and `led: true`).
- The value is clamped to the dataset's widget range before band lookup (mirrors analog-widget
  semantics); 3 s per-dataset, per-severity-tier cooldown.
- `AlarmBand.blink` (`Keys::Blink`, JSON `blink`, default false) is rendering-only: LED panels
  flash while the band is active. LED datasets with no bands synthesize a runtime
  `[ledHigh, +inf)` band inside `LEDPanel` (severity -1 = dataset color); nothing is migrated
  in the project file — the editor only pre-fills a band from `ledHigh` when the dialog opens.

## Dashboard Tools — External Windows Only

The four tools (terminal/Console, notification log [Pro], clock, stopwatch) are **never canvas
widgets**. `reconfigureDashboard` registers them in the widget map unconditionally (predicate:
`SerialStudio::isDashboardTool`); `Taskbar::rebuildModel` skips them, so they never appear in
workspaces, search, or saved canvas layouts. The `Dashboard::*Enabled` flags are pure
view-state: setters persist to QSettings and emit only their own changed signal — **toggling a
tool must not emit `widgetCountChanged` or touch the widget map** (that re-introduces the
full dashboard rebuild this design removed). `DashboardCanvas.qml::syncToolWindows` maps each
flag to an `ExternalWidgetWindow`; a user closing the window flips the flag back, so
enabled == window visible. Tool windows are excluded from the per-project `externalWindows`
widgetSettings entry (their flags already persist globally).

## Plot X-Axis (Time / Samples / Dataset) & the TimeRing

`Dataset::xAxisId` selects the plot X source, and
there are three **live** modes: `kXAxisTime (-2)` the **default**, `kXAxisSamples (-1)`, or a
dataset `uniqueId (>=0)` (`Frame.h`). All three modes are **free** for every build. A dataset-X
plot resolves whenever its `xAxisId` names a live dataset (the `xAxisId >= 0 &&
datasets.contains(...)` check in `Plot.cpp` / `Dashboard::registerXAxisIfNeeded`); an unresolved
id **falls through to Samples** via the shared carrier in `configureLineSeries`. Samples is live: a shared monotonic index ring
(`m_pltXAxis`, `fillRange`) + the per-dataset y ring, rendered via `downsampleMonotonic`.
Deserialize **preserves `-1` verbatim** (Frame.cpp:301, `ss_jsr(obj, Keys::XAxis, kXAxisTime)`);
`migrateLegacyXAxisIds` (ProjectModelLoading.cpp:158) keeps Time and Samples untouched, remaps
legacy positive frame-indices to dataset uniqueIds, and maps any other `<= 0` / unresolvable id
to Time. **Selector reality**: the per-dataset X combo lists Time | Samples | every dataset
(`ProjectModel::xDataSources`); the multiplot **group** combo is Time | Samples only and fans the
chosen value into every member dataset's `xAxisId` (`ProjectEditor` `kGroupView_xAxis`), read back
canonically from `datasets.front()` (`useTimeXAxisGroup`), a known encoding wart slated for a
group-level `xAxisId` field. **Carrier invariant**: `m_pltValues` holds one `DSP::LineSeries` per
plot widget **including time plots** (index-aligned with the widget list; `m_pltValues.size() !=
plotCount` is the reconfigure trigger, Dashboard.cpp:2008, asserted at :2151); a time plot's
carrier is effectively a placeholder, since its curve renders from the `TimeRing`, not the
carrier's y ring. **Time plots do NOT use the raw sample ring.** They use a per-curve
`DSP::TimeRing` (`DSP.h`): a bounded `(time, value)` ring that **decimates on ingest** to a
**min/max envelope pair** per `interval = 2 * windowSec / capacity` second cell (two slots
reserved per cell so a saturated source still spans the window). Cell boundaries sit on an
**absolute time grid** and `appendDecimated` (`DSP.h`) maintains the open cell's slots in
place, so both envelope edges survive, slot contents are independent of sampling phase (no
beat aliasing / shimmer -- the old drifting single peak-pick had both), and the newest
sample is visible immediately at any input rate. Capacity is sized in `Dashboard.cpp` by
`timeRingCapacity(plotTimeRangeSec)`: `min(plotTimeRange * kAssumedMaxRateHz, kMaxTimeRingSamples)`
with a floor of `kDefaultPlotBuckets` (`50000` Hz assumption, `262144` cap, `1024` floor). Storage
is `m_plotTimeRings` / `m_multiplotTimeRings` (keyed by widget index; the multiplot one is a
`std::vector<TimeRing>` per curve). The hotpath appends `numericValue` at `m_plotDisplayTimeSec`
via `m_timePushes` (single plots) and `m_multiplotPushes` with its `TimeCurve` list (multi). The
widget side calls `Dashboard::plotTimeRing(idx)` / `multiplotTimeRings(idx)` and renders through
`DSP::downsampleTimeWindow(ring.time, ring.value, ...)`: a viewport decimation of the
already-decimated ring whose pixel columns are bucketed on an **absolute column-width lattice**
(anchor quantized to the column width, drawing still uses true newest-rebased positions), so
per-column sample membership stays stable as the window slides -- a newest-anchored bucket grid
re-grouped every render and shimmered like heat haze. This is why 10 s of 48 kHz audio works:
the ring caps at `kMaxTimeRingSamples` and `appendDecimated` collapses bursts into bounded
envelope slots, bounded memory/CPU, axis fixed at `[-T, 0]` (never recompute the axis from raw
extremes). **Display
clock** (`m_plotDisplayTimeSec`, `hotpathRxFrame`): sources without a cadence stamp many frames
at one coarse wall-clock tick (~15 ms on Windows), which would compress them onto a single
decimator interval and lose temporal spread; the display clock spreads same-timestamp frames
by a smoothed per-sample period so sub-tick windows still render. It is self-correcting
(n samples over a gap fill it exactly) and display-only: `m_relativeFrameTimeSec` and exported
timestamps stay raw. Fine-timestamp sources (audio) hit the n==1 path and are unchanged. Ticks
render the **magnitude** in an adaptive unit (`PlotWidget.qml` `timeAxis` + `secondsAgoFormat`
+ `timeUnitFactor`/`timeUnitName`): the title and ticks switch between `s` / `ms` / `us` from
the span, so e.g. a 10 ms window reads `Time (ms)` with `10 8 6 4 2 0`. Dataset-X plots, Samples
plots, FFT, GPS, 3D keep the raw-ring + downsample path.

## Downsampler Cost Model

All three downsamplers in `DSP.h` (`downsampleMonotonic`,
`downsampleTimeWindow`, `downsampleWindowAbsolute`) are single-pass — the visible span resolves
via `dsLowerBound`/`dsUpperBound` binary searches (monotonic X/time is a hard precondition,
including for `downsampleMonotonic`'s endpoint-derived X bounds), the bucket accumulation is the
only walk over the samples, and the Y bounds come from the filled columns (`dsColumnYBounds`,
O(columns)). **Visible-window push**: `PlotCommon.setDownsampleFactor` differentiates — time-axis
plots get `dataW = plotArea.width` (no zoom multiplier) plus `model.setVisibleXWindow(xVisibleMin,
xVisibleMax)` (re-pushed from `onXVisibleMinChanged`, so pan updates it too), and the models
intersect it with the full range (`clampToVisibleX`) before downsampling — zooming in *narrows*
the binary-searched sample scan instead of re-bucketing the full range at zoom resolution.
Non-time plots (FFT, dataset-X, samples-axis) keep `dataW = width * zoom`. Draw cadence is
`TimerEvents::uiTimeout` — 60 Hz default, user-configurable 1-240 (`uiRefreshRate` setting), so
per-draw costs scale with that, not a fixed rate.

## Log-Frequency FFT Rendering (specs 0016/0018) & Display Ballistics (spec 0017)

The log frequency axis (`fftLogX`) uses the **studio-analyzer recipe** (spec 0018, which
superseded 0016's multi-resolution cascade the same day — uniform latency beat extra
low-band resolution): one FFT at the configured window for the whole spectrum, and the
sparse low decades rendered smooth by `FFTPlot::buildLogRenderCurve` — a Fritsch-Carlson
monotone cubic (PCHIP) through the bins in log-x space, resampled onto a uniform
`kLogRenderPoints` (2048) log grid. Monotone interpolation never overshoots, so peaks
stay honest; bin 0 clamps onto bin 1's log position (DC has none) and the axis starts at
the first bin. `rebuildLogBinTable` caches the per-bin log-x table + buffers at ctor and
plan rebuild; per tick the pipeline is `computeBinSpectrum` (dB + 3-bin boxcar +
optional ballistics per bin) then `emitLinearSpectrum` or `buildLogRenderCurve`.
`configureFftSeries` clamps `fftSamples` to `[1, kMaxFftRingSamples]` — untrusted
project input; an unclamped negative reaches the ring allocator as a wrapped size_t
(the same latent hole still exists on the Waterfall ring, flagged, out of lane).
Optional per-dataset **display ballistics** (`fftBallistics`/`fftBallisticsRelease`,
spec 0017, off by default): instant attack, wall-clock exponential release (default
300 ms) applied per FFT bin in `computeBinSpectrum`, upstream of both emit paths —
display-only, allocation-free, analysis untouched.

## FFT Frequency Markers (spec 0019)

Per-dataset spectral watchlist: `Dataset::fftMarkers` (`Keys::FFTMarkers`, additive — array
omitted when empty) holds `FrequencyMarker` entries (`freq`, optional `endFreq` band, label,
color, optional `warningDb`/`alarmDb` display-dB thresholds; NaN = unset, never serialized).
Edited via `FrequencyMarkersEditor.qml` (AlarmBandsEditor grammar: launcher signal →
lazy Loader in DatasetView, commit via `ProjectEditor::commitFrequencyMarkers`); API =
`project.dataset.get/setFFTMarkers` + the `fftMarkers` update key. **Monitoring is
widget-local and display-dB** (post-ballistics `m_binDb` in `FFTPlot::updateMarkerValues`,
`m_smoothed` row in `Waterfall::updateMarkerStates`) — WYSIWYG by design, no AlarmMonitor
integration, nothing runs when the widget is hidden. FFTPlot resolves per-marker bin windows
(point = +/- 2 bins) at ctor and in `rebuildFftPlan` (bin width follows FFT size — a missed
re-resolve silently mis-aims markers), evaluates peaks per tick allocation-free, and exposes
config via the `markers` QVariantList plus live values via `markerPeakDb(i)`/`markerState(i)`
polled on `markerValuesChanged` (no per-tick containers). QML renders bands/lines in
`plot.curveLayer` at `z: -1` (under the curve stroke); label chips live in a separate layer
parented directly to the PlotWidget over the plot area — ABOVE its internal mouse overlay,
because chips are clickable (click = transient spotlight via `root.selectedMarker`, dims the
other markers; chip MouseArea passes wheel through so zoom still works). Both layers map Hz
through the same `xVisibleMin/xVisibleRange` transform as PlotCurve (`log10` world coords on
the log axis). Waterfall paints markers per-paint after the cached axis layer (escalation
tint changes per row — do NOT move them into `m_axisLayer`); its Hz→x mapping is
single-sourced in `visibleFreqWindow()`, shared with `drawXAxis`, the hover cursor, and the
markers — and it works in **world units**: linear Hz, or log10-Hz when the dataset's
`fftLogX` is on (the Waterfall honors it since 2026-07-18). Log mode resamples each spectrum
row onto a log-spaced column grid via a LUT rebuilt in `allocateFftPlan`
(`rebuildLogColumnTable`; domain = first bin → Nyquist, the FFTPlot convention; degenerate
sizes fall back to linear via `m_logActive`); zoom/pan transfer unchanged because
`computeSourceRect` and `visibleFreqWindow` are proportionally identical mappings. Marker
*monitoring* stays in linear bin space — the display axis must never change what is
measured. Waterfall chips are click-to-spotlight like the FFT's: `drawMarkerChip` captures
per-paint hit rects (`m_chipHitRects`, mutable), `mousePressEvent` hit-tests them BEFORE
starting drag-to-pan, hover shows a pointing-hand over chips. Toolbar toggles persist as
widgetSettings `showFrequencyMarkers` on both widgets.

## GPU Curve Rendering (`Widgets::PlotCurve`)

Plot, FFT, and MultiPlot curves render through a
custom scene-graph item (independent per-segment quads, 8 verts + 18 indices per visible segment,
each extruded along its own perpendicular — shared join cross-sections collapse to hairlines on
near-reversals — with a 1 px feather band straddling the stroke edge for AA without MSAA) instead
of QtGraphs `LineSeries` — the
QtGraphs `PointRenderer` strokes through `QQuickShape`/`QPainterPath`, re-triangulating on the
CPU every update, which stalled on audio-rate curves. The `LineSeries` objects remain as pure
**data carriers** (the models still `draw()` into them; `PlotCurve.source` follows the series'
`update()` signal) but are **never added to the graph**; only the `ScatterSeries` stay in the
`GraphsView` (interpolation None + axis anchoring). `PlotCurve` items live in
`PlotWidget.curveLayer` (a clipped item tracking the plot area, above `PlotAreaFill`, below the
crosshair overlay) and map world coordinates with the same visible-window transform as the
cursors. Offscreen stretches are culled by per-segment X-interval overlap, so zoomed series cost
the visible slice; NaNs break the ribbon into runs (true gaps). MultiPlot instantiates one
`PlotCurve` per curve with an inline carrier (`source: LineSeries {}`), and its `onUiTimeout`
loop draws the carriers from the `_curves` Instantiator (graph `seriesList` now only holds
scatter).

## Plot Time Range

`Dashboard::plotTimeRange` (seconds, default 10, **1 ms min**) is the ring
window `T`; `setPlotTimeRange` rebuilds each `TimeRing` at the new capacity (configurePlot /
configureMultiPlot in `Dashboard.cpp`). **Per-project, mirroring `pointCount`**: in ProjectFile
it lives in the `.ssproj` (`ProjectModel::plotTimeRange` / `Keys::PlotTimeRange`, edited in the
project overview); elsewhere it's QSettings `Dashboard/PlotTimeRange` (edited in Settings).
Dashboard syncs `m_plotTimeRange` from the project on `operationModeChanged` and persists to
QSettings only outside ProjectFile. Both UI controls are an oscilloscope-style **editable**
SpinBox snapping typed input to a 1 ms..300 s ladder. **API**: `dashboard.setTimeRange{seconds}` /
`dashboard.getTimeRange` (alias `project.dashboard.setTimeRange`); the old `setPoints`/`getPoints`
commands were removed with the rename. The legacy `points` (`kDefaultPlotPoints = 1000`) still
sizes the raw rings for dataset-X / FFT / GPS / 3D; the "Points" controls were removed from the UI.

## Waterfall Follows the Time Range (Pro)

`syncHistoryToTimeRange` sets `m_historySize =
round(plotTimeRange * fps)` (clamped 16..4096) on `plotTimeRangeChanged` / `fpsChanged` and at
construction, so its time axis (`historySize / fps`) reads the Time Range. fps is the row cadence
(one row per dashboard `updated` tick), not the sample rate; sub-second ranges clamp to 16 rows.

## AxisRangeDialog

Hides its X section for time plots (`timeAxis` from the widget model); the manual
X min/max is meaningless when X is the Time Range. Y range stays editable.

## Area-Under-Plot Fill (`Widgets::PlotAreaFill`)

Driven via `PlotWidget.qml`'s `areaFillSource` /
`areaFillBaseline` / `areaFillColor`: the curve is rasterized into per-pixel-column min/max
envelopes (one O(points) pass; segments bridge every column they cross, clipped to the visible
window with a `kMaxBridgedColumns` budget for non-monotonic curves), then emitted as one
degenerate-stitched GPU triangle strip with a peak quad above and a valley quad below the baseline
per column. Geometry is O(item width), independent of point density — a zoomed audio-rate series
costs the same as a sparse one — and columns are watertight (the old per-point strip self-crossed
into bowtie quads at every baseline crossing, washing out dense bipolar fills). Per-vertex alpha:
0.12 at the baseline, 0.50 at the data's per-sign extreme (gradient anchors to the data, not the
axis range); the fill color is a saturation-deepened (`1-(1-s)^2`, hue-preserving) variant of the
curve color so pastel themes stay vivid. Overlaid on the GraphsView plot area, tracking
`xVisibleMin`/`yVisibleMin` under zoom/pan. It replaced the QtGraphs `AreaSeries` (whose per-tick
CPU shape re-triangulation stalled audio-rate curves) and the bipolar `drawClamped` split series.
Baseline rules: Plot = 0 when bipolar, `maxY` when all-negative (inverted mountain), else `minY`;
FFT always uses `minY` (floor). NaN samples break the column run and leave a real gap. The fill
renders above the curve stroke and below the crosshair overlay; it follows the curve series'
`update()` signal, so paused plots freeze it for free.

## Plot Sweep / Trigger Mode (Pro)

Oscilloscope sweep for **time-axis** Plot/MultiPlot. `DSP::SweepEngine`
(`DSP.h`) owns a front/back decimating `TimeRing` per curve; `advance(now, trigValue)` runs on the hotpath
(alloc-free), detects a level+edge crossing (interpolated `t0`), honors holdoff + Auto/Normal/Single, and
swaps `back`->`front` when `sweepTime > activeWindow()`. The capture width is `activeWindow()` =
`timebaseSec` when set (0 < it < `windowSec`) else the full `windowSec`. Completion re-arms and falls
through in the same `advance` call so the next trigger starts immediately, refreshing continuously rather
than stalling a full window; in Auto, the free-run timeout is `activeWindow()` (not `windowSec`). Each sweep
is phase-locked to its interpolated `t0`, so successive completed sweeps overlay as a stationary trace.
`display(curve)` is threshold-gated on `kLiveWindowSec` (0.1s): short windows return the completed `front`
(frozen, phase-locked overlay), but windows wider than the threshold return the live `back` while `sweeping`
so long ranges grow left-to-right in real time instead of stalling a multi-second hold; before the first
completion it always returns `back`. The Dashboard holds `m_plotSweep`/`m_multiplotSweep` (keyed by widget index),
fed from `TimePush::sweep`/`MultiPush::sweep` in `updateLineSeries`/`updateDataSeries` via the
`feedSweep`/`feedMultiSweep` lambdas; engines are created in `configureLineSeries`/`configureMultiLineSeries`
for time plots and the config (including `timebaseSec`) survives a Time-Range rebuild via
`restorePlotSweepConfig`/`restoreMultiplotSweepConfig`. When enabled the widget axis is `[0, activeWindow]`
(vs rolling `[-T, 0]`) and `updateData` renders the held sweep through `DSP::downsampleWindowAbsolute`
(no newest-rebase). Config lives per-widget in `widgetSettings`
(`sweepEnabled`/`sweepMode`/`triggerEdge`/`triggerLevel`/`holdoff`/`sweepTimebase`(+`triggerSource` for
MultiPlot); `sweepTimebase` is ms, 0 = match time range). QML wiring is a Pro-gated toolbar toggle +
`TriggerDialog.qml` (with the optional "Timebase (ms)" field), and the trigger-level line drawn in
`PlotWidget.qml` (`sweepMode`/`triggerLevel`). Setters are runtime-gated on a valid commercial
token (`isValid()` + `SS_LICENSE_GUARD()`; tier compares were removed 2026-07, trial = Pro).
`SweepMode`/`TriggerEdge` enums live in `SerialStudio.h`.

## Output Widgets (Pro)

`app/src/UI/Widgets/Output/`, QML in `app/qml/Widgets/Dashboard/Output/`:
Button/Toggle/Slider/TextField/Panel sharing `Base`. User JS converts UI state → device
bytes (`app/rcc/scripts/output/*.js`); `OutputCodeEditor` edits; `TransmitTestDialog`
previews. Protocol helpers (CRC, NMEA, Modbus, SLCAN, GRBL, GCode, SCPI, binary packet)
injected into the engine. Gated on a valid commercial token (`isValid()` +
`SS_LICENSE_GUARD()`; the 2026-07 tier removal made trial = Pro, enum now
`None=0, Trial=2, Pro=3, Enterprise=4`, display-only).

## Dashboard Freeze Mode (Pro) — spec 0007

One stored flag, one derived flag, one input gate — keep the three roles separate:

- **Stored**: `ProjectModel::frozen` (`Keys::Frozen`, project JSON root, absent = false).
  `setFrozen` is license-gated **only in the enable direction** (unfreeze must always work);
  the loader (`loadFrozen`) and `serializeToJson` bypass the gate on purpose so an unlicensed
  load/save cycle never strips the flag. `newJsonFile` resets it.
- **Effective**: read-only `UI::Dashboard::frozen` = `ProjectModel::frozen() &&
  SerialStudio::activated()`, notify wired to `frozenChanged` + `activatedChanged` (covers
  online/offline/trial — late activation re-derives without reload). Computed getter by
  design: binding-time reads only, never on the frame path, no cached flag to invalidate.
  QML consumes only this property.
- **Input gate**: `WindowManager::frozen` (plain bool, bound from `Cpp_UI_Dashboard.frozen`
  in DashboardCanvas). Early-outs in `startManualPress` (first statement — closes caption
  drag, body drag, edge resize from both event entry points), `childMouseEventFilter`,
  `mousePressEvent` (both branches), `mouseDoubleClickEvent`, `updateHoverCursor`.
  `setFrozen(true)` aborts an in-flight drag/resize without committing geometry.

Chrome hides via `WidgetDelegate.frozen` (`headerVisible`/`shadowEnabled`) and the
`hasToolbar` mirror; non-chrome escape hatches are gated at `DashboardLayout`
(close/minimize/toggleAutoLayout — the shortcut path opens the license dialog when the
setter refuses an enable) and in `Taskbar.qml` (entry-click restore, right-click
remove-from-workspace, auto-layout button). Freeze is orthogonal to `taskbarHidden` and
deliberately mode-agnostic (persists across operation-mode switches within a session).

## Manual Layout Mode — Smart Guides & the 48x48 Floor (spec 0010)

Manual-mode (auto-layout off) drag/resize snapping lives in `UI::Snap`
(`app/src/UI/SnapGuides.h/.cpp`): a pure, stateless resolver (`resolveMoveSnap` /
`resolveResizeSnap`) that `WindowManager` feeds per mouse move with the candidate rect, the
sibling rects cached at gesture start (`cacheSnapSiblings`), and the grid settings. Rules the
resolver encodes: nearest candidate within 6 px per axis, edges beat centers beat spacing on
ties, any smart candidate suppresses grid quantization on that axis, all candidates stay
canvas-bounded, and resize picks never move a non-moving edge. Alt (sampled from the move
event's modifiers) bypasses snapping; the geometry badge still tracks. Visuals publish through
`WindowManager` notify properties (`alignmentGuides`, `spacingIndicators`, `sizeMatchRect`,
`manualGestureActive`/`manualGestureGeometry`) rendered by `DashboardCanvas.qml`, and are
cleared on release, `setFrozen(true)`, and `clear()` — the same abort points the freeze input
gate uses. The old half/quarter-canvas Aero snap is **auto-mode only** (drag-to-swap
indicator); manual mode is pure freeform + guides. Grid prefs are QSettings
(`WindowManager_GridEnabled`/`WindowManager_GridSize`), toggled from the canvas context menu.
Minimum window size is mode-dependent via `WidgetDelegate.minimumWidth/Height`
(48x48 manual, 356x320 auto) which feed `implicitWidth/Height` — the floor
`computeResizedGeometry` and `constrainWindows` (48 fallback in manual, 100x80 in auto) read.
`MiniWindow` collapses caption chrome progressively below 200 px (external button, title,
minimize, maximize; close never hides) and its `externControlWidth`/`windowControlsWidth`
count only visible controls, which the C++ caption hit-test depends on.

## Widget Toolbars — `WidgetToolbar.qml` Owns the Policy

Every canvas widget toolbar lives in `app/qml/Widgets/Dashboard/WidgetToolbar.qml`: a 48 px
band hosting buttons in a horizontal `Flickable` that **scrolls when too narrow instead of
hiding** (edge fades signal overflow; `interactive` only on overflow). Visibility policy:
`shown = available && !frozen && parent.height >= minWidgetHeight` — width never hides it.
Widgets declare buttons as children, set `windowRoot` (frozen reads
`windowRoot.frozen === true`, so external pop-outs — where `frozen` is undefined — are
unaffected) and expose `readonly property bool hasToolbar: toolbar.shown` for the
delegate/band mirror. Layout-agnostic: consumers anchor it (or use Layout props — ImageView).
Do NOT reintroduce per-widget `width >= toolbar.implicitWidth` hiding or imperative
`hasToolbar` assignments — the scroll policy is what removed that binding-loop hazard.
Terminal keeps its own toolbar (dashboard tool, external-window only — freeze never
reaches it).

## Workspaces (`UI::Taskbar`)

`app/qml/MainWindow/Taskbar/`: user-defined dashboard tabs.
Persisted under `"workspaces"`. **Workspace IDs ≥ 1000**, group IDs < 1000.
`Taskbar::deleteWorkspace(id)` branches on the threshold — don't cross-wire. Edits stage
in memory + `setModified(true)`; no autosave.

## Waterfall / Spectrogram (Pro)

`UI/Widgets/Waterfall.h/.cpp`: per-dataset Pro widget
reusing the dataset's FFT settings. Class IS the painted item (`QQuickPaintedItem`).
Toggle via `DatasetWaterfall = 0b01000000`; persists as `Keys::Waterfall` (omit when false).
`Keys::WaterfallYAxis` non-zero → **Campbell mode**: rows placed by another dataset's
value (e.g. RPM) instead of time. `commercialCfg()` flags any project using waterfall.
