---
spec: 0034-async-task-trees
phase: plan
status: shelved      # reverted 2026-07-30; see spec.md
updated: 2026-07-30
---

# Plan 0034 — Declarative async orchestration for I/O flows

> **Shelved 2026-07-30 (38c9ef66):** the connection-flow layer this plan designs was removed;
> the design below describes code that no longer exists. Current contract:
> doc/claude/architecture/io.md. The Async engine itself survives (MQTT::Publisher +
> diagnostics probes).

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Read the relevant `doc/claude/` sub-docs and the *actual code*
> before writing this — a plan grounded in a stale mental model is worse than no plan.
> Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

Add a small, in-repo, event-loop-only task-tree engine under `app/src/Async/` — six
combinators (sequential group, parallel group, timeout, retry, wait-for-signal, invoke)
plus a runner that owns the root and cancels on destruction — and one shared
`Async::RetryPolicy` carrying the backoff schedule and attempt cap. Drivers gain an
optional non-blocking `beginOpen()` hook on `HAL_Driver` whose default implementation
calls today's synchronous `open()`, so every unmigrated driver keeps working with no edit.
`IO::DeviceManager` owns one runner per device and, for drivers that opt in, runs a flow
composed by `IO::Flows` instead of calling `open()` directly; the same flows supervise a
dropped link and re-run the open sequence under the retry policy. v1 migrates the TCP
path (removing a ~4.5 s main-thread block) and the MQTT path (collapsing two duplicate
reconnect implementations onto one policy, one of them on the publisher's existing worker
thread — which proves the engine is thread-affine rather than main-thread-only). Nothing
on the frame hotpath is touched.

## Affected subsystems & files

| File | Change |
|------|--------|
| `app/src/Async/TaskTree.h` | **New.** `Outcome`, `StepError`, `Task` base, `SequentialGroup`, `ParallelGroup`, `TimeoutTask`, `RetryTask`, `SignalTask`, `InvokeTask`, `TaskRunner`. |
| `app/src/Async/TaskTree.cpp` | **New.** Engine implementation: child ownership, start/cancel propagation, first-failure semantics, timer wiring. |
| `app/src/Async/RetryPolicy.h` | **New.** Backoff schedule, attempt cap, reset rule; two named policies (initial connect, auto-reconnect). |
| `app/src/Async/RetryPolicy.cpp` | **New.** The constants and the delay computation, in one place (R5). |
| `app/src/Async/AsyncClock.h` | **New.** One-line indirection over `QTimer` start so unit tests can drive a virtual clock (R10). |
| `app/src/IO/ConnectionFlows.h` | **New.** `IO::Flows` free functions composing per-driver trees + the supervision wrapper. |
| `app/src/IO/ConnectionFlows.cpp` | **New.** TCP open flow, MQTT reconnect flow, `makeSupervised()`. |
| `app/src/IO/HAL_Driver.h` | Add `supportsAsyncOpen()`, `beginOpen()`, `abortOpen()` with behavior-preserving defaults; add `openFinished(bool, QString)` and `linkDropped()` signals. **Read in full before editing** — this header also defines `CapturedData`, the hotpath transport. |
| `app/src/IO/DeviceManager.h` / `.cpp` | Own an `Async::TaskRunner`; route `open()` through a flow when the driver opts in; stop discarding the open result (`DeviceManager.cpp:126`); emit `openFinished(deviceId, ok, reason)`. |
| `app/src/IO/ConnectionManager.cpp` | Tolerate a pending open in `connectDevice()` (`:683-712`) and `connectAllDevices()` (`:860-865`); make the wait-cursor bracket (`:703`/`:710`) leak-proof under async completion; keep the `shutdownDrivers()` drain-then-destroy ordering (`:882-907`) valid with a runner in the picture. |
| `app/src/IO/Drivers/Network.h` / `.cpp` | Replace the blocking retry loop (`:238-256`) with non-blocking primitives; store and abort the `QHostInfo` lookup id (`:444`) so cancel is real and stale callbacks cannot land; emit `linkDropped()` on unsolicited socket disconnect. |
| `app/src/IO/Drivers/MQTT.h` / `.cpp` | Replace `scheduleReconnectIfActive()` (`:1076-1108`) and its heap `QMetaObject::Connection*` lambda with a composed flow; emit `linkDropped()` on unsolicited broker disconnect. |
| `app/src/MQTT/Publisher.h` / `.cpp` | Replace the worker-side reconnect (`:490-579`, `:565-579`) with the same composed flow, run by a runner created on the worker thread in `bootstrap()` (`:158-168`). |
| `app/src/API/Handlers/IOManagerHandler.cpp` | Extend the existing `io.getStatus` response (`:180-188`) with `linkState`, `reconnectAttempt`, and `activeFlows`; no new command. |
| `app/CMakeLists.txt` | Add the new TUs to `SOURCES`/`HEADERS`. No new Qt component, no new `lib/` target. |
| `tests/integration/test_link_recovery.py` | **New.** The 100-severance loop plus the steady-state assertions (AC5, AC6). |
| `tests/README.md` | One row for the new integration file. |
| `doc/claude/architecture/io.md` | Add a section describing the orchestration layer and the migration hook. |
| `CLAUDE.md` | One bullet: connection orchestration lives in task trees; reconnect policy is declared once. |

