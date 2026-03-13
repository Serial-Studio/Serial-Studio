# CLAUDE.md

Guidance for Claude Code when working in this repository.

## How Claude Should Work in This Repo

These rules govern Claude's behavior before writing any code.

### Always read before writing

**Never suggest or write code for a file you have not read in the current session.**
If a user asks to change, fix, or extend something, read the relevant file(s) first — even
if you think you know what's there. This prevents stale assumptions from corrupting edits.

**Before writing any new driver**, read `app/src/IO/Drivers/BluetoothLE.h` and
`app/src/IO/Drivers/BluetoothLE.cpp` in full. They are the canonical reference.

**Before touching any hotpath code** (FrameBuilder, CircularBuffer, FrameReader, Dashboard),
read the complete current implementation first. Hotpath bugs are silent and hard to profile.

**Before adding or changing any Qt signal/slot connection**, read how the existing connections
in that file are wired. Incorrect connection order or type (direct vs. queued) causes races
or missed signals that are very hard to debug.

### Plan before multi-file changes

For any change touching more than 3 files, state the full plan (which files change and why)
before writing any code. Get confirmation if the scope is unclear.

### Edit, don't rewrite

Prefer targeted `Edit` calls over full-file rewrites. Rewrites destroy git blame, risk
introducing drift from the current implementation, and make review harder. Only rewrite
a file when the user explicitly asks or when >70% of the file changes.

### No response preamble, no trailing summary

Do not narrate what you are about to do ("I'll now read the file…"). Do not summarize
what you just did at the end of a response ("I've updated X, Y, and Z…"). The diff is
self-evident. Lead with the result or the first action.

## Project Overview

Serial Studio is a cross-platform telemetry dashboard built with Qt 6.9.2 and C++20.
It receives data from UART, TCP/UDP, Bluetooth LE, Audio, Modbus TCP/RTU, CAN Bus, MQTT,
Raw USB (libusb bulk/isochronous), HID devices (hidapi), and Process I/O (stdout or named pipe),
then visualizes it in real-time with 15+ widget types. Target data rate: 256 KHz+.

## Build

```bash
# Release
cmake -B build -DPRODUCTION_OPTIMIZATION=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

# Debug + sanitizers
cmake -B build -DDEBUG_SANITIZER=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)
```

Key CMake flags: `ENABLE_HARDENING`, `ENABLE_PGO` (3-stage: GENERATE → run → USE),
`USE_SYSTEM_ZLIB`, `USE_SYSTEM_EXPAT` (required for Flathub).

## Directory Structure

```
app/src/
├── IO/          HAL drivers + CircularBuffer + FrameReader + ConnectionManager + DeviceManager
│   └── Drivers/ UART, Network, BluetoothLE, Audio, CANBus, HID, Modbus, Process, USB
├── DataModel/   FrameBuilder, Frame, FrameConsumer, FrameParser, ProjectModel, ProjectEditor
│               DBCImporter, JsCodeEditor, FrameParserTestDialog
├── UI/          Dashboard + 15+ widget types
├── API/         TCP server port 7777 (MCP + legacy JSON-RPC)
│   └── Handlers/ 20 handler classes (one per domain)
├── Console/     Terminal handler + export
├── CSV/ MDF4/   File playback & export
├── MQTT/        MQTT client
├── Licensing/   LemonSqueezy, Trial, MachineID, SimpleCrypt (Commercial)
├── Platform/    CSD (client-side decorations), NativeWindow
├── Misc/        JsonValidator, ThemeManager, ModuleManager, utilities
├── AppState.h   Singleton: OperationMode, projectFilePath, FrameConfig (source of truth)
├── SerialStudio.h Central enums (BusType, OperationMode, FrameDetection)
└── Concepts.h   C++20 concepts (Numeric, ByteLike, Driver, Frameable…)
app/qml/         Declarative UI
lib/             KissFFT, QCodeEditor, mdflib, OpenSSL
```

## Architecture

### Critical Files

| File | Purpose |
|------|---------|
| `app/src/SerialStudio.h` | Central enums (BusType, OperationMode, FrameDetection) |
| `app/src/Concepts.h` | C++20 concepts (Numeric, ByteLike, Driver, Frameable…) |
| `app/src/AppState.h` | Singleton: owns OperationMode, project file path, and derived FrameConfig for device 0. Single source of truth for mode. |
| `app/src/IO/CircularBuffer.h` | Lock-free SPSC ring buffer with KMP delimiter search |
| `app/src/IO/FrameReader.h` | Immutability-based thread safety (NO mutexes) |
| `app/src/IO/FrameConfig.h` | Plain aggregate: startSequence, finishSequence, checksumAlgorithm, operationMode, frameDetection. Passed from AppState → ConnectionManager → DeviceManager → FrameReader. |
| `app/src/IO/HAL_Driver.h` | Abstract driver interface; `ByteArrayPtr = shared_ptr<const QByteArray>` |
| `app/src/DataModel/FrameBuilder.cpp` | Hotpath: zero-copy to Dashboard, single alloc to exports. `hotpathRxSourceFrame(sourceId, data)` for multi-source. |
| `app/src/DataModel/FrameConsumer.h` | Lock-free worker template (dual-trigger flush) |
| `app/src/DataModel/ProjectModel.cpp` | Pure data model: groups, actions, config, file I/O, serialization. `m_widgetSettings` is the single store for all UI persistence. |
| `app/src/DataModel/ProjectEditor.cpp` | Editor controller: tree model, form models (group/dataset/action/project), selection state, combobox data, all item-change handlers. |

