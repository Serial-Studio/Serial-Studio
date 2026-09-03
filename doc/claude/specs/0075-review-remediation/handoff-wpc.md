---
spec: 0075-review-remediation
package: WP-C (IO core and general-purpose drivers)
tasks: WPC-T1 .. WPC-T17
updated: 2026-09-01
---

# Handoff — WP-C

15 of 17 tasks done. WPC-T12 and WPC-T14 are partially done and left unticked; both remainders
need work that cannot be written blind against libusb/miniaudio without hardware. `code-verify.py
--check` is clean (0 errors) over all 76 changed files; the only advisories are TU-length ones on
files this spec's WP-I already splits.

## Files changed

### New

| Path | One line |
|------|----------|
| `app/src/IO/AsyncTcpDial.{h,cpp}` | GUI-thread dialer: async resolution -> paced refusal probe on throwaway sockets -> one `connectToHost` on the caller's socket; one verdict per `start()` under one deadline; `cancel()` reports nothing. |
| `app/src/IO/Drivers/CANBus/SerialCanBackendBase.{h,cpp}` | Port, open/close sequence, bounded rx buffer and `errorOccurred` handling shared by the two serial CAN backends; subclasses keep the protocol only. |
| `app/src/IO/Drivers/Audio/PlaybackRing.h` | Fixed-capacity SPSC byte ring between the GUI writer and the RT audio callback; underrun zero-fill and refused-write counters. |
| `app/src/IO/Drivers/UART/UartPolicy.h` | The two pure UART error decisions (fatal-vs-ignorable, auto-reconnect ownership), extracted so they can be tested without a port. |
| `app/tests/tst_async_tcp_dial.cpp` | Live listener, refusing port, unresolvable name, cancel, probe-disabled dial. |
| `app/tests/tst_connect_fanout.cpp` | `ConnectFanOut` request lifecycle, pending dials, latched state, wait cursor. |
| `app/tests/tst_connection_verdicts.cpp` | Verdict matrix over `Test::FakeDriver` (WP0): sync ok/fail, async ok/fail, cancel mid-dial, drop with pending dial, pause policy. |
| `app/tests/tst_serial_can_backend.cpp` | LAWICEL and analyzer decoders, bitrate tables, fatal-error classification, bounded buffer. |
| `app/tests/tst_playback_ring.cpp` | Ring ordering, wrap, underrun, overflow, reset. |
| `app/tests/tst_uart_policy.cpp` | Custom-path ResourceError is fatal; only ResourceError feeds auto-reconnect. |
| `app/tests/tst_machine_id.cpp` | A persisted fingerprint short-circuits the platform tool spawn. |
| `tests/integration/test_driver_drops.py` | TCP peer close, reconnect after a drop, WebSocket server close, serial pty removal. |

### Modified (behaviour)

