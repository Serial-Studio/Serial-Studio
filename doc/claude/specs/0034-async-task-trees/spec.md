---
spec: 0034-async-task-trees
title: Declarative async orchestration for I/O flows
status: shelved      # draft -> approved -> in-progress -> done | shelved
created: 2026-07-25
author: Claude (roadmap R10, with Alex)
---

# Spec 0034 — Declarative async orchestration for I/O flows

> **Shelved 2026-07-30 (commit 38c9ef66).** The flow layer shipped and was then removed:
> `IO::ConnectionFlows` and the `HAL_Driver` async-open hooks are gone, driver opens are
> synchronous again, and drop recovery is per-driver. Only the engine survives —
> `app/src/Async/` (TaskTree, RetryPolicy, AsyncClock), used by `MQTT::Publisher` and the
> spec-0035 diagnostics probes. Read this file as history, not as the current design; current
> state is in [../../architecture/io.md](../../architecture/io.md).

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.
>
> Roadmap parent: spec 0030, item R10 ("declarative async orchestration for I/O flows").
> Wave 1, no dependencies; pairs with R9 (connection diagnostics, spec 0035).

## Problem / Motivation

Every connection lifecycle in the application — open, retry, handshake, teardown, and the
reconnect that follows a dropped link — is hand-rolled per driver as a mix of booleans,
one-shot signal lambdas, blocking waits, and deferred `invokeMethod` continuations. There
is no shared notion of a step, a timeout, a retry, or a cancel. The consequence is not
theoretical: an audit of the I/O stack on 2026-07-25 found the same four defect classes
repeated across drivers, each fixed (or not fixed) independently.

**The measured inventory.** Each row is a distinct hand-rolled async flow, with the
sequencing mechanism it uses today:

| # | Flow | Where | Mechanism today | Timeout | Retry | Cancel |
|---|------|-------|-----------------|---------|-------|--------|
| F1 | TCP connect with retry | `Network.cpp:238-256` (consts `:29-31`) | **blocking** `waitForConnected(600)` + `QThread::msleep(300)` in a 5-iteration `for` loop on the main thread | 600 ms/attempt | 5 fixed attempts, 300 ms flat | **none** — loop cannot be interrupted |
| F2 | Async DNS lookup | `Network.cpp:440-445`, `:512-524` | `QHostInfo::lookupHost` callback; lookup id discarded | none | none | **none** — a superseded lookup's callback still stomps `m_hostExists` |
| F3 | Socket-error teardown | `Network.cpp:529-549` | error signal → `ConnectionManager::disconnectDevice` + modal box; suppressed while `m_connecting` (`:531-532`) | n/a | n/a | boolean suppression flag only |
| F4 | UDP bind + multicast join | `Network.cpp:182-230` | straight-line calls, fail → `close()` | none | none | n/a |
| F5 | Primary connect sequence | `ConnectionManager.cpp:683-712`, `:860-865` | synchronous fan-out over devices; `DeviceManager::open` **discards** the driver's `bool` result (`DeviceManager.cpp:126`) | none | none | none — a multi-source connect cannot be aborted partway |
| F6 | Device rebuild continuations | `ConnectionManager.cpp:1274-1350` | three hand-rolled `QMetaObject::invokeMethod(..., QueuedConnection)` continuations (`:1336`, `:1344`, `:1349`) | none | none | **none** — a newer rebuild does not invalidate an older rebuild's queued reconnect |
| F7 | Shutdown / teardown | `ConnectionManager.cpp:882-907` | manual "drain the map into a local, then destroy" trick (`:891-893`) to survive re-entrant `configurationChanged` during driver destruction | n/a | n/a | ordering trick, not a guard |
| F8 | UI-driver config save | `ConnectionManager.cpp:93-95`, `:1238` | single-shot `QTimer`, 750 ms debounce | n/a | n/a | implicit |
| F9 | BLE discovery → connect → service discovery → characteristic subscribe | `BluetoothLE.cpp:854-894`, `:644-694`, `:1143-1201`, `:1089-1138`, `:1001-1042` | 8 pending/probe state flags (`BluetoothLE.h:145-154`), a cross-service probe retry loop (`:1116-1122`), a static cross-instance fan-out list (`s_instances`) | **none anywhere in the driver** | probe loop only | `close()` + `disconnect(this)`; **no epoch guard**; `QLowEnergyController::errorOccurred` is **never connected** |
| F10 | MQTT source reconnect | `MQTT.cpp:1076-1108`, called from 14 setters (`:454`-`:664`) | boolean `m_reconnectPending` + heap `QMetaObject::Connection*` one-shot lambda freeing itself | none | none | intent boolean `m_userWantsOpen` re-checked in the lambda |
| F11 | MQTT publisher reconnect | `Publisher.cpp:490-579` (worker thread) | structurally identical re-implementation of F10 with an extra queued `finishPendingReconnect()` hop | none | none | fresh `m_cfg.enabled` re-read |
| F12 | MQTT test-connection probe | `Publisher.cpp:688-724` | throwaway client + inline 5000 ms `QTimer` + a `done` shared flag making `report()` idempotent | 5000 ms (inline literal) | none | `done` flag |
| F13 | Modbus TCP connect with retry | `Modbus.cpp:462-496` (consts `:39-41`) | **blocking** nested `QEventLoop::exec()` + `QThread::msleep(300)` in a 5-iteration loop — a near-copy of F1 with different constants | 800 ms/attempt | 5 fixed attempts, 300 ms flat | **none** |
| F14 | Modbus poll cycle | `Modbus.cpp:1170-1191`, `:1298-1340` | hand-rolled sequential iterator over register groups: `m_currentGroupIndex` advanced from the reply handler; in-flight guard by `QModbusReply*` pointer identity (`:1306-1309`) | Qt Modbus device timeout | delegated to Qt Modbus (3) | timer stop + `deleteLater` |
| F15 | UART auto-reconnect on resource loss | `UART.cpp:781-809`, `:758-762` | the **only** drop-recovery in the app: `m_pendingReconnect` polled off the shared 1 Hz `TimerEvents` tick | none | level-triggered, unbounded, no backoff | `close()` clears the flag |
| F16 | Process launch / crash reaction | `Process.cpp:175-220`, `:507-531` | blocking `waitForStarted(3000)`; crash is reported and the device disconnected, never relaunched | 3000 ms / 2000 ms blocking waits | none | `m_pipeRunning` atomic; no guard on queued pipe-error callbacks |
| F17 | USB transfer pool + drain | `USB.cpp:986-1029`, `:1195-1285`, `:1370-1501` | self-resubmitting iso callbacks; teardown busy-polls `QThread::msleep(5)` to a 2000 ms deadline; four atomics stand in for phase | per-transfer libusb timeouts | none (hard-stop on failure) | atomics + bounded drain |
| F18 | X/Y/ZMODEM transfer state machines | `XMODEM.h:66-74`, `YMODEM.h:44-56`, `ZMODEM.h:90-109` | three hand-rolled enum state machines (7, 10, and 8+7 phases) each with its own timeout timer and retry counter; ZMODEM adds a `singleShot(0)` self-yield chunk loop (`ZMODEM.cpp:506`) | 10-15 s single-shot | 10 attempts, no backoff | `cancelTransfer()`; ZMODEM's `:478-479` re-check is the one explicit stale-continuation guard in the stack |

Five defect classes fall out of that table:

1. **The UI freezes on a bad address.** F1 blocks the main thread for up to ~4.5 s
   (5 × 600 ms wait + 4 × 300 ms sleep) with the wait cursor up; F13 does the same for up
   to ~6 s via a nested event loop. Cancel is impossible, because the code that would
   process the cancel is the code that is blocked.
2. **Almost nothing recovers from a dropped link.** Exactly one driver retries after an
   unsolicited drop — UART (F15), level-triggered off a shared 1 Hz tick, with no attempt
   cap and no backoff. Everywhere else the source stays dead: F10/F11 only re-open after
   a *configuration edit*, and F16 reports a process crash without ever relaunching. This
   is what the roadmap's acceptance criterion targets.
