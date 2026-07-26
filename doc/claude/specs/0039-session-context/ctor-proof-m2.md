---
spec: 0039-session-context
phase: verification (milestone M2)
status: code-complete (maintainer launch gates open)
updated: 2026-07-25
---

# Ctor-edge proof re-derivation — spec 0039 M2

M1's verdict was *"preserved, not re-derived"* ([`ctor-proof.md:177-180`](./ctor-proof.md)), because
spec 0039 added one object with zero constructor out-edges and converted no pinned module. M2
breaks that last clause on purpose: each wave moves one session subsystem from a Meyers accessor to
a context-owned slot, so the proof must be **re-derived, once per wave**.

This file carries one section per unit of work. The scaffolding sections (M2-T1..M2-T6) record the
state of the subject *before* the first wave; the wave sections (M2-T7..M2-T14) each record the
eleven checks below. The consolidated verdict, the full eight-wave check table, and the maintainer
launch-gate checklist are in the final section (M2-T17).

## The eleven checks

| # | Check | How |
|---|-------|-----|
| C1 | Grep symmetry against the pinned order | every adopted class still named again below the function |
| C2 | INV-1 | `restoreLastProject()` still the last statement of `setupCrossModuleConnections()` |
| C3 | INV-2 | context properties after wiring, before `m_engine.load` |
| C4 | INV-3 | message handler installed after `Console::Handler` and `NotificationCenter` exist |
| C5 | Out-edge check on the context | ctor and dtor still empty; no member other than the id and the slots |
| C6 | Adoption order == pinned order, line for line | positions match one to one |
| C7 | No `adopt*()` outside the root | linter rule `arch-session-adopt-site` |
| C8 | No module ctor reaches an unadopted slot | startup abort is the runtime half; spec-0001 out-edge table is the read half |
| C9 | ProjectModel's ctor closure still names nothing new | the standing re-proof grep |
| C10 | Reverse-order teardown | `shutdown()` releases in exactly the reverse of the adoption sequence |
| C11 | INV-6: engine before objects | `~ModuleManager` precedes the `shutdown()` call site |

## Subject of the proof, as observed on 2026-07-25

`Misc::ModuleManager::instantiateCoreModules()` force-constructs 32 classes (27 in a GPL build;
five are `BUILD_COMMERCIAL`-only). Positions are the numbers every wave section refers to:

```
 1 Misc::Translator              12 AppState                      23 Sessions::Player *
 2 Misc::TimerEvents             13 Licensing::MachineID *        24 Sessions::Export *
 3 Misc::CommonFonts             14 Licensing::LemonSqueezy *     25 Sessions::DatabaseManager *
 4 Misc::WorkspaceManager        15 Licensing::OfflineLicense *   26 MQTT::Publisher *
 5 DataModel::NotificationCenter 16 Licensing::Trial *            27 CSV::Export
 6 Misc::ProblemCenter           17 DataModel::FrameBuilder       28 MDF4::Export
 7 Misc::ConnectionDiagnostics   18 IO::ConnectionManager         29 Console::Export
 8 Misc::ThemeManager            19 Console::Handler              30 DataModel::FrameParser
 9 Misc::ExtensionManager        20 API::Server                   31 UI::WidgetExtensions
10 DataModel::ControlScript      21 CSV::Player                   32 UI::Dashboard
```

The eight session subsystems sit at positions 5, 11, 12, 17, 18, 19, 30, 32. `ProjectModel` (11)
precedes `AppState` (12) and `UI::Dashboard` is last: both spec-0001 anchors hold.

**Drift note.** The M1 proof recorded 31 entries. Entry 31 (`UI::WidgetExtensions`) arrived from a
parallel spec while this scaffolding landed and is not part of spec 0039. Every wave rebases on the
current function and re-runs C6 rather than on this snapshot — the plan's cross-spec collision risk
is real and already firing.

## M2-T1 — Ownership scaffolding (recorded)

`app/src/SessionContext.{h,cpp}` gained eight `std::unique_ptr<T>` slots, eight `adopt*()` methods,
`shutdown()`, and `sealed()`.

- **C5 holds.** `SessionContext::SessionContext(int)` is still `: m_sessionId(session_id) {}` and
  `~SessionContext()` is still `{}`. Members are the session id plus the eight slots and nothing
  else. The slots are default-initialized by `unique_ptr`, not by a constructor statement, so no
  statement exists in either body for the new linter rule to find.
- **Accessors are unchanged forwards.** Each still returns `X::instance()`. The plan's M2-T1 says so
  explicitly, and the alternative — a `m_x ? *m_x : X::instance()` fallback — is refused: after a
  wave flips `X::instance()` into `return SessionContext::current().x();`, that fallback is
  unbounded recursion, not a graceful degradation. Each wave converts its own accessor to the
  fail-fast slot read in the same diff that flips its `instance()`.
- **Nothing is adopted.** `grep -rn "adopt" app/src/Misc/ModuleManager.cpp` is empty, so behavior is
  bit-identical to M1 and no `--benchmark-hotpath` run is implied by this task.

**Open decision the first wave must settle.** The plan's sketch spells the accessor guard
`SS_ASSERT_FATAL(m_projectModel != nullptr)`. No such macro exists — `app/src/SSAssert.h` ships
`SS_ASSERT(cond, action)` and `SS_ASSERT_LOG(cond)`, both of which continue in a release build.
A fail-fast accessor needs a form that aborts in release too (`SS_ASSERT(m_x, qFatal(...))`, or a
new `SS_ASSERT_FATAL` in `SSAssert.h`). Wave A picks one; every later wave copies it.

## M2-T2 — Deterministic teardown skeleton (recorded)

`shutdown()` releases the slots in the exact reverse of the pinned positions above:

```
32 Dashboard -> 30 FrameParser -> 19 Console::Handler -> 18 ConnectionManager
-> 17 FrameBuilder -> 12 AppState -> 11 ProjectModel -> 5 NotificationCenter
```

- **C10 holds by read-back.** The `reset()` sequence in `SessionContext::shutdown()` is the reverse
  of the adoption sequence the waves will write, position for position.
- **C11 holds.** `app/src/main.cpp` now reads, in order: `app.exec()`; the `ModuleManager` scope
  closes (the QML engine dies); `IO::ConnectionManager::instance().shutdownDrivers()`;
  `qInstallMessageHandler(nullptr)`; `SessionContext::current().shutdown()`;
  `Platform::AppPlatform::releaseAdjustedArgv(...)`; `return status`. The engine dies before the
  objects (INV-6), the `:184` `ConnectionManager` use precedes the release that would make it a
  null-slot fatal, and the message handler is uninstalled before the console and notification slots
  can be released — the post-routine at `ModuleManager.cpp:607` runs later, inside
  `~QCoreApplication`, and is now belt-and-braces.
- **Inert today.** Every slot is null, so `shutdown()` is eight no-op `reset()` calls.

## M2-T3 — Pre-root reach audit and relocation (recorded)

Fail-fast accessors turn any reach *before* the pinned sequence into a fatal. Full list as of this
tree, over every entry path:

| Entry path | Site | Session classes reached | Status |
|------------|------|-------------------------|--------|
| Commercial runtime mode | `main.cpp:155` (was) | `AppState` | **Relocated** (below) |
| `--benchmark-hotpath` / `--benchmark-frames` / `--benchmark-seconds` / `--min-fps` | `CLI::runHotpathBenchmark` -> `Benchmark::HotpathBenchmark` (18 `::instance()` sites) | `ProjectModel`, `AppState`, `FrameParser`, `FrameBuilder`, `Dashboard` (+ `CSV/MDF4/Sessions::Export`, `API::Server`, `GRPCServer`) | **Fixed** by running the pinned sequence first |
| `--dump-api-schema` | `CLI::dumpApiSchema` | none — `API::CommandHandler`, `API::CommandRegistry` only | Safe; recorded for the later, non-session waves |
| `--selftest` / `--selftest-suite` (`SS_INAPP_TESTS`) | `CLI::runSelfTests` | none | Safe |
| `--activate` / `--deactivate` / `--selftest-offline-license` / `--validate-guards` | `CLI` commercial early exits | none — licensing only | Safe |
| `--theme` | `CLI::applyThemeOverride` (`main.cpp:153`) | none — `Misc::ThemeManager` is an application global | Safe, stays pre-root |
| every path | `Misc::CrashTracker::instance()` (`main.cpp:109,119,123,157,166`) | none | Safe, stays pre-root |
| in-app benchmark dialog | `Benchmark::BenchmarkRunner` (22 sites) | several | Not pre-root: it runs inside a live composition root |

**The `main.cpp:155` relocation.** The naive move — call `AppState::instance().setEphemeralSession(true)`
*after* `bootstrapModuleManager` — is wrong and was rejected: `bootstrapModuleManager` runs
`initializeQmlInterface()` -> `setupCrossModuleConnections()` -> `restoreLastProject()`, and
`restoreLastProject()` opens the saved project, which drives `AppState::onProjectLoaded()` and
persists `project_file_path` unless the ephemeral flag is already set. Setting the flag afterwards
would let an operator run overwrite the main application's last project — the exact thing
`setEphemeralSession` exists to prevent. The plan's second option is the one taken: the flag is
routed through composition-root state.

- `main.cpp` passes `moduleManager.setEphemeralSession(cli.runtimeMode())` alongside
  `setHeadless()`, and no longer includes `AppState.h`.
- `Misc::ModuleManager` stores it and applies it in `setupCrossModuleConnections()` at the moment it
  first takes the `AppState` pointer — after `instantiateCoreModules()`, before
  `appState->setupExternalConnections()`, and therefore before `restoreLastProject()`. The observable
  ordering of "flag set, then project restored" is preserved exactly.
- `cli.runtimeMode()` already returns `false` in a GPL build, so the relocated call needs no
  `#ifdef` and the commercial-only `applyThemeOverride()` keeps its guard.

**The benchmark relocation.** `Misc::ModuleManager::instantiateCoreModules()` became a `public
static` member (its body uses no member state and is otherwise unchanged), and
`CLI::runHotpathBenchmark()` calls it immediately before `HotpathBenchmark::runAndReport(...)`.
`HotpathBenchmark::runAndReport` calls `SessionContext::current().shutdown()` after `printReport`
and before the PGO profile write and `std::_Exit`.

