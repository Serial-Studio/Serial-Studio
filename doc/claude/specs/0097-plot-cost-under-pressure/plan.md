---
spec: 0097-plot-cost-under-pressure
phase: plan
status: approved     # approved 2026-10-13, with spec amendments R7, R9/AC8, R13, growth constraint
updated: 2026-10-13
---

# Plan 0097 — Plots pay for what arrives and what they draw

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

The plan has four independent parts, each fixing one measured cost.

- **Memory (R1–R8).** A time-plot ring stops decimating while it can afford every sample.
  - Level 0 becomes an exact sample FIFO in exact-sized storage. Nothing about it is pinned.
  - The envelope pyramid gets its own base grid, so it no longer depends on level 0's
    decimation.
  - A ring grows at block granularity whenever the next block would evict a sample still
    inside the history window. The ring needs no rate estimate, so it can never drop a real
    sample to wrap-around.
  - A ring that hits the per-curve ceiling or the 512 MB rail falls back to today's min/max
    grid. It flags itself, and that flag reaches the API and the plot.
  - Sweep engines keep their configuration but allocate rings only while sweep is on.
- **Render (R9–R11).** Opaque plot curves draw through a new signed-distance stroke
  material, using four vertices per segment, with no join fans and no per-vertex color math.
  - Translucent strokes keep the existing tessellated path for exact parity.
  - The area fill hoists its color math out of the vertex loop.
- **Console (R12–R14).** Each `Terminal` instance stops processing while it is not visible
  and replays a line-bounded display history from the `Console::Handler` when it is shown.
  The terminal buffer writes runs instead of single characters.
- **Durability (R15).** A `code-verify` rule pins `lockMemoryResident` to the pipeline. New
  ctest suites pin exactness, the byte bounds, lazy sweep allocation and stroke geometry, and
  a performance test replays Scenario P.

## Affected subsystems & files

