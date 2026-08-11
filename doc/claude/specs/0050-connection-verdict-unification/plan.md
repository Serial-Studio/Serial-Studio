---
spec: 0050-connection-verdict-unification
phase: plan
status: approved     # approved verbally 2026-08-10 with the live-text-apply addendum
updated: 2026-08-10
---

# Plan 0050 — Verdict unification: openFinished, sweep retirement, setter-guard lint

> **Phase 2 of 4 — the HOW.** Covers the scope deferred from the 2026-08-10 "ship tonight"
> pass: the parts of spec 0050 already landed (sync Network dial, Modbus pre-probe, reopen
> removal, edit lock, queued error boxes, failure-to-drop-path wiring) are prior art this
> plan builds on, commit `def58b5d`.

## Approach (one paragraph)

Give the connect-attempt outcome a single push-based owner: a new `HAL_Driver::openFinished
(bool ok, QString reason)` signal, emitted **exactly once per open attempt** through a
latched protected helper, replaces the `settlePendingDialVerdicts()` polling sweep. Sync
drivers never emit it — their `open()` return is the verdict, unchanged. The four async
drivers (BluetoothLE, Modbus, MQTT, Process) and semi-async CANBus report both success and
failure through the same edge; ConnectionManager consumes it in one slot that settles the
pending attempt, publishes state, and quietly closes the device on a failed dial (retiring
the per-driver `disconnectDevice(this)` calls added as tonight's stopgap for *dial*
failures — the established-link drop path keeps them). AC8 lands as a `driver-setter-guard`
rule in `scripts/code-verify.py` that fails any concrete setter in `app/src/IO/Drivers/`
lacking a same-value early return or an `isOpen()` gate, with the one live violation (UART
`setPortIndex`) fixed in the same change.

## Affected subsystems & files

| File | Change |
|------|--------|
| `app/src/IO/HAL_Driver.h` | `openFinished` signal; protected `reportOpenFinished(ok, reason)` + `armOpenReport()` latch (plain bool, no timer) |
| `app/src/IO/ConnectionManager.h` | drop `settlePendingDialVerdicts()`; add `onDriverOpenFinished(bool, QString)` slot |
| `app/src/IO/ConnectionManager.cpp` | `connectDevice(int)` arms the latch before `open()`; wire `openFinished` in `setBusType()` + `buildDeviceForSource()`; delete sweep + its call in `notifyConnectedStateChanged()`; failed-dial teardown centralized here (silent close, never `sessionClosed`) |
| `app/src/IO/Drivers/BluetoothLE.cpp` | `announceGattReady()` → report(true); `onControllerError()` → report(false, reason); remove stopgap `disconnectDevice(this)` |
| `app/src/IO/Drivers/Modbus.cpp` | `onStateChanged(Connected)` → report(true); `failDial()` → report(false, reason); remove stopgap `disconnectDevice(this)` |
| `app/src/IO/Drivers/MQTT.cpp` | `onStateChanged(Connected)` → report(true); dial-window `onErrorChanged()` → report(false, reason); remove stopgap block |
| `app/src/IO/Drivers/Process.cpp` | Launch: `QProcess::started` → report(true), `FailedToStart`/early exit → report(false); pipe: peer-attach marshal → report(true), pipe error while unconnected → report(false) |
| `app/src/IO/Drivers/CANBus.cpp` | plugin left in `ConnectingState`: `onStateChanged` Connected → report(true), Unconnected-while-connecting → report(false) |
| `app/src/IO/Drivers/UART.cpp` | `setPortIndex()` same-value guard on the clamped index (lint compliance) |
| `scripts/code_verify_rules.py` (or inline in `code-verify.py`, whichever hosts C++ body rules) | new `driver-setter-guard` rule, error severity, scoped to `app/src/IO/Drivers/*.cpp` |
| `.code-report` | regenerate via `--check` |
| `tests/integration/test_connection_verdicts.py` | NEW — formalizes tonight's manual battery (AC1/AC2/AC7) |
| `app/qml/MainWindow/Panes/SetupPanes/Drivers/*.qml` | text fields apply on `onTextEdited` (live apply; grep-confirmed list at implement) |
| `doc/claude/architecture/io.md` | verdict-ownership section updated to the signal design |

## Architecture & data flow

Main thread only. `connectDevice(int)` calls `driver->armOpenReport()` then
`DeviceManager::open()`. Sync drivers return the final verdict; `armOpenReport`'s latch is
cleared by ConnectionManager immediately when `!isConnecting()` after open returns, so a
sync driver can never emit a late verdict. Async drivers keep the attempt pending; their
existing state-machine slots call `reportOpenFinished()`, which emits only while the latch
is armed and disarms it (emit-exactly-once; a later established-link drop cannot re-emit a
verdict). ConnectionManager's `onDriverOpenFinished` resolves the device id from
`sender()` (same reverse lookup the drop path uses), removes it from the pending set, calls
the existing `onDeviceOpenFinished(id, ok, reason)` (diagnostics + conclude + notify), and
on failure closes the device without emitting `sessionClosed` — identical semantics to the
stopgap path, minus the "device dropped" warning for a dial that merely failed.
`refreshConnectedState()` (queued) stays: it publishes connected-state transitions; it just
no longer doubles as the verdict sweep. Process's pipe thread already marshals peer events
to the main thread; `reportOpenFinished` is called only from those marshaled slots.

## Hotpath & threading impact

