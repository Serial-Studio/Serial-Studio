---
name: ss-hotpath
description: >-
  Serial Studio data-hotpath rules and the 256 kHz throughput gate. Use BEFORE editing or
  reviewing FrameReader, CircularBuffer, FrameBuilder, ConnectionManager, DeviceManager, or
  Dashboard frame-draw code — anything on the Driver → FrameReader → FrameBuilder → Dashboard
  path. Covers SPSC/pipeline-thread rules, DirectConnection requirement, the no-alloc/no-copy slot
  pool, source-owns-time, and how to measure throughput with --benchmark-hotpath.
paths:
  - app/src/IO/FrameReader.*
  - app/src/IO/CircularBuffer.h
  - app/src/IO/ConnectionManager.*
  - app/src/IO/DeviceManager.*
  - app/src/DataModel/FrameBuilder.*
  - app/src/DataModel/HotpathOptimization.h
  - app/src/UI/Dashboard.*
  - app/src/DSP.h
  - app/src/DSPSimd.h
  - app/src/IO/StreamWorker.*
  - app/src/IO/PipelineHost.*
---

# Serial Studio — data hotpath

You are touching the highest-risk code in the repo. Read the target file **in full** first
(`doc/claude/architecture/dataflow.md` has the full data-flow and threading model). These rules are
non-negotiable; violating them causes silent frame drops, not compile errors.

**Verbalize before the first edit** (J-space discipline 1+3, `doc/claude/j-space.md`): state
in chat, in your own words, the 3-5 hard rules below that *this specific change* is exposed
to — e.g. "this adds a hop after `onFrameReady`, so it must be `DirectConnection` and
allocation-free". Hotpath edits look like familiar Qt code, which is exactly the automatic
mode where these rules get violated silently; naming the binding rules at the point of
action is the deliberate-mode interrupt. Do not proceed straight from pattern-match to `Edit`.

## Data flow

`Driver → FrameReader::processData (pipeline thread) → PipelineHost::routeFrames →
FrameBuilder → shared TimestampedFramePtr → PipelineHost dashboard ring →
Dashboard::onDisplayTick (GUI) | CSV / MDF4 / API / Sessions (detached copy)`

Dense typed sources (audio) skip all of that: driver `SampleBlock` → per-source
`IO::StreamWorker` thread → bounded display update / export block / latest values, all per
block. Never per-sample across a thread boundary.

## Hard rules

- **The frame pipeline lives on `IO::PipelineHost`'s thread, not the GUI thread** (spec 0051).
  GUI→pipeline calls self-marshal via `PipelineHost::runOnObjectThread` (event-loop backed);
  pipeline→GUI reads via `runOnGuiThreadBlocking`. A plain `BlockingQueuedConnection` from the
  GUI into the pipeline deadlocks against a script's apiCall — never add one.
- **`FrameReader` and `CircularBuffer` are pipeline-thread / SPSC. Never add a mutex.** Reconfigure
  by recreating via `resetFrameReader()` / `reconfigure()`, never by locking.
- **Hotpath signal hops must be `Qt::DirectConnection`.** A queued connection between two
  pipeline-thread objects fills the slot queue at 10+ kHz and drops frames. GUI↔pipeline
  traffic must stay chunk/command/tick rate.
- **No allocation and no block copy on the dashboard path.** Draw blocks from
  `FrameBuilder::claimBlockSlot()` (the slot pool) — never `make_shared<DataModel::DataBlock>`
  directly. The one detached `clone_block_trimmed` copy in the async-sink fan-out is
  intentional (slow export path, gated on a sink being on, keeps a backlog from pinning the
  pool) — not a violation.
- **The hotpath reads cached flags** (`m_operationMode`, `m_playerOpen`, `m_anyAsyncSink`,
  `m_captureLatestFrame`, `m_changeDriven`, Dashboard `m_streamAvailable`). A new input to any
  of them must wire its change signal to the cache refresh, or frames/exports silently stop
  (mechanics in `doc/claude/architecture/dataflow.md` "Cached Hotpath Flags"; see also
  `doc/claude/common-mistakes.md`). `streamAvailable()` also reads the spec-0040 mirror flag —
  a plain module-static bool via `API::MirrorSession::mirroring()`, never a construction
  (`doc/claude/architecture/mirror.md`).
