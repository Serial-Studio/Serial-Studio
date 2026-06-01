# Architecture — Critical Invariants

> Extracted from CLAUDE.md. Read the relevant subsection in full before touching that
> subsystem. The most dangerous rules (threading, hotpath connections, no-alloc) are
> summarized inline in CLAUDE.md under "Threading & Hotpath — Non-Negotiable".

## Data Flow

```
Driver  (driver thread OR main, depending on driver)
  │ HAL_Driver::dataReceived(CapturedDataPtr)           AutoConnection
  ▼
FrameReader::processData  (main thread)
  │ appends to CircularBuffer (SPSC); tracks per-chunk timestamps;
  │ KMP / multi-pattern delimiter scan; enqueues completed frames
  │ to lock-free ReaderWriterQueue<CapturedDataPtr>; emits readyRead once
  ▼
DeviceManager::onReadyRead  (main, DirectConnection)
  │ while try_dequeue: Q_EMIT frameReady(deviceId, frame)
  ▼
ConnectionManager::onFrameReady  (main)
  │ routes to FrameBuilder::hotpathRxFrame / hotpathRxSourceFrame
  ▼
FrameBuilder  (main)
  │ parse → apply per-dataset transforms → mutate m_frame / m_sourceFrames
  │ Dashboard gets the pooled TimestampedFramePtr (acquireFrame slot, fast recycle);
  │ async sinks get one detached make_shared copy (their backlog can't pin the pool)
  ▼
Dashboard (pooled)   |   CSV / MDF4 / API / gRPC / Sessions / MQTT (detached copy)
```

## Timestamp Ownership — Source Owns Time

Timing is stamped at the driver boundary and preserved downstream. Do not re-stamp in
export or report workers.

- `IO::CapturedData` (`HAL_Driver.h`): `data` (`QByteArray`, inline COW — no second
  `shared_ptr` indirection), `timestamp` (steady_clock), `frameStep` (ns cadence),
  `logicalFramesHint`. `CapturedDataPtr` is the hotpath transport.
- Drivers publish via `HAL_Driver::publishReceivedData(...)`. When cadence is known, fill
  `frameStep`; when backdatable (e.g. audio: `timestamp = now - step * (totalFrames - 1)`),
  do so. Never emit timing-free `QByteArray`.
- When a driver hops to the main thread (`QMetaObject::invokeMethod`, queued connection),
  capture `SteadyClock::now()` **before** queueing and pass it to `publishReceivedData`.
  Default-constructed timestamps fire on the receiving thread — silent bug.
- `FrameReader` is a splitter, not a clock: `appendChunk` records `PendingChunk
  { nextFrameTimestamp, frameStep }`; `frameTimestamp(endOffsetExclusive)` walks pending
  chunks and advances each chunk's clock by `frameStep` per logical frame.
- `FrameBuilder` interpolates only when one captured chunk expands into N parsed frames:
  publishes at `data->timestamp + step * i`.
- Export workers use `FrameConsumerWorkerBase::monotonicFrameNs(frame->timestamp, baseline)`
  as a strictly-increasing safety net against same-ns collisions on coarse clocks (Windows
  `steady_clock` ~15 ms). Not the source of truth.
- Debug order when timing looks wrong: driver stamp → `CapturedData` propagation → FrameReader
  split → FrameBuilder fan-out → export/report. Never patch PDF/Chart.js first.

## Threading Rules — DO NOT VIOLATE