### Data Flow

```
Driver thread  →  CircularBuffer (SPSC, KMP)  →  main thread
Main thread    →  FrameReader → FrameBuilder → Frame
               →  Dashboard::hotpathRxFrame(const Frame&)   ← 0 allocs
               →  [exports only] make_shared<TimestampedFrame>  ← 1 alloc
               →  CSV / MDF4 / API::Server worker threads
```

### AppState — Single Source of Truth for OperationMode

`AppState` (singleton, `app/src/AppState.h`) is the authoritative owner of:
- **`operationMode`** — persisted to QSettings (`"operation_mode"`); other modules react to its `operationModeChanged()` signal.
- **`projectFilePath`** — persisted to QSettings (`"project_file_path"`); emits `projectFileChanged()` when it changes.
- **`frameConfig`** — derived from mode + project source[0]; emits `frameConfigChanged(config)` to reconfigure device 0.

QML context property: `Cpp_AppState`.

**Initialization order** (enforced by `ModuleManager`):
1. All `setupExternalConnections()` calls (wires signals between all singletons).
2. `AppState::restoreLastProject()` — loads saved project AFTER all listeners are wired, so FrameBuilder and Dashboard receive the first `jsonFileChanged` signal correctly.

**Frame config derivation rules (`deriveFrameConfig()`):**
- `ProjectFile` mode + source[0] exists → use source[0] delimiters, checksum, and frameDetection.
- `ProjectFile` mode + no sources → use `/*` / `*/`, no checksum, project frameDetection.
- `DeviceSendsJSON` or `QuickPlot` → always `/*` / `*/`, no checksum, `EndDelimiterOnly`.

**Signal flow on project load:**
1. `ProjectModel::jsonFileChanged` → `AppState::onProjectLoaded()`
2. Path updated + persisted, `projectFileChanged()` emitted.
3. `FrameBuilder::syncFromProjectModel()` called.
4. New FrameConfig derived, `frameConfigChanged(config)` emitted → `ConnectionManager` reconfigures device 0.

`setOperationMode(mode)` guard-returns if mode is unchanged — no duplicate signal emissions.

### Threading Rules — DO NOT VIOLATE

| Component | Rule |
|-----------|------|
| `FrameReader` | Config set **once** before `moveToThread()`. On settings change, call `IO::ConnectionManager::resetFrameReader()` (device 0) or `DeviceManager::reconfigure()` (other devices) to destroy and recreate. **Never add mutexes.** |
| `CircularBuffer` | **SPSC only.** One producer thread, one consumer thread. Never use as MPMC. |
| `Dashboard` | Updated on **main thread only**. Receives `const Frame&` — no copy, no allocation. |
| Export workers | Lock-free enqueue from main thread, batch-processed on worker thread. |

### Operation Modes & Delimiters

| Mode | Value | Frame delimiters | CSV delimiter | JS parser |
|------|-------|-----------------|---------------|-----------|
| ProjectFile | 0 | Configurable (`frameStart`/`frameEnd`/`frameDetection`) | Via JS | ✅ |
| DeviceSendsJSON | 1 | Fixed `/*` `*/` | N/A (JSON) | ❌ |
| QuickPlot | 2 | None (line-based CR/LF/CRLF) | Comma only | ❌ |

Delimiter stack: `frameEnd` → optional `lineTerminator` → `parseCsvValues()` (comma, hardcoded).
Custom delimiters require ProjectFile mode + JavaScript `frameParser`.

### ProjectModel / ProjectEditor split

`ProjectModel` is a pure data model (groups, actions, config, file I/O). `ProjectEditor` is the editor controller owning the Qt item models and all UI state. QML accesses data ops via `Cpp_JSON_ProjectModel` (context property) and editor state via `Cpp_JSON_ProjectEditor` (context property). Enum values are accessed via the registered QML type names `ProjectModel` and `ProjectEditor`.

Signal flow for add/delete/duplicate operations:
1. `ProjectModel` emits `groupsChanged()`/`actionsChanged()` + a targeted hint signal (`groupAdded(id)`, `datasetAdded(groupId, datasetId)`, etc.)
2. `buildTreeModel()` fires via `Qt::QueuedConnection` on `groupsChanged`/`actionsChanged`
3. The hint-signal handler fires (also queued) — searches the freshly built item maps and calls `setCurrentIndex()` → `onCurrentSelectionChanged()` → correct view + form model

### Multi-Source (Multi-Device) Architecture

Each project can have multiple `DataModel::Source` entries (`Frame.h`). Key rules:

| Component | Rule |
|-----------|------|
| `IO::ConnectionManager` | Singleton orchestrator. Owns `m_devices` map of `DeviceManager*` for live connections, `m_editingDrivers` map for ProjectEditor editing instances, and one **UI-config driver** per type (`m_uartUi`, `m_networkUi`, `m_bluetoothLEUi`, etc.) that QML binds to. Accessors: `uart()`, `network()`, `bluetoothLE()`, `audio()`, `canBus()`, `hid()`, `modbus()`, `process()`, `usb()` — all return raw pointers, never open a connection. Registered as `Cpp_IO_Manager`. |
| `IO::DeviceManager` | Non-singleton: owns one `unique_ptr<HAL_Driver>` + one `FrameReader` + one `QThread`. Emits `frameReady(deviceId, frame)` and `rawDataReceived(deviceId, data)`. Created fresh for every connection. |
| **IO drivers** | **No singletons.** `UART`, `Network`, `BluetoothLE`, `Audio`, `CANBus`, `HID`, `Modbus`, `Process`, `USB` all have public constructors and no `static instance()`. Three populations: (1) UI-config instances owned by `ConnectionManager`; (2) live-connection instances owned by `DeviceManager` via `createDriver()`; (3) editing instances owned by `m_editingDrivers`. QML context properties (`Cpp_IO_Serial`, `Cpp_IO_Network`, etc.) point to the UI-config instances. |
| `IO::FrameConfig` | Plain aggregate carrying FrameReader config (startSequence, finishSequence, checksumAlgorithm, operationMode, frameDetection). Passed to DeviceManager at construction. |
| `FrameBuilder` | `hotpathRxSourceFrame(sourceId, data)` routes per-source frames through `m_sourceFrames[]` → `hotpathTxMergedFrame()`. Source 0 uses this path in `ProjectFile` mode. |
| `FrameParser` | Per-source JS engine map (`QMap<int, SourceEngine*>`) keyed by `sourceId`. `parseMultiFrame(frame, sourceId)` resolves the correct engine by sourceId directly (NOT by code hash). |
| `FrameReader` | Must be configured with per-source `frameStart`/`frameEnd`/`checksumAlgorithm`/`frameDetection` before `moveToThread()`. |
| GPL enforcement | `ProjectModel::openJsonFile()` truncates `m_sources` to 1 when `#ifndef BUILD_COMMERCIAL` and shows an info message box. `addSource()` is gated likewise. The `sources.size() > 1` guard in form builders naturally makes the source selector invisible on GPL — no `#ifdef` needed in UI code. |
| Setup.qml | Shows a redirect panel (instead of hardware config) when `sourceCount > 1 && operationMode == ProjectFile`. |
| API | `project.source.*` commands via `API::Handlers::SourceHandler`. `add/delete/update` are Commercial-only; others are GPL. |
| Source JSON keys | `"title"`, `"sourceId"`, `"busType"`, `"frameStart"`, `"frameEnd"`, `"frameDetection"`, `"decoder"`, `"hexadecimalDelimiters"`, `"frameParserCode"`, `"connection"` (opaque driver settings object). |
| `connectionSettings` keys (Network) | `"socketTypeIndex"` (0=TCP, 1=UDP), `"address"`, `"tcpPort"`, `"udpLocalPort"`, `"udpRemotePort"`. |

### Stable Device Identifiers (`__deviceId__`)

Drivers that enumerate physical devices (UART, Modbus RTU, BLE, USB, HID) implement two virtual methods on `HAL_Driver`:

- `QJsonObject deviceIdentifier() const` — returns stable hardware metadata (VID/PID/serial for USB-based devices, address/name for BLE) for the currently selected device.
- `bool selectByIdentifier(const QJsonObject& id)` — searches the current device list for a match and auto-selects it. Returns `false` if no match is found (the user can still select any device manually).

The identifier is stored under the `"__deviceId__"` key inside `Source::connectionSettings`. It is saved by `captureSourceSettings()` and restored by `restoreSourceSettings()` (after applying all driver properties, so the identifier-based match overrides any stale index). The same logic runs in `ConnectionManager::driverForEditing()` and `rebuildDevices()`.

**Matching strategy by bus type:**

| Bus type | Primary match | Fallback match | Notes |
|----------|--------------|----------------|-------|
| UART | VID+PID+serial (score 150) | description (10), portName (5) | `portName` differs per OS; VID/PID/serial from `QSerialPortInfo` |
| Modbus RTU | VID+PID+serial (score 150) | description (10), portName (5) | Same as UART; only active when `protocolIndex == 0` |
| BLE | address (100) | name (10) | On macOS, addresses are randomized — `name` is the primary identifier. If the device is not found, the identifier is saved in `m_pendingIdentifier` and `startDiscovery()` is called. `onDeviceDiscovered()` checks new devices against the pending identifier. |
| USB | VID+PID+serial | VID+PID only | Serial read via `libusb_get_string_descriptor_ascii()` |
| HID | VID+PID+serial | VID+PID only | Serial from `hid_device_info::serial_number` |

**Key design rule:** The saved identifier is always a *hint*, never a constraint. If the device is not found, the user retains full control of the device selector. The identifier never blocks connections or forces project recreation.

**Editing driver refresh:** When UI-config drivers emit device-list-change signals (`availablePortsChanged`, `devicesChanged`, `deviceListChanged`), `ConnectionManager` clears `m_editingDrivers` and emits `deviceListRefreshed()`. `ProjectEditor::buildSourceModel()` connects to this signal to rebuild the form with fresh ComboBox options.

### ProjectEditor Form Model Patterns

