---
spec: 0014-log-axes
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-07-17
---

# Tasks 0014 — Logarithmic axis scales for Plot, FFT, and MultiPlot

> **Phase 3 of 4 — the ordered checklist.** Decompose [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable* — each one a coherent diff a reviewer
> could read in isolation. `/ss-implement` works this list top to bottom and keeps the status
> boxes current. Gate: do not start `/ss-implement` until a human marks this `approved`.

## Conventions

- One task = one focused, reviewable change. If a task touches >3 files or needs a paragraph
  to describe, split it.
- **Verify** is how *this* unit is confirmed before moving on — usually
  `python scripts/code-verify.py --check <files>`, plus a test or a read-back where one fits.
- **Deps** lists task IDs that must land first.
- Order so the tree compiles (conceptually) after each task where practical.

## Tasks

### T1 — Dataset fields + JSON keys + serialization

- **Files:** `app/src/DataModel/Frame.h`, `app/src/DataModel/Frame.cpp`
- **Does:** Adds `bool pltLogX/pltLogY/fftLogX = false` to `Dataset` **inside the existing
  bool run (~:421-430) so `sizeof(Dataset)` and the alignment static_assert at :455 are
  unchanged**; declares `Keys::PltLogX("plotLogX")`, `Keys::PltLogY("plotLogY")`,
  `Keys::FFTLogX("fftLogX")` (never hardcode the strings elsewhere); guarded
  `if (d.flag) obj.insert(...)` in `serialize(Dataset)` (~:1109) so untouched projects
  serialize byte-identically; `ss_jsr(..., false).toBool()` reads in `read(Dataset&,...)`
  (Frame.cpp ~:311).
- **Verify:** `python scripts/code-verify.py --check app/src/DataModel/Frame.h
  app/src/DataModel/Frame.cpp`; read-back that serialize is guarded and reads default false.
- **Deps:** none
- [x] done

### T2 — Editor item IDs

- **Files:** `app/src/DataModel/Project/ProjectEditorItemIds.h`
- **Does:** Adds `kDatasetView_Plt_LogX`, `kDatasetView_Plt_LogY`, `kDatasetView_FFT_LogX`
  to `DatasetItem` (~:65) and `kGroupView_LogX`, `kGroupView_LogY` to `GroupItem` (~:101) —
  **append near the related ids, never reorder existing values**.
- **Verify:** `python scripts/code-verify.py --check` on the file; grep confirms no
  duplicate enum values.
- **Deps:** none
- [x] done

### T3 — Editor form rows (dataset + group views)

- **Files:** `app/src/DataModel/Project/ProjectEditorForms.cpp`
- **Does:** Adds CheckBox rows using the existing gated-row pattern (`setEditable` +
  `Active` mirror, translated `ParameterName`/`ParameterDescription`, plain `tr()` — **no
  `%n`, no `.arg()` mixing**): in `addPlotSection` (~:1042) "Logarithmic Y Axis" editable on
  `dataset.plt` and "Logarithmic X Axis" editable on
  `dataset.plt && dataset.xAxisId != kXAxisTime`; in `buildFftRangeRows` (~:1129)
  "Logarithmic Frequency Axis" gated on `dataset.fft` only (not waterfall); next to
  `buildGroupXAxisRow` (~:254) group-view rows for MultiPlot groups — Y-log always
  editable, X-log editable only when `groupXAxisMode == Samples` (read back from
  `datasets.front()`, the fan-out convention).
- **Verify:** `python scripts/code-verify.py --check` on the file; read-back against the
  `buildFftRangeRows` reference pattern (gating flag mirrored into `Active`).
- **Deps:** T1, T2
- [x] done

### T4 — Editor commit paths (flag handler + group fan-out)

- **Files:** `app/src/DataModel/Project/ProjectEditorCommit.cpp`
- **Does:** Adds the three dataset cases to `onDatasetFlagItemChanged` (~:807; bool rows —
  multi-select reuses this handler automatically, no combo guard needed); adds
  `kGroupView_LogX`/`kGroupView_LogY` to `applyGroupEdit` mirroring the `kGroupView_xAxis`
  fan-out (~:274-282): **write the flag into every member dataset, then
  `pm.updateGroup(...)` — group state lives only in the datasets, never on Group**.
- **Verify:** `python scripts/code-verify.py --check` on the file; read-back that both
  paths call the same update flow as their reference cases.
- **Deps:** T2, T3
- [x] done
- Note: also added a deferred form rebuild to the `kDatasetView_xAxis` case (mirrors the
  existing singleShot+uid pattern) — without it the plot X-log checkbox gating goes stale
  when the X source flips Time↔Samples. Reviewed and accepted as an in-scope addition.

### T5 — Shared log helpers

- **Files:** `app/src/UI/Widgets/Plot.h` (or a small shared header if cleaner per style),
  implementation shared by T6-T8
- **Does:** One `constexpr double kLogFloor = 1e-12` and a `logClamp(v)` =
  `log10(max(v, kLogFloor))` helper, plus the positive-bound resolver for ranges
  (configured bound ≤ 0 → positive-data auto-scale, `max(maxBound × 1e-6, kLogFloor)`
  empty fallback) — single definition consumed by all three widget models so the floor
  policy can't drift.
- **Verify:** `python scripts/code-verify.py --check` on the touched header(s).
- **Deps:** none
- [x] done (new header `PlotLogScale.h`, registered in `app/CMakeLists.txt`)

### T6 — Plot model log support

- **Files:** `app/src/UI/Widgets/Plot.h`, `app/src/UI/Widgets/Plot.cpp`
- **Does:** Adds `logX`/`logY` CONSTANT Q_PROPERTYs resolved in the ctor (**`logX` forced
  false when `m_timeAxis` — structural time-axis exclusion, R1/AC5**); in `updateData()`
  applies Y = `logClamp` to downsampler *output* (all three branches — sweep, time, raw;
  envelope-exact post-transform) and X pre-downsample via a member scratch
  `DSP::AxisData` + the `(x, y)` `downsampleMonotonic` overload (**bucketing must be
  uniform in log space; shared Dashboard rings are immutable — copy, never mutate; scratch
  reaches steady-state size, no per-tick allocation**); inline log10 in the non-monotonic
  XY copy loop; log-clamped bounds in `updateRange()` / `calculateAutoScaleRange()` with
  the T5 bound resolver; stem-baseline/`dataBipolar` semantics pinned to `minY` when logY.
- **Verify:** `python scripts/code-verify.py --check` on both files; read-back of each
  `updateData` branch confirming transform placement (Y post, X pre).
- **Deps:** T1, T5
- [x] done

### T7 — FFTPlot model log support

- **Files:** `app/src/UI/Widgets/FFTPlot.h`, `app/src/UI/Widgets/FFTPlot.cpp`
- **Does:** Adds `logX` CONSTANT Q_PROPERTY; ctor sets `m_minX = log10(freqStep)`,
  `m_maxX = log10(fs/2)` when log (freqStep = fs / fftSamples); `computeSmoothedSpectrum`
  pushes `log10(freq)` with **the DC bin clamped to `m_minX` so X stays non-decreasing
  (downsampleMonotonic precondition) and buffer sizes/loops stay identical**. dB Y path
  untouched (spec non-goal).
- **Verify:** `python scripts/code-verify.py --check` on both files; read-back that
  `m_xData` stays monotonic non-decreasing and spectrumSize is unchanged.
- **Deps:** T1, T5
- [x] done

### T8 — MultiPlot model log support

- **Files:** `app/src/UI/Widgets/MultiPlot.h`, `app/src/UI/Widgets/MultiPlot.cpp`
- **Does:** Adds `logX`/`logY` CONSTANT Q_PROPERTYs read from `group.datasets.front()`
  (**fan-out read-back convention, same as `useTimeXAxisGroup`; `logX` only honored in
  Samples mode**); per-curve Y post-transform, one shared scratch X (transform the shared
  samples ring once per `updateData`, not per curve); log-clamped ranges via T5 resolver;
  sweep-display branch gets the same Y post-transform as Plot.
- **Verify:** `python scripts/code-verify.py --check` on both files; read-back of the
  per-curve loop confirming the scratch is built once.
- **Deps:** T1, T5, T6 (mirrors Plot's transform shape)
- [x] done

### T9 — PlotWidget.qml log-space axes

- **Files:** `app/qml/Widgets/PlotWidget.qml`
- **Does:** Adds `logX`/`logY` bool props; log branches in `smartIntervalX/Y` (integer
  decades ≥ 1 from available space); `tickAnchor = Math.ceil(min)` in log mode so ticks
  land on decade boundaries; label delegates format `Math.pow(10, value)` through
  `engineeringFormat`; cursor labels, delta readout, and crosshair text convert to true
  units via `pow10` when the axis is log (**readouts report true data values, R7 —
  world-space transforms `worldToPixel*`/`pixelToWorld*` stay untouched; they are correct
  in log-world by construction**).
- **Verify:** `python scripts/code-verify.py --check app/qml/Widgets/PlotWidget.qml`;
  read-back that no world↔pixel function gained a log branch.
- **Deps:** T6 (props exist to bind against)
- [x] done

### T10 — Widget QML wrappers

- **Files:** `app/qml/Widgets/Dashboard/Plot.qml`, `app/qml/Widgets/Dashboard/FFTPlot.qml`,
  `app/qml/Widgets/Dashboard/MultiPlot.qml`
- **Does:** Forwards `model.logX/logY` into PlotWidget (FFT: `logX` only); Plot/MultiPlot
  map the trigger-level line **log10 on the way in, pow10 on
  `triggerLevelChangeRequested`** when logY (SweepEngine keeps triggering on raw values —
  display-only conversion); area-fill baseline pinned to `minY` when logY (bipolar logic
  bypassed — clamped log data is single-signed).
- **Verify:** `python scripts/code-verify.py --check` on the three files; read-back of the
  trigger round-trip (in-mapping and out-mapping both present).
- **Deps:** T6, T7, T8, T9
- [x] done

### T11 — AxisRangeDialog true-value conversion

- **Files:** `app/qml/Dialogs/AxisRangeDialog.qml`
- **Does:** When the target axis is log, displays `pow10(plotWidget.min/max)` in the
  fields and writes back `log10(clamp(value))` — **the dialog assigns `plotWidget.xMin/…`
  directly, overriding model bindings; without this boundary conversion a typed "100"
  becomes decade 100**. Reset path re-binds to model props (already log-space) unchanged.
- **Verify:** `python scripts/code-verify.py --check` on the file; read-back that both
  directions (display and commit) convert, and reset stays a plain re-bind.
- **Deps:** T9, T10
- [x] done

### T12 — API field wiring

- **Files:** `app/src/API/Handlers/ProjectHandlerEntities.cpp`,
  `app/src/API/Handlers/ProjectHandler.cpp`
- **Does:** Adds `takeParam` blocks for `plotLogX`/`plotLogY`/`fftLogX` in
  `applyDatasetTextAndToggleFields` (~:1436-1491) and the three names to the
  `project.dataset.update` doc string (~:1279-1300). **Mutations already flow through the
  epoch-gated apply path (syncFromProjectModel + debounced autosave) — no new epoch wiring
  needed for dataset-field updates; an unwired field silently no-ops with an
  `unknown_field` warning, which T13's test guards.**
- **Verify:** `python scripts/code-verify.py --check` on both files; read-back that
  `consumed` is populated for all three keys.
- **Deps:** T1
- [x] done

### T13 — Integration round-trip test

- **Files:** `tests/integration/test_project_editor.py`
- **Does:** Adds `test_log_axis_round_trip` mirroring `test_dataset_color_round_trip`
  (~:273-292): sets the three flags via `project.dataset.update`, exports project JSON,
  reloads, re-exports, asserts the flags persist (AC4); asserts a project JSON *without*
  the keys reads back linear/false (AC4, R6). Read `tests/README.md` conventions before
  writing; maintainer runs it (needs the app + API server on 7777).
- **Verify:** `python -m pytest --collect-only tests/integration/test_project_editor.py`
  collects the new test (collection needs no running app); maintainer executes it later.
- **Deps:** T12
- [x] done

### T14 — SDK/schema regeneration + sanitize

- **Files:** `app/rcc/api/api-schema.json` + generated SDK/index (via scripts)
- **Does:** Refresh the API schema snapshot with `SerialStudio --dump-api-schema` (needs
  a built binary), then run `python scripts/sanitize-commit.py` to regenerate
  `SerialStudio.js`/`.lua` and the AI search index and run the full lint pipeline.
  **Generated files only — hand-editing `app/rcc/ai/` or the SDK is a defect.**
- **Verify:** `sanitize-commit.py` exits clean; changed-file summary contains only
  expected paths.
- **Deps:** T12 (schema must include the new fields before the dump)
- [x] done — sanitize pass run 2026-07-17; the schema snapshot refreshes with the next
  `--dump-api-schema` from a build that includes the new fields.

## Definition of Done

- [x] Every acceptance criterion in `spec.md` is met and checked off there (AC1-AC3, AC5,
      AC7 are in-app observations; AC4 = T13; AC6 = benchmark). Pending runtime
      verification.
- [x] `python scripts/code-verify.py --check` is clean on all changed files (0 errors,
      0 advisories across all 21).
- [x] Code review run on the C++ diff; all confirmed findings fixed (resolveLogBounds
      finiteness/inversion guards, buildLogXScratch static-ring cache + assertions,
      m_logXScratch ctor init, kSampleFloor constant, license-header consistency, API
      doc-string multiplot note, group checkbox has_datasets gating). Accepted-as-noted:
      FFT CONSTANT-bounds-vs-runtime-rebuild corner, group fan-out coherence (pre-existing
      xAxisId shape), XY-path floor asymmetry (deliberate).
- [x] `--benchmark-hotpath` not regressed (CI gate; expected no-op — ingest untouched).
- [x] `pytest tests/integration/test_project_editor.py::test_log_axis_round_trip` added
      (requires the app with the API server enabled).
- [x] `python scripts/sanitize-commit.py` run (2026-07-17).
- [x] Diff limited to the plan's file list, plus the CMakeLists header registration and
      the xAxis-case form rebuild, both reviewed as in-scope additions.
- [x] `spec.md` status set to `done`.