## Architecture & data flow

```
ConnectionManager::connectDevice(id)              (main thread)
  │  no longer assumes open() completed when it returns
  ▼
DeviceManager::open(mode)
  │  driver->supportsAsyncOpen() ? run a flow : call driver->open(mode)  [unchanged path]
  ▼
Async::TaskRunner (one per DeviceManager, thread-affine)
  │  owns the root Task; destructor cancels
  ▼
IO::Flows::makeSupervised( makeOpenFlow(driver), RetryPolicy::autoReconnect() )
  │
  ├─ RetryTask(policy)
  │    └─ SequentialGroup
  │         ├─ SignalTask   resolve host      (cancel -> QHostInfo::abortHostLookup)
  │         ├─ TimeoutTask  connect socket    (SignalTask over connected/errorOccurred)
  │         └─ InvokeTask   post-open setup   (subscribe, apply properties)
  │
  └─ on success: watch driver->linkDropped(); on drop, re-run the RetryTask
  ▼
DeviceManager::openFinished(deviceId, ok, reason)
  ▼
ConnectionManager  -> connectedChanged() exactly once per real state change
```

Engine semantics, stated so review can veto them before code exists:

- **`Task`** is a `QObject` with `start()` / `cancel()` slots and one
  `finished(Outcome, StepError)` signal. `Outcome` is `Success`, `Failure`, `Cancelled`,
  or `TimedOut`. A task emits `finished` exactly once; the engine asserts that.
- **`SequentialGroup`** starts child *i+1* only after child *i* reports `Success`. Any
  other outcome finishes the group with that outcome and that child's `StepError`,
  and no further child is started.
- **`ParallelGroup`** starts all children; the first non-`Success` cancels the remaining
  siblings and finishes the group with that outcome. Present because BLE (v2) needs it;
  v1's flows are sequential.
- **`TimeoutTask`** wraps one child. On expiry it cancels the child and finishes
  `TimedOut` — indistinguishable from a failure to the parent (spec R2).
- **`RetryTask`** wraps one child and a `RetryPolicy`. On child failure it waits the
  policy's delay for the current attempt, then restarts the child. Cancel during the wait
  stops the wait and runs nothing further (spec R3). Success resets the attempt count.
- **`SignalTask`** connects a success signal and any number of failure signals on a
  `QPointer`-guarded sender, and finishes on the first that fires. It takes an optional
  abort callable, which is how `QHostInfo::abortHostLookup` and
  `QAbstractSocket::abort` are reached from `cancel()`.
- **`InvokeTask`** runs a `std::function<bool(QString&)>` synchronously and maps the
  result. This is what `HAL_Driver::beginOpen()`'s default implementation uses, so an
  unmigrated driver participates in a tree with today's exact semantics (spec R11).
- **`TaskRunner`** holds the root as a `std::unique_ptr<Task>`, exposes `run()`,
  `cancel()`, `isRunning()`, and cancels in its destructor. It is the only object a
  caller holds; nothing outside the engine touches raw `Task` pointers.