This is the task that gates waves C and D, and it is also the one behavior change in the scaffolding
pass that a reader should not wave through: the benchmark process now constructs the whole pinned
set (32 classes) instead of lazily constructing the ~12 it touches. That is deliberate — after a
wave lands, a lazily-constructed session subsystem is a null-slot fatal — but it is unmeasured here.
**Maintainer gate: run `--benchmark-hotpath` and confirm all nine gated tiers are unchanged before
Wave A.** If a module constructed on that path misbehaves headless, the fix is to make it inert
under `HotpathBenchmark::active()`, not to skip the pinned order.

## M2-T4 — Second-instance bleed census

Recorded separately in [`m2-bleed-census.md`](./m2-bleed-census.md).

## M2-T5 — Address-lifetime audit

Recorded in the results section of [`m2-plan.md`](./m2-plan.md).

## M2-T6 — Linter rules (recorded)

Two blocking rules land in `scripts/code_verify_rules.py`, documented in `scripts/code-verify.py`'s
report prose next to `arch-session-context-bypass`:

- **`arch-context-ctor-nonempty`** — any statement in the body of `SessionContext::SessionContext`
  or `SessionContext::~SessionContext` (matched in `app/src/SessionContext.{h,cpp}`). This is C5,
  made executable: it is the guard on the Meyers-guard recursion that the plan calls the one failure
  mode that aborts at startup.
- **`arch-session-adopt-site`** — a `.adoptX(` / `->adoptX(` call in any first-party C++ file other
  than `app/src/Misc/ModuleManager.cpp`. This is C7.

Both are errors, not advisories: unlike the migration ratchets, neither has existing debt to
amortize and both failures are startup-time.

**Verified.** Scratchpad snippets with a statement in each of the two special members, and with
`ctx.adoptProjectModel(...)` / `SessionContext::current().adoptDashboard(...)` outside the root,
each trip exactly their own rule; the same adoption inside a file whose path tail is
`/app/src/Misc/ModuleManager.cpp` reports nothing. Repo-wide `code-verify --check` reports **0
errors** before and after, so the blocking-error count is unchanged.

## The ownership spelling, settled by Wave A

Every wave copies these three spellings verbatim; they are recorded once here rather than in each
wave section.

**Fail-fast accessor** (the open decision M2-T1 left to Wave A). No new macro: `SS_ASSERT` already
aborts in debug via `qt_assert` and runs its action in release, so a `qFatal` action is fatal in
both configurations and names the slot.

```cpp
DataModel::NotificationCenter& SessionContext::notifications() const
{
  SS_ASSERT(m_notifications != nullptr,
            qFatal("SessionContext: %s accessed before adoption", "notifications"));
  return *m_notifications;
}
```

**Constructor access.** The plan's tradeoff table chose `friend` over a public constructor, but the
plan's sketch spells the root line `std::make_unique<X>()`, and `std::make_unique` is not a friend of
anything — a private constructor cannot be reached through it. The shape that keeps both halves is a
private `SessionContext::create<T>()` (one `new T()` inside the context, so friendship is what
enables it) plus `friend class Misc::ModuleManager;` on the context, so the composition root is the
only caller of either:

```cpp
// SessionContext.h, private
friend class Misc::ModuleManager;
template<typename T> [[nodiscard]] static std::unique_ptr<T> create() { ... }

// the adopted class, private
friend class ::SessionContext;
explicit NotificationCenter();

// ModuleManager.cpp, the pinned line, substituted in place
ctx.adoptNotifications(SessionContext::create<DataModel::NotificationCenter>());
```

**Destructor access.** `std::default_delete<T>` destroys the slot from its own context, not the
context's, so an adopted class needs a public destructor. Seven of the eight already have one
implicitly; `NotificationCenter` declared its destructor private and had it moved to the public
block, which makes it consistent with its siblings rather than looser than them. The constructor —
the half that actually prevents a second instance — stays private in all eight.

**One shared statement in the root.** `auto& ctx = SessionContext::current();` is the first statement
of `instantiateCoreModules()`, added once by Wave A. It is the publication point the plan requires:
`current()`'s Meyers guard is released before the first module is constructed, so no module ctor can
re-enter it.

## Wave A — `DataModel::NotificationCenter` (M2-T7)

Pinned position 5. Four edits:

| # | File | Edit |
|---|------|------|
| 1 | `app/src/Misc/ModuleManager.cpp:636,642` | `auto& ctx = SessionContext::current();` as the first statement; `:642` `(void)DataModel::NotificationCenter::instance();` -> `ctx.adoptNotifications(SessionContext::create<DataModel::NotificationCenter>());`, in place |
| 2 | `app/src/DataModel/NotificationCenter.cpp:84` | `instance()` body is now `return SessionContext::current().notifications();`; the Meyers `static NotificationCenter self;` is gone |
| 3 | `app/src/DataModel/NotificationCenter.h` | `class SessionContext;` forward declaration, `friend class ::SessionContext;` in the private block, destructor moved to the public block |
| 4 | `app/src/SessionContext.cpp:252` | `notifications()` converted from a forward to the fail-fast slot read |

**Pre-adoption reach: none.** `grep -rn "NotificationCenter::instance()" app/src` returns 30 sites in
11 files. None is on an entry path that runs before `instantiateCoreModules()`: `main.cpp` and
`Misc/CLI.cpp` contain zero (`CLI.cpp:337` calls `instantiateCoreModules()` before the benchmark
runs, per M2-T3); the four modules pinned *ahead* of position 5 (`Translator`, `TimerEvents`,
`CommonFonts`, `WorkspaceManager`) contain zero; the process message handler that posts into
`instance()` (`ModuleManager.cpp:213,223`) is installed at `ModuleManager.cpp:606`, two statements
after `setupCrossModuleConnections()`. The five `static auto& nc = instance();` caches inside the
class's own Lua bindings (`NotificationCenter.cpp:504,519,534,549,564`) resolve at script-call time.

| # | Check | Verdict |
|---|-------|---------|
| C1 | Grep symmetry | `DataModel::NotificationCenter` is named again below the function at `ModuleManager.cpp:688` (wiring capture), `:769` (`dataReset -> clearAll`), `:804` (`Cpp_Notifications`). Holds. |
| C2 | INV-1 | Every `setupExternalConnections()` call precedes `appState->restoreLastProject()` (`ModuleManager.cpp:760`). **Drift, not caused by this wave:** three statements now *follow* it (`refreshRepositories()` and two `uiDashboard` connects, present at HEAD, from a parallel spec), so `restoreLastProject()` is no longer the literal last statement of `setupCrossModuleConnections()`. INV-1's substance (wiring before restore) holds; its literal phrasing no longer matches the function and belongs to whoever added the trailing statements. |
| C3 | INV-2 | `setupCrossModuleConnections()` `:604` -> `registerCoreContextProperties` `:612` -> `registerImageProvidersAndLoadQml()` `:619` -> `m_engine.load` `:929`. Unchanged by this wave. Holds. |
| C4 | INV-3 | `qInstallMessageHandler(MessageHandler)` at `:606` runs after `:604`, so both `Console::Handler` (position 19) and the now-adopted `NotificationCenter` (position 5) exist. Holds. |
| C5 | Context out-edges | `SessionContext::SessionContext(int)` is still `: m_sessionId(session_id) {}`, `~SessionContext()` still `{}`; `arch-context-ctor-nonempty` silent. The new `create<T>()` is a private static member *function* template, not a member, so the only data members remain the id and the eight slots. Holds. |
| C6 | Adoption order == pinned order | The `ModuleManager.cpp` diff against HEAD is two added lines (the publication statement) and exactly one line changed in place at position 5. Positions 1-4 and 6-32 are byte-identical. Holds. |
| C7 | No `adopt*()` outside the root | `arch-session-adopt-site` silent repo-wide; the only call is `ModuleManager.cpp:642`. Holds. |
| C8 | No ctor reaches an unadopted slot | `NotificationCenter::NotificationCenter()` read in full: `QObject(nullptr)`, four member initializers, `moveToThread(qApp->thread())`, two `QSettings` reads. Zero singleton out-edges — the reason the plan put it first. Holds. |
| C9 | ProjectModel ctor closure | `grep -rn "SessionContext" app/src/DataModel/ProjectModel.cpp app/src/DataModel/Project/ app/src/main.cpp app/src/Misc/CLI.cpp` -> only `main.cpp:37,186` (the include and the shutdown call). The closure names nothing new. Holds. |
| C10 | Reverse-order teardown | `shutdown()` releases `m_notifications` last (`SessionContext.cpp:184`), and position 5 is the earliest of the eight. Holds. |
| C11 | INV-6 | `main.cpp`: `app.exec()` -> `~ModuleManager` (engine dies) -> `ConnectionManager::shutdownDrivers()` `:183` -> `qInstallMessageHandler(nullptr)` `:185` -> `SessionContext::current().shutdown()` `:186`. Unchanged. Holds. |

`code-verify --check`: **0 errors**, both new arch rules silent, and no finding in any of the four
touched files. (The repo advisory total moves during this session because parallel agents are editing
`UI/Dashboard.cpp`; the delta attributable to this wave is 0.)

**Not verified here (maintainer):** three-mode launch and the `--benchmark-hotpath` M2 baseline. No
build is run by this pass.

## Wave B1 — `Console::Handler` (M2-T8)

Pinned position 19. Four edits:

| # | File | Edit |
|---|------|------|
| 1 | `app/src/Misc/ModuleManager.cpp:660` | `(void)Console::Handler::instance();` -> `ctx.adoptConsole(SessionContext::create<Console::Handler>());`, in place |
| 2 | `app/src/Console/Handler.cpp:144` | `instance()` body is now `return SessionContext::current().console();`; the Meyers `static Handler singleton;` is gone |
| 3 | `app/src/Console/Handler.h` | `class SessionContext;` forward declaration and `friend class ::SessionContext;` in the private block. The destructor is implicit and already public, so nothing else moves |
| 4 | `app/src/SessionContext.cpp:204` | `console()` converted from a forward to the fail-fast slot read |

