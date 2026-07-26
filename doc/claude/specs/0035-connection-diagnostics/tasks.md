---
spec: 0035-connection-diagnostics
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement); T0-T18 code-complete,
                     # maintainer acceptance (AC1-AC4, AC7, AC10-AC12) and the CMake
                     # registration below still pending
updated: 2026-07-25
---

# Tasks 0035 — Connection diagnostics

> **Phase 3 of 4 — the ordered checklist.** Decompose [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable* — each one a coherent diff a reviewer
> could read in isolation. `/ss-implement` works this list top to bottom and keeps the status
> boxes current. Gate: do not start `/ss-implement` until a human marks this `approved`.

## Conventions

- One task = one focused, reviewable change. Each task that creates a C++ file also adds its
  `SOURCES` / `HEADERS` entry to `app/CMakeLists.txt` in the same diff; that CMake edit does
  not count against the three-file guideline.
- **Verify** is how *this* unit is confirmed before moving on. The developer builds and runs
  the app — never invoke `cmake`, a compiler, or the binary from a task.
- **Deps** lists task IDs that must land first.
- Tasks marked **[maintainer]** cannot be verified without the maintainer running the app;
  tasks marked **[hardware]** additionally need physical hardware or a specific OS state.
- The tree must stay conceptually compilable after every task: T1-T3 land a runner with no
  checks registered, and each later check task adds one populated slice.

## Tasks

### T0 — Confirm the problem-center panel is reachable

- **Files:** none (verification only), or `app/qml/Commands/AppCommandBindings.qml` if the
  binding must be added here.
- **Does:** Spec 0033 declared `app.problems` in `app/rcc/commands/app.json` but bound it in
  no `*CommandBindings.qml`, so `app.showProblemCenter()` has zero callers and the panel is
  unreachable. Every acceptance criterion that ends "the user sees the finding" depends on
  it. Confirm the binding exists. If spec 0033 is still open, the fix lands **there** — do
  not absorb it. Only if 0033 has closed, add the one binding here and say so in the commit.
- **Verify:** `grep -n "app.problems" app/qml/Commands/*.qml` returns a binding; the command
  appears in the palette. **[maintainer]** for the palette half.
- **Deps:** none
- [x] done -- verification only. The binding already exists in `AppCommandBindings.qml:57` and
  `ProjectEditorCommandBindings.qml:81`; spec 0033 closed the gap. Nothing absorbed here.

### T1 — Diagnostics types and the runner header

- **Files:** `app/src/Misc/Diagnostics/DiagnosticsShared.h`,
  `app/src/Misc/ConnectionDiagnostics.h`
- **Does:** Declares `Bus` / `BusMask` / `Verdict` / `Result`, the shared `trDiag()`
  translation helper and `makeResult()`, and the `Misc::ConnectionDiagnostics` singleton:
  `run(BusMask)`, `runInstant(BusMask)`, `cancel()`, `onOpenFailed(Bus, QString)`,
  `setupExternalConnections()`, the `running` / `lastRunTime` / `hasFailure` /
  `failureTitle` / `failureRemedy` properties, and `Q_INVOKABLE runAll()`. Constructor is
  private and declared inert (member init only, per the spec-0001 constraint); no
  in-header member initialization.
- **Verify:** `python scripts/code-verify.py --check app/src/Misc/ConnectionDiagnostics.h
  app/src/Misc/Diagnostics/DiagnosticsShared.h` clean. Header block order and
  `[[nodiscard]]` coverage match `app/src/Misc/ProblemCenter.h`.
- **Deps:** none
- [x] done -- `Misc/Diagnostics/DiagnosticsShared.h` (Bus/BusMask/Verdict/Result, `trDiag`,
  `busSlug`/`busFromSlug`/`checkerId`/`verdictName`, `makeResult`, `severityOf`, `toFinding`,
  all header-inline) + `Misc/ConnectionDiagnostics.h`. `Q_INVOKABLE runAll()` landed as a
  `public slots:` entry instead, per the CLAUDE.md "never Q_INVOKABLE void" rule.

### T2 — Runner core: cache, instant dispatch, checker registration

- **Files:** `app/src/Misc/ConnectionDiagnostics.cpp`, `app/CMakeLists.txt`
- **Does:** Implements the per-bus result cache, the instant-check dispatch table (empty for
  now), `runInstant()`, `runAll()`, the property getters, and `setupExternalConnections()`
  registering the five `diagnostics.serial` / `.bluetooth` / `.network` / `.broker` /
  `.audio` checkers into `Misc::ProblemCenter` with trigger `OnDemand` only. Each checker is
  a pure reader that converts cached `Result`s into `ProblemCenter::Finding`s. Every
  singleton reach happens here, never in the constructor.
- **Verify:** `python scripts/code-verify.py --check` clean on the file. Read back: the
  constructor body contains no `::instance()` call; the checkers contain no probing work.
- **Deps:** T1
- [x] done -- cache, instant dispatch, `store`/`clearScope`/`publish`/`appendCached`, property
  getters, and the five `OnDemand` checkers registered in `setupExternalConnections()`. The
  runner's `TaskRunner::finished` hop is wired there too, so the constructor stays a leaf.
  **CMake entry still owed** (see the note at the end of this file).

### T3 — Composition-root wiring and the QML context property

- **Files:** `app/src/Misc/ModuleManager.cpp`
- **Does:** Three lines mirroring `ProblemCenter` (`:627`, `:687`, `:779`): instantiate in
  `instantiateCoreModules()`, call `setupExternalConnections()` in
  `setupCrossModuleConnections()`, and expose `Cpp_Misc_ConnectionDiagnostics` in
  `registerCoreContextProperties()`. Re-run the spec-0001 ctor-edge proof and record that the
  new node has no outgoing constructor edges.
- **Verify:** `python scripts/code-verify.py --check` clean; the ctor-edge argument is
  written into the commit message. **[maintainer]** app starts without the Meyers-guard
  abort.
- **Deps:** T2
- [x] done -- `instantiateCoreModules()` right after `ProblemCenter`,
  `setupExternalConnections()` in `setupCrossModuleConnections()`, and the
  `Cpp_Misc_ConnectionDiagnostics` context property. Ctor-edge proof: the constructor is a
  member-init list only (`m_running`, `m_activeScope`, `m_lastRun`, `m_runner`, `m_results`,
  `m_autoRunClocks`) with no `::instance()` call and no `connect()`, so the new node has zero
  outgoing constructor edges and the proof stays trivial.

### T4 — Portable device-node access probe

- **Files:** `app/src/Misc/Diagnostics/DeviceAccess.h`,
  `app/src/Misc/Diagnostics/DeviceAccess.cpp`, `app/CMakeLists.txt`
- **Does:** `probeDeviceNode(path)` returning `{exists, readable, writable, ownerGroup,
  accountInGroup, sessionHasGroup}`. POSIX branch: `stat` for `st_gid`,
  `access(R_OK|W_OK)` as the ground truth (so a udev ACL or `uaccess` grant is not reported
  as a failure), `getgrgid` for the group name and its member list, `getgroups` for the live
  session's supplementary groups, `getpwuid(getuid())` for the account name. Windows branch
  fills `exists` and leaves the rest at their unknown defaults.
- **Verify:** `python scripts/code-verify.py --check` clean; each function inside the
  40-80-line band; no in-body comments; `[[nodiscard]]` on every non-void return.
- **Deps:** T1
- [x] done -- `probeDeviceNode()` (stat + access + getgrgid + getgroups + getpwuid) and
  `currentAccountName()`. Added one field beyond the plan's struct, `accessKnown`, because the
  Windows branch needs a way to say "unknown" that a bool pair cannot express. Both group loops
  carry fixed bounds (`kMaxGroupMembers`, `kMaxSupplementaryGroups`).

### T5 — Serial checks

- **Files:** `app/src/Misc/Diagnostics/SerialChecks.h`,
  `app/src/Misc/Diagnostics/SerialChecks.cpp`, `app/CMakeLists.txt`
- **Does:** Instant checks fed from `QSerialPortInfo::availablePorts()` and
  `ConnectionManager::instance().uart()`: (1) no ports present, with a per-platform remedy
  naming the common USB-serial driver families; (2) the selected port no longer present;
  (3) enumerated port nodes that are not writable, grouped by owning group so N ports in one
  group produce one finding. The remedy follows the four-row table in `plan.md` — the
  `usermod` command appears only in the "not a member" row, and the "member but stale
  session" row says to log out and back in instead. The command is assembled from
  untranslated literals and passed as a `%1` argument into a translated sentence.
- **Verify:** `python scripts/code-verify.py --check` clean. Read back: no shell command
  appears inside a `tr()` / `trDiag()` string literal.
- **Deps:** T2, T4
- [x] done -- no-ports (per-platform driver-family remedy), selected-port-missing (read from the
  driver's persisted `IO_Serial_SelectedDevice`), and inaccessible nodes grouped by owning group
  through the plan's four-row table. Every command (`sudo usermod ...`, `lsusb`, module names) is
  a `%1` argument assembled from untranslated literals; the static test asserts it.

### T6 — BluetoothLE static adapter accessor

- **Files:** `app/src/IO/Drivers/BluetoothLE.h`, `app/src/IO/Drivers/BluetoothLE.cpp`
- **Does:** Adds `[[nodiscard]] static bool adapterPoweredOn();` which calls the existing
  `initializeSharedState()` and returns `s_adapterAvailable`, so diagnostics get a
  self-initializing read without depending on whether the driver has discovered yet. Confirm
  `initializeSharedState()` is idempotent and does **not** start discovery before relying on
  it; if it does, gate the new accessor on the already-initialized path instead. No other
  behavior change; the shared discovery agent is not touched.
- **Verify:** `python scripts/code-verify.py --check` clean; read back that no call path
  from the new accessor reaches `startDiscovery()`.
- **Deps:** none
- [x] done -- `adapterPoweredOn()` calls `initializeSharedState()`, which is guarded on
  `s_initialized`, constructs only a `QBluetoothLocalDevice`, and contains no `start()` call:
  discovery is started from `startDiscovery()` alone, which this path never reaches.

### T7 — Bluetooth checks

- **Files:** `app/src/Misc/Diagnostics/BluetoothChecks.h`,
  `app/src/Misc/Diagnostics/BluetoothChecks.cpp`, `app/CMakeLists.txt`
- **Does:** Instant checks only: platform support, adapter present and powered on (via T6),
  and Bluetooth permission **status** where the platform has one — status is read, never
  requested, so no system dialog can appear as a side effect of a run. Each failure carries
  the platform-appropriate remedy (turn Bluetooth on; grant the permission in system
  settings).
- **Verify:** `python scripts/code-verify.py --check` clean. Read back: no
  `QBluetoothDeviceDiscoveryAgent` is constructed, no `QBluetoothLocalDevice` is constructed,
  and no permission *request* call exists.
- **Deps:** T2, T6
- [x] done -- platform support (via the driver's `operatingSystemSupported()`), permission
  status through `QCoreApplication::checkPermission(QBluetoothPermission{})` behind
  `QT_CONFIG(permissions)`, and adapter power via T6. No discovery agent, no local device, and no
  `requestPermission` anywhere in the file.

### T8 — `ProbeTask` and the async run assembly

- **Files:** `app/src/Misc/ConnectionDiagnostics.cpp`
- **Does:** Adds the `ProbeTask` wrapper (owns one child, converts `(Outcome, StepError)`
  into a cached `Result` through a reporter callable, always finishes `Success` except on
  `Cancelled`, which propagates), the `Async::TaskRunner` member, `run(BusMask)` building the
  root `SequentialGroup` under an overall timeout, `cancel()`, the re-entrancy guard, and the
  `TaskRunner::finished` handler that clears `m_running`, calls `ProblemCenter::runNow()`,
  and emits `runFinished()`. No probes exist yet — the tree is empty until T9.
- **Verify:** `python scripts/code-verify.py --check` clean. Read back: no `waitFor*`, no
  `QEventLoop`, no `QThread::msleep`, no `processEvents` anywhere in the file; every probe
  captures configuration by value and holds no driver pointer.
- **Deps:** T2
- [x] done -- `ProbeTask` (in `Misc::detail`, no `Q_OBJECT` since it adds no signal), the
  `TaskRunner` member, `run(BusMask)` under a 15 s root timeout, `cancel()`, the re-entrancy
  refusal, and the `finished` handler. No `waitFor*`, `QEventLoop`, `processEvents` or sleep
  anywhere in the diagnostics sources -- asserted by `tests/scripts/test_diagnostics_static.py`.

### T9 — Host and broker reachability probes

- **Files:** `app/src/Misc/Diagnostics/NetworkChecks.h`,
  `app/src/Misc/Diagnostics/NetworkChecks.cpp`, `app/CMakeLists.txt`
- **Does:** `HostLookupTask` (async `QHostInfo::lookupHost`, id retained, `doCancel()` calls
  `abortHostLookup`, literal addresses short-circuit) and `TcpProbeTask` (async
  `connectToHost`, `abort()` on `connected`, never writes or reads a byte, never performs a
  TLS or protocol handshake), composed as `sequential(HostLookup, timeout(TcpProbe))` inside
  a `ProbeTask` whose reporter maps `(Outcome, StepError::step)` onto the three distinct
  verdicts — name did not resolve / connection refused / timed out — each with its own
  remedy. One instance for the network source, one `BUILD_COMMERCIAL` instance for the MQTT
  source; both skipped when the host is unconfigured. Also the instant config sanity checks
  (empty host, zero port).
- **Verify:** `python scripts/code-verify.py --check` clean. Read back: the reporter switches
  on `StepError::step`, never on an `errorString()` substring; no `write()` or `read()` call
  on the probe socket.
- **Deps:** T8
- [x] done -- `HostLookupTask` (literal-address short circuit, `abortHostLookup` on cancel) and
  `TcpProbeTask` (connect, `abort()` on `connected`, never `read()`/`write()`), composed as
  `sequential(timeout(lookup, 2s), timeout(probe, 3s))`. The reporter switches on
  `StepError::step` only. **CMake entry still owed.**

### T10 — Audio checks (commercial)

- **Files:** `app/src/IO/Drivers/Audio.h`, `app/src/Misc/Diagnostics/AudioChecks.h`,
  `app/src/Misc/Diagnostics/AudioChecks.cpp`, `app/CMakeLists.txt`
- **Does:** Adds `[[nodiscard]] bool backendReady() const noexcept` to `Audio` (`m_init` has
  no accessor today; header-inline getter, no constructor change), then instant checks:
  backend initialized, at least one input device present, the previously selected input
  device still present (named in the finding), and microphone permission **status** where the
  platform has one. Everything in this task, including the CMake entries, sits inside the
  `BUILD_COMMERCIAL` block.
- **Verify:** `python scripts/code-verify.py --check` clean; read back that a GPL
  configuration compiles the runner with the audio slice absent, not with a stubbed-out
  checker.
- **Deps:** T2
- [x] done -- `Audio::backendReady()` header-inline getter plus backend / no-inputs /
  selected-input-gone / microphone-permission checks. The whole `AudioChecks` slice sits behind
  `BUILD_COMMERCIAL` at the call site in `runInstant()`, so a GPL build compiles the runner
  without it rather than with a stub. **CMake entry owed inside the commercial block.**

### T11 — Auto-run on a failed connection

- **Files:** `app/src/IO/ConnectionManager.cpp`, `app/src/IO/ConnectionManager.h`
- **Does:** `onDeviceOpenFinished(int deviceId, bool ok, const QString& reason)` stops
  discarding its arguments. On `!ok`: resolve the failing device's bus, and call
  `ConnectionDiagnostics::onOpenFailed(bus, reason)`, which runs that bus's instant checks
  inline (so the remedy is available immediately), publishes, and starts the bus-scoped
  probing run — rate-limited per bus by a `QElapsedTimer` so a retry loop cannot spam probes.
  On `ok`: clear that bus's cached results, per the plan's staleness recommendation.
  `concludeConnectRequest()` still runs on every path.
- **Verify:** `python scripts/code-verify.py --check` clean. Read back: the hook is scoped to
  the failing bus only, and nothing on this path can open a driver. **[maintainer]** a failed
  connect produces a diagnostics finding and a successful one clears it.
- **Deps:** T5, T9
- [x] done -- `diagnosticsBusFor()` resolves the failing device's bus by `qobject_cast` on its
  driver; `!ok` calls `onOpenFailed(bus, reason)` (instant checks always, probing run only when
  the per-bus 30 s `QElapsedTimer` window has elapsed), `ok` calls `onOpenSucceeded(bus)` which
  clears that bus's cache and re-arms the window. `concludeConnectRequest()` still runs on every
  path, and nothing on this path can open a driver.

### T12 — API handler

- **Files:** `app/src/API/Handlers/DiagnosticsHandler.h`,
  `app/src/API/Handlers/DiagnosticsHandler.cpp`, `app/src/API/CommandHandler.cpp`,
  `app/CMakeLists.txt`
- **Does:** GPL static-only handler registering `diagnostics.run` (optional `bus` slug;
  starts the run and returns immediately with the completed instant results, the buses still
  probing, and the declared worst-case duration) and `diagnostics.status`. Follows the
  `ProblemsHandler.cpp:128-170` registration form and the `sessions.exportToCsv`
  ack-and-poll precedent; the handler never blocks. One `#include` plus one
  `registerCommands()` line in the GPL block of `initializeHandlers()`, after
  `ProblemsHandler`.
- **Verify:** `python scripts/code-verify.py --check` clean. Read back: no `QEventLoop`, no
  `waitFor*`, and no busy loop in the handler; an unknown `bus` slug returns
  `ErrorCode::InvalidParam` naming the valid values.
- **Deps:** T8
- [x] done -- `diagnostics.run` (optional `bus`, ack-and-poll, carries the completed instant
  results, the buses still probing and `estimatedMs`) and `diagnostics.status`. An unknown or
  unsupported slug returns `InvalidParam` naming the valid values. Registered in the GPL block of
  `initializeHandlers()` after `ProblemsHandler`. **CMake entry owed.**

### T13 — Assistant safety tier and scope description

- **Files:** `app/rcc/ai/command_safety.json`, `app/src/AI/ToolDispatcher.cpp`
- **Does:** Adds `diagnostics.run` and `diagnostics.status` to the `safe` tier (each command
  must appear in exactly one tier) and a `diagnostics` entry to `scopeDescriptions()` so the
  new top-level scope is not blank in `meta.listCategories`.
- **Verify:** `python scripts/registry-verify.py` clean;
  `python scripts/code-verify.py --check app/src/AI/ToolDispatcher.cpp` clean; the JSON
  parses and neither id appears in a second tier.
- **Deps:** T12
- [x] done -- both ids in the `safe` tier only, plus a `diagnostics` blurb in
  `scopeDescriptions()`. `registry-verify.py` clean.

### T14 — Command manifest and bindings

- **Files:** `app/rcc/commands/app.json`, `app/qml/Commands/AppCommandBindings.qml`,
  `app/qml/Commands/DashboardCommandBindings.qml`
- **Does:** One manifest entry — `app.connectionDiagnostics`, title "Connection
  Diagnostics", icon `commands/tools` (already shipped in all four tiers), contexts
  `["app","dashboard"]`, category `tools`, order 11 — plus the matching binding in each
  context's bindings file, routed to `app.runConnectionDiagnostics()`.
- **Verify:** `python scripts/registry-verify.py` clean (manifest schema, id, icon, icon
  render-size lint, binding presence). **[maintainer]** the command appears in the palette
  and the Start menu Tools submenu in both contexts.
- **Deps:** T2
- [x] done -- `app.connectionDiagnostics` (title "Connection Diagnostics", icon `commands/tools`,
  category `tools`, order 11) declared once in `app/rcc/commands/app.json`, with contexts
  `["app","dashboard","editor"]` and bindings in `AppCommandBindings.qml` +
  `ProjectEditorCommandBindings.qml` -- the `app.problems` shape, so the editor window reaches it
  too (the plan's `DashboardCommandBindings.qml` holds no `app.*` entries; the dashboard palette
  already consumes `AppCommandBindings` through its `[dashboard, app]` binding-set order). Each
  binding routes to `app.runConnectionDiagnostics()` and is disabled while a run is in flight.
  `scripts/generate-command-strings.py` re-run; `registry-verify.py` clean. `commands/tools` ships
  in tiers 24 and 32 (not all four, as `plan.md` claimed) -- it resolves, so no new SVG.

### T15 — Setup-pane entry point and result banner

- **Files:** `app/qml/main.qml`, `app/qml/MainWindow/Panes/Setup.qml`
- **Does:** `main.qml` gains `function runConnectionDiagnostics()` beside
  `showProblemCenter()` (`:495`), starting the run and opening the problem center.
  `Setup.qml` gains, under `SetupPanes.Hardware { id: hardware }` (`:401`), a
  `Widgets.IconButton` modelled on the "Open Project Editor" button (`:504-508`), and beneath
  it a banner shown only while `Cpp_Misc_ConnectionDiagnostics.hasFailure` that displays
  `failureTitle` and `failureRemedy` in a read-only, selectable, monospaced field so the
  command can be copied verbatim, plus a link that opens the full list. Theme tokens only —
  no hard-coded colors, sizes, or spacing.
- **Verify:** `python scripts/code-verify.py --check` clean on both QML files;
  `python scripts/registry-verify.py` clean. **[maintainer]** the button runs a check, the
  banner appears after a failure and clears after a success, and the remedy text can be
  selected and copied.
- **Deps:** T11, T14
- [x] done -- `main.qml` gained `runConnectionDiagnostics()` beside `showProblemCenter()` (starts
  the run, then opens the panel). `Setup.qml` gained, under `SetupPanes.Hardware`, a
  `Widgets.IconButton` ("Run Connection Diagnostics", disabled and relabelled while running, icon
  from the exempt `buttons/` folder like the sibling "Open Project Editor" button) and beneath it a
  banner gated on `hasFailure`: `failureTitle` in the `alarm_critical` theme color plus
  `failureRemedy` in a read-only, `selectByMouse` `TextEdit` at `customMonoFont(0.9, false)`, and a
  "View All Findings" button that opens the problem center. Theme tokens only. `code-verify --check`
  clean on both files (the pane's remaining advisories are pre-existing `qml-offscale-spacing` on
  untouched lines); `registry-verify.py` clean.

### T16 — Runnable static test

- **Files:** `tests/scripts/test_diagnostics_static.py`
- **Does:** No app, no Qt. Asserts: both commands appear in exactly one safety tier and it is
  `safe`; `app.connectionDiagnostics` exists in the manifest with an icon that resolves in
  the icon tree; both bindings files bind the id; the five checker ids registered in
  `ConnectionDiagnostics.cpp` are exactly the ids the API handler documents.
- **Verify:** `pytest tests/scripts/test_diagnostics_static.py -v` passes here.
- **Deps:** T13, T14
- [x] done -- 18 tests pass. The original 12 (safety tiers, GPL-block registration, no commercial
  guard, the `diagnostics` scope description, the bus slugs against `kBusCount`, checker ids derived
  from the slugs, every id documented by the handler, no blocking primitive in any diagnostics
  source, the `usermod` command outside every `trDiag()` literal, neither command destructive) plus
  the six deferred registry assertions: the manifest entry's kind/category/icon/contexts, the icon
  resolving in the icon tree, both bindings files carrying the map entry, the `QtObject` and the
  `app.runConnectionDiagnostics()` call, the `main.qml`/`Setup.qml` entry points, the remedy field
  being read-only + selectable + monospaced, and the `Cpp_Misc_ConnectionDiagnostics` context
  property plus its `setupExternalConnections()` wiring.
  `pytest tests/scripts/ -v` -> 268 passed.

### T17 — Integration tests

- **Files:** `tests/integration/test_connection_diagnostics.py`
- **Does:** Covers AC5 (three distinct reachability verdicts: unresolvable name, closed local
  port, black-holed address — with the timeout verdict asserted to arrive *within* the
  declared budget, not after), AC6 (a local listener records a connection carrying zero
  application bytes), AC8 (`diagnostics.run` returns immediately with instant results,
  `diagnostics.status` reports completion inside the worst case, `problems.list` then returns
  the diagnostics findings), AC9 (a bus-scoped run yields findings for that bus only; a
  second failure inside the rate-limit window starts no second probing run).
- **Verify:** listed in `plan.md` for the maintainer. **[maintainer]** — needs the app
  running with the API server enabled on `localhost:7777`.
- **Deps:** T12
- [x] done -- 10 tests in `tests/integration/test_connection_diagnostics.py`: AC5 as four tests
  (unresolvable `.invalid` name, refused loopback port, black-holed `10.255.255.1` settling inside
  the run's own `estimatedMs` budget, and the three remedies asserted distinct), AC6 through a
  local `CountingListener` fixture that records connections and bytes so the probe is proven to
  carry zero application bytes, AC8 as three tests (ack under 1 s with the full instant-result
  shape, `diagnostics.status` settling plus `problems.list` serving the findings, and an unknown
  bus slug rejected by name), AC9 as two tests (a failed network open leaves the other buses'
  finding counts unchanged; repeated failures re-report without accumulating). **[maintainer]** --
  verified here only by `py_compile` and `pytest --collect-only` (10 collected); black clean.
  Honest limit noted in the file: the rate limit's suppressed probing run is internal state, so the
  second test asserts the observable property (findings stay identical), not the suppression.

### T18 — Documentation

- **Files:** `doc/help/API-Reference.md`, `doc/claude/architecture/io.md`
- **Does:** A `### Diagnostics Commands (2)` section documenting both commands, their
  parameters, the ack-and-poll shape, and the pointer to `problems.list` for reading
  findings. `io.md` records the diagnostics runner, the instant-versus-probing split, and the
  `onDeviceOpenFinished` hook.
- **Verify:** `python scripts/documentation-verify.py` clean on `doc/help/API-Reference.md`;
  every documented parameter checked against the handler's schema.
- **Deps:** T12
- [x] done -- `doc/help/API-Reference.md` gained `### Diagnostics Commands (2)` after the Problems
  section: the instant-versus-probing table, both commands with parameters, return shapes and
  examples in the surrounding sections' format, and the pointer to `problems.list` filtered by
  `checkerId`. `doc/claude/architecture/io.md` records the runner, the two check classes, the
  cache/reader split forced by the synchronous checker contract, and the
  `onDeviceOpenFinished` failure/success hooks. `tests/README.md` lists both new test files.
  Every documented parameter and field checked against `DiagnosticsHandler.cpp`.
  `documentation-verify.py` -> 121 files scanned, 0 findings.

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` is met and checked off there — including the
      **[maintainer]** and **[hardware]** ones (AC1-AC4, AC7, AC10-AC12), which the developer
      confirms; they cannot be closed from here.
- [ ] AC1 specifically: on a Linux account outside the device group, a failed serial connect
      shows the port, the owning group, and the exact `usermod` command — not a timeout.
- [x] `python scripts/code-verify.py --check` clean on all changed C++/QML files — repo-wide
      3217 files, 0 errors; the new QML blocks add no advisory.
- [x] `python scripts/registry-verify.py` clean (command manifest, icon resolution and render
      size, bindings, commercial guards).
- [x] `python scripts/documentation-verify.py` clean on the changed Markdown (0 findings).
- [x] `pytest tests/scripts/test_diagnostics_static.py -v` passes (18 tests).
- [ ] `qt-cpp-review` run on the C++ diff; findings addressed or noted.
- [x] No `waitFor*`, `QEventLoop`, `processEvents`, or thread sleep introduced anywhere in
      the diff — the counterfactual check for this feature, since it lives next to the exact
      code spec 0034 exists to remove. Asserted by `test_diagnostics_never_block_the_event_loop`.
- [ ] `--benchmark-hotpath` shows no regression on all nine gated tiers (guard only; the
      feature does not touch the parse path). **[maintainer]**
- [x] The spec-0001 ctor-edge proof re-run and recorded for the new composition-root entry
      (recorded under T3: the constructor is a member-init list with no `::instance()` call).
- [ ] `python scripts/sanitize-commit.py` run; working tree clean of new lint debt.
- [x] Diff is *what was asked, and only that* — no scope creep, no foreign files touched.
      In particular, the `app.problems` binding gap (T0) stays with spec 0033 unless that
      spec has closed.
- [ ] `spec.md` status set to `done`.

## Build registration owed (the developer edits `app/CMakeLists.txt`)

`SOURCES`:

```
  src/Misc/ConnectionDiagnostics.cpp
  src/Misc/Diagnostics/DeviceAccess.cpp
  src/Misc/Diagnostics/SerialChecks.cpp
  src/Misc/Diagnostics/BluetoothChecks.cpp
  src/Misc/Diagnostics/NetworkChecks.cpp
  src/API/Handlers/DiagnosticsHandler.cpp
```

`HEADERS`:

```
  src/Misc/ConnectionDiagnostics.h
  src/Misc/Diagnostics/DiagnosticsShared.h
  src/Misc/Diagnostics/DeviceAccess.h
  src/Misc/Diagnostics/SerialChecks.h
  src/Misc/Diagnostics/BluetoothChecks.h
  src/Misc/Diagnostics/NetworkChecks.h
  src/API/Handlers/DiagnosticsHandler.h
```

Inside the `if(BUILD_COMMERCIAL)` block:

```
  src/Misc/Diagnostics/AudioChecks.cpp     # SOURCES
  src/Misc/Diagnostics/AudioChecks.h       # HEADERS
```

## Post-landing UX revision (2026-07-25, maintainer request)

- Setup-pane button and failure banner removed; the entry point is now a "Run Diagnostics"
  button in the Problem Center dialog header (plus the existing command palette entry, which
  opens the dialog and runs). Static tests updated to pin the new hosting.
- `reportMissingSelection` no longer consults the UI driver's `portList()` (which reads
  `["Select Port"]` until the pane refreshes, producing a false "port is gone" on a valid
  selection); it now matches the persisted name against `QSerialPortInfo::availablePorts()`
  (port name or system location) and accepts existing custom device paths.
