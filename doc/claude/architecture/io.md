# Architecture — IO & Drivers

> Part of the architecture corpus ([index](../architecture.md)). Read this file in full
> before touching `core/Devices/IO/` driver, manager, or protocol code. New drivers go through
> the `ss-new-driver` skill (BluetoothLE is the canonical reference).

## IO Architecture — No Singleton Drivers

- 14 drivers, **public ctors**, no `static instance()`.
- `ConnectionManager` (singleton, `Cpp_IO_Manager`) owns one **UI-config** instance per type:
  `instance().uart()`, `.network()`, `.bluetoothLE()`, etc. QML context properties
  (`Cpp_IO_Serial`, etc.) point at these.
- `IO::DriverFactory::create()` (spec 0070) makes **fresh** instances for live connections, owned by `DeviceManager`.
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

## Opening a Link — One Synchronous Call, Several Async Dials Behind It

`DeviceManager::open(mode)` starts the `FrameReader` if it is null and then calls
`m_driver->open(mode)` directly. There is no orchestration layer: `DeviceManager` owns no
task runner, and nothing sits between `ConnectionManager::connectDevice()` and the driver.
What *has* changed (spec 0075) is how many drivers finish inside that call: most now return
"attempt started" and settle later through the `openFinished` latch. The call is still
synchronous; the verdict often is not.
<!-- claim-verify off -->
The spec-0034 `IO::ConnectionFlows` layer and the hook family it drove (`supportsAsyncOpen`,
`beginOpen`, `abortOpen`, `openTimeoutMsec`, `linkDropped`) were removed 2026-07-30; spec
0034's docs describe a design that is no longer in the tree.
<!-- claim-verify on --> The only async-open surface
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
  connect-request bookkeeping now lives in `IO::ConnectFanOut` (the facade's `m_fanOut`
  member) and `concludeConnectRequest()` survives only to
  settle the wait cursor and to make `toggleConnection()` treat an in-flight request as
  "connected" so the button aborts instead of stacking a second attempt.