**Pre-adoption reach: none.** `grep -rn "Console::Handler::instance()" app/src` returns 31 sites in
seven files. `main.cpp` and `Misc/CLI.cpp` contain zero. The four sites in `IO/ConnectionManager.cpp`
(`:613,643,688,1694`) are the only ones in a module pinned *earlier* than 19 — position 18, one line
above — and all four sit in runtime data-path members (`processPayload`, `processMultiSourcePayload`,
`writeDataToDevice`, `onRawDataReceived`), none in `~ConnectionManager`'s constructor closure, so
they resolve on the first byte received rather than during the pinned order. The rest are the API
console handler, the Terminal widget, `Console/Export.cpp:287`, and the root itself
(`ModuleManager.cpp:193,204,205` message handler, `:706` wiring, `:799` context property), all after
`setupCrossModuleConnections()`.

| # | Check | Verdict |
|---|-------|---------|
| C1 | Grep symmetry | `Console::Handler` is named again below the function at `ModuleManager.cpp:706` (`setupExternalConnections`) and `:799` (`Cpp_Console_Handler`). Holds. |
| C2 | INV-1 | Unchanged from Wave A, including the same recorded drift (three statements follow `restoreLastProject()` at `:760`). Substance holds. |
| C3 | INV-2 | Unchanged: `:604` wiring -> `:612` core properties -> `:619` QML load. Holds. |
| C4 | INV-3 | This is the wave that exercises it. `qInstallMessageHandler(MessageHandler)` at `ModuleManager.cpp:606` runs two statements after `setupCrossModuleConnections()` at `:604`, so the handler that reaches `Console::Handler::instance()` at `:193,204,205` cannot fire before position 19 is adopted. Nothing installs a message handler earlier: the only other `qInstallMessageHandler` calls in `app/src` are three uninstalls (`main.cpp:185`, `ModuleManager.cpp:372,607`). Holds. |
| C5 | Context out-edges | Unchanged from Wave A: ctor and dtor empty, members are the id plus the eight slots; `arch-context-ctor-nonempty` silent. Holds. |
| C6 | Adoption order == pinned order | One line changed in place at position 19; positions 1-18 and 20-32 byte-identical. Holds. |
| C7 | No `adopt*()` outside the root | `arch-session-adopt-site` silent; two calls, both in `instantiateCoreModules()`. Holds. |
| C8 | No ctor reaches an unadopted slot | `Console::Handler::Handler()` read in full (`Handler.cpp:68-139`): `clear()` (member state only), `Misc::CommonFonts::instance()` (position 3), `IO::availableChecksums()` (free function), `availableFonts()` -> `CommonFonts` again, `Misc::TimerEvents::instance()` (position 2) for the `uiTimeout` connection, `updateFont()` -> `CommonFonts` again. Both out-edges are application globals pinned *earlier* and *not adopted*, which is exactly the session/application split the plan wanted this wave to prove: an owned module may still depend on a Meyers-held app global. Holds. |
| C9 | ProjectModel ctor closure | Re-run: `SessionContext` still appears only in `main.cpp:37,186`. Holds. |
| C10 | Reverse-order teardown | `shutdown()` releases `m_console` third (`SessionContext.cpp:179`), after `m_dashboard` (32) and `m_frameParser` (30) and before `m_connectionManager` (18). Reverse of 19's position among the eight. Holds. |
| C11 | INV-6 | Unchanged: engine dies at the `ModuleManager` scope exit, `shutdownDrivers()` at `main.cpp:183`, `qInstallMessageHandler(nullptr)` at `:185`, `shutdown()` at `:186`. A `qWarning()` after the shutdown point cannot reach the released console slot because the handler is uninstalled one line earlier and the only statements between are `releaseAdjustedArgv` and `return`. Holds. |

**The mutable file statics stay, and that is correct.** `cachedTimestampStr()`'s `static qint64
s_lastMs` / `static QString s_cached` (`Handler.cpp:54,55`) and `checksumMethods()`'s `static
QStringList list` (`:318`) are internal-linkage function-local statics in the TU, not class state.
Ownership does not move them and M2 does not try to: they are recorded in
[`m2-bleed-census.md`](./m2-bleed-census.md) as M3 work (a second instance would share the timestamp
cache silently). They are unaffected by adoption because they outlive nothing they are read by — both
are reached only from member functions of the one live instance.

`code-verify --check`: **0 errors**, both new arch rules silent, no finding in the four touched files.

**Not verified here (maintainer):** three-mode launch.

## Wave B2 — `DataModel::FrameParser` (M2-T9)

Pinned position 30. Four edits:

| # | File | Edit |
|---|------|------|
| 1 | `app/src/Misc/ModuleManager.cpp:673` | `(void)DataModel::FrameParser::instance();` -> `ctx.adoptFrameParser(SessionContext::create<DataModel::FrameParser>());`, in place |
| 2 | `app/src/DataModel/Scripting/FrameParser.cpp:78` | `instance()` body is now `return SessionContext::current().frameParser();`; the Meyers `static FrameParser singleton;` is gone |
| 3 | `app/src/DataModel/Scripting/FrameParser.h` | `class SessionContext;` forward declaration and `friend class ::SessionContext;` in the private block; destructor implicit and already public |
| 4 | `app/src/SessionContext.cpp:220` | `frameParser()` converted from a forward to the fail-fast slot read |

**Pre-adoption reach: none.** `grep -rn "FrameParser::instance()" app/src` returns 17 sites in ten
files; `main.cpp` and `Misc/CLI.cpp` contain zero. Position 30 is late, so the reaches worth checking
are the ones from modules pinned *ahead* of it, and every one lands in a runtime member rather than a
constructor: `FrameBuilder.cpp:981,1408,1576,1766,1830` (position 17) sit in `onConnectedChanged`,
`trySpanLane`, `decodeProjectChannels`, `beginDatasetPass`, `refreshDatasetCaptureFlag` — the
`FrameBuilder` constructor is `:125` and reaches none of them; `Project/ProjectModelSources.cpp:268`
(position 11) sits in `ProjectModel::updateSourceFrameParser`, an editor path, not the ctor closure;
`Misc/Problems/ScriptCheckers.cpp:178` (`ProblemCenter` is position 6) sits in the free function
`checkParserErrors`, and `ProblemCenter`'s constructor is an empty body with four member
initializers. The two benchmark files run after `CLI.cpp:337` calls `instantiateCoreModules()`.

| # | Check | Verdict |
|---|-------|---------|
| C1 | Grep symmetry | `DataModel::FrameParser` is named again below the function at `ModuleManager.cpp:697` (`setupExternalConnections`). Holds. |
| C2 | INV-1 | Unchanged from Wave A, same recorded drift. Substance holds. |
| C3 | INV-2 | Unchanged. Holds. |
| C4 | INV-3 | Unchanged; this wave touches neither the handler nor its two sinks. Holds. |
| C5 | Context out-edges | Unchanged: ctor and dtor empty, id plus eight slots; `arch-context-ctor-nonempty` silent. Holds. |
| C6 | Adoption order == pinned order | One line changed in place at position 30; every other position byte-identical. Holds. |
| C7 | No `adopt*()` outside the root | `arch-session-adopt-site` silent; three calls, all in `instantiateCoreModules()`. Holds. |
| C8 | No ctor reaches an unadopted slot | **The wave's point.** `FrameParser::FrameParser()` read in full (`FrameParser.cpp:47-73`): `engineForSource(0)` -> `languageForSource(0)` -> `ProjectModel::instance()` (position 11, earlier). Source 0 exists by then — `ProjectModel`'s own ctor closure runs `newJsonFile()`, which seeds a default source through `seedDefaultFrameParser()` (`Project/ProjectModelShared.h:115`, a pure function) with `frameParserLanguage = SerialStudio::Native` — so the engine actually built at startup is a `CFrameParser`, whose constructor is `: m_parser(nullptr) {}` with zero out-edges. The scripted branches were read too, because a saved project can select them at load time: `JsScriptEngine`'s ctor reaches `FrameBuilder::instance()` (17) and `NotificationCenter::instance()` (5, **already adopted by Wave A** — the first live assertion) through `ScriptApiCall::installHelperBridgesJS`, and `LuaScriptEngine::createState()` reaches the same two (`LuaScriptEngine.cpp:167,184`). Neither reaches position 32: `DashboardApi::installJS`/`installLua` and `DeviceWriteApi::installJS`/`installLua` only push function pointers and bridges (`DashboardBridge::DashboardBridge` is `: QObject(parent) {}`), and every `UI::Dashboard::instance()` in those TUs sits inside a callback body. The remaining ctor edges are `TimerEvents` (2), `Translator` (1), and `qApp`. All out-edges point strictly earlier in the pinned order. Holds. |
| C9 | ProjectModel ctor closure | Re-run: `SessionContext` still only in `main.cpp:37,186`. Holds. |
| C10 | Reverse-order teardown | `shutdown()` releases `m_frameParser` second (`SessionContext.cpp:178`), immediately after `m_dashboard` (32). Position 30 is second-highest of the eight. Holds. |
| C11 | INV-6 | Unchanged. Holds. |

**The `aboutToQuit` connection still fires exactly once, and still before destruction.** It is made in
the constructor (`FrameParser.cpp:63-70`) with `this` as the context object, and the constructor now
runs exactly once because `adoptFrameParser()` asserts an empty slot (INV-5) and the composition root
is the only caller (C7). The order at quit is unchanged in the half that matters and strictly better
in the other half: `QCoreApplication` emits `aboutToQuit` while `app.exec()` is unwinding, so the
lambda clears `m_engines` and refreshes the caches *before* `main.cpp:186` reaches
`SessionContext::current().shutdown()`, which is where `~FrameParser` now runs (previously
`__cxa_finalize`, after `qApp` was gone). Qt removes the connection on destruction either way, so the
signal cannot reach a released slot, and no second registration exists to double the teardown.

`code-verify --check`: **0 errors**, both new arch rules silent, no finding in the four touched files.

**Not verified here (maintainer):** three-mode launch, a project with a Lua parser (that engine is
built from project load, not from the constructor, so it exercises the same two edges after every
module exists), and the recorded `--benchmark-hotpath` run.

## Wave C1 — `AppState` (M2-T10)

Pinned position 12, immediately after `ProjectModel` (11). Four edits:

| # | File | Edit |
|---|------|------|
| 1 | `app/src/Misc/ModuleManager.cpp:651` | `(void)AppState::instance();` -> `ctx.adoptAppState(SessionContext::create<AppState>());`, in place |
| 2 | `app/src/AppState.cpp:56` | `instance()` body is now `return SessionContext::current().appState();`; the Meyers `static AppState singleton;` is gone |
| 3 | `app/src/AppState.h` | `class SessionContext;` forward declaration and `friend class ::SessionContext;` in the private block; destructor implicit and already public |
| 4 | `app/src/SessionContext.cpp:196` | `appState()` converted from a forward to the fail-fast slot read |

**Pre-adoption reach: none, and M2-T3's relocation is what makes that true.** `main.cpp` contains zero
`AppState` references — `bootstrapModuleManager` (`main.cpp:75-98`) passes
`moduleManager.setEphemeralSession(cli.runtimeMode())`, a plain member store, and the flag reaches
`AppState` inside `setupCrossModuleConnections()` at `ModuleManager.cpp:692`, after the pinned order
and before `restoreLastProject()`. `Misc/CLI.cpp`'s two sites (`:474,481`) are in
`applyProjectAndAutoConnect`, called from `main.cpp:168` — after `bootstrapModuleManager` at `:161`.
The benchmark entry point calls `Misc::ModuleManager::instantiateCoreModules()` at `CLI.cpp:337`
before `HotpathBenchmark::runAndReport`, so `HotpathBenchmark.cpp`'s reach is post-root;
`BenchmarkRunner` only runs inside a live root. `verifyShortcutProject()` (`main.cpp:150`, pre-root)
reaches only `Misc::ShortcutGenerator`.

**Three guards inside the `ProjectModel` ctor closure carry this wave, and all three were read.**
`ProjectModel` (11) constructs before `AppState` (12), and its closure has three paths that *name*
`AppState::instance()`. None executes it:

1. `ProjectModel.cpp:1501` — the `AppState` reach in `newJsonFile()` is inside `if (m_initialized)`,
   and `m_initialized` is set true only at the end of the constructor (`:191`). The function-local
   `static auto& appState` is declared *inside* that block, so it is not even initialized during
   construction.
2. `ProjectModelPersistence.cpp:398` — `scheduleAutoSave()` is wired to nine ctor-time `connect`s and
   does fire while `newJsonFile()` emits, but it returns at `:395` on `m_filePath.isEmpty()`
   (`newJsonFile()` sets `m_filePath = ""` at `:1497`, before the emissions), and the `static auto&
   appState` at `:398` sits after that guard.
3. `ControlScript.cpp:258` — the closure calls `ControlScript::setCode("")`, which returns
   immediately at `:193` because `m_code` is already empty; even if it did not, `shouldRun()` returns
   at `:254` on `!m_ready`, and `m_ready` becomes true only in `setupExternalConnections()` (`:83`),
   long after the pinned order. The `AppState::instance()` and `ConnectionManager::instance()`
   statics at `:257-258` sit after that guard.

The lambda at `ProjectModel.cpp:172-186` that reaches `AppState` is connected *after* `newJsonFile()`
runs, which the existing `// code-verify off` comment at `:165-169` already explains. These guards
are load-bearing for startup from this wave onward: weakening any of them turns a silent
out-of-order construction into a named startup abort.

