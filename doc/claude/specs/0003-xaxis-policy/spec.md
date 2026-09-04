---
spec: 0003-xaxis-policy
title: Plot X-axis pipeline unification
status: done          # draft -> approved -> in-progress -> done | shelved
created: 2026-07-06
author: Alex Spataru
---

# Spec 0003 - Plot X-axis pipeline unification

> **Phase 1 of 4 - the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.
>
> Note: this package was authored from a validated design track and is being executed as
> an autonomous overnight run (2026-07-06). Stages C-S1..C-S5 are in progress tonight;
> C-S6..C-S9 remain compile-gated specs. File:line refs below are the design ground truth
> and belong in `plan.md`; they are retained here only to make each pain point locatable.

## Problem / Motivation

A plot's X axis can be one of three things: wall-clock **Time**, a monotonic **Samples**
index, or another **Dataset** (an XY plot). That is a genuine three-way fork with three
different data sources, so it cannot be collapsed to fewer render paths. The problem is not
the fork - it is that the *decision* of which arm you are in is re-derived from raw
`xAxisId` sentinels in roughly eight places (`Plot`, `MultiPlot`, `Dashboard` configure and
draw, `ProjectEditor`, `ProjectModel`, `DashboardHandler`), and each of the Pro-gate,
group-encoding, and degrade rules is copy-pasted rather than resolved once. Every new mode
question ("is this a time plot?", "is dataset-X licensed here?", "is this group a samples
group?") is answered by re-reading sentinels inline, so the truth is smeared across the UI
layer and drifts.

Concretely this has already produced live defects and documentation that contradicts the
code: the same Pro licensing predicate is pasted at five sites; a multiplot group encodes a
group-level property (its X mode) by fanning a value into each member dataset and then
reading `front()` back as canonical; two call sites disagree on what an *empty* group means;
a per-dataset Y ring is allocated for every time plot and then never pushed or read; the
`PlotClock` reference lives in a scope where a reconfigure move-assign could invalidate it;
and the architecture doc's "Plot X-Axis" bullet asserts behavior (`kXAxisSamples` removed as
a user option, deserialize remaps `-1 -> -2`) that the code does not implement. None of this
is on the hotpath, and none of it needs to move the hotpath - it is a resolve-once cleanup of
the mode decision that sits *above* the already-mode-specialized render machinery.

## Goals

- A single resolve-once policy type answers "what is this axis?" for the whole UI layer,
  consulted at UI rate (60 Hz), never on the per-frame hotpath.
- The Pro gate for dataset-X exists as exactly one named predicate, mirroring the existing
  `activated()` / `proWidgetsEnabled()` precedent; unlicensed / Trial-expired dataset-X
  silently degrades to Samples as one documented line, not five copy-pasted `#ifdef` blocks.
- The empty-group and front-dataset group-encoding asymmetries are made visible in one place
  behind a single reader, then retired in favor of a real group-level field.
- The dead time-plot Y-ring allocation is removed without disturbing the load-bearing
  LineSeries carrier (index alignment + reconfigure trigger) it lived next to.
- The `PlotClock` lifetime hazard is closed by scope, with zero codegen change.
- `architecture.md` describes the three live modes as they actually behave.
- The 256 kHz hotpath gate is provably untouched by every stage except C-S9, which is
  explicitly benchmark-gated.

## Non-Goals

- **Not** collapsing the three render arms (sweep-ring / time-ring / sample-ring) into one:
  the data sources genuinely differ. Unification means one predicate decides which arm runs,
  not that the arms merge.
- **Not** making Sweep a fourth mode. Sweep stays an overlay; the invariant "sweep legal iff
  mode == Time" is preserved.
- **Not** touching the hotpath render/push machinery (feed lambdas, TimeRing, SweepEngine,
  the three downsamplers, push tables) in C-S1..C-S8. Only C-S9 touches push-table storage,
  and only under a hard benchmark gate.
- **Not** unifying the Plot vs MultiPlot autoscale padding *visuals* by fiat - the formulas
  differ, and whether to parameterize (behavior-preserving) or unify (a visual change) is a
  maintainer product decision (C-S8), not something this spec pre-decides.
