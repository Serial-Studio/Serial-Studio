---
spec: 0055-unified-block-lane
phase: plan
status: approved     # draft -> approved (gate before /ss-tasks)
updated: 2026-08-16
---

# Plan 0055 — Unified Block Publication Lane

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Read the relevant `doc/claude/` sub-docs and the *actual code*
> before writing this — a plan grounded in a stale mental model is worse than no plan.
> Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

Introduce one column-major payload, `DataModel::DataBlock`, carrying N samples of M datasets
plus a timebase that is either a uniform grid (`dt != 0`) or explicit per-sample offsets
(`dt == 0`), and make it the only thing any consumer ingests. The parse pipeline is left
completely alone: `FrameReader`, `trySpanLane`, `applyDatasetValuesSpans` and the frame slot
pool keep producing a `Frame` per parsed frame exactly as they do today — that `Frame` simply
stops being *published* and becomes the staging buffer for a per-source block accumulator that
flushes on the display tick or a sample cap, whichever comes first (D1). Structure travels
separately as a `StructureSnapshot` published only when the pool generation bumps, which is the
signal `Dashboard` already keys its reconfigure on, so per-frame `compare_frames()` disappears
rather than moving. On the dense side, `StreamProcessor` stops emitting its two near-duplicate
payloads (`StreamDisplayUpdate` and `StreamBlockItem` carry the same samples twice today) and
emits one `DataBlock` instead. The shape is chosen over a row-major envelope of frames because
a 48 kHz source cannot afford one `Frame` per sample, and over a full replacement of `Frame`
because keeping the parse product intact puts the 256 kHz gate out of scope.

## Affected subsystems & files

