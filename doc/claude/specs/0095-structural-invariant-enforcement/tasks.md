---
spec: 0095-structural-invariant-enforcement
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-10-07
---

# Tasks 0095 — Structural invariant enforcement

> **Phase 3 of 4 — the ordered checklist.** Decomposes [`plan.md`](./plan.md). Milestone order
> M6 → M2 → M3 → M1 → M4 → M5; each milestone leaves the tree green and can land on its own.
> Every C++ task's Verify includes `python scripts/syntax-check.py <files>` and
> `python scripts/code-verify.py --check <files>`, which are not repeated below.

## Conventions

- One task = one focused, reviewable change. If a task touches >3 files or needs a paragraph
  to describe, split it.
- **Deps** lists task IDs that must land first.

## M6 — race-free flag inputs

### T1 — ControlScript running flag atomic

- **Files:** `core/Pipeline/DataModel/Scripting/ControlScript.h`, `ControlScript.cpp`
- **Does:** `m_running` becomes `std::atomic<bool>` with explicit relaxed `load`/`store` at
  every site. *Invariant:* it is read by the pipeline thread in `refreshLatestFrameCapture`,
  and the `runningChanged` emission order is unchanged, so the cache refresh still fires.
- **Verify:** syntax-check; grep shows no bare `m_running` read or write remains.
- **Deps:** none
- [x] done

### T2 — API server enabled flag and client count atomic

- **Files:** `core/Api/API/Server.h`, `Server.cpp`
- **Does:** `m_enabled` becomes `std::atomic<bool>` and `m_clientCount` becomes
  `std::atomic<int>`, both relaxed. *Invariant:* this changes storage only. Auth, consent and
  dispatch are untouched (a protected surface). `enabledChanged` and `clientCountChanged` still
  drive `sinkActivityChanged`.
- **Verify:** syntax-check; diff touches no auth or consent code.
- **Deps:** none
- [x] done

### T3 — MQTT publisher enabled flag atomic

- **Files:** `core/Storage/MQTT/Publisher.h`, `Publisher.cpp`
- **Does:** ~~`m_enabled` becomes `std::atomic<bool>`~~ **As built:** `sinkActive()` reads the
  existing atomic mirror `m_hotEnabled`, which `setEnabled` already keeps in step. The other
  `m_enabled` reads are GUI-thread. Line ~:1334 turned out to be a GUI notification handler,
  not the frame tap.
- **Verify:** syntax-check.
- **Deps:** none
- [x] done

### T4 — AudioExport atomic session summary

- **Files:** `core/Ui/UI/Widgets/AudioExport.h`, `AudioExport.cpp`
- **Does:** add `std::atomic<bool> m_anyActiveSession`, stored after each of the four
  `m_activeSessions` mutations. `hasActiveSessions()` reads the atomic, so the `QSet` is never
  read cross-thread.
- **Verify:** syntax-check; grep confirms that every `m_activeSessions` mutation is followed by
  the store.
- **Deps:** none
- [x] done

### T5 — ProjectModel change-driven flag atomic

- **Files:** `core/Pipeline/DataModel/ProjectModel.h`, `ProjectModel.cpp`,
  `Project/ProjectLoader.cpp` (+ `Project/ProjectPersistence.cpp` read site)
- **Does:** `m_changeDrivenTransforms` becomes `std::atomic<bool>`, with every access an
  explicit load or store. *Invariant:* the ctor closure is protected. The only ctor change is
  the constant initializer at :67, which reaches nothing. The undo/`setModified` flow in the
  setter is unchanged.
- **Verify:** syntax-check; the ctor diff is the initializer line only.
- **Deps:** none
- [x] done

### T6 — IBlockSink contract, gRPC sink, read-back

- **Files:** `core/Core/DataModel/IBlockSink.h`, `app/src/API/GRPC/GRPCServer.h/.cpp`
  (**added during implementation**: an eighth sink the plan missed)
- **Does:** the `sinkActive()` `@brief` states the contract (R22). gRPC `m_enabled` becomes
  `std::atomic<bool>`. **As built:** there is no new threading ctest, because a test of
  `std::atomic` proves nothing. AC11 rests on the read-back below plus the maintainer's TSan
  build.
