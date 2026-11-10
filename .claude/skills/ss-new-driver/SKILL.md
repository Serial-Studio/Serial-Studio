---
name: ss-new-driver
description: >-
  Scaffold a new Serial Studio I/O driver (a new data source under core/Devices/IO/Drivers/). Use
  when adding support for a new bus/transport — e.g. "add a <X> driver", "support reading from
  <Y>", "new data source". Encodes the canonical driver pattern and every registration touch-point
  so the new driver actually shows up in the UI, CLI, and connection manager.
argument-hint: "[DriverName]"
---

# Serial Studio — new I/O driver

**Before writing anything, read `core/Devices/IO/Drivers/BluetoothLE.h` and `BluetoothLE.cpp` in
full.** They are the canonical reference for the driver contract — match their structure,
signal/slot wiring, and `driverProperties()` shape rather than inventing a new layout. After
the read, restate the driver contract in chat in 2-3 sentences (pure virtuals, publish path,
timestamp-at-boundary) before scaffolding — a contract you've just named is one the new code
follows, not one it drifts from (`doc/claude/j-space.md`).

A driver subclasses `IO::HAL_Driver` (`core/Core/IO/HAL_Driver.h`) and must implement the pure
virtuals: `close`, `isOpen`, `isReadable`, `isWritable`, `configurationOk`, `write`, `open`,
`driverProperties`, and `setDriverProperty`. Also consider the non-pure virtuals with default
bodies — `deviceIdentifier()`, `selectByIdentifier()`, `applyConnectionSettings()` — which
drive device selection and reconnection. Received bytes are published via
`publishReceivedData(...)` — **stamp the timestamp at the driver boundary** (source owns time;
see [ss-hotpath]). Never re-stamp downstream. A setter saves its default through
`persistSetting(m_settings, key, value)` / `forgetSetting(...)`, never `m_settings.setValue`: the
factory turns persistence off on live instances, so a project source's settings never overwrite
the Setup pane's (a sub-object that writes settings itself gets `setPersistent()` forwarded).

## Touch-points to wire (verify each against an existing driver)

1. `core/Devices/IO/Drivers/<Name>.h` / `.cpp` — the driver class, SPDX header, `.h` ordering rules.
2. `core/Core/SerialStudio.h` — add the value to the `BusType` enum (QML uses `SerialStudio.BusType.*`,
   never integer literals).
3. `core/Devices/IO/ConnectionManager.{h,cpp}` — accessor (e.g. `network()` / `uart()` analogue)
   plus signal forwarding; the bus lookups live in `ConnectionManager/DriverUiRegistry.cpp`
   (`forBusType()`) and the live-instance switch in `ConnectionManager/DriverFactory.cpp`
   (`DriverFactory::create()`, spec 0070). Update both.
4. `core/Devices/CMakeLists.txt` — add `IO/Drivers/<Name>.h` and `.cpp` to the source lists (sources are
   listed explicitly, not globbed; commercial drivers go in the guarded
   `if(BUILD_COMMERCIAL)` block).
5. QML configuration UI — the driver panes are bespoke forms (only the Project Editor renders
   `driverProperties()` generically; a row one mode does not use stays in the list and carries a
   `visibleWhen` rule, never a conditional append, because the list is also what a project
   persists): create
   `app/qml/MainWindow/Panes/SetupPanes/Drivers/<Name>.qml`, add its `Loader` to the
   `StackLayout` in `SetupPanes/Hardware.qml` **at the bus's enum position** (the layout
   indexes by `Cpp_IO_Manager.busType`), and register the new .qml in the `QML_SOURCES` list
   in `app/CMakeLists.txt`.
6. `core/Core/EnumLabels.cpp` — add the bus to the `busTypeSlug()` and `busTypeLabel()`
   switches (the API's string names for the bus; commercial buses go inside the
   `#ifdef BUILD_COMMERCIAL` block).
7. `core/Ui/ProjectEditor/ProjectEditorIcons.h` — add the bus to the `busTypeIcon()`
   switch, and `core/Ui/ProjectEditor/EditorForms.cpp` — add it to the `busTypes`
   combobox list in the source form model (positional: it must follow enum order), plus the
   name array in `app/qml/ProjectEditor/Views/FlowDiagram/DiagramIcons.qml` (indexed by enum
   value).
8. Icons — icons resolve by name through `Cpp_Misc_IconRegistry.icon("devices", name, size)`,
   never a `qrc:/` path. Add `app/rcc/iconpacks/basic/devices/{16,24,32}/<name>.png` (plus
   `@2x`) with `<file>` entries in `app/rcc/basic.qrc`, and
   `app/rcc/iconpacks/fine-line/devices/48/<name>.svg` in `app/rcc/art-free.qrc`.
9. QML context and command — `registry.add("Cpp_IO_<Name>", ...)` in
   `app/src/Misc/ModuleManager.cpp`, the matching name in `core/Ui/Misc/ContextRegistry.cpp`, and
   a `driver.<bus>` toggle in `app/rcc/commands/app.json` bound in
   `app/qml/Commands/AppCommandBindings.qml` (`registry-verify.py` checks the pair).
10. API — a per-driver `io.<bus>.*` handler under `core/Api/API/Handlers/`, registered in
    `core/Api/API/CommandHandler.cpp` and listed in `core/Api/CMakeLists.txt`.
11. CLI (optional) — options are declared in `app/src/Misc/CLI.h` and added in `CLI.cpp`; the
    wiring lives in `app/src/Misc/CLI/CliBusConfig.cpp` (`setupUartConnection` /
    `setupTcpConnection`) or, for industrial buses, `CliIndustrialConfig.cpp`.
12. Tests — the bus's slug and label rows in `app/tests/tst_enum_labels.cpp`, and `bus_map` in
    `tests/utils/api_client.py` if integration tests should reach it via `io.setBusType`.

The list above drifts as the app grows. Before declaring done, grep a recently added bus value
(e.g. `grep -rni "iec104" app core tests scripts`) and mirror every switch/list it appears in.

## Rules

- This is a multi-file change (>3 files): state the plan and get confirmation before executing.
- Follow `doc/claude/code-style.md` (header ordering, `[[nodiscard]]`, no in-header member init,
  `Q_EMIT` not `emit`, Christmas-tree ordering).
- Run `python scripts/code-verify.py --check` on the new files before handoff.
- Do not build or run the app — leave compilation and runtime testing to the developer.
