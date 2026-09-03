# Architecture — Dashboard, Widgets & Plot Internals

> Part of the architecture corpus ([index](../architecture.md)). Read this file in full
> before touching `UI::Dashboard`, any widget, the plot/time-ring/sweep/waterfall render
> paths, alarms, workspaces, or output widgets. Dashboard ingest is hotpath — read
> [dataflow.md](dataflow.md) too, and let the `ss-hotpath` skill fire.

## Dashboard Ingest — Pre-resolved Push Tables

**The ingest lives in `UI::DashboardIngest`** (`app/src/UI/Dashboard/DashboardIngest.{h,cpp}`,
spec 0075 F1), a member sub-object of the `UI::Dashboard` facade. It owns `applyBlock`, the
`applyBlockValues` / `applyBlockColumn` split, the `feed*` helpers, `advancePlotClock`,
`foldExtremes`, the `update*Series` family and **every push table**. Two seams hold it to the
facade:

- **`UI::IngestBindings`** binds facade state by reference (the widget map, the axis and series
  containers, the time rings, the sweep engines, the dataset and extreme tables, the push-table
  vectors, `m_layoutValid`, `m_updateRequired`, `m_plotClocks`, `m_plotDisplayTimeSec`, ...).
  Every entry stays owned by `UI::Dashboard`: the push tables hold raw pointers **into** these
  containers, so handing the ingest its own copies would move what they address.
- **`UI::IngestHost`** is the callback interface the facade implements: the series allocators the
  ingest cannot own because they size the buffers the push tables then point at
  (`configureLineSeries`, `configureMultiLineSeries`, `configureFftSeries`, `configureGpsSeries`,
  and the commercial 3D/waterfall pair), `handleMissingDataset()` for the layout-repair hand-off,
  and three const queries.

**`resetPlotClocks()` stayed in the facade on purpose.** `m_plotClocks` and
`m_plotDisplayTimeSec` are ONE state; the ingest binds both by reference and writes them only
through `advancePlotClock`, so the single clear path is still `Dashboard::resetPlotClocks()`.
Note also that the `PlotClock&` `advancePlotClock` resolves must not outlive the call —
`reconfigureDashboard` move-assigns `m_plotClocks`.

`Dashboard::hotpathRxFrame` does no per-frame container lookups; everything is resolved at
reconfigure and the per-frame walk is pointer-only.

- **Value propagation** (`m_valuePushes`, built by `buildValuePushes` per source in row-major
  group/dataset order from `m_datasetReferences`): `applyBlockValues` walks it positionally
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

## Display Tick — Frame Drain + Stream Ingest (spec 0051)

`Dashboard::onDisplayTick` (the `uiTimeout` slot) is now the single entry point for data on
the GUI thread, and it runs in this order:

1. **Block ring drain** — pop every finished `DataModel::DataBlockPtr` the pipeline thread
   queued (`PipelineHost::dequeueDashboardBlock`) and run `hotpathRxFrame` on each. The publisher no
   longer calls the Dashboard directly; nothing about the push tables or `structureGeneration`
   revalidation changed, only who calls them and when.
2. **Stream worker drain** — for each `IO::StreamWorker`, publish the current display budget
   into its atomics (bucket count = `points()/2`, window = `plotTimeRange`) and apply every
   pending `DataBlock` through `applyBlock` (spec 0055: one ring, one drain, both lanes).
3. **One coalesced `updated()`** if anything set `m_updateRequired`.

**`applyBlock` splits into per-block and per-sample work, and the per-sample half is bounded
twice.** Per **block**: `applyBlockValues` writes the widget dataset copies once, from the
block's last sample (writing them per column was redundant — `ValuePush::targets` is built from
`m_datasetReferences`, so both writes hit the same set); GPS, 3D and waterfall series advance
once after the column loop (GPS is three columns of one block, so feeding it per column tripled
its fix rate); and one `advancePlotClock`. Per **sample**, inside `applyBlockColumn`: the extreme
fold, `appendDecimated` into each plot and multiplot time ring, `sweep.advance()` for a stream-fed
sweep, the FFT/waterfall `push()`, and `feedSampleRings` for the Samples-axis, dataset-X and
Samples-mode multiplot lanes. That loop allocates nothing (plain stores into pre-sized
`DSP::FixedQueue`s) and is bounded by the block's own cap (`kStreamBlockSampleCap`, 4096) and
again by `push_ring_tail`'s ring-capacity clamp, which pushes only the newest `capacity` samples
— so a 4096-sample block into a 1000-point plot costs 1000 stores, not 4096. Never add a rate cap
or a per-view reduction on top; the reduction is `appendDecimated`'s and the ring's.

