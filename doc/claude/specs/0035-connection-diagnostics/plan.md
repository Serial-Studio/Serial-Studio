---
spec: 0035-connection-diagnostics
phase: plan
status: draft        # draft -> approved (gate before /ss-tasks)
updated: 2026-07-25
---

# Plan 0035 — Connection diagnostics

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Read the relevant `doc/claude/` sub-docs and the *actual code*
> before writing this — a plan grounded in a stale mental model is worse than no plan.
> Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

Add one session-scoped singleton, `Misc::ConnectionDiagnostics`, that owns an ordered check
list, an `Async::TaskRunner`, and a per-bus result cache — and registers one
spec-0033 checker per bus that does nothing but *read that cache*. This split is forced by
the collector's contract: `ProblemCenter::Checker` is `std::function<void(QList<Finding>&)>`
and must return findings within the call, so an asynchronous probe cannot live inside a
checker. Checks are therefore classified as **instant** (plain functions, answered from
enumeration, `stat`/`access`, tracked adapter state, and permission status — microseconds,
no IPC) or **probing** (name resolution and a bounded TCP connect, expressed on the
spec-0034 task tree with explicit timeouts). A run executes the instant checks inline,
publishes immediately, then runs the probing checks on the event loop and publishes again
when the tree finishes. The decisive simplification is that *only host reachability is
probing*: Bluetooth reduces to the adapter-power boolean the driver already tracks plus a
read-only permission-status query, and audio reduces to backend-ready plus device
enumeration — so no diagnostics run ever starts a radio scan, contends with the driver's
shared discovery agent, or raises a permission dialog. The auto-on-failure trigger hangs off
`ConnectionManager::onDeviceOpenFinished`, which already receives `(deviceId, ok, reason)`
and currently `Q_UNUSED`s all three.

## Affected subsystems & files

