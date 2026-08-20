---
spec: 0039-session-context
phase: plan
status: draft        # draft -> approved (gate before /ss-tasks)
updated: 2026-07-25
---

# Plan 0039 — Session context over global singletons

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Read the relevant `doc/claude/` sub-docs and the *actual code*
> before writing this — a plan grounded in a stale mental model is worse than no plan.
> Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

Add `SessionContext` (`app/src/SessionContext.{h,cpp}`, global namespace, alongside
`AppState`): a plain, non-copyable class whose accessors *forward* to the existing
`instance()` singletons and whose constructor does nothing and holds nothing. Because it
owns no state and constructs no module, it adds zero edges to the spec-0001 constructor
graph — that is the property the whole v1 design is built around, and it is what keeps the
pinned order untouched. `Misc::ModuleManager` pins its construction on one new line in
`setupCrossModuleConnections()`, placed *after* the call to `instantiateCoreModules()` and
*before* the first `setupExternalConnections()`; the pinned-order body itself is not
edited. Three pilot classes (`Misc::BackupManager`, `DataModel::ProtoImporter`,
`DataModel::DBCImporter`) then take `SessionContext&` as a constructor parameter, keep
their `instance()` accessors as the composition/QML entry point, and stop reaching for
globals in their bodies. The linter gains a regression rule for converted classes and a
census mode that counts what the current `arch-singleton-instance` advisory misses — the
969 `static auto&` cache sites — against a checked-in baseline. Nothing on the frame path
is touched.

## Affected subsystems & files

| File | Change |
|------|--------|
| `app/src/SessionContext.h` | New. `class SessionContext`, virtual forwarding accessors, `static SessionContext& current()`. |
| `app/src/SessionContext.cpp` | New. Accessor bodies (one-line forwards), `current()` process-default instance. |
| `app/CMakeLists.txt` | Add `src/SessionContext.cpp` to the sources list (near `src/AppState.cpp`, line ~352) and `src/SessionContext.h` to headers (near line ~503). Not commercial-gated. |
| `app/src/Misc/ModuleManager.cpp` | One line in `setupCrossModuleConnections()` after `instantiateCoreModules()` (line 661) pinning context construction. `instantiateCoreModules()` (lines 619-654) is **not** edited. |
| `app/src/Misc/BackupManager.{h,cpp}` | Pilot 1. `explicit BackupManager(SessionContext&)`; `m_ctx` member; `m_projectModel` capture and `setupExternalConnections()` reach both come from the context. |
| `app/src/DataModel/Importers/ProtoImporter.{h,cpp}` | Pilot 2. `explicit ProtoImporter(SessionContext&)`; extract the pure generator behind a public `[[nodiscard]] QJsonObject projectFromProtoFile(const QString&)` that touches no session state; `confirmImport()` reaches the project model via the context. |
| `app/src/DataModel/Importers/DBCImporter.{h,cpp}` | Pilot 3 (commercial). Same shape; `confirmImport()` (DBCImporter.cpp:209) uses the context; pure `projectFromMessages(...)` exposed for the unit tier. |
| `scripts/code_verify_rules.py` | Add `SessionContext.cpp` to `_SINGLETON_ROOT_FILES`; new advisory rule `arch-session-context-bypass`; census classifier helper. |
| `scripts/code-verify.py` | Register the new advisory kind; document both rules in the report prose; add the `--singleton-census` mode. |
| `scripts/singleton-census.json` | New. Checked-in baseline: per-class and per-form counts. |
| `scripts/sanitize-commit.py` | Run the census check in the pipeline (after `code-verify --check`). |
| `doc/claude/architecture/startup.md` | New section "Session Context" — the session/application split, the publication point, the injection convention. |
| `CLAUDE.md` | Three lines under "Startup & Composition Root" pointing at the section and stating the convention for new classes. |
| `doc/claude/directory-map.md` | One-line entry for `SessionContext.{h,cpp}`. |
| `app/tests/tst_proto_importer.cpp` | New. Qt Test case per spec 0032's `app/tests/` layout: constructs a context + importer on the stack and asserts the generated project. |

## Architecture & data flow

**Shape of the type.** `SessionContext` is a plain class (no `Q_OBJECT`, no QML exposure —
per the spec non-goal that nothing binds to it prematurely), copy and move deleted, with a
virtual destructor and virtual accessors:

- `appState()`, `projectModel()`, `frameBuilder()`, `frameParser()`, `connectionManager()`,
  `dashboard()`, `console()`, `notifications()` — each `[[nodiscard]] virtual X& f() const`.