3. **Stale async callbacks are guarded ad hoc or not at all.** F2, F6, F9, and F16 have no
   generation/epoch token. The staleness protections that do exist are all different from
   each other: a `QPointer` auto-null (`DeviceManager.h:76`), a reply-pointer identity
   check (F14), two boolean suppression flags, two static re-entrancy booleans in the BLE
   cross-instance forwarding, and one explicit continuation re-check (F18, ZMODEM).
4. **Errors are dropped on the floor.** F5 discards the driver's open result
   (`DeviceManager.cpp:126`), F9 never connects the BLE controller's error signal at all,
   and both BLE error handlers have a silent `default:` case. A failed connect surfaces as
   "not connected", with no reason.
5. **The same policy is written five times with five different constants.** F1 uses
   5 attempts / 600 ms / 300 ms; F13 uses 5 / 800 ms / 300 ms; F18 uses 10 attempts with a
   10-15 s timeout and no backoff, in three separate implementations. None of them backs
   off geometrically, and ZMODEM's retry counter is reset only at transfer start, never on
   a successful block — so retries accumulate across a whole session instead of per block.
   That last one is a live defect that a shared, tested policy would not have.

The cost is also structural. In the canonical driver, roughly 550 of ~1140 non-table
lines are sequencing, flag-shuffling, and cross-instance state propagation, against
roughly 130 lines that issue actual Qt Bluetooth calls — a 4:1 orchestration-to-work
ratio. Across the whole I/O stack the sequencing code adds up to somewhere near 1700
lines, none of it shared. Each new driver re-derives that scaffolding, and each reconnect
bug is fixed in one driver at a time.

## Goals

- Connection lifecycles are expressed as composable task trees — sequential and parallel
  groups with built-in timeout, cancellation, and error propagation — rather than as
  per-driver boolean state machines.
- Retry and backoff policy is written once and shared, so a reconnect fix lands for every
  flow that opted in, not for one driver.
- Attempting a connection never blocks the user interface, and pressing cancel or
  disconnect takes effect immediately, at any point in the sequence.
- A link that drops mid-stream comes back on its own, repeatedly, without accumulating
  timers, signal connections, sockets, or device objects.
- A failed step reports *which* step failed and why, in a form the connection-diagnostics
  work (spec 0035) and the problem center (spec 0033) can consume.
- The orchestration engine is testable without hardware: a flow's sequencing, timeout,
  retry, and cancel semantics can be asserted with fake steps in the C++ unit tier
  (spec 0032).

## Non-Goals

- **No user-visible redesign.** No new dialogs, no new settings pages, no change to the
  connect/disconnect controls, no change to the wording or timing of existing error
  message boxes beyond what removing the main-thread block necessarily changes.
- **Not a rewrite of the drivers.** Drivers keep their transport code; only the
  sequencing around it moves. A driver that has not been migrated keeps working
  unchanged — migration is per-flow and opt-in.
- **Not all twelve flows in v1.** v1 migrates the two flows with the worst recurring
  defect record (see Requirement R7); the rest follow in later waves under this same
  spec number or a successor.
- **No new threads.** This introduces no thread, no thread pool, and no mutex. It is an
  event-loop construct.
- **Nothing on the frame hotpath.** Task trees run at connection lifecycle boundaries
  only. No per-frame work, no new hotpath signal hop, no new cached-flag input.
- **No general-purpose job system.** This is not a replacement for the export workers,
  the MQTT publisher worker, the player workers, or `FrameConsumer`. Those keep their
  existing threading.
- **No persistence of in-flight flows.** A flow does not survive application restart.
- **Not a diagnostics UI.** Producing structured step failures is in scope; displaying
  them is spec 0033 / 0035.

## Requirements

1. **R1 — Composable flows.** A connection lifecycle is declared as a tree of steps with
   sequential and parallel grouping, where the declaration reads as the sequence itself
   rather than as a set of callbacks that infer their position from flags.
2. **R2 — Every step has a deadline.** Any step may declare a timeout; expiry is
   indistinguishable from a step failure to the enclosing group, and the step's own
   resources are released. A flow can therefore never hang forever, which is the current
   state of BLE (flow F9 has no timeout of any kind).
