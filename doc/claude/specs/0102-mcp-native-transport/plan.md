---
spec: 0102-mcp-native-transport
phase: plan
status: approved     # draft -> approved (gate before /ss-tasks); 2026-11-03, maintainer
updated: 2026-11-03
---

# Plan 0102 — MCP Session Hygiene

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Read the relevant `doc/claude/` sub-docs and the *actual code*
> before writing this — a plan grounded in a stale mental model is worse than no plan.
> Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

A connection becomes an MCP session the first time the reception machine routes one of its
messages to the MCP handler. At that moment, before the dispatch, `ClientReception` sets a new
`ConnectionState::mcpSession` flag and calls a new host hook. `Server` then tears down any
mirror or stream subscription the socket already held and queues one call to the worker that
adds the socket to a new `m_mcpSockets` set. The worker's three broadcast lanes (frames, raw
bytes, lifecycle events) skip that set. The two opt-in lanes (mirror, stream) are closed at
subscribe time: `MCPHandler` answers a `tools/call` of a connection-scoped push verb with an
`isError` tool result, and `routeConnectionCommand` refuses the same verbs sent as legacy lines
on an MCP session. `handleToolsCall` keeps the `RemoteDispatchScope` and `registry.execute`
exactly where they are and changes only how a failed `CommandResponse` is wrapped: it becomes a
successful JSON-RPC result with `isError: true`. `initialize` stops advertising `subscribe`, and
both subscription methods answer `-32601`. The mute is a separate flag rather than a reuse of
the mirror viewer's `streamFrames`, because that one covers only the frame lane, and
`mirror.md` pins that only `mirror.subscribe` may flip it.

## Affected subsystems & files

Every path below was confirmed by grep on 2026-11-03. Amended during `/ss-tasks` (2026-11-03):
the push-verb refusal text moves to `MCPProtocol.h` as `MCP::kPushRefusal`, because
`MCPHandler.cpp` cannot reach a file-local constant in `Server.cpp`, and a new `Server`
accessor would be one more public symbol for a string.

