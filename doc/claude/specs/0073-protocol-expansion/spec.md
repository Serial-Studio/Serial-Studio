---
spec: 0073-protocol-expansion
title: Industrial Protocol Expansion (Sparkplug B, J1939/ISO-TP, DBC extended mux, S7comm, EtherNet/IP, IEC 104, InfluxDB sink)
status: in-progress  # draft -> approved -> in-progress -> done | shelved
created: 2026-08-27
author: Alex Spataru
---

# Spec 0073 — Industrial Protocol Expansion

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

This spec is an umbrella covering seven roadmap items in two tiers plus one export lane.
Tier A items extend drivers that already exist; Tier B items are new drivers. XCP/CCP is
explicitly excluded (its own future spec). Delivery is phased (see Constraints); each
phase is independently shippable and independently verifiable.

## Problem / Motivation

Serial Studio speaks the *modern* half of industrial telemetry — OPC UA, plain MQTT,
Modbus, raw CAN — but not the payload conventions and legacy controllers that the
industrial installed base actually runs. Each gap below is a deal blocker today, not a
hypothetical:

- **Sparkplug B.** Industrial MQTT deployments (Ignition, HiveMQ, AWS IoT, Cirrus Link
  ecosystems) treat Sparkplug B as table stakes. Serial Studio's MQTT driver delivers only
  raw payloads, so a Sparkplug topic namespace arrives as undecodable protobuf blobs. The
  building blocks (an MQTT subscriber and a protobuf importer) both already ship; what is
  missing is the convention that staples them together.
- **J1939 transport protocol.** The DBC importer already decodes 29-bit extended IDs, but
  nothing reassembles multi-packet PGNs (J1939 TP, BAM and RTS/CTS). Any PGN longer than
  8 bytes arrives as unrelated raw frames and its signals *silently never appear* on the
  dashboard — no error, no warning. Silent data loss is the worst failure mode this
  product can have. Heavy equipment, agriculture, marine, and trucking fleets all live
  above 8 bytes.
- **DBC extended multiplexing.** The importer explicitly skips extended-multiplex signals
  and tells the user so in a warning string ("only simple multiplexing is supported").
  A known, user-visible hole; closing it is cheap credibility with CAN users.
- **Legacy PLCs (S7, EtherNet/IP).** OPC UA only reaches modern CPUs. Fielded S7-300/400
  and most S7-1200 units, plus every Rockwell CompactLogix/ControlLogix, cannot feed
  Serial Studio without a KEPServerEX-class gateway (~$1k+/seat). "Your PLC does not
  speak OPC UA and you should not have to buy a gateway to use us" is one sales story
  shipped as a pair; Modbus already proved these buyers convert.
- **IEC 60870-5-104.** Utilities and substations. Client-side 104 is roughly Modbus-TCP
  effort and opens a market that pays well.
- **Outbound historian sink.** Historian records to local SQLite only. Enterprise buyers
  want data *leaving* the box into their existing time-series stack. This is an export
  lane, not a driver, and likely closes deals faster than any single new protocol.

## Goals

- A user on a Sparkplug B deployment points Serial Studio at their broker, enables one
  option, and sees named metrics on the dashboard — no manual protobuf schema work.
- A user replaying or streaming J1939 traffic sees signals from >8-byte PGNs decode
  exactly as 8-byte ones do today; multi-frame diagnostic (ISO-TP) payloads are
  reassembled by the same machinery.
- A DBC file using extended multiplexing imports without the "only simple multiplexing"
  warning and its signals decode correctly.
- A user with an S7 or Rockwell PLC connects directly (IP + addressing info), picks
  variables to poll, and gets datasets — no OPC UA gateway in between.
- A user with an IEC 104 outstation connects as a 104 client and receives both
  interrogated and spontaneous points as datasets.
- A user can configure Serial Studio to continuously push acquired datasets into an
  InfluxDB instance while the dashboard runs unaffected.
- Every item above is Pro-gated except DBC extended multiplexing, which ships as part of
  the existing DBC import behavior.