`HAL_Driver` migration hook, with defaults chosen so the diff cannot change an unmigrated
driver's behavior:

- `supportsAsyncOpen()` returns `false` in the base class.
- `beginOpen(mode)` default: call `open(mode)` and `Q_EMIT openFinished(result, reason)`
  synchronously — byte-identical to today for every driver that does not override it.
- `abortOpen()` default: `close()`.
- `linkDropped()` is emitted by a driver only when a link that was successfully open goes
  down *without* a close request. No unmigrated driver emits it, so supervision is inert
  for them.

## Hotpath & threading impact

- **Touches the hotpath?** **No.** Nothing in this change runs per frame. The
  `Driver → FrameReader → FrameBuilder → Dashboard` path is untouched: no edit to
  `FrameReader`, `CircularBuffer`, `FrameBuilder`, or `Dashboard`, and no change to the
  three known `Qt::DirectConnection` hops (`DeviceManager::frameReady →
  ConnectionManager::onFrameReady`, `DeviceManager::rawDataReceived →
  ConnectionManager::onRawDataReceived`, `FrameReader::readyRead →
  DeviceManager::onReadyRead`). `DeviceManager` gains a member and a new
  connection-lifecycle signal (`openFinished`), neither of which is on the per-frame path.
  `--benchmark-hotpath` is run as a negative check (AC10), not because a regression is
  expected.
- **New cross-thread signal/slot?** **No new cross-thread hop is introduced.** The engine
  is *thread-affine*, not thread-safe: every `Task` in a tree lives on the thread that
  created its `TaskRunner`, all internal connections resolve to direct calls within that
  thread, and its timers are that thread's timers. Two runners on two threads share
  nothing — `RetryPolicy` is a value, copied into each. The Network flow runs on the main
  thread; the publisher flow runs on the *already existing* `m_workerThread`, created in
  `PublisherWorker::bootstrap()` (`Publisher.cpp:158-168`) alongside the `QMqttClient` it
  drives, so it never reaches across threads to the client. The existing main↔worker hops
  (`Publisher.cpp:828`, `:1961-1964`, and the atomics at `Publisher.h:462-464`) are
  unchanged. No mutex is added anywhere, and `FrameReader`/`CircularBuffer` stay
  main-thread SPSC.
- **New input to a cached hotpath flag?** **No new input, but one existing input changes
  frequency and must be contained.** `m_streamAvailable` (Dashboard) and the frame-pool
  generation bump both key off connect/disconnect transitions. Automatic recovery must not
  turn one user-visible outage into N transitions: the retry attempts are *internal to the
  flow* and emit nothing, and `openFinished`/`connectedChanged` fire only when the device's
  real open state changes. This is the single most likely silent-breakage vector in the
  change and is called out again under Risks with its test.
- **Timestamp ownership** — unchanged. Sources still stamp at the driver boundary via
  `publishReceivedData`; a reconnect creates no timestamp, re-stamps nothing, and does not
  backdate. `FrameReader` is recreated through the existing `resetFrameReader()` /
  `reconfigure()` path on reconnect, exactly as a manual reconnect does today.

## Data model & persistence

None. No `Frame.h` `Keys::` addition, no project-JSON shape change, no schema or writer
version bump, no Sessions DB change, no `widgetSettings` change. Retry constants are
compile-time in `RetryPolicy.cpp`, not user settings — if the open question about making
automatic recovery a per-source setting is answered "yes", that adds one boolean to the
source record and is called out here as the only persistence impact it would have.

## API / SDK surface

- `io.getStatus` (`IOManagerHandler.cpp:180-188`) gains three read-only fields:
  `linkState` (`idle` / `connecting` / `connected` / `retrying`), `reconnectAttempt`
  (integer, 0 when not retrying), and `activeFlows` (integer, the number of running
  runners). No new command, no schema change to any existing input, no new handler file.
  The description string is extended so the assistant reaches for it when a user reports a
  connection that keeps dropping.
- `activeFlows` exists specifically so AC5 can assert steady state after 100 severance
  cycles without inspecting process memory.
