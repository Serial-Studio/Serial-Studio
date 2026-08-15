---
spec: 0040-remote-dashboard
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
# Retro-flip 2026-07-25: campaign-level approval (spec 0030); phase gate not run.
updated: 2026-07-25
---

# Tasks 0040 — Remote dashboard attach

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

## Dependency gates — read before starting anything

- **T1-T6 have no dependency on spec 0039.** They produce documents and test assets only: no
  file under `app/` is edited. They are safe to run while 0039 is in flight, and they exist so
  that if the measurement in T5/T6 invalidates the design, it is discovered before any app code
  exists.
- **T7 and everything after it are blocked on spec 0039 reaching M2** — the milestone where the
  session context *owns* the session subsystems rather than forwarding to singletons, which is
  what makes attach reversible (see `plan.md`, "Session context dependency"). Do not start T7
  because 0039 M1 landed; M1 is the seam, not the ownership.
- **0039 M3 is not a gate for this spec** and must not be waited for. It gates concurrent
  local + remote sessions, which v1 explicitly refuses to do (T22).
- **Spec 0032 (C++ unit tier) is a soft dependency only.** If it has landed when T7 is reached,
  the codec tests from T4 are ported to its target; if not, they stay as Python and the port is
  a follow-up.

## Tasks

### Phase P0 — protocol, fixtures, evidence (no 0039 dependency, no app code)

### T1 — Write the normative wire contract

- **Files:** `doc/claude/specs/0040-remote-dashboard/wire-protocol.md`
- **Does:** Specifies the mirror wire in full: the `{"mirror":{...}}` push envelope; the
  `structure` and `snapshot` message kinds and every field; epoch and layout-hash semantics;
  the required message ordering after connect (handshake → `mirror.getInfo` →
  `mirror.subscribe{frames:false}` → `mirror.getStructure`); the `mirror.subscribe` /
  `setRate` / `unsubscribe` / `getInfo` / `getStructure` request and response shapes; the wire
  version integer and the forward-compatibility rules (ignore unknown fields, drop unknown
  kinds); and the error cases (unknown epoch, hash mismatch, version mismatch, rate out of
  range).
- **Verify:** `python scripts/documentation-verify.py`; read-back against `plan.md`'s
  "Architecture & data flow" — every field named there appears, and nothing is specified that
  the plan does not need.
- **Deps:** none
- [x] done — `wire-protocol.md` written. `documentation-verify.py` does not target
  `doc/claude/specs/**` (its default set is the top-level docs, `doc/help`, `examples`,
  `app/rcc/ai`, `app/translations`), so the doc is exempt by design. Three **[correction]**
  sections record where ground truth contradicted `plan.md`:
  1. **Message order.** T1 above says `getInfo → subscribe → getStructure`; `plan.md`'s risk
     register says subscribe must be first. The risk register is right — `processItems`
     (`Server.cpp:300-324`) writes `{"frames":...}` to every socket from the first flush, so
     anything before `mirror.subscribe{frames:false}` is a firehose window. Normative order is
     subscribe-first; its response carries the whole `getInfo` payload.
  2. **Timestamps are not transportable.** `TimestampedFrame::timestamp` is a
     `std::chrono::steady_clock::time_point` (`Frame.h:1601-1605`) with a process-local
     arbitrary epoch, so `plan.md`'s "carries the remote's frame timestamp" is not
     implementable as written. The wire carries per-source relative nanoseconds (`tNs[]`),
     which is exactly what `Dashboard` consumes (`clk.origin` rebase, `Dashboard.cpp:1749-1756`).
  3. **Heartbeat added.** Without it "capture idle" and "link dead" are byte-identical
     silence and R8/R9 cannot be satisfied.
  Also: the auth handshake is *lazy*, not unconditional — `Server.cpp:1323` pre-authenticates
  loopback peers, so an unsolicited `auth` line is an error.

### T2 — Build mirror fixtures from checked-in example projects

- **Files:** `tests/fixtures/mirror/` (new), plus a small generator script alongside it
- **Does:** Produces two fixture streams — a small project and the widest available example
  project — each a recorded NDJSON sequence of one `structure` message and a run of `snapshot`
  messages, encoded per T1 from the example `.ssproj` files already in `examples/`. Synthetic
  values are fine; the point is that the byte shapes and sizes are real.
- **Verify:** every line parses as JSON; the structure message's ordered dataset list matches the
  source project's row-major group/dataset order; snapshot value counts match that list's length.
- **Deps:** T1
- [x] done — `tests/fixtures/mirror/` holds `generate_fixtures.py` (idempotent; `--check` fails
  on drift), `manifest.json`, three streams, and five edge streams. All three verify assertions
  are tests in T4, recomputed straight from the `.ssproj` so a generator bug cannot hide behind
  the generator. Fixtures: `small` (LorenzAttractor, 6 datasets), `wide` (System Monitor, 59),
  and — beyond the task's two — `multisource` (Dual Drone Telemetry, 48 datasets across sources
  `[0, 1]`), the only checked-in project that exercises the `(sourceId, uniqueId)` ordering rule
  the positional format is defined over. Edge streams: `epoch-mismatch`, `hash-mismatch`,
  `version-mismatch`, `length-mismatch`, `forward-compat`, `structure-chunked`.

### T3 — Python mirror client

- **Files:** `tests/utils/mirror_client.py`
- **Does:** A `MirrorClient` sibling to `tests/utils/api_client.py`, reusing its NDJSON framing:
  connect, token handshake, the T1 message ordering, structure/snapshot decode, epoch tracking
  with automatic re-fetch, staleness detection, and reconnect. Usable against fixtures (file
  source) or a live socket, so the codec tests do not need an app.