| File | Change |
|------|--------|
| `core/Api/API/Server/ConnectionState.h` | Add `bool mcpSession = false;` with a one-line comment: set on the first JSON-RPC message, after which the connection receives replies only (spec 0102). The aggregate keeps its in-struct defaults, following the existing fields. |
| `core/Api/API/Server/ClientReception.h` | `ReceptionHost` gains `virtual void enterMcpSession(QTcpSocket* socket, ConnectionState& state) = 0;`. The hook is pure virtual, so a host that does not mute fails to compile instead of falling back to a silent default. |
| `core/Api/API/Server/ClientReception.cpp` | `handleJsonMessage`, MCP branch (line 553): before `dispatchMcp`, `if (!state->mcpSession) { state->mcpSession = true; m_host.enterMcpSession(socket, *state); }`. Nothing reads `state` after the dispatch (spec 0075 I1 holds). No other function changes; the auth handshake, the raw-write gate and the limits are untouched. |
| `core/Api/API/Server.h` / `Server.cpp` | (1) `enterMcpSession` override. If the socket holds a mirror subscription, call `mirrorPublisher().unsubscribe` and clear `mirrorSubscribed`. If it holds a stream subscription, clear `streamSubscribed` and `streamPending`, call `refreshStreamSubscriberFlag()` and emit `streamSubscribersChanged`. Then queue `setSocketMcpSession(socket, sessionId)` on the worker, in the same shape as `setStreamFrames` (line 726). (2) `isStreamCommand` stays private. A new public static `isConnectionCommand(name)` returns `MirrorCommands::isMirrorCommand(name) \|\| isStreamCommand(name)`, so the five-verb list lives in one place. (3) `routeConnectionCommand` (line 681): on `state.mcpSession`, a connection command is answered with `CommandResponse::makeError(id, ExecutionError, MCP::kPushRefusal)` and returns true. |
| `core/Api/API/Server/ServerWorker.h` / `ServerWorker.cpp` | New public slot `setSocketMcpSession(QTcpSocket*, const QString& sessionId)`, session-id checked like `setSocketStreamFrames`. New `QSet<QTcpSocket*> m_mcpSockets`, cleared in `addSocket`, `removeSocket` and `closeResources` the way `m_mutedSockets` is. `processItems`, `writeRawData` and `broadcastEvent` skip members of the set, and the check runs **before** `underWriteCap`, so an MCP socket is never counted in `droppedBroadcasts`. `writeRawData` builds its target list before serializing, matching `processItems`, so an all-MCP server serializes nothing. `writeToSocket`, `writeMirrorPayload` and `writeStreamBlock` are unchanged. |
| `core/Api/API/MCPProtocol.h` | `inline constexpr QLatin1StringView kPushRefusal` in `namespace API::MCP`: the push-verb refusal text below, the one copy both `Server.cpp` and `MCPHandler.cpp` read. |
| `core/Api/API/MCPHandler.h` / `MCPHandler.cpp` | `handleToolsCall`: a non-object `arguments` gets `-32602`, and a name for which `Server::isConnectionCommand` is true gets `connectionVerbResult()`. Both checks run before `hasCommand`. The scope and `execute` lines (312-316) stay unchanged, and a failed response is wrapped by a new private `toolErrorResult(name, arguments, response)`. `handleInitialize`: `resources` becomes `{}`. `resources/subscribe` and `resources/unsubscribe` route to one `refuseSubscription(request)`, which answers `-32601`. `ClientSession::subscribedResources` and the two old handlers are removed, which also removes their `m_sessions[sessionId]` `operator[]` inserts. |
| `app/tests/tst_client_reception.cpp` | `TableHost` overrides `enterMcpSession` and records each call. New case `firstMcpMessageEntersTheSessionOnce`: two JSON-RPC lines call the hook once, and a legacy command line never calls it. |
| `app/tests/tst_server_auth.cpp` | `StubHost` gains the override (a counter). This is a compile requirement; no test expectation changes. |
| `app/tests/fuzz/fuzz_api_json.cpp` | `FuzzHost` gains the override as an empty body (compile requirement). |
| `app/tests/tst_server_worker_caps.cpp` | New case `mcpSocketsReceiveRepliesOnly`. After `setSocketMcpSession`, `writeRawData` and `broadcastEvent` leave `bytesToWrite()` unchanged and do not count in `droppedBroadcasts`, while `writeToSocket` still writes. Real loopback sockets, the donor pattern of `broadcastsAreSkippedOverTheCap`. |
| `tests/integration/test_mcp.py` | New class `TestSessionHygiene` (AC1, AC2). Rewrites of the error-shape tests (AC3) and the subscription tests (AC4); the complete list is under Test & verification. `MCPClient` gains a `tool_error(name, args)` helper that asserts `isError` and returns the joined text. |
| `tests/unit/test_mcp_bridge.py` | Q2 (accepted): a local fake server sends a `{"frames":...}` line and then a JSON-RPC line, and the test checks that the bridge's stdout carries only the JSON-RPC line. |
| `tests/README.md` | Update the catalog rows for `test_mcp.py` (session hygiene, error shape) and `test_mcp_bridge.py` (the line filter, Q2). |
| `doc/help/API-Reference.md` | In the MCP section, new "Session rules", "Errors" and "Resources" subsections (R4). The Connection Handling comment at line 621 becomes "go to every client except an MCP session". A Changelog entry under the in-progress release says that MCP tool failures are now `isError` results and that subscription is no longer advertised. |
| `doc/help/Data-Flow.md` | Line 87 currently says the API broadcasts frames "using MCP (JSON-RPC 2.0) or the legacy protocol", which becomes false. It changes to: legacy clients receive the broadcast, and an MCP session reads frames on request. |
| `examples/MCP Client/README.md` | Remove "Resources support **subscription**" (line 175) and the `resources/subscribe` bullet (517). Rewrite the anomaly example (540-545) and the sample prompt (379) to poll `resources/read`. Add one paragraph on `isError`. |
| `examples/MCP Client/client.py` | `call_tool` reports `result["isError"]` as a failure with its text. `_read_response` skips every line that has no `jsonrpc` key, not only `{"data"}`. |
| `examples/MCP Client/claude_desktop_bridge.py` | The banner at line 209 drops "182+ commands" in favor of "Tools: every listed API command (tools/list)". With Q2 accepted, `tcp_to_stdout` forwards only lines that parse as a JSON array or as an object with `"jsonrpc": "2.0"`, and logs the dropped count once. Framing stays newline-delimited, which spec 0112 decided. |
| `doc/claude/architecture/mirror.md` | One sentence after the `streamFrames` bullet: an MCP session is muted through `ConnectionState::mcpSession` and the worker's `m_mcpSockets`, never through `streamFrames`, so the rule that only `mirror.subscribe` flips that flag still holds. |

