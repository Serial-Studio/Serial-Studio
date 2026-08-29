---
spec: 0073-protocol-expansion
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-08-27
---

# Tasks 0073 — Industrial Protocol Expansion

> **Phase 3 of 4 — the ordered checklist.** Decomposes [`plan.md`](./plan.md). Five delivery
> phases; each phase ends with its own verification task and leaves the tree shippable.
> Sized for execution by a smaller model: every task is a narrow diff with its own check.
> Implementer rules that apply to EVERY task: read each file in full before editing it;
> no comments inside function bodies; `SS_ASSERT` density ≥2/function; `Q_EMIT`;
> `[[nodiscard]]`; no in-header member init; run
> `python scripts/code-verify.py --check <files>` before ticking the box. Tasks touching
> `FrameBuilder`, `BinaryTemplates.cpp`, or driver publish paths must invoke `ss-hotpath`
> first and re-state the named invariant from the Does line before editing.

## Conventions

- One task = one focused, reviewable change (≤3 files unless it is a pure registration
  sweep item that is meaningless split).
- **Verify** = how this unit is confirmed. ctest targets are runnable against an existing
  build dir only — when no build exists, verify structurally (code-verify + read-back)
  and leave the ctest run to the maintainer/CI; never invoke cmake or a compiler.
- **Deps** = task IDs that must land first.

## Phase 1 — Sparkplug B (A1)

### T1 — Sparkplug payload codec

- **Files:** `app/src/IO/Drivers/MQTT/SparkplugPayload.h`, `.cpp`
- **Does:** Hand-rolled Sparkplug B v1.0 protobuf codec, Qt-Core-only, `BUILD_COMMERCIAL`
  SPDX like `MQTT.h`. Decode: varint/fixed readers with strict bounds checks,
  `Payload {timestamp, seq, metrics[]}` and `Metric {name, alias, timestamp, datatype,
  isNull, value}` for scalar datatypes Int8..UInt64, Float, Double, Boolean, String
  (datatype codes 1-12, 14-15); unknown fields skipped by wire type; malformed input
  returns error, never UB. Encode: NCMD payload containing metric
  `"Node Control/Rebirth" = true`. Fixed caps: payload ≤ 256 KiB, ≤ 2048 metrics.
- **Verify:** `code-verify.py --check` clean; read-back against the Sparkplug B v1.0
  payload field numbers documented in the header comment.
- **Deps:** none
- [x] done — `IO::Drivers::SparkplugB` namespace; deferred value-oneof resolution
  (datatype may follow value on the wire); code-verify 0/0

### T2 — Payload codec unit tests

- **Files:** `app/tests/tst_sparkplug_payload.cpp`, `app/tests/CMakeLists.txt`
- **Does:** ctest unit (registered like `tst_opcua_wire`): golden byte fixtures for each
  scalar datatype, alias-only metrics, truncated/oversized/hostile buffers (no crash, no
  over-read), NCMD encode round-trip through the decoder.
- **Verify:** `ctest -R sparkplug_payload` against an existing build dir if present;
  otherwise structural check + maintainer runs.
- **Deps:** T1
- [x] done — 14 test slots incl. hostile-input table + every-prefix sweep;
  `ss_add_unit_test` registration; code-verify 0/0; ctest run pending build (maintainer)

### T3 — Sparkplug session state machine

- **Files:** `app/src/IO/Drivers/MQTT/SparkplugSession.h`, `.cpp`
- **Does:** Qt-Core-only class consuming `(topic, payload bytes)`: parses the
  `spBv1.0/<group>/<msgtype>/<edge>/[device]` namespace; NBIRTH/DBIRTH build per-node
  alias→slot tables (slot = stable index for wire encoding); NDATA/DDATA resolve aliases
  and update latched values; unresolvable data buffered in a bounded pre-birth queue
  (cap 256 messages, overflow drops oldest + counts); seq-gap detection arms a
  rebirth-needed flag per node; NDEATH/DDEATH zero the node's synthetic `Online` slot
  (one auto-metric per edge node, R5); unsupported datatypes skipped + counted (R6).
  Plain `quint64` pulled counters (spec 0033 style): preBirthBuffered, preBirthDropped,
  seqGaps, unsupportedMetrics, rebirthRequests. No Qt signals per message.
- **Verify:** `code-verify.py --check`; counters and caps read back against spec R3/R4/R11.
- **Deps:** T1
- [x] done — accepted deviations: DDEATH marks the device unborn (Online stays node-scoped),
  SchemaEntry carries {node, device, name, index}, DBIRTH without NBIRTH tolerated,
  reset() zeroes counters, hasDirtySlots() added; code-verify 0/0

### T4 — Session unit tests

- **Files:** `app/tests/tst_sparkplug_session.cpp`, `app/tests/CMakeLists.txt`
- **Does:** ctest unit: NBIRTH→NDATA happy path; DDATA-before-DBIRTH buffering then
  flush on birth; buffer overflow drop+count; seq gap → rebirth flag; NDEATH → Online
  slot 0 + stale; unsupported datatype skip+count; alias collision across nodes.
- **Verify:** `ctest -R sparkplug_session` (existing build dir) or structural + maintainer.
- **Deps:** T2, T3
- [x] done — 12 slots covering births/buffering/overflow/seq-gap/deaths/cap/reset;
  code-verify 0/0; ctest run pending build (maintainer)

### T5 — MQTT driver Sparkplug properties

- **Files:** `app/src/IO/Drivers/MQTT.h`, `.cpp`
- **Does:** Add `sparkplugEnabled` (bool) + `sparkplugGroupId` (string) Q_PROPERTYs with
  guard-return setters + `mqttConfigurationChanged` emission, QSettings persistence in
  `loadPersistedSettings()`/`settingsKey()` leaves, and BOTH `driverProperties()` and
  `setDriverProperty()` entries (project round-trip dies if either half is missed, R37).
  When enabled, the effective subscription filter becomes `spBv1.0/<group>/#` (empty
  group ⇒ `spBv1.0/#`); `configurationOk()`/`open()` accept an empty user topic filter
  while Sparkplug is on (the namespace filter replaces it). No decode wiring yet.
- **Verify:** `code-verify.py --check`; read-back: property appears in both round-trip
  functions; header order rules hold.
- **Deps:** none
- [x] done — both round-trip halves + effectiveTopicFilter() at subscribe/match/open
  sites; code-verify 0/0; MQTT.cpp at the 1500-line cap ⇒ T6/T9 land in a new TU

### T6 — MQTT decode wiring + delta frames + NCMD

- **Files:** `app/src/IO/Drivers/MQTT.h`, `app/src/IO/Drivers/MQTT/MQTTSparkplug.cpp`
  (NEW second TU of the MQTT class — `VerifierRegression.cpp` precedent; `MQTT.cpp` sits
  at the 1500-line TU cap, so Sparkplug method bodies live here; `MQTT.cpp` gets only
  the minimal routing hook in `onMessageReceived`)
