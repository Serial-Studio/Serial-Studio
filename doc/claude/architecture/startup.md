# Architecture — Startup, Composition Root & AppState

> Part of the architecture corpus ([index](../architecture.md)). Read this file in full
> before touching ModuleManager, AppState, operation modes, singleton construction, or the
> updater. The ctor-closure rules are also summarized in CLAUDE.md under
> "Startup & Composition Root — Non-Negotiable".

## Composition Root & Construction Order (ModuleManager)

`Misc::ModuleManager` is the composition root in all but name. `initializeQmlInterface` starts the
timers, wires everything through `setupCrossModuleConnections()` (an ordered run of
`setupExternalConnections()` calls followed by `restoreLastProject()`), installs the message
handler, registers the `Cpp_*` QML context properties, then loads `main.qml`. Three standing
invariants hold it together:

- **All `setupExternalConnections()` run before `restoreLastProject()`.** Every module's wiring
  runs before `restoreLastProject()` inside `setupCrossModuleConnections`; a module that reacts
  to project load must have its wiring in place first. `restoreLastProject` is no longer the
  literal last call — the widget-extension refresh and the `MirrorPublisher`/`MirrorSession`
  construction (spec 0040) follow it, deliberately after the pinned order and project restore.
- **Context properties come after wiring, before the QML load.** `registerCoreContextProperties` /
  `registerCommercialContextProperties` / `registerAppMetadataProperties` run after
  `setupCrossModuleConnections()` and before `registerImageProvidersAndLoadQml` (`m_engine.load`),
  so QML never binds a half-wired object.
- **`qInstallMessageHandler(MessageHandler)` runs only after `Console::Handler` and
  `NotificationCenter` exist.** `MessageHandler` reaches both on the first warning **from any
  thread**, and since spec 0039 neither `instance()` constructs anything: they forward to
  `SessionContext::current()`, whose accessors `qFatal` with the slot's name when the module has
  not been adopted yet. So a warning emitted before the composition root reached those two slots
  is not a late off-thread construction any more, it is an immediate named abort. Installing the
  handler after `setupCrossModuleConnections` is what keeps that impossible.

- **`SessionContext::current()` is first reached as the opening statement of
  `instantiateCoreModules()` (spec 0039 M2)** — the composition root takes `auto& ctx =
  SessionContext::current()` before the pinned order runs and adopts the nine owned modules
  inline as each constructs. A class that takes the context by injection can therefore never be
  constructed before the context exists. See "Session Context" below for the ownership contract.

**Known pre-order exemption (found 2026-08-28, pre-existing):** `ModuleManager` holds
`NativeWindow` by value, so its ctor runs before `instantiateCoreModules()` and reaches
`ThemeManager::instance()`, whose ctor pulls `WorkspaceManager` and `Translator` — those three
construct ahead of the pinned sequence on every platform. Benign today (order-independent
leaves), but it means a ctor init-list capture inside ThemeManager/NativeWindow cannot rely on
the pinned order; treat those files as pre-root code.

**Pinned instantiation order** (the topological order the modules must construct in, verbatim from
`instantiateCoreModules()`): `Translator`, [`MachineID`, `LemonSqueezy`, `OfflineLicense`,
`Trial`, commercial], `TimerEvents`, `CommonFonts`, `WorkspaceManager`,
`NotificationCenter`, `Misc::ProblemCenter`, `Misc::ConnectionDiagnostics`, `ThemeManager`,
`ExtensionManager`, `ControlScript`, **`ProjectModel` before `AppState`**, `FrameBuilder`,
`IO::PipelineHost`, `IO::ConnectionManager`,
`Console::Handler`, `API::Server`, `CSV::Player`, `MDF4::Player`, [`Sessions::Player`,
`Sessions::Export`, `Sessions::DatabaseManager`, `MQTT::Publisher`, commercial], `CSV::Export`,
`MDF4::Export`, `Console::Export`, `FrameParser`, `UI::WidgetExtensions`, and `UI::Dashboard`
**last** (its ctor wires multiple core modules, the file/session players, and `TimerEvents`).

Two entries in that list carry their own reason to sit where they do:

- **`ProblemCenter` (spec 0033) is registered right after `NotificationCenter`** — it forwards
  aggregate findings there, and its own ctor is inert (no polling, no counter reads, no singleton
  reach), so it is safe this early. `ConnectionDiagnostics` (spec 0035) follows it for the same
  reason and because the diagnostics runner reports through the problem center. Both get their
  wiring later, in the `setupExternalConnections()` block.
