---
spec: 0066-opcua-driver
title: OPC UA Client Driver
status: done          # closed 2026-08-25
created: 2026-08-22
author: Alex Spataru
---

# Spec 0066 — OPC UA Client Driver

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

Serial Studio reaches the factory floor through Modbus and CAN only. Every modern PLC and
SCADA platform exposes its tags through OPC UA (IEC 62541): Rockwell through FactoryTalk
Linx Gateway, Kepware/KEPServerEX, Inductive Automation Ignition, Siemens S7-1200/1500 and
WinCC, Beckhoff TwinCAT, Codesys, B&R, Wago, Schneider. A user who wants to plot a Logix tag
today must buy and configure a gateway that re-publishes it over Modbus TCP or MQTT, losing
tag names, types, and timestamps on the way and paying for a license whose only job is to
feed Serial Studio. Requests for "FactoryTalk support" are really requests for this bridge.

OPC UA is the one client that covers all of those vendors at once. The value of the driver
is that a Pro user points Serial Studio at an endpoint, browses the server's address space,
ticks the tags they care about, and gets a working dashboard project with correctly typed,
correctly named datasets, without writing a parser or knowing how tags are encoded.

## Goals

- A Pro user connects Serial Studio to any OPC UA server endpoint that offers security
  policy None, authenticating anonymously or with username/password.
- The user browses the server's address space from inside Serial Studio, selects tags, and
  the app generates a complete project: one group per chosen folder, one dataset per tag,
  named after the tag's display name, typed after its value type, plus a parser.
- Selected tags update on the dashboard at the server's publishing rate through a
  subscription, or through timed reads when the server refuses subscriptions, with no user
  intervention to pick between the two.
- Tag values keep their native type end to end: booleans drive LEDs, integers and floats
  plot with their real precision, strings reach string-capable widgets and the console.
- Each tag value carries the server's source timestamp so recordings and CSV exports stay
  faithful to the PLC clock, not the poll clock.
- A Python OPC UA server simulator ships as an example, doubles as the integration-test
  fixture, and runs on every CI leg that runs the Pro integration tier.
- The feature is documented in the in-app help manual alongside the other drivers.

## Non-Goals

- OPC UA **server** mode: Serial Studio never exposes its own datasets as OPC UA nodes.
- OPC UA **PubSub** (UDP/MQTT transport profiles).
- **Historical access** (HA), **methods**, **alarms & conditions**, **events**, and
  **server redundancy / failover**.
- **Writing** tag values from the dashboard (output widgets); a later spec may add it.
- Any secured channel (Basic256Sha256 or other encrypting policies), X.509 client
  authentication, and server-certificate trust management. The Qt OPC UA backend Serial
  Studio ships with is built without encryption support (`open62541_security: no`), so a
  secure channel needs a toolchain change first; it is a follow-up spec, not this one.
- Non-binary transports (HTTPS, WebSocket).
- Complex / structured (extension-object) tag values; arrays beyond one dimension.
- A FactoryTalk-, Kepware-, or Ignition-specific driver. The driver is "OPC UA"; vendor
  names appear only in documentation as compatibility statements.
- Any change to the acquisition pipeline, frame reader, or frame builder.

## Requirements

1. **R1** — A new Pro data source named "OPC UA" appears in the connection setup next to
   Modbus, and is absent from GPL builds.
2. **R2** — The user can enter an endpoint URL (`opc.tcp://host:port/path`) and the app
   lists the server's advertised endpoints with their security policy and mode, letting the
   user pick one; endpoints whose policy is not None are shown greyed out with the reason
   ("secure channel not supported in this version").
3. **R3** — The user can authenticate anonymously or with a username and password; the
   password is stored the same way other driver secrets are stored today (encrypted vault,
   never in the project file).
4. **R4** — When username/password is selected, the connection pane shows a persistent
   warning that credentials travel unencrypted over a None-policy channel, and the
   connection log records the same warning once per connect.
5. **R5** — Connecting reports success or a human-readable failure reason (unreachable,
   bad credentials, certificate rejected, unsupported policy) exactly once per attempt, and
   the connect control never stays stuck in a connecting state.
6. **R6** — A tag browser dialog walks the server's address space from the Objects folder,
   shows each node's display name, node id, value type, and access level, and lets the user
   tick variable nodes (and whole folders, selecting all readable variables beneath them).
7. **R7** — Generating a project from the selection creates one group per selected folder
   (or one group for loose selections) and one dataset per tag: display name as the dataset
   title, engineering-unit text as the unit when the server exposes it, plot enabled for
   numeric types, LED for booleans, string tags routed to string-capable widgets.
8. **R8** — Supported value types are Boolean, SByte/Byte, Int16/UInt16, Int32/UInt32,
   Int64/UInt64, Float, Double, String, and one-dimensional arrays of those (expanded to one
   dataset per element). Unsupported types are listed in the browser as not selectable.
9. **R9** — After connecting, every selected tag is delivered through a subscription at a
   user-configurable publishing interval (default 100 ms, minimum the server's revised
   minimum). If the server rejects subscriptions or monitored items, the driver switches to
   timed reads at the same interval and tells the user it did so in the connection status.
10. **R10** — Every delivered value carries the server's source timestamp; when the server
    omits it, the receipt time is used and marked as such in diagnostics.
11. **R11** — A tag whose read returns a bad status code keeps its last good value on the
    dashboard and the bad status is visible in connection diagnostics; it does not tear the
    frame or the connection down.
12. **R12** — Losing the connection (socket drop, server restart) is reported through the
    normal disconnect path, and reconnecting restores the same subscription without
    regenerating the project.
