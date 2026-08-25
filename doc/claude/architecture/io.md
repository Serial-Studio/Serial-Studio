# Architecture — IO & Drivers

> Part of the architecture corpus ([index](../architecture.md)). Read this file in full
> before touching `app/src/IO/` driver, manager, or protocol code. New drivers go through
> the `ss-new-driver` skill (BluetoothLE is the canonical reference).

## IO Architecture — No Singleton Drivers

- 11 drivers, **public ctors**, no `static instance()`.
- `ConnectionManager` (singleton, `Cpp_IO_Manager`) owns one **UI-config** instance per type:
  `instance().uart()`, `.network()`, `.bluetoothLE()`, etc. QML context properties
  (`Cpp_IO_Serial`, etc.) point at these.
- `createDriver()` makes **fresh** instances for live connections, owned by `DeviceManager`.
- `configurationOk()` checks the **UI** driver, not the live one. UI driver's
  `configurationChanged` forwards to `ConnectionManager::configurationChanged`. All drivers
  must `Q_EMIT configurationChanged()` from their ctor.
- Live drivers may have empty device lists. UART/Modbus call `refreshSerialDevices()` /
  `refreshSerialPorts()` in `open()` if empty.

## Typed Stream Lane (specs 0051, 0055)

Dense typed sample sources still bypass the frame *parser*, but since spec 0055 they no longer have
a lane of their own downstream: they publish the same `DataModel::DataBlock` everything else does.
Two things decide the lane: the driver (`HAL_Driver::isStreamCapable()`, true for Audio) and the
per-source project override `streamLane` (`""`/absent = auto, `"on"`, `"off"`); `IO::streamLaneOn()`
is the one resolver. A lane-active driver publishes `IO::SampleBlock` (interleaved float32 +
channel count + `t0` + `dt`) through `publishSampleBlock()`; a lane-off audio source keeps the
legacy CSV text path, so the branch inside `Audio::processInputBuffer` is deliberate, not dead code.

- **`IO::StreamWorker`** (GUI facade) owns one `QThread` per stream source and the pause atomic
  (`setPaused` mirrors the session pause, the stream-lane counterpart of `PipelineHost::routeFrames`'
  gate); `ConnectionManager::rebuildStreamWorkers()` creates them beside the DeviceManagers, runs
  again at the connect edge (`connectDevice()`) so the config captures the driver settings the
  session actually opens with, and `stopStreamWorkers()` joins them FIRST in
  `ModuleManager::stopFrameConsumerWorkers()`. `stop()` is: disconnect the feed, queue engine
  teardown (script states die on their own thread), quit, bounded 5 s wait, then
  **warn-and-abandon** on a hung Fast-mode script (R21, spec 0046 precedent).
- **`IO::StreamProcessor`** (worker-affine) does every per-sample thing: channel extraction into a
  reused float64 scratch, `transform_block(samples, info)` once per block (frozen R9 info payload)
  or the per-sample `transform(value)` fallback, FFT ring append, latest values, and the block's
  columns. Safe/Fast mode is the project's `luaFastMode` (interpreter + count hook + 100 ms deadline
  / JIT + no hook); `ffi` and `jit` are never opened. A failed or aborted transform counts an error
  and the block falls back to raw. Stream transforms get the shared data-table API routed through
  the `readTableView`/`writeTableStore` marshal; a re-entrant block delivered mid-wait is dropped
  and counted, never processed concurrently.
- **Everything leaving the worker is per block, and goes to ONE place (spec 0055 D8):**
  `blockReady(DataBlockPtr)` queued to `FrameBuilder::ingestStreamBlock`, plus `latestValuesReady`
  queued to `FrameBuilder::ingestStreamValues` for the data-table store's single writer. The worker
  owns no display ring and fans out to no sink: routing dense blocks straight to the sinks from the
  GUI would give each sink's SPSC queue a second producer, which is exactly why two sinks per format
  used to exist. Blocks are pooled (`kBlockPoolSlots`); pool or ring exhaustion drops a whole block
  and counts it, which is the only backpressure -- the worker never strides or caps a source's rate.

## Opening a Link — Synchronous, Per-Driver