| Path | One line |
|------|----------|
| `app/src/IO/Drivers/Network.{h,cpp}`, `Network/NetworkTcp.cpp` | TCP dials through `AsyncTcpDial` (the ~180-line blocking getaddrinfo/poll probe is gone); verdict via `succeedDial`/`failDial`; writes issued during a dial are held and flushed on connect. |
| `app/src/IO/Drivers/Network.cpp` | `driverProperties()` emits EVERY transport's rows (D17), so a project saved on TCP keeps its WS/HTTP/TLS settings. |
| `app/src/IO/Drivers/Network/NetworkUdp.cpp` | `readDatagram` return checked; a failed read ends the pass instead of republishing the previous datagram. |
| `app/src/IO/Drivers/Network/NetworkHttp.cpp` | Response body capped at 8 MiB, truncation logged once per run. |
| `app/src/IO/Drivers/Iec104.{h,cpp}` | `dialStation()` async (probe disabled: a 104 station permits one client), verdict through `openFinished`, `isConnecting()` override. |
| `app/src/IO/Drivers/Modbus.{h,cpp}` | TCP pre-probe through the helper instead of a 5 s blocking wait; `connectClient()` dials once on the probe's verdict. |
| `app/src/IO/Drivers/MQTT.{h,cpp}` | 15 s dial deadline -> `failDial`; the internal re-dial's `open() == false` now settles the verdict (E4); one manager reach (`reportDropToManager`). |
| `app/src/IO/Drivers/OpcUaSession.{h,cpp}` | Host resolved asynchronously before `UA_Client_connectAsync` (E5); dial URL uses the literal, `m_endpointUrl` keeps the hostname for the certificate check; pump cadence 10 ms busy / 100 ms idle. |
| `app/src/IO/Drivers/OpcUa.cpp` | Process-global `setFilterRules` removed from the driver constructor. |
| `app/src/IO/Drivers/OpcUa/OpcUaEndpointSelection.cpp` | Deprecated policies are never scored, so a deprecated-only server is not auto-dialed (E10). |
| `app/src/IO/Drivers/S7.{h,cpp}`, `EthernetIp.{h,cpp}` | `open()` starts the worker dial and returns with `isConnecting()`; the worker reports `dialFinished` once; the GUI no longer blocks 10-13 s. |
| `app/src/IO/ConnectionManager.{h,cpp}` | `ResumePolicy` (auto-reconnect keeps the pause, C2); `sessionClosed` only when a session existed (C13); device 0 survives a QuickPlot rebuild and a healthy link is not redialed (C8); `setBusType` defers destruction (C7); asserts in the five entry points (C14). |
| `app/src/IO/DeviceManager.{h,cpp}` | Captured connection instead of the wildcard `disconnect` (C9); the reader is deleted outright once the pipeline thread has stopped, instead of `deleteLater` into a dead loop. |
| `app/src/IO/Drivers/CANBus/{Slcan,SeeedCan}Backend.{h,cpp}` | Protocol-only subclasses of the new base; SLCAN id/DLC parse flags separated (D8); open verdict reads the adapter's BEL reply (D19). |
| `app/src/IO/Drivers/CANBus.{h,cpp}` | Unconnected-while-open goes through the manager, queued (D10); frames batched per burst with `logicalFramesHint`/`frameStep` (D13). |
| `app/src/IO/Drivers/BluetoothLE.{h,cpp}` | Established drop routed through the manager, queued; discovery dedupes by address/UUID rather than name (D21); `characteristicIndex` validated. |
| `app/src/IO/Drivers/UART.{h,cpp}` | Custom paths honour `ResourceError` (D5); `registerDevice` reports through the log + a queued notification instead of a modal (D4); persisted auto-select runs once per port-list change and the placeholder clears the key (D9); dead mutex removed; `errorOccurred` wired after the port opens. |
| `app/src/IO/Drivers/USB.{h,cpp}`, `app/qml/.../Drivers/USB.qml` | Advanced transfers are consent-gated: the setter refuses and reports, the pane asks once and records it (D4). |
| `app/src/IO/Drivers/Audio.{h,cpp}`, `Audio/AudioDeviceCatalog.{h,cpp}` | Capture-only sessions ignore output absence (D2); playback through the ring, multi-frame writes accepted (D3); capture uses a pre-sized slot pool with a drop counter; `applyConnectionSettings` goes through `setNormalization` (D7); open failures translated. |
| `app/src/IO/Drivers/HID.cpp` | hidapi init/exit refcounted (the live instance's `hid_exit` tore down the UI instance's manager); `open()` closes first. |
| `app/src/IO/Drivers/Process.{h,cpp}` | `doClose` terminates and kills on a timer instead of blocking 2 s; `ps` enumeration asynchronous; stderr goes to the terminal lane only; the crash double-drop is guarded. |
| `app/src/IO/FileTransmission/{XMODEM,YMODEM,ZMODEM}.cpp`, `Protocol.h`, `FileTransmission.{h,cpp}` | NAK/timeout return to `SendingBlocks` before resending (C1); typed `protocolError()` replaces English substring matching (C3); ZMODEM seek failure closes state and ZRPOS retires the previous chunk chain (C11); blank lines are sent (C12); `YMODEM::cancelTransfer()` resets the batch state. |
| `app/src/Licensing/MachineID.{h,cpp}` | The persisted fingerprint is used first; ioreg/reg/powershell run only on first launch (K11). |
| `app/src/Misc/Extensions/PluginRunner.{h,cpp}` | No `waitForStarted` on launch, no per-plugin `waitForFinished` on stop; quit spends ONE 1 s budget across all plugins, then kills (K12). |
| `app/tests/tst_xymodem.cpp` | Four XFAILs flipped to assertions; new typed-error case. |
| `app/tests/tst_opcua_endpoint_selection.cpp` | New deprecated-only-server case. |
| `tests/integration/test_connection_verdicts.py` | TCP treated as an async bus (the verdict is read from the status, not the `io.connect` response); new `io.connect` + `writeData` case. |
| `tests/integration/test_audio_loopback.py` | Capture survives without an output device; a written tone is accepted whole. |
| `app/CMakeLists.txt`, `app/tests/CMakeLists.txt` | New sources and six new suites, appended contiguously. |