## Non-Goals

- **XCP / CCP** — excluded from this spec entirely; future spec.
- **DNP3** — deferred to its own spec; this spec covers IEC 60870-5-104 client only.
- **Sparkplug B host-application role** (STATE topic, primary-host arbitration of edge-node
  online status) — Serial Studio publishes as an Edge Node only, never as a primary host.
- **A full UDS diagnostic client** (session control, security access, service scheduling)
  — this spec delivers ISO-TP segmentation/reassembly so multi-frame messages decode;
  interpreting UDS services beyond that is out of scope.
- **Outbound sinks other than InfluxDB** (TimescaleDB, Kafka, gRPC push) — the sink lane
  should not preclude them, but only InfluxDB ships under this spec.
- **IEC 104 server/outstation mode** and **IEC 61850** — client only, 104 only.
- **PLC writes.** S7, EtherNet/IP, and IEC 104 support is read/subscribe telemetry only;
  no command/write path to the controller in this spec.
- **New visualization widgets.** All features feed the existing dataset/widget model.

## Requirements

### A1 — Sparkplug B (option on the existing MQTT driver)

1. **R1** — The MQTT connection setup offers a Sparkplug B mode (off by default) with a
   configurable group/namespace filter. With it off, MQTT behavior is byte-for-byte
   today's raw-payload behavior.
2. **R2** — With Sparkplug B enabled, metrics carried in NBIRTH/DBIRTH and NDATA/DDATA
   messages appear as named datasets, using the metric names and alias tables declared in
   the birth certificates.
3. **R3** — Data messages that arrive before their birth certificate (unresolvable
   aliases) are never shown as garbage: they are buffered until the birth arrives or
   dropped, and the condition is visible to the user (a status/diagnostic indication, not
   silence).
4. **R4** — Sequence-number gaps and alias misses trigger a rebirth request (NCMD) to the
   affected edge node, and the desync condition is user-visible.
5. **R5** — Node/device death (NDEATH/DDEATH, including last-will delivery) marks the
   affected metrics as stale/offline on the dashboard rather than freezing silently at
   the last value.
6. **R6** — Sparkplug decoding handles at minimum the scalar metric datatypes (integers,
   floats, boolean, string) from the Sparkplug B v1.0 payload definition; unsupported
   datatypes are skipped with a visible diagnostic count, never a crash or garbage value.

### A1b — Sparkplug B outbound publishing (amendment, 2026-08-27)

> Added after Phase 1 landed, at the maintainer's request; was previously a Non-Goal.
> Builds on the A1 codec and rides the existing `MQTT::Publisher` sink.

39. **R39** — The user can enable a Sparkplug B Edge Node mode on the MQTT publisher,
    configuring group id, edge node id and an optional device id; disabled by default,
    leaving today's raw/JSON publishing byte-for-byte unchanged.
40. **R40** — On connect the publisher emits an NBIRTH (and a DBIRTH when a device id is
    set) declaring every published dataset as a metric with a stable alias, then emits
    NDATA/DDATA carrying only changed metrics, addressed by alias.
41. **R41** — The `bdSeq` and `seq` counters follow the Sparkplug B specification: `seq`
    increments modulo 256 on every message including births, and `bdSeq` increments once
    per connection and is carried in both the NBIRTH and the registered will.
42. **R42** — An NDEATH will is registered with the broker at connect time, so an
    ungraceful disconnect marks the node offline without Serial Studio publishing
    anything.
43. **R43** — An inbound NCMD carrying `Node Control/Rebirth` re-publishes the birth
    certificate; other commands are ignored and counted.
44. **R44** — Publishing metric datatypes covers the same scalar set the decoder accepts,
    with dataset values mapped by their declared type; a value that cannot be represented
    is skipped and counted, never published as a wrong-typed metric.

### A2 — J1939 transport protocol + ISO-TP (on the existing CAN driver)