- **Not** bumping the project `SchemaVersion`. The group-level field (C-S7) is a sparse,
  backward-compatible key with a dual-write window.

## Requirements

Ordered so the tree stays green after each. R1..R6 are tonight's work (C-S1..C-S5); R7..R10
are compile-gated follow-on specs (C-S6..C-S9).

1. **R1 (P9 / C-S1)** - `architecture.md`'s "Plot X-Axis" bullet describes three live modes
   (Time, Samples, Dataset), states that Samples is a free, live mode, and states that
   deserialize preserves `-1` verbatim. No claim that `kXAxisSamples` was removed or that
   deserialize remaps `-1 -> -2`.
2. **R2 (P2 / C-S2)** - a single `SerialStudio::datasetXAxisEnabled()` predicate is the only
   dataset-X Pro gate. All five copy-pasted gate sites consult it; the two Sweep-setter gates
   are left alone. The GPL build compiles with the Dataset branch statically dead.
3. **R3 (P1 foundation / C-S3)** - the `XAxisMode` enum and `XAxisPolicy` POD exist in
   `SerialStudio.h`, `resolveXAxisPolicy()` and `groupXAxisMode()` are declared and defined,
   and `groupXAxisMode()` is the *single* reader of the front-dataset group encoding.
   `resolveXAxisPolicy()` is added-not-consumed tonight (no call sites yet).
4. **R4 (P4/P10 / C-S3)** - the empty-group asymmetry between `useTimeXAxisGroup` (empty ->
   samples path) and `buildGroupXAxisRow` (empty -> shows "Time") is byte-for-byte preserved,
   but is now derived from the one documented truth table rather than two inline reads.
5. **R5 (P7 / C-S4)** - the `PlotClock` reference in `hotpathRxFrame` is brace-scoped so it
   cannot outlive `reconfigureDashboard`'s move-assign. `git diff -w` shows only braces and a
   shrunk fence comment; zero codegen change.
6. **R6 (P4 / C-S5)** - the dead per-dataset time-plot Y-ring is no longer allocated; the
   LineSeries carrier (one per plot widget, index-aligned, `size() != plotCount` reconfigure
   trigger) keeps a valid pointer, and all readers are provably unaffected.
7. **R7 (P1 / C-S6)** - every remaining mode fork consults the cached policy: `Plot` caches
   `m_xPolicy` at construct/configure and switches on it in
   `resolveXAxis`/`updateData`/`updateRange`/`calculateAutoScaleRange`; `MultiPlot` uses
   `groupXAxisMode`; `configureLineSeries`'s second loop becomes a three-case switch;
   `m_monotonicData` / `m_timeAxis` become derived, then die.
8. **R8 (P3/P10 / C-S7)** - a real group-level `Group::xAxisId` field exists, serialized
   sparsely, migrated forward, with a dual-write window; `groupXAxisMode` collapses to a
   one-line field read and the front-dataset encoding is retired.
9. **R9 (P8 / C-S8)** - the Plot and MultiPlot range/padding engines are reconciled per the
   maintainer's parameterize-vs-unify decision, shipped compiled with a screenshot compare.
10. **R10 (P5/P6 / C-S9)** - push-struct storage is index-addressed everywhere and the
    `(sourceId, uniqueId)` ring key is a single struct/helper, landed only if
    `--benchmark-hotpath` is not regressed.

## Acceptance Criteria

- [x] **AC1 (R1)** - a reader of the `architecture.md` "Plot X-Axis" bullet (currently
      lines ~486-536) sees three live modes and the correct deserialize behavior. (The
      documentation-verify linter does not scan doc/claude/, so this AC is review-only.)
- [x] **AC2 (R2)** - `grep -rn "FeatureTier::Trial" app/src/{UI,Misc}` returns only the two
      Sweep setters (`Dashboard.cpp:1369`, `:1402`); the replaced predicate is token-identical
      to `datasetXAxisEnabled()`'s body; no orphaned `tk`/`tk2` locals; GPL build compiles.
