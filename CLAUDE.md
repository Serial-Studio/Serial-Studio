# CLAUDE.md

## Behavioral Rules

- **Read before writing.** Never edit a file you haven't read this session.
- **Read `BluetoothLE.h/.cpp`** before writing any new driver — it's the canonical reference.
- **Read hotpath code** (FrameBuilder, CircularBuffer, FrameReader, Dashboard) in full before touching it.
- **Read existing signal/slot connections** in a file before adding or changing any.
- **Plan before multi-file changes.** For >3 files, state the plan and get confirmation.
- **Edit, don't rewrite.** Use targeted `Edit` calls. Rewrite only when asked or >70% changes.
- **No preamble, no trailing summary.** Lead with the action or result.
- **Do not create markdown/doc files** unless explicitly asked. Share info conversationally.
- **Update CLAUDE.md** for any significant architectural change.

## Build

```bash
cmake -B build -DPRODUCTION_OPTIMIZATION=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

# Debug + sanitizers
cmake -B build -DDEBUG_SANITIZER=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)
```

Key flags: `ENABLE_HARDENING`, `ENABLE_PGO`, `ENABLE_GRPC`, `USE_SYSTEM_ZLIB`, `USE_SYSTEM_EXPAT`.

## Project Overview

Serial Studio: cross-platform telemetry dashboard, Qt 6.9.2 + C++20.
Data sources: UART, TCP/UDP, BLE, Audio, Modbus, CAN Bus, MQTT, USB (libusb), HID (hidapi), Process I/O.
15+ visualization widgets, 6 output (control) widgets, 256 KHz+ target data rate.
Frame parsers in JavaScript (QJSEngine) or Lua 5.4 (embedded `lua54`). Per-dataset value transforms
in either language. Output widgets, Modbus, CAN Bus, MDF4, 3D, ImageView, file transfer protocols
(XMODEM/YMODEM/ZMODEM), Modbus map importer, and Session Database are Pro features.

## Directory Structure

```
app/src/
├── IO/          ConnectionManager, DeviceManager, CircularBuffer, FrameReader, FrameConfig
│   ├── Drivers/ UART, Network, BluetoothLE, Audio, CANBus, HID, Modbus, Process, USB
│   └── FileTransmission/ Protocol base, XMODEM, YMODEM, ZMODEM, CRC utilities
├── DataModel/   FrameBuilder, Frame, FrameConsumer, IScriptEngine (JS + Lua),
│                 ProjectModel, ProjectEditor, DataTable(Store), ExportSchema,
│                 ModbusMapImporter, DBCImporter, DatasetTransformEditor,
│                 TransmitTestDialog, OutputCodeEditor, ScriptTemplates
├── UI/          Dashboard, Taskbar (workspaces), visualization + output widget types
│   └── Widgets/Output/  Button, Toggle, Slider, TextField, Panel (+ PanelLayout), Base
├── API/         TCP server port 7777 (MCP + legacy JSON-RPC)
│   └── Handlers/ 25 handler classes (incl. DataTables, Sessions, Workspaces)
├── Console/     Terminal + export
├── CSV/ MDF4/   File playback & export
├── Sessions/    (Pro) DatabaseManager + SQLite::Export + SQLite::Player — session DB
├── MQTT/        MQTT client
├── Licensing/   LemonSqueezy, Trial, MachineID, SimpleCrypt, CommercialToken (FeatureTier)
├── Platform/    CSD, NativeWindow
├── Misc/        JsonValidator, ThemeManager, ModuleManager
├── AppState.h   Singleton: OperationMode, projectFilePath, FrameConfig
├── SerialStudio.h  Central enums (BusType, OperationMode, FrameDetection)
└── Concepts.h   C++20 concepts (Numeric, ByteLike, Driver, Frameable…)
app/qml/
├── DatabaseExplorer/  Session browsing UI (Pro)
├── MainWindow/        Setup, Toolbar, Panes
├── ProjectEditor/     Views: DataTables, SystemDatasets, UserTable, Workspaces,
│                       OutputWidget, FlowDiagram, AddWidgetDialog, ConstantsLibraryDialog
├── Widgets/           Ribbon{Toolbar,Section,Spacer}, ProjectTable{Header,Row},
│                       Dashboard/Output/{Button,Toggle,Slider,TextField,OutputPanel}
└── Dialogs/           SqlitePlayer, FileTransmission, etc.
lib/                   KissFFT, QCodeEditor, mdflib, OpenSSL, lua (lua54), QuaZip, hidapi, QSimpleUpdater
```

## Architecture — Critical Invariants

### Data Flow

