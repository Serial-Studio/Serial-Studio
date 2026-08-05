---
spec: 0034-async-task-trees
phase: tasks
status: shelved      # reverted 2026-07-30; see spec.md
updated: 2026-07-30
---

# Tasks 0034 — Declarative async orchestration for I/O flows

> **Shelved 2026-07-30 (38c9ef66):** the connection-flow layer these tasks built was removed;
> checked items below describe code that no longer exists. Current contract:
> doc/claude/architecture/io.md. The Async engine itself survives (MQTT::Publisher +
> diagnostics probes).

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
- T1-T7 build and prove the engine with zero I/O edits. T8-T11 wire the layer in without
  changing any driver's behavior. T12-T14 migrate the two v1 flows. Stopping after T11 leaves
  a working tree with the engine present and unused — a deliberate safe halt point.

## Tasks

### T1 — Engine declarations

- **Files:** `app/src/Async/TaskTree.h`, `app/src/Async/AsyncClock.h`
- **Does:** Declare `Async::Outcome`, `Async::StepError`, the `Task` base (one
  `finished(Outcome, StepError)` signal, `start()`/`cancel()` slots), `SequentialGroup`,
  `ParallelGroup`, `TimeoutTask`, `RetryTask`, `SignalTask`, `InvokeTask`, and `TaskRunner`.
  Declare `AsyncClock` as the single indirection over timer scheduling so tests can inject a
  virtual clock. Declarations only — no bodies.
- **Verify:** `python scripts/code-verify.py --check app/src/Async/TaskTree.h app/src/Async/AsyncClock.h`
  clean; header layout matches the repo order (`Q_OBJECT` → signals → ctor → public → slots →
  private), `[[nodiscard]]` on every non-void return, no in-header member init, SPDX header.
- **Deps:** none
- [x] done — `TaskTree.h` declares the full vocabulary; `AsyncClock.h` is header-only (abstract
  `AsyncClock` + `SystemClock` over `QObject::startTimer`, no `Q_OBJECT` so it needs no moc).

### T2 — Task base, InvokeTask, and TaskRunner

- **Files:** `app/src/Async/TaskTree.cpp`
- **Does:** Implement the `Task` base (emit-once discipline, asserted), `InvokeTask` (runs a
  `std::function<bool(QString&)>` and maps the result), and `TaskRunner` (owns the root,
  `run()` cancels any previous root first, destructor cancels, constructor asserts thread
  affinity).
- **Verify:** `code-verify --check` clean; read-back that `finished` cannot be emitted twice
  and that `~TaskRunner` cannot re-enter a caller.
- **Deps:** T1
- [x] done — emit-once asserted in `reportFinished`; `~TaskRunner` disconnects the root before
  cancelling it, so teardown notifies nobody.

### T3 — Sequential and parallel groups

