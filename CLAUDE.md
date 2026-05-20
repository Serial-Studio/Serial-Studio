# CLAUDE.md

## Behavioral Rules

- **Read before writing.** Never edit a file you haven't read this session.
- **Read hotpath code in full** (`FrameBuilder`, `CircularBuffer`, `FrameReader`, `Dashboard`)
  before touching it. **Read `BluetoothLE.h/.cpp`** before writing any new driver — it's the
  canonical reference.
- **Read existing signal/slot wiring** in a file before adding or changing any.
- **Plan before multi-file changes** (>3 files): state the plan, get confirmation.
- **Edit, don't rewrite.** Targeted `Edit` calls; full rewrite only when asked or >70% changed.
- **No preamble, no trailing summary.** Lead with the action or the result.
- **Do not create markdown/doc files** unless asked. Share info conversationally.
- **Update CLAUDE.md** for any architectural change that future me would otherwise miss.
- **`scripts/` is the style contract.** When in doubt, run it; don't restate it here.

## Build

```bash
cmake -B build -DPRODUCTION_OPTIMIZATION=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

# Debug + sanitizers
cmake -B build -DDEBUG_SANITIZER=ON -DCMAKE_BUILD_TYPE=Debug
```

Flags: `ENABLE_HARDENING`, `ENABLE_PGO`, `ENABLE_GRPC`, `USE_SYSTEM_ZLIB`, `USE_SYSTEM_EXPAT`.

## Scripts

All scripts in `scripts/` are CWD-independent and write LF endings on every platform. Safe
to run from any directory.

| Script | Role |
|--------|------|
| `sanitize-commit.py` | Top-level driver: chmod (POSIX) → expand-doxygen → clang-format → code-verify --fix → clang-format → code-verify --check → documentation-verify → search-index rebuild → conventional-commit prompt. **Run before every commit.** |
| `code-verify.py` | Structural + tone linter for C++/QML/H. `--fix` rewrites in place; `--check` regenerates `.code-report`. Errors block CI; advisories are baseline-debt cleanup. |
| `documentation-verify.py` | Markdown linter for AI-narration / marketing copy. Read-only; writes `.doc-report`. Targets `README.md`, `AGENTS.md`, `doc/help/**`, `examples/**/README.md` (CLAUDE.md is exempt). |
| `expand-doxygen.py` | Rewrites single-line `/** text */` into the canonical 3-line block. |

Suppression: wrap a region in `// code-verify off` / `// code-verify on` (C++ and QML);
`<!-- doc-verify off -->` / `<!-- doc-verify on -->` (Markdown). Suppressions are a
code-review trigger — fix root cause when possible.

`.code-report` and `.doc-report` are the cleanup checklists. If a rule appears as advisory,
that means the existing codebase has baseline debt — new code should still clear it.

## Project Overview

Serial Studio: cross-platform telemetry dashboard, Qt 6.9.2 + C++20. Data sources: UART,
TCP/UDP, BLE, Audio, Modbus, CAN Bus, MQTT, USB (libusb), HID (hidapi), Process I/O. 15+
visualization widgets, 6 output (control) widgets, 256 KHz+ target data rate. Frame parsers
in JavaScript (`QJSEngine`) or Lua 5.4 (embedded `lua54`). Per-dataset value transforms in
either language. Pro features: Output widgets, Modbus, CAN Bus, MDF4, 3D, ImageView,
Waterfall, file-transfer protocols (X/Y/ZMODEM), Modbus map importer, Session Database.

## Directory Structure