7. **R7** — J1939 TP.CM/TP.DT broadcast (BAM) sessions are reassembled: a >8-byte PGN
   defined in the loaded DBC decodes into its signals exactly as a single-frame PGN does.
8. **R8** — J1939 TP peer-to-peer (RTS/CTS) sessions addressed to monitored traffic are
   reassembled on a listen-only basis (Serial Studio observes both directions; it does
   not participate in the handshake).
9. **R9** — Incomplete or aborted transport sessions (missing DT packet, TP.CM abort,
   timeout) are discarded and counted in a user-visible diagnostic; they never produce a
   partially-decoded frame.
10. **R10** — ISO-TP (ISO 15765-2) segmented messages (first/consecutive/flow-control
    frames) are reassembled by the same machinery on a listen-only basis, so multi-frame
    payloads on diagnostic IDs become complete frames available for decoding.
11. **R11** — Reassembly is bounded: concurrent session count and per-session size have
    fixed caps, and cap overruns drop whole sessions with a diagnostic count — memory use
    cannot grow with malformed traffic.
12. **R12** — Single-frame (≤8 byte) CAN decode behavior and throughput are unchanged
    when no transport-protocol traffic is present.

### A3 — DBC extended multiplexing

13. **R13** — DBC files using extended multiplexing (`SG_MUL_VAL_`) import without the
    "only simple multiplexing is supported" warning, and the previously-skipped signals
    are created as datasets.
14. **R14** — Extended-multiplex signals decode correctly: a signal appears only when its
    multiplexor selector ranges match, including nested/chained multiplexor definitions.
15. **R15** — DBC files with only simple multiplexing (or none) import and decode exactly
    as before.

### B1 — Siemens S7comm driver (new driver)

16. **R16** — A new S7 connection type lets the user specify the CPU endpoint (IP, rack,
    slot — or the equivalent addressing for S7-1200/1500) and connect to S7-300, S7-400,
    and S7-1200 class CPUs.
17. **R17** — The user defines a list of variables to acquire (data block number, offset,
    type — the conventional S7 absolute-address form) and a polling interval; each
    variable becomes a dataset updated at that interval.
18. **R18** — Supported variable types cover at minimum: BOOL, BYTE/WORD/DWORD, INT/DINT,
    REAL, and STRING reads.
19. **R19** — Connection loss is detected and reported through the same reconnect UX as
    existing network drivers; a failed connect attempt reports a reason.
20. **R20** — The driver appears everywhere existing drivers do: connection setup UI,
    CLI, API server, and project files (a saved project reopens with its S7 config).

### B2 — EtherNet/IP driver (new driver)

21. **R21** — A new EtherNet/IP connection type lets the user specify the controller
    endpoint (IP, and where needed the CIP routing path) for CompactLogix/ControlLogix
    class controllers.
22. **R22** — The user defines a list of controller tags *by symbolic name* plus a
    polling interval; each tag becomes a dataset updated at that interval.
23. **R23** — Supported tag types cover at minimum: BOOL, SINT/INT/DINT, REAL, and
    STRING; array elements addressable by index.
24. **R24** — Connection loss/retry and failure reporting behave as R19.
25. **R25** — Registration surface as R20 (setup UI, CLI, API server, project files).

### B4 — IEC 60870-5-104 client driver (new driver)

26. **R26** — A new IEC 104 connection type lets the user connect as a client to an
    outstation (IP/port, common address of ASDU) and start data transfer.
27. **R27** — On connect, the driver performs a general interrogation; interrogation
    results and subsequent spontaneous transmissions both update datasets keyed by
    information object address.
28. **R28** — Supported ASDU types cover at minimum the common monitor-direction types:
    single-point, double-point, measured value normalized/scaled/short-float, and
    integrated totals — each with and without timestamps (CP56Time2a).
29. **R29** — Quality descriptors (invalid/not-topical/blocked/substituted) are surfaced
    per point, not silently discarded; link supervision (TESTFR, t1/t2/t3 behavior)
    keeps the connection alive per the standard's client obligations.
