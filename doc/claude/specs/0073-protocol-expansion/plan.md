---
spec: 0073-protocol-expansion
phase: plan
status: approved     # draft -> approved (gate before /ss-tasks)
updated: 2026-08-27
---

# Plan 0073 — Industrial Protocol Expansion

> **Phase 2 of 4 — the HOW.** Satisfies every requirement in [`spec.md`](./spec.md).
> Delivery is five phases in roadmap order; each phase leaves the tree shippable and is
> planned here as an independent unit sharing common foundations. Implementation is
> executed by a smaller model in narrow, individually-verified tasks (see tasks.md).

## Approach (one paragraph)

Every feature reuses one of two proven shapes already in the tree. Inbound protocol work
follows the **OPC UA pattern**: a driver-side C++ decoder/session object keeps protocol
state on the GUI thread, latches named values into indexed slots, encodes only-changed
slots into an `OpcUaWire`-format delta frame (`[version][index u16][type u8][payload]*`),
publishes it through `publishReceivedData()` with source timestamps, and a registered
native binary template (`BinaryTemplates.cpp`) decodes it back into datasets from a
`schema` param written by a driver-side `buildProject()` generator. Outbound work follows
the **FrameConsumer sink pattern**: a worker-thread consumer fed one trimmed `DataBlock`
clone from `FrameBuilder::publishBlock`, gated by the cached `m_anyAsyncSink` flag.
Tier A items (Sparkplug B, J1939/ISO-TP, extended mux) add zero new connection surface —
they are options on the MQTT driver, the CAN driver, and the DBC importer's Lua codegen.
Tier B items (S7, EtherNet/IP, IEC 104) each pay the full new-driver registration sweep
mapped below.

## Shared foundations (all phases)

- **Wire vocabulary**: reuse `app/src/IO/Drivers/OpcUaWire.h` unchanged (it is already
  transport-neutral: index/type/payload, latching decoder, caps, `tst_opcua_wire`
  coverage). New drivers include it; no rename, no second vocabulary.
- **Native template host**: `app/src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp`
  — the existing `OpcUaParser` walk (line ~2449) is factored into a small shared
  wire-latch helper so `sparkplug`, `s7`, `ethernetip`, `iec104` templates are thin
  schema-param wrappers, registered under `BUILD_COMMERCIAL` beside `opcua`.
- **Project generation**: each inbound feature ships a `buildProject()`/`generateProject()`
  twin of `OpcUa.cpp:1517` (sets `Keys::FrameDetection = NoDelimiters`,
  `Decoder = Binary`, `FrameParserLanguage = Native`, template id + schema params, and
  copies non-`Password` `driverProperties()` into `Keys::SourceConn`).
- **Pro gating** (three layers, per existing drivers): CMake `BUILD_COMMERCIAL` source
  blocks; `createDriver()` factory guard (`CommercialToken::current().isValid() &&
  SS_LICENSE_GUARD()`); runtime re-check in `open()` + data path. Sink adds the
  Sessions-style `LemonSqueezy::activatedChanged` force-disable hook (late-activation
  requirement R36).
- **Diagnostics**: plain `quint64` counters on the owning object, pulled at 1 Hz /
  by `io.<bus>.getStatus`-style verbs — never pushed (specs 0033/0035).

## Affected subsystems & files

### Phase 1 — Sparkplug B (A1)

