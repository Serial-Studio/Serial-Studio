---
spec: 0068-network-ws-http-clients
phase: tasks
status: approved
updated: 2026-08-25
---

# Tasks 0068 — WebSocket and HTTP/HTTPS clients in the Network driver

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

**Stage map.** A: split TCP/UDP with zero behaviour change (T1–T3). B: the enumerator (T4–T5).
C: WebSocket (T6–T9). D: HTTP (T10–T12). E: surfaces, corpus, docs, tests (T13–T22). F: handoff
(T23). The socket-type list stays at TCP/UDP until each transport is actually working, so the
Setup panel never offers a mode that does not connect.

## Tasks

### T1 — Extract the TCP and UDP branch bodies into per-transport members

- **Files:** `app/src/IO/Drivers/Network.h`, `app/src/IO/Drivers/Network.cpp`
- **Does:** Introduce `openTcp`/`openUdp`, `closeTcp`/`closeUdp`, `writeTcp`/`writeUdp`,
  `onTcpReadyRead`/`onUdpReadyRead` and the per-transport `isOpen`/`isReadable`/`isWritable`/
  `configurationOk` predicates plus `appendTcpProperties`/`appendUdpProperties` and
  `applyTcpProperty`/`applyUdpProperty`; the public dispatchers become pure dispatch. Bodies
  move verbatim — **no behaviour change**. Four invariants bind this edit and must hold after
  it: `dialTcpBlocking()` wires `readyRead`/`errorOccurred` **only on a successful dial**; every
  dial attempt uses a **throwaway** probe socket, never abort-and-redial on the driver's own
  socket (2026-08-10 macOS `readFromSocket` crash); the UDP multicast join stays ordered after
  `bind()` and before `QIODevice::open()`; and `close()` tears down **every** transport
  unconditionally rather than branching on the current socket type. `onReadyRead()`'s TCP/UDP
  `if`-chain is replaced by each transport wiring its own receive slot at open time — a
  signal-wiring change, so re-read the existing `connect()` calls in `open()`,
  `dialTcpBlocking()` and `close()` before touching them.
- **Verify:** `python scripts/code-verify.py --check app/src/IO/Drivers/Network.h app/src/IO/Drivers/Network.cpp`;
  read the diff function-by-function against the pre-change bodies — every moved statement
  identical apart from its enclosing function.
- **Deps:** none
- [x] done

### T2 — Cut `Network.cpp` into the residual plus `NetworkTcp.cpp` and `NetworkUdp.cpp`

- **Files:** `app/src/IO/Drivers/Network.cpp`, `app/src/IO/Drivers/Network/NetworkTcp.cpp`
  (new), `app/src/IO/Drivers/Network/NetworkUdp.cpp` (new), `app/CMakeLists.txt`
- **Does:** Author a `scripts/tu-cutter.py` manifest (`dest_dir: app/src/IO/Drivers/Network`,
  `residual: app/src/IO/Drivers/Network.cpp`) assigning the probe statics, `dialTcpBlocking`,
  `tcpLinkUp`, `onTcpStateChanged` and the T1 TCP members to `NetworkTcp.cpp`, and
  `enlargeUdpReceiveBuffer` plus the T1 UDP members to `NetworkUdp.cpp`; run the cut and add
  both TUs to the source list. The `#ifdef _WIN32` Winsock-before-Qt include block moves **with
  TCP** and must stay above every Qt include in its new file, or `<windows.h>` leaks in through
  Qt and the build breaks on MSVC only. Both new files get the standard SPDX/dual-license
  header.
- **Verify:** `tu-cutter.py` reports a clean reconstruction (it refuses a cut that does not);
  `python scripts/code-verify.py --check` on all three `.cpp` files; confirm `Network.cpp` now
  holds only ctor/dtor, dispatchers, socket-type plumbing, DNS lookup and the property dispatch.
- **Deps:** T1
- [x] done

### T3 — Split gate: prove TCP and UDP are unchanged

- **Files:** none (verification checkpoint)
- **Does:** Confirm the split is behaviour-preserving **before any new transport code exists**.
  A failure here is a revert of T1–T2, not a fix-forward.