- **Files:** `app/src/Async/TaskTree.cpp`
- **Does:** Implement `SequentialGroup` (child *i+1* starts only on child *i*'s `Success`;
  any other outcome finishes the group with that outcome and that child's `StepError`) and
  `ParallelGroup` (start all; first non-`Success` cancels the siblings and finishes).
- **Verify:** `code-verify --check` clean; read-back that no child is started after a group
  has finished, and that cancel propagates to every started child exactly once.
- **Deps:** T2
- [x] done — shared `Group` base owns children and the `m_cancelling` fence, so a child's
  Cancelled during an unwind can never finish the group from inside it.

### T4 — Timeout and signal-wait tasks

- **Files:** `app/src/Async/TaskTree.cpp`
- **Does:** Implement `TimeoutTask` (on expiry, cancel the child and finish `TimedOut`) and
  `SignalTask` (connect one success signal plus N failure signals on a `QPointer`-guarded
  sender, finish on the first to fire, run the optional abort callable on `cancel()`).
- **Verify:** `code-verify --check` clean; read-back that every connection a `SignalTask`
  makes is torn down on every exit path, including cancel and sender destruction.
- **Deps:** T3
- [x] done — `SignalTask` defers every connection to `doStart()` (a signal fired before the step
  is reached cannot complete it) and releases them on success, failure, cancel, sender death,
  and destruction.

### T5 — Retry policy and RetryTask

- **Files:** `app/src/Async/RetryPolicy.h`, `app/src/Async/RetryPolicy.cpp`,
  `app/src/Async/TaskTree.cpp`
- **Does:** Implement the one shared policy — backoff schedule, attempt cap, reset rule —
  exposed as two named policies (`initialConnect`, `autoReconnect`), and `RetryTask`, which
  waits the policy's delay between attempts, resets the attempt count on success, and treats
  a cancel during the backoff wait as a full stop.
- **Verify:** `code-verify --check` clean; the constants exist in exactly one file (grep the
  tree for stray retry/backoff literals in `app/src/Async/`).
- **Deps:** T4
- [x] done — every backoff/attempt literal lives in `RetryPolicy.cpp`'s Constants block;
  `RetryTask` emits nothing between attempts and resets the count on success.
- Post-review amendment (2026-07-25): `initialConnect()` is now a **flat** 300 ms schedule
  (`kConnectInitialMsec` = `kConnectCeilingMsec` = 300, growth 1.0) across its 5 attempts, replacing
  the 400 ms geometric ramp to a 3000 ms ceiling. A user-initiated connect runs behind a wait
  cursor, so its total retry budget has to match the pre-migration blocking wait instead of growing
  past it. `autoReconnect()` keeps the geometric schedule; `tst_async_engine`'s
  `retryPolicySchedule` table was updated to the flat values.

### T6 — Build registration

- **Files:** `app/CMakeLists.txt`
- **Does:** Add the new `app/src/Async/` sources and headers to `SOURCES`/`HEADERS`. No new Qt
  component, no new `lib/` target, no new `option()`.
- **Verify:** `code-verify --check` clean; read-back that the diff adds only file entries and
  touches no flag, component, or link line.
- **Deps:** T5
- [x] done — landed via the coordinator (the file is owned by the harness agent this pass).
  `app/CMakeLists.txt` now carries `src/Async/TaskTree.cpp`, `src/Async/RetryPolicy.cpp` and
  `src/IO/ConnectionFlows.cpp` in `SOURCES`, and `src/Async/TaskTree.h`,
  `src/Async/RetryPolicy.h`, `src/Async/AsyncClock.h` and `src/IO/ConnectionFlows.h` in
  `HEADERS`. File entries only: no Qt component, no `lib/` target, no `option()`, no flag or
  link line touched.

### T7 — Engine unit tests

- **Files:** the C++ unit target from spec 0032 (R3 tier)
- **Does:** Cover AC1-AC3: sequential ordering and first-failure propagation with a populated
  `StepError`; parallel completion and sibling cancellation; timeout expiry; the exact backoff
  sequence and cap against the virtual clock; attempt-count reset on success; cancel mid-step
  and cancel mid-backoff; `finished` emitted exactly once for every task shape. Fake steps
  only — no socket, device, or broker.
- **Verify:** `ctest` green locally in seconds (maintainer runs). If spec 0032's target has
  not landed, this task blocks on it rather than inventing a second test mechanism.
- **Deps:** T6; spec 0032
- [x] done — `app/tests/tst_async_engine.cpp`: 30 cases over `VirtualClock` + `FakeStep` fakes
  (AC1-AC3). Registered: `app/tests/CMakeLists.txt` declares `ss_add_unit_test(tst_async_engine
  SOURCES tst_async_engine.cpp Async/TaskTree.cpp Async/RetryPolicy.cpp LIBS Qt6::Core)`.
  Running it is still the maintainer's step.

### T8 — HAL_Driver migration hook

- **Files:** `app/src/IO/HAL_Driver.h`
- **Does:** Add `supportsAsyncOpen()` (base returns `false`), `beginOpen(mode)` (base calls
  `open(mode)` and emits the result synchronously), `abortOpen()` (base calls `close()`), and
  the `openFinished(bool, QString)` / `linkDropped()` signals. Defaults must make every
  unmigrated driver byte-identical to today.
- **Verify:** Read `HAL_Driver.h` **in full** before editing — it also defines `CapturedData`,
  the hotpath transport. `code-verify --check` clean; grep confirms no driver overrides the
  new members yet; nothing in the file's hotpath section is touched.
- **Deps:** T6
- [x] done — `supportsAsyncOpen()` (false), `beginOpen()` (synchronous `open()` + immediate
  `openFinished`), `abortOpen()` (`close()`), plus the two signals. No driver overrides any of
  them, and `CapturedData` / `publishReceivedData` are untouched.

### T9 — DeviceManager owns the flow

- **Files:** `app/src/IO/DeviceManager.h`, `app/src/IO/DeviceManager.cpp`
- **Does:** Add one `Async::TaskRunner` member; route `open()` through a flow when
  `supportsAsyncOpen()` is true and keep the existing synchronous call otherwise; stop
  discarding the driver's open result (`DeviceManager.cpp:126`); emit
  `openFinished(deviceId, ok, reason)`. Cancel the runner *before* `killFrameReader()` in
  `close()`.
- **Verify:** Read the file's existing signal wiring first. `code-verify --check` clean;
  read-back that `frameReady` and `rawDataReceived` remain `Qt::DirectConnection` and that no
  new work lands on the per-frame path.
- **Deps:** T8
- [x] done — one `Async::TaskRunner m_runner` (parented to the DeviceManager, so it is destroyed
  before the driver it drives) plus an `m_opening` flag and `isOpening()`. `open()` keeps the
  literal `m_driver->open(mode)` call for every driver that has not opted in and now reports its
  result; `close()` cancels the runner *before* the driver close and `killFrameReader()`. Per-attempt
  driver failures are swallowed while the flow is running, so only a flow's final verdict reaches
  the owner. `frameReady` / `rawDataReceived` wiring untouched.

### T10 — ConnectionManager tolerates a pending open

- **Files:** `app/src/IO/ConnectionManager.cpp`
- **Does:** Make `connectDevice()` (`:683-712`) and `connectAllDevices()` (`:860-865`) correct
  when an open completes later: tie the wait-cursor bracket (`:703`/`:710`) to flow completion
  so it cannot leak, do not assume device *i* finished before device *i+1* starts, and keep
  `shutdownDrivers()`' drain-then-destroy ordering (`:882-907`) valid with a runner present.
  `connectedChanged()` must fire once per real state change, never once per attempt.
- **Does not:** touch `rebuildDevices()`' three queued continuations (`:1336`, `:1344`,
  `:1349`) — flow F6 is a later wave.
