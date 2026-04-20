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
  │ HAL_Driver::dataReceived(ByteArrayPtr)              AutoConnection
  ▼
FrameReader::processData  (main thread)
  │ appends to CircularBuffer (SPSC), KMP-scans for delimiters
  │ enqueues completed frames to lock-free ReaderWriterQueue<QByteArray>
  │ emits readyRead once per processData call
  ▼
DeviceManager::onReadyRead  (main thread, direct)
  │ while try_dequeue: Q_EMIT frameReady(deviceId, frame)  DirectConnection
  ▼
ConnectionManager::onFrameReady  (main thread)
  │ routes to FrameBuilder::hotpathRxFrame / hotpathRxSourceFrame
  ▼
FrameBuilder  (main thread)
  │ parses → applies per-dataset transforms → mutates m_frame
  │ hotpathTxFrame(m_frame): Dashboard::hotpathRxFrame(const Frame&)  ← 0 allocs
  │                          + make_shared<TimestampedFrame> for export workers  ← 1 alloc [[unlikely]]
  ▼
CSV / MDF4 / API::Server / gRPC  (worker threads, lock-free enqueue from main)
```

### Threading Rules — DO NOT VIOLATE

| Component | Rule |
|-----------|------|
| `FrameReader` | **Currently runs on the main thread.** Config set **once** before construction; recreate via `ConnectionManager::resetFrameReader()` / `DeviceManager::reconfigure()` on change. **Never add mutexes.** Delimiters are `QList<QByteArray>` (multi-pattern); single-delimiter uses KMP fast path, multi-delimiter uses `CircularBuffer::findFirstOfPatterns()` single-pass scan. Historical note: `m_threadedExtraction` / `moveToThread` was removed in beeda4c0; if it ever returns, flip `DeviceManager::frameReady` / `rawDataReceived` connections back to `Qt::QueuedConnection` in `ConnectionManager::wireDevice()`. |
| `CircularBuffer` | **SPSC only.** One producer, one consumer. Never MPMC. Even with FrameReader on the main thread, keep SPSC — the driver side still writes from whatever thread emitted `dataReceived`. |
| `Dashboard` | **Main thread only.** Receives `const Frame&` — no copy. |
| Export workers | Lock-free enqueue from main, batch on worker thread. `make_shared<TimestampedFrame>` is the only allocation on the tx path (guarded by `[[unlikely]]`). |

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
- `DeviceManager`: non-singleton, owns one `HAL_Driver` + `FrameReader`. No worker thread — FrameReader runs on the main thread (see beeda4c0).
- `configurationOk()` checks the **UI driver** (not the live driver). UI driver `configurationChanged` is forwarded to `ConnectionManager::configurationChanged`.
- All drivers must emit `configurationChanged()` from their constructors (connect specific signals like `portIndexChanged` → `configurationChanged`).
- Live drivers may lack enumerated device lists. UART/Modbus call `refreshSerialDevices()`/`refreshSerialPorts()` in `open()` if lists are empty.

### BluetoothLE — Shared Static Discovery

- Discovery state is **static** (`s_devices`, `s_deviceNames`, `s_discoveryAgent`, `s_localDevice`, `s_adapterAvailable`), shared across all `BluetoothLE` instances via `s_instances` list.
- Each instance maintains its own connection state (`m_controller`, `m_service`, `m_characteristics`).
- `startDiscovery()` is a no-op if already running. Device list is append-only (never cleared during rediscovery) — indices are stable.
- `selectDevice(0)` is ignored when `m_deviceIndex >= 0` or `m_deviceConnected` (prevents QML combobox rebuild from resetting selection).
- `selectService()` / `setCharacteristicIndex()`: if called on an instance without a controller/service, forwards to the instance that has one.
- `onServiceDiscoveryFinished()` / `configureCharacteristics()`: propagate service/characteristic names to all other instances with the same `m_deviceIndex` so QML can display them.
- `setDriverProperty()` sets `m_deviceIndex`/`m_selectedCharacteristic` directly (no placeholder compensation) since `driverProperties()` returns raw internal values.

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
- Source JSON keys: `title`, `sourceId`, `busType`, `frameStart`, `frameEnd`, `frameDetection`, `decoder`, `hexadecimalDelimiters`, `frameParserCode`, `connection`.

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

- `app/src/UI/Widgets/Output/`: `Button`, `Toggle`, `Slider`, `TextField`, `Panel` (+ `PanelLayout`),
  sharing a common `Base`. QML counterparts in `app/qml/Widgets/Dashboard/Output/`.
- Output widgets run user-authored JS "output scripts" (`app/rcc/scripts/output/*.js`) to convert
  UI state into bytes sent back to the device. `DataModel::OutputCodeEditor` edits the script;
  `DataModel::TransmitTestDialog` previews the serialized frame.
- Protocol helper functions (CRC, NMEA, Modbus, SLCAN, GRBL, GCode, SCPI, binary packet…) are
  injected into the engine so templates can stay short.
- Gated by `FeatureTier >= Pro` in `CommercialToken` — tier enum is
  `None=0, Hobbyist=1, Trial=2, Pro=3, Enterprise=4`.

### Workspaces (Dashboard)

- User-defined dashboard tabs that group selected widgets. Persisted in the project file under
  `"workspaces"`. `UI::Taskbar` owns the active-workspace state.
- **Workspace IDs ≥ 1000** (distinguishes them from group IDs). Group IDs stay below 1000;
  `Taskbar::deleteWorkspace(id)` routes to `ProjectModel::deleteWorkspace()` for IDs ≥ 1000 and
  `hideGroup()` for IDs ≥ 0 — do not cross-wire these branches.
- Taskbar has a filter/search bar for large projects.
- Workspace edits stage into memory + `setModified(true)` (no autosave to disk — matches
  widget-layout persistence rule).

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

Reference: `app/src/IO/Drivers/BluetoothLE.h`. Required order:

```
// clang-format off
Q_OBJECT
Q_PROPERTY(type name            ← multi-line, one attribute per line
           READ  name
           NOTIFY nameChanged)
// clang-format on

signals:                         ← immediately after Q_PROPERTY block

private:                         ← constructor + deleted copy/move (singletons)

public:                          ← instance() first, then [[nodiscard]] getters
public slots:                    ← void methods callable from QML
private slots:                   ← onXxx() handlers
private:                         ← helpers
private:                         ← member variables (separate block)
```

Key rules:
- `[[nodiscard]]` on every non-void return, including `Q_INVOKABLE` getters.
- **Never `Q_INVOKABLE void`** — use `public slots:` instead. `Q_INVOKABLE` is only for non-void returns.
- Christmas-tree ordering (shortest line→longest line) within each block.
- `noexcept` on trivial const getters that only read members.
- Non-singleton: constructor in `public:`, deleted operators right after.
- **No in-header member initialization** (e.g., `int m_foo = 0;`). Initialize in the constructor member init list.

### Source File (.cpp) Layout

Use 98-dash `//---` banners to separate concern groups. Reference: `BluetoothLE.cpp`.

```cpp
//--------------------------------------------------------------------------------------------------
// Constructor & singleton access functions
//--------------------------------------------------------------------------------------------------
```

### Signals & Connections

- Always `Q_EMIT`, never bare `emit`.
- Always `signals:`, `public slots:`, `private slots:` — never `Q_SIGNALS:`, `public Q_SLOTS:`, etc.
- Short `connect()` on one line; long form: one argument per line.
- Never use `SIGNAL()`/`SLOT()` macros.

### Comments & Doxygen

- Prefer self-documenting code. Comments explain intent ("why"/"what"), not mechanics.
- **Doxygen mandatory**: class `@brief` in `.h` above `class`; every function in `.cpp`.
- Tags: `@brief` (always), `@param` (non-trivial), `@return` (non-void). No `@author`/`@date`.

**Function body comments — the gold standard is `LemonSqueezy.cpp`:**

Every non-trivial function must have a one-line `//` section header above **each** logical
block, so a reader can skim the function and understand its flow without reading the code.
A function with 5 logical blocks needs 5 comments, not just one at the top.

| Rule | Example |
|------|---------|
| Exactly one line per comment | `// Persist connection settings and bus type into source[0]` |
| Comment every logical block, not just the first | See LemonSqueezy.cpp `readSettings()`, `activate()` |
| Skip comments for trivial functions (getters, 1–3 lines) | Doxygen is sufficient |
| No multi-line `/* */` inside function bodies | Use single `//` lines |
| No inline end-of-line comments | `x = 1; // bad` |
| No comments inside brace blocks | `if (x) { // bad` |
| Never duplicate the Doxygen | If the `@brief` says "Returns the port index", don't start the body with `// Return the port index` |
| Never state what the code literally does | `// Set m_foo to bar` ← useless. Say **why**. |
| Blank line after each comment + its code block | Separates the sections visually |