Consistent patterns used across all form builders (`buildGroupModel`, `buildDatasetModel`, `buildSourceModel`):

- **Title editing** — update tree item text in-place via `m_*Items` map + emit `selectedTextChanged()`. Do NOT call a mutating `ProjectModel` function that emits `*Changed` on every keystroke — it triggers tree rebuild → re-selection → form rebuild, breaking cursor position.
- **`#ifdef BUILD_COMMERCIAL` in form builders** — only needed for features that must not compile at all on GPL (e.g. `addSource`). Source/device selectors that are naturally hidden when `sources.size() == 1` do NOT need `#ifdef` — the size guard is sufficient because GPL truncates sources to 1 on load.
- **Bus type change** — sets `m_awaitingContextRebuild = true`, connects one-shot to `ConnectionManager::contextsRebuilt`, clears flag and calls `buildSourceModel` inside the lambda. The `sourcesChanged` lambda skips form rebuild while this flag is set to prevent the stale driver flashing in.
- **`onCurrentSelectionChanged` source guard** — if already in `SourceView` for the same `sourceId`, return early to prevent unnecessary `buildSourceModel` calls triggered by tree item text updates.

### ProjectEditor Tree View Roles

| Role | UserRole | QML name | Set on |
|------|----------|----------|--------|
| `TreeViewIcon` | +1 | `treeViewIcon` | all items |
| `TreeViewText` | +2 | `treeViewText` | all items |
| `TreeViewExpanded` | +3 | `treeViewExpanded` | group/source items |
| `TreeViewFrameIndex` | +4 | `treeViewFrameIndex` | dataset items (≥0), -1 otherwise |
| `TreeViewSourceName` | +14 | `treeViewSourceName` | source items only (non-empty when multi-source); empty on all other items |
| `TreeViewSourceId` | +15 | `treeViewSourceId` | source/group/dataset items; 0-based |

Tree badge rendering in `ProjectStructure.qml`:
- **`[A]` / `[B]` source badge** — shown on source items only (`treeViewSourceName !== ""`). Text = `"[" + String.fromCharCode(65 + treeViewSourceId) + "]"`. Groups and datasets have `treeViewSourceName = ""` so this badge is always hidden on them.
- **`[A-1]` frame index** — shown on dataset items only (`depth > 1 && treeViewFrameIndex >= 0`). Text = `"[" + String.fromCharCode(65 + treeViewSourceId) + "-" + treeViewFrameIndex + "]"`. Both badges use monospace font, opacity 0.7.

### ProjectEditor: Project Root View

The project root form (`buildProjectModel`) shows **only Project Information** (title field). Frame detection, delimiter, checksum, and decoder settings are **per-source** and live in the Source view (`buildSourceModel`) — they must NOT be added back to the project root.

### ProjectEditor: Dataset General Section Row Order

In `addGeneralSection()`, the row order within General Information is:
1. Dataset Title
2. Input Device (combobox) — only shown when `sources.size() > 1`
3. Frame Index
4. Measurement Unit

### ProjectToolbar: Bus Type Selector (removed)

The bus type toolbar was removed from `ProjectToolbar.qml`. Bus type is now changed via the Source view form model (ComboBox row in `buildSourceModel`).

### Common Mistakes to Avoid

These are the recurring errors that look correct but break things silently:

| Mistake | Why it breaks | Correct approach |
|---------|--------------|-----------------|
| Calling `buildTreeModel()` directly inside an item-change handler | Destroys the model while Qt is still iterating it | Use `QTimer::singleShot(0, ...)` |
| Adding mutexes to `FrameReader` or `CircularBuffer` | Violates the immutability/SPSC contract; introduces deadlock risk | Recreate via `resetFrameReader()` or `reconfigure()` |
| Using `QMap::operator[]` on `m_sourceFrames[sourceId]` | Silently inserts a default entry for invalid sourceIds | Use `.find()` and validate first |
| `disconnect(nullptr)` as the slot argument | Disconnects ALL connections from that signal, not just the lambda | Capture the `QMetaObject::Connection` and disconnect it specifically |
| Calling a mutating `ProjectModel` function on every title keystroke | Triggers tree rebuild → re-selection → form rebuild → cursor jumps | Update tree item text in-place via `m_*Items` map only |
| Writing `emit` instead of `Q_EMIT` | Breaks grep, may conflict with other frameworks | Always `Q_EMIT` |
| Setting `font.pixelSize` etc. in QML | Partially overrides the font, breaks typography system | Assign whole font: `font: Cpp_Misc_CommonFonts.uiFont` |
| Hardcoding bus type as integer `0`, `1`, etc. | Silent mismatch when enum order changes | Use `SerialStudio::BusType::UART` etc. |
| Accessing a driver via `createDriver()` for UI config | Creates a live connection instance, not a UI-config instance | Use `ConnectionManager::instance().uart()` etc. |
| Force-rebuilding `buildSourceModel` on every selection change | Causes the stale driver to flash in during bus-type change | Guard with `m_awaitingContextRebuild` flag |

### Known Architectural Debt

