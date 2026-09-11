---
spec: 0082-render-parallelism
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-09-11
---

# Tasks 0082 — Render Parallelism

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
- The assistant never compiles. Every "build", "ctest" or "benchmark" verify step is the
  maintainer's; the assistant's verify is the linter plus a read-back against the plan.
- **Half B (T5 onward) does not start until T5's measurement record says go.** A no-go closes
  T6 through T12 as shelved with the number recorded.

## Tasks

### Half A — multi-chain reductions

### T1 — Chain-count template on the SSE4 and NEON reductions

- **Files:** `core/Core/DSPSimd.h`
- **Does:** `SimdSse4::minF64`, `maxF64`, `minMaxF64`, `finiteMinMaxPointF<kLane>` and the
  `SimdNeon` twins gain `template<int kChains>` (`static_assert(kChains == 1 || kChains == 2)`).
  Each keeps `kChains` accumulators seeded from `p[0]` (or the caller's `lo`/`hi` for the point
  kernel), strides `2 * kChains` elements per iteration with the entry guard `n >= 2 * kChains`,
  folds chain 1 into chain 0 with the same operand order the loop uses (`min_pd(acc1, acc0)`,
  `vbslq(vcltq(acc1, acc0), acc1, acc0)`), then runs the existing horizontal tail unchanged.
  The public kernels call `<2>`; the `n >= 4` guards in the public kernels stay, since the
  two-chain SSE body consumes four per iteration. Binding invariants named here: min and max
  are the only operations that may split (order-independent; a NaN element never wins in any
  chain and a NaN seed poisons every chain alike); no other kernel gains a chain; the scalar
  tail and the oracle TU are untouched; nothing allocates; the header stays under the 1500-line
  census cap.
- **Verify:** `python scripts/code-verify.py --check core/Core/DSPSimd.h`;
  `python scripts/code-verify.py --tu-census --check`; read-back that every fold uses the
  loop's operand order.
- **Deps:** none
- [x] done (header 1300 lines, census unchanged)

### T2 — Chain-count template on the AVX2 reductions

- **Files:** `core/Core/DSPSimdAvx2.h`
- **Does:** `SimdAvx2::minF64`, `maxF64`, `minMaxF64`, `finiteMinMaxPointF<kLane>` gain the
  same `template<int kChains>`: stride `4 * kChains`, guard `n >= 4 * kChains`, pairwise
  `_mm256_min_pd(acc1, acc0)` / `_mm256_max_pd(acc1, acc0)` fold, then the existing split into
  128-bit halves, the single `_mm256_zeroupper()` after every 256-bit value is dead, and the
  `fold128Min/Max` tail. The `DSPSimd.h` call sites pass `<2>`; their `n >= 8` guard stays
  (one iteration of the two-chain body). Binding invariants: zeroupper still exactly once per
  body and only after both chains and both accumulators are reduced to 128-bit halves; every
  body stays `SS_NEVER_INLINE SS_TARGET_AVX2`; no FMA.
- **Verify:** `python scripts/code-verify.py --check core/Core/DSPSimdAvx2.h core/Core/DSPSimd.h`;
  `grep -n zeroupper core/Core/DSPSimdAvx2.h` shows one call per reduction body, after the
  extracts.
- **Deps:** T1
- [x] done (one zeroupper per body, each after both chains fold and both halves extract)

### T3 — Reduction micro-benchmark

- **Files:** `app/tests/tst_dsp_reduction_bench.cpp` (new), `app/tests/CMakeLists.txt`
- **Does:** QtTest suite, non-gating: for each level in `DSP::supportedSimdLevels()` and each
  of the four reductions, fills 65 536 doubles (ramp payload), times `kChains = 1` and
  `kChains = 2` through the lane helpers directly (200 repetitions, median of per-call
  `QElapsedTimer` nanoseconds), prints one line per kernel and level
  (`level kernel ns/elem chains=1 chains=2 ratio`), and asserts only that both chain counts
  return bit-identical results to the `DspSimdScalar` oracle for that input. Registered with
  `ss_add_unit_test`, linking `SerialStudio::Core` and the oracle TU exactly as
  `tst_dsp_kernels` does, placed before the aggregate target.
- **Verify:** `python scripts/code-verify.py --check app/tests/tst_dsp_reduction_bench.cpp`;
  maintainer: `ctest -R tst_dsp_reduction_bench -V` and record the ratios in `plan.md` under
  a new "Benchmark record" line (AC2). Ratio below 1.5 on every level means revert the public
  call sites to `<1>` and keep the template as the record.
- **Deps:** T2
- [x] done (maintainer ctest run and ratio record pending)

### T4 — Half A docs, gates and hotpath sanity

- **Files:** `doc/claude/architecture/kernels.md`
- **Does:** One paragraph under "Runtime Lanes" naming the chain template, the two-chain
  production choice, why min and max may split and why nothing else may, and where the
  benchmark lives. Then the gate set: `code-verify --check`, `--tu-census --check`,
  `claim-verify.py`; maintainer runs `ctest -R tst_dsp_kernels` (AC1) and one
  `--benchmark-hotpath` on Auto and one pinned `--simd sse4` as the standing rule for any
  kernel edit (AC10, expected flat).