| File | Change |
|------|--------|
| `app/src/Misc/ConnectionDiagnostics.h` | **New.** Singleton: `Bus` enum + `BusMask`, `Verdict` enum, `Result` struct, `run(BusMask)`, `runInstant(BusMask)`, `cancel()`, `onOpenFailed(...)`, `Q_PROPERTY running/lastRunTime/hasFailure/failureTitle/failureRemedy`, `Q_INVOKABLE runAll()`. Inert constructor (member init only). |
| `app/src/Misc/ConnectionDiagnostics.cpp` | **New.** Check registry, result cache, instant dispatch, task-tree assembly, `ProbeTask`, `setupExternalConnections()` (registers the five `diagnostics.*` checkers into `ProblemCenter`). |
| `app/src/Misc/Diagnostics/DiagnosticsShared.h` | **New.** `Result`/`Verdict`/`Bus` aliases, `trDiag()` (shared "Diagnostics" translation context), `makeResult(...)`, `toFinding(...)`, code-string constants. Mirrors the `Misc/Problems/*` file shape. |
| `app/src/Misc/Diagnostics/DeviceAccess.h` | **New.** Portable device-node access probe: `struct DeviceAccess { bool exists, readable, writable; QString ownerGroup; bool accountInGroup, sessionHasGroup; }` + `probeDeviceNode(const QString& path)`. |
| `app/src/Misc/Diagnostics/DeviceAccess.cpp` | **New.** POSIX implementation (`<unistd.h>`, `<sys/stat.h>`, `<grp.h>`, `<pwd.h>`): `stat` for `st_gid`, `access(R_OK|W_OK)` for ground truth, `getgrgid` for the group name and member list, `getgroups` for the live session's supplementary groups. Windows branch reports `exists` only. |
| `app/src/Misc/Diagnostics/SerialChecks.h/.cpp` | **New.** Instant checks: no ports present; selected port missing; port node not writable (grouped by owning group); per-platform driver-family remedy. |
| `app/src/Misc/Diagnostics/BluetoothChecks.h/.cpp` | **New.** Instant checks: platform support, adapter present + powered, Bluetooth permission status. |
| `app/src/Misc/Diagnostics/NetworkChecks.h/.cpp` | **New.** Instant: host/port configured and sane. Probing: `HostLookupTask` + `TcpProbeTask` + the reachability reporter, shared by the network source and (commercial) the MQTT source. |
| `app/src/Misc/Diagnostics/AudioChecks.h/.cpp` | **New, `BUILD_COMMERCIAL` only.** Instant: backend initialized, any input device present, selected input device still present, microphone permission status. |
| `app/src/IO/Drivers/BluetoothLE.h/.cpp` | Add `[[nodiscard]] static bool adapterPoweredOn();` — calls the existing (idempotent) `initializeSharedState()` and returns `s_adapterAvailable`. `adapterAvailable()` is an instance method that can read the flag before shared state exists; diagnostics need the static, self-initializing form. No behavior change. |
| `app/src/IO/Drivers/Audio.h` | Add `[[nodiscard]] bool backendReady() const noexcept { return m_init; }` — `m_init` has no accessor today. Header-inline getter only, no ctor change. |
| `app/src/IO/ConnectionManager.h/.cpp` | `onDeviceOpenFinished(int, bool, const QString&)` (`.h:221`, `.cpp:1300`) stops discarding its arguments: on `!ok`, resolve the failing device's bus and call `Misc::ConnectionDiagnostics::instance().onOpenFailed(bus, reason)` before `concludeConnectRequest()`. |
| `app/src/Misc/ModuleManager.cpp` | Three lines, mirroring `ProblemCenter` (`:627`, `:687`, `:779`): instantiate in `instantiateCoreModules()`, `setupExternalConnections()` in `setupCrossModuleConnections()`, context property `Cpp_Misc_ConnectionDiagnostics` in `registerCoreContextProperties()`. |
| `app/src/API/Handlers/DiagnosticsHandler.h/.cpp` | **New.** GPL static-only handler: `diagnostics.run`, `diagnostics.status`. |
| `app/src/API/CommandHandler.cpp` | One `#include` + `Handlers::DiagnosticsHandler::registerCommands();` in the GPL block of `initializeHandlers()` (`:218-272`), after `ProblemsHandler`. |
| `app/rcc/ai/command_safety.json` | Add `diagnostics.run` and `diagnostics.status` to `"safe"`. |
| `app/src/AI/ToolDispatcher.cpp` | Add a `diagnostics` entry to `scopeDescriptions()` — a new top-level scope otherwise gets an empty blurb in `meta.listCategories`. |
| `app/rcc/commands/app.json` | One manifest entry: `app.connectionDiagnostics`, title "Connection Diagnostics", icon `commands/tools`, contexts `["app","dashboard"]`, category `tools`, order 11. |
| `app/qml/Commands/AppCommandBindings.qml` | `"app.connectionDiagnostics": root.cmdConnectionDiagnostics` + the `QtObject`. |
| `app/qml/Commands/DashboardCommandBindings.qml` | Same id, dashboard-side binding. |
| `app/qml/main.qml` | `function runConnectionDiagnostics()` beside `showProblemCenter()` (`:495`): starts the run and opens the problem center. |
| `app/qml/MainWindow/Panes/Setup.qml` | Under `SetupPanes.Hardware { id: hardware }` (`:401`): a `Widgets.IconButton` "Run Connection Diagnostics" modelled on the "Open Project Editor" button (`:504-508`), plus a collapsed result banner bound to `Cpp_Misc_ConnectionDiagnostics.hasFailure` showing the title and a selectable, monospaced remedy. |
| `app/CMakeLists.txt` | `SOURCES`/`HEADERS` entries for the new C++ files (audio checks inside the `BUILD_COMMERCIAL` block at `:651`). |
| `tests/integration/test_connection_diagnostics.py` | **New.** AC5, AC6, AC8, AC9 (maintainer runs; app up with API server). |
| `tests/scripts/test_diagnostics_static.py` | **New.** Runnable static test: safety tiers, manifest entry, bindings present, checker ids match the handler's documented ids. |
| `doc/help/API-Reference.md` | New `### Diagnostics Commands (2)` section. |
| `doc/claude/architecture/io.md` | Record the diagnostics runner and the `onDeviceOpenFinished` hook. |

## Architecture & data flow