- **Verify:** `code-verify --check` clean; grep every `connectedChanged` emitter on the path
  and confirm none is reachable from inside a retry attempt. Highest-risk edit in the diff —
  run `qt-cpp-review` on it specifically.
- **Deps:** T9
- [x] done — the bracket became `beginWaitCursor()` / `endWaitCursor()`, idempotent against a
  single `m_waitCursorActive` bool, so at most one override cursor is ever pushed and a late
  completion cannot pop a cursor it does not own. `connectDevice()` marks the request pending,
  fans out, then calls `concludeConnectRequest()`, which restores the cursor and emits
  `connectedChanged()` exactly once, when no device reports `isOpening()`. Re-check points:
  open completion, disconnect, and rebuild. With no driver opting in, the whole request still
  concludes inside the fan-out, so the emitted sequence is byte-identical to today.

### T11 — Flow composition layer

- **Files:** `app/src/IO/ConnectionFlows.h`, `app/src/IO/ConnectionFlows.cpp`,
  `app/CMakeLists.txt`
- **Does:** Add `IO::Flows` with `makeOpenFlow(driver, mode)`, the TCP-specific step
  composition, `makeMqttReconnectFlow(...)`, and `makeSupervised(flow, policy)` which re-runs
  the flow on `linkDropped()`. No driver is edited by this task — the layer is present and
  exercised only by the tasks that follow.
- **Verify:** `code-verify --check` clean; the tree still behaves exactly as before because
  no driver reports `supportsAsyncOpen()` yet. Safe halt point.
- **Deps:** T10
- [x] done — `IO::DriverOpenTask` (one bounded open attempt, finishing on the driver's own
  outcome so a failure carries its reason), `IO::SupervisorTask` (arms the drop watch on success
  and re-runs the flow on `linkDropped()`, emitting nothing itself), and the two composers
  `Flows::makeOpenFlow()` / `Flows::makeSupervised()`. The TCP-specific step composition and
  `makeMqttReconnectFlow()` are **deliberately deferred to T12/T13**: both depend on driver
  internals (the stored `QHostInfo` lookup id, the broker connect/subscribe signals) that those
  tasks add, and writing them against unread driver code would be speculative. `app/CMakeLists.txt`
  registration is the coordinator's (see the handoff list).

### T12 — Network TCP migration

- **Files:** `app/src/IO/Drivers/Network.h`, `app/src/IO/Drivers/Network.cpp`
- **Does:** Replace the blocking retry loop (`:238-256`, constants `:29-31`) with non-blocking
  primitives driven by the flow: no `waitForConnected`, no `QThread::msleep` on the main
  thread. Store the `QHostInfo::lookupHost` id (`:444`, discarded today) so `cancel()` aborts
  the real lookup and a superseded callback cannot stomp `m_hostExists` (`:512-524`). Emit
  `linkDropped()` on an unsolicited socket disconnect; override `supportsAsyncOpen()`.
- **Verify:** `code-verify --check` clean; grep the file for `msleep` and `waitFor` and
  confirm both are gone. Maintainer observation for AC4: an unreachable address no longer
  freezes the window, and disconnect during the attempt returns to idle immediately.