- **Async dials are visible through `HAL_Driver::isConnecting()`** (default `false`). Eleven
  classes override it: `Network` (`m_dialPending`, now true for TCP as well as WebSocket and
  HTTP; only UDP settles synchronously), `Iec104` (`m_dial.active()`), `Modbus`, `MQTT`,
  `OpcUa`, `S7`, `EthernetIp`, `CANBus`, `BluetoothLE`, `Process`, and the ctest double
  <!-- claim-verify off -->
  `Test::FakeDriver` (`app/tests/support/`).
  <!-- claim-verify on -->
  UART, USB, Audio and HID do not override it: their opens settle inside the
  call. (`OpcUaSession::isConnecting()` exists too but is not an override — the session is not a
  `HAL_Driver`.) `toggleConnection()` aborts when any device reports an in-flight dial, and
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
- **The dial doctrine (spec 0050, rebuilt in spec 0075).** The doctrine is unchanged: a
  THROWAWAY probe socket per attempt (5 s deadline, 250 ms pace on refusal, covering a
  control-script helper's bind window), then the driver's own socket connects exactly once, with
  its readyRead/errorOccurred handlers wired only on success. **Never abort-and-redial a
  long-lived run-loop-registered socket**: stale CFSocket sources fire into the freed engine and
  crash on macOS (observed 2026-08-10; same family as the 2026-06 socket ABA race). What changed
  is that none of it blocks the GUI thread any more. It lives in **`IO::AsyncTcpDial`**
  (`core/Devices/IO/AsyncTcpDial.{h,cpp}`), a GUI-thread QObject that sequences one dial:
  `QHostInfo::lookupHost` (skipped when the host is already an address literal), addresses
  reordered **IPv4 first** (a `localhost` resolving `::1` ahead of an IPv4-only listener used to
  cost a whole attempt), an optional QTimer-paced refusal probe on throwaway sockets, then one
  `connectToHost()` on the caller's socket **to the resolved literal**, so `connectToHost` never
  runs its own synchronous resolver. `finished(bool ok, const QString& reason)` is emitted
  **exactly once per `start()`** under a single deadline covering resolution, probing and
  connect; `report()` tears its own state down *before* emitting, so a caller that restarts a
  dial from its own handler runs against an idle object; `cancel()` ends an attempt with **no**
  verdict (a user cancel is not an open failure) and the destructor cancels. `active()` is what
  the drivers publish as `isConnecting()`. Three entry points, because not every caller owns the
  socket: `start()` (full sequence, Network TCP), `startProbe()` (probe only, for stacks that own
  their own connect: Modbus TCP), and `startResolve()` (resolution only, `resolvedAddress()`
  carries the literal: `OpcUaSession`). `Iec104` calls `start()` with
  `setProbeEnabled(false)` — a strict 104 station permits ONE client and would count the probe
  socket as it. **MQTT does not use the helper**: it keeps a plain 15 s `QTimer` dial deadline
  that funnels into `failDial()`.
  <!-- claim-verify off -->
  `dialTcpBlocking()` no longer exists.
  <!-- claim-verify on -->
  `tst_async_tcp_dial` pins success, refusal, unresolvable host and cancel-emits-nothing.
- **A write issued during a dial is held, not lost.** The spec-0050 promise that a control
  script's `io.connect()` + `writeData()` sequence just works used to hold because
  `connectToHost()` had already been called and QTcpSocket buffered. With probe-then-connect
  nothing is connected yet, so `Network::write()` routes a TCP write made while `m_dialPending`
  into `queueTcpWrite()` (capped at 1 MiB; an over-cap write is refused whole) and flushes the
  buffer once on a successful verdict, clearing it on failure and on `closeTcp()`. The API
  write gates (`io.writeData`, the raw lane's `Server::deviceConnected()`) admit a write while
  `isConnecting()` for the same reason; a "Not connected" refusal there would drop it.
- **`io.connect`'s response flag is a public contract, and TCP changed meaning.**
  `IOManagerHandler::connect()` answers `connected: manager.isConnected()` immediately, so for an
  async bus that flag means "the attempt started", not "the link is up" — which is now also true
  for TCP. Read the verdict from `io.getStatus` / `linkState()` instead. Any future
  sync-to-async conversion owes the same correction to the docs, the AI corpus and
  `tests/integration/test_connection_verdicts.py`.
- **Every async dial failure must still reach `ConnectionManager::disconnectDevice(this)`** —
  BLE (`onControllerError`), Modbus (`failDial`), MQTT (dial-window `onErrorChanged`) all do —
  so a pending verdict settles and "connecting" always resolves; the earlier design stranded
  those verdicts and wedged the connect button. The prior async retry/watchdog stack
  (10x300 ms + 15 s + peerPort validation) bounced healthy links and earned a telehack.com
  IP ban; do not reintroduce it. An ESTABLISHED link that errors reports once and stays
  down — post-drop auto-recovery exists only as UART's opt-in auto-reconnect checkbox, and an
  auto-reconnect **keeps the session pause**: `ConnectionManager::ResumePolicy` is `Resume` for a
  user connect and `KeepPause` for a driver's own recovery, because an adapter blip is not a
  request to start streaming again into a session the user deliberately paused.
  **There is no reopen-on-config-edit machinery** (removed
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
  stored keys on project load, so gating on the active type silently drops settings). For the same
  reason `driverProperties()` emits **every** transport's rows unconditionally, so a project saved
  while on TCP keeps its WebSocket, HTTP and TLS settings. Only UDP still settles synchronously;
  **TCP, WebSocket and HTTP dial async**, so `isConnecting()` returns `m_dialPending` for those
  three. Their verdict funnels are
  `succeedDial()` / `failDial()`, and `failDial()` only REPORTS, because `onDriverOpenFinished`
  already closes the device on `ok == false` and tearing down there too double-closes. HTTP's opening
  request IS the connect verdict (no separate HEAD probe: many REST endpoints answer 405, and a
  POST source would fire two side-effecting requests per session) and its body publishes like any
  poll. Only ONE reply is ever in flight; an overlapping poll tick increments a skip counter and
  returns. A post-connect poll failure keeps the link UP, logs once per failure run, and counts
  (`pollsOk`/`pollsFailed`/`pollsSkipped`/`consecutiveFailures`, pulled by `io.network.getStatus`,
  never pushed). An HTTP response body is capped at **8 MiB** (`readCappedBody`), the first
  truncation of a run logged once. `urlForCurrentMode()` is the SINGLE validation rule shared by
  `configurationOk()` and `open()`. UDP reads check `readDatagram()`'s return: a failed read ends
  the pass instead of republishing whatever the reused buffer still held from the previous
  datagram, and a single pass is bounded at 256 datagrams.
- **OPC UA (specs 0066/0067) owns its stack, discovers before it dials, and publishes delta
  frames on a tick.** The protocol stack is `lib/open62541` (1.5.7 amalgamation) over
  `lib/mbedtls` (3.6 LTS), both vendored and statically linked; **`Qt6::OpcUa` is not used and
  not linked**, so the shipped package carries no Qt OPC UA module and no backend plugin, and the
  driver's capabilities are a property of THIS build rather than of the machine it runs on.
  `SS_ENABLE_OPCUA=OFF` (or a GPL build) hides the bus entirely.
- **`OpcUaSession` resolves the host itself before it hands open62541 a URL.**
  `UA_Client_connectAsync()` resolves inside itself with a synchronous getaddrinfo (upstream's own
  "TODO: Make this non-blocking"), so an unresolvable host froze the window for the resolver's
  timeout, which no dial deadline of ours could shorten (E5). `startResolution()` runs
  `AsyncTcpDial::startResolve()` first and `dialUrl()` substitutes the resolved literal, while
  `m_endpointUrl` keeps the typed hostname because that is what the certificate hostname check has
  to see. The pump cadence is adaptive: 10 ms while something is outstanding (dialing, a read in
  flight, a live subscription, queued reads or browses), 100 ms idle — three idle sessions used to
  cost 300 wake-ups per second between them.
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
  (`ClientCallScope` raises a per-session re-entrancy depth around every open62541 call, not
  only the pump — a flag that covered the pump alone let a failed dial destroy the client
  `connectAsync()` was still standing on) rather than freeing the object the stack is standing on. `OpcUaTypes.h` is the
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
- **Secure channels are configuration on the session (spec 0067 stage 2).** All six policies are
  supported (`None` through `Aes256_Sha256_RsaPss`, the `kPolicyUris` table in
  `OpcUaEndpointSelection.cpp`); `Basic128Rsa15` and `Basic256` are labelled deprecated and never
  auto-selected. **"Never auto-selected" is enforced by not scoring them at all**: a deprecated
  candidate the user did not explicitly configure is `continue`d past, so a server offering
  nothing else leaves the choice empty rather than being dialed over Basic128Rsa15. Scoring one
  at 0 against an initial best of -1 is what made a deprecated-only server auto-dial (E10);
  `tst_opcua_endpoint_selection` now carries that case.
  `selectBestEndpoint()` otherwise picks the most secure endpoint the chosen identity can use.
  `OpcUaSecurity` owns the per-INSTALLATION identity and trust store under
  `AppConfigLocation/OpcUa`: the client certificate is generated on first secure use and REUSED
  (a server operator trusts this installation once), the private key is written owner-only and
  never exported, and trusted server certificates are files keyed by SHA-256 fingerprint so a
  server that re-keys is a NEW decision. The session REPLACES open62541's trust-list check with its
  own `verifyCertificate` hook, and keeps the four refusal causes apart (untrusted, expired, not
  yet valid, hostname mismatch) because they have four different fixes. A refusal is still ONE
  verdict through `failDial()`; the trust prompt is emitted QUEUED and accepting only RECORDS the
  decision, so the reconnect is a new attempt with its own verdict. **TRUST is read before the
  hostname check**, so an accepted self-signed certificate dialed by IP is no longer refused for a
  hostname mismatch (E11). Identity is anonymous, username/password or an X.509 token.
  A password crosses an unencrypted channel only when the user has granted
  `OpcUaSecurity::plaintextPasswordAllowed()` — a per-INSTALLATION acknowledgement beside the
  trust store, **default off**, replacing the unconditional `allowNonePolicyPassword = true` that
  shipped every None-policy login's password in the clear without asking. It is deliberately NOT
  a driver property: `applyConnectionSettings` replays every key of a project's `connection`
  object through `setDriverProperty`, so anything exposed there would be granted by opening
  someone else's project. Nothing secret enters the project file:
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
- **The spec-0073 PLC pollers (S7, EthernetIp) are blocking pollers on a driver-owned
  QThread, dialed asynchronously.** `open()` posts `beginDial` to the worker
  (`Qt::QueuedConnection`) and returns immediately with `isConnecting()` true; the worker reports
  `dialFinished(ok, reason)` exactly once and the driver forwards it through `openFinished`. The
  GUI thread no longer blocks for the 10-13 s a dead controller costs. A verdict landing after
  the user closed the session is dropped by the `m_connecting` guard. Every protocol exchange
  still blocks until the controller answers, so the socket and the tag handles live on the worker
  thread only.
  **Both workers derive from `IO::Drivers::PolledPlcWorkerBase`**
  (`core/Devices/IO/Drivers/PolledPlcWorkerBase.{h,cpp}`, spec 0075 E8), which owns the
  protocol-independent half: the `std::atomic<bool>` abort latch, the poll timer, the
  change-latch table (`latchChannel` — an unchanged value costs no wire entry), the `OpcUaWire`
  delta encoder (`publishDirtySlots`, double-buffered frames, dirty marks consumed by the
  publish), the report-once link loss, the one-shot dial verdict, and the three pulled counters
  `readsOk` / `readsFailed` / `framesPublished`. Subclasses implement exactly three hooks:
  `connectToPlc()`, `pollTick()`, `releaseResources()`. What stays per driver is the protocol:
  `S7PollWorker` keeps the ISO-on-TCP handshake through the in-house `core/Protocols/S7/IsoTsap.h`
  + `S7Pdu` codec (spec 0076), its chunk plan, and its own two extra atomics `m_lastFault` /
  `m_itemErrors`;
  `EipPollWorker` keeps the vendored-libplctag seam (`kEipBackend`, so the TU reads the same with
  or without the lib; every `plc_tag_create`/read/destroy happens on the worker) and its
  dead-tick watchdog. `kEipBackend` is a label, not an injectable seam, which is why the worker
  suite (`tst_ethernetip_worker`) drives `PolledPlcWorkerBase` through a scripted stub instead.
  Both drivers are read-only — `write()` returns -1, there is no write path. Worker counters are
  ATOMIC, a deliberate, header-documented deviation from spec 0033's plain `quint64`, because the
  poll thread increments while the GUI samples at 1 Hz. A lost link emits `linkLost` and the
  driver queues `disconnectDevice(this)`.
- **Iec104 stays GUI-thread like OPC UA and DISCOVERS its point table.** `dialStation()` runs
  through `AsyncTcpDial` with the probe disabled (same one-verdict doctrine, reported through
  `openFinished`; `doClose()` cancels the dial so a cancelled attempt reports nothing), then
  sends STARTDT and a general interrogation;
  the station's answer builds the point table — nothing is configured — and
  `generateProject()` turns it into a project. TESTFR keepalives at t3 hold the link, and a
  tick publishes changed points. **Slot identity is `(ioa, typeId)`, not the IOA alone**
  (`Iec104Proto::slotKey()`, `m_slotForKey`): one address reported under two type ids is two
  slots, and a report's live `kind` overwrites a restored one, so a measured value and a
  single-point indication on the same address can no longer overwrite each other. Monitor
  direction only: no control direction exists, `write()` returns -1. The in-house stack lives in
  `core/Protocols/Iec104/Apci.h` + `Asdu` (spec 0076).
- **All three industrial pollers publish through the OPC UA wire lane.** Dirty slots encode
  into `OpcUaWire` delta frames (`wireTypeFor`, `kMaxTags` cap) latched by the same native
  template family, stamped with the poll's own capture time and clamped monotonic on the GUI
  side (`qMax(stampNs, m_lastStampNs + 1)`) so a rounded steady clock never hands the
  pipeline two frames with one timestamp.
- **Sparkplug B rides the MQTT driver, split by direction (specs 0073/0074).** Inbound:
  `SparkplugSession` (under `Drivers/MQTT/`) is a Qt-Core-only, QObject-free state machine
  that turns the `spBv1.0` namespace into a flat table of latched slots for the same OPC UA
  delta encoder — slot indices NEVER move under a rebirth, counters are polled (spec 0033),
  no drop is silent. **`reset()` keeps the slot table** and clears only birth state, buffered
  traffic, values and counters: clearing it on a reconnect renumbered every slot, and
  `sparkplugStateChanged(connected)` sets the group filter on every connect (E2). A filter change
  is safe for the same reason a slot key carries its own group, so an old-group slot can never
  collide with a new one. The table survives the app too: `Keys::SparkplugSlots` persists it in
  the MQTT connection block (`slotsJson()` / `restoreSlots()`), and because `QJsonObject`
  iterates in sorted key order, `applyConnectionSettings` applies `sparkplugEnabled` <
  `sparkplugGroupId` < `sparkplugSlots`, so the table is restored after the filter is set. A
  restore is refused whole once the session already holds slots of its own, so the UI instance
  can never clobber a live table. Outbound: `MQTT::SparkplugPublisher` owns the edge-node lifecycle
  (metric registry, alias table, `bdSeq`/`seq`, birth/data/death payloads) as pure
  {topic, payload} pairs with no I/O and no QObject, so the ctest tier drives the whole state
  machine without a broker. Aliases are stable once assigned — a multi-source project must
  never renumber a live host's learned alias table, and a metric-set change triggers a
  rebirth instead (spec 0074).
- **`CanBackends` registers libusb/serial adapters as synthetic CANBus plugins** — GsUsb
  (candleLight), Seeed/Waveshare USB-CAN Analyzer (CH340 serial), SLCAN/LAWICEL — each a
  `QCanBusDevice` backend keyed by a plugin key beside Qt's own plugins. The two **serial**
  adapters share `IO::Drivers::SerialCanBackendBase`
  (`core/Devices/IO/Drivers/CANBus/SerialCanBackendBase.{h,cpp}`), which owns everything that is not
  the wire protocol: the `QSerialPort`, the open/close sequences with their open-ack timeout, the
  bounded receive buffer (`kMaxRxBufferSize` 65536; over-cap clears the buffer and counts a drop)
  and the fatal-versus-ignorable `errorOccurred` classification, so a dead adapter reports the
  same way whichever one is speaking. Subclasses supply only `validateBitrate`, `sendInit`,
  `drainBuffer` and optionally `openReplyIsError` / `sendShutdown`: `SlcanBackend` keeps the
  LAWICEL ASCII grammar (with the id and DLC parse flags now separated, D8, and the open verdict
  read from the adapter's BEL reply, D19), `SeeedCanBackend` the analyzer's variable-length
  packets. `GsUsbCanBackend` is NOT a subclass — it is USB, not serial.
  `core/Protocols/CAN/CanReassembly.h` (spec 0076) holds the two fixed-cap reassemblers (J1939-21 TP: TP.CM announcements plus BAM and
  RTS/CTS sessions; ISO 15765-2: FirstFrame + ConsecutiveFrames) with pulled counters
  (spec 0033), so >8-byte PGNs and multi-frame diagnostics decode like single frames instead
  of silently never appearing.
- **`sessionClosed` means the USER (or an API client / player takeover) ended a session that
  existed** — it fires only from the explicit no-argument `disconnectDevice()` path, and only
  when a session was actually open (C13). Driver-initiated drops, a cancelled dial,
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
  async failures too. There is NO polling sweep:
<!-- claim-verify off -->
  `settlePendingDialVerdicts()` is gone;
<!-- claim-verify on -->
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
  when the firmware advertises it (spec 0049; wire vocabulary in
  `core/Protocols/CAN/GsUsbProtocol.h`, spec 0076, shared
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

## ConnectionManager's Sub-objects

The facade owns its concerns as member sub-objects under `core/Devices/IO/ConnectionManager/`, one
class per `.h/.cpp` pair (`ConnectFanOut`, `DeviceIoRouter`, `DeviceTableQuery`, `DriverFactory`,
`DriverUiRegistry`, `ReplyCapture`, `StreamConfigBuilder`, `StreamWorkerPool`, `UiDriverSync`).
The four that matter on the connect path:
What stays in `ConnectionManager.cpp` is connect/disconnect orchestration, because
it needs `QObject::sender()` (`onDriverOpenFinished` resolves which driver reported), `Q_EMIT`,
`connect(...)` with `this` as context, and the facade's own Q_INVOKABLE per-bus QML accessors.

- **`ConnectFanOut` (`m_fanOut`)** — the connect-request bookkeeping: request lifecycle, pending
  dial ids (`notePendingDial` / `takePendingDial`), the latched connected/connecting snapshots and
  the wait cursor. It queries no device and emits nothing.
- **`DeviceIoRouter` (`m_io`)** — what crosses the device link and how it is framed: the
  delimiters and checksum the readers are rebuilt from, the inbound payload fan-in to the console
  and the API/session/MQTT/gRPC taps, and the outbound write path with its reply capture.
  ConnectionManager's byte path is this class.
- **`DeviceTableQuery` (`m_query`)** — every read over the live device table: open counts,
  `linkState()`, the 1 Hz `linkStats()` sample, the configuration verdict and the id lookups the
  connect fan-outs iterate. Read-only by construction, so nothing here can mutate a device or emit
  a signal. **`IO::LinkStats` is declared in `DeviceTableQuery.h`**, not in `ConnectionManager.h`;
  `ConnectionManager::linkStats()` forwards.
- **`StreamWorkerPool`** — the per-source `IO::StreamWorker` lifecycle joined first in
  `ModuleManager::stopFrameConsumerWorkers()`.

Other IO invariants worth naming at the point of action:

- **UART's two error decisions are pure and extracted.** `IO::Drivers::UartPolicy`
  (`UART/UartPolicy.h`, header-only) holds `isFatalPortError(error, customPath)` and
  `shouldAutoReconnect(error, enabled)`. A custom device path is exempt from
  `UnsupportedOperationError` **only** — never from `ResourceError`, whose exemption left the port
  "open" after an unplug with `write()` still returning byte counts (D5). `tst_uart_policy` pins
  both without a real port.
- **USB advanced transfers are consent-gated.** `setTransferMode(AdvancedControl)` refuses
  without a recorded `USB/advancedTransferConsent`, reporting through the log plus a **queued**
  NotificationCenter warning rather than a modal: a modal blocked a non-interactive caller and
  asked a question nobody was there to answer (D4, same class as R5.5).
- **Audio playback goes through an SPSC ring.** `Audio/PlaybackRing.h` is a fixed-capacity byte
  ring between the GUI writer and the real-time callback: a write that does not fit is refused
  whole and counted, a short read zero-fills and counts an underrun, and both counters are pulled
  (spec 0033). No allocation, no lock, no blocking on the callback side. A capture-only session no
  longer fails because the machine has no output device (D2).
- **hidapi init/exit is refcounted.** `hid_init`/`hid_exit` sit behind an internal refcount, because
  a live instance's destructor used to tear down the UI instance's IOHIDManager (D11); `open()`
  closes first.
- **Process closes asynchronously.** `doClose()` terminates and kills on a timer instead of
  blocking two seconds on a child that ignores SIGTERM, `ps` enumeration is asynchronous, and the
  crash double-drop is guarded because a crashing process reports through BOTH `finished()` and
  `errorOccurred()`.
- **Modbus RTU framing is its own Qt-Core-only unit.**
  `core/Protocols/Modbus/ModbusRtuCodec.{h,cpp}` (spec 0076) holds
  exactly two free functions, `functionCodeForType()` and `appendCrc()` (CRC-16/Modbus, low octet
  first), so what a consumer validates is testable without a device. **The request cap is
  type-aware**: `ModbusRegisterGroups::maxCountForType()` gives FC01/FC02 their own 2000-bit
  ceiling against FC03/FC04's 125 registers, enforced at both `add()` and `restore()`; sharing the
  register ceiling refused four fifths of a legal coil read (E12). A failed poll publishes a
  `[unit, fc, 0]` placeholder and steps the cursor from BOTH failure exits, so a dropped reply
  cannot put two frames of the same group back to back (E3).

## The Async Task-Tree Engine (`core/Core/Async/`, spec 0076)

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
XMODEM/YMODEM/ZMODEM (the protocol classes live in `core/Protocols/FileTransfer/`, spec 0076;
the facade stays in `core/Devices/IO/FileTransmission.h/.cpp`). Incoming data routes from
`ConnectionManager::onRawDataReceived` →
`FileTransmission::onRawDataReceived` (guarded by `active()`). Protocols emit
`writeRequested(QByteArray)`; controller calls `ConnectionManager::writeData()`.