| # | Check | Verdict |
|---|-------|---------|
| C1 | Grep symmetry | `AppState` is named again below the function at `ModuleManager.cpp:686` (wiring capture), `:692` (`setEphemeralSession`), `:693` (`setupExternalConnections`), `:760` (`restoreLastProject`), and the `Cpp_AppState` context property. Holds. |
| C2 | INV-1 | Every `setupExternalConnections()` precedes `restoreLastProject()` at `:760`; `setEphemeralSession(m_ephemeralSession)` at `:692` precedes both, which is the property M2-T3 traded the `main.cpp:155` reach for. Same recorded drift (three trailing statements). Substance holds. |
| C3 | INV-2 | Unchanged. Holds. |
| C4 | INV-3 | Unchanged. Holds. |
| C5 | Context out-edges | Unchanged: ctor and dtor empty, id plus eight slots; `arch-context-ctor-nonempty` silent. Holds. |
| C6 | Adoption order == pinned order | One line changed in place at position 12; every other position byte-identical. Holds. |
| C7 | No `adopt*()` outside the root | `arch-session-adopt-site` silent; four calls, all in `instantiateCoreModules()`. Holds. |
| C8 | No ctor reaches an unadopted slot | `AppState::AppState()` read in full (`AppState.cpp:39-51`): the reference member `m_projectModel(DataModel::ProjectModel::instance())` (position 11, one line earlier — the spec-0001 anchor, now executable) and `deriveFrameConfig()`, whose ProjectFile branch reads `m_projectModel.sources()` / `.frameDetection()` (`AppState.cpp:246-251`) and whose QuickPlot and ConsoleOnly branches return before touching it. No other out-edge; `m_frameBuilder` is initialized to `nullptr` and filled in `setupExternalConnections()`. Holds. |
| C9 | ProjectModel ctor closure | Re-run: `SessionContext` still only in `main.cpp:37,186`. The closure was additionally read in full for the three guards above. Holds. |
| C10 | Reverse-order teardown | `shutdown()` releases `m_appState` sixth (`SessionContext.cpp:182`), after `m_frameBuilder` (17) and before `m_projectModel` (11) — so `AppState`'s `ProjectModel&` reference member is still valid when `~AppState` runs. Exact reverse of the pinned order. Holds. |
| C11 | INV-6 | Unchanged. Holds. |

**One residual pre-adoption exposure, recorded not fixed.** `Platform::FileOpenEventFilter` is
installed on `qApp` at `main.cpp:127`, before the composition root, and its `eventFilter`
(`FileOpenEventFilter.cpp:45-46`) reaches `AppState::instance()` and `ProjectModel::instance()`
directly, with no deferral. A `QFileOpenEvent` can only be delivered while an event loop spins, and
the normal path runs none between `:127` and `bootstrapModuleManager` at `:161` — so this is not
reachable today. It becomes reachable if anything pre-root ever spins an event loop (a modal dialog,
a nested `processEvents`), and the failure would then be a named startup abort rather than today's
silent out-of-order construction. It applies identically to Wave C2 (`ProjectModel`), which should
decide whether to defer the handler with a queued invocation. Not changed here: it is outside this
wave's four edits.

`code-verify --check`: **0 errors**, both new arch rules silent, no finding in the four touched files.

**Not verified here (maintainer):** launch on a machine whose saved `operation_mode` is `ProjectFile`
(the only branch of `deriveFrameConfig()` that touches `ProjectModel` during construction), plus
QuickPlot and ConsoleOnly, and `--benchmark-hotpath`, which is a blocking gate from this wave onward.

## Wave C2 — `DataModel::ProjectModel` (M2-T11)

Pinned position 11, the first of the eight and the one whose constructor closure is a protected
surface. Four edits:

| # | File | Edit |
|---|------|------|
| 1 | `app/src/Misc/ModuleManager.cpp:650` | `(void)DataModel::ProjectModel::instance();` -> `ctx.adoptProjectModel(SessionContext::create<DataModel::ProjectModel>());`, in place |
| 2 | `app/src/DataModel/ProjectModel.cpp:201` | `instance()` body is now `return SessionContext::current().projectModel();`; the Meyers `static ProjectModel singleton;` is gone. `SessionContext.h` added to the include block |
| 3 | `app/src/DataModel/ProjectModel.h` | `class SessionContext;` forward declaration next to the two existing global forwards, `friend class ::SessionContext;` in the private block; destructor implicit and already public |
| 4 | `app/src/SessionContext.cpp:242` | `projectModel()` converted from a forward to the fail-fast slot read |

Plus the residual exposure Wave C1 recorded and deferred to this wave (`FileOpenEventFilter`, below).

**Pre-adoption reach: none.** `grep -rn "ProjectModel::instance()" app/src` returns 215 sites in 56
files. The ones that could execute before position 11 were each read:

- `main.cpp` contains zero. The only pre-root entry points that name a session class are
  `verifyShortcutProject()` (reaches `Misc::ShortcutGenerator` only) and the file-open filter below.
- `Misc/CLI.cpp:476` sits in `applyProjectAndAutoConnect`, called from `main.cpp:168` after
  `bootstrapModuleManager`. `CLI.cpp:337` runs `instantiateCoreModules()` before the benchmark, so
  `Benchmark/HotpathBenchmark.cpp` is post-root (verified in Wave C1); `BenchmarkRunner` only runs
  inside a live root.
- The ten modules pinned *ahead* of 11 contain exactly one reach, and it is not in a constructor:
  `Misc::ProblemCenter` (position 6) names it at `ProblemCenter.cpp:406`, inside
  `setupExternalConnections()`; `ProblemCenter::ProblemCenter()` is `: m_infoCount(0),
  m_errorCount(0), m_warningCount(0), m_lastRun() {}`. The checker free functions
  (`Misc/Problems/{Project,Script,Extension}Checkers.cpp`) are registered by that same function and
  run on the 1 Hz tick. `Misc::ControlScript` (position 10) contains zero.
- `Misc/DemoLauncher.cpp:72` is inside the `startDemo()` public slot, and `DemoLauncher` itself is
  first constructed at `ModuleManager.cpp:814` (context-property registration), after the root.

**The ctor closure re-proven, in full.** `ProjectModel::ProjectModel()` (`ProjectModel.cpp:76-192`)
was read end to end. It wires 40 ctor-time `connect`s, then calls `newJsonFile()` at `:169` behind
the existing `// code-verify off` note, then connects the auto-workspace regen lambda, then sets
`m_initialized = true` and `m_history.setEnabled(true)`. The closure reaches four singletons and
**none of them is its own slot** — `grep -rn "ProjectModel::instance()" app/src/DataModel/ProjectModel.cpp
app/src/DataModel/Project/ app/src/DataModel/Scripting/ControlScript.cpp` returns exactly one hit,
the definition of `instance()` itself, so adoption cannot re-enter the empty slot it is filling.

The three guards Wave C1 identified are load-bearing for this wave too, and all three were re-read
against the live files:

1. `ProjectModel.cpp:1500` — `newJsonFile()`'s `AppState` (12) and `UI::Dashboard` (32) reaches are
   both inside `if (m_initialized)`, and `m_initialized` becomes true only at `:190`. The
   function-local `static auto& appState` (`:1501`) and `static auto& dashboard` (`:1503`) are
   declared *inside* that block, so neither is even initialized during construction. Both classes are
   pinned later than 11; without this guard adoption would be a named fatal (AppState) or a Meyers
   re-entry (Dashboard, not yet adopted).
