# Architecture — Data Flow, Threading & the Hotpath

> Part of the architecture corpus ([index](../architecture.md)). Read this file in full
> before touching anything on the Driver → FrameReader → FrameBuilder → Dashboard path.
> The most dangerous rules are also summarized inline in CLAUDE.md under
> "Threading & Hotpath — Non-Negotiable"; the `ss-hotpath` skill re-states them at edit time.

## Data Flow

```
Driver  (driver thread OR main, depending on driver)
  │ HAL_Driver::dataReceived(CapturedDataPtr)           AutoConnection
  ▼
FrameReader::processData  (main thread)
  │ appends to CircularBuffer (SPSC); tracks per-chunk timestamps;
  │ delimiter scan: vectorized memchr for 1-byte delimiters, memchr-anchored
  │ + memcmp for <= 8-byte patterns on the linear region, KMP for long or
  │ wrap-straddling patterns; extracted frames fill REUSED CapturedData pool
  │ slots (use_count()==1 probe, peekRangeInto writes the slot's QByteArray
  │ in place — steady-state zero-allocation; backlog falls back to heap);
  │ enqueues to lock-free ReaderWriterQueue<CapturedDataPtr>; emits readyRead
  ▼
DeviceManager::onReadyRead  (main, DirectConnection)
  │ while try_dequeue: Q_EMIT frameReady(deviceId, frame)
  ▼
ConnectionManager::onFrameReady  (main)
  │ routes to FrameBuilder::hotpathRxFrame / hotpathRxSourceFrame
  ▼
FrameBuilder  (main)
  │ parse → apply per-dataset transforms → mutate m_frame / m_sourceFrames
  │ Native + PlainText takes the span fast lane (trySpanLane): the engine tokenizes the
  │ raw bytes into the member QByteArrayView scratch (IScriptEngine::parseUtf8Spans,
  │ -1 = unsupported → QList fallback) and applyDatasetValuesSpans writes datasets in
  │ place (assign_utf8_in_place) DIRECTLY into the claimed pool slot — single write per
  │ dataset, steady-state zero-allocation. On this lane m_frame / m_sourceFrames stay
  │ structural templates only (frame() consumers — CSV/MDF4 worker templates,
  │ configureActions — read structure/actions, never live values). JS/Lua always take
  │ the QList<QStringList> path, which still refreshes the template frame's values.
  │ Dashboard gets the pooled TimestampedFramePtr (acquireFrame slot, fast recycle);
  │ async sinks get one detached make_shared copy (their backlog can't pin the pool).
  │ A slot is free exactly when the pool's shared_ptr is its only reference; acquireFrame
  │ probes use_count()==1 and hands out an ALIASING shared_ptr (no per-frame control block,
  │ no deleter). Pool slots fast-path reuse only when generation + sourceId + structure
  │ match; the generation bumps (invalidateFramePool) on project sync/save, QuickPlot
  │ rebuild, op-mode change, and connect/disconnect — stale slots full-assign once, then
  │ recycle. copy_frame_values deep-copies value strings IN PLACE (assign_string_in_place)
  │ so producer strings stay unique and never detach-allocate.
  │ Per-frame singleton polls are cached: operationMode / player-open / any-async-sink /
  │ Dashboard streamAvailable are members refreshed by their owning signals; table-store
  │ dataset capture only runs when a script can read it back (transforms, Lua parser
  │ engines, injected table APIs) — native/script-less projects skip it entirely.
  ▼
Dashboard (pooled)   |   CSV / MDF4 / API / gRPC / Sessions / MQTT (detached copy)
```

## Timestamp Ownership — Source Owns Time

Timing is stamped at the driver boundary and preserved downstream. Do not re-stamp in
export or report workers.

- `IO::CapturedData` (`HAL_Driver.h`): `data` (`QByteArray`, inline COW — no second
  `shared_ptr` indirection), `timestamp` (steady_clock), `frameStep` (ns cadence),
  `logicalFramesHint`. `CapturedDataPtr` is the hotpath transport.