```
app/src/
├── IO/              ConnectionManager, DeviceManager, CircularBuffer, FrameReader, FrameConfig
│   ├── Drivers/     UART, Network, BluetoothLE, Audio, CANBus, HID, Modbus, Process, USB
│   └── FileTransmission/  Protocol base, XMODEM, YMODEM, ZMODEM, CRC utilities
├── DataModel/       Frame, FrameBuilder, FrameConsumer, DataTable(Store), ExportSchema,
│   │                ProjectModel, ProjectEditor, NotificationCenter
│   ├── Scripting/   IScriptEngine, FrameParser, JsScriptEngine, JsWatchdog,
│   │                LuaScriptEngine, LuaCompat, ScriptTemplates
│   ├── Editors/     JsCodeEditor, OutputCodeEditor, PainterCodeEditor,
│   │                DatasetTransformEditor, CodeFormatter
│   ├── Importers/   DBCImporter, ModbusMapImporter, ProtoImporter
│   └── Dialogs/     FrameParserTestDialog, TransmitTestDialog
├── UI/              Dashboard, Taskbar (workspaces), visualization + output widget types
│   └── Widgets/Output/  Button, Toggle, Slider, TextField, Panel (+ PanelLayout), Base
├── API/             TCP server port 7777 (MCP + legacy JSON-RPC), 25 handlers
├── Console/         Terminal + export
├── CSV/ MDF4/       File playback & export
├── Sessions/  (Pro) DatabaseManager + SQLite::Export + SQLite::Player
├── MQTT/            Publisher (FrameConsumer-based, threaded, rate-limited 1-30 Hz)
├── Licensing/       LemonSqueezy, Trial, MachineID, CommercialToken (FeatureTier)
├── Platform/        CSD, NativeWindow
├── Misc/            JsonValidator, ThemeManager, ModuleManager
├── AppState.h       Singleton: OperationMode, projectFilePath, FrameConfig
├── SerialStudio.h   Central enums (BusType, OperationMode, FrameDetection)
└── Concepts.h       C++20 concepts
app/qml/             DatabaseExplorer/, MainWindow/, ProjectEditor/, Widgets/, Dialogs/
lib/                 KissFFT, QCodeEditor, mdflib, OpenSSL, lua54, QuaZip, hidapi, QSimpleUpdater
```

## Architecture — Critical Invariants

### Data Flow

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
  │ one TimestampedFramePtr per parsed frame, shared with every consumer
  ▼