2. `Project/ProjectModelPersistence.cpp:395` — `scheduleAutoSave()` is reached from nine ctor-time
   connections (`markDirty` plus seven direct `&ProjectModel::scheduleAutoSave` connects) and does
   fire while `newJsonFile()` emits, but it returns at `:395` on `m_filePath.isEmpty()`;
   `newJsonFile()` sets `m_filePath = ""` at `:1497`, before every emission. The `static auto&
   appState` at `:398` sits after that guard.
3. `Scripting/ControlScript.cpp:193` — `newJsonFile()` calls `controlScript.setCode("")` at `:1476`;
   `setCode` returns at `:193` because `m_code` is already default-empty. Even if it did not,
   `shouldRun()` returns at `:254` on `!m_ready` (set true only in `setupExternalConnections()`,
   `:83`), and the `IO::ConnectionManager::instance()` / `AppState::instance()` statics at `:257-258`
   sit after that guard. `ConnectionManager` is position 18 — this guard is what keeps Wave D2 safe
   as well.

The rest of the closure is inert with respect to the pinned order: `watchProjectFile()`
(`ProjectModelPersistence.cpp:464-476`) touches only `m_fileWatcher` and `QFile`;
`seedDefaultFrameParser()` is a pure function (`Project/ProjectModelShared.h`); `setModified(false)`
(`ProjectModel.cpp:1854`) touches `m_history` and emits; `DataModel::ProjectHistory`'s constructor,
`clear()`, and `setEnabled()` contain no `::instance()` call at all (`grep` over
`Project/ProjectHistory.cpp` is empty), so the undo store added by spec 0031 adds no ctor out-edge.
The `NotificationCenter` reaches in the persistence TU (`:493`) are in `resolveDiskFileChange`, a
debounced watcher slot, and position 5 is adopted before 11 in any case.

| # | Check | Verdict |
|---|-------|---------|
| C1 | Grep symmetry | `DataModel::ProjectModel` is named again below the function at `ModuleManager.cpp:702` (`setupExternalConnections`) and `:803` (`Cpp_JSON_ProjectModel`). Holds. |
| C2 | INV-1 | Every `setupExternalConnections()` precedes `appState->restoreLastProject()` at `:762`, including `ProjectModel`'s own at `:702`. Same recorded drift (three statements follow the restore). Substance holds. |
| C3 | INV-2 | Unchanged: wiring -> `registerCoreContextProperties` -> `registerImageProvidersAndLoadQml()` -> `m_engine.load`. Holds. |
| C4 | INV-3 | Unchanged; this wave touches neither the message handler nor its two sinks. Holds. |
| C5 | Context out-edges | Unchanged: ctor `: m_sessionId(session_id) {}`, dtor `{}`, members are the id plus the eight slots; `arch-context-ctor-nonempty` silent. Holds. |
| C6 | Adoption order == pinned order | One line changed in place at position 11; positions 1-10 and 12-32 byte-identical. Holds. |
| C7 | No `adopt*()` outside the root | `arch-session-adopt-site` silent; five calls, all in `instantiateCoreModules()`. Holds. |
| C8 | No ctor reaches an unadopted slot | The closure section above. Reaches `ControlScript` (10, earlier, still a Meyers app-global) unconditionally, and `AppState` (12), `Dashboard` (32), `ConnectionManager` (18) only behind the three guards, none of which opens during construction. `NotificationCenter` (5) is adopted before this wave runs. Holds. |
| C9 | ProjectModel ctor closure names nothing new | This wave *is* C9. `grep -rn "SessionContext" app/src/DataModel/ProjectModel.cpp app/src/DataModel/Project/ app/src/main.cpp app/src/Misc/CLI.cpp` -> the new include and `instance()` body in `ProjectModel.cpp`, plus `main.cpp:37,186`. The closure itself names nothing new. Holds. |
| C10 | Reverse-order teardown | `shutdown()` releases `m_projectModel` seventh (`SessionContext.cpp:183`), after `m_appState` (12) and before `m_notifications` (5). Position 11 is second-lowest of the eight, so `AppState`'s `ProjectModel&` reference member is still valid when `~AppState` runs. Holds. |
| C11 | INV-6 | Unchanged: `app.exec()` -> `~ModuleManager` -> `shutdownDrivers()` (`main.cpp:183`) -> `qInstallMessageHandler(nullptr)` (`:185`) -> `shutdown()` (`:186`). `ModuleManager.cpp:380`'s `flushAutoSave()` runs from `~ModuleManager`, before the release. Holds. |

**The `FileOpenEventFilter` residual, resolved.** Wave C1 recorded that the filter is installed on
`qApp` at `main.cpp:127`, before the composition root, and that its `eventFilter` reached
`AppState::instance()` and `DataModel::ProjectModel::instance()` inline. Both are now adopted, so an
inline reach is a named fatal rather than an out-of-order construction, and the exposure had to close
here. The fix taken is the smallest of the three considered:

- *Rejected:* gate the handler on `SessionContext::current().sealed()` and drop the event. It
  silently discards a double-clicked project, and `sealed()` is false for the whole of M2's
  migration.
- *Rejected:* move the filter installation after `bootstrapModuleManager`. A `QFileOpenEvent`
  delivered during `QApplication` construction on macOS would then have no filter at all.
- *Taken:* keep the filter where it is and defer the work.
  `QMetaObject::invokeMethod(qApp, [path] { openProjectFile(path); }, Qt::QueuedConnection)`, with
  the two singleton reaches moved into the file-local `openProjectFile()`. The queued call cannot run
  before the event loop spins, and the first spin is `app.exec()` at `main.cpp:180` — past
  `bootstrapModuleManager` (`:161`) and past `applyProjectAndAutoConnect` (`:168`). The filter still
  consumes the event and still returns `true`.

The one observable change: a project opened by file association now lands *after*
`restoreLastProject()` and after any `--project` CLI argument, instead of racing them. That is the
intended precedence (an explicitly opened file wins) and was previously the opposite whenever the
event arrived pre-root.

`code-verify --check`: **0 errors**, both new arch rules silent, and no finding in any of the five
touched files.

**Not verified here (maintainer):** three-mode launch, plus project open / edit / autosave /
restore-last-project, plus a double-clicked `.ssproj` on macOS (the path the deferral changes), and
`--benchmark-hotpath`.

## Wave D1 — `DataModel::FrameBuilder` (M2-T12)

Pinned position 17, the hotpath class. Four edits, and **no edit to any per-frame code path**:

| # | File | Edit |
|---|------|------|
| 1 | `app/src/Misc/ModuleManager.cpp:658` | `(void)DataModel::FrameBuilder::instance();` -> `ctx.adoptFrameBuilder(SessionContext::create<DataModel::FrameBuilder>());`, in place |
| 2 | `app/src/DataModel/FrameBuilder.cpp:186` | `instance()` body is now `return SessionContext::current().frameBuilder();`; the Meyers `static FrameBuilder singleton;` is gone. `SessionContext.h` added to the include block |
| 3 | `app/src/DataModel/FrameBuilder.h` | `class SessionContext;` forward declaration above `namespace DataModel`, `friend class ::SessionContext;` in the private block; destructor implicit and already public |
| 4 | `app/src/SessionContext.cpp:232` | `frameBuilder()` converted from a forward to the fail-fast slot read |

**Nothing on the frame path moved.** The diff touches the constructor's access specifier region, the
`instance()` body, and one include; `parseFrame`, `trySpanLane`, `parseUtf8Spans`,
`applyDatasetValuesSpans`, `hotpathTxFrame`, `acquireFrame`, and the frame pool are byte-identical.
The cached hotpath flags are untouched in both halves of their contract: `m_operationMode` is still
seeded at the end of `setupExternalConnections()` (`FrameBuilder.cpp:582`) and refreshed in
`onOperationModeChanged()` (`:823`), and `m_playerOpen` / `m_anyAsyncSink` / `m_captureLatestFrame` /
`m_changeDriven` keep the same refresh wiring. No signal connection was added, removed, or changed
type, so no `Qt::DirectConnection` hop moved.

**`instance()` is never on the frame path, and INV-4 is why the 53 frozen references stay valid.**
`grep -rn "FrameBuilder::instance()" app/src` returns 57 sites in 31 files. Every one binds once:
43 are `static auto& frameBuilder = ...` function-local caches (including all six in
`IO/ConnectionManager.cpp` and both in `DataModel/Scripting/ScriptApiCall.cpp`, the two TUs closest
to the frame path), five are constructor-init reference members (`UI/Widgets/Painter.cpp:129`,
`DataModel/ProjectEditor.cpp:57`, `Editors/FrameParserModel.cpp:97`,
`Editors/DatasetTransformEditor.cpp:68`, `Dialogs/TransmitTestDialog.cpp:41`), three are pointer
captures inside `setupExternalConnections()` (`AppState.cpp:120`, `MDF4/Export.cpp:579`,
`Sessions/Export.cpp:677`), and the rest are composition-root, problem-checker (1 Hz), or
connection-boundary reads (`CSV/Export.cpp:391` sits inside the `connectedChanged` lambda). A slot is
filled once and released only by `shutdown()`, so the address behind every one of those references
never changes — the `SessionContext::current()` Meyers guard is entered at binding time, not per
frame, and the parse pipeline gains zero instructions.

**Pre-adoption reach: none.** `main.cpp` and `Misc/CLI.cpp` contain zero `FrameBuilder::instance()`.
Of the sixteen modules pinned ahead of 17, only `DataModel::ProjectModel` (11) names it, at
`Project/ProjectModelPersistence.cpp:374` (`syncRuntime()`, reached from `autoSave()` behind the
1500 ms debounce timer, so it needs a spinning event loop) and
`Project/ProjectModelWorkspaces.cpp:647` (`buildAutoWorkspaces()`, reached only from
`regenerateAutoWorkspacesUnnotified()`, which the constructor wires *after* `newJsonFile()` and which
early-returns unless the mode is `ProjectFile`). Neither is in the constructor closure re-proven in
Wave C2. `AppState.cpp:120` is in `setupExternalConnections()`. The two checker sites
(`Misc/Problems/LinkCheckers.cpp:233`, `ScriptCheckers.cpp:200`) run on the 1 Hz tick, registered
from `ProblemCenter::setupExternalConnections()`. `Benchmark/HotpathBenchmark.cpp` is post-root via
`CLI.cpp:337`.