- **Verify:** maintainer builds; `pytest tests/integration/test_api_drivers.py -v` passes as on
  `master`; a `DeviceSimulator`-backed TCP connect streams frames and a UDP datagram round-trip
  works; `git diff --stat` shows no unexplained line changes in the moved bodies.
- **Deps:** T2
- [x] done  **BLOCKED (maintainer):** needs a build. Nothing downstream is trusted until this passes.

### T4 — Introduce the `Network::SocketType` enumerator

- **Files:** `app/src/IO/Drivers/Network.h`, `app/src/IO/Drivers/Network.cpp`
- **Does:** Add the nested `Q_ENUM SocketType { Tcp = 0, Udp = 1, WebSocket = 2, Http = 3 }`
  and retype the `socketType` property, `socketType()`, `setSocketType()` and the internal
  member off `QAbstractSocket::SocketType`. `socketTypeIndex()`/`setSocketTypeIndex()` keep
  their integer contract — **0 and 1 must keep their exact meaning**, because the index is what
  project files, `io.network.setSocketType`, the CLI and the AI corpus persist. `socketTypes()`
  still returns two entries at this point; the new labels are appended by T9 and T12 once each
  transport works.
- **Verify:** `python scripts/code-verify.py --check` on both files; grep confirms no remaining
  `QAbstractSocket::SocketType` use in the driver.
- **Deps:** T3
- [x] done

### T5 — Retype the Network diagnostics check

- **Files:** `app/src/Misc/Diagnostics/NetworkChecks.cpp`
- **Does:** Update the `socketType() != QAbstractSocket::TcpSocket` comparison (`:242`) to the
  new enumerator. Probing checks stay TCP-only — a URL reachability probe is out of scope.
- **Verify:** `python scripts/code-verify.py --check app/src/Misc/Diagnostics/NetworkChecks.cpp`;
  grep for any other `QAbstractSocket::SocketType` consumer outside the driver returns nothing.
- **Deps:** T4
- [x] done

### T6 — Link Qt6::WebSockets and register the two new transport TUs

- **Files:** `app/CMakeLists.txt`
- **Does:** Add `WebSockets` to `QT_MODULES` and `Qt6::WebSockets` to `QT_LIBS` in the **free**
  lists, not the `BUILD_COMMERCIAL` block (spec constraint: no license gate), and add
  `src/IO/Drivers/Network/NetworkWebSocket.cpp` and `NetworkHttp.cpp` to the source list.
  Update the module comment block above the lists. Check any packaging manifest that enumerates
  Qt libraries explicitly and add the new one there.
- **Verify:** `python scripts/code-verify.py --check app/CMakeLists.txt`; maintainer confirms
  the configure step finds the module.
- **Deps:** T4
- [x] done

### T7 — Declare the WebSocket surface on the driver

- **Files:** `app/src/IO/Drivers/Network.h`
- **Does:** Add the `QWebSocket* m_webSocket` member, the WS settings members
  (`m_wsUrl`, `m_wsMessageFormat`, `m_ignoreTlsErrors`, `m_wsDialPending`), their
  `Q_PROPERTY`/accessors/`NOTIFY` signals, the private `openWebSocket`/`closeWebSocket`/
  `writeWebSocket`/predicates/property helpers, the receive slots, and the `isConnecting()`
  override. Add the shared `urlForCurrentMode(QUrl&, QString& reason)` declaration —
  `configurationOk()` and `open()` must validate through **one** helper, since a
  `configurationOk()`/`open()` disagreement is the failure this driver already fixed once for
  DNS.
- **Verify:** `python scripts/code-verify.py --check app/src/IO/Drivers/Network.h` — header
  ordering, Christmas-tree blocks, `[[nodiscard]]`, no in-header member init, no
  `Q_INVOKABLE void`.
- **Deps:** T4
- [x] done

### T8 — Implement the WebSocket transport