- **~27 singletons** — high coupling, hard to test (IO drivers are no longer singletons)
- **Large widgets** — Terminal (1,708), GPS (1,417), Plot3D (1,521) lines
- **No automated tests** — no GTest/Catch2 yet (integration tests via Python pytest + API server)
- **Live-connection driver instances share QSettings** — `DeviceManager` creates fresh drivers via `createDriver()` which calls the public constructor. That constructor reads `QSettings` (saved baud rate, port index, etc.) — same as the UI-config instance does. This means a live-connection UART driver will pick up whatever the UI has saved, which is correct for single-source but worth knowing for multi-source setups where each source has its own `connectionSettings` applied via `setDriverProperty()` after construction.

## Code Style

### Formatting (enforced by clang-format)

- 100-column limit, 2-space indent, Allman braces, pointer binds to name
- Run `clang-format` before committing

### Naming

| Kind | Convention | Example |
|------|------------|---------|
| Classes | `CamelCase` | `FrameReader` |
| Functions | `camelCase` | `hotpathRxFrame` |
| Local variables / parameters | `lower_case` | `frame_data` |
| Private member variables | `m_lowerCase` (prefix + camelCase) | `m_deviceIndex` |
| Public / protected member variables | `lower_case` | `sourceId` |
| Constants / constexpr | `kCamelCase` | `kMaxBufferSize` |
| Macros | `UPPER_CASE` | `BUILD_COMMERCIAL` |

**`m_` prefix is mandatory for all private member variables.** Never use bare `lower_case_`
(trailing underscore only) — the `m_` prefix is the house style.

### Control Flow — The Most Important Rules

**Maximum nesting depth: 3 levels.** Use early returns and early continues instead.

❌ Bad:
```cpp
void process(const Frame &frame)
{
  if (frame.isValid())
    for (const auto &g : frame.groups())
      if (g.isEnabled())
        for (const auto &d : g.datasets())
          if (d.hasValue())            // level 5 — unacceptable
            doWork(d);
}
```

✅ Good:
```cpp
void process(const Frame &frame)
{
  if (!frame.isValid())
    return;

  for (const auto &g : frame.groups())
  {
    if (!g.isEnabled())
      continue;

    processGroup(g);
  }
}

void processGroup(const Group &g)
{
  for (const auto &d : g.datasets())
  {
    if (d.hasValue())
      doWork(d);
  }
}
```

**Single-statement bodies: no braces.**

❌ `if (x) { return true; }`
✅ `if (x) return true;`

**Single-statement bodies on their own line: always follow with a blank line.**

When the body of an `if`, `for`, `while`, or `else` is on a separate line (brace-free),
add a blank line after it before the next sibling statement. This is mandatory — it
makes the boundary of the braces-free body unambiguous at a glance.

❌ Bad:
```cpp
if (!frame.isValid())
  return;
doWork(frame);
```

✅ Good:
```cpp
if (!frame.isValid())
  return;

doWork(frame);
```

❌ Bad (multiple consecutive brace-free ifs):
```cpp
if (s["interpolate"] !== undefined)
  root.interpolate = s["interpolate"]
if (s["showArea"] !== undefined)
  root.showArea = s["showArea"]
```

✅ Good:
```cpp
if (s["interpolate"] !== undefined)
  root.interpolate = s["interpolate"]

if (s["showArea"] !== undefined)
  root.showArea = s["showArea"]
```

The one exception is guard-clause chains at the top of a function where all branches
return/continue and the intent is visually obvious from context.

**Functions: target 40–80 lines.** Split anything over 100 lines into named helpers.

**Guard clauses over nested error handling:**

❌ `if (ok) { if (valid) { if (!empty) { doWork(); } } }`
✅ `if (!ok || !valid || empty) return; doWork();`

### C++ Header Structure — Canonical Layout

**`app/src/IO/Drivers/BluetoothLE.h` is the exemplary reference file.** When writing or
reviewing any `.h` file, match its exact section order and conventions.

#### Required section order inside a class

```
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(…)
  // clang-format on

signals:
  …

private:                          // singleton: constructor + deleted copy/move + operators
  explicit ClassName();
  ClassName(ClassName&&)                 = delete;
  ClassName(const ClassName&)            = delete;
  ClassName& operator=(ClassName&&)      = delete;
  ClassName& operator=(const ClassName&) = delete;

public:
  static ClassName& instance();   // singleton accessor first

  // overrides / interface methods
  void close() override;

  [[nodiscard]] …                 // const / query functions — all [[nodiscard]]

public slots:
  …                               // intentional side-effects invocable from QML/signals

private slots:
  …                               // internal signal handlers (named on…)

private:                          // private helpers (non-slot)
  void helperFoo();

private:                          // member variables — separate private: block
  int  m_foo;
  bool m_bar;
  …
```

**`signals:` immediately follows the `Q_PROPERTY` block** — it is the first named access
specifier after the macro block. Never put anything between the `clang-format on` line and
`signals:`.

#### `Q_PROPERTY` formatting

Every `Q_PROPERTY` is written in multi-line form with the macro keywords vertically aligned,
wrapped in `// clang-format off` / `// clang-format on`. One attribute per line:

```cpp
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(int deviceCount
             READ deviceCount
             NOTIFY devicesChanged)
  Q_PROPERTY(int deviceIndex
             READ  deviceIndex
             WRITE selectDevice
             NOTIFY devicesChanged)
  Q_PROPERTY(bool operatingSystemSupported
             READ operatingSystemSupported
             CONSTANT)
  // clang-format on
```