30. **R30** — Registration surface as R20 (setup UI, CLI, API server, project files).

### S1 — InfluxDB outbound sink

31. **R31** — The user can enable an InfluxDB sink (endpoint URL, org/bucket/token for
    InfluxDB 2.x line-protocol API) that continuously writes acquired dataset values
    while acquisition runs.
32. **R32** — Points are written with the acquisition timestamp (source time, not
    write-out time), a measurement/tag mapping derived from group/dataset identity, and
    batched writes — one HTTP request per batch, not per sample.
33. **R33** — Sink backpressure or outage (unreachable endpoint, HTTP errors, slow
    server) never stalls or slows acquisition or the dashboard: the sink drops data
    beyond a bounded buffer and reports dropped-point and error counts to the user.
34. **R34** — Sink configuration round-trips through the project file, and the token is
    not stored in plain text in the project file.
35. **R35** — The sink can be toggled while a session runs, taking effect without
    restarting acquisition.

### Cross-cutting

36. **R36** — Pro gating: A1, A2, B1, B2, B4, and S1 are Pro features (A2 rides the
    already-Pro CAN driver). A3 ships ungated as part of standard DBC import behavior.
    Features gated at load time also respond correctly to late/asynchronous license
    activation — activating Pro after startup enables them without a restart leaving
    stale fallback state.
37. **R37** — Every new option and driver round-trips through project save/load, and
    projects saved by older versions load unchanged (no format break).
38. **R38** — No third-party dependency may be added whose license is incompatible with
    GPL-3.0-or-later + commercial dual licensing, and every added file/dependency keeps
    the repository REUSE-compliant.

## Acceptance Criteria

- [ ] **AC1** (R1-R6) — C++ unit tier: canned Sparkplug B payload sequences (NBIRTH →
  NDATA, DDATA-before-DBIRTH, sequence gap, NDEATH, unsupported datatype) decode to the
  expected metric sets, buffering/drop behavior, rebirth trigger, and stale marking.
- [ ] **AC2** (R1) — Regression: with Sparkplug mode off, existing MQTT integration tests
  pass unchanged.
- [ ] **AC3** (R1-R5, in-app) — Maintainer observation against a live broker (e.g.
  Mosquitto + a Sparkplug simulator): metrics appear named on the dashboard, killing the
  simulator marks them stale, restarting it rebirths them.
- [ ] **AC4** (R7-R12) — C++ unit tier: canned CAN frame sequences for BAM, RTS/CTS,
  ISO-TP single/multi-frame, interleaved concurrent sessions, aborts, timeouts, and
  cap-overrun cases produce exactly the expected reassembled frames and diagnostic
  counts.
- [ ] **AC5** (R7, in-app) — Maintainer observation: a recorded J1939 log with >8-byte
  PGNs replayed against a DBC shows the previously-missing signals on the dashboard.
- [ ] **AC6** (R12) — `--benchmark-hotpath` gates pass unchanged; a CAN-heavy benchmark
  run with TP code compiled in but idle shows no measurable regression.
- [ ] **AC7** (R13-R15) — C++ unit tier: DBC fixtures with extended multiplexing
  (including nested selectors and range lists) import with zero skipped signals and
  decode the documented expected values; simple-mux fixtures decode unchanged.
- [ ] **AC8** (R16-R30) — Per new driver, C++ unit tier for the protocol state machine
  against canned exchanges (connect handshake, poll/response, interrogation, malformed
  responses, disconnect detection) without a live device.
- [ ] **AC9** (R16-R30, in-app) — Maintainer observation per driver against a software
  endpoint (Snap7 demo server / a CIP simulator or real PLC / an IEC 104 test server):
  connect, datasets update, unplug detection, reconnect.
- [ ] **AC10** (R20, R25, R30) — pytest integration tier: each new driver is listed,
  configurable, and connect/disconnect-able through the API server; a project file
  containing each new driver config round-trips through save/load.