Out of this repository, for the maintainer: `Serial-Studio/claude-plugin` ships `bridge.py`,
which must stay byte-identical with the examples bridge. Mirror the banner change and the Q2
filter there, and check that its bundled skill does not describe JSON-RPC errors
as the failure signal.

Not edited, by design: `ServerAuth.*`, `ConsentGate.*`, `AuthPrimitives.*`, `ConsentVerdict.h`,
`DeviceWriteVerdict.h`, `LaunchConsent.*`, `CommandCapabilities.*`, `SealedPolicy.*`,
`CommandRegistry.*`, `command_safety.json`, the generated API surfaces, and every handler under
`core/Api/API/Handlers/`.

## Architecture & data flow

**Push paths a socket can receive today.** All five are written by `API::ServerWorker` on the
API worker thread.

| # | Lane | Producer → worker slot | Gate today | After this plan |
|---|------|------------------------|------------|-----------------|
| P1 | Per-frame broadcast `{"frames":[...]}` | `Server::ingestBlock` (pipeline thread, `enqueueData`) → `processItems` | `m_mutedSockets` (mirror viewers), write cap | + skip `m_mcpSockets` |
| P2 | Raw device bytes `{"data":"<b64>"}` | `IRawByteTap::onDeviceBytes` / `onInjectedBytes` → `hotpathTxData` → `writeRawData` | write cap only (mirror viewers receive it too) | + skip `m_mcpSockets` |
| P3 | Lifecycle events `{"event":"connected"}` | `ModuleManager` lambda on `ConnectionManager::connectedChanged` → `broadcastLifecycleEvent` → `broadcastEvent` | write cap only | + skip `m_mcpSockets` |
| P4 | Mirror payloads | `MirrorPublisher::payloadReady` → `Server::sendMirrorPayload` → `writeMirrorPayload` | `mirrorSubscribed` on the GUI thread | never set on an MCP session (refused at subscribe, torn down on entry) |
| P5 | Typed stream blocks `{"streamBlock":...}` | `StreamWorker::blockReady` → `Server::pushStreamBlock` → `writeStreamBlock` | `streamSubscribed` on the GUI thread | as for P4 |

The spec names P1, P4 and P5. P2 and P3 are covered by R1's "pushes nothing ... that is not an
MCP message", and `API-Reference.md` line 621 documents all three broadcasts as going to every
client.

**How a `tools/call` could switch a push on.** `mirror.subscribe`, `mirror.setRate`,
`mirror.unsubscribe`, `stream.subscribe` and `stream.unsubscribe` are connection-scoped. They
are not registered in `CommandRegistry` (the comments on `MirrorHandler::registerCommands` and
`Server::isStreamCommand` say so),
so today a `tools/call` with one of these names answers "Unknown tool" (`-32602`). `project.batch`
and the legacy batch re-dispatch through `registry.execute`, so they cannot reach these verbs
either. The one open path is a legacy `{"type":"command","command":"mirror.subscribe"}` line on
the same socket: `handleJsonMessage` serves both protocols per message, and
`routeConnectionCommand` would attach the push. No registry command attaches a per-socket push.
AC2 needs a tool result for these names instead of the unknown-tool protocol error, so
`handleToolsCall` recognizes them before `hasCommand`.

**Sequence on the first MCP message** (GUI thread, then the API worker):