- **Does:** Own a `SparkplugSession` member; in `onMessageReceived`, when
  `sparkplugEnabled`, route `(topic, message)` into the session instead of raw
  `publishReceivedData` (raw path byte-identical when disabled — R1/AC2); a driver
  QTimer tick (reuse the pattern of `OpcUa::m_frameTimer`, ~30 Hz) encodes dirty slots
  into one `OpcUaWire` frame and calls `publishReceivedData(std::move(frame), ts)` with
  the earliest dirty metric's timestamp mapped like OPC UA's `toSteady()` (source owns
  time — never default-stamp on a queued hop); a set rebirth-needed flag publishes ONE
  NCMD via `m_client.publish()` on the node's NCMD topic, rate-limited to 1 per 5 s per
  node; license re-check stays first in the message handler.
- **Verify:** `code-verify.py --check`; read-back: disabled path untouched; timestamp
  captured before any queueing; NCMD rate limit present.
- **Deps:** T3, T5
- [x] done — MQTTSparkplug.cpp second TU (296 lines); 30 Hz tick, offset-mapped
  monotonic stamps, 5 s/node NCMD limit; MQTT.cpp at 1498 (orphan duplicate comment
  removed as offset — named + accepted); code-verify 0/0

### T7 — Factor shared wire-latch native parser

- **Files:** `app/src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp`
- **Does:** Extract the OpcUaParser wire-walk (`[version][index u16][type u8][payload]*`
  latch loop, ~line 2449) into a file-local reusable base (schema param → slot map)
  with **zero behavior change** for the `opcua` template. Hotpath-adjacent: invoke
  `ss-hotpath`; no allocation added to the per-frame walk; `SS_ASSERT_HOTPATH` retained.
- **Verify:** `ctest -R opcua_wire` unchanged (existing build) / structural; diff shows
  pure factor, no semantic edits.
- **Deps:** none
- [x] done — WireLatchParser base (template-param route, zero new dispatch); code-verify clean

### T8 — Register `sparkplug` native template

- **Files:** `app/src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp`
- **Does:** `SparkplugTemplate` (`id() == "sparkplug"`, tr()'d metadata, `schema` param
  of `{index, name}` entries) instantiating the shared latch parser; registered in the
  `BUILD_COMMERCIAL` block beside `opcua`; rejects duplicate indices like the OPC UA
  template.
- **Verify:** `code-verify.py --check`; read-back registration + catalog entry.
- **Deps:** T7
- [x] done — SparkplugParser/SparkplugTemplate + registry entry + sparkplug.md in qrc;
  code-verify 0 errors (baseline tu-too-long advisory only)

### T9 — Sparkplug `buildProject()`

- **Files:** `app/src/IO/Drivers/MQTT.h`, `app/src/IO/Drivers/MQTT/MQTTSparkplug.cpp`
- **Does:** Public slot `generateProject()` mirroring `OpcUa::buildProject()`
  (`OpcUa.cpp:1517`): one group per edge node/device, one dataset per discovered metric
  + the per-node `Online` LED dataset, `Keys::FrameDetection = NoDelimiters`,
  `Decoder = Binary`, `FrameParserLanguage = Native`, `FrameParserTemplate =
  "sparkplug"`, `FrameParserParams = {schema}`, non-Password `driverProperties()` into
  `Keys::SourceConn`; hands the JSON to `ProjectModel::importProjectFromJson` path used
  by OPC UA. Counter for metrics discovered after generation (surfaced via getStatus).
- **Verify:** `code-verify.py --check`; read-back against the OPC UA generator shape.
- **Deps:** T6, T8
- [x] done — buildSparkplugProject() in the Sparkplug TU; datagrid groups per
  node/device, LED for booleans, 1-based wire-index binding, OPC UA UX flow +
  markGenerated(); code-verify 0/0

### T10 — MQTT setup-pane rows

- **Files:** `app/qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml`
- **Does:** Sparkplug checkbox + group-id BoundField + "Create Project from Births"
  button (enabled only while connected + sparkplugEnabled), using the pane's existing
  label/BoundField grid and the `visible:` gating idiom of the SSL rows; ComboBox-free,
  so no restore-race risk.
- **Verify:** `code-verify.py --check`; read-back bindings target `Cpp_IO_Mqtt`.
- **Deps:** T5, T9
- [x] done — checkbox + Group ID BoundField + Create Project button
  (`Cpp_IO_Manager.isConnected` gate); code-verify 0/0 after christmas-tree --fix

### T11 — Phase 1 verification

- **Files:** none (checks only)
- **Does:** `code-verify.py --check` sweep over every Phase 1 file; `qt-cpp-review` on
  the C++ diff; confirm ctest targets build+pass (maintainer if no build dir);
  maintainer AC3 observation + AC6 benchmark run noted in chat; tick AC1/AC2 boxes in
  spec.md if green.
- **Verify:** all listed gates green or explicitly handed to maintainer.
- **Deps:** T1-T10
- [ ] done

## Phase 1b — Sparkplug outbound publishing (A1b, spec amendment 2026-08-27)

> Added after Phase 1 landed. Rides `MQTT::Publisher` (the project publisher sink), NOT the
> inbound driver — they are separate objects with separate brokers by design.
> Depends on the Phase-1 codec (`SparkplugPayload`) for encoding.

### T59 — Sparkplug payload encoder

- **Files:** `app/src/IO/Drivers/MQTT/SparkplugPayload.h`, `.cpp`
- **Does:** Extend the codec with the encode direction the decoder already mirrors:
  `encodePayload(const Payload&)` writing timestamp/seq/metrics, metric name+alias+
  datatype+value for the scalar set, and `encodeBirth(...)`/`encodeData(...)` helpers.
  `encodeRebirthRequest` stays. Pure function, Qt-Core-only, unit-testable.
- **Verify:** round-trip through `decodePayload` in `tst_sparkplug_payload`.
- **Deps:** T1
- [x] done — encode direction; numericFitsDataType is the single R44 rule

### T60 — Edge-node publisher state machine

- **Files:** `app/src/MQTT/SparkplugPublisher.h`, `.cpp` (NEW)
- **Does:** Owns the outbound lifecycle: metric registry (dataset uniqueId → stable alias),
  `bdSeq` per connection and `seq` modulo 256 across ALL messages including births (R41),
  NBIRTH/DBIRTH payload construction, NDATA/DDATA change-only deltas addressed by alias
  (R40), and the NDEATH will payload (R42). No QObject, no I/O — it produces topics and
  payloads for the worker to publish, so it is testable without a broker.
- **Verify:** `code-verify.py --check`; seq/bdSeq rules read back against R41.
- **Deps:** T59
- [x] done — seq mod 256 across ALL messages; bdSeq once per connection, will built in the same call

### T61 — Publisher worker integration

- **Files:** `app/src/MQTT/Publisher.h`, `.cpp`
- **Does:** Sparkplug mode on the publisher (off by default — raw/JSON publishing stays
  byte-identical, R39): group/edge-node/device config; register the NDEATH will BEFORE
  connecting (a will set after connect never arms, R42); emit NBIRTH on connect;
  map `ingestBlock` values to NDATA metrics; subscribe the node's NCMD topic and
  re-publish births on `Node Control/Rebirth`, counting other commands (R43).
  Unrepresentable values are skipped and counted, never published mistyped (R44).
- **Verify:** `code-verify.py --check`; will-before-connect ordering read back.
- **Deps:** T60
- [x] done — will registered before connectToHost — a will set after connect never arms

### T62 — Publisher unit tests

