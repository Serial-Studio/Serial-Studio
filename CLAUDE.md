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
15+ widget types, 256 KHz+ target data rate.

## Directory Structure

```
app/src/
├── IO/          ConnectionManager, DeviceManager, CircularBuffer, FrameReader, FrameConfig
│   ├── Drivers/ UART, Network, BluetoothLE, Audio, CANBus, HID, Modbus, Process, USB
│   └── FileTransmission/ Protocol base, XMODEM, YMODEM, ZMODEM, CRC utilities
├── DataModel/   FrameBuilder, Frame, FrameConsumer, FrameParser, ProjectModel, ProjectEditor
├── UI/          Dashboard + widget types
├── API/         TCP server port 7777 (MCP + legacy JSON-RPC)
│   └── Handlers/ 20 handler classes
├── Console/     Terminal + export
├── CSV/ MDF4/   File playback & export
├── MQTT/        MQTT client
├── Licensing/   LemonSqueezy, Trial, MachineID, SimpleCrypt
├── Platform/    CSD, NativeWindow
├── Misc/        JsonValidator, ThemeManager, ModuleManager
├── AppState.h   Singleton: OperationMode, projectFilePath, FrameConfig
├── SerialStudio.h  Central enums (BusType, OperationMode, FrameDetection)
└── Concepts.h   C++20 concepts (Numeric, ByteLike, Driver, Frameable…)
app/qml/         Declarative UI
lib/             KissFFT, QCodeEditor, mdflib, OpenSSL
```

## Architecture — Critical Invariants

### Data Flow

```
Driver thread → CircularBuffer (SPSC, KMP) → main thread
Main thread   → FrameReader → FrameBuilder → Frame
              → Dashboard::hotpathRxFrame(const Frame&)      ← 0 allocs
              → [exports only] make_shared<TimestampedFrame>  ← 1 alloc
              → CSV / MDF4 / API::Server worker threads
```

### Threading Rules — DO NOT VIOLATE

| Component | Rule |
|-----------|------|
| `FrameReader` | Config set **once** before `moveToThread()`. On change: destroy and recreate via `resetFrameReader()` / `reconfigure()`. **Never add mutexes.** |
| `CircularBuffer` | **SPSC only.** One producer, one consumer. Never MPMC. |
| `Dashboard` | **Main thread only.** Receives `const Frame&` — no copy, no allocation. |
| Export workers | Lock-free enqueue from main, batch on worker thread. |

### AppState — Single Source of Truth

`AppState` singleton (`Cpp_AppState` in QML) owns `operationMode`, `projectFilePath`, `frameConfig`.
- `operationMode` persisted to QSettings; others react to `operationModeChanged()`.
- `frameConfig` derived from mode + project source[0]; emits `frameConfigChanged(config)`.
- Init order: all `setupExternalConnections()` first, then `restoreLastProject()`.
- `setOperationMode()` guard-returns if unchanged — no duplicate signals.

### Operation Modes

| Mode | Delimiters | CSV delimiter | JS parser |
|------|-----------|---------------|-----------|
| ProjectFile (0) | Configurable per-source | Via JS | Yes |
| DeviceSendsJSON (1) | Fixed `/*` `*/` | N/A | No |
| QuickPlot (2) | Line-based (CR/LF/CRLF) | Comma | No |

### IO Architecture — No Singleton Drivers

- All 9 drivers have **public constructors**, no `static instance()`.
- `ConnectionManager` (singleton, `Cpp_IO_Manager`) owns one **UI-config instance** per type.
- Accessors: `ConnectionManager::instance().uart()`, `.network()`, `.bluetoothLE()`, etc.
- `createDriver()` makes fresh instances for live connections (owned by `DeviceManager`).
- QML context properties (`Cpp_IO_Serial`, `Cpp_IO_Network`, etc.) → UI-config instances.
- `DeviceManager`: non-singleton, owns one `HAL_Driver` + `FrameReader` + `QThread`.
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
| Mutexes in FrameReader/CircularBuffer | Recreate via `resetFrameReader()`/`reconfigure()` |
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