13. **R13** — The project file records the endpoint, security selection, authentication
    mode (never the password in plain text), publishing interval, and the selected tag list,
    so reopening the project reconnects and resubscribes with no browsing step.
14. **R14** — The API server exposes the OPC UA driver like the other drivers: its
    properties are readable and writable, the endpoint list and the tag browser are
    reachable as commands so the integration suite can drive the whole flow headless.
15. **R15** — A Python OPC UA server simulator lives under the examples tree with a project
    file and README; it serves a realistic industrial tag tree (booleans, integers, floats,
    strings, at least one array) with time-varying values and at least one tag that
    periodically reports a bad status.
16. **R16** — An integration test module drives the simulator end to end: endpoint
    discovery, anonymous and username/password connect, browse, project generation,
    subscription delivery, poll fallback, bad-status handling, reconnect, and project
    round-trip; it is marked Pro and skips when no simulator is reachable.
17. **R17** — A help-manual page "OPC UA" under the Drivers section documents the driver,
    its limits, the vendor compatibility list, and the simulator walkthrough, and is
    registered in the manual index.
18. **R18** — CI installs the simulator's Python dependency and launches the simulator
    before the Pro integration tier on the legs that run it, mirroring how the MQTT broker
    is launched today.

## Acceptance Criteria

- [x] **AC1** — `pytest tests/integration -m opcua` passes against a running Pro build with
      the simulator up; every R16 flow is a named test (R5, R9, R10, R11, R12, R13, R14, R16).
- [x] **AC2** — The same module reports skipped, not failed, when the simulator port is
      closed (R16), and the whole driver source is absent from a GPL build (R1, checked by the
      GPL CI leg compiling without the OPC UA Qt module linked).
- [x] **AC3** — Maintainer observation: connect to the simulator from the GUI, open the tag
      browser, tick a folder, generate the project; the dashboard shows every tag with its
      display name and type, strings included, updating at the publishing interval (R2, R6,
      R7, R8, R9).
- [x] **AC4** — Maintainer observation: connect with username/password to the simulator;
      the unencrypted-credentials warning is visible in the pane and appears once in the
      connection log; a simulator started with only a secured endpoint shows that endpoint
      greyed out and connect stays disabled (R2, R3, R4).
- [x] **AC5** — Recording a session and exporting CSV shows per-row timestamps matching the
      simulator's source timestamps within one publishing interval (R10).
- [x] **AC6** — Killing the simulator mid-session flips the connection state to disconnected
      with a reason; restarting the simulator and pressing connect resumes updates with no
      project regeneration (R12).
- [x] **AC7** — `--benchmark-hotpath` passes all nine gates unchanged on the Pro build that
      includes the driver (Constraints).
- [x] **AC8** — `scripts/code-verify.py --check` reports no new errors; the generated-artifact
      and registry checks pass (Constraints).
- [x] **AC9** — The help page renders in the in-app manual, links from the Drivers index,
      and `documentation-verify.py` passes (R17).
- [x] **AC10** — The example appears in the in-app examples gallery with a screenshot and
      project file, flagged Pro (R15).
- [x] **AC11** — CI: the Linux and macOS Pro integration legs run the `opcua` marker with the
      simulator running; Windows drops the marker the way it drops `requires_broker` (R18).

## Constraints & Invariants

- Pro-only: gated behind the commercial build flag at the source-list, enum, UI, and
  documentation level, same tier as Modbus and CAN Bus.
- Zero acquisition-pipeline change. The driver must deliver data through the same captured-
  data path every non-stream driver uses; the frame reader, frame builder, block pools, and
  the nine benchmark gates are untouched. OPC UA rates (tens to hundreds of Hz) do not
  justify the dense-stream lane.
- Source owns time: the timestamp attached to a frame is the server's source timestamp
  captured at the driver boundary, never re-derived downstream.
- Exactly one connect verdict per attempt; an async dial that only reports success is a
  defect (wedged connect button, known failure mode).
- No new vendored library: the Qt OPC UA module (already installed in every CI toolchain)
  is the only new build-time dependency. Its shipped open62541 backend has no encryption
  support, which is what bounds this spec to policy None. The test simulator's Python package is a test/
  example dependency only and lives in the single Python requirements manifest.
- No per-value allocation churn in steady state beyond what Modbus already does; a
  subscription callback must not trigger project-model or UI work.
- Secrets follow the existing driver-secret storage; the project file never holds a
  plain-text password.
- Generated artifacts (property tables, API surface, registries) are regenerated from their
  source descriptors, never hand-edited; new commands go through the command registry.
- Trademarks (FactoryTalk, KEPServerEX, Ignition, TwinCAT, ...) appear only in documentation
  as nominative compatibility statements, never in driver, enum, UI, or file names.
- Must coexist with the existing connection/diagnostics model: pulled 1 Hz counters (values
  received, bad-status count, subscription vs poll mode, reconnects), never pushed per value.

## Open Questions

- Publishing interval vs. sampling interval: expose both, or expose one "update rate" and
  set sampling = publishing? Recommendation: one rate in the first version.
- Tag selection cap: should the browser refuse or warn above N tags (frame width, UI)?
  Recommendation: warn above 512, hard cap 2048, matching the widest Modbus map imports.
- Should string tags be length-capped in the frame schema (e.g. 256 bytes) to keep the
  frame bounded? Recommendation: yes, truncate with a diagnostic counter.
- Does the Modbus register-map importer page in the help manual gain a sibling section, or
  does OPC UA browse get its own auto-generation page? Recommendation: own page, cross-link.