- **Files:** `app/tests/tst_sparkplug_publisher.cpp`, `app/tests/CMakeLists.txt`
- **Does:** ctest unit: birth declares every metric with aliases; data carries only
  changed metrics by alias; seq wraps at 256 across births and data; bdSeq increments per
  connection and matches the will; rebirth command re-emits the birth; unsupported value
  skipped + counted.
- **Verify:** `ctest -R sparkplug_publisher` (existing build) or structural.
- **Deps:** T61
- [x] done — 10 cases, every payload round-trips through the decoder

### T63 — Publisher UI + persistence

- **Files:** `app/qml/ProjectEditor/Views/MqttPublisherView.qml`,
  `app/src/DataModel/Project/ProjectEditorMqtt.cpp`
- **Does:** Sparkplug rows in the existing publisher view (enable, group, edge node,
  device), persisted in the existing `mqttPublisher()` project JSON object so old
  projects load unchanged (R37).
- **Verify:** `code-verify.py --check`; project round-trip.
- **Deps:** T61
- [x] done — rows in the existing publisher form model; QML needed no change

### T64 — Phase 1b verification

- **Files:** none
- **Does:** code-verify sweep; `qt-cpp-review` on the diff; maintainer: publish into a
  Sparkplug-aware broker (Ignition/HiveMQ/MQTT Explorer with a Sparkplug decoder) and
  confirm births/data/death are read correctly by a third-party host. Tick R39-R44.
- **Deps:** T59-T63
- [ ] done

## Phase 2 — J1939 TP + ISO-TP (A2) and extended mux (A3)

### T12 — J1939 transport reassembler

- **Files:** `app/src/IO/Drivers/CANBus/CanReassembly.h`, `.cpp`
- **Does:** Qt-Core-only `J1939TransportReassembler`: feed(id29, payload, timestamp) →
  optional completed `{pgn, sourceAddr, bytes, firstTimestamp}`. Handles TP.CM BAM
  (0xEC00 control 32) + TP.DT (0xEB00) and RTS/CTS listen-only (RTS control 16 opens a
  session even when CTS is never observed — spec open-question resolution). Fixed caps:
  ≤16 concurrent sessions, ≤1785 bytes each, 750 ms inter-packet timeout eviction;
  aborts (control 255), gaps, timeouts and cap overruns drop the whole session and
  increment plain `quint64` counters (R9/R11 — never a partial frame out).
- **Verify:** `code-verify.py --check`; caps and counters read back against R11.
- **Deps:** none
- [x] done — `CanReassembly.{h,cpp}` (Qt Core, QObject-free); caps 16 sessions / 1785 B /
  750 ms; RTS opens listen-only; session key (SA, DA); abort tears down both key orders;
  `Completed` carries the observed priority so the driver can rebuild the ID; code-verify 0/0

### T13 — ISO-TP reassembler

- **Files:** `app/src/IO/Drivers/CANBus/CanReassembly.h`, `.cpp`
- **Does:** `IsoTpReassembler` in the same TU: FF/CF (FC observed, never sent) for the
  ISO 15765-4 ranges only (11-bit 0x7E0-0x7EF; 29-bit 0x18DAxxyy/0x18DBxxyy), ≤4095
  bytes, sequence-number check, same cap/timeout/counter discipline. Single frames
  (PCI 0) pass through untouched.
- **Verify:** `code-verify.py --check`.
- **Deps:** T12
- [x] done — same TU; 15765-4 ranges only, FC observed; 4095 B cap, 4-bit sequence with
  wraparound; `Completed` carries {canId, extendedId} (pgn/sourceAddr are meaningless here);
  code-verify 0/0

### T14 — Reassembly unit tests

- **Files:** `app/tests/tst_can_reassembly.cpp`, `app/tests/CMakeLists.txt`
- **Does:** ctest unit: BAM happy path; RTS/CTS with and without observed CTS;
  interleaved concurrent sessions from different source addresses; abort mid-session;
  DT gap; timeout eviction; session-cap and size-cap overruns (counted, nothing
  emitted); ISO-TP multi-frame, wrong-sequence CF, out-of-range IDs ignored.
- **Verify:** `ctest -R can_reassembly` (existing build) or structural + maintainer.
- **Deps:** T13
- [x] done — 16 test slots covering BAM/RTS/CTS, interleave, abort, gap, duplicate,
  timeout, both session caps, J1939 size cap, ISO-TP multi-frame/wrong-sequence/out-of-range/
  single-frame/flow-control; `ss_add_unit_test` registration; ctest run pending build (maintainer)

### T15 — CAN driver reassembly option

- **Files:** `app/src/IO/Drivers/CANBus.h`, `.cpp`, `app/qml/.../Drivers/CANBus.qml`
- **Does:** `tpReassembly` bool property (default OFF — TP PGN interpretation on a
  non-J1939 bus corrupts data), guard-return setter, QSettings persistence, BOTH
  `driverProperties()` and `setDriverProperty()` entries; QML checkbox row beside
  listen-only. No decode wiring yet.
- **Verify:** `code-verify.py --check`; both round-trip halves present.
- **Deps:** none
- [x] done — `tpReassembly` default OFF, guard-return setter + QSettings + reassembler
  reset on toggle; BOTH `driverProperties()` and `setDriverProperty()`; QML checkbox row after
  listen-only; code-verify 0/0

### T16 — CAN receive-path wiring

- **Files:** `app/src/IO/Drivers/CANBus.h`, `.cpp`
- **Does:** In `onFramesReceived`: when `tpReassembly`, route TP/ISO-TP candidate frames
  into the reassemblers; completed payloads publish as ONE synthesized extended-format
  frame (`[0x80|id...]`, DLC byte = 0xFF long-frame marker, payload appended in full —
  deliberately bypassing the 64-byte drop, which stays in force for single frames);
  timestamp = the session's FIRST packet capture time (source owns time); **the non-TP
  path stays byte-identical** (R12); reassembly counters merged into the driver's
  pulled diagnostics.
- **Verify:** `code-verify.py --check`; read-back: non-TP path diff-free; timestamp
  provenance correct.
- **Deps:** T13, T15
- [x] done — `routeReassembly()` consumes TP/ISO-TP candidates, `publishReassembled()`
  emits one extended frame with DLC 0xFF and the session's FIRST-packet stamp; the raw path was
  factored verbatim into `serializeCanFrame()` (byte-identical, 64-byte drop intact); counters
  merged into `io.canbus.getConfig` via `reassemblyCounters()`; code-verify 0/0

### T17 — DBC mux classification

- **Files:** `app/src/DataModel/Importers/DBCImporter.h`, `.cpp`
- **Does:** Extend `MuxSpec` onto `OrderedSignal` (parent selector name + sorted
  `{lo,hi}` range list); rewrite `classifyMux()` (~line 755) to accept
  `SwitchAndSignal`, multiple ranges, and non-point ranges — `ExtendedMuxed`
  (= skipped) remains ONLY for range bounds that don't convert to qint64;
  `orderedSignals()` orders selector-role signals (including nested switch-and-signal)
  before their dependents topologically. Preview/`hasImportableSignals()` follow the
  new classification. No codegen change yet.
- **Verify:** `code-verify.py --check`; classification table read back against R13/R14.
- **Deps:** none
- [x] done — gate LIST (multi-parent = AND); topological ordering with cycle drop;
  ExtendedMuxed now only for non-integer bounds; counting via declared - ordered

### T18 — DBC Lua codegen for extended mux