- `sessionId()` — `[[nodiscard]] int` — 0 for the process-default context; distinct values
  exist only so M3's plurality has somewhere to land and so a test can assert which context
  a pilot holds.

Accessors are virtual for exactly one reason: the R3 unit tier can subclass the context and
override a single accessor once an interface exists to return (M2 work). They are not
virtual for polymorphism the hotpath would pay for — no hotpath call site exists, and the
`perf-virtual-hotpath` advisory does not apply off the frame path.

**Why forwarding, not capture.** Two shapes were live: hold references captured at
construction, or forward per call. Forwarding wins because it makes the ctor-edge argument
airtight rather than merely likely — a context that captures references must be constructed
after every module it names, and any future reordering silently re-introduces a
construction edge. A forwarding context can be constructed at any point in startup and
still cannot construct anything, because `X::instance()` is only evaluated when a *caller*
asks. The cost is that v1 keeps the Meyers guard on every access, which is what the code
does today anyway, on paths that are not hot.

**Publication point.** `SessionContext& SessionContext::current()` returns a
function-local static process-default context. It is the single sanctioned global reach and
is the thing M3 replaces with a scoped/registered current. Sanctioned callers: the
composition root, and the `instance()` accessor body of an injected class
(`static ProtoImporter inst(SessionContext::current());`). Everything else takes the
context by constructor parameter. The linter enforces both halves.

**Composition root sequence** (unchanged except for one inserted line):

1. `instantiateCoreModules()` — the pinned order, byte-identical to today.
2. **`(void)SessionContext::current();`** — new. Pins *when* the context comes into being,
   after the whole pinned order has run and before any wiring.
3. The `setupExternalConnections()` sequence, then `restoreLastProject()` (INV-1).
4. Context-property registration (INV-2), message handler (INV-3), QML load.

Every pilot is first constructed strictly after step 2 — verified: `BackupManager` first
exists at `ModuleManager.cpp:677`, `ProtoImporter` at the context-property registration
(`:763`), `DBCImporter` at the commercial context-property registration (`:800`). No pilot
appears in `instantiateCoreModules()`, and none is reachable from `main.cpp` or `CLI.cpp`.

**Injection convention** (documented in `startup.md`, enforced by the new rule):

> A class that needs session state takes `SessionContext&` in its constructor and stores it
> as a reference member. Its `instance()` accessor, if it has one, passes
> `SessionContext::current()`. Application-wide services — translator, theme manager,
> fonts, icon registry, workspace manager, timer events, licensing — are **not** in the
> context and keep their existing acquisition. New classes do not call `X::instance()` for
> anything the context names.

## Hotpath & threading impact

- **Touches the hotpath?** **No.** No file on the Driver → FrameReader → FrameBuilder →
  Dashboard path is edited. `FrameBuilder`, `ConnectionManager`, `Dashboard`, `FrameReader`,
  and `CircularBuffer` keep their spec-0001 captured members and their cached flags
  unchanged. The context *names* the frame pipeline (so the type is complete and R7 has its
  eventual seam) but no per-frame code calls it in v1. This is a design constraint, not an
  outcome: a task-level check greps the diff for hotpath files and fails if any appear.
  `--benchmark-hotpath` is therefore not a gate for this spec.
- **New cross-thread signal/slot?** None. `SessionContext` has no signals and no slots; the
  pilot conversions change constructor signatures and member access only, never connection
  types. `BackupManager`'s three existing `ProjectModel` connections keep their current
  (default, same-thread) types.
- **New input to a cached hotpath flag?** None. No cached flag gains or loses an input.
- **Timestamp ownership** — untouched. No frame is created, copied, or re-stamped anywhere
  in this change.

## Data model & persistence

None. No `Frame.h` `Keys::` additions, no project-JSON change, no `widgetSettings` change,
no Sessions DB change, no writer-version bump. Pilot 2 and 3 keep producing byte-identical
project JSON — the generator split is a visibility change (private method promoted to a
public pure method), not a logic change, and the task-level verification is a read-back
that the moved body is verbatim.

## API / SDK surface

None. No handler is added, renamed, or re-registered; `CommandHandler::initializeHandlers()`
is untouched. The API handlers are among the heaviest `instance()` consumers
(`ProjectHandlerEntities.cpp` 29 sites, `DashboardHandler.cpp` 25, ...) and are deliberately
*not* pilots: they are static-method handlers whose conversion means threading a context
through a static dispatch surface, which is M2 work with a real design question attached.

## QML / UI