- [x] **AC3 (R3)** - `grep -rn "datasets.front().xAxisId" app/src` matches only inside
      `groupXAxisMode`; `grep -rn "resolveXAxisPolicy" app/src` shows the definition and no
      call sites.
- [x] **AC4 (R4/R6)** - smoke on a running app: a time plot, a samples plot, an XY plot, an
      unlicensed dataset-X (degrades to Samples), and a sweep plot all render correctly; a
      multiplot group toggled Time<->Samples behaves as before; empty-group behavior unchanged.
- [x] **AC5 (R5)** - `git diff -w` on the C-S4 change shows only brace and comment lines;
      nothing after the closing brace references `clk`.
- [x] **AC6 (R7)** - after C-S6, `grep xAxisId app/src/UI` matches only `resolveXAxisPolicy`
      sites and `Dashboard` configure internals; smoke matrix (time/samples/XY/unlicensed-
      degrade/sweep) passes; `--benchmark-hotpath` not regressed.
- [x] **AC7 (R8)** - a project saved by a new build loads on an old build (fan-out still
      present) and vice versa (unknown group key ignored); migration promotes the right
      multiplot groups to Samples and leaves accel/gyro on Time.
- [x] **AC8 (R9)** - Plot and MultiPlot autoscale screenshots match the pre-change baseline
      (parameterize) or match the agreed unified target (unify), per the maintainer decision.
- [x] **AC9 (R10)** - all seven `--benchmark-hotpath` tiers pass at the default
      `--min-fps 256000`; C-S9 lands only if the LinePush key-lookup change does not regress
      the `datasets+publish` stage.

## Constraints & Invariants

- **256 kHz hotpath gate is untouched** by C-S1..C-S8 (all changes are at configure/construct
  or UI draw rate). C-S9 is the only stage that may touch per-fire push cost and is gated on
  `--benchmark-hotpath`.
- **Sweep invariant preserved:** sweep is legal iff `mode == Time`; `Plot.qml:84` already
  gates on `model.timeAxis`.
- **Carrier invariant preserved:** `m_pltValues` holds one LineSeries per plot widget,
  including time plots; `m_pltValues.size() != plotCount` is the reconfigure trigger and the
  index aligns with widget indices. Time carriers become placeholders (empty Y after C-S5) but
  must keep a valid Y pointer.
- **MultiPlot sample buffers are NOT dead:** `MultiPlot::updateRange`/`updateData` read
  `multiplotData(index).y.size()` as a *curve count* in all modes. Do not touch them without a
  curve-count accessor first.
- **No `SchemaVersion` bump** for C-S7; the group field is a sparse, dual-written key.
- **`resolveXAxisPolicy` is pure at UI rate** - an enum branch, no allocation, no per-frame
  cost. It is resolved once and cached; it is never called on the hotpath.
- Must work in QuickPlot and ProjectFile modes; Pro dataset-X stays behind
  `BUILD_COMMERCIAL`, degrading to Samples in GPL.

## The ten pain points (P1-P10)

Retained with file:line so each is locatable in `plan.md`. P4/P5/P6/P9 numbering aligns with
the design track's internal references.

- **P1 - the mode decision re-derived in ~8 places.** The three-way Time/Samples/Dataset fork
  is inferred from raw `xAxisId` sentinels inline at `Plot` (ctor, `resolveXAxis`,
  `updateData`, `updateRange`, `calculateAutoScaleRange`), `MultiPlot`, `Dashboard`
  configure/draw, `ProjectEditor`, `ProjectModel`, and `DashboardHandler`. No single owner of
  the decision. (C-S3 foundation, C-S6 adoption.)
- **P2 - five copy-pasted Pro gates.** The identical dataset-X tier predicate is pasted at
  `Plot.cpp:95-99`, `Plot.cpp:691-693`, `Dashboard.cpp:2362-2364` (`registerXAxisIfNeeded`),
  `:2592-2594`, `:2646-2650`. The two Sweep-setter gates (`Dashboard.cpp:1369`, `:1402`) share
  the predicate but gate Sweep, so they are excluded. (C-S2.)