- Drivers publish via `HAL_Driver::publishReceivedData(...)`. When cadence is known, fill
  `frameStep`; when backdatable (e.g. audio: `timestamp = now - step * (totalFrames - 1)`),
  do so. Never emit timing-free `QByteArray`.
- When a driver hops to the main thread (`QMetaObject::invokeMethod`, queued connection),
  capture `SteadyClock::now()` **before** queueing and pass it to `publishReceivedData`.
  Default-constructed timestamps fire on the receiving thread — silent bug.
- `FrameReader` is a splitter, not a clock: `appendChunk` records `PendingChunk
  { nextFrameTimestamp, frameStep }`; `frameTimestamp(endOffsetExclusive)` walks pending
  chunks and advances each chunk's clock by `frameStep` per logical frame.
- `FrameBuilder` interpolates only when one captured chunk expands into N parsed frames:
  publishes at `data->timestamp + step * i`.
- Export workers use `FrameConsumerWorkerBase::monotonicFrameNs(frame->timestamp, baseline)`
  as a strictly-increasing safety net against same-ns collisions on coarse clocks (Windows
  `steady_clock` ~15 ms). Not the source of truth.
- Debug order when timing looks wrong: driver stamp → `CapturedData` propagation → FrameReader
  split → FrameBuilder fan-out → export/report. Never patch PDF/Chart.js first.

## Threading Rules — DO NOT VIOLATE