3. **R3 — Cancellation is immediate and total.** Cancelling a running flow stops the
   current step, releases what it holds, and runs no further step. Cancel is available at
   every point of the sequence, including while a retry backoff is pending. No step blocks
   the thread it runs on, so a cancel issued from the UI is always processed.
4. **R4 — Errors propagate with provenance.** A failing step reports a machine-readable
   step identity plus a human-readable reason; the enclosing group propagates the first
   failure and the flow reports it to its owner. No failure is silently discarded, and no
   error path terminates in an unhandled `default:`.
5. **R5 — One retry policy.** Retry-with-backoff exists in exactly one place: a named
   policy carrying the backoff schedule, the attempt cap, and the reset rule. Every flow
   that retries uses that policy; no flow carries its own interval or attempt loop.
6. **R6 — Automatic recovery from a dropped link.** When a link that was successfully
   open drops without the user asking for it, the owning flow re-runs the open sequence
   under the retry policy until it succeeds, the cap is reached, or the user cancels.
   Recovery emits the same connection-state signals a manual reconnect emits — no more
   often, and in the same order.
7. **R7 — v1 migration set.** Two flows migrate in v1: the TCP connect/retry/reconnect
   path (F1, plus F2/F3 as its supporting steps) and the MQTT reconnect path (F10 and
   F11, which collapse onto one shared policy). BLE discovery/pairing (F9) is v2. All
   other flows are untouched by v1.
8. **R8 — Behavior parity for migrated flows.** For every user-observable outcome that
   exists today, a migrated flow behaves the same: same success and failure end states,
   same signals, same error text, same persisted settings. Two deltas are deliberate and
   are the point of the change: the connect attempt no longer blocks the interface (R3),
   and a dropped link now recovers (R6).
9. **R9 — No leaks across repeated cycles.** Running a flow to completion, to failure, or
   to cancellation releases every timer, signal connection, socket, and driver object it
   created. Repeating a connect/drop/recover cycle many times returns the process to a
   steady state.
10. **R10 — Testable without hardware.** The engine's sequencing, timeout, retry, cancel,
    and error-propagation semantics are assertable with fake steps and a controllable
    clock, in-process, with no device, socket, or broker present.
11. **R11 — Incremental adoption.** A driver that has not been migrated continues to work
    through the existing synchronous path with no edit to that driver. Adding a driver to
    the orchestration layer is opt-in per driver.

## Acceptance Criteria

- [ ] **AC1 (R1, R4, R10)** — C++ unit tier (spec 0032): a suite over fake steps asserts
      sequential ordering, parallel completion, first-failure propagation, and that a
      reported failure carries both the step identity and a non-empty reason.
- [ ] **AC2 (R2, R3, R10)** — C++ unit tier: a step that never completes is failed by its
      timeout; a flow cancelled mid-step runs no subsequent step; a flow cancelled during
      a backoff wait never re-attempts. Asserted against a controllable clock, in
      milliseconds of wall time, not seconds.
- [ ] **AC3 (R5, R10)** — C++ unit tier: the retry policy produces the specified backoff
      sequence, stops at the cap, and resets its attempt count on a successful open.
- [ ] **AC4 (R3, R8)** — Maintainer observation: pointing the TCP source at an
      unreachable address and pressing connect leaves the interface responsive
      throughout, and pressing disconnect during the attempt returns to idle immediately.
      Today this freezes the window for roughly four and a half seconds.
- [ ] **AC5 (R6, R9)** — Integration test (maintainer runs against the live app with the
      API server on port 7777): a test severs a live TCP link 100 times mid-stream; after
      each severance the source reconnects on its own and frames resume, and after the
      hundredth cycle the process is at a steady state — no growth in open devices, active
      flows, or connection count. This is the roadmap's stated acceptance criterion for
      R10.
- [ ] **AC6 (R6, R8)** — Integration test: the same severance loop against the MQTT source
      recovers, and the publisher recovers, both under the one shared policy.
- [ ] **AC7 (R8)** — Integration test: the existing driver/API connection tests pass
      unchanged against the migrated flows; connect, disconnect, and error reporting keep
      their current outcomes.
