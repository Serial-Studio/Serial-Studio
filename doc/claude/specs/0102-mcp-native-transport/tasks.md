---
spec: 0102-mcp-native-transport
phase: tasks
status: approved     # 2026-11-03: reviewed and approved by the main session under the maintainer's delegation (plans: maintainer, tasks: main session)
updated: 2026-11-03
---

# Tasks 0102 — MCP Session Hygiene

> **Phase 3 of 4 — the ordered checklist.** Decompose [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable* — each one a coherent diff a reviewer
> could read in isolation. `/ss-implement` works this list top to bottom and keeps the status
> boxes current. Gate: do not start `/ss-implement` until a human marks this `approved`.

## Conventions

- One task = one focused, reviewable change. If a task touches >3 files or needs a paragraph
  to describe, split it. T4 is the one exception, and it says why.
- **Verify** is how *this* unit is confirmed before moving on. For every C++ file it means
  `python3 scripts/syntax-check.py <file>` plus `python3 scripts/code-verify.py --check <files>`;
  the task's own check follows. A syntax-check SKIP (stale compile database) is reported as
  "unverified", never as "compiles".
- **Deps** lists task IDs that must land first.
- **Executor** marks a task as mechanical and delegable to a cheaper model working against the
  donor it names (CLAUDE.md "Delegation & Cost"). The donor line exists for every task, so the
  output matches a file already in the tree. A task marked `main session` stays with the
  planner, because it reshapes a refusal, touches the reception machine, or is the manual the
  maintainer reads first.
- **Commit shape.** `plan.md` and this file land in their own `docs(spec):` commit before T1.
  After that the commits go in order: T1 (the premise test, alone), T2 and T3 (worker lane),
  T4 and T5 (session entry), T6 to T8 (refusals and the wire), T9 (the MCP tests), T10 and T11
  (bridge, example client), T12 and T13 (docs). No task touches a file in `commit-shape.json`'s
  `api-auth` set, and a diff that does is a scope error.
- **The security line, named once.** No gate's verdict changes. T6 adds one new refusal (push
  verbs on an MCP session). T7 changes only the envelope around refusals the gates already
  return. The unauthenticated-broadcast issue is fixed separately and is not in this list.

## Tasks

### T1 — Premise test: an MCP session receives only JSON-RPC while a device streams (AC1)

- **Files:** `tests/integration/test_mcp.py`
- **Does:** Adds `class TestSessionHygiene` with
  `test_mcp_session_receives_only_jsonrpc_while_streaming(api_client, clean_state,
  device_simulator)`. Steps:
  1. Open a fresh `MCPClient` and a plain socket.
  2. `initialize` the MCP client. Its `request()` already skips lines that come before the
     reply.
  3. Switch to Quick Plot, configure TCP 127.0.0.1:9000 and connect, so the `connected`
     lifecycle event fires inside the window.
  4. For 5 s, stream `1,2,3\n` and send a `ping` each second.
  5. Assert that every MCP line after the `initialize` reply parses and has
     `jsonrpc == "2.0"`, and that the plain socket saw both `"frames"` and `"data"` lines.

  The test is written before any C++ change: the spec's premise check requires it to fail on
  master.
- **Donor:** `tests/integration/test_block_lane_sinks.py::_capture` (mode first, then network,
  connect and stream) and `tests/integration/test_sealed_session.py::test_per_frame_broadcast_carries_no_script`
  (raw socket read loop).
- **Verify:** The maintainer runs `pytest tests/integration/test_mcp.py -k TestSessionHygiene -v`
  against the current build and records a **failure** in the handoff, with frame, raw-data or
  event lines on the MCP socket. That failure is the runtime confirmation of the premise.
- **Deps:** none
- **Executor:** yes, Sonnet.
- [x] done

### T2 — Worker: a replies-only socket set that the three broadcast lanes skip

- **Files:** `core/Api/API/Server/ServerWorker.h`, `core/Api/API/Server/ServerWorker.cpp`
- **Does:**
  - Adds public slot `setSocketMcpSession(QTcpSocket*, const QString& sessionId)` and
    `QSet<QTcpSocket*> m_mcpSockets`.
  - `addSocket`, `removeSocket` and `closeResources` clear the set beside `m_mutedSockets`.
  - `processItems`, `writeRawData` and `broadcastEvent` skip its members.
  - `writeRawData` builds its target list before serializing, as `processItems` does, and
    returns early when the list is empty.
  - Binding invariants: the slot drops the call when the session id no longer matches (a
    reused socket pointer is never muted); the `m_mcpSockets` check sits **before**
    `underWriteCap`, so a skip is never counted in `droppedBroadcasts`; `writeToSocket`,
    `writeMirrorPayload` and `writeStreamBlock` are untouched.