- **Verify:** every gate exits 0; the ctest run is green; both benchmark runs pass their gates.
- **Deps:** T3
- [x] done (gates clean; `tst_dsp_kernels` 43 611 rows green on MSVC with the chains; benchmark Auto and `--simd sse4` both flat against each other, see spec 0081 T12 for the numbers and the local 4x-tier caveat)

### Half B — measurement, then staged workers

### T5 — R4 measurement (maintainer)

- **Files:** `doc/claude/specs/0082-render-parallelism/plan.md` (Measurement record)
- **Does:** On the four-waterfall project with every waterfall and FFT plot live and a device
  producing, sample the release build for 30 s (Visual Studio CPU Usage or WPR on Windows,
  `sample` on macOS), filtered to the GUI thread. Fill the record: inclusive share of
  `Widgets::Waterfall::updateData` plus `Widgets::FFTPlot::updateData`, the tick period, the
  handler wall time p50/p95, and the go/no-go line against the 20 % threshold.
- **Verify:** the three blanks in the record are filled and dated. Go opens T6; no-go marks T6
  through T12 shelved in this file and sets the spec's Half B to shelved with the number.
- **Deps:** none (may run in parallel with Half A)
- [x] done (no-go: GUI user-mode share under 5 % of the tick, tick not overrunning; see the plan's Measurement record)

### T6 — Render worker pool bound into the services

- **Files:** `core/Core/Services.h`, `core/Core/Services.cpp`, `app/src/Misc/ModuleManager.cpp`
- **Does:** `Core::Services` gains `QThreadPool& renderWorkers` (forward declaration only in
  the header). `bootstrapCoreServices()` creates a `static QThreadPool` sized
  `qBound(1, QThread::idealThreadCount() - 2, 4)`, `setThreadPriority(QThread::LowPriority)`,
  `setExpiryTimeout(0)`, and passes it into the static `Services`. Binding invariants: the
  pool is constructed before any module and never reached through a singleton; the pipeline
  thread's priority registration is untouched; the headless, selftest and benchmark roots call
  the same `bootstrapCoreServices()`, so every root binds it.
- **Verify:** `python scripts/code-verify.py --check` on the three files;
  `python scripts/layer-verify.py`; `--singleton-census --check` flat.
- **Deps:** T5 (go)
- [ ] shelved 2026-09-11 (T5 no-go)

### T7 — `Widgets::SpectralStager`

- **Files:** `core/Ui/UI/Widgets/Spectral/SpectralStager.h` (new),
  `core/Ui/UI/Widgets/Spectral/SpectralStager.cpp` (new), `core/Ui/CMakeLists.txt`
- **Does:** `QRunnable` (autoDelete off) plus `enable_shared_from_this`. Owns the FFT plan,
  `m_samples`, `m_fftOutput`, `m_db`, `m_smoothed`, `m_logRow`, two `Result` slots (`db`,
  `smoothed`, `rgbRow`, `generation`, `configEpoch`) and `std::atomic<quint32> m_published`.
  `configure(...)` snapshots size, mode (FFT-only or waterfall row), log-column table, LUT,
  dB range and epoch, reallocating only there. `float* input()`; `bool submit(QThreadPool&,
  quint64 generation)` sets `m_self = shared_from_this()`, `tryStart`s, and on false runs
  `run()` inline and returns false; `const Result* adopt()` returns the newest unread slot or
  null (acquire on `m_published`); `run()` transforms into the back slot, publishes (release),
  resets `m_self` last. Binding invariants: the worker touches only stager-owned buffers, never
  a widget, a dashboard ring or a Qt GUI object; no allocation in `run()`, `submit()` or
  `adopt()`; single producer (worker) and single consumer (GUI) per stager; `m_self` is the only
  thing keeping the stager alive during a run, so dropping the owner never blocks.
- **Verify:** `python scripts/code-verify.py --check` on both files; `layer-verify.py` (owned
  sources, no upward include).
- **Deps:** T6
- [ ] shelved 2026-09-11 (T5 no-go)

### T8 — Stager unit suite

- **Files:** `app/tests/tst_spectral_stager.cpp` (new), `app/tests/CMakeLists.txt`
- **Does:** Slots: `sequenceIncreasesAndNeverRepeats` (submit N inputs through a real
  `QThreadPool`, adopt after each completion, sequence strictly increasing, no double adopt);
  `workerMatchesInline` (same input through `submit` and through inline `run()`, every result
  buffer bit-identical, at every runtime level via `DSP::setActiveSimdLevel`);
  `staleEpochIsDiscarded` (reconfigure between submit and adopt, adopt returns null);
  `droppingTheOwnerMidRunIsSafe` (submit, reset the owner `shared_ptr`, `waitForDone`, no
  crash, no leak under ASan); `saturatedPoolRunsInline` (pool with one thread blocked, `submit`
  returns false and a result is adoptable immediately). Registered with `ss_add_unit_test`.
- **Verify:** `python scripts/code-verify.py --check app/tests/tst_spectral_stager.cpp`;
  maintainer: `ctest -R tst_spectral_stager` locally; CI runs it under the ASan and TSan legs
  (AC7, AC9).
- **Deps:** T7
- [ ] shelved 2026-09-11 (T5 no-go)

### T9 — Waterfall adopt-then-submit

- **Files:** `core/Ui/UI/Widgets/Waterfall.h`, `core/Ui/UI/Widgets/Waterfall.cpp`
- **Does:** The widget owns a `std::shared_ptr<SpectralStager>` and a config epoch.
  `updateData()` becomes: adopt (matching epoch → memcpy `rgbRow` into the image row via a
  reduced `paintRowInto`, `markRow`, marker states from `smoothed`, `update()`; stale epoch →
  discard), then submit (arrival generation moved and stager idle → `simdWindowedRealFill` into
  `input()`, zero-fill the tail, `submit(Core::services().renderWorkers, generation)`; false →
  the inline result is adopted in the same tick). `allocateFftPlan`, `rebuildLogColumnTable`,
  `rebuildColorLut`, `setMinDb`, `setMaxDb` bump the epoch and `configure()` the stager.
  `releaseHistoryImage()` and the destructor drop the owner reference. Campbell mode adopts
  the row into the Y-indexed position as today. One `qInfo` line on the first inline fallback
  per process (R11). Binding invariants: the arrival-generation idle gate from the 2026-09-11
  fix is reused unchanged (no new samples means no submit and no FFT); the ring is read only on
  the GUI thread, into the stager's input; rows are adopted in submit order; no per-tick
  allocation.
- **Verify:** `python scripts/code-verify.py --check` on both files; maintainer observation
  with a steady sensor (still scrolls), a hide/show cycle and an FFT-size change while live.
- **Deps:** T8
- [ ] shelved 2026-09-11 (T5 no-go)

### T10 — FFT plot adopt-then-submit

- **Files:** `core/Ui/UI/Widgets/FFTPlot.h`, `core/Ui/UI/Widgets/FFTPlot.cpp`
- **Does:** Same shape with the FFT-only stager mode: the worker produces `Result::db`; the
  GUI adopts it into the bin spectrum, then runs `buildLogRenderCurve` / `emitLinearSpectrum`,
  markers and `downsampleMonotonic` as today (they write GUI-owned `QVector`s). Plan rebuilds
  and dB-range changes bump the epoch. Same fallback and lifetime rules as T9.
- **Verify:** `python scripts/code-verify.py --check` on both files; maintainer: FFT peak
  markers agree between the worker path and the forced-inline path on the same input (AC5).
- **Deps:** T9
- [ ] shelved 2026-09-11 (T5 no-go)

### T11 — Half B docs

- **Files:** `doc/claude/architecture/dashboard.md`, `CLAUDE.md`
- **Does:** `dashboard.md`: the stager, adopt-then-submit, the one-tick latency, the epoch
  rule, the pool bound and priority, the fallback line, and that the arrival gate is unchanged.
  `CLAUDE.md`: one clause in the Threading & Hotpath list: render staging workers read only
  stager-owned buffers, never a dashboard ring or a publish-path object, and hand results back
  through one atomic per stager.
- **Verify:** `python scripts/claim-verify.py` clean; `python scripts/documentation-verify.py`.
- **Deps:** T10
- [ ] shelved 2026-09-11 (T5 no-go)

### T12 — Half B gates, review and observations

- **Files:** none new
- **Does:** `code-verify --check` on every touched file, `layer-verify.py`,
  `--singleton-census --check`, `--tu-census --check`, `--dup-census --check`,
  `claim-verify.py`; `qt-cpp-review` on the C++ diff; counterfactual check: the rule this diff
  most risks is a worker touching GUI-owned state, and the evidence is the stager's interface
  (raw buffers only) plus `tst_spectral_stager` running it with no widget. Maintainer
  observations: AC5 (identical images worker versus inline), AC6 (thread listing, pipeline band
  unchanged, worker count equals the bound), AC7 (hide/show/resize/close loop), AC8 (page-fault
  script flat), AC9 (pool-absent switch), AC10 (benchmark gates unchanged).
- **Verify:** every command exits 0; each observation recorded against its AC in `spec.md`.
- **Deps:** T11
- [ ] shelved 2026-09-11 (T5 no-go)

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` is met and checked off there, or Half B's are
      marked shelved with the T5 number.
- [ ] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [ ] `qt-cpp-review` run on the C++ diff; findings addressed or noted.
- [ ] `--benchmark-hotpath` not regressed: Auto and `--simd sse4` runs after Half A pass their
      gates; AC10 confirms Half B cannot move them.
- [ ] Relevant `pytest` targets identified for the maintainer: none apply (no API surface);
      the ctest tier carries `tst_dsp_kernels`, `tst_dsp_reduction_bench`, `tst_spectral_stager`.
- [ ] `python scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [ ] Diff is *what was asked, and only that* — no scope creep, no foreign files touched.
- [ ] `spec.md` status set to `done` (or Half B `shelved` with the measurement).