`DeviceManager::open(mode)` starts the `FrameReader` if it is null and then calls
`m_driver->open(mode)` directly. There is no orchestration layer: `DeviceManager` owns no
task runner, and nothing sits between `ConnectionManager::connectDevice()` and the driver.
The spec-0034 `IO::ConnectionFlows` layer and the hook family it drove (`supportsAsyncOpen`,
`beginOpen`, `abortOpen`, `openTimeoutMsec`, `linkDropped`) were removed 2026-07-30; spec
0034's docs describe a design that is no longer in the tree. The only async-open surface
today is spec 0050's bare `HAL_Driver::openFinished(bool, reason)` verdict signal — a
namesake of a removed 0034 hook, but a different, much smaller thing (see below).

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
- **Spec-0050 dial doctrine (2026-08-10).** Network TCP connects synchronously: `open()`
  blocks in `dialTcpBlocking()` under the connect fan-out's wait cursor and the return value
  is the final verdict — no `isConnecting()` override, no dial timers, no pending-dial
  verdicts. The endpoint wait uses a THROWAWAY probe socket per attempt (5 s deadline,
  250 ms pace on refusal, covering a control-script helper's bind window); the driver's own
  socket then connects exactly once, and its readyRead/errorOccurred handlers are wired only
  on success. **Never abort-and-redial a long-lived run-loop-registered socket**: stale
  CFSocket sources fire into the freed engine and crash `readFromSocket` on macOS (observed
  2026-08-10; same family as the 2026-06 socket ABA race). Modbus TCP cannot block
  (QModbusClient limitation) but runs the same throwaway pre-probe inside `open()`, then
  `connectDevice()` dials once; a dial setback fails once for both protocols. **Every async
  dial failure must reach `ConnectionManager::disconnectDevice(this)`** — BLE
  (`onControllerError`), Modbus (`failDial`), MQTT (dial-window `onErrorChanged`) all do —
  so a pending verdict settles and "connecting" always resolves; the earlier design stranded
  those verdicts and wedged the connect button. The prior async retry/watchdog stack
  (10x300 ms + 15 s + peerPort validation) bounced healthy links and earned a telehack.com
  IP ban; do not reintroduce it. An ESTABLISHED link that errors reports once and stays
  down — post-drop auto-recovery exists only as UART's opt-in auto-reconnect checkbox.
  A control script's `io.connect()` + `writeData()` sequence just works: `open()` returns
  with the link established. **There is no reopen-on-config-edit machinery** (removed
  2026-08-10): connection settings are UI-locked while connected or dialing
  (`SetupPanes/Hardware.qml` StackLayout gate; BLE's post-connect pickers exempt), and
  `ProjectModel::setSource0ConnectionSettings` no-ops on identical settings so persistence
  echoes cannot churn undo history or autosave.
- **The Network driver is one class, four transports, five TUs (spec 0068).** `Network.h` is the
  facade; `Network.cpp` holds the ctor/dtor, the four-way dispatchers and the property model, and
  `Drivers/Network/Network{Tcp,Udp,WebSocket,Http}.cpp` each own one transport end to end. The
  selector is the driver's own `Network::SocketType` enum (`Tcp=0, Udp=1, WebSocket=2, Http=3`)
  because Qt's socket-type enum cannot name the last two; the NUMBERING is the socket-type index
  persisted by project files, `io.network.setSocketType`, the CLI and the AI corpus, so it is
  append-only. Two dispatchers do NOT branch on the current type: `close()` tears down
  every transport (a type changed while a link is up would otherwise strand the open socket) and
  `setDriverProperty()` offers each key to every transport (`applyConnectionSettings()` replays all
  stored keys on project load, so gating on the active type silently drops settings). TCP and UDP
  keep their synchronous verdicts; **WebSocket and HTTP dial async**, so `isConnecting()` is
  overridden and returns `m_dialPending`, true only for those two. Their verdict funnels are
  `succeedDial()` / `failDial()`, and `failDial()` only REPORTS, because `onDriverOpenFinished`
  already closes the device on `ok == false` and tearing down there too double-closes. HTTP's opening
  request IS the connect verdict (no separate HEAD probe: many REST endpoints answer 405, and a
  POST source would fire two side-effecting requests per session) and its body publishes like any
  poll. Only ONE reply is ever in flight; an overlapping poll tick increments a skip counter and
  returns. A post-connect poll failure keeps the link UP, logs once per failure run, and counts
  (`pollsOk`/`pollsFailed`/`pollsSkipped`/`consecutiveFailures`, pulled by `io.network.getStatus`,
  never pushed). `urlForCurrentMode()` is the SINGLE validation rule shared by `configurationOk()`
  and `open()`.
- **OPC UA (specs 0066/0067) owns its stack, discovers before it dials, and publishes delta
  frames on a tick.** The protocol stack is `lib/open62541` (1.5.7 amalgamation) over
  `lib/mbedtls` (3.6 LTS), both vendored and statically linked; **`Qt6::OpcUa` is not used and
  not linked**, so the shipped package carries no Qt OPC UA module and no backend plugin, and the
  driver's capabilities are a property of THIS build rather than of the machine it runs on.
  `SS_ENABLE_OPCUA=OFF` (or a GPL build) hides the bus entirely.
- **`OpcUaSession` is the only object that sees a `UA_` type.** It is a plain `QObject` affine to
  the driver's (GUI) thread: a `QTimer` calls `UA_Client_run_iterate()`, open62541 dispatches its C
  callbacks from inside that call, and static trampolines recover the session from
  `config->clientContext` and immediately translate into Qt types. Nothing is `moveToThread`'d and
  the amalgamation ships at `UA_MULTITHREADING 100`, which adds a recursive client lock and
  thread-local state but spawns NO threads of its own -- that is why the callbacks stay inline,
  and it is also what makes it legal for the tag model to issue a new browse from inside a browse
  reply. **Teardown ordering is load-bearing**:
  the pump stops and `clientContext` is nulled BEFORE `UA_Client_delete()`, every trampoline
  null-checks, and a `close()` arriving from inside a callback defers the delete to a queued call
  (`s_inPump`) rather than freeing the object the stack is standing on. `OpcUaTypes.h` is the
  transport-neutral vocabulary (Qt Core only, so the ctest tier links it without the stack) and
  `OpcUaMarshal` is the single seam, pinned by `tst_opcua_marshal`.
- **Requests are answered POSITIONALLY, so the session stages them.** The Read service returns
  values in request order with no node id on the reply: `PendingRead` holds the rows and merges on
  arrival. A browse carries a caller-supplied `BrowseQuery::token` back with its reply, because the
  picker browses one node for up to three different reasons (level expansion, has-children probe,
  units lookup) and the node id alone cannot route the answer. `MaxNodesPerRead` is read by the
  session itself and consumed internally. Subscriptions are ONE
  `UA_Client_Subscriptions_create_async` plus ONE `UA_Client_MonitoredItems_createDataChanges_async`
  for every tag; the **tag index rides in the monitored-item context**, never a map keyed on the
  returned id, because open62541 registers items locally before the create reply arrives and a map
  would drop the initial value of every slow-changing tag.
- **The dial doctrine of spec 0066 is preserved verbatim.** With no discovered endpoint selected,
  `open()` runs `discoverEndpoints()` FIRST (`m_pendingDial`) over a bare None-policy channel, which
  the OPC UA specification requires every server to accept for the Discovery services.
  `dialEndpoint()` then dials the discovered endpoint with the user's typed host:port substituted:
  servers advertise their own hostname, which rarely resolves from the engineering laptop.
  Selection requires `endpointUsable()` AND `endpointAcceptsToken(authMode)`, and an explicit pick
  survives re-discovery. The ONE verdict is `connected()` → `reportOpenFinished(true)` or the
  `failDial()` funnel (session `connectFailed`, 15 s `m_dialTimer`, discovery failure) →
  `reportOpenFinished(false)`; an established drop queues `disconnectDevice(this)`.
- **Secure channels are configuration on the session (spec 0067 stage 2).** All five policies are
  supported; `Basic128Rsa15` and `Basic256` are labelled deprecated and never auto-selected, and
  `selectBestEndpoint()` otherwise picks the most secure endpoint the chosen identity can use.
  `OpcUaSecurity` owns the per-INSTALLATION identity and trust store under
  `AppConfigLocation/OpcUa`: the client certificate is generated on first secure use and REUSED
  (a server operator trusts this installation once), the private key is written owner-only and
  never exported, and trusted server certificates are files keyed by SHA-256 fingerprint so a
  server that re-keys is a NEW decision. The session REPLACES open62541's trust-list check with its
  own `verifyCertificate` hook, and keeps the four refusal causes apart (untrusted, expired, not
  yet valid, hostname mismatch) because they have four different fixes. A refusal is still ONE
  verdict through `failDial()`; the trust prompt is emitted QUEUED and accepting only RECORDS the
  decision, so the reconnect is a new attempt with its own verdict. Identity is anonymous,
  username/password (`allowNonePolicyPassword` is what lets a password cross an unencrypted
  channel at all) or an X.509 token. Nothing secret enters the project file:
  `DriverProperty::Password` keeps the password out, and only the PATHS of a user certificate and
  key are persisted.
- **On Connected the driver subscribes every tag at once.** Refused tags go to `m_polledTags`
  (batched read, chunked to `MaxNodesPerRead`, ONE read outstanding, because a queued read behind
  a slow PLC grows latency without bound), every refused item is refused *individually* and only an
  all-refused verdict flips `m_pollMode`. A 1 Hz watchdog falls back to polling when nothing has
  arrived for `kSilenceFactor` publishing periods: a server that silently drops the subscription
  otherwise leaves the dashboard frozen and the status reading "Subscribed". The revised publishing
  interval is adopted from the session and drives both timers; a live interval change goes through
  `UA_Client_Subscriptions_modify_async`. Value quality is decided by the OPC UA **severity bits**
  (Good and Uncertain publish, Bad keeps the last good value and lands in `badTags()`), never by
  `!= Good`. `m_frameTimer` encodes only dirty slots into one `OpcUaWire` frame
  (`[version][index u16][type u8][payload]*`, capped at `kMaxFrameBytes`, cursor rotated so high
  indices cannot starve, buffer moved to the pipeline and re-reserved per tick) and calls
  `publishReceivedData()` stamped with the earliest source timestamp mapped through a per-connect
  steady-clock offset PLUS the server-to-local offset sampled at connect (un-NTP'd PLCs are
  followed, not rejected; the stamp never goes backwards). The slot layout carries each slot's wire
  type, so it is independent of `m_tags`; a tag edit during a live session is deferred and applied
  on close. The `opcua` native template (`BinaryTemplates.cpp`, `BUILD_COMMERCIAL`) latches the
  frame by the project's `schema` param (entries carry the node id) and rejects duplicate indices;
  `tst_opcua_wire` pins the vocabulary. **`OpcUaTagModel` is strictly lazy**: one Browse per
  expansion plus ONE batched Read for that level, Objects *and* Variables expandable (PLC structs
  expose members as child Variables), units/EURange resolved only for ticked tags through a bounded
  queue. Crawling the address space on open is what makes a picker unusable on a 100k-node gateway.
  Only the UI-config instance persists (`setPersistent(false)` on the live one); the password lives
  in `MQTT::CredentialVault` under the `opcua` scope. Diagnostics are pulled counters read by
  `io.opcua.getStatus`.
- **`sessionClosed` means the USER (or an API client / player takeover) ended the session** —
  it fires only from the explicit `disconnectDevice()` path. Driver-initiated drops,
  `rebuildDevices` churn, and failed dials never emit it: `API::ProcessLauncher` reaps every
  script-launched helper on this signal, and those helpers usually serve the very link that
  is dropping or retrying (the dual-drone example died to a source-0 drop reaping the helper
  while source 1 was still dialing). A drop is a link event; the session outlives it.
- **The verdict has ONE owner per attempt (spec 0050).** Sync drivers: the `open()` return
  value, passed by `connectDevice(int)` to `onDeviceOpenFinished(deviceId, ok, reason)`.
  Async drivers (BLE, Modbus, MQTT, Process, async CAN plugins): the
  `HAL_Driver::openFinished(ok, reason)` signal, emitted **exactly once per attempt**
  through the base-class latch (`armOpenReport()` by the manager before `open()`;
  `reportOpenFinished()` by the driver on BOTH outcomes; disarmed on first report, on a
  synchronous settle, and on user cancel). `ConnectionManager::onDriverOpenFinished` settles
  the pending id, quietly closes the device on a failed dial (never `sessionClosed`), and
  forwards to `onDeviceOpenFinished` — so the spec-0035 diagnostics auto-trigger now sees
  async failures too. There is NO polling sweep: `settlePendingDialVerdicts()` is gone;
  never re-add a "check isOpen() later" settlement path. A driver that dials async and does
  not report both outcomes wedges the connect button — that is the bug class this design
  exists to kill. The reason string is never shown — diagnostics ignore it and the driver
  surfaces its own error.
- **Nothing reports a drop centrally, but every drop must reach the UI.** A driver that loses
  its link either calls `disconnectDevice(this)` with a **queued** error box (`UART` and
  `Network` are the reference; a synchronous modal inside an open() or error stack spins a
  nested event loop mid-emission) or guarantees a `configurationChanged` emission on the state
  transition (`Modbus`, `CANBus`, `MQTT`). BLE hooks `QLowEnergyController::errorOccurred`
  (a failed dial emits no `disconnected`); Process marshals a pipe-peer close to
  `onPipeClosed()` from the read thread. CANBus rate-limits its error box (one per 5 s) so a
  flapping bus cannot stack a modal storm. The CANable (gs_usb) backend negotiates CAN FD
  when the firmware advertises it (spec 0049; wire vocabulary in `GsUsbProtocol.h`, shared
  with `tst_gsusb_protocol`), detects mid-session unplug in its own read loop, and feeds a
  process-lifetime libusb hot-plug callback (delivered by a dedicated event-pump thread on the
  shared context — libusb only dispatches hot-plug from `libusb_handle_events()`) whose only
  action is a queued, debounced interface-list refresh on the UI driver — never USB work or Qt
  state on the callback thread; `~CANBus()` disarms the notifier via
  `clearHotplugNotifier()` so a late callback never touches a freed driver.
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