- **Deps:** T11
- [x] done — `beginOpen()` returns immediately and runs a driver-owned `TaskRunner` over
  `sequential[dns-lookup, socket-connect(600 ms), socket-activate]` (UDP swaps in a `udp-bind`
  step carrying today's bind + buffer + multicast-join). `waitForConnected` and `QThread::msleep`
  are gone from the file; the 600 ms per-attempt window is kept as `kTcpConnectTimeoutMs`, and the
  five attempts are now the shared `RetryPolicy::initialConnect()` cap that `DeviceManager` wraps
  the flow in (same count, one place — R5). The lookup id is stored, superseded lookups are
  aborted, and `lookupFinished()` drops an answer belonging to a lookup it no longer owns.
  `linkDropped()` is emitted on an unsolicited socket disconnect. The generic step lives in
  `ConnectionFlows` as `IO::SocketConnectTask`, which dials from inside its own start so a connect
  completing inside `connectToHost()` cannot be missed.
- [x] **F3 follow-up (2026-07-25, maintainer-approved behavior change).** The first pass left F3's
  error teardown untouched, which made TCP auto-recovery *wired but inert*: a peer close raises
  `errorOccurred(RemoteHostClosedError)` before/with `disconnected()`, and the old handler ran
  `ConnectionManager::disconnectDevice(this)` + a modal, so the supervisor was cancelled before
  `linkDropped()` could reach it and AC5 could never pass. `Network::onErrorOccurred()` now routes
  the error into `linkDropped()` while a supervised flow holds a TCP link that came up (guarded by
  `socketType() == TcpSocket && m_userWantsOpen && isSignalConnected(linkDropped)`, the last term
  being exactly "the supervisor's drop watch is armed"). UDP and any link no flow is watching keep
  the immediate teardown and its box, unchanged. The teardown and the modal now happen only when
  the recovery gives up or the user disconnects: `DeviceManager` remembers a link that came up
  (`m_linkEstablished`) and, on a give-up that follows one, emits `linkLost(deviceId, reason)`
  carrying the last attempt's `StepError` reason; `ConnectionManager::onDeviceLinkLost()` (queued,
  so teardown and a modal never run inside the finishing flow's own emission) closes the source and
  raises one box, latched by `m_linkLossNotified` and re-armed by `connectDevice(int)` so a project
  whose sources drop together cannot stack modals. Initial-connect failures are untouched: the link
  never came up, so no `linkLost` is emitted and the `initialConnect` cap + diagnostics path stays
  as it was. AC5 is now genuinely reachable for TCP.

### T13 — MQTT source-driver migration

- **Files:** `app/src/IO/Drivers/MQTT.h`, `app/src/IO/Drivers/MQTT.cpp`
- **Does:** Replace `scheduleReconnectIfActive()` (`:1076-1108`) and its heap
  `QMetaObject::Connection*` self-freeing lambda with the composed flow; keep all 14 setter
  call sites (`:454`-`:664`) working by routing them at the flow instead of re-implementing
  the disconnect-wait-reopen dance. Emit `linkDropped()` on an unsolicited broker disconnect;
  override `supportsAsyncOpen()`. Post-connect subscribe (`:886-897`) becomes a step, so its
  failure is reported as a step failure rather than a bare message box.
- **Verify:** `code-verify --check` clean; `m_reconnectPending` and the heap `Connection*` are
  gone. Commercial build only — confirm the GPL build still compiles with the file excluded.
- **Deps:** T12
- [x] done — `scheduleReconnectIfActive()` now requests the disconnect and runs one composed tree
  (`mqtt-reconnect`) on the driver's runner, so a second settings change supersedes the first
  instead of stacking one-shot connections; `m_reconnectPending` and the heap
  `QMetaObject::Connection*` are gone, and all 14 setter call sites are unchanged. `beginOpen()`
  runs `sequential[broker-dial, broker-connect(15 s), broker-subscribe]`, so a broker that accepts
  the session but refuses the filter fails the open (with the filter named) instead of leaving a
  connected source that receives nothing — the subscribe message box moved out of
  `onStateChanged()`. `linkDropped()` fires on an unsolicited broker disconnect. Error boxes are
  now one per open request (`m_failureNotified`), because a retried attempt would otherwise stack
  modals. GPL build untouched: the file is commercial-only and nothing MQTT-specific leaked into
  `ConnectionFlows`.

### T14 — MQTT publisher worker migration

- **Files:** `app/src/MQTT/Publisher.h`, `app/src/MQTT/Publisher.cpp`
- **Does:** Replace the worker-side reconnect (`:490-579` and `finishPendingReconnect()`
  `:565-579`) with the same shared flow, run by a `TaskRunner` constructed in
  `bootstrap()` (`:158-168`) on the worker thread. The runner's destructor replaces the manual
  `m_reconnectConn` teardown in `~PublisherWorker` (`:140-153`). Proves the engine is
  thread-affine rather than main-thread-only.
- **Verify:** `code-verify --check` clean; read-back that no engine object is constructed on
  the main thread and later used on the worker, and that the existing main↔worker atomics
  (`Publisher.h:462-464`) are untouched. No mutex added.
- **Deps:** T13
- [x] done — the worker-side reconnect is one tree
  (`wait-disconnect → broker-apply → retry(dial → connect)` under
  `RetryPolicy::autoReconnect()`), run by a `TaskRunner` created in `bootstrap()` **on the worker
  thread**; `finishPendingReconnect()`, `m_reconnectPending` and `m_reconnectConn` are gone, and
  `~PublisherWorker` just drops the runner. `closeBroker()`/`closeResources()` cancel it. The
  main↔worker atomics are untouched and no mutex was added.

### T15 — Expose link state over the API

- **Files:** `app/src/API/Handlers/IOManagerHandler.cpp`
- **Does:** Extend the existing `io.getStatus` response (`:180-188`) with `linkState`,
  `reconnectAttempt`, and `activeFlows`, and extend its description so the assistant reaches
  for it on a "connection keeps dropping" report. No new command, no input-schema change.
- **Verify:** `code-verify --check` clean; `python scripts/sanitize-commit.py` regenerates the
  SDK and search index without drift complaints.
- **Deps:** T14
- [x] done — `io.getStatus` now returns `linkState` (`idle`/`connecting`/`retrying`/`connected`),
  `reconnectAttempt` and `activeFlows`, and its description tells the assistant to read them on a
  "connection keeps dropping" report; `_summary` gains a retrying/connecting sentence naming the
  attempt. The handler could not compute any of it from the existing surface, so the read path is
  three new const accessors — `ConnectionManager::linkState()` / `reconnectAttempt()` /
  `activeFlowCount()`, aggregating `DeviceManager::hasActiveFlow()` / `reconnectAttempt()`, which
  in turn reads `IO::SupervisorTask::attempt()` (the `RetryTask`'s own counter, so a timed-out
  attempt is counted too). `reconnectAttempt` is 1 during a first attempt and >1 only while the
  policy is re-attempting, which is exactly the `connecting` vs `retrying` split. No new command,
  no input-schema change. **Pending:** `app/rcc/api/api-schema.json` is a snapshot refreshed by
  running `SerialStudio --dump-api-schema`, which needs a build — the maintainer's step before
  `generate-sdk` picks the new fields up.

### T16 — Link-recovery integration tests

- **Files:** `tests/integration/test_link_recovery.py`
- **Does:** Write AC5/AC6's tests: `test_tcp_link_recovers_100_severances` (test hosts the
  peer, severs it 100 times mid-stream, asserts each cycle reconnects and frames resume),
  `test_steady_state_after_severance_loop` (`io.getStatus` reports the same `activeFlows` and
  device count before and after), `test_cancel_during_connect_is_immediate`, and
  `test_mqtt_link_recovers` (commercial build, skipped otherwise).
