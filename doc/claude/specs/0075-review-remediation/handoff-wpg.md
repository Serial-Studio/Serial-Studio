# WP-G handoff — API surface (WPG-T1 .. WPG-T8)

All eight tasks are done and ticked in `tasks.md`. `python3 scripts/code-verify.py --check` is
clean (0 errors, 0 advisories) on every changed file; `registry-verify.py` and
`claim-verify.py --quiet` are clean; the new pytest files collect under `--strict-markers`.
Nothing was compiled, launched or committed.

## Files changed

| Path | Change |
|------|--------|
| `app/src/API/Server/ClientReception.h` | `consumeBytes(socket, sessionId, data)`; `ReceptionHost::stateFor()`; `authorizeDeviceWrite()` returns `DeviceWriteVerdict`; new `drainBuffer`/`reportBufferFlood`/`rejectHttpPreamble`/`authorizeRawWrite`/`looksLikeHttpRequest`; `processJsonLine` dropped. |
| `app/src/API/Server/ClientReception.cpp` | Loop re-resolves the connection every iteration and after every dispatch (I1); HTTP request-line sniff closes with no readable answer (I2); `handshakeSeen` gates raw forwarding (I2); handshake no longer re-enters `consumeBytes`, so bytes count once (I12); buffered JSON is consumed before dispatch; consent refusals go through `refusedWriteResponse`. |
| `app/src/API/Server/ConnectionState.h` | `handshakeSeen`, `firstBytesSeen`. |
| `app/src/API/Server/DeviceWriteVerdict.h` (new) | `enum class DeviceWriteVerdict {Allowed, Denied, ConsentRequired}`, in its own header so a stub host links neither ServerAuth nor QtWidgets. |
| `app/src/API/Server/ServerAuth.{h,cpp}` | `authorizeDeviceWrite()` never blocks: unset consent posts `showDeviceWriteConsentPrompt` queued and returns `ConsentRequired` (I1/T2); `authorizeRemoteCommand` keeps its bool contract (`Allowed` only). |
| `app/src/API/Server.{h,cpp}` | `stateFor()` override; `onDataReceived` no longer hands out a reference; two loopback listeners (v4 + v6) or one dual-stack `Any` when external (I10); `API/Port` setting + `port`/`setPort` Q_PROPERTY; `startListening`/`stopListening`/`dropConnections` replace the two duplicated listen blocks; `acceptConnection` resolves its sender; header section order fixed (I14). |
| `app/src/API/Server/ServerWorker.{h,cpp}` | `exceedsWriteCap`/`maxPendingWriteBytes` as the single cap decision; `broadcastEvent` now skips over-cap sockets; `writeToSocket` drops a backlogged client (`dropBackloggedClient`, counted) (I6); `droppedBroadcasts()`/`backlogDisconnects()` accessors. |
| `app/src/API/PathPolicy.{h,cpp}` | `PathParamPolicy{name, allowMissing}` + `declaredPathParams()`; the 13-command declaration table (I3/I7). |
| `app/src/API/CommandRegistry.{h,cpp}` | `CommandDefinition::pathParams`, filled at registration; `rejectDisallowedPaths` enforces every declared param once in `execute()` with `PATH_NOT_ALLOWED`; the four-name `commandUsesFilePathPolicy` hack removed. |
| `app/src/API/Handlers/{CSVPlayer,MDF4Player,ProjectFileCommands,OpcUa,Process,Sessions}Handler.cpp` | Ad hoc `isPathAllowed` checks and now-unused includes removed; the registry owns the gate. |
| `app/src/API/CommandProtocol.h` | `ErrorCode::{ConsentRequired,PathNotAllowed,WriteBacklog,WriteDenied,WriteFailed}`; `Limits::kMaxApiRawBytes` shared by TCP and gRPC. |
| `app/src/API/GRPC/PendingCall.h` (new) | The abortable marshal (mutex + condvar + `abandon()`), header-only and grpc-free so it is unit-testable. |
| `app/src/API/GRPC/GRPCServer.{h,cpp}` | `marshalToGui()` replaces both `BlockingQueuedConnection`s; `stopServer()` abandons parked calls before `Shutdown` (I5); peer parsed with `QHostAddress` (I8); `WriteRawData` enforces `kMaxApiRawBytes`; `WRITE_DENIED`/`WRITE_FAILED` come from the shared enum (I9). |
| `app/qml/Dialogs/Settings/SettingsGeneralPage.qml` | Port SpinBox beside the enable toggle; the label no longer hard-codes 7777. |
| `scripts/registry-verify.py`, `scripts/mirror-wire.json` (new) | `check_mirror_wire`: digests `MirrorProtocol.h` + `wireUniqueId()` + `rebuildStructure()` against a checked-in baseline; a change without a `kWireVersion` bump fails, `--accept-mirror-wire` re-seeds (I13). |
| `app/CMakeLists.txt`, `app/tests/CMakeLists.txt` | New headers registered; four suites + the fuzz target added. |