| # | Check | Verdict |
|---|-------|---------|
| C1 | Grep symmetry | `DataModel::FrameBuilder` is named again below the function at `ModuleManager.cpp:705` (`setupExternalConnections`) and `:807` (`Cpp_JSON_FrameBuilder`). Holds. |
| C2 | INV-1 | Unchanged from Wave C2, same recorded drift. Substance holds. |
| C3 | INV-2 | Unchanged. Holds. |
| C4 | INV-3 | Unchanged. Holds. |
| C5 | Context out-edges | Unchanged: ctor `: m_sessionId(session_id) {}`, dtor `{}`; `arch-context-ctor-nonempty` silent. Holds. |
| C6 | Adoption order == pinned order | One line changed in place at position 17; every other position byte-identical. Holds. |
| C7 | No `adopt*()` outside the root | `arch-session-adopt-site` silent; six calls, all in `instantiateCoreModules()`. Holds. |
| C8 | No ctor reaches an unadopted slot | `FrameBuilder::FrameBuilder()` read in full (`FrameBuilder.cpp:125-181`): 34 member initializers, the `kFramePoolSize` slot-pool reserve and fill (the one allocation, and it is startup-time), a `BUILD_COMMERCIAL` `Licensing::LemonSqueezy::instance()` capture (position 14, four lines earlier, still a Meyers app-global) for the `activatedChanged` -> `syncFromProjectModel` hook, and a `qApp` `aboutToQuit` connect that sets `m_shuttingDown` and destroys the transform engines. No session slot is reached: `AppState` (12), `ProjectModel` (11), and `NotificationCenter` (5) are all adopted *before* 17 in any case, and none of them is named in the constructor. Holds. |
| C9 | ProjectModel ctor closure | Re-run: `grep -rn "SessionContext" app/src/DataModel/ProjectModel.cpp app/src/DataModel/Project/` -> only Wave C2's include and `instance()` body. The closure names nothing new. Holds. |
| C10 | Reverse-order teardown | `shutdown()` releases `m_frameBuilder` fifth (`SessionContext.cpp:181`), after `m_connectionManager` (18) and before `m_appState` (12) — so `AppState`'s `m_frameBuilder` pointer and every export module's captured pointer are released before their owners run. Exact reverse of 17's rank among the eight. Holds. |
| C11 | INV-6 | Unchanged. The `aboutToQuit` lambda still fires while `app.exec()` unwinds, so `destroyTransformEngines()` runs before `~FrameBuilder` at `main.cpp:186` instead of after `qApp` is gone. Holds. |

`code-verify --check`: **0 errors**, both new arch rules silent, no finding in the four touched files.

**Not verified here (maintainer):** `--benchmark-hotpath` on **all nine tiers** against the Wave A
baseline (this is the wave the benchmark gate exists for), three-mode launch, and both GPL and
commercial builds (the `LemonSqueezy` ctor edge only exists in the latter).

## Wave D2 — `IO::ConnectionManager` (M2-T13)

Pinned position 18. Rebased on the live file, which gained flow/recovery/diagnostics work from a
parallel spec since the census was taken. Four edits:

| # | File | Edit |
|---|------|------|
| 1 | `app/src/Misc/ModuleManager.cpp:659` | `(void)IO::ConnectionManager::instance();` -> `ctx.adoptConnectionManager(SessionContext::create<IO::ConnectionManager>());`, in place |
| 2 | `app/src/IO/ConnectionManager.cpp:153` | `instance()` body is now `return SessionContext::current().connectionManager();`; the Meyers `static ConnectionManager singleton;` is gone. `SessionContext.h` added to the include block |
| 3 | `app/src/IO/ConnectionManager.h` | `class SessionContext;` forward declaration, `friend class ::SessionContext;` in the private block, and `~ConnectionManager();` moved from the private block to the public one |
| 4 | `app/src/SessionContext.cpp:252` | `connectionManager()` converted from a forward to the fail-fast slot read |

The destructor move is the second instance of the case Wave A settled on `NotificationCenter`:
`std::default_delete<IO::ConnectionManager>` destroys the slot from the context's scope, so the
destructor must be public. It stays defined out of line in the `.cpp`, which is what keeps the ten
`std::unique_ptr` driver members destructible from a TU that only sees the header.

**Pre-adoption reach: none.** `grep -rn "ConnectionManager::instance()" app/src` returns 217 sites in
56 files, the largest of the eight. The ones that can execute before position 18:

- `main.cpp:183` is the *only* site in `main.cpp`, and it is `shutdownDrivers()` after `app.exec()`
  returns — three lines before `SessionContext::current().shutdown()` at `:186`, so the slot is still
  filled. `Misc/CLI.cpp`'s 17 sites all sit in post-`bootstrapModuleManager` command handlers.
- Of the seventeen modules pinned ahead of 18, four name it, and none from a constructor:
  `DataModel::ControlScript` (10) at `ControlScript.cpp:85` (`setupExternalConnections`) and `:257`
  (`shouldRun()`, behind the `!m_ready` guard re-proven in Wave C2 — this is the guard that keeps
  the `ProjectModel` ctor closure from reaching position 18);
  `DataModel::ProjectModel` (11) at `ProjectModel.cpp:1439` (`setupExternalConnections`),
  `Project/ProjectModelLoading.cpp:602` and `Project/ProjectModelSources.cpp:286,316` (project-load
  and source-edit paths, not the ctor closure); `DataModel::FrameBuilder` (17) at
  `FrameBuilder.cpp:503` (`setupExternalConnections`) and five `static auto&` caches in runtime
  members — `FrameBuilder`'s constructor (read in full in Wave D1) names none of them;
  `Misc::ConnectionDiagnostics` (7) at `Misc/Diagnostics/{Audio,Bluetooth,Network}Checks.cpp`, which
  are check functions run from the diagnostics runner, and whose owner's constructor is
  `: m_running(false), ... {}` with an empty body.
- `Misc/Problems/LinkCheckers.cpp:174,232` run on `ProblemCenter`'s 1 Hz tick.

**The ten UI drivers construct inside this constructor, and all ten were read.** The initializer list
builds `IO::Drivers::{UART,Network,BluetoothLE}` plus, under `BUILD_COMMERCIAL`,
`{Audio,CANBus,HID,MQTT,Modbus,Process,USB}` — ten constructor bodies that now run while
`m_connectionManager` is still empty, so a self-reach would be a named fatal rather than the Meyers
recursion it was before. A body scan of each constructor's full brace span (42, 49, 38, 56, 44, 13,
50, 86, 12, and 55 lines) finds exactly two `::instance()` calls in total, both in
`Drivers/Audio.cpp`: `Misc::TimerEvents::instance()` (`:342`, position 2) and
`Misc::Translator::instance()` (`:345`, position 1). Both are application globals pinned earlier and
not adopted. **No driver constructor reaches `ConnectionManager::instance()`**, and none reaches any
session slot.

**The four `Console::Handler` sites stay as-is.** Wave B1 recorded them at `:613,643,688,1694`; the
parallel work shifted them to `:615` (`processPayload`), `:645` (`processMultiSourcePayload`), `:690`
(`writeDataToDevice`), and `:1696` (`onRawDataReceived`). All four are still runtime data-path
members reached on the first byte received, all four are still `static auto&` binds, and position 19
is adopted one line after position 18 — long before any byte arrives.

| # | Check | Verdict |
|---|-------|---------|
| C1 | Grep symmetry | `IO::ConnectionManager` is named again below the function at `ModuleManager.cpp:689` (wiring capture), `:699` (`setupExternalConnections`), `:754` (the lifecycle-broadcast connect), `:781` and `:837` (context properties). Holds. |
| C2 | INV-1 | `ioManager->setupExternalConnections()` at `:699` precedes `appState->restoreLastProject()` at `:762`. Same recorded drift. Substance holds. |
| C3 | INV-2 | Unchanged. Holds. |
| C4 | INV-3 | Unchanged; the handler is installed at `:606`, after position 19 exists. Holds. |
| C5 | Context out-edges | Unchanged: ctor `: m_sessionId(session_id) {}`, dtor `{}`; `arch-context-ctor-nonempty` silent. Holds. |
| C6 | Adoption order == pinned order | One line changed in place at position 18; every other position byte-identical. Holds. |
| C7 | No `adopt*()` outside the root | `arch-session-adopt-site` silent; seven calls, all in `instantiateCoreModules()`. Holds. |
| C8 | No ctor reaches an unadopted slot | `ConnectionManager::ConnectionManager()` read in full (`ConnectionManager.cpp:69-117`): ten `make_unique` driver members (above), two self-connects (`busTypeChanged` -> `configurationChanged` -> `connectedChanged`), the 750 ms `m_uiDriverSaveTimer` whose timeout lambda reaches `ProjectModel` (11) and `AppState` (12) — both adopted earlier, and the lambda needs a spinning event loop in any case — and a `qApp` `aboutToQuit` -> `disconnectAllDevices` connect. No reach to a slot pinned later than 18. Holds. |
| C9 | ProjectModel ctor closure | Re-run: the closure still names nothing new, and Wave C2's third guard (`ControlScript::shouldRun()` returning on `!m_ready` before `IO::ConnectionManager::instance()` at `ControlScript.cpp:257`) is what makes that true for *this* wave specifically. Holds. |
| C10 | Reverse-order teardown | `shutdown()` releases `m_connectionManager` fourth (`SessionContext.cpp:180`), after `m_dashboard` (32), `m_frameParser` (30), and `m_console` (19), and before `m_frameBuilder` (17). Exact reverse of 18's rank among the eight. This is the wave C10 exists for: `~ConnectionManager` -> ten driver destructors (the USB event-thread join, `libusb`/`hidapi` teardown, the Process named-pipe stop) now run at `main.cpp:186` with `qApp` alive and the QML engine already dead, instead of from `__cxa_finalize`. `main.cpp:183`'s `shutdownDrivers()` still precedes the release, so the ordered driver stop happens before the destructor's `disconnectAllDevices()` fallback. Holds. |
| C11 | INV-6 | Unchanged, and load-bearing here: the engine dies at the `ModuleManager` scope exit, so no QML binding can call into a driver while it is being destroyed. Holds. |

`code-verify --check`: **0 errors**, both new arch rules silent. One advisory is reported in
`ConnectionManager.cpp` at `:1407` (`arch-singleton-instance`, the bare
`Misc::ConnectionDiagnostics::instance()` in `onDeviceOpenFinished`); it belongs to the parallel
flow/diagnostics work that added that function, is not in this wave's four edits, and is left alone.