Dashboard / CSV / MDF4 / API / gRPC / Sessions  (shared parsed frame)
```

### Timestamp Ownership — Source Owns Time

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

### Threading Rules — DO NOT VIOLATE

| Component | Rule |
|-----------|------|
| `FrameReader` | **Main thread.** Config set once before construction; recreate via `ConnectionManager::resetFrameReader()` / `DeviceManager::reconfigure()`. **Never add mutexes.** Single-delimiter uses KMP; multi uses `CircularBuffer::findFirstOfPatterns()` (single-pass, stack array ≤8). Preserves driver timing via `PendingChunk` spans. (Historical: threaded extraction removed in beeda4c0; if it returns, `DeviceManager::frameReady` / `rawDataReceived` go back to `Qt::QueuedConnection`.) |
| `CircularBuffer` | **SPSC only.** Driver writes from whatever thread emitted `dataReceived`; reader is FrameReader on main. Never MPMC. |
| `Dashboard` | **Main thread only.** Reads the shared `TimestampedFramePtr`. |
| Export workers | Lock-free enqueue from main; batch on worker thread. Consume the shared frame published by FrameBuilder. |

**Hotpath signal hops must be `Qt::DirectConnection`.** A queued connection between two
main-thread objects costs a `QMetaCallEvent` alloc + event-queue insertion per emit; at
10+ kHz that fills FrameReader's 4096-slot queue faster than the consumer drains and
trips `Frame queue full — frame dropped`. Known direct sites:

- `DeviceManager::frameReady → ConnectionManager::onFrameReady`
- `DeviceManager::rawDataReceived → ConnectionManager::onRawDataReceived`
- `FrameReader::readyRead → DeviceManager::onReadyRead` (AutoConnection resolves Direct)

### AppState — Single Source of Truth

`AppState` (`Cpp_AppState`) owns `operationMode`, `projectFilePath`, `frameConfig`.

- `operationMode` persists to QSettings; everything else reacts to `operationModeChanged()`.
- `frameConfig` is derived from mode + project source[0]; emits `frameConfigChanged(config)`.
- Init order: all `setupExternalConnections()` first, then `restoreLastProject()`.
- `setOperationMode()` guard-returns if unchanged.

### Operation Modes

| Mode | Delimiters | CSV delim | JS parser | Dashboard |
|------|-----------|-----------|-----------|-----------|
| ProjectFile (0) | Per-source | Via JS | Yes | Yes |
| ConsoleOnly (1) | None (short-circuits) | N/A | No | No |
| QuickPlot (2) | Line-based (CR/LF/CRLF) | Comma | No | Yes |

ConsoleOnly (replaced DeviceSendsJSON, 2026-04) bypasses CircularBuffer + queue;
`FrameBuilder::hotpathRxFrame` is a no-op; raw bytes reach the terminal via
`DeviceManager::rawDataReceived`.

### IO Architecture — No Singleton Drivers

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

### BluetoothLE — Shared Static Discovery

- Discovery state is **static** (`s_devices`, `s_discoveryAgent`, `s_adapterAvailable`),
  shared via `s_instances`. Connection state is per-instance (`m_controller`, `m_service`).
- Device list is **append-only** during rediscovery — indices stay stable.
- `selectDevice(0)` is ignored when already selected/connected (combobox-rebuild guard).
- Property access (`selectService`, characteristic select) on an instance without a
  controller forwards to the one that has it.
- `setDriverProperty()` writes raw values; `driverProperties()` returns raw internals — no
  placeholder compensation.

### ProjectModel / ProjectEditor Split

- `ProjectModel` (`Cpp_JSON_ProjectModel`): pure data — groups, actions, config, file I/O.
- `ProjectEditor` (`Cpp_JSON_ProjectEditor`): editor controller — tree model, form models,
  selection, comboboxes.
- QML enum access: `ProjectModel.SomeEnum` / `ProjectEditor.SomeEnum` — **not** `Cpp_JSON_*`.
- `groupsChanged` → `buildTreeModel()` is `Qt::QueuedConnection`; selection runs via hint
  signals afterwards.
- Title edits update the tree item in-place via `m_*Items` — never call a mutating
  `ProjectModel` function on every keystroke.

### Multi-Source Architecture

- `DataModel::Source` entries in `Frame.h`. `FrameBuilder::hotpathRxSourceFrame(sourceId, data)`
  routes per-source frames. `FrameParser` keeps one engine per `sourceId`.
- GPL: `openJsonFile()` truncates `m_sources` to 1; `addSource()` is gated by
  `BUILD_COMMERCIAL`.
- Bus type change: set `m_awaitingContextRebuild`, wait for one-shot `contextsRebuilt`, then
  `buildSourceModel`. Don't force-rebuild on selection.

### Project File JSON Keys — `Keys::` Namespace

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

### Frame Parser — Dual Language (JS + Lua)

- `IScriptEngine` is the abstraction. Two impls:
  - `JsScriptEngine` — `QJSEngine` with `ConsoleExtension + GarbageCollectionExtension`
    only (**not** `AllExtensions`). Watchdog: **always route through
    `IScriptEngine::guardedCall()`**; never call `parseFunction.call()` directly.
  - `LuaScriptEngine` — Lua 5.4 (`lib/lua/lua54`), one `lua_State*` per source.
- Per-source `frameParserLanguage` ("javascript" | "lua") picks the engine at
  `compileFrameParser()`. Templates in `app/rcc/scripts/{js,lua}/` + `templates.json`.

### Per-Dataset Value Transforms

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
- **JS watchdog is frame-level**, not per-call. `applyDatasetValues` arms
  `m_jsTransformWatchdog` once if a JS engine exists for the source, runs the dataset loop,
  and disarms it. The 100 ms budget covers the whole frame's transforms collectively.
  `applyTransformJs` no longer touches the timer.
- Non-finite numeric results are rejected (`[[unlikely]]` guarded) and `rawValue` is returned.
- **Editor**: `DatasetTransformEditor` prefills a multiline-comment placeholder when the
  dataset has no transform; `onApply` discards code that doesn't define `transform()` via
  `definesTransformFunction()` so the placeholder never persists.

### Data Tables — Central Data Bus

- `DataModel::DataTableStore`: flat pre-allocated register store, **no hotpath allocation**.
- **System table `__datasets__`**: auto-generated. Two registers per dataset:
  `raw:<uniqueId>`, `final:<uniqueId>`. Populated by FrameBuilder during parsing.
- **User tables**: defined in project JSON under `"tables"`. Registers are `Constant`
  (read-only at runtime) or `Computed` (reset per frame, writable by transforms).
- **Transform API** (injected at compile time): `tableGet`, `tableSet`, `datasetGetRaw`,
  `datasetGetFinal`. Lua = C closures; JS = `TableApiBridge` QObject.
- **Processing order**: group-array then dataset-array. A transform sees raw of ALL datasets,
  final of EARLIER datasets only.
- `applyTransform` returns `QVariant` (double or QString). `Dataset` has
  `rawNumericValue`/`rawValue` snapshots and `virtual_` (no frame index — transform-only).

### Export Architecture & Sessions DB (Pro)

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

### Other Subsystems

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
  reusing the dataset's FFT settings. Class IS the painted item (`QuickPaintedItemCompat`).
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

## Common Mistakes — Silent Breakage

These are gotchas the linter can't always catch. Rule-driven mistakes are listed once in
the style sections below — don't restate.

| Mistake | Fix |
|---------|-----|
| `QMetaObject::invokeMethod(...)` forwarding received data without capturing time | Capture `SteadyClock::now()` in the callback, pass to `publishReceivedData(data, timestamp)`. |
| Export/report worker calling `steady_clock::now()` to stamp a frame | Use `monotonicFrameNs(frame->timestamp, baseline)` on the shared `TimestampedFramePtr`. |
| `buildTreeModel()` inside an item-change handler | Defer with `QTimer::singleShot(0, ...)`. |
| Mutexes in FrameReader / CircularBuffer | Recreate via `resetFrameReader()` / `reconfigure()`. SPSC is the contract. |
| `Qt::QueuedConnection` on the frame hotpath when both ends are on main | `Qt::DirectConnection`. Queued = queue-full drops at 10+ kHz. |
| `QMap::operator[]` on `m_sourceFrames[sourceId]` | `.find()` and validate — `operator[]` silently inserts. |
| Mutating `ProjectModel` on every keystroke | Update the tree item in-place via `m_*Items`. |
| `createDriver()` for UI config | `ConnectionManager::instance().uart()` etc. |
| Force-rebuilding `buildSourceModel` on selection | Guard with `m_awaitingContextRebuild`. |
| Live driver with empty device list | Call `refreshSerialDevices()` / `refreshSerialPorts()` in `open()` if empty. |
| BLE `selectDevice(index)` with placeholder compensation in `setDriverProperty` | `setDriverProperty` is raw; `selectDevice` subtracts 1. |
| Querying the **live** driver for `configurationOk()` | Check the **UI** driver — live may not be synced yet. |
| Setter without guard return | `if (m_foo == foo) return;` before assign + emit. |
| Looking for session DB code under `app/src/SQLite/` | It's `app/src/Sessions/` — `namespace Sessions` for DatabaseManager, `namespace SQLite` for Export/Player. |
| Mixing workspace IDs with group IDs | Workspace IDs are ≥ 1000; `Taskbar::deleteWorkspace()` branches on that. |
| Stamping `current_writer_version()` on every live `Frame::serialize` | Only project saves (`ProjectModel::serializeToJson`) carry version metadata. Live frames keep `schemaVersion = 0`. |
| `Per-dataset transformCode` with a leftover placeholder | `DatasetTransformEditor::onApply` discards code that doesn't define `transform()`. |
| `std::make_shared<DataModel::TimestampedFrame>(...)` directly on the FrameBuilder hotpath | Use `acquireFrame(src)` / `acquireFrame(src, ts)`. Direct `make_shared` bypasses the slot pool and brings back per-frame heap allocs. |
| Treating `CapturedData::data` as a smart pointer (`*data->data`, `data->data->size()`, `if (!data->data)`) | It's a `QByteArray` now. Use `data->data`, `data->data.size()`, `data->data.isEmpty()`. The shared_ptr indirection was removed because `QByteArray` is already COW with atomic refcount. |
| Per-call `m_jsTransformWatchdog.start()/stop()` inside `applyTransformJs` | The watchdog is armed once per frame in `applyDatasetValues`. Don't reintroduce per-call timer churn. |
| Running heavy work synchronously inside a `QFileDialog::fileSelected` slot (open another dialog, parse a file, mutate model) | On macOS `fileSelected` fires from inside `QFileDialog::done()`, which runs from an NSSavePanel KVO callback (`ViewBridge`/`NSRemoteViewMarshal`). Re-entering Qt synchronously can leave the `WA_DeleteOnClose` dialog deleted under the panel and crash on return. **Always wrap the body in `QMetaObject::invokeMethod(this, [...] { ... }, Qt::QueuedConnection)`** so `done()` unwinds first. Same applies to slots calling `dialog->deleteLater()` — defer the *work*, not just the deletion. |

## Code Style

`scripts/code-verify.py` enforces this — read its `--check` output, don't re-derive the rules.

### Formatting

- 100-column limit, 2-space indent. Pointer/ref binds to type (`int* p`, `const Foo& r`).
- Braces: function bodies on new line, control statements on same line.
- `RemoveBracesLLVM` strips braces from single-statement bodies. `BinPackArguments` /
  `BinPackParameters` = false (one per line when wrapping).
- LF endings everywhere. ASCII-only in user-facing Markdown (CLAUDE.md and code comments
  exempt). Run `clang-format`.

### Naming

| Kind | Convention | Example |
|------|------------|---------|
| Classes / Enums | `CamelCase` | `FrameReader`, `BusType` |
| Functions | `camelCase` | `hotpathRxFrame` |
| Locals / params | `lower_case` | `frame_data` |
| Static vars | `s_lower_case` | `s_devices` |
| Private members | `m_camelCase` | `m_deviceIndex` |
| Public/protected members | `lower_case` | `sourceId` |
| Constants / constexpr | `kCamelCase` | `kMaxBufferSize` |
| Macros | `UPPER_CASE` | `BUILD_COMMERCIAL` |

### Control Flow

- **Max 3 nesting levels.** Early returns, early continues, extract functions.
- **No braces on single-statement bodies.** Blank line after a brace-free body on its own line.
- **Guard clauses** over nested error handling.
- **Functions: 40-80 lines target**, hard limit 100. Split bigger work.

```cpp
if (!frame.isValid())
  return;