Both lanes share `advancePlotClock(sourceId, t0, blockSpanSec)` — the per-source clock is advanced
from the block timestamp and **never cleared** (the `bulkLoadPlotWindow` clear/re-anchor
semantics are deliberately not reused). Stream widget targets resolve through a lazy
`uniqueId → StreamTargets` cache holding **indexes only** (a layout rebuild reallocates the
ring nodes, so cached pointers would dangle): `plotIndexes`, `multiplotCurves`, `fftIndexes`,
`yLinePushIndexes`, `xLinePushIndexes`, `multiSampleIndexes` and the commercial
`waterfallIndexes`. It is cleared with the push tables in `clearPushTables()`.

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
- **A band consumer reports nothing until its first finite sample (spec 0075 N3).**
  `Widgets::Bar` (which `Gauge` and `Meter` derive from) latches `hasData` on the first finite
  value through `latchData()` and clears it only on `Dashboard::dataReset`, which also drops the
  extreme hold so a reconnect cannot keep the previous session's min/max. While unlatched
  `activeBandSeverity()` is **-1** and `activeBandLabel()` is **empty**, both through the shared
  pure gate `Widgets::Bands::reportedSeverity(bands, activeIndex, hasData)` in `UI/WidgetBands.h`
  — the nearest-band clamp above it is right for overrange data and wrong for the placeholder
  0.0 a widget shows before its first byte, which used to alarm forever on any project whose
  bands sit above zero. QML gates on the pair: both infinite blink animations and the digital
  box's `targetColor` read `alarmTriggered && hasData` in `Bar.qml`, `Gauge.qml` and `Meter.qml`.
  `latchData()` returns whether the latch just closed, because `updateData` early-returns unless
  the value *changed* and a first sample numerically equal to 0.0 would otherwise never publish
  the transition. `UI::AlarmMonitor` is unchanged and stays dataset-level.
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

**Frame annotation layer (spec 0059).** `Console::Handler` owns a `Console::AnnotationModel`
(bounded annotations over absolute byte offsets, interned texts, decoder-declared rows/classes,
a 1 MiB retained byte copy for payload extraction; also the table model), an
`AnnotationDecoder` (user JS `decoder = { rows, classes, decode(bytes, offset, ctx) }` in its own
`QJSEngine` under a 200 ms `JsWatchdog`, chunk cadence on the GUI thread, bounded carry-over,
disabled on throw/timeout) and an `AnnotationFilter` proxy; `hotpathRxData` /
`hotpathRxDeviceData` feed the decoder. `ConsoleAnnotations.qml` (ribbon toggle in
`Terminal.qml`, Utilities lane) shows the track strip, the filterable table with CSV export, the
per-class payload view and the decoder editor. Nothing on the frame pipeline; JS only.

Load-bearing rules, all earned against a 48 kHz audio stream (2026-08-17):

- **`annotate()` stages, `commitPending()` publishes.** `annotate()` pushes into a pending vector
  and emits nothing; `Console::Handler`'s `uiTimeout` lambda calls `commitPending()`, which does
  ONE `beginInsertRows` and at most one `countChanged` per tick. A decoder emitting thousands of
  records per second used to open one model transaction each, through a
  `QSortFilterProxyModel`. `ingestBytes()` only marks the count dirty for the same reason.
  Anything that reads `count()` right after `annotate()` (tests, headless callers) must commit
  first.
- **The layer is gated on a view being on screen.** `AnnotationDecoder::setViewerActive(viewer,
  on)` tracks panels by identity (a `destroyed()` guard each, so a console window torn down with
  its panel open cannot wedge the gate); `feed()` returns before `ingestBytes()` when the set is
  empty, so a closed panel costs the console stream neither the 1 MiB-window copy nor a `decode()`
  call per chunk. `ConsoleAnnotations.qml` registers on `visible`, which follows both the ribbon
  toggle and the Console pane being the shown view. The gate is orthogonal to `enabled`: armed but
  unwatched is paused. Resuming clears the carry -- its bytes predate the gap, and splicing them
  onto what arrives after hands `decode()` a frame that was never on the wire. Two `Terminal.qml`
  instances can exist (Console pane + the dashboard console tool window), which is why this is a
  viewer set and not a bool.
