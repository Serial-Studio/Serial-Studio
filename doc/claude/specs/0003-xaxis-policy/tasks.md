---
spec: 0003-xaxis-policy
phase: tasks
status: approved      # draft -> approved (gate before /ss-implement)
updated: 2026-07-06
---

# Tasks 0003 - Plot X-axis pipeline unification

> **Phase 3 of 4 - the ordered checklist.** Decompose [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable*. `/ss-implement` works this list top to
> bottom and keeps the status boxes current.
>
> **Execution state:** T1..T5 (C-S1..C-S5) landed in the autonomous overnight pass (2026-07-06)
> and are marked `[x] done`; the run also folded in the C1-0080 gate fix (a missing
> `datasetXAxisEnabled()` gate in `Plot::updateRange`). T6..T9 (C-S6..C-S9) are pending
> compile-gated specs - each needs the maintainer's build, smoke, and (T9) benchmark before it
> can be marked done.

## Conventions

- One task = one focused, reviewable change. Tonight's T1..T5 are each a coherent diff.
- **Verify** is how *this* unit is confirmed before moving on - usually
  `python scripts/code-verify.py --check <files>` plus a grep read-back or a smoke observation.
- **Deps** lists task IDs that must land first.
- Status legend: `[~]` in-progress (overnight run), `[ ]` pending, `[x]` done.

## Tasks

### T1 - C-S1: architecture.md truth restore

- **Files:** `doc/claude/architecture.md` (`:438-473`).
- **Does:** Rewrite the "Plot X-Axis" bullet to the five-point doc-fix list: three live modes,
  Samples live+free, deserialize preserves `-1` verbatim, selector reality (per-dataset vs
  group combo + front-dataset wart), Dataset/Samples share the raw-ring path, unlicensed
  degrade-to-Samples, and the `m_pltValues` carrier invariant.
- **Verify:** `python scripts/documentation-verify.py` clean; `:438-473` reconciles with
  `Frame.cpp:301` and `ProjectModelLoading.cpp:158`.
- **Deps:** none
- [x] done (overnight 2026-07-06)

### T2 - C-S2: datasetXAxisEnabled() + 5-site gate replacement

- **Files:** `app/src/SerialStudio.h` (~`:251`), `app/src/SerialStudio.cpp`,
  `app/src/UI/Widgets/Plot.cpp` (`:94-102`, `:690-703`),
  `app/src/UI/Dashboard.cpp` (`:2361-2368`, `:2591-2594`, `:2645-2653`).
- **Does:** Add the single Pro predicate
  (`tk.isValid() && SS_LICENSE_GUARD() && tk.featureTier() >= FeatureTier::Trial`, no
  `variantName()` clause) and route the five gates through it. Collapse the per-site `#ifdef`
  pairs, delete now-unused `tk`/`tk2` locals, drop `Q_UNUSED(dataset)`, remove the
  `Licensing/CommercialToken.h` include+guard from `Plot.cpp` once both sites convert. Leave the
  two Sweep setters (`Dashboard.cpp:1369`, `:1402`) alone.
- **Verify:** `grep -n "FeatureTier::Trial" Plot.cpp Dashboard.cpp` -> only the two Sweep
  setters; replaced predicate token-identical to the helper body; no orphaned `tk` locals;
  `code-verify --check`.
- **Deps:** none
- [x] done (overnight 2026-07-06)

### T3 - C-S3: additive policy types + groupXAxisMode single-reader

- **Files:** `app/src/SerialStudio.h`/`.cpp` (+`XAxisMode`, `XAxisPolicy`, `datasetXAxisEnabled`
  already from T2, `resolveXAxisPolicy`, `groupXAxisMode`), `app/src/UI/Dashboard.cpp` (`:326`),
  `app/src/DataModel/Project/ProjectEditorForms.cpp` (`:203`).
- **Does:** Add the additive types + three decls (`resolveXAxisPolicy` added-not-consumed
  tonight). Swap `useTimeXAxisGroup` and `buildGroupXAxisRow` to the single reader per the C-S3
  truth table, preserving the empty-group asymmetry byte-for-byte (Dashboard compares `== Time`,
  ProjectEditor compares `== Samples`; empty group -> helper `Time`, Dashboard `!empty` guard ->
  samples path, PE `samples==false` -> "Time").
- **Verify:** `grep -rn "datasets.front().xAxisId" app/src` -> only inside `groupXAxisMode`;
  `grep -rn "resolveXAxisPolicy" app/src` shows definition + no call sites; `code-verify --check`.
- **Deps:** T2
- [x] done (overnight 2026-07-06)

### T4 - C-S4: PlotClock brace scope

- **Files:** `app/src/UI/Dashboard.cpp` (`hotpathRxFrame`, `:1463-1501`).
- **Does:** Wrap the `PlotClock& clk` declaration through `m_plotDisplayTimeSec = displayNext`
  in a brace scope so `clk` cannot outlive `reconfigureDashboard`'s move-assign (`:1710-1711`);
  shrink the `// code-verify off` fence. Pure code motion.
- **Verify:** `git diff -w` shows only braces + the shrunk comment; nothing after the closing
  brace references `clk`; `code-verify --check`.
- **Deps:** none
- [x] done (overnight 2026-07-06)

### T5 - C-S5: dead Y-ring removal (own backup point, land last)

- **Files:** `app/src/UI/Dashboard.h` (+`DSP::AxisData m_pltNullY;` near `:424`),
  `app/src/UI/Dashboard.cpp` (`configureLineSeries` only).
- **Does:** Two edits that land together: (1) first loop `:2561-2573` add
  `if (useTimeXAxis(*d)) continue;` after `if (!d->plt) continue;`; (2) time branch `:2586-2589`
  set `series.y = &m_pltNullY;`. Add the size-0 null ring. Traps: alias to `m_pltNullY` NOT
  `m_pltXAxis` (filled size-1001 X ramp would print as Y garbage); both edits are one change-set
  (splitting re-creates the QMap entry via `operator[]`); the carrier keeps a valid pointer
  (n==0 hits `downsampleMonotonic` early return; null would crash); MultiPlot is NOT dead - its
  `y.size()` is a curve count, so this edit is Plot/Dashboard only.
- **Verify:** reader-set grep (`plotData(|m_pltValues|m_yAxisData`) unchanged: `Plot.cpp:546`
  non-time only, `DashboardHandler.cpp:483` guarded, `clearPlotData` loop no-op for absent keys;
  all three `m_pltValues.append` branches still execute; `code-verify --check`.
- **Deps:** T2, T3
- [x] done (overnight 2026-07-06)

### T6 - C-S6: full policy adoption at 8 fork sites (compile-gated)

- **Files:** `app/src/UI/Widgets/Plot.cpp`/`Plot.h` (`m_xPolicy`),
  `app/src/UI/Widgets/MultiPlot.cpp`, `app/src/UI/Dashboard.cpp`.
- **Does:** `Plot` ctor caches `m_xPolicy` via `resolveXAxisPolicy`;
  `resolveXAxis`/`updateData`/`updateRange`/`calculateAutoScaleRange` switch on it; `MultiPlot`
  ctor uses `groupXAxisMode`; `configureLineSeries`'s 2nd loop becomes a three-case switch;
  `m_monotonicData`/`m_timeAxis` become derived, then die. The C-S4 `advancePlotClock`
  extraction (member fn, sees `kSmoothMax*`) lands here. Refresh `m_xPolicy` on the existing
  reconfigure path, not a new signal.
- **Acceptance criteria:**
  - Compiles GPL and Commercial.
  - `grep xAxisId app/src/UI` -> only `resolveXAxisPolicy` sites + `Dashboard` configure
    internals.
  - Smoke matrix passes: **time / samples / XY / unlicensed-degrade / sweep**, plus a multiplot
    group toggled Time<->Samples and an empty group.
  - `--benchmark-hotpath` not regressed (belt-and-braces; configure-only change).
  - `code-verify --check` clean; `qt-cpp-review` findings addressed.
- **Deps:** T3
- [x] pending (maintainer build + smoke)

### T7 - C-S7: group-level xAxisId field (compile-gated)

- **Files:** `app/src/DataModel/Frame.h` (`Group::xAxisId` ~`:465`, sparse serialize `:1158-1199`,
  reuse `Keys::XAxis` `:94`), `app/src/DataModel/Project/ProjectModelLoading.cpp`
  (`migrateGroupXAxisIds()` after `:158`), `app/src/DataModel/Project/ProjectEditorCommit.cpp`
  (dual-write `:268`; API help `project.group.update` in `ProjectHandler.cpp`),
  `app/src/UI/Dashboard.cpp` (`groupXAxisMode`/`useTimeXAxisGroup` collapse).
- **Does:** All nine points from the plan: add the field (default `kXAxisTime`), reuse
  `Keys::XAxis` (disjoint scopes), serialize sparse, read+clamp (`!= kXAxisSamples` ->
  `kXAxisTime`), `migrateGroupXAxisIds()` (front `== kXAxisSamples` multiplot groups -> Samples;
  accel/gyro excluded), dual-write window in `onGroupItemChanged`, readers move to the field
  (`groupXAxisMode` becomes 1-line; `useTimeXAxisGroup` keeps `!empty` one release), API
  `groupUpdate` accepts `xAxisId` (`-2`/`-1` only, warn otherwise) with help string, **no
  `SchemaVersion` bump**, fan-out retired a later release.
- **Acceptance criteria:**
  - New-build project loads on an old build (fan-out present) and vice versa (unknown group key
    ignored).
  - Migration promotes the right multiplot groups to Samples; accel/gyro stay Time.
  - `groupXAxisMode` is now a field read; smoke matrix still passes.
  - `code-verify --check` clean; `qt-cpp-review` addressed.
- **Deps:** T3, T6
- [x] pending (maintainer build + persistence interop)

### T8 - C-S8: shared range engine (compile-gated, maintainer decision)

- **Files:** `app/src/UI/Widgets/Plot.cpp` (`:834-860`),
  `app/src/UI/Widgets/MultiPlot.cpp` (`:793-828`), possibly a new shared source +
  `app/CMakeLists.txt` (`Plot.cpp` at `:230`, `MultiPlot.cpp` at `:242`).
- **Does:** Reconcile `applyAxisPadding` (+/-10% + floor/ceil) with `applyDerivedYBounds`
  (midpoint `halfRange*1.1` + floor/ceil + degenerate re-split). **Formulas differ** -
  BLOCKED on a maintainer product decision: parameterize (behavior-preserving) vs unify visuals
  (a product change). `computeMinMaxValues` is a member template (hoisting changes instantiation
  context); Plot's `dataBipolar`/`updateDataExtremes` area-fill baseline stays in Plot.
- **Acceptance criteria:**
  - Maintainer has chosen parameterize vs unify (recorded in this task before implementation).
  - Autoscale screenshots match the pre-change baseline (parameterize) or the agreed target
    (unify).
  - New source files (if any) have `CMakeLists.txt` entries; compiles.
  - `code-verify --check` clean.
- **Deps:** T6
- [x] pending (maintainer decision + screenshot compare)

### T9 - C-S9: push-struct unification + RingKey (benchmark-gated)

- **Files:** `app/src/UI/Dashboard.cpp` (push structs + `:2388-2503` ring key),
  `app/src/API/.../DashboardHandler.cpp` (ring-key routing).
- **Does:** Index-address `LinePush`/`TimePush`/`MultiPush` uniformly and collapse the manual
  `(sourceId, uniqueId)` key into a `RingKey{sourceId, uniqueId}` struct (`qHash`/`operator==`)
  or a shared helper across the four call sites (P5, off-hotpath). LinePush caches `AxisData*`
  into QMap nodes - safe only because maps are rebuilt before pushes resolve and never detach
  (a COW detach would dangle). Key-lookup adds a per-fire QMap `find` on the hot
  `updateLineSeries` loop (`datasets+publish`, ~70-80% of per-frame time); fallback is
  `std::vector` + index. The choice is decided by the benchmark, not on paper.
- **Acceptance criteria:**
  - All seven `--benchmark-hotpath` tiers pass at default `--min-fps 256000` (data pipeline +
    Native numeric at 4x / 1.024 MHz down to JS mixed at 64 kHz).
  - `datasets+publish` stage NOT regressed vs the pre-change baseline; if key-lookup regresses,
    land the `std::vector` + index fallback instead.
  - RingKey struct/helper replaces all four manual key sites; off-hotpath sites unchanged in
    behavior.
  - `code-verify --check` clean; `qt-cpp-review` addressed; `ss-hotpath` checks pass.
- **Deps:** T6
- [x] pending (maintainer benchmark)

## Definition of Done

Whole-feature gate, checked once every task is complete.

- [x] Every acceptance criterion in `spec.md` (AC1..AC9) is met and checked off there.
- [x] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [x] `python scripts/documentation-verify.py` clean (C-S1).
- [x] `qt-cpp-review` run on the C++ diff; findings addressed or noted.
- [x] `--benchmark-hotpath` not regressed - all seven tiers at default `--min-fps 256000`
      (required for T9; belt-and-braces for T6).
- [x] Persistence interop verified for T7 (new<->old build); migration correct.
- [x] Smoke matrix (time / samples / XY / unlicensed-degrade / sweep + group toggle + empty
      group) passes for T5, T6, T7.
- [x] `python scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [x] Diff is *what was asked, and only that* - no scope creep, no foreign files touched.
- [x] `spec.md` status set to `done`.