| File | Change |
|------|--------|
| `app/src/DataModel/DataBlock.h` | **New.** `BlockColumn`, `DataBlock`, `DataBlockPtr`, `StructureSnapshot(Ptr)`, the pooled-slot policy reuse, and the `appendRow` / `sampleTimeNs` inline helpers. |
| `app/src/DataModel/Frame.h` | No struct changes. Add `Keys::` entries only if the session/mirror payloads need new project keys (expected: none). |
| `app/src/DataModel/FrameBuilder.h/.cpp` | Replace `hotpathTxFrame` / `publishReplayFrame` with block staging + flush. New: `stageParsedFrame`, `flushOpenBlocks`, `publishBlock`, `publishStructureSnapshot`, block pool (`claimBlockSlot`, mirroring `claimPoolSlot`). `replayChannels` / `replayChannelSpans` / `replayChannelsTyped` collapse into `replayBlock`. `ingestStreamValues` keeps its table-store role unchanged. |
| `app/src/DataModel/FramePoolPolicy.h` | Reused as-is for the block pool budget; no change expected. |
| `app/src/IO/PipelineHost.h/.cpp` | Dashboard ring retyped to `DataBlockPtr`; add the `m_flushDue` atomic written by the GUI display tick and the queued `flushOpenBlocks()` nudge. `publishFrameToDashboard` → `publishBlockToDashboard`. |
| `app/src/IO/StreamWorker.h/.cpp` | `StreamDisplayUpdate` + `StreamBlockItem` collapse into `DataBlock`. `blockReady` carries `DataBlockPtr`; the display ring retypes. `StreamConfig`/`StreamChannelConfig` unchanged. |
| `app/src/IO/ConnectionManager.h/.cpp` | `wireStreamWorkerSinks` fans `blockReady` to the single per-consumer `ingestBlock`. `refreshStreamExportFlags` unchanged in shape. `startReplayStreamSource` / `ReplayStreamSource` removed (players emit blocks directly). |
| `app/src/IO/ReplayStreamSource.h` | **Delete.** Superseded by `FrameBuilder::replayBlock`. |
| `app/src/UI/Dashboard.h/.cpp` | `hotpathRxFrame` + `applyStreamUpdate` + `applyStreamChannel` collapse into `applyBlock` / `applyBlockColumn`. `drainDashboardRing` and `drainStreamWorkers` merge. Reconfigure moves onto `StructureSnapshot`; `compare_frames()` per frame is deleted. `m_lastFrame` stays as the GUI value mirror. |
| `app/src/CSV/Export.h/.cpp` | One worker. `StreamExportWorker` / `StreamExport` deleted. New sparse single-file row writer with the bounded reorder buffer (D3). `writeSnapshotRow` kept, fed from block latest values (D4). |
| `app/src/MDF4/Export.h/.cpp` | One worker. `StreamExportWorker` / `StreamExport` deleted; a stream source becomes one more `IChannelGroup` in the same `.mf4` via the existing `buildChannelGroups` path. |
| `app/src/Sessions/Export.h/.cpp` | Frame queue + stream queue collapse to one block queue (raw-bytes and table-snapshot queues unchanged). `writeFrameReadings` + `writeStreamBlocks` → `writeBlocks`; `hashReadingRow`/`hashStreamBlock` → `hashBlockRow` (legacy functions kept for verifying legacy archives). |
| `app/src/Sessions/BlockReader.h` | **New.** Shared decode helper materialising `ReadingRow`s from `blocks`, the shared SELECT column list, and the per-session storage probe. Added at implementation time: six readers query `readings` directly and a SQL view cannot expand a float64 blob into rows. |
| `app/src/CSV/SparseRowMerger.h` | **New.** The sparse k-way row merge, extracted so it is unit-testable without the file/workspace/session machinery. |
| `app/src/Sessions/DatabaseWorker.cpp`, `app/src/Sessions/ReportData.cpp` | Legacy-read branches for the delete cascade, CSV export streaming, session stats and every report aggregate. Added at implementation time. |
| `app/src/Sessions/StreamBlockCodec.h` | Extended: text and explicit-times blob codecs beside the existing float64 sample codec. Encode/decode stay co-located. |
| `app/src/Sessions/DatabaseManager.h/.cpp` | `user_version` 3 + additive migration; `kCaptureFormatVersion` bump; legacy-read branch. |
| `app/src/Sessions/Player.h/.cpp` | Emits `DataBlock`s; `startReplayStreamSource` removed. `replaySeekSeries` unchanged. |
| `app/src/Sessions/PlayerLoaderWorker.h/.cpp` | Index/fetch against `blocks` for v3, legacy tables for v1/v2. |
| `app/src/Sessions/Verifier.cpp`, `VerifierRegression.cpp` | Re-record + diff over blocks; spec-0047 provenance key derives from block sample time. Legacy archives keep the legacy path. |
| `app/src/CSV/Player.cpp`, `app/src/MDF4/Player.cpp` | `replayChannelSpans` / `replayChannelsTyped` call sites become `replayBlock`. |
| `app/src/API/Server.h/.cpp` | `hotpathTxFrame` + `ingestStreamBlock` collapse into `ingestBlock`; `writeStreamBlock` becomes the single frame/stream wire writer. |
| `app/src/API/GRPC/GRPCServer.h/.cpp` | `hotpathTxFrame` → `ingestBlock`. **Closes the gRPC blind spot.** |
| `app/src/MQTT/Publisher.h/.cpp` | `hotpathTxFrame` → `ingestBlock`. **Closes the MQTT blind spot.** |
| `app/src/API/Mirror/MirrorPublisher.cpp` | Reads the block stream instead of `Dashboard::rawFrame()`; wire carries columns + timebase. |
| `app/src/API/Mirror/MirrorProtocol.h` | `kWireVersion` 1 → 2. |
| `app/src/API/Mirror/MirrorSession.cpp` | Viewer reconstructs blocks, not frames, and feeds `Dashboard::applyBlock`. |
| `app/src/UI/Widgets/AudioExport.h/.cpp` | `ingestStreamBlock` → `ingestBlock`. |
| `app/src/Benchmark/HotpathBenchmark.cpp` | Publish-tail counters follow the rename; add a block-flush readout beside `datasets+publish`. |
| `app/src/Misc/ModuleManager.cpp` | `stopFrameConsumerWorkers()` drops the two `streamSink().stopWorker()` calls with the sinks they stopped. Added at implementation time. |
| `scripts/code-verify.py` | `_HOTPATH_ASSERT_ALLOWED` gains the spec-0055 per-sample TUs (`DataBlock.h`, `PipelineHost.h/.cpp`, `StreamWorker.h/.cpp`). Added at implementation time with maintainer approval — growing that list is a review decision by the lint's own comment. |
| `app/tests/` | New `tst_data_block`, `tst_csv_sparse_writer`, `tst_sink_flags`, `tst_sessions_legacy_archive`; extend `tst_stream_block_codec` and `tst_stream_worker`. |
| `tests/integration/` | New MQTT / gRPC / mirror stream-visibility cases (AC2). |
| `doc/claude/architecture/{dataflow,io,export,mirror}.md`, `CLAUDE.md` | Doctrine updates once implemented. |

