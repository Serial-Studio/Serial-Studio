---
name: ss-new-driver
description: >-
  Scaffold a new Serial Studio I/O driver (a new data source under app/src/IO/Drivers/). Use
  when adding support for a new bus/transport — e.g. "add a <X> driver", "support reading from
  <Y>", "new data source". Encodes the canonical driver pattern and every registration touch-point
  so the new driver actually shows up in the UI, CLI, and connection manager.
argument-hint: "[DriverName]"
---

# Serial Studio — new I/O driver

**Before writing anything, read `app/src/IO/Drivers/BluetoothLE.h` and `BluetoothLE.cpp` in
full.** They are the canonical reference for the driver contract — match their structure,
signal/slot wiring, and `driverProperties()` shape rather than inventing a new layout.

A driver subclasses `IO::HAL_Driver` (`app/src/IO/HAL_Driver.h`) and must implement the pure
virtuals: `close`, `isOpen`, `isReadable`, `isWritable`, `configurationOk`, `write`, `open`,
`driverProperties`, and `setDriverProperty`. Received bytes are published via
`publishReceivedData(...)` — **stamp the timestamp at the driver boundary** (source owns time;
see [ss-hotpath]). Never re-stamp downstream.

## Touch-points to wire (verify each against an existing driver)

1. `app/src/IO/Drivers/<Name>.h` / `.cpp` — the driver class, SPDX header, `.h` ordering rules.
2. `app/src/SerialStudio.h` — add the value to the `BusType` enum (QML uses `SerialStudio.BusType.*`,
   never integer literals).
3. `app/src/IO/ConnectionManager.{h,cpp}` — accessor (e.g. `network()` / `uart()` analogue),
   construction, `setBusType` dispatch, and signal forwarding.
4. `app/CMakeLists.txt` — add `src/IO/Drivers/<Name>.cpp` to the `SOURCES` list (sources are
   listed explicitly, not globbed).
5. QML configuration UI — add the bus to the device-setup panel that renders `driverProperties()`.
6. `app/src/API/EnumLabels.cpp` — add the bus to the `busTypeSlug()` and label switches (the API's
   string names for the bus; commercial buses go inside the `#ifdef BUILD_COMMERCIAL` block).
7. `app/src/DataModel/ProjectEditor.cpp` — add the bus to the `busTypeIcon()` switch and the
   `busTypes` combobox list in the source form model.
8. CLI (optional) — if it should be launchable headless, add options in `app/src/Misc/CLI.{h,cpp}`
   following the existing `setupUartConnection` / `setupTcpConnection` pattern.
9. `tests/utils/api_client.py` (optional) — add the bus to `bus_map` if integration tests should
   reach it via `io.setBusType`.

The list above drifts as the app grows. Before declaring done, grep a recently added bus value
(e.g. `grep -rn "BusType::HidDevice" app/src`) and mirror every switch/list it appears in.

## Rules

- This is a multi-file change (>3 files): state the plan and get confirmation before executing.
- Follow `doc/claude/code-style.md` (header ordering, `[[nodiscard]]`, no in-header member init,
  `Q_EMIT` not `emit`, Christmas-tree ordering).
- Run `python scripts/code-verify.py --check` on the new files before handoff.
- Do not build or run the app — leave compilation and runtime testing to the developer.