```cpp
// Good: every block commented, each comment is one short line
void LemonSqueezy::activate(const QString& key)
{
  // Skip if license key format is invalid
  if (!isValidKeyFormat(key))
    return;

  // Avoid repeat activation requests
  if (m_busy)
    return;

  // Enable busy status
  m_busy = true;
  Q_EMIT busyChanged();

  // Obtain machine ID & build JSON payload
  const auto machineId = MachineID::instance().machineHash();
  const auto fingerprint = MachineID::instance().fingerprint();

  // Generate the JSON data
  QJsonObject payload;
  payload["license_key"]  = key;
  payload["instance_name"] = fingerprint;

  // Setup network request
  QNetworkRequest request(activateUrl());
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

  // Send the activation request
  auto* reply = m_nam.post(request, QJsonDocument(payload).toJson());
  connect(reply, &QNetworkReply::finished, this, [this, reply] { onActivateReply(reply); });
}

// Bad: comment only on first block, rest left uncommented
void ConnectionManager::rebuildDevices()
{
  // Snapshot current state before rebuilding       ← good
  const auto opMode = AppState::instance().operationMode();
  const bool wasConnected = isConnected();

  bool willRebuildDevice0 = (opMode != SerialStudio::ProjectFile);  ← no comment, what is this?
  if (opMode == SerialStudio::ProjectFile) {
    ...
  }

  for (auto it = m_devices.begin(); ...) {          ← no comment, what does this loop do?
    ...
  }
}
```

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