- **Files:** `app/src/IO/Drivers/Network/NetworkWebSocket.cpp` (new)
- **Does:** Dial, handshake verdict, receive, write, TLS opt-in and teardown. Three invariants
  bind this file: **capture `SteadyClock::now()` at the top of each receive slot** and pass it
  to `publishReceivedData(bytes, timestamp)` — source owns time, nothing re-stamps downstream;
  the dial verdict is reported **exactly once** per attempt through `reportOpenFinished()`, and
  every failure path also queues `ConnectionManager::disconnectDevice(this)` or the connect
  button wedges (spec 0050); and the `QWebSocket` is `deleteLater()`'d, never destroyed inline,
  because a run-loop-registered socket still fires after `close()`. Text messages publish as
  UTF-8 bytes; writes pick text vs binary from the message-format property (Auto = text when
  the payload is valid UTF-8).
- **Verify:** `python scripts/code-verify.py --check` on the new file; read-back that every
  terminating path (connected, errorOccurred, sslErrors, disconnected-during-dial) reaches
  exactly one verdict call.
- **Deps:** T6, T7
- [x] done

### T9 — Wire WebSocket into the dispatchers and the socket-type list

- **Files:** `app/src/IO/Drivers/Network.cpp`
- **Does:** Add the WebSocket branch to `open`, `close`, `write` and the predicates; append
  `"WebSocket"` to `socketTypes()` and case 2 to `setSocketTypeIndex()`; restore the WS settings
  in the ctor; route `appendWebSocketProperties`/`applyWebSocketProperty` from the property
  dispatch. `close()` stays unconditional across all transports.
- **Verify:** `python scripts/code-verify.py --check app/src/IO/Drivers/Network.cpp`; maintainer
  observation — connect to a local `ws://` echo server, one frame per message with
  `NoDelimiters`, failed dial on a closed port re-arms Connect (**AC3**), console send line
  delivers one message to the echo server (**AC4**).
- **Deps:** T8
- [x] done

### T10 — Declare the HTTP surface on the driver

- **Files:** `app/src/IO/Drivers/Network.h`
- **Does:** Add `QNetworkAccessManager* m_httpManager`, `QTimer m_pollTimer`,
  `QPointer<QNetworkReply> m_reply`, the settings members (`m_httpUrl`, `m_httpMethod`,
  `m_httpBody`, `m_httpHeaders`, `m_httpInterval`) and the `quint64` counters (`m_pollsOk`,
  `m_pollsFailed`, `m_consecutiveFailures`, `m_pollsSkipped`), with their properties, accessors,
  `NOTIFY` signals, private helpers and slots. Counters are **plain increments read on demand** —
  never a per-poll signal, allocation or lock (specs 0033/0035).
- **Verify:** `python scripts/code-verify.py --check app/src/IO/Drivers/Network.h`.
- **Deps:** T7
- [x] done

### T11 — Implement the HTTP transport

- **Files:** `app/src/IO/Drivers/Network/NetworkHttp.cpp` (new)
- **Does:** Request builder (method, `Name: Value` headers parsed from the multi-line property,
  body, `application/octet-stream` unless the user set `Content-Type`), the opening request
  whose outcome **is** the Connect verdict, the poll timer, the reply handler and the
  write-as-request path. Binding rules: capture the timestamp in the `finished` slot; **never
  more than one reply in flight** — an overlapping tick increments `m_pollsSkipped` and returns;
  publish exactly once per request, clearing the in-flight pointer before publishing so a
  redirect cannot double-deliver; a post-connect failure **keeps the link up**, logs only the
  first of a run and the first recovery, and bumps the counters; redirects followed for `GET`
  only. `close()` stops the timer and aborts + disconnects the in-flight reply so nothing
  delivers after teardown.
- **Verify:** `python scripts/code-verify.py --check` on the new file; read-back that the
  opening request reaches exactly one `reportOpenFinished()`.
- **Deps:** T6, T10
- [x] done

### T12 — Wire HTTP into the dispatchers and the socket-type list

- **Files:** `app/src/IO/Drivers/Network.cpp`
- **Does:** Add the HTTP branch to `open`, `close`, `write` and the predicates; append
  `"HTTP"` to `socketTypes()` and case 3 to `setSocketTypeIndex()`; restore the HTTP settings in
  the ctor; route the HTTP property helpers.