| File | Change |
|------|--------|
| `app/src/IO/Drivers/MQTT/SparkplugPayload.{h,cpp}` | NEW. Minimal hand-rolled Sparkplug B v1.0 protobuf codec: varint/fixed readers, `Payload`/`Metric` structs (scalar datatypes 1-12 + 14-15), NCMD "Node Control/Rebirth" encoder. No libprotobuf (absent from default build). Qt-Core-only so ctest links it standalone. |
| `app/src/IO/Drivers/MQTT/SparkplugSession.{h,cpp}` | NEW. Birth-certificate state machine: per-edge-node/device state, alias→metric tables from NBIRTH/DBIRTH, seq-gap detection, bounded pre-birth buffer (drop+count on overflow), NDEATH/DDEATH stale marking, rebirth-request trigger, pulled diagnostic counters. Emits slot-index value updates. |
| `app/src/IO/Drivers/MQTT.{h,cpp}` + `app/src/IO/Drivers/MQTT/MQTTSparkplug.cpp` (NEW second TU, same class — `MQTT.cpp` sits at the 1500-line TU cap) | Properties `sparkplugEnabled`, `sparkplugGroupId`; topic subscription switches to `spBv1.0/<group>/#` when enabled; `onMessageReceived` routes to session; delta-frame tick encoding via `OpcUaWire`; NCMD publish via `m_client.publish()` (driver stays `isWritable() == false`); `driverProperties()`/`setDriverProperty()` round-trip; `buildProject()` from discovered metrics; license re-checks unchanged. Sparkplug method bodies live in the new TU. |
| `app/src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp` | Factor shared wire-latch parser out of `OpcUaParser`; register `sparkplug` template (`BUILD_COMMERCIAL`). |
| `app/qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml` | Sparkplug checkbox + group-id field + "Create Project from Births" button, `visible:`-gated rows (sslEnabled idiom). |
| `app/tests/tst_sparkplug_payload.cpp`, `tst_sparkplug_session.cpp` + `app/tests/CMakeLists.txt` | NEW ctest units (AC1). |
| `app/CMakeLists.txt` | New sources in the `BUILD_COMMERCIAL` block. |

### Phase 2 — J1939 TP + ISO-TP (A2) and extended mux (A3)

| File | Change |
|------|--------|
| `app/src/IO/Drivers/CANBus/CanReassembly.{h,cpp}` | NEW. `J1939TransportReassembler` (BAM via TP.CM/TP.DT PGNs 0xEC00/0xEB00; RTS/CTS listen-only observing both directions, best-effort when CTS unseen) + `IsoTpReassembler` (FF/CF/FC, applied to the ISO 15765-4 standardized diagnostic ID ranges only). Fixed caps: concurrent sessions, per-session bytes (1785 J1939 / 4095 ISO-TP), timeout eviction; abort/timeout/cap drops counted; pure C++, Qt-Core-only. |
| `app/src/IO/Drivers/CANBus.{h,cpp}` | New checkbox property `tpReassembly` (default off, persisted, in `driverProperties()`); `onFramesReceived` feeds TP frames to the reassemblers and publishes completed payloads as synthesized extended-format frames (DLC byte = 0xFF marker for >64-byte payloads, which bypass the current 64-byte drop); counters exposed for 1 Hz pull. |
| `app/src/DataModel/Importers/DBCImporter.{h,cpp}` | A3: `classifyMux()` accepts `SwitchAndSignal`, multi-parent, and range multiplexing; `OrderedSignal` carries a mux spec (parent name + ranges); topological selector-first ordering; `signalSpecLine()` emits `mux = {p="Sig", r={{lo,hi},...}}`; `parse()` loop keeps a by-name selector-value table and matches ranges (nested chains included); warning string reduced to the genuinely-unsupported remainder (non-integer range bounds). |
| `app/qml/MainWindow/Panes/SetupPanes/Drivers/CANBus.qml` | Reassembly checkbox row. |
| `app/tests/tst_can_reassembly.cpp`, `tst_dbc_importer.cpp` + `app/tests/CMakeLists.txt` | NEW ctest units (AC4, AC7): canned TP/ISO-TP sequences incl. interleave/abort/cap-overrun; DBC mux classification + generated-Lua golden checks against extended-mux fixtures. |
| `tests/fixtures/` (dbc) | NEW extended-mux DBC fixtures (nested selectors, range lists). |

### Phase 3 — S7comm (B1) + EtherNet/IP (B2)