## Safety-Critical Code — NASA Power of Ten Rules

This application is commonly used for mission-critical data acquisition.
All code changes MUST be validated against these rules (adapted from NASA JPL's Power
of Ten for C++ / Qt). Violations in hotpath code are blockers.

### Rule 1 — No Complex Control Flow

- **No `goto`, `setjmp`, `longjmp`.**
- **No unbounded recursion.** All recursive functions must have a depth parameter
  with a hard cap. Prefer iterative solutions. Document max depth at call site.
- Existing bounded recursion: `FrameParser::parseMultiFrame` (depth ≤ 2),
  `JsonValidator` (depth ≤ 128), `Taskbar::findItemByWindowId` (depth ≤ 3),
  `ConversionUtils` (depth ≤ 64).

### Rule 2 — All Loops Must Have Fixed Upper Bounds

- Every loop must have a provable upper bound. For `while(true)` loops, document
  why termination is guaranteed (e.g., "consumes ≥1 byte per iteration from a
  finite buffer").
- Loops processing external data must have explicit `kMaxIterations` guards:
  ```cpp
  constexpr int kMaxIterations = 10000;
  for (int i = 0; i < kMaxIterations && condition; ++i) { ... }
  ```

### Rule 3 — No Dynamic Memory Allocation After Initialization

- Never add `new`, `malloc`, `make_shared`, or container growth (`.append()`,
  `.push_back()`) in the dashboard hotpath. Pre-allocate in constructors.
- `FrameBuilder::hotpathTxFrame` allows one `make_shared` for export consumers
  (guarded by `[[unlikely]]`). Dashboard path must remain zero-alloc.

### Rule 4 — Functions ≤60 Lines

- Target 40–60 lines per function, hard limit 80 for new code. Split anything
  over 100 lines. Extract logical blocks into named helper functions.

### Rule 5 — Assertion Density ≥2 Per Function

- Every new or modified function must have at least 2 meaningful assertions:
  - Pre-conditions: validate parameters at entry.
  - Post-conditions: validate return values and state after mutations.
  - Loop invariants: assert expected state within loops.
- Use `Q_ASSERT` for debug-only checks. For runtime safety in release builds,
  use explicit `if (!condition) return error;` guards.
- Never use `assert(true)` or trivially-true assertions.
  ```cpp
  void processFrame(const Frame& frame, int sourceId)
  {
    Q_ASSERT(!frame.groups.empty());
    Q_ASSERT(sourceId >= 0);
    // ... processing ...
    Q_ASSERT(result.isValid());
  }
  ```

### Rule 6 — Smallest Possible Scope

- Declare variables at first use. No function-top declarations.

### Rule 7 — Check Return Values, Validate Parameters

- At system boundaries (driver I/O, file I/O, network, API input): always check
  return values. Cast to `(void)` only with documented justification.
- Every non-void function call at a system boundary must have its return value
  checked. Use `[[nodiscard]]` on all non-void returns.
- `try_enqueue()` return values must be checked and logged on failure.
- File `seek()` return values must be checked in all protocol implementations.
- `FrameParser::guardedCall()` provides a runtime watchdog timer for JS calls.
  Always use `guardedCall()` instead of direct `parseFunction.call()`.

### Rule 8 — Limited Preprocessor Use

- Preprocessor limited to `#include`, `#pragma once`, `#ifdef BUILD_COMMERCIAL`,
  `#ifdef ENABLE_GRPC`, and platform guards. No token pasting, no variadic macros.
- Keep conditional compilation minimal. Each `#ifdef` doubles the test matrix.

### Rule 9 — Restricted Pointer Use

- Avoid `reinterpret_cast` except for byte-level access (`const uint8_t*` casts).
  Prefer `std::bit_cast` (C++20) where possible.
- No multi-level pointer dereferencing. No raw function pointers (use type-safe
  `connect()` / `std::function`).

### Rule 10 — All Warnings Enabled, Zero Warnings

- Build with `-Wall -Wextra -Wpedantic` at minimum.
- Enable `ENABLE_HARDENING` flag for production builds.
- Run static analysis (clang-tidy, cppcheck) with zero warnings policy.
- Fix the root cause of any warning. Never suppress without documented justification.

### Modbus Map Importer (Pro)

- `DataModel::ModbusMapImporter` (`ModbusMapImporter.h/.cpp`): imports CSV / XML / JSON register
  maps and auto-generates a Modbus project (groups, datasets, polling). Preview UI in
  `app/qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml`.
- Paired with `IO::Drivers::Modbus` multi-group polling (see Modbus.cpp `generateRegisterGroupProject`).

### File Transmission Protocols (Pro)

- `IO::FileTransmission` controller + `IO::Protocols::{XMODEM,YMODEM,ZMODEM}` concrete protocols
  (see section above). Plain text and raw binary modes still available without Pro.

## MCP / API Testing

TCP port 7777, MCP (JSON-RPC 2.0) + legacy.
Client: `cd "examples/MCP Client" && python3 client.py`.
~290 registered commands across 25 handlers. New handlers added in v3.3: `DataTablesHandler`,
`SessionsHandler`, `WorkspacesHandler`. See `API/CommandHandler::initializeHandlers()` for the
full list.