- **Native + PlainText parses through the span fast lane** (`trySpanLane` → `parseUtf8Spans` →
  `applyDatasetValuesSpans`): byte views + in-place QString writes (`assign_utf8_in_place` /
  `assign_string_in_place`, never implicit-share assignment — a share-assign re-links buffers
  and degrades back to per-frame mallocs), zero steady-state allocation. Keep anything you add
  to that lane allocation-free.
- **Every dashboard publish site stamps `structureGeneration = m_framePoolGeneration`** —
  pool slot and heap fallback alike. The dashboard skips per-frame `compare_frames()`
  revalidation when the cached per-source generation matches; a frame left at the default `0`
  (or stale) makes `Dashboard::hotpathRxFrame` reconfigure every frame or never reconfigure
  after a real layout change. The generation only advances via `invalidateFramePool()`.
- **`m_captureLatestFrame`** (control script running or API server on) gates the latest-frame
  capture behind `io.getLatestFrame`: one retained `CapturedDataPtr` per source (the pool probe
  skips pinned slots) plus the channel tokens. Keep it gated and allocation-free.
- **Source owns time.** Stamp at the driver boundary; never re-stamp in export/report workers
  (`monotonicFrameNs(...)` is the safety net only).
- **Optimization macros come from `app/src/DataModel/HotpathOptimization.h`** (`SS_FORCE_INLINE`,
  `SS_FLATTEN`, `SS_HOT`/`SS_COLD`, `SS_RESTRICT`, `SS_ASSUME`, `SS_NO_UNROLL`, ...). Annotate
  the `.h` declaration and `.cpp` definition in lockstep. Never add a fast-math / no-unwind /
  GCC `optimize("...")` macro (breaks the IEEE-stable math + Lua-unwind invariants). `SS_ASSUME`
  must restate a guard that already ran, never a precondition on a parsed frame.
- **Fixed loop bounds + assertion density ≥ 2 per function** (NASA Power of Ten). The frame
  extractors cap iterations at `kMaxFramesPerCall`; keep any new loop bounded the same way.
- **Asserts on the per-frame/per-cell kernels are `SS_ASSERT_HOTPATH(cond)`** (debug-only,
  compiles out of release): even the evaluated pass-path branch of `SS_ASSERT` is measurable
  at rate (the 2026-07 sweep cost ~5% throughput). Admissible only where the condition
  restates a guard that provably already ran — a condition derived from device bytes keeps
  `SS_ASSERT` and its release recovery. The `hotpath-assert-scope` lint pins the macro to
  the hotpath TUs.

## Block publication, time rings & kernels — the newest invariants

- **One publication payload, one ingestion path (spec 0055).** Nothing publishes a
  `TimestampedFrame` any more: `FrameBuilder` stages parsed rows into a pooled
  `DataModel::DataBlock` and flushes on the display tick or a sample cap. Two caps, not one --
  `kFrameBlockSampleCap` (64) because frame-lane columns carry a display string per sample,
  `kStreamBlockSampleCap` (4096) for the numeric-only dense lane. Dense sources (Audio, or
  `streamLane` on) do all per-sample work on their own `IO::StreamWorker` thread but emit
  `blockReady` **queued to the pipeline thread**, the SINGLE producer for every sink. Sinks
  share ONE `clone_block_trimmed` copy. Structure travels separately as a `StructureSnapshot`
  on pool-generation bumps. Never add a rate cap or a per-view reduction.
  Detail: `doc/claude/architecture/dataflow.md`.
- **Time rings are sized from a rate, never a sample count alone** (`kMaxRateSizedRingSamples`
  ceiling): stream lane sizes at build from the real rate (`streamRingCapacity`); the frame lane
  re-sizes a *saturated* ring once from the plot clock's smoothed period (`growTimeRing`, upward
  only). `m_plotClocks` and `m_plotDisplayTimeSec` are ONE state — cleared, saved and restored
  together via `Dashboard::resetPlotClocks()`, never one without the other. `appendDecimated`
  clamps sub-cell backward jitter forward; a jump back over a whole cell drops the retained span
  (clamping it wedges the ring shut). Detail: `doc/claude/architecture/dashboard.md`.