- **Panel UI state lives in `Settings { category: "ConsoleAnnotations" }`**, not in the project:
  `saveWidgetSetting` is a no-op outside Project File mode.
- **The store is a `std::deque`.** Trimming (`dropOldest`, `trimToRetainedBytes`) costs the
  dropped records, not a memmove of the survivors.
- **`AnnotationDecoder::reset()` empties the model BEFORE re-reading the offset.** Reading first
  left the decoder annotating at the old stream position while the byte window restarted at zero:
  every later record landed past the window, escaped trimming (its `end >= m_bytesStart` always
  held) and never reached the strip — a full store with empty lanes.
- **The strip anchors on `labelledEnd`, never `retainedEnd`.** Bytes are counted on arrival but
  records only land on the tick, so a narrow window anchored on the byte counter sits past the
  newest bar and reads as empty. Anchoring on the labels also freezes the strip while paused.
- **The strip is painted, not instantiated.** `trackStrip()` returns flat pixel geometry
  (`x, width, start, end, class, merged` per mark) and the lane is a `Canvas`; texts ride along
  only under `kLabelledSpanBudget` (256) marks. One scene-graph item per mark, rebuilt per tick,
  is what made a decimated lane crawl. `trackSpans()` keeps the readable map form.
- **Decimation merges to one pixel, never to one blob.** `collectRuns()` merges neighbours of one
  class only when the record, the gap before it, AND the resulting cluster all fit inside
  `minSpanBytes`; merging on the gap alone collapsed a lane of legible records separated by one
  delimiter byte.