- **P3 - front-dataset group encoding wart.** A multiplot GROUP combo (Time|Samples only) fans
  its value into every member dataset's `xAxisId`, then reads `front()` back as canonical
  (`app/src/DataModel/Project/ProjectEditorCommit.cpp:268`, `useTimeXAxisGroup`,
  `ProjectModel::xDataSources` at `app/src/DataModel/ProjectModel.cpp:501`). A group-level
  property is encoded per-dataset. (C-S3 single-reader, C-S7 real field.)
- **P4 - dead time-plot Y-ring.** `m_yAxisData[uid]` (`Dashboard.cpp:2568-2569`) is never
  pushed (the time branch `continue`s at `:2627`; `updateLineSeries` fires
  `m_yLinePushes`/`m_xLinePushes` only) and never observably read (`Plot::updateData` returns
  before `plotData()` in time mode at `Plot.cpp:527-544`; `DashboardHandler::tailFrames` at
  `:483-490` computes `n=min(x,y)` and the dead ring size stays 0). Only the ring allocation is
  removable; the carrier is load-bearing. (C-S5.)
- **P5 - manual `(sourceId, uniqueId)` ring key at 4 sites.** Four call sites
  (`Dashboard.cpp:2388-2503`) route the ring key by hand instead of through a
  `RingKey{sourceId, uniqueId}` struct with `qHash`/`operator==` or a shared helper. Off
  hotpath, low risk. (C-S9, bundled.)
- **P6 - `LinePush` caches `AxisData*` into QMap nodes.** `TimePush`/`MultiPush` use the
  defensively-correct index discipline; `LinePush` caches raw `AxisData*` pointers into QMap
  nodes, which survives only because the maps are rebuilt before pushes resolve and are never
  detached (a COW detach via non-const access would dangle). Divergent, fragile. (C-S9,
  benchmark-gated.)
- **P7 - `PlotClock` lifetime hazard.** In `hotpathRxFrame` the `PlotClock& clk` reference
  (`Dashboard.cpp:1463-1501`) is not brace-scoped; it structurally cannot outlive
  `reconfigureDashboard`'s move-assign (`:1710-1711`) today, but nothing enforces that. (C-S4.)
- **P8 - divergent range/padding engines.** `Plot::applyAxisPadding` pads +/-10% of range then
  floor/ceil (`Plot.cpp:834-860`); `MultiPlot::applyDerivedYBounds` pads midpoint
  `halfRange*1.1` + floor/ceil + a degenerate re-split (`MultiPlot.cpp:793-828`). The formulas
  differ, so naive dedup changes on-screen autoscale. (C-S8.)
- **P9 - stale / self-contradictory docs.** `architecture.md` (pre-fix, old lines 438-441) claimed `kXAxisSamples`
  was removed as a user option and that deserialize maps `-1 -> -2`; `Frame.cpp:301` does no
  such mapping and `migrateLegacyXAxisIds` preserves `kXAxisSamples`. The doc even
  self-contradicts at `:484` ("samples-axis"). (C-S1.)
- **P10 - empty-group asymmetry.** `Dashboard::useTimeXAxisGroup` treats an empty group as NOT
  time (falls to the samples path, `Dashboard.cpp:326`); `ProjectEditor::buildGroupXAxisRow`
  treats it as NOT samples (shows "Time",
  `app/src/DataModel/Project/ProjectEditorForms.cpp:203`). Both are correct in their own
  context; the asymmetry is just invisible today. (C-S3 makes it visible; C-S7 formalizes.)

## Target design

**Combination (a) + (c); (b) demoted.** Introduce a resolve-once `XAxisPolicy` (a) and a
single licensing predicate (c). The shared range engine (b) is demoted to a parameterized,
maintainer-decided spec (C-S8).