- **The commercial licensing block is the FIRST thing built after `Translator`** (spec 0042), and
  `OfflineLicense` and `Trial` are therefore also ahead of `restoreLastProject()`: their ctors install
  the `CommercialToken`, and anything that bakes `SerialStudio::activated()` into derived state at load
  time (auto workspaces, driver lists, dashboard layout) reads a fallback value if the token
  arrives late. Late or async activation still needs a `LemonSqueezy::activatedChanged` hook — the
  pinning fixes the startup path only (2026-07-09: Plot3D degraded to MultiPlot on
  offline-activated machines). `activatedChanged` fires only on real token-validity transitions
  (`LemonSqueezy::notifyEntitlementMaybeChanged()`, 2026-08-04) — redundant emissions used to loop
  live-device rebuilds. Consumer inventory:
  [../specs/0042-license-token-hardening/consumers.md](../specs/0042-license-token-hardening/consumers.md).
  Three shapes of that block are worth naming because a QML binding reads them at paint rate:
  **`Licensing::MonotonicClock::now()` persists its anti-rewind floor at most once a minute**
  (`kPersistIntervalMs`, 60000; in between, the cached floor still catches a rewind, so the
  guarantee is unchanged while a property read costs no `QSettings` access at all);
  `Trial::daysRemaining()` is cached against the current date and invalidated on every expiry
  move; and `LemonSqueezy::requestFinished(ok, reason)` is emitted exactly once per
  activate/deactivate on every path, including the pre-flight refusals, so `--activate` /
  `--deactivate` can wait on a verdict instead of a timeout. A **refused** deactivation no longer
  clears the local cache: only `deactivated == true` does.

**The `ProjectModel`-before-`AppState` rule kills a live hazard.** `AppState`'s ctor calls
`deriveFrameConfig()`, whose ProjectFile branch calls `ProjectModel::instance()` (AppState.cpp), so
on a machine whose saved `operation_mode` is ProjectFile, ProjectModel is constructed *inside*
AppState's ctor; on a QuickPlot machine it is constructed later. `ProjectModel`'s ctor then calls
`newJsonFile()`, which emits `groupsChanged` while AppState is still mid-init (the fenced comment at
ProjectModel.cpp:162 exists for exactly this reason). Constructing ProjectModel first makes the
settings-conditional edge impossible.

**The list above is machine-checked.** `scripts/doc-anchors.json` carries an `ordered` anchor,
`composition-root-order`, that extracts every construction site out of `instantiateCoreModules()`
and requires this paragraph to name them in the same sequence; `claim-verify.py` fails on a
reorder. The seven singletons whose last namespace segment is `Player` or `Export` are excluded
from the ordered capture because the doc-side match keys on that last segment alone and cannot
tell `CSV::Export` from `Console::Export`; the two companion anchors
(`composition-root-players`, `composition-root-exports`) pin their presence instead. Keep the
whole list inside one paragraph: the anchor's doc scope ends at the first blank line.

`ModuleManager::instantiateCoreModules()` (called first inside `setupCrossModuleConnections`)
enforces this order directly in code: it force-constructs every core singleton in the pinned
sequence above (ProjectModel before AppState; the four-entry commercial licensing block and the
session/MQTT block under `BUILD_COMMERCIAL`; Dashboard last), replacing the old
settings-dependent lazy first-use order. Spec `doc/claude/specs/0001-composition-root/` keeps the
ctor-edge proof; spec `0039-session-context/ctor-proof.md` re-runs it for the `SessionContext`
publication line.

**The pinned order creates a protected surface: everything reachable from ProjectModel's ctor
(`newJsonFile()`, `watchProjectFile()`, `scheduleAutoSave()`, `ControlScript::setCode`) runs
BEFORE AppState and Dashboard exist.** Calling `AppState::instance()` or `UI::Dashboard::instance()`
from that closure recurses the Meyers guard on ProjectFile machines and aborts at startup
(`__cxa_guard_acquire detected recursive initialization` — this shipped and crashed once, 2026-07-07).
`newJsonFile()`'s Dashboard sync is gated on `m_initialized` (set at the end of the ctor);
`scheduleAutoSave()` is safe only because the empty-`m_filePath` early-return precedes its AppState
read. Any new code in this closure must keep those guards or add its own `m_initialized` gate.

