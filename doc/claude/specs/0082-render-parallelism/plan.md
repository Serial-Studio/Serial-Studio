---
spec: 0082-render-parallelism
phase: plan
status: approved     # draft -> approved (gate before /ss-tasks)
updated: 2026-09-11
---

# Plan 0082 — Render Parallelism

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Read the relevant `doc/claude/` sub-docs and the *actual code*
> before writing this — a plan grounded in a stale mental model is worse than no plan.
> Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

Two independent halves. **ILP:** the four reduction helpers in each lane namespace of
`core/Core/DSPSimd.h` (`SimdSse4`, `SimdNeon`) and `core/Core/DSPSimdAvx2.h` (`SimdAvx2`) gain a
`template<int kChains>` parameter; the loop keeps `kChains` accumulators seeded from `p[0]` and
strides `kChains * width` elements, then folds the chains pairwise with the same `min_pd`/`max_pd`
operand order before the existing per-lane tail. Production calls `kChains = 2`; a new non-gating
ctest instantiates 1 and 2 over 64k elements and prints the ratio (R3), so the single-chain
baseline is the same source, not a copy. **TLP, conditional on the R4 measurement:** a
`Widgets::SpectralStager` (new, `core/Ui/UI/Widgets/Spectral/`) owns each spectral widget's FFT
plan, scratch buffers and a double-buffered result slot; the GUI thread fills its float input with
the existing windowing kernel and `tryStart()`s it on a dedicated `QThreadPool` bound into
`Core::Services` by the composition root; the worker runs the FFT, power/dB, smoothing and the
waterfall's LUT row into the back buffer and publishes a sequence number; the widget adopts the
front buffer on the next display tick. A failed `tryStart()` or an unavailable pool runs the same
stager body inline on the GUI thread, which is also the R11 fallback and the R9 oracle.

## Affected subsystems & files

### Half A — multi-chain reductions (unconditional)

| File | Change |
|------|--------|
| `core/Core/DSPSimd.h` | `SimdSse4::minF64/maxF64/minMaxF64/finiteMinMaxPointF` and the `SimdNeon` twins become `template<int kChains>`: `kChains` accumulators, stride `2 * kChains`, entry guard `n >= 2 * kChains` (SSE, 2-wide) folded pairwise (`acc0 = min(acc1, acc0)`) before the existing horizontal tail. Public kernels call `<2>`. |
| `core/Core/DSPSimdAvx2.h` | Same for `SimdAvx2::minF64/maxF64/minMaxF64/finiteMinMaxPointF<kLane>`: stride `4 * kChains`, guard `n >= 4 * kChains`, pairwise `_mm256_min_pd(acc1, acc0)` fold before the 128-bit halves and the single `_mm256_zeroupper()`. Public kernels call `<2>`; the AVX2 entry guard in `DSPSimd.h` stays `n >= 8`, which is exactly one iteration of the 4-wide two-chain body. |
| `app/tests/tst_dsp_reduction_bench.cpp` (new) | QtTest, non-gating: for each supported level and each reduction, times `kChains = 1` versus `kChains = 2` over 65 536 doubles (ramp payload, 200 repetitions, median), prints `level kernel ns/elem chains=1 chains=2 ratio`, asserts only that both produce bit-identical results to the scalar oracle. Calls the lane helpers directly through the template parameter. |
| `app/tests/CMakeLists.txt` | Register `tst_dsp_reduction_bench` with `ss_add_unit_test`, linking `SerialStudio::Core` and the scalar-oracle TU like `tst_dsp_kernels`. |
| `doc/claude/architecture/kernels.md` | One paragraph under "Runtime Lanes": chains, why min/max may split (order-independent, NaN never wins, NaN seed poisons every chain alike) and why nothing else may. |

`tst_dsp_kernels.cpp` needs no change: its length table (1..33, 63..65, 255, 1024) already
crosses every chain boundary (8 and 16 on SSE/NEON, 16 and 32 on AVX2), and the level column runs
each lane.

### Half B — measurement, then staged workers (built only if R5 passes)