```
ClientReception::handleJsonMessage(socket, sessionId, bytes)
  state = stateFor(socket, sessionId)                 // resolved once, before any dispatch
  validateJsonMessage(...)
  MCP::isMCPMessage(bytes) -> true
    if !state->mcpSession:
      state->mcpSession = true
      host.enterMcpSession(socket, *state)            // Server: teardown mirror/stream, then
        QMetaObject::invokeMethod(worker, "setSocketMcpSession", Queued, socket, sessionId)
    response = host.dispatchMcp(bytes, sessionId)     // may spin an event loop (tools/call)
    host.sendResponse(socket, response)               // queued writeToSocket, after the mute
```

Both queued calls go from the GUI thread to the same receiver, so the worker applies the mute
before it writes the first MCP reply. Any broadcast the worker wrote before the mute precedes
that reply on the wire, which is why AC1's five-second window, counted from the `initialize`
response, is deterministic. The interval between a TCP connect and the first MCP message is
not covered by R1: a plain-TCP listener must keep receiving the broadcast before it speaks.
The bridge filter (Q2) closes that interval for the bridge path.

**Reply-shape change (R2).** Only the code after `registry.execute` changes:

```
response = registry.execute(name, QString(), arguments)   // under RemoteDispatchScope, unchanged
response.success  -> makeSuccess(id, {content:[{type:text, text:<compact result JSON>}]})
!response.success -> makeSuccess(id, toolErrorResult(name, arguments, response))
  {
    content: [
      {type: text, text: "<errorMessage>"},
      {type: text, text: <compact JSON {
          error: {code: <errorCode>, message: <errorMessage>, data: <errorData>},
          tool, providedArguments,
          parameter, receivedValue, receivedType, suggestion   // the existing "must be" hint
          result                                               // only when response.result != {}
      }>}
    ],
    isError: true
  }
```

`errorData` carries what the registry already attaches (`category`, `inputSchema`,
`commandName`, `dryRunHint`) and what the gates attach. Today the MCP path drops all of it,
because it forwards `response.result` and never reads `errorData`. `receivedType` changes from
the Qt variant name (`QString`) to the JSON type name (`string`, `number`, `bool`, `object`,
`array`, `null`), which is Q5. JSON-RPC errors stay for: a parse error, an invalid request, an
empty or oversized batch, an uninitialized session (`-32600`), an unknown method (`-32601`), a
missing or non-string tool name, an unknown tool, a non-object `arguments`, a missing resource
URI, an unknown resource or prompt (`-32602`), and resource subscription (`-32601`).

**Text for the push-verb refusal** (`MCP::kPushRefusal` in `MCPProtocol.h`, used by both the tool
result and the legacy line): "This connection is an MCP session and receives replies only;
connection-scoped push commands are not available on it. Read telemetry with resources/read
serialstudio://frame/current, or poll dashboard.getData or dashboard.tailFrames." A text like
this reads as an instruction to the model, which is the intent.

### Invariants that bind this change

1. **Never hold `ConnectionState` across a dispatch** (spec 0075 I1). The flag and the hook run
   before `dispatchMcp`, and nothing reads `state` afterward.
2. **Every worker hop carries the session id**, because socket pointers are reused.
   `setSocketMcpSession` checks it the way `setSocketStreamFrames` does.
3. **Only `mirror.subscribe` flips `streamFrames`** (`mirror.md`). The MCP mute is a separate
   flag.
4. **The verdict precedes the reply shape.** `RemoteDispatchScope` wraps `registry.execute` for
   its full lifetime, and reshaping happens only after the response exists.
5. **Non-MCP clients see no change.** Only a socket whose message `MCP::isMCPMessage` accepted
   is marked, and the legacy protocol is object-with-`type` only (`parseMessage`), so no legacy
   message can trip the mark.

### Decision-visible surface: API server auth and consent (Trust Contract)

**No gate's verdict changes.** Every gate runs in the same order, with the same inputs, under the
same `RemoteDispatchScope`, and the gate files listed as "Not edited" above stay untouched. Only
the JSON-RPC envelope around a refusal changes. These gates reach `handleToolsCall`'s
`!response.success` branch, so their refusal **shape** changes on MCP only (the legacy TCP
reply is unchanged):