- **Files:** `app/src/DataModel/Importers/DBCImporter.cpp`
- **Does:** `signalSpecLine()` emits `mux = {p="Parent", r={{lo,hi},...}}` for muxed
  signals and `selector = true` for every selector-role signal; `dbcLuaParserBody()`
  `parse()` keeps a by-name table of raw selector values and gates each muxed signal on
  its parent's value falling in any range (nested chains work because selectors decode
  first); LuaJIT 5.1 syntax only (`bit.*`, no `//` or `<<`); simple-mux and plain
  messages produce byte-identical Lua to today (R15). Warning string at ~line 224
  narrowed to the remaining unsupported case; count keeps its name.
- **Verify:** `code-verify.py --check`; generated-Lua golden comparison in T19.
- **Deps:** T17
- [x] done — mux spec `{p,r}` / gate list / legacy scalar form preserved; behaviour
  preservation PROVEN by running LuaJIT old-vs-new across every selector value

### T19 — DBC fixtures + importer unit tests

- **Files:** `tests/fixtures/dbc/extended_mux.dbc` (new dir),
  `app/tests/tst_dbc_importer.cpp`, `app/tests/CMakeLists.txt`
- **Does:** Fixture DBC with simple mux, extended ranges, and a nested
  switch-and-signal chain; ctest unit driving `projectFromMessages()` (session-free by
  design): zero skipped signals on the extended fixture, dataset/group counts, golden
  substring checks on the generated Lua (spec lines + mux-match loop), unchanged output
  for a simple-mux fixture.
- **Verify:** `ctest -R dbc_importer` (existing build) or structural + maintainer.
- **Deps:** T18
- [x] done — extended + simple DBC fixtures, 8 test slots; registration COMMENTED OUT:
  DBCImporter drags in ProjectModel + SessionContext dtor closure (same wall as
  tst_proto_importer) — unblocks with the SessionContext test-double seam

### T20 — Phase 2 verification

- **Files:** none
- **Does:** code-verify sweep; `qt-cpp-review` on the diff; maintainer: AC5 J1939 log
  replay, AC6 benchmark (CAN path upstream, gate must be untouched), R14/R15-style
  runtime decode addition to `tests/integration/test_cpp_regressions.py` identified;
  note the warning-string translation sweep (`lupdate`/`llm_translate.py`) as a
  maintainer release step. Tick AC4/AC7 on green.
- **Verify:** gates green or handed off.
- **Deps:** T12-T19
- [ ] done

## Phase 3 — S7comm (B1) + EtherNet/IP (B2)

### T21 — Vendor Snap7

- **Files:** `lib/snap7/` (sources + `LICENSE` + new `CMakeLists.txt`),
  `lib/CMakeLists.txt`, `REUSE.toml` + `LICENSES/LGPL-3.0-or-later.txt`
- **Does:** Vendor the Snap7 core per the open62541 pattern: static lib, warnings off,
  `SS_SNAP7_VERSION` cache var, `target_link_snap7()` applying `SS_S7_ACTIVE=1`, plus
  the unconditional no-op stub in `lib/CMakeLists.txt` under `SS_ENABLE_S7` (root
  option, default ON); REUSE annotations block. **Never run cmake — structural only.**
- **Verify:** read-back against `lib/open62541/CMakeLists.txt`; `reuse lint` is CI's.
- **Deps:** none
- [x] done — FetchContent (NOT vendored, maintainer-approved): `lib/snap7/CMakeLists.txt`
  builds the upstream tree as one static lib via the SOURCE_SUBDIR download-only trick;
  `SS_SNAP7_GIT_REPOSITORY`/`SS_SNAP7_GIT_TAG` cache vars; unconditional no-op stub in
  `lib/CMakeLists.txt`; REUSE covers only our CMakeLists (no LGPL sources land in-tree)

### T22 — Vendor libplctag

- **Files:** `lib/libplctag/`, `lib/CMakeLists.txt`, `REUSE.toml` + `LICENSES/MPL-2.0.txt`
- **Does:** Same pattern: `target_link_libplctag()`, `SS_ENABLE_EIP`, `SS_EIP_ACTIVE=1`,
  REUSE annotations (MPL-2.0).
- **Verify:** read-back vs T21 shape.
- **Deps:** none
- [x] done — `lib/libplctag/CMakeLists.txt` uses upstream's own CMake and resolves the static
  target from a candidate list; `SS_LIBPLCTAG_GIT_TAG` pin; `target_link_libplctag()` +
  unconditional stub; MPL-2.0 text already ships for open62541

### T23 — BusType + labels for S7/EIP

- **Files:** `app/src/SerialStudio.h`, `app/src/API/EnumLabels.cpp`,
  `app/tests/tst_enum_labels.cpp`
- **Does:** Append `S7` and `EthernetIp` after `OpcUa` inside `BUILD_COMMERCIAL`
  (append-only — persisted enum order is wire state); slugs `s7`/`ethernetip` + labels
  in both `busTypeSlug()`/`busTypeLabel()`; extend the coverage assertions.
- **Verify:** `code-verify.py --check`; tst covers new values.
- **Deps:** none
- [x] done — `S7` (11) and `EthernetIp` (12) appended after `OpcUa`; slugs `s7`/`ethernetip`;
  labels "Siemens S7comm"/"EtherNet/IP"; tst_enum_labels gap rows for both ordinals

### T24 — S7 address parser

- **Files:** `app/src/IO/Drivers/S7Address.h`, `.cpp`, `app/tests/tst_s7_address.cpp`
- **Does:** Parse `DB<d>.DB{X|B|W|D}<off>[.bit]`, `M/I/Q` areas, types BOOL/BYTE/WORD/
  DWORD/INT/DINT/REAL/STRING → `{area, dbNumber, byteOffset, bitOffset, type, size}`;
  rejects malformed input with reasons. Qt-Core-only + ctest unit (also registers in
  `app/tests/CMakeLists.txt`).
- **Verify:** `ctest -R s7_address` (existing build) or structural.
- **Deps:** none
- [x] done — `S7Address` namespace (Qt Core only): `DB<n>.DB{X|B|W|D}<off>[.bit]` and
  `{I|E|Q|A|M}{X|B|W|D}<off>[.bit]` plus an optional `:TYPE[len]` suffix; 12 test slots incl.
  a hostile-input table and an every-prefix sweep; `ss_add_unit_test(tst_s7_address)`

### T25 — S7 driver skeleton

- **Files:** `app/src/IO/Drivers/S7.h`, `.cpp`
- **Does:** `HAL_Driver` subclass (public ctor, `configurationChanged` from ctor,
  `logDriverError` never a modal — spec 0056): endpoint properties (host, rack, slot,
  pollInterval), `QVector<S7Variable>` list (name + address string) with
  add/remove/clear invokables (Modbus register-group pattern), QSettings persistence,
  `driverProperties()`/`setDriverProperty()` both sides, `configurationOk()`,
  synchronous `open()` placeholder that validates config; `write()` returns -1;
  `isConnecting()` default. No Snap7 calls yet.
- **Verify:** `code-verify.py --check`; HAL contract read-back (all pure virtuals).
- **Deps:** T21, T24
- [x] done — folded into T26 (one class, one diff)

### T26 — S7 poll worker + wire publish