```
Driver  (driver thread OR main thread, depending on driver)
  │ HAL_Driver::dataReceived(CapturedDataPtr)           AutoConnection
  ▼
FrameReader::processData  (main thread)
  │ appends raw bytes to CircularBuffer (SPSC), tracks chunk timestamps
  │ KMP-scans for delimiters, assigns each completed frame a source-derived timestamp
  │ enqueues completed frames to lock-free ReaderWriterQueue<CapturedDataPtr>
  │ emits readyRead once per processData call
  ▼
DeviceManager::onReadyRead  (main thread, direct)
  │ while try_dequeue: Q_EMIT frameReady(deviceId, frame)  DirectConnection
  ▼
ConnectionManager::onFrameReady  (main thread)
  │ routes to FrameBuilder::hotpathRxFrame / hotpathRxSourceFrame
  ▼
FrameBuilder  (main thread)
  │ parses → applies per-dataset transforms → mutates m_frame / m_sourceFrames
  │ creates one TimestampedFramePtr per parsed frame and shares it with every consumer
  ▼
Dashboard / CSV / MDF4 / API::Server / gRPC / Sessions  (shared parsed frame object)
```

### Timestamp Ownership — Source Owns Time

Timing is stamped at the driver boundary and preserved downstream. Do not reintroduce export-only or report-only timestamp synthesis.

- `IO::CapturedData` (`HAL_Driver.h`): shared chunk carrying `data` (`ByteArrayPtr`), `timestamp` (steady_clock), `frameStep` (cadence in ns), `logicalFramesHint`. `CapturedDataPtr` is the transport type on the whole hotpath.
- Drivers publish via `HAL_Driver::publishReceivedData(...)`. Never emit timing-free `QByteArray`. When a driver knows cadence, fill `frameStep`; when it can backdate (e.g. audio — `Audio.cpp::processInputBuffer()` sets `timestamp = now - step * (totalFrames - 1)`), do so.
- If a driver posts to the main thread (queued connection, `QMetaObject::invokeMethod`), capture `SteadyClock::now()` **before** queueing and pass it to `publishReceivedData`. Default-constructed timestamps fire on the receiving thread — a latent bug.
- `FrameReader` propagates timing: `appendChunk` records `PendingChunk { nextFrameTimestamp, frameStep }`; `frameTimestamp(endOffsetExclusive)` walks pending chunks to stamp each extracted logical frame, advancing that chunk's clock by `frameStep`. It is a splitter, not a clock source.
- `FrameBuilder` does the one legitimate interpolation: when a captured chunk expands into N parsed frames, publishes at `data->timestamp + step * i`. Single `TimestampedFramePtr` is shared with dashboard, CSV, MDF4, API, gRPC, Sessions, MQTT.
- Export workers derive strictly-increasing offsets from `FrameConsumerWorkerBase::monotonicFrameNs(frame->timestamp, baseline)` — a safety net against same-ns collisions on coarse clocks (Windows `steady_clock` ~15 ms), not the source of truth.
- Debugging checklist when timing looks wrong: driver stamp → `CapturedData` propagation → FrameReader split → `FrameBuilder` fan-out → export/report. Never patch PDF/Chart.js first.

### Threading Rules — DO NOT VIOLATE

| Component | Rule |
|-----------|------|
| `FrameReader` | **Currently runs on the main thread.** Config set **once** before construction; recreate via `ConnectionManager::resetFrameReader()` / `DeviceManager::reconfigure()` on change. **Never add mutexes.** Delimiters are `QList<QByteArray>` (multi-pattern); single-delimiter uses KMP fast path, multi-delimiter uses `CircularBuffer::findFirstOfPatterns()` single-pass scan. It must preserve driver-side timing metadata by tracking pending chunk spans alongside the byte buffer. Historical note: `m_threadedExtraction` / `moveToThread` was removed in beeda4c0; if it ever returns, flip `DeviceManager::frameReady` / `rawDataReceived` connections back to `Qt::QueuedConnection` in `ConnectionManager::wireDevice()`. |
| `CircularBuffer` | **SPSC only.** One producer, one consumer. Never MPMC. Even with FrameReader on the main thread, keep SPSC — the driver side still writes from whatever thread emitted `dataReceived`. |
| `Dashboard` | **Main thread only.** Receives the same `TimestampedFramePtr` object that export/API consumers receive and reads `frame->data` from it. |
| Export workers | Lock-free enqueue from main, batch on worker thread. They consume the shared `TimestampedFramePtr` published by `FrameBuilder`; timestamp synthesis in export workers is only a monotonic safety net now, not the primary source of time. |

### Signal Connection Hops — Hotpath

Same-thread signal hops on the frame path **must be `Qt::DirectConnection`**.
A queued connection between two main-thread objects is pure overhead: every emit
becomes a `QMetaCallEvent` allocation + event-queue insertion + deferred dispatch.
At 10+ kHz frame rates that extra work lets the FrameReader's 4096-slot queue
fill faster than the consumer drains it → `[FrameReader] Frame queue full — frame dropped`.

