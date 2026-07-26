---
spec: 0039-session-context
phase: plan (milestone M2)
status: in-progress  # draft -> approved -> in-progress (waves land one at a time)
updated: 2026-07-25
---

# M2 — Ownership: the context stops forwarding and starts owning

> **Milestone 2 of the M1/M2/M3 path recorded in [`spec.md`](./spec.md).** M1 landed the seam:
> a stateless `SessionContext` whose accessors forward to the existing Meyers singletons, three
> injected pilots, a linter rule, and a census. M2 moves *ownership*. M3 (plurality) is out of
> scope and is kept honest in "M2 versus M3" below.
>
> Gate: this is a plan, not a task list to start. Do not implement before a human marks it
> `approved`. Every wave is one build-and-launch cycle (spec 0001's standing rule).

## What M2 must deliver, and to whom

Two consumers define the deliverable, and they want different halves of it.

**R7 / spec 0040 (remote dashboard attach)** states the gate precisely
([`0040/plan.md:286-290`](../0040-remote-dashboard/plan.md)): *"M2 is where the context stops
forwarding and starts owning the session subsystems. What this plan consumes from it: the
ability to snapshot and restore the session's project document and dashboard-facing state as
one unit, so attach is reversible and detach genuinely returns the user to where they were. v1
keeps one dashboard object and swaps its feed."* So R7 needs the **owned set to be enumerable
and reachable as one unit through an injected handle**, and it needs detach to be deterministic.
It explicitly does **not** need a second context (that is M3, and 0040 says so).

**The unit tier (AC3, T13)** is blocked on a *link* set, not on ownership: `ProtoImporter.cpp`
drags `Misc::Utilities`, `ProjectModel` + its 18 TUs, and `SerialStudio` into any suite that
links it. Ownership alone does not cut that edge. This plan says so out loud and carries one
scoped task (M2-T16) that does cut it, rather than letting the roadmap keep pointing at "M2"
for something M2's mechanism does not by itself provide.

**The roadmap (R4)** asks for two contexts with no state bleed. M2 does not deliver that; see
"M2 versus M3".

## Ownership mechanism — the verdict

**The context holds `std::unique_ptr<T>` slots. Construction stays line-for-line inside
`ModuleManager::instantiateCoreModules()`, in the same pinned order, one line per module.
`T::instance()` becomes a one-line forwarder into the context. The context's constructor stays
empty.**

Shape, per owned class (three edits, no more):

```cpp
// (1) SessionContext.h / .cpp -- a slot, an adopt, and an accessor that cannot return null
void SessionContext::adoptProjectModel(std::unique_ptr<DataModel::ProjectModel> model)
{
  SS_ASSERT(!m_projectModel, return);          // single assignment; INV-5
  SS_ASSERT(model != nullptr, return);
  m_projectModel = std::move(model);
}

DataModel::ProjectModel& SessionContext::projectModel() const
{
  SS_ASSERT_FATAL(m_projectModel != nullptr);  // reached before adoption, or after shutdown
  return *m_projectModel;
}

// (2) ModuleManager.cpp -- the pinned line, substituted in place
//     was: (void)DataModel::ProjectModel::instance();
ctx.adoptProjectModel(std::make_unique<DataModel::ProjectModel>());

// (3) ProjectModel.cpp -- the accessor stops being the owner
DataModel::ProjectModel& DataModel::ProjectModel::instance()
{
  return SessionContext::current().projectModel();
}
```

Four properties make this the shape to build:

1. **It is a substitution, not a sweep.** Nothing outside those three edits changes per class.
   The 1,573 `::instance()` sites, the 66 QML context properties, the 172 `&X::instance()`
   connect arguments, and the 90 constructor captures all keep working unmodified, because they
   go through `instance()` and `instance()` still returns the same object at the same address.
2. **It is per class.** A class is adopted independently of the other seven; the tree compiles
   and runs after each single flip. That is what makes eight waves possible instead of one flag
   day — the option spec 0001 rejected twice.
3. **The pinned order stays where the proof can see it.** The order remains 31 consecutive lines
   in `instantiateCoreModules()`, in the same file, in the same sequence; the diff for a wave is
   one line changing from `(void)X::instance();` to `ctx.adoptX(std::make_unique<X>());`. The
   proof's subject keeps its shape even though the proof itself must be re-derived.
4. **The pinned order becomes executable.** Today a module whose ctor reaches a module pinned
   *later* silently gets a lazily-constructed one, out of order, and the ordering claim lives
   only in a document. After adoption the same reach hits a null slot and aborts at startup with
   a named module. The ctor-edge proof stops being prose and becomes an assertion — which is the
   direct answer to spec 0001's refutation of the service locator (*"does nothing about the
   actual disease - unpinned lazy construction order"*, `0001/spec.md:70-73`). M2 is the shape
   that does something about it.

### Refuted: the constructor owns the modules

The obvious alternative — `SessionContext`'s constructor runs the pinned order and initializes
its members — is **refused**, and this is the single most important line in the plan.

`SessionContext::current()` is a function-local static. If the context's constructor built the
modules, then a module constructor that reaches any singleton (`FrameBuilder`'s ctor calls
`LemonSqueezy::instance()`; `FrameParser`'s ctor reaches `FrameBuilder` through
`LuaScriptEngine.cpp:167`; `AppState`'s ctor calls `ProjectModel::instance()`) would re-enter
`current()` *while its guard is still held* — `__cxa_guard_acquire detected recursive
initialization`, abort at startup. That is exactly the crash that shipped on 2026-07-07 from
ProjectModel's ctor closure, one level up.

**Invariant M2 adds: `SessionContext`'s constructor and destructor stay empty. Ownership is
installed by `adopt*()` calls after `current()` has fully returned, and released by an explicit
`shutdown()`.** The context is published (`auto& ctx = SessionContext::current();`) as the first
statement of `instantiateCoreModules()`, so the guard is released before the first module is
built. A linter rule pins this (M2-T6).

### Refuted: lazy fallback in the accessor

`return m_projectModel ? *m_projectModel : ProjectModel::instance();` would make every pre-root
reach keep working. It is refused: a fallback re-creates unpinned lazy construction inside the
very object built to end it, makes the ordering assertion unfirable, and leaves two live
instances possible (the owned one and the Meyers one) the moment the fallback fires once. The
pre-root reaches are real (below) and are fixed by relocating them, not by tolerating them.

### Why this is not the service locator spec 0001 rejected

Spec 0001 rejected a service locator because it *"keeps the exact same global state behind a
slower, grep-hostile lookup, adds indirection cost on a path where 256 kHz forbids it, and does
nothing about the actual disease"*. Point by point:

- *Grep-hostile*: slots are named members with named `adopt*()`/accessor pairs — eight greppable
  symbols, not a `get<T>()` type map. `grep -n adopt app/src/Misc/ModuleManager.cpp` is the
  ownership manifest.
- *Indirection on the hotpath*: the resolved reference is frozen at every call site that matters
  (see the next section); per-frame code pays nothing, and the benchmark gates the waves that
  touch frame-path classes.
- *Does nothing about the disease*: it does the opposite — it converts the ordering claim into a
  startup assertion and the teardown order into an explicit sequence.

The part of the refutation that still binds is the flag-day objection, which is why M2 is eight
one-class waves and touches no call site.

## The static-cache invariant — say it explicitly

The census baseline (`scripts/singleton-census.json`, 1,573 occurrences across 191 files) breaks
down as root 151, accessor 65, ctor-capture 92, deferred 19, **static-cache 1,097**, loose 149.
An independent grep finds 1,082 of the `static auto& x = X::instance();` form plus 11
`static const auto* x = &X::instance();` pointer caches (all `ThemeManager`, an app-global). The
eight session classes account for 775 of the 1,573 sites: `ConnectionManager` 213,
`ProjectModel` 205, `AppState` 139, `Dashboard` 84, `FrameBuilder` 56, `Console::Handler` 31,
`NotificationCenter` 28, `FrameParser` 19. (`spec.md` quotes 1,524/969 from the census taken when
it was written; the tree has since drifted upward through seven parallel specs. The checked-in
baseline is authoritative.)

Every one of those sites resolves `X::instance()` **once**, on first call, and holds the
resulting reference for the life of the process. Three more families do the same thing by other
means: 90 constructor init-list captures across 30 TUs (14 widget models capture `Dashboard`),
the three `inline` helpers at `UI/Dashboard.h:586-602` (`GET_GROUP`, `GET_DATASET`,
`VALIDATE_WIDGET` — each carries its own function-local static in every TU that includes the
header), and 172 `&X::instance()` arguments whose addresses are retained inside
`QMetaObject::Connection` objects. Plus 66 QML context properties (48 core, 18 commercial) that
hold raw pointers for the engine's lifetime.

**Therefore M2 adds two invariants, and they are what makes the whole milestone a substitution
rather than a 1,400-site sweep:**

- **INV-4 (address stability).** An owned subsystem's address never changes for the life of a
  session. The slot is assigned exactly once and is never reseated, reset, swapped, or
  reconstructed while the session runs. Every frozen reference, captured member, retained
  connect address, and QML context property therefore stays valid with zero edits.
- **INV-5 (single assignment).** `adopt*()` asserts the slot is empty. There is no `replace`,
  no `reset`, no `emplace` API. The only transition out of "owned" is `shutdown()`, which is the
  last statement of the session and after which nothing may touch a session subsystem.

INV-4 is also the reason R7 must swap the dashboard's *feed and state*, never its objects — which
is precisely the design 0040 already chose ("one dashboard, swapped feed").

The cost being paid for INV-4 is that the objects move from `.bss` to the heap. That is a
locality change on code the hotpath touches, which is why the benchmark is a gate on waves C and
D rather than a formality.

### Hotpath impact, concretely

- No per-frame call site changes. `FrameBuilder`, `Dashboard`, and `ConnectionManager` reach
  their collaborators through captured members and frozen statics, not through per-frame
  `instance()` calls. The frozen sites in hotpath TUs — `ConnectionManager.cpp` 44,
  `FrameBuilder.cpp` 42, `Dashboard.cpp` 38, `DashboardHandler.cpp` 25, `FrameReader.cpp` 1 —
  each pay one extra pointer load exactly once, on first call.
- The 149 `loose` sites pay one extra load per call. None is on the frame path (the advisory
  reports zero loose reaches in hotpath files).
- No signal, no connection type, and no cached hotpath flag changes. `m_operationMode`,
  `m_playerOpen`, `m_anyAsyncSink`, `m_captureLatestFrame`, `m_changeDriven`, and Dashboard's
  `m_streamAvailable` gain and lose nothing.
- `--benchmark-hotpath` is a **hard gate** on waves C and D, and is run and recorded on A and B
  as well because the accessor bodies of classes the parse path calls have changed.

## QML impact

None, by construction. `registerCoreContextProperties` / `registerCommercialContextProperties`
keep calling `ctx->setContextProperty("Cpp_X", &X::instance())` verbatim; `instance()` returns
the same object, at the same address, at the same point in startup. INV-2 (properties after
wiring, before `m_engine.load`) is untouched. The context itself is still not a `QObject`, still
not registered, still not bindable — the M1 non-goal holds through M2 and is revisited at M3.

Two QML-adjacent lifetime facts M2 must respect and verify (M2-T5):

- `QQmlEngine::setObjectOwnership(nc, QQmlEngine::CppOwnership)` at
  `DataModel/NotificationCenter.cpp:624`, where the same pointer is exposed to every Pro script
  engine as `__nc` via `newQObject`. A script engine that outlives the session would hold a
  dangling wrapper; today nothing does, and M2 must keep it that way.
- `~ModuleManager` (implicit) destroys `m_engine` at `main.cpp:182`, *before* the shutdown point
  chosen below. Engine dies, then objects. That ordering is load-bearing and becomes INV-6.

## Teardown — the win, and the new hazards

### What today actually does

`main.cpp` runs `app.exec()` at `:181`, drops the `ModuleManager` scope at `:182` (destroying
the QML engine), calls `IO::ConnectionManager::instance().shutdownDrivers()` at `:184`, returns
at `:187`, destroys `QApplication`, and only then lets `__cxa_finalize` destruct every Meyers
singleton **in reverse first-touch order, after `qApp` is gone**. Ordered teardown exists only
as `ModuleManager::onQuit()` (`:353-390`) — a hand-maintained list of 14 `instance()` calls wired
to both `QQmlApplicationEngine::quit` and `aboutToQuit`.

That arrangement has produced the same crash three times
(`project_dbworker_static_dtor_crash_2026_06`):

1. `~DatabaseManager -> closeDatabase -> QSqlQuery` under `__cxa_finalize`, because macOS Cmd-Q
   bypassed the only signal `onQuit` was wired to.
2. `~Assistant -> Redactor::scrub` touching a function-local `static QList` that was built later
   and therefore destroyed earlier. Fixed by making that list intentionally immortal.
3. `~ConnectionManager -> ~USB -> QThread::wait` with no `qApp`, on the headless benchmark path
   that has no `ModuleManager` and no `onQuit` at all.

The class has exactly two preconditions: **(a)** the destructor runs after `qApp` and after other
statics are gone, and **(b)** it touches something that was built later than itself.

### What M2 does about it

`SessionContext::shutdown()` destroys the owned slots **in reverse pinned order**, at an explicit
point inside `main`, while `qApp` is alive:

```
app.exec()                              main.cpp:181
~ModuleManager  (QML engine dies)       main.cpp:182
ConnectionManager::shutdownDrivers()    main.cpp:184
qInstallMessageHandler(nullptr)         new -- before the console/notification slots die
SessionContext::current().shutdown()    new -- reverse pinned order, qApp alive
return status                           main.cpp:187
~QApplication, then __cxa_finalize
```

This kills **both** preconditions for every owned module:

- **(a) is gone**: an owned destructor now runs before `main` returns, so `qApp`, the Qt plugin
  registries, QtSql, and every TU-local static are still alive. The scattered `if (qApp)` guards
  (`DatabaseWorker.cpp:124`, `DatabaseManager.cpp:192`) become belt-and-braces instead of the
  only thing standing between the app and a crash on quit.
- **(b) is gone among owned modules**: destruction is the exact reverse of a pinned construction
  order, so a module's dependencies are constructed before it and destroyed after it. The
  "singleton dtor touches a later-built static" shape is not expressible inside the owned set.

Two further consequences worth naming:

- **The benchmark path gets teardown for free.** `--benchmark-hotpath` builds no `ModuleManager`
  and runs no `onQuit`; `HotpathBenchmark::disableConsumers()` no longer replicates the driver
  and session teardown that the 2026-06 fix added. Once the benchmark bootstraps through the
  same pinned sequence (M2-T3), it can call the same `shutdown()`, and instance 3 of the crash
  class stops being reachable from that entry point.
- **`onQuit()` stops being the whole story.** It keeps its job — quiescing (flush autosave, close
  files, stop timers, disconnect devices) while the event loop is alive — but it is no longer
  also the *only* destruction discipline. The plan does not rewrite `onQuit` in M2; it stops
  being load-bearing for lifetime.

### New hazards that earlier destruction introduces

Destroying earlier is strictly safer for the owned set, and strictly *newer* for everything that
assumed those objects outlive the process. Each of these is a task-level check:

- **A warning after `shutdown()`** would reach `MessageHandler`, which reaches `Console::Handler`
  and `NotificationCenter`. Hence `qInstallMessageHandler(nullptr)` immediately before
  `shutdown()`, not in the post-routine at `ModuleManager.cpp:595` (which runs later, in
  `~QCoreApplication`).
- **`main.cpp:184` uses `ConnectionManager` after `~ModuleManager`.** `shutdown()` must come
  after that line, or the call is a null-slot fatal.
- **Frozen references become dangling after `shutdown()`.** Every one of the ~1,100 cached
  statics holds `&*m_x`. Nothing may touch a session subsystem after the shutdown point; because
  that point is the last statement of `main`'s body, nothing does — but the accessor's null
  assert turns any future violation into a named fatal instead of a use-after-free, which is
  strictly better than today's destroyed-Meyers-static UB.
- **`qApp`-registered hooks** (event filters, `aboutToQuit` connections, post routines) now
  outlive their objects. Qt auto-removes both on `QObject` destruction, so this is a verification
  item, not a redesign — but it is exactly the kind of assumption that has bitten here before
  (`Sessions::Player` installs a `qApp` event filter and never removes it explicitly).
- **Out of scope, and stated as such**: the exposure survives for singletons M2 does not adopt —
  `AI::Assistant`, `JsWatchdogThread` (whose dtor does a `BlockingQueuedConnection` + `wait()`
  with no `qApp` guard and no `onQuit` caller), and the eight `FrameConsumer` subclasses whose
  `~FrameConsumer` joins a worker thread unguarded. M2 fixes the class *for the session set*;
  extending ownership to the export/AI/watchdog family is a later milestone, and this plan must
  not be read as having fixed them.

## Pre-root reaches — the landmine that gates every wave

Fail-fast accessors mean any reach *before* `instantiateCoreModules()` runs is a fatal. Three
exist today:

| Site | Reaches | Runs |
|------|---------|------|
| `main.cpp:155` | `AppState::instance().setEphemeralSession(true)` | commercial runtime mode, before `bootstrapModuleManager` |
| `CLI::runHotpathBenchmark` -> `Benchmark::HotpathBenchmark` | `ProjectModel`, `AppState`, `FrameParser`, `FrameBuilder`, `Dashboard`, `CSV/MDF4/Sessions::Export`, `API::Server`, `GRPCServer` (18 cache sites in `HotpathBenchmark.cpp`, 21 in `BenchmarkRunner.cpp`) | `cli.process(app)` at `main.cpp:135`, before any `ModuleManager` |
| `CLI::dumpApiSchema` | `API::CommandHandler`, `API::CommandRegistry` | same early-exit path (not in the session eight; recorded for the later waves) |

The benchmark one is the serious one: **the 256 kHz CI gate itself runs without the composition
root**. If the pinned sequence stays private to `ModuleManager::initializeQmlInterface`, adopting
`FrameBuilder` breaks the gate that is supposed to prove the adoption safe. M2-T3 makes the
pinned sequence callable from the benchmark entry point and relocates the `main.cpp:155` reach;
it lands **before** any wave, and waves C and D cannot start without it.

## Ctor-edge proof — re-derived, not preserved

M1's verdict was *"preserved, not re-derived. Spec 0039 adds one object whose constructor has
zero out-edges ... converts no pinned module to constructor injection"*
([`ctor-proof.md:177-180`](./ctor-proof.md)). **M2 breaks that last clause on purpose.** The proof
must be re-derived, once per wave, in `ctor-proof-m2.md` (one section per wave, same format as
M1's file). The check list is M1's five plus six new ones:

| # | Check | How |
|---|-------|-----|
| C1 | Grep symmetry against the pinned order | every adopted class still named again below the function |
| C2 | INV-1 | `restoreLastProject()` still the last statement of `setupCrossModuleConnections()` |
| C3 | INV-2 | context properties after wiring, before `m_engine.load` |
| C4 | INV-3 | message handler installed after `Console::Handler` and `NotificationCenter` exist |
| C5 | Out-edge check on the context | ctor and dtor still empty; no member other than the id and the slots |
| C6 | **Adoption order == pinned order, line for line** | `diff` the pre-wave `(void)X::instance();` sequence against the post-wave `ctx.adoptX(...)` sequence; positions must match one to one |
| C7 | **No `adopt*()` outside the root** | grep; linter rule `arch-session-adopt-site` |
| C8 | **No module ctor reaches an unadopted slot** | the startup abort is the runtime half; the read half is the spec-0001 out-edge table for the adopted class |
| C9 | **ProjectModel's ctor closure still names nothing new** | `grep -rn "SessionContext" ProjectModel.cpp Project/ main.cpp CLI.cpp` — the standing re-proof rule from `0001/tasks.md:171-178` |
| C10 | **Reverse-order teardown** | `shutdown()` releases slots in exactly the reverse of the adoption sequence; read-back diff |
| C11 | **INV-6: engine before objects** | `~ModuleManager` (engine) at `main.cpp:182` precedes the `shutdown()` call site |

C6 is the one that keeps the diff reviewable: a wave's `ModuleManager.cpp` diff must be exactly
one line changed, in place.

## Wave set — which modules first, and why

Chosen from the census (site count = blast radius) crossed with construction risk. One wave per
build-and-launch cycle; no two ordering-sensitive changes in one build (spec 0001's rule).

| Wave | Module | Sites | Why here |
|------|--------|-------|----------|
| **A** | `DataModel::NotificationCenter` | 28 | Lowest-risk of the eight and the mechanism proof. Pinned entry 5, so almost nothing precedes it; its ctor reaches no other singleton (`QObject(nullptr)` + `moveToThread(qApp->thread())` + `QSettings`); no hotpath role. If the three-edit flip, the null assert, the shutdown ordering, and the proof format do not work here, they work nowhere — and the cost of finding out is 28 sites. |
| **B1** | `Console::Handler` | 31 | Second-lowest count; exercises INV-3 (it is half of the message-handler precondition) and the "no warnings after shutdown" ordering. Its ctor pulls `CommonFonts`, an app-global that is not adopted — proving the session/application split survives ownership. |
| **B2** | `DataModel::FrameParser` | 19 | Fewest sites of all, but deliberately *not* first: its ctor calls `engineForSource(0)`, and Lua engine construction reaches `FrameBuilder::instance()` (`LuaScriptEngine.cpp:167`). That is the first adoption whose ctor edge becomes a live assertion — worth doing on a small blast radius, and worth doing *after* the mechanism is proven. |
| **C1** | `AppState` | 139 | The pinned pair, first half. Its ctor calls `ProjectModel::instance()` through `deriveFrameConfig()`, so on a ProjectFile machine this is the wave that proves adoption survives the settings-conditional edge. Requires M2-T3 (the `main.cpp:155` reach) to have landed. |
| **C2** | `DataModel::ProjectModel` | 205 | Highest-risk single module in the tree: the protected ctor closure (`newJsonFile`, `watchProjectFile`, `scheduleAutoSave`, `ControlScript::setCode`) that has already produced one shipped startup abort. Own wave, own proof, mandatory three-mode launch, and the standing re-proof rule applies to it by name. |
| **D1** | `DataModel::FrameBuilder` | 56 | Hotpath head. Its ctor pre-allocates the frame slot pool and (commercial) reaches `LemonSqueezy` and `qApp::aboutToQuit`. First wave where `--benchmark-hotpath` is a blocking gate. |
| **D2** | `IO::ConnectionManager` | 213 | Owns 3 (GPL) / 10 (commercial) driver `unique_ptr`s whose destructors join threads and call `libusb_exit`/`hid_exit`. This is the wave where the teardown win is proven or disproven: `~ConnectionManager` moves from `__cxa_finalize` to a live-`qApp` point, which is instance 3 of the crash class, fixed structurally. Interacts with `main.cpp:184`. |
| **D3** | `UI::Dashboard` | 84 | Last in the pinned order, last adopted — its ctor touches five core modules plus two players and `TimerEvents`, so every other session module must already be adopted for its edges to resolve. Also the wave that proves the `Dashboard.h:586-602` inline helpers (one frozen static per including TU) survive INV-4 untouched. |

Rejected orderings, named so they are not re-proposed:

- **ProjectModel first, because R7 wants it most.** Refused: highest site count *and* the
  protected ctor closure. A mechanism bug there is a startup abort on every ProjectFile machine,
  and the mechanism has not been proven yet at that point.
- **All eight in one wave, because the edits are mechanical.** Refused for the same reason spec
  0001 refused the flag day, and because C6's one-line-per-wave diff is what makes each wave
  reviewable.
- **Hotpath trio first, to front-load the benchmark risk.** Refused: the benchmark entry point
  itself does not run the composition root (see pre-root reaches), so the trio cannot be measured
  safely until M2-T3 lands, and M2-T3 is easier to get right once the mechanism exists.

## M2 versus M3 — what "two contexts" actually needs

**M2 makes a second context *ownable*. It does not make it *reachable*. That is M3.**

After M2, `SessionContext ctx2; ctx2.adoptProjectModel(std::make_unique<ProjectModel>());`
compiles and runs — the ctors are already `private` with copy/move deleted and will become
accessible to the context, so a second real instance can exist and be owned. What a second
context still cannot do:

1. **The ~1,100 frozen static caches bind to whichever context served the first call.**
   `X::instance()` resolves through `SessionContext::current()`, which is a process-global
   function-local static. Any code that is not injected — which is all of it except three pilot
   classes — is permanently bound to context 0 after its first call. A second context's objects
   are unreachable through them.
2. **`current()` has no scoping mechanism.** Making it scoped (explicit passing, an RAII
   current-scope, or thread affinity) is the M3 design question, and it has a real answer to
   pick, not a mechanical edit.
3. **The 66 QML context properties name context 0's objects.** A second context has no QML
   surface at all until the context becomes bindable (the `QObject` question deferred from M1).
4. **Class-level state inside the eight may bleed.** File-statics and function-local statics
   inside those classes are shared by every instance regardless of ownership. M2-T4 censuses
   this; the result is an input to M3's plan, not something M2 fixes.

So the roadmap's R4 acceptance — *two independent contexts in one test process with no state
bleed* — remains **unmet after M2**, and the handoff must say so in those words. What M2 turns it
from is "structurally impossible" into "one scoping decision and one call-site sweep away".

**What M2 does hand R7** (and this is the whole gate): the eight session subsystems are owned by
one object, enumerable through one injected handle, with a defined lifecycle (`adopt` -> live ->
`shutdown`) and a deterministic teardown. `MirrorSession` takes `SessionContext&`, reads the
session's project document and dashboard-facing state through it, snapshots and restores that
*state* on attach/detach, and never touches object identity (INV-4). That is exactly 0040's
"one dashboard, swapped feed", and it needs nothing from M3.

## Address-lifetime audit — M2-T5 results

Reproduced on 2026-07-25 against the working tree. Two number sets are given where they differ:
the checked-in baseline (`scripts/singleton-census.json`, which is what the gate compares) and the
live tree, which several parallel specs have moved since the baseline was taken. The counts are
evidence for one claim — **every one of these families holds an address, not a lookup, so INV-4 is
what keeps them valid through M2 with zero edits.**

| Family | Baseline | Live tree | Why INV-4 keeps it valid |
|--------|----------|-----------|--------------------------|
| Function-local static caches (`static auto& x = X::instance();`) | 1,097 | 1,102 lines match, of which 12 are the `static const auto*` pointer form | Each resolves once on first call and holds the reference forever. The slot is assigned once and never reseated, so the referent outlives every reader up to `shutdown()`. |
| Constructor init-list captures (`m_x(X::instance())`) | 92 | 92 | Same argument, one level earlier: the capture happens after adoption (all captors are constructed after the pinned order) and the address never moves. |
| Retained connect addresses (`&X::instance()` as an argument) | 172 (plan text) | 174 occurrences | `QMetaObject::Connection` stores sender/receiver pointers. Qt drops the connection on `QObject` destruction, which now happens at `shutdown()` instead of `__cxa_finalize` — strictly earlier and strictly better ordered. |
| QML context properties | 66 (48 core + 18 commercial, plan text) | 63 (45 core + 18 commercial) | The engine holds raw pointers for its own lifetime. INV-6 makes `~ModuleManager` (and therefore `~QQmlApplicationEngine`) run *before* `shutdown()`, so no property can outlive its object. |
| Header inline helpers | 3 | 3 | `Dashboard.h:586-602`: `GET_GROUP` (static at `:588`), `GET_DATASET` (`:594`), `VALIDATE_WIDGET` (`:600`). Each is one shared static per program, bound on first call. Wave D3 proves INV-4 by leaving `Dashboard.h` out of its diff entirely. |
| Script-engine bridge pointers | 1 + 6 call sites | same | `NotificationCenter.cpp:623` takes `&instance()`, `:624` sets `QQmlEngine::CppOwnership`, `:627` wraps it with `newQObject`, `:628` publishes it as `__nc`. `installScriptApi(QJSEngine*)` is called from `Painter.cpp:158`, `ScriptApiCall.cpp:577,604`, `FrameBuilder.cpp:2500`, `LuaScriptEngine.cpp:184`, `DatasetTransformEditor.cpp:635,716`. No engine outlives the session today, and nothing in M2 may make one do so. |

The live-tree drift (census total 1,573 -> 1,583) is entirely from parallel specs
(`IO/ConnectionManager.cpp`, `IO/Drivers/{Modbus,UART}.cpp`, `Misc/ModuleManager.cpp`,
`Misc/Problems/ExtensionCheckers.cpp`, `UI/WidgetExtensions.cpp`). The scaffolding pass's own net
contribution is **-1** (`main.cpp`, from the M2-T3 relocation); re-baselining is deliberately left
to whoever lands the parallel work, because accepting now would bake in someone else's unfinished
diff.

**INV-6, verified by read-back of `app/src/main.cpp`:**

```
180  status = app.exec();
181  }                                            <- ~ModuleManager: the QML engine dies here
183  IO::ConnectionManager::instance().shutdownDrivers();
185  qInstallMessageHandler(nullptr);
186  SessionContext::current().shutdown();
188  Platform::AppPlatform::releaseAdjustedArgv(argc, argv);
189  return status;
```

Engine before objects (INV-6); the `:183` `ConnectionManager` use precedes the release that would
otherwise make it a null-slot fatal; the message handler is uninstalled before the console and
notification slots can be released, so no warning can reach a released slot. The post-routine at
`ModuleManager.cpp:607` still uninstalls the handler a second time inside `~QCoreApplication`, which
is now redundant rather than load-bearing.

## Tradeoffs

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Ownership holder | `unique_ptr` slots / `optional<T>` members / raw `new` + manual delete | **`unique_ptr` slots** — heap address stable for INV-4, single-assignment enforceable, reverse-order release trivial; `optional<T>` in-place would put the objects in the context's storage and make the address depend on the context's own lifetime |
| Where construction lives | context ctor / context `init()` method / **the root, line for line** | **The root** — keeps the proof's subject in one readable sequence, keeps the diff one line per wave, and avoids the Meyers-guard recursion that a constructing ctor guarantees |
| Un-adopted reach | fail-fast / lazy fallback / silent null | **Fail-fast** — a fallback re-creates unpinned lazy construction and makes the executable proof unfirable |
| Adopt API | typed `adoptX()` per module / templated `adopt<T>()` | **Typed** — eight greppable symbols answer spec 0001's "grep-hostile lookup" objection directly |
| Rollout | one wave per module / one wave for the eight | **One per module** — spec 0001's build-gate rule, and C6 stays a one-line diff |
| Shutdown point | `~ModuleManager` / `onQuit` / **last statement of `main`** | **Last statement of `main`** — after the engine dies and after `shutdown Drivers()` at `:184`, while `qApp` lives; `onQuit` keeps quiescing, not destroying |
| Ctor visibility | public / `friend SessionContext` | **`friend`** — keeps the ctors out of general reach while letting the context build them; copy/move stay deleted |
| Census expectation | claim a reduction / state it stays flat | **State it stays flat** — adoption does not delete a single `instance()` call site; the trend-down is M3's sweep and claiming it here would be false |

## Risks

- **Meyers-guard recursion.** The one failure mode that aborts at startup. Mitigation: empty
  context ctor (invariant + linter rule C7/M2-T6), publication as the first statement of the
  pinned sequence, and the abort itself is loud and names the module.
- **Pre-root reach becomes a fatal.** Three known sites, one of which is the CI benchmark gate.
  Mitigation: M2-T3 lands first and blocks waves C and D.
- **A wave surfaces a latent out-of-order ctor edge.** This is the mechanism working as intended,
  but it turns a silent lazy construction into a startup abort mid-wave. Mitigation: one wave per
  build cycle, three-mode maintainer launch per wave, and the spec-0001 out-edge table read before
  each adoption.
- **Earlier destruction breaks something that assumed process-lifetime objects.** Mitigation:
  M2-T5's address-lifetime audit, the `qInstallMessageHandler(nullptr)` reordering, and the
  explicit "nothing after `shutdown()`" rule.
- **Hotpath regression from heap placement.** `.bss` to heap changes locality on classes the frame
  path dereferences constantly. Mitigation: `--benchmark-hotpath` blocking on waves C and D, run
  and recorded on A and B, with the nine-tier table compared rather than the headline number.
- **Cross-spec collision in `instantiateCoreModules()`.** Specs 0033 and 0035 added entries during
  0039 M1; spec 0040 states in its plan that the function "is not edited" by its own diff and
  constructs `MirrorPublisher`/`MirrorSession` after it. Mitigation: each wave rebases on the
  current function and re-runs C6; 0040's plan note is accurate about its own diff and needs no
  change, but its T9+ tasks should read this file before starting.
- **Accidental second instance.** Making the ctors reachable invites one. Mitigation: `friend`
  rather than public, copy/move stay deleted, `adopt*()` asserts an empty slot (INV-5).
- **Scope creep into the other 23 pinned modules.** The exports, players, licensing, and AI
  singletons all look adoptable. Mitigation: the eight are the scope; the crash-class exposure of
  the un-adopted family is stated as *remaining*, not fixed.

## Tasks

Conventions as in [`tasks.md`](./tasks.md): one task = one reviewable diff; **Verify** is how the
unit is confirmed; waves are one build-and-launch cycle each and may not be batched.

### M2-T1 — Ownership scaffolding in `SessionContext`

- **Files:** `app/src/SessionContext.h`, `app/src/SessionContext.cpp`
- **Does:** Eight `std::unique_ptr<T>` slots, eight `adopt*()` methods (assert empty slot, assert
  non-null argument), and `[[nodiscard]] bool sealed() const`. Accessors keep forwarding to
  `X::instance()` for now; the ctor and dtor stay empty. Nothing is adopted yet, so behavior is
  unchanged.
- **Verify:** `code-verify --check` clean; read-back confirms the ctor body is still empty and the
  only non-slot member is the session id; `grep` shows no `adopt` call site anywhere yet.
- **Deps:** none
- [x] done

### M2-T2 — Deterministic teardown skeleton

- **Files:** `app/src/SessionContext.h`, `app/src/SessionContext.cpp`, `app/src/main.cpp`
- **Does:** `void shutdown()` releasing slots in reverse adoption order (no-op while all are
  null). Call site in `main.cpp` after `ConnectionManager::shutdownDrivers()` (`:184`) and before
  `return status`, preceded by `qInstallMessageHandler(nullptr)`. Records INV-6 (engine dies
  first) in the file's header comment.
- **Verify:** read-back of the release order against the adoption order; `main.cpp` diff is three
  lines; app behavior unchanged (nothing is owned yet).
- **Deps:** M2-T1
- [x] done

### M2-T3 — Pre-root reach audit and relocation

- **Files:** `app/src/main.cpp`, `app/src/Misc/CLI.cpp`, `app/src/Misc/ModuleManager.{h,cpp}`,
  `app/src/Benchmark/*`
- **Does:** Relocate `AppState::instance()` at `main.cpp:155` to after `bootstrapModuleManager`
  (or route the ephemeral-session flag through CLI state). Make the pinned sequence callable
  outside `initializeQmlInterface` and call it from the `--benchmark-hotpath` entry point, so the
  benchmark runs against a bootstrapped context and can call `shutdown()` on the way out. Record
  the full pre-root reach list in `ctor-proof-m2.md`.
- **Verify:** `--benchmark-hotpath` runs and reports the nine tiers unchanged (maintainer);
  `grep` shows no `::instance()` on a session class before the pinned sequence on any entry path;
  `dumpApiSchema` path documented as touching no session class.
- **Deps:** M2-T1. **Blocks waves C and D.**
- [x] done

### M2-T4 — Second-instance bleed census

- **Files:** `doc/claude/specs/0039-session-context/m2-bleed-census.md`
- **Does:** For each of the eight, list file-static and function-local static state, `qApp` hooks
  (event filters, `aboutToQuit` connections, post routines), and any registration into a global
  registry — i.e. everything a second instance would share or double-register. Read-only.
- **Verify:** every class has an entry; each finding names file:line; the summary states plainly
  whether a second instance is safe today, per class.
- **Deps:** none (can run in parallel with T1-T3)
- [x] done

### M2-T5 — Address-lifetime audit and INV-4/INV-5/INV-6

- **Files:** `doc/claude/specs/0039-session-context/m2-plan.md` (this file, results section)
- **Does:** Confirm and record the four frozen-address families with counts (static caches,
  ctor captures, retained connect addresses, QML context properties), the `Dashboard.h:586-602`
  inline helpers, the `NotificationCenter` `newQObject`/`CppOwnership` exposure, and the
  `~ModuleManager`-before-`shutdown()` ordering.
- **Verify:** counts reproduced by grep and cross-checked against `singleton-census.json`; each
  family has an explicit one-line statement of why INV-4 keeps it valid.
- **Deps:** none
- [x] done

### M2-T6 — Linter rules for the ownership invariants

- **Files:** `scripts/code_verify_rules.py`, `scripts/code-verify.py`
- **Does:** `arch-context-ctor-nonempty` (error: a statement in `SessionContext`'s ctor or dtor
  body) and `arch-session-adopt-site` (error: an `adopt*()` call outside
  `Misc/ModuleManager.cpp`). Both blocking, because both are the crash-class guards; document
  them next to `arch-session-context-bypass`.
- **Verify:** scratchpad snippets trip each rule and nothing else; repo-wide error count is
  unchanged on the clean tree.
- **Deps:** M2-T1
- [x] done

### M2-T7 — Wave A: adopt `DataModel::NotificationCenter`

- **Files:** `app/src/SessionContext.cpp`, `app/src/Misc/ModuleManager.cpp`,
  `app/src/DataModel/NotificationCenter.cpp`, `ctor-proof-m2.md`
- **Does:** The three-edit flip. `ModuleManager.cpp` diff is exactly one line, in place.
- **Verify:** C1-C11 recorded as the "Wave A" section of `ctor-proof-m2.md`; `code-verify --check`
  clean; **maintainer**: launch in ProjectFile, QuickPlot, ConsoleOnly; **maintainer**:
  `--benchmark-hotpath` recorded as the M2 baseline (not yet a blocking gate).
- **Deps:** M2-T1, M2-T2, M2-T6
- [x] done

### M2-T8 — Wave B1: adopt `Console::Handler`

- **Files:** as above + `app/src/Console/Handler.cpp`
- **Does:** Three-edit flip. Confirms INV-3 still holds with an owned handler and that no warning
  can reach a released slot.
- **Verify:** C1-C11 (Wave B1); a deliberate `qWarning()` after the shutdown point is proven
  impossible by reading `main.cpp` order; three-mode launch (maintainer).
- **Deps:** M2-T7
- [x] done

### M2-T9 — Wave B2: adopt `DataModel::FrameParser`

- **Files:** as above + `app/src/DataModel/Scripting/FrameParser.cpp`
- **Does:** Three-edit flip. First adoption whose ctor edge (`engineForSource(0)` ->
  `LuaScriptEngine.cpp:167` -> `FrameBuilder::instance()`) is now an assertion against an
  earlier-pinned module.
- **Verify:** C1-C11 (Wave B2), with C8 explicitly naming the FrameBuilder edge; three-mode launch
  plus a project with a Lua parser (maintainer); `--benchmark-hotpath` recorded.
- **Deps:** M2-T8
- [x] done

### M2-T10 — Wave C1: adopt `AppState`

- **Files:** as above + `app/src/AppState.cpp`
- **Does:** Three-edit flip. `deriveFrameConfig()`'s ProjectFile branch now asserts that
  `ProjectModel` was adopted first — the spec-0001 anchor, executable.
- **Verify:** C1-C11 (Wave C1); **launch on a machine whose saved `operation_mode` is ProjectFile**
  (the branch that only fires there); `--benchmark-hotpath` **blocking gate** from here on.
- **Deps:** M2-T3, M2-T9
- [x] done

### M2-T11 — Wave C2: adopt `DataModel::ProjectModel`

- **Files:** as above + `app/src/DataModel/ProjectModel.cpp`
- **Does:** Three-edit flip on the highest-risk module. No edit inside the protected ctor closure.
- **Verify:** C1-C11 (Wave C2) with C9 run verbatim (`newJsonFile`, `watchProjectFile`,
  `scheduleAutoSave`, `ControlScript::setCode` name nothing new); three-mode launch **plus**
  project open, edit, autosave, and restore-last-project (maintainer); benchmark gate.
- **Deps:** M2-T10
- [x] done

### M2-T12 — Wave D1: adopt `DataModel::FrameBuilder`

- **Files:** as above + `app/src/DataModel/FrameBuilder.cpp`
- **Does:** Three-edit flip. Slot-pool allocation and the commercial `LemonSqueezy` ctor edge move
  under the assertion.
- **Verify:** C1-C11 (Wave D1); `--benchmark-hotpath` **all nine tiers** compared against the
  Wave A baseline, not just the headline number; three-mode launch; GPL and commercial builds.
- **Deps:** M2-T11
- [x] done

### M2-T13 — Wave D2: adopt `IO::ConnectionManager`

- **Files:** as above + `app/src/IO/ConnectionManager.cpp`
- **Does:** Three-edit flip. `~ConnectionManager` (and every driver dtor it owns: thread joins,
  `libusb_exit`, `hid_exit`) moves from `__cxa_finalize` to the live-`qApp` shutdown point.
- **Verify:** C1-C11 (Wave D2), C10 emphasised; **maintainer**: connect and disconnect a real
  device, then quit via window close, Cmd-Q, and Dock-quit on macOS — the three paths that
  produced the 2026-06 crash; `--benchmark-hotpath`; USB/HID driver quit with a device attached.
- **Deps:** M2-T12
- [x] done

### M2-T14 — Wave D3: adopt `UI::Dashboard`

- **Files:** as above + `app/src/UI/Dashboard.cpp`
- **Does:** Three-edit flip on the last pinned entry. No change to `Dashboard.h`'s three inline
  helpers — proving INV-4 by leaving them alone is the point.
- **Verify:** C1-C11 (Wave D3); `Dashboard.h` is **not** in the diff; `--benchmark-hotpath` nine
  tiers; three-mode launch with a live dashboard and a replay.
- **Deps:** M2-T13
- [x] done — the header gains only the `friend` + forward declaration every wave needed; the three
  inline helpers and every other declaration are untouched. Maintainer gates open.

### M2-T15 — Documentation: ownership, lifecycle, teardown

- **Files:** `doc/claude/architecture/startup.md`, `CLAUDE.md`, `doc/claude/directory-map.md`
- **Does:** The "Session Context" section M1's T12 never wrote, now covering M2: the
  session/application split, `adopt` -> live -> `shutdown`, INV-4/INV-5/INV-6, the empty-ctor
  rule and why (guard recursion), the shutdown point and why (crash class), and the negative rule
  ("do not call `current()` from a method body"). CLAUDE.md gains at most four lines under
  "Startup & Composition Root".
- **Verify:** `documentation-verify.py` clean; M1's AC6 closes with it; the teardown paragraph
  names the crash class and the three historical instances.
- **Deps:** M2-T14
- [x] done — 2026-07-25; see T12's status note in `tasks.md` (one section covers both).
  Deviation: the teardown paragraph names the `__cxa_finalize` crash class but does not
  enumerate the three historical instances individually.

### M2-T16 — Close AC3: split `ProtoImporter`'s generator into its own TU

- **Files:** `app/src/DataModel/Importers/ProtoProjectGenerator.{h,cpp}` (verbatim move of
  `projectFromMessages` / `projectFromProtoFile`), `ProtoImporter.{h,cpp}`,
  `app/tests/CMakeLists.txt`, `app/CMakeLists.txt`
- **Does:** The honest fix for AC3. Ownership does not cut the link set; a TU split does. Moving
  the pure generator into a TU that reaches no `Misc::Utilities`, no `ProjectModel`, and no
  `SerialStudio` lets `tst_proto_importer` link against the generator alone. Verbatim body move
  (`tu-cutter.py` discipline), no logic edit.
- **Verify:** read-back diff shows the moved bodies are character-identical; `ctest` green on
  `tst_proto_importer` (maintainer); AC3 checked off in `spec.md` with the mechanism named
  accurately (TU split, enabled by but not caused by ownership).
- **Deps:** M2-T14
- [ ] done
- **Status (2026-07-25): REFUTED as designed — do not execute without a design change.**
  Two ground-truth facts break the premise:
  1. The generation path is not `SerialStudio`-free: `projectFromMessages` / `buildGroups` /
     `selectGroupWidget` call `SerialStudio::groupWidgetId` (`ProtoImporter.cpp:1123,1223,
     1243,1245`, defined in `SerialStudio.cpp:520`), and the `detail::` proto lexer/parser
     (`ProtoImporter.cpp:44-812`) is shared with the UI half (`showPreview` constructs
     `detail::Parser` at `:963`), so a clean cut also needs the parser hoisted into a shared
     internal header or third TU.
  2. Even a perfect split leaves the suite unlinkable: the test builds a stack
     `SessionContext`, and `~SessionContext` destroys eight `unique_ptr` members — that needs
     the vtables of the six modules with implicit dtors (each lives in its `moc_*.cpp`) plus
     the out-of-line `~ConnectionManager` (`ConnectionManager.h:145`) and
     `~NotificationCenter` (`NotificationCenter.h:87`) and their closures. The wall is the
     context's dtor closure, not the accessors the CMake note blamed.
  Viable paths (maintainer pick): (a) a `SessionContext` test-double seam (interface or
  link-time substitutable teardown), (b) linking the eight module TUs into the suite
  (defeats the point), or (c) dropping the stack-context from the test and constructing the
  importer against `SessionContext::current()` — refuted too, `current()` lives in
  `SessionContext.cpp`. AC3 stays gated; `app/tests/CMakeLists.txt` comment updated to match.

### M2-T17 — Consolidated proof and benchmark sweep

- **Files:** `doc/claude/specs/0039-session-context/ctor-proof-m2.md`
- **Does:** Roll the eight per-wave sections into one verdict: the proof is **re-derived**, with
  the adoption sequence, the reverse-teardown sequence, and the nine-tier benchmark table before
  and after the whole milestone.
- **Verify:** every wave section present; the verdict states re-derived (not preserved) in those
  words; benchmark deltas within noise on all nine tiers.
- **Deps:** M2-T14
- [x] done — all eight wave sections present, the verdict is stated as **re-derived, not preserved**,
  and the eight-wave C1-C11 table plus the maintainer launch-gate checklist are consolidated in one
  section. The nine-tier benchmark comparison is the one part left open: it is a maintainer run, and
  the checklist carries it.

### M2-T18 — Handoff

- **Files:** `spec.md` (M2 status), this file (status)
- **Does:** Record: census total unchanged and why (adoption deletes no call site); the teardown
  class fixed for the eight and **still open** for the export/AI/watchdog family; R7's gate
  satisfied with the specific handshake ("owned set, one handle, deterministic detach; identity
  never swapped"); and the roadmap's R4 criterion **not met**, with the four concrete reasons and
  M3 named as the carrier.
- **Verify:** the "Definition of Done" below is fully checked or the exception is stated.
- **Deps:** M2-T15, M2-T16, M2-T17
- [ ] done

## Definition of Done

- [x] All eight session subsystems are owned by `SessionContext`; `instance()` on each is a
      one-line forward; `instantiateCoreModules()` holds the same pinned sequence with one
      `adopt*()` per line, in the same order.
- [x] `SessionContext`'s constructor and destructor are empty; the linter blocks a statement in
      either, and blocks an `adopt*()` call outside the composition root.
- [x] `shutdown()` releases in reverse pinned order at the last statement of `main`, with the
      message handler uninstalled first and `shutdownDrivers()` still preceding it.
- [x] `ctor-proof-m2.md` records C1-C11 per wave and a **re-derived** verdict.
- [ ] `--benchmark-hotpath` nine tiers within noise versus the Wave A baseline, recorded.
- [ ] Maintainer launch in ProjectFile, QuickPlot, ConsoleOnly after every wave; macOS quit via
      window close, Cmd-Q, and Dock-quit after Wave D2.
- [ ] `code-verify --check` clean, blocking-error count unchanged; `--singleton-census --check`
      passes with the total **unchanged** and that fact stated rather than spun.
      *Half done and stated rather than spun:* `code-verify --check` reports **0 errors** on the
      final tree and neither arch rule fires, so the blocking-error count is unchanged. The census
      `--check` **fails on this tree**, and not because of M2: the eight waves *remove* two sites
      net (the root's `(void)UI::Dashboard::instance();` and the last forwarding accessor body), while
      parallel specs added 13 (`total 1573 -> 1586`, `static-cache 1097 -> 1122`) across extension
      bucketing, project-editor commit/wiring, and the diagnostics checkers. Re-baselining with
      `--accept` would absorb another agent's growth into this milestone's ledger, so it was not run.
- [ ] `startup.md` documents ownership, lifecycle, teardown, and the empty-ctor rule; M1's AC6
      closes.
- [ ] AC3 closed by the TU split, with the mechanism described accurately.
- [ ] The handoff states, in these words, that the roadmap's "two independent contexts with no
      state bleed" criterion is **not met by M2** and that M3 carries it.
