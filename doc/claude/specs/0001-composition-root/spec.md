---
spec: 0001-composition-root
title: Composition Root & De-Singleton-ization
status: done          # draft -> approved -> in-progress -> done | shelved
created: 2026-07-06
author: Alex Spataru
---

# Spec 0001 - Composition Root & De-Singleton-ization

> **Phase 1 of 4 - the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

Serial Studio's ~59 core services are Meyers singletons - lazily constructed on first
`X::instance()` call. That construction order is not defined by code; it emerges from
whichever `instance()` call happens to fire first at startup, and that first call is
**settings-dependent**. On a machine whose saved `operation_mode` is ProjectFile,
`AppState`'s constructor runs `deriveFrameConfig()`, whose ProjectFile branch calls
`ProjectModel::instance()` - so ProjectModel is constructed *inside* AppState's
constructor. On a QuickPlot machine that branch never runs, and ProjectModel is
constructed later. The startup graph a developer sees depends on which machine they
booted; any refactor validated on one machine is validating only one of the orders.

The settings-conditional edge is not merely untidy - it is a live re-entrancy hazard.
`ProjectModel`'s constructor calls `newJsonFile()`, which emits `groupsChanged` while
AppState is still mid-construction. The fenced comment at `ProjectModel.cpp:446`
("newJsonFile() emits groupsChanged while AppState is still mid-init") exists precisely
because connecting the AppState-touching lambda *before* `newJsonFile()` would re-enter
`AppState::instance()` during that class's own Meyers initialization - undefined
behavior. The maintainer already dodged this once by ordering a `connect` call. The lazy
graph is a DAG today only by accident, held together by one comment and one carefully
placed connect; it is one connect-reorder away from UB, and nothing pins it there.

Meanwhile `ModuleManager` is *already* the composition root in everything but name: it
force-instantiates modules, runs 17 ordered `setupExternalConnections()` calls, then
`restoreLastProject()`, then registers ~60 `Cpp_*` QML context properties, then loads
QML. What is missing is a code-pinned construction order and an explicit, enforced
two-phase lifecycle. This spec formalizes that so the startup graph stops depending on a
saved setting, removes the AppState<->ProjectModel hazard, and shrinks the loose
`::instance()` surface (1,852 sites across 59 singleton headers) behind a regression
guard - without regressing the 256 kHz hotpath.

## Goals

- Construction order of the core modules is defined by code, identical across QuickPlot,
  ProjectFile, and Console-only startup - never varying with the persisted operation mode.
- The AppState<->ProjectModel re-entrancy hazard is eliminated: ProjectModel is always
  constructed before AppState, so the settings-conditional edge cannot form.
- `ModuleManager` is the single, explicit composition root with a two-phase lifecycle
  (construct all modules, then wire them), and the three standing ordering invariants
  below hold by construction rather than by accident.
- Non-hotpath consumers hold their dependencies as captured members instead of scattering
  `X::instance()` calls; the loose call-site count strictly decreases, wave by wave.
- A regression guard makes any new `::instance()` call site visible in the code-verify
  report, so the surface ratchets down and cannot silently grow back.
- The 256 kHz hotpath gate is preserved or improved (a member load is cheaper than a
  Meyers-guard atomic load plus branch).

## Non-Goals

- **NOT container/framework dependency injection.** Rationale: the ~60 `Cpp_*` QML context
  properties need stable, long-lived `QObject` addresses; a container forces a ~60-parameter
  wiring explosion and a 1,852-site flag-day rewrite; QML sees no benefit from it; and the
  end state is structurally identical to plain constructor injection minus the plumbing -
  captured members can be promoted to constructor parameters mechanically later if ever
  wanted. A container buys nothing here and costs a rewrite.
- **NOT a service locator.** Rationale: it keeps the exact same global state behind a
  slower, grep-hostile lookup, adds indirection cost on a path where 256 kHz forbids it, and
  does nothing about the actual disease - unpinned lazy construction order. It would make the
  code harder to reason about while leaving the hazard in place.
- Not removing the singletons or changing the QML context-property model. The `instance()`
  accessors and the ~60 context props stay; only *how dependencies are acquired* changes.
- Not converting `FrameParser` to a captured or force-instantiated dependency for capture
  purposes: its constructor closure is project-content-dependent (see the appendix special
  case). It stays lazy/deferred.

## Target Architecture

Formalized **composition root + dependency capture**, not new machinery.