for (const auto& g : frame.groups())
{
  if (!g.isEnabled())
    continue;

  processGroup(g);
}
```

### C++ Headers

Reference: `app/src/IO/Drivers/BluetoothLE.h`. Order: `Q_OBJECT` → `Q_PROPERTY` block
(clang-format off, one attribute per line) → `signals:` → private ctor + deleted copy/move
(singletons) → `public:` (`instance()` first, then `[[nodiscard]]` getters) → `public slots:`
→ `private slots:` → `private:` helpers → `private:` members.

- `[[nodiscard]]` on every non-void return.
- **Never `Q_INVOKABLE void`** — `public slots:`. `Q_INVOKABLE` is for non-void returns only.
- Christmas-tree ordering (shortest-to-longest line) within each block.
- `noexcept` on trivial const getters that only read members.
- **No in-header member init** (`int m_foo = 0;` is forbidden). Use the ctor init list.

### Signals & Connections

- `Q_EMIT`, never bare `emit`.
- `signals:` / `public slots:` / `private slots:` — never `Q_SIGNALS:` / `public Q_SLOTS:`.
- Short `connect()` on one line; long form one arg per line. Never `SIGNAL()` / `SLOT()`.
- Never `disconnect(nullptr)` as the slot — capture the `QMetaObject::Connection` and
  disconnect that.
- Never call `parseFunction.call()` directly on a `QJSEngine` parser — always
  `IScriptEngine::guardedCall()`.

### Comments & Doxygen

**Code is the spec. Comments label sections; they don't narrate.**

**Headers (.h)** — only two kinds of comments allowed:
1. SPDX banner at top.
2. `/** @brief ... */` directly above every type-level definition: classes, structs, `enum` /
   `enum class`, top-level `typedef`, top-level `using`-aliases. One `@brief` per definition
   — helper structs and payload typedefs need their own, not just the primary class.

No function doxygen above member declarations, no trailing `/**< ... */`, no multi-tag
verbose blocks, no inline `//`. Names + types are the documentation. Exempt from `@brief`:
forward declarations, nested types inside a class body, `using Base::Base;` imports, type
aliases declared inside a function body.