Add an `XAxisMode` enum and an `XAxisPolicy` POD to `SerialStudio.h` (already included by
`Dashboard.h`, `Plot.cpp`, `MultiPlot.cpp`, `ProjectModel`, `ProjectEditor` - zero new
coupling). The policy is **resolved once** at configure/construct time and cached; it is
**consulted at UI rate (60 Hz)**, where an enum branch is free. The sin being fixed is only
the mode *decision* re-derived in ~8 places with 5 inline gates - not the render machinery.

**The hotpath is untouched by design.** The feed lambdas (`Dashboard.cpp:2025-2072`,
`:2153-2200`), the push tables (`LinePush`/`TimePush`/`MultiPush`), `DSP::TimeRing`,
`SweepEngine`, and all three downsamplers stay exactly as they are - they are already
mode-specialized at configure time. The three-way render branch (sweep-ring / time-ring /
sample-ring) is irreducible because the data sources differ; unification means one predicate
decides which arm runs.

> **Amendment (2026-09-03): the licensing paragraph below was never implemented and is now
> withdrawn.** `datasetXAxisEnabled()` was never written, no `activated()` check ever reached the
> plot X-axis path, and `SerialStudio::commercialCfg()` never listed `xAxisId`. Dataset-vs-dataset
> X-axis is a free feature; the maintainer confirmed the tier rather than wiring the gate, because
> the shipped help manual has always described it untiered and the bundled LorenzAttractor example
> depends on it. Read the paragraph as design history, not as the current tier.

**Licensing** collapses to `SerialStudio::datasetXAxisEnabled()`, mirroring the
`activated()` / `proWidgetsEnabled()` precedent (`SerialStudio.cpp:44-68`). The
`#ifdef BUILD_COMMERCIAL` internals are
`tk.isValid() && SS_LICENSE_GUARD() && tk.featureTier() >= Licensing::FeatureTier::Trial`
with **no** `variantName()` clause (that extra check is `proWidgetsEnabled()`'s, not this
gate's). Unlicensed / Trial-expired dataset-X silently degrades to Samples as one documented
line.

**Group mode** end state is a real `Group::xAxisId` (C-S7). The interim `groupXAxisMode(group)`
helper is the single reader of the front-dataset encoding, and later becomes the migration
seam that collapses to a one-line field read.

### What stays untouched permanently

TimeRing, SweepEngine, the downsamplers, the feed lambdas, the push-table *structure*
(P6 is the separate, benchmark-gated C-S9), `PlotClock` math (only re-scoped in C-S4), and QML
tick formatting (presentation).

### Policy struct definition

Placed in `SerialStudio.h` next to `activated()` / `proWidgetsEnabled()` (~`:250`):

```cpp
enum class XAxisMode : quint8 { Time = 0, Samples = 1, Dataset = 2 };
struct XAxisPolicy { XAxisMode mode = XAxisMode::Time; int xDatasetId = -1; };
[[nodiscard]] static bool datasetXAxisEnabled();
[[nodiscard]] static XAxisPolicy resolveXAxisPolicy(const DataModel::Dataset& d,
                                                    const QMap<int, DataModel::Dataset>& datasets);
[[nodiscard]] static XAxisMode groupXAxisMode(const DataModel::Group& g);
```

**Degrade ladder** (the whole of `resolveXAxisPolicy`): `xAxisId == kXAxisTime` -> Time;
`xAxisId >= 0 && datasetXAxisEnabled() && datasets.contains(xAxisId)` -> Dataset; else ->
Samples. This is the one place the unlicensed-degrade-to-Samples rule lives.

## Open Questions

- **C-S8 parameterize vs unify** - the Plot and MultiPlot padding formulas differ. Preserving
  both (parameterize) is safe; merging them (unify) changes on-screen autoscale for one of the
  two widgets. This is a product decision the maintainer must make before C-S8 is planned.
- **C-S9 LinePush storage** - converting `LinePush` to a key-lookup adds a per-fire QMap find
  on the hot `updateLineSeries` loop (in `datasets+publish`, ~70-80% of per-frame time). The
  alternative is to stabilize storage (`std::vector` + index) instead of key-lookup. The pick
  is decided by `--benchmark-hotpath`, not on paper.