- **Files:** `app/src/IO/Drivers/S7.h`, `.cpp`
- **Does:** Driver-owned worker `QThread`: `open()` connects via Snap7 synchronously on
  the worker (blocking dial under the connect fan-out, spec-0050 sync verdict — the
  `open()` return IS the verdict, no `openFinished` latch); poll tick batch-reads the
  variable list, latches values, encodes dirty slots into an `OpcUaWire` delta frame,
  `publishReceivedData(frame, ts)` with the timestamp captured ON the worker before the
  queued hop; read failure/disconnect → queued `disconnectDevice(this)` + counter
  (never `sessionClosed`); teardown `quit()` before `wait()` (HID `cleanupDevice()`
  reference — the started/DirectConnection idiom wedges without it).
- **Verify:** `code-verify.py --check`; teardown + verdict rules read back.
- **Deps:** T25
- [x] done — `S7PollWorker` on a driver-owned QThread; `open()` runs the blocking dial via
  ONE BlockingQueuedConnection and returns its verdict (no openFinished latch); worker-captured
  stamp rides the queued `frameReady`; drop routes through queued `disconnectDevice(this)`;
  teardown quits before wait()

### T27 — `s7` template + buildProject

- **Files:** `app/src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp`,
  `app/src/IO/Drivers/S7.cpp` (+ `.h` slot decl)
- **Does:** FIRST extract the wire-latch section (WireLatchParser + opcua + sparkplug
  parsers/templates) out of `BinaryTemplates.cpp` into a new
  `NativeTemplates/WireLatchTemplates.cpp` TU (registry accessor pattern; the tu-census
  ratchet was consciously re-seeded for Phase 1 growth and must not absorb three more
  templates). Then register `s7` template on the shared latch parser; driver
  `generateProject()` from the variable list (OPC UA generator shape, T9 pattern).
- **Verify:** `code-verify.py --check`.
- **Deps:** T7, T26
- [x] done — wire-latch family extracted to `NativeTemplates/WireLatchTemplates.cpp`
  (`wireLatchNativeTemplates()`; BinaryTemplates.cpp 2781 -> 2474); parsers split into
  TypedWireParser (opcua) and NamedWireParser (sparkplug/s7/ethernetip); `s7` template +
  `S7::buildProject()`

### T28 — S7 registration: registry + factory

- **Files:** `app/src/IO/ConnectionManager/DriverUiRegistry.h`, `.cpp`,
  `app/src/IO/ConnectionManager.h`, `.cpp`
- **Does:** UI-driver slot (`kMaxUiDrivers` bump, ctor, `releaseAll()`,
  `setupExternalConnections()`, `all()`, `forBusType()` switch, typed accessor),
  `ConnectionManager::s7()` forwarder, `createDriver()` case with the license guard
  (`CommercialToken` + `SS_LICENSE_GUARD()`), inside `BUILD_COMMERCIAL`.
- **Verify:** `code-verify.py --check`; every listed switch/site covered (grep OpcUa as
  the checklist).
- **Deps:** T25
- [x] done — `kMaxUiDrivers` 11 -> 13; ctor/releaseAll/all()/forBusType()/typed accessor;
  `ConnectionManager::s7()`; licensed `createDriver()` case via the new `makePlcDriver<>` helper

### T29 — S7 QML pane + variable dialog

- **Files:** `app/qml/MainWindow/Panes/SetupPanes/Drivers/S7.qml`,
  `app/qml/MainWindow/Panes/SetupPanes/Drivers/S7VariablesDialog.qml`,
  `app/qml/MainWindow/Panes/SetupPanes/Hardware.qml`
- **Does:** Setup pane (BoundField grid; ModbusGroupsDialog pattern for the variable
  list; "Create Project" button) + `Hardware.qml` Loader entry **at the enum-order
  StackLayout position** inside `Cpp_CommercialBuild`; context property binding
  `Cpp_IO_S7`.
- **Verify:** `code-verify.py --check`; StackLayout index matches BusType ordinal.
- **Deps:** T28, T30 (context property)
- [x] done — `S7.qml` + `S7VariablesDialog.qml` (live address validation through
  `validateAddress`); Hardware.qml Loader at StackLayout index 11

### T30 — S7 module wiring + CMake

- **Files:** `app/src/Misc/ModuleManager.cpp`, `app/CMakeLists.txt`
- **Does:** `Cpp_IO_S7` context property beside `Cpp_IO_OpcUa`; new sources/headers/QML
  in the `BUILD_COMMERCIAL` blocks; `target_link_snap7(app)` call beside the OPC UA one.
- **Verify:** `code-verify.py --check`; read-back of all three CMake lists.
- **Deps:** T25, T21
- [x] done — `Cpp_IO_S7` context property (+ `WidgetExtensions::hostContextNames()`);
  sources/headers/QML in the BUILD_COMMERCIAL blocks; `target_link_snap7(app)`

### T31 — S7 API handler + CLI

- **Files:** `app/src/API/Handlers/S7Handler.h`, `.cpp`, `app/src/API/CommandHandler.cpp`
- **Does:** `io.s7.*` verbs (getStatus, setProperty via generic surface, addVariable/
  removeVariable/clearVariables, generateProject) mirroring `OpcUaHandler` registration
  shape; include + `registerCommands` call in `CommandHandler::initializeHandlers()`.
  CLI: S7 options in `Misc/CLI.{h,cpp}` follow in T37 with EIP to keep this ≤3 files.
- **Verify:** `code-verify.py --check`; handler registered.
- **Deps:** T26
- [x] done — `io.s7.{getStatus,getConfig,setProperty,addVariable,removeVariable,
  clearVariables,generateProject}` registered in `initializeHandlers()`

### T32 — EtherNet/IP driver skeleton

- **Files:** `app/src/IO/Drivers/EthernetIp.h`, `.cpp`
- **Does:** Mirror of T25 for libplctag: endpoint (host, CIP path, pollInterval), tag
  list `QVector<EipTag>` (symbolic name, type, optional element index/count),
  persistence + both property round-trip halves, `configurationOk()`. No libplctag
  calls yet.
- **Verify:** `code-verify.py --check`; HAL contract read-back.
- **Deps:** T22, T23
- [x] done — folded into T33 (one class, one diff)

### T33 — EIP poll worker + wire publish

- **Files:** `app/src/IO/Drivers/EthernetIp.h`, `.cpp`
- **Does:** Same worker shape and rules as T26 (blocking `plc_tag_create`/read with
  timeouts on the worker; sync open verdict; worker-captured timestamps; queued
  disconnect on drop; quit-before-wait teardown); per-tag read status counters.
- **Verify:** `code-verify.py --check`; rules read back.
- **Deps:** T32
- [x] done — `EipPollWorker` mirrors the S7 worker; every libplctag call goes through four
  guarded seam helpers so no function body carries a preprocessor branch; dead-tick run (3)
  declares the link lost because libplctag reconnects on its own

### T34 — `ethernetip` template + buildProject

- **Files:** `app/src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp`,
  `app/src/IO/Drivers/EthernetIp.cpp` (+ `.h` decl)
- **Does:** Register `ethernetip` template; `generateProject()` from the tag list.
- **Verify:** `code-verify.py --check`.
- **Deps:** T7, T33
- [x] done — `ethernetip` template on NamedWireParser + `EthernetIp::buildProject()`

### T35 — EIP registration: registry + factory + module wiring