- **Read-back (AC11), all eight `sinkActive()`:**
  - CSV, MDF4: `consumerEnabled()`, an atomic.
  - InfluxDB, Sessions: `m_exportEnabled`, an atomic.
  - MQTT: `m_hotEnabled`, an atomic.
  - API Server: `m_enabled` / `m_clientCount`, now atomic.
  - gRPC: `m_enabled`, now atomic; `m_clientCount` was already atomic.
  - AudioExport: `m_anyActiveSession`, a new atomic summary.

  Checker inputs: `ControlScript::running()` and `ProjectModel::changeDrivenTransforms()` are
  now atomic.
- **Verify:** code-verify clean; read-back above.
- **Deps:** T1–T5
- [x] done

## M2 — typed republish lanes

### T7 — Lane handle types

- **Files:** `core/Pipeline/DataModel/RepublishGate.h`
- **Does:** add `DashboardLane` and `ExportLane`, each with `static constexpr bool
  kFeedsExports`. The bool-taking `needed` and `notePublished` become private and are reachable
  only through the lanes. Add `dashboardLane()` and `exportLane()` accessors. *Invariant:* any
  lane marks a change as sink-dirty, and only an export publish clears it.
- **Verify:** syntax-check.
- **As built:** a single `RepublishLane` handle (identity fixed at issue, `feedsExports()`)
  instead of two types plus templates. `FrameBuilder.cpp` sits exactly at the TU-census worst-file
  baseline (2898), and the templates would have added lines. One pass can no longer ask with
  one lane and record with another, and the gate's bool API is private. AC3's probe becomes
  "`gate.needed(k, c, true)` from outside fails to compile".
- **Deps:** none
- [x] done

### T8 — FrameBuilder republish templated on lane

- **Files:** `core/Pipeline/DataModel/FrameBuilder.h`, `FrameBuilder.cpp`
- **Does:** `republishFrames`, `republishOneFrame` and `emitRepublishedFrame` become
  `template <class Lane>`. `dashboardTick()` uses `ExportLane`; `reprocessFrames()` and the
  stream-dirty refresh use `DashboardLane`. The sink-mask line uses `!Lane::kFeedsExports ||
  !frameIsTableFed(frame)`. *Invariant:* the export lane must never be discharged by a masked
  pass (spec 0064). `frameIsTableFed` stays lane-independent.
- **Verify:** syntax-check; read FrameBuilder.cpp in full first (hotpath file).
- **Deps:** T7
- [x] done

### T9 — Port and extend the lane tests

- **Files:** `app/tests/tst_republish_lanes.cpp`
- **Does:** port the 7 tests to the lane handles, and add a test where N masked refreshes are
  followed by an export publish (AC4). Run the AC3 compile-fail probe once (not checked in).
- **Verify:** `ctest -R tst_republish_lanes` (**pending**: no unit build or compile DB under
  `build/`); added `lanesCarryTheirIdentity`.
- **Deps:** T8
- [x] done

## M3 — rate-sized rings, unified plot clocks

### T10 — RingCapacity type

- **Files:** `core/Pipeline/DSP.h`
- **Does:** add `RingCapacity` with a private int ctor, `fromRate`, `atLeast`, `atMost` and
  `value()`. `kMaxRateSizedRingSamples` moves here. The `TimeRing` and `EnvelopeRing` ctors and
  `resizeCapacity` take `RingCapacity`. *Invariant:* a ring is sized from rate × window, never
  a sample count alone.
- **Verify:** syntax-check on DSP.h dependents.
- **Deps:** none
- [x] done

### T11 — TimeRingSizing policies

- **Files:** `core/Ui/UI/Dashboard/TimeRingSizing.h/.cpp` (new), the CMake source list for
  `SerialStudioUi`
- **Does:** move the frame-lane, stream-lane and growth-decision policies verbatim out of
  Dashboard.cpp as free functions returning `RingCapacity` / `std::optional<RingCapacity>`.
- **Verify:** syntax-check; `layer-verify.py`.
- **Deps:** T10
- [x] done

### T12 — Dashboard uses the policies