| File | Change |
|------|--------|
| `core/Pipeline/DSP.h` | Several changes:<br>• `FixedQueue`: an exact-storage constructor tag; internal wrap by compare instead of by mask (one code path); `storageMask()` and `nextIndex()` assert power-of-two storage; `makeStorage` never pins.<br>• `TimeRing`: an exact/decimating mode, an exact-mode push path, `setDecimating(window)` and `bytes()`.<br>• `EnvelopeRing`: its own base-cell accumulator and `baseInterval`, coarse levels sized from the window, `growFor(...)` coverage growth, `exact()` and `bytes()`.<br>• New `DSP::RingBudget` (rail accounting).<br>• `SweepEngine`: split `configure` into a stored shape plus `allocate()` and `release()`. |
| `core/Pipeline/DSP/FixedQueue.h` *(new, added at T1b)*, `core/Pipeline/CMakeLists.txt` | `FixedQueue` moved byte for byte out of `DSP.h` to keep it under the `--tu-census` ratchet. |
| `core/Pipeline/DSPDownsample.h` | `downsampleTimeWindow(EnvelopeRing)` reads level geometry through `baseInterval`. No other change: `spanFromFixedQueue` already handles any storage size. |
| `core/Ui/UI/Dashboard/TimeRingSizing.{h,cpp}` | `frameLaneCapacity` and `grownCapacity` are replaced by `initialCapacity(windowSec, rateHz)` and the coverage-growth target helper. The `historyWindowSec` headroom is unchanged. |
| `core/Ui/UI/Dashboard.{h,cpp}` | Several changes:<br>• `configureLineSeries` and `configureMultiLineSeries` use the new initial sizing and create config-only sweep engines.<br>• The tick call to `growTimeRings`/`growTimeRing` is removed.<br>• Owns `m_plotBudget` (`DSP::RingBudget`, 512 MB).<br>• New `plotMemory()` report and a sweep allocate/release hook.<br>• Rebuild sizes new rings from the snapshot capacity so `restore*` can still splice. |
| `core/Ui/UI/Dashboard/DashboardIngest.{h,cpp}` | Before each block column is appended to a time ring, a `[[unlikely]]` per-block `growFor` check runs (stream plot, stream multiplot, frame plot and frame multiplot paths). The `feedMultiSweep` guard gains `back.empty()`. |
| `core/Ui/UI/Dashboard/PlotControlBank.{h,cpp}` | `setPlotSweep`/`setMultiplotSweep` allocate through a Dashboard callback before setting `enabled = true` and release on disable. `disableSweeps` releases. `restore*SweepConfig` allocates before `takeSegmentsFrom`. |
| `core/Ui/UI/Dashboard/ReplaySeekEngine.cpp` | The splice match moves from `level0.interval` to `capacity + mode + baseInterval`. The pre-size before bulk load is derived from the seek window. |
| `core/Ui/UI/Widgets/Plot.{h,cpp}`, `MultiPlot.{h,cpp}` | Read-only `historyExact` property (polled in `updateData`; emits only when the value flips). |
| `core/Ui/UI/Widgets/SdfStroke.{h,cpp}` *(new)* | Pure count/emit helpers for the signed-distance ribbon (4 vertices, 6 static indices per segment, run-end cap flags). Unit-testable like `GpuStroke`. |
| `core/Ui/UI/Widgets/SdfStrokeMaterial.{h,cpp}` *(new)* | `QSGMaterial` plus shader with a std140 block: matrix, opacity, premultiplied color, face/edge half-widths and intensity. |
| `app/shaders/stroke_sdf.{vert,frag}` *(new)* | Capsule distance, flat run-end caps, a 1 px feather, and the HDR boost copied verbatim from `Misc/HdrTransfer.h`. |
| `core/Ui/UI/Widgets/PlotCurve.{h,cpp}` | Opaque strokes go through `SdfStroke` with `SdfStrokeMaterial`. Translucent strokes (color alpha < 1 or item opacity < 1) take the existing `GpuStroke` path. |
| `core/Ui/UI/Widgets/PlotAreaFill.cpp` | Precompute premultiplied float RGB once per paint. Per-vertex work is alpha only. |
| `core/Ui/CMakeLists.txt`, `app/CMakeLists.txt` | Add the two new source pairs and the `stroke_sdf` shaders to `qt_add_shaders`. |
| `core/Ui/Console/Handler.{h,cpp}` | Line-bounded display history of exactly what `displayString` emitted since the last `cleared()`, capped at the scrollback lines, with an accessor. Per-tick pending cap for floods (C4). |
| `core/Ui/UI/Widgets/Terminal.{h,cpp}` | `append` returns early while hidden and marks the buffer stale. On becoming visible and stale (and not paused), reset and replay the history once. |
| `core/Ui/UI/Widgets/Terminal/TerminalBuffer.{h,cpp}` | `appendText` run fast path (cursor at the row end, run fits before the wrap), plus row capacity reserved to the column count. `replaceData` remains the overwrite path. |
| `app/qml/Widgets/Dashboard/Plot.qml`, `MultiPlot.qml` | A "Decimated" chip with a tooltip when `!model.historyExact`. |
| `core/Ui/ApiHandlers/DashboardHandler.cpp` | `getStatus` gains a `plotMemory` summary. `tailFrames` rows gain `historyBytes`, `retainedRateHz` and `exact`. The description strings are unchanged, so nothing generated moves. |
| `scripts/code-verify.py` | New error rule `lock-memory-scope`: `(un)lockMemoryResident(` is allowed only in `core/Pipeline/IO/FrameReader.cpp` and `core/Pipeline/Platform/AppPlatform.{h,cpp}`. Patterned on `hotpath-assert-scope`. |
| `app/tests/` (`CMakeLists.txt`, suites) | New `tst_ring_exactness` and `tst_sdf_stroke_geometry`. Updates to `tst_time_ring_sizing`, `tst_envelope_ring`, `tst_sweep_segments`, `tst_replay_seek_engine` and `tst_terminal_buffer` (run-path equivalence plus a hidden-replay equivalence). |
| `tests/performance/test_plot_pressure.py` *(new)*, `tests/integration/test_dashboard_configuration.py`, a console integration test | Scenario P replay (AC6, AC9), the `plotMemory` fields, and AC11. |
| `CLAUDE.md`, `doc/claude/architecture/dashboard.md`, `.claude/skills/ss-hotpath/SKILL.md`, `doc/help/API-Reference.md` | Rewrite the time-ring sizing and growth rules (`growTimeRing` is gone), describe the SDF stroke, add the `Enforced: code-verify:lock-memory-scope` marker, and update the API examples. `claim-verify.py` must stay clean. |