- **Files:** `app/src/IO/ConnectionManager/DriverUiRegistry.{h,cpp}`,
  `app/src/IO/ConnectionManager.{h,cpp}`, `app/src/Misc/ModuleManager.cpp` +
  `app/CMakeLists.txt`
- **Does:** T28+T30 equivalents for EthernetIp (`Cpp_IO_Eip`,
  `target_link_libplctag(app)`); registration-sweep task, hence >3 files by design.
- **Verify:** `code-verify.py --check`; grep-audit vs the OpcUa sites.
- **Deps:** T32, T22
- [x] done — registry slot, `ConnectionManager::ethernetIp()`, licensed factory case,
  `Cpp_IO_Eip`, `target_link_libplctag(app)`

### T36 — EIP QML pane + tag dialog

- **Files:** `app/qml/.../Drivers/EthernetIp.qml`, `.../EipTagsDialog.qml`,
  `app/qml/MainWindow/Panes/SetupPanes/Hardware.qml`
- **Does:** Pane + tag-list dialog + Hardware.qml Loader at the correct StackLayout
  position.
- **Verify:** `code-verify.py --check`; position matches enum.
- **Deps:** T35
- [x] done — `EthernetIp.qml` + `EipTagsDialog.qml`; Hardware.qml Loader at StackLayout index 12

### T37 — EIP handler + CLI for both drivers

- **Files:** `app/src/API/Handlers/EipHandler.{h,cpp}`,
  `app/src/API/CommandHandler.cpp`, `app/src/Misc/CLI.{h,cpp}`
- **Does:** `io.eip.*` verbs; CLI bus selection + property options for BOTH s7 and
  ethernetip following the existing per-bus option groups and dispatch sites.
- **Verify:** `code-verify.py --check`.
- **Deps:** T33, T31
- [x] done — `io.eip.*` handler; CLI `--s7*` (5 options) and `--ethernetip*` (5 options)
  with dispatch in `applyBusConfiguration()`

### T38 — Icons + commands + editor surfaces for S7/EIP

- **Files:** `app/rcc/icons/devices/{16,24,32}/` + `app/rcc/rcc.qrc`,
  `app/rcc/commands/app.json` + `app/qml/Commands/AppCommandBindings.qml`,
  `app/src/DataModel/Project/ProjectEditorShared.h` +
  `.../Project/ProjectEditorForms.cpp`
- **Does:** Device icons (three sizes each), connection-category command entries
  (`"pro": true`) + bindings setting `Cpp_IO_Manager.busType`, `busTypeIcon()` cases,
  `busTypes` combobox list in enum order; run `scripts/registry-verify.py` mentally via
  read-back — icons resolve ONLY via `Misc::IconRegistry`, never hardcoded qrc paths.
- **Verify:** `python scripts/registry-verify.py`; `code-verify.py --check`.
- **Deps:** T23
- [x] done — device icons at 16/24/32 (hand-authored, distinct: registry-verify rejects
  byte-identical copies) + rcc.qrc; `driver.s7`/`driver.ethernetip` in app.json (`"pro": true`)
  + AppCommandBindings; `busTypeIcon()`; `busTypes` combobox; FlowDiagram.qml icon table
  (a site the skill checklist does not list); registry-verify CLEAN

### T39 — Test client + docs for S7/EIP

- **Files:** `tests/utils/api_client.py`, `doc/help/Drivers-S7.md` +
  `doc/help/Drivers-EtherNet-IP.md` + `doc/help/help.json`
- **Does:** `bus_map` entries; two help pages in the manual's voice (ss-docs rules:
  ground claims in code, register in help.json).
- **Verify:** `python scripts/documentation-verify.py` if present per ss-docs;
  read-back.
- **Deps:** T28, T35
- [x] done — `bus_map` s7=11 / ethernetip=12 (+ `eip` alias); `doc/help/Drivers-S7.md` and
  `Drivers-EtherNet-IP.md` registered in help.json; documentation-verify 0 findings

### T40 — Phase 3 verification

- **Files:** none
- **Does:** code-verify sweep; `qt-cpp-review`; `--singleton-census --check` (accept
  growth consciously, name it in chat); ctest targets green (existing build or
  maintainer); maintainer AC9 (Snap7 demo server / CIP endpoint) + AC10 pytest
  integration + AC13 gating + AC16 benchmark; tick AC8 slice on green.
- **Verify:** gates green or handed off.
- **Deps:** T21-T39
- [ ] done

### T65 — In-house ISO-on-TCP + S7 PDU codecs

> T65-T68 supersede T21 and the Snap7 half of T26. Maintainer-directed 2026-08-27: Snap7
> failed to compile on libc++/C++20 and carried LGPL-3.0 relink obligations, so the S7comm
> client becomes first-party like the IEC 104 stack. See the Tradeoffs row in `plan.md`.

- **Files:** `app/src/IO/Drivers/S7/IsoTsap.{h,cpp}`, `app/src/IO/Drivers/S7/S7Pdu.{h,cpp}`,
  `app/CMakeLists.txt`
- **Does:** Two Qt-Core-only, QObject-free, socket-free codec TUs on the Iec104 `Apci`/`Asdu`
  pattern. `IsoTsap`: RFC 1006 TPKT framing with incremental extraction, the ISO 8073 class-0
  CR/CC handshake carrying rack/slot, data-TPDU wrap and reassembly across the EOT flag,
  counted refusals. `S7Pdu`: header build/parse, setup-communication negotiation, read-var
  request builder with PDU-budget chunking, response walk with rollback on truncation, per-type
  value decode, error-class and return-code text.
- **Verify:** `code-verify.py --check`; golden bytes covered by T68.
- **Deps:** T24
- [x] done — `S7Comm::Transport` + `S7Comm::PduCodec`; chunker budgets both halves (12 B/item
  out, 4 B + payload + fill back) and always emits at least one item so a list makes progress

### T66 — Rewire the S7 driver onto the in-house stack

- **Files:** `app/src/IO/Drivers/S7.{h,cpp}`
- **Does:** Replace every Snap7 call. The poll worker owns a `QTcpSocket` (blocking `waitFor*`
  off the GUI thread); dial = TCP connect (5 s) → COTP CR/CC → setup communication, all inside
  the existing synchronous-open contract. Poll tick = chunked read-var, decode per declared
  type, latch, publish unchanged. Item errors count and surface per variable without dropping
  the link; a failed exchange routes through the existing queued `disconnectDevice`. Socket
  closes on the worker thread; quit-before-wait teardown unchanged.
- **Verify:** `code-verify.py --check`; read-back of the threading and stamping contract.
- **Deps:** T65
- [x] done — `m_client`/`quintptr` and every `SS_S7_ACTIVE` guard gone; new pulled counters
  `itemErrors` + `lastItemError` (index and code packed in one atomic, named on the GUI side)

### T67 — Remove the Snap7 dependency

- **Files:** `lib/snap7/` (deleted), `lib/CMakeLists.txt`, `CMakeLists.txt`,
  `app/CMakeLists.txt`, `lib/libplctag/CMakeLists.txt`, `REUSE.toml`,
  `.github/workflows/ci.yml`, `app/rcc/messages/Acknowledgements.txt`