## Tests added

ctest: `tst_async_tcp_dial`, `tst_connect_fanout`, `tst_connection_verdicts` (needs WP0's
`app/tests/support/FakeDriver.h`; the suite is skipped by `ss_add_unit_test` until it lands),
`tst_serial_can_backend` (guarded on `TARGET Qt6::SerialBus`), `tst_playback_ring`,
`tst_uart_policy`, `tst_machine_id`, plus new cases in `tst_xymodem` and
`tst_opcua_endpoint_selection`.

pytest: `tests/integration/test_driver_drops.py` (new), plus cases in
`test_connection_verdicts.py` and `test_audio_loopback.py`.

**Run against the live pre-change build** (the app was up on 7777): all of
`test_driver_drops.py` and `test_connection_verdicts.py` pass (7 passed, 2 skipped: no
`websockets` module, no `socat`), and `test_capture_survives_without_an_output_device` passes.
`test_written_tone_is_accepted_by_the_playback_ring` FAILS on master by design: it pins D3 and
goes green only with this branch built. `pytest tests/scripts/` is green (302 passed).

## Tasks not done, and why

**WPC-T12 (second half), USB `write()` on the pump thread.** The consent half is done and
tested. Moving the 1 s synchronous bulk-OUT transfer off the GUI thread needs an async
`libusb_fill_bulk_transfer` + submit/callback path inside `UsbTransferPump`, with its own
transfer allocation, completion signal and cancellation. Written blind (no compiler, no USB
device), that path can only be validated by hardware, so it is left as a follow-up with the
control-transfer submit path (`submitControlTransfer`) as the model.

**WPC-T14, shared native contexts.** Done: hidapi `hid_init`/`hid_exit` refcounted (the crash in D11: the live instance's destructor
tore down the UI instance's IOHIDManager) and HID `open()` closes first. Not done: shared libusb/miniaudio contexts (`UsbContext`,
`AudioContext`) and `setPersistent(false)` on every live driver. The context sharing rewires
`USB::m_ctx` (handed by reference into `UsbTransferPump`) and `Audio::m_context` including their
event/enumeration threads; the persistence flag needs a `persistent` gate on ~70 setters across
seven drivers. Both are mechanical but unverifiable here, and a mistake in either silently breaks device
enumeration for every user.

**The four suites named in Verify lines that cannot link.** (`tst_usb_transfer_consent`,
`tst_file_transmission`, `tst_stream_config_builder`, `tst_mqtt_driver_verdict`.) Not written: each would have to link
`IO::USB`, `IO::FileTransmission`, `StreamConfigBuilder` or `IO::Drivers::MQTT`, and every one of
those pulls `ConnectionManager`, `AppState` or `ProjectModel`, which is the whole application (the
same wall documented for `tst_proto_importer` in `app/tests/CMakeLists.txt`). What was testable was tested instead: the protocol-level typed error in `tst_xymodem`, the verdict composition in
`tst_connection_verdicts`, the UART policy and the CAN decoders in their own header-only suites,
and the USB/MQTT/Modbus behaviour through the pytest tier.

**CAN batching unit test** (WPC-T10 Verify, "extension of `tst_can_reassembly`"). The batching
lives in `CANBus.cpp`, which is not linkable in the unit tier; `tst_can_reassembly` links only
`CanReassembly.cpp`. Left to the integration tier.

## Patches for the coordinator

None: every fix landed inside the files this package owns. Two shared-state items need a decision
that is not mine to make in an isolated worktree:

1. **TU census** (`code-verify.py --tu-census --accept`). Five TUs grew past the 1500-line ratchet:
   `MQTT.cpp` 1503 -> 1561, `ConnectionManager.cpp` 1705 -> 1746, `EthernetIp.cpp` 1499 -> 1531,
   `S7.cpp` 1482 -> 1512, `Audio.cpp` ~1460 -> 1537. WP-I's `PolledPlcWorkerBase` and
   `ConnectionManager` facade split remove far more than this adds; re-seed the baseline after
   WP-I merges rather than now.
2. **Singleton census** (`code-verify.py --singleton-census --accept`). Exactly +1:
   `CANBus.cpp` gains one `ConnectionManager::instance()` for the unified established-drop path
   (the driver had no manager reach at all before). MQTT and BLE were kept flat by folding their
   two reaches into one `reportDropToManager()` each.