- No commercial-only surface: the fields are unconditional, and the MQTT-specific values
  simply never appear in a GPL build because the bus does not exist there.
- No `EnumLabels.cpp` slug is added — `linkState` is a plain string, not a bus enum.

## QML / UI

None in v1. The spec's non-goals exclude UI, and `linkState` is deliberately exposed only
over the API so spec 0035 (connection diagnostics) owns how a retrying source is presented.
The existing connect/disconnect controls, the wait cursor, and the error message boxes keep
their current wording and behavior; the only visible difference is that the window stays
responsive during a connect attempt.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| **D1 — Engine source** | (A) link `Qt6::TaskTree`, the Qt 6.11 module; (B) vendor Qt Creator's in-tree `solutions/tasking` under `lib/`; (C) minimal in-repo engine | **(C).** Six reasons below; (A) is the recorded fallback. |
| **D2 — Who owns a running flow** | `ConnectionManager` owns a global runner; each driver owns its own; `DeviceManager` owns one per device | **`DeviceManager`.** It is already the per-device lifecycle owner (it holds the `FrameReader` and does the `killFrameReader()` teardown), so per-device cancel and destruction come free and `shutdownDrivers()`' existing drain ordering keeps working. A global runner would need its own device demultiplexing; a driver-owned runner would put policy back inside drivers, which is what this spec removes. |
| **D3 — Where policy lives** | Per-driver constants (today); one policy object shared by all flows | **One shared `Async::RetryPolicy`,** named per use (`initialConnect`, `autoReconnect`). Spec R5. It also fixes the fifth defect class by construction: today the same policy exists five times with three different constant sets. |
| **D4 — Engine thread model** | Thread-safe (locks) so any thread can drive any tree; thread-affine (one tree, one thread) | **Thread-affine.** A thread-safe engine would need mutexes, which the repo's threading rules forbid on principle in I/O code, and would buy nothing: each flow drives objects that already have a fixed thread affinity (`QMqttClient` must live on its worker thread; `QTcpSocket` on main). Affinity is asserted in the `TaskRunner` constructor. |
| **D5 — Driver migration mechanism** | Migrate all drivers at once; a virtual hook with a behavior-preserving default | **Virtual hook with a default.** Spec R11 and AC9: the v1 diff must not touch UART, BLE, Modbus, CAN, Audio, USB, HID, or Process. The default `beginOpen()` reproduces today's synchronous call exactly. |
| **D6 — v1 migration set** | Start with BLE (biggest LOC win); start with Network + MQTT | **Network + MQTT.** BLE is the largest orchestration surface (~550 lines) but has the least test coverage and the most device-specific behavior, so it is the worst place to prove a new engine. Network TCP and MQTT are the two flows that carry the recurring defect classes (a main-thread block, and two duplicate reconnect implementations), are both exercisable from a pytest harness with no hardware, and together cover the sequential, timeout, retry, and worker-thread cases. BLE is v2, once the engine has a green test tier under it. |
| **D7 — Drop detection** | Poll connection state on a timer (UART's current approach); a driver-emitted `linkDropped()` signal | **Driver-emitted signal.** Polling reintroduces per-driver timers and a fixed latency floor; the sockets and the MQTT client already emit a disconnect signal, so the driver just has to distinguish "user asked" from "it fell over" — which every migrated driver already tracks (`m_userWantsOpen`, `m_cfg.enabled`). |
| **D8 — Unit-test seam** | Real `QTimer` and real wall time; an injectable clock | **Injectable clock** (`Async::AsyncClock`). AC2 and AC3 must assert a multi-attempt backoff schedule; against real time that is a multi-second test, which is exactly the kind of slow test the R3 tier exists to avoid. The indirection is one function. |
| **D9 — Where the engine lives** | `app/src/Misc/`; a new `app/src/Async/` | **New `app/src/Async/`.** `Misc/` is the utilities grab-bag; a subsystem with its own unit-test suite and a documented contract earns a directory, consistent with the existing top-level split (`IO/`, `DataModel/`, `API/`, `Platform/`). |

### D1 in full — why an in-repo engine over the named library

Spec 0030 recorded "one vetted library candidate (BSD-licensed, standalone, Qt-native)".
Research on 2026-07-25 shows that description conflates two different artifacts with two
different licenses, and neither is BSD:

- **Qt TaskTree** is an official Qt module as of Qt 6.11, target `Qt6::TaskTree`, but it is
  **Technology Preview** — explicitly outside Qt's compatibility promise. It is offered
  under commercial licenses from The Qt Company, or LGPLv3, or GPLv2.
- **Qt Creator's `solutions/tasking`** is the same code shipped inside Qt Creator's tree.
  Its per-file SPDX header is Qt Creator's, not BSD. Vendoring it into a build that
  produces a proprietary binary needs that header verified first, and if it is the usual
  Qt Creator GPL-with-exception expression, it is disqualified outright.

Against that, the case for (C):

1. **License.** `BUILD_COMMERCIAL` ships a proprietary binary. (B) is blocked or
   unverified. (A) is usable, but only under the Qt commercial license or dynamic linking —
   a real constraint the plan should not silently assume on the maintainer's behalf. An
   in-repo engine carries the repo's own `GPL-3.0-only OR LicenseRef-SerialStudio-Commercial`
   header and raises no question at all.
2. **Stability.** Technology Preview means the API can change in 6.12. Building the
   connection layer of a telemetry application on that trades a known, bounded maintenance
   cost for an unbounded one.
3. **Build and deploy surface.** (A) adds a Qt component that must be present in every CI
   Qt installation on three platforms and travel through three deployment paths. Preview
   modules are not always selected by default in Qt installers.
4. **Fit.** The needed subset is six combinators. Tasking ships task adapters, storage,
   conditionals, iterators, barriers, and network/process wrappers — the app would take on
   a general job system to obtain sequential-plus-timeout-plus-retry.
5. **Quality gates.** Vendored code under `lib/` is compiled with `-w` and is outside
   `code-verify.py`'s scope (`lib/CMakeLists.txt:118-140`). Code that owns connection
   lifecycles should be inside the repo's linter, its Power-of-Ten rules, and its unit
   tier — which is exactly what (C) allows and (A)/(B) do not.
6. **Precedent.** The repo already builds its own icon registry, command registry, SIMD
   layer, and undo history rather than importing frameworks for them.

**Recorded fallback, so review does not relitigate it later.** If the in-repo engine
exceeds roughly 1000 lines, or if nested cancellation / step-storage semantics prove
subtler than six combinators suggest, switch to (A). To keep that switch mechanical rather
than a rewrite, the in-repo vocabulary deliberately mirrors Tasking's (`Group`,
`Sequential`, `Parallel`, `Timeout`, `onDone`), and every driver-facing call goes through
`IO::Flows`, so only the engine files change.