## Code Style

### Formatting

100-column limit, 2-space indent, Allman braces, pointer binds to name. Run `clang-format`.

### Naming

| Kind | Convention | Example |
|------|------------|---------|
| Classes | `CamelCase` | `FrameReader` |
| Functions | `camelCase` | `hotpathRxFrame` |
| Locals / params | `lower_case` | `frame_data` |
| Private members | `m_camelCase` | `m_deviceIndex` |
| Public/protected members | `lower_case` | `sourceId` |
| Constants | `kCamelCase` | `kMaxBufferSize` |
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

for (const auto &g : frame.groups())
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
- `[[nodiscard]]` on every non-void return.
- Christmas-tree ordering (shortest→longest) within each block.
- `noexcept` on trivial const getters that only read members.
- Non-singleton: constructor in `public:`, deleted operators right after.

### Source File (.cpp) Layout

Use 98-dash `//---` banners to separate concern groups. Reference: `BluetoothLE.cpp`.

```cpp
//--------------------------------------------------------------------------------------------------
// Constructor & singleton access functions
//--------------------------------------------------------------------------------------------------
```

### Signals & Connections

- Always `Q_EMIT`, never bare `emit`.
- Short `connect()` on one line; long form: one argument per line.
- Never use `SIGNAL()`/`SLOT()` macros.

### Comments & Doxygen

- Prefer self-documenting code. Comments explain intent ("why"/"what"), not mechanics.
- **Function body comments**: one-line `//` section headers above each logical block, separated by blank lines. No multi-line `/* */` inside function bodies. No inline end-of-line comments. No comments inside brace blocks (e.g., `if (x) { // do thing }`).
- **Doxygen mandatory**: class `@brief` in `.h` above `class`; every function in `.cpp`.
- Tags: `@brief` (always), `@param` (non-trivial), `@return` (non-void). No `@author`/`@date`.

```cpp
// Good: section-header comments, one per logical block
void ExportWorker::processItems(const std::vector<ExportDataPtr>& items)
{
  // No items, abort
  if (items.empty())
    return;

  // No device connected, abort
  if (!IO::ConnectionManager::instance().isConnected())
    return;

  // No file open, create a new one
  if (!isResourceOpen())
    createFile();

  // Write output to file
  if (m_textStream.device())
  {
    for (const auto& dataPtr : items)
      m_textStream << dataPtr->data;

    m_textStream.flush();
  }
}
```

### QML

- **Christmas-tree property order** (shortest→longest). `id` first, blank line after.
- **Typography**: always `font: Cpp_Misc_CommonFonts.uiFont` etc. Never individual `font.*` sub-properties.
- **Reactive bindings**: expose as `Q_PROPERTY` + `NOTIFY`. No comma-expression hacks.
- **Enum access**: `SerialStudio.BusType`, `ProjectModel.SomeEnum`, `ProjectEditor.SomeEnum`.

Font helpers: `uiFont`, `boldUiFont`, `monoFont`, `customUiFont(fraction, bold)`, `customMonoFont(fraction, bold)`, `widgetFont(fraction, bold)`.
Scales: `kScaleSmall`=0.85, `kScaleNormal`=1.0, `kScaleLarge`=1.25, `kScaleExtraLarge`=1.50.

### Performance

- Hotpaths: zero-copy const refs, `[[likely]]`/`[[unlikely]]`, static-cached singletons.
- Never allocate on the dashboard path. Never copy a Frame.
- KMP for delimiters, `constexpr` CRC tables.
- Profile first — don't optimize without data.

### Enums

Never hardcode as integers. Use `SerialStudio::BusType::UART` etc. In QML: `SerialStudio.BusType`.

### Licensing

Maintain SPDX headers: `GPL-3.0-only`, `LicenseRef-SerialStudio-Commercial`, or both.
Validate at system boundaries only (API input, file I/O, network). Trust internal data.

## MCP / API Testing

TCP port 7777, MCP (JSON-RPC 2.0) + legacy.
Client: `cd "examples/MCP Client" && python3 client.py`.
182 tools, 2 resources.