- **Touches the hotpath?** No. Connect lifecycle only; no `FrameReader`/`CircularBuffer`/
  `FrameBuilder`/Dashboard-draw code is read or written.
- **New cross-thread signal/slot?** No. `openFinished` is emitted and consumed on the main
  thread (Process pipe events are already marshaled before reaching the reporting slot).
  Connection type: default (direct, same thread).
- **New input to a cached hotpath flag?** None.
- **Timestamp ownership** — untouched; no export/report worker changes.

## Data model & persistence

None. No project-JSON, settings, or DB shape changes.

## API / SDK surface

No new commands. Observable improvement: `io.connect` on async buses still returns
"attempt started", but `io.getStatus.linkState` can no longer report `connecting`
indefinitely (every attempt now settles). No schema change; no regeneration needed beyond
the standard sanitize pass.

## QML / UI

**Live text apply (maintainer addendum, 2026-08-10).** Driver text fields currently commit
only on `editingFinished` (Enter or focus loss) — deliberate armor from the reopen era, when
every keystroke would have restarted DNS and redialed the link. That armor is obsolete:
reopen machinery is deleted and settings are locked while connected. Text fields in the
driver setup panes switch to applying on **`onTextEdited`** (fires for user input only —
`onTextChanged` would loop with the programmatic write-back each pane's `Connections`
handler performs). Surfaces: `SetupPanes/Drivers/Network.qml` (address), plus an implement-
time grep for the same `editingFinished`-commit pattern across the other driver panes
(Modbus host, MQTT hostname/topic, Process executable/arguments, UART custom device).
Per-keystroke hostname DNS lookups are now inert (nothing downstream of a lookup can touch
a connection); no debounce needed.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Verdict transport | (A) `openFinished` signal + latch; (B) completion callback passed to `open()`; (C) keep success-sweep + failure-drop-path | **A** — one owner, push-based, deletes the poll; B fights the codebase's signal idiom and adds functor-lifetime hazards across `rebuildDevices()`; C leaves two owners and the sweep alive |
| Emit-once enforcement | per-driver discipline vs base-class latch | **Latch in HAL_Driver** — one mechanism, uncheatable; per-driver discipline is exactly what stranded verdicts before |
| Failed-dial teardown | per-driver `disconnectDevice(this)` (tonight's stopgap) vs centralized in `onDriverOpenFinished` | **Centralized** — drivers report, the manager acts; removes N copies of teardown ordering and the misleading "device dropped" log for plain dial failures |
| Lint severity | advisory vs error | **Error, scoped to `IO/Drivers/`** — the missing guard was the telehack loop's engine; advisory baseline-debt would let the next driver ship one. Scoping avoids re-litigating non-driver setters |
| Dispatcher setters (`setDriverProperty`) | lint them vs exempt | **Exempt dispatchers** (they fan out to concrete setters, which are the guarded surface) — rule matches `set*` with a scalar/QString parameter, skips overrides of `setDriverProperty` |

## Risks & mitigations

- **A driver path that never reports** (the old stuck-"connecting" class): the latch makes
  the gap visible — `disconnectDevice(id)`/`close()` clears the pending attempt on user
  cancel exactly as today, and the new pytest cycles (AC7) assert no pending residue after
  20 cycles per bus. BLE's error enumeration is the riskiest (multiple QLowEnergyController
  error paths); mitigation: report from `onControllerError` + the `disconnected`-before-
  ready path, both of which already funnel every failure today.
- **Double emission** (e.g., Modbus Connected then transport drop mid-attempt): impossible
  by construction — the latch disarms on first report.
- **`sender()` lookup on a rebuilt driver**: `rebuildDevices()` disconnects the old driver's
  signals before destroying it (existing pattern), so a late `openFinished` from a doomed
  instance cannot arrive; noted in implement checklist to keep that ordering.
- **Silent-breakage classes** (common-mistakes exposure): modal-in-error-stack — none added
  (reports carry no UI); queued-vs-direct — all same-thread; scope creep — file list above
  is the lane.
- **Lint false positives**: rule runs on tree-sitter body text where available (same as
  other cxx body rules); fallback regex documented in the rule. Verified against all ten
  drivers before landing (Audio's `isOpen()` gates pass; UART fixed).

## Test & verification plan

- **Integration (app up, API on)** — NEW `tests/integration/test_connection_verdicts.py`:
  - AC1: Network TCP + Modbus TCP dial to closed local port → status settles disconnected
    within 6 s, `linkState` never stuck `connecting` (poll during the window).
  - AC2: same file, asserts the connecting flag drops for Modbus/Process dead endpoints.
  - AC7: 20x connect/disconnect per scriptable bus (Network, Modbus, Process) → final
    status equals fresh status; no helper duplicates (`system.runningProcesses` or pgrep).
  - AC5 (already met, regression-pinned): re-apply identical settings 10x → no reconnect,
    no undo growth.
- **Manual (maintainer, stage hardware)**: AC2-BLE — dial a powered-off peripheral, verdict
  resolves with one notice; AC3 edit-lock visual per bus.
- **Static**: `python scripts/code-verify.py --check` clean tree-wide including the new
  rule; `.code-report` regenerated; `sanitize-commit.py` before commit; `qt-cpp-review`
  on the diff before handoff.
- **Hotpath**: not touched; no benchmark run required (maintainer may still run
  `--benchmark-hotpath` as part of the normal push gate).