Never write `Q_PROPERTY` on a single line. The `READ`, `WRITE`, `NOTIFY`, and `CONSTANT`
keywords must each appear on their own indented line, left-aligned under the opening
parenthesis of the type.

Rules derived from the exemplary file:

1. **`signals:` comes immediately after the `Q_PROPERTY` block** — it is the first named
   access specifier. QML/connections readers scan signals before constructors or methods.
2. **Constructor + deleted operators in their own `private:` block** (singleton classes).
   Non-singleton classes put the constructor in `public:` but still group deleted
   operators immediately after.
3. **`static ClassName& instance()` is the first line of the `public:` block** for singletons.
4. **Christmas-tree function declarations** — within each access-specifier block, order
   declarations shortest line first, longest line last (same principle as QML properties).
5. **`[[nodiscard]]` on every non-`void` function** — getters, queries, `open()`, `write()`,
   `isOpen()`, etc. Any function that returns a value callers must not silently discard.
6. **`void` functions are slots** — if a `public` or `private` method returns `void` and
   reacts to an event or is callable from QML, it lives in `public slots:` or
   `private slots:`, not in a bare `public:`/`private:` block.
7. **`public slots:` and `private slots:` are separate sections** — never mix them into a
   single `slots:` block.
8. **Private helpers and private member variables are separate `private:` blocks** —
   helpers first, then variables. This makes the variable list easy to scan in isolation.

❌ Bad (single-line `Q_PROPERTY`, mixed, no `[[nodiscard]]`, slots in wrong section):
```cpp
class Foo : public QObject {
  Q_OBJECT
  Q_PROPERTY(int count READ count NOTIFY countChanged)   // ← single-line: forbidden
public:
  static Foo& instance();
  explicit Foo();
  bool isOpen();        // missing [[nodiscard]]
  void startWork();     // slot buried in public:
  int  count();         // missing [[nodiscard]]
private:
  void onData();        // private slot mixed with helpers
  void helper();
  int  m_count;
  bool m_open;
};
```

✅ Good (matches BluetoothLE.h pattern):
```cpp
class Foo : public QObject {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(int  count
             READ  count
             NOTIFY countChanged)
  Q_PROPERTY(bool isOpen
             READ  isOpen
             NOTIFY openChanged)
  // clang-format on

signals:
  void countChanged();

private:
  explicit Foo();
  Foo(Foo&&)            = delete;
  Foo(const Foo&)       = delete;
  Foo& operator=(Foo&&) = delete;
  Foo& operator=(const Foo&) = delete;

public:
  static Foo& instance();

  [[nodiscard]] int  count() const noexcept;
  [[nodiscard]] bool isOpen() const noexcept;

public slots:
  void startWork();

private slots:
  void onData(const QByteArray& bytes);

private:
  void helper();

private:
  bool m_open;
  int  m_count;
};
```

### QML Property Order — Christmas Tree

Within each QML object, order properties shortest line first, longest line last (the "Christmas tree" principle). `id` is always first, separated from the remaining properties by a blank line.

❌ Bad:
```qml
Rectangle {
  anchors.fill: parent
  id: myRect
  color: "red"
  width: 10
}
```

✅ Good:
```qml
Rectangle {
  id: myRect

  width: 10
  color: "red"
  anchors.fill: parent
}
```

### QML Bindings

**`Q_INVOKABLE` functions with arguments are not reactive in QML bindings.** When a binding
calls a `Q_INVOKABLE` with arguments, QML's binding engine does not track it. The correct
fix is to expose the result as a `Q_PROPERTY` with `NOTIFY` on the C++ side so the binding
is naturally reactive — not to add workarounds in QML.

The existing `widgetFontRevision` + comma pattern in this codebase is a known wart from
before this rule existed. Do not introduce new instances of it and do not invent new
workarounds (named hacks like `widgetFont083`, block expressions, etc.) — fix the C++ API.

### QML Typography — Always Use `Cpp_Misc_CommonFonts`

**Never set individual `font.*` sub-properties in QML.** Always assign the `font` property as a whole from `Cpp_Misc_CommonFonts`. Setting `font.pixelSize`, `font.bold`, `font.family`, etc. individually overrides only part of the font and breaks consistency with the user's font preferences and the application's typography system.

| Property / method | Use for |
|---|---|
| `Cpp_Misc_CommonFonts.uiFont` | Standard UI text (labels, descriptions) |
| `Cpp_Misc_CommonFonts.boldUiFont` | Emphasized UI text (section headers, titles) |
| `Cpp_Misc_CommonFonts.monoFont` | Code, data values, terminal output, monospaced badges |
| `Cpp_Misc_CommonFonts.customUiFont(fraction, bold)` | Scaled UI font — e.g. `customUiFont(1.2)` for slightly larger text |
| `Cpp_Misc_CommonFonts.customMonoFont(fraction, bold)` | Scaled mono font |
| `Cpp_Misc_CommonFonts.widgetFont(fraction, bold)` | Widget data display (respects user font preference) |

