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
- **Source owns time.** Stamp at the driver boundary; never re-stamp in export/report workers.
- **Fixed loop bounds + assertion density ≥ 2 per function** (NASA Power of Ten). The frame
  extractors cap iterations at `kMaxFramesPerCall`; keep any new loop bounded the same way.

## Measure, don't guess

The documented "256 kHz data rate" is a CI gate, not a slogan. To check throughput locally
after a change, build the app and run the in-process end-to-end pipeline benchmark:

```
serial-studio-pro --headless --benchmark-hotpath --min-fps 256000
```

It loads a project via `ProjectModel::loadFromJsonDocument` and drives the path —
`FrameReader` extraction → `FrameBuilder` → frame parser → per-dataset transforms → Dashboard.
It first runs an ungated **data-pipeline** pass (`FrameReader` extraction only, no parse) that
reports raw driver throughput in the MHz range (`HOTPATH_DATA_FPS`), then a **Lua** pipeline
(gated; this exit code is the release gate), an ungated **JS** pipeline,
an ungated **Lua + all exporters** pipeline (CSV/MDF4/Sessions/API/gRPC) that prints the
exporter slowdown (`hotpath: exporters cost N.NNx throughput`) and trains the export hot paths,
and an ungated **Lua + dashboard** pipeline that loads an all-widget-types project, flips
`Benchmark::HotpathBenchmark::active()` so `Dashboard::streamAvailable()` accepts headless frames, arms
every plot/FFT/multiplot/waterfall/GPS/3D widget, and trains the per-frame dashboard sub-hotpaths
(`hotpath: dashboard costs N.NNx throughput`). The gated runs disable the parse-budget guard (an
interactive throttle a 100%-duty benchmark would trip every window) and run no exporters/dashboard,
so the gate is pure parse capacity; the exporter and dashboard phases are not gated (workers can't
keep up with a flat-out producer, so the pool exhausts into heap fallback — that penalty is the
readout). Throughput = `FrameBuilder::parsedFrameCount()` / elapsed.
`--benchmark-frames N` sets the minimum workload; `--benchmark-seconds N` sets the minimum wall-clock
window (default 10) — each run lasts until both floors are met. `--benchmark-output FILE` mirrors the
report to a file (default: stdout only, no file). Source:
`app/src/Benchmark/HotpathBenchmark.cpp`. CI runs this on every PR (`test.yml`) and as a hard
release gate on the shipped PGO binary (`deploy.yml`). The same engine backs an in-app dialog
(`Benchmark::BenchmarkRunner`, `app/src/Benchmark/`, exposed as `Cpp_Benchmark_Runner`) reachable
from About → Benchmark. **Do not regress the parse hotpath.**

After any change here, re-read the diff against these rules before handing off, and run
`python scripts/code-verify.py --check <files>` (hotpath violations are blockers, not advisories).