| Gate | Where it refuses | Error code it carries |
|------|------------------|-----------------------|
| Sealed-session policy (`SealedPolicy::allowedWhenSealed`) | `CommandRegistry::execute` | `SEALED_SESSION` |
| Remote gate, unclassified command (`CommandCapabilities::of` empty) | `ServerAuth::authorizeRemoteCommand` | `EXECUTION_ERROR` |
| Remote gate, control-script-only | same | `EXECUTION_ERROR` |
| Script-change consent (`InstallsScript` / `LoadsProject`) | same, `m_scriptInstall` | `CONSENT_REQUIRED` / `EXECUTION_ERROR` (denied) |
| Device-write consent (`WritesDevice`) | same, `m_deviceWrite` | `CONSENT_REQUIRED` / `EXECUTION_ERROR` |
| UI-drive consent (`DrivesUi`) | same, `m_uiDrive` | `CONSENT_REQUIRED` / `EXECUTION_ERROR` |
| Path policy (`rejectDisallowedPaths`) | `CommandRegistry::execute` | `PATH_NOT_ALLOWED` |
| Launch consent (`IO::authorizeLaunch` via `ProcessLauncher` / project load) | inside the handler | the handler's code |
| License: `IO::EntitlementGate` on connect (spec 0094), `Core::License::activated()` checks in Pro handlers (e.g. `DataTablesHandler`), Pro-feature refusals | inside the handler | `OPERATION_FAILED` and others |