- **Donor:** `ServerWorker::setSocketStreamFrames` and `m_mutedSockets` in the same file (the
  slot and set shape), and `processItems` (the target-list early-out).
- **Verify:** syntax-check and code-verify on both files. A read-back confirms each of the three
  lanes tests `m_mcpSockets` first in its condition.
- **Deps:** none
- **Executor:** yes, Sonnet.
- [x] done
  (Main session 2026-11-03: the skip lives in `receivesBroadcasts()`, which the auth fix staged in this
  worktree added; all three lanes call it first, before `underWriteCap`.)

### T3 — ctest: an MCP socket gets replies but no raw bytes or events

- **Files:** `app/tests/tst_server_worker_caps.cpp`
- **Does:** Adds `mcpSocketsReceiveRepliesOnly`. After
  `m_worker->setSocketMcpSession(m_serverSide, "s1")`:
  - `writeRawData` and `broadcastEvent` leave `bytesToWrite()` unchanged and
    `droppedBroadcasts()` at 0;
  - `writeToSocket` still appends its payload;
  - a call carrying a stale session id (`"s0"`) mutes nothing.
- **Donor:** `TstServerWorkerCaps::broadcastsAreSkippedOverTheCap` (same fixture, same
  `bytesToWrite` measurement).
- **Verify:** syntax-check and code-verify on the file. The maintainer runs
  `ctest -R tst_server_worker_caps` after a build.
- **Deps:** T2
- **Executor:** yes, Sonnet.
- [x] done

### T4 — Session entry: the flag, the host hook and the server's teardown

- **Files:** `core/Api/API/Server/ConnectionState.h`, `core/Api/API/Server/ClientReception.h`,
  `core/Api/API/Server/ClientReception.cpp`, `core/Api/API/Server.h`, `core/Api/API/Server.cpp`
- **Does:**
  - `ConnectionState` gains `bool mcpSession = false;` with a one-line comment.
  - `ReceptionHost` gains pure virtual `enterMcpSession(QTcpSocket*, ConnectionState&)`.
  - In `ClientReception::handleJsonMessage`'s MCP branch, before `dispatchMcp`:
    `if (!state->mcpSession) { state->mcpSession = true; m_host.enterMcpSession(socket, *state); }`.
  - `Server::enterMcpSession`:
    1. If `mirrorSubscribed`: unsubscribe from the mirror publisher and clear the flag.
    2. If `streamSubscribed`: clear the flag and `streamPending`, call
       `refreshStreamSubscriberFlag()` and emit `streamSubscribersChanged`.
    3. Queue `setSocketMcpSession(socket, sessionId)` on the worker, in the shape of
       `setStreamFrames`.
  - Why five files: the pure virtual and its only production implementation must land together,
    or the app does not compile.
  - Binding invariants:
    - nothing reads `state` after `dispatchMcp` (spec 0075 I1);
    - the session id rides the queued hop;
    - `streamFrames` is never written here (`mirror.md`: only `mirror.subscribe` flips it);
    - `handleAuthHandshake`, `authorizeRawWrite` and the limits are untouched.
- **Donor:** `Server::setStreamFrames` (queued hop), `Server::teardownConnection` and
  `streamUnsubscribe` (the release sequence), and the `handshakeSeen` assignment in the same
  branch (the per-connection flag).
- **Verify:** syntax-check and code-verify on all five files. A read-back of the MCP branch shows
  the mark before the dispatch and no `state->` after it. The test targets compile only after
  T5.
- **Deps:** T2
- **Executor:** main session (the reception machine hosts the auth handshake; queued wiring).
- [x] done

### T5 — Test hosts implement the hook; reception ctest for the mark

- **Files:** `app/tests/tst_client_reception.cpp`, `app/tests/tst_server_auth.cpp`,
  `app/tests/fuzz/fuzz_api_json.cpp`
- **Does:**
  - `TableHost` overrides `enterMcpSession` and counts calls.
  - `StubHost` overrides it with a counter.
  - `FuzzHost` overrides it with an empty body.
  - New case `firstMcpMessageEntersTheSessionOnce`:
    1. Two pipelined JSON-RPC lines call the hook once and leave `mcpSession` true.
    2. On a fresh connection, a legacy `{"type":"command"}` line never calls it.
  - No existing assertion changes.