**Source (.cpp)** — every function definition gets a one-line `/** @brief ... */` directly
above it. Ctors, dtors, slots, helpers, every one. No `@param`/`@return`/`@note`. Use 98-dash
`//---` banners for concern groups. In-body comments: one-line `//` section headers above a
logical block, only when not self-evident. **Forbidden**: inline EOL comments, multi-line
`//` prose, `/* ... */` inside function bodies, restating the code, AI narration ("we",
"Note that", tutorial voice, "this used to...", hedging, bare `TODO`).

### QML

- **Christmas-tree property order** by **total rendered line length** (shortest first).
  `id` first, blank line after.
- **Typography**: `font: Cpp_Misc_CommonFonts.uiFont` etc. Individual `font.*` sub-properties
  only in dashboard widgets that compute dynamic pixel sizes (zoom-dependent).
- **Reactive bindings**: `Q_PROPERTY` + `NOTIFY`. No comma-expression hacks.
- **Enums**: `SerialStudio.BusType`, `ProjectModel.SomeEnum`. Never hardcoded integers.
- **No inline `//` comments mid-statement**; section headers on their own line only.

Font helpers: `uiFont`, `boldUiFont`, `monoFont`, `customUiFont(fraction, bold)`,
`customMonoFont(fraction, bold)`, `widgetFont(fraction, bold)`. Scales: `kScaleSmall=0.85`,
`kScaleNormal=1.0`, `kScaleLarge=1.25`, `kScaleExtraLarge=1.50`.

