---
spec: 0068-network-ws-http-clients
title: WebSocket and HTTP/HTTPS clients in the Network driver
status: done          # closed 2026-08-25
created: 2026-08-25
author: Alex Spataru
---

# Spec 0068 — WebSocket and HTTP/HTTPS clients in the Network driver

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

The Network driver speaks exactly two protocols, TCP and UDP, and the manual states the
limitation outright: "Serial Studio is a TCP client only... If the device expects to push data
to a listener, run a small TCP server in front of Serial Studio." That instruction — write a
bridge process — is also the only answer available today for the two transports that most
modern telemetry sources actually expose: a WebSocket endpoint and an HTTP/HTTPS REST API.

A device gateway, a cloud broker, or a lab test rig that publishes over `wss://` cannot be
connected at all without a user-written relay that terminates the WebSocket and re-emits raw
bytes on a local TCP port. The same is true for any device or service whose only interface is
a REST endpoint returning a JSON reading per request: to chart it, the user has to write a
polling script, and at that point Serial Studio has been reduced to a viewer for that script's
output. Both cases lose the things the app exists to provide — the connect/disconnect UI,
project-file persistence, the console, output widgets, the recording and export fan-out.

Nothing about either protocol is exotic for this app. A WebSocket message is message-shaped
like a UDP datagram, and an HTTP response body is a discrete chunk of bytes; both drop into
the existing acquisition pipeline unchanged. What is missing is a first-class transport choice
so the user configures a URL instead of building a bridge.

## Goals

- A user can select **WebSocket** as the Network socket type, enter a `ws://` or `wss://`
  URL, click Connect, and see the endpoint's messages flow into the dashboard.
- A user can select **HTTP** as the Network socket type, enter an `http://` or `https://`
  URL and a poll interval, click Connect, and see each response body flow into the dashboard
  as a frame.
- Both are usable from the console and output widgets: sending in WebSocket mode delivers a
  message to the peer; sending in HTTP mode issues one request carrying that payload, so an
  API can be *interacted with*, not only read.
- Both are configurable through every surface TCP/UDP already are — Setup panel, project file,
  JSON-RPC API, and the in-app AI's device commands — with no new concepts to learn beyond a
  URL, a method, and an interval.
- Both ship in the free build, alongside TCP and UDP, with no license gate.
- Existing TCP and UDP projects, scripts, and API clients keep working byte-for-byte.

## Non-Goals

- **No server roles.** Serial Studio does not listen for inbound WebSocket or HTTP
  connections. It remains a client for every Network socket type.
- **No Server-Sent Events, HTTP long-polling, or HTTP/2 server push.** HTTP mode is
  request/response polling only.
- **No OAuth flows, cookie jars, or credential managers.** Authentication is whatever the
  user puts in a request header.
- **No response templating or per-poll transformation inside the driver.** The response body
  reaches the parser as received; shaping it is the frame parser's job, as it is for every
  other driver.
- **No WebSocket subprotocol negotiation UI, compression tuning, or fragmentation control.**
- **No change to frame detection.** Neither transport introduces a new framing mode; both
  publish bytes into the existing FrameReader.
- **No MQTT-over-WebSocket.** MQTT stays its own driver.
- **No renumbering** of the existing socket type indices, and no migration of stored settings.

## Requirements

1. **R1** — The Network **Socket Type** selector offers four entries in this order: TCP (0),
   UDP (1), WebSocket (2), HTTP (3). Indices 0 and 1 keep their current meaning everywhere
   they are persisted or scripted.
2. **R2** — In WebSocket mode the user configures a single **URL** field accepting `ws://` and
   `wss://`; the separate address/port/multicast fields are not shown. A URL with any other
   scheme, or a malformed one, blocks Connect and says why.
3. **R3** — In WebSocket mode, every text or binary message received from the peer is
   delivered to the acquisition pipeline as one discrete chunk, in arrival order. A project
   using `NoDelimiters` frame detection therefore gets exactly one frame per message; a
   project using a delimiter gets the same result it would over TCP.