- **Persistence is two-sided.** `widgetSettings("console")` carries the decoder with the `.ssproj`
  but is a **no-op outside `ProjectFile` mode** (`ProjectModel.cpp:1203`), so `Settings { category:
  "ConsoleAnnotations" }` also stores decoder code/enabled plus window bytes, current tab and the
  payload hex toggle. Project copy wins when present; the app copy is the Quick Plot / Console
  Only fallback. Superseded once the decoder becomes a `Source` field.

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
`std::vector<EnvelopeRing>` per curve). **Since spec 0057 the history ring is a
`DSP::EnvelopeRing`**: `level0` is the `TimeRing` just described, unchanged, and `levels[k-1]`
is a bounded `FixedQueue<EnvelopeCell>` of time-ordered `{t0, v0, t1, v1}` extreme pairs, each
cell covering `16^k` level-0 grid cells (identity = level-0 cell index `>> 4k`, exact integer
nesting). A completed level-0 cell folds into every coarser open cell (`foldOpenCell`, at most
nine merges, never a rescan); coarse levels are sized `ceil(cells0 / 16^k) + 1` while at least
three cells remain, so they cost 16/15 of level 0's bytes and never allocate after construction
(level 0's `resizeCapacity` rebuilds them from its retained slots). Sweep engines keep plain
`TimeRing`s. The hotpath appends `numericValue` at `m_plotDisplayTimeSec`
via `m_timePushes` (single plots) and `m_multiplotPushes` with its `TimeCurve` list (multi). The
widget side calls `Dashboard::plotTimeRing(idx)` / `multiplotTimeRings(idx)` and renders through
`DSP::downsampleTimeWindow(ring, ...)`, which asks `EnvelopeRing::selectLevel(span, pixels,
oldest)` for the coarsest level whose cell span is at or under one render column *and* whose
oldest cell still reaches the window (else it falls to a finer level, level 0 always qualifying),
then feeds the shared `dsTimeWindowCore` with either level 0's slots or the coarse level's
`2 * cells` points, always rebased to level 0's newest sample. Wide windows therefore read
O(pixels) cells instead of O(samples); narrow windows read level 0 exactly as before. The API's
`dashboard.tailFrames` reads `level0`. `tst_envelope_ring` pins the per-level brute-force
contract. The plain-ring overload is a viewport decimation of the
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
(n samples over a gap fill it exactly) and display-only: the plot clock's `relativeFrameTimeSec` and exported
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
`configureFftSeries` normalizes `fftSamples` through `Widgets::normalizedFftSize()`
(`UI/Widgets/FFTWindow.h`) — untrusted project input; an unclamped negative would reach the ring
allocator as a wrapped `size_t`. `configureWaterfallSeries` runs the same function under the
waterfall's own lower ceiling, `kMaxWaterfallFftSize` (65536), so both rings are clamped from one
shared transform-size contract.
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
tint changes per row — do NOT move them into the overlay's cached `m_layer`); its Hz→x mapping is
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
cursors.

**Geometry buffers are grow-only with a padded degenerate tail (spec 0075 N4).** `QSGGeometry`
carries ONE pair of counts over one buffer and no separate capacity, so any count change re-lays
it out, hundreds of KB per curve per frame once the per-segment join fans move the count by a
few. `Widgets::GpuStroke::reserveGeometry(geometry, vertices, indices)` therefore reallocates
only when what is held is too small, at 1.5x headroom (`kGeometryHeadroomNumerator` /
`kGeometryHeadroomDenominator`, capped at `kMaxGeometry`, index capacity rounded up to whole
triangles), and returns **whether it reallocated**. `padGeometryTail()` then makes the unused
tail harmless: every spare vertex repeats the last real vertex and every spare index is 0, so the
extra triangles are degenerate and draw nothing. The padding must be real coordinates, never left
uninitialised, because the batch renderer computes bounds over the *whole* vertex buffer and
garbage there corrupts batching and clipping even though no index references it. `PlotCurve.cpp`
and `PlotAreaFill.cpp` both call the pair in place of an exact-fit `allocate()`. The trade is
real and named: roughly 1.5x more uploaded bytes per frame in exchange for zero per-frame
malloc/free. `tst_plot_curve_geometry` pins that `reserveGeometry` returns false in the steady
state across 100 frames of both a stationary and a wobbling count. Offscreen stretches are culled by per-segment X-interval overlap, so zoomed series cost
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
`dashboard.getTimeRange` (alias `project.dashboard.setTimeRange`); the old
<!-- claim-verify off -->
`dashboard.setPoints`/`dashboard.getPoints`
commands were removed with the rename.
<!-- claim-verify on --> The point count itself did NOT go away: `Dashboard::points` is still a
writable `Q_PROPERTY`, the control-script SDK still exposes `setPlotPoints`
(`DataModel::DashboardBridge`, plus the Lua global, both over one `coreSetPlotPoints`), and the
legacy `points` (`kDefaultPlotPoints = 1000`) still sizes the raw rings for dataset-X / FFT /
GPS / 3D. Only the "Points" controls were removed from the UI. A point-count change goes through
`rebuildLineSeriesPreservingState()`, which snapshots and restores what a bare reconfigure would
drop: the retained time rings, the sweep configuration with its captured segments, and each
widget's run/pause flag (F4). `setPlotTimeRange` shares it.

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
CPU shape re-triangulation stalled audio-rate curves) and
<!-- claim-verify off -->
the bipolar `drawClamped` split series.
<!-- claim-verify on -->
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
`SweepMode`/`TriggerEdge` enums live in `SerialStudio.h`. **Retained segments (spec 0061):** the
engine keeps a bounded ring of `SweepSegment`s (deep copies of `front` taken in `completeSweep()`
via `retainFront()`, pre-sized by `setSegmentRetention(n)`, n clamped to 64 and to 32 MB per plot);
`segment(0)` is the newest, `resetState()`/`clearSegments()` drop them, `takeSegmentsFrom()` carries
them across rebuilds. `Widgets::Plot` exposes retention only (`sweepRetention` + the read-only
`sweepSegmentCount`/`sweepSegmentCapacity`) and draws every retained segment age-dimmed under the live
trace (`drawSegment`, `PlotCurve` repeater in `Plot.qml`); the toolbar control is the retention pill,
and `sweepRetention` persists per widget. The stepper, the pinned reference and the overlay toggle were
removed 2026-08-17 as UI nobody could read; `MultiPlot` has no segment surface at all (it never rendered
one). Session-DB persistence of segments is not implemented yet.