- **Donor:** `TstClientReception::rawForwardingWaitsForOneJsonMessage` (same helpers:
  `authenticatedState`, `TableHost::connections`). The override shape follows the existing
  `dispatchMcp` overrides in each host.
- **Verify:** syntax-check and code-verify on the three files. The maintainer runs
  `ctest -R "tst_client_reception|tst_server_auth"` and builds the `fuzz_api_json` target if
  the fuzz tier is configured.
- **Deps:** T4
- **Executor:** yes, Haiku for the three overrides and Sonnet for the new case.
- [x] done

### T6 — Refuse connection-scoped push verbs on an MCP session (legacy line)

- **Files:** `core/Api/API/MCPProtocol.h`, `core/Api/API/Server.h`, `core/Api/API/Server.cpp`
- **Does:**
  - `MCPProtocol.h` gains `inline constexpr QLatin1StringView kPushRefusal` with the plan's
    refusal text. It is the one copy both sides read.
  - `Server` gains a public static `isConnectionCommand(name)` =
    `MirrorCommands::isMirrorCommand(name) || isStreamCommand(name)`.
  - In `routeConnectionCommand`, on `state.mcpSession` a connection command is answered with
    `CommandResponse::makeError(id, ErrorCode::ExecutionError, MCP::kPushRefusal)` and the
    function returns true.
  - Binding invariant: this is a **new refusal** that narrows what an MCP socket may do. It is
    not a change to any gate, and no `api-auth` file is touched.
- **Donor:** `MirrorCommands::isMirrorCommand` (predicate shape) and the existing
  `routeConnectionCommand` branches.
- **Verify:** syntax-check and code-verify on `Server.cpp` and `MCPProtocol.h`. A read-back
  confirms the refusal runs only when `state.mcpSession` is set, so a non-MCP mirror viewer's
  `mirror.subscribe` path is byte-identical.
- **Deps:** T4
- **Executor:** main session (a new refusal on a decision-visible surface).
- [x] done

### T7 — `tools/call` failures become `isError` results; push verbs answer as a tool error

- **Files:** `core/Api/API/MCPHandler.h`, `core/Api/API/MCPHandler.cpp`
- **Does:** `handleToolsCall`, in order:
  1. A non-object `arguments` gets `-32602`.
  2. `Server::isConnectionCommand(name)` gets the private
     `connectionVerbResult(id) → {content:[{text: MCP::kPushRefusal}], isError:true}`.
  3. Missing name and unknown tool keep their `-32602`.
  4. The `RemoteDispatchScope` plus `registry.execute` lines stay unchanged.
  5. A failed response goes through the new private `toolErrorResult(name, arguments,
     response)`, which returns two text blocks (the message, then compact JSON with
     `error{code,message,data}`, `tool`, `providedArguments`, the existing `must be` hint, and
     `result` only when non-empty) and `isError: true`, all inside `makeSuccess`.

  `receivedType` reports the JSON type name (`QJsonValue` type: `string`, `number`, `bool`,
  `object`, `array`, `null`).

  Binding invariant: the scope's lifetime still encloses `execute`, and the verdict always
  comes before the reshaping. The helpers keep `handleToolsCall` under 80 lines.
- **Donor:** the success branch of `handleToolsCall` (content block shape) and the existing
  `errorData` hint block it replaces.
- **Verify:** syntax-check and code-verify on both files. A read-back confirms that the only
  `makeError` calls left in `handleToolsCall` are the four protocol faults, and that
  `RemoteDispatchScope` is constructed on the line directly above `execute`.
