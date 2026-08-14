# Architecture — Data Flow, Threading & the Hotpath

> Part of the architecture corpus ([index](../architecture.md)). Read this file in full
> before touching anything on the Driver → FrameReader → FrameBuilder → Dashboard path.
> The most dangerous rules are also summarized inline in CLAUDE.md under
> "Threading & Hotpath — Non-Negotiable"; the `ss-hotpath` skill re-states them at edit time.

## Data Flow

Since spec 0051 M3 the frame pipeline runs on a dedicated **processing thread**
("FramePipeline", owned by `IO::PipelineHost`, a SessionContext module adopted between
FrameBuilder and ConnectionManager). FrameReaders, the FrameParser engines and the
FrameBuilder all live there; the GUI thread only drains finished pooled frames from an
SPSC ring on the display tick (`Dashboard::onDisplayTick`).

```
Driver  (driver thread OR main, depending on driver)
  │ HAL_Driver::dataReceived(CapturedDataPtr)           Auto → QUEUED to pipeline (chunk rate)
  ▼
FrameReader::processData  (pipeline thread — moveToThread at creation)
  │ appends to CircularBuffer (SPSC); tracks per-chunk timestamps;
  │ delimiter scan: vectorized memchr for 1-byte delimiters, memchr-anchored
  │ + memcmp for <= 8-byte patterns on the linear region, KMP for long or
  │ wrap-straddling patterns; extracted frames fill REUSED CapturedData pool
  │ slots (use_count()==1 probe, peekRangeInto writes the slot's QByteArray
  │ in place — steady-state zero-allocation; backlog falls back to heap);
  │ enqueues to lock-free ReaderWriterQueue<CapturedDataPtr>; emits readyRead
  ▼
PipelineHost::routeFrames  (pipeline thread, DirectConnection — same thread)
  │ drains the reader queue; routes by ATOMIC operation-mode/paused mirrors
  │ (written on the GUI thread at transition rate) to FrameBuilder; MQTT raw-frame
  │ fan-out rides along
  ▼
FrameBuilder  (pipeline thread — moveToThread as the LAST composition-root step)
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
PipelineHost dashboard ring (SPSC, 8192 = pool size; gated on the dashboardAccepting
mirror so no-dashboard sessions never pin pool slots; full ring = counted drop, the
pool-exhaustion warning carries the user signal)
  ▼
Dashboard::onDisplayTick drain (GUI thread, uiTimeout)   |   CSV / MDF4 / API / gRPC /
Sessions / MQTT (detached copy, SPSC queues — single producer is now the pipeline thread)
```

### Cross-thread marshal protocol (spec 0051 M3)

The pipeline is single-threaded internally; the thread boundary is crossed only by:

- **GUI → pipeline commands**: FrameBuilder/FrameParser public mutators self-marshal
  (`invokeOnBuilderThreadBlocking` / `IO::PipelineHost::runOnObjectThread`). GUI-side
  waits spin a QEventLoop so the GUI keeps serving the pipeline's own blocking apiCall
  dispatches — never a plain BlockingQueuedConnection from GUI to pipeline (deadlock),
  with ONE exception: the player replay lanes (`replayChannels*`) use plain
  BlockingQueued because transform engines are torn down during replay, so no apiCall
  cycle can exist and the borrowed span/text pointers must stay alive.
- **pipeline → GUI reads**: `runOnGuiThreadBlocking` snapshots GUI-owned state
  (ProjectModel sources/groups/tables). Plain blocking is safe by protocol: the GUI
  never plain-blocks on the pipeline. `resolveDecoderMethod` never reaches ProjectModel —
  it reads the builder-local `m_frame.sources` snapshot + `m_projectDecoderMethod`,
  refreshed on `ProjectModel::sourceChanged`.
- **apiCall from pipeline engines** (parser/transform scripts): dispatches to the GUI
  via BlockingQueued with the parked-flag bracket
  (`PipelineHost::setPipelineParkedOnGui`); while parked, GUI-side marshals run inline
  against the quiescent pipeline — exactly the old single-thread mid-frame semantics.
- **Table store**: single-writer on the pipeline thread. Every external access path
  (Lua closures, TableApiBridge JS methods, API datatables verbs, the 1 Hz session
  snapshot) marshals to the builder thread; GUI-side script engines pay the hop only
  at command/paint rate.
- **Headless/benchmark bootstraps never move the builder** (they skip
  `setupCrossModuleConnections`), so the spec-0044 verifier and `--benchmark-hotpath`
  stay single-threaded by construction and every marshal hits its same-thread fast path.