## Architecture & data flow

```
Driver (driver thread)
  │ dataReceived(CapturedDataPtr)                        queued, chunk rate  [UNCHANGED]
  ▼
FrameReader::processData        (pipeline thread)                           [UNCHANGED]
  │ delimiter scan, pooled CapturedData slots, lock-free queue
  ▼
PipelineHost::routeFrames       (pipeline thread, Direct)                   [UNCHANGED]
  ▼
FrameBuilder parse              (pipeline thread)                           [UNCHANGED]
  │ trySpanLane / applyDatasetValuesSpans write into the frame slot
  ▼
FrameBuilder::stageParsedFrame  (pipeline thread)                           [NEW]
  │ append one row into the per-source open DataBlock (pooled slot, pre-sized
  │ column vectors -- plain stores, no allocation)
  │ flush when samples == kBlockSampleCap, or when PipelineHost::flushDue is set,
  │ or when the pool generation changed (a block never straddles a layout change)
  ▼
FrameBuilder::publishBlock      (pipeline thread)
  ├─ PipelineHost block ring (SPSC) ──> Dashboard::onDisplayTick drain
  └─ if m_anyAsyncSink: ONE DataBlockPtr handed to every sink -- NO detached deep copy
       CSV / MDF4 / Sessions / API / gRPC / MQTT / Mirror

Dense source (audio, or streamLane=on)
  │ SampleBlock (float32 interleaved)                                       [UNCHANGED]
  ▼
StreamProcessor  (per-source worker thread)                                 [UNCHANGED work]
  │ channel extract, transform_block/transform, FFT ring, latest values
  ▼
one DataBlock  (was: StreamDisplayUpdate + StreamBlockItem, same samples twice)
  ├─ display ring ──> Dashboard drain
  ├─ queued to GUI ──> the same seven sinks (GUI stays the single SPSC producer)
  └─ latestValuesReady ──> FrameBuilder::ingestStreamValues                 [UNCHANGED]

Structure
  FrameBuilder::publishStructureSnapshot(StructureSnapshotPtr)  on generation bump only
  ──> same ring, ahead of the blocks that carry that generation
```

**Why the deep copy dies.** Today `hotpathTxFrame` builds
`make_shared<TimestampedFrame>(frame->data, ...)` per frame whenever any async sink is live —
a full `Frame` copy including every `Dataset`'s six `QString`s and its `alarmBands` /
`fftMarkers` vectors. A `DataBlock` carries values only, is pooled, and is shared by
`shared_ptr` with no copy at all. Sinks cannot pin the pool because a block is released as
soon as each worker's `m_writeBuffer.clear()` runs (the existing `FrameConsumerWorker`
contract, unchanged), and the pool budget bounds materialised slots exactly as
`kFramePoolBudgetBytes` does today.

**Flush trigger, without a timer on the pipeline thread.** `PipelineHost` gains
`alignas(64) std::atomic<bool> m_flushDue`, written `true` by `Dashboard::onDisplayTick` on
the GUI thread (transition rate, same discipline as the existing mode/paused/connected
mirrors) and cleared by the pipeline inside `flushOpenBlocks()`. A producing source notices
it on its next staged row — zero added cost, one relaxed load per frame. A source that has
gone quiet would otherwise never flush its partial block, so the display tick also posts one
queued `flushOpenBlocks()` per tick (one queued call per tick, never per frame).

**Structure/value split at the Dashboard.** `Dashboard::hotpathRxFrame` currently revalidates
with `compare_frames()` unless the cached per-source `structureGeneration` matches. After the
change, structure arrives only as a `StructureSnapshot`, so reconfigure runs exactly once per
layout change and the per-frame comparison is deleted outright. A block whose generation does
not match the last snapshot is dropped and counted — the same rule that keeps a stale slot
from painting today. `m_lastFrame` and `m_datasetReferences` stay: `applyBlockColumn` writes
the last sample of each column into every dataset copy, which is what `dashboard.getData`, the
painter widgets and the mirror publisher read.

## Hotpath & threading impact

