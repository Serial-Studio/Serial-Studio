---
spec: 0066-opcua-driver
phase: plan
status: approved
updated: 2026-08-22
---

# Plan 0066 — OPC UA Client Driver

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Read the relevant `doc/claude/` sub-docs and the *actual code*
> before writing this — a plan grounded in a stale mental model is worse than no plan.
> Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

A new commercial `IO::Drivers::OpcUa` HAL driver wraps one `QOpcUaClient` (Qt OPC UA,
open62541 backend, policy None). It discovers endpoints, connects asynchronously with the
spec-0050 one-verdict latch, browses the address space into a tag model the QML picker
consumes, and subscribes to the selected variable nodes through `QOpcUaNode::enableMonitoring`
(poll fallback via `QOpcUaClient::readNodeAttributes` on a timer). Every monitored update
writes into a per-tag value cache; a driver-side tick at the publishing interval serializes
the tags that changed into one compact binary delta frame (`[u16 tag][u8 type][payload]`*)
and publishes it through the ordinary `publishReceivedData()` path, stamped with the earliest
source timestamp in that frame. The frame is decoded by a new `opcua` native template
(`NativeLatchParser`: schema param lists each tag's index and type; unchanged tags latch), so
project generation writes a schema object instead of generated Lua and the parse runs on the
Native span lane. Nothing downstream of the driver changes.

## Affected subsystems & files

Touch-points confirmed with `grep -rn "BusType::ModBus"` and the `ss-new-driver` list.

| File | Change |
|------|--------|
| `app/src/IO/Drivers/OpcUa.h` / `.cpp` | **New.** Driver: properties (endpoint URL, selected endpoint index, auth mode, username, password, publishing interval, tag list), endpoint discovery, async dial + verdict latch, browse, subscription/poll, value cache, delta-frame tick, project generation, diagnostics counters. Commercial SPDX header. |
| `app/src/IO/Drivers/OpcUaTagModel.h` / `.cpp` | **New.** `QAbstractItemModel` over the browsed address space for the picker (lazy `browseChildren` per expanded folder, check state, type/access columns, folder tri-state). Lives next to the driver, owned by it. |
| `app/src/IO/Drivers/OpcUaWire.h` | **New.** Header-only wire vocabulary shared by the driver encoder and the native template decoder: type tags, string length cap, max tags, `encode*/decode*` helpers (mirrors `CANBus/GsUsbProtocol.h` so the ctest unit sees one definition). |
| `app/src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp` | Add `OpcUaTemplate` / `OpcUaParser` (id `opcua`): `schema` JSON param `[{"i":0,"t":"f32"}, ...]`, latch decode of the delta frame. Registered beside `modbus`. |
| `app/src/DataModel/Scripting/NativeTemplates/NativeTemplateRegistry.cpp` | Register the new template. |
| `app/rcc/scripts/native/opcua.md` + `app/rcc/rcc.qrc` | Per-template documentation page for the native parser pane. |
| `app/src/SerialStudio.h` | `BusType::OpcUa` appended at the END of the commercial block (enum order indexes `Hardware.qml`'s StackLayout and `api_client.py`'s `bus_map`). |
| `app/src/IO/ConnectionManager.h` / `.cpp` | `m_opcUaUi` slot, `opcUa()` accessor, the three `BusType` switches, `wireUiDriver`, `setupExternalConnections`, bus-name list, `rebuildDevices` teardown. |
| `app/src/Misc/ModuleManager.cpp` | `Cpp_IO_OpcUa` context property. |
| `app/src/UI/WidgetExtensions.cpp` | Add `Cpp_IO_OpcUa` to the reserved context-name list. |
| `app/src/API/EnumLabels.cpp` | `opcua` slug / "OPC UA" label. |
| `app/src/API/Handlers/OpcUaHandler.h` / `.cpp` | **New.** `opcua.*` commands: get/set configuration, `discoverEndpoints`, `getEndpoints`, `browse(nodeId)`, `getTags`, `setTags`, `addTag`, `removeTag`, `clearTags`, `generateProject`, `getStatus` (mode subscription/poll, counters). Registered in `CommandHandler::initializeHandlers()` under `BUILD_COMMERCIAL`. |
| `app/rcc/api/api-schema.json`, `sdk-symbols.json`, `proto-fields.json` + generator inputs | Regenerated through the spec-0036/0037 generator, never hand-edited; gRPC field numbers append-only. |
| `app/src/DataModel/Project/ProjectEditorShared.h` | `busTypeIcon()` case. |
| `app/src/DataModel/Project/ProjectEditorForms.cpp` | Bus combobox entry (commercial block). |
| `app/rcc/icons/devices/{16,24,32}/opcua.svg` + `app/rcc/rcc.qrc` | Driver icon, three sizes like `modbus.svg`. |
| `app/rcc/commands/app.json` + `app/qml/Commands/AppCommandBindings.qml` | `driver.opcua` toggle command + binding (`scripts/registry-verify.py`). |
| `app/qml/MainWindow/Panes/SetupPanes/Hardware.qml` | `Loader` at the new enum position. |
| `app/qml/MainWindow/Panes/SetupPanes/Drivers/OpcUa.qml` | **New.** Pane: endpoint URL + Discover button, endpoint combo (secured entries disabled with reason), auth mode, username/password, unencrypted-credentials warning banner, publishing interval, tag count + "Browse Tags…" + "Generate Project" buttons. |
| `app/qml/MainWindow/Panes/SetupPanes/Drivers/OpcUaTagBrowser.qml` | **New.** Tree picker over `OpcUaTagModel` (mirrors `ModbusGroupsDialog.qml` chrome). |
| `app/qml/ProjectEditor/Views/FlowDiagram.qml` | Add `"opcua"` to the bus-name array. |
| `app/qml/AI/AssistantPanel.qml` | Optional suggestion chip ("Set up an OPC UA subscription"). |
| `app/src/Misc/CLI.h` / `.cpp` | `--opcua <url>`, `--opcua-user`, `--opcua-pass`, `--opcua-interval`, `--opcua-tag <nodeId[:name]>` (repeatable), mirroring `setupModbusTcpConnection`. |
| `app/CMakeLists.txt` | `OpcUa` in the commercial `QT_MODULES`/`QT_LIBS`; new `.cpp`/`.h`/`.qml` entries in the commercial source and QML lists; header comment line. |
| `app/tests/tst_opcua_wire.cpp` + `app/tests/CMakeLists.txt` | ctest unit: encode/decode round-trip for every type, string cap, max-tag bound, latch semantics through the template. |
| `tests/utils/api_client.py` | `"opcua": 10` in `bus_map` (also fix the known `mqtt` note if still missing). |
| `tests/integration/test_opcua_driver.py` | **New.** R16 flows; `@pytest.mark.opcua`, `pro`, `requires_opcua_sim`. |
| `tests/integration/conftest.py` | `opcua_simulator` fixture: skip unless `127.0.0.1:4840` answers. |
| `tests/pytest.ini` | `opcua` and `requires_opcua_sim` markers. |
| `tests/requirements.txt` | `asyncua>=1.1`. |
| `examples/OPC UA PLC Simulator/{opcua_plc_simulator.py, OPC UA PLC Simulator.ssproj, README.md, doc/screenshot.png}` + `examples/examples.json` | Simulator (asyncua server, industrial tag tree, bad-status tag, optional `--user/--pass`, `--port`), project, gallery entry (`requiresPro: true`, category "Industrial Automation"). |
| `doc/help/Drivers-OPC-UA.md` + `doc/help/help.json` | Manual page, registered after Modbus in the Drivers section. |
| `doc/help/Auto-Generating-Projects.md` | Cross-link paragraph for the tag browser. |
| `.github/workflows/ci.yml` | Integration matrix: `opcua_sim: true` on Linux/macOS legs, start the simulator (`python examples/.../opcua_plc_simulator.py &` + port wait) beside Mosquitto; Windows drops `requires_opcua_sim`. |
| `doc/claude/architecture/io.md`, `doc/claude/directory-map.md`, `CLAUDE.md` overview line | Driver doctrine entry (subscription/poll, delta frame, one-verdict), directory rows, data-source list. |

## Architecture & data flow

Threads: the driver lives on the GUI thread like every non-stream driver; `QOpcUaClient`
delivers all signals there. Nothing new touches the pipeline thread.

1. **Discovery.** `setEndpointUrl()` → Discover → `QOpcUaClient::requestEndpoints(url)` →
   `endpointsRequestFinished` fills `m_endpoints`; `endpointList` property exposes
   "policy · mode · url" strings and a parallel `endpointSelectable` list (None only).
   `configurationOk()` = selected endpoint valid && policy None && (anon || username set)
   && tag list non-empty.
2. **Dial (spec 0050).** `open()` creates a fresh client (`QOpcUaProvider::createClient
   ("open62541")`), applies `QOpcUaAuthenticationInformation` (anonymous or username), sets
   `m_connecting = true`, calls `connectToEndpoint(selected)`, returns `true`. Verdict owner:
   `stateChanged(Connected)` → `reportOpenFinished(true)`; `errorChanged` / `connectError` /
   `stateChanged(Disconnected)` while `m_connecting` → `failDial(reason)` →
   `reportOpenFinished(false, reason)` + `ConnectionManager::disconnectDevice(this)`.
   `close()` disarms, disconnects, deletes the client via `deleteLater`. `isConnecting()`
   returns `m_connecting`. An established-link drop (`disconnected` after connect) calls
   `disconnectDevice(this)` with a queued error log (never a modal, spec 0056) and emits
   `configurationChanged`.
3. **Browse.** `OpcUaTagModel::fetchMore(parent)` → `node->browseChildren()` →
   `browseFinished` inserts rows; each variable row also triggers `readAttributes(DataType |
   AccessLevel | DisplayName | Description)` to fill type/access columns and the
   EngineeringUnits property when present. Selection state is a set of `nodeId → TagSpec
   {name, type, unit, arrayLen}` kept in the driver (`m_tags`, bounded `kMaxTags = 2048`,
   warn banner above 512).
4. **Subscribe.** On `Connected`, for each tag: `client->node(nodeId)` →
   `enableMonitoring(Value, QOpcUaMonitoringParameters(publishingInterval))`. All nodes share
   one subscription (Qt groups by identical parameters). `enableMonitoringFinished` with a
   bad status on ALL tags, or a `BadServiceUnsupported`/`BadTooManySubscriptions`, flips
   `m_pollMode = true` and starts `m_pollTimer` at the same interval calling
   `readNodeAttributes(all Value items)`; `readNodeAttributesFinished` feeds the same cache
   path. Status string exposes "Subscription" or "Polling (server refused subscriptions)".
5. **Value cache.** `valueAttributeUpdated` / read result → `TagSlot {QVariant value,
   UaStatusCode status, QDateTime sourceTs, bool dirty}`; `attributeError` bad → keep last
   good value, `++m_badStatusCount`, no dirty. Missing source timestamp → receipt time +
   `++m_unstampedCount`.
6. **Publish tick.** `m_frameTimer` (publishing interval) collects dirty slots into one
   `QByteArray` (reserved once at subscribe, capacity = worst case), encodes via
   `OpcUaWire::encode*`, clears dirty, converts the earliest dirty source timestamp to a
   `steady_clock` point (offset computed once per connect from `QDateTime::currentDateTime`
   vs `steady_clock::now`) and calls `publishReceivedData(std::move(frame), ts)`. Empty
   dirty set → no frame. Frame detection for the generated source is `NoDelimiters`, decoder
   `Binary`, language `Native`, template `opcua`, params `{"schema":[...]}`.
7. **Decode.** `OpcUaParser` (latch, `count = schema.size()`): walks entries, bounds-checked
   against `kMaxBytesPerFrame`, `storeAt(index, text)` per known index; unknown index or
   truncated entry stops the walk (counted by the parser's existing error path). Booleans
   store `"0"/"1"`, integers decimal, floats `QString::number(v, 'g', 9)` / doubles `17`,
   strings UTF-8 raw (capped at `kMaxStringBytes = 256` by the encoder).
8. **Project generation.** `generateProject()` mirrors `Modbus::buildProject()`: one group
   per distinct parent folder of the selected tags (browse path cached in `TagSpec`), dataset
   title = display name, unit = EngineeringUnits `displayName` when read, `plt` for numeric,
   `led` for Boolean, string datasets flagged with `readsStringValues`-compatible widget
   (datagrid/console), arrays expanded to `name[i]`. Writes `frameParserTemplate = "opcua"`
   and `frameParserParams = schema`; conn settings from `driverProperties()` (password type
   skipped by `ProjectModelSources.cpp:293`). Tags persist in conn settings as a JSON array
   under `tags` so R13 reopens without browsing (`applyConnectionSettings` → `setTags`).
9. **Diagnostics (specs 0033/0035).** Plain counters on the driver (`valuesReceived`,
   `badStatus`, `unstamped`, `framesPublished`, `reconnects`, `pollMode`), read by
   `opcua.getStatus` and the pane's status line on the existing 1 Hz tick; nothing pushed.

## Hotpath & threading impact

- **Touches the hotpath?** No. The driver is a GUI-thread producer feeding
  `publishReceivedData()`; frames enter the pipeline through the existing chunk path. The new
  native template is a per-frame decoder like `modbus`/`binary_tlv`: bounds-checked loops,
  no allocation beyond `storeAt`'s QString assignment (same as every latch template),
  `SS_ASSERT_HOTPATH` on the walk. `--benchmark-hotpath` gates are unaffected (the template is
  not in the benchmark set); the `opcua` template gets a ctest unit instead.
- **New cross-thread signal/slot?** No. `QOpcUaClient` and the driver share the GUI thread.
- **New input to a cached hotpath flag?** No.
- **Timestamp ownership** — the frame carries the earliest source timestamp of its dirty
  tags (server clock mapped to steady clock once per connect); `publishReceivedData` stamps
  at the driver boundary; nothing re-stamps.

## Data model & persistence

- No new `Keys::` in `Frame.h`: the tag list and auth mode ride in the source's
  `connectionSettings` object (existing `SourceConn`), schema rides in `frameParserParams`.
- Conn-settings shape: `{"endpointUrl", "endpointIndex", "authMode": 0|1, "username",
  "publishingInterval", "tags": [{"id": "ns=2;s=Plant.Line1.Temp", "name", "t": "f64",
  "unit", "n": arrayLen, "path": "Plant/Line1"}]}`. Password never written (vault).
- Credentials: reuse `MQTT::CredentialVault` keyed by endpoint host:port (same SimpleCrypt
  settings vault; class stays in `MQTT/`, the driver includes it). Name collision with MQTT
  entries avoided by a `"opcua/"` prefix in the settings key: add an optional `scope`
  argument to `CredentialVault` rather than a second vault class.
- Schema param format is versioned by the template id only; the encoder and decoder share
  `OpcUaWire.h`, and a wire change bumps a `kWireVersion` byte at frame start (decoder
  rejects mismatches, counted).

## API / SDK surface

- `opcua.getConfiguration`, `opcua.setEndpointUrl`, `opcua.discoverEndpoints` (async →
  returns when `endpointsRequestFinished` fires, bounded 5 s), `opcua.getEndpoints`,
  `opcua.setEndpointIndex`, `opcua.setAuthMode`, `opcua.setUsername`, `opcua.setPassword`,
  `opcua.setPublishingInterval`, `opcua.browse {nodeId}` (one level, returns children with
  type/access), `opcua.getTags`, `opcua.setTags`, `opcua.addTag`, `opcua.removeTag`,
  `opcua.clearTags`, `opcua.generateProject`, `opcua.getStatus`.
- `EnumLabels.cpp`: slug `opcua`, label `OPC UA`. `api_client.py` `bus_map["opcua"] = 10`.
- All behind `BUILD_COMMERCIAL`; policy preflight for `generateProject` follows the
  registry-level pattern from commit 3493077d5.

## QML / UI

- `OpcUa.qml`: same `GroupBox`/`GridLayout` chrome as `Modbus.qml`; endpoint combo uses the
  restore-race guard (`Component.onCompleted` index sync, `onActivated` write) used by the
  other driver panes; disabled endpoint entries render with the `disabled` palette and a
  tooltip reason; `authMode == 1` shows a warning `Label` with the theme's warning colour.
  Pane is locked while connected/dialing through the existing StackLayout gate.
- `OpcUaTagBrowser.qml`: `TreeView` over `OpcUaTagModel`, checkbox delegate, folder tri-state,
  type/access columns, search field filtering display names, tag count + soft-limit banner,
  "Select All Readable", OK/Cancel. Fonts via the shared auto-scale.
- Status line under the pane: "Subscribed · 128 tags · 10.0 Hz" or "Polling (server refused
  subscriptions)" from `getStatus` fields.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Frame encoding / parser | (a) Modbus-style snapshot frame + generated Lua; (b) delta frame + new `opcua` native latch template; (c) CSV text + `delimited` template | **(b)**. No generated code to keep in sync with the tag list, Native span lane instead of the 64 kHz Lua tier, strings latch as QString natively, delta keeps 2048-tag frames small. The "mirror Modbus" premise did not bind: Modbus predates native templates. |
| Publish cadence | Per data-change emission vs. one frame per publishing tick | **Tick.** Subscriptions deliver N notifications per publish; one frame per tick bounds GUI→pipeline traffic to command rate and matches the spec's "one update rate". |
| Subscription grouping | One `QOpcUaNode` per tag with `enableMonitoring` vs. raw `QOpcUaClient` batch API | **Per node.** Qt batches nodes with identical monitoring parameters into one subscription; the node API also gives `sourceTimestamp(attr)` and `valueAttributeError()` for free. |
| Secure channel | Ship None-only now vs. rebuild qtopcua with OpenSSL first vs. vendor open62541 | **None-only** (maintainer decision 2026-08-22; spec amended). Follow-up spec for the secure channel. |
| Credential storage | Reuse `MQTT::CredentialVault` with scope vs. new `OpcUa` vault class | **Reuse with a scope prefix.** One encrypted vault, one SimpleCrypt key; moving the class out of `MQTT/` is out of lane. |
| Tag model ownership | Driver-owned `QAbstractItemModel` vs. QML `ListModel` filled by API calls | **Driver-owned.** Lazy browse needs a real `fetchMore`; the API `browse` command reuses the same code path. |
| Poll fallback trigger | Automatic on subscription refusal vs. user toggle | **Automatic** (spec R9), with the mode surfaced in status; no toggle to misconfigure. |

## Risks & mitigations

- **Wedged connect button** (io.md, spec 0050): every failure path in the dial window must
  reach `reportOpenFinished(false)` AND `disconnectDevice(this)`. Mitigation: one private
  `failDial()` funnel, a 15 s dial timer as the last resort (no retries, no redial), and a
  pytest that connects to a closed port and asserts `io.getStatus` leaves `connecting`.
- **Modal on the connect/error stack** (spec 0056): driver logs through `logDriverError`,
  never a `QMessageBox`, except the project-generation confirmation which runs from a user
  click on the GUI thread (same as Modbus).
- **`QOpcUaClient` deleted while signals in flight**: `close()` disconnects all client
  signals before `deleteLater`; node objects are children of the client.
- **Server-clock skew**: source timestamps mapped through a per-connect offset; a timestamp
  more than 5 s off the receipt time falls back to receipt time and counts as unstamped
  (prevents ring rewinds; dashboard time rings never rewind).
- **Oversized frames**: `kMaxTags`, `kMaxStringBytes`, and per-tag worst-case reserve bound
  the frame; the decoder bounds every read against `kMaxBytesPerFrame`.
- **Enum order**: appending `OpcUa` after `Mqtt` keeps every existing integer; `Hardware.qml`
  Loader index and `bus_map` must use 10. Lint: `tst_enum_labels` covers slug/label.
- **Generated-artifact drift**: API surface regenerated by the spec-0036/0037 generator;
  `scripts/registry-verify.py` and `code-verify.py --check` in the task list.
- **Translation churn**: new `tr()` strings; `.ts/.qm` regeneration stays the maintainer's
  step (Trust Contract: never touch derived artifacts).
- **Qt module absent on a dev box**: `find_package(Qt6 COMPONENTS OpcUa)` only in the
  commercial block; GPL builds never reference it (AC2).

## Test & verification plan

- **Unit (ctest, maintainer builds, I may run):** `app/tests/tst_opcua_wire.cpp` — round-trip
  every wire type, string cap/truncation, max-tag bound, version byte rejection, template
  latch semantics (dirty subset updates, unknown index ignored, truncated entry stops walk).
  → AC8 support, Constraints.
- **Integration (maintainer runs app + simulator; I run pytest):**
  `tests/integration/test_opcua_driver.py`:
  - `test_discover_endpoints` — R2 (secured endpoint greyed when sim started `--secure-only`).
  - `test_connect_anonymous` / `test_connect_username` / `test_bad_credentials_verdict` /
    `test_closed_port_settles` — R3, R4 (log line), R5 → AC1, AC4.
  - `test_browse_tree` / `test_unsupported_type_flagged` — R6, R8.
  - `test_generate_project_groups_and_types` — R7, R8 (string/bool/array datasets).
  - `test_subscription_delivery_rate` / `test_poll_fallback` (sim `--no-subscriptions`) — R9.
  - `test_source_timestamps_in_csv` — R10 → AC5.
  - `test_bad_status_keeps_last_value` — R11.
  - `test_reconnect_after_sim_restart` — R12 → AC6 (sim restarted by the test via subprocess).
  - `test_project_roundtrip_no_browse` — R13.
  - `test_api_surface` — R14.
  - All `@pytest.mark.opcua @pytest.mark.pro @pytest.mark.requires_opcua_sim`; fixture skips
    when `4840` is closed → AC2.
- **Example/gallery:** `examples.json` validated by the existing examples test; screenshot
  captured by the maintainer → AC10.
- **Docs:** `python scripts/documentation-verify.py doc/help/Drivers-OPC-UA.md`; `help.json`
  entry → AC9.
- **Hotpath:** not touched; maintainer runs `--benchmark-hotpath` once on the Pro build for
  AC7 as a regression guard.
- **Static:** `python scripts/code-verify.py --check` on every new/edited file;
  `scripts/registry-verify.py`; `qt-cpp-review` on the driver before handoff;
  `python scripts/sanitize-commit.py` before commit.
- **CI:** `ci.yml` integration matrix gains the simulator step; Windows leg marker expr gains
  `and not requires_opcua_sim` → AC11. GPL build leg proves AC2's link check.
