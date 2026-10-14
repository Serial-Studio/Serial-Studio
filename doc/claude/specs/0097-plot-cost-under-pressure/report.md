---
spec: 0097-plot-cost-under-pressure
phase: report
updated: 2026-10-13
---

# Report 0097 — Baselines and acceptance results

## Baseline (T0) — HEAD db2debec4, before any change

**Machine and build:**
- The maintainer's Mac, running Darwin 27.0.0.
- The app is `/Applications/Serial Studio Pro.app` (4.1.0), built 2026-10-13 17:11 from
  db2debec4.
- HDR is on (`App.HdrEnabled = 1`), with a 60 Hz display tick and a 10 s time range.

**Method:**
- A throwaway harness drove the running app over the API.
  - It loaded a project of N single-curve time plots and started a local TCP feeder.
  - The feeder sent CSV frames at 1 kHz with all N values in each frame.
  - The harness connected, then streamed for 15 s.
- **Footprint** is the "Physical footprint" from `vmmap --summary`.
- **Heap** is the DIRTY column of vmmap's "IOAccelerator" row. On macOS that row is the
  mimalloc heap (VM tag 100), not GPU memory.

### Memory (Scenario P, frame lane, 1 kHz)

| Time plots | Footprint before connect | During (15 s) | Growth | Heap dirty growth |
|-----------:|-------------------------:|--------------:|-------:|------------------:|
| 1          | 851.4 MB                 | 869.3 MB      | +18 MB | +13 MB            |
| 10         | 858.5 MB                 | 970.6 MB      | +112 MB| +120 MB           |
| 30         | 860.3 MB                 | 1331.2 MB     | +471 MB| +425 MB           |

**Attribution by code:**
- Each time-plot curve allocates about 12.3 MB of `mlock`-pinned rings:
  - a 262,144-slot history ring (4 MB) plus its pyramid (0.27 MB);
  - two sweep rings at the same size (8 MB) with sweep off.
- That leaves about 2–3 MB per plot of widget and scene-graph overhead.
- After disconnecting with the 30-plot project still loaded, the footprint fell back to
  940 MB, so the rings are released on disconnect.

**Idle baseline** (out of scope, recorded for context), with a trivial project and nothing
connected:

| Component | Size |
|---|---|
| Footprint | ~850 MB |
| Dirty heap | 382 MB |
| IOSurface | 167 MB |
| GPU-owned (graphics) | 157 MB |
| Peak footprint after the 30-plot run | 1.4 GB |

