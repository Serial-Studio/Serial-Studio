# Architecture — IO & Drivers

> Part of the architecture corpus ([index](../architecture.md)). Read this file in full
> before touching `app/src/IO/` driver, manager, or protocol code. New drivers go through
> the `ss-new-driver` skill (BluetoothLE is the canonical reference).

## IO Architecture — No Singleton Drivers

- 10 drivers, **public ctors**, no `static instance()`.
- `ConnectionManager` (singleton, `Cpp_IO_Manager`) owns one **UI-config** instance per type:
  `instance().uart()`, `.network()`, `.bluetoothLE()`, etc. QML context properties
  (`Cpp_IO_Serial`, etc.) point at these.
- `createDriver()` makes **fresh** instances for live connections, owned by `DeviceManager`.
- `configurationOk()` checks the **UI** driver, not the live one. UI driver's
  `configurationChanged` forwards to `ConnectionManager::configurationChanged`. All drivers
  must `Q_EMIT configurationChanged()` from their ctor.
- Live drivers may have empty device lists. UART/Modbus call `refreshSerialDevices()` /
  `refreshSerialPorts()` in `open()` if empty.

## Async Orchestration — Task Trees & Connection Flows (spec 0034)

Connection lifecycles are declared as task trees instead of per-driver boolean state machines.
The engine is `app/src/Async/`: `TaskTree.{h,cpp}` (`Async::Task` base + `SequentialGroup`,
`ParallelGroup`, `TimeoutTask`, `RetryTask`, `SignalTask`, `InvokeTask`, `TaskRunner`, plus the
`sequential()` / `parallel()` / `timeout()` / `retry()` / `awaitSignal()` / `invoke()` builders),
`RetryPolicy.{h,cpp}`, and `AsyncClock.h` (the timer indirection the unit tests drive with a
virtual clock).

- **Thread-affine, never thread-safe.** A tree lives on the thread that created its
  `TaskRunner` (asserted in the ctor); no mutex, no new thread, no cross-thread hop. The MQTT
  publisher's runner is built in `PublisherWorker::bootstrap()` on the worker thread.
- **A task emits `finished(Outcome, StepError)` exactly once.** `StepError` carries the step
  identity plus a reason, which is what spec 0035's network checks read to tell *not resolved*
  from *refused* from *timed out*.
- **`TaskRunner` is the only handle a caller holds.** `run()` cancels the previous root first;
  the destructor cancels silently (it disconnects before cancelling, so teardown notifies
  nobody).
- **Retry/backoff constants live only in `RetryPolicy.cpp`** — `RetryPolicy::initialConnect()`
  and `RetryPolicy::autoReconnect()`. No driver, flow, or protocol may carry its own interval
  or attempt loop.

Drivers opt in through `HAL_Driver`, whose defaults keep every unmigrated driver byte-identical
to before: `supportsAsyncOpen()` returns `false`, `beginOpen(mode)` calls `open(mode)` and emits
`openFinished(ok, reason)` synchronously, `abortOpen()` calls `close()`, `openTimeoutMsec()`
returns `0` (take the shared 15 s per-attempt ceiling; a driver whose handshake legitimately runs
longer returns its own). A migrated driver also emits `linkDropped()` when a link that was open
goes down *without* a close request.

`IO::ConnectionFlows` composes the trees: `DriverOpenTask` (one bounded open attempt),
`SocketConnectTask` (dials from inside its own start, so a connect completing inside
`connectToHost()` cannot be missed), `SupervisorTask` (arms the drop watch on success and
re-runs the flow on `linkDropped()`), and the `Flows::makeOpenFlow()` / `makeSupervised()` /
`makeSocketConnect()` composers.

- `DeviceManager` owns one runner per device. `open()` runs
  `Flows::makeSupervised(makeOpenFlow(...), RetryPolicy::initialConnect(),
  RetryPolicy::autoReconnect())` when the driver opts in and otherwise keeps the literal
  synchronous `open()` call, whose result is no longer discarded; `close()` cancels the runner
  **before** `killFrameReader()`. The two policies are not interchangeable: the first sequence
  runs short because the user waits behind it, and `SupervisorTask` moves the retry wrapper onto
  the recovery schedule (`RetryTask::setPolicy()`) before re-running a dropped link.