- **Touches the hotpath? — Yes, the publish tail only.** `FrameReader`, `CircularBuffer`,
  `trySpanLane`, `parseUtf8Spans`, `applyDatasetValuesSpans` and the frame slot pool are
  **not** modified; the parse product is unchanged and still lands in a pooled `Frame`. What
  changes is what happens after `applyDatasetValues*` returns. Rules preserved:
  - *No allocation:* block columns are pre-sized on the pooled slot; `appendRow` does
    `values[n] = dataset.numericValue` and, for text columns, `assign_string_in_place` into
    the pre-existing `QString` slot (never share-assign — `common-mistakes.md` row 21).
    Steady state allocates nothing.
  - *No `Frame` copy:* the per-frame `make_shared<TimestampedFrame>` fan-out copy is removed,
    not relocated. This is expected to be a throughput **gain** on the exporter tier.
  - *SPSC:* the PipelineHost ring keeps exactly one producer (the pipeline thread) and one
    consumer (GUI). Stream sinks keep their fan-in through the GUI-affine `ingestBlock` so
    each consumer queue still has one producer.
  - *DirectConnection:* no new in-pipeline signal hop. Staging and flushing are plain calls
    on the pipeline thread; the only new cross-thread traffic is one atomic store and one
    queued call per display tick.
  - *`structureGeneration`:* every published block stamps the generation it was staged
    under, and a mid-block generation change forces a flush first — a block can never carry
    two layouts (`common-mistakes.md` row 23, generalised).
  - *Assertions:* per-sample append uses `SS_ASSERT_HOTPATH`; block-boundary work uses
    `SS_ASSERT`.
  - *Benchmark plan:* record all nine gated tiers plus the `lua+exporters` and
    `lua+dashboard` readouts on the pre-change build, then again after, on the same machine.
    The parser tiers must be flat within noise (nothing on that path moved); the exporter
    readout is expected to improve. Use the run to pick `kBlockSampleCap` by sweeping it.
- **New cross-thread signal/slot?** — One: `StreamProcessor::blockReady(DataBlockPtr)` is
  retyped, not added, and stays queued to the GUI (block rate). `flushOpenBlocks()` is a new
  queued GUI→pipeline call at display-tick rate — fire-and-forget, never blocking, so it
  cannot deadlock against a script's `apiCall`.
- **New input to a cached hotpath flag?** — No new flag. `m_anyAsyncSink` gains two members
  (MQTT and gRPC already participate; the mirror publisher becomes a sink and must be OR'd in
  and have its subscribe/unsubscribe wired to `refreshAnyAsyncSink`). Missing that wiring is
  exactly the `refreshStreamExportFlags` bug recorded in `export.md` — a recording that looks
  valid and contains nothing — so it gets its own task and its own test.
- **Timestamp ownership** — unchanged. The driver stamps `CapturedData::timestamp`;
  `FrameReader` spaces intra-chunk frames by `frameStep`; a staged row records that stamp as
  its offset from the block's `t0`. Nothing re-stamps. `monotonicFrameNs` stays the same-ns
  collision safety net on the frame-derived lane only and is never applied to a uniform-grid
  block, matching today's stream rule.

## Data model & persistence

**`DataBlock`** (new, `app/src/DataModel/DataBlock.h`):

- `sourceId`, `blockNumber`, `structureGeneration`
- `t0` (steady_clock), `dt` (ns; `0` means irregular), `times` (ns offsets from `t0`, empty
  when `dt != 0`), `samples`
- `columns`: per dataset `uniqueId`, `values` (`std::vector<double>`), `text`
  (`std::vector<QString>`, empty unless the dataset is non-numeric), `hasText`

Values are `double` throughout — the dense lane's float32 is the driver's own precision and is
widened once at the `StreamProcessor` boundary exactly as it is today, so no consumer loses
precision (spec constraint "precision only increases"). Per D2 a dense source never populates
`text`, so a numeric block is allocation-identical to today's `StreamBlockItem`.

**Sessions schema v3.** `readings` and `stream_blocks` unify into `blocks`:
`block_id INTEGER PRIMARY KEY AUTOINCREMENT`, `session_id`, `unique_id`, `t0_ns`, `dt_ns`,
`sample_count`, `values BLOB` (little-endian float64, existing codec), `texts BLOB` (NULL for
numeric), `times BLOB` (NULL when `dt_ns != 0`). A frame-lane block of one sample is a
one-element blob — no special case. Covering indexes on `(session_id, unique_id, t0_ns)` and
`(session_id, t0_ns)`; plain `INSERT`, never `INSERT OR IGNORE`; ties broken by `block_id`.
`PRAGMA user_version = 3`, additive migration, and the v1/v2 readers stay in place read-only
(R8). The app never writes the legacy layout.

