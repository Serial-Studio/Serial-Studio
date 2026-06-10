---
name: ss-hotpath
description: >-
  Serial Studio data-hotpath rules and the 256 kHz throughput gate. Use BEFORE editing or
  reviewing FrameReader, CircularBuffer, FrameBuilder, ConnectionManager, DeviceManager, or
  Dashboard frame-draw code — anything on the Driver → FrameReader → FrameBuilder → Dashboard
  path. Covers SPSC/main-thread rules, DirectConnection requirement, the no-alloc/no-copy slot
  pool, source-owns-time, and how to measure throughput with --benchmark-hotpath.
paths:
  - app/src/IO/FrameReader.*
  - app/src/IO/CircularBuffer.h
  - app/src/IO/ConnectionManager.*
  - app/src/IO/DeviceManager.*
  - app/src/DataModel/FrameBuilder.*
  - app/src/DataModel/HotpathOptimization.h
  - app/src/UI/Dashboard.*
---

# Serial Studio — data hotpath

You are touching the highest-risk code in the repo. Read the target file **in full** first
(`doc/claude/architecture.md` has the full data-flow and threading model). These rules are
non-negotiable; violating them causes silent frame drops, not compile errors.

## Data flow

`Driver → FrameReader::processData (main) → DeviceManager::onReadyRead →
ConnectionManager::onFrameReady → FrameBuilder → shared TimestampedFramePtr →
Dashboard / CSV / MDF4 / API / Sessions`

## Hard rules

- **`FrameReader` and `CircularBuffer` are main-thread / SPSC. Never add a mutex.** Reconfigure
  by recreating via `resetFrameReader()` / `reconfigure()`, never by locking.
- **Hotpath signal hops must be `Qt::DirectConnection`.** A queued connection between two
  main-thread objects fills the slot queue at 10+ kHz and drops frames.
- **No allocation and no `Frame` copy on the dashboard path.** Draw frames from
  `FrameBuilder::acquireFrame()` (the slot pool) — never `make_shared<TimestampedFrame>` directly.
  The one detached copy in the `hotpathTxFrame` async-sink fan-out is intentional (slow export
  path, gated on a sink being on, keeps a backlog from pinning the pool) — not a violation.
- **The hotpath reads cached flags** (`m_operationMode`, `m_playerOpen`, `m_anyAsyncSink`,
  `m_captureLatestFrame`, Dashboard `m_streamAvailable`). A new input to any of them must wire
  its change signal to the cache refresh, or frames/exports silently stop
  (see `doc/claude/common-mistakes.md`).
- **Native + PlainText parses through the span fast lane** (`trySpanLane` → `parseUtf8Spans` →
  `applyDatasetValuesSpans`): byte views + in-place QString writes (`assign_utf8_in_place`),
  zero steady-state allocation. Keep anything you add to that lane allocation-free.
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

## Measure, don't guess

The documented "256 kHz data rate" is a CI gate, not a slogan. To check throughput after a
change, build the app and run the in-process end-to-end benchmark:

```
serial-studio-pro --headless --benchmark-hotpath --min-fps 256000
```

It loads a project via `ProjectModel::loadFromJsonDocument` and drives the real pipeline —
`FrameReader` extraction → `FrameBuilder` → frame parser → per-dataset transforms. The exit
code (the release gate) fails if any gated tier misses.

**Seven gated runs**, all tiered off `--min-fps` (so a `--min-fps 1` PGO training run stays
effectively ungated). Gated runs disable the parse-budget guard (an interactive throttle a
100%-duty benchmark would trip every window) and run no exporters/dashboard, so the gate is
pure parse capacity:

| Run | Tier | Default gate |
|-----|------|--------------|
| data-pipeline (`FrameReader` extraction only, no parse; `HOTPATH_DATA_FPS`) | 4x | 1.024 MHz |
| native(numeric) | 4x | 1.024 MHz |
| native(mixed) | 2x | 512 kHz |
| lua(numeric) | 1x | 256 kHz |
| js(numeric), lua(mixed) | 0.5x | 128 kHz |
| js(mixed) | 0.25x | 64 kHz |

Mechanics and readouts:

- Throughput = `FrameBuilder::parsedFrameCount()` / elapsed. The synthetic chunk — string
  columns included — is built once before the timed loop, so chunk/string construction never
  sits in the measurement.
- A Native stage breakdown prints as `hotpath-stage[native]` (extract / tokenize /
  datasets+publish). `datasets+publish` is ~70-80% of per-frame time — gate any change there
  with this benchmark.
- Three ungated Lua reference rows follow: `lua+exporters` (CSV/MDF4/Sessions/API/gRPC, mixed
  workload; prints `hotpath: exporters cost N.NNx throughput`), `lua+dashboard` (loads an
  all-widget-types project, flips `HotpathBenchmark::active()` so `Dashboard::streamAvailable()`
  accepts headless frames, arms every plot/FFT/multiplot/waterfall/GPS/3D widget; prints
  `hotpath: dashboard costs N.NNx`), and `lua+dashboard(off)` (same project, dashboard ingest
  off; prints the ingest on-vs-off cost). Exporter/dashboard workers can't keep up with a
  flat-out producer, so the pool exhausts into heap fallback — that penalty is the readout,
  not a gate.
- An ungated engine × {numeric, mixed} × {exporters, dashboard} coverage matrix runs last so
  CI and PGO training exercise every consumer/engine combination.
- `--benchmark-frames N` sets the minimum workload; `--benchmark-seconds N` the minimum
  wall-clock window (default 10) — each run lasts until both floors are met.
  `--benchmark-output FILE` mirrors the report to a file (default: stdout only).

Source: `app/src/Benchmark/HotpathBenchmark.cpp`. CI runs it on every PR (`test.yml`) and as a
hard release gate on the shipped PGO binary (`deploy.yml`). The same engine backs the in-app
About → Benchmark dialog (`Benchmark::BenchmarkRunner`, exposed as `Cpp_Benchmark_Runner`).
**Do not regress the parse hotpath.**

After any change here, re-read the diff against these rules before handing off, and run
`python scripts/code-verify.py --check <files>` (hotpath violations are blockers, not advisories).