- [ ] **AC8 (R4)** — Maintainer observation: a failed connect reports which step failed
      (address resolution, socket connect, subscribe) rather than a bare timeout, in a
      form spec 0035 can consume.
- [ ] **AC9 (R11)** — Code observation: no non-migrated driver is edited by the v1 diff
      beyond the shared base-class hook that gives them their default behavior.
- [ ] **AC10** — `--benchmark-hotpath` gates unchanged (no hotpath edit is expected at
      all; this is the negative check).

## Constraints & Invariants

- **Nothing on the hotpath.** No task tree, timer, or step runs per frame. The
  `Driver → FrameReader → FrameBuilder → Dashboard` path is not touched, its
  `Qt::DirectConnection` hops are unchanged, and no cached hotpath flag gains a new input.
- **No new threads and no mutexes.** Orchestration is thread-affine: a flow runs entirely
  on the thread that owns the driver it drives. `FrameReader`/`CircularBuffer` remain
  main-thread SPSC.
- **Reconnect must not amplify connection-state churn.** Connect/disconnect already bumps
  the frame-pool generation and drives `Dashboard` stream availability. Automatic recovery
  must emit those transitions exactly once per real state change, not once per attempt.
- **Timestamp ownership is unchanged.** Sources still stamp at the driver boundary; a
  reconnect does not re-stamp or backdate anything.
- **No heavy dependency.** Spec 0030's constraint applies: nothing heavier than a single
  header-friendly library, and the plan phase must confirm the candidate or specify the
  minimal in-repo equivalent.
- **The dual-license model is a hard filter on any dependency.** The commercial build ships
  a proprietary binary; anything vendored must permit that. The roadmap's description of
  the candidate library as BSD-licensed needs verification before it can be relied on
  (see Open Questions).
- **Composition-root ordering (spec 0001) is untouched.** Nothing here changes singleton
  construction order or reaches a singleton from a constructor closure.
- **The commercial/GPL driver split stays as-is.** MQTT is a commercial-build source;
  orchestration code shared with it must compile in the GPL build without it.
- **Existing driver contracts hold.** `configurationOk()` still checks the UI driver, live
  drivers may still have empty device lists, and every driver still emits
  `configurationChanged()` from its constructor.

## Open Questions

- **Library or in-repo?** Spec 0030 names one vetted candidate. Research on 2026-07-25
  found that the name covers two different things with two different licenses: Qt's
  TaskTree is an official but *Technology Preview* module in Qt 6.11 under
  LGPLv3/GPLv2/commercial, while Qt Creator's in-tree Tasking solution carries Qt
  Creator's own license. Neither is plainly BSD, which is what the roadmap assumed. The
  plan phase must resolve this against the commercial build's licensing before choosing.
- **What is the backoff schedule?** Proposal for the plan phase: a short first delay so a
  transient blip recovers invisibly, geometric growth to a ceiling of a few seconds, and
  an attempt cap that a user-initiated connect resets. Exact numbers, and whether the cap
  is finite at all for a source the user explicitly asked to stay connected, are a
  maintainer decision.
- **Is automatic recovery on by default?** R6 is a genuine behavior addition against R8's
  parity rule. Recommendation: on by default for sources the user explicitly connected,
  because the current behavior (silent death) is the defect the roadmap named. If the
  maintainer prefers strict parity in v1, recovery becomes a per-source setting and AC5
  runs with it enabled.
- **Does the user see recovery happening?** Some indication that a source is retrying
  rather than connected is arguably necessary, but it is UI, which the non-goals exclude.
  Proposal: v1 exposes the retrying state through existing connection state only, and
  spec 0035 owns surfacing it.
- **Does the API expose flow state?** AC5's leak assertion needs something observable
  over the API to count active flows. Reusing an existing status field is preferred to
  adding one; the plan phase confirms whether one suffices.
- **How far does v2 go?** F9 (BLE) is the largest win by line count but also the least
  covered by tests. Whether it lands as one migration or is split (discovery, then
  service/characteristic setup) is a plan-phase call.
