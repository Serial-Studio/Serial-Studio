---
spec: 0043-connection-reliability
title: Connection reliability sweep (post-0034 removal, trial/licensing loop)
status: in-progress  # implemented same-day; maintainer ACs pending
created: 2026-08-04
author: Alex Spataru (executed by Claude)
---

# Spec 0043 — Connection reliability sweep

> Retroactive spec: the analysis, fixes, and remaining gap register from the 2026-08-04
> reliability sweep of the IO connection layer and licensing signal paths. The work targets
> one requirement: the app must be trustworthy on a test stand or ground station — the
> connect button always tells the truth, and no background machinery may take the link away
> from the user or refuse to give it back.

## Problem

After the spec-0034 connection-flow layer was removed (38c9ef66) and its follow-up
(c608efbf), users saw two failure classes:

1. **Connect button stops reflecting reality.** Connected-state emissions were scattered
   across five call paths with per-path `had_request`/`settled` bookkeeping; async dialers
   (TCP, BLE) had holes where a failed dial reported nothing at all.
2. **Connect/disconnect loop the user cannot stop.** Licensing signals rebuilt live devices
   on every emission, and emissions fired even when nothing changed: `Trial::onServerReply`
   emitted `enabledChanged` on *every* reply (network errors included), forwarded blanket
   into `LemonSqueezy::activatedChanged`, wired into `ConnectionManager::rebuildDevices`,
   whose tail auto-reconnects. An offline trial machine looped on every background refresh.

## Root causes and fixes (all landed 2026-08-04)

| # | Root cause | Fix |
|---|-----------|-----|
| 1 | `activatedChanged` emitted without an actual entitlement transition (`clearLicenseCache` unconditional; Trial blanket-forward; error-path emissions) | `LemonSqueezy::notifyEntitlementMaybeChanged()` — idempotent publisher comparing `CommercialToken` validity; every entitlement mutation funnels through it |
| 2 | Trial startup refresh showed modals offline and re-emitted state | Silent startup fetch (`m_silentFetch`); `enabledChanged` only on real state change; token-clear now tier-checked, not variant-string-checked |
| 3 | Connected-state emissions scattered, per-path `settled` logic | `ConnectionManager::notifyConnectedStateChanged()` — single idempotent publisher (flag + open-device count); all lifecycle paths funnel through it |
| 4 | `connectAllDevices`/`disconnectAllDevices`/`disconnectDevice()` iterated `m_devices` while closes could spin the event loop into `rebuildDevices` (iterator UB) | Snapshot id lists; `rebuildDevices` coalesces reentrant triggers into one queued follow-up |
| 5 | Network TCP dial blocked the GUI thread (`waitForConnected` ×5 + `msleep`), un-abortable | Async dial: `connectToHost` + bounded refusal retry (5×, 300 ms) + 15 s per-attempt timeout; `close()` cancels everything; failure = queued box + `disconnectDevice(this)` |
| 6 | `toggleConnection` stacked a second dial during an async attempt | `HAL_Driver::isConnecting()` (default false) + overrides in Network/BLE/MQTT/Modbus/CANBus; toggle aborts in-flight dials |
| 7 | UART auto-reconnect dead: flag set on live driver, 1 Hz poll only on UI driver | Driver-owned 1 s `m_reconnectTimer` armed by `handleError()`, disarmed by `close()`; port matched by name, not stale index |
| 8 | BLE failed dial died silently (`QLowEnergyController::errorOccurred` never connected) | `onControllerError()` → report + `close()`; BLE `error` box now queued |
| 9 | Process NamedPipe peer close left `isOpen()` true forever | Read-loop exits marshal `onPipeClosed()` → flag drops, queued disconnect + box |
| 10 | MQTT `open()` no-op while teardown in flight (quick disconnect→connect never redialed) | Re-arms `m_userWantsOpen` + `scheduleReconnectIfActive()`; already-Connected returns true |
| 11 | Modbus `msleep(300)` froze GUI between attempts | Event-processing abortable pause |
| 12 | CANBus error modal storm (one modal per bus error, synchronous mid-emission) | Queued + rate-limited (5 s) box |
| 13 | Legacy variant names broke UI (`variantName.indexOf("Pro")` gates, garbled app name) | Gate on new `isOnlineActivated` property; `updateAppNameFromVariant` falls back to plain APP_NAME on non-canonical strings |
| 14 | Selecting a Pro bus without entitlement silently produced no device | `setBusType` raises a queued "requires license/trial" box when driver creation is refused |
| 15 | `trialEnabled`/`trialExpired`/`trialAvailable` QML bindings stale across license transitions | `licenseDataChanged` now feeds `enabledChanged` + `availableChanged` |

## Same-day regression round (async dial fallout, found by the example projects)

The first landing broke three bundled examples. Root causes and fixes, landed 2026-08-04:

| # | Regression | Root cause | Fix |
|---|-----------|-----------|-----|
| R1 | ISS Tracker "reloads every 5 s" | Script does `io.connect(); delay(300); writeData(...)`. Blocking dial guaranteed the socket was up; async dial made `DeviceManager::write` drop the HTTP request silently (`isOpen()` false mid-dial), so the server closed the idle keep-alive → error → disconnect → repeat per loop | `DeviceManager::write` accepts data while `isConnecting()`; QTcpSocket buffers and flushes on connect |
| R2 | Drone/CAN helpers killed (exit 15) in a loop | `onConnect()` launches the helper, dials race its startup; failed dials emitted `sessionClosed` with nothing connected, and `ProcessLauncher::onSessionClosed` reaps all helpers — killing the process the retry needed | `sessionClosed` only when a live session existed: never from `rebuildDevices`, and `disconnectDevice(HAL_Driver*)` gates it on was-connected |
| R3 | Dial patience halved | Refusal retry window shrank from ~4 s (blocking loop) to 1.5 s | `kTcpConnectAttempts` 5 → 10 (~3 s of refusal patience) |
| R4 | "Changing ports/options does not affect the driver" | Live socket kept the old endpoint until a manual reconnect | Endpoint edits on a live Network driver reopen it after a 500 ms debounce; closed drivers never self-dial |

Lesson recorded: the blocking dial carried two undocumented contracts — "connected when
`io.connect()` returns" for scripts, and seconds of patience for script-launched servers.
Replacing a blocking primitive means re-providing its implicit guarantees, not just its API.
The bundled examples (ISS, dual-drone, CAN) are the de-facto regression suite for the
script + helper-process + multi-source connect paths; run them before shipping IO changes.

Also landed with this round: the six example scripts with third-party Python imports
(ecu_simulator, csv2wav, the three LTE modem bridges, daqbridge) bootstrap their own
dependencies on first run — a private venv next to the script (or under
`~/.serial-studio/example-venvs` for read-only installs), then re-exec — so PEP 668
"externally managed" system Pythons (Homebrew, Debian) never block an example.
`dual_drone_telemetry.py` and the ISS example are stdlib-only and need no bootstrap.

## Deferred gap register (next pass; from the 2026-08-04 driver audit)

- **No "connecting" feedback in the UI.** An async dial shows nothing; users double-click
  Connect and each click aborts their own dial (the telehack.com report). Needs an
  `isConnecting` surface on ConnectionManager + button/QML treatment, decided deliberately.
- **`Dashboard::handleMissingDataset` force-disconnects** (`Dashboard.cpp` retry-failure path)
  — an explicit `disconnectDevice()` that reaps helpers; if a transient widget-model failure
  can trigger it, a streaming session dies hard. Verify against the CAN example.
- **Modbus TCP** has no config-change reconnect (Network now does; MQTT always did).

- **USB/HID/CANBus/Process `open()` synchronous modals** — mid-open-stack modals remain in
  USB (4 sites + endpoint activation), HID (1), CANBus open-path helpers, Process (2).
  Convert to queued boxes like UART/Network/MQTT.
- **Process Launch-mode**: `onProcessFinished`/`onProcessError` raise the modal *before*
  scheduling the disconnect (state lies while the box is up); `waitForStarted(3000)` blocks
  the GUI; NamedPipe `isOpen()` reads true during the dial (needs a real connected flag +
  `isConnecting()` override).
- **Modbus**: nested `QEventLoop` connect window is still live UI (reentrancy bounded by
  `isConnecting()`+toggle-abort now, but a full async dial like Network's is the end state).
- **USB**: `cancelAndDrainTransfers` can sleep the GUI up to 2 s in iso mode; device unplug
  zeroes the selection before the read loop reports.
- **Audio**: a backend-stopped stream whose device still enumerates is undetected (no
  miniaudio stop callback registered).
- **HID `close()`** never emits `configurationChanged` (asymmetric with USB/Modbus/CANBus).
- **MQTT** `scheduleReconnectIfActive` leaks a heap `QMetaObject::Connection*` if the driver
  dies with a reconnect pending.
- **`linkState()`** still reports only connected/idle; decided against adding "connecting"
  this pass because it is a documented API surface — revisit deliberately if ground-station
  UX needs it.
- **Trial-expiry mid-session** leaves the sealed token valid until the next server verdict
  or restart (named as intended grace behavior; keep deliberate).

## Acceptance criteria (maintainer-run)

- [ ] AC1: Trial machine, no network: startup shows no dialogs, an open UART/TCP link
      survives the failed background trial refresh, no reconnect cycle.
- [ ] AC2: TCP connect to a dead host: UI stays responsive, button click aborts the dial,
      one error box appears after retries/timeout, button reads disconnected.
- [ ] AC3: Unplug a USB-serial adapter with auto-reconnect on: link drops with one box
      suppressed (resource error), replug reconnects within ~2 s; manual disconnect while
      unplugged never reconnects on replug.
- [ ] AC4: BLE connect to a powered-off device: error box appears, button recovers.
- [ ] AC5: NamedPipe writer closes: app reports pipe closed, device reads disconnected.
- [ ] AC6: Activated license with a legacy variant name: license key visible in License
      Management, window title not garbled, all Pro features work.
- [ ] AC7: `--benchmark-hotpath` unchanged (no hotpath TU was touched on the frame path).