**Stream-lane sources feed the same engines by a second path (spec 0051 M4).** Audio and any other
`isStreamCapable()` source never reaches `updateLineSeries`/`updateDataSeries`, where the frame lane
advances the sweep inline once per display tick at `m_plotDisplayTimeSec`; until this was wired the
trigger was simply dead for those sources while the plain time rings kept updating, so the plot looked
alive. `applyBlockColumn` now calls `feedPlotBlockSweep` per active plot the column targets, and
`applyBlock` calls `feedMultiplotBlockSweep` per enabled, active multiplot after the column loop (a
multiplot needs one sweep time from its trigger curve applied to every curve, which a per-channel hook
cannot produce). Both drive `advance()` **once per sample**, at `baseSec + i * dt` off the block's
uniform grid, so **trigger resolution is the source's, not the display's** — the reduction happens
afterwards, when the sweep time that `advance()` returns is written through `appendDecimated`. Curves
pair by sample index because one source's columns share a block grid. A multiplot no column of the
block feeds resolves to a null trigger and is skipped, which is what keeps frame-fed multiplots out of
the stream path; a multiplot mixing both lanes would be advanced by both clocks and is not supported.

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
minimize, maximize; close never hides) and its `menuControlWidth`/`windowControlsWidth`
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

## Widget Extensions (spec 0038) — `UI::WidgetExtensions`

Installable dashboard widgets: `info.json` + one QML file, validated eagerly, compiled lazily.
Catalog = `UI::WidgetExtensions` (`app/src/UI/WidgetExtensions.{h,cpp}` + `WidgetExtensionManifest.cpp`),
built after ProjectModel and before Dashboard; its ctor is a leaf (member init only), and the
first `rescan()` + the `ExtensionManager`/`WorkspaceManager` edges live in
`setupCrossModuleConnections()`. `rescan()` reads `:/extensions/widget/*` first, then
`<workspace>/Extensions/widget/*` (a disk id may not shadow a bundled one), and touches no
`QQmlComponent` — compilation happens in `DashboardWidget::createExtensionItem()` the first time
a project places the widget. Schema of record: `app/rcc/extensions/schema/widget-manifest.json`,
gated by `registry-verify.py` + `tests/scripts/test_widget_manifests.py`.

**Two identity mechanisms, and they are not interchangeable:**

- *Third-party packages* resolve to `SerialStudio::DashboardExtension = 100` — explicitly valued,
  after the `#ifdef BUILD_COMMERCIAL` block, so no existing ordinal moves in either build and the
  `QMap`-keyed widget buckets always iterate extension widgets last. One enum value serves both
  scopes, so `isGroupWidget`/`isDatasetWidget` stay enum-pure (both false) and
  `Dashboard::widgetSlot(type, relativeIndex)` is the single group-vs-dataset discriminator;
  group-scope slots occupy `[0, groupCount)` of the shared bucket and dataset-scope slots follow
  (`datasetBucketBase`). Persisted keys (workspaces, freeze title mode, display titles, per-widget
  settings) substitute `"ext:<id>"` for the numeric type token via
  `WidgetExtensions::persistedTypeToken`; on the ProjectModel side the single formatter is the
  file-local `extension_scope_key()` used by `freezeTitleMode`, `setFreezeTitleMode`,
  `promptRenameWidget`, `widgetDisplayTitle`, and `setWidgetDisplayTitle`.
- *Bundled conversions* (`compass`, `datagrid`) declare `"replaces": "<builtin string>"` and keep
  their existing enum value, ordering, and project files. `builtinReplacement()` wins in
  `DashboardWidget::setWidgetIndex`, so only the implementation moves. `builtinWidgetId()` maps
  free, non-tool widget strings only — a bundled package can never become a Pro widget.

**Rules that bite:**

- `readsStringValues: true` is what puts a package's `ExtensionData` into `buildValuePushes`'
  `string_targets` (`addExtensionStringTargets`) — the declarable form of the stale-string gotcha
  above. Reconfigure-time only; the per-frame walk is untouched and a project with no package
  builds bit-identical push tables.
- Reserved ids are R10's mechanism: `WidgetExtensions::reservedIds()`, the schema's `reservedId`
  enum, and every widget string `SerialStudio::getDashboardWidget*` resolves must agree —
  `registry-verify.py` fails when they drift. No catalog data can select a Pro enumerator.
