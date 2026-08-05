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

- **Drop recovery is each driver's own business.** UART arms `m_pendingReconnect` plus its own
  1 s `m_reconnectTimer` in `handleError()` on a `QSerialPort::ResourceError` (live instances
  recover too — the timer belongs to the instance, not the UI driver's 1 Hz rescan), matches
  the returned port by name, and `close()` disarms both so a manual disconnect is always final.
  MQTT funnels every configuration change and error into `scheduleReconnectIfActive()`. Modbus
  and Network TCP dial asynchronously with driver-local timer retries (below). A new driver
  that needs recovery adds it locally — there is no shared supervisor to inherit.
- **The connected state is published idempotently.** Every lifecycle path funnels into the
  private `notifyConnectedStateChanged()`, which emits `connectedChanged()` only when the
  `isConnected()` flag or the open-device count actually moved. Callers never reason about
  whether another path already reported; calling it twice is harmless. The
  `m_connectPending` / `m_connectFanOut` pair and `concludeConnectRequest()` survive only to
  settle the wait cursor and to make `toggleConnection()` treat an in-flight request as
  "connected" so the button aborts instead of stacking a second attempt.
- **Async dials are visible through `HAL_Driver::isConnecting()`** (default `false`).
  Network (TCP), BluetoothLE, MQTT, Modbus, CANBus and Process override it;
  `toggleConnection()` aborts when any device reports an in-flight dial, and
  `ConnectionManager::isConnecting` (NOTIFY `connectingChanged`, published by the same
  idempotent snapshot) drives the toolbar button's "Connecting…" label. Modbus mirrors
  Network's timer-driven dial (10 refusal retries at 300 ms, 15 s timeout, `close()`
  cancels; RTU gets one attempt) and its 500 ms endpoint-edit reopen debounce (host, port,
  protocol, serial parameters — a closed driver never dials on its own). Process reports
  the launch phase and the pipe's wait-for-writer window as connecting instead of faking an
  open channel. Audio arms miniaudio's stopped notification while open: a backend-initiated
  stop (device yanked, exclusive-mode steal) disconnects the device instead of streaming
  silence, and `closeDevice()` disarms it before `ma_device_uninit` so teardown's own stop
  never re-enters. `linkState()` reports `connected`, `connecting` or `idle` (connected
  wins when a live session and a dialing device coexist); `io.getStatus` mirrors it.
- **Network TCP dials asynchronously.** `open()` starts `connectToHost()` and returns true
  ("attempt started"); a refused dial retries up to 10 times with a 300 ms backoff (a server a
  control-script onConnect() just launched needs seconds to listen), a 15 s per-attempt timer
  bounds a hung dial, and `close()` cancels everything — nothing may redial after it returns.
  Success reaches the UI via `stateChanged` → `configurationChanged` →
  `refreshConnectedState()`; failure via a queued error box + `disconnectDevice(this)`.
  **Writes flow during the dial**: `DeviceManager::write` accepts data while
  `isConnecting()` and QTcpSocket buffers it until the connect lands, so a control script's
  `io.connect()` + `writeData()` sequence works without waiting out the dial (the ISS example
  broke silently without this). An endpoint edit (address/port/socket type/multicast) on a
  live driver reopens it after a 500 ms debounce; a closed driver never dials on its own.
- **`sessionClosed` means the USER (or an API client / player takeover) ended the session** —
  it fires only from the explicit `disconnectDevice()` path. Driver-initiated drops,
  `rebuildDevices` churn, and failed dials never emit it: `API::ProcessLauncher` reaps every
  script-launched helper on this signal, and those helpers usually serve the very link that
  is dropping or retrying (the dual-drone example died to a source-0 drop reaping the helper
  while source 1 was still dialing). A drop is a link event; the session outlives it.
- **`connectDevice(int)` reports the outcome itself.** `DeviceManager::open()` returns the
  driver's verdict instead of discarding it, and `connectDevice(int)` passes that to
  `onDeviceOpenFinished(deviceId, ok, reason)` — the only thing driving the spec-0035
  diagnostics auto-trigger now that no signal reports an open. Use the open call's return value,
  **never `isOpen()`**: MQTT, TCP and BLE dial asynchronously, so `isOpen()` is still false when
  a perfectly good attempt returns, and diagnostics would probe on every connect. The
  consequence is that a *later* async failure does not auto-trigger anything; the synchronous
  refusals diagnostics care about most (missing port, permissions) do. The reason string is
  never shown — diagnostics ignore it and the driver surfaces its own error.
- **Nothing reports a drop centrally, but every drop must reach the UI.** A driver that loses
  its link either calls `disconnectDevice(this)` with a **queued** error box (`UART` and
  `Network` are the reference; a synchronous modal inside an open() or error stack spins a
  nested event loop mid-emission) or guarantees a `configurationChanged` emission on the state
  transition (`Modbus`, `CANBus`, `MQTT`). BLE hooks `QLowEnergyController::errorOccurred`
  (a failed dial emits no `disconnected`); Process marshals a pipe-peer close to
  `onPipeClosed()` from the read thread. CANBus rate-limits its error box (one per 5 s) so a
  flapping bus cannot stack a modal storm.
- **`rebuildDevices()` reacts to real transitions only.** It is wired to
  `LemonSqueezy::activatedChanged`, which since 2026-08-04 fires only when
  `CommercialToken` validity actually flipped (`notifyEntitlementMaybeChanged()`), and to the
  project structure/operation-mode signals. It coalesces reentrant triggers into one queued
  follow-up, and the connect/disconnect fan-outs iterate id snapshots because a close can spin
  the event loop into another rebuild.

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

`ConnectionManager::onDeviceOpenFinished(deviceId, ok, reason)` is the auto-trigger, called
straight from `connectDevice(int)` once the synchronous open returns: on failure it resolves the
failing device's bus and calls `onOpenFailed()` (instant checks always, probing run only outside
the 30 s per-bus window); on success it calls `onOpenSucceeded()`, which clears that bus's cached
results.

## File Transmission (Pro)

`IO::FileTransmission` + `IO::Protocols::*`: controller +
XMODEM/YMODEM/ZMODEM. Incoming data routes from `ConnectionManager::onRawDataReceived` →
`FileTransmission::onRawDataReceived` (guarded by `active()`). Protocols emit
`writeRequested(QByteArray)`; controller calls `ConnectionManager::writeData()`.