Known direct-connection sites:
- `DeviceManager::frameReady → ConnectionManager::onFrameReady` (wireDevice)
- `DeviceManager::rawDataReceived → ConnectionManager::onRawDataReceived` (wireDevice)
- `FrameReader::readyRead → DeviceManager::onReadyRead` (AutoConnection, resolves to Direct)

### AppState — Single Source of Truth

`AppState` singleton (`Cpp_AppState` in QML) owns `operationMode`, `projectFilePath`, `frameConfig`.
- `operationMode` persisted to QSettings; others react to `operationModeChanged()`.
- `frameConfig` derived from mode + project source[0]; emits `frameConfigChanged(config)`.
- Init order: all `setupExternalConnections()` first, then `restoreLastProject()`.
- `setOperationMode()` guard-returns if unchanged — no duplicate signals.

### Operation Modes

| Mode | Delimiters | CSV delimiter | JS parser | Dashboard |
|------|-----------|---------------|-----------|-----------|
| ProjectFile (0) | Configurable per-source | Via JS | Yes | Yes |
| ConsoleOnly (1) | None — FrameReader short-circuits | N/A | No | No (console only) |
| QuickPlot (2) | Line-based (CR/LF/CRLF) | Comma | No | Yes |

ConsoleOnly replaced the old DeviceSendsJSON mode in 2026-04: raw bytes reach the
terminal via `DeviceManager::rawDataReceived` only; FrameReader skips the circular
buffer and queue, and `FrameBuilder::hotpathRxFrame` is a no-op for this mode.

### IO Architecture — No Singleton Drivers

- All 9 drivers have **public constructors**, no `static instance()`.
- `ConnectionManager` (singleton, `Cpp_IO_Manager`) owns one **UI-config instance** per type.
- Accessors: `ConnectionManager::instance().uart()`, `.network()`, `.bluetoothLE()`, etc.
- `createDriver()` makes fresh instances for live connections (owned by `DeviceManager`).
- QML context properties (`Cpp_IO_Serial`, `Cpp_IO_Network`, etc.) → UI-config instances.
- `DeviceManager`: non-singleton, owns one `HAL_Driver` + `FrameReader`. FrameReader runs on the main thread (see beeda4c0).
- Drivers publish `IO::CapturedDataPtr` via `HAL_Driver::publishReceivedData(...)`. See the Timestamp Ownership section above.
- `configurationOk()` checks the **UI driver** (not the live driver). UI driver `configurationChanged` is forwarded to `ConnectionManager::configurationChanged`.
- All drivers must emit `configurationChanged()` from their constructors (connect specific signals like `portIndexChanged` → `configurationChanged`).
- Live drivers may lack enumerated device lists. UART/Modbus call `refreshSerialDevices()`/`refreshSerialPorts()` in `open()` if lists are empty.

### BluetoothLE — Shared Static Discovery

- Discovery state is **static** (`s_devices`, `s_discoveryAgent`, `s_adapterAvailable`), shared across all instances via `s_instances`. Connection state is per-instance (`m_controller`, `m_service`).
- Device list is **append-only** during rediscovery — indices stay stable.
- `selectDevice(0)` is ignored when already selected/connected (prevents QML combobox rebuilds from resetting selection).
- `selectService()` / characteristic selection on an instance without a controller forwards to the instance that has one.
- `setDriverProperty()` sets raw values directly; `driverProperties()` returns raw internals (no placeholder compensation).

### ProjectModel / ProjectEditor Split

- `ProjectModel` (`Cpp_JSON_ProjectModel`): pure data — groups, actions, config, file I/O.
- `ProjectEditor` (`Cpp_JSON_ProjectEditor`): editor controller — tree model, form models, selection.
- QML enum access: `ProjectModel.SomeEnum` / `ProjectEditor.SomeEnum` (NOT `Cpp_JSON_*`).
- Signal flow: `groupsChanged` (queued) → `buildTreeModel()`, then hint signals → selection.
- Title edits: update tree item text in-place via `m_*Items` map — never call a mutating `ProjectModel` function on every keystroke.

### Multi-Source Architecture

- Each project has `DataModel::Source` entries in `Frame.h`.
- `FrameBuilder::hotpathRxSourceFrame(sourceId, data)` routes per-source frames.
- `FrameParser`: per-source JS engine map keyed by `sourceId`.
- GPL: `openJsonFile()` truncates `m_sources` to 1; `addSource()` gated by `BUILD_COMMERCIAL`.
- Bus type change: `m_awaitingContextRebuild` flag → one-shot `contextsRebuilt` → `buildSourceModel`.
- Source JSON keys: `title`, `sourceId`, `busType`, `frameStart`, `frameEnd`, `frameDetection`, `decoder`/`decoderMethod`, `checksum`/`checksumAlgorithm`, `hexadecimalDelimiters`, `frameParserCode`, `frameParserLanguage`, `connection`.