- The dataset widget picker is generated (`app/rcc/properties/dataset.json` → `DatasetForm.cpp`).
  Packages reach it through the `extensibleMap` option source: fixed built-in rows first, then
  `PropertyHooks::widgetExtensionOptions()` appended, so stored combo indices never move.
- **Trust model: no sandbox, and nothing may claim otherwise.** Package QML shares the app's QML
  engine and privileges. `canInstantiate()` is default-deny (`qmlUrl()` returns empty without
  consent, recorded per id *and version*); the `Cpp_*` shadowing in `createExtensionItem()` is a
  speed bump, exempted for bundled packages so the two conversions stay verbatim copies.
  `UI::WidgetExtensions::hostContextNames()` is no longer a hand-kept mirror: it returns
  `Misc::ContextRegistry::objectNames()`, the same table the composition root registers through.
  `Misc::ContextRegistry` (`app/src/Misc/ContextRegistry.{h,cpp}`, spec 0075 G4) is the
  collect-then-apply helper `ModuleManager` fills with `registry.add(name, object)` and flushes
  once with `registry.apply(ctx)` — twice per session, once for the common globals and once for
  the commercial ones. `registry-verify.py` compares that table against the `registry.add` call
  sites in both directions and asserts the forwarding, so a name can no longer drift out of one
  side.
- Every load-time rejection is a `Misc::ProblemCenter` finding through the `extension.widget`
  checker (`widget-manifest-invalid`, `widget-id-reserved`, `widget-replaces-forbidden`,
  `widget-api-version`, `widget-host-incompatible`, `widget-qml-missing`,
  `widget-dependency-missing`, `widget-not-installed`, `widget-consent-required`,
  `widget-load-failed`); the canvas shows `ExtensionPlaceholder.qml`, never an empty slot.

## Workspaces (`UI::Taskbar`)

`app/qml/MainWindow/Panes/Dashboard/Taskbar.qml`: user-defined dashboard tabs.
Persisted under `"workspaces"`. **Workspace IDs ≥ 1000**, group IDs < 1000.
`Taskbar::deleteWorkspace(id)` branches on the threshold — don't cross-wire. Edits stage
in memory + `setModified(true)`; no autosave.

## Waterfall / Spectrogram (Pro)

`UI/Widgets/Waterfall.h/.cpp`: per-dataset Pro widget
reusing the dataset's FFT settings. **`Waterfall` is a `QQuickItem` with `updatePaintNode`**, not
a `QQuickPaintedItem`. Toggle via `DatasetWaterfall = 0b01000000`; persists as `Keys::Waterfall`
(omit when false). `Keys::WaterfallYAxis` non-zero → **Campbell mode**: rows placed by another
dataset's value (e.g. RPM) instead of time. `commercialCfg()` flags any project using waterfall.

The widget composes four sub-objects under `UI/Widgets/Waterfall/`: `WaterfallColorMap` (the
eight maps plus a 256-entry LUT bake, so the spectrogram indexes a table instead of evaluating a
map per pixel), `WaterfallOverlay` (the cached raster of border, axes, markers and hover cursor,
re-rendered only when a drawn readout actually moved), `WaterfallTiles` (the pure decomposition
of the visible span into per-band quads across the ring seam) and `WaterfallSpectrogramNodes`
(the scene-graph half: dirty-row/dirty-band bookkeeping and both draw paths).

**Two draw paths, one preferred and one fallback.**

- **Ring texture (preferred).** `WaterfallRingTexture` is a `QSGTexture` over one persistent
  `QRhiTexture` sized once for the widget's life. At **sync** — inside `updatePaintNode`, GUI
  thread blocked — changed scanlines are staged: `stageRow()` memcpy's one scanline into a
  preallocated `QByteArray` slot (`kStagedRowSlots`, 8; one row is the steady state and the spare
  slots absorb a Campbell burst or a missed tick), and past the slots it escalates to a full
  `stageImage()` rather than growing. At **prepare**, on the render thread,
  `commitTextureOperations(QRhi*, QRhiResourceUpdateBatch*)` uploads either one full image or one
  sub-rect per staged row. **That method runs with the GUI thread already released**, unlike
  `updatePaintNode`, which is the whole reason the staging buffers exist: the texture may not
  read the widget's live `QImage` at upload time, only memory it owns. The scroll stays a
  source-rect offset; `m_topRow`/`m_writeRow`/`m_filledOnce` remain one ring state and no pixel
  is ever moved. `WaterfallTiles::decompose` is reused with `tileRows == imageHeight`, so both
  paths share the seam math and `tst_waterfall_tiles` still pins it.