- **Deps:** T6
- **Executor:** main session (reshapes every gate's refusal envelope).
- [x] done
  (Main session 2026-11-03: the shaping helpers are file-local statics in `MCPHandler.cpp`, not private
  members; `receivedType` uses the JSON Schema names, so a boolean reads `boolean`.)

### T8 — Stop advertising resource subscription (R3)

- **Files:** `core/Api/API/MCPHandler.h`, `core/Api/API/MCPHandler.cpp`
- **Does:**
  - In `handleInitialize`, `capabilities.resources` becomes `{}`.
  - `resources/subscribe` and `resources/unsubscribe` route to one private
    `refuseSubscription(request)`. It answers `-32601`, saying subscription is not supported
    and naming `resources/read serialstudio://frame/current` plus `dashboard.getData` and
    `dashboard.tailFrames`.
  - Removes `ClientSession::subscribedResources`, `handleResourcesSubscribe` and
    `handleResourcesUnsubscribe`, together with their `m_sessions[sessionId]` inserts. The
    `Method::` constants stay.
- **Donor:** `MCPHandler::handleResourcesRead`'s unknown-resource `makeError` branch.
- **Verify:** syntax-check and code-verify on both files. `grep -n subscribedResources core/`
  returns nothing.
- **Deps:** T7 (same files; sequential to keep the diffs separate)
- **Executor:** yes, Sonnet.
- [x] done

### T9 — MCP integration tests for AC2, AC3 and AC4; rewrite the error-shape tests

- **Files:** `tests/integration/test_mcp.py`
- **Does:**
  - `MCPClient` gains `tool_error(name, args)`, which asserts `isError` and returns the joined
    text.
  - New in `TestSessionHygiene`:
    - `test_push_verbs_answer_with_a_tool_error`: `stream.subscribe` and `mirror.subscribe`,
      then 5 s of streaming with JSON-RPC only.
    - `test_legacy_push_verb_on_an_mcp_session_is_refused`: `EXECUTION_ERROR`, and no
      `streamBlock` line within 2 s.
  - Rewritten to expect an `isError` result:
    - the 12 `TestToolsCallErrors` failure cases named in the plan;
    - `TestOmittedTools::test_control_script_only_call_meets_the_gate`, which reads the tool
      name from the diagnostics block.
  - New:
    - `test_wrong_type_names_parameter_and_type` (`baudRate` and `string` in the text);
    - `test_non_object_arguments_is_a_protocol_error`.
  - `test_initialize_returns_capabilities` gains `"subscribe" not in caps["resources"]`.
  - `test_subscribe_current_frame` and `test_unsubscribe_current_frame` expect `-32601`, and
    `test_subscribe_missing_uri_fails` is tightened to `-32601`.
  - `test_unknown_tool_returns_error`, `test_missing_tool_name` and
    `test_methods_require_initialize` are left as they are.
- **Donor:** the existing classes in the same file, and T1's streaming setup for the AC2
  window.
- **Verify:** `python3 -m pytest tests/integration/test_mcp.py --collect-only -q` collects
  without error, which the agent can run without the app. The maintainer runs the full file
  against a build with T2 to T8.
- **Deps:** T1 (shared setup); T7, T8 for the run to pass
- **Executor:** yes, Sonnet.
- [x] done

### T10 — Bridge: drop non-JSON-RPC lines; banner without a command count

- **Files:** `examples/MCP Client/claude_desktop_bridge.py`, `tests/unit/test_mcp_bridge.py`
- **Does:**
  - In `tcp_to_stdout`, a line is forwarded only if it parses as a JSON array, or as an object
    with `"jsonrpc": "2.0"`. Dropped lines are counted, and the count is logged once to stderr.
  - The banner line `Available tools: 182+ commands` becomes `Tools: every listed API command
    (tools/list)`.
  - Framing stays newline-delimited.
  - New unit test `test_bridge_forwards_only_jsonrpc_lines`: a local `socket` server on an
    ephemeral port, reached through `SERIAL_STUDIO_API_PORT`, sends a `{"frames":[]}` line, a
    `{"data":"AA=="}` line and a JSON-RPC response. The bridge's stdout must carry exactly the
    JSON-RPC line. Closing stdin and the server ends the run.
- **Donor:** `tests/unit/test_mcp_bridge.py::test_bridge_exits_with_the_fix_when_nothing_listens`
  (spawn with `-I`, environment override, timeout).
- **Verify:** `python3 -m pytest tests/unit/test_mcp_bridge.py -v`, which the agent can run
  without the app.
- **Deps:** none
- **Executor:** yes, Sonnet.
- [x] done

### T11 — Example client and its README follow the session rules

- **Files:** `examples/MCP Client/client.py`, `examples/MCP Client/README.md`
- **Does:**
  - `client.py`: `call_tool` treats `result.get("isError")` as a failure and prints its text;
    `_read_response` skips every line without a `jsonrpc` key.
  - README:
    - removes line 175 ("Resources support **subscription**") and the `resources/subscribe`
      bullet at line 517;
    - rewrites the line 379 prompt and the anomaly example at lines 540-545 to poll
      `resources/read`;
    - adds one paragraph: failures arrive as `isError` tool results, and an MCP connection
      receives replies only.
- **Donor:** the existing `read_resource` / `call_tool` methods (client); the README's own
  "Specification Compliance" list (docs).
- **Verify:** `python3 -I -m py_compile "examples/MCP Client/client.py"`;
  `python3 scripts/documentation-verify.py --no-report "examples/MCP Client/README.md"`;
  `python3 scripts/term-verify.py`.
- **Deps:** T7, T8 (documents their behavior)
- **Executor:** yes, Sonnet, with the `ss-docs` skill for the README.
- [x] done

### T12 — Manual: session rules, error shape, resources

- **Files:** `doc/help/API-Reference.md`, `doc/help/Data-Flow.md`
- **Does:**
  - In API-Reference's MCP section, adds "Session rules", "Errors" and "Resources"
    subsections:
    - Session rules: the first JSON-RPC message makes a connection an MCP session, which then
      receives replies only. Telemetry is read with `resources/read` or by polling
      `dashboard.getData` / `dashboard.tailFrames`. Push commands are refused.
    - Errors: an `isError` tool result versus the list of protocol faults.
    - Resources: no subscription.
  - Line 621's comment becomes "go to every client except an MCP session".
  - A Changelog entry is added under the in-progress release.
  - `Data-Flow.md` line 87: legacy clients receive the broadcast, and an MCP session reads
    frames on request.
- **Donor:** the existing "How It Works" and "Getting Started with MCP" subsections (voice and
  length).
- **Verify:** `python3 scripts/documentation-verify.py --no-report doc/help/API-Reference.md
  doc/help/Data-Flow.md`; `python3 scripts/term-verify.py`.
- **Deps:** T7, T8
- **Executor:** main session with the `ss-docs` skill (the page the maintainer reads first).
- [x] done

### T13 — AI-facing doc and test catalog

- **Files:** `doc/claude/architecture/mirror.md`, `tests/README.md`
- **Does:**
  - `mirror.md`: after the `streamFrames` bullet, one sentence: an MCP session is muted
    through `ConnectionState::mcpSession` and the worker's `m_mcpSockets`, never through
    `streamFrames`.
  - `tests/README.md`:
    - the `test_mcp.py` rows (lines 84 and 513) gain session hygiene and the error shape;
    - the `test_mcp_bridge.py` rows (lines 232 and 546) gain the line filter;
    - the C++ catalog gains the two new ctest cases, if it lists them.
- **Donor:** the adjacent bullets and rows in each file.
- **Verify:** `python3 scripts/claim-verify.py` (it resolves `mcpSession` and `m_mcpSockets`
  in the tree); `python3 scripts/documentation-verify.py --no-report doc/claude/architecture/mirror.md
  tests/README.md`.
- **Deps:** T2, T4, T9, T10
- **Executor:** yes, Haiku.
- [x] done

### T14 — Maintainer: build, run, and mirror the plugin bridge

- **Files:** none in this repository
- **Does:**
  1. Rebuild.
  2. `ctest -R "tst_server_worker_caps|tst_client_reception|tst_server_auth"`.
  3. With the app up and the API server on: `pytest tests/integration/test_mcp.py -v` (AC1 to
     AC5), then `pytest tests/ -m "not destructive" -v` (AC5).
  4. **Copy `examples/MCP Client/claude_desktop_bridge.py` to `Serial-Studio/claude-plugin` as
     `bridge.py`, byte-identical (`cmp` exits 0).** Read that repository's bundled skill and
     README for any text that treats a JSON-RPC error as the failure signal or promises
     resource subscription, and update it there.
- **Verify:** every command above is green; `cmp` exits 0; the plugin repository's own history
  carries the copy.
- **Deps:** T1 to T13
- **Executor:** maintainer.
- [ ] done

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` is met and checked off there:
  - AC1 by T1 (failing first), then T2 and T4;
  - AC2 by T6, T7 and T9;
  - AC3 by T7 and T9;
  - AC4 by T8 and T9;
  - AC5 by T11 to T14.
- [ ] `python3 scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [ ] `python3 scripts/syntax-check.py` ran on every edited C++ file; any SKIP is named in the
  handoff as unverified.
- [ ] `qt-cpp-review` run on the C++ diff (T2 to T8); findings addressed or noted.
- [ ] Hotpath untouched by construction: no pipeline file, no cached flag, no per-frame hop.
  `--benchmark-hotpath` is not required, and the handoff says so.
- [ ] Maintainer-run checks identified (T14). The agent-runnable ones ran:
  `pytest tests/unit/test_mcp_bridge.py`, the `test_mcp.py` collect-only, and
  `pytest tests/scripts/`.
- [ ] `python3 scripts/sanitize-commit.py` run before each commit; `layer-verify.py` and the
  singleton census unchanged.
- [ ] Diff is *what was asked, and only that*: no `api-auth` file, no handler, no
  `command_safety.json`, no generated surface touched, and the unauthenticated-broadcast fix
  is not in this diff.
- [ ] `spec.md` status set to `done`.