- **Verify:** Maintainer runs `pytest tests/integration/test_link_recovery.py -v` against the
  live app with the API server on port 7777. Also confirm `test_api_drivers.py`,
  `test_driver_api_comprehensive.py`, and `test_workflows.py` still pass (AC7).
- **Deps:** T15
- [x] done — `tests/integration/test_link_recovery.py`, 14 collected cases. The test hosts the
  peer (`SeverablePeer`, a loopback listener whose accepted socket can be cut), so a severance is
  a socket close and the app must come back unasked. The 100 severances are a
  `@pytest.mark.parametrize` over 10 chunks of 10 sharing one module-scoped link: 100 cycles of
  (sever -> reconnect -> frame resumes), each with its own API round trips, does not reliably fit
  the 30 s per-test cap, and chunking keeps it one uninterrupted run of the same connection.
  `test_steady_state_after_severance_loop` compares `activeFlows` against the baseline the rig
  recorded at connect and asserts the peer holds exactly one socket and accepted exactly
  `1 + severances` connections — the process-side stand-in for "no growth in open devices or
  connection count", since `io.getStatus` carries no device-count field.
  `test_cancel_during_connect_is_immediate` asserts latency, not verdict: `io.disconnect` may
  answer "Not connected" while an attempt is in flight, which is today's contract.
  `test_mqtt_link_recovers` proxies the broker (`_BrokerProxy`) so a test that does not own it
  can still cut the session; skipped off a commercial build or without a broker on 127.0.0.1:1883,
  and runs 5 severances rather than 100 for the same cap reason. **Pending (maintainer):** the
  actual run against the live app, plus AC7's `test_api_drivers.py`,
  `test_driver_api_comprehensive.py`, `test_workflows.py`. Static checks green here: `py_compile`,
  `black --check`, `pytest --collect-only` (14 tests).

### T17 — Documentation

- **Files:** `doc/claude/architecture/io.md`, `CLAUDE.md`, `tests/README.md`
- **Does:** Add an orchestration-layer section to `io.md` (where flows live, the
  `beginOpen()`/`linkDropped()` migration hook, which flows are migrated and which are not);
  one bullet in `CLAUDE.md` stating that connection orchestration is task-tree based and that
  retry policy is declared once; one row in the `tests/README.md` integration table.
- **Verify:** Read-back against the code that landed (these three files are outside
  `documentation-verify.py`'s target set, so accuracy is checked by reading, not by a linter);
  `python scripts/sanitize-commit.py` clean.
- **Deps:** T16
- [x] done — `doc/claude/architecture/io.md` gains "Async Orchestration — Task Trees & Connection
  Flows (spec 0034)" ahead of the diagnostics section that already cites it: engine files, the
  thread-affine/emit-once/one-policy invariants, the `HAL_Driver` hook and its behavior-preserving
  defaults, what `ConnectionFlows` composes, `DeviceManager`/`ConnectionManager` ownership, the
  migrated-vs-untouched driver split, and the three `io.getStatus` fields. `CLAUDE.md` gains one
  bullet in Threading & Hotpath (task trees, retry declared once, nothing per frame, no mutex, no
  new thread) pointing at `io.md`. `tests/README.md` gains the integration-table row and the tree
  entry. `documentation-verify.py`: 121 files, 0 findings.

## Tasks — v2 (BLE)

### T18 — BLE flow migration (F9)

- **Files:** `app/src/IO/Drivers/BluetoothLE.h`, `app/src/IO/Drivers/BluetoothLE.cpp`,
  `app/src/IO/HAL_Driver.h`, `app/src/IO/ConnectionFlows.cpp`
- **Does:** Migrate F9 (discovery -> connect -> service discovery -> characteristic subscribe)
  onto the task tree with real per-phase timeouts, `QLowEnergyController::errorOccurred` wired
  into the flow, an epoch guard against a superseded attempt, a real `abortOpen()`, and
  `supportsAsyncOpen()` true. Shared static discovery, the device-list behavior the QML combobox
  reads, the success-path signal order, and `linkDropped()` on an unsolicited disconnect are
  preserved.
- **Verify:** `code-verify --check` clean on the four files; read-back of the composed tree
  against the driver's own signal chain. Maintainer: connect to a real peripheral (success path
  unchanged), power the adapter off mid-attempt (named failure instead of a silent hang), and
  disconnect during an attempt (returns to idle at once).