**Not verified here (maintainer):** connect and disconnect a real device, then quit via window close,
Cmd-Q, and Dock-quit on macOS (the three paths behind the 2026-06 exit crash); quit with a USB or HID
device still attached; `--benchmark-hotpath`; three-mode launch.

## Wave D3 — `UI::Dashboard` (M2-T14)

Pinned position 32, the **last** entry, which makes this the highest-exposure wave of the eight:
every one of the 31 modules constructed before it must reach it from no constructor at all, because
after this flip a pre-adoption reach is a named fatal instead of a lazy Meyers build. Rebased on the
live file, which gained DashboardExtension bucketing from a parallel spec during this session (the
three inline helpers moved from `Dashboard.h:586-602` to `:610-626`; `instantiateCoreModules()`
gained `MDF4::Player` at position 22). Four edits:

| # | File | Edit |
|---|------|------|
| 1 | `app/src/Misc/ModuleManager.cpp:675` | `(void)UI::Dashboard::instance();` -> `ctx.adoptDashboard(SessionContext::create<UI::Dashboard>());`, in place |
| 2 | `app/src/UI/Dashboard.cpp:293` | `instance()` body is now `return SessionContext::current().dashboard();`; the Meyers `static Dashboard instance;` is gone. `SessionContext.h` added to the include block |
| 3 | `app/src/UI/Dashboard.h` | `class SessionContext;` forward declaration above `namespace UI`, `friend class ::SessionContext;` as the first line of the existing private block. The constructor was already private and the destructor is implicit and public, so nothing moves |
| 4 | `app/src/SessionContext.cpp:214` | `dashboard()` converted from the last remaining forward to the fail-fast slot read |

**On the plan's "`Dashboard.h` is not in the diff".** What that verify line protects is the three
inline helpers, and they are byte-identical. The header does gain the same two declaration lines
every prior wave needed — `std::make_unique` is not a friend of anything, so `create<T>()`'s
`new T()` cannot reach a private constructor without the `friend`. No member, no signal, no method
declaration, and no helper changed; the two lines are the entire header diff attributable to this
wave.

**Pre-adoption reach: none — and this is the wave where that claim carries the most weight.**
`grep -rn "Dashboard::instance()" app/src` returns **85 sites in 37 files** (the checked-in census
records 84 for `UI::Dashboard`; the +1 is the parallel extension-bucketing work, consistent with the
drift note above). Every site was mapped to its enclosing function:

- **`main.cpp` contains zero.** `Misc/CLI.cpp` has two — `applyVisualizationOptions()` (`:521`) and
  `applyOperatorRuntimeSettings()` (`:770`) — and both are called from `main.cpp:172,177`, inside
  the `ModuleManager` scope that opens at `:160`, so both are post-root.
- **The benchmark path is post-root.** `CLI::runHotpathBenchmark()` calls
  `Misc::ModuleManager::instantiateCoreModules()` at `CLI.cpp:337` and only then
  `Benchmark::HotpathBenchmark::runAndReport(...)` at `:339`, so the two reaches in
  `HotpathBenchmark.cpp` (`:181` `setActive`, `:487` `activateDashboardWidgets`) and the two in
  `BenchmarkRunner.cpp` (`:465,500`) run against a filled slot. This is exactly the property M2-T3
  was landed for, and it is what keeps the 256 kHz gate runnable after this wave.
- **No constructor of any earlier-pinned class reaches it.** The reaches that live in earlier-pinned
  modules are: `DataModel::ProjectModel` (11) at `ProjectModel.cpp:1403,1408,1420`
  (`setupExternalConnections()`), `:1536` (`newJsonFile()`, **inside the `if (m_initialized)` guard
  at `:1533`**, and `m_initialized` becomes true only at the end of the constructor), `:1626,1649`
  (`setPointCount` / `setPlotTimeRange`, editor paths) and `Project/ProjectModelLoading.cpp:966,991`
  (project-load paths); `DataModel::FrameBuilder` (17) at `FrameBuilder.cpp:867,2264,2307`
  (`republishFrames`, `hotpathTxFrame`, `publishReplayFrame` — all runtime, and its constructor was
  read in full in Wave D1); `Misc::ExtensionManager` (9) at `ExtensionManager.cpp:1660,1754,1788`
  (`stopPlugin`, `onDashboardAvailableChanged`, `onPluginFinished`); `CSV::Player` (21),
  `MDF4::Player` (22), and `Sessions::Player` (23) at seven, seven, and six playback members
  (`nextFrame`, `previousFrame`, `seekWindowStartRow`, `performSeekTick`, `performSeekSettle`,
  `buildSeekWindow`, `updateData` / `catchUpToTarget`); `DataModel::FrameParser` (30) through the
  file-local helpers in `Scripting/DashboardApi.cpp:81,103,122` and
  `Scripting/DeviceWriteApi.cpp:259`, which are script-callback bodies (Wave B2 already proved the
  parser's constructor reaches neither).
- **The four constructors that could plausibly have hidden one were read in full**:
  `CSV::Player::Player()` (`:160-194`), `MDF4::Player::Player()` (`:57-84`),
  `Sessions::Player::Player()` (`:54-87`), and `Misc::ExtensionManager::ExtensionManager()`
  (`:81-102`). None names `Dashboard`, and none calls a member that does — the players wire only
  `performSeekTick` / `performSeekSettle` to their own timers (`QTimer::timeout`, so an event loop
  is required in any case), and `ExtensionManager`'s three ctor callees (`loadInstalledManifest`,
  `applyFilter`, `rebuildInstalledPlugins`) touch settings, filters, and metadata only.
- The remaining sites are QML-side objects the engine builds after the root (18 `m_dashboard(...)`
  constructor-init reference captures across `DashboardWidget`, `Taskbar`, `WindowManager` and 15
  widget classes), the API/MCP command handlers, `UI::AlarmMonitor::setupExternalConnections()`
  (`:61,64`), and the root's own three (`ModuleManager.cpp:675,691,793`).

**Verdict: no earlier-pinned constructor reaches position 32.** The wave proceeds.

**INV-4 accounting for the 85 sites.** 57 are `static auto& dashboard = UI::Dashboard::instance();`
function-local caches, 18 are constructor-init reference members, and 10 are the remainder listed
below. Every one of them binds the address exactly once, and a slot is filled once and released only
by `shutdown()`, so none needs an edit:

| Family | Count | Why INV-4 keeps it valid |
|--------|-------|--------------------------|
| `static auto&` function-local caches | 57 | Resolved on first call, which is after the root in every one of the mapped functions; the address never moves afterwards |
| Constructor-init reference members (`, m_dashboard(UI::Dashboard::instance())`) | 18 | The 15 widgets plus `DashboardWidget`, `Taskbar`, `WindowManager` are built by the QML engine, which lives strictly inside the root and dies at `~ModuleManager` (INV-6) — before the slot is released |
| The three `Dashboard.h` inline helpers (`:612,618,624`) | 3 (of the 57) | `GET_GROUP` / `GET_DATASET` / `VALIDATE_WIDGET` each carry a **per-TU** function-local static, so the header multiplies one binding into one per including TU. INV-4 is what makes that harmless: every copy resolves to the same never-moving address, and each resolves on its first widget draw, long after adoption. Leaving them untouched is the point of the wave |
| QML context property (`ModuleManager.cpp:793`) | 1 | `Cpp_UI_Dashboard` holds a raw pointer for the engine's lifetime; INV-2 puts the registration after wiring and INV-6 kills the engine before `shutdown()` |
| Root wiring capture (`:691`) and pinned line (`:675`) | 2 | Inside `instantiateCoreModules()` / `setupCrossModuleConnections()` |
| `ProjectModel::setupExternalConnections()` (`:1403,1408,1420`) | 3 | Two are `&instance()` arguments retained inside `QMetaObject::Connection`, one is a direct read; all post-root, and Qt drops the connections when `~Dashboard` runs |
| `AlarmMonitor::setupExternalConnections()` (`:61,64`) | 2 | A retained pointer member plus a local reference, taken after the root |
| The definition and the accessor (`Dashboard.cpp:293`, `SessionContext.cpp:216`) | 2 | Rewritten by this wave |

**Nothing on the frame or draw path moved.** The diff touches the header's private-block access
region, the `instance()` body, and one include. `hotpathRxFrame`, `updateDashboardData`,
`reconfigureDashboard`, every push table, and every DSP ring are byte-identical. The cached hotpath
flag is untouched in both halves of its contract: `m_streamAvailable` is still seeded in the
constructor by `updateStreamAvailable()` (`Dashboard.cpp:267`) and still refreshed by
`connectStreamAvailableInputs()` (`:429-458`), whose four `Qt::DirectConnection` hops from
`ConnectionManager::connectedChanged`, `CSV::Player::openChanged`, `MDF4::Player::openChanged`, and
(commercial) `Sessions::Player::openChanged` are unchanged in count, order, and connection type. The
`Misc::TimerEvents::uiTimeout` -> `updated()` render tick at `:246-251` is likewise unchanged. No
signal connection was added, removed, or retyped by this wave.