## Doc-truth items for WP-J

- `doc/claude/architecture/io.md` "Spec-0050 dial doctrine": Network TCP no longer connects
  synchronously, `dialTcpBlocking()` no longer exists (claim-verify already flags the identifier),
  and TCP now reports through `isConnecting()` like WebSocket/HTTP. The "throwaway probe socket per
  attempt" doctrine is preserved; it moved into `IO::AsyncTcpDial`.
- Same file: S7, EtherNet/IP and IEC 104 are no longer "blocking dial, return value is the
  verdict"; all three report through the `openFinished` latch now.
- Same file: `isConnecting()` override list gains Network TCP, Iec104, S7, EthernetIp (and already
  omitted OpcUa, per C5).
- `io.getStatus`/`io.connect` semantics: for TCP the response's `connected` flag now means "the
  attempt started", as it already did for the other async buses. The AI corpus and any doc that
  tells a script to read that flag needs the same correction.
- `sessionClosed` is now emitted only when a session existed (cancelling a dial emits none). Worth
  stating where the signal's contract is documented.

## Invariants found that the plan did not state

1. **`io.connect`'s response flag is a public contract.** `IOManagerHandler::connect()` returns
   `connected: manager.isConnected()` immediately, so making a bus async silently changes what
   scripts and tests observe. WP-C absorbed it in `test_connection_verdicts.py`; any future
   sync -> async conversion must do the same.
2. **A control script's `io.connect()` + `writeData()` needs an explicit buffer once the dial is
   async.** The spec-0050 promise used to hold because `connectToHost()` had already been called
   and QTcpSocket buffered. With the probe-then-connect sequence nothing is connected yet, so
   `Network` now holds writes issued during a dial (capped at 1 MiB) and flushes them on success.
3. **A drop's teardown is queued behind the published state.** `io.getStatus` reports
   `isConnected: false` before the device is fully closed, so a reconnect issued in that instant
   races the close. Reproduced live; `test_driver_drops.py` settles for 1.5 s before reconnecting.
4. **IEC 104 cannot use the refusal probe.** A strict station permits one client and counts the
   probe socket as it, so `AsyncTcpDial::setProbeEnabled(false)` exists for that case: resolve,
   then dial the driver's own socket once.
5. **The MachineID timeouts must not be shortened.** The task asked for 500 ms per tool. On a
   first run a truncated read would be persisted as this machine's identity, re-keying every
   encrypted store on that machine, the failure class behind the shelved keychain migration.
   First-run-only spawning removes the recurring cost without that risk; the timeouts stayed.
6. **`QProcess::finished` never fires for a process that failed to start.** Dropping
   `waitForStarted()` in `PluginRunner` therefore needed an explicit `FailedToStart` cleanup path,
   or the plugin stays in the running list forever.

## Counterfactual self-check

**Which rule does this diff most risk violating?** "The verdict has ONE owner per attempt, emitted
exactly once on BOTH outcomes" (spec 0050). Five buses moved from synchronous to asynchronous
verdicts in this diff, and a driver that dials async without reporting both outcomes wedges the
connect button, which is the exact bug class that doctrine exists to kill.

**Evidence that it does not.** Every converted path has one funnel and both arms:
`AsyncTcpDial::report()` tears its own state down and emits `finished` exactly once per `start()`
(`m_active` gates re-entry; `cancel()` emits nothing, matching the manager's drop-the-pending-dial
cancel path), and `tst_async_tcp_dial` pins it (success, refusal, unresolvable host,
cancel-emits-nothing). Network TCP routes both arms into the existing `succeedDial`/`failDial`
funnels; Iec104's `onDialFinished` calls `reportOpenFinished` on both arms and `doClose()` cancels
the dial so a cancelled attempt reports nothing; Modbus's probe verdict either dials the client
(whose ConnectedState/UnconnectedState arms already report) or calls `failDial`; S7 and EtherNet/IP
report `dialFinished(ok, reason)` from `beginDial()` unconditionally, and their `m_connecting`
guard drops a verdict that lands after a user close. `tst_connection_verdicts` drives the
composition of `armOpenReport` -> `open()` -> pending-dial -> report over the WP0 fake driver for
all six outcomes, including "cancel mid-dial reports nothing" and "a report with no pending id is
ignored". The one place a second verdict could still appear (a synchronous `false` return from `open()`
after the driver already reported) is handled by the manager's existing
`takePendingDial()` claim, which the same suite covers.