## Architecture & data flow

### M — memory

**Exact level 0.**
- A `TimeRing` in exact mode appends by clamping sub-cell backward jitter forward (as today)
  and then pushing. It has no cell logic, so every sample lands in its own slot with its
  exact time and value.
- A jump back by more than the base interval still clears the ring. The time-ring rule
  "a restarted clock drops the span" holds.
- Decimating mode is today's code, unchanged. It is entered only through `setDecimating`.

**The pyramid gets its own grid.**
- `EnvelopeRing` keeps `baseInterval = 2 · windowHist / capacity`, which is today's
  `interval` formula, plus an open base-cell accumulator fed on every append.
- A crossed base-cell boundary folds into the coarse levels through today's `foldCell`.
- Levels are sized from `windowHist / baseInterval`, which equals today's `capacity / 2`. In
  decimating mode the base cell is the level-0 cell, so pyramid behavior is byte-for-byte
  today's (R3).
- `selectLevel`, `cellIndex` and `levelSpanSec` read `baseInterval`.

**Coverage growth (R1, R2), at block granularity on the GUI thread.**
- Before a block's `n` samples append to a ring, `growFor(n, tNewest, budget)` runs, but only
  when `size + n > capacity`, which is one compare on the steady-state path.
- It grows only when the samples that would be evicted are newer than
  `tNewest − windowHist`. Evicting samples older than the window is the steady state.
- The target is `ceil(density · windowHist · 1.1)`, where density is the samples retained
  per second of retained span. The target is at least `size + n`, at most
  `RingCapacity::kMaxSamples`, and at most what the budget grants.
- If the target cannot cover the window, the ring calls `setDecimating(windowHist)` at its
  current capacity and reports `exact() == false`.
- Growth keeps every retained sample (`FixedQueue::resize` keeps the newest, and nothing in
  the window is discarded) and rebuilds the levels from level 0. Today's
  `rebuildLevelsFromLevel0` does that rebuild cold.
- Growth count is logarithmic: at most about 10 steps per ring per session.