- **Files:** `core/Ui/UI/Dashboard.h`, `Dashboard.cpp`
- **Does:** `makeHistoryRing` and `growTimeRing` call TimeRingSizing. **As built:** the `cap`
  locals were NOT dead, because they size the sweep engine. They now come from
  `frameLaneCapacity(m_plotTimeRange)`. `makeHistoryRing` moved into TimeRingSizing.
  Dashboard.cpp went from 1812 to 1756 lines. *Invariant:* the growth path is upward only, keeps its 1.5×
  hysteresis, and resizes only a saturated ring.
- **Verify:** syntax-check; read Dashboard.cpp in full first (hotpath file).
- **Deps:** T11
- [x] done

### T13 — PlotClockState

- **Files:** `core/Ui/UI/Dashboard/DashboardIngest.h/.cpp`, `core/Ui/UI/Dashboard.h/.cpp`,
  `core/Ui/UI/Dashboard/ReplaySeekEngine.h` (comment)
- **Does:** `PlotClockState { sources, displayTimeSec, reset() }`. `IngestBindings` binds one
  reference. `resetPlotClocks()` calls `reset()`. The save/restore in `reconfigureDashboard`
  copies the struct. *Invariant:* clocks and display time are one state, and a ring's clock
  never rewinds. Keep the tight-reference rule around the `sources` move-assign.
- **Verify:** syntax-check.
- **Deps:** T12
- [x] done

### T14 — Ring tests

- **Files:** `app/tests/tst_time_ring_sizing.cpp` (new), `tst_envelope_ring.cpp`,
  `tst_replay_seek_engine.cpp`, `tst_dashboard_ingest.cpp`, `app/tests/CMakeLists.txt`
- **Does:** new sizing test covering 44.1/48 kHz × 10 s, saturated growth and the ceiling.
  Convert existing literals to `RingCapacity::fromRate`, and make the ingest fixture bind
  `PlotClockState`. Run the AC5 probe once.
- **Verify:** `ctest -R "time_ring_sizing|envelope_ring|replay_seek|dashboard_ingest"`.
- **Deps:** T13
- [x] done

### T15 — Benchmark checkpoint A (maintainer)

- **Does:** the maintainer builds and runs `--benchmark-hotpath`. All nine gates must pass.
- **Deps:** T1–T14
- [ ] done

## M1 — cached-flag cross-check

### T16 — Cached<T> and CachedFlagChecker

- **Files:** `core/Pipeline/DataModel/CachedFlag.h` (new)
- **Does:** `Cached<T>` holds a `T` only and has a registering ctor. The checker is a
  `std::array<Entry,16>` of function pointers with an owner context, a two-consecutive-mismatch
  rule, an `isSettled` skip, and `SS_ASSERT(..., repair)`. *Invariant:* no allocation, and a
  per-frame read is a plain load.
- **As built:** `CachedFlagSpec` (name, fresh, repair, settled, optional mirror reader).
  `Cached<T>` converts to `const T&`, so the `QuickPlotBuilder`/`ReplayIngest` reference bindings
  still bind the member and never a temporary. It keeps `operator=(T)`, so the existing assignment
  sites are unchanged. The frame-builder derivations, settled predicates and specs live in a new
  static-only friend, `FrameBuilder/BuilderFlagAudit.{h,cpp}`, so `FrameBuilder.cpp` went from 2898
  to 2891 lines and the singleton census is unchanged (800/421). Dashboard's checker also audits
  the PipelineHost accepting and operation-mode mirrors (new `PipelineHost::dashboardAccepting()`).
- **Verify:** syntax-check.
- **Deps:** none
- [x] done

### T17 — Checker unit test

- **Files:** `app/tests/tst_cached_flags.cpp` (new), `app/tests/CMakeLists.txt`
- **Does:** covers mismatch then repair on the second check, the skip for unsettled entries,
  and the capacity bound (AC1).
- **Verify:** `ctest -R tst_cached_flags`.
- **Deps:** T16
- [x] done

### T18 — FrameBuilder flags on Cached<T>

- **Files:** `core/Pipeline/DataModel/FrameBuilder.h`, `FrameBuilder.cpp`
- **Does:** five flags become `Cached<T>`. Extract `deriveCaptureLatest()`,
  `deriveChangeDriven()` and `deriveCaptureDatasetValues()`, and make each refresh call
  `flag.set(deriveX())`. Run the checker from the existing 1 Hz lambda (~:569). Repair goes
  through the existing refresh slots. *Invariant:* refresh slots stay pipeline-affine and
  auto-queued from GUI emitters, and are never "fixed" to Direct. The dataset pair is skipped
  while `m_captureFlagsDirty` is set or the epoch has moved. No per-frame work is added.