- **Verify:** `python -m py_compile`; drives both T2 fixtures end to end from the file source and
  reports the decoded dataset count and value ranges.
- **Deps:** T1, T2
- [x] done — `tests/utils/mirror_client.py`. Codec plus client: `layout_hash`,
  `encode_structure` / `encode_snapshot` / `encode_heartbeat` / `encode_structure_chunks`,
  `decode_push`, and a `MirrorClient` that runs on either a `SocketMirrorSource` or a
  `FileMirrorSource`. Follows `api_client.py` idioms (same NDJSON framing, same command
  envelope, context manager, no third-party deps) and is importable standalone so a pure-codec
  test does not pull in numpy/pyserial through `tests/utils/__init__.py`. Drive-through:
  `python3 tests/utils/mirror_client.py tests/fixtures/mirror/*.ndjson` reports
  `small` 6 / `wide` 59 / `multisource` 48 datasets, 60 snapshots each, 0 dropped.

### T4 — Codec conformance tests against the fixtures

- **Files:** `tests/unit/test_mirror_protocol.py`
- **Does:** Round-trips both fixtures; asserts the epoch-mismatch path drops the snapshot and
  requests structure; asserts layout-hash mismatch behaves identically; asserts unknown fields
  and unknown message kinds are tolerated; asserts a version mismatch is reported rather than
  half-decoded. No app, no network.
- **Verify:** `pytest tests/unit/test_mirror_protocol.py -v` (agent can run — pure Python).
- **Deps:** T3
- [x] done — `tests/unit/test_mirror_protocol.py`, **29 tests, all green**
  (`python3 -m pytest tests/unit/test_mirror_protocol.py -q`). Covers the T2 verification
  assertions, full round-trip of all three fixtures, layout-hash reproducibility and
  reorder-detection, string/non-finite out-of-band encoding, the `precision` parameter,
  epoch mismatch (drop + exactly one structure request + recovery on the next structure),
  length mismatch, hash mismatch (structure refused), version mismatch (reported, not
  half-decoded), forward compatibility (unknown fields / unknown `kind` / unrelated top-level
  push keys), chunk reassembly out of order, the rate bounds and watchdog constants, and the
  relative-timestamp invariant. No marker used — `pytest.ini` runs `--strict-markers` and has
  no `unit` marker; `tests/unit/` is outside `testpaths`, matching the existing
  `test_updates_manifest.py`, so it is run by explicit path.

### T5 — Bandwidth and message-size measurement

- **Files:** `doc/claude/specs/0040-remote-dashboard/bandwidth.md`
- **Does:** Records measured bytes/second for both fixtures at 1, 20, and 60 Hz, and the
  structure-message size for each; states the resulting headroom against `kMaxApiMessageBytes`
  (1 MB), `kMaxApiBufferBytes` (4 MB), and `kMaxApiBytesPerWindow` (128 MB/s); and states the
  dataset count at which the structure message would approach the message cap. This is the
  evidence for AC6 and the input to the chunked-structure decision.
- **Verify:** numbers are computed from the T2 fixtures by a script, not estimated in prose; the
  document states the method.
- **Deps:** T2
- [x] done — `bandwidth.md`, every figure produced by
  `python3 tests/fixtures/mirror/measure_bandwidth.py --markdown`. Two findings that change
  the plan:
  1. **Snapshot cost is 19-32 B/dataset at full precision, not the plan's estimated 10-12.**
     The shortest round-trip repr of a double is up to 17 digits. The plan's *conclusion*
     survives: worst measured mirror rate is 66.5 KB/s (59 datasets, 60 Hz), 0.05% of the
     128 MB/s byte cap. `precision` is added as an optional subscribe parameter (halves the
     cost at 6 significant digits) but is not the default.
  2. **The structure message crosses the 1 MB cap at ~2,784 datasets, not ~80,000.** Measured
     by a scaling sweep that replicates the widest project's groups: 374 B/dataset marginal,
     6,597 B fixed. A Modbus-map or DBC import reaches that routinely, so **chunked structure
     delivery is now part of wire version 1** (`wire-protocol.md` §6.2), not a contingency.
     This adds scope to T7, T10, T11, and T14 that this file does not yet carry.
  The live AC6 leg is maintainer-run: `tests/manual/mirror_bandwidth_live.py` drives a real
  device at two rates two orders of magnitude apart and prints a PASS/FAIL table to paste into
  `bandwidth.md`. It cannot run before T7-T13 exist.

### T6 — Backward-compatibility matrix for existing API clients

- **Files:** `doc/claude/specs/0040-remote-dashboard/wire-protocol.md` (compat section)
- **Does:** Enumerates every existing server→client push (`{"frames":...}`, `{"data":...}`,
  `{"event":...}`, `{"type":"response",...}`) and states why a new top-level `mirror` key is
  invisible to a client that key-sniffs; states that `streamFrames` defaults to `true` so an
  unmodified client sees no change; and lists the concrete regression checks the maintainer runs
  in T24 against `tests/utils/api_client.py`.
- **Verify:** cross-checked against the push sites in `app/src/API/Server.cpp` by reading them;
  `python scripts/documentation-verify.py`.