| File | Change |
|------|--------|
| `lib/snap7/`, `lib/libplctag/` + `lib/CMakeLists.txt`, `REUSE.toml` | NEW dependency modules, FETCHED at configure time (libusb precedent) rather than vendored: Serial-Studio-authored `CMakeLists.txt`, `target_link_snap7()`/`target_link_libplctag()` + unconditional no-op stubs, `SS_ENABLE_S7`/`SS_ENABLE_EIP` root options, REUSE annotations. **SUPERSEDED for Snap7 (2026-08-27, tasks T65-T68): the module, the option and the link stub are gone and the S7comm client is first-party — see the Tradeoffs row.** Only libplctag (MPL-2.0, static) remains. |
| `app/src/IO/Drivers/S7.{h,cpp}` | NEW driver. Endpoint (IP/rack/slot), flat variable list `QVector<S7Variable>` (Modbus register-group pattern) persisted as QSettings JSON + `driverProperties()`; poll worker `QThread` (blocking reads never on the GUI thread; HID `quit()`-before-`wait()` teardown; the transport is the first-party `S7/IsoTsap` + `S7/S7Pdu` pair since T65); values latched → `OpcUaWire` delta frames; `s7` template; `buildProject()`. |
| `app/src/IO/Drivers/S7Address.{h,cpp}` | NEW. Parser for `DB5.DBD20`-style absolute addresses → (area, db, offset, type). Qt-Core-only, unit-tested. |
| `app/src/IO/Drivers/EthernetIp.{h,cpp}` | NEW driver. Endpoint + CIP path, symbolic tag list (name, type, optional array index) via libplctag; same poll-worker/wire/template/`buildProject()` shape; `ethernetip` template. |
| Registration sweep (both drivers) | `SerialStudio.h` BusType (append after `OpcUa`); `DriverUiRegistry.{h,cpp}` (incl. `kMaxUiDrivers`); `ConnectionManager.{h,cpp}` accessors + `createDriver()` license-gated cases; `Hardware.qml` StackLayout panes (position = enum order); `ModuleManager.cpp` context properties; `EnumLabels.cpp` slugs/labels; `API/Handlers/S7Handler.*`, `EipHandler.*` + `CommandHandler.cpp`; `CLI.{h,cpp}` options; `ProjectEditorShared.h` icons + `ProjectEditorForms.cpp` combobox; `app/rcc/commands/app.json` + `AppCommandBindings.qml`; icons + `rcc.qrc`; `tests/utils/api_client.py` bus_map; `doc/help/` pages + `help.json`. |
| `app/qml/.../Drivers/S7.qml`, `EthernetIp.qml` + variable-list dialogs | NEW setup panes (Modbus groups-dialog pattern). |
| `app/tests/tst_s7_address.cpp`, tag-list round-trip tests | NEW ctest units (AC8 for the in-house parts; protocol engines are the vendored libs'). |

### Phase 4 — IEC 60870-5-104 client (B4)

| File | Change |
|------|--------|
| `app/src/IO/Drivers/Iec104/Apci.{h,cpp}`, `Asdu.{h,cpp}` | NEW in-house codec (see Tradeoffs — no GPLv3 lib in the commercial build): APCI I/S/U framing, k/w windows, t1/t2/t3 timers, STARTDT/TESTFR; ASDU decode for M_SP_NA/M_DP_NA/M_ME_NA/M_ME_NB/M_ME_NC/M_IT_NA + their CP56Time2a twins, C_IC_NA interrogation encode; quality descriptors surfaced. Qt-Core-only, fully ctest-able. |
| `app/src/IO/Drivers/Iec104.{h,cpp}` | NEW driver. QTcpSocket with the spec-0050 blocking-dial pattern; general interrogation on STARTDT con; dynamic IOA→slot latch table (OPC UA discovery precedent); wire frames + `iec104` template; `buildProject()` from interrogated points; quality/supervision counters pulled. |
| Registration sweep | Same checklist as Phase 3, one driver. |
| `app/tests/tst_iec104_apci.cpp`, `tst_iec104_asdu.cpp` | NEW ctest units against canned exchanges incl. malformed input (AC8). |

### Phase 5 — InfluxDB sink (S1)

| File | Change |
|------|--------|
| `app/src/InfluxDB/LineProtocol.h` | NEW header-only line-protocol formatter (escaping, ns timestamps, batching boundaries) — unit-testable without network. |
| `app/src/InfluxDB/Export.{h,cpp}` | NEW `FrameConsumer<DataBlockPtr>` sink. Worker owns its `QNetworkAccessManager` created on the worker thread (`MQTT::PublisherWorker` precedent); batches blocks → one HTTP POST per batch to the v2 `/api/v2/write` endpoint; bounded queue (8192); sink-local dropped-point/HTTP-error counters (base `FrameConsumer` has none — deliberate sink-local bookkeeping); source timestamps from block timebase, never re-stamped; Pro-gated with `activatedChanged` force-disable (Sessions pattern). |
| `app/src/DataModel/FrameBuilder.cpp` | The three sink touch-points: `enabledChanged → refreshAnyAsyncSink()` connect (~line 839), OR-term in `refreshAnyAsyncSink()` (~992), `ingestBlock` call in `publishBlock` (~3315, `BUILD_COMMERCIAL` block). **Hotpath file — implementer reads it in full + invokes `ss-hotpath` first.** |
| `app/src/DataModel/ProjectModel.*` (+ `Frame.h` `Keys::`) | Project-file `QJsonObject` config (`influxSink`: url, org, bucket, measurement mapping, enabled) mirroring `mqttPublisher()`; token NOT in the project file — `MQTT::CredentialVault` scope `"influxdb"`. |
| `app/src/API/Handlers/` + `DashboardHandler.cpp:328` doc string | `influx.*` verbs (setConfig/getStatus/enable); update the tick doc string that enumerates sinks. |
| `app/qml/MainWindow/Panes/Setup.qml` + `app/qml/ProjectEditor/Views/InfluxSinkView.qml` | Enable switch beside CSV/MDF4/Historian; config view (MqttPublisherView pattern). |
| `app/tests/tst_influx_lineprotocol.cpp` | NEW ctest unit (AC11). |

## Architecture & data flow

- **Inbound (all phases 1-4)**: protocol I/O and state live driver-side (GUI thread, or a
  driver-owned poll worker for the blocking S7/EIP libraries), exactly one
  `publishReceivedData(bytes, sourceTimestamp)` per delta tick / reassembled frame →
  queued `dataReceived` → `FrameReader` (`NoDelimiters`: chunk == frame) → `FrameBuilder`
  → native template latch → datasets. No new lane, no pipeline-thread objects, no change
  to FrameReader/CircularBuffer.
- **Sparkplug session flow**: MQTT message → `SparkplugSession::ingest(topic, payload)` →
  (birth: rebuild metric table, flush pre-birth buffer; data: resolve aliases, update
  slots; death: mark stale + synthesize per-node `Online` LED metric = 0 satisfying R5
  observably; desync: count + emit rebirth request) → dirty slots encoded on the existing
  frame-tick cadence.
- **CAN reassembly flow**: `onFramesReceived` intercepts TP PGNs / ISO-TP PCI frames
  before byte serialization; non-TP frames publish exactly as today (R12); completed
  sessions publish one synthesized long frame; the generated DBC Lua decodes it
  unchanged (its `frame_id()`/`extract()` never read DLC, payload base is fixed at 6).
- **Sink flow**: `publishBlock` → shared `clone_block_trimmed` → `InfluxDB::Export::
  ingestBlock` guard (`enabled && !isAnyPlayerOpen`) → lock-free enqueue → worker batch →
  line protocol → HTTP POST. Replay masking and RepublishGate semantics inherited from
  the existing fan-out — the sink is downstream of `m_maskSinks`, so replay can never
  re-record (spec 0064 untouched).

## Hotpath & threading impact

- **Touches the hotpath?** Phase 5 only, and only the `FrameBuilder` sink fan-out
  (three localized edits listed above). No edits to `FrameReader`, `CircularBuffer`,
  the span lane, parse, or staging. Sink off ⇒ identical code path (one more term in a
  flag refresh that runs at toggle rate). Phases 1-4 are upstream of the pipeline (driver
  boundary) plus generated-Lua/native-template additions that ride existing decode paths.
  `--benchmark-hotpath` full tier table must pass unchanged after every phase (AC6/AC16);
  implementer reads `FrameBuilder.cpp` in full and invokes `ss-hotpath` before the Phase 5
  fan-out task and before any `BinaryTemplates.cpp` edit.
- **New cross-thread signal/slot?** S7/EIP poll workers publish via
  `publishReceivedData` from their worker thread — `dataReceived` is already
  auto-queued to the pipeline; timestamps captured on the worker before queueing
  (common-mistakes rule). Worker teardown uses `quit()` before `wait()` (HID
  `cleanupDevice()` reference). InfluxDB worker uses the existing `FrameConsumer`
  machinery — no new connection types.
- **New input to a cached hotpath flag?** Yes, one: `InfluxDB::Export::enabledChanged`
  must be wired into `refreshAnyAsyncSink()` alongside the OR-term — missing either half
  is the "valid-looking empty recording" failure; the task list pins both edits together
  with `tst`-level and integration verification.
- **Timestamp ownership**: every inbound feature stamps at the driver boundary
  (Sparkplug: broker-delivery steady time per tick, earliest-dirty like OPC UA;
  reassembly: first-packet capture time; S7/EIP/IEC104: poll/receive time on the worker,
  IEC104 preferring CP56Time2a mapped via the OPC UA-style connect offset). The sink
  writes block timebase values, never `now()`.

## Data model & persistence

- No project schema bump. New driver configs ride the existing opaque
  `Keys::SourceConn` snapshot (`driverProperties()` round-trip, `Password`-typed
  values excluded). Sparkplug/reassembly options are new driver properties in the same
  snapshot — old projects simply lack the keys (defaults off, R37).
- Influx sink config: new top-level project object (`Keys::` addition in `Frame.h`,
  serialized beside `mqttPublisher`); absent ⇒ disabled. Token in `CredentialVault`
  (machine-bound QSettings ciphertext), satisfying R34.
- Variable/tag lists (S7, EIP) persist as QSettings JSON arrays (OPC UA `tags` pattern)
  AND surface through `driverProperties()` so projects capture them.
- No Sessions DB schema change in any phase.

## API / SDK surface

- New handlers: `io.s7.*`, `io.eip.*`, `io.iec104.*` (setProperty/getStatus/
  generateProject verbs mirroring `io.opcua.*`), `influx.*`; `EnumLabels.cpp` slugs
  (`s7`, `ethernetip`, `iec104`) behind `BUILD_COMMERCIAL`; `tst_enum_labels` keeps
  coverage honest. MQTT/CAN options reach the API through the existing generic
  `setDriverProperty` surface — no new handler for Tier A. gRPC untouched (no dataset
  property changes ⇒ no field-number ledger edits). `bus_map` in
  `tests/utils/api_client.py` gains the three new buses.

## QML / UI

- Tier A: rows added inside the existing `MQTT.qml` / `CANBus.qml` panes
  (BoundField + `visible:` gating idioms already in those files). No new pane.
- Tier B: three new `SetupPanes/Drivers/*.qml` panes + variable-list dialogs
  (ModbusGroupsDialog pattern), loaded in `Hardware.qml`'s StackLayout **in enum
  order**, inside `Loader { active: Cpp_CommercialBuild }`.
- Phase 5: Setup.qml switch (gated `dataExportAllowed && Cpp_CommercialBuild`) +
  `InfluxSinkView.qml` project-editor view.
- ComboBox restore-race and focus/echo rules per common-mistakes (unfocused-sync
  BoundField idiom).

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Sparkplug decode | Lua codegen (ProtoImporter-style); vendor libprotobuf/Eclipse Tahu; driver-side hand-rolled codec | **Hand-rolled driver-side codec** (maintainer-confirmed 2026-08-27: libprotobuf via the gRPC dep is allowed, but independent preferred) — birth/alias/seq state and NCMD publishing cannot live in a stateless parser; libprotobuf is absent from `ENABLE_GRPC=OFF` builds and Sparkplug's payload schema is small and frozen (v1.0). |
| Sparkplug dataset discovery | Fully dynamic datasets on every birth; explicit user-triggered `buildProject()` from births | **User-triggered generation** (OPC UA precedent) — mid-session structural churn fights the StructureSnapshot model; new metrics after generation are counted + surfaced, regenerate on demand. |
| Reassembly location | FrameReader/pipeline stage; CAN driver callback | **Driver callback** — structured ID+payload frames exist only there; keeps the 256 kHz path untouched and the reassembler a pure testable class. |
| ISO-TP scope | Heuristic on all IDs; user-configured ID pairs; standardized ISO 15765-4 ranges only | **Standardized ranges only** — deterministic, zero config, no false positives on non-diagnostic traffic. |
| >255-byte frame encoding | New wire format; DLC=0xFF marker on the existing extended layout | **DLC marker** — generated Lua never reads DLC; zero compatibility break. |
| IEC 104 stack | Vendor lib60870 (GPLv3, dual-licensed by MZ); in-house client subset | **In-house** (maintainer-confirmed 2026-08-27) — GPLv3 cannot link into the commercial build; the client subset (8 ASDU types + APCI) is bounded and fully unit-testable. |
| S7/EIP read scheduling | GUI-thread timer (OPC UA/Modbus pattern); driver-owned poll worker thread | **Poll worker** — Snap7/libplctag calls block; a slow PLC would freeze the UI. Audio/HID worker precedents; strict quit-before-wait teardown. |
| Influx config home | QSettings (CSV/MDF4 pattern); project file (MQTT Publisher pattern) | **Project file** — multi-field endpoint config belongs with the project; token alone goes to the vault. |
| Drop counting in sink | Extend `FrameConsumer` base (affects six sinks); sink-local counters | **Sink-local** — no behavior change for existing sinks; base-class change is its own future spec if wanted. |
| Influx protocol | v1.x + v2.x; v2.x only | **v2.x only** (spec open question resolved: v1 deferred). |
| Snap7 linkage | static; dynamic; **no Snap7 at all** | **REMOVED — the S7comm client is first-party** (maintainer-directed 2026-08-27, superseding the dynamic-linkage decision recorded here earlier the same day). Two independent reasons closed it: Snap7 does not compile on libc++ with C++20 (its own `byte` typedef collides with `std::byte`, and it reaches for `abi::__forced_unwind`), and being LGPL-3.0 it could only combine with the proprietary build under §4 — a shipped, swappable shared object plus a relink path we would have to keep open in every release, forever, for one driver. The in-house stack (`app/src/IO/Drivers/S7/IsoTsap.*` + `S7Pdu.*`) is the same call this spec already made for IEC 104: TPKT/COTP and the S7 read service are a bounded, fully unit-testable wire format, and the codec halves are Qt-Core-only, so the whole protocol is driven by `tst_s7comm_isotsap` / `tst_s7comm_pdu` with no controller. The build now carries no LGPL dependency anywhere. |

## Risks & mitigations

- **The LGPL obligation was removed rather than managed** (see Tradeoffs). It would have been
  a RELEASE-time risk, not a build-time one: dynamic linkage satisfies LGPL-3.0 §4 only while
  the shared library actually ships as a replaceable file, so a packaging change that folded
  it into the binary would have reintroduced the violation with a green build. No dependency
  in the tree is LGPL today. Any future one must be checked against proprietary distribution,
  not only against the GPL, before it is added.
- **Silent empty recordings** (cached-flag miss): both `refreshAnyAsyncSink` edits land
  in one task with an integration test that records against a live block stream.
- **Wedged connect button** (async-dial verdict): S7/EIP/IEC104 use synchronous opens
  (blocking dial on `open()`, Network-TCP precedent) — no `openFinished` latch to
  mis-emit; drops route through `disconnectDevice(this)` with queued error reporting.
- **Untrusted network input**: all three new codecs + both reassemblers + Sparkplug
  payload parser are bounds-checked, cap-limited, fuzz-style unit-tested with malformed
  fixtures; no allocation growth with hostile traffic (fixed session/buffer caps).
- **Ctor-edge / composition root**: no new singletons in core-module order; sink follows
  Sessions::Export shape (self-owned singleton wired in `setupExternalConnections`).
  Singleton census will grow — re-seed via `--singleton-census --accept` consciously,
  named in chat at that task.
- **Warning-string translation churn** (A3): string change confined to one `tr()` site;
  `lupdate`/`llm_translate.py` sweep is the maintainer's release step, noted in tasks.
- **StackLayout order drift** (Hardware.qml vs enum): one task adds both sides together
  and the API integration test connects each new bus by slug.
- **Generated-file discipline**: no dataset-property manifest changes needed; if a task
  discovers otherwise, stop and amend plan (registry edits are their own workflow).
- **`sessionClosed` semantics**: new drivers never emit it on drops (io.md rule);
  reconnect UX is driver-local like UART/Modbus.

## Test & verification plan

| AC | Check |
|----|-------|
| AC1 | `ctest`: `tst_sparkplug_payload`, `tst_sparkplug_session` (birth/data/death/seq-gap/pre-birth/unsupported-datatype fixtures). |
| AC2 | Existing MQTT integration tests unchanged (`pytest tests/integration/` vs running app). |
| AC3 | Maintainer: Mosquitto + Sparkplug simulator observation. |
| AC4 | `ctest`: `tst_can_reassembly` (BAM/RTS-CTS/ISO-TP/interleave/abort/timeout/cap fixtures + diag counts). |
| AC5 | Maintainer: J1939 log replay against DBC. |
| AC6/AC16 | Maintainer/CI: `--benchmark-hotpath` full tier table on PGO binary, per phase. |
| AC7 | `ctest`: `tst_dbc_importer` (classification + Lua golden output vs extended-mux fixtures); runtime decode via an R14/R15-style `test_cpp_regressions.py` addition (integration, app up). |
| AC8 | `ctest`: `tst_s7_address`, `tst_iec104_apci`, `tst_iec104_asdu`, tag-list round-trips. |
| AC9 | Maintainer: Snap7 demo server / real PLC or simulator / IEC 104 test server (e.g. a lib60870 CLI server on another box). |
| AC10 | `pytest tests/integration/`: bus listed via API, config round-trip through project save/load, connect/disconnect per driver (app up, API server on). |
| AC11 | `ctest`: `tst_influx_lineprotocol` + bounded-buffer drop behavior in `tst`-level worker test. |
| AC12 | Maintainer: unreachable endpoint observation + live InfluxDB point arrival. |
| AC13 | Maintainer: unactivated build gating + live activation flip. |
| AC14 | `pytest`: project-file load/save/load byte-stability incl. new configs. |
| AC15 | `reuse lint` (CI) after each vendored-lib task. |
| Static | `scripts/code-verify.py --check` per task; `qt-cpp-review` before each phase handoff; `sanitize-commit.py` before every commit. |