| # | Check | Verdict |
|---|-------|---------|
| C1 | Grep symmetry | `UI::Dashboard` is named again below the function at `ModuleManager.cpp:691` (wiring capture), `:765` and `:770` (the two trailing connects), and `:793` (`Cpp_UI_Dashboard`). Holds. |
| C2 | INV-1 | `appState->restoreLastProject()` at `:762` follows every `setupExternalConnections()`. Same drift recorded since Wave A: `refreshRepositories()` and the two `uiDashboard` connects follow it, so it is not the literal last statement. Substance holds. |
| C3 | INV-2 | `setupCrossModuleConnections()` `:604` -> `registerCoreContextProperties(c)` `:614` -> `registerImageProvidersAndLoadQml()` `:621` -> `m_engine.load` `:931`. Unchanged. Holds. |
| C4 | INV-3 | `qInstallMessageHandler(MessageHandler)` at `:606` still runs after `:604`, so positions 5 and 19 exist before any warning can route. Unchanged by this wave. Holds. |
| C5 | Context out-edges | `SessionContext::SessionContext(int)` is still `: m_sessionId(session_id) {}` and `~SessionContext()` still `{}`; members are the id plus the eight slots; `arch-context-ctor-nonempty` silent. Holds. |
| C6 | Adoption order == pinned order | One line changed in place at position 32; positions 1-31 byte-identical. All eight `adopt*()` calls now sit at positions 5, 11, 12, 17, 18, 19, 30, 32 — the same positions their `(void)X::instance();` lines held. Holds. |
| C7 | No `adopt*()` outside the root | `arch-session-adopt-site` silent; eight calls, all in `instantiateCoreModules()`. Holds. |
| C8 | No ctor reaches an unadopted slot | `UI::Dashboard::Dashboard()` read in full (`Dashboard.cpp:156-269`): 18 member initializers, then six `static auto&` binds — `CSV::Player` (21), `MDF4::Player` (22), `IO::ConnectionManager` (18), `AppState` (12), `DataModel::FrameBuilder` (17), `DataModel::ProjectModel` (11) — plus `Sessions::Player` (23) and `Licensing::LemonSqueezy` (14) under `BUILD_COMMERCIAL`, `Misc::TimerEvents` (2), and the same players/manager again through `connectStreamAvailableInputs()`. **Every one is pinned strictly earlier**, which is what being last buys: five of them are session slots already filled by Waves C1, C2, D1, D2, and the rest are Meyers-held application globals. The two ctor bodies that then run, `updateStreamAvailable()` and `restorePersistedSettings()`, read those same references and `QSettings` only. Holds. |
| C9 | ProjectModel ctor closure | Re-run: `grep -rn "SessionContext" app/src/DataModel/ProjectModel.cpp app/src/DataModel/Project/ app/src/main.cpp app/src/Misc/CLI.cpp` -> Wave C2's include and `instance()` body plus `main.cpp:37,186`. The closure names nothing new, and its one `Dashboard` reach stays behind the `m_initialized` guard. Holds. |
| C10 | Reverse-order teardown | `shutdown()` releases `m_dashboard` **first** (`SessionContext.cpp:177`), which is the exact reverse of position 32 being constructed last. Its six earlier-pinned collaborators are therefore all still alive when `~Dashboard` runs. With this wave the sequence is fully populated for the first time: 32 -> 30 -> 19 -> 18 -> 17 -> 12 -> 11 -> 5. Holds. |
| C11 | INV-6 | Unchanged: `app.exec()` (`main.cpp:180`) -> `~ModuleManager` at the scope close `:181` (the QML engine dies, so no widget's `m_dashboard` reference outlives the object) -> `shutdownDrivers()` `:183` -> `qInstallMessageHandler(nullptr)` `:185` -> `shutdown()` `:186`. Holds, and it is load-bearing here: the 18 constructor-init reference captures all live in QML-owned objects that the engine destroys first. |

`code-verify --check`: **0 errors**, both new arch rules silent, no finding in any of the four
touched files. `pytest tests/scripts/`: 301 passed.

**Not verified here (maintainer):** three-mode launch with a live dashboard and a replay;
`--benchmark-hotpath` on all nine tiers against the Wave A baseline.

## M2 consolidation — the eight slots, the verdict, and what is still open (M2-T17)

**Verdict: the ctor-edge proof is re-derived, not preserved.** M1 could only claim the weaker form
because spec 0039 had added one object with zero constructor out-edges and converted no pinned
module. M2 converted eight, one wave at a time, and each wave re-ran the eleven checks against the
live composition root rather than against a snapshot — which mattered, because the function drifted
three times underneath the milestone (`UI::WidgetExtensions` and `MDF4::Player` were added by
parallel specs, and `ConnectionManager` / `Dashboard` were edited by parallel work mid-wave).

### Adoption sequence, as it now stands in `instantiateCoreModules()`

| Pinned position | Class | Wave | Task |
|-----------------|-------|------|------|
| 5 | `DataModel::NotificationCenter` | A | M2-T7 |
| 11 | `DataModel::ProjectModel` | C2 | M2-T11 |
| 12 | `AppState` | C1 | M2-T10 |
| 17 | `DataModel::FrameBuilder` | D1 | M2-T12 |
| 18 | `IO::ConnectionManager` | D2 | M2-T13 |
| 19 | `Console::Handler` | B1 | M2-T8 |
| 30 | `DataModel::FrameParser` | B2 | M2-T9 |
| 32 | `UI::Dashboard` | D3 | M2-T14 |

The other 24 pinned entries are unchanged `(void)X::instance();` lines. The session/application split
is now visible in the source: an adopted module may depend on a Meyers-held application global
(`Translator`, `TimerEvents`, `CommonFonts`, `ThemeManager`, `ControlScript`, the licensing four),
and every such edge points strictly earlier in the pinned order.

### Teardown sequence, now fully populated

`SessionContext::shutdown()` (`SessionContext.cpp:175-185`) releases:

```
32 Dashboard -> 30 FrameParser -> 19 Console::Handler -> 18 ConnectionManager
-> 17 FrameBuilder -> 12 AppState -> 11 ProjectModel -> 5 NotificationCenter
```

which is the exact reverse of the table above, position for position. Called from `main.cpp:186`,
after `~ModuleManager` has destroyed the QML engine (INV-6), after `shutdownDrivers()` at `:183`,
and after `qInstallMessageHandler(nullptr)` at `:185`. Both preconditions of the
`__cxa_finalize` teardown crash class are removed for the owned eight: destructors run with `qApp`
alive, and no owned module is destroyed before something it depends on.

### The eleven checks across all eight waves

| # | Check | A | B1 | B2 | C1 | C2 | D1 | D2 | D3 | Note |
|---|-------|---|----|----|----|----|----|----|----|------|
| C1 | Grep symmetry | ok | ok | ok | ok | ok | ok | ok | ok | every adopted class is still named again below the function |
| C2 | INV-1 | ok* | ok* | ok* | ok* | ok* | ok* | ok* | ok* | *substance holds (wiring precedes `restoreLastProject()`); its literal phrasing broke before M2 started, when a parallel spec added three statements after the restore at `ModuleManager.cpp:762`. Not caused by, and not fixed by, this milestone |
| C3 | INV-2 | ok | ok | ok | ok | ok | ok | ok | ok | wiring -> context properties -> `m_engine.load`, untouched by all eight waves |
| C4 | INV-3 | ok | ok | ok | ok | ok | ok | ok | ok | B1 is the wave that exercised it: the handler at `:606` is installed after positions 5 and 19 |
| C5 | Context out-edges | ok | ok | ok | ok | ok | ok | ok | ok | ctor and dtor still empty in all eight; `arch-context-ctor-nonempty` silent throughout |
| C6 | Adoption == pinned order | ok | ok | ok | ok | ok | ok | ok | ok | each wave's `ModuleManager.cpp` diff is exactly one line changed in place |
| C7 | No `adopt*()` outside the root | ok | ok | ok | ok | ok | ok | ok | ok | `arch-session-adopt-site` silent; final count eight, all in `instantiateCoreModules()` |
| C8 | No ctor reaches an unadopted slot | ok | ok | ok | ok | ok | ok | ok | ok | every constructor read in full; the interesting edges are B2's `FrameParser -> FrameBuilder/NotificationCenter`, C1's `AppState -> ProjectModel` reference member, C2's three guards, D2's ten driver constructors, and D3's six earlier-pinned binds |
| C9 | ProjectModel closure | ok | ok | ok | ok | ok | ok | ok | ok | the standing grep names only Wave C2's include and `instance()` body plus `main.cpp:37,186` |
| C10 | Reverse-order teardown | ok | ok | ok | ok | ok | ok | ok | ok | fully populated by D3; the release order is the reverse of the table above |
| C11 | INV-6 | ok | ok | ok | ok | ok | ok | ok | ok | engine dies at the `ModuleManager` scope close, objects at `main.cpp:186` |

Two behavior changes were made deliberately during the milestone and are recorded where they
happened, not folded away here: the `main.cpp:155` `AppState` reach was routed through
`ModuleManager::setEphemeralSession()` (M2-T3), and `Platform::FileOpenEventFilter` now defers its
project open through a queued call (Wave C2), which makes a double-clicked project land after
`restoreLastProject()` instead of racing it.

### Maintainer launch gates — the whole checklist, in one place

None of these can be run by the implementing pass; all of them are open.

- [ ] Launch in **ProjectFile**, **QuickPlot**, and **ConsoleOnly** (every wave asked for this; the
      milestone needs it once on the final tree). ProjectFile specifically covers the only branch of
      `AppState::deriveFrameConfig()` that touches `ProjectModel` during construction.
- [ ] Open, edit, autosave, and restore a project; confirm restore-last-project still runs.
- [ ] Double-click a `.ssproj` on macOS — the path Wave C2's deferral changed.
- [ ] Connect and disconnect a real device, then quit via **window close**, **Cmd-Q**, and
      **Dock-quit** on macOS: the three paths behind the 2026-06 exit crash. Repeat with a USB or HID
      device still attached (thread join, `libusb_exit`, `hid_exit`).
- [ ] Load a project with a **Lua** parser (Wave B2's ctor edge) and one with a live dashboard plus a
      replay (Wave D3).
- [ ] `--benchmark-hotpath`: **all nine tiers** against the Wave A baseline, not just the headline
      number. Waves C and D are the reason the gate exists — the objects moved from `.bss` to the
      heap, and `FrameBuilder`, `ConnectionManager`, and `Dashboard` are the three the parse path
      touches.
- [ ] Build and launch **both GPL and commercial** configurations (the `LemonSqueezy` ctor edges in
      `FrameBuilder` and `Dashboard` exist only in the latter).

### What M2 does not fix, stated plainly

- The teardown crash class is closed **for the owned eight only**. `AI::Assistant`,
  `JsWatchdogThread`, the eight `FrameConsumer` subclasses, the exports, the players, the session
  database, and the licensing family still destruct under `__cxa_finalize` after `qApp` is gone.
- The census total is **unchanged** by the milestone: adoption rewrites accessor bodies, it deletes
  no call site. That is the design (INV-4), not an oversight.
- Two invariants are now executable rather than documented — a pre-adoption reach is a named fatal
  (fail-fast accessor) and a second construction is impossible outside the root (private ctor +
  `friend`, `adopt*()` asserting an empty slot) — but the roadmap's "two independent contexts with
  no state bleed" criterion is **not met by M2**; see `m2-bleed-census.md` and M2-T18.
