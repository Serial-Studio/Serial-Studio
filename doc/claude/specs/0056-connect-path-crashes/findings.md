# 0056 — Connect-path crashes found by the integration sweep

Status: C1 and C3 fixed, C2 open with a reliable reproducer. Collected 2026-08-16 while running
`pytest tests/integration/` against a spec-0055 build. None of these are caused by spec 0055;
C1's mechanism was *removed* by the 0055 follow-up, the other two predate it.

The suite kills the app roughly once per full pass, which is why nothing past
`test_change_driven_transforms.py` has ever been run. Fixing these is a prerequisite for the
sweep being able to complete at all.

## C1 — nested `QEventLoop` inside `connectDevice()` (FIXED)

Crash report: `Serial-Studio-Pro-2026-08-16-212616.ips`.

```
QtPrivate::QCallableObject<CSV::Export::setupExternalConnections()::$_1, ...>::impl
QEventLoop::exec(...)                      <- nested loop
-[NSApplication run]                       <- re-entered
IO::ConnectionManager::connectDevice(int)
API::Handlers::IOManagerHandler::connect(...)
API::Server::onDataReceived(...)           <- outer frame is a socket read
```

The three file sinks each fetched their template frame on `connectedChanged` via
`FrameBuilder::invokeOnBuilderThreadBlocking`, which spins a nested `QEventLoop` on the GUI
thread. Dispatched from an API command, that nested loop pumps the same API socket and re-enters
the connect path.

**Fixed** by having `FrameBuilder::onConnectedChanged` emit `sessionStructureReady(const Frame&)`
from the pipeline thread, consumed queued by CSV / MDF4 / Sessions. No blocking marshal remains on
the connect path.

One blocking marshal deliberately survives at `Sessions/Export.cpp` `captureTableSnapshots()` — a
periodic timer callback with no socket handler above it on the stack. Same mechanism, different
exposure; removing it means restructuring the table-snapshot pull.

## C2 — socket read notification into freed memory (OPEN)

Reproduces 7/7 with:

```
pytest "tests/integration/test_change_driven_transforms.py::TestChangeDrivenEquivalence"
```

Test 1 passes; the app dies during test 2. The API socket then reports whatever it was mid-call
on — `Connection closed by server`, `Broken pipe`, `Connection reset by peer` — so the pytest
failure text varies and is not itself a signal.

Always the same fault, always on `com.apple.main-thread`:

```
QAbstractSocketPrivate::canReadNotification()
QApplication::notify(...)
__CFSocketPerformV0                        <- CF run-loop source, not a Qt posted event
```

Under lldb at the fault:

```
frame #0  QAbstractSocketPrivate::canReadNotification() + 208
->  ldr x8, [x8, #0x100]     ; x8 = 0
    mov x0, x20
    blr x8                   ; virtual call
x19 = 0x77d948ca00           ; QAbstractSocketPrivate*
x20 = 0x77d10c62f0           ; object being called -- vtable pointer reads back 0
```

So a socket-side object is **destroyed while a CFSocket source is still scheduled for it**, and
the next run-loop pass makes a virtual call into the corpse. Across builds the fault address has
been `0x100` (zeroed vtable) and later a garbage pointer (memory recycled), consistent with
"freed" rather than "null field".

### What the maintainer observed

The crash always lands **just after dismissing a "Network socket error" message box**. A modal
`exec()` pumps the main loop, so queued work — API commands, deferred deletes — runs while the box
is up, and the world changes underneath whatever is waiting on it.

### Confirmed by experiment (same binary, same two tests)

| Variant | Peer behaviour | Result |
|---|---|---|
| Session-scoped simulator | never drops | passes |
| Session-scoped + stop/start between tests | drops while disconnected | passes |
| Session-scoped + drop while connected | drops mid-connection | passes |
| Original (function-scoped simulator) | listener destroyed + recreated per test | **crashes** |

No error box => no crash. The crash needs the peer-loss path that raises it.

### Hypotheses tested and REJECTED

Recording these so nobody re-runs them:

1. **Nested loop from the sink template fetch (C1).** Fixed, and C2 still reproduced afterwards.
2. **`ServerWorker::removeSocket()` leaving an armed notifier before `deleteLater()`.** Hardened;
   crash unchanged. Reverted -- it was justified only as a C2 fix and was not one.