**MMCSS coexistence contract (Windows).** Registering the main thread with MMCSS
(`AvSetMmThreadCharacteristics`) **before the Qt message handler is installed** — or treating
the `QThread::start` priority warning it triggers as a real failure — is a mistake. Qt's default
`QThread::InheritPriority` reads the creator's raw priority (MMCSS-managed ~25, not a
`THREAD_PRIORITY_*` constant) and feeds it back to `SetThreadPriority`, which rejects it — the
thread still starts and lands at NORMAL, **its exact pre-MMCSS inherited value**, so the failure
is benign; explicit priorities (named constants, e.g. `Audio.cpp` `setPriority(HighestPriority)`)
are unaffected. The contract: register only via
`Platform::AppPlatform::registerIngestThreadWithMmcss()`, called AFTER `qInstallMessageHandler`
(ModuleManager) so the targeted filter eats the warning, and never start a QThread expecting it
to inherit the boosted band.

**The band is per thread, so each acquisition thread registers itself (spec 0075 N2).** A
`QThread` never inherits the characteristic, and a process-wide "already registered" guard would
let the first caller silence every later one, so `AppPlatform`'s latch is `thread_local` and
`AppPlatform::mmcssRegisteredOnCurrentThread()` reports it. The composition root no longer calls
the registration itself: `ModuleManager` calls `IO::PipelineHost::registerIngestThread()`, still
immediately after `qInstallMessageHandler`, which posts the registration onto the pipeline thread
with a plain `Qt::QueuedConnection` (never a blocking GUI-to-pipeline wait, never a per-frame
hop), and each `IO::StreamWorker` posts the same call onto its own event loop next to
`compileEngines`. `PipelineHost` itself lives on the GUI thread, so the post goes through
`m_frameBuilder` and is skipped entirely while `m_frameBuilder->thread()` is not the pipeline
thread: the headless and benchmark bootstraps never call `relocateProcessingObjects()`, so they
register nothing and the benchmark keeps its own direct call on the thread it drives.

**Two roles, two profiles, one latch.** `registerIngestThreadWithMmcss()` claims "Pro Audio" for a
thread whose missed deadline is a dropped measurement (the pipeline, the stream workers);
`registerRenderThreadWithMmcss()` claims "Games" for the GUI thread, so another process cannot cost
the user frames, and `ModuleManager` calls it on the GUI thread beside the pipeline post. The
renderer gets "Games" rather than "Pro Audio" on purpose: a 60 Hz repaint in the audio band starves
every other process on a small machine, including our own acquisition. Both go through the one
`thread_local` latch, so a thread cannot change profile underneath itself. On macOS the equivalent
already exists and is unrelated to MMCSS: Performance Mode pins the main thread to
`QOS_CLASS_USER_INTERACTIVE`. `tst_mmcss_registration` pins the per-thread latch and the two-role
contract (it runs on every platform: the latch is recorded everywhere, only the Windows API behind
it is skipped).

## Session Context (spec 0039)

`SessionContext` (`app/src/SessionContext.h`) is the session/application ownership split: a
plain non-QObject, non-copyable class holding nine `unique_ptr` slots — AppState,
`UI::Dashboard`, `Console::Handler`, `FrameParser`, `FrameBuilder`, `ProjectModel`,
`IO::PipelineHost`, `IO::ConnectionManager`, `NotificationCenter`. Those classes have private
ctors with `friend class ::SessionContext`; the composition root constructs them via
`SessionContext::create<T>()` and `adopt*()` inside `instantiateCoreModules()`. The contract:

- **Ctor and dtor stay empty.** Constructing a module inside the `SessionContext` ctor
  re-enters the `current()` Meyers guard from that module's own ctor and aborts
  (`__cxa_guard_acquire` recursive init). Construction lives in `instantiateCoreModules()` only.
- **INV-4: adopted addresses never change.** ~60 QML context properties hold raw pointers to
  the owned modules; a slot, once filled, keeps its address for the session. The tree-wide
  `static auto& x = X::instance();` cache and every `m_x(X::instance())` ctor capture rest on
  exactly this, and on there being one session per process: a second session would leave both
  pointing at the dead one, so neither idiom may outlive INV-4 (spec 0075, K7).
- **INV-5: the only exit from a filled slot is `shutdown()`.** `adopt*()` asserts the slot is
  empty and the pointer non-null; there is no re-adopt.