## Tests added

| Path | Covers |
|------|--------|
| `app/tests/tst_client_reception.cpp` (new) | Host stub with a real `QHash` table: entry erased mid-dispatch, table rehashed mid-dispatch (64 late connections), single-count byte accounting, HTTP sniff (+ data-driven method table), raw-forward handshake gate, `CONSENT_REQUIRED` refusal. |
| `app/tests/tst_path_policy_registry.cpp` (new) | Sweeps every command in `app/rcc/api/api-schema.json`: a path-shaped parameter with no declaration fails. Reads the snapshot via `SS_API_SCHEMA_PATH`. |
| `app/tests/tst_server_worker_caps.cpp` (new) | Real loopback pair, no event loop: cap boundary, response under cap, broadcast/mirror skipped over cap, response lane drops the client. |
| `app/tests/tst_grpc_pending_call.cpp` (new) | dispatch/wait/abandon/timeout and the four-handlers-parked shutdown shape. |
| `app/tests/fuzz/fuzz_api_json.cpp` + 12 corpus seeds (new) | `consumeBytes` on an authenticated and on an unauthenticated connection, whole and split. |
| `app/tests/tst_server_auth.cpp` | Adapted to the new host interface (stub publishes its state, session-id addressing). |
| `tests/security/test_http_on_api_socket.py` (new) | Every HTTP verb closes with no readable answer; the body command never runs; raw lane gated; normal clients unaffected. |
| `tests/security/test_path_policy_all_commands.py` (new) | 12 guarded command/parameter pairs answer `PATH_NOT_ALLOWED`; `openDatabase` creates no `-wal`/`-shm` sibling; an allowed temp file reaches the handler; traversal out of a root is refused. |
| `tests/security/test_write_backlog.py` (new) | A non-reading client is dropped; a healthy peer keeps working. |
| `tests/integration/test_api_ipv6.py` (new) | `::1` is served, whatever `localhost` resolves to is served, IPv4 still works. |
| `tests/integration/test_grpc_lifecycle.py` (new) | Port follows the API state, garbage is survivable, churn does not wedge the GUI-served JSON API. Skips without a gRPC build. |
| `tests/security/test_unknown_input_hardening.py` | The 10-thread stress `xfail` is now a plain assertion. |

## Tasks not done

None. Two deliverables land as patches below because their file belongs to another package.

## Patches for the coordinator

### 1. `app/src/Misc/CLI.cpp` (WP-H's file) — `--api-port`

`app/src/Misc/CLI.h`, after the existing `apiTokenOpt`-style options:

```cpp
  QCommandLineOption apiPortOpt{
    "api-port",
    QObject::tr("Listen for API clients on <port> instead of 7777."),
    "port"};
```

`CLI.cpp`, next to `m_parser.addOption(m_opts.dumpApiSchemaOpt);`:

```cpp
  m_parser.addOption(m_opts.apiPortOpt);
```

and in the same block that applies `--api-external` / `--api-token` (after the composition root
exists, before `restoreLastProject()`):

```cpp
  if (m_parser.isSet(m_opts.apiPortOpt)) {
    bool ok            = false;
    const int port     = m_parser.value(m_opts.apiPortOpt).toInt(&ok);
    static auto& server = API::Server::instance();
    if (!ok || port < 1 || port > 65535) {
      qWarning() << "[CLI] --api-port needs a port between 1 and 65535; ignoring"
                 << m_parser.value(m_opts.apiPortOpt);
    } else
      server.setPort(port);
  }
```

`API::Server::setPort()` persists `API/Port` and rebinds a listening server, so the flag works
whether or not the API is already enabled.

### 2. `app/src/Misc/CLI.cpp` (WP-H's file) — project `x-pathParams` into `api-schema.json`

In `CLI::dumpApiSchema()`, inside the per-command loop, after the `required` insert:

```cpp
    if (!def.pathParams.isEmpty()) {
      QJsonArray pathParams;
      for (const auto& policy : def.pathParams) {
        QJsonObject entry;
        entry.insert(QStringLiteral("name"), policy.name);
        entry.insert(QStringLiteral("allowMissing"), policy.allowMissing);
        pathParams.append(entry);
      }

      entry.insert(QStringLiteral("x-pathParams"), pathParams);
    }
```

(`entry` is the existing per-command object; rename the inner variable if it shadows.) The
snapshot itself needs a maintainer build (`SerialStudio --dump-api-schema app/rcc/api/api-schema.json`)
— `generate-property-registry.py --check-snapshot` only compares the `project.dataset.update`
`properties` slice, so it stays green either way, and `tst_path_policy_registry` reads the
declaration from C++ rather than from the snapshot, so nothing is blocked on that rebuild.

### 3. `REUSE.toml` (shared) — cover the fuzz corpora