| Component | Rule |
|-----------|------|
| `FrameReader` | **Main thread.** Config set once before construction; recreate via `ConnectionManager::resetFrameReader()` / `DeviceManager::reconfigure()`. **Never add mutexes.** Single-delimiter uses KMP; multi uses `CircularBuffer::findFirstOfPatterns()` (single-pass, stack array ≤8). Preserves driver timing via `PendingChunk` spans. (Historical: threaded extraction removed in beeda4c0; if it returns, `DeviceManager::frameReady` / `rawDataReceived` go back to `Qt::QueuedConnection`.) |
| `CircularBuffer` | **SPSC only.** Driver writes from whatever thread emitted `dataReceived`; reader is FrameReader on main. Never MPMC. |
| `Dashboard` | **Main thread only.** Reads the shared `TimestampedFramePtr`. |
| Export workers | Lock-free enqueue from main; batch on worker thread. Consume a detached `make_shared` copy of the frame (NOT the Dashboard's pooled slot), so a slow worker's backlog can't pin the pool. |

**Hotpath signal hops must be `Qt::DirectConnection`.** A queued connection between two
main-thread objects costs a `QMetaCallEvent` alloc + event-queue insertion per emit; at
10+ kHz that fills FrameReader's 4096-slot queue faster than the consumer drains and
trips `Frame queue full — frame dropped`. Known direct sites:

- `DeviceManager::frameReady → ConnectionManager::onFrameReady`
- `DeviceManager::rawDataReceived → ConnectionManager::onRawDataReceived`
- `FrameReader::readyRead → DeviceManager::onReadyRead` (AutoConnection resolves Direct)

## AppState — Single Source of Truth

`AppState` (`Cpp_AppState`) owns `operationMode`, `projectFilePath`, `frameConfig`.

- `operationMode` persists to QSettings; everything else reacts to `operationModeChanged()`.
- `frameConfig` is derived from mode + project source[0]; emits `frameConfigChanged(config)`.
- Init order: all `setupExternalConnections()` first, then `restoreLastProject()`.
- `setOperationMode()` guard-returns if unchanged.

## Operation Modes

| Mode | Delimiters | CSV delim | JS parser | Dashboard |
|------|-----------|-----------|-----------|-----------|
| ProjectFile (0) | Per-source | Via JS | Yes | Yes |
| ConsoleOnly (1) | None (short-circuits) | N/A | No | No |
| QuickPlot (2) | Line-based (CR/LF/CRLF) | Comma | No | Yes |

ConsoleOnly (replaced DeviceSendsJSON, 2026-04) bypasses CircularBuffer + queue;
`FrameBuilder::hotpathRxFrame` is a no-op; raw bytes reach the terminal via
`DeviceManager::rawDataReceived`.

## IO Architecture — No Singleton Drivers

- 9 drivers, **public ctors**, no `static instance()`.
- `ConnectionManager` (singleton, `Cpp_IO_Manager`) owns one **UI-config** instance per type:
  `instance().uart()`, `.network()`, `.bluetoothLE()`, etc. QML context properties
  (`Cpp_IO_Serial`, etc.) point at these.
- `createDriver()` makes **fresh** instances for live connections, owned by `DeviceManager`.
- `configurationOk()` checks the **UI** driver, not the live one. UI driver's
  `configurationChanged` forwards to `ConnectionManager::configurationChanged`. All drivers
  must `Q_EMIT configurationChanged()` from their ctor.
- Live drivers may have empty device lists. UART/Modbus call `refreshSerialDevices()` /
  `refreshSerialPorts()` in `open()` if empty.

## ProjectModel / ProjectEditor Split

- `ProjectModel` (`Cpp_JSON_ProjectModel`): pure data — groups, actions, config, file I/O.
- `ProjectEditor` (`Cpp_JSON_ProjectEditor`): editor controller — tree model, form models,
  selection, comboboxes.
- QML enum access: `ProjectModel.SomeEnum` / `ProjectEditor.SomeEnum` — **not** `Cpp_JSON_*`.
- `groupsChanged` → `buildTreeModel()` is `Qt::QueuedConnection`; selection runs via hint
  signals afterwards.
- Title edits update the tree item in-place via `m_*Items` — never call a mutating
  `ProjectModel` function on every keystroke.

## Rolling Backups — `BackupManager`

- Auto-snapshots the project on a 5s debounce. The **whole-project SHA-1** over
  `serializeToJson()` is the sole write arbiter: identical content never duplicates a snapshot,
  any byte difference (incl. `frameParserCode`) does. Restore round-trips parser code + engines.
- Trigger is **decoupled from the dirty flag**. `setModified()` suppresses the flag for a
  structurally empty project (no groups/actions/tables/workspaces), but still emits
  `contentTouched` so parser-only edits on an empty project reach the snapshot path. Wire any new
  "edit that should back up but not dirty the project" through `contentTouched`, not a forced
  `modifiedChanged`.

## Multi-Source Architecture

- `DataModel::Source` entries in `Frame.h`. `FrameBuilder::hotpathRxSourceFrame(sourceId, data)`
  routes per-source frames. `FrameParser` keeps one engine per `sourceId`.
- GPL: `openJsonFile()` truncates `m_sources` to 1; `addSource()` is gated by
  `BUILD_COMMERCIAL`.
- Bus type change: set `m_awaitingContextRebuild`, wait for one-shot `contextsRebuilt`, then
  `buildSourceModel`. Don't force-rebuild on selection.

## Project File JSON Keys — `Keys::` Namespace

Every JSON key used in `.json`/`.ssproj` files is declared in `namespace Keys` at the top
of `app/src/DataModel/Frame.h` as `inline constexpr QLatin1StringView` (alias `KeyView`).

- **Never hardcode** `"busType"`, `"frameStart"`, etc. in writers/readers or MCP handlers —
  use `Keys::BusType`, `Keys::FrameStart`. (`code-verify.py:keys-hardcoded-literal`.)
- `ss_jsr(obj, Keys::Foo, default)` is the canonical reader.
- **Legacy aliases (read canonical first, write both)**: `checksum` ↔ `checksumAlgorithm`,
  `decoder` ↔ `decoderMethod`. Older Serial Studio versions still load 3.3+ files.
- **Schema versioning** (`kSchemaVersion = 1`): `ProjectModel::serializeToJson()` always
  stamps `schemaVersion`, `writerVersion`, `writerVersionAtCreation`. Live runtime frames
  broadcast over the API keep `schemaVersion = 0` — `Frame::serialize` only emits when the
  Frame already carries a stamp. `current_writer_version()` lives in `Frame.cpp` so
  `Frame.h` doesn't need `AppInfo.h`.
- Use `obj.contains(Keys::Foo)` to detect "field absent", not `std::isnan` on a default-zero
  read.

## Frame Parser — Dual Language (JS + Lua)

- `IScriptEngine` is the abstraction. Two impls:
  - `JsScriptEngine` — `QJSEngine` with `ConsoleExtension + GarbageCollectionExtension`
    only (**not** `AllExtensions`). Watchdog: **always route through
    `IScriptEngine::guardedCall()`**; never call `parseFunction.call()` directly.
  - `LuaScriptEngine` — Lua 5.4 (`lib/lua/lua54`), one `lua_State*` per source.
- **JS interruption is cross-thread.** A `QTimer` on the thread running `QJSValue::call()`
  can never fire — the event loop is blocked while the script runs (this was a real,
  shipped no-op against `while(true){}`). `JsWatchdogThread` (a dedicated `QThread` polling
  armed `JsWatchdog`s every 20 ms) flips `setInterrupted(true)` from off-thread, which Qt
  documents as thread-safe. Lua uses an in-engine `LUA_MASKCOUNT` hook + `QDeadlineTimer`
  instead. Every JS engine (parser, transform, Painter, Output, MQTT) holds a `JsWatchdog`
  that registers with the thread; `arm()`/`disarm()` are lock-free atomic-deadline stores
  safe on the hotpath. **`setInterrupted(true)` may appear only in `JsWatchdogThread.cpp`**
  — `code-verify.py:js-interrupt-off-thread` blocks it anywhere else.
- Per-source `frameParserLanguage` ("javascript" | "lua") picks the engine at
  `compileFrameParser()`. Templates in `app/rcc/scripts/{js,lua}/` + `templates.json`.

## Per-Dataset Value Transforms

- Each dataset may carry a `transformCode` string (language matches its source). The user
  defines `function transform(value) -> number`.
- **Compile-once, call-many.** `FrameBuilder::compileTransforms()` runs on project load /
  connection open. One Lua state or `QJSEngine` per source; per-dataset function refs cached
  in `m_transformEngines`.
- **Lua isolation**: `luaL_dostring` once; top-level `local`s become upvalues in the
  `transform` closure, so two datasets sharing the same Lua state don't clobber each other.
- **JS isolation**: user code is wrapped in an IIFE at compile time so top-level `var`s are
  closure-scoped per dataset.
- **Hotpath**: `applyTransform(language, uniqueId, rawValue, info)` → cached per-source
  engine pointer (`m_luaEngineForSource` / `m_jsEngineForSource`, refreshed on `sourceId`
  change in `applyDatasetValues`; **no `std::map::find` per dataset per frame**) →
  `lua_pcall` / `QJSValue::call`. Single-arg transforms skip the info table / object
  allocation: `acceptsInfo` is detected at compile time via `lua_getinfo(">u")` (Lua) and
  `function.length` (JS) and stored on the per-dataset ref.
- **JS watchdog is frame-level**, not per-call. `applyDatasetValues` arms the active source's
  per-engine watchdog (`m_jsEngineForSource->jsWatchdog->arm()`) once, runs the dataset loop,
  and disarms it. The 100 ms budget covers the whole frame's transforms collectively, and the
  interrupt is delivered off-thread by `JsWatchdogThread` (see Frame Parser). On timeout
  `applyTransformJs` sets `m_jsTransformTimedOut`; the user-facing notification is posted once
  from the main thread after the loop, never from the watchdog thread.
- Non-finite numeric results are rejected (`[[unlikely]]` guarded) and `rawValue` is returned.
- **Editor**: `DatasetTransformEditor` prefills a multiline-comment placeholder when the
  dataset has no transform; `onApply` discards code that doesn't define `transform()` via
  `definesTransformFunction()` so the placeholder never persists.

## Data Tables — Central Data Bus

- `DataModel::DataTableStore`: flat pre-allocated register store, **no hotpath allocation**.
- **System table `__datasets__`**: auto-generated. Two registers per dataset:
  `raw:<uniqueId>`, `final:<uniqueId>`. Populated by FrameBuilder during parsing.
- **User tables**: defined in project JSON under `"tables"`. Registers are `Constant`
  (read-only at runtime) or `Computed` (writable by transforms, **persists across frames** —
  no automatic per-frame reset). The `defaultValue` is the value at project load only.
  Computed registers are the natural place for filter state, integrators, edge counters,
  and latched flags; for a "clear me each frame" effect, the transform writes the reset
  value explicitly at the top of an early dataset.
- **Transform API** (injected at compile time): `tableGet`, `tableSet`, `datasetGetRaw`,
  `datasetGetFinal`. Lua = C closures; JS = `TableApiBridge` QObject.
- **Processing order**: group-array then dataset-array. A transform sees raw of ALL datasets,
  final of EARLIER datasets only.
- `applyTransform` returns `QVariant` (double or QString). `Dataset` has
  `rawNumericValue`/`rawValue` snapshots and `virtual_` (no frame index — transform-only).

## Export Architecture & Sessions DB (Pro)

- `DataModel::ExportSchema` (`ExportSchema.h`): shared column layout. `buildExportSchema(frame)`
  produces sorted columns + `uniqueIdToColumnIndex` map. CSV and MDF4 export raw + transformed.
- **Session DB lives in `app/src/Sessions/`** (NOT `app/src/SQLite/`):
  - `Sessions::DatabaseManager` — singleton owning the open `.db`; backs `app/qml/DatabaseExplorer/`.
  - `SQLite::Export` (`Sessions/Export.h/.cpp`): `FrameConsumer`-based; tables
    `sessions/columns/readings/raw_bytes/table_snapshots`; second lock-free queue for raw
    bytes via `ConnectionManager::onRawDataReceived`. WAL mode, batch transactions.
  - `SQLite::Player`: replays a stored session through the FrameBuilder pipeline.
  - Per-sample tables use **surrogate rowid PKs** (`reading_id`, `raw_id`, `snapshot_id`
    `INTEGER PRIMARY KEY AUTOINCREMENT`) with covering indexes on
    `(session_id, unique_id, timestamp_ns)` and `(session_id, timestamp_ns)`. Use plain
    `INSERT` — **never `INSERT OR IGNORE`** — `timestamp_ns` collisions are routine.
  - Break ts ties with `reading_id` in ORDER BY / MIN/MAX subqueries. `DISTINCT timestamp_ns`
    stats undercount on collisions.

## Other Subsystems

- **Plot X-Axis (Time / Dataset)**: `Dataset::xAxisId` selects the plot X source: `kXAxisTime (-2)`,
  the **default**, or a dataset `uniqueId (>=0)` (`Frame.h`). The old `kXAxisSamples (-1)` is **removed
  as a user option** (kept only as a migration sentinel: deserialize maps `-1 -> -2`,
  `migrateLegacyXAxisIds` maps legacy index/samples -> Time). Time is free; dataset-as-X stays
  Pro/Trial-gated. **Time plots do NOT use the raw sample ring.** They use a per-curve
  `DSP::TimeRing` (`DSP.h`): a bounded `(time, value)` ring that **decimates on ingest** to one
  representative sample per `interval = windowSec / capacity` second. The decimator keeps the
  **larger-magnitude** sample seen during the interval (`appendDecimated` in `DSP.h`), so
  transients survive even at extreme zoom-out. Capacity is sized in `Dashboard.cpp` by
  `timeRingCapacity(plotTimeRangeSec)`: `min(plotTimeRange * kAssumedMaxRateHz, kMaxTimeRingSamples)`
  with a floor of `kDefaultPlotBuckets` (`50000` Hz assumption, `262144` cap, `1024` floor). Storage
  is `m_plotTimeRings` / `m_multiplotTimeRings` (keyed by widget index; the multiplot one is a
  `std::vector<TimeRing>` per curve). The hotpath appends `numericValue` at `m_plotDisplayTimeSec`
  via `m_timePushes` (single plots) and `m_multiplotPushes` with its `TimeCurve` list (multi). The
  widget side calls `Dashboard::plotTimeRing(idx)` / `multiplotTimeRings(idx)` and renders through
  `DSP::downsampleTimeWindow(ring.time, ring.value, ...)`: no envelope, just a viewport decimation
  of the already-decimated ring. This is why 10 s of 48 kHz audio works: the ring caps at
  `kMaxTimeRingSamples` and `appendDecimated` collapses bursts into peak-preserving slots, bounded
  memory/CPU, axis fixed at `[-T, 0]` (never recompute the axis from raw extremes). **Display
  clock** (`m_plotDisplayTimeSec`, `hotpathRxFrame`): sources without a cadence stamp many frames
  at one coarse wall-clock tick (~15 ms on Windows), which would compress them onto a single
  decimator interval and lose temporal spread; the display clock spreads same-timestamp frames
  by a smoothed per-sample period so sub-tick windows still render. It is self-correcting
  (n samples over a gap fill it exactly) and display-only: `m_relativeFrameTimeSec` and exported
  timestamps stay raw. Fine-timestamp sources (audio) hit the n==1 path and are unchanged. Ticks
  render the **magnitude** in an adaptive unit (`PlotWidget.qml` `timeAxis` + `secondsAgoFormat`
  + `timeUnitFactor`/`timeUnitName`): the title and ticks switch between `s` / `ms` / `us` from
  the span, so e.g. a 10 ms window reads `Time (ms)` with `10 8 6 4 2 0`. Dataset-X plots, FFT,
  GPS, 3D keep the raw-ring + downsample path.
- **Plot Time Range**: `Dashboard::plotTimeRange` (seconds, default 10, **1 ms min**) is the ring
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
- **Waterfall follows the Time Range** (Pro): `syncHistoryToTimeRange` sets `m_historySize =
  round(plotTimeRange * fps)` (clamped 16..4096) on `plotTimeRangeChanged` / `fpsChanged` and at
  construction, so its time axis (`historySize / fps`) reads the Time Range. fps is the row cadence
  (one row per dashboard `updated` tick), not the sample rate; sub-second ranges clamp to 16 rows.
- **AxisRangeDialog** hides its X section for time plots (`timeAxis` from the widget model); the manual
  X min/max is meaningless when X is the Time Range. Y range stays editable.
- **Plot Sweep / Trigger mode (Pro)**: oscilloscope sweep for **time-axis** Plot/MultiPlot. `DSP::SweepEngine`
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
  `PlotWidget.qml` (`sweepMode`/`triggerLevel`). Setters are runtime-gated to `FeatureTier >= Trial`. `SweepMode`/
  `TriggerEdge` enums live in `SerialStudio.h`.
- **Output Widgets (Pro)** (`app/src/UI/Widgets/Output/`, QML in `app/qml/Widgets/Dashboard/Output/`):
  Button/Toggle/Slider/TextField/Panel sharing `Base`. User JS converts UI state → device
  bytes (`app/rcc/scripts/output/*.js`); `OutputCodeEditor` edits; `TransmitTestDialog`
  previews. Protocol helpers (CRC, NMEA, Modbus, SLCAN, GRBL, GCode, SCPI, binary packet)
  injected into the engine. Gated `FeatureTier >= Pro`
  (`None=0, Hobbyist=1, Trial=2, Pro=3, Enterprise=4`).
- **Workspaces** (`UI::Taskbar`, `app/qml/MainWindow/Taskbar/`): user-defined dashboard tabs.
  Persisted under `"workspaces"`. **Workspace IDs ≥ 1000**, group IDs < 1000.
  `Taskbar::deleteWorkspace(id)` branches on the threshold — don't cross-wire. Edits stage
  in memory + `setModified(true)`; no autosave.
- **Waterfall / Spectrogram (Pro)** (`UI/Widgets/Waterfall.h/.cpp`): per-dataset Pro widget
  reusing the dataset's FFT settings. Class IS the painted item (`QQuickPaintedItem`).
  Toggle via `DatasetWaterfall = 0b01000000`; persists as `Keys::Waterfall` (omit when false).
  `Keys::WaterfallYAxis` non-zero → **Campbell mode**: rows placed by another dataset's
  value (e.g. RPM) instead of time. `commercialCfg()` flags any project using waterfall.
- **File Transmission (Pro)** (`IO::FileTransmission`, `IO::Protocols::*`): controller +
  XMODEM/YMODEM/ZMODEM. Incoming data routes from `ConnectionManager::onRawDataReceived` →
  `FileTransmission::onRawDataReceived` (guarded by `active()`). Protocols emit
  `writeRequested(QByteArray)`; controller calls `ConnectionManager::writeData()`.
- **Modbus Map Importer (Pro)**: `DataModel::ModbusMapImporter` imports CSV/XML/JSON →
  auto-generates a Modbus project; preview in `ModbusPreviewDialog.qml`. Pairs with
  `IO::Drivers::Modbus::generateRegisterGroupProject`.