- [ ] **AC11** (R31-R35) — C++ unit tier: line-protocol formatting (escaping, timestamp
  precision, batching boundaries) and the bounded-buffer drop/count behavior under a
  stalled writer.
- [ ] **AC12** (R33, in-app) — Maintainer observation: with acquisition running against
  an unreachable InfluxDB endpoint, dashboard frame rate is unaffected and the UI shows
  accumulating error/drop counts; pointing it at a live InfluxDB shows points arriving
  with source timestamps.
- [ ] **AC13** (R36) — In-app: with an unactivated build, each Pro item is gated exactly
  like existing Pro features; activating while running enables them without restart.
- [ ] **AC14** (R37) — pytest: project files from the current release load with no
  behavior change; new-feature configs survive save → load → save byte-stable.
- [ ] **AC15** (R38) — `reuse lint` passes; license inventory updated for any new
  third-party code.
- [ ] **AC16** (global) — Full `--benchmark-hotpath` tier table passes at its current
  floors on the PGO binary with all features compiled in and idle.

## Constraints & Invariants

- **Tier A adds zero new connection surface.** A1-A3 must not add a new bus/connection
  type, setup pane, or API/CLI surface — they are options on the MQTT and CAN drivers
  and the DBC importer. Tier B items each pay the full new-driver registration sweep.
- **The 256 kHz hotpath gate is untouchable.** No per-frame allocation, locking, or
  signaling may be added to the acquisition path by any feature here; idle features must
  cost nothing measurable.
- **Silent data loss is forbidden in both directions.** Every drop this spec introduces
  (pre-birth Sparkplug data, aborted TP sessions, sink overflow) must be counted and
  user-visible; conversely fixing the J1939 silent loss must not introduce partially
  decoded frames.
- **Reassembly and buffering are bounded** (fixed session caps, fixed buffer sizes) —
  malformed or hostile traffic must not grow memory. All three new drivers parse
  network-facing binary input and must treat it as untrusted.
- **Sink writes never touch the acquisition thread's timing**; source timestamps are
  authoritative (stamp at the driver boundary, never re-stamp at write-out).
- **Pro gating uses the existing activation system** and must handle late activation
  (no baked-at-load fallback state that survives activation).
- **Project-file compatibility is one-way safe:** old files load in the new version;
  files not using new features remain loadable by the current release where the existing
  format guarantees that today.
- **Dependency licenses:** LGPL/MPL/BSD-class or GPL-compatible only (Snap7-class LGPL
  and libplctag-class MPL-2.0 are acceptable precedents); REUSE compliance maintained.
- **Delivery is phased in roadmap order** — (1) Sparkplug B, (2) J1939 TP + extended
  mux, (3) S7comm + EtherNet/IP as a pair, (4) IEC 104, (5) InfluxDB sink may ship any
  time after (1) — and each phase must leave the tree shippable; no phase may depend on
  a later one.
- **Implementation will be executed by a smaller model in small increments:** the plan
  and task breakdown must decompose into independently verifiable tasks of narrow file
  scope, each with its own check, so code quality survives the handoff.

## Open Questions

- Sparkplug B: is a per-connection *primary host application* (STATE subscription)
  configuration needed for the deployments we target, or is plain group filtering
  enough for v1?
- J1939 RTS/CTS listen-only reassembly requires seeing both handshake directions; on
  interfaces where only one direction is visible, is best-effort reassembly (accept DT
  stream after RTS without CTS confirmation) acceptable? (Recommendation: yes,
  listen-only tools conventionally do this.)
- S7 optimized-block access on S7-1200/1500 (absolute addressing disabled by default on
  new projects): document as a limitation for v1, or in scope? (Recommendation:
  document as limitation; classic addressing covers the gateway-tax market this
  targets.)
- InfluxDB 1.x compatibility (username/password, database/retention-policy addressing):
  in scope or 2.x-only? (Recommendation: 2.x-only for v1.)
- Token storage for the sink (R34): reuse the existing MQTT credential-vault approach,
  or system keychain? (Plan-phase decision, flagged here because it affects R34's
  wording.)
