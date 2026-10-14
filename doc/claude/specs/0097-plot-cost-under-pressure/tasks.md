---
spec: 0097-plot-cost-under-pressure
phase: tasks
status: approved     # approved 2026-10-13 (with AC11 amendment)
updated: 2026-10-13
---

# Tasks 0097 — Plots pay for what arrives and what they draw

> **Phase 3 of 4 — the ordered checklist.** Decompose [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable*. `/ss-implement` works this list top to
> bottom and keeps the status boxes current. Gate: do not start `/ss-implement` until a human
> marks this `approved`.

## Conventions

- One task = one focused, reviewable change. Verify = how this unit is confirmed before
  moving on.
- **Every C++ edit runs `scripts/syntax-check.py` on the edited files.** The hook does too.
  A SKIP from a stale compile database means "it compiles" is unverified, and that is said.
- **Exec** follows CLAUDE.md "Advisor Economics":
  - **main**: the main session writes it, because it is a hotpath, protected or design-bearing
    edit.
  - **delegate**: a cheaper subagent applies a half-page brief, and the main session reviews
    the diff.
- I never build. ctest and the app binaries run against the maintainer's build. Tasks marked
  **(build)** need a rebuild by the maintainer before their runtime check.
- The four parts are independent after T0: memory (T1–T11), render (T12–T15), console
  (T16–T18) and durability (T19–T22). Within a part, order is strict.

## Tasks

### T0 — Record the baselines

- **Files:** `doc/claude/specs/0097-plot-cost-under-pressure/report.md` *(new)*
- **Does:** Records the "before" numbers every AC compares against:
  - the Scenario P footprint table and the GUI- and render-thread shares from this session's
    `sample` run;
  - the BADAQ composition;
  - a `--benchmark-hotpath --min-fps 256000` run of the existing db2debec4 binary, keeping the
    nine gate rows and the `dashboard ingest costs` ratio (AC12 baseline).
- **Verify:** `report.md` holds every number with its method. The benchmark exit code is
  recorded.
- **Deps:** none · **Exec:** main
- [x] done — baselines in report.md (benchmark: all gates PASS, ingest 1.01x)

### Part M — memory

### T1 — `FixedQueue`: exact storage, compare-wrap, never pin

- **Files:** `core/Pipeline/DSP.h`
- **Does:**
  - Adds an exact-storage construction tag (storage == capacity, honored by `resize`).
  - Every queue wraps internally by compare-subtract (`endIndex`, `wrappedIndex`,
    `advance`, `resize`).
  - `storageMask()` and `nextIndex()` assert power-of-two storage.
  - `makeStorage` no longer pins (R6).
- **Invariant:** the per-sample `push` stays allocation-free and branch-cheap. The
  power-of-two default stays, because `simdRingsToPoints` and the FFT gather assert it.
  Copies still alias their storage (`shared_ptr`), because the snapshots and
  `tst_sweep_segments:152` rely on it.
- **Verify:** `syntax-check.py`. **(build)** ctest `tst_envelope_ring`, `tst_sweep_segments`,
  `tst_replay_seek_engine` and the DSP SIMD bit-exact suite are all unchanged and green.
- **Deps:** T0 · **Exec:** main
- [x] done — syntax-check ok via 5 DSP.h includers; ctest pending maintainer build

### T2 — `TimeRing` exact mode, and the exactness suite

- **Files:** `core/Pipeline/DSP.h`, `app/tests/tst_ring_exactness.cpp` *(new)*,
  `app/tests/CMakeLists.txt`
- **Does:**
  - Adds an Exact/Decimating mode. The exact append clamps sub-cell jitter forward and
    pushes; a jump back by more than one base interval still clears the ring.
  - Adds `setDecimating(windowSec)` and `bytes()`.
  - Storage uses the exact tag.
  - Existing constructor call sites default to Decimating, so behavior is unchanged until T6.
  - The new suite pins exact-mode retention sample for sample, including under ±20% jitter
    and with equal adjacent values.
- **Invariant:** capacity comes only from `RingCapacity` (`compile:DSP::RingCapacity`), and
  a restarted clock drops the span.
- **Verify:** `syntax-check.py`. **(build)** `tst_ring_exactness` passes, and the existing
  ring suites are unchanged.
- **Deps:** T1 · **Exec:** main
- [x] done — syntax-check ok; tst_ring_exactness SKIP (tests not in compile db) — ctest pending build

### T3 — `EnvelopeRing`: a pyramid grid independent of level 0

- **Files:** `core/Pipeline/DSP.h`, `core/Pipeline/DSPDownsample.h`,
  `app/tests/tst_envelope_ring.cpp`
- **Does:**
  - Adds `baseInterval` (today's `2·windowHist/capacity`) and a base-cell accumulator fed on
    every append.
  - Coarse levels are sized from `windowHist / baseInterval`.
  - `selectLevel`, `cellIndex` and `levelSpanSec` read `baseInterval`, and so does
    `downsampleTimeWindow(EnvelopeRing)`.
  - Adds `exact()` and `bytes()`.
  - Test changes: the suite gains exact-mode brute-force cases plus a check that decimating
    mode equals today; the level-capacity asserts are updated only where the grid moved.
- **Invariant:** decimating mode is byte-for-byte today's pyramid (R3), and wide windows
  still read O(pixels) (spec 0057).
- **Verify:** `syntax-check.py`. **(build)** `tst_envelope_ring` and `tst_ring_exactness`
  pass.
- **Deps:** T2 · **Exec:** main
- [x] done — syntax-check ok; pyramid owns its accumulator with level 0's exact boundary math, so decimating mode is unchanged and tst_envelope_ring is left untouched as the regression signal; exact-mode pyramid cases live in tst_ring_exactness; ctest pending build

### T4 — Coverage growth and the rail

- **Files:** `core/Pipeline/DSP.h`, `app/tests/tst_ring_exactness.cpp`
- **Does:**
  - Adds the `DSP::RingBudget{used, limit, binding}` struct.
  - Adds `EnvelopeRing::growFor(n, tNewest, budget&)`. It grows only when the samples that
    would be evicted are newer than `tNewest − windowHist`.
    - The target is `ceil(density · windowHist · 1.1)`, at least `size + n`, and capped by
      `kMaxSamples` and the budget's grant.
    - A refused or capped grant calls `setDecimating` and flags the ring inexact.
    - Growth keeps every retained sample and rebuilds the levels cold.
  - Tests cover:
    - AC1: the rate × window × lane matrix, and bytes ≤ 1.25× need;
    - AC2: a rate step up and a rate step down;
    - AC5: the rail binds and the refused ring decimates with a flag.
- **Invariant:** growth never runs per sample, the count is logarithmic per ring, and no
  ring ever shrinks.
- **Verify:** `syntax-check.py`. **(build)** `tst_ring_exactness` passes.
- **Deps:** T3 · **Exec:** main
- [x] done — syntax-check ok; RingBudget + RingLimit + growFor/coverageNeed; levels use exact storage too; ctest pending build

### T5 — `SweepEngine`: shape, `allocate()`, `release()`

- **Files:** `core/Pipeline/DSP.h`, `app/tests/tst_sweep_segments.cpp`
- **Does:**
  - Splits `configure` so it stores the shape (curves, capacity, window) without allocating.
  - `allocate()` builds front, back and segments at the stored retention. `release()` frees
    them and keeps the trigger configuration.
  - `takeSegmentsFrom` requires an allocated engine.
  - The suite gains AC3: off holds zero bytes, on allocates, off releases. Existing
    assertions call `allocate()` where they used to rely on `configure()`.
- **Verify:** `syntax-check.py`. **(build)** `tst_sweep_segments` passes.
- **Deps:** T1 · **Exec:** main
- [x] done — syntax-check ok; configure() = setShape() + allocate() so existing callers keep their meaning; AC3 case added; ctest pending build

### T6 — Ring sizing policy

- **Files:** `core/Ui/UI/Dashboard/TimeRingSizing.{h,cpp}`, `app/tests/tst_time_ring_sizing.cpp`
- **Does:**
  - Adds `initialCapacity(windowSec, rateHz)` and makes `historyRing` choose the mode:
    - a stream source gets `fromRate(windowHist, rate)`, exact under the ceiling and
      decimating at it;
    - the frame lane gets `fromRate(windowHist, kInitialFrameLaneRateHz = 1000)` with an
      `atLeast(8192)` floor, exact.
  - Removes `frameLaneCapacity` and `grownCapacity` once T7 no longer calls them (same
    landing).
  - Rewrites the suite around the new policy.
- **Invariant:** capacity derives from a rate, never a bare count.
- **Verify:** `syntax-check.py`. **(build)** `tst_time_ring_sizing` passes.
- **Deps:** T4 · **Exec:** main
- [x] done — syntax-check ok; DEVIATION from plan: frame lane starts at fromRate(window, 100 Hz) floored at 1024 slots (20 KB at default) instead of 1 kHz / 8192, since per-block growth absorbs the first blocks; sweepCapacity() added; ctest pending build

### T7 — Dashboard: sizing, budget, rebuild capacity, `plotMemory()`

- **Files:** `core/Ui/UI/Dashboard.{h,cpp}`
- **Does:**
  - `configureLineSeries` and `configureMultiLineSeries` use `initialCapacity` and create
    config-only sweep engines.
  - Adds `m_plotBudget` (512 MB), recomputed from the live rings at reconfigure.
  - Deletes `growTimeRings`/`growTimeRing` and their tick call.
  - Rebuild constructs rings at the snapshot capacity, so the restore still splices.
  - Adds `plotMemory()`, an on-demand O(curves) report: bytes, rail, binding, inexact list.
  - Adds the sweep allocate/release callback used by T9.
- **Invariant:** rings exist before `buildLinePushes` and `buildMultiplotPushes`. The rebuild
  step names and order stay, because `tests/scripts/test_cpp_regressions.py` greps them. A
  rebuild never seeds a ring. `resetPlotClocks` is untouched. Lookups use `find`/`value`, not
  an inserting `operator[]`.
- **Verify:**
  - `syntax-check.py`, `code-verify.py --check`, and `pytest tests/scripts/test_cpp_regressions.py`
    (I can run these).
  - **(build)** the ctest suite.
- **Deps:** T5, T6 · **Exec:** main
- [x] done — syntax-check ok, test_cpp_regressions 74 passed; growTimeRing(s) deleted; sweeps are shape-only at configure; budget + report live in the new PlotMemoryLedger (T7b); rebuild-capacity handled by splice-on-same-window in restore (moved to T10)

### T8 — Ingest: the per-block growth check

- **Files:** `core/Ui/UI/Dashboard/DashboardIngest.{h,cpp}`
- **Does:**
  - Before a block column appends to a time ring, adds
    `if (size + n > capacity) [[unlikely]] growFor(...)`. This covers the stream plot, the
    stream multiplot, the frame plot and the frame multiplot.
  - A grown ring whose sweep is enabled triggers the T7 re-allocate callback.
  - `feedMultiSweep` gains the `back.empty()` guard.
- **Invariant (hotpath):**
  - The per-sample loop stays allocation-free, and growth is cold at block granularity.
  - No new signal hops, and push tables stay untouched (they hold no ring pointers).
- **Verify:** `syntax-check.py`. **(build)** `tst_dashboard_ingest`, and the first
  `--benchmark-hotpath` checkpoint for Part M (AC12: all nine gates, ingest ratio ≤
  baseline).
- **Deps:** T7 · **Exec:** main
- [x] done — syntax-check ok; growth check hoisted above every per-sample loop (uniform: exact tNewest; irregular: per-block pre-pass reserveFrameLaneRings); grown ring re-shapes an enabled sweep via TimeRingSizing::shapeSweep; budget + plotTimeRange added to IngestBindings (tst_dashboard_ingest fixture + link list updated); benchmark checkpoint PENDING maintainer build

### T9 — Lazy sweep wiring

- **Files:** `core/Ui/UI/Dashboard/PlotControlBank.{h,cpp}`, `core/Ui/UI/Dashboard.cpp`
  (binding only)
- **Does:**
  - `setPlotSweep` and `setMultiplotSweep` allocate through the callback before
    `enabled = true`, and release on disable.
  - `disableSweeps` (license loss) releases.
  - `restore*SweepConfig` allocates before `takeSegmentsFrom`.
- **Invariant:** the license gate (`isValid()` + `SS_LICENSE_GUARD()`) runs before any
  allocation, and the trigger configuration persists while disabled.
- **Verify:** `syntax-check.py`. **(build)** `tst_sweep_segments`, and a sweep on/off smoke
  test against the running app through the API.
- **Deps:** T7, T8 · **Exec:** main
- [x] done — syntax-check ok; license gate untouched, allocation only after it admits; restore allocates before takeSegmentsFrom (history rings restored first); retention changes charge the budget; ctest + API smoke PENDING build

### T10 — Replay seek engine

- **Files:** `core/Ui/UI/Dashboard/ReplaySeekEngine.cpp`, `app/tests/tst_replay_seek_engine.cpp`
- **Does:**
  - The splice match moves from `level0.interval` to capacity + mode + `baseInterval`.
  - `bulkLoadPlotWindow` pre-sizes each ring from the seek window's sample count before
    replaying.
- **Invariant:** the clock never rewinds, and `resetPlotClocks` stays the single clear path.
- **Verify:** `syntax-check.py`. **(build)** `tst_replay_seek_engine`; the maintainer runs
  `tests/integration/test_replay_timeline.py`.
- **Deps:** T7 · **Exec:** main
- [x] done — syntax-check ok; restore splices whole at an unchanged window (keeps learned capacity/mode/limit), replays at the saved density otherwise; bulk load pre-sizes via growFor against the budget; coverageNeed takes no doubling step for an empty ring (exact bulk sizing); 2 new tst_replay_seek_engine cases; ctest PENDING build

### T11 — The exactness surface: widgets, QML chip, API

- **Files:**
  - **T11a:** `core/Ui/UI/Widgets/Plot.{h,cpp}`, `core/Ui/UI/Widgets/MultiPlot.{h,cpp}`
  - **T11b:** `app/qml/Widgets/Dashboard/Plot.qml`, `MultiPlot.qml`
  - **T11c:** `core/Ui/ApiHandlers/DashboardHandler.cpp`, `doc/help/API-Reference.md`,
    `tests/integration/test_dashboard_configuration.py`
- **Does:**
  - **(a)** A read-only `historyExact` property, polled in `updateData` and emitted only when
    it flips.
  - **(b)** A "Decimated" text chip with a tooltip, no icon and no animation.
  - **(c)** Adds a `getStatus.plotMemory` summary and three new fields on each `tailFrames`
    row. Description strings stay unchanged, so nothing generated moves.
- **Verify:**
  - `code-verify.py --check`, `syntax-check.py` (a, c), and `qt-qml-review` on (b).
  - **(build)** the `test_dashboard_configuration.py` fields against the running app.
- **Deps:** T7 · **Exec:** a is main; b and c are delegated
- [x] T11a done · [x] T11b done · [x] T11c done — reviewed: chip re-parented under PlotWidget in MultiPlot.qml and anchored to curveLayer (agent had it in the legend Item / on plotArea without the graph offset); getStatus computes the report once; QML not yet seen in the running app (needs build)

### Part R — render

### T12 — `SdfStroke` geometry helpers

- **Files:** `core/Ui/UI/Widgets/SdfStroke.{h,cpp}` *(new)*, `core/Ui/CMakeLists.txt`,
  `app/tests/tst_sdf_stroke_geometry.cpp` *(new)* + `app/tests/CMakeLists.txt`
- **Does:**
  - Count and emit 4 vertices and 6 static-pattern indices per visible finite segment. Each
    vertex carries `{corner, a, b, capFlags}`, with flat-cap flags at run ends.
  - The attribute set, a grow-only reserve and a padded tail.
- **Verify:** `syntax-check.py`. **(build)** `tst_sdf_stroke_geometry` covers:
  - AC8: equal counts at 0°, 90° and 180° turns, and a dense envelope ≤ 8·W vertices;
  - no reallocation across 100 steady frames.
- **Deps:** T0 · **Exec:** main
- [x] done — SdfStroke.{h,cpp} syntax-check ok; 6-case tst_sdf_stroke_geometry registered (SKIP: tests not in compile db — ctest pending build)

### T13 — `SdfStrokeMaterial` and shaders

- **Files:** `core/Ui/UI/Widgets/SdfStrokeMaterial.{h,cpp}` *(new)*,
  `app/shaders/stroke_sdf.{vert,frag}` *(new)*, `app/CMakeLists.txt` (`qt_add_shaders`)
- **Does:**
  - A std140 block with matrix, opacity, premultiplied color, face/edge half-widths and
    intensity.
  - The fragment shader computes capsule distance with flat run-end caps and a 1 px linear
    feather equal to `kFeatherPx`. When `intensity > 1` it applies the HDR boost.
  - No back-face cull.
  - `tst_hdr_transfer`'s header comment lists the new copy.
- **Invariant:** the transfer bodies mirror `Misc/HdrTransfer.h` line for line (spec 0089).
- **Verify:** `syntax-check.py` on the C++. **(build)** qsb compiles in the maintainer's
  build.
- **Deps:** T12 · **Exec:** main
- [x] done — material syntax-check ok; shaders added to the hdr_shaders qt_add_shaders block; HDR transfer block verified byte-identical to stroke_hdr.frag; tst_hdr_transfer's comment already covers app/shaders/*.frag (no edit needed); qsb compile PENDING build

### T14 — `PlotCurve` switches paths

- **Files:** `core/Ui/UI/Widgets/PlotCurve.{h,cpp}`
- **Does:**
  - Opaque strokes go through `SdfStroke` with `SdfStrokeMaterial`.
  - `m_color.alphaF() < 1 || opacity() < 1` keeps the `GpuStroke` path.
  - The node is recreated when the path changes (the attribute set differs), and the
    material intensity follows `HdrOutput::effectiveBoost`.
- **Invariant:** `updatePaintNode` runs in sync with the GUI thread blocked, so nothing
  allocates in steady state (grow-only).
- **Verify:** `syntax-check.py`. **(build)** `tst_plot_curve_geometry` passes, and a smoke
  test with a 1 kHz plot in the running app.
- **Deps:** T13 · **Exec:** main
- [x] done — syntax-check ok; opaque → SDF node (static indices on realloc, padded tail, style on the material), translucent (color alpha or item opacity < 1) → existing ribbon path; node replaced on a path switch; opacityChanged → update(); tst_plot_curve_geometry + visual smoke PENDING build

### T15 — `PlotAreaFill` color hoist

- **Files:** `core/Ui/UI/Widgets/PlotAreaFill.cpp`
- **Does:** Converts the `QColor` to premultiplied float RGB once per `updatePaintNode`.
  `setFillVertexColor` takes floats and a per-vertex alpha.
- **Verify:** `syntax-check.py`, `code-verify.py --check`. **(build)** visual.
- **Deps:** T0 · **Exec:** delegate
- [x] done — delegated + reviewed: FillRgb hoisted once per emitColumns, identical lround formula; syntax-check ok

### Part C — console

### T16 — `TerminalBuffer` run fast path

- **Files:** `core/Ui/UI/Widgets/Terminal/TerminalBuffer.{h,cpp}`, `app/tests/tst_terminal_buffer.cpp`
- **Does:**
  - When the cursor is at its row end and the run fits before the column count, append the
    run to the row string and its colors in one call each.
  - Rows reserve the column count on creation.
  - `replaceData` stays the overwrite path.
  - The suite gains a check that the run path equals the per-character path on rows,
    colors, repeat counts and cursor, for a mixed stream with CR, BS, TAB, SGR and wraps.
- **Verify:** `syntax-check.py`. **(build)** `tst_terminal_buffer` passes.
- **Deps:** T0 · **Exec:** main
- [x] done — syntax-check ok; appendRun() fast path (row-end runs, one append + one color insert), replaceData() unchanged as the overwrite fallback; runAndCharacterFeedsAgree (VT-100 feed, colors on/off) added; ctest PENDING build

### T17 — `Console::Handler`: display history and flood cap

- **Files:** `core/Ui/Console/Handler.{h,cpp}`
- **Does:**
  - Keeps a line-bounded history of exactly the emitted `displayString` text since the last
    `cleared()`, capped at `m_scrollbackLines`, with a `displayHistory()` accessor.
  - Per tick, drops leading complete lines that carry no ESC beyond the scrollback before
    emitting (R14).
- **Invariant:** the history resets at every `cleared()` emission (clear, device switch,
  disconnect).
- **Verify:** `syntax-check.py`, `code-verify.py --check`.
- **Deps:** T0 · **Exec:** main
- [x] done — syntax-check ok; position-tracked history (displayMark/displaySince, monotonic char count, amortized line trim at 1.25x scrollback), reset at clear/device switch/connect; flood cap drops ESC-free leading lines past the scrollback

### T18 — `Terminal` visibility gate and replay

- **Files:** `core/Ui/UI/Widgets/Terminal.{h,cpp}`, `app/tests/tst_terminal_buffer.cpp`
- **Does:**
  - `append` returns early while `!isVisible()` and marks the buffer stale.
  - On `visibleChanged → true`, when stale and not paused, the terminal resets as on
    `cleared()` and replays `displayHistory()` once.
  - AC11 unit check: replaying a hidden plain-text stream, and a stream that colors each
    line with its own SGR codes, gives buffers equal to an always-visible feed of the same
    stream.
- **Invariant:** each instance gates itself, and the single direct connection at
  Terminal.cpp:101 is unchanged.
- **Verify:** `syntax-check.py`. **(build)** `tst_terminal_buffer`; the maintainer hides the
  console, streams, then shows it (AC11 observation).
- **Deps:** T16, T17 · **Exec:** main
- [x] done — syntax-check ok; DESIGN UPGRADE vs plan: hidden view records the console's display mark (new TerminalCatchUp sub-object, T18b, approved in chat to keep Terminal.cpp under the TU ratchet) and on show continues from it when still retained (exact incl. ANSI/VT100), else clears and replays the retained history (the bounded R13 case); gate sits on a dedicated onDisplayString slot so the welcome guide's internal append() is unaffected; tests: tracker semantics + hidden/caught-up equivalence; ctest + AC11 in-app observation PENDING build

### Part D — durability and close-out

### T19 — `lock-memory-scope` lint rule

- **Files:** `scripts/code-verify.py`
- **Does:** An error-severity rule patterned on `hotpath-assert-scope`.
  `(un)lockMemoryResident(` is allowed only in `core/Pipeline/IO/FrameReader.cpp` and
  `core/Pipeline/Platform/AppPlatform.{h,cpp}`.
- **Verify:** `code-verify.py --check` is clean on the tree. A seeded call in a scratch copy
  of `DSP.h` fails it (AC4).
- **Deps:** T1 · **Exec:** delegate
- [x] done — delegated + reviewed: find_lock_memory_scope_violations (error), 3-file allowlist, fence-aware; tree scan 0 findings; fires on a synthetic non-allowlisted path

### T20 — Scenario P performance test

- **Files:** `tests/performance/test_plot_pressure.py` *(new)*
- **Does:** Promotes this session's harness:
  - a TCP feeder at 1 kHz, the 30-plot and 4×8 multiplot projects, and 10 plots;
  - footprint growth through `vmmap --summary` on macOS, or RSS elsewhere, plus
    `plotMemory.historyBytes` (AC6);
  - optional macOS `sample` thread shares (AC9).
  - It skips when the API is down, notes the GUI consent prompt, and opens a fresh client per
    phase.
- **Verify:** It runs green against the rebuilt app. The numbers go into `report.md`.
- **Deps:** T11c · **Exec:** delegate
- [x] done — delegated + reviewed: 3 tests collect (10 plots, 30 plots, 4x8 multiplots), consent → skip with instructions, fresh client per phase, optional AC9 sample profile; never run against the app (would replace the open project); pgrep/sample parsing unexercised until the first real run

### T21 — Docs

- **Files:** `CLAUDE.md`, `doc/claude/architecture/dashboard.md`, `.claude/skills/ss-hotpath/SKILL.md`
- **Does:**
  - Rewrites the time-ring sizing and growth rules: exact level 0, coverage growth,
    `growTimeRing` gone, the rail, lazy sweep, no pinning.
  - Adds the SDF stroke to "GPU Curve Rendering", and the console gate.
  - Adds `Enforced: code-verify:lock-memory-scope, ctest:tst_ring_exactness` markers.
- **Verify:** `claim-verify.py` reports no new errors, and `documentation-verify.py` is
  clean on the changed docs.
- **Deps:** T1–T19 · **Exec:** main
- [x] done — CLAUDE.md time-ring bullet (+ctest:tst_ring_exactness, code-verify:lock-memory-scope markers), dashboard.md (exact rings, pyramid accumulator, SDF stroke, lazy sweep, Time-Ring Sizing rewrite, restore splice), ss-hotpath skill bullet, directory-map entries; claim-verify 0 errors; documentation-verify finding count unchanged vs HEAD (15 pre-existing)

### T22 — Acceptance run and report

- **Files:** `report.md`, `spec.md` (AC boxes)
- **Does:** After the maintainer's build, runs:
  - ctest;
  - `--benchmark-hotpath` (AC12);
  - T20 (AC6, AC9);
  - the audio rig (AC7);
  - the AC10 side-by-side (maintainer).
  Records the results against T0, and checks off the ACs.
- **Contingency (only if AC9's sync or blocked target misses):** **T22b** moves SDF emission
  into `updatePolish()` with a staging buffer (`PlotCurve.{h,cpp}`), then re-measures.
- **Deps:** all · **Exec:** main
- [ ] done

### T7b — `PlotMemoryLedger` sub-object *(added 2026-10-13, approved in chat)*

- **Files:** `core/Ui/UI/Dashboard/PlotMemoryLedger.{h,cpp}` *(new)*, `core/Ui/CMakeLists.txt`,
  `core/Ui/UI/Dashboard.{h,cpp}`
- **Does:** Owns `DSP::RingBudget` (the 512 MB rail), `refresh()` (recounts from the live
  rings and sweeps after a rebuild, restore, bulk load or reset) and `report()`. It binds the
  stores by reference, like `ReplaySeekEngine`. Its purpose is to keep `Dashboard.cpp` under
  the `--tu-census` ratchet (1809 → 1772).
- **Verify:** `syntax-check.py` passes on `Dashboard.cpp` and `PlotMemoryLedger.cpp`, and the
  TU census shrank to 2600 (baseline 2637).
- **Deps:** T7 · **Exec:** main
- [x] done

### T18b — `TerminalCatchUp` sub-object *(added 2026-10-13, approved in chat)*

- **Files:** `core/Ui/UI/Widgets/Terminal/TerminalCatchUp.{h,cpp}` *(new)*,
  `core/Ui/CMakeLists.txt`, `app/tests/CMakeLists.txt`
- **Does:** Holds a hidden terminal's skip mark: `skip`, `pending`, `take` and `reset`. It is
  pure (no handler dependency), so it can be unit-tested. Its purpose is to keep
  `Terminal.cpp` under the `--tu-census` ratchet: the aggregate is now 2636 against a baseline
  of 2637.
- **Verify:** `syntax-check.py` passes, and so does `--tu-census --check`.
- **Deps:** T18 · **Exec:** main
- [x] done

### T1b — Extract `FixedQueue` from `DSP.h` *(added 2026-10-13, approved in chat)*

- **Files:** `core/Pipeline/DSP/FixedQueue.h` *(new)*, `core/Pipeline/DSP.h`,
  `core/Pipeline/CMakeLists.txt`
- **Does:** Moves `FixedQueue`, `ExactStorage` and `roundUpToPowerOfTwo` byte for byte into
  their own header, which `DSP.h` includes. T1–T4 had pushed `DSP.h` to 1702 lines, past the
  `--tu-census` ratchet.
- **Verify:** a scripted byte-for-byte comparison of the moved block; the TU census is back
  to baseline (2637); `layer-verify.py` reports 0 errors; `syntax-check.py` passes on the 8
  includers.
- **Deps:** T4 · **Exec:** main
- [x] done

### Review fixes — `qt-cpp-review` DoD pass *(2026-10-13)*

Six review lenses ran on the C++ diff. Lint found nothing. The valid findings were fixed in place:
- **Rail (R7):** every allocation now goes through the budget:
  - initial history rings, through `historyRing(…, budget)`;
  - the replay resize;
  - sweep allocation and growth.
  - `refresh()` also binds when `used > limit`.
  - `configureLineSeries` and `configureMultiLineSeries` refresh after clearing, so a rebuild
    is never charged twice.
- **Exactness verdict:**
  - Stream rings start at rate × window × 1.1, so they never grow at their first wrap.
  - Frame-lane rings start at the 1024 floor, not 100 Hz (R5 at 10 Hz).
  - `growFor` decimates only when the grown ring cannot hold the window's samples, so a ring
    that fits is never flagged as ceiling-limited.
- **Sweep (R4):**
  - History growth grows a running sweep in place (`SweepEngine::growCapacity`,
    `TimeRingSizing::growSweep`); it is idempotent per block.
  - It never re-shapes, so a frozen single-shot trace survives. The segment store restarts
    at the new size.
- **Restore:** a decimating saved ring never splices over an exact live ring. The replay does
  not carry the saved verdict.
- **Console:**
  - Flood cap and line trim are skipped while duplicates collapse.
  - The cap keeps the newline that closes the open line.
  - History is capped by characters (output with no newlines).
- **Render:**
  - `SdfStrokeMaterial` sets `RequiresFullMatrix`; per Qt's renderer docs it prevents all
    batching.
  - `useSdfStroke` checks inherited opacity.
- **API:**
  - The `reason` field is a full switch (`none` included).
  - The summary counts rail-limited curves separately.
  - A missed ring lookup returns an exact sentinel.
- **Hygiene:**
  - `releaseSweep` split out of the bool-trap call.
  - Asserts added on the new functions.
  - `plan.md`'s per-sample-cost bullet corrected.

**Noted, not changed (investigation targets):**
- SDF geometry is grow-only, so pad and upload scale with the peak.
- `stroke_sdf.frag` recomputes the HDR boost per fragment; it is byte-identical to
  `stroke_hdr.frag` by rule.
- `PlotAreaFill` is the largest remaining sync cost.
- AC9 is borderline with HDR on, because the layer grab is out of scope.
- `SweepEngine::configure` is used only by tests now.

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` is met and checked off there (AC11 per T18:
      the unit equivalence plus the maintainer's observation).
- [ ] `python scripts/code-verify.py --check` is clean on all changed files, including the
      new `lock-memory-scope`.
- [ ] `scripts/syntax-check.py` passed on every edited C++ file, or each SKIP is stated.
- [ ] `qt-cpp-review` has run on the C++ diff (done, see "Review fixes" above) and
      `qt-qml-review` on the QML (not yet run). Findings are addressed or noted.
- [ ] `--benchmark-hotpath` passes all nine gates, with the dashboard ingest ratio no worse
      than the T0 baseline.
- [ ] The `pytest` targets ran or are listed for the maintainer: `test_plot_pressure.py`,
      `test_dashboard_configuration.py`, `test_replay_timeline.py`, `test_dashboard_lanes.py`
      and the audio rig.
- [ ] `python scripts/sanitize-commit.py` has run.
- [ ] The diff is what was asked and only that. No foreign files are touched, including the
      untracked `0096-deployment-packages/`.
- [ ] `spec.md` status is set to `done`.