The corpus seeds are raw bytes with no place for an SPDX header. Add to the first-party
tooling block (the one that already lists `tests/**`):

```toml
  "app/tests/fuzz/corpus/**",
```

WP-A, WP-D and WP-H add corpora under the same root, so one entry covers all of them.

### 4. `app/src/API/CommandHandler.cpp` and `app/src/API/MCPHandler.cpp` (unowned) — optional

Both refuse a consent-gated command with `ErrorCode::ExecutionError` / `"Device write denied by
user"`. Now that an unanswered consent is a *retryable* refusal, these two sites should say so:

```cpp
    if (!server.authorizeRemoteCommand(request.command)) {
      return CommandResponse::makeError(
        request.id, ErrorCode::ConsentRequired,
        QStringLiteral("Device writes need the user's consent; a prompt was shown, retry after "
                       "it is answered"));
    }
```

I left both files untouched (out of my task's file list); the current text is still accurate for
a genuine denial, only imprecise for a pending one.

## Invariants found that the plan did not state

1. **`system.exec` must stay out of the path-policy table.** It is control-script only
   (`commandIsControlScriptOnly`) and its `program`/`workingDir` are resolved from `PATH` and the
   project directory — declaring them would break every control script that launches `python`.
   `tst_path_policy_registry` encodes the exemption with that reason, next to the three other
   path-shaped-but-not-a-path parameters (`askPath`, the dataset-address `path` of
   `project.dataset.getByPath` / `assistant.dataset.resolve`, and the property-name `key` of the
   `setProperty` verbs — `io.opcua.setUserCertificate.key` *is* a real path and is declared).
2. **The two outbound lanes need different verdicts, not one cap.** Broadcast/mirror/stream are
   producer-paced, so skipping is correct and self-healing (the existing documented behaviour).
   A command response answers something the client asked for: skipping it wedges that client
   forever, so the response lane disconnects instead. Applying one rule to both would either
   re-introduce unbounded growth or start dropping responses silently.
3. **A by-reference functor across a thread marshal is only safe if every waiter abandons before
   its frame dies.** `PendingCall::abandon()` blocks behind an in-flight `dispatch()` (the
   `MarshalCall` property), so `marshalToGui()` abandons on *both* the success and the timeout
   path, not only on failure.
4. **`ClientReception` had a second post-dispatch use of the connection state** the plan did not
   name: `processBufferedJson` cleared `state.buffer` *after* `handleJsonMessage` returned. The
   buffer is now consumed before the dispatch.
5. **`sessions.openDatabase` is declared `allowMissing = true`.** The finding is arbitrary
   *location*, not creation: the command legitimately creates a database, and confining it to the
   allowlist roots closes I3 without breaking that.
6. **The pytest write-backlog probe needs a fat response.** The 200-messages-per-second limiter
   makes 16 MiB unreachable with ordinary commands inside a 30 s test; the suite uses
   `meta.listCommands` and skips (rather than fails) if the cap is not reached in budget.
7. **`ServerAuth` is effectively part of WP-G's surface.** WPG-T2 lists only `Server.cpp` and
   `ClientReception.cpp`, but the consent state machine lives in `ServerAuth`; no other package
   claims the file, so it was edited here.

## Counterfactual self-check

**Which rule does this diff most risk violating?** "Never touch, revert, or restore files outside
your own edits" / staying in the package's lane — WP-G's task list names ten files, and the diff
touches twenty-eight.

**Evidence it does not:** every extra file is either named in the task's *Does* line (the six
handlers whose ad hoc checks WPG-T4 explicitly removes), the mechanical consequence of an
interface change I was told to make (`tst_server_auth.cpp` implements `ReceptionHost` and would
not compile otherwise; `ServerAuth.{h,cpp}` owns the consent state WPG-T2 changes;
`CommandProtocol.h` is where WP-G's newly specified error codes live), or a new file of my own
(`DeviceWriteVerdict.h`, `PendingCall.h`, the four suites, the fuzz target, `mirror-wire.json`). I
verified with `grep` over `tasks.md` that no other work package claims any of them. The three
files I was told are someone else's — `Misc/CLI.cpp`, `REUSE.toml`, and the two consent-message
call sites — are untouched and appear above as patches instead.

**Runner-up risk:** "no reference into `m_connections` may survive a call that can spin an event
loop". Evidence: `consumeBytes` no longer takes a `ConnectionState&` at all; every helper that
can dispatch (`drainBuffer`, `handleJsonMessage`, `processNoNewlineBuffer`, `processBufferedJson`)
takes `(socket, sessionId)` and re-resolves through `stateFor`; the two helpers that still take a
reference (`processRawLine`, `processRawJsonCommand`) are called with a pointer resolved in the
same statement block and touch no state after their write; and `tst_client_reception` erases the
entry and rehashes the table from inside `dispatchCommand`, which is a use-after-free under ASan
for any version that kept a reference.