- **Verify:** syntax-check; read FrameBuilder.cpp in full first.
- **Deps:** T16, T6
- [x] done

### T19 — BlockPublisher sink flag on Cached<bool>

- **Files:** `core/Pipeline/DataModel/FrameBuilder/BlockPublisher.h`, `BlockPublisher.cpp`
- **Does:** `m_anyAsyncSink` becomes `Cached<bool>` with a derive helper shared with
  `refreshSinkFlag`, registered in FrameBuilder's checker.
- **Verify:** syntax-check.
- **Deps:** T18
- [x] done

### T20 — Dashboard and PipelineHost GUI-side checker

- **Files:** `core/Ui/UI/Dashboard.h`, `Dashboard.cpp`, `core/Pipeline/IO/PipelineHost.h/.cpp`
- **Does:**
  - `m_streamAvailable` becomes `Cached<bool>`, derived from `streamAvailable()`, with a second
    entry comparing it to the accepting mirror.
  - The PipelineHost operation-mode mirror is checked against its GUI-side source.
  - The checks run from the existing GUI 1 Hz slots.
  - *Invariant:* the Dashboard side stays Direct-wired and GUI-affine, and repair goes through
    `updateStreamAvailable()` so the accepting mirror is pushed.
- **Verify:** syntax-check; `--singleton-census --check` (no new reaches).
- **Deps:** T16
- [x] done

### T21 — Doc anchor and R5 reconcile

- **Files:** `scripts/doc-anchors.json`, `doc/claude/architecture/dataflow.md`,
  `doc/claude/common-mistakes.md`
- **Does:**
  - Add the `cached-hotpath-flags` ordered anchor.
  - dataflow.md lists the `Cached<T>` members and defines a cached hotpath flag as a
    `Cached<T>`.
  - Scope common-mistakes :24 to the Dashboard side (R5).
  - Run the AC2 probe once.
- **Verify:** `python scripts/claim-verify.py`.
- **Deps:** T18–T20
- [x] done

### T22 — Benchmark checkpoint B (maintainer)

- **Does:** run `--benchmark-hotpath` (all nine gates) and `pytest tests/integration/ -v`.
- **Deps:** T16–T21
- [ ] done

## M4 — construction-order guard

### T23 — ModuleConstruction helper

- **Files:** `core/Core/ModuleConstruction.h/.cpp` (new), `app/src/SessionContext.h/.cpp`
- **Does:** a module-static "constructing" marker and `reportUnavailable(module)`.
  `SessionContext::create<T>` sets and clears the marker under a scope guard. *Invariant:* no
  library includes `SessionContext.h`, and nothing is constructed or reached.
- **As built:** `Core::ModuleConstruction::Scope` keys on `T::staticMetaObject`, because all
  nine modules are QObjects. `create<T>` opens the scope only for QObject types.
  `unavailableReason()` is the testable text and `reportUnavailable()` calls `qFatal` with it.
- **Verify:** syntax-check; `layer-verify.py`.
- **Deps:** none
- [x] done

### T24 — Nine accessors report their cause

- **Files:** the nine `instance()` definitions (AppState, Dashboard, Console Handler,
  FrameParser, FrameBuilder, ProjectModel, PipelineHost, ConnectionManager, NotificationCenter)
  — split into T24a (pipeline: 5) and T24b (ui/devices/core: 4)
- **Does:** swap the `qFatal` literal for `reportUnavailable("X")`.
- **Verify:** syntax-check; `--singleton-census --check`.
- **Deps:** T23
- [x] done

### T25 — composeSession entry point

- **Files:** `app/src/Misc/ModuleManager.h/.cpp`, `app/src/Misc/CLI.cpp`
- **Does:** `composeSession(BindMode::Full|SchemaOnly)`, with instantiate and bind made
  private. *Invariant:* copy each root's existing connections→bind order verbatim (GUI ~:1177,
  headless ~:1053, benchmark, schema dump), because a sink left unbound produces a
  valid-looking empty recording. Run the AC8 probe once.