- **INV-6: `shutdown()` runs while `qApp` is alive, after the QML engine dies** (`main.cpp`,
  plus the benchmark harness). Never move it into a destructor or atexit path — that
  reinstates the `__cxa_finalize` teardown-crash class. Release order in `shutdown()` is the
  exact reverse of the pinned instantiation order; update the two in lockstep.
- **Never call `SessionContext::current()` from a method body.** Sanctioned sites: the
  composition root, and a class's own `instance()` accessor passing the context into its ctor.
  The nine legacy `instance()` accessors are thin forwarders to `current()`, so existing call
  sites keep working. The `arch-session-context-bypass` advisory and the singleton-census gate
  (`scripts/singleton-census.json` baseline; `code-verify.py --singleton-census --check` fails
  on any increase) hold the line — don't add new `instance()` reach or a casual fourth pilot.
- **Injection pilots** (ctor takes `SessionContext&`): `Misc::BackupManager`,
  `DataModel::ProtoImporter`, `DataModel::DBCImporter`, plus `API::MirrorPublisher` /
  `API::MirrorSession` from spec 0040.

**The frame pipeline moves threads as the last wiring step (spec 0051 M3).**
`setupCrossModuleConnections()` ends with `IO::PipelineHost::relocateProcessingObjects()`,
which `moveToThread`s `FrameBuilder` and `FrameParser` onto the processing thread. Everything
before it — `restoreLastProject()`, the initial `readCode()`, every `setupExternalConnections`
— therefore runs same-thread, and only steady-state traffic crosses the boundary. The headless
and benchmark bootstraps call `instantiateCoreModules()` without
`setupCrossModuleConnections()`, so they stay single-threaded by construction (which is why
the spec-0044 verifier and `--benchmark-hotpath` measure the same pipeline they always did).
The pipeline thread is joined in `stopFrameConsumerWorkers()` **before**
`SessionContext::shutdown()` frees the modules, with `prepareShutdown()` queued ahead of the
quit so Lua states and QJSEngines die on the thread that owns them.

M3 (a real second session) is not started; everything above is single-context with session
id 0. Ctor-edge proofs: `0039-session-context/ctor-proof.md` (M1) and `ctor-proof-m2.md`
(M2 ownership).

## AppState — Single Source of Truth

`AppState` (`Cpp_AppState`) owns `operationMode`, `projectFilePath`, `frameConfig`.

- `operationMode` persists to QSettings; everything else reacts to `operationModeChanged()`.
- `frameConfig` is derived from mode + project source[0]; emits `frameConfigChanged(config)`.
- Init order: all `setupExternalConnections()` first, then `restoreLastProject()`.
- `setOperationMode()` guard-returns if unchanged.

## Operation Modes

| Mode | Delimiters | CSV delim | JS parser | Dashboard |
|------|-----------|-----------|-----------|-----------|
| ProjectFile (0) | Per-source | Via JS | Yes | Yes |
| ConsoleOnly (1) | None (short-circuits) | N/A | No | No |
| QuickPlot (2) | Line-based (CR/LF/CRLF) | Comma | No | Yes |

ConsoleOnly (replaced DeviceSendsJSON, 2026-04) bypasses CircularBuffer + queue;
`FrameBuilder::hotpathRxFrame` is a no-op; raw bytes reach the terminal via
`DeviceManager::rawDataReceived`.

## Packaging-Aware Updater

`ModuleManager::configureUpdater()` resolves the QSimpleUpdater
appcast key (repo-root `updates.json`) in three tiers: the CI-stamped `ss-config.json`
(`packageType` + `arch`) read from `applicationDirPath()` (macOS also `../Resources`), then
runtime probing (`APPIMAGE` env var on Linux; `GetCurrentPackageFullName` on Windows so a
Store install is never offered the MSI), then the legacy per-OS keys. `windows-msix` is
open-url-only (Store owns updates). Three things must stay in sync: the ci.yml stamp steps
(one per package; deb/rpm are two separate ldnp runs, macOS stamps before codesign, MSI via
`-DSS_PACKAGE_TYPE` in `app/CMakeLists.txt`), the key table in `ModuleManager.cpp`, and the
`updates.json` keys (shape pinned by `tests/unit/test_updates_manifest.py`). Dev builds have
no stamp and keep today's behavior.