None. All three pilots keep their `instance()` accessors and their existing context
properties (`Cpp_Misc_BackupManager`, `Cpp_JSON_ProtoImporter`, `Cpp_JSON_DBCImporter`),
registered at the same point in startup, at the same addresses, with unchanged
`Q_PROPERTY` / `Q_INVOKABLE` surfaces. The context is not registered as a context property
in v1 (spec non-goal). No new components, no ComboBox restore-race surface.

## Preserving the spec-0001 ctor-edge proof

This is the load-bearing section; the plan is shaped around it.

**The proof is preserved, not re-derived, and here is why.** Spec 0001's proof establishes
that every module in `instantiateCoreModules()` self-initializes and connects only to
objects it itself forces, in an order where `ProjectModel` precedes `AppState` and
`Dashboard` is last. This change:

1. **Does not edit `instantiateCoreModules()`.** The pinned body (ModuleManager.cpp:619-654)
   is not touched — no entry added, removed, or reordered. The proof's subject is unchanged.
2. **Adds one object whose constructor has zero out-edges.** `SessionContext`'s constructor
   body is empty and it holds no members that could construct anything. Its accessors
   evaluate `X::instance()` only when called, and no caller exists before wiring.
3. **Inserts its construction after the entire pinned order has run.** Even if the ctor
   later grew an edge, every module it could reach is already constructed at that point, so
   no new construction edge can form at that call site.
4. **Keeps ProjectModel's ctor closure clean.** `SessionContext` is not named in
   `ProjectModel.cpp`, `newJsonFile()`, `watchProjectFile()`, `scheduleAutoSave()`, or
   `ControlScript::setCode`, and no pilot is reachable from that closure.
5. **Converts no module in the pinned order.** All three pilots are constructed after
   `instantiateCoreModules()` returns (call sites verified above), so no pinned ctor gains a
   `SessionContext&` parameter.

**The re-run is still mandatory and is a task, not an assumption.** T3 re-runs spec 0001's
own verification recipe and records the output in this spec directory as `ctor-proof.md`:

- Grep symmetry: every class named in `instantiateCoreModules()` still appears later in
  `setupCrossModuleConnections()` or was already transitively constructed, and the order
  matches the spec-0001 table entry for entry.
- INV-1: `restoreLastProject()` is still the last call in `setupCrossModuleConnections()`.
- INV-2: context-property registration still runs after wiring and before `m_engine.load`.
- INV-3: `qInstallMessageHandler` still runs after `Console::Handler` and
  `NotificationCenter` exist.
- New out-edge check: `SessionContext`'s constructor body is empty and it declares no member
  of a module type — asserted by reading the file and by a grep that the header declares no
  data member other than the session id.
- Maintainer launch in all three operation modes (the spec-0001 build gate for any change to
  the composition root), which is also AC7.

**What would break the preservation argument** — and therefore must not be done in this
spec: giving the context captured reference members; constructing it inside
`instantiateCoreModules()`; naming it anywhere reachable from ProjectModel's ctor closure;
converting a pinned module to constructor injection. Each of those moves the proof from
"unchanged" to "must be re-derived", which is M2 work.

## Linter and census design

**Rule 1 — sanction the forwarding file.** `app/src/SessionContext.cpp` joins
`_SINGLETON_ROOT_FILES` (currently `main.cpp` and `Misc/ModuleManager.cpp`). Its accessor
bodies are the forwarding bridge; flagging them would be noise, and its role is composition
root by definition.

**Rule 2 — `arch-session-context-bypass` (new, advisory).** In any file that mentions
`SessionContext&` (i.e. a converted class), flag every `::instance()` occurrence *including*
the forms the existing rule sanctions — the `static auto&` cache idiom and constructor
init-list captures — with the sole exception of the class's own `instance()` accessor body
(the composition entry, which is where `SessionContext::current()` is legitimately passed).
Rationale: those two forms are precisely what a converted class would relapse into, and the
existing `arch-singleton-instance` rule would stay silent. Advisory, matching the repo's
ratchet philosophy; the blocking-error count does not move (AC4).

**Census — `python scripts/code-verify.py --singleton-census`.** Classifies every
`::instance()` occurrence under `app/src` into six buckets and reports per class and per
bucket:

| Bucket | Today | Meaning |
|--------|-------|---------|
| root | 137 | `main.cpp` + `ModuleManager.cpp` (+ `SessionContext.cpp` after T1) |
| accessor | 63 | the class's own `X& X::instance()` definition |
| ctor-capture | 90 | `m_x(X::instance())` in an init list |
| deferred | 20 | `m_x = &X::instance();` in `setupExternalConnections()` |
| static-cache | **969** | `static auto& x = X::instance();` — the real global-state surface |
| loose | remainder | anything else; matches the existing advisory (0 today) |