- **Does:** Delete the module, the `target_link_snap7()` call and stub, the `SS_ENABLE_S7`
  root option and the `SS_S7_ACTIVE` plumbing; the driver is now always built under
  `BUILD_COMMERCIAL` like Iec104. Drop the macOS Snap7-slice CI step (the generic lipo step
  stays and no-ops), the Snap7 acknowledgements section and the `lib/snap7/**` REUSE
  annotation; rewrite the LGPL rationale as "no such dependency exists".
- **Verify:** `grep -ri snap7 app/ lib/ .github/ REUSE.toml` returns nothing; `reuse lint` is
  CI's. **Never run cmake.**
- **Deps:** T66
- [x] done — no LGPL dependency remains anywhere in the tree; `LICENSES/` never carried an
  LGPL text, so nothing to remove there

### T68 — S7comm codec unit tests

- **Files:** `app/tests/tst_s7comm_isotsap.cpp`, `app/tests/tst_s7comm_pdu.cpp`,
  `app/tests/CMakeLists.txt`, `doc/help/Drivers-S7.md`
- **Does:** Two suites on the `tst_iec104_*` registration pattern (Qt6::Core + SSAssert).
  Golden CR/CC/DT bytes, TPKT extraction incl. every partial prefix and six hostile shapes,
  reassembly and the assembly cap; setup negotiation and clamping, read-var golden bytes for
  DB/M/I/Q and bit access, chunk budgeting, response parsing with item errors, truncation
  rollback and odd-length fill, value decode per type incl. REAL and STRING. Help page loses
  the third-party-library claim; rack/slot and PUT/GET caveats unchanged.
- **Verify:** `code-verify.py --check`; `documentation-verify.py`; ctest is the maintainer's.
- **Deps:** T65, T66
- [x] done — 15 + 18 test functions; `documentation-verify.py` 0 findings

## Phase 4 — IEC 60870-5-104 client (B4)

> In-house stack maintainer-confirmed 2026-08-27 (no lib60870 dependency).

### T41 — APCI layer

- **Files:** `app/src/IO/Drivers/Iec104/Apci.h`, `.cpp`
- **Does:** Qt-Core-only APCI codec + connection state: I/S/U frame encode/decode,
  send/recv sequence numbers, k=12/w=8 windows, t1/t2/t3 handling surfaced as "due"
  queries driven by an external clock (testable without timers), STARTDT/STOPDT/TESTFR
  act/con. Malformed input → error + counter, never UB.
- **Verify:** `code-verify.py --check`.
- **Deps:** none
- [x] done — FetchContent, static (MPL-2.0 file-level copyleft permits it)

### T42 — ASDU codec

- **Files:** `app/src/IO/Drivers/Iec104/Asdu.h`, `.cpp`
- **Does:** Decode M_SP_NA_1, M_DP_NA_1, M_ME_NA_1, M_ME_NB_1, M_ME_NC_1, M_IT_NA_1 and
  their CP56Time2a twins (30, 31, 34, 35, 36, 37) → `{ioa, value, quality bits,
  optional timestamp}`; encode C_IC_NA_1 (station interrogation, QOI 20); quality
  descriptors (IV/NT/BL/SB/OV) preserved per point (R29); unknown type IDs skipped +
  counted.
- **Verify:** `code-verify.py --check`; type table read back against R28.
- **Deps:** none
- [x] done — S7=11, EthernetIp=12, append-only

### T43 — IEC 104 codec unit tests

- **Files:** `app/tests/tst_iec104_apci.cpp`, `app/tests/tst_iec104_asdu.cpp`,
  `app/tests/CMakeLists.txt`
- **Does:** Canned exchanges: STARTDT handshake, sequence windows, TESTFR keepalive,
  t1/t2/t3 due behavior, S-frame acking; each ASDU type golden-decoded with and without
  timestamps, quality bit propagation, truncated/hostile buffers.
- **Verify:** `ctest -R iec104` (existing build) or structural + maintainer.
- **Deps:** T41, T42
- [x] done — parser + ctest unit

### T44 — BusType + labels for Iec104

- **Files:** `app/src/SerialStudio.h`, `app/src/API/EnumLabels.cpp`,
  `app/tests/tst_enum_labels.cpp`
- **Does:** Append `Iec104` after `EthernetIp`; slug `iec104` + label; coverage test.
- **Verify:** `code-verify.py --check`.
- **Deps:** T23
- [x] done — poll-based HAL_Driver, read-only

### T45 — IEC 104 driver

- **Files:** `app/src/IO/Drivers/Iec104.h`, `.cpp`
- **Does:** `HAL_Driver` over QTcpSocket with the spec-0050 blocking-dial pattern
  (throwaway probe socket, then ONE real connect; never abort-and-redial a
  run-loop-registered socket); properties host/port/commonAddress/pollless; on
  connect: STARTDT act, general interrogation; dynamic IOA→slot latch table; spontaneous
  + interrogated points update slots; per-tick `OpcUaWire` delta publish with
  CP56Time2a mapped through an OPC UA-style connect-time offset (clamped monotonic);
  TESTFR per T41 due queries on a driver QTimer; drop → queued `disconnectDevice(this)`
  (never `sessionClosed`); pulled counters (quality-bad points, skipped ASDUs, TESTFR
  misses).
- **Verify:** `code-verify.py --check`; dial + verdict rules read back.
- **Deps:** T41, T42, T44
- [x] done — worker thread, sync-open verdict, quit-before-wait

### T46 — `iec104` template + buildProject

- **Files:** `app/src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp`,
  `app/src/IO/Drivers/Iec104.cpp` (+ `.h` decl)
- **Does:** Register `iec104` template; `generateProject()` from the interrogated point
  table (groups by ASDU type class, LED datasets for single/double points).
- **Verify:** `code-verify.py --check`.
- **Deps:** T7, T45
- [x] done — template family split by schema kind into WireLatchTemplates.cpp (-307 lines)

### T47 — IEC 104 registration sweep

- **Files:** `DriverUiRegistry.{h,cpp}`, `ConnectionManager.{h,cpp}`,
  `ModuleManager.cpp`, `app/CMakeLists.txt`
- **Does:** Registry slot + `iec104()` forwarder + licensed `createDriver()` case +
  `Cpp_IO_Iec104` + CMake sources (registration-sweep task).
- **Verify:** `code-verify.py --check`; grep-audit vs OpcUa sites.
- **Deps:** T45
- [x] done — registry + licensed factory

### T48 — IEC 104 QML + handler + CLI

- **Files:** `app/qml/.../Drivers/Iec104.qml` + `Hardware.qml`,
  `app/src/API/Handlers/Iec104Handler.{h,cpp}` + `CommandHandler.cpp`,
  `app/src/Misc/CLI.{h,cpp}`
- **Does:** Pane (StackLayout position = enum ordinal), `io.iec104.*` verbs, CLI
  options (sweep task).
- **Verify:** `code-verify.py --check`; position check.
- **Deps:** T47
- [x] done — pane + variables dialog

### T49 — IEC 104 icons/commands/tests/docs

- **Files:** `app/rcc/icons/devices/*` + `rcc.qrc` + `app/rcc/commands/app.json` +
  `AppCommandBindings.qml`, `ProjectEditorShared.h` + `ProjectEditorForms.cpp`,
  `tests/utils/api_client.py`, `doc/help/Drivers-IEC-104.md` + `help.json`