### Project File JSON Keys — Keys:: Namespace

- **Single source of truth**: every JSON key used in `.json`/`.ssproj` project files is declared in `namespace Keys` at the top of `app/src/DataModel/Frame.h` as `inline constexpr QLatin1StringView` (alias `KeyView`). Never hardcode `"busType"`, `"frameStart"`, etc. in writer/reader code — use `Keys::BusType`, `Keys::FrameStart`. This applies to MCP API handlers too (their param names mirror the file format).
- `ss_jsr(obj, Keys::Foo, default)` is the canonical reader; takes `QLatin1StringView` so `Keys::*` doesn't allocate per call.
- **Legacy aliases (read with fallback, write both)**: `checksum` ↔ `checksumAlgorithm`, `decoder` ↔ `decoderMethod`. `read(Source&)`, `read_io_settings`, `ProjectModel::loadFromJsonDocument` prefer canonical and fall back to legacy. Writers emit BOTH forms so older Serial Studio versions can still load files written by 3.3+.
- **Schema versioning** (`kSchemaVersion = 1`): every project save stamps `schemaVersion`, `writerVersion`, `writerVersionAtCreation` at the JSON root. `Frame::serialize` only emits these when the Frame already carries a stamp — live runtime frames (broadcast over the API at kHz rates) keep `schemaVersion = 0` so the stamp is suppressed. The canonical project save path is `ProjectModel::serializeToJson()`, which always stamps.
- `current_writer_version()` (declared in Frame.h, defined in Frame.cpp) returns the running app version so Frame.h doesn't need to include `AppInfo.h`.

### Frame Parser — Dual Language (JS + Lua)

- `DataModel::IScriptEngine` is the parser abstraction. Two implementations:
  - `JsScriptEngine` — Qt `QJSEngine` with `ConsoleExtension + GarbageCollectionExtension` only
    (NOT `AllExtensions`). Runtime watchdog via `guardedCall()` (never call `parseFunction.call()`
    directly — see NASA Rule 7).
  - `LuaScriptEngine` — embedded Lua 5.4 (`lib/lua/lua54`). One `lua_State*` per source.
- Per-source `frameParserLanguage` field in project JSON ("javascript" or "lua") selects the
  engine at `compileFrameParser()` time. Lua example scripts live in `app/rcc/scripts/lua/`,
  JS in `app/rcc/scripts/js/`, both listed in `templates.json`.
- QML picker in `FrameParserView` / `SourceFrameParserView` switches between the two editors.

### Per-Dataset Value Transforms