## Risks & mitigations

- **Reconnect flapping amplifies connection-state churn.** Each connect/disconnect bumps
  the frame-pool generation and toggles Dashboard stream availability. If retry attempts
  leaked out as state transitions, 100 severances would become hundreds of pool
  invalidations. *Mitigation:* attempts are internal to `RetryTask` and emit nothing;
  `openFinished` fires once per real state change. AC5 asserts steady state after the loop,
  and the plan's task list includes a read-back of every `connectedChanged` emitter on the
  path.
- **`connectDevice()` no longer completes synchronously.** Today `ConnectionManager::
  connectDevice()` (`:683-712`) brackets the call with `setOverrideCursor` /
  `restoreOverrideCursor` and callers read `isConnected()` immediately after. *Mitigation:*
  the cursor bracket is tied to flow completion (or removed, since the flow no longer
  blocks); `connectAllDevices()` (`:860-865`) must not assume device *i* finished before
  device *i+1* starts. This is the highest-risk edit in the diff and gets its own task.
- **Re-entrancy during teardown.** `shutdownDrivers()` (`:882-907`) already drains
  `m_devices` into a local before destroying it, because a driver's `close()` re-emits
  `configurationChanged()` during destruction. A runner cancelling from `~TaskRunner` must
  not re-enter `ConnectionManager`. *Mitigation:* cancel is synchronous and emits only
  `finished(Cancelled, ...)`; `DeviceManager` cancels its runner *before* `killFrameReader()`
  in `close()`, and the flow's completion handler checks the runner is still the current one.