### Performance

- Hotpaths: zero-copy const refs, `[[likely]]`/`[[unlikely]]`, static-cached singletons.
- **Never allocate on the dashboard path. Never copy a Frame.**
- KMP for single-delimiter; `CircularBuffer::findFirstOfPatterns()` for multi (single-pass,
  stack array ≤8, no heap).
- `constexpr` CRC tables. Profile first.

### Licensing

SPDX headers required: `GPL-3.0-only`, `LicenseRef-SerialStudio-Commercial`, or both.
Validate at system boundaries only (API input, file I/O, network). Trust internal data.

## Safety-Critical Code — NASA Power of Ten

Mission-critical telemetry. Hotpath violations are blockers.

1. **No `goto`/`setjmp`/`longjmp`.** No unbounded recursion — every recursive function has a
   hard depth cap (`FrameParser::parseMultiFrame` ≤2, `JsonValidator` ≤128,
   `Taskbar::findItemByWindowId` ≤3, `ConversionUtils` ≤64).
2. **Loops have fixed upper bounds.** External-data loops use explicit `kMaxIterations`.
   `while(true)` only with a provable termination invariant — document it.
3. **No allocation after init on the hotpath.** No `new`/`make_shared`/`.append()` on the
   dashboard path. `FrameBuilder::acquireFrame()` draws each `TimestampedFramePtr` from a
   fixed-size slot pool (`kFramePoolSize = 1024`); the slot is recycled when the last
   consumer drops the shared_ptr (custom deleter flips `inUse` to false). Don't bypass the
   pool with a direct `std::make_shared<TimestampedFrame>(...)` on the hotpath — that
   re-introduces a per-frame heap alloc. Pool exhaustion logs once and falls back to
   `make_shared` so the producer never blocks. The perf-* advisories (`code-verify.py`)
   catch accidental hot-path allocation, regex construction, locking, logging, throwing,
   large by-value params, `shared_ptr` by-value, runtime divide/modulo, `pow()`,
   `dynamic_cast`, virtual calls, large stack buffers, false-sharing, recursion in hot loops.
4. **Functions 40-80 lines, hard limit 100.** Nesting ≤3. Split into helpers.
5. **Assertion density ≥2 per function.** Pre/post-conditions + invariants. `Q_ASSERT` for
   debug; `if (!cond) return;` for release safety. No `assert(true)`.
6. **Smallest scope.** Declare at first use. No function-top var blocks. Anonymous namespaces
   only for true file-locals.
7. **Check return values at system boundaries** (driver/file/network/API). `[[nodiscard]]`
   everywhere. `try_enqueue()` failures must be logged. JS calls go through
   `IScriptEngine::guardedCall()`, never direct.
8. **Minimal preprocessor.** Only `#include`, `#pragma once`, `#ifdef BUILD_COMMERCIAL` /
   `ENABLE_GRPC`, platform guards. No token pasting, no variadic macros.
9. **No `reinterpret_cast`** except byte-level access (`const uint8_t*`). Prefer
   `std::bit_cast`. No raw function pointers. No `dynamic_cast` on the hotpath — refactor
   to a tag or invariant-checked `static_cast`.
10. **Zero warnings.** `-Wall -Wextra -Wpedantic`, `ENABLE_HARDENING` for production. Fix
    root cause; never suppress without justification.

## MCP / API Testing

TCP port 7777, MCP (JSON-RPC 2.0) + legacy. Client: `cd "examples/MCP Client" && python3
client.py`. ~290 commands across 25 handlers. v3.3 added `DataTablesHandler`,
`SessionsHandler`, `WorkspacesHandler`. Full list in
`API/CommandHandler::initializeHandlers()`.