Predefined scale constants (use as the `fraction` argument):
- `kScaleSmall` = 0.85 — secondary/helper text
- `kScaleNormal` = 1.00 — default
- `kScaleLarge` = 1.25 — sub-headings
- `kScaleExtraLarge` = 1.50 — headings

❌ Bad:
```qml
Label {
  font.pixelSize: 13
  font.bold: true
  font.family: "Helvetica"
}
```

✅ Good:
```qml
Label {
  font: Cpp_Misc_CommonFonts.boldUiFont
}

Label {
  font: Cpp_Misc_CommonFonts.customUiFont(1.25, true)
}
```

### Comments

**Prefer self-documenting code over comments.**

Ideally, functions contain no comments at all — the code explains itself through
good naming. When a comment is necessary, one line is the limit except in very
unusual cases.

**No inline end-of-line comments.**

❌ `doSomething(); // do x y z`

✅ Single-line block comment above the line (only when the intent is not obvious):
```cpp
// KMP search requires the needle to be re-initialised after a partial match
m_kmpState = 0;
```

✅ TODO/FIXME for known issues:
```cpp
// TODO: exponential backoff on reconnection
// FIXME: possible race if FrameReader destroyed mid-processData
```

### Doxygen Comments — Mandatory for Every Generated Function and Class

**Every function and class you generate must have a Doxygen comment.** No exceptions.

#### Placement

| What | Where |
|------|-------|
| Class `@brief` | `.h` file, immediately above the `class` keyword |
| Every function | `.cpp` file, immediately above the definition |

Never put function Doxygen in the header. Never omit the class Doxygen from the header.

#### Header — class only

```cpp
/**
 * @brief Serial Studio driver for Bluetooth Low Energy devices.
 */
class BluetoothLE : public HAL_Driver
{
  ...
};
```

One `@brief` sentence. Do not document individual members or methods in the header block.

#### Source — every function

```cpp
/**
 * @brief Closes the Bluetooth LE connection and resets all internal state.
 */
void IO::Drivers::BluetoothLE::close()
{
  ...
}

/**
 * @brief Writes data to the currently selected BLE characteristic.
 * @param data The bytes to send.
 * @return Number of bytes written, or 0 on failure.
 */
qint64 IO::Drivers::BluetoothLE::write(const QByteArray &data)
{
  ...
}
```

**Tags to use (keep it brief):**
- `@brief` — one sentence, always present.
- `@param name` — one per parameter; omit for trivial single-param getters/setters.
- `@return` — what is returned and what edge values mean; omit for `void`.
- `@note` / `@warning` — only when genuinely non-obvious.

Do **not** add `@author`, `@date`, `@version`, or `@file` — SPDX headers cover ownership.

### Source File Section Separators

Use 98-character `//---` banners to divide a `.cpp` file into named concern groups.
Each group contains all functions that belong to the same logical concern.

```cpp
//--------------------------------------------------------------------------------------------------
// Constructor & singleton access functions
//--------------------------------------------------------------------------------------------------

// ... constructor, instance() ...

//--------------------------------------------------------------------------------------------------
// HAL driver implementation
//--------------------------------------------------------------------------------------------------

// ... close(), isOpen(), isReadable(), isWritable(), configurationOk(), write(), open() ...

//--------------------------------------------------------------------------------------------------
// Driver specifics
//--------------------------------------------------------------------------------------------------

// ... getters, public slots, queries unique to this driver ...

//--------------------------------------------------------------------------------------------------
// Private slots
//--------------------------------------------------------------------------------------------------

// ... onXxx() signal handlers ...

//--------------------------------------------------------------------------------------------------
// Private helpers
//--------------------------------------------------------------------------------------------------

// ... internal helpers not exposed in the header ...
```

Rules:
- The banner is exactly 98 dashes after `// ` (100 columns total including `// `).
- The label line is `// <Title>` with one space before and after the text, centred by eye — match
  the style of the existing files, not a rigid formula.
- Every concern that has at least one function gets its own banner; do not lump unrelated
  functions together just to avoid a banner.
- Banners go in the `.cpp` only — never in headers.
- `BluetoothLE.cpp` is the canonical reference for banner names and grouping.

### Signal Emission — Always `Q_EMIT`

Always use `Q_EMIT` to emit signals. Never use the bare `emit` keyword.

❌ `emit dataReceived(bytes);`
✅ `Q_EMIT dataReceived(bytes);`

`Q_EMIT` is a no-op macro that makes signal emissions grep-able and visually distinct from
ordinary function calls. It also avoids conflicts in codebases that mix Qt with other
frameworks that define their own `emit`.

### `connect()` Call Formatting

Short connections that fit within 100 columns may be written on one line. Any connection
that would exceed 100 columns must be broken into the multi-line form with each argument
on its own line, aligned under the opening parenthesis:

```cpp
// Short form — fits on one line
connect(this, &Foo::changed, this, &Foo::onChanged);

// Long form — one argument per line
connect(m_discoveryAgent,
        &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
        this,
        &IO::Drivers::BluetoothLE::onDeviceDiscovered);

// Lambda — brace on same line as the lambda introducer
connect(m_controller, &QLowEnergyController::connected, this, [this]() {
  m_deviceConnected = true;
  m_controller->discoverServices();
  Q_EMIT deviceConnectedChanged();
});
```