3. **The spec-0050 probe socket.** `waitForTcpEndpoint` created/aborted/destroyed a `QTcpSocket`
   every 250 ms of a failing dial. Replaced with a raw non-blocking `connect()`/`select()` that
   registers no run-loop source. Crash unchanged. **Change kept on its own merits.**
4. **The API server's socket handoff** (`moveToThread` on a live, already-read socket). Disproved
   directly: a session-scoped API client -- one connection, one handoff -- still crashes.
5. **`Network`'s own sockets dying with the driver.** Converted to heap objects aborted and
   `deleteLater()`d by a new `~Network()`, so a late source would find a live aborted socket.
   Crash unchanged. **Recommend reverting** (see below).

### Changes currently in the tree from this hunt

- `rebuildDevices()` / `dropUnavailablePrimaryDevice()` retire devices through `deleteLater()`
  instead of destroying them inside the map erase. Keep: destroying a driver synchronously under
  a modal's nested loop is wrong regardless of C2.
- The raw-socket probe in `Network.cpp`. Keep.
- `~Network()` + heap `m_tcpSocket`/`m_udpSocket`. **Did not fix C2**, changes driver socket
  ownership, and leaks two sockets at exit when no event loop remains to run the deferred delete.
  Revert unless it earns its place another way.

### Next step

Stop guessing; get the allocation record. One AddressSanitizer build

```
-DCMAKE_CXX_FLAGS="-fsanitize=address -fno-omit-frame-pointer"
-DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address"
```

then the two-test repro. ASan prints both the allocation stack and the freeing stack of the exact
object, which names the socket and the teardown path outright. Five hypotheses have been spent on
inference; the sixth should be spent on evidence.

Manual reduction has already failed three times: progressively closer hand-built approximations
of the fixture teardown all survive, and only the real teardown (listener object destroyed with
its accepted connection, watcher thread joined, fresh listener on the same port) reproduces.

## C3 — `hid_free_enumeration` invalid free on project load

Crash report: `Serial-Studio-Pro-2026-08-16-214823.ips`, `SIGABRT`,
`___BUG_IN_CLIENT_OF_LIBMALLOC_POINTER_BEING_FREED_WAS_NOT_ALLOCATED`.

```
hid_free_enumeration
IO::Drivers::HID::enumerateDevices()
IO::Drivers::HID::setDiscoveryPaused(bool)
IO::ConnectionManager::setupExternalConnections()::$_2
IO::ConnectionManager::rebuildDevices()
DataModel::ProjectModel::emitProjectLoadedSignals(bool)
API::Handlers::ProjectHandler::loadFromJSON(...)
```

`HID::enumerateDevices()` (`app/src/IO/Drivers/HID.cpp:315`) is:

```cpp
hid_free_enumeration(m_deviceInfoList);
m_deviceInfoList = hid_enumerate(0x0000, 0x0000);
```

Between those two statements `m_deviceInfoList` is a dangling pointer. `hid_enumerate()` walks
IOKit on macOS and can pump the run loop, so a re-entrant `enumerateDevices()` — from the 250 ms
`m_enumTimer`, or from the `sourceStructureChanged → rebuildDevices` direct connection this very
stack shows — frees the same pointer a second time.

Independent of the driver-list reentrancy, it is also unsound for the same reason a
double-close is: the member must never name freed memory.

**Fixed.** `enumerateDevices()` is now a re-entrancy latch around a new
`refreshDeviceEnumeration()` holding the old body — the body has an early return on "labels
unchanged", so an inline latch would leak and wedge enumeration permanently. Inside, the member is
nulled between the free and the re-enumerate so it can never name freed memory. The destructor
already did this correctly (`HID.cpp:93-94`); `enumerateDevices()` was the outlier.

## Not yet triaged

- `test_2d_array_parsing.py` — 2 failures before the 0055 follow-up, 6 after. Both counts were
  measured against an app that crashed later in the same pass, so some may be collateral. Needs
  the actual assertion text, on a run that survives.
- `test_change_driven_transforms.py` — 1 failure plus 3 errors; the errors are the app dying.