```
  run(BusMask scope)
     |
     |-- 1. clear cache slices for buses in scope
     |
     |-- 2. INSTANT checks (plain fns, inline, microseconds)
     |        SerialChecks / BluetoothChecks / NetworkChecks(config) / AudioChecks
     |        -> m_results[bus]
     |        -> ProblemCenter::runNow()          (findings visible immediately)
     |
     `-- 3. PROBING checks (Async task tree, event loop, bounded)
              SequentialGroup "connection-diagnostics"
                +- ProbeTask "network-reachability"
                |     `- timeout(15s overall root) > sequential
                |            +- HostLookupTask   (QHostInfo::lookupHost, 2s)
                |            `- timeout(TcpProbeTask, 3s)  (connect, abort, no bytes)
                `- ProbeTask "broker-reachability"        [BUILD_COMMERCIAL]
                      `- same shape against MQTT hostname/port
              TaskRunner::finished -> m_running=false
                                   -> ProblemCenter::runNow()
                                   -> Q_EMIT runFinished()

  ProblemCenter checkers (registered once, trigger = OnDemand):
      "diagnostics.serial" | "diagnostics.bluetooth" | "diagnostics.network"
      "diagnostics.broker" | "diagnostics.audio"
      each is `[bus](QList<Finding>& out){ appendCached(bus, out); }` — a pure reader.