| File | Change |
|------|--------|
| *(no file)* | **R4 measurement**, maintainer-run, recorded below in "Measurement record" before any file in this half is touched. |
| `core/Core/Services.h` / `.cpp` | Add `QThreadPool& renderWorkers` to `Core::Services` (forward-declare `QThreadPool`). Nothing else changes; the struct stays a bag of root-bound references. |
| `app/src/Misc/ModuleManager.cpp` | `bootstrapCoreServices()`: a `static QThreadPool renderWorkers` sized `qBound(1, QThread::idealThreadCount() - 2, 4)` (GUI + pipeline reserved, cap 4), `setThreadPriority(QThread::LowPriority)`, expiry timeout 0 so idle workers park, passed into the static `Services`. |
| `core/Ui/UI/Widgets/Spectral/SpectralStager.h` / `.cpp` (new) | `class SpectralStager : public QRunnable, public std::enable_shared_from_this<SpectralStager>`, `setAutoDelete(false)`. Owns: `kiss_fftr_cfg`, `m_samples` (float in), `m_fftOutput`, `m_db`, `m_smoothed`, `m_logRow`, two `Result` buffers (`db`, `smoothed`, `rgbRow`, `generation`, `configEpoch`) and `std::atomic<quint32> m_published`. API: `configure(size, mode, logTable, lut, dbRange, epoch)`; `float* input()` (GUI writes via `simdWindowedRealFill`); `bool submit(pool, generation)` (holds `m_self = shared_from_this()`, `tryStart`; on false runs `run()` inline and returns false); `const Result* adopt()` (returns the newest unread result or null); `void run() override` (transform into the back buffer, publish, release `m_self`). Never touches a widget or a Qt GUI object. |
| `core/Ui/UI/Widgets/Waterfall.h` / `.cpp` | Replace the inline transform in `updateData()` with: adopt-then-submit. Adopt: if `adopt()` returns a result whose `configEpoch` matches, `paintRowInto` from `rgbRow` (memcpy), marker states from `smoothed`, `update()`; else discard. Submit: if the ingest generation moved and the stager is idle, window into `input()`, `submit()`. Inline path when `submit()` returns false. `allocateFftPlan/rebuildLogColumnTable/rebuildColorLut/setMinDb/setMaxDb` bump the config epoch and `configure()` the stager. Hidden/destroyed: drop the `shared_ptr`; a run in flight finishes on the worker and dies with the last reference. |
| `core/Ui/UI/Widgets/FFTPlot.h` / `.cpp` | Same shape, worker slice = FFT + `computeBinSpectrum` into `Result::db`; GUI keeps `buildLogRenderCurve` / `emitLinearSpectrum`, markers and `downsampleMonotonic` (they write GUI-owned `QVector`s). |
| `core/Ui/CMakeLists.txt` | Register the two new sources. |
| `app/tests/tst_spectral_stager.cpp` (new) + `app/tests/CMakeLists.txt` | Ordering (sequence numbers strictly increase, no duplicate adopt), bit-exact (worker result equals inline `run()` on the same input, every runtime level), stale epoch discarded, cancellation (drop the owner mid-run, pool drains, no crash), fallback (`tryStart` on a saturated pool runs inline and still publishes). |
| `doc/claude/architecture/dashboard.md` | Waterfall/FFT section: the stager, the adopt-then-submit tick, the one-tick latency, the epoch rule, the pool bound, and the log line on fallback. |
| `CLAUDE.md` | One clause in the Threading table: render staging workers read only stager-owned buffers, never a dashboard ring or a publish-path object. |

Not touched: `core/Ui/UI/Dashboard.cpp` (hotpath TU, no change needed since widgets already read
rings on the GUI thread and the generation accessor exists), `DashboardIngest`, anything under
`core/Pipeline/`.

## Architecture & data flow

**Half A.** Pure kernel change. Callers and signatures of the public kernels are unchanged; only
the lane helpers' bodies and their entry guards move. The oracle TU is untouched.

**Half B, one display tick for one waterfall (GUI thread unless stated):**

1. `Dashboard::updated` → `Waterfall::updateData()`.
2. **Adopt.** `stager->adopt()` returns the newest result the worker published since the last
   adopt, or null. If its `configEpoch` matches the widget's current epoch: memcpy `rgbRow` into
   the ring image row (`paintRowInto` reduced to the copy + `markRow`), update marker states from
   `smoothed`, `update()`. If the epoch is stale, discard.
3. **Submit.** If `waterfallGeneration()` moved since the last submit and the stager is idle:
   `simdWindowedRealFill` from the dashboard ring into `stager->input()` (this is the copy; the
   worker never sees the ring), zero-fill the tail, `submit(pool, generation)`.
4. **Worker** (`SpectralStager::run`, pool thread): `kiss_fftr` → `simdPowerSpectrumDb` →
   smoothing → log-column mapping → LUT to `rgbRow` in the back buffer; store `generation` and
   `configEpoch`; `m_published.store(seq, release)`; `m_self.reset()`.
5. Next tick: step 2 adopts it. Steady state: one tick of latency, one row per tick, identical
   pixels to today.

**Fallback.** `submit()` returning false (pool saturated or `renderWorkers` unavailable) runs
`run()` inline; the widget then adopts in the same tick, which is exactly today's behaviour. A
single log line on the first fallback per process (R11).

**Lifetime.** The runnable holds a `shared_ptr` to itself only for the duration of a run. The
widget owns the other reference. Widget destruction or `releaseHistoryImage()` drops the owner
reference; a run in flight completes on the worker, publishes into a slot nobody reads, and the
stager is destroyed on the worker thread when `m_self` resets. No join on the GUI thread, no
widget pointer on the worker.