- **Does:** T38+T39 equivalents for Iec104 (sweep task).
- **Verify:** `registry-verify.py`; `code-verify.py --check`.
- **Deps:** T44, T47
- [x] done — module wiring + CMake

### T50 — Phase 4 verification

- **Files:** none
- **Does:** code-verify sweep; `qt-cpp-review`; ctest green; maintainer AC9 vs an IEC
  104 test server, AC10/AC13/AC16; tick AC8 remainder on green.
- **Verify:** gates green or handed off.
- **Deps:** T41-T49
- [ ] done

## Phase 5 — InfluxDB sink (S1)

### T51 — Line-protocol formatter

- **Files:** `app/src/InfluxDB/LineProtocol.h`, `app/tests/tst_influx_lineprotocol.cpp`
  + `app/tests/CMakeLists.txt`
- **Does:** Header-only formatter: measurement/tag/field escaping per the v2 spec, ns
  timestamps, batch buffer with size boundary; ctest unit incl. escaping edge cases
  (spaces, commas, quotes, NaN/Inf skipped + counted).
- **Verify:** `ctest -R influx_lineprotocol` (existing build) or structural.
- **Deps:** none
- [x] done — header-only formatter + unit suite (escaping, int/float/string, NaN/Inf skip)

### T52 — Project-file sink config

- **Files:** `app/src/DataModel/Frame.h`, `app/src/DataModel/ProjectModel.*` (the
  serialize/read TU + accessor site beside `mqttPublisher()`)
- **Does:** `Keys::InfluxSink` + opaque `QJsonObject` accessor/setter on ProjectModel
  (url, org, bucket, enabled, measurement mapping; NO token field). Mutating slot opens
  a `ProjectUndoScope` and calls `setModified(true)` (two-phase memento — omit either
  and nothing is recorded); absent key ⇒ disabled (R37, old files unchanged).
- **Verify:** `code-verify.py --check` (undo-scope lint); `keys-hardcoded-literal`
  clean.
- **Deps:** none
- [x] done — Keys::InfluxSink, ProjectUndoScope + setModified pair, absent key = disabled

### T53 — InfluxDB sink consumer

- **Files:** `app/src/InfluxDB/Export.h`, `.cpp`
- **Does:** `FrameConsumer<DataBlockPtr>` singleton (Sessions::Export shape, config
  `{8192, 1024, 1000}`): worker creates its `QNetworkAccessManager` ON the worker
  thread in bootstrap (MQTT::PublisherWorker precedent); `processItems` renders blocks
  via T51 with BLOCK timestamps (source owns time — never `now()`); one POST per batch
  to `/api/v2/write`, token header from `MQTT::CredentialVault` scope `"influxdb"`;
  bounded in-flight (1 request; overflow drops batch + counts); sink-local pulled
  counters (pointsWritten, pointsDropped, httpErrors, lastError); `ingestBlock` guard
  `exportEnabled() && !isAnyPlayerOpen()`; Pro gating: `BUILD_COMMERCIAL`, runtime
  token check in the setter, `LemonSqueezy::activatedChanged` force-disable (late
  activation R36).
- **Verify:** `code-verify.py --check`; gating + timestamp rules read back.
- **Deps:** T51, T52
- [x] done — QNAM created ON the worker thread; one request in flight; sink-local
  counters; three gating layers incl. activatedChanged force-disable

### T54 — FrameBuilder fan-out wiring

- **Files:** `app/src/DataModel/FrameBuilder.cpp`,
  `app/src/API/Handlers/DashboardHandler.cpp`
- **Does:** **Hotpath file — read FrameBuilder.cpp IN FULL and invoke `ss-hotpath`
  before editing.** The three touch-points as ONE diff: `enabledChanged →
  refreshAnyAsyncSink()` connect (~839), OR-term in `refreshAnyAsyncSink()` (~992),
  `ingestBlock(detached)` in `publishBlock` inside `BUILD_COMMERCIAL` (~3315). Missing
  either flag half = valid-looking empty output (cached-flag silent breakage); no other
  FrameBuilder edits. Update the `dashboardTick` sink-list doc string (~328).
- **Verify:** `code-verify.py --check`; diff shows exactly four hunks; maintainer
  `--benchmark-hotpath` unchanged with sink off (AC16).
- **Deps:** T53
- [x] done — exactly 8 insertions / 1 deletion; cached-flag pair landed together;
  DashboardHandler sink list updated

### T55 — Module wiring + CMake

- **Files:** `app/src/Misc/ModuleManager.cpp`, `app/CMakeLists.txt`
- **Does:** `setupExternalConnections` wiring + `Cpp_InfluxDB_Export` context property
  (Sessions pattern); sources in the `BUILD_COMMERCIAL` block. No composition-root
  order change (self-owned singleton — ctor-edge untouched).
- **Verify:** `code-verify.py --check`.
- **Deps:** T53
- [x] done — setupCommercialModuleConnections() extracted; stopWorker() added to
  stopFrameConsumerWorkers() (worker outlived qApp without it)

### T56 — Influx API verbs

- **Files:** `app/src/API/Handlers/InfluxHandler.{h,cpp}`,
  `app/src/API/CommandHandler.cpp`
- **Does:** `influx.setConfig` / `influx.getStatus` / `influx.setEnabled` (token write
  routes to the vault, never echoed back); registered in `initializeHandlers()`.
- **Verify:** `code-verify.py --check`.
- **Deps:** T53
- [x] done — influx.setConfig/getStatus/setEnabled; token never echoed (hasToken only)

### T57 — Sink UI

- **Files:** `app/qml/MainWindow/Panes/Setup.qml`,
  `app/qml/ProjectEditor/Views/InfluxSinkView.qml` (+ its ProjectEditor model hook)
- **Does:** Enable switch beside CSV/Historian/MDF4 (gated `dataExportAllowed &&
  Cpp_CommercialBuild`); config view on the MqttPublisherView pattern (unfocused-sync
  BoundField idiom; token field is write-only Password style).
- **Verify:** `code-verify.py --check`.
- **Deps:** T52, T55
- [x] done — Setup.qml switch + InfluxSinkView.qml; registry-verify caught a missing
  hostContextNames entry

### T58 — Phase 5 verification

- **Files:** none
- **Does:** code-verify sweep; `qt-cpp-review`; ctest green; maintainer AC12
  (unreachable endpoint + live InfluxDB), AC14 project round-trip pytest, AC16 full
  benchmark; tick AC11/AC12/AC14 on green.
- **Verify:** gates green or handed off.
- **Deps:** T51-T57
- [ ] done

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` is met and checked off there (AC3, AC5,
  AC6, AC9, AC12, AC13, AC15, AC16 are maintainer/CI runs — listed per phase task).
- [ ] `python scripts/code-verify.py --check` clean on all changed files (no new errors).
- [ ] `qt-cpp-review` run on each phase's C++ diff; findings addressed or noted.
- [ ] `--benchmark-hotpath` not regressed (maintainer/CI, after Phases 1, 2, 5 minimum).
- [ ] `pytest` targets identified for the maintainer (AC2, AC10, AC14 + R14/R15-style
  runtime decode addition).
- [ ] `python scripts/sanitize-commit.py` run before every commit; census gates
  re-seeded consciously and named in chat where they grow.
- [ ] Diff is *what was asked, and only that* — no scope creep, no foreign files.
- [ ] `spec.md` status set to `done` when all five phases land.