- **Deps:** T1
- [x] done — `wire-protocol.md` §9, cross-checked against every push site by reading
  `Server.cpp`: `{"frames":...}` (`processItems`, `:307-317`), `{"data":...}`
  (`writeRawData`, `:216-219`), `{"event":...}` (`broadcastEvent` via
  `broadcastLifecycleEvent`, `:764-769`), `{"type":"response",...}` (`sendResponseToSocket`),
  and the MCP JSON-RPC branch (`:926-934`). None of the four non-MCP pushes carries a `type`
  field, so every existing client already key-sniffs; `api_client.py::command` skips any
  object that is not the response it awaits (`api_client.py:159`). §9 also states the
  `streamFrames: true` default and the deliberately unchanged `refreshAnyAsyncSink` accounting.
  §9.1 lists six concrete regression checks for T24. Not run through
  `documentation-verify.py`: `doc/claude/specs/**` is outside its target set.

---

> ## ⛔ GATE — spec 0039 M2 must have landed before T7.
> Nothing below this line starts until the session context owns the session subsystems. If M2
> is not done, stop here and report; do not implement a singleton-swapping workaround.
>
> **Gate satisfied code-side (2026-07-25).** All eight session subsystems are owned by
> `SessionContext` (`adoptAppState` / `adoptDashboard` / `adoptConsole` / `adoptFrameParser` /
> `adoptFrameBuilder` / `adoptProjectModel` / `adoptConnectionManager` / `adoptNotifications`,
> `SessionContext.h:83-90`), and `instantiateCoreModules()` constructs each through
> `SessionContext::create<T>()`. `MirrorPublisher` therefore takes a `SessionContext&` and never
> reaches a session singleton. The maintainer-run launch gates of 0039 M2 stay open and are not
> re-litigated here.

---

### Phase P1 — capture side

### T7 — `MirrorProtocol.h` — pure encode/decode

- **Files:** `app/src/API/Mirror/MirrorProtocol.h`, `app/CMakeLists.txt`
- **Does:** Header-only, no singletons, no UI: wire-version constant, message-kind tags,
  `encodeStructure` / `encodeSnapshot` / `decodeStructure` / `decodeSnapshot`, layout-hash
  computation over the ordered `(sourceId, uniqueId)` list. Implements T1 exactly.
- **Verify:** `python scripts/code-verify.py --check app/src/API/Mirror/MirrorProtocol.h`; the
  T2 fixtures decode with this header's field names (read-back comparison against
  `wire-protocol.md`).
- **Deps:** T1, gate
- [x] done — `app/src/API/Mirror/MirrorProtocol.h`, header-only in `namespace API::Mirror`:
  wire-version / rate / precision / chunk constants, `Kind` / `Command` / `ErrorCode` /
  `NonFinite` tag sets, `DatasetId`, `SnapshotValue`, `layoutHash()`, `roundSignificant()`,
  `encodeLine()`, `encodeStructure()`, `encodeSnapshot()`, `encodeHeartbeat()`,
  `chunkStructure()`. Decode is deliberately **not** here: T14 owns the viewer half, and an
  unused decoder would ship untested. Parity with `tests/utils/mirror_client.py` was checked
  mechanically rather than by eye — every constant (including the FNV-1a offset/prime) and
  every emitted field name of all four payload kinds compares equal, and the header's
  `layoutHash` transcribed step for step reproduces all three fixture hashes
  (`3af81b130b65f883` / `640661713f3eadcd` / `8dc889a382eb074b`). `code-verify --check` clean.
  Two house-rule deviations worth knowing: `roundSignificant` goes through the `'g'` formatter
  (`pow()` is a `perf-pow-call` advisory and the formatter is the more faithful reading of
  "significant digits"), which costs the header an `#include "SerialStudio.h"` for
  `toDouble()`.

### T8 — Per-connection frame-stream opt-out

- **Files:** `app/src/API/Server.h`, `app/src/API/Server.cpp`
- **Does:** `ConnectionState` gains `streamFrames` (default `true`), `mirrorSubscribed`,
  `mirrorHz`. `ServerWorker` gains a parallel per-socket flag plus a queued, session-id-tagged
  `setSocketStreamFrames(socket, sessionId, bool)`, and `processItems` skips sockets whose flag
  is false. No change to `refreshAnyAsyncSink` accounting.
- **Verify:** `python scripts/code-verify.py --check` on both files; read-back that the session
  id is carried and verified on the new queued call exactly as `writeToSocket` does; confirm the
  default leaves existing clients' behavior byte-identical.
