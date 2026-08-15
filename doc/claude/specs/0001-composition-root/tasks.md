---
spec: 0001-composition-root
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-07-06
---

# Tasks 0001 - Composition Root & De-Singleton-ization

> **Phase 3 of 4 - the ordered checklist.** Decompose [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable* - each one a coherent diff a reviewer
> could read in isolation. `/ss-implement` works this list top to bottom and keeps the status
> boxes current. Gate: do not start `/ss-implement` until a human marks this `approved`.

## Conventions

- One task = one focused, reviewable change (one stage, one S4 wave, or one S5 pentagon
  class). If a task touches >3 files or needs a paragraph to describe, split it.
- **Verify** is how *this* unit is confirmed before moving on - usually the grep-symmetry or
  advisory-count check named in `plan.md`, plus `python3 scripts/code-verify.py --check` and a
  launch smoke test.
- **Deps** lists task IDs that must land first.
- **Build gate:** one task per build; launch all three operation modes after S3 and each S5
  class; `--benchmark-hotpath` after S6. Never batch two ordering-sensitive changes.
- **Status note:** T1, T2, and T3 completed tonight (autonomous overnight run, 2026-07-06). T3
  passed the orchestrator's constructor re-verification gate: `instantiateCoreModules()` landed
  in `ModuleManager.cpp` (called first in `setupCrossModuleConnections`), which required adding
  the `Licensing/MachineID.h` include. T4 onward is morning work.

## Tasks

### T1 (S1) - Dependency census + init-order contract (docs)

- **Files:** `doc/claude/specs/0001-composition-root/{spec,plan,tasks}.md`,
  `doc/claude/architecture.md` (new "Composition Root & Construction Order -- ModuleManager"
  subsection).
- **Does:** Capture the verified ctor-edge graph (spec appendix), the pinned S3 instantiation
  order, the three invariants INV-1..INV-3, and the per-file `::instance()` inventory
  (1,852 sites / 59 headers) as pre-scoping for the morning waves.
- **Verify:** `python3 scripts/documentation-verify.py` clean; every claimed ctor edge
  cross-checked against source.
- **Deps:** none
- [x] done

### T2 (S2) - Regression guard: `arch-singleton-instance` advisory rule

- **Files:** `scripts/code_verify_rules.py`, `scripts/code-verify.py`.
- **Does:** Add the advisory rule (regex `\b([A-Za-z_][\w:]*)::instance\(\)`, tree-sitter
  scoped) with the five sanctioned exemptions; register the kind in `_ADVISORY_KINDS`.
- **Verify:** `python3 scripts/code-verify.py app/src` before/after -> identical
  blocking-error count, advisory report gains only the new kind; synthetic `Foo::instance()`
  snippet fires; each sanctioned pattern does not.
- **Deps:** T1
- [x] done

### T3 (S3) - Pin the construction order (composition root)

- **Files:** `app/src/Misc/ModuleManager.{h,cpp}`.
- **Does:** Add private `void instantiateCoreModules();`, called first in
  `setupCrossModuleConnections()`; body force-instantiates each core module in the pinned
  order from `plan.md` (ProjectModel before AppState; Dashboard last). List only modules the
  root already constructs transitively.
- **Verify:** grep symmetry -- every class in `instantiateCoreModules` also appears later in
  `setupCrossModuleConnections` or is already transitively constructed; order matches the
  spec table; `restoreLastProject()` still after all `setupExternalConnections()` (INV-1).
  Build + launch all three operation modes + `--benchmark-hotpath`.
- **Deps:** T1 (T2 recommended)
- **Status:** done -- passed the orchestrator ctor re-verification gate;
  `instantiateCoreModules()` + call site landed in `ModuleManager.cpp`, adding a
  `Licensing/MachineID.h` include.
- [x] done

### T4 (S4 wave 1) - Leaf capture: `Misc/*` + `UI/Widgets/*`

- **Files:** `app/src/Misc/*`, `app/src/UI/Widgets/*` consumers of
  CommonFonts/TimerEvents/ThemeManager.
- **Does:** Replace non-hotpath `X::instance()` calls with ctor init-list reference members
  for the verified leaves (CommonFonts, TimerEvents, ThemeManager near-leaf).
- **Verify (per file):** `grep -c "X::instance()"` in the `.cpp` = 0 outside the ctor init
  list; header gained exactly one member per dep; init-list position matches member order;
  advisory count strictly decreases; `-Wreorder`/zero-warnings clean; launch smoke test.
- **Deps:** T3
- [ ] done

### T5 (S4 wave 2) - Leaf capture: `Console/`, `CSV/`, `MDF4/`

- **Files:** `app/src/Console/*`, `app/src/CSV/*`, `app/src/MDF4/*` (CSV::Player,
  MDF4::Player, Console consumers).
- **Does:** Same leaf-capture transformation as T4 for these subsystems.
- **Verify:** same per-file checks as T4; advisory count strictly decreases; launch smoke
  test.
- **Deps:** T4
- [ ] done

### T6 (S4 wave 3) - Leaf capture: `DataModel/` editors

- **Files:** `app/src/DataModel/*` editor consumers (NotificationCenter, WorkspaceManager,
  CommandHandler, Translator).
- **Does:** Same leaf-capture transformation for the DataModel editors. Do **not** touch the
  pentagon classes here (that is S5).
- **Verify:** same per-file checks as T4; advisory count strictly decreases; launch smoke
  test.
- **Deps:** T5
- [ ] done

### T7 (S5) - Pentagon deferred pointer capture

- **Files:** `app/src/AppState.{h,cpp}`, `app/src/IO/ConnectionManager.{h,cpp}`,
  `app/src/DataModel/FrameBuilder.{h,cpp}`, `app/src/DataModel/ProjectModel.{h,cpp}`,
  `app/src/UI/Dashboard.{h,cpp}`.
- **Does:** Each class gains `X* m_x` (nullptr in ctor init list), assigned at the top of its
  own `setupExternalConnections()`, with `Q_ASSERT(m_x)` at use sites; method-body call sites
  convert to `m_x->`. Ctor bodies and pre-wiring surfaces keep direct `instance()` (AppState
  `deriveFrameConfig()`; ProjectModel `newJsonFile()`/`setModified()`/`watchProjectFile()`;
  Dashboard ctor connects; ConnectionManager `m_uiDriverSaveTimer` lambda). AppState may
  ctor-capture ProjectModel post-S3; **never ctor-capture AppState** from ProjectModel.
  **One class per morning build, in order:**
  - [x] AppState
  - [ ] ConnectionManager
  - [ ] FrameBuilder
  - [ ] ProjectModel
  - [ ] Dashboard
- **Verify (per class):** header gained exactly the pointer members; every converted call
  site is in a method proven to run after wiring; pre-wiring surface still uses direct
  `instance()`; advisory count decreases; launch all three operation modes.
- **Deps:** T3 (T6 recommended)
- [ ] done

### T8 (S6) - Hotpath capture (benchmark-gated, last)

- **Files:** `app/src/IO/ConnectionManager.{h,cpp}`, `app/src/UI/Dashboard.{h,cpp}`.
- **Does:** Convert the `static auto&` cache statics in
  `ConnectionManager::{onFrameReady,onRawDataReceived,processPayload,processMultiSourcePayload}`
  and `Dashboard::updateStreamAvailable` to the T7 pointer members; replace the per-frame
  `AppState::instance().operationMode()` poll in `onFrameReady` with a cached `m_operationMode`
  refreshed on `operationModeChanged`. All connections stay `Qt::DirectConnection`.
- **Verify:** converted sites are members not statics; `m_operationMode` wired to
  `operationModeChanged`; `ss-hotpath` review; **`--benchmark-hotpath` all seven tiers**, no
  regression vs the pre-T8 baseline.
- **Deps:** T7
- [ ] done

### T9 (S7) - Ratchet the sanctioned surface

- **Files:** `scripts/code_verify_rules.py`, `scripts/code-verify.py`.
- **Does:** Shrink the sanctioned surface to `{ModuleManager.cpp, main.cpp, own instance()
  def, setupExternalConnections bodies}` (drop the interim `static auto&` cache-idiom
  exemption removed in T8). Optionally promote the pentagon to blocking per-class. New
  services take ctor params.
- **Verify:** the four remaining sanctioned patterns produce no finding; interim `static
  auto&` sites now report or are gone; blocking-error count unchanged unless a per-class
  blocking promotion is deliberately enabled.
- **Deps:** T8
- [ ] done

## Definition of Done

<The whole-feature gate, checked once every task is complete.>

- [ ] Every acceptance criterion in `spec.md` (AC1-AC5) is met and checked off there.
- [ ] `python3 scripts/code-verify.py --check` is clean on all changed files (no new errors);
      the advisory `arch-singleton-instance` count is strictly lower than at S2 baseline.
- [ ] `qt-cpp-review` run on each C++ diff; findings addressed or noted.
- [ ] `--benchmark-hotpath` not regressed on any of the seven tiers (after T8).
- [ ] App launches and runs correctly in QuickPlot, ProjectFile, and Console-only modes.
- [ ] `python3 scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [ ] Diff is *what was asked, and only that* - no scope creep, no foreign files touched.
- [ ] `spec.md` status set to `done`.

## Post-landing incident (2026-07-07 morning)

- [x] REGRESSION FIXED: the cycle-2 fix that added AppState/Dashboard sync inside
      `newJsonFile()` ran inside ProjectModel's ctor and recursed the Meyers guard on
      ProjectFile machines (`__cxa_guard_acquire` abort at startup). The T3 ctor-edge
      proof was correct when written; a later fix wave invalidated it. Fix: `m_initialized`
      flag gates the sync; guard documented in architecture.md (protected-surface note).
      Standing rule: any edit to ProjectModel's ctor closure (newJsonFile, watchProjectFile,
      scheduleAutoSave, setCode chain) re-triggers the ctor-edge check before merge.