**Config epochs.** FFT size, window, log-x table, colour LUT and dB range all live in the stager's
`configure()` snapshot with an epoch; a result carries the epoch it was computed under, so a
result from before a reconfigure is discarded on adopt (R8).

## Hotpath & threading impact

- **Touches the hotpath?** Half A touches the kernels the plot draw path calls (finite min/max
  over plot points, min/max over rings). No allocation, no lock, no signal; the change is two
  registers instead of one inside the same loop, and the entry guards move up by one vector
  width. Half B touches only widget code on the GUI thread plus worker threads; it never reads
  `FrameReader`, `CircularBuffer`, `FrameBuilder` or a dashboard ring from a worker. The
  `--benchmark-hotpath` gates cannot move (AC10); the plan still asks for one Auto and one
  `--simd sse4` run before and after Half A as the standing rule for kernel edits.
- **New cross-thread signal/slot?** No. Hand-off is one atomic sequence number per stager
  (release on publish, acquire on adopt) over a two-slot result buffer with a single producer
  (the worker) and a single consumer (the GUI thread). Submission is `QThreadPool::tryStart`.
- **New input to a cached hotpath flag?** No.
- **Timestamp ownership.** Rows are placed by the generation they were submitted with, which is
  the ingest's arrival order; worker completion time is never consulted.
- **Thread priority.** Workers run at `QThread::LowPriority`; the pipeline thread's band and the
  stream workers' registration are untouched (R7). The pool is bounded to `idealThreadCount - 2`,
  capped at 4, floored at 1.

## Data model & persistence

None. No settings key, no project JSON, no schema.

## API / SDK surface

None.

## QML / UI

None. The widgets' QML faces are unchanged; the only observable difference is that a row appears
one tick later than today when the worker path is active, which at the display tick rates in use
is not visible.

## Measurement record (R4, filled in by the maintainer before Half B starts)

Method: on the four-waterfall project with every waterfall and FFT plot live and a device
producing, attach a CPU sampler to the release build for 30 s. Windows: Visual Studio Diagnostic
Tools "CPU Usage" or Windows Performance Recorder's sampled CPU profile, filtered to the GUI
thread. macOS: the `sample` recipe in `common-mistakes.md`. Report: (a) inclusive share of the GUI
thread in `Widgets::Waterfall::updateData` + `Widgets::FFTPlot::updateData` (transform, smoothing,
row paint); (b) the display tick period from the configured fps; (c) whether the tick is being
missed (tick handler wall time versus period).

- GUI-thread transform share: **under 5 %** of the tick budget (threshold: 20 %). Method used:
  the Visual Studio CPU profile had no symbols (plain Release kit, no PDB) and its totals were
  inflated by the debugger session, so the share was bounded with Windows performance counters
  sampled for 16 s while four audio sources fed four waterfalls and their FFT plots: process 55 %
  of one core (12 logical cores); GUI thread 12.9 % of one core, split 5.2 % user mode and 7.7 %
  kernel mode; page faults 81/s. All GUI user-mode work is 0.87 ms of the 16.7 ms tick, and the
  transforms are a subset of it.
- Tick period: 16.7 ms (60 fps); handler wall time: not overrunning (GUI thread 2.2 ms per tick
  in total, p95 of the GUI CPU samples 15.5 % of a core, so 2.6 ms).
- Decision (R5): **no-go**, 2026-09-11. Half B (T6-T12) shelved with these numbers.

## Benchmark record (R3 / AC2, maintainer's Windows MSVC Release build, 2026-09-11)

`tst_dsp_reduction_bench`, 65 536 doubles, 200 repetitions, median ns per element:

| Lane | Kernel | chains=1 | chains=2 | ratio |
|------|--------|----------|----------|-------|
| SSE4 | minF64 | 0.4471 | 0.2289 | 1.95 |
| SSE4 | maxF64 | 0.4471 | 0.2228 | 2.01 |
| SSE4 | minMaxF64 | 0.4578 | 0.2289 | 2.00 |
| SSE4 | finiteMinMaxPointF | 0.4990 | 0.3448 | 1.45 |
| AVX2 | minF64 | 0.2335 | 0.1190 | 1.96 |
| AVX2 | maxF64 | 0.2411 | 0.1266 | 1.90 |
| AVX2 | minMaxF64 | 0.2380 | 0.1221 | 1.95 |
| AVX2 | finiteMinMaxPointF | 0.2975 | 0.2365 | 1.26 |