| Component | Rule |
|-----------|------|
| `FrameReader` | **Main thread.** Config set once before construction; recreate via `ConnectionManager::resetFrameReader()` / `DeviceManager::reconfigure()`. **Never add mutexes.** Single-delimiter uses KMP; multi uses `CircularBuffer::findFirstOfPatterns()` (single-pass, stack array ≤8). Preserves driver timing via `PendingChunk` spans. (Historical: threaded extraction removed in beeda4c0; if it returns, `DeviceManager::frameReady` / `rawDataReceived` go back to `Qt::QueuedConnection`.) |
| `CircularBuffer` | **SPSC only.** Driver writes from whatever thread emitted `dataReceived`; reader is FrameReader on main. Never MPMC. |
| `Dashboard` | **Main thread only.** Reads the shared `TimestampedFramePtr`. |
| Export workers | Lock-free enqueue from main; batch on worker thread. Consume a detached `make_shared` copy of the frame (NOT the Dashboard's pooled slot), so a slow worker's backlog can't pin the pool. |

**Hotpath signal hops must be `Qt::DirectConnection`.** A queued connection between two
main-thread objects costs a `QMetaCallEvent` alloc + event-queue insertion per emit; at
10+ kHz that fills FrameReader's 65536-slot queue faster than the consumer drains and
trips `Frame queue full — frame dropped`. Known direct sites:

- `DeviceManager::frameReady → ConnectionManager::onFrameReady`
- `DeviceManager::rawDataReceived → ConnectionManager::onRawDataReceived`
- `FrameReader::readyRead → DeviceManager::onReadyRead` (AutoConnection resolves Direct)

## Cached Hotpath Flags

The hotpath reads **cached** flags, never live getters: `m_operationMode`, `m_playerOpen`,
`m_anyAsyncSink`, `m_captureLatestFrame`, `m_changeDriven`, and Dashboard
`m_streamAvailable`. A new input to any of them must wire its change signal to the matching
cache refresh (`updateStreamAvailable` / `refreshAnyAsyncSink` / the player lambdas) or
frames/exports silently stop. Wire the refresh with `Qt::DirectConnection` — a queued
refresh lags a full event-loop turn behind frames already flowing. `streamAvailable()` also
carries the spec-0040 mirror input: a leading `[[unlikely]]` read of
`API::MirrorSession::mirroring()`, deliberately a plain module-static bool (never a
`MirrorSession` construction — the getter runs inside Dashboard's ctor); see
[mirror.md](mirror.md).

- `m_changeDriven` (project property `changeDrivenTransforms`, opt-in/off by default) skips a
  virtual dataset's transform when none of its captured read-set slots changed since its last
  run (per-slot version vs `DataTableStore::writeClock`); refreshed in
  `refreshDatasetCaptureFlag`. "Changed" means value change, not write: the store's
  computed-register write paths treat an identical value as a successful no-op and skip the
  version bump, so a parser rewriting the same value every frame doesn't defeat the skip.
- `m_captureLatestFrame` (control script running or API server on) gates the latest-frame
  capture behind `io.getLatestFrame`: it retains one `CapturedDataPtr` per source (the
  FrameReader pool probe skips pinned slots) plus the channel tokens — keep it gated and
  allocation-free.

## Diagnostic Counters — Pulled at 1 Hz (spec 0033)

The problem center reads link and script health from **plain counters polled on
`Misc::TimerEvents::timeout1Hz`**, never from a per-frame signal. The frame path only
increments; nothing on it emits, allocates, locks, or calls into `Misc::ProblemCenter`.

- **`FrameReader`** (main-thread, SPSC — plain `quint64`, no atomics): `m_bytesIn` (chunk size
  in `processData`), `m_framesExtracted` (next to the existing `noteDroppedFrame` accounting in
  `enqueueCaptured` and the `NoDelimiters` branch), `m_checksumErrors` (inside the existing
  `ValidationStatus::ChecksumError` branch), and `m_totalOverflowBytes`, accumulated inside the
  existing `if (overflow > 0)` guard immediately before `resetOverflowCount()` — which
  previously destroyed the number. `resetDiagnosticCounters()` clears all four. The per-failure
  checksum hex dump is throttled to the same 5 s pattern `noteDroppedFrame` uses.
- **`FrameBuilder`**: `m_transformErrors` increments in the existing transform-error branches;
  `m_lastTransformError` / `m_lastTransformDatasetUniqueId` are assigned **only when the failing
  dataset differs from the last recorded one**, so a dataset that throws every frame allocates
  the message once rather than per frame (`noteTransformError`, `SS_COLD`).
- **Reading them.** `DeviceManager::frameReader()` exposes the reader; `ConnectionManager::
  linkStats()` sums the per-device counters into an `IO::LinkStats` POD. It is called from the
  1 Hz tick only — no caching, no signal, no call site on the frame path. Script health comes
  from `FrameParser::scriptStats()` (per-source engine counters) and `FrameBuilder::
  parsedFrameCount()`.

**A `FrameReader` is recreated, not reused** (`resetFrameReader()` / `DeviceManager::
reconfigure()`), so the counters restart at zero on every connect and config change. The link
checkers therefore work on **deltas against the previous sample and treat any decrease as a
reset**, never on absolute totals. Finding text is bucketed ("more than 1,000") so it stays
stable while the condition is stable — a live count in the string would reset the panel's model
once per second.

None of these counters is an input to a cached hotpath flag, and none of them gates frame
processing.

## Replay Ingestion (spec 0020)

ProjectFile replay does not travel the byte pipeline: players call
`FrameBuilder::replayChannels(sourceId, channels, recordedTs)` with already-split cells, and
`publishReplayFrame` fans out to the dashboard (pooled slot) plus API/gRPC observers only —
**recording sinks never see replayed frames**. While a player is open, transform engines are
destroyed and `m_captureDatasetValues` is forced off (`refreshDatasetCaptureFlag` gates on
`!m_playerOpen`); the player `openChanged` lambdas set `m_captureFlagsDirty` on both edges and
recompile transforms on close. Scrubbing bulk-fills plot rings via
`Dashboard::bulkLoadPlotWindow` (rings only, plot clocks reset, times normalized to end at 0);
a debounced settle pass replays the exact trailing window through `replayChannels`.

## Hotpath Benchmark — The 256 kHz CI Gate

256 kHz is a CI gate, not a slogan. `--benchmark-hotpath` (`Benchmark::HotpathBenchmark`) drives the
real parse pipeline in-process — `FrameReader` extraction → `FrameBuilder` → frame parser →
per-dataset transforms → Dashboard — against a project loaded programmatically via
`ProjectModel::loadFromJsonDocument`. Seven runs are gated, all tiered off `--min-fps` (default
256000) so a `--min-fps 1` PGO training run stays effectively ungated: **data-pipeline** at 4x
(1.024 MHz; `runDataPipeline` — `FrameReader` extraction only, no parse; `HOTPATH_DATA_FPS`),
**Native numeric** at 4x (1.024 MHz; `CFrameParser` delimited template,
`HOTPATH_NATIVE_FPS`), **Native mixed** at 2x (512 kHz),
**Lua numeric** at `min-fps` (256 kHz), **JS numeric** at half (128 kHz), **Lua mixed**
(numeric + string columns) at half (128 kHz), **JS mixed** at a quarter (64 kHz).
Numeric runs drop both the 3 string chunk columns and the string datagrid group from the project;
mixed runs keep them. The synthetic chunk is built once *before* the timed loop (string columns
included), so chunk/string construction never contaminates the measurement. The exit code (and
`HOTPATH_PASS`) is nonzero if *any* gated run misses its tier. It then runs an ungated **Lua +
all exporters live** pipeline (CSV/MDF4/Sessions/API/gRPC, mixed workload — the
exporter-slowdown readout compares against the Lua-mixed baseline) for PGO
training, and an ungated **Lua + dashboard** pipeline that loads an all-widget-types project, sets
`HotpathBenchmark::active()` (which `Dashboard::streamAvailable()` honors so headless frames are
accepted with no live device), arms every plot/FFT/multiplot/waterfall/GPS/3D widget, and trains
the per-frame dashboard sub-hotpaths + a dashboard-slowdown readout. The gated runs disable the
`FrameBuilder` parse-budget guard (an interactive 80%-duty throttle that a 100%-duty benchmark
would trip every window) via `setParseBudgetEnabled(false)` and run **no** exporters or dashboard,
so the gate measures pure parse capacity; the exporter and dashboard phases are deliberately *not*
gated (their consumers can't drain faster than a flat-out producer, so the 8192-slot pool exhausts
into the heap-fallback path — that penalty is the point of the readout). Each run lasts until both
the `--benchmark-frames` floor (default 1M) and the `--benchmark-seconds` window (default 10) are
met. Throughput = `FrameBuilder::parsedFrameCount()` / elapsed; `--benchmark-output FILE` mirrors
the report to a file (default: stdout only). `ci.yml` (the only workflow) runs it per push/PR
as a hard gate on the PGO-optimized binary. Don't regress the parse hotpath. (The `ss-hotpath` skill auto-activates
on hotpath edits and re-states this check.)

The optimization/hardening/sanitizer/allocator flags this gate is measured under live in four
cmake modules (`cmake/Optimization.cmake`, `Hardening.cmake`, `Sanitizers.cmake`, `MiMalloc.cmake`),
one per-toolchain branch each; the `cpp-compiler-flags` skill maps them and the two-stage PGO flow.

**CI gotcha — benchmarking the Windows GUI-subsystem exe.** Running the `--benchmark-hotpath`
(or any CLI) path of the GUI-subsystem (`WIN32_EXECUTABLE TRUE`) Windows exe and expecting the
shell to wait + capture stdout fails: a `/SUBSYSTEM:WINDOWS` binary **detaches from the
launching console** — `cmd`/PowerShell don't wait for it, its stdout is unwired, and the
`AttachConsole`+`CONOUT$` fallback writes to the console *screen buffer* that GitHub's
pipe-based log capture never reads → CI hangs with no output and no exit code
(`Start-Process -Wait` and plain `bash` both fail differently). For CI, benchmark a throwaway
`editbin /SUBSYSTEM:CONSOLE` **copy** of the exe — a console-subsystem image stays attached, so
the shell waits, stdout pipes through, and the exit code propagates. Leave the shipped exe
`/SUBSYSTEM:WINDOWS` so it never flashes a console for end users. Background:
<https://www.devever.net/~hl/win32con>