Never mix the two styles for the same call. Sender, signal, receiver, and slot are always
in that order — never use the old `SIGNAL()`/`SLOT()` macros.

### `noexcept` on Trivial Getters

Mark `const` accessor functions `noexcept` when their body cannot throw — i.e. when they
only read member variables or call other `noexcept` functions:

```cpp
[[nodiscard]] bool isOpen()       const noexcept;
[[nodiscard]] int  deviceIndex()  const noexcept;
[[nodiscard]] bool adapterAvailable() const noexcept;
```

Do **not** mark `noexcept` functions that call Qt I/O, allocate, or invoke anything that
may throw. When in doubt, omit `noexcept` rather than lie to the compiler.

### Performance

- **Hotpaths**: zero-copy const refs, static-cached singleton refs, `[[likely]]`/`[[unlikely]]`
- **Allocations**: 0 on the dashboard path; 1 `make_shared<TimestampedFrame>` when exports active
- **Algorithms**: KMP for delimiters (O(n+m)), `constexpr` CRC tables
- **Never allocate in hotpaths.** Never copy a Frame on the dashboard path.
- Profile first (perf/Valgrind/Instruments) — don't optimize without data.

## Development Rules

1. **Read the file before editing it.** Never suggest changes to code you haven't read in the current session.
2. **Plan before touching more than 3 files.** State which files change and why; get confirmation if scope is unclear.
3. **Prefer Edit over Write.** Rewrite a whole file only when the user asks or >70% changes.
4. **No mutexes in FrameReader or CircularBuffer.** Immutability is the design.
5. **No inline end-of-line comments.** Block comments or self-documenting names only.
6. **Max 3 nesting levels.** Use early returns, early continues, extract functions.
7. **No braces on single-statement bodies** (enforced by `RemoveBracesLLVM`). Always follow a brace-free body on its own line with a blank line before the next sibling statement.
8. **Validate at system boundaries only** (API input, file I/O, network). Trust internal data.
9. **Maintain SPDX headers.** `GPL-3.0-only`, `LicenseRef-SerialStudio-Commercial`, or both.
10. **Do not create markdown/doc files** unless explicitly asked.
11. **Update CLAUDE.md** for any significant architectural change.
12. **Prefer self-documenting code over comments.** Aim for zero comments inside function bodies. One line max when a comment is truly necessary.
13. **Make QML bindings reactive via `Q_PROPERTY`, not workarounds.** If a `Q_INVOKABLE` result needs to be reactive, expose it as a `Q_PROPERTY` with `NOTIFY` on the C++ side. Never introduce new comma-expression hacks or named variants to work around missing signals.
14. **Add Doxygen to every generated function and class.** Class `@brief` in the `.h` above `class`; every function's Doxygen in the `.cpp` above its definition. Never in the header. See "Doxygen Comments" for tags and examples.
15. **Follow the C++ Header Structure rules.** `Q_PROPERTY` block first (inside `clang-format off/on`), then `signals:`, then constructor/operators `private:` block, then `public:`, `public slots:`, `private slots:`, private helpers, private variables — each in their own section. `[[nodiscard]]` on every non-`void` return. Christmas-tree ordering within each block. See `app/src/IO/Drivers/BluetoothLE.h`.
16. **Use `//---` section banners in `.cpp` files** to separate logical concern groups (constructor, HAL impl, driver specifics, private slots, private helpers). See "Source File Section Separators" and `app/src/IO/Drivers/BluetoothLE.cpp` as the reference.
17. **Always use `Q_EMIT`** to emit signals. Never use the bare `emit` keyword.
18. **Format `connect()` calls correctly.** One line if it fits; multi-line with one argument per line otherwise. Never use `SIGNAL()`/`SLOT()` macros.
19. **Mark trivial `const` accessors `noexcept`.** Only when the body cannot throw. Never lie to the compiler — omit `noexcept` if the function calls anything that may throw.
20. **Never hardcode enum values as raw integers.** Always use the named enum from `SerialStudio.h` (e.g. `SerialStudio::BusType::UART`, not `0`). When storing as `int` in a struct field, use `static_cast<int>(SerialStudio::BusType::UART)` at the assignment site. In QML, use the registered `SerialStudio.BusType` enum names, not numeric literals.
21. **Never set individual `font.*` sub-properties in QML.** Always assign the `font` property as a whole from `Cpp_Misc_CommonFonts` (e.g. `font: Cpp_Misc_CommonFonts.uiFont`). Setting `font.pixelSize`, `font.bold`, `font.family`, etc. individually breaks typography consistency. See "QML Typography" for the full reference.

## File Creation Policy

Only create files when the user explicitly asks, or when implementing actual source/config files.

Do **not** create: SUMMARY.md, IMPROVEMENTS.md, TODO.md, STATUS.md, design docs, example files.

Share information conversationally instead.

## MCP / API Testing

API server runs on TCP port 7777. Supports MCP (JSON-RPC 2.0) and legacy JSON-RPC simultaneously.

```bash
# Start client
cd "examples/MCP Client" && python3 client.py
```

182 tools (all API commands), 2 resources (`serialstudio://frame/current`, `serialstudio://frame/history`).

See `examples/MCP Client/README.md` for full documentation.
