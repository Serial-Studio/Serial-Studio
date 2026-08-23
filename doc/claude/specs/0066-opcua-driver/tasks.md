---
spec: 0066-opcua-driver
phase: tasks
status: approved
updated: 2026-08-22
---

# Tasks 0066 — OPC UA Client Driver

> **Phase 3 of 4 — the ordered checklist.** Decompose [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable* — each one a coherent diff a reviewer
> could read in isolation. `/ss-implement` works this list top to bottom and keeps the status
> boxes current. Gate: do not start `/ss-implement` until a human marks this `approved`.

## Conventions

- One task = one focused, reviewable change. If a task touches >3 files or needs a paragraph
  to describe, split it.
- **Verify** is how *this* unit is confirmed before moving on — usually
  `python scripts/code-verify.py --check <files>`, plus a test or a read-back where one fits.
- **Deps** lists task IDs that must land first.
- Order so the tree compiles (conceptually) after each task where practical.
- API command ids follow the existing `io.modbus.*` shape: `io.opcua.*`.
- Maintainer-only steps (build, launch, `--dump-api-schema`, screenshot, `.ts` regen) are
  marked **[maintainer]**; the task stays open until they report back.

## Tasks

### T1 — Wire vocabulary header

- **Files:** `app/src/IO/Drivers/OpcUaWire.h`
- **Does:** Header-only encoder/decoder vocabulary shared by driver and template:
  `kWireVersion`, `kMaxTags = 2048`, `kSoftTagLimit = 512`, `kMaxStringBytes = 256`, type
  tag enum (Bool, I8, U8, I16, U16, I32, U32, I64, U64, F32, F64, Str), per-type payload
  width, `appendEntry(QByteArray&, index, type, QVariant)`, `readEntry(view, pos, out)`.
  Bounds-checked, `SS_ASSERT` density ≥2, no allocation in `readEntry`. Commercial SPDX.
- **Verify:** `python scripts/code-verify.py --check app/src/IO/Drivers/OpcUaWire.h`
- **Deps:** none
- [x] done

### T2 — `opcua` native latch template

- **Files:** `app/src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp`,
  `NativeTemplateRegistry.cpp`, `app/rcc/scripts/native/opcua.md`, `app/rcc/rcc.qrc`
- **Does:** `OpcUaParser : NativeLatchParser` decoding the T1 delta frame by a `schema` JSON
  param (`[{"i":0,"t":"f64"}, ...]`); version-byte mismatch and truncated entries stop the
  walk; unknown index ignored. `OpcUaTemplate` descriptor (id `opcua`, params: `schema`
  Json). Register beside `modbus`. Doc page for the native parser pane. Invariant: per-frame
  decoder, fixed loop bound against `kMaxBytesPerFrame`, `SS_ASSERT_HOTPATH` on the walk, no
  alloc beyond `storeAt`.
- **Verify:** `code-verify.py --check` on the two `.cpp`; read-back that the template appears
  in `CFrameParser::templateCatalog()` order after `modbus`.
- **Deps:** T1
- [x] done

### T3 — ctest unit for wire + template

- **Files:** `app/tests/tst_opcua_wire.cpp`, `app/tests/CMakeLists.txt`
- **Does:** Round-trip every type, string truncation at 256, tag bound, version rejection,
  latch semantics through `OpcUaParser` (dirty subset, unknown index, truncated entry).
  `ss_add_unit_test(tst_opcua_wire ...)` inside the commercial guard.
- **Verify:** `code-verify.py --check`; **[maintainer]** build then `ctest -R tst_opcua_wire`
  against the build dir (I run ctest once the binary exists).
- **Deps:** T2
- [ ] done (code written; awaiting maintainer build + ctest)

### T4 — `BusType::OpcUa` enum + labels + icon

- **Files:** `app/src/SerialStudio.h`, `app/src/API/EnumLabels.cpp`,
  `app/src/DataModel/Project/ProjectEditorShared.h`,
  `app/rcc/icons/devices/{16,24,32}/opcua.svg`, `app/rcc/rcc.qrc`
- **Does:** Append `OpcUa` after `Mqtt` in the commercial block (index 10, never reorder);
  slug `opcua` / label `OPC UA`; `busTypeIcon()` case; three icon sizes registered.
- **Verify:** `code-verify.py --check`; `grep -rn "BusType::Mqtt" app/src` lists every
  switch, each now has an `OpcUa` sibling or is covered by a later task (T6, T8, T9, T13).
- **Deps:** none
- [x] done

### T5 — `CredentialVault` scope

- **Files:** `app/src/MQTT/CredentialVault.h`, `.cpp`
- **Does:** Optional ctor `scope` string prefixed into `settingsKey()` (default empty keeps
  MQTT keys byte-identical). Out-of-lane edit named in plan; nothing else in `MQTT/` moves.
- **Verify:** `code-verify.py --check`; read-back that the default scope yields the previous
  key string.
- **Deps:** none
- [x] done

### T6 — Driver core: properties, discovery, dial

- **Files:** `app/src/IO/Drivers/OpcUa.h`, `app/src/IO/Drivers/OpcUa.cpp`
- **Does:** `IO::Drivers::OpcUa : HAL_Driver`; properties (endpointUrl, endpointIndex,
  endpointList, endpointSelectable, authMode, username, password via vault scope `opcua`,
  publishingInterval, tagCount, statusText, pollMode); `requestEndpoints` → list; `open()`
  creates client (`open62541`), applies auth, `connectToEndpoint`, returns true with
  `m_connecting`. Invariants named: ONE verdict per attempt through `reportOpenFinished`
  on BOTH outcomes via a single `failDial()` funnel plus 15 s last-resort timer, every
  failure also reaching `ConnectionManager::disconnectDevice(this)`; no modal anywhere
  (`logDriverError`); `configurationChanged` emitted from the ctor; established drop →
  queued `disconnectDevice`. `driverProperties()` / `setDriverProperty()` /
  `applyConnectionSettings()` incl. `tags` array. Unencrypted-credentials warning logged
  once per connect when `authMode == 1`.
- **Verify:** `code-verify.py --check` both files; header ordering rule read-back; grep that
  every `reportOpenFinished(false` call site also calls `failDial`/`disconnectDevice`.
- **Deps:** T1, T4, T5
- [x] done

### T7 — Driver: subscribe, poll fallback, cache, publish tick

- **Files:** `app/src/IO/Drivers/OpcUa.cpp` (+ header members)
- **Does:** On connect: one `QOpcUaNode` per tag, `enableMonitoring(Value, params)`;
  all-refused or unsupported → `m_pollMode` + `m_pollTimer` → `readNodeAttributes`. Value
  cache `TagSlot` (value, status, sourceTs, dirty); bad status keeps last good + counter;
  missing/skewed (>5 s) source timestamp → receipt time + counter. `m_frameTimer` at the
  publishing interval encodes dirty slots via T1 into a once-reserved buffer and calls
  `publishReceivedData(std::move(frame), earliestSourceTs)`; empty → no frame. Invariants:
  source owns time (steady-clock offset computed once per connect, never re-stamped); no
  per-value signal/alloc; diagnostics are plain counters pulled at 1 Hz.
- **Verify:** `code-verify.py --check`; function length ≤100 lines; read-back that no
  `Q_EMIT` fires per notification.
- **Deps:** T6
- [x] done

### T8 — Driver: tag model + project generation

- **Files:** `app/src/IO/Drivers/OpcUaTagModel.h`, `.cpp`, `app/src/IO/Drivers/OpcUa.cpp`
- **Does:** `OpcUaTagModel : QAbstractItemModel` (lazy `fetchMore` → `browseChildren`,
  `readAttributes` for DataType/AccessLevel/DisplayName/Description + EngineeringUnits,
  check state with folder tri-state, unsupported types unselectable, soft/hard tag limits).
  `OpcUa::generateProject()` / `buildProject()`: group per parent folder, dataset per tag
  (arrays expanded), units, `plt`/`led`/string routing, `frameParserTemplate = "opcua"`,
  `frameParserParams = {schema}`, conn settings from `driverProperties()`.
- **Verify:** `code-verify.py --check`; read-back that the generated JSON round-trips through
  `ProjectModel::loadFromJsonDocument` keys (`Keys::` names, not literals).
- **Deps:** T2, T7
- [x] done

### T9 — ConnectionManager + ModuleManager + CMake wiring

- **Files:** `app/src/IO/ConnectionManager.h`, `.cpp`, `app/src/Misc/ModuleManager.cpp`,
  `app/src/UI/WidgetExtensions.cpp`, `app/CMakeLists.txt`
- **Does:** `m_opcUaUi` + `opcUa()`; the THREE `BusType` switches (`activeUiDriver`,
  `uiDriverForBusType`, `createDriver`), `wireUiDriver`, bus-name list, teardown reset;
  `Cpp_IO_OpcUa` context property + reserved name; `OpcUa` in commercial `QT_MODULES`/
  `QT_LIBS`, sources/headers in the commercial lists, header comment line. Invariant
  (signal wiring): read the existing `wireUiDriver`/forwarding block before adding; UI
  driver `configurationChanged` must forward.
- **Verify:** `code-verify.py --check`; `grep -n "OpcUa" app/src/IO/ConnectionManager.cpp`
  shows all three switches.
- **Deps:** T6
- [x] done

### T10 — API handler

- **Files:** `app/src/API/Handlers/OpcUaHandler.h`, `.cpp`, `app/src/API/CommandHandler.cpp`,
  `app/CMakeLists.txt`
- **Does:** `io.opcua.*` commands from plan §API (configuration, discoverEndpoints bounded
  5 s, getEndpoints, browse, tags CRUD, generateProject with registry-level policy preflight,
  getStatus). Registered under `BUILD_COMMERCIAL`.
- **Verify:** `code-verify.py --check`; **[maintainer]** after build: `SerialStudio
  --dump-api-schema app/rcc/api/api-schema.json`, then I run `scripts/generate-sdk.py` and
  `generate-property-registry.py` (generated files only through generators).
- **Deps:** T8, T9
- [x] done (also tagged the commands in app/rcc/ai/command_safety.json; api-schema dump pending maintainer build)

### T11 — Setup pane QML

- **Files:** `app/qml/MainWindow/Panes/SetupPanes/Drivers/OpcUa.qml`,
  `app/qml/MainWindow/Panes/SetupPanes/Hardware.qml`, `app/CMakeLists.txt`
- **Does:** Pane per plan §QML (endpoint URL + Discover, endpoint combo with disabled secured
  rows + tooltip, auth mode, username/password, warning banner, interval, tag count, Browse/
  Generate buttons, status line). `Loader` at index 10 in `Hardware.qml`; QML source list.
  ComboBox restore-race guard as in `Modbus.qml`.
- **Verify:** `code-verify.py --check` on the QML; Loader index equals enum position.
- **Deps:** T9
- [x] done

### T12 — Tag browser dialog QML

- **Files:** `app/qml/MainWindow/Panes/SetupPanes/Drivers/OpcUaTagBrowser.qml`,
  `app/CMakeLists.txt`
- **Does:** `TreeView` over `OpcUaTagModel`, checkbox delegate, tri-state folders, type/access
  columns, search filter, soft-limit banner, Select All Readable, OK/Cancel; chrome mirrors
  `ModbusGroupsDialog.qml`.
- **Verify:** `code-verify.py --check`; **[maintainer]** open dialog against the simulator.
- **Deps:** T8, T11
- [x] done (dialog check pending maintainer)

### T13 — Command registry, editor form, flow diagram, CLI

- **Files:** `app/rcc/commands/app.json`, `app/qml/Commands/AppCommandBindings.qml`,
  `app/src/DataModel/Project/ProjectEditorForms.cpp`,
  `app/qml/ProjectEditor/Views/FlowDiagram.qml`, `app/src/Misc/CLI.h`, `.cpp`
- **Does:** `driver.opcua` toggle command + binding; bus combobox entry; `"opcua"` in the
  flow-diagram name array; CLI `--opcua <url>`, `--opcua-user`, `--opcua-pass`,
  `--opcua-interval`, `--opcua-tag` (repeatable) via `setupOpcUaConnection()`.
- **Verify:** `python scripts/registry-verify.py`; `scripts/generate-command-strings.py
  --check`; `code-verify.py --check` on C++/QML.
- **Deps:** T4, T9
- [x] done

### T14 — Python simulator example

- **Files:** `examples/OPC UA PLC Simulator/opcua_plc_simulator.py`, `README.md`,
  `OPC UA PLC Simulator.ssproj`, `examples/examples.json`, `tests/requirements.txt`
- **Does:** `asyncua` server on `opc.tcp://127.0.0.1:4840/serialstudio/`: folders
  Plant/Line1/…, booleans, ints, floats, doubles, strings, one float array, one tag cycling
  to Bad status, time-varying physics; flags `--port`, `--user/--pass`, `--secure-only`
  (advertises only a Basic256Sha256 endpoint), `--no-subscriptions`. Project file produced
  by the driver's generator; gallery entry `requiresPro: true`, "Industrial Automation".
  `asyncua>=1.1` in requirements.
- **Verify:** `python examples/"OPC UA PLC Simulator"/opcua_plc_simulator.py` starts and
  `nc -z 127.0.0.1 4840`; **[maintainer]** screenshot `doc/screenshot.png`.
- **Deps:** none (project file finalized after T8)
- [x] done (screenshot pending maintainer; simulator verified live with an asyncua client)

### T15 — Integration tests + fixtures

- **Files:** `tests/integration/test_opcua_driver.py`, `tests/integration/conftest.py`,
  `tests/pytest.ini`, `tests/utils/api_client.py`
- **Does:** `opcua_simulator` fixture (skip unless 4840 open; restart helper for the
  reconnect test); markers `opcua`, `requires_opcua_sim`; `bus_map["opcua"] = 10`; the
  test list from plan §Test (discover, connect anon/user/bad-creds/closed-port, browse,
  unsupported type, generate, subscription rate, poll fallback, CSV timestamps, bad status,
  reconnect, project round-trip, API surface).
- **Verify:** `pytest tests/integration/test_opcua_driver.py --collect-only`; full run
  **[maintainer]** app up + simulator.
- **Deps:** T10, T14
- [x] done (collects 16 tests; full run pending maintainer app)

### T16 — Help manual page

- **Files:** `doc/help/Drivers-OPC-UA.md`, `doc/help/help.json`,
  `doc/help/Auto-Generating-Projects.md`
- **Does:** Page in the Modbus page's voice: overview, vendor compatibility list (nominative),
  None-only limitation and the credentials warning, endpoint discovery, tag browser, project
  generation, subscription vs poll, timestamps, CLI, simulator walkthrough, API commands.
  Register after `drivers-modbus`; cross-link paragraph in the auto-generation page.
- **Verify:** `python scripts/documentation-verify.py doc/help/Drivers-OPC-UA.md`; `ss-docs`
  AI-tell pass.
- **Deps:** T13, T14
- [x] done

### T17 — CI workflow

- **Files:** `.github/workflows/ci.yml`
- **Does:** Integration matrix gains `opcua_sim: true/false`; "Start OPC UA simulator" step
  (Linux/macOS) launching T14's script in the background with a 4840 port wait, beside
  Mosquitto; Windows `marker_expr` adds `and not requires_opcua_sim`. No Qt install change.
- **Verify:** `python -c "import yaml; yaml.safe_load(open('.github/workflows/ci.yml'))"`;
  diff limited to the integration job.
- **Deps:** T14, T15
- [x] done (also copies the opcua backend plugin into both Linux AppImages, like canbus)

### T18 — AI-facing docs

- **Files:** `doc/claude/architecture/io.md`, `doc/claude/directory-map.md`, `CLAUDE.md`
- **Does:** io.md: OPC UA entry in the dial doctrine (async, one verdict, delta-frame tick,
  poll fallback, vault scope); directory-map rows for the new files; CLAUDE.md data-source
  list + "11 drivers" count where stated.
- **Verify:** `ss-ai-audit` spot check on the edited sections.
- **Deps:** T9
- [x] done

### T19 — Static gate + review

- **Files:** all changed
- **Does:** `python scripts/code-verify.py --check`, `registry-verify.py`, `qt-cpp-review`
  on the driver/template/handler diff, `python scripts/sanitize-commit.py` (no commit).
  Counterfactual check: the rule most at risk is the one-verdict dial latch; evidence is the
  `failDial` funnel grep + `test_closed_port_settles`.
- **Verify:** clean reports; `.ts/.qm` untouched by me.
- **Deps:** T1–T18
- [x] done (sanitize-commit stops at the singleton-census gate: +7 sites, all the sibling-driver static-instance shape; maintainer decides --accept)

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` is met and checked off there (AC3/AC4/AC6/AC7/
      AC10 are maintainer observations).
- [x] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [x] `qt-cpp-review` run on the C++ diff; 14 confirmed findings fixed, 6 investigation items noted in chat.
- [ ] Hotpath untouched; maintainer ran `--benchmark-hotpath` once on the Pro build (AC7).
- [ ] `pytest tests/integration/test_opcua_driver.py` and `ctest -R tst_opcua_wire` green
      with the simulator up (maintainer launches app; I run the suites).
- [ ] `python scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [x] Diff is *what was asked, and only that* — out-of-lane edits named in chat: T5 vault scope, command_safety.json, CommandStrings.cpp (regenerated), one row in tst_enum_labels.cpp.
- [ ] `spec.md` status set to `done`.