**Initial sizing (`TimeRingSizing::initialCapacity`).**
- A stream source starts at `fromRate(windowHist, rate)`, exact under the ceiling and
  decimating at the ceiling (today's stream sizing, now flagged).
- A frame-lane source starts at `fromRate(windowHist, kInitialFrameLaneRateHz = 1000)` with
  an 8192-slot floor. That is 200 KB at the default range, and it grows on demand.
- Capacity still comes only from a rate, so the compile-enforced `RingCapacity` rule holds.

**Exact storage (R5).**
- `FixedQueue` constructed with the exact tag allocates exactly `capacity` slots.
- Every `FixedQueue` now wraps internally by compare-and-subtract, which is equally cheap
  and valid for any storage size. The power-of-two default and the mask API stay for the
  sample-count `AxisData` users (`simdRingsToPoints`, the FFT gather) that assert it.
- Time rings and sweep rings use the exact tag.

**Rail (R7).**
- `DSP::RingBudget{used, limit = 512 MB, binding}` is owned by the Dashboard and recomputed
  from the live rings on every reconfigure.
- Growth asks for a grant. A refused grant means the ring stays at its size and decimates
  from then on. No ring ever shrinks.

**Lazy sweep (R4).**
- Every time plot keeps a config-only `SweepEngine` in `m_plotSweep`/`m_multiplotSweep`, so
  trigger settings pushed while disabled still persist.
- `PlotControlBank` calls a Dashboard callback to `allocate()` before setting
  `enabled = true`, and `release()` on disable or license loss.
- Allocation is sized from the paired history ring: capacity scaled by `T / windowHist`,
  which is the same density.
- When an enabled sweep's history ring grows, the sweep re-allocates at the new density. That
  resets the sweep in progress and its retained segments once. It happens in the first
  seconds after connect, and it is accepted.

**Observability (R8).**
- `Dashboard::plotMemory()` walks the rings on demand (O(curves)) and returns:
  - total bytes;
  - the rail and whether it is binding;
  - the inexact count;
  - for inexact curves only, a list with `uniqueId`, rate, bytes and reason
    (`ceiling` or `rail`).
- Widgets poll `exact()` once per tick, a cheap read.

### R — render

**`SdfStroke`** emits one quad per visible finite segment (`a → b`):
- the four corners are `a − d·e ± n·e` and `b + d·e ± n·e`, where `e = face + kFeatherPx`;
- each vertex carries `{corner.xy, a.xy, b.xy, capFlags}`;
- indices are a static pattern written only when capacity grows;
- `QSGGeometry` uses `StreamPattern` for vertices and `StaticPattern` for indices;
- `reserveGeometry`'s grow-only and pad-the-tail contract carries over, with its own
  attribute set.

**The shaders.**
- The vertex shader transforms the corner and passes the corner, `a`, `b` and the caps.
- The fragment shader takes the distance to segment `ab`; at a flagged run end it clamps to
  the end plane, so caps are flat at runs and gaps, as today.
- Coverage is `clamp((edge − d) / kFeatherPx, 0, 1)`, which is today's linear 1 px feather.
- The output is the premultiplied uniform color × coverage × opacity, HDR-boosted through
  the copied transfer when `intensity > 1`.
- Round joins come from the capsule overlap, so no fan is emitted at any turn angle.
- Cost per segment is one `sqrt` and 24 float writes.

**`PlotCurve`** keeps `projectToPixels` and run splitting, then branches:
- `m_color.alphaF() < 1 || opacity() < 1` → the existing `GpuStroke` path (sweep segments
  dim through item opacity, and overlapping capsules would double-blend at joins);
- otherwise → the SDF path.
- The material is swapped like the HDR swap today: one SDF material whose intensity is 1 in
  SDR.

**`PlotAreaFill`** converts `QColor` to premultiplied floats once per `updatePaintNode`.
`setFillVertexColor` takes floats.

**Contingency, built only if AC9 misses.** Move the SDF emission into `updatePolish()` into
a widget-owned staging buffer, so it overlaps the render thread's previous frame and sync
becomes a memcpy.

### C — console

**Display history (`Console::Handler`).**
- Every `displayString` emission is also appended to a line-bounded history: a deque of
  complete lines plus an open tail, capped at `m_scrollbackLines`.
- The history resets at every `cleared()` emission: `clear()`, device switch and disconnect
  clear.

**Gate (`Widgets::Terminal`).**
- `append` returns early while `!isVisible()` and sets `m_stale`. Each of the up to two
  instances gates itself, because both receive the signal over the same direct connection.
- On `visibleChanged → true`, with `m_stale` set and not `m_paused`, the terminal resets its
  buffer exactly as on `cleared()` and calls `append(handler.displayHistory())` once.

**Run fast path (`TerminalBuffer::appendText`).**
- When the cursor sits at the end of its row and the run fits before `m_columns`, the run is
  appended to the row `QString` in one call and its `CharColor`s in one `insert`.
- Rows reserve `m_columns` on creation.
- Anything else (an overwrite or a wrap boundary) falls back to the per-character
  `replaceData`.

**Flood cap (R14).**
- When one tick's `m_pendingDisplay` holds more complete lines than `m_scrollbackLines`,
  leading complete lines that contain no ESC byte are dropped before emitting. A visible
  result would have scrolled them out anyway.
- Escape-bearing text is never dropped.

## Hotpath & threading impact

**Touches the hotpath? Yes, the GUI-side dashboard ingest only.** The pipeline thread,
`FrameReader`, `CircularBuffer`, `FrameBuilder`, the span lane and block publication are
untouched.
- **The per-sample loop costs about what it did.** In exact mode it does a clamp check, the
  pyramid's open-cell fold and a push, in place of today's min/max cell update. The
  `FixedQueue` wrap changes from `&` to compare-subtract. The DoD review corrected this bullet
  (it first read "cheaper"): the work is the same cost class, so AC12 guards it.
- **One new compare per block per target ring** (`size + n > capacity`).
- **The growth body is `[[unlikely]]`, cold, and allocates.** It runs at block granularity,
  bounded to about 10 times per ring per session, and never inside the per-sample loop.
  - This is a named deviation from "the ingest allocates nothing". The
    precedent is the 3D ring's `[[unlikely]] ring->resize()`.
  - It is the only way to guarantee no sample is lost at any UI refresh rate (1–240 Hz)
    without reserving for the worst case, which is the bug.
- **Benchmark plan.** `--benchmark-hotpath` must pass all nine gates. The
  `lua+dashboard(off)` → `dashboard ingest costs` ratio must be no worse than the db2debec4
  baseline (AC12).

**New cross-thread signal/slot? No.**
- Sweep allocation is a direct call within the GUI thread (`PlotControlBank` → Dashboard
  callback).
- The terminal gate and replay are GUI-thread only.

**New input to a cached hotpath flag? No.** No new cached flag.

**Timestamp ownership: unchanged.**
- Rings store the timestamps the plot clock already produced.
- Exact mode stops altering them (no cell snapping), so it is strictly closer to the source
  stamp.

## Data model & persistence

- **No project or file-format change.**
  - Sweep configuration still persists through the existing `widgetSettings` keys.
  - Retention semantics are unchanged.
- **No `Keys::` addition.**

## API / SDK surface

**Reply-only additions, which need no regeneration.** The schema, SDK, proto and ledger
describe request parameters only, and the description strings stay unchanged.

**`dashboard.getStatus` gains `plotMemory`:**
- `historyBytes`
- `railBytes`
- `railBinding`
- `timeCurves`
- `inexactCurves`
- `inexact`: an array with `uniqueId`, `rateHz`, `bytes` and `reason`. It is empty in
  normal operation, so the reply stays small for the assistant's frequent calls.
- `_summary` gains one clause only when something is inexact.

**`dashboard.tailFrames` rows gain** `historyBytes`, `retainedRateHz` and `exact` for
time-axis plots.

The `doc/help/API-Reference.md` examples are updated, and
`tests/integration/test_dashboard_configuration.py` asserts the new fields.

## QML / UI

- **`Plot.qml` and `MultiPlot.qml`.** A small text chip shows in the plot area's top-left
  corner while `!model.historyExact`:
  - it reads `qsTr("Decimated")`;
  - its tooltip is `qsTr("History limit reached: zooming shows min/max per cell, not every
    sample")` (it covers both the per-curve ceiling and the rail; reworded at the DoD review);
  - it uses theme colors, has no icon (so the IconRegistry is not involved) and no
    animation (so the motion contract is not involved).
- **No other UI change.** No new setting (spec non-goal).

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| How exactness is achieved | (a) Today's grid with finer cells. (b) An exact level-0 FIFO with an independent pyramid grid. (c) A byte budget that decimates. | **(b).** With (a), exact needs at most one sample per cell, which means 2–4× memory and still drops samples under timestamp jitter or equal adjacent values. (c) violates the spec's deciding constraint. |
| What triggers growth | (a) Tick-time lookahead from the plot clock's EMA. (b) A per-block coverage check in the ingest. (c) Reserve for the worst case (today). | **(b).** It is rate-free and cannot wrap away an in-window sample at any refresh rate. (a) is noisy for its first frames and blind to the inter-block gap on uniform blocks. (c) is the bug. |
| Storage size | (a) Keep power-of-two (up to 2× need). (b) Exact storage with compare-wrap. | **(b).** It saves up to 6 MB per 48 kHz channel. `spanFromFixedQueue` already handles any size, and the power-of-two users keep the default. |
| How the rail picks victims | (a) Water-fill, fastest curve first. (b) A refused grant freezes that ring's size and it decimates. | **(b).** (a) needs shrinking rings, which throws away history the spec protects, and the rail only binds in pathological setups. Spec R7 is amended to match. |
| Stroke geometry | (a) Keep tessellation but hoist the color and skip fans when dense. (b) An SDF capsule material. (c) Incremental scrolling geometry. (d) Column spans for dense data. | **(b).** Cost is independent of density and turn angle, about 7× less upload per segment, and there is no per-vertex color. (a) still carries fans per reversal and only trims a constant. (c) is a large rewrite (y-rescale, zoom, sweep). (d) changes how dense curves look. |
| Translucent strokes | (a) SDF everywhere. (b) The legacy path when alpha or opacity is below 1. | **(b).** Overlapping capsules double-blend at joins and would bead the dimmed sweep segments. The legacy path is kept anyway for Plot3D. |
| Where the console gate lives | (a) A viewer set in the Handler. (b) Per Terminal instance. | **(b).** Two terminals share one direct connection, so a Handler gate cannot stop a hidden one while the other is visible. |
| How R8 is exposed | (a) A new `dashboard.getPlotMemory` command. (b) Reply fields on `getStatus` and `tailFrames`. | **(b).** (a) means `command_safety.json`, regenerated `api-schema.json` (needs a commercial build dump), the SDK and the proto. Replies are schema-free. |
| Console exactness on show (spec open question) | (a) An unbounded raw history. (b) Line-bounded history at the scrollback size. | **(b).** It is exact for plain text. With ANSI or VT100, state from before the retained window (SGR color, cursor addressing, clears) can differ. Spec R13 is amended to this bounded form. |

## Risks & mitigations

- **SDF stroke visual parity (R11).** Feather, flat caps at run ends and the HDR transfer
  copy are matched to today's numbers.
  - AC10 is a side-by-side at 1x and 2x device pixel ratio.
  - The legacy path remains selectable in code, which is the rollback.
- **Pyramid and exact-FIFO correctness.** `tst_ring_exactness` replays brute-force contracts
  (in the style of `tst_envelope_ring`) through growth steps, mode flips and wrap.
  `tst_envelope_ring`'s level-size assertions change only where the base grid moved.
- **Rebuild and restore splice.** New rings take the snapshot capacity, and the splice match
  moves to capacity, mode and base interval.
  - `tests/scripts/test_cpp_regressions.py` greps the names and order of the rebuild steps,
    so those names stay.
  - `bulkLoadPlotWindow` pre-sizes from the seek window before replaying.
- **Sweep license transitions.** `disableSweeps` on `LicenseStateChanged` releases.
  - The widget-side `sweepEnabled` can be true while the engine is unallocated.
    `display()` already returns its static empty ring, and MultiPlot shows an empty sweep,
    as it does today when unlicensed.
- **Rail accounting drift.** `used` is recomputed from the live rings at reconfigure, and
  growth deltas are the only incremental writes.
- **HDR transfer drift.** The new frag mirrors `Misc/HdrTransfer.h` line for line, and
  `tst_hdr_transfer`'s header comment lists it among the copies.
- **Software renderer.** Custom materials need RHI. Plots already require it (QtGraphs and
  the existing custom stroke node), so there is no regression.
- **Console replay latency.** Replaying up to 1000 lines on show takes milliseconds with the
  run path. It is measured in AC11.
- **Flood cap and duplicate-collapse.** When the cap drops lines, the repeat counts reflect
  only the lines that reached the terminal. This is documented as a known limitation.
- **`common-mistakes.md` classes exposed:**
  - the time-ring rules (rate-derived capacity, a clock that never rewinds, no seeding on
    rebuild), each kept by construction and pinned by tests;
  - `operator[]` inserts on Qt containers in the new Dashboard code, so use `find`/`value`;
  - cross-thread nothing.

## Test & verification plan

**Unit (ctest, maintainer-built, I may run against an existing build dir):**
- **`tst_ring_exactness`:**
  - **AC1:** rates 10 Hz / 1 kHz / 10 kHz / 48 kHz / 192 kHz × windows 1 / 10 / 300 s, on
    both the stream and frame paths, with ±20% jitter on the frame path. Retained equals
    input, sample for sample, from sample 1 when under the ceiling. Above the ceiling it
    equals today's grid output, and bytes are ≤ 1.25× need.
  - **AC2:** a rate step up grows the ring with no loss; a step down keeps the ring.
  - **AC5:** a rail-binding configuration with the refused-grant ring decimates and is
    flagged.
- **`tst_sweep_segments` (AC3):** off holds zero bytes, on allocates, off releases; the
  existing assertions are adapted to `allocate()`.
- **`tst_time_ring_sizing`:** rewritten around `initialCapacity` and the growth target.
- **`tst_envelope_ring`:** adapted, plus a decimating-mode equals-today check.
- **`tst_replay_seek_engine`:** adapted to the new splice match.
- **`tst_sdf_stroke_geometry` (AC8):**
  - vertices = 4 × visible segments and indices = 6 × visible segments for any turn angle
    (straight, 90°, 180° reversal);
  - a dense min/max envelope at W columns emits ≤ 8W vertices;
  - grow-only reserve with a padded tail across 100 frames.
- **`tst_terminal_buffer`:**
  - the run path and the per-character path produce identical rows, colors, repeat counts
    and cursor for a mixed stream;
  - a hidden-then-replayed plain-text stream equals an always-visible one (AC11, unit
    level).

**Static and CI (AC4):** `code-verify.py --check` with the new `lock-memory-scope` rule. A
seeded violation in a scratch copy proves it fails. `claim-verify.py` stays clean after the
doc edits.

**Integration and performance (maintainer runs, app up with the API server; I may drive it):**
- **`tests/performance/test_plot_pressure.py` (AC6, AC9):**
  - It replays this session's Scenario P harness (the single-plot and multiplot variants),
    asserts the footprint growth and `plotMemory.historyBytes` bounds, and records BADAQ's
    `historyBytes` at connect.
  - The AC9 thread shares come from `sample` on macOS, parsed as in the spec's measurement.
    The before/after profiles are attached to the spec report.
- **`tests/integration/test_dashboard_configuration.py`:** the `plotMemory` and `tailFrames`
  fields.
- **Console integration (AC11):** stream an ANSI-colored pattern with the console hidden,
  show it, and compare the tail through the console API.
- **Audio (AC7):** reuse the `SS_AUDIO_CAPTURE_MATCH` rig from
  `test_dashboard_lanes.py`/`test_audio_loopback.py`, asserting
  `tailFrames.retainedRateHz ≈ 48000` and `exact`.

**Hotpath (AC12):** `--benchmark-hotpath --min-fps 256000`, all nine gates, with the
`dashboard ingest costs` ratio compared to the db2debec4 baseline.

**Visual (AC10):** the maintainer runs a side-by-side of the R11 scene list, before and
after, with HDR on and off, light and dark themes, and 1x and 2x.

**Before handoff:** `qt-cpp-review` on the C++ diff, `qt-qml-review` on the two QML edits,
`scripts/syntax-check.py` on every C++ file edited, and `sanitize-commit.py` before any
commit.