- **Deps:** T12
- [x] done — flow is `sequential[ble-discovery, ble-dial, ble-connect, parallel[ble-services,
  ble-subscribe]]` on a driver-owned `Async::TaskRunner`. Windows: `ble-discovery` takes the shared
  agent's own `lowEnergyDiscoveryTimeout()` (fallback `kBleScanWindowMs` 15 s) and only waits at all
  when no device is selected yet but a saved identifier is pending — otherwise it passes
  immediately, which is the synchronous open's precondition kept intact; `kBleConnectTimeoutMs`,
  `kBleServicesTimeoutMs`, `kBleSubscribeTimeoutMs` are 15 s each. The GATT phase is a
  `ParallelGroup` on purpose: the driver announces `gattReady()` from *inside*
  `onServiceDiscoveryFinished()`, so a subscribe step armed after that handler returned would wait
  on an edge that already passed — both waits are armed at link-up instead.
  `QLowEnergyController::errorOccurred` is connected for the first time (spec's live defect) and
  reports through `linkFailed()` carrying `errorString()`; three flow-internal relay signals
  (`linkEstablished`, `servicesResolved`, `linkFailed`) let the tree be composed before the
  controller exists, which is what makes the discovery step possible at all. `ble-dial` creates the
  controller and *queues* `connectToDevice()`, so every wait is armed before the dial can complete;
  the queued dial carries `m_openEpoch`, bumped by `beginOpen()`/`abortOpen()`, so a superseded
  attempt cannot dial a controller a newer attempt already replaced. `abortOpen()` bumps the epoch,
  clears the open intent, cancels the runner and closes; `close()` deliberately does *not* cancel
  the runner because it doubles as the driver's internal reset (`selectDevice()`, adapter
  power-off) and would otherwise cancel the flow that is still legitimately running.
  `openTimeoutMsec()` is a new `HAL_Driver` hook (default `0` = keep the shared 15 s ceiling, so no
  other driver changes) that BLE widens to scan + connect + GATT: without it the shared ceiling
  would preempt every BLE phase deadline and a slow-but-working peripheral would be cut off, which
  the synchronous path never did. Dead state removed: `m_pendingServiceIndex` (never assigned a
  positive value anywhere; its `onServiceDiscoveryFinished()` branch could not fire). Kept:
  `m_probeServiceIndex` and the cross-service probe loop, `m_pendingServiceUuid` /
  `m_pendingNotifyUuid` / `m_pendingCharacteristicIndex` / `m_pendingIdentifier` — those are
  resolved-configuration memory, not sequencing flags, and rewriting the probe loop as a tree would
  change GATT resolution on real devices. `code-verify --check`: 4 files, 0 errors, 0 advisories.
  **Note for v1's AC9:** that criterion ("no unmigrated driver edited") named BLE as untouched; this
  task is the deliberate v2 exception, and no *other* unmigrated driver was touched.

## Tasks — v2 (drivers & protocols)

### T19 — UART auto-reconnect on the shared policy (F15)

- **Files:** `app/src/IO/Drivers/UART.h`, `app/src/IO/Drivers/UART.cpp`,
  `app/src/Async/TaskTree.h/.cpp`, `app/src/IO/ConnectionFlows.h/.cpp`,
  `app/src/IO/DeviceManager.cpp`, `app/src/IO/ConnectionManager.cpp`
- **Does:** Replace the level-triggered `m_pendingReconnect` reconnect polled off the 1 Hz
  `TimerEvents` tick with `linkDropped()` + the `SupervisorTask`, under
  `RetryPolicy::autoReconnect()`. The user's `autoReconnect` checkbox still gates recovery, the
  1 Hz port rescan still feeds the device list, and every other serial error still disconnects
  and reports as before.
- **Verify:** `code-verify --check` clean; `m_pendingReconnect` and the tick's
  `connectionManager.connectDevice()` are gone; the drop is emitted outside the error-handler
  lock. Maintainer: unplug an adapter mid-stream with auto-reconnect on and re-plug it.
- **Deps:** T18
- [x] done — `supportsAsyncOpen()` is true, so `DeviceManager` opens UART through
  `supervised(retry(open))`. `handleError()` now splits into a locked `applyErrorPolicy()` that
  returns *whether the link should be recovered* and an unlocked `Q_EMIT linkDropped()`, because
  the recovery re-enters the driver to close and re-open the port and the handler mutex is not
  recursive. A `ResourceError` on a port the user asked to auto-reconnect no longer calls
  `disconnectDevice(this)` — tearing the device down would take the supervisor with it — so the
  device stays alive and the retry is silent, which is what the tick did. Everything else keeps
  today's `disconnectDevice()` + message box.
  Two shared-file changes were needed and are the reviewable part of this task:
  (1) **`RetryTask::setPolicy()` + a recovery policy on `SupervisorTask`.** `DeviceManager`
  wrapped every supervised flow in `initialConnect()` (5 attempts, flat 300 ms), which is right
  for a connect the user waits behind and wrong for a drop: a port that reappears 30 s later
  would never be caught. `makeSupervised()` now takes both policies and the supervisor swaps the
  retry wrapper onto `autoReconnect()` (60 attempts, 500 ms -> 5 s geometric, ~4.5 min budget)
  before re-running the flow. This matches `plan.md`'s own diagram, and it applies to Network and
  MQTT drops too — deliberately, since their supervision had the same too-short budget.
  (2) **`ConnectionManager::onDeviceOpenFinished()` emits `connectedChanged()` when a failure
  arrives with no connect request behind it.** That case is exactly "a supervised link gave up";
  nothing else reported it, so the UI would have kept claiming *connected* after the cable was
  gone for good — a regression against the tick, which disconnected immediately. Also
  `SupervisorTask::attempt()` now returns 0 once the supervisor stops running, so `linkState`
  cannot report `retrying` forever after a give-up.
  Behavior deltas, stated rather than hidden: during recovery the source now reads *connected*
  (the drop no longer tears the device down), which is the spec's stated model — `io.getStatus`
  reports `retrying` and spec 0035 owns surfacing it; and the recovery budget is finite (60
  attempts) where the tick was unbounded. A failed *user* connect now runs 5 attempts instead of
  1, so the "Failed to connect to serial port" box is latched to the first attempt of a sequence
  via `ConnectionManager::reconnectAttempt() <= 1` instead of firing once per attempt, and it is
  raised through a queued call: a modal spins the event loop, and a connect request arriving
  inside it (the API server can send one) would replace the very flow whose stack raised the box.
  The port-index restore the tick did (`setPortIndex(m_lastSerialDeviceIndex)`, which compensates
  for a vanished port shifting every later index) is kept in `refreshSerialDevices()`, now gated
  on a flow being in flight rather than on `m_pendingReconnect`.