- **Retry attempts emit nothing.** A per-attempt failure is swallowed while the runner is
  running, so only a flow's final verdict reaches the owner and a recovering link cannot
  amplify connection-state churn (frame-pool generation, Dashboard stream availability).
- `ConnectionManager::connectDevice()` no longer assumes the open completed: the wait cursor is
  an idempotent `beginWaitCursor()` / `endWaitCursor()` pair and `concludeConnectRequest()`
  emits `connectedChanged()` once, when no device reports `isOpening()`.
- **Migrated in v1:** Network (TCP + UDP), the MQTT source driver, the MQTT publisher worker.
  **Migrated in v2:** BLE — `sequential[ble-discovery, ble-dial, ble-connect,
  parallel[ble-services, ble-subscribe]]`, per-phase windows in `BluetoothLE.cpp` (scan window
  from the shared agent, 15 s each for connect/services/subscribe), `QLowEnergyController::
  errorOccurred` wired into the flow, an `m_openEpoch` guard on the queued dial, and
  `openTimeoutMsec()` widened to scan + connect + GATT. The GATT phase is *parallel* because the
  driver announces `gattReady()` from inside its own service-discovery handler: a step armed
  after that handler returned would wait on an edge that already passed. Shared static discovery
  is unchanged — the flow asks for a scan, never restarts or stops one.
  Also in v2: **UART**, whose auto-reconnect (the app's only pre-0034 drop recovery, previously
  polled off the 1 Hz tick) is now `linkDropped()` + the supervisor, still gated on the user's
  `autoReconnect` checkbox and still fed by the 1 Hz port rescan; and **Modbus**, whose blocking
  `QEventLoop` connect loop became one bounded `modbus-connect` wait (800 ms per attempt) with
  the five attempts and the 300 ms between them coming from `RetryPolicy::initialConnect()`.
  **Not migrated:** CAN, Audio, USB, HID, Process, and the file-transfer protocols — they run
  the base-class defaults untouched.
- **Nothing here runs per frame.** Task trees exist at connection-lifecycle boundaries only.
- Observable over the API: `io.getStatus` reports `linkState`
  (`idle`/`connecting`/`retrying`/`connected`), `reconnectAttempt`, and `activeFlows`
  (`ConnectionManager::linkState()` / `reconnectAttempt()` / `activeFlowCount()`).

## Connection Diagnostics (spec 0035)

`Misc::ConnectionDiagnostics` (`Cpp_Misc_ConnectionDiagnostics`) answers "is this machine
set up to connect?" without opening a link. Checks split in two:

- **Instant** (`Misc/Diagnostics/{Serial,Bluetooth,Network,Audio}Checks`): serial ports and
  `access(R_OK|W_OK)` on the device node (the Linux group remedy is composed from the node's
  real `st_gid`), BLE adapter power via `BluetoothLE::adapterPoweredOn()`, audio backend +
  input devices, and host/port sanity. They answer inside the call.
- **Probing** (`NetworkChecks`): `HostLookupTask` + `TcpProbeTask` on the spec-0034 task
  tree under explicit timeouts (2 s lookup, 3 s connect). The probe connects, aborts, and
  never sends a byte. `StepError::step` — not an error string — picks between the
  *not resolved* / *refused* / *timed out* verdicts.

Results cache per bus; the five `diagnostics.<bus>` problem-center checkers only *read* that
cache, because `ProblemCenter::Checker` must return findings synchronously. Diagnostics never
touch a driver's configuration and start no discovery scan.

`ConnectionManager::onDeviceOpenFinished(deviceId, ok, reason)` is the auto-trigger: on
failure it resolves the failing device's bus and calls `onOpenFailed()` (instant checks
always, probing run only outside the 30 s per-bus window); on success it calls
`onOpenSucceeded()`, which clears that bus's cached results.

## File Transmission (Pro)

`IO::FileTransmission` + `IO::Protocols::*`: controller +
XMODEM/YMODEM/ZMODEM. Incoming data routes from `ConnectionManager::onRawDataReceived` →
`FileTransmission::onRawDataReceived` (guarded by `active()`). Protocols emit
`writeRequested(QByteArray)`; controller calls `ConnectionManager::writeData()`.