**Reproducibility (spec 0044).** `hashReadingRow` / `hashStreamBlock` are superseded by one
`hashBlockRow` over the canonical block byte layout; both legacy functions are retained
verbatim so a legacy archive re-hashes identically. `DatabaseManager::kCaptureFormatVersion`
bumps. **Regression (spec 0047).** The provenance key stays `(chunk, rank)` with the same
arithmetic; it now derives from a block's per-sample time rather than a reading's
`timestamp_ns`. Verify mode still never injects.

**Mirror wire.** `Mirror::kWireVersion` 1 → 2; the payload gains the column/timebase shape. A
version mismatch is already refused with a message on both ends (`MirrorClient.cpp:403,548`);
that path is exercised as AC11.

**Project JSON.** Unchanged. No new `Keys::` entries expected; `streamLane` keeps its current
meaning and D2 does not change which sources take the dense lane.

## API / SDK surface

- `API::Server`: `ingestBlock` replaces the frame and stream ingestion pair. The existing
  `stream.subscribe` verb and its `writeStreamBlock` wire writer become the single
  frame-and-stream delivery path, so an existing stream subscriber keeps working and a
  frame-only subscriber keeps receiving exactly today's per-frame payload (D5).
- `API::GRPC::GRPCServer` and `MQTT::Publisher` gain `ingestBlock`; both stay behind their
  existing `ENABLE_GRPC` / `BUILD_COMMERCIAL` guards. Neither gains a new verb — they simply
  start receiving dense-source data.
- No new handler is registered in `CommandHandler::initializeHandlers()`; no `EnumLabels`
  slug changes; no generated-SDK regeneration expected (no dataset property is added, so the
  spec-0036/0037 generators are untouched).

## QML / UI

No new components. `SetupPanes` and the Export preferences tab are unchanged: the CSV
interval-snapshot control keeps its meaning and its disabled-by-default value (D4). The only
user-visible UI difference is that a recording session now yields one file per format instead
of one plus one per stream source, which needs no control.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Block layout | Row-major envelope (`vector<TimestampedFramePtr>`) / column-major (per-dataset arrays) | **Column-major.** The envelope is the naive reading of "publish a list of frames" but forces one `Frame` per sample for dense sources — already rejected for the session player (`export.md`: `replayChannelsTyped` "would mean one synthesized frame per sample"). Column-major is also what both existing dense payloads already are. |
| Scope of the parse path | Replace `Frame` end-to-end / keep `Frame` as the parse product and change only the publish tail | **Keep `Frame`.** The span lane already treats `m_frame` as a structure-only template writing into pool slots, so the slot becomes a staging buffer with no parse code touched — which takes the 256 kHz gate out of scope and contains all risk to the tail, where the win is. |
| Structure delivery | Ride along in every block / separate snapshot on generation bump | **Separate snapshot.** Structure changes at project-edit rate; carrying it per block would re-create the deep copy the change exists to remove, and it lets the per-frame `compare_frames()` be deleted rather than moved. |
| Timebase representation | Always explicit per-sample times / always uniform grid / either, discriminated by `dt` | **Either, `dt == 0` means irregular.** Parsed frames genuinely have irregular times and dense sources genuinely have a grid; forcing one costs either an N-element array per dense block or a lie about parsed timing. |
| Block flush trigger | Fixed count / display tick / tick-or-cap / adaptive | **Tick-or-cap (D1, maintainer-decided).** Bounds latency for a slow source and memory for a fast one. Cap value is a benchmark sweep, not a guess. |
| Dense sources and strings | Numeric-only / strings via transforms / fully general | **Numeric-only (D2, maintainer-decided).** Keeps string storage off the per-sample worker path; the contract still carries text for parsed sources, so widening later needs no wire change. |
| CSV multi-rate rows | Sparse / forward-fill / snapshot-only | **Sparse (maintainer-decided at spec time).** Lossless and invents nothing; the interval snapshot mode remains the dense alternative. |
| CSV row ordering | Bounded reorder window / arrival order / global sort at close | **Bounded reorder window (D3, maintainer-decided).** Downstream tools assume time-ordered CSV; a global sort at close costs a full rewrite and loses everything on a crash. |
| Sessions schema | Unify into `blocks` / widen `readings` and keep `stream_blocks` | **Unify.** One writer, one hash, one reader, one index strategy; keeping two tables preserves exactly the split this spec exists to remove. |
| `StreamWorker` | Delete / keep, emitting blocks | **Keep.** It is the dense lane's per-sample transform engine and its Safe/Fast-mode teardown discipline is hard-won (R21). Only its two output payloads merge. |
| Published wire shape | Block shape everywhere / frame shape by default, block on opt-in / block + compat shim | **Frame shape by default (D5).** The spec's one-path goal is about ingestion; collapsing the wire too would break every API, gRPC and SDK client for no internal benefit. |
| Block cap scope | One global cap / cap 1 on the frame lane / per-source rate-derived | **One global cap (D6).** One knob, swept in the benchmark; the frame-lane-only alternative forfeits the queue-op savings and the rate-derived one is a drifting heuristic. |
| Reorder window sizing | Fixed / one display tick / adaptive bounded | **Fixed (D7).** Deterministic to test; a tick is too short when a worker stalls, and an adaptive size is not deterministically assertable. |
| `ReplayStreamSource` | Keep / delete | **Delete.** It exists solely so a session player could reach `StreamWorker`; with players emitting blocks directly it has no remaining caller, and the plain-BlockingQueued replay exception (borrowed span/text pointers) also disappears because block values are owned. |