- **Stale continuations, the exact class this spec exists to remove.** *Mitigation:* every
  `SignalTask` guards its sender with `QPointer`; `TaskRunner::run()` cancels any previous
  root before starting a new one; and the `QHostInfo` lookup id is stored so cancel aborts
  the real lookup instead of merely ignoring its result (`Network.cpp:444` discards it today).
- **The publisher's reconnect lives on a worker thread.** `~PublisherWorker`
  (`Publisher.cpp:140-153`) currently disconnects `m_reconnectConn` by hand. *Mitigation:*
  the runner is a member of `PublisherWorker`, constructed in `bootstrap()` on the worker
  thread and destroyed with the worker on that thread; its destructor replaces the manual
  disconnect. The `TaskRunner` constructor asserts its thread affinity so a
  wrong-thread construction fails loudly in debug rather than racing in release.
- **Behavior drift in error text.** Removing the blocking loop changes *when* an error is
  reported, and F3 currently suppresses per-attempt errors via `m_connecting`
  (`Network.cpp:531-532`). *Mitigation:* the flow reports only the final failure, matching
  today's user-visible outcome; AC7 runs the existing driver/API tests unchanged.
- **The C++ unit tier does not exist yet.** AC1-AC3 depend on spec 0032 (R3), authored in
  parallel. *Mitigation:* the engine is written with no Qt GUI dependency and no singleton
  reach, so it drops into that target unchanged. If 0032 has not landed when this is
  implemented, the engine tests become the first consumers of that target and the tasks
  block on it rather than inventing a second test mechanism.
- **Scope creep into the other ten flows.** The inventory is tempting. *Mitigation:* AC9
  makes "no unmigrated driver edited" a checkable criterion, and the task list names the
  exact file set.

## Test & verification plan

- **Unit (C++ tier, spec 0032 target — the agent writes them, the maintainer runs
  `ctest`):**
  - Sequential ordering; a failing child stops the group and propagates its `StepError`.
  - Parallel completion; first failure cancels siblings.
  - Timeout expiry finishes `TimedOut` and cancels the child.
  - Retry produces the policy's exact delay sequence against the virtual clock; stops at
    the cap; resets the attempt count on success.
  - Cancel mid-step and cancel mid-backoff both run nothing further.
  - `finished` is emitted exactly once by every task shape.
  - `RetryPolicy` schedule and cap values.
- **Unit (`tests/scripts/`):** not applicable — no JS parser logic is involved.
- **Integration (maintainer runs; app up with the API server on `localhost:7777`):**
  - `tests/integration/test_link_recovery.py` — **new.**
    `test_tcp_link_recovers_100_severances`: the test hosts the peer, severs it 100 times
    mid-stream, and asserts each cycle reconnects and frames resume (AC5).
    `test_steady_state_after_severance_loop`: `io.getStatus` reports the same
    `activeFlows` and device count before and after the loop (AC5, R9).
    `test_cancel_during_connect_is_immediate`: connect to a black-holed address, then
    `io.disconnect`, and assert the API answers within a normal round trip rather than
    after the old ~4.5 s block (AC4's automatable half).
    `test_mqtt_link_recovers` (commercial build): the same severance loop over MQTT (AC6).
  - `tests/integration/test_api_drivers.py`, `test_driver_api_comprehensive.py`,
    `test_workflows.py` — must pass unchanged (AC7).
- **Maintainer observation:** AC4 (window stays responsive against an unreachable TCP
  address; disconnect during the attempt returns to idle immediately) and AC8 (a failed
  connect names the step that failed).
- **Hotpath:** `--benchmark-hotpath` as a negative check (AC10). No hotpath file is edited,
  so any movement means something reached the frame path that should not have.
- **Static:** `python scripts/code-verify.py --check` on every changed file;
  `qt-cpp-review` on the C++ diff before handoff; `python scripts/sanitize-commit.py`
  before commit.