- **Deps:** T7
- [x] done — `ConnectionState` gains `streamFrames` (default **true**), `mirrorSubscribed`,
  `mirrorHz`, `mirrorPrecision`. The worker's parallel state is a `QSet<QTcpSocket*>
  m_mutedSockets` (empty for every ordinary client) set by the queued
  `setSocketStreamFrames(socket, sessionId, enabled)`, which drops the call on a session-id
  mismatch exactly as `writeToSocket` does; entries are cleared in `addSocket`, `removeSocket`,
  and `closeResources`, so the set can never outlive its socket generation. `processItems`
  skips muted sockets and returns before serializing when every connected client is muted
  (`m_sockets.count() == m_mutedSockets.count()`) — a strict subset of the T8 skip, not a
  change to the accounting `refreshAnyAsyncSink` does, which is untouched. A client that never
  sends `mirror.subscribe` traverses exactly the same code as before: one `QSet::contains` on
  an empty set per socket per flush.

### T9 — Connection-scoped mirror control messages

- **Files:** `app/src/API/Server.cpp`
- **Does:** A mirror branch in `handleJsonMessage` ahead of generic dispatch (same shape as the
  existing MCP branch, because these need the socket): `mirror.subscribe {hz, frames}`,
  `mirror.setRate {hz}`, `mirror.unsubscribe`. Clamps `hz` to `[1, 60]`, rejects out-of-range
  with the documented error, and updates `ConnectionState` plus the worker flag from T8.
- **Verify:** `python scripts/code-verify.py --check`; read-back that an unauthenticated
  connection cannot subscribe, and that disconnect clears mirror state alongside the existing
  MCP session cleanup.
- **Deps:** T8
- [x] done — `handleJsonMessage` gains one branch after the MCP branch and before the registry
  dispatch: `isMirrorCommand()` routes `mirror.subscribe` / `setRate` / `unsubscribe` to
  `handleMirrorCommand()`. Subscribe validates `wireVersion`, `hz ∈ [1,60]` (refused, never
  clamped), and `precision ∈ [0,17]`, registers with the publisher, and then applies
  `frames` (default `false`) through `setStreamFrames()`. Its result is the publisher's
  `getInfo` payload plus `hz` / `effectiveHz` / `frames` / `precision` — and `connectionId`,
  because `sessionId` in the shared payload is the capture session's id
  (`SessionContext::sessionId()`), which is what a viewer actually wants to identify; the API
  connection's own id is the added field. Unauthenticated connections cannot reach this branch
  at all: `onDataReceived` routes them to `handleAuthHandshake` first. Disconnect clears the
  subscription next to `MCPHandler::clearSession` in `onSocketDisconnected`, and every path
  that clears `m_connections` (`setEnabled`, `applyExternalConnections`) also clears the
  subscriber set — guarded on `m_mirrorLinked` so the publisher is never constructed from
  `Server`'s own constructor, which runs inside the pinned singleton order.

### T10 — `MirrorHandler` — stateless registry commands

- **Files:** `app/src/API/Handlers/MirrorHandler.h`, `app/src/API/Handlers/MirrorHandler.cpp`,
  `app/src/API/CommandHandler.cpp`
- **Does:** Registers `mirror.getInfo` (wire version, app version, epoch, layout hash, operation
  mode, dataset count, viewers-allowed, and the names of the connection-scoped commands) and
  `mirror.getStructure`. Registration line added in `initializeHandlers()`.
- **Verify:** `python scripts/code-verify.py --check`; command names follow the `<scope>.<verb>`
  convention; the generated SDK/schema surfaces them after `sanitize-commit.py`.
- **Deps:** T7
- [x] done — `app/src/API/Handlers/MirrorHandler.{h,cpp}`, registered in
  `CommandHandler::initializeHandlers()`. `mirror.getStructure` takes the optional `part` and
  answers `MIRROR_STRUCTURE_TOO_LARGE` when the project needs more than 64 chunks, so a viewer
  is told the project is unmirrorable instead of rendering a truncated dashboard. The SDK /
  `api-schema.json` surfaces are regenerated by `sanitize-commit.py` from a **commercial**
  build dump, which is the maintainer's step; `generate-sdk.py --check` cannot see these two
  commands until that dump is refreshed.

### T11 — `MirrorPublisher` — snapshot production and fan-out

- **Files:** `app/src/API/Mirror/MirrorPublisher.h`, `app/src/API/Mirror/MirrorPublisher.cpp`,
  `app/CMakeLists.txt`
- **Does:** Subscribes to `Dashboard::updated` / `widgetCountChanged` / `dataReset`; owns the
  epoch counter and the ordered dataset list; builds one snapshot per tick and fans it to
  subscribers through the existing session-id-tagged socket write; per-subscriber rate divider
  and last-sent epoch. Returns immediately when there are no subscribers, allocating nothing.
- **Verify:** `python scripts/code-verify.py --check`; read-back that no `MirrorPublisher` code
  is reachable from `hotpathRxFrame` or any per-frame function, and that the zero-subscriber path
  is a leading early return.
- **Deps:** T9, T10
- [x] done — `app/src/API/Mirror/MirrorPublisher.{h,cpp}`, a `SessionContext&`-injected singleton
  (`MirrorPublisher::instance()` is the only `SessionContext::current()` caller, matching the
  `BackupManager` precedent). **Stronger than the task asked:** with no subscriber there is no
  early return to reach, because there is no connection — `activate()` makes the
  `Dashboard::updated` / `widgetCountChanged` / `dataReset` links and starts the heartbeat timer
  on the 0→1 viewer transition, `deactivate()` breaks all three on 1→0, and each is held as a
  captured `QMetaObject::Connection`. That is R12/R13 as absence rather than as a branch.
  Per tick: one snapshot is built (`collectValues()` walks `Dashboard::rawFrame()`, the same
  combined frame `buildValuePushes()` walks, so the positional order is the wire's order by
  construction) and encoded once per **distinct precision**, not once per viewer; each viewer
  carries only `hz`, `precision`, `lastEpoch`, `lastSnapshot`. Structure is cached per epoch and
  pushed to a viewer before any snapshot carrying that epoch. Two decisions to review:
  1. **`tNs` is the publisher's own monotonic clock, per source, rebased at each epoch** — not
     `Dashboard`'s `PlotClock::origin`, which is private and whose accessor would put a second
     edit into `Dashboard.{h,cpp}` (T17 owns the only sanctioned one). The wire only ever
     consumes `tNs` deltas, and at a 20 Hz mirror each snapshot already collapses many frames,
     so the per-frame stamp is not reconstructable at this cadence either way. If the maintainer
     wants remote frame-clock fidelity, it is a one-accessor follow-up, deliberately not bundled.
  2. **`collectValues()` returning false forces an epoch bump** instead of publishing. It can
     only fire if the dashboard's walk stopped matching the epoch's identity list without either
     structural signal — the server-side mirror of the client's length check.

### T12 — Non-interactive opt-in for headless machines (R16)

- **Files:** `app/src/Misc/CLI.h`, `app/src/Misc/CLI.cpp`
- **Does:** Adds `--api-external` (the non-interactive equivalent of the modal "Allow External
  API Connections" confirmation, which an offscreen process cannot answer) and
  `--api-token <hex>` (provision or pin the token). Applied next to the existing `--api-server`
  handling. Neither flag has any effect unless explicitly passed.
- **Verify:** `python scripts/code-verify.py --check`; read-back that omitting the flags leaves
  the persisted settings untouched and the bind address unchanged; `--help` text states the
  security implication of `--api-external`.
- **Deps:** T9
- [x] done — `--api-external` and `--api-token <hex>` registered in `CliOptions`, applied by the
  new `CLI::applyApiServerOptions()` at the top of `applyProjectAndAutoConnect()`, which returns
  immediately when none of the three API flags was passed: an omitted flag writes nothing and
  the bind address is untouched. Order is token → external → enable, so a headless box is
  provisioned with the operator's credential instead of a freshly generated one. The flags stay
  orthogonal: `--api-external` does **not** imply `--api-server`, it only pre-answers the modal.
  Server side, `setExternalConnections()` was split — the post-confirmation body moved verbatim
  into `applyExternalConnections(bool)`, and the new `allowExternalConnections()` slot calls it
  without the dialog (the GUI path still shows the modal, unchanged). `setAuthToken()` refuses
  anything under 32 hex characters rather than quietly weakening the credential.
  `--help` states that any host reaching 7777 can read live data and that the transport is not
  encrypted.

### T13 — Composition-root wiring (capture side)

- **Files:** `app/src/Misc/ModuleManager.cpp`, `app/CMakeLists.txt`
- **Does:** Constructs `MirrorPublisher` in `setupCrossModuleConnections()` **after**
  `instantiateCoreModules()` returns. `instantiateCoreModules()` is not edited, so the spec-0001
  pinned-order body stays byte-identical and its ctor-edge proof is not re-triggered.
- **Verify:** `python scripts/code-verify.py --check`; read-back that the pinned-order body is
  unchanged and that the publisher is not reachable from `ProjectModel`'s constructor closure.
- **Deps:** T11
- [x] done — one line at the end of `setupCrossModuleConnections()`:
  `(void)API::MirrorPublisher::instance();`. `instantiateCoreModules()` is byte-identical, so
  the spec-0001 ctor-edge proof is not re-triggered. The publisher's constructor stores the
  context reference, sets a timer interval, and connects that timer — it touches no session
  module, so it is inert with respect to `ProjectModel`'s constructor closure; the dashboard
  links are made later, on the first subscribe.
  **Not done here (cannot be):** `app/CMakeLists.txt` is off-limits for this pass, so the four
  new files are not yet in `SOURCES` / `HEADERS`. See the registration block in the handoff.

### Phase P1 — viewing side

### T14 — `MirrorClient` — transport, handshake, staleness

- **Files:** `app/src/API/Mirror/MirrorClient.h`, `app/src/API/Mirror/MirrorClient.cpp`,
  `app/CMakeLists.txt`
- **Does:** Owns a `QTcpSocket` on the main thread; performs the token handshake; issues the T1
  message sequence with `frames:false` as the *first* request after auth; decodes pushes via
  `MirrorProtocol.h`; runs the staleness watchdog (`3 × 1/hz`, clamped `[500 ms, 3 s]`); reconnects
  with exponential backoff capped at 30 s, re-fetching structure on each success.
- **Verify:** `python scripts/code-verify.py --check`; read-back that every `connect(...)`
  captures its `Connection` where it is later broken, and that no partial line is ever parsed
  (framing matches `api_client.py`).
- **Deps:** T7
- [x] done — `app/src/API/Mirror/MirrorClient.{h,cpp}`. One main-thread `QTcpSocket`, subscribe as the
  first request, NDJSON framing that never parses a partial line, lazy auth (the challenge carries
  no request id, so it is recognized by its message and the outstanding request is replayed),
  chunked-structure pull and push reassembly, watchdog `3 x 1/effectiveHz` clamped `[500 ms, 3 s]`,
  backoff `1,2,4,8,16,30 s`. **Decode lives here, not in `MirrorProtocol.h`** — T7 deliberately
  left the viewer half to this task, and `layoutHash()` is the one piece shared back. Two
  additions beyond the task: the `failed` signal carries a `fatal` flag (a version mismatch,
  refused credential, or viewer-limit refusal cannot be retried, and T22's restore has to know
  the difference), and `stale` starts *true* so a link that has never delivered a push cannot
  read healthy.

### T15 — `MirrorSession` — attach/detach lifecycle

- **Files:** `app/src/API/Mirror/MirrorSession.h`, `app/src/API/Mirror/MirrorSession.cpp`,
  `app/src/Misc/ModuleManager.cpp` (construction after the pinned order + `Cpp_API_Mirror`
  context property, registered after wiring and before the QML load — INV-2)
- **Does:** Takes the 0039 session context. On attach: snapshots local session state (project
  document, operation mode, plot time range, frozen flag), loads the mirrored structure through
  the ordinary project-load path, and builds the frame template for the epoch. On detach or
  disconnect: restores the snapshot. Exposes `attached`, `endpoint`, `live`, `stale`, and the
  last error to QML.
- **Verify:** `python scripts/code-verify.py --check`; read-back that restore runs on *every*
  exit path including abnormal disconnect, and that no session state is written before the
  snapshot is taken.
- **Deps:** T14
- [x] done — `app/src/API/Mirror/MirrorSession.{h,cpp}`, constructed in
  `setupCrossModuleConnections()` after the pinned order and registered as `Cpp_API_Mirror` before
  the QML load. Attach snapshots the local project document, path, **modified flag**, operation
  mode, plot range and frozen flag *before the first write*; the mirrored project loads through
  `ProjectModel::loadFromJsonDocument(doc)` with an **empty source path**, which is what keeps the
  debounced autosave from writing the remote's layout over the user's file. Restore runs from
  `detach()` and from a fatal `MirrorClient::failed`, so an abnormal end is not a parked dead
  endpoint. The modified flag is restored too: without it a user with unsaved edits would get the
  content back but lose the save prompt.

### T16 — Snapshot injection into the dashboard

- **Files:** `app/src/API/Mirror/MirrorSession.cpp`
- **Does:** Per arriving snapshot: validate epoch and layout hash (mismatch → drop and re-fetch),
  assign values positionally into the epoch's frame template, and publish one
  `TimestampedFramePtr` into `UI::Dashboard::hotpathRxFrame` carrying the *remote's* timestamp.
  No re-stamping, and no path to `FrameBuilder::hotpathTxFrame` (a viewer's export sinks must
  never see mirrored frames).
- **Verify:** `python scripts/code-verify.py --check`; read-back confirming the timestamp is the
  decoded remote value and that no export/consumer sink is reachable from this function.
- **Deps:** T15
- [x] done — Per epoch, `buildTemplates()` rebuilds one `TimestampedFrame` per source from the
  loaded project, filtered exactly as `FrameBuilder::buildEnabledGroups` filters it and walked in
  ascending-sourceId / group / dataset order, then **refuses the epoch unless the layout hash it
  computes over its own templates equals the announced one** — the check that makes assigning bare
  positional numbers safe. Snapshots assign into those templates and publish straight to
  `UI::Dashboard::hotpathRxFrame`; `FrameBuilder::hotpathTxFrame` is never reached, so a viewer's
  export sinks cannot see a mirrored frame. **Correction to the task text:** the timestamp is not
  "the remote's timestamp" — `TimestampedFrame::timestamp` is a process-local `steady_clock`
  point, so per `wire-protocol.md` §7.1 the wire carries relative `tNs` and the viewer
  reconstructs `attachAnchorLocal + (tNs - tNsAtAttach)` per source. Deltas are the remote's;
  only the origin is local. One hazard closed that the task did not name: mirrored frames stamp
  `structureGeneration` in a `1 << 48` band so a mirrored epoch can never alias the local frame
  pool's generation counter and skip a reconfigure.

### T17 — The one cached-flag input (hotpath-adjacent)

- **Files:** `app/src/UI/Dashboard.h`, `app/src/UI/Dashboard.cpp`
- **Does:** `streamAvailable()` gains the mirror-attached input; `connectStreamAvailableInputs()`
  wires `MirrorSession::attachedChanged` into `updateStreamAvailable()` with
  **`Qt::DirectConnection`**. `hotpathRxFrame` is not edited.
- **Verify:** `python scripts/code-verify.py --check`; invoke `ss-hotpath` and state the
  invariants before editing; **maintainer runs `--benchmark-hotpath` before and after** and
  compares against the tier table; diff review confirms this is the only capture-path file in
  the change.
- **Deps:** T16
- [x] done — `streamAvailable()` gains `API::MirrorSession::mirroring()` as a leading
  `[[unlikely]]` early return, next to the existing benchmark one; `hotpathRxFrame` is
  byte-identical and the per-frame cost is unchanged (that path reads the cached
  `m_streamAvailable`, never this function). **Deviation from the task text, deliberate:** the
  connection is *not* made in `connectStreamAvailableInputs()`. That function runs inside
  `Dashboard`'s constructor, i.e. inside `instantiateCoreModules()`'s pinned order, and reaching
  `MirrorSession::instance()` from there would construct a post-order module inside the ctor
  closure — the spec-0001 failure that shipped and crashed once. Instead `MirrorSession`'s own
  constructor (built after the pinned order) wires
  `attachedChanged -> Dashboard::updateStreamAvailable` with **`Qt::DirectConnection`**, and
  `mirroring()` is a plain module-flag read that constructs nothing. The direct-connection
  requirement and the cached-flag semantics are unchanged; only the side that owns the `connect`
  moved. `connectStreamAvailableInputs()`'s doxygen records this so the input stays discoverable.
  **Maintainer gate still open:** `--benchmark-hotpath` before/after.

### Phase P1 — UI, gating, docs

### T18 — Attach dialog

- **Files:** `app/qml/Dialogs/RemoteAttach.qml`
- **Does:** Host, port, token, remembered endpoints, connect/disconnect, and an error line that
  distinguishes refused / unauthorized / version-mismatch / unreachable. Endpoint ComboBox guards
  `onCurrentIndexChanged` with the standard `if (count <= 0) return`.
- **Verify:** `python scripts/code-verify.py --check` on the QML; maintainer opens the dialog and
  confirms layout, theme, and error strings.
- **Deps:** T15
- [x] done — `app/qml/Dialogs/RemoteAttach.qml`, plus its `DialogLoader` and `app.showRemoteAttach()`
  in `main.qml` (two files the task did not list; a dialog with no loader is unreachable). Host,
  port, token, mirror rate, remembered endpoints, Attach/Detach, and a status line that separates
  *not attached* / *attached, no data* / *attached, stale* / *attached, live*. The endpoint combo
  guards `onCurrentIndexChanged` with the standard `if (count <= 0) return`. The refusal reason
  renders under the status line, and the v1 trust model is stated where the credential is typed.
  Remembered endpoints persist under `Mirror/RecentEndpoints`; **the token never touches disk.**

### T19 — Command, binding, icon

- **Files:** `app/rcc/commands/app.json`, `app/qml/Commands/AppCommandBindings.qml`,
  `app/rcc/icons/commands/{16,24,32,48}/`
- **Does:** One `app.remoteAttach` manifest entry, one binding, one tiered icon — the spec-0028
  recipe, with a `Cpp_CommercialBuild` guard if the Pro question is answered yes.
- **Verify:** `python scripts/registry-verify.py`; `python scripts/generate-command-strings.py
  --check`; the command appears in the palette and Start menu.
- **Deps:** T18
- [x] done — one `app.remoteAttach` entry in `app/rcc/commands/app.json` (contexts `app` +
  `dashboard`, category `tools`), one `cmdAppRemoteAttach` binding in `AppCommandBindings.qml`, and
  a new tiered icon `commands/remote-attach` at 16/24/32 with its `rcc.qrc` entries. **No
  `Cpp_CommercialBuild` guard:** the Pro question in `plan.md` is still open and the capture half
  shipped ungated, so gating only the viewer would be an answer nobody has given yet — flagged for
  the maintainer rather than decided here. `registry-verify.py` CLEAN;
  `generate-command-strings.py --check` up to date. One extra file: `Cpp_API_Mirror` had to be
  added to `WidgetExtensions::hostContextNames()`, which `registry-verify` enforces for every
  registered context property (spec 0038 T17).

### T20 — Remote / stale indicators

- **Files:** `app/qml/MainWindow/Panes/Setup.qml`,
  `app/qml/MainWindow/Panes/Dashboard/DashboardLayout.qml`
- **Does:** Shows attached endpoint and live/stale state, reusing the existing API-server
  indicator component shape in both files so it reads as one system.
- **Verify:** `python scripts/code-verify.py --check`; maintainer confirms the indicator changes
  state on attach, on detach, and when the link is severed.
- **Deps:** T18
- [x] done — the existing API-server indicator in `Setup.qml` and `DashboardLayout.qml` is one
  component duplicated in both files; both got the identical edit, so it still reads as one system.
  When attached it shows `Remote <endpoint> - Live / No Data / Stale` and pulses on `live` instead
  of on `clientCount`; detached, its text and animation are byte-equivalent to before. The three
  states are now derived from two readonly properties (`mirrored`, `pulsing`) rather than repeated
  inline conditions.

### T21 — Unavailable-over-remote widget states

- **Files:** `app/qml/Widgets/Dashboard/FFTPlot.qml`,
  `app/qml/Widgets/Dashboard/Waterfall.qml`, `app/qml/Widgets/Dashboard/Plot3D.qml`
- **Does:** Each renders an explicit "not available over remote attach" state bound to
  `MirrorSession.attached` — never an empty or zeroed widget (AC5).
- **Verify:** `python scripts/code-verify.py --check`; maintainer attaches with a project
  containing all three and confirms the state, while scalar and plot widgets in the same project
  render live.
- **Deps:** T20
- [x] done — `FFTPlot.qml`, `Waterfall.qml` and `Plot3D.qml` each gain one overlay bound to
  `Cpp_API_Mirror.attached` that says the widget is unavailable over a remote attach and why (the
  mirror carries dataset values, not the raw sample stream these three transform). Inlined in the
  three files rather than factored into a shared component: a fourth QML file would need a
  `CMakeLists.txt` registration this pass is not allowed to make.

### T22 — Attach refusal and detach restore

- **Files:** `app/src/API/Mirror/MirrorSession.cpp`, `app/qml/Dialogs/RemoteAttach.qml`
- **Does:** Refuses to attach while the local session has a device or player open, with a clear
  reason in the dialog; verifies the local project, operation mode, plot time range, and frozen
  flag are restored on detach and after an abnormal disconnect.
- **Verify:** `python scripts/code-verify.py --check`; maintainer: attach while connected (refused
  with reason), then disconnect locally, attach, detach, and confirm the previous project and mode
  are back.
- **Deps:** T21
- [x] done — refusal is `MirrorSession::canAttach`, derived from
  `Dashboard::streamAvailable()` (which already covers the device and all three players, so the
  predicate cannot drift from the one the dashboard itself uses) and surfaced in the dialog as a
  named reason plus a disabled Attach button; `attach()` re-checks before acting, so the refusal
  is correct at the moment it matters even if a signal is one turn late. Restore runs on
  `detach()` **and** on a fatal link failure, and puts back project document, path, modified flag,
  operation mode, plot range and frozen flag. A *transient* disconnect deliberately does not
  restore: the client reconnects with backoff and the indicator reads stale, which is R8/R9.

### T23 — Viewer limit and multi-viewer fan-out

- **Files:** `app/src/API/Mirror/MirrorPublisher.cpp`, `app/src/API/Server.cpp`
- **Does:** Adds the `API/MaxViewers` setting (default: all allowed while the API server is on),
  enforced at subscribe; confirms one snapshot per tick is fanned out with per-subscriber rate
  dividers and last-sent epochs rather than rebuilt per viewer.
- **Verify:** `python scripts/code-verify.py --check`; read-back that the snapshot is built once
  per tick; maintainer runs two viewers and confirms consistency (AC3).
- **Deps:** T22
- [x] done — **verification only, no code changed.** Both halves already landed in T9/T11 and were
  re-read to confirm it: `MirrorPublisher::maxViewers()` reads `API/MaxViewers` (default `-1` ->
  `Server::maxClients()`, i.e. all allowed while the API server is on), `subscribe()` refuses past
  the ceiling, and `Server::mirrorSubscribe` answers `MIRROR_VIEWER_LIMIT`. Fan-out builds the
  snapshot **once per tick** (`collectValues()` -> `m_values`), encodes it once per *distinct
  precision* via `m_tickLines`, and carries only `hz` / `precision` / `lastEpoch` / `lastSnapshot`
  per viewer. The two-viewer consistency run is a maintainer gate (AC3).

### T24 — Integration test file

- **Files:** `tests/integration/test_remote_mirror.py`
- **Does:** Attach and read live values; structure epoch after a remote project change; detach
  leaves the capture running; reattach; two simultaneous viewers agree; severed link marks stale
  within bound and recovers; wrong token refused; loopback-only bind refuses a remote attach; and
  the T6 regression checks that an unmodified `api_client.py` sees no behavior change.
- **Verify:** `python -m py_compile`; markers registered in `pytest.ini`; **maintainer runs it**
  against two live instances (the agent cannot).
- **Deps:** T23
- [x] done — `tests/integration/test_remote_mirror.py`, 7 tests
  (`test_attach_yields_a_verified_structure`, `test_live_values_flow_at_display_cadence`,
  `test_detach_leaves_capture_running_and_reattach_recovers`,
  `test_two_viewers_agree_and_are_independent`,
  `test_stale_when_nothing_arrives_within_the_watchdog`, `test_wrong_token_is_refused`,
  `test_plain_api_client_unaffected_while_a_viewer_is_attached` — the T6 regression check).
  `python3 -m py_compile` clean; `mirror`/`integration`/`network` markers registered in
  `tests/pytest.ini:50`. Every leg is environment-gated (`SS_MIRROR_HOST/PORT/TOKEN`) and skips
  cleanly with no live instance, so this checkbox covers only "the file exists and is correct" —
  actually running it against two live capture instances is the maintainer leg named in the
  task and stays open under AC1-AC3.

### T25 — Help page (trust model)

- **Files:** `doc/help/` (new page), `doc/help/help.json`
- **Does:** How to enable remote attach on both sides, including the headless provisioning flags
  from T12, and the v1 trust model stated plainly: unencrypted transport, token protects the
  network hop only, loopback pre-authenticated, all-or-nothing credential — therefore trusted
  network or tunnel (R15, AC9).
- **Verify:** `python scripts/documentation-verify.py`; page registered in `help.json`; invoke
  `ss-docs` for voice and structure.
- **Deps:** T24
- [x] done — 2026-07-25 via `ss-docs`: `doc/help/Remote-Dashboard.md` (Connectivity section),
  registered in `help.json`, back-linked from `API-Reference.md`; trust model stated per R15
  (unencrypted transport, token = full API credential not a viewer pass, loopback exempt,
  trusted network or tunnel). `documentation-verify.py` clean. Ran ahead of T24 (integration
  test is maintainer-run).

### T26 — Architecture doc

- **Files:** `doc/claude/architecture/remote.md`, `doc/claude/architecture.md`, `CLAUDE.md`
- **Does:** The subsystem doc — wire contract summary, the `hotpathRxFrame` mirror seam, what is
  and is not mirrored, the cached-flag input and why it is direct-connected, the deferred
  async-sink accounting, and the 0039 dependency shape. One index row; two lines in `CLAUDE.md`.
- **Verify:** `python scripts/documentation-verify.py`; every factual claim traced to a file the
  task author read.
- **Deps:** T25
- [x] done — 2026-07-25, as `doc/claude/architecture/mirror.md` (named after the
  `API::Mirror` namespace rather than the planned `remote.md`); index row added in
  `architecture.md`, CLAUDE.md gained a "Remote Dashboard Mirror (spec 0040)" block plus the
  cached-flag note in the hotpath section, and `dataflow.md`'s Cached Hotpath Flags names the
  `mirroring()` disjunct. Ran ahead of T25 (help page still open) — the dependency was
  ordering convenience, not a content gate.

### T27 — Final verification pass

- **Files:** none (verification only)
- **Does:** Full-diff self-review against `spec.md`; confirms the only capture-path file is
  `Dashboard.cpp` and its change is the named input; records the AC6 measurement and the
  benchmark comparison in this spec directory.
- **Verify:** `python scripts/code-verify.py --check` on all changed files;
  `python scripts/registry-verify.py`; `qt-cpp-review` on the C++ diff;
  `python scripts/sanitize-commit.py`; maintainer runs `--benchmark-hotpath` and the three
  operation-mode launches.
- **Deps:** T26
- [ ] done

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` is met and checked off there.
- [ ] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [ ] `python scripts/registry-verify.py` clean (command + icon added in T19).
- [ ] `qt-cpp-review` run on the C++ diff; findings addressed or noted.
- [ ] `--benchmark-hotpath` compared before/after and within tier; the diff contains exactly one
      capture-path file (`Dashboard.cpp`) and exactly one cached-flag input, wired direct.
- [ ] AC6 bandwidth evidence and the benchmark comparison are recorded in this spec directory.
- [ ] `pytest tests/unit/test_mirror_protocol.py` green (agent-runnable);
      `tests/integration/test_remote_mirror.py` identified for the maintainer to run.
- [ ] Existing `tests/integration/` suite re-run as a smoke check — no existing API behavior
      changed.
- [ ] `python scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [ ] Diff is *what was asked, and only that* — read-only mirror, no control channel, no
      transport security work, no foreign files touched.
- [ ] `spec.md` status set to `done`, with the P2/P3/P4 phases still listed as deferred rather
      than quietly dropped.