Totals are compared against `scripts/singleton-census.json`. `--singleton-census --check`
fails when the **total** or the **static-cache bucket** increases; a decrease is accepted and
prints the new numbers with instructions to re-baseline via `--singleton-census --accept`.
`sanitize-commit.py` runs `--check` after `code-verify --check`. This is what makes the
roadmap's "tracked and trends down" criterion real: measured against the current advisory it
reads zero and can never trend anywhere.

## Pilot selection

Selected from the per-file census (outgoing `instance()` calls, excluding each class's own
accessor) for: non-hotpath, one or two session dependencies, a pure-logic core worth unit
testing, and no presence in the pinned order.

| Pilot | Session deps | Why this one |
|-------|--------------|--------------|
| `Misc::BackupManager` | ProjectModel (1) | Lifecycle class with a `setupExternalConnections()` body and a `Q_ASSERT(m_projectModel)` use pattern — proves injection for a wired class. GPL. Small: one deferred pointer plus one direct call, both replaced by the context reference. |
| `DataModel::ProtoImporter` | ProjectModel (1) | Always-built (not commercial), and its generation path is pure: parse `.proto` → project JSON, with the model touched only at `confirmImport()` (`ProtoImporter.cpp:1004`). This is the AC3 unit-test carrier. |
| `DataModel::DBCImporter` | ProjectModel (1) | Same shape behind `BUILD_COMMERCIAL` — proves the pattern survives the commercial boundary and that `SessionContext` needs no commercial symbol. Its decode logic is a known regression class (Motorola byte order, 2026-06), so the unit-tier reach is worth something immediately. |

Explicitly rejected as pilots: `UI::AlarmMonitor` (dashboard-adjacent, per-frame risk);
`Console::Export` and `Misc::ExtensionManager` (5 and 4 deps, network and file I/O);
every API handler (static dispatch, M2 design question); anything in the pinned order.

## Interaction with R3 (spec 0032, C++ unit tier)

R4 depends on R3 in the roadmap, and the pilot work is where the dependency pays off. Spec
0032 (authored in parallel) settles the shape: Qt Test, one `tst_<name>.cpp` per target under
`app/tests/`, wired into `ctest`. The named test is `app/tests/tst_proto_importer.cpp`:

```
SessionContext ctx;                                   // stack, no composition root
DataModel::ProtoImporter importer(ctx);               // stack, ctor now public
const auto project = importer.projectFromProtoFile(fixture);  // pure path
// assert: group count, dataset count, field -> dataset name/type mapping,
//         repeated-field handling, nested-message flattening
```

The project model accessor is never dereferenced, so no singleton is constructed and the
test runs in milliseconds without QML, without `restoreLastProject()`, and without a
`QQmlApplicationEngine`. It also adds the first `app/tests/` case that exercises *application*
logic rather than a leaf algorithm, which is the qualitative step R4 contributes to R3's tier.
That is the concrete, honest claim for this milestone: a pilot's
pure logic became reachable from a unit test, and the mechanism that made it reachable is
the injected context. A `FakeSessionContext` overriding `projectModel()` to return a stub
requires an interface to return — that is M2 work and is deliberately out of scope.

**Ordering with spec 0032:** if 0032's test target does not exist when this lands, the test
source lands anyway with its CMake registration commented and AC3 stays unchecked until
0032 lands. The pilot conversions do not block on it.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Context contents | forwarding accessors / captured references / owned members | **Forwarding** — the only shape that provably adds zero ctor edges, which is what keeps spec 0001's proof intact rather than re-derived. Ownership is M2. |
| Publication | process-default `current()` / registered pointer with fatal-if-unset / no publication (pass everywhere) | **Process-default `current()`** — no new crash class, no plumbing through `main`, and it is exactly one seam the linter can watch. Scoped/registered current is M3, where it is actually needed. |
| Accessors virtual? | virtual / non-virtual | **Virtual** — costs nothing off the hotpath and is the hook the unit tier will need at M2; making them non-virtual now means changing every pilot signature later. |
| Context as `QObject`? | `QObject` / plain class | **Plain class** — a `QObject` invites a context property and QML bindings before the semantics are settled, and nothing needs signals in v1. Revisit at M3 (open question in `spec.md`). |
| Pilot count | 1 / 3 / "convert as touched" | **3** — one wired class, one pure-logic class, one commercial class. One proves too little (no commercial boundary, no unit-test carrier); more is scope creep against a seam that has not been reviewed in use yet. |
| Census gate severity | report-only / block on increase / block on non-decrease | **Block on increase** (total and static-cache bucket) — a ratchet that never blocks is not a ratchet, and blocking on non-decrease would make every unrelated feature commit fail. |
| Where the pin goes | inside `instantiateCoreModules()` / after it in `setupCrossModuleConnections()` | **After it** — leaves the pinned-order body byte-identical, so spec 0001's proof subject is literally unchanged. |
| Pilot ctor visibility | keep private (friend the test) / make public | **Public** — a second stack instance is the whole point of AC3, `friend` declarations for test types are a worse coupling, and copy/move stay deleted so nothing accidental changes. |

