---
spec: 0050-connection-verdict-unification
phase: tasks
status: approved     # implemented 2026-08-10 (same night, maintainer standing approval)
updated: 2026-08-10
---

# Tasks 0050 — Verdict unification: openFinished, sweep retirement, setter-guard lint

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

## Tasks

### T1 — HAL_Driver openFinished signal + emit-once latch

- **Files:** `app/src/IO/HAL_Driver.h`
- **Does:** Add `signals: void openFinished(bool ok, const QString& reason);` plus protected
  `armOpenReport()` / `reportOpenFinished(bool, const QString&)` where the report emits only
  while armed and disarms on first emission. Plain bool latch, no timer. Signal-wiring
  invariant: header-only change, no connects here.
- **Verify:** `python scripts/code-verify.py --check app/src/IO/HAL_Driver.h`
- **Deps:** none
- [x] done

### T2 — ConnectionManager consumes openFinished, retires the sweep

- **Files:** `app/src/IO/ConnectionManager.h`, `app/src/IO/ConnectionManager.cpp`
- **Does:** `connectDevice(int)` arms the latch before `DeviceManager::open()` and disarms it
  when the attempt settles synchronously; wire `openFinished -> onDriverOpenFinished` in
  `setBusType()` and `buildDeviceForSource()` (UniqueConnection, same-thread); new slot
  resolves the id via the existing sender reverse-lookup, removes it from the pending set,
  forwards to `onDeviceOpenFinished(id, ok, reason)`, and on `!ok` closes the device WITHOUT
  `sessionClosed` (helpers survive a failed dial). Delete `settlePendingDialVerdicts()` and
  its call inside `notifyConnectedStateChanged()`. Binding invariants named for implement:
  `connectedChanged` publishes only via idempotent `notifyConnectedStateChanged()`;
  `rebuildDevices()` must keep disconnecting a doomed driver's signals before destruction so
  a late verdict cannot arrive.
- **Verify:** code-verify on both files; grep proves zero remaining `settlePendingDialVerdicts`
  references.
- **Deps:** T1
- [x] done

### T3 — BluetoothLE reports both outcomes

- **Files:** `app/src/IO/Drivers/BluetoothLE.cpp`
- **Does:** `announceGattReady()` -> `reportOpenFinished(true)`; `onControllerError()` ->
  `reportOpenFinished(false, errorString)` and drop the stopgap
  `ConnectionManager::disconnectDevice(this)` (manager teardown now handles failed dials).
  Keep the queued error-signal box wiring untouched.
- **Verify:** code-verify; read-back confirms every controller failure path funnels through
  `onControllerError` or the pre-ready `disconnected -> close` path.
- **Deps:** T2
- [x] done

### T4 — Modbus reports both outcomes