### T20 — Modbus connect without the nested event loop (F13)

- **Files:** `app/src/IO/Drivers/Modbus.h`, `app/src/IO/Drivers/Modbus.cpp`
- **Does:** Replace the blocking `QEventLoop::exec()` + `QThread::msleep()` retry loop
  (5 attempts / 800 ms / 300 ms) with a `beginOpen()` flow shaped like Network's: dial, then a
  bounded wait on the client's own state transition. Commercial file; every license guard and
  message-box text is preserved.
- **Verify:** `code-verify --check` clean; `QEventLoop`, `QThread` and `connectWithRetry()` are
  gone from the file. Maintainer: point Modbus TCP at a port nothing is listening on and confirm
  the window stays responsive, then confirm a control-script-launched server still connects.
- **Deps:** T19
- [x] done — `supportsAsyncOpen()` true; `open()` delegates to `beginOpen()` and reports only
  whether an attempt is in flight. `beginOpen()` closes, builds and wires the client
  (`prepareDevice()`), dials `connectDevice()`, and either settles immediately (RTU opens its
  serial port synchronously — the state is already `Connected` before any signal could be waited
  on) or runs `timeout(awaitSignal(modbus-connect), 800 ms)` on a driver-owned `TaskRunner`. The
  wait is armed on two new flow-internal signals, `deviceConnected()` / `deviceDisconnected()`,
  emitted from the existing `onStateChanged()`, so a refused connection fails the step at once
  instead of burning the whole window. Constants: the 800 ms per-attempt window stays as
  `kConnectTimeoutMs`; `kTcpConnectAttempts` (5) and `kTcpConnectBackoffMs` (300) are deleted
  because `RetryPolicy::initialConnect()` is already 5 attempts at a flat 300 ms — the same
  budget, now declared once (R5). Delta: RTU inherits those 5 attempts where it previously had
  one; each RTU failure is fast (the client refuses synchronously) and the "Modbus Connection
  Failed" box is latched to the first attempt, so the user sees one box either way.
  `finishOpen()` is the single settle point — poll timer + `configurationChanged` on success,
  `doClose()` + the box + `openFinished(false, reason)` on failure — and `close()` cancels the
  runner first so a pending attempt cannot re-dial behind it. All three of this driver's open-path
  boxes are raised through the same queued call as T19's, for the same reentrancy reason.

### T21 — ZMODEM retry counter reset (F18)

- **Files:** `app/src/IO/FileTransmission/ZMODEM.cpp`
- **Does:** Fix the live defect the spec names in defect class 5: `m_retryCount` was reset only in
  `startTransfer()`, so NAKs and timeouts accumulated across a whole session and a long transfer
  aborted on unrelated, already-recovered errors.
- **Verify:** `code-verify --check` clean; XMODEM/YMODEM read back as already correct.
- **Deps:** none (pure protocol state, no task tree)
- [x] done — `parseReceivedHeader()` clears `m_retryCount` on `kZRINIT` (receiver ready, and the
  post-`ZEOF` confirmation) and on `kZACK`, which are this implementation's only unambiguous
  positive acknowledgments — the same rule XMODEM (`XMODEM.cpp:121`) and YMODEM
  (`YMODEM.cpp:87/109/129/206`) already apply on `ACK`, so the siblings needed no change. `kZRPOS`
  is deliberately *not* a reset point: the receiver also sends it to request a retransmission
  after a CRC error, so resetting there would let a permanently corrupt link retry forever
  instead of hitting the cap.

## Implementation status (2026-07-25)

Every task T1-T17 has landed in the tree, build registration included. What is *not* verified yet,
stated so nobody reads the checklist as a green build:

- **T6 (build registration) is closed.** `app/CMakeLists.txt` carries the three `Async` /
  `ConnectionFlows` sources and their four headers; `app/tests/CMakeLists.txt` registers
  `tst_async_engine` against `TaskTree.cpp` + `RetryPolicy.cpp`. The earlier note in this section
  claiming T6 was open predated that landing.
- **Nothing here was built or run** — the repo rule stands. Verification in this pass was
  `code-verify.py --check` (clean on every file touched), `documentation-verify.py` (0 findings),
  `black --check` and `pytest --collect-only` on the new test file.
- **Maintainer acceptance criteria are all pending:** AC1-AC3 (`ctest` on the engine suite),
  AC4/AC8 (observation against an unreachable address), AC5-AC7 (`pytest` against the live app),
  AC10 (`--benchmark-hotpath` negative check).