AC2 met: the three f64 reductions double on both lanes; the point kernel gains less because its
per-iteration work (two loads, an unpack, a compare, two blends) already overlaps the compare
latency. Two chains stay the production choice. `tst_dsp_kernels`: 43 610 bit-exact rows passed
at every level on the same build.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| How the benchmark gets a single-chain baseline | Keep a copy of the old loop in the test; build twice; template chain count | **Template chain count.** One source, both instantiations, no intrinsics duplicated in a test, and production picks `<2>` explicitly. |
| Chain count | 2; 4 | **2** now. Two chains already halve the latency bound; four costs registers on SSE (the point kernel holds two accumulators per chain plus constants) and the benchmark will say whether more pays. Template makes 4 a one-token change. |
| Where the pool lives | `QThreadPool::globalInstance()`; a Meyers static in `core/Ui`; a root-bound `QThreadPool` in `Core::Services` | **Root-bound in `Core::Services`.** Follows `AsyncToolRunner`'s own-pool precedent without a new singleton, keeps the census flat, and lets the root size it once next to the other services. |
| Submission primitive | `QtConcurrent::run` per tick; persistent `QRunnable` + `tryStart` | **Persistent runnable.** No per-tick allocation, natural inline fallback when the pool is busy, autoDelete off. |
| What crosses to the worker | The ring itself; a windowed float copy | **Windowed copy.** The ring is mutated by the ingest on the GUI thread every tick; the windowing kernel already produces the float array, so the copy is work the GUI does today, not new work. |
| Worker slice for the FFT plot | Whole `updateData`; FFT + bin spectrum only | **FFT + bin spectrum.** Curve building and downsampling write GUI-owned `QVector`s and are cheap relative to the transform; keeping them on the GUI avoids double-buffering the plot data. |
| Row cadence when a transform outlasts a tick | Queue every tick's input; coalesce | **Coalesce.** One transform in flight per widget; a tick that finds it busy submits nothing and the next submit uses the newest ring. This is the same "one row per tick with new samples" contract as today, minus a GUI stall. Named here because R6's "never dropped" is read as "no produced row is lost", not "one transform per tick regardless of cost". |
| Cancellation | Blocking join on hide/destroy; self-owning run | **Self-owning run.** A `shared_ptr` held for the run's duration; the GUI never waits, a stale result is dropped by epoch. |

## Risks & mitigations

- **Half A changes a bit somewhere.** Only min and max split; the fold keeps the operand order
  the SSE tail already used; the level-column suite compares every length at every level against
  the scalar oracle, and the signed-zero exception is already tested separately.
- **The benchmark shows no gain.** AC2 reverts the chain count to 1 via the template default and
  records the numbers; the template stays as documentation of what was measured.
- **Half B built without a measurement.** The task list places R4 as a maintainer task with the
  record above as its output; no Half B task starts until the go/no-go line is filled.
- **Worker touches a GUI object.** The stager's interface is raw buffers only; `tst_spectral_stager`
  runs the stager with no widget at all, and the CLAUDE.md clause names the rule for future
  edits.
- **Use-after-free on widget teardown.** Self-owning run plus epoch discard; the cancellation test
  drops the owner mid-run under the sanitizer legs.
- **Pool starves the pipeline.** Low priority, bounded count, and the pipeline thread's MMCSS or
  nice band is registered from its own thread and unaffected; AC6 checks the thread listing.
- **Silent-breakage classes from `common-mistakes.md`.** The idle gate (arrival generation) is
  reused unchanged, so a stationary signal keeps scrolling; the hidden-release behaviour is
  unchanged; no cached hotpath flag or queued hop is added.

## Test & verification plan

- **AC1** — `ctest -R tst_dsp_kernels` at every level (Linux CI, maintainer on MSVC once).
- **AC2** — `ctest -R tst_dsp_reduction_bench -V` prints the ratios; maintainer records them.
- **AC3 / AC4** — the Measurement record above, filled by the maintainer.
- **AC5** — maintainer: same project, worker path and forced-inline path, compare a texture dump
  or screenshot after N ticks; FFT markers agree.
- **AC6** — maintainer's thread listing with the four-waterfall project live.
- **AC7** — `tst_spectral_stager` cancellation and epoch slots under ASan/TSan (`build/asan`,
  `build/tsan` legs already run every ctest), plus a maintainer loop of hide/show/resize/close.
- **AC8** — maintainer's page-fault script, steady state with workers active.
- **AC9** — `tst_spectral_stager` fallback slot (saturated pool) plus the `renderWorkers`-absent
  build switch the stager honours.
- **AC10** — the existing `--benchmark-hotpath` CI gates, unchanged.
- **Static:** `python scripts/code-verify.py --check` on every touched file; `layer-verify.py`
  (new `core/Ui` sources owned, `Core::Services` gains a Qt forward declaration only);
  `--singleton-census --check` flat; `--tu-census --check` (`DSPSimd.h` grows by the chain
  templates; stays under 1500); `claim-verify.py` after the doc edits; `qt-cpp-review` before
  handoff; `sanitize-commit.py` before commit.