- Each dataset may carry a `transformCode` string (Lua or JS, language matches the dataset's source).
  A transform is a small user script defining `function transform(value) → number`.
- **Compile-once, call-many**: `FrameBuilder::compileTransforms()` runs when the project loads or connection opens. It builds one shared Lua state or `QJSEngine` per source and caches per-dataset function references (`m_transformEngines` map).
- **Lua isolation**: each dataset's chunk runs once via `luaL_dostring`. Top-level `local` declarations become upvalues captured by the `transform` closure — so `local ema`/`local alpha` give each dataset its own state, even when the Lua state is shared.
- **JS isolation**: the user's code is wrapped in an IIFE at compile time (`(function(){ <code>; return typeof transform === 'function' ? transform : null; })()`) so top-level `var` declarations are closure-scoped per dataset. Without this, multiple datasets using the same template (e.g. two EMAs) would clobber each other's globals on the shared `QJSEngine`.
- **Hotpath**: `applyTransform(sourceId, uniqueId, rawValue)` is called on every frame for every dataset with `!transformCode.isEmpty()`. Lookup is an `std::map::find`; Lua call is a `lua_rawgeti` + `lua_pcall`; JS call is a `QJSValue::call`. Non-finite results (NaN/Inf) are rejected and `rawValue` is returned unchanged (`[[unlikely]]` guarded).
- **Templates**: `app/rcc/scripts/transforms/{js,lua}/<name>.{js,lua}` + `templates.json` manifest with translations. The manifest loader is `DataModel::loadScriptTemplateManifest()` (shared with parser/output templates). New templates must be added to `rcc.qrc` in **both** `js/` and `lua/` sections.
- **Editor UX**: `DatasetTransformEditor` prefills the editor with a multiline-comment placeholder when the dataset has no transform. QTextEdit's `setPlaceholderText` only renders the first line of multiline hints, so prefill instead. `onApply` detects "code does not define transform()" via `definesTransformFunction()` (disposable engine compile + lookup) and emits an empty string in that case so the placeholder is never persisted to the project.

### Data Tables — Central Data Bus

- `DataModel::DataTableStore` (`DataTable.h/.cpp`): flat pre-allocated register store, no hotpath allocation.
- **System table (`__datasets__`)**: auto-generated from template frame. Two registers per dataset: `raw:<uniqueId>` and `final:<uniqueId>`. Populated by FrameBuilder during parsing.
- **User tables**: defined in project JSON under `"tables"` key. Registers are `Constant` (read-only at runtime) or `Computed` (reset per frame, writable by transforms).
- **Transform API** (injected at compile time): `tableGet(table, reg)`, `tableSet(table, reg, val)`, `datasetGetRaw(uid)`, `datasetGetFinal(uid)`. Lua uses C closures; JS uses `TableApiBridge` QObject.
- **Processing order**: datasets processed in group-array then dataset-array order. A transform can read raw values of ALL datasets and final values of EARLIER datasets.
- `applyTransform` accepts/returns `QVariant` (double or QString), not just double.
- `Dataset` struct has `rawNumericValue`, `rawValue` (snapshots before transform), `virtual_` (no frame index, computed entirely from transforms).

### Output Widgets (Pro)

- `app/src/UI/Widgets/Output/`: `Button`, `Toggle`, `Slider`, `TextField`, `Panel` (+ `PanelLayout`), sharing `Base`. QML in `app/qml/Widgets/Dashboard/Output/`.
- User-authored JS "output scripts" (`app/rcc/scripts/output/*.js`) convert UI state → device bytes. `OutputCodeEditor` edits; `TransmitTestDialog` previews.
- Protocol helpers (CRC, NMEA, Modbus, SLCAN, GRBL, GCode, SCPI, binary packet…) are injected into the engine.
- Gated `FeatureTier >= Pro`. Tier enum: `None=0, Hobbyist=1, Trial=2, Pro=3, Enterprise=4`.

### Workspaces (Dashboard)

- User-defined dashboard tabs grouping selected widgets. Persisted under `"workspaces"`. `UI::Taskbar` owns active-workspace state and has a filter/search bar.
- **Workspace IDs ≥ 1000**, group IDs < 1000. `Taskbar::deleteWorkspace(id)` branches: ≥1000 → `ProjectModel::deleteWorkspace()`, ≥0 → `hideGroup()`. Don't cross-wire.
- Workspace edits stage into memory + `setModified(true)` — no autosave to disk.

### Export Architecture

- `DataModel::ExportSchema` (`ExportSchema.h`): shared column layout for all exporters. `buildExportSchema(frame)` produces sorted columns with `uniqueIdToColumnIndex` map.
- CSV and MDF4 both export raw + transformed values per dataset.
- **Session Database (Pro)** lives in `app/src/Sessions/` (NOT `app/src/SQLite/`):
  - `Sessions::DatabaseManager` — singleton that owns the open `.db` file, manages browsing,
    tagging, CSV export, and project metadata. Backs `app/qml/DatabaseExplorer/`.
  - `SQLite::Export` (`Sessions/Export.h/.cpp`): `FrameConsumer`-based with
    sessions/columns/readings/raw_bytes/table_snapshots tables. Second lock-free queue for
    raw bytes from `ConnectionManager::onRawDataReceived`. WAL mode, batch transactions.
  - `SQLite::Player` (`Sessions/Player.h/.cpp`): replays a stored session back through the
    FrameBuilder pipeline (see `app/qml/Dialogs/SqlitePlayer.qml`).
  - **Per-sample tables use surrogate rowid PKs.** `readings`, `raw_bytes`, `table_snapshots`
    are keyed by `reading_id` / `raw_id` / `snapshot_id` `INTEGER PRIMARY KEY AUTOINCREMENT`
    with covering indexes on `(session_id, unique_id, timestamp_ns)` and
    `(session_id, timestamp_ns)`. Writers use plain `INSERT` — never `INSERT OR IGNORE` —
    because `timestamp_ns` collisions are routine (Windows `steady_clock` ~15 ms granularity,
    audio at ≥48 kHz). No schema versioning, no migration — nightly feature, old .db files
    are regenerated.
  - **Break timestamp ties with `reading_id`.** All queries that need deterministic "first"
    / "last" / per-frame ordering must include `reading_id` in `ORDER BY` or in
    `MIN/MAX`-based subqueries. `DISTINCT timestamp_ns`-based stats undercount on collisions
    — acceptable for display, not authoritative.
  - **Session reports** (`ReportData.cpp`, `HtmlReport.cpp`): `loadChartSeries` passes raw
    samples through verbatim under the 10k-point budget and switches to min/max bucket
    decimation above. `DatasetSeries::totalSamples` carries the real row count for chart
    subtitles. Chart-card CSS pins the landscape printable area per selected paper size via
    `{{CHART_PAGE_WIDTH_MM}}` / `{{CHART_PAGE_HEIGHT_MM}}` so A3 / Letter / Legal print
    without stretching.

### File Transmission — Protocol Architecture

- `IO::FileTransmission` (singleton, `Cpp_IO_FileTransmission`): controller that owns protocol instances and manages plain text / raw binary modes.
- `IO::Protocols::Protocol`: abstract base class with `startTransfer()`, `cancelTransfer()`, `processInput()`, signals `writeRequested()`, `progressChanged()`, `statusMessage()`, `finished()`.
- `IO::Protocols::XMODEM`: 128/1024-byte blocks, CRC-16, ACK/NAK/CAN. YMODEM inherits from XMODEM.
- `IO::Protocols::YMODEM`: adds block 0 (filename + size), dual-EOT, end-of-batch signaling.
- `IO::Protocols::ZMODEM`: streaming with ZDLE escaping, hex/binary-32 headers, CRC-32, ZRPOS crash recovery.
- Incoming device data is routed from `ConnectionManager::onRawDataReceived()` → `FileTransmission::onRawDataReceived()` (guarded by `active()` + `[[unlikely]]`).
- Protocols emit `writeRequested(QByteArray)` → controller calls `ConnectionManager::writeData()`.

### Common Mistakes — Silent Breakage

| Mistake | Fix |
|---------|-----|
| Function doxygen / member-variable comments / signal comments in a header | Headers hold only the license banner and **one** class-level `/** @brief */`. Everything else is names-as-documentation. |
| Multi-line `//` narrative blocks, inline comments on declarations/members | One-line `//` section headers in `.cpp`; no header comments except the two allowed above |
| `QMetaObject::invokeMethod(...)` forwarding received data without a captured timestamp | Capture `SteadyClock::now()` in the callback, pass it to `publishReceivedData(data, timestamp)` so stamping stays at acquisition |
| Report/export worker calling `steady_clock::now()` to stamp a frame | Use the timestamp already on the shared `TimestampedFramePtr` via `monotonicFrameNs(frame->timestamp, baseline)` |
| `buildTreeModel()` inside item-change handler | `QTimer::singleShot(0, ...)` |
| Mutexes in FrameReader/CircularBuffer | Recreate via `ConnectionManager::resetFrameReader()` / `DeviceManager::reconfigure()` |
| `Qt::QueuedConnection` on the frame hotpath when sender & receiver are both on the main thread | Use `Qt::DirectConnection` — queued costs a `postEvent` per frame and causes queue-full drops |
| `QMap::operator[]` on `m_sourceFrames[sourceId]` | Use `.find()` and validate |
| `disconnect(nullptr)` as slot | Capture `QMetaObject::Connection`, disconnect that |
| Mutating ProjectModel on every keystroke | Update tree item in-place via `m_*Items` |
| `emit` instead of `Q_EMIT` | Always `Q_EMIT` |
| `font.pixelSize` etc. in QML | `font: Cpp_Misc_CommonFonts.uiFont` |
| Hardcoded bus type integers | `SerialStudio::BusType::UART` etc. |
| `createDriver()` for UI config | `ConnectionManager::instance().uart()` etc. |
| Force-rebuilding `buildSourceModel` on selection | Guard with `m_awaitingContextRebuild` |
| Live driver with empty device list | Call `refreshSerialDevices()` etc. in `open()` if empty |
| BLE `selectDevice(index)` with placeholder compensation on `setDriverProperty` | `setDriverProperty` sets raw value directly, `selectDevice` subtracts 1 |
| Querying live driver for `configurationOk()` | Check the UI driver — live driver may not be synced yet |
| Setter without guard return | Always `if (m_foo == foo) return;` before assigning + emitting |
| `Q_INVOKABLE void foo()` | Move to `public slots:` — `Q_INVOKABLE` is for non-void returns only |
| `int m_foo = 0;` in header | Initialize in constructor member init list, not in-class |
| `Q_SIGNALS:` / `public Q_SLOTS:` | Use `signals:` / `public slots:` (lowercase, no `Q_` prefix) |
| Per-dataset `transformCode` in header-filled editor, never dropping the placeholder | `DatasetTransformEditor::onApply` discards code if `definesTransformFunction` returns false |
| Looking for session DB code under `app/src/SQLite/` | It lives in `app/src/Sessions/` — `namespace Sessions` for DatabaseManager, `namespace SQLite` for Export/Player |
| Calling `parseFunction.call(args)` directly on a JS parser | Always route through `IScriptEngine::guardedCall()` — watchdog-protected |
| Mixing workspace IDs with group IDs | Workspace IDs are always `>= 1000`; `Taskbar::deleteWorkspace()` branches on that threshold |
| `Q_INVOKABLE void` on output-widget helper | `public slots:` — same rule as every other QObject |
| Hardcoded JSON keys (`"busType"`, `"frameDetection"`, `"datasetId"`…) in writers/readers | Use `Keys::BusType` etc. from `Frame.h`. New keys go there first, then propagate. |
| Stamping `current_writer_version()` on every live `Frame::serialize` call | Live frames must have `schemaVersion = 0` and empty writer strings — only project-save flows (`ProjectModel::serializeToJson`) carry version metadata. |
| Reading `"checksum"` or `"decoder"` directly | Prefer canonical `Keys::ChecksumAlgorithm` / `Keys::DecoderMethod`, fall back to legacy keys. Writers emit BOTH for back-compat. |
| `std::isnan(...)` guards on fields read with non-NaN defaults (e.g. `ss_jsr(..., 0)`) | Dead code — use `obj.contains(Keys::Foo)` to detect "field absent in JSON". |

## Code Style

### Formatting

100-column limit, 2-space indent. Pointer/reference bind to type (`int* ptr`, `const Foo& ref`).
Braces: function bodies on new line, control statements on same line (`AfterFunction: true`,
`AfterControlStatement: Never`). `RemoveBracesLLVM` strips braces from single-statement bodies.
`BinPackArguments`/`BinPackParameters` are `false` — one per line when they don't fit.
Run `clang-format`.

### Naming

| Kind | Convention | Example |
|------|------------|---------|
| Classes | `CamelCase` | `FrameReader` |
| Functions | `camelCase` | `hotpathRxFrame` |
| Locals / params | `lower_case` | `frame_data` |
| Static variables | `lower_case` | `s_devices` |
| Private members | `m_camelCase` | `m_deviceIndex` |
| Public/protected members | `lower_case` | `sourceId` |
| Constants / constexpr | `kCamelCase` | `kMaxBufferSize` |
| Enums / enum constants | `CamelCase` | `BusType`, `UART` |
| Macros | `UPPER_CASE` | `BUILD_COMMERCIAL` |

### Control Flow

- **Max 3 nesting levels.** Early returns, early continues, extract functions.
- **No braces on single-statement bodies.** Always blank line after a brace-free body on its own line.
- **Guard clauses over nested error handling.**
- **Functions: 40–80 lines target**, split over 100.

```cpp
// Good: early return + blank line after brace-free body
if (!frame.isValid())
  return;

for (const auto& g : frame.groups())
{
  if (!g.isEnabled())
    continue;

  processGroup(g);
}
```

### C++ Header Layout

Reference: `app/src/IO/Drivers/BluetoothLE.h`. Section order: `Q_OBJECT` → `Q_PROPERTY` block (clang-format off, multi-line, one attribute per line) → `signals:` → private ctor + deleted copy/move (singletons) → `public:` (instance() first, then `[[nodiscard]]` getters) → `public slots:` → `private slots:` → `private:` helpers → `private:` member vars.

- `[[nodiscard]]` on every non-void return, including `Q_INVOKABLE` getters.
- **Never `Q_INVOKABLE void`** — use `public slots:`. `Q_INVOKABLE` is for non-void returns only.
- Christmas-tree ordering (shortest→longest line) within each block.
- `noexcept` on trivial const getters that only read members.
- Non-singleton: ctor in `public:`, deleted operators right after.
- **No in-header member init** (`int m_foo = 0;` is forbidden). Initialize in the ctor member init list.

### Source File (.cpp) Layout

Use 98-dash `//---` banners to separate concern groups (constructor, getters, slots…). Reference: `BluetoothLE.cpp`.

### Signals & Connections

- Always `Q_EMIT`, never bare `emit`.
- Always `signals:`, `public slots:`, `private slots:` — never `Q_SIGNALS:`, `public Q_SLOTS:`, etc.
- Short `connect()` on one line; long form: one argument per line.
- Never use `SIGNAL()`/`SLOT()` macros.

### Comments & Doxygen

**Write like a programmer, not a novelist.** Code is the spec. Comments label sections; they do not narrate. Would removing the comment cost the reader anything? If no, don't write it.

**Headers (.h) — strict rule.** Only two comments allowed:
1. SPDX license banner at the top.
2. **One** class-level `/** @brief ... */` directly above the class declaration.

No function doxygen, member-variable comments, signal/slot comments, `@param`/`@return`/`@note`/`@see`, or inline `//`. Names + types are the documentation.

**Source (.cpp).**
- One-line `//` section header above each logical block inside a function body. That's the only kind of in-body comment allowed.
- Single-line `/** @brief ... */` on a non-trivial function is allowed when the name isn't self-explanatory. Default is no function doxygen.
- 98-dash `//---` banners separate concern groups.
- **Forbidden**: inline end-of-line comments, multi-line `//` prose, `/* ... */` inside function bodies, restating what code literally does (`// Set m_foo to bar`), repeating the `@brief` as the body's first line. Load-bearing context goes in the commit message.

### QML

- **Christmas-tree property order**: sort properties by **total line length** (shortest line first, longest last). `id` first, blank line after. This applies to the full rendered line including the value, not just the property name.
- **Typography**: always `font: Cpp_Misc_CommonFonts.uiFont` etc. Never individual `font.*` sub-properties **except** in dashboard widgets that compute dynamic pixel sizes (e.g., zoom-dependent `font.pixelSize` + `font.family`). Use `font:` helpers wherever the size is static.
- **Reactive bindings**: expose as `Q_PROPERTY` + `NOTIFY`. No comma-expression hacks.
- **Enum access**: `SerialStudio.BusType`, `ProjectModel.SomeEnum`, `ProjectEditor.SomeEnum`.

Font helpers: `uiFont`, `boldUiFont`, `monoFont`, `customUiFont(fraction, bold)`, `customMonoFont(fraction, bold)`, `widgetFont(fraction, bold)`.
Scales: `kScaleSmall`=0.85, `kScaleNormal`=1.0, `kScaleLarge`=1.25, `kScaleExtraLarge`=1.50.

### Performance

- Hotpaths: zero-copy const refs, `[[likely]]`/`[[unlikely]]`, static-cached singletons.
- Never allocate on the dashboard path. Never copy a Frame.
- KMP for single-delimiter modes; `CircularBuffer::findFirstOfPatterns()` for multi-delimiter (single-pass byte scan, no heap allocation — stack array of ≤8 `PatInfo`).
- `constexpr` CRC tables.
- Profile first — don't optimize without data.

### Enums

Never hardcode as integers. Use `SerialStudio::BusType::UART` etc. In QML: `SerialStudio.BusType`.

### Licensing

Maintain SPDX headers: `GPL-3.0-only`, `LicenseRef-SerialStudio-Commercial`, or both.
Validate at system boundaries only (API input, file I/O, network). Trust internal data.

## Safety-Critical Code — NASA Power of Ten

Used for mission-critical telemetry. Hotpath violations are blockers.

1. **No `goto`/`setjmp`/`longjmp`.** No unbounded recursion — every recursive function has a hard depth cap. Existing limits: `FrameParser::parseMultiFrame` ≤2, `JsonValidator` ≤128, `Taskbar::findItemByWindowId` ≤3, `ConversionUtils` ≤64.
2. **Loops have fixed upper bounds.** External-data loops use explicit `kMaxIterations` guards. `while(true)` only if termination is provable from invariants (e.g. "consumes ≥1 byte per iteration of a finite buffer") — document it.
3. **No allocation after init on the hotpath.** No `new`/`make_shared`/`.append()` on the dashboard path. `FrameBuilder` creates one `TimestampedFramePtr` per parsed frame and shares it across consumers; don't add a second ownership layer.
4. **Functions 40–80 lines.** Hard limit 100. Split bigger work into named helpers.
5. **Assertion density ≥2 per function.** Pre-/post-conditions + loop invariants. `Q_ASSERT` for debug; `if (!cond) return` for release-mode safety. No `assert(true)`.
6. **Smallest scope.** Declare at first use. No function-top var blocks.
7. **Check return values at system boundaries** (driver/file/network/API). `[[nodiscard]]` everywhere. `try_enqueue()` results must be logged on failure. JS calls go through `IScriptEngine::guardedCall()`, never direct `parseFunction.call()`.
8. **Minimal preprocessor.** Only `#include`, `#pragma once`, `#ifdef BUILD_COMMERCIAL`/`ENABLE_GRPC`, platform guards. No token pasting, no variadic macros.
9. **No `reinterpret_cast`** except for byte-level access (`const uint8_t*`). Prefer `std::bit_cast`. No raw function pointers — type-safe `connect()` / `std::function`.
10. **Zero warnings.** `-Wall -Wextra -Wpedantic`. `ENABLE_HARDENING` for production. Fix root cause; never suppress without justification.

## Modbus Map Importer & File Transmission (Pro)

`DataModel::ModbusMapImporter` imports CSV/XML/JSON register maps → auto-generates Modbus project; preview in `ModbusPreviewDialog.qml`. Paired with `IO::Drivers::Modbus::generateRegisterGroupProject`. File transmission: see protocol section above.

## MCP / API Testing

TCP port 7777, MCP (JSON-RPC 2.0) + legacy.
Client: `cd "examples/MCP Client" && python3 client.py`.
~290 registered commands across 25 handlers. New handlers added in v3.3: `DataTablesHandler`,
`SessionsHandler`, `WorkspacesHandler`. See `API/CommandHandler::initializeHandlers()` for the
full list.