- **AC5 was unreachable for TCP until the F3 follow-up under T12 landed** (2026-07-25): the socket
  error beat `linkDropped()` to the supervisor and tore the flow down. `test_link_recovery.py`
  already assumed recovery rather than the old modal, so it needed no change; it exercises the
  recovery path only, never a give-up, and therefore never raises the new lost-link modal (a
  severance recovers on the first re-attempt, far short of `autoReconnect()`'s 60-attempt cap).
- **`app/rcc/api/api-schema.json` is stale** against T15's three new `io.getStatus` fields; it is
  refreshed by `SerialStudio --dump-api-schema`, which needs a build.

## Deferred: the `configurationChanged` -> `connectedChanged` chain (investigated 2026-07-25)

`ConnectionManager.cpp:95-96` wires `configurationChanged` straight into `connectedChanged`, so
`rebuildDevices()`'s unconditional `Q_EMIT configurationChanged()` (`:1534`) fires
`connectedChanged` on every rebuild — which makes the guarded emission four lines below
(`if (!settled && wasConnected != isConnected())`, `:1537`) unreachable in practice. This was
flagged during the double-emission fix and is **left as-is deliberately**; it is not a one-line
repair, and the obvious repair is wrong.

The chain cannot be replaced by a "only emit when the bool flipped" latch, because
`connectedChanged` is not the notifier for one bool. It is the `NOTIFY` for `isConnected`,
`readOnly`, `readWrite`, and `connectedDeviceCount` (`ConnectionManager.h:80-117`), and it is
also the de-facto notifier for *per-device* connection state:
`app/qml/MainWindow/Panes/Dashboard/WidgetDelegate.qml:429-441` re-evaluates
`Cpp_IO_Manager.isDeviceConnected(deviceIndex)` from an `onConnectedChanged` handler, and
`Console::Handler::onDevicesChanged` (`Console/Handler.cpp:552`) rebuilds its device list from
the same signal. None of those are functions of the aggregate `isConnected()` bool — source 2 of
3 dropping leaves the bool at `true` while both counters and the per-device predicate change.
`ConnectionManager::disconnectDevice(HAL_Driver*)` (`:1101-1108`) already emits unconditionally
for exactly that case, so "connection topology changed" is the signal's real contract and a bool
latch would swallow real changes.

The chain is nonetheless doing damage, which is why this is deferred rather than closed. Because
every driver-property edit forwards `HAL_Driver::configurationChanged` into the same chain
(`:1214-1217`, `wireUiDriver`), editing a baud rate re-fires `connectedChanged`, and four
consumers act on the emission without re-checking state:

- `Console/Export.cpp:291-294` wires `connectedChanged` directly to `closeFile` — aborts an
  in-progress console recording.
- `IO/FileTransmission.cpp:468-471` wires it directly to `stopTransmission` — cancels an
  in-flight X/Y/ZMODEM transfer.
- `UI/Widgets/Terminal.cpp:118-123` calls `clear()` whenever it fires while connected — wipes
  visible scrollback.
- `app/qml/MainWindow/MainWindow.qml:113-132` can re-pop the runtime "connection lost" dialog.

`Dashboard.cpp:187` is a fifth, wasteful rather than destructive: a full `resetData(true)` on
every emission while disconnected.

The hotpath is safe either way, which is why this is not urgent: `FrameBuilder::onConnectedChanged`
(`FrameBuilder.cpp:501`) latches on `m_lastConnectedState` before `invalidateFramePool()` bumps the
frame-pool generation, and `Dashboard::updateStreamAvailable` (`Dashboard.cpp:435`) is a pure
recompute of `m_streamAvailable`. Neither can be disturbed by a spurious emission.

The real fix is therefore per-consumer, not central: guard those four call sites on the state they
actually care about (and split a `connectionTopologyChanged` signal off from `connectedChanged` if
the per-device consumers deserve their own notifier). That is a multi-file behavioral change to
export, file-transfer, and terminal code, outside 0034's scope — it wants its own spec.

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` is met and checked off there.
- [ ] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [ ] `qt-cpp-review` run on the C++ diff; findings addressed or noted. T10 reviewed
      specifically — it is the edit most likely to change connection semantics.
- [ ] `--benchmark-hotpath` run as the negative check (AC10): no hotpath file was edited, so
      any movement means work reached the frame path that should not have.
- [ ] `ctest` green on the engine unit suite (T7), in seconds, with no device present.
- [ ] `pytest tests/integration/test_link_recovery.py` plus `test_api_drivers.py`,
      `test_driver_api_comprehensive.py`, and `test_workflows.py` identified for the
      maintainer to run (listed in `plan.md`).
- [ ] AC9 verified by reading the diff: no unmigrated driver (UART, BLE, Modbus, CAN, Audio,
      USB, HID, Process, the file-transfer protocols) is edited beyond the shared
      `HAL_Driver` hook.
- [ ] No mutex, no new thread, and no new `Qt::QueuedConnection` on the frame path anywhere in
      the diff.
- [ ] `python scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [ ] Diff is *what was asked, and only that* — no scope creep, no foreign files touched. In
      particular, the ten flows outside the v1 set are inventoried in `spec.md` and left alone.
- [ ] `spec.md` status set to `done`.