4. **R4** — In WebSocket mode, anything written by the user (console send line, action
   widgets, output widgets, the API's write command) is transmitted as a single WebSocket
   message. Text-mode payloads are sent as a text message, hex/binary payloads as binary.
5. **R5** — In HTTP mode the user configures a **URL** accepting `http://` and `https://`, an
   HTTP **method**, an optional request **body**, an optional list of custom request
   **headers** (name/value pairs), and a **poll interval** in milliseconds.
6. **R6** — While connected in HTTP mode, the driver issues the configured request once per
   poll interval and delivers each response **body** to the acquisition pipeline as one
   discrete chunk. Nothing but the body reaches the parser — no status line, no headers, no
   synthesized envelope.
7. **R7** — A poll interval of 0 means "do not poll": the driver connects, issues no periodic
   requests, and only sends a request when the user writes. This is the mode for a
   command/response API driven entirely from action or output widgets.
8. **R8** — In HTTP mode, a user write issues one immediate request carrying the written bytes
   as the request body, using the configured method, URL and headers. Its response body is
   delivered to the pipeline exactly like a polled one.
9. **R9** — A failed poll (transport error, TLS failure, or a non-2xx status) does **not**
   disconnect the link. The driver logs the first failure of a run and keeps polling;
   consecutive-failure and total-failure counts are visible as driver diagnostics on the
   existing 1 Hz diagnostics tick. A subsequent success resets the consecutive count.
10. **R10** — `wss://` and `https://` validate the server certificate chain by default. A
    per-project **Ignore TLS errors** option, off by default, allows a self-signed or
    hostname-mismatched certificate for lab endpoints; when it is on, the console states that
    certificate verification was bypassed.
11. **R11** — Clicking Connect produces exactly one verdict, at most once per attempt, with
    the same UI behaviour as TCP: WebSocket reports the handshake outcome (including HTTP
    upgrade rejections and TLS failures) with the reason shown to the user; HTTP reports the
    outcome of a single probe request against the configured URL, so an unreachable host or a
    404 is caught at Connect rather than silently at the first poll. Neither mode may leave
    the Connect button wedged in a connecting state.
12. **R12** — When the peer closes a WebSocket, or the link drops, the app returns to the
    disconnected state and says why, the same way a dropped TCP link does.
13. **R13** — All new settings persist across app restarts, and are captured in and restored
    from a project file's source settings, exactly as the TCP/UDP settings are.
14. **R14** — The JSON-RPC API exposes every new setting under `io.network.*`, and
    `io.network.listSocketTypes` and `getConfig` report the four types and the active
    configuration. New setters are classified as device control (behind the **Allow device
    control** toggle for the in-app AI); new readers are read-only.
15. **R15** — The `--benchmark-hotpath` gates and the acquisition pipeline's per-frame cost
    are unchanged by this feature. No new work is added to the publish path for TCP/UDP
    sources.
16. **R16** — The manual documents both transports: when to choose each, every Setup field,
    the polling semantics, the TLS option, and the new API commands. The driver page's
    "TCP client only" note is corrected to reflect the four transports.

## Acceptance Criteria

- [ ] **AC1** (R1, R14) — `pytest tests/integration/` — an API test asserts
      `io.network.listSocketTypes` returns four entries with indices 0=TCP, 1=UDP,
      2=WebSocket, 3=HTTP, and that setting index 0 or 1 still round-trips through
      `getConfig` with the pre-existing field names and values.
- [ ] **AC2** (R2, R5, R13) — `pytest tests/integration/` — setting each new property through
      `io.network.*` and reading it back through `getConfig` returns the written value;
      an invalid URL scheme is rejected with an error response rather than silently stored.
- [ ] **AC3** (R3, R11, R12) — Maintainer observation against a throwaway local WebSocket
      echo server: connecting to `ws://127.0.0.1:<port>` succeeds, each server message appears
      as one frame in the console with `NoDelimiters` selected, killing the server returns the
      app to disconnected with a stated reason, and connecting to a closed port fails at
      Connect with a reason and a re-armed Connect button.
- [ ] **AC4** (R4, R8) — Maintainer observation: typing into the console send line while
      connected in WebSocket mode delivers one message to the echo server; doing the same in
      HTTP mode produces exactly one request in the test server's log, with the typed bytes as
      its body, and the response appears as a frame.
- [ ] **AC5** (R6, R7) — Maintainer observation against a local HTTP test server returning a
      JSON reading: with a 250 ms interval the dashboard updates ~4x/s and the server logs
      ~4 requests/s; with the interval set to 0 the server logs no requests until a write.
- [ ] **AC6** (R9) — Maintainer observation: stopping the HTTP test server mid-run leaves the
      app connected, logs one error rather than a per-poll flood, and shows a rising failure
      count; restarting the server resumes charting without a reconnect.
- [ ] **AC7** (R10) — Maintainer observation against a self-signed `https://` endpoint:
      the connection is refused with a certificate error by default, and succeeds with the
      console warning when **Ignore TLS errors** is enabled.
- [ ] **AC8** (R13) — Maintainer observation: a project saved while in HTTP mode reopens with
      the URL, method, headers, body and interval restored, and connects without re-entry.
- [ ] **AC9** (R15) — `--benchmark-hotpath` on the maintainer's build passes every gate, with
      results inside the historical run-to-run band.
- [ ] **AC10** (R16, all) — `scripts/code-verify.py --check` and
      `scripts/documentation-verify.py` report no new errors, and the Network driver page
      documents both transports.

## Constraints & Invariants

- **The transport identity is a first-class Serial Studio enumeration, not Qt's socket-type
  enum.** `QAbstractSocket::SocketType` cannot name a WebSocket or an HTTP client, so the
  driver's notion of "which transport" must be its own enumerator covering all four. Whatever
  crosses the API, project files and the CLI stays the integer index of R1.
- **Socket type indices are a compatibility surface.** 0=TCP and 1=UDP appear in project
  files, in `io.network.setSocketType`, in the AI command corpus and in the manual. New types
  append; nothing renumbers, and no stored value migrates.
- **A driver open is a synchronous call, but a WebSocket handshake is asynchronous.** The
  async dial verdict has exactly one owner and is reported exactly once per attempt; a mode
  that only reports success wedges the Connect button. HTTP mode's Connect verdict has the
  same obligation.
- **Nothing bypasses the driver's single ingestion point.** Both transports publish bytes the
  same way TCP and UDP do; no new path into FrameBuilder, no per-message queued hop into the
  pipeline, no reduction or rate cap in the driver.
- **No allocation or work added to the existing publish path.** TCP and UDP throughput must
  be indistinguishable before and after.
- **Free build.** Both transports compile and run without `BUILD_COMMERCIAL` and without an
  activated license, on Windows, macOS and Linux, in every packaging target.
- **Qt only, and only modules that are redistributable under the app's GPL build.** No new
  third-party dependency, and no module that forces a licensing change on GPL users.
- **Timers must not run on the acquisition pipeline thread**, and the poll must not block the
  GUI thread — a slow or hung endpoint may never freeze the UI, however long it takes to
  time out.
- **A driver teardown must be final.** After disconnect, no poll timer, in-flight request, or
  reconnect attempt may deliver anything or redial.
- **`configurationOk()` must agree with `open()`.** A URL good enough to arm the Connect
  button must be a URL the open path accepts, and vice versa; gating one on an async verdict
  the other does not see is the failure mode this driver already fixed once for DNS.

## Open Questions

All resolved at approval (2026-08-25); each proposal below was accepted as written.

- **Default poll interval and its floor.** *Resolved:* default 1000 ms, minimum 10 ms, no
  maximum beyond the field's int range. A floor above 0 keeps a user from accidentally
  hammering a public API; 0 stays reserved for "manual only" per R7.
- **Overlapping polls.** *Resolved:* if a response has not arrived when the next interval
  elapses, skip that tick — never more than one request in flight — and count the skip.
  Requests are never queued.
- **Redirects.** *Resolved:* follow up to a small fixed number of redirects for `GET`, and
  never for methods carrying a body.
- **Content-Type for written payloads.** *Resolved:* send `application/octet-stream` unless
  the user supplied a `Content-Type` header, which always wins.
- **Whether the WebSocket URL field replaces or reuses the Remote Address field.** *Resolved:*
  a separate URL setting. The two are validated differently, and users switch modes back and
  forth without wanting their TCP/UDP address rewritten.
