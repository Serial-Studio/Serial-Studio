# Threading & Hotpath — Non-Negotiable

The rules most likely to cause silent breakage. Full detail (data flow, threading table,
cached flags, benchmark mechanics) in
[doc/claude/architecture/dataflow.md](architecture/dataflow.md); invoke the
`ss-hotpath` skill before editing these paths (nothing loads it for you) and it re-states them,
including the spec-0055 block caps, the time-ring/plot-clock rules, and the kernel macros.

- **The frame pipeline runs on `IO::PipelineHost`'s processing thread (spec 0051 M3), not the
  GUI thread.** FrameReaders, `FrameParser` and `FrameBuilder` all live there; the GUI drains
  finished pooled frames from an SPSC ring in `Dashboard::onDisplayTick`. Public
  FrameBuilder/FrameParser mutators SELF-MARSHAL — GUI→pipeline waits go through
  `IO::PipelineHost::runOnObjectThread` (event-loop-backed), pipeline→GUI reads through
  `runOnGuiThreadBlocking`. Never add a plain `BlockingQueuedConnection` from the GUI into the
  pipeline.
- **`runOnObjectThread` is for command-rate waits, never for a GUI-thread script API.** The wait
  spins a nested `QEventLoop` that fires the display tick and re-enters the calling script.
  `DataModel::readTableView` / `writeTableStore` (`DataTable.h`) hold the ONE routing rule, and
  both the JS bridge and the Lua closures go through them. **No `lua_*` call may run inside a
  routed lambda.** Any new GUI-callable script API follows the same rule:
  [doc/claude/architecture/scripting.md](architecture/scripting.md) "Data Tables".
- **`FrameReader` and `CircularBuffer` are pipeline-thread / SPSC. Never add mutexes.** Recreate
  via `resetFrameReader()` / `reconfigure()`. Enforced: code-verify:spsc-lock.
- **In-pipeline signal hops must be `Qt::DirectConnection`.** Queued between two
  pipeline-thread objects fills the 65536-slot queue at 10+ kHz and drops frames. GUI↔pipeline
  traffic is chunk/command/tick rate only — never a per-frame queued emission. Enforced:
  code-verify:pipeline-queued-hop.
- **No allocation on the publish path.** Blocks come from a pool (`use_count()==1` probe,
  aliasing shared_ptr) with columns sized once at bind — `DataModel::BlockStager` owns it on the
  frame lane, `StreamProcessor::claimBlockSlot()` on the stream lane. The one copy is
  `clone_block_trimmed`, made by `DataModel::BlockPublisher` for the async sinks — deliberate and
  gated on a sink being on.
- **One publication payload, one ingestion path (spec 0055).** Nothing publishes a
  `TimestampedFrame` any more: `FrameBuilder`'s `BlockStager` stages parsed rows into a pooled
  `DataModel::DataBlock` and flushes on the display tick or a sample cap (`kFrameBlockSampleCap`
  64, `kStreamBlockSampleCap` 4096); `BlockPublisher` fans the finished block out. Dense sources do per-sample work on their own
  `IO::StreamWorker` thread but emit `blockReady` **queued to the pipeline thread**, the SINGLE
  producer for every sink. Structure travels separately as a `StructureSnapshot`. Never add a
  rate cap or a per-view reduction; an overrun drops whole blocks and counts them.
- **The two republish lanes never share one "already republished" mark (spec 0064)**, or the
  dashboard stays live while every recording comes out empty. Enforced:
  compile:DataModel::RepublishLane, ctest:tst_republish_lanes. Detail: dataflow.md "The Two
  Republish Lanes".
- **The session boundary is the sink contract (spec 0075).** Recording sinks close on
  `FrameBuilder::sessionBoundary(connected, paused)` — pause included — and the builder flushes
  every open block BEFORE emitting, so the tail lands in the file that was open. The export-time
  tie-break is per source (`monotonicSourceNs`) and a uniform-grid block never takes it.
- **Time rings are sized from a rate, never a sample count, and a ring's clock never rewinds.**
  Since spec 0097 they start rate-true and grow per block on coverage (`EnvelopeRing::growFor`, the
  one named allocation on the ingest path, never inside a per-sample loop), keep every sample up
  to the per-curve ceiling or the 512 MB rail, and display storage is never pinned. Enforced:
  compile:DSP::RingCapacity, compile:UI::PlotClockState, ctest:tst_time_ring_sizing,
  ctest:tst_ring_exactness, code-verify:lock-memory-scope. Rules + the 2026-08 incidents:
  [dashboard.md](architecture/dashboard.md) "Time-Ring Sizing & the Plot Clocks".
- **Native + PlainText parses through the span fast lane** (`trySpanLane` → `parseUtf8Spans` →
  `applyDatasetValuesSpans`), zero steady-state allocation. Hotpath flags are `DataModel::Cached<T>`
  re-derived at 1 Hz (spec 0095); a new input wires its change signal to the cache refresh, a new
  cache is never a plain `bool`. Enforced: ctest:tst_cached_flags, anchor:cached-hotpath-flags.
  Read dataflow.md "Cached Hotpath Flags" before touching any.
- **Source owns time.** Stamp at the driver boundary; never re-stamp in export/report
  workers (use `monotonicFrameNs(...)` as the safety net only). Enforced: code-verify:--clock-census.
- **Driver opens are synchronous calls; most drivers dial asynchronously behind them.** An async
  dial's verdict has ONE owner, `HAL_Driver::openFinished(ok, reason)`, emitted exactly once per
  attempt; a driver that reports only success wedges the connect button. Doctrine:
  [doc/claude/architecture/io.md](architecture/io.md) "Opening a Link".
- **Diagnostics are pulled, never pushed (specs 0033/0035).** Pipeline counters are plain
  `quint64` increments polled on the 1 Hz tick, never a per-frame signal, allocation or lock; a
  recreated `FrameReader` zeroes them, so consumers work on deltas. Detail: dataflow.md
  "Diagnostic Counters" (the polled-PLC atomic deviation: io.md).
- **JS scripts**: always `JsScriptEngine::guardedCall()`, never `parseFunction.call()`.
  `setInterrupted(true)` only in `JsWatchdogThread.cpp`. Enforced: code-verify:js-raw-call,
  code-verify:js-interrupt-site.
- **256 kHz is a CI gate, not a slogan.** `--benchmark-hotpath` runs nine gates tiered off
  `--min-fps` (default 256000) as a hard gate in `ci.yml`; don't regress it. Enforced:
  anchor:hotpath-min-fps. Tier table and the ~70-80% `datasets+publish` share: `ss-hotpath` skill.
- **The message bus is not a hotpath primitive** (spec 0076): frames and blocks keep the pooled
  SPSC path. Enforced: code-verify:bus-on-hotpath. Detail: directory-map.md "Message bus".
- **Reuse the kernels; never inline intrinsics or invent a macro** — `core/Core/DSPSimd.h`
  (bit-exact per lane, runtime-dispatched) and `core/Core/HotpathOptimization.h`; never fast-math,
  no-unwind or GCC `optimize("...")`, and no TU is compiled wide. Rules:
  [doc/claude/architecture/kernels.md](architecture/kernels.md). Debt: no
  intrinsic-header lint yet.