- **Kernels and macros:** portable SIMD lives in `app/src/DSPSimd.h` (spec 0021, per-lane
  bit-exact vs scalar) — reuse it, never inline intrinsics at a call site. Optimizer macros live
  in `app/src/DataModel/HotpathOptimization.h`; annotate `.h` and `.cpp` in lockstep, never add a
  fast-math / no-unwind / GCC `optimize("...")` macro, and use `SS_ASSERT_HOTPATH` (not
  `SS_ASSERT`) on per-frame/per-cell kernels. Detail: `doc/claude/architecture/kernels.md`.

## Measure, don't guess

The documented "256 kHz data rate" is a CI gate, not a slogan. To check throughput after a
change, build the app and run the in-process end-to-end benchmark:

```
serial-studio-pro --headless --benchmark-hotpath --min-fps 256000
```

It loads a project via `ProjectModel::loadFromJsonDocument` and drives the real pipeline —
`FrameReader` extraction → `FrameBuilder` → frame parser → per-dataset transforms. The exit
code (the release gate) fails if any gated tier misses.

**Nine gated runs**, all tiered off `--min-fps` (so a `--min-fps 1` PGO training run stays
effectively ungated). The seven parser gates disable the parse-budget guard (the spec-0051
fair-share governor, which a 100%-duty benchmark would engage) and run no exporters/dashboard, so
they measure pure parse capacity; the two Lua reference floors run with consumers on and
exist to catch a consumer-path collapse, not to measure parsing:

| Run | Tier | Default gate |
|-----|------|--------------|
| data-pipeline (`FrameReader` extraction only, no parse; `HOTPATH_DATA_FPS`) | 4x | 1.024 MHz |
| native(numeric) | 4x | 1.024 MHz |
| native(mixed) | 2x | 512 kHz |
| lua(numeric) | 1x | 256 kHz |
| js(numeric), lua(mixed) | 0.5x | 128 kHz |
| js(mixed) | 0.25x | 64 kHz |
| lua+exporters, lua+dashboard (floors) | 0.5x | 128 kHz |

Mechanics and readouts:

- Throughput = `FrameBuilder::parsedFrameCount()` / elapsed. The synthetic chunk — string
  columns included — is built once before the timed loop, so chunk/string construction never
  sits in the measurement.
- A Native stage breakdown prints as `hotpath-stage[native]` (extract / tokenize /
  datasets+publish). `datasets+publish` is ~70-80% of per-frame time — gate any change there
  with this benchmark.
- Three Lua reference rows follow: `lua+exporters` (CSV/MDF4/Sessions/API/gRPC, mixed
  workload; prints `hotpath: exporters cost N.NNx throughput`), `lua+dashboard` (loads an
  all-widget-types project, flips `HotpathBenchmark::active()` so `Dashboard::streamAvailable()`
  accepts headless frames, arms every plot/FFT/multiplot/waterfall/GPS/3D widget; prints
  `hotpath: dashboard costs N.NNx`), and `lua+dashboard(off)` (same project, dashboard ingest
  off; prints the ingest on-vs-off cost). Exporter/dashboard workers can't keep up with a
  flat-out producer, so the pool exhausts into heap fallback — that penalty is the readout.
  The first two carry the 0.5x floor gates; `lua+dashboard(off)` stays ungated.
- An ungated engine × {numeric, mixed} × {exporters, dashboard} coverage matrix runs last so
  CI and PGO training exercise every consumer/engine combination.
- `--benchmark-frames N` sets the minimum workload; `--benchmark-seconds N` the minimum
  wall-clock window (default 10) — each run lasts until both floors are met.
  `--benchmark-output FILE` mirrors the report to a file (default: stdout only).

Source: `app/src/Benchmark/HotpathBenchmark.cpp`. CI (`ci.yml`, the only workflow) runs it on
every push/PR as a hard gate on the PGO-optimized binary — the same binary that ships (PGO
GENERATE → `--min-fps 1` training run → PGO USE → gated `--min-fps 256000`). The same engine backs the in-app
About → Benchmark dialog (`Benchmark::BenchmarkRunner`, exposed as `Cpp_Benchmark_Runner`).
**Do not regress the parse hotpath.**

After any change here, re-read the diff against these rules before handing off, and run
`python scripts/code-verify.py --check <files>` (hotpath violations are blockers, not advisories).
