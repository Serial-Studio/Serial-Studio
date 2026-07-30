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

## Opening a Link — Synchronous, Per-Driver

`DeviceManager::open(mode)` starts the `FrameReader` if it is null and then calls
`m_driver->open(mode)` directly. There is no orchestration layer: `HAL_Driver` declares no
async-open hook, `DeviceManager` owns no task runner, and nothing sits between
`ConnectionManager::connectDevice()` and the driver. The spec-0034 `IO::ConnectionFlows` layer
and the `HAL_Driver` hooks it drove (`supportsAsyncOpen`, `beginOpen`, `abortOpen`,
`openTimeoutMsec`, `openFinished`, `linkDropped`) were removed 2026-07-30; spec 0034's docs
describe a design that is no longer in the tree.

- **Drop recovery is each driver's own business.** UART sets `m_pendingReconnect` on a
  `QSerialPort::ResourceError` and retries off the 1 Hz `TimerEvents` tick, gated on the user's
  `autoReconnect` checkbox and fed by the same 1 Hz port rescan. MQTT funnels every
  configuration change and error into `scheduleReconnectIfActive()`. Modbus and Network dial
  inside `open()`. A new driver that needs recovery adds it locally — there is no shared
  supervisor to inherit.
- **The connect request still settles once.** `ConnectionManager` keeps the
  `m_connectPending` / `m_connectFanOut` pair, the idempotent
  `beginWaitCursor()` / `endWaitCursor()`, and `concludeConnectRequest()`, which emits
  `connectedChanged()` exactly once. Because opens complete synchronously,
  `hasPendingOpen()` is a constant `false` and the request always settles inside the fan-out.
- **Three `ConnectionManager` accessors are constant stubs** kept only so the QML and API
  surfaces do not change shape: `activeFlowCount()` returns `0`, `reconnectAttempt()` returns
  `0`, and `linkState()` returns `connected` or `idle` — never `connecting` or `retrying`. Their
  Doxygen still describes the removed flows. `io.getStatus` reports these values as-is.
- **`ConnectionManager::onDeviceOpenFinished()` is currently unreachable** — `DeviceManager` no
  longer emits an open-finished signal and nothing else calls it, so the spec-0035 diagnostics
  auto-trigger below never fires. Diagnostics still run when invoked explicitly.

## The Async Task-Tree Engine (`app/src/Async/`)

The engine outlived the connection flows and is still built and unit-tested
(`tst_async_engine`, `tst_async_combinators`): `TaskTree.{h,cpp}` (`Async::Task` base +
`SequentialGroup`, `ParallelGroup`, `TimeoutTask`, `RetryTask`, `SignalTask`, `InvokeTask`,
`TaskRunner`, plus the `sequential()` / `parallel()` / `timeout()` / `retry()` /
`awaitSignal()` / `invoke()` builders), `RetryPolicy.{h,cpp}`, and `AsyncClock.h` (the timer
indirection the unit tests drive with a virtual clock).

Two consumers remain: `MQTT::Publisher` (its reconnect flow, runner built in
`PublisherWorker::bootstrap()` on the worker thread) and the spec-0035 diagnostics probes
(`Misc::ConnectionDiagnostics` + `Misc/Diagnostics/NetworkChecks`). No IO driver uses it.

- **Thread-affine, never thread-safe.** A tree lives on the thread that created its
  `TaskRunner` (asserted in the ctor); no mutex, no new thread, no cross-thread hop.
- **A task emits `finished(Outcome, StepError)` exactly once.** `StepError` carries the step
  identity plus a reason, which is what the network checks read to tell *not resolved* from
  *refused* from *timed out*.
- **`TaskRunner` is the only handle a caller holds.** `run()` cancels the previous root first;
  the destructor cancels silently (it disconnects before cancelling, so teardown notifies
  nobody).
- **Retry/backoff constants live only in `RetryPolicy.cpp`** — `RetryPolicy::initialConnect()`
  and `RetryPolicy::autoReconnect()`. No consumer may carry its own interval or attempt loop.
- **Nothing here runs per frame.** Trees exist at lifecycle boundaries only.

## Connection Diagnostics (spec 0035)

`Misc::ConnectionDiagnostics` (`Cpp_Misc_ConnectionDiagnostics`) answers "is this machine
set up to connect?" without opening a link. Checks split in two:

- **Instant** (`Misc/Diagnostics/{Serial,Bluetooth,Network,Audio}Checks`): serial ports and
  `access(R_OK|W_OK)` on the device node (the Linux group remedy is composed from the node's
  real `st_gid`), BLE adapter power via `BluetoothLE::adapterPoweredOn()`, audio backend +
  input devices, and host/port sanity. They answer inside the call.
- **Probing** (`NetworkChecks`): `HostLookupTask` + `TcpProbeTask` on the `Async::` task
  tree under explicit timeouts (2 s lookup, 3 s connect). The probe connects, aborts, and
  never sends a byte. `StepError::step` — not an error string — picks between the
  *not resolved* / *refused* / *timed out* verdicts.

Results cache per bus; the five `diagnostics.<bus>` problem-center checkers only *read* that
cache, because `ProblemCenter::Checker` must return findings synchronously. Diagnostics never
touch a driver's configuration and start no discovery scan.

`ConnectionManager::onDeviceOpenFinished(deviceId, ok, reason)` was the auto-trigger: on
failure it resolves the failing device's bus and calls `onOpenFailed()` (instant checks
always, probing run only outside the 30 s per-bus window); on success it calls
`onOpenSucceeded()`, which clears that bus's cached results. **It is dead code since the
connection flows were removed** — no signal reaches it, so nothing auto-runs diagnostics after
a failed open. Rewiring it means calling it from the synchronous open path in
`ConnectionManager::connectDevice()`.

## File Transmission (Pro)

`IO::FileTransmission` + `IO::Protocols::*`: controller +
XMODEM/YMODEM/ZMODEM. Incoming data routes from `ConnectionManager::onRawDataReceived` →
`FileTransmission::onRawDataReceived` (guarded by `active()`). Protocols emit
`writeRequested(QByteArray)`; controller calls `ConnectionManager::writeData()`.