```

**Why the cache/reader split is not optional.** `ProblemCenter::run()` calls each matching
checker and immediately snapshots its output (`ProblemCenter.cpp:246-270`). There is no
continuation and no way for a checker to say "ask me again later". The runner therefore
owns the results and the checkers are readers. Because `runNow()` fires
`ProjectChanged|LinkSample|OnDemand`, a user pressing the panel's re-run button re-reads
the cache without re-probing — correct, and the reason the diagnostics checkers must be
`OnDemand`-only and idempotent.

**`ProbeTask` — why a new node type.** `SequentialGroup` stops at the first non-`Success`
child and `ParallelGroup` cancels its siblings, so a failing probe would abort the rest of
the run. `ProbeTask` wraps one child, converts its `(Outcome, StepError)` into a cached
`Result` through a reporter callable, and always finishes `Success` — except on
`Cancelled`, which it propagates so `cancel()` genuinely stops the run. It lives in
`ConnectionDiagnostics.cpp`, not in `Async/`, because it is a diagnostics policy and not a
general combinator; if spec 0034 later grows a generic ignore-failure node this collapses
into it.

**Three reachability verdicts from one reporter.** `Async::StepError` carries
`{step, reason}` — its header comment states it exists "in a form the connection
diagnostics and problem-center specs can consume without parsing prose". The reporter
switches on `(outcome, error.step)`: `Failure` at step `host-lookup` → *name did not
resolve*; `Failure` at step `tcp-probe` → *connection refused*; `TimedOut` at either →
*timed out*. No string matching on `errorString()`.

**Linux device-group logic (the acceptance criterion).** `access(path, R_OK|W_OK)` is the
ground truth for pass/fail — it accounts for udev ACLs and `systemd-logind` `uaccess`
grants, which a naive group-membership test would falsely fail. The group data is used
*only to compose the remedy*:

| `access` | account in `gr_mem` | group in `getgroups()` | Verdict and remedy |
|---|---|---|---|
| ok | — | — | pass |
| denied | no | no | Error — `sudo usermod -aG <group> <user>`, then log out and back in |
| denied | yes | no | Error — membership already exists; **log out and back in** (no command repeated) |
| denied | yes | yes | Error — group membership is not the cause; name the mode/owner and point at udev rules |

The group name is read from the node's actual `st_gid`, so `dialout`, `uucp`, and `plugdev`
are all handled without a hardcoded table. The account name comes from `getpwuid(getuid())`.
Ports sharing one owning group produce **one** finding naming them, not one per port.

**Threading.** Everything is main-thread. `TaskRunner` is constructed on the main thread and
its `SystemClock` binds its timers there; `QTcpSocket` and `QHostInfo::lookupHost` are used
in their asynchronous, signal-based forms only. No worker thread, no mutex, no
`waitFor*`, no nested event loop, no `QThread::msleep` — the F1/F13 defects spec 0034 exists
to remove are not reintroduced here.

## Hotpath & threading impact

- **Touches the hotpath?** **No.** Nothing in this feature is reachable from
  `FrameReader`, `CircularBuffer`, `FrameBuilder`, the span fast lane, or the Dashboard
  draw. No counter is added to any parse-path object. The only I/O-layer edit is inside
  `ConnectionManager::onDeviceOpenFinished`, which runs once per settled open attempt.
- **New cross-thread signal/slot?** **No.** Every object created here lives on the main
  thread; all connections are same-thread `AutoConnection` (direct in practice).
- **New input to a cached hotpath flag?** **No.** Diagnostics neither gate nor are gated by
  `m_operationMode`, `m_playerOpen`, `m_anyAsyncSink`, `m_captureLatestFrame`,
  `m_changeDriven`, or Dashboard `m_streamAvailable`.
- **Timestamp ownership** — unaffected. Diagnostics never see a frame and never stamp one.
- **Idle cost** — zero. No timer is armed unless a run is in progress; `TaskRunner` holds no
  timer between runs and `SystemClock` schedules only from a live `TimeoutTask`.

## Data model & persistence

None. No `Frame.h` `Keys::` additions, no project-JSON change, no schema version bump, no
`widgetSettings` entry, no Sessions DB table, no `QSettings` key. Results are session state
in memory and are discarded on exit. The one persistence-adjacent read is of existing driver
settings (hostname, port, selected device index), and it is read-only.

## API / SDK surface

Two new commands in a new `diagnostics` scope, registered from
`app/src/API/Handlers/DiagnosticsHandler.cpp` and hooked into
`CommandHandler::initializeHandlers()` in the unconditional (GPL) block after
`ProblemsHandler`. Both are non-mutating and go in the `safe` tier.

**`diagnostics.run`** — optional `bus` (string slug: `serial`, `bluetooth`, `network`,
`broker`, `audio`; omitted means every bus the build supports). Starts the run and returns
**immediately**, following the `sessions.exportToCsv` ack-and-poll precedent
(`SessionsHandler.cpp:506-524`) — the dispatch signature `CommandResponse(QString, QJsonObject)`
is synchronous and cannot defer a response, and blocking it would be the exact defect this
spec forbids. The response carries the instant results, which are already complete:

```jsonc
{
  "started": true,
  "running": true,                 // false when the scope had no probing checks
  "buses": ["serial", "network"],
  "instant": [ { "bus", "verdict", "code", "title", "explanation", "remedy" } ],
  "probing": ["network"],          // which buses still have work in flight
  "estimatedMs": 10000,            // declared worst case for the started scope
  "hint": "Poll diagnostics.status, then read findings with problems.list."
}
```

An agent diagnosing the Linux group case gets its answer in this first response — the
serial checks are instant, so no polling is needed for the acceptance-criterion path.

**`diagnostics.status`** — no params. `{ running, lastRun, buses, counts: {pass, info,
warning, failure}, hint }`.

Findings are read through the existing `problems.list` filtered by
`checkerId: "diagnostics.<bus>"`. **No third command duplicates that read surface** — this
is the recommendation for the spec's open question on scope naming: a distinct
`diagnostics` scope for the two verbs (so an agent finds them under the feature's name),
and the existing `problems` scope for the read. `ToolDispatcher::scopeDescriptions()` gains
a `diagnostics` blurb so the new scope is not blank in `meta.listCategories`.

## QML / UI

- **Command.** `app.connectionDiagnostics` in `app/rcc/commands/app.json` (icon
  `commands/tools`, already shipped in all four tiers — no new SVG, no `registry-verify.py`
  icon work), bound in `AppCommandBindings.qml` and `DashboardCommandBindings.qml` to
  `app.runConnectionDiagnostics()`.
- **`main.qml`.** `function runConnectionDiagnostics() { Cpp_Misc_ConnectionDiagnostics.runAll(); problemCenter.activate(); }` beside `showProblemCenter()` (`:495`).
- **`Setup.qml`.** The spec's R12 surface. Under `SetupPanes.Hardware { id: hardware }`
  (`:401`), inside the same `ColumnLayout` so it is visible in every operation mode:
  a `Widgets.IconButton` ("Run Connection Diagnostics", icon `commands/tools`) modelled on
  the existing "Open Project Editor" button (`:504-508`), and beneath it a banner visible
  only while `Cpp_Misc_ConnectionDiagnostics.hasFailure`, showing `failureTitle` plus
  `failureRemedy` in a read-only, selectable, monospaced `TextEdit` so the command can be
  copied verbatim, and a link that opens the problem center for the full list.
  This banner is what the user sees after a failed connect — persistent, next to the port
  selector they must change, and not dependent on the transient connecting overlay's
  lifetime.
- **No new panel, no new model.** The list lives in the spec-0033 `ProblemCenter.qml`
  window, which already renders `title` / `explanation` / `remedy` per row.
- **Prerequisite, not owned here:** `app.problems` is declared in the manifest but bound in
  no `*CommandBindings.qml`, so `app.showProblemCenter()` has zero callers and the panel is
  currently unreachable. That binding belongs to spec 0033. If 0033 is still open when this
  lands, it lands there; if 0033 has closed, T0 below adds it as an explicitly-flagged
  one-line prerequisite.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Async results vs. the synchronous checker contract | (a) extend `ProblemCenter::Checker` to an async form; (b) runner owns results, checkers read a cache | **(b)** — (a) would change a contract spec 0033 just shipped and force every existing checker to care about completion. The cache split is ~30 lines and leaves 0033 untouched. |
| Bluetooth probe | (a) start a short discovery scan and read the agent's error; (b) read the driver's tracked adapter-power flag + permission status | **(b)** — (a) contends with the driver's process-wide shared discovery agent, is slow, and raises a macOS permission dialog the user did not ask for. (b) is instant, free, and cannot interfere. Cost: a broken-but-powered adapter is not detected. |
| Serial permission ground truth | (a) group-membership comparison; (b) `access(R_OK\|W_OK)`; (c) both | **(c) with (b) deciding** — a udev ACL or `uaccess` grant makes (a) report a false failure. `access` decides pass/fail; group data only composes the remedy. |
| Sequencing engine | (a) hand-rolled `QTimer` + state flags; (b) the spec-0034 task tree | **(b)** — the timeout/cancel semantics are exactly what `TimeoutTask` provides, and `StepError.step` gives the three reachability verdicts without parsing error strings. Cost: a dependency on a spec that is still `draft`. |
| Failure-not-aborting-the-run | (a) `ParallelGroup`; (b) run each probe on its own `TaskRunner`; (c) a local `ProbeTask` wrapper | **(c)** — (a) cancels siblings on first failure, (b) loses ordering and the single cancel point. ~40 lines, local to the diagnostics TU. |
| API response for a slow run | (a) block the handler; (b) ack + `diagnostics.status` polling | **(b)** — the registry's `CommandFunction` is synchronous; blocking it stalls the API connection and the UI. Matches `sessions.exportToCsv`. |
| Where the remedy is shown after a failure | (a) modal box; (b) a second targeted notification; (c) a persistent banner in the setup pane | **(c)** — (a) is the pattern the app is moving away from; (b) duplicates the problem center's aggregate notification. (c) puts the command next to the control the user must change and needs no reasoning about the connecting overlay's lifetime. |
| ModemManager grabbing the port (Linux) | (a) ship a D-Bus service-registered check; (b) defer | **(b) defer** — it is the second-most-common Linux serial failure, but it is IPC, which makes it a probing check, and it pulls D-Bus into `Misc/`. Worth a follow-up once the instant set is proven. |
| "Chip present, no driver" on Windows/macOS | (a) enumerate USB VID/PID and match a chip table; (b) report "no ports found" with the common driver families | **(b)** — (a) needs the USB layer, which only exists in commercial builds, so the check would be missing exactly where a GPL user needs it. (b) works everywhere for near-zero cost. |
| Diagnostics icon | (a) reuse `commands/tools`; (b) add a dedicated SVG in all four tiers | **(a)** — the command sits in the Tools submenu and reuse costs nothing. A dedicated icon is a later cosmetic change through the spec-0028 pipeline. |

## Risks & mitigations

- **The auto-run feeds itself.** A probe that fails could be mistaken for a connection
  failure and re-trigger. *Mitigation:* diagnostics never open a driver and never emit
  `openFinished`; the auto-run is additionally rate-limited per bus (30 s window,
  `QElapsedTimer` per bus), and `SupervisorTask` already collapses retries so
  `onDeviceOpenFinished` fires once per *settled* attempt, not once per retry.
- **Re-entrancy: a second run while one is in flight.** *Mitigation:* `TaskRunner::run()`
  cancels the previous root by contract; `ConnectionDiagnostics::run()` additionally
  refuses when `m_running` and the requested scope is already covered, so results cannot
  interleave.
- **Lifetime: a probe outliving the driver whose config it read.** *Mitigation:* the probe
  captures the hostname and port **by value** at build time; it never holds a driver
  pointer. `TaskRunner`'s destructor cancels silently.
- **Firewall / outbound-connection prompts.** A TCP probe to a broker can raise a host
  firewall prompt. *Mitigation:* probes run only for a host the user has already configured
  and only on an explicit or failure-triggered run — never on a timer, never at startup.
- **`initializeSharedState()` may not have run** when the static BLE accessor is first
  called, so `s_adapterAvailable` would read stale-false. *Mitigation:* the new static
  calls `initializeSharedState()` itself; confirm it is idempotent (guarded on the static
  local-device pointer) before relying on it, and that calling it does not start discovery.
- **Composition-root recursion (spec 0001).** A constructor that reaches
  `ConnectionManager::instance()` or `ProblemCenter::instance()` aborts at startup — this
  has shipped and crashed before. *Mitigation:* the constructor is member-init only; every
  singleton reach happens in `setupExternalConnections()`, exactly as `ProblemCenter` does.
  Adding an entry to `instantiateCoreModules()` re-triggers the ctor-edge proof; a node with
  no outgoing constructor edges keeps it trivial.
- **Translation of a shell command.** A translator "helpfully" localizing
  `sudo usermod -aG dialout alex` produces a command that does not work. *Mitigation:* the
  sentence is `trDiag("Run %1, then log out and back in.")`; the command is a `%1`
  argument assembled from untranslated literals and never appears inside a `tr()` string.
- **`code-verify.py` in-body comments and function length.** The Linux group logic wants
  narration. *Mitigation:* split into `probeDeviceNode()` / `describeGroupRemedy()` /
  `groupRemedyCommand()` so each stays inside the 40-80-line band and the *why* folds into
  each `@brief`, per the repo comment rule.
- **Findings can go stale.** A reachability result stands until the next run. *Mitigation
  (pending the spec's open question):* the recommendation is to clear a bus's cached
  results on a successful open of that bus, which is a one-line addition to the same
  `onDeviceOpenFinished` hook on the `ok` branch.

## Test & verification plan

- **Unit (runnable here):** `tests/scripts/test_diagnostics_static.py` — asserts
  `diagnostics.run` / `diagnostics.status` are in exactly one safety tier and that tier is
  `safe`; asserts `app.connectionDiagnostics` exists in `app/rcc/commands/app.json` with an
  icon that resolves in the icon tree; asserts both `*CommandBindings.qml` files bind the
  id; asserts the five checker ids in `ConnectionDiagnostics.cpp` match the ids the handler
  documents. No app, no Qt.
- **Integration (maintainer runs, app up with API server on 7777):**
  `tests/integration/test_connection_diagnostics.py` — AC5 (three distinct reachability
  verdicts against an unresolvable name, a closed local port, and a black-holed address),
  AC6 (a local listener sees a connection carrying zero bytes), AC8 (`diagnostics.run`
  returns immediately with instant results; `diagnostics.status` reports completion within
  the declared worst case; `problems.list` then returns the diagnostics findings), AC9
  (bus-scoped run produces findings for that bus only; a second failure inside the
  rate-limit window starts no second probing run).
- **Hardware / platform (maintainer runs, no automation possible):** AC1 and AC2 need a
  Linux machine, a USB-serial adapter, and an account outside the device group — including
  the "added but not re-logged-in" state, which cannot be simulated in-process. AC3 needs
  each of the three platforms with no adapter attached. AC4 needs a machine with a
  Bluetooth adapter that can be switched off. AC7 needs a commercial build and a removable
  audio input device. AC10 and AC11 are observations in the running app.
- **Hotpath:** `--benchmark-hotpath` (AC12) as a no-regression confirmation only; the
  feature does not touch the parse path, so this is a guard, not a tuning loop.
- **Static:** `python scripts/code-verify.py --check` on every changed file;
  `python scripts/registry-verify.py` after the manifest and bindings land;
  `qt-cpp-review` on the C++ diff before handoff; `python scripts/sanitize-commit.py`
  before commit.