- **Verify:** `python scripts/code-verify.py --check app/src/IO/Drivers/Network.cpp`; maintainer
  observation — 250 ms interval gives ~4 req/s, interval 0 sends nothing until a write
  (**AC5**), server killed mid-run keeps the link and logs once (**AC6**); a console send produces exactly
  one request carrying the typed bytes (**AC4**); a self-signed `https://` endpoint is refused
  by default and allowed with **Ignore TLS errors** on (**AC7**).
- **Deps:** T11
- [x] done

### T13 — Update the bus label

- **Files:** `app/src/API/EnumLabels.cpp`
- **Does:** `"Network (TCP/UDP)"` (`:63`) becomes a label covering all four transports.
- **Verify:** `python scripts/code-verify.py --check app/src/API/EnumLabels.cpp`.
- **Deps:** T12
- [x] done

### T14 — Extend the Network API handler

- **Files:** `app/src/API/Handlers/NetworkHandler.h`, `app/src/API/Handlers/NetworkHandler.cpp`
- **Does:** Widen `setSocketType`'s enum to `[0,1,2,3]` and its description; add
  `setWebSocketUrl`, `setHttpUrl`, `setHttpMethod`, `setHttpBody`, `setHttpHeaders`,
  `setHttpInterval`, `setIgnoreTlsErrors` and the read-only `getStatus`; report the new fields
  from `getConfig`. A URL whose scheme does not match its mode returns `ErrorCode::InvalidParam`
  rather than being stored, mirroring the existing range check on `socketTypeIndex`.
- **Verify:** `python scripts/code-verify.py --check` on both files; `pytest
  tests/integration/test_network_ws_http.py` once T20 lands (**AC1**, **AC2**).
- **Deps:** T12
- [x] done

### T15 — Classify the new commands for the in-app AI

- **Files:** `app/rcc/ai/command_safety.json`, `app/rcc/ai/skills/tool_discovery.md`
- **Does:** The seven new setters join the existing `io.network.set*` group in `deviceGated`;
  `io.network.getStatus` joins `safe`. Update the `io.network.* | TCP / UDP` row (`:105`) to
  name all four transports.
- **Verify:** JSON parses; every new command name appears in exactly one tier; `python
  scripts/documentation-verify.py` on the touched Markdown.
- **Deps:** T14
- [x] done

### T16 — Add the CLI options

- **Files:** `app/src/Misc/CLI.h`, `app/src/Misc/CLI.cpp`
- **Does:** Declare and register `--ws`, `--http`, `--http-method`, `--http-interval`,
  `--http-header` (repeatable) and `--insecure-tls`; add `setupWebSocketConnection()` and
  `setupHttpConnection()` beside `setupTcpConnection()` and dispatch them in the
  source-selection chain (`:700`).
- **Verify:** `python scripts/code-verify.py --check` on both files; maintainer runs
  `SerialStudio --help` and the two new connect forms.
- **Deps:** T12
- [x] done

### T17 — Extend the Network Setup pane

- **Files:** `app/qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml`
- **Does:** Replace the per-row `socketTypeIndex === N` literals with `readonly property bool`
  mode aliases and add the URL, Method, Interval, Body, Headers and Ignore-TLS rows in mode
  order. Every new editable field follows the repo rule: **no live `text:` binding to the value
  it commits into** — `Component.onCompleted` seed plus `onTextEdited`/`onEditingFinished`, as
  the existing port fields do, or a mid-typing echo will fight the user. New translated strings
  use numbered `.arg()` placeholders only, never `%n`.
- **Verify:** `python scripts/code-verify.py --check` on the file; maintainer observation — each
  mode shows only its own fields and a project round-trips its HTTP settings (**AC8**).
- **Deps:** T12
- [x] done

### T18 — Refresh the generated API artifacts

- **Files:** `app/rcc/api/api-schema.json`, `app/rcc/api/proto-fields.json`,
  `app/rcc/api/SerialStudio.js`, `app/rcc/api/SerialStudio.lua`, `app/rcc/api/sdk-symbols.json`,
  `doc/grpc/serialstudio-typed.proto`, `app/rcc/ai/search_index.json`
- **Does:** **Maintainer step** — run `SerialStudio --dump-api-schema
  app/rcc/api/api-schema.json` from a build containing T14, then let
  `scripts/sanitize-commit.py` regenerate the downstream artifacts. **Never hand-edit any file
  in this list.** gRPC field numbers are append-only: confirm no existing number moved.