- **64-row tiles (fallback).** `WaterfallRingTexture::supported()` returns false on a big-endian
  target (`QImage::Format_RGB32` is `0xffRRGGBB`, whose little-endian byte order is exactly
  BGRA8, and no per-tick format conversion exists on either path), when `QQuickWindow::rhi()` is
  null (the software renderer), when the format is unsupported, or when the size exceeds
  `QRhi::TextureSizeMax`. A texture that fails to create latches `m_ringUnavailable` for the
  widget's life. `WaterfallSpectrogramNodes::sync()` then draws the history as 64-row textured
  bands with per-band dirty flags. This path is fully correct, just more upload bytes.

**Two behaviours that surprise people.** The **idle gate**:
`WaterfallRingTexture::captureRowIfChanged()` memcmp's the smoothed row against the previous
tick's, and an identical row writes no scanline and schedules no frame — so a disconnected or
silent source sits still, and a *perfectly* constant live signal also stops scrolling (visually
indistinguishable once the history is uniform, distinguishable during the first fill).
**Campbell mode is exempt**, because its row position is driven by another dataset's value and
moves independently of the spectrum. The **hidden release**: `itemChange(ItemVisibleHasChanged,
false)` calls `releaseHistoryImage()` and requests a GPU teardown, so **a hidden waterfall loses
its history** — becoming visible again rebuilds the image at the floor color and it refills from
live data. Switching workspace tabs is where a user sees it. That is what "a hidden widget
releases its image and textures" costs.

## Time-Ring Sizing & the Plot Clocks — Non-Negotiable

**Time rings are sized from a rate, never from a sample count alone**
(`kMaxRateSizedRingSamples` is the shared ceiling): the stream lane sizes at build from the
source's real rate (`streamRingCapacity`), the frame lane cannot know its rate then, so a
*saturated* ring re-sizes once from the plot clock's smoothed period (`growTimeRing`, upward
only). A ring bounded in samples alone runs out of history in seconds (44.1 kHz filled a 10 s
axis to 5.9 s, 2026-08-15).

**A time ring's clock never rewinds, and the clocks never outlive their display time.**
`appendDecimated` clamps sub-cell backward jitter forward to keep the grid monotonic, but a jump
back over a whole cell drops the retained span: clamping it instead wedges the ring shut (no new
cell can open) until wall time climbs past the stale stamp, and the plot draws a single point at
the right edge meanwhile. Producer side: `m_plotClocks` and `m_plotDisplayTimeSec` are ONE state,
cleared, saved and restored together via `Dashboard::resetPlotClocks()`, never one without the
other. Clearing the map alone left QuickPlot audio blank for seconds after each rebuild
(2026-08-18).

**A rebuild never seeds a time ring.** `updateDataSeries()` called with no source refills the
sample-count series only. `m_plotDisplayTimeSec` is a single global owned by whichever source
published last, so seeding every ring from it rewinds every other source's ring; the rings a
rebuild keeps come from `restorePlotTimeRings()`, and the next real block appends on the source's
own clock.

**A uniform-grid block continues from the previous block's span, never from the smoothed cadence.**
`applyBlockColumn()` writes a block out to `base + (samples - 1) * dt`, so the next block's base
must clear that. Passing `blockSpanSec` to `advancePlotClock()` makes the continuation term
`previous base + previous samples * dt` and takes `qMax` with the driver's own `t0`: both advance
at the sample rate, so the clock can neither rewind nor ratchet. The EMA `samplePeriodSec` is still
maintained (`growTimeRing` sizes frame-lane rings from it) but must not drive the block lane — it
averages over callback sizes, so it undershoots a long block and overshoots a short one. Both
failure modes shipped on 2026-08-18: the undershoot wiped 48 kHz audio roughly once a second, and
"fixing" it by folding the span into `displayTimeSec` double-counted it and ran the axis at 2x with
a one-block gap between every block.