`DispatchPolicyScope` (the assistant's tiers) is shadowed by `RemoteDispatchScope` and never
applies to an MCP call. `remoteProgramChanged`, the consent prompts and the license prompts'
suppression under `RemoteDispatchScope::active()` behave as before.

**One new refusal, named here because it narrows what a socket may do.** An MCP session can
no longer subscribe to mirror or stream pushes, whether through `tools/call` or through a legacy
line on the same socket. Failure modes:

- A client that mixed an MCP session and a mirror viewer on one socket loses the mirror. That
  mix is unsupported, and the documented fix is a second socket.
- A client that subscribed first and sent JSON-RPC later loses its subscription when it sends
  JSON-RPC.

Neither failure mode touches authentication, consent or licensing.

**R2's failure modes:**

- A scripted MCP client that tests only for `"error" in response` now reads a failure as a
  success. Mitigations: the Changelog entry, the API reference, `client.py`, and the
  claude-plugin follow-up.
- `CONSENT_REQUIRED` stays visible to the model through `error.code` in the second text block,
  so retry-after-prompt still works.

## Hotpath & threading impact

- **Touches the hotpath?** No. `FrameReader`, `CircularBuffer`, `FrameBuilder`, the `Dashboard`
  draw and the span lane are not edited. `Server::ingestBlock`, the pipeline-thread entry
  (`enqueueData` plus the stream-subscriber atomic), is not edited. `ServerWorker` is an
  asynchronous sink consumer on its own thread, downstream of `BlockPublisher`. Its lanes gain
  one `QSet::contains` per socket per write, which only removes writes. `--benchmark-hotpath` is
  not required, and neither is `ss-hotpath`.
- **New cross-thread signal/slot?** No new connection. There is one new
  `QMetaObject::invokeMethod(..., Qt::QueuedConnection)` from the GUI thread to the API worker,
  at most once per connection: the existing GUI→worker command-rate pattern of
  `setSocketStreamFrames`, never per frame.
- **New input to a cached hotpath flag?** No. `sinkActive()` is unchanged (decision 8), and
  `m_anyStreamSubscriber` is re-derived through the existing `refreshStreamSubscriberFlag()`
  when the entry teardown drops a stream subscription.
- **Timestamp ownership:** untouched. Nothing is stamped or re-stamped.

## Data model & persistence

None. No `Keys::`, project JSON, QSettings key, Historian schema or `Frame.h` change.
`ConnectionState` and `MCPHandler::ClientSession` are runtime-only.

## API / SDK surface

- **No new or renamed command.** `command_safety.json`, `CommandCapabilities`, `SealedPolicy`,
  the generated schema and SDK and `api-schema.json` are unchanged. The five connection verbs
  stay unregistered and unlisted in `tools/list`.
- **MCP wire changes** (MCP sessions only): `isError` tool results (R2); a `-32602` for a
  non-object `arguments`; `capabilities.resources` is `{}`; `resources/subscribe` and
  `resources/unsubscribe` answer `-32601` (R3); a push-verb tool error (R1); no broadcast lines
  after the first JSON-RPC message (R1).
- **Legacy wire:** unchanged, except that a connection-scoped push verb sent as a legacy line
  on a connection that already sent JSON-RPC gets an `EXECUTION_ERROR`. No new `ErrorCode`
  constant.
- gRPC: untouched.

## QML / UI

None. The change is headless.

## Tradeoffs & alternatives considered

Named candidates for the overall shape, sketched before converging:

- **A, "Mute at the door" (chosen).** Mark on the first MCP message, before dispatch. The
  worker skips the three broadcast lanes, and the opt-in lanes are refused at subscribe time.
- **B, "Reuse the mirror mute".** Call `setStreamFrames(false)`. It covers P1 only and leaks
  P2 and P3. It also breaks `mirror.md`'s invariant that only `mirror.subscribe` flips the flag.
- **C, "Filter at the bridge only".** This drops the constraint that the server must fix the
  problem. The constraint binds: `API-Reference.md` documents MCP clients that connect to port
  7777 directly, and AC1 tests the socket. C is kept as a complement for the
  connect-to-initialize window (Q2).
- **D, "Broadcast on opt-in".** This drops the constraint that plain clients see no change.
  The constraint binds: the "every client" push is documented behavior, and `nc` listeners never
  send a message.

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| 1. Mute mechanism | (a) new `mcpSession` flag plus worker `m_mcpSockets`; (b) reuse `streamFrames`/`m_mutedSockets`; (c) also widen `m_mutedSockets` to P2/P3 | **(a)**. (b) leaks raw bytes and events. (c) would change what mirror viewers receive, which the spec's non-goals exclude. |
| 2. What marks a session (Q1) | (a) the first message routed to MCP; (b) only a successful `initialize` | **(a)**. It is a superset of "from the moment it sends `initialize`", needs no second JSON parse, and sets the flag before dispatch, so nothing reads `state` after a dispatch. `isMCPMessage` already makes such a socket an MCP speaker. |
| 3. Where opt-in lanes close | (a) refuse at subscribe and tear down on entry (GUI thread); (b) drop in `writeMirrorPayload`/`writeStreamBlock` | **(a)**. A worker-side drop would leave a phantom subscriber holding `m_anyStreamSubscriber` true, so stream workers would keep building payloads for nobody, and stream acks would spin. |
| 4. A subscription held before the first MCP message | tear down / keep | **Tear down**. R1 says "from the moment ... pushes nothing", and keeping it would deliver mirror or stream lines on an MCP session. |
| 5. `ReceptionHost` hook | (a) pure virtual `enterMcpSession`; (b) virtual with a no-op default; (c) look up the socket by session id inside `dispatchMcp` | **(a)**. A no-op default hides a missing mute. (c) scans the table on every MCP message and buries the decision in a dispatcher. Cost of (a): three test stubs get one override each. |
| 6. `isError` content layout | (a) a plain-message block plus a JSON diagnostics block; (b) one JSON block; (c) `structuredContent` | **(a)**. A host that shows one block shows the readable message, and the model also gets the structured detail. (c) is defined only from 2025-06-18 and is tied to `outputSchema`, which no tool declares. |
| 7. Subscription refusal code | `-32601` / `-32602` | **`-32601` for both subscribe and unsubscribe**. The method is not offered once the capability is gone. The message names the read alternatives. |
| 8. `sinkActive()` ignoring MCP-only clients | change / leave | **Leave**. Changing it would add a new input to a pipeline-thread flag. The existing empty-target early-out in `processItems` (and the new one in `writeRawData`) already skips serialization. |
| 9. P3 lifecycle events on MCP | mute / translate to MCP notifications | **Mute**. No MCP notification means "device connected", and translating would be new scope. |
| 10. Rejected: a C++ unit test for `toolErrorResult` | add / skip | **Skip**. `MCPHandler` drags in the registry, `Server` and the handler context, and the pytest AC3 asserts the wire directly. The added lines are not worth it (maintainer's rule). |

## Risks & mitigations

- **A pre-mute broadcast reaches the bridge before `initialize` is answered.** These lines
  precede the first reply on the wire. Mitigation: the bridge filter (Q2). Direct TCP clients
  skip non-JSON-RPC lines, which `client.py` will show.
- **Dangling state after a dispatch** (`common-mistakes`, spec 0075 I1). The flag and the hook
  run before the dispatch. The new `tst_client_reception` case runs under the sanitizer build
  like its neighbors.
- **A reused socket pointer gets muted.** `setSocketMcpSession` checks the session id, and
  `addSocket` and `removeSocket` clear the set the way they clear `m_mutedSockets`.
- **The over-cap counter is inflated by MCP skips.** The `m_mcpSockets` check is ordered before
  `underWriteCap`, and the ctest case asserts `droppedBroadcasts` stays 0.
- **Function and TU limits.** `handleToolsCall` would pass 80 lines inline, so the error
  wrapper and the push-verb answer become private helpers. Projected sizes: `Server.cpp` about
  1190 lines, `MCPHandler.cpp` about 620, `ServerWorker.cpp` about 535. All stay under 1500.
- **Doc drift.** `Data-Flow.md` line 87 and `API-Reference.md` line 621 currently state the
  pre-change behavior, and both are in the file list. `documentation-verify.py`,
  `term-verify.py` and `claim-verify.py` (for the `mirror.md` symbols) gate the docs.
- **Out-of-repo copy drift.** The claude-plugin `bridge.py` must stay byte-identical, and that
  is named as a maintainer follow-up.
- **Out of scope:** broadcasts reaching an unauthenticated external socket are fixed separately
  (main session, its own change); no task here touches it.

## Test & verification plan

- **Premise first (runtime confirmation).** Write AC1 before any C++ change and run it against
  the current build. It must fail, with frame, raw-data or event lines on the MCP socket. Then
  implement.
- **Unit (agent-runnable):**
  - `pytest tests/unit/test_mcp_bridge.py`: the existing bounded-exit case, plus the Q2
    line-filter case. No app needed.
  - `pytest tests/scripts/`: unchanged, run as a regression.
- **C++ ctest** (the maintainer builds; the agent may run `ctest` against an existing build dir):
  - `tst_client_reception` gains `firstMcpMessageEntersTheSessionOnce`.
  - `tst_server_worker_caps` gains `mcpSocketsReceiveRepliesOnly`.
  - `tst_server_auth` changes for compilation only.
  - `fuzz_api_json` must still build.
- **Integration (app up, API server on):** `pytest tests/integration/test_mcp.py -v`.

  | AC | Test | Asserts |
  |----|------|---------|
  | AC1 | `TestSessionHygiene::test_mcp_session_receives_only_jsonrpc_while_streaming` (`api_client`, `clean_state`, `device_simulator`; setup copied from `test_block_lane_sinks._capture`) | MCP socket: `initialize` first, then the device is connected so the lifecycle event fires inside the window. Frames stream for 5 s (Quick Plot, `1,2,3\n`), with a `ping` every second. Every MCP line after the `initialize` reply parses and has `jsonrpc == "2.0"`. A plain socket opened alongside receives `"frames"` and `"data"` lines. |
  | AC2 | `test_push_verbs_answer_with_a_tool_error` | `tools/call` of `stream.subscribe` and of `mirror.subscribe` each return a result with `isError: true` whose text names `serialstudio://frame/current`. Then 5 s of streaming: JSON-RPC only. |
  | AC2 (legacy line) | `test_legacy_push_verb_on_an_mcp_session_is_refused` | A `{"type":"command","command":"stream.subscribe"}` line after `initialize` gets `success: false`, `EXECUTION_ERROR`, and no `streamBlock` line follows within 2 s. |
  | AC3 | rewritten `TestToolsCallErrors` + `TestOmittedTools::test_control_script_only_call_meets_the_gate` + new `test_wrong_type_names_parameter_and_type` + new `test_non_object_arguments_is_a_protocol_error` | Failures are `result.isError is True` with no `error` key. The baud-rate string call's text contains `baudRate` and `string`. The remote-gate refusal (`system.runningProcesses`) has the same shape, with the tool name in the diagnostics. Unknown tool, missing name, uninitialized session and non-object `arguments` still raise `MCPError`. |
  | AC4 | `test_initialize_returns_capabilities` (extended), `test_subscribe_current_frame` / `test_unsubscribe_current_frame` (rewritten) | `"subscribe" not in caps["resources"]`. Both methods raise `MCPError` with code `-32601`. |
  | AC5 | full `test_mcp.py`; `pytest tests/ -m "not destructive" -v`; `python3 scripts/documentation-verify.py`, `term-verify.py`, `claim-verify.py` | Green. The only changed `test_mcp.py` expectations are the ones listed below. |

  **Existing tests whose expectations change** (all in `tests/integration/test_mcp.py`):
  - R2 (`MCPError` becomes `isError` result), in `TestToolsCallErrors`:
    `test_set_baud_rate_missing_param`, `test_set_baud_rate_wrong_type`,
    `test_set_port_index_negative`, `test_set_tcp_port_out_of_range`, `test_set_tcp_port_zero`,
    `test_set_bus_type_invalid`, `test_project_file_open_missing_param`,
    `test_project_set_title_empty`, `test_connect_without_valid_config_fails`,
    `test_disconnect_when_not_connected_fails`, `test_write_data_when_not_connected_fails`,
    `test_network_address_empty_fails`.
  - R2, in `TestOmittedTools`: `test_control_script_only_call_meets_the_gate`. It currently
    reads `exc.data["tool"]`; it will read the tool from the diagnostics block.
  - R3: `TestMCPResources::test_subscribe_current_frame` and `test_unsubscribe_current_frame`
    flip from success to `-32601`. `test_subscribe_missing_uri_fails` still passes; tighten it
    to `-32601`. `TestMCPLifecycle::test_initialize_returns_capabilities` gains the AC4
    assertion.
  - Unchanged and still correct: `test_unknown_tool_returns_error`, `test_missing_tool_name`,
    `test_methods_require_initialize`, every batch test, and
    `tests/integration/test_api_surfaces.py`. Its `tools_list()` marks that function-scoped
    `api_client` as an MCP session, which mutes broadcasts only, and its tests read replies only.

  The C++ test stubs (`StubHost`, `TableHost`, `FuzzHost`) change for compilation only. No
  assertion in them changes.
- **Hotpath:** not touched, so no `--benchmark-hotpath` run.
- **Static:** `python3 scripts/syntax-check.py` on every edited C++ file (the four `core/` pairs,
  `tst_client_reception.cpp`, `tst_server_worker_caps.cpp`, `tst_server_auth.cpp`,
  `fuzz_api_json.cpp`); `python3 scripts/code-verify.py --check` on the same; `qt-cpp-review`
  before handoff; `python3 scripts/sanitize-commit.py` before commit. Doc edits go through the
  `ss-docs` skill.

## Open questions (decided 2026-11-03 by the maintainer: all five as recommended)

- **Q1. Mark on the first MCP-routed message instead of strictly on `initialize`?**
  Recommended: yes. It is a superset of the spec's trigger, sets the flag before any dispatch,
  and needs no second parse. No legacy message can trip it.
- **Q2. Should the bridge drop lines that are not JSON-RPC?** These are the broadcasts that
  arrive between its TCP connect and the host's `initialize`. Recommended: yes. R1 cannot cover
  that interval without breaking plain listeners. This is a content filter, not a framing
  change, and it must be mirrored to `Serial-Studio/claude-plugin`'s `bridge.py`.
- **Q3. Refuse the connection-scoped push verbs sent as legacy lines on an MCP session?**
  Recommended: yes. R1 says "pushes nothing", and the legacy line is the only path that
  attaches a push to an MCP socket today.
- **Q4. Reshape the other legacy-shaped lines an MCP socket can receive?** These are the
  token-handshake replies, the rate-limit, size and flood notices sent before a disconnect, and
  replies to legacy commands sent on the same socket. Recommended: no, out of scope. Each one
  answers something the client sent or ends the session, and none is a push.
- **Q5. Report `receivedType` as the JSON type name (`string`) instead of Qt's `QString`?**
  Recommended: yes. The model reads JSON, and AC3 asserts "the received type", which a Qt class
  name serves poorly.