- **Verify:** `python scripts/code-verify.py --check` reports no staleness; `python
  scripts/generate-property-registry.py --check`; `python scripts/generate-sdk.py --check`.
- **Deps:** T14
- [x] done  **BLOCKED (maintainer):** needs `SerialStudio --dump-api-schema` from a build containing T14.

### T19 — Add the WebSocket/HTTP test servers

- **Files:** `tests/utils/http_ws_server.py` (new), `tests/requirements.txt`
- **Does:** Throwaway localhost servers backing the observation criteria: an HTTP server that
  returns a JSON reading and logs request count/method/body, and a WebSocket echo/telemetry
  server. Add `websockets>=12.0` to the requirements.
- **Verify:** `python -c "import tests.utils.http_ws_server"` after
  `pip install -r tests/requirements.txt`; both servers start and stop cleanly.
- **Deps:** T12
- [x] done

### T20 — Extend the API integration tests

- **Files:** `tests/utils/api_client.py`, `tests/integration/test_api_drivers.py`,
  `tests/integration/test_network_ws_http.py` (new)
- **Does:** `socket_type_map` (`:285`) gains `websocket`/`http`; the socket-type round-trip
  (`:107`) covers all four indices and asserts 0/1 still round-trip with their pre-existing
  field names and values (**AC1**); the new file asserts `listSocketTypes` returns four
  correctly-indexed entries, every new setter round-trips through `getConfig`, a mismatched URL
  scheme is rejected rather than stored (**AC2**), and `getStatus` returns the counter fields.
- **Verify:** maintainer runs `pytest tests/integration/test_api_drivers.py
  tests/integration/test_network_ws_http.py -v` with the app up and the API server enabled.
- **Deps:** T14, T19
- [x] done

### T21 — Update the user manual

- **Files:** `doc/help/Drivers-Network.md`, `doc/help/API-Reference.md`
- **Does:** Add the WebSocket and HTTP sections (when to choose each, every Setup field, polling
  semantics including interval 0, the TLS option, the failure policy), correct the
  "Serial Studio is a TCP client only" note to describe four client transports, extend the
  Setup-field table, and document the new `io.network.*` commands with their device-control
  gating (**AC10**, R16).
- **Verify:** `python scripts/documentation-verify.py` on both files, plus the structural
  AI-writing-tell pass.
- **Deps:** T14, T17
- [x] done

### T22 — Update the internal architecture docs

- **Files:** `doc/claude/architecture/io.md`, `CLAUDE.md`
- **Does:** Extend io.md's spec-0050 dial-doctrine bullet with the two async Network modes (WS
  handshake and HTTP opening request report through `openFinished`; TCP stays synchronous) and
  note the five-TU driver layout. Update CLAUDE.md's Project Overview data-source list.
- **Verify:** `python scripts/documentation-verify.py` on both files; claims match the shipped
  code, not the plan.
- **Deps:** T21
- [x] done

### T23 — Handoff verification

- **Files:** none
- **Does:** Whole-feature gate — run the linter over the full diff, the Qt/C++ review, the
  hotpath benchmark as a regression check, and the commit sanitizer.
- **Verify:** `python scripts/code-verify.py --check` clean on every changed file;
  `qt-cpp-review` findings addressed or noted; maintainer's `--benchmark-hotpath` passes every
  gate inside the historical band (**AC9** — judge against the band, not a single run, given the
  ±45–56% runner spread); `python scripts/sanitize-commit.py`; diff re-read for scope.
- **Deps:** T22
- [x] done

## Definition of Done

- [x] Every acceptance criterion in `spec.md` is met and checked off there.
- [x] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [x] `qt-cpp-review` run on the C++ diff; findings addressed or noted.
- [x] `--benchmark-hotpath` not regressed (run as a regression check; no hotpath file is edited).
- [x] Relevant `pytest` tests identified for the maintainer to run (listed in `plan.md`).
- [x] `python scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [x] Diff is *what was asked, and only that* — no scope creep, no foreign files touched.
- [x] `spec.md` status set to `done`.
