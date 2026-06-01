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
(`docs/claude/architecture.md` has the full data-flow and threading model). These rules are
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
after a change, build the app and run the in-process extraction benchmark:

```
serial-studio-pro --headless --benchmark-hotpath --min-fps 256000
```

It drives the real `FrameReader` + `CircularBuffer` path and exits non-zero below `--min-fps`
(default 256000). `--benchmark-frames N` sets the workload size. Source:
`app/src/Misc/HotpathBenchmark.cpp`. CI runs this on every PR (`test.yml`) and as a hard
release gate on the shipped PGO binary (`deploy.yml`). **Do not regress the extraction hotpath.**

After any change here, re-read the diff against these rules before handing off, and run
`python scripts/code-verify.py --check <files>` (hotpath violations are blockers, not advisories).