## Risks & mitigations

- **The seam becomes a service locator.** `SessionContext::current()` is a global by any
  other name. Mitigation: exactly two sanctioned call sites per class (composition root, own
  accessor), enforced by `arch-session-context-bypass`; the census counts it; `startup.md`
  states the convention in the negative ("do not call `current()` from a method body").
- **Claiming more than was achieved.** The roadmap's acceptance criterion is not met by this
  spec. Mitigation: `spec.md` says so explicitly and records M1/M2/M3; the handoff summary
  must repeat it; the spec status may not be set to `done` with AC-language implying
  plurality.
- **Silent ctor-edge regression.** The preservation argument holds only while the context
  stays stateless. Mitigation: T3's recorded `ctor-proof.md`, the empty-ctor/no-member
  greps, and an explicit "what would break this" list in this plan.
- **Scope creep into a fourth, fifth, tenth class.** 1,524 sites are an inviting target.
  Mitigation: pilot list is fixed in this plan; the census gate blocks *increases*, it does
  not reward decreases; conversions beyond the pilots are a separate spec.
- **Accidental hotpath touch.** Mitigation: a task-level diff check that no file under the
  Driver → FrameReader → FrameBuilder → Dashboard path appears in the diff; if one does, stop
  and invoke `ss-hotpath`.
- **Pilot behavior drift during the generator split.** Promoting a private generator to a
  public pure method risks a subtle body edit. Mitigation: verbatim move only (the
  `tu-cutter.py` discipline — never re-type code), plus a read-back diff of the moved body.
- **Census false positives blocking unrelated commits.** A new legitimate `instance()` in an
  unconverted class would trip the gate. Mitigation: `--accept` re-baseline path documented
  in the failure message, and the gate names the offending file so the decision is one
  glance; the total may legitimately rise once and be accepted, which is the point of a
  checked-in baseline rather than a hard ceiling.
- **Commercial-build divergence.** Pilot 3 is `BUILD_COMMERCIAL`-only. Mitigation:
  `SessionContext` includes and names no commercial type; the GPL build is verified to
  compile with pilots 1 and 2 only (maintainer build gate).

## Test & verification plan

- **Unit (agent can run):** none in `tests/scripts/` (no JS surface). The linter work is
  self-tested with synthetic snippets in the scratchpad: a `Foo::instance()` and a
  `static auto& f = Foo::instance();` inside a file that mentions `SessionContext&` must both
  report; the same two in a file that does not mention it must not; the census classifier is
  checked against hand-counted numbers on three known files (`ModuleManager.cpp` 130,
  `BackupManager.cpp` 3, `DBCImporter.cpp` 2).
- **Unit (maintainer runs, after spec 0032):** `ctest --test-dir <builddir>
  --output-on-failure` (presets since removed) —
  `tst_proto_importer` per AC3.
- **Static (agent runs):** `python scripts/code-verify.py --check` on every changed C++ file;
  blocking-error count before and after the new rule must be identical;
  `python scripts/code-verify.py --singleton-census --check` against the new baseline;
  `qt-cpp-review` on the C++ diff; `python scripts/documentation-verify.py` after the doc
  task; `python scripts/sanitize-commit.py` before commit.
- **Integration (maintainer runs):** launch in ProjectFile, QuickPlot, and ConsoleOnly —
  startup, project restore, backup snapshot on edit, `.proto` import, and (commercial) `.dbc`
  import all behave as before. Existing `pytest tests/integration/` as a smoke check against
  the live app; no new pytest file (nothing user-facing changed).
- **Hotpath:** not applicable and proven so — the diff contains no hotpath file (task-level
  check). `--benchmark-hotpath` is not a gate for this spec.