- **Files:** `app/src/IO/Drivers/Modbus.cpp`
- **Does:** `onStateChanged(ConnectedState)` -> `reportOpenFinished(true)`; `failDial()` ->
  `reportOpenFinished(false, reason)` and remove the stopgap `disconnectDevice(this)` call.
  Queued-box rule stays (failDial's box already queues).
- **Verify:** code-verify; grep confirms no `disconnectDevice(this)` remains in Modbus.cpp
  outside established-link paths (there are none).
- **Deps:** T2
- [x] done

### T5 — MQTT reports both outcomes

- **Files:** `app/src/IO/Drivers/MQTT.cpp`
- **Does:** `onStateChanged(Connected)` -> `reportOpenFinished(true)`; dial-window
  `onErrorChanged()` -> `reportOpenFinished(false, reason)` replacing the stopgap
  disconnect block (keep `m_userWantsOpen`/`m_reconnectPending` clearing).
- **Verify:** code-verify; read-back of onStateChanged/onErrorChanged.
- **Deps:** T2
- [x] done

### T6 — Process reports both outcomes (both modes)

- **Files:** `app/src/IO/Drivers/Process.cpp`
- **Does:** Launch mode: `QProcess::started` -> report(true); `FailedToStart` or exit while
  still Starting -> report(false, reason). Pipe mode: the marshaled peer-attach slot ->
  report(true); pipe error while `!m_pipeConnected` -> report(false, reason). Threading
  invariant named: reports fire only from main-thread slots (pipe events are already
  marshaled); never from the pipe thread.
- **Verify:** code-verify; read-back confirms no report call sits on the pipe thread.
- **Deps:** T2
- [x] done

### T7 — CANBus reports when a plugin dials async

- **Files:** `app/src/IO/Drivers/CANBus.cpp`
- **Does:** `onStateChanged`: Connected while latch armed -> report(true); Unconnected while
  armed -> report(false, errorString). gs_usb path is synchronous and never reports (latch
  cleared by CM when open returns settled).
- **Verify:** code-verify; read-back.
- **Deps:** T2
- [x] done

### T8 — UART setPortIndex same-value guard

- **Files:** `app/src/IO/Drivers/UART.cpp`
- **Does:** Early-return when the clamped index equals `m_portIndex` and the persisted name
  is already current, so re-application cannot emit. Keeps auto-reconnect's
  `setPortIndex + connectDevice()` flow working (explicit connect follows regardless of the
  skipped emit).
- **Verify:** code-verify; UART section of the new lint rule (T9) passes.
- **Deps:** none
- [x] done

### T9 — driver-setter-guard lint rule

- **Files:** `scripts/code_verify_rules.py` (or the C++ body-rule host in
  `scripts/code-verify.py` if tree-sitter is unavailable there), `.code-report`
- **Does:** Error-severity rule scoped to `app/src/IO/Drivers/*.cpp`: a concrete
  `set*(scalar|QString)` method must contain a same-value early return or an `isOpen()`
  gate; `setDriverProperty` dispatch overrides exempt. Regenerate `.code-report`.
- **Verify:** `python scripts/code-verify.py --check` clean tree-wide; intentionally break a
  guard locally to see the rule fire, then restore.
- **Deps:** T8
- [x] done

### T10 — Live text apply in driver panes

- **Files:** `app/qml/MainWindow/Panes/SetupPanes/Drivers/Network.qml` + grep-confirmed
  peers (Modbus host, MQTT hostname/topic, Process executable/arguments, UART custom device)
- **Does:** Text fields commit on `onTextEdited` (user input only — `onTextChanged` would
  loop with each pane's programmatic `Connections` write-back; that asymmetry is the binding
  invariant). Keep `editingFinished` fallback for the empty-field default-address case.
- **Verify:** code-verify on touched QML; manual read-back of each pane's write-back handler
  for loop-freedom.
- **Deps:** none
- [x] done

### T11 — Verdict integration tests

- **Files:** `tests/integration/test_connection_verdicts.py` (new)
- **Does:** AC1 dead-port verdicts (Network + Modbus TCP settle disconnected =<6 s, never
  stuck `connecting`); AC2 connecting-flag drop for Modbus/Process dead endpoints; AC7 20x
  connect/disconnect cycles per scriptable bus with fresh-state equality + single-helper
  assertion; AC5 regression pin (10x identical settings re-apply -> no reconnect, no undo
  growth via `project.getStatus`/undo depth).
- **Verify:** `pytest tests/integration/test_connection_verdicts.py -v` against a running
  app (maintainer launches).
- **Deps:** T2-T7
- [x] done

### T12 — Docs sync

- **Files:** `doc/claude/architecture/io.md`, `CLAUDE.md` (io bullet only if wording drifts)
- **Does:** Verdict-ownership paragraph updated to the signal design; sweep references
  removed; live-text-apply note added to the io/QML notes.
- **Verify:** `python scripts/code-verify.py --check` on the markdown (doc rules).
- **Deps:** T2-T10
- [x] done

## Definition of Done

- [x] Every acceptance criterion in `spec.md` is met and checked off there (AC3/AC2-BLE
      remain maintainer-manual on stage hardware).
- [x] `python scripts/code-verify.py --check` is clean on all changed files (no new errors),
      including the new `driver-setter-guard` rule tree-wide.
- [x] `qt-cpp-review` run on the C++ diff; findings addressed or noted.
- [x] Hotpath untouched (plan says none); no benchmark regression expected or introduced.
- [x] `pytest tests/integration/test_connection_verdicts.py` listed for the maintainer.
- [x] `python scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [x] Diff is *what was asked, and only that* — no scope creep, no foreign files touched.
- [x] `spec.md` status set to `done`.