**BADAQ** (the maintainer's field project, `TAM-Firmware/Serial Studio/BADAQ.ssproj`):

| Item | Count |
|---|---|
| Groups | 125 |
| Datasets | 734 |
| Sources | 4 (CAN, 2× DigiDAQ, load bank) |
| Time plots | 27 |
| FFTs | 4 |
| Data grids | 91 |
| Bar panels | 8 |
| Multiplots | 0 |

At about 12.3 MB per time-plot curve, its plot rings take about 330 MB at connect.

### GUI thread (Scenario P, 30 plots, 1 kHz)

**Method:** `sample <pid> 10` while streaming, giving 3718 samples per thread (about 2.7 ms
per sample). Call-graph shares are of wall time.

| Thread | Bucket | Share of wall |
|---|---|---|
| GUI | blocked in `QWaitCondition::wait` under `QQuickWindow::event` (waiting on the render thread) | 53% |
| GUI | `Console::Handler::displayString` → `Terminal::append` → `TerminalBuffer` (console not shown) | 31% |
| GUI | `Plot::draw` (downsampling plus series replace), via the Plot.qml binding | ~5% |
| Render | idle (waiting) | 42% |
| Render | `syncSceneGraph` (GUI blocked) | 31% |
| Render | of which `PlotCurve::updatePaintNode` | ~19% |
| Render | of which `PlotAreaFill::updatePaintNode` | ~8% |
| Render | `renderSceneGraph` (mostly `QSGRhiLayer::grab`: the scene rendered into the HDR layer) | 25% |

- **Process CPU:** about 50% while streaming.
- **Stroke geometry:** a min/max envelope reverses direction at almost every point.
  - Each point emits 8 body vertices plus a round-join fan of up to 19 vertices and 72
    indices (`kFanMax` 8).
  - That comes to roughly 820 KB of geometry per 1200-point curve per frame.
  - Each vertex re-derives its color through `QColor::redF/greenF/blueF`.

### Hotpath benchmark (AC12 baseline)

**Run:** `--headless --benchmark-hotpath --min-fps 256000` on the installed binary, while the
GUI instance stayed open and idle.
- The binary is a CI build: assertion paths point at `/Users/runner/...`, and it includes
  the Safe-mode Lua memory budget from 91e821613.
- Exit code 0, all nine gates PASS, native lane: span.

| Run | Frames/s | Gate |
|---|---:|---:|
| data-pipeline | 39,469,531 | 1,024,000 |
| native(numeric) | 4,764,057 | 1,024,000 |
| native(mixed) | 3,537,695 | 512,000 |
| lua(numeric) | 1,644,488 | 256,000 |
| js(numeric) | 639,619 | 128,000 |
| lua(mixed) | 1,204,836 | 128,000 |
| js(mixed) | 485,551 | 64,000 |
| lua+exporters | 1,191,778 | 128,000 |
| lua+dashboard | 974,841 | 128,000 |
| lua+dashboard(off) | 987,211 | ungated |

| Readout | Baseline |
|---|---|
| `dashboard ingest costs` (AC12 reference) | **1.01x** |
| `dashboard costs` | 1.69x |
| Stage breakdown | extract 25 ns, tokenize 15 ns, datasets+publish 170 ns |
| Peak RSS | 286 MB |

A side finding, outside this spec: the run printed
`serialstudio.assert: assertion failed: m_inner == nullptr ... LuaMemoryBudget.cpp:85 in
LuaMemoryBudget::install`. This is outside spec 0097 and was fixed separately at the
maintainer's request. `LuaScriptEngine::loadScript` keeps the previous state alive while it
builds the new one, but both shared the engine's single budget. So every Safe-mode parser load
asserted, and in release the new state ran with no budget at all. Each state now owns its
budget (`std::unique_ptr`), and the budget is restored together with its state on rollback.

## Acceptance results

### Static (no build needed), 2026-10-13

- `scripts/syntax-check.py` passes on every edited or new C++ TU that has a compile entry
  (Dashboard, DashboardIngest, PlotControlBank, ReplaySeekEngine, PlotMemoryLedger,
  TimeRingSizing, Plot, MultiPlot, PlotCurve, SdfStroke, SdfStrokeMaterial, PlotAreaFill,
  DashboardHandler, Console Handler, Terminal, TerminalBuffer, TerminalCatchUp).
- **The test TUs are a SKIP**, because the compile database has no test entries:
  `tst_ring_exactness`, `tst_sdf_stroke_geometry`, and the edits to `tst_sweep_segments`,
  `tst_time_ring_sizing`, `tst_replay_seek_engine`, `tst_dashboard_ingest` and
  `tst_terminal_buffer`. They compile for the first time in the maintainer's build.
- `code-verify.py --check` on the change set reports 0 errors. Its only advisories are the
  pre-existing `cxx-tu-too-long` on `Dashboard.cpp` and `Terminal.cpp`.
- `--tu-census --check` passes (2636 against a 2637 baseline). `layer-verify` reports nothing
  from this change. Its one error is `tst_deploy_resource_tree.cpp`, from the concurrent spec
  0096 session.
- `claim-verify.py` reports 0 errors across the AI-facing docs.
- `documentation-verify.py` finds no new findings in the edited docs.
- `reuse lint-file` passes on every new file.
- **AC4 (CI check):** `lock-memory-scope` reports 0 findings on the tree and fires on a
  synthetic non-allowlisted call.
- The HDR transfer block in `stroke_sdf.frag` is byte-identical to `stroke_hdr.frag`.

### Static review (`qt-cpp-review`), 2026-10-13

- **Lint:** 0 findings.
- **Six analysis lenses:** findings were fixed as listed in `tasks.md` "Review fixes". The
  ones that would have broken an acceptance criterion:
  - every stream ring grew 1.25× at its first wrap, and rings that fit were flagged
    ceiling-limited (AC5);
  - the 10 Hz frame-lane AC1 rows exceeded the byte floor (R5);
  - initial, replay and sweep allocations bypassed the 512 MB rail (R7);
  - history growth reset a running or frozen sweep (R4);
  - the console flood cap miscounted collapsed duplicates (R13).
- **After the fixes:**
  - `syntax-check.py` passes on every edited TU.
  - The edited test files, which SKIP in the database, pass a manual `-fsyntax-only` with a
    core TU's flags.
  - `code-verify.py --check` reports 0 errors and 0 new advisories.
  - `--tu-census --check` reports 2635 against the 2637 baseline.
  - `claim-verify.py` reports 0 errors.

### Runtime (pending the maintainer's build)

Each item is checked off in spec.md once run:
- ctest: AC1, AC2, AC3, AC5, AC8 and AC11 at unit level;
- `--benchmark-hotpath` (AC12), compared to the baseline above;
- `tests/performance/test_plot_pressure.py` (AC6, and AC9 with `SS_PLOT_PRESSURE_PROFILE=1`);
- the audio rig (AC7);
- the visual side-by-side (AC10);
- the in-app console hide/show (AC11).