## Risks & mitigations

- **Throughput regression on the parser tiers.** Mitigated structurally (no parse code
  changes) and verified by a before/after nine-tier `--benchmark-hotpath` run on the same
  machine. If any parser tier moves outside noise, the staging append is the only suspect and
  the cap sweep is the first knob.
- **A missed `refreshAnyAsyncSink` input silently records nothing.** This exact bug shipped
  once for stream export (`export.md`: "recording produces a valid-looking file containing
  nothing"). The mirror publisher becoming a sink is a new input to that flag. Mitigation: a
  dedicated task, and a test that starts each sink alone and asserts a non-empty artifact.
- **Latency for slow sources.** A 1 Hz source now waits up to one display tick before its
  value reaches an export or MQTT. Bounded by D1 and measured; if a control-loop consumer
  proves sensitive, the cap for single-sample blocks can be 1 (immediate flush) with no
  design change.
- **Legacy session archives.** R8 is the highest-value regression surface. Mitigation: a
  checked-in legacy archive fixture in `ctest`, asserting open + replay values + the same
  0044 verdict; the legacy hash functions and readers are kept verbatim rather than
  re-derived.
- **API client compatibility.** Closed by D5: the default wire shape does not change, so no
  existing API, gRPC or SDK client is affected. The residual risk is that the frame-shaped
  serializer must now be driven from a block rather than a `Frame`; a single-sample block must
  serialize byte-identically to today's output, which the existing `tests/integration` API
  assertions already pin.
- **Mirror wire break.** Version bump plus the existing mismatch refusal; fixtures under
  `tests/fixtures/mirror/` regenerate (`mirror.md` rule).
- **`QMap::operator[]` on per-source maps** (`common-mistakes.md` row 14) — the new
  per-source open-block map must use `find()`, not `operator[]`.
- **Scope creep.** The file list above is the lane. Anything outside it gets named in chat
  before it is touched.

## Test & verification plan

- **Unit — C++ `ctest` (maintainer builds, I may run against an existing build dir):**
  - `tst_data_block` — append/flush semantics, both timebases, generation-forced flush, pool
    reuse allocates nothing after warm-up (AC3).
  - `tst_csv_sparse_writer` — sparse row emission, reorder window ordering, one row per
    distinct instant, no forward fill (AC5); snapshot mode still dense (AC6).
  - `tst_stream_block_codec` — extended for text and explicit-times blobs; encode/decode
    round-trip bit-exact (AC3).
  - New legacy-archive fixture case — open, replay, and 0044 verdict against a
    pre-change-recorded `.db` (AC7).
  - `tst_stream_worker` — updated for the single-payload emission.
- **Integration — `pytest` (maintainer runs the app with the API server on):**
  - `tests/integration/test_stream_visibility.py` (new) — audio-only project: MQTT broker
    receives messages, gRPC client receives values, mirror viewer's sample count over a fixed
    window matches the sample rate (AC2). Each must fail on the pre-change build.
  - Existing suite must pass unchanged (AC12).
  - Mirror version-mismatch refusal (AC11).
- **Hotpath — `--benchmark-hotpath` (maintainer runs):** all nine gated tiers plus the
  `lua+exporters`, `lua+dashboard`, `lua+dashboard(off)` readouts, before and after, same
  machine (AC9). Also the vehicle for the `kBlockSampleCap` sweep.
- **Maintainer observation:** record a 48 kHz + low-rate project; confirm exactly one `.csv`
  and one `.mf4` (AC4, AC5), open the `.mf4` in a third-party reader and confirm two channel
  groups on independent time bases (AC4); replay the session and compare the trace and the
  scrub/settle behaviour against the live capture (AC8); synthetic overload drops whole
  blocks and increments the counter (AC10).
- **Static:** `python scripts/code-verify.py --check <files>` on every touched file;
  `qt-cpp-review` before handoff; `python scripts/sanitize-commit.py` before commit.

## Resolved decisions (plan phase)

Settled with the maintainer at plan review. These bind implementation the same way D1-D4 in
the spec do.

- **D5 — The wire keeps the frame shape by default.** API and gRPC clients continue to receive
  today's per-frame payload unchanged; the block-shaped payload is delivered only to clients
  that opt in, which the existing `stream.subscribe` verb already is. No API version bump, no
  appended proto fields, no SDK regeneration, no client breakage. `API::Server` already carries
  both serializers (the frame JSON path and `writeStreamBlock`), so this is status quo on the
  wire while the internal ingestion path unifies — the spec's "one ingestion path per consumer"
  goal is about ingestion, not about collapsing published wire formats.
- **D6 — Per-lane block sample cap, chosen by benchmark sweep.** *(Amended at implementation
  time; the original decision was one global cap.)* The two lanes carry different payloads, so
  one cap cannot serve both: a frame-lane column must carry the parsed display string for
  **every** dataset, numeric ones included, because `updateDashboardData` writes
  `dataset.value` into DataGrid / BarPanel / the API-serialised `m_lastFrame`, and re-rendering
  that string from the double changes what the user sees (`"22.40"` becomes `"22.4"`). A dense
  column is numeric-only (D2). So `kFrameBlockSampleCap` (starting at 64) bounds the
  string-carrying frame lane and `kStreamBlockSampleCap` (starting at 4096) bounds the numeric
  dense lane; both are swept in `--benchmark-hotpath`. Rejected: one global cap with
  re-rendered numeric strings (a visible DataGrid precision change), and publishing the
  consumer's string-target set back to the pipeline so the producer could carry text
  selectively (correct and leaner, but adds a GUI->pipeline channel this spec does not have --
  revisit if block memory ever binds).
- **D8 — The pipeline thread is the single producer for every sink, both lanes.** *(Added at
  implementation time.)* Collapsing each format's two consumers into one would have left that
  consumer's SPSC queue with two producers -- the pipeline thread for frame-lane blocks and the
  GUI thread for stream-lane blocks -- which is precisely why two sinks existed. So a stream
  worker no longer fans out to sinks or carries its own display ring: it emits `blockReady`
  queued to `FrameBuilder` (pipeline-affine), and `publishBlock` drives the dashboard ring and
  the sinks for both lanes. Costs one queued hop per block (block rate, ~100/s for a 48 kHz
  source); buys a genuinely single ingestion path, the single-producer invariant intact, and the
  deletion of `StreamWorker`'s display ring, `dequeueDisplayBlock`, `Dashboard::drainStreamWorkers`
  and `drainStreamWorker`. Rejected: per-sink MPSC queues (drops the lock-free SPSC contract) and
  keeping a second queue per sink (preserves the split this spec exists to remove).

- **D7 — One fixed CSV reorder window, chosen by benchmark sweep.** Start at 250 ms; residue
  flushes at session close. Predictable memory (~384 KB at 48 kHz stereo), predictable lag, and
  deterministic to assert in `tst_csv_sparse_writer`. Rejected: a one-tick window (would emit
  out-of-order rows when a worker stalls) and an adaptive window (runtime-variable size is not
  deterministically testable).