- **The in-app benchmark dialog borrows the same property in a live session**: it drives
  the pipeline with plain synchronous calls from the GUI thread, so
  `BenchmarkRunner::beginSession()` takes FrameBuilder/FrameParser onto the GUI thread and
  `endSession()` hands them back. Run is blocked while a device is connected or a recording is
  open (any other producer both pollutes the rows and, for a live device, reaches a builder that
  no longer lives on the pipeline thread). The session is `ephemeralSession` +
  `setSettingsPersistent(false)` throughout and `abortSession()` runs on `aboutToQuit`, so a quit
  mid-run persists none of the synthetic project, forced-on exporters, or 10 s plot range.

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
| `PipelineHost` | GUI-affine module owning the processing `QThread`, the dashboard SPSC ring and the atomic mode/paused/connected/dashboardAccepting mirrors. Joined FIRST in `ModuleManager::stopFrameConsumerWorkers()` (bounded 5 s wait, warn-and-abandon on a hung Fast-mode script, R21) with FrameBuilder/FrameParser `prepareShutdown()` queued ahead of quit() so script engines die on their own thread. |
| `FrameReader` | **Pipeline thread** (configured on GUI before `moveToThread`, then registered via `PipelineHost::registerFrameReader`). Recreate via `ConnectionManager::resetFrameReader()` / `DeviceManager::reconfigure()` — the old reader's deleteLater executes on the pipeline loop. **Never add mutexes.** Single-delimiter uses KMP; multi uses `CircularBuffer::findFirstOfPatterns()` (single-pass, stack array ≤8). Preserves driver timing via `PendingChunk` spans. |
| `CircularBuffer` | **SPSC only.** Producer/consumer are both the pipeline thread (processData appends and scans). Never MPMC. |
| `FrameBuilder` / `FrameParser` | **Pipeline thread** after the composition root's final `relocateProcessingObjects()` step. All external mutators self-marshal; the parse path is plain same-thread calls. Ownership moves only through `PipelineHost::moveProcessingObjectsTo()`, which releases every script engine on the outgoing thread and rebuilds them via `readCode()` on the new one — a `lua_State` / `QJSEngine` used off its creating thread corrupts the QV4 heap (crashed the in-app benchmark's JS phase, 2026-08-12). |
| `Dashboard` | **GUI thread only.** Ingests pooled `TimestampedFramePtr`s from the PipelineHost ring on the display tick (`onDisplayTick`). |
| Export workers | Lock-free enqueue from the pipeline thread (single producer); batch on worker thread. Consume a detached `make_shared` copy of the frame (NOT the pooled slot), so a slow worker's backlog can't pin the pool. |

**In-pipeline signal hops must be `Qt::DirectConnection`.** A queued connection between two
pipeline-thread objects costs a `QMetaCallEvent` alloc + event-queue insertion per emit; at
10+ kHz that fills FrameReader's 65536-slot queue faster than the consumer drains and
trips `Frame queue full — frame dropped`. GUI↔pipeline traffic is chunk/command/tick rate,
never per-frame signals. Known frame-path sites:

- `FrameReader::readyRead → PipelineHost::routeFrames` (explicit Direct, same thread)
- `routeFrames → FrameBuilder::hotpathRx*` (plain call)
- `FrameBuilder → PipelineHost dashboard ring` (SPSC enqueue, no signal)
- `DeviceManager::rawDataReceived → ConnectionManager::onRawDataReceived` (Direct, GUI raw path)

## Cached Hotpath Flags

The hotpath reads **cached** flags, never live getters: `m_operationMode`, `m_playerOpen`,
`m_anyAsyncSink`, `m_captureLatestFrame`, `m_changeDriven`, and Dashboard
`m_streamAvailable`. A new input to any of them must wire its change signal to the matching
cache refresh (`updateStreamAvailable` / `refreshAnyAsyncSink` / the player lambdas) or
frames/exports silently stop. **Two-thread refresh rule (spec 0051 M3):** FrameBuilder's
refresh slots are pipeline-affine, so their connections auto-queue from GUI emitters —
that is correct and cannot be "fixed" back to Direct: refreshes are FIFO in the pipeline's
event queue, so they can lag in-flight frames by queued hops but can never be torn or
lost. Dashboard-side (`m_streamAvailable`) stays GUI-affine with Direct wiring, and it
pushes the `PipelineHost::setDashboardAccepting` atomic mirror the publish gate reads.
PipelineHost's own mode/paused/connected mirrors are written Direct on the GUI thread at
transition rate. `streamAvailable()` also
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

## Parse-Load Governor (spec 0051)

`FrameBuilder` owns one `DataModel::ParseBudget` (header-only, `ParseBudget.h`): parse time is
charged **per source** into EWMA duty estimates (leaky integrator, tau 250 ms) plus a shared
total. While total duty stays under 90% of one core nobody is thinned; past it, only sources
above their fair share (`0.90 / active sources`) are decimated — every Nth frame processed, N
proportional to the overrun, recovering continuously as the EWMA decays. There is no fixed
window, no global latch, and no message box: a light source can never be starved by a heavy
one. Diagnostics follow spec 0033 — `parseLoadSnapshot()` / `parseBudgetThinning()` are pulled
by the 1 Hz `link.statistics` checker (`Misc/Problems/LinkCheckers.cpp`, banded text) and the
Dashboard taskbar badge poll; nothing on the frame path signals, allocates, or locks.
`setParseBudgetEnabled(false)` (benchmark) bypasses both the skip gate and the accounting.

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
`FrameBuilder` parse-budget guard (the spec-0051 fair-share governor, which a 100%-duty benchmark
would engage) via `setParseBudgetEnabled(false)` and run **no** exporters or dashboard,
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