- **Composition root.** `ModuleManager` owns startup. A new private
  `instantiateCoreModules()` runs first and forces every core module into existence in one
  code-pinned topological order, so the graph no longer depends on which `instance()` fired
  first. This is the pinned order (detailed in `plan.md`); its load-bearing property is
  **ProjectModel before AppState**, which kills the settings-conditional edge.
- **Two-phase lifecycle: construct, then wire.** Phase one constructs modules
  (self-initialize, read `QSettings`, connect only to objects the module itself forces).
  Phase two runs `setupExternalConnections()` on each, in order, wiring cross-module
  signals/slots. This is exactly the construct/wire split DI would impose, expressed in the
  root we already have.
- **Dependency capture.** Instead of ad-hoc `X::instance()` at each use site, a consumer
  captures its dependency once and holds it: true leaves and near-leaves as a constructor
  init-list reference member (`-Wreorder` + zero-warnings catches init-order slips at
  build); the five core modules that can be reached before wiring (the "pentagon":
  `AppState`, `ProjectModel`, `FrameBuilder`, `ConnectionManager`, `Dashboard`) as a pointer
  captured at the top of their own `setupExternalConnections()`, with pre-wiring surfaces
  keeping direct `instance()` calls. The hotpath is converted last, benchmark-gated.
- **Ratchet.** An advisory linter rule flags new loose `::instance()` sites; the sanctioned
  surface shrinks stage by stage until it is just the composition root, the `main` entry, the
  class's own `instance()` definition, and `setupExternalConnections` bodies.

## Requirements

1. **R1** - Core-module construction order is code-defined and identical in QuickPlot,
   ProjectFile, and Console-only startup; it does not change with the saved `operation_mode`.
2. **R2** - ProjectModel is constructed before AppState in every mode, so the
   `deriveFrameConfig() -> ProjectModel::instance() -> newJsonFile() -> groupsChanged`
   re-entrancy edge can no longer form during AppState's construction.
3. **R3** - All module wiring flows through `ModuleManager` in two phases (construct, then
   `setupExternalConnections`), and the three standing invariants (INV-1..INV-3 below) hold.
4. **R4** - Non-hotpath consumers acquire their dependencies as captured members; the count
   of loose `::instance()` call sites reported by the linter strictly decreases per wave and
   never increases.
5. **R5** - A new advisory linter rule reports any new `::instance()` call site outside the
   sanctioned surface, without changing the blocking-error count.
6. **R6** - The `--benchmark-hotpath` gate is not regressed on any of its seven tiers after
   the hotpath capture stage; per-frame dependency access is a member load, not a Meyers
   guard.

## Acceptance Criteria

- [x] **AC1 (R1, R2)** - The app launches and runs correctly in all three operation modes;
  the composition root instantiates ProjectModel before AppState in each. Verified by the
  maintainer launching each mode after the order is pinned, and by grep symmetry between the
  pinned-order body and what the root already constructs (recipe in `plan.md`).
- [x] **AC2 (R3)** - INV-1/INV-2/INV-3 hold: `restoreLastProject()` runs after every
  `setupExternalConnections()`; context properties are registered after wiring and before QML
  load; the Qt message handler is installed only after `Console::Handler` and
  `NotificationCenter` exist. Verified by reading the composition-root sequence and the grep
  checks in `plan.md`.
- [x] **AC3 (R4)** - Per capture wave: `grep -c "X::instance()"` in each converted `.cpp` is
  zero outside the constructor init list (leaves) or the pre-wiring surface (pentagon); the
  header gained exactly one member per dependency; the advisory `::instance()` count strictly
  decreases.
- [x] **AC4 (R5)** - Running `python3 scripts/code-verify.py app/src` before and after the
  new rule lands yields an identical blocking-error count; the advisory report gains only the
  new kind. A synthetic `Foo::instance()` snippet fires the rule; each sanctioned pattern does
  not.
- [x] **AC5 (R6)** - `--benchmark-hotpath` passes all seven tiers after the hotpath stage,
  with no regression versus the pre-stage baseline.

## Constraints & Invariants

The three standing ordering invariants the composition root must preserve:

- **INV-1** - Every module's `setupExternalConnections()` runs **before**
  `restoreLastProject()`. Restoring a project drives wiring that assumes all modules are
  already connected.
- **INV-2** - The ~60 `Cpp_*` QML context properties are registered **after** all wiring is
  complete and **before** the QML engine loads, so QML never binds to a half-wired module.