- **As built:** every root already bound immediately after instantiating, and the headless root
  bound as the first line of `setupHeadlessSessionConnections()`. That call was removed (composeSession
  binds now), so the order is unchanged on every root.
- **Verify:** syntax-check; `claim-verify.py` (`composition-root-order` anchor intact).
- **Deps:** none
- [x] done

### T26 — Dead flag and startup doc truth

- **Files:** `core/Pipeline/DataModel/ProjectModel.h/.cpp`,
  `Project/ProjectPersistence.h`, `doc/claude/architecture/startup.md`
- **Does:**
  - Delete `m_initialized`.
  - Fix startup.md :103, :129 and :230.
  - Narrow the ctor-edge re-run trigger to new function-static reaches (census-visible) (R14,
    R15).
  - *Invariant:* the ctor edit is a pure deletion and adds no reach.
- **Verify:** syntax-check; `claim-verify.py`.
- **Deps:** T25
- [x] done

### T27 — Construction test

- **Files:** `app/tests/tst_module_construction.cpp` (new), `app/tests/CMakeLists.txt`
- **Does:** checks the self-reach-during-construction and reach-before-create messages (AC7).
- **Verify:** `ctest -R tst_module_construction`.
- **Deps:** T23
- [x] done

## M5 — enforcer markers and triage

### T28 — claim-verify enforcer check

- **Files:** `scripts/claim-verify.py`, `scripts/tests/test_claim_verify_enforcers.py` (new)
- **Does:** add `check_enforcers()` covering the `code-verify`, `ctest`, `anchor`, `script`,
  `hook` and `compile` kinds, plus an `enforcer-missing` error.
- **Verify:** `pytest scripts/tests/test_claim_verify_enforcers.py`; a seeded bogus marker
  fails (AC9).
- **Deps:** none
- [x] done

### T29 — Ledger tags to markers and the debt section

- **Files:** `doc/claude/common-mistakes.md`
- **Does:** convert the 5 `Codified:` tags to markers. Add an "Unenforced invariants" section
  ordered by blast radius (R19). It includes the player-mask upstream hop and a new plain-bool
  cache as the residual.
- **Verify:** `claim-verify.py`.
- **Deps:** T28
- [x] done

### T30 — CLAUDE.md triage

- **Files:** `CLAUDE.md`
- **Does:** mark each binding bullet (Threading & Hotpath, Startup, Subsystem Contracts) with
  `Enforced:`, `Debt:` or nothing. Collapse the enforced ones to one sentence plus their marker.
  Record the before and after line counts.
- **As built:** every binding bullet now carries `Enforced:` (resolved by claim-verify), `Debt:`,
  or nothing (judgment). Line counts: CLAUDE.md went from 445 to 448 and the live guidance from
  5297 to 5305 lines. It did not shrink, because the newly enforced rules were short and the
  markers add a line each. The gain is that every rule's status is now explicit and checked;
  real shrinkage needs the Debt items closed.
- **Verify:** `claim-verify.py`; the canary anchors (`qt-version`, `cxx-standard`) still pass.
- **Deps:** T29, M1–M4 landed
- [x] done

### T31 — Architecture docs receive moved rationale

- **Files:** `doc/claude/architecture/{dataflow,dashboard,startup}.md`
- **Does:** move the rationale and incident history from collapsed CLAUDE.md bullets into the
  linked doc, so nothing is lost.
- **Verify:** `claim-verify.py`; diff review shows moved text, not deleted text.
- **Deps:** T30
- [x] done

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` (AC1–AC11) is met and checked off there.
- [ ] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [ ] `--tu-census`, `--singleton-census`, `layer-verify.py` and `claim-verify.py` pass with no
  baseline growth.
- [ ] `qt-cpp-review` run on the C++ diff; findings addressed or noted.
- [ ] `--benchmark-hotpath` passes all nine gates (T15, T22).
- [ ] Maintainer runs `pytest tests/integration/ -v` (export/recording paths).
- [ ] `python scripts/sanitize-commit.py` run.
- [ ] Diff is *what was asked, and only that*.
- [ ] `spec.md` status set to `done`.
