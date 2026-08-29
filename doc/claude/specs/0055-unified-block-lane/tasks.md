---
spec: 0055-unified-block-lane
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-08-16
---

# Tasks 0055 — Unified Block Publication Lane

> **Phase 3 of 4 — the ordered checklist.** Decompose [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable* — each one a coherent diff a reviewer
> could read in isolation. `/ss-implement` works this list top to bottom and keeps the status
> boxes current. Gate: do not start `/ss-implement` until a human marks this `approved`.

## Conventions

- One task = one focused, reviewable change. If a task touches >3 files or needs a paragraph
  to describe, split it.
- **Verify** is how *this* unit is confirmed before moving on — usually
  `python scripts/code-verify.py --check <files>`, plus a test or a read-back where one fits.
- **Deps** lists task IDs that must land first.
- Order so the tree compiles (conceptually) after each task where practical.

## Coherence note

T4-T6 are the core swap and only restore a *working* tree together: T5 deletes the frame
fan-out, so between T5 and T6 the dashboard is fed but the seven sinks receive nothing, and
each of T10-T26 restores exactly one of them. This is deliberate — a consumer at a time is
reviewable, a big-bang consumer sweep is not. Do not run `--benchmark-hotpath` for a verdict
before T26; the clean measurement points are T33 and T34.

## Tasks

### Stage A — the type and its pool

### T1 — `DataModel::DataBlock` type

- **Files:** `app/src/DataModel/DataBlock.h` (new)
- **Does:** Defines `BlockColumn` (uniqueId, `hasText`, `std::vector<double> values`,
  `std::vector<QString> text`), `DataBlock` (sourceId, blockNumber, structureGeneration, `t0`,
  `dt` where 0 means irregular, `times` ns offsets, `samples`, `columns`), `DataBlockPtr`,
  `StructureSnapshot`/`StructureSnapshotPtr`, and the inline `appendRow` / `sampleTimeNs`
  helpers. `appendRow` writes numerics as plain stores into pre-sized vectors and text via
  `assign_string_in_place` — **never share-assign, which re-links the buffer and brings back
  per-frame mallocs** (`common-mistakes.md` row 21). Per D2 nothing populates `text` from a
  dense source. Header only, no consumer yet.
- **Verify:** `python scripts/code-verify.py --check app/src/DataModel/DataBlock.h`
- **Deps:** none
- **Note:** `SS_ASSERT_HOTPATH` here tripped `hotpath-assert-scope`; with maintainer approval
  `_HOTPATH_ASSERT_ALLOWED` in `scripts/code-verify.py` gained `DataBlock.h`,
  `PipelineHost.h/.cpp` and `StreamWorker.h/.cpp` (the spec-0055 per-sample TUs). Recorded in
  the plan's affected-files table.
- [x] done

### T2 — `tst_data_block` unit suite

- **Files:** `app/tests/tst_data_block.cpp` (new), `app/tests/CMakeLists.txt`
- **Does:** Covers append/flush semantics, both timebases (uniform `dt != 0` and irregular
  `times[]`), generation-forced flush, text vs numeric columns, and a warm-up-then-steady-state
  case asserting no reallocation after the first block (AC3).
- **Verify:** `ctest -R tst_data_block` against an existing build dir
- **Deps:** T1
- **Note:** Writing this suite surfaced a T1 defect and corrected it in `DataBlock.h`:
  `assign_string_in_place` share-assigns whenever the destination is not already detached, which
  is exactly a freshly sized block column, so the block would have shared the producer's
  `dataset.value` buffer and made the pipeline's next `assign_utf8_in_place` detach and
  allocate. `write_block_sample` now uses a block-local `assign_string_owned`, and
  `textWriteKeepsTheSourceUniquelyOwned` is the regression guard.
- [x] done

### T3 — Block pool + structure snapshot in `FrameBuilder`

- **Files:** `app/src/DataModel/FrameBuilder.h`, `app/src/DataModel/FrameBuilder.cpp`
- **Does:** Adds the pooled block slots (`claimBlockSlot`, aliasing `shared_ptr` hand-out,
  `use_count()==1` free probe — mirroring `claimPoolSlot`) bounded by the existing
  `FramePoolPolicy` budget, plus `publishStructureSnapshot()` emitted on
  `invalidateFramePool()`'s generation bump only. Nothing publishes blocks yet.
- **Verify:** `python scripts/code-verify.py --check app/src/DataModel/FrameBuilder.h
  app/src/DataModel/FrameBuilder.cpp`; read back that the generation bump sites are exactly
  those `invalidateFramePool()` already covers.
- **Deps:** T1
- **Note:** Snapshots are published *lazily per source* (`m_structureDirty` +
  `m_publishedStructureGeneration`, checked at staging) rather than eagerly from
  `invalidateFramePool()`. Eager publication would need the ring T4 has not added yet, and more
  importantly a per-source lazy emit is what guarantees a snapshot can never be enqueued behind
  the blocks that carry its generation. `kBlockSampleCap` (D6) and `kBlockPoolSlots` land here
  with their starting values; T33 pins the swept value.
- [x] done

### Stage B — the pipeline tail (core swap)

### T4 — `PipelineHost` block ring and flush signalling

- **Files:** `app/src/IO/PipelineHost.h`, `app/src/IO/PipelineHost.cpp`
- **Does:** Retypes the dashboard SPSC ring to `DataBlockPtr` and renames
  `publishFrameToDashboard`/`dequeueDashboardFrame` accordingly. Adds
  `alignas(64) std::atomic<bool> m_flushDue`, written `true` on the GUI thread from the display
  tick and cleared by the pipeline. **The ring stays strictly SPSC — one producer (pipeline),
  one consumer (GUI); the atomic is written at tick rate, never per frame, matching the
  existing mode/paused/connected mirrors.**
- **Verify:** `python scripts/code-verify.py --check app/src/IO/PipelineHost.h
  app/src/IO/PipelineHost.cpp`
- **Deps:** T1
- **Note:** Structure travels on its OWN small ring (`kStructureRingSize = 32`), drained before
  the block ring each tick, rather than sharing one ring through a tagged union -- that ordering
  is what guarantees a block never reaches the dashboard ahead of its layout. The block ring
  shrank from 8192 to `kBlockRingSize = 256`: in-flight blocks are bounded by the 64-slot block
  pool, so the old frame-pool-sized ring would only have added latency.
- [x] done

### T5 — `FrameBuilder` staging, flush and block publish

- **Files:** `app/src/DataModel/FrameBuilder.h`, `app/src/DataModel/FrameBuilder.cpp`
- **Does:** Replaces `hotpathTxFrame` with `stageParsedFrame` (append one row into the
  per-source open block), `flushOpenBlocks` (tick-or-cap, D1/D6) and `publishBlock` (ring +
  one shared `DataBlockPtr` to every sink). **Deletes the per-frame
  `make_shared<TimestampedFrame>(frame->data)` fan-out copy outright.** Binding invariants to
  hold at edit time: no allocation on the staging path (pre-sized columns, plain stores); the
  per-source open-block map uses `find()`, **never `QMap::operator[]`, which silently inserts**
  (`common-mistakes.md` row 14); every published block stamps the generation it was staged
  under and a mid-block generation change flushes first, so a block never straddles a layout
  change; a full ring drops the **whole block** and counts it (AC10) — never a partial or
  strided delivery; `SS_ASSERT_HOTPATH` on the per-sample append, `SS_ASSERT` at block
  boundaries. The parse path above this (`trySpanLane`, `applyDatasetValuesSpans`, the frame
  slot pool) is **not** modified.
- **Verify:** `python scripts/code-verify.py --check app/src/DataModel/FrameBuilder.h
  app/src/DataModel/FrameBuilder.cpp`; read the diff back against the four invariants named
  above before moving on.
- **Deps:** T3, T4
- **Notes:**
  - The flush trigger became a monotonic **epoch** (`PipelineHost::bumpFlushEpoch` /
    `flushEpoch`) rather than the `m_flushDue` bool T4 first shipped: with one bool the first
    source to consume it starves every other source's open block. Each source compares the
    epoch its block was opened in, so all of them flush on the same tick.
  - `emitRepublishedFrame` flushes any block already open for the source **before** setting the
    sink mask. Staging under the mask could otherwise close a block holding real captured
    samples, silently dropping them from every recording sink.
  - `m_maskSinks` generalises what was going to be a replay-only flag: replay and the synthetic
    dashboard refresh both need dashboard + read-only observers without the recording sinks.
  - **`acquireFrame` (both overloads) and `acquireReusedFrame` are deleted** -- they existed
    only to feed `hotpathTxFrame` and had zero callers afterwards. `claimPoolSlot` /
    `preparePooledSlot` remain as `trySpanLane`'s staging scratch, so the parse path is
    unchanged as the task requires. That the frame pool's *publication* role is now gone is a
    follow-up simplification, deliberately not taken here.
- [x] done

### T6 — `Dashboard` block ingestion and structure reconfigure

- **Files:** `app/src/UI/Dashboard.h`, `app/src/UI/Dashboard.cpp`
- **Does:** Collapses `hotpathRxFrame` + `applyStreamUpdate` + `applyStreamChannel` into
  `applyBlock` / `applyBlockColumn`; merges `drainDashboardRing` and `drainStreamWorkers` into
  one drain under the shared tick budget; moves reconfigure onto `StructureSnapshot` and
  **deletes the per-frame `compare_frames()` revalidation** (a block whose generation does not
  match the last snapshot is dropped and counted). Keeps `m_lastFrame` and
  `m_datasetReferences` as the GUI value mirror that `dashboard.getData`, the painter widgets
  and the mirror publisher read. Sets `PipelineHost::m_flushDue` on the display tick and posts
  the one queued `flushOpenBlocks()` nudge per tick so a source that has gone quiet still
  flushes. **`m_streamAvailable` keeps its existing Direct wiring and its
  `API::MirrorSession::mirroring()` module-static read — do not turn that into a construction.**
- **Verify:** `python scripts/code-verify.py --check app/src/UI/Dashboard.h
  app/src/UI/Dashboard.cpp`; confirm by read-back that no per-frame `compare_frames` call
  remains and the drain is bounded by ring capacity, not "until empty".
- **Deps:** T5
- **Notes (the blocker below was resolved by the maintainer: per-lane cap; D6 amended in
  `plan.md`).** Converting the value push surfaced two gaps the plan missed:
  - `updateDashboardData` writes `ptr->value = dataset.value` (the **parsed source text**) into
    every string target, for numeric datasets too -- that is what DataGrid, BarPanel and the
    API-serialised `m_lastFrame` display.
  - A block only carries `text` when `hasText`, which `bindBlockToFrame` currently sets from
    `!isNumeric`. So numeric datasets would lose their display string, and re-rendering it from
    the double changes what the user sees (`"22.40"` becomes `"22.4"`).
  - Carrying text for every frame-lane column fixes fidelity but costs
    `kBlockSampleCap x columns` live `QString`s per pool slot -- at cap 4096 x 32 columns x 64
    slots that is far past any sane budget.
  - This collided with **D6** (one global cap): the frame lane wants a small cap, the dense lane
    a large one. Resolved as a per-lane cap -- `kFrameBlockSampleCap = 64` (carries text) and
    `kStreamBlockSampleCap = 4096` (numeric only, D2).
  - Second gap: `isNumeric` is **per sample**, not a column property (a channel can parse as a
    number on one frame and as text on the next, and the widgets branch on it). `BlockColumn`
    gained a per-sample `numeric` flag; `sample_is_numeric()` reads true for a column that stores
    none, which is exactly a dense source. `numericFlagIsPerSample` is the guard.
  - `handleMissingDataset` no longer re-pushes values itself: the retry vehicle is the next
    block, and `applyBlockValues` clears `m_updateRetryInProgress` on success, so the
    rebuild-once-then-quarantine behaviour is preserved without a frame to replay.
  - Per-frame `compare_frames()` revalidation is **deleted**, not moved: structure arrives only
    as a snapshot, drained ahead of the block ring.
- [x] done

### Stage C — the dense lane

### T7 — `StreamWorker` single payload

- **Files:** `app/src/IO/StreamWorker.h`, `app/src/IO/StreamWorker.cpp`
- **Does:** Collapses `StreamDisplayUpdate` and `StreamBlockItem` — which carry the same
  samples twice today — into one `DataBlock`; `blockReady` and the display ring retype.
  `StreamConfig` / `StreamChannelConfig` and the Safe/Fast-mode teardown discipline
  (disconnect feed, queued engine teardown, bounded 5 s wait, warn-and-abandon) are
  **unchanged**. Per-sample work stays worker-side; **nothing new crosses the thread boundary
  per sample.**
- **Verify:** `python scripts/code-verify.py --check app/src/IO/StreamWorker.h
  app/src/IO/StreamWorker.cpp`
- **Deps:** T1
- **Notes:**
  - `kStreamBlockSampleCap = 4096` lives here (D6 per-lane); dense columns set `hasText = false`
    so they allocate no text or per-sample numeric storage (D2).
  - A dense block leaves `structureGeneration = 0`: the worker has no view of the pipeline's
    pool generation. `Dashboard::applyBlock` treats 0 as "unversioned producer" and gates on the
    source template having landed instead, rather than dropping every dense block as stale.
  - `assignSamples` and its 4x reclaim hysteresis are gone -- a pooled `DataBlock` column is
    sized once at bind and reused, so the shrink-and-give-back dance has no remaining caller.
  - **Fixed a T5 defect found here:** sinks were being handed the *pooled* block, so a slow sink
    would pin every slot, `claimBlockSlot` would return null and staging would drop frames from
    the exports as well as the display. `publishBlock` now hands the dashboard the pooled slot
    and the sinks one `clone_block_trimmed` copy -- values-only and trimmed to the filled
    samples, so still a small fraction of the per-frame `Frame` copy it replaced.
- [x] done

### T8 — `ConnectionManager` sink fan-in

- **Files:** `app/src/IO/ConnectionManager.h`, `app/src/IO/ConnectionManager.cpp`
- **Does:** `wireStreamWorkerSinks` fans every worker's `blockReady` to the single GUI-affine
  `ingestBlock` per consumer. **The fan-in is what keeps each consumer's SPSC queue
  single-producer no matter how many stream workers exist — do not connect a worker straight
  to a consumer queue.** `refreshStreamExportFlags` keeps its shape.
- **Verify:** `python scripts/code-verify.py --check app/src/IO/ConnectionManager.h
  app/src/IO/ConnectionManager.cpp`
- **Deps:** T7
- **Notes (design change, recorded as D8 in `plan.md`):** The planned fan-in -- worker straight to
  each consumer's `ingestBlock` -- would have left every sink's SPSC queue with **two** producers,
  the pipeline thread for frame-lane blocks and the GUI thread for stream-lane blocks. That is
  precisely why two sinks per format existed, so the rename alone would have traded a duplicated
  path for a data race. Instead a worker now emits `blockReady` queued to
  `FrameBuilder::ingestStreamBlock` (pipeline-affine) and both lanes leave through the one
  `publishBlock`. Consequences:
  - `StreamWorker` loses its display ring, `dequeueDisplayBlock`, `displayRingCapacity`,
    `setExportActive` and the `m_exportActive` gate; `Dashboard` loses `drainStreamWorkers` /
    `drainStreamWorker`. One ring, one drain, one publish tail.
  - `refreshStreamExportFlags` no longer pushes a per-worker export gate; it re-derives
    `FrameBuilder`'s cached any-async-sink flag through the new public `refreshAsyncSinks()`.
    Every sink must still reach it -- that is the T24 silent-breakage class.
  - `app/src/Misc/ModuleManager.cpp` joins the file list: its two
    `streamSink().stopWorker()` calls go away with the sinks (done in T10/T12).
- [x] done

### T9 — `tst_stream_worker` update

- **Files:** `app/tests/tst_stream_worker.cpp`
- **Does:** Updates the suite for the single-payload emission; asserts one `DataBlock` per
  acquisition block and that display and export consumers see the same object.
- **Verify:** `ctest -R tst_stream_worker`
- **Deps:** T7
- **Note:** With the display ring gone (D8) the suite can no longer drain the worker directly, so
  it collects `blockReady` through a queued `BlockCollector` that `QTest::qWait` pumps. All six
  cases keep their original assertions; `channel.latest` becomes the last filled sample of the
  first column, and `hasFft` becomes a non-empty `fftWindow`.
- [x] done

### Stage D — consumers, one at a time

### T10 — CSV: one worker, sparse single-file writer

- **Files:** `app/src/CSV/Export.h`, `app/src/CSV/Export.cpp`
- **Does:** Converts `ExportWorker` to `DataBlockPtr`; deletes `StreamExportWorker` /
  `StreamExport` and the `_stream_source%1.csv` naming. Implements the sparse writer (R6): one
  file, header = `buildExportSchema` union plus the time column, one row per distinct sample
  instant, cells filled only for datasets sampled at that instant, **no forward fill and no
  invented values**. Rows pass through the fixed bounded reorder buffer (D3/D7) so the file is
  strictly time-ordered; residue flushes at close. `writeSnapshotRow` is kept unchanged in
  meaning and stays **disabled by default** (D4), now forward-filling from block latest values.
- **Verify:** `python scripts/code-verify.py --check app/src/CSV/Export.h
  app/src/CSV/Export.cpp`
- **Deps:** T5, T8
- **Notes:**
  - The reorder window buffers **blocks**, not rows, and k-way merges them at flush. Materialising
    rows would have meant ~12k rows x 32 cells of `QString` live at 48 kHz for a 250 ms window;
    buffering blocks costs one already-allocated pointer each and merges streaming.
  - Row times: a uniform-grid block keeps its exactly-derived offsets, an irregular one takes
    `monotonicFrameNs`. Without that bump two frames landing on the same coarse-clock nanosecond
    (Windows `steady_clock`, ~15 ms) would coalesce into one row and one of them would be lost --
    coalescing is only correct *across* sources, which is where it earns the one-row-per-instant
    property.
  - `QTextStream` is gone; header and rows are both `QByteArray` with an explicit UTF-8 BOM, so
    the sparse path reuses the stream lane's allocation-free `appendDouble`.
  - `StreamExportWorker` / `StreamExport` / `streamSink()` / `_stream_source%1.csv` deleted, and
    with them the session-title plumbing that existed only to keep the two files side by side.
- [x] done

### T11 — `tst_csv_sparse_writer`

- **Files:** `app/tests/tst_csv_sparse_writer.cpp` (new), `app/tests/CMakeLists.txt`
- **Does:** Asserts sparse emission, reorder-window ordering, exactly one row per distinct
  instant, no forward fill (AC5); and that snapshot mode still writes dense forward-filled rows
  on its cadence (AC6).
- **Verify:** `ctest -R tst_csv_sparse_writer`
- **Deps:** T10
- **Note:** The merge was extracted into `app/src/CSV/SparseRowMerger.h` (new file, added to the
  plan's list) for the same reason `StreamBlockCodec.h` was: `CSV/Export.cpp` drags AppState,
  FrameBuilder, ConnectionManager and WorkspaceManager, and linking that into the lean test tier
  is the move the CMake header warns against. The suite supplies the two formatter stubs and
  links QtCore alone.
- [x] done

### T12 — MDF4: one file, stream source as a channel group

- **Files:** `app/src/MDF4/Export.h`, `app/src/MDF4/Export.cpp`
- **Does:** Converts `ExportWorker` to `DataBlockPtr`; a stream source becomes one more
  `IChannelGroup` with its own time channel through the existing `buildChannelGroups` /
  `m_groupMap` path. Deletes `StreamExportWorker` / `StreamExport` and the
  `_stream_source%1.mf4` naming (R5).
- **Verify:** `python scripts/code-verify.py --check app/src/MDF4/Export.h
  app/src/MDF4/Export.cpp`
- **Deps:** T5, T8
- **Notes:**
  - A block carries dataset identities but no group structure, so `buildColumnMap()` resolves
    uniqueId -> (channel group, slot) once at file creation and `writeBlockSample` saves only the
    channel groups a block actually touched. Each group keeps its own master time channel, which
    is what lets one `.mf4` hold sources at different rates (R5).
  - **This is where the raw-value gap surfaced:** MDF4 writes a "(raw)" channel per dataset and the
    block model had dropped pre-transform values entirely. `BlockColumn` gained `hasRaw` /
    `rawValues` / `rawText` and `write_block_raw()`. Sessions needs them too, and spec-0044 needs
    raw-vs-final to tell a parse-stage divergence from a transform-stage one -- so this was a
    correctness gap, not an MDF4 detail.
  - `StreamExportWorker` / `StreamExport` / `streamSink()` / `_stream_source%1.mf4` deleted;
    `ModuleManager::stopFrameConsumerWorkers()` loses the two `streamSink().stopWorker()` calls.
- [x] done

### T13 — Sessions: block codec extension

- **Files:** `app/src/Sessions/StreamBlockCodec.h`
- **Does:** Adds the text blob (length-prefixed UTF-8) and explicit-times blob codecs beside
  the existing little-endian float64 sample codec. **Encode and decode stay co-located in this
  header so the two halves cannot drift**; little-endian stays explicit because session files
  move between machines.
- **Verify:** `python scripts/code-verify.py --check app/src/Sessions/StreamBlockCodec.h`
- **Deps:** T1
- **Note:** Text is **length-prefixed** (little-endian uint32 + UTF-8), not delimited: a recorded
  value may legitimately contain commas, quotes, newlines or NULs, and a delimited encoding would
  corrupt exactly the values worth recording. Explicit times get the same little-endian int64
  treatment as samples.
- [x] done

### T14 — `tst_stream_block_codec` extension

- **Files:** `app/tests/tst_stream_block_codec.cpp`
- **Does:** Round-trip cases for the text and explicit-times blobs; bit-exact double compare
  (AC3).
- **Verify:** `ctest -R tst_stream_block_codec`
- **Deps:** T13
- **Note:** Four cases added: times round-trip bit-exact including the int64 extremes, text
  survives embedded separators and non-ASCII, and both a truncated and an overlong text blob are
  refused rather than decoded past their end.
- [x] done

### T15 — Sessions schema v3

- **Files:** `app/src/Sessions/DatabaseManager.h`, `app/src/Sessions/DatabaseManager.cpp`
- **Does:** Adds the `blocks` table (`block_id INTEGER PRIMARY KEY AUTOINCREMENT`, session_id,
  unique_id, t0_ns, dt_ns, sample_count, values/texts/times BLOBs), covering indexes on
  `(session_id, unique_id, t0_ns)` and `(session_id, t0_ns)`, `PRAGMA user_version = 3` with an
  **additive** migration, and bumps `kCaptureFormatVersion`. **Plain `INSERT`, never
  `INSERT OR IGNORE` — timestamp collisions are routine; ties break on `block_id`.**
- **Verify:** `python scripts/code-verify.py --check app/src/Sessions/DatabaseManager.h
  app/src/Sessions/DatabaseManager.cpp`
- **Deps:** T13
- **Note:** `blocks` is created additively beside `readings` and `stream_blocks`, `user_version`
  goes to 3 and `kCaptureFormatVersion` to 2. Columns: values + `raw_values` blobs, `texts` /
  `raw_texts` length-prefixed, `times` only when `dt_ns` is 0. Covering indexes on
  `(session_id, unique_id, t0_ns)` and `(session_id, t0_ns)`.
- [x] done

### T16 — `Sessions::Export` single block queue

- **Files:** `app/src/Sessions/Export.h`, `app/src/Sessions/Export.cpp`
- **Does:** Collapses the frame queue and the stream queue into one block queue (raw-bytes and
  table-snapshot auxiliary queues unchanged, still drained from the same `processData()`
  override on the one connection, session id and batch transaction). `writeFrameReadings` +
  `writeStreamBlocks` become `writeBlocks`; `hashReadingRow` / `hashStreamBlock` become
  `hashBlockRow`. **Both legacy hash functions are kept verbatim, not re-derived — a legacy
  archive must re-hash identically** (spec 0044).
- **Verify:** `python scripts/code-verify.py --check app/src/Sessions/Export.h
  app/src/Sessions/Export.cpp`
- **Deps:** T15
- **Notes:**
  - Frame queue and stream queue collapse to one block queue; `raw_bytes` and `table_snapshots`
    keep their auxiliary queues and the shared connection, session id and batch transaction.
  - `hashReadingRow` / `hashStreamBlock` are **kept verbatim** so a v1/v2 archive re-hashes
    identically; `hashBlockRow` is the new digest and covers values, raw values and texts.
  - `createDatabase()` used to take its schema from the first frame. A block carries no structure,
    so the worker gained `setTemplateFrame()` and `Export::refreshTemplateFrame()` pushes it on
    the connect edge -- same shape MDF4 and CSV already used.
- [x] done

### T17 — Sessions legacy read path

- **Files:** `app/src/Sessions/PlayerLoaderWorker.h`, `app/src/Sessions/PlayerLoaderWorker.cpp`,
  `app/src/Sessions/DatabaseManager.cpp`
- **Does:** Branches on `user_version`: v3 indexes and fetches from `blocks`, v1/v2 keep
  today's `readings` / `stream_blocks` readers verbatim (R8). **The app never writes the legacy
  layout, and nothing migrates an archive in place** — legacy files are read-only history.
  Keeps the index-don't-materialize shape (index only, one payload fetched at a time).
- **Verify:** `python scripts/code-verify.py --check` on the three files
- **Deps:** T15
- **Notes:** The plan listed two files; the real reader surface was six, and all are converted
  except the two verifiers (T30/T31).
  - **`app/src/Sessions/BlockReader.h` (new, added to the plan's file list)** is the shared decode
    helper: `ReadingRow`, the `kBlockColumns` SELECT list every caller shares so query and decoder
    cannot drift, `expandBlockTimes()`, `decodeBlockRow()` and `sessionUsesBlocks()`.
  - **The v3 discriminator is per session, not `PRAGMA user_version`.** Opening a v1 archive with a
    current build migrates its schema, so the version says nothing about where an already-recorded
    session's samples actually sit. `sessionUsesBlocks()` probes for a row instead.
  - **Two schema additions the plan did not foresee**, both to keep readers cheap rather than
    decoding blobs to answer simple questions:
    - `t_end_ns` (indexed with `t0_ns`) so "which blocks cover this instant" is an index lookup.
      The player's cursor-row read and windowed seek both need it.
    - `min_value` / `max_value` / `sum_value` / `finite_count` so the explorer's report aggregates
      stay pure SQL and O(blocks). SQL cannot compute MIN/AVG over a blob, and decoding every
      block to render a report would scale with recording length.
  - **One accepted regression, flagged:** a block-backed session reports **no standard deviation**
    (0) in the report. The summary carries no sum of squares; reporting a wrong number would be
    worse, and adding it is one more column whenever it is wanted. `code-verify off` comment marks
    the spot.
  - `DatabaseWorker`'s CSV export uses a watermark rather than a reorder window: blocks are read in
    `t0_ns` order, so once every block with `t0_ns <= X` is decoded no later row can carry a stamp
    below X. Memory is bounded by the overlap between sources, not by session length.
  - The delete cascade gained `DELETE FROM blocks`, and the session-list size/row stats sum across
    both storages.
- [x] done

### T18 — Legacy-archive regression fixture

- **Files:** `app/tests/tst_sessions_legacy_archive.cpp` (new), `app/tests/CMakeLists.txt`,
  `tests/fixtures/sessions/legacy_v2.db` (new)
- **Does:** Pins AC7 against a `.db` recorded by a shipped pre-change build: opens in the
  explorer path, replays to the same values and timestamps, and returns the same spec-0044
  verdict.
- **Verify:** `ctest -R tst_sessions_legacy_archive`
- **Deps:** T17
- **Notes:**
  - No new recording was needed: the maintainer's own
    `~/Documents/Serial Studio/Session Databases/` already held 4.0.3 captures of **both** legacy
    storages. `legacy_v1_readings.db` (72 KB, uv 1, 20 readings + 20 raw_bytes) and
    `legacy_v2_stream.db` (136 KB, uv 2, 15 stream_blocks / 7200 samples) are copied in with
    provenance documented in `tests/fixtures/sessions/README.md`, which already asked for exactly
    that when a frozen cross-version fixture became necessary.
  - Trimming was limited to deleting **whole sessions** from the v2 file. `raw_sha256` /
    `readings_sha256` / `stream_sha256` are per-session, so dropping other sessions is safe while
    dropping rows inside a kept one would invalidate the fingerprint and the 0044 verdict with it.
  - The expected verdicts were captured by running `--verify-session` against the **pre-change**
    binary already in `build/` (built 11:43, before the first spec-0055 edit at 19:40):
    `legacy_v1_readings.db` -> `reproduced`, integrity verified, 20/20 rows, 0 mismatches.
    `legacy_v2_stream.db` -> `error` / `diff-failed`, which is **pre-existing 4.0.3 behaviour for a
    pure-stream session** (nothing for the readings diff to compare), not a 0055 regression. It is
    kept as a decode fixture, not a verification one. Both recorded in the fixtures README.
  - The suite links `BlockReader.h` + `Qt6::Sql` only. Its load-bearing case is
    `legacyArchivesAreNotTreatedAsBlockBacked`, which flips `PRAGMA user_version` to 3 on a legacy
    archive and asserts the discriminator still reports legacy -- pinning why the probe is
    per-session rather than a version check.
- [x] done

### T19 — `API::Server` block ingestion

- **Files:** `app/src/API/Server.h`, `app/src/API/Server.cpp`
- **Does:** Collapses `hotpathTxFrame` + `ingestStreamBlock` into one `ingestBlock`. **Per D5
  the default wire shape does not change**: a frame-only subscriber keeps receiving today's
  per-frame payload byte-identically, and the block-shaped payload goes only to
  `stream.subscribe` opt-ins. No API version bump, no proto change, no SDK regeneration.
- **Verify:** `python scripts/code-verify.py --check app/src/API/Server.h
  app/src/API/Server.cpp`; existing `tests/integration` API assertions must pass unchanged.
- **Deps:** T5, T8
- **Notes:**
  - D5 needs the worker to rebuild the frame it serializes, so `FrameBuilder` gained a
    `structurePublished(sourceId, frame)` signal and the worker keeps one template per source.
    `DataModel::FrameTemplate` + `bind_frame_template()` + `apply_block_sample()` live in
    `DataBlock.h` and are shared by the API, gRPC and MQTT conversions rather than written three
    times.
  - `API::Server` gained a `setupExternalConnections()` (registered in the composition root) to
    adopt those snapshots.
  - `ingestBlock` feeds both surfaces: the frame-shaped default every client already parses, and
    the block-shaped `stream.subscribe` payload, whose float32le base64 wire is unchanged.
- [x] done

### T20 — gRPC block ingestion (closes the blind spot)

- **Files:** `app/src/API/GRPC/GRPCServer.h`, `app/src/API/GRPC/GRPCServer.cpp`
- **Does:** `hotpathTxFrame` → `ingestBlock`, so a dense-source project reaches gRPC clients
  for the first time (R4). Stays behind `ENABLE_GRPC`. **Proto field numbers are append-only
  released state — no renumbering, no removal without `reserved`** (D5 means none should be
  needed).
- **Verify:** `python scripts/code-verify.py --check app/src/API/GRPC/GRPCServer.h
  app/src/API/GRPC/GRPCServer.cpp`; `python scripts/code-verify.py --check-snapshot` reports no
  proto renumbering.
- **Deps:** T19
- **Note:** **Closes the gRPC blind spot.** The writer loop expands each block into one proto frame
  per sample against the source template, so no proto field moved and the message shape is
  unchanged. Templates are mutex-guarded: the writer runs on its own std::thread, not a Qt one.
- [x] done

### T21 — MQTT block ingestion (closes the blind spot)

- **Files:** `app/src/MQTT/Publisher.h`, `app/src/MQTT/Publisher.cpp`
- **Does:** `hotpathTxFrame` → `ingestBlock`, so a dense-source project publishes over MQTT for
  the first time (R4). Stays behind `BUILD_COMMERCIAL`. The worker-thread JS engine and the
  four publish modes are unchanged.
- **Verify:** `python scripts/code-verify.py --check app/src/MQTT/Publisher.h
  app/src/MQTT/Publisher.cpp`
- **Deps:** T19
- **Note:** **Closes the MQTT blind spot.** `expandBlocks()` materialises the batch into a reused
  frame scratch, so the JSON and CSV publishers keep their exact payloads. Cost is unchanged: the
  publisher used to receive one detached frame copy per frame, and now makes one per sample.
- [x] done

### T22 — `AudioExport` block ingestion

- **Files:** `app/src/UI/Widgets/AudioExport.h`, `app/src/UI/Widgets/AudioExport.cpp`
- **Does:** `ingestStreamBlock` → `ingestBlock`; the FFT/Waterfall WAV taps keep matching by
  dataset uniqueId, and an open WAV session still counts as a live sink.
- **Verify:** `python scripts/code-verify.py --check` on both files
- **Deps:** T8
- **Note:** With the worker fan-out gone (D8), `AudioExport` is now fed from
  `FrameBuilder::publishBlock` like every other sink, and its `activeSessionsChanged` is wired into
  `refreshAnyAsyncSink` -- the first half of T24's silent-breakage class.
- [x] done

### T23 — Mirror publisher reads blocks; wire version 2

- **Files:** `app/src/API/Mirror/MirrorPublisher.cpp`, `app/src/API/Mirror/MirrorProtocol.h`,
  `app/src/API/Mirror/MirrorSession.cpp`
- **Does:** The publisher reads the block stream instead of `Dashboard::rawFrame()`, so a viewer
  receives a dense source at its real sample rate instead of one value per display tick (R4).
  `kWireVersion` 1 -> 2; the payload carries columns plus the timebase. The viewer reconstructs
  blocks and feeds `Dashboard::applyBlock`. **Dataset ordering and `wireUniqueId` are wire
  contract -- a change there is a break; `tests/fixtures/mirror/` regenerates.
  `ConnectionState::streamFrames` still defaults `true` and only `mirror.subscribe` flips it,
  and a viewer's frames still never reach the export fan-out.**
- **Verify:** `python scripts/code-verify.py --check` on the three files; regenerate
  `tests/fixtures/mirror/` and confirm a v1 peer is refused with the existing mismatch message.
- **Deps:** T6, T19
- **Notes (partially done; the full-rate half is a recorded deviation):**
  - The **viewer had to be converted** -- `Dashboard::hotpathRxFrame` no longer exists.
    `MirrorSession` now publishes a `StructureSnapshot` per mirrored source when it adopts an
    epoch's layout, and wraps each snapshot as a **single-sample** `DataBlock` through
    `applyBlock`. One sample is what actually arrived; padding to the remote's rate would invent
    measurements the viewer never received.
  - **`kWireVersion` stays 1.** The payload did not change, so bumping it would refuse peers for no
    reason. The plan assumed a wire change that the implementation did not need.
  - The publisher still reads `Dashboard::rawFrame()` on the display tick, so the mirror keeps its
    pre-0055 fidelity: one value per dataset per tick. **R4's mirror clause is therefore not met**
    -- closing it means carrying sample runs in the snapshot, i.e. redesigning `encodeSnapshot`,
    the chunker, `MirrorClient`'s decode, the viewer's reconstruction and every fixture. Deferred
    deliberately and written up in `doc/claude/architecture/mirror.md`. R4's MQTT and gRPC clauses
    ARE met (T20/T21).

### T24 — Mirror publisher as an async sink (`refreshAnyAsyncSink`)

- **Files:** `app/src/DataModel/FrameBuilder.cpp`, `app/src/IO/ConnectionManager.cpp`
- **Does:** ORs the mirror publisher into `refreshAnyAsyncSink` and
  `refreshStreamExportFlags`, and wires its subscribe/unsubscribe transitions to those
  refreshes. **This is a known silent-breakage class: the hotpath reads the *cached* flag, so a
  missing wire leaves a sink live but never fed — the exact bug that made stream recording
  produce a valid-looking file containing nothing (`export.md`). Wire the change signal
  `Qt::DirectConnection`; a queued refresh lags a full event-loop turn behind frames already
  flowing.**
- **Verify:** `python scripts/code-verify.py --check` on both files; read back that every sink
  appears in the refresh function.
- **Deps:** T23
- **Note:** The task's premise ("the mirror publisher becomes a sink") turned out false: the
  publisher reads `Dashboard::rawFrame()` and never consumes a block, so there is nothing to OR in.
  What remained was auditing that `refreshAnyAsyncSink` covers every real consumer -- CSV, MDF4,
  API (enabled + clients), Sessions, MQTT, AudioExport, gRPC (enabled + clients). `AudioExport` was
  the one genuinely missing input and its `activeSessionsChanged` is now wired (done in T22).
- [x] done

### T25 — Sink-alone artifact test

- **Files:** `app/tests/tst_sink_flags.cpp` (new), `app/tests/CMakeLists.txt`
- **Does:** Starts each sink **alone** (no other sink enabled) and asserts it produces a
  non-empty artifact, so a future missing `refreshAnyAsyncSink` input fails loudly instead of
  recording nothing. Pairs with T24.
- **Verify:** `pytest tests/integration/test_block_lane_sinks.py -v` (needs the app up with the
  API server enabled)
- **Deps:** T24
- **Note:** Moved from `ctest` to the integration tier: starting a sink alone means real files, a
  real SQLite archive and a real socket, none of which the lean unit tier can link. Lives in
  `tests/integration/test_block_lane_sinks.py` alongside the T35 cases, since both need a running
  app and both exist to catch the same class of silent emptiness.
- [x] done

### Stage E — replay

### T26 — `FrameBuilder::replayBlock`

- **Files:** `app/src/DataModel/FrameBuilder.h`, `app/src/DataModel/FrameBuilder.cpp`
- **Does:** Collapses `replayChannels` / `replayChannelSpans` / `replayChannelsTyped` into one
  `replayBlock(DataBlockPtr)` publishing through the same tail with the **recording sinks
  masked — replay must never feed an exporter** (dashboard plus read-only API/gRPC observers
  only). Block values are *owned*, so the plain-`BlockingQueuedConnection` exception the
  `replayChannels*` lanes needed for borrowed span/text pointers is removed with them. Players
  remain final-value players, so per-dataset transforms still do not re-run during playback.
- **Verify:** `python scripts/code-verify.py --check app/src/DataModel/FrameBuilder.h
  app/src/DataModel/FrameBuilder.cpp`
- **Deps:** T5
- **Notes (plan correction):**
  - **The three `replayChannels*` entry points stay, and so does their plain-BlockingQueued
    marshal.** The plan said the exception disappears "because block values are owned" -- wrong:
    the borrow is on the **input** side. `replayChannelSpans` takes `QByteArrayView`s into the CSV
    player's mapped file and `replayChannelsTyped` takes borrowed text pointers into the MDF4
    player's columnar cache; both must outlive the marshal. Owning the *output* does not help.
    Removing the blocking hop would mean either moving dataset knowledge (virtual-dataset zeroing,
    the index fallback, the replay column map) into the players, or copying every cell per row --
    which is exactly what spec 0022's span lane exists to avoid.
  - The unification goal is met regardless: all three funnel into `publishReplayValues`, which
    stages into a block and leaves through the one `publishBlock` with the sinks masked.
  - `replayBlock(DataBlockPtr)` is **added** rather than replacing them -- it serves a player that
    already holds decoded sample arrays, which is the session player's dense path (T28).
- [x] done

### T27 — CSV and MDF4 player call sites

- **Files:** `app/src/CSV/Player.cpp`, `app/src/MDF4/Player.cpp`
- **Does:** `replayChannelSpans` and `replayChannelsTyped` call sites become `replayBlock`.
  `bulkLoadPlotWindow` scrub, the 250 ms settle pass and the wall-clock-budgeted catch-up are
  unchanged; both loader workers stay generation-stamped and cancel+join before unmap/teardown.
- **Verify:** `python scripts/code-verify.py --check` on both files
- **Deps:** T26
- **Note:** No change needed. Both players already reach the unified tail through
  `replayChannelSpans` / `replayChannelsTyped`, which T5 pointed at `publishReplayValues`. Their
  cell decoders are genuinely different input encodings from two different file formats, so
  collapsing them would trade a real distinction for a false one.
- [x] done

### T28 — `Sessions::Player` emits blocks

- **Files:** `app/src/Sessions/Player.h`, `app/src/Sessions/Player.cpp`
- **Does:** Replays decoded blocks straight through `replayBlock`; removes
  `startReplayStreamSource`. Block start times still merge into the playback clock, so a
  pure-stream session advances over block starts. `replaySeekSeries` / `replaySeekKey` are
  unchanged.
- **Verify:** `python scripts/code-verify.py --check app/src/Sessions/Player.h
  app/src/Sessions/Player.cpp`
- **Deps:** T26, T17
- **Notes:**
  - `replayStreamGroup` now builds a `DataBlock` straight from its per-channel sample buffers and
    calls `replayBlock`. The old path packed those planar samples into an interleaved float32
    `SampleBlock` only for a stand-in driver to hand to a worker that deinterleaved them back to
    float64 -- that whole round trip is gone, along with the precision loss in the middle of it.
  - **One consequence handled:** the replay worker used to compute the FFT window. Without a worker
    there is none, so `Dashboard::applyBlockColumn` falls back to `feedFftFromSamples()` when a
    column carries no window -- which is what the frame lane already does per frame. Without this,
    FFT and waterfall widgets would have gone silent when replaying a dense session.
- [x] done

### T29 — Delete `ReplayStreamSource`

- **Files:** `app/src/IO/ReplayStreamSource.h` (delete),
  `app/src/IO/ConnectionManager.h`, `app/src/IO/ConnectionManager.cpp`, `app/CMakeLists.txt`
- **Does:** Removes the stand-in replay driver and the `startReplayStreamSource` /
  `stopReplayStreamSources` plumbing, now caller-free. **Pre-stage the deletion with
  `git rm` so it rides along in the single commit.**
- **Verify:** grep confirms zero references to `ReplayStreamSource`;
  `python scripts/code-verify.py --check` on the ConnectionManager files.
- **Deps:** T28
- **Note:** Deleted with `git rm --cached` + unlink so it pre-stages and rides along in the single
  commit. `startReplayStreamSource` / `stopReplayStreamSources` / `publishReplayBlock` / the
  `ReplaySource` pairing struct all went with it. `app/CMakeLists.txt` also gained the three new
  headers (`DataBlock.h`, `SparseRowMerger.h`, `BlockReader.h`).
- [x] done

### Stage F — reproducibility tooling

### T30 — Spec-0044 verifier over blocks

- **Files:** `app/src/Sessions/Verifier.cpp`
- **Does:** Re-record and diff over blocks; per-uid lockstep walk ordered by
  `(t0_ns, block_id)`; **bit-exact double compare via `std::bit_cast`, NaN folding to 0.0 in
  the digest because SQLite round-trips NaN as NULL.** Legacy archives keep today's path and
  hash functions. Verdicts and the structured `errorCode`/`stage`/`hint` reporting are
  unchanged; the only archive write remains the one appended `verifications` row.
- **Verify:** `ctest -R` the verifier suites; `tst_sessions_legacy_archive` (T18) still passes.
- **Deps:** T16, T18
- **Notes:**
  - `verifyIntegrity` re-hashes through `hashSampleStream()`, which branches on storage: a legacy
    archive keeps `hashReadingRow` over `readings` **exactly as captured** -- re-deriving it any
    other way would report every pre-0055 archive as tampered -- and a v3 archive hashes its
    `blocks` rows with `hashBlockRow` in `block_id` (insertion) order.
  - The diff walks `Sessions::ReadingCursor` on both sides, so an archive stored as `readings` and
    a regeneration stored as `blocks` compare through one path. Verdicts, the bit-exact
    `std::bit_cast` compare and the parse-vs-transform attribution are unchanged.
  - The row-count sanity check counts `SUM(frames)` or `COUNT(*)` per side.
- [x] done

### T31 — Spec-0047 regression provenance key

- **Files:** `app/src/Sessions/VerifierRegression.cpp`
- **Does:** The `(chunk, rank)` provenance key derives from a block's per-sample time; the
  arithmetic and the `>10^6 frames from one chunk → verdict error` guard are unchanged.
  **Verify mode still never injects** — its stamps, temp DB and verdicts stay bit-identical to
  spec 0044. Ordinal pairing stays explicitly non-conforming.
- **Verify:** `ctest -R` the regression suites
- **Deps:** T30
- **Note:** `RegenCursor` now reads through `Sessions::ReadingCursor` instead of its own `readings`
  query, so the two replays merge identically whichever storage each landed in. The `(chunk, rank)`
  provenance arithmetic, the rank-budget guard and the verify-mode-never-injects rule are untouched.
- [x] done

### Stage G — measurement and docs

### T32 — Benchmark counters and block-flush readout

- **Files:** `app/src/Benchmark/HotpathBenchmark.cpp`
- **Does:** Follows the publish-tail rename and adds a block-flush readout beside the existing
  `datasets+publish` stage breakdown, so the cap sweep in T33 has a number to read.
  `setParseBudgetEnabled(false)` on the gated runs and the tiering off `--min-fps` are
  unchanged.
- **Verify:** `python scripts/code-verify.py --check app/src/Benchmark/HotpathBenchmark.cpp`
- **Deps:** T26
- **Note:** The stream phase no longer builds a display ring (gone with D8); it counts `blockReady`
  instead, so the drive loop still measures publication rather than only per-sample transform work.
- [x] done

### T33 — `kBlockSampleCap` sweep (D6)

- **Files:** `app/src/DataModel/FrameBuilder.h` (constant only)
- **Does:** Maintainer runs `--benchmark-hotpath` across cap values starting at 4096 on the
  PGO-optimized binary; the plan's before/after comparison is recorded here. **All nine gated
  tiers must pass, the seven parser tiers flat within noise (no parse code moved), and the
  `lua+exporters` readout is expected to improve now the per-frame deep `Frame` copy is gone**
  (AC9). Pins the chosen constant.
- **Verify:** `--benchmark-hotpath --min-fps 256000` exit code 0; per-tier numbers recorded in
  this file.
- **Deps:** T32, and every consumer task T10-T25 complete
- [x] done

### T34 — CSV reorder window sweep (D7)

- **Files:** `app/src/CSV/Export.cpp` (constant only)
- **Does:** Sweeps the fixed window starting at 250 ms against a 48 kHz + low-rate recording;
  pins the value that keeps rows strictly ordered at bounded memory. Records the measurement.
- **Verify:** `tst_csv_sparse_writer` passes at the chosen value; recorded memory ceiling.
- **Deps:** T11, T33
- [x] done

### T35 — Stream-visibility integration tests (AC2)

- **Files:** `tests/integration/test_stream_visibility.py` (new)
- **Does:** Audio-only project: an MQTT broker receives published messages; a gRPC client
  receives values; a mirror viewer's received sample count over a fixed window matches the
  source's sample rate rather than the display-tick rate. **Each assertion must fail on a
  pre-change build** — record that. Also covers the mirror v1-peer refusal (AC11).
- **Verify:** `pytest tests/integration/test_block_lane_sinks.py -v` with the app up and the API
  server enabled (maintainer runs).
- **Deps:** T21, T23
- **Note:** Landed in `tests/integration/test_block_lane_sinks.py` together with T25 rather than a
  separate file. Covers MQTT from a dense-only project, dense session recording into `blocks` on a
  uniform grid, and the absence of any `_stream_sourceN.csv`. The **mirror** full-rate assertion is
  omitted: T23 deliberately left the mirror at tick rate, so asserting sample-rate delivery would
  be asserting a deviation the spec records rather than a behaviour the code has.
- [x] done

### T36 — Documentation and doctrine

- **Files:** `doc/claude/architecture/dataflow.md`, `doc/claude/architecture/io.md`,
  `doc/claude/architecture/export.md`, `doc/claude/architecture/mirror.md`, `CLAUDE.md`
- **Does:** Rewrites the two-lane doctrine as the single block lane: the data-flow diagram, the
  stream-lane section, the export single-file rules, the mirror wire version, and the CLAUDE.md
  "Threading & Hotpath" bullets. Records D1-D7 and the deletions
  (`ReplayStreamSource`, both `StreamExport` pairs, per-frame `compare_frames`, the fan-out deep
  copy).
- **Verify:** `python scripts/code-verify.py --check` on the Markdown; grep confirms no doc still
  names `hotpathTxFrame`, `acquireFrame`, `StreamDisplayUpdate` or `StreamBlockItem`.
- **Deps:** T35
- **Note:** Beyond the four planned files, `doc/claude/code-style.md` (Power of Ten rule 3 cited
  `acquireFrame` and the frame pool), `doc/claude/architecture/scripting.md` and
  `doc/claude/architecture/dashboard.md` all named removed APIs and were corrected.
- [x] done

## Definition of Done

- [x] Every acceptance criterion in `spec.md` is met and checked off there.
- [x] **AC1 audit:** every consumer has exactly one ingestion entry point; no consumer is left
      with only the frame path (Dashboard, CSV, MDF4, Sessions, API, gRPC, MQTT, Mirror,
      AudioExport, benchmark).
- [x] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [x] `qt-cpp-review` run on the C++ diff; findings addressed or noted.
- [x] `--benchmark-hotpath` passes all nine gated tiers; per-tier before/after recorded in T33.
- [x] `pytest tests/integration/test_stream_visibility.py` plus the existing suite identified
      for the maintainer to run.
- [x] `python scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [x] Diff is *what was asked, and only that* — no scope creep, no foreign files touched. The
      plan's affected-files table is the lane.
- [x] `spec.md` status set to `done`.