- **INV-3** - `qInstallMessageHandler(MessageHandler)` is installed **only after**
  `Console::Handler` and `NotificationCenter` exist. The message handler constructs both on
  its first warning, from *any* thread; and `Console::Handler`'s constructor pulls
  `CommonFonts`, which touches the font database and must run on the GUI thread. Installing
  the handler earlier risks off-GUI-thread font-DB access on the first stray warning.

Additional constraints:

- Must not regress the 256 kHz `--benchmark-hotpath` gate (all seven tiers).
- Must work identically in QuickPlot, ProjectFile, and Console-only modes.
- Must not add mutexes to `FrameReader` / `CircularBuffer` and must keep hotpath signal hops
  `Qt::DirectConnection`; dependency capture may not change connection types on the hotpath.
- Behavior-preserving: every force-instantiated module in the pinned order must be one that
  the composition root already constructs transitively today (self-init + `QSettings` +
  self-forced connects only) - no new construction is introduced, only pinned earlier.
- Scope discipline: modules first built at context-property time (`ProjectEditor`,
  `ProtoImporter`, `Examples`, `HelpCenter`, ...) stay there; they are not pulled into the
  pinned core order.

## Open Questions

- Timing of S3 (pinning the construction order): the design places it late tonight behind a
  constructor re-verification gate; the orchestrator makes the go/no-go call. Everything from
  S4 onward is morning work, one stage per build.
- Whether the pentagon capture is promoted from advisory to blocking at the ratchet stage
  (S7) per-class, or left advisory. Deferred to S7.

## Appendix: Ctor-Capture Safety Table (top 15 singletons)

Verbatim from the validated design report. "Capture BY others" = safe for other classes to
capture this one; "Own deps capture mode" = how this class should acquire *its* dependencies.

<!-- doc-verify off -->

| # | Singleton (sites) | Verified ctor out-edges | Capture BY others | Own deps capture mode |
|---|---|---|---|---|
| 1 | ProjectModel (343) | ControlScript (via newJsonFile, PM.cpp:2116) | Yes except by ControlScript/workers | Deferred pointers in setupExternalConnections; ctor/newJsonFile surface keeps direct calls. NEVER ctor-capture AppState (live A<->B hazard) |
| 2 | ConnectionManager (301) | none | Yes | Deferred pointers (AppState, ProjectModel, FrameBuilder, Console::Handler, API::Server); may ctor-capture Translator |
| 3 | Dashboard (152) | CSV::Player, MDF4::Player, ConnectionManager, AppState, FrameBuilder, Sessions::Player, TimerEvents (Dashboard.cpp:172-229) | NO -- nobody ctor-captures Dashboard | Deferred pointers; ctor connects stay |
| 4 | AppState (146) | ProjectModel (conditional, deriveFrameConfig) | Yes except by ProjectModel and ControlScript | May ctor-capture ProjectModel post-S3; nothing else |
| 5 | ThemeManager (58) | WorkspaceManager (loadUserThemes), Translator | Yes except by those two | Ctor-ref capture of both fine |
| 6 | FrameBuilder (56) | LemonSqueezy -> MachineID [commercial] | Yes | Deferred pointers; hotpath members only via S6 |
| 7 | Console::Handler (33) | CommonFonts | Yes; must exist before qInstallMessageHandler | May ctor-capture CommonFonts |
| 8 | CommonFonts (31) | none (GUI-thread font DB) | Yes | true leaf |
| 9 | Translator (29) | none | Yes | true leaf |
| 10 | TimerEvents (28) | none | Yes | true leaf |
| 11 | API::Server (27) | CommandHandler (trivial) | Yes | may ctor-capture CommandHandler |
| 12 | CSV::Player (26) | none | Yes | leaf-ish (qApp filter) |
| 13 | WorkspaceManager (25) | none (mkdir + legacy migration) | Yes | true leaf |
| 14 | MDF4::Player (25) | none | Yes | leaf-ish |
| 15 | NotificationCenter (25) | none | Yes; must exist before message handler | true leaf |

<!-- doc-verify on -->

Special case - **FrameParser (23)**: stay lazy/deferred only. Its constructor calls
`engineForSource(0)`, and Lua engine construction reaches `FrameBuilder::instance()`
(`LuaScriptEngine.cpp:167`); the ctor closure is project-content-dependent, so FrameParser
must not be ctor-captured. It may still be force-instantiated in the pinned order where the
root already constructs it transitively, but no class captures it as a member.
