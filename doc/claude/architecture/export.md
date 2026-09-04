# Architecture — Export & Sessions DB

> Part of the architecture corpus ([index](../architecture.md)). Read this file in full
> before touching CSV/MDF4 export, the Sessions database, or replay.

## Export Architecture & Sessions DB (Pro)

### What every sink shares

- **The session boundary.** All three recording sinks close on
  `FrameBuilder::sessionBoundary(bool connected, bool paused)` — connect, disconnect **and
  pause/resume** — never on `connectedChanged` / `pausedChanged` directly. The builder flushes
  every open block *before* emitting, so a sink's `close()` drains the samples staged while its
  file was open rather than finding the file gone and opening a second file for the tail. That
  ordering is the contract; see [dataflow.md](dataflow.md) "The Session Boundary".
- **Per-source time.** The monotonic tie-break is per source
  (`FrameConsumerWorkerBase::monotonicSourceNs`), and a uniform-grid block never takes it at all:
  its offsets are exactly derived, so bumping them would falsify the grid. Two sources recorded
  into one file therefore keep their own clocks.
- **`DataModel::ExportStructure`** (`core/Pipeline/DataModel/ExportStructure.{h,cpp}`) is the schema
  half every worker owns by value: the template frame a file's columns are created from, the two
  ways it is adopted (`setTemplateFrame` from the pipeline, `applyPublishedStructure` from a
  structure snapshot, which only fills an **empty** slot so an open file keeps its schema and an
  empty frame never wipes an adopted template), plus the two static path rules
  `sanitizeTitle(title, fallback)` and `sessionDir(workspaceKey, title, fallback)` — a title can
  neither escape the workspace nor scrub away to nothing. The fallback is a parameter because the
  three lanes disagree: CSV and Sessions fall back to `"Untitled"`, MDF4 to `"SerialStudio"`, so
  no lane's folder name changed when the three copies collapsed into one.
- **A write failure is not silent.** `Sessions::Export` checks every
  `transaction()` / `commit()` / `exec()` result into `noteWriteFailure`: a latched
  `writeFailed` flag, a `droppedBlocks` count, a queued `writeErrorChanged` to the GUI, `isOpen()`
  reading false afterwards (it is the recording indicator, so the latch folds into it rather than
  into a new property), and `finalizeSession` storing NULL digests instead of a fingerprint over
  lost rows. `sessions.getStatus` exposes `writeFailed`, `rawOverruns`, `droppedBlocks` and
  `currentSessionId`; CSV's interval mode closes and reports the same way its sparse path does.
- **The live session cannot be edited out from under itself.** `sessions.delete` and the
  DatabaseManager's delete / notes / tag-assign verbs refuse
  `Sessions::Export::currentSessionIdOrNone()` with `SESSION_LIVE`, reported as a
  NotificationCenter warning rather than a modal.
- `DataModel::ExportSchema` (`ExportSchema.h`): shared column layout. `buildExportSchema(frame)`
  produces sorted columns + `uniqueIdToColumnIndex` map. CSV and MDF4 export raw + transformed.
- **CSV logging cadence (`CSVExportInterval` setting, spec 0023)**: `CSV::Export` holds an
  `exportInterval` (ms; QSettings, default 0) forwarded to the worker via queued invoke
  (`setSnapshotIntervalMs`). At **0** the worker writes one row per received frame — the
  historical behavior — via `writeRow(frameTs)` inside `processItems`. At **>0** `processItems`
  only forward-fills `m_lastFinalValues` and a worker-owned `Qt::PreciseTimer`
  (`writeSnapshotRow`) writes one full-schema row every N ms, using `now()` against the session
  reference timestamp; it drains the queue first so cell staleness is bounded by the interval,
  and never writes before the session's first frame (lazy file creation). This turns CSV into a
  bounded-size trend log for multi-source / high-rate projects (the CSV schema is the union of
  **all** sources' datasets, so at per-frame cadence a 48 kHz audio source emits ~48 k wide,
  mostly-forward-filled rows/s; MDF4/Sessions stay full-rate and sparse and are the right home
  for sample-rate data). Applies live to an open recording. The `csvExport.setInterval` API and
  the Preferences → Export tab both drive `exportInterval`.
- **One file per format per session (spec 0055).** The per-source `*_stream_sourceN.csv` /
  `.mf4` files are gone with the sinks that wrote them: every consumer now ingests one
  `DataModel::DataBlock` from the pipeline thread, both lanes.
  - **CSV writes SPARSE rows into one file**: one row per distinct sample instant, cells filled
    only for the datasets sampled at that instant, nothing forward-filled and nothing invented.
    The merge lives in `CSV/SparseRowMerger.h` (extracted so it is unit-testable without QFile,
    the workspace manager or a session) and buffers **blocks, not rows** -- at 48 kHz a 250 ms
    reorder window is ~12k rows, which as cells would be hundreds of thousands of live QStrings.
    A uniform-grid block keeps its exactly-derived offsets; an irregular one takes
    `monotonicSourceNs`, because without that bump two frames landing on the same coarse-clock
    nanosecond would coalesce into one row and one of them would be lost. The bump is **per
    source** for the same reason coalescing is only correct ACROSS sources: one global last-offset
    let a fast source ratchet a slow one's timestamps forward. `CSVExportInterval` still switches
    to dense forward-filled rows and still defaults to disabled (D4).
  - **MDF4 writes one `.mf4`** with a channel group per project group *and* per stream source,
    each on its own master time channel -- which is what lets one file hold sources at different
    rates. `buildColumnMap()` resolves uniqueId -> (channel group, slot) once at file creation,
    since a block carries dataset identities but no group structure. **`createTimeChannel` sets
    `Sync(Time)`** and the absolute-epoch write into the master is gone (B10): the master is a
    relative time base, and stamping it with a wall-clock epoch made every group's time axis
    disagree with the others. The reader accepts sync `Time` or the legacy `None`
    (`isTimeMaster()`), so archives written by older builds still load. Text channels declare
    UTF-8. `MDF4::PlayerLoaderWorker` decodes **per channel group** into time-major columnar
    arrays (one timestamp vector per group plus one value vector per channel, appended in
    `OnSample` order) and merges them k-way by key, so the memory bound is samples x
    channels-in-group rather than the old dense per-instant map.
- **Sessions store one unified `blocks` table (spec 0055, schema `user_version` 3)**: one row per
  dataset per published block, for both lanes. `values_blob` + `raw_values` are little-endian
  float64, `texts`/`raw_texts` length-prefixed UTF-8 (a recorded value may contain commas, quotes
  or NULs, so delimiting would corrupt exactly the values worth recording), `times` present only
  when `dt_ns` is 0. `t_end_ns` is indexed with `t0_ns` so a reader finds the blocks covering an
  instant by lookup instead of decoding, and `min_value`/`max_value`/`sum_value`/`finite_count`
  keep the explorer's report aggregates pure SQL -- SQL cannot compute MIN/AVG over a blob.
  `hashBlockRow` is the spec-0044 digest; `hashRawChunk` / `hashReadingRow` are **kept verbatim**
  so a legacy archive re-hashes identically.
  - **Legacy archives are read-only history (R8).** `readings` and `stream_blocks` still open,
    replay and verify; the app never writes them again. Every reader goes through
    `Sessions/BlockReader.h` -- `decodeBlockRow()`, `expandBlockTimes()`, the `ReadingCursor` that
    streams either storage a block at a time, and `sessionUsesBlocks()`. **That probe is
    per-session, deliberately NOT `PRAGMA user_version`**: opening a legacy archive with a current
    build migrates its schema, so the version says nothing about where an already-recorded
    session's samples sit. `tests/fixtures/sessions/` holds frozen 4.0.3 captures of both legacy
    storages; `tst_sessions_legacy_archive` pins the probe against them.
  - **Replay re-enters through the unified tail.** The session player builds `DataBlock`s straight
    from its stored blobs and calls `FrameBuilder::replayBlock`, which publishes with the recording
    sinks masked. The spec-0054 `ReplayStreamSource` stand-in driver and its replay `StreamWorker`
    are deleted, along with the float32 interleave round trip they required. Because no worker sits
    in front of a replay any more there is no precomputed FFT window, so `applyBlockColumn` falls
    back to feeding the FFT and waterfall series from the samples -- what the frame lane always did.
- **The Historian lives in `core/Storage/Sessions/`** (`namespace Sessions` for all three classes):
  - `Sessions::DatabaseManager` — singleton owning the open `.db`; backs `app/qml/DatabaseExplorer/`.
  - `Sessions::Export` (`Sessions/Export.h/.cpp`): `FrameConsumer`-based; tables
    `sessions` / `columns` / `blocks` / `raw_bytes` / `table_snapshots` (`readings` and
    `stream_blocks` are read-only legacy, see below); a second lock-free queue carries raw bytes
    from `ConnectionManager::onRawDataReceived` and shares the block lane's flush trigger through
    `noteSecondaryEnqueued`, counting `rawOverruns` when it cannot keep up. WAL mode, batch
    transactions. Recording is refused outside a data mode: `setExportEnabled` returns false in
    ConsoleOnly, so there is no console-only raw-DB recording branch.
  - `Sessions::Player`: replays a stored session through the FrameBuilder pipeline using the
    **final** (post-transform) reading columns, with a uid->cell replay column map installed via
    `FrameBuilder::setReplayColumnMap` (same mechanism as MDF4). **All three players count as
    final-value players** (`SerialStudio::isFinalValuePlayerOpen`), so per-dataset transforms
    never re-run during playback — they read live inputs (data tables) that don't exist then.
    Raw columns are only a fallback for pre-final-column session files.
  - **The three players share `DataModel::ReplayPlaybackEngine`** (`ReplayPlaybackEngine.{h,cpp}`,
    held by value as `m_engine` in each): the scrub timer chain (`kSeekTickMs` 33 +
    `kSeekSettleMs` 250), the **playback epoch** that retires a superseded `play()`'s timer chain
    (a stale chain used to keep advancing a paused player), the steady-clock anchor that makes the
    **recording** own replay time (`anchorSteadyBase` / `steadyTimestampFor`), the catch-up fill
    gate (`kCatchUpBudgetMs` 20, wall-clock budgeted rather than a fixed row batch), the trailing
    `seekWindowStartRow` walk and `formatTimestamp`. It is **composed, not inherited**, because
    the players differ in storage, not in mechanics. Two deltas from folding three copies into
    one: `formatTimestamp` now clamps at zero everywhere (MDF4's copy did, the other two did
    not — identical output for the non-negative offsets they actually produce), and MDF4's
    `setProgress` rounding is deliberately **not** unified, since each player's clamp moves the
    seek cursor by a row.
  - **CSV/MDF4 players stream instead of materializing (spec 0022)**: the CSV player maps the
    file (`QFile::map`) and a `CSV::PlayerLoaderWorker` thread builds only row offsets +
    per-row seconds (`indexing`/`indexProgress` properties; playback clamps to the growing
    frontier and auto-resumes); rows split on demand via the byte-level
    `splitReplayRowSpans` (semantics-identical twin of `splitReplayRow`) and inject through
    `FrameBuilder::replayChannelSpans` (UTF-8 views, in-place writes). The MDF4 player decodes
    on an `MDF4::PlayerLoaderWorker` thread (mdflib pointers never leave the worker; the
    ns-quantized cache-key merge is unchanged) into columnar per-channel vectors and injects
    through `FrameBuilder::replayChannelsTyped` (native doubles + borrowed text — no per-cell
    `QString::number`/`toDouble` round trip; display strings stay 'g'/10-identical via
    std::to_chars). Both workers are generation-stamped and cancel+join before unmap/teardown;
    re-opening mid-index is safe.
  - **ProjectFile replay bypasses the byte round-trip (spec 0020)**: all three players hand
    their already-split cells to `FrameBuilder::replayChannels(sourceId, channels, recordedTs)`
    — no `joinReplayRow` → bytes → re-split. It publishes via the slot pool through
    `publishReplayValues`: dashboard + read-only observers (API/gRPC, only with a client
    connected) and **never a recording sink** (CSV/MDF4/Sessions export, MQTT, InfluxDB) — replay
    cannot re-record itself. Recorded timestamps ride the frame (players anchor a steady base
    per `anchorSteadyBase` and stamp rows with recorded deltas).
  - **QuickPlot replay keeps the RFC-4180 byte rows**: players synthesize rows with
    `DataModel::joinReplayRow` and FrameBuilder splits them with `splitReplayChannels` /
    `splitReplayRow` (`FrameParserPipeline.h`). The live QuickPlot split
    (`splitQuickPlotChannels`) is untouched — the quote-aware splitter only runs when
    `m_playerOpen` is set.
  - **Tape scrub (spec 0020)**: `setProgress` in all three players coalesces slider ticks to
    ~30 Hz; each tick calls `Dashboard::bulkLoadPlotWindow` (rings rebuilt directly from the
    player's row storage via `replaySeekSeries`/`replaySeekKey`; Sessions uses a windowed
    `readings` range query with forward-fill) plus one cursor-row inject for scalar widgets;
    a 250 ms settle timer then replays the exact trailing window through `replayChannels`
    (FFT/waterfall/GPS/3D correct at rest). Playback catch-up is wall-clock budgeted
    (~20 ms/pass) instead of a fixed 100-row batch — lossless, stretches when underpowered.
  - **`table_snapshots` capture**: `Sessions::Export::captureTableSnapshots` (main thread,
    `TimerEvents::timeout1Hz`) diffs `FrameBuilder::tableStore().snapshot()` (skipping the
    `__datasets__` system table) against the last tick and enqueues changed registers to the
    worker, which batches them into `table_snapshots`. Replay does NOT need them (finals are
    replayed); they exist for post-hoc inspection.
  - Per-sample tables use **surrogate rowid PKs** (`reading_id`, `raw_id`, `snapshot_id`
    `INTEGER PRIMARY KEY AUTOINCREMENT`) with covering indexes on
    `(session_id, unique_id, timestamp_ns)` and `(session_id, timestamp_ns)`. Use plain
    `INSERT` — **never `INSERT OR IGNORE`** — `timestamp_ns` collisions are routine.
  - Break ts ties with `reading_id` in ORDER BY / MIN/MAX subqueries. `DISTINCT timestamp_ns`
    stats undercount on collisions.
  - **View-state bundle (spec 0062)**: `sessions.view_state` (nullable TEXT, `kUserVersion` 4)
    carries `UI::Dashboard::viewStateJson()` — per-widget cursors, zoom/pan, crosshair mode and
    pause pushed by `Plot.qml`/`MultiPlot.qml` through `saveWidgetViewState` (500 ms coalesce),
    snapshotted by `Sessions::Export` beside the project snapshot and written by the worker at
    session start, on a 1.5 s debounce (`ExportWorker::storeViewState`) and in `finalizeSession`.
    `Sessions::Player` captures the pre-session view state, applies the bundle after
    `restoreProjectFromJson` (widgets read it in `Component.onCompleted`), restores it on close,
    and posts a Notification-Center warning when the embedded project differs from the one on
    disk (the recording's project always wins). View state never marks the project modified.

## Reproducibility Verification (spec 0044)

- **Capture-side fingerprints.** `ExportWorker` keeps two incremental SHA-256 hashes on the
  worker thread: raw chunks (in `writeRawBytes`, insertion = `raw_id` order) and block
  rows (in `insertBlockRow`), over the canonical byte layout in
  `Sessions::hashRawChunk` / `hashBlockRow` (`Export.h` free functions — the verifier
  reuses them, never re-derive the layout). `finalizeSession()` stamps them into the
  session row together with `app_version`, `capture_format`
  (`DatabaseManager::kCaptureFormatVersion`), `repro_class` (JSON: controlScript /
  transformsPresent / tablesPresent / virtualDatasets), and the link-loss counters sampled
  at 1 Hz on the main thread (`Export::sampleSessionHealth`, delta-accumulated,
  decrease = reader reset). All new `sessions` columns are nullable; NULL means legacy
  capture. `PRAGMA user_version = 1`; migration is `migrateSessionsTable` (additive ALTERs,
  same pattern as `migrateColumnsTable`).
- **Verification is a child process** (`--verify-session <db> [--verify-session-id N]
  [--verify-keep-regen]`, pair with `--headless`): `CLI::runSessionVerification()` builds
  the pinned composition root (benchmark pattern) and runs `Sessions::Verifier` — archive
  opened read-only; integrity re-hash; classification check; then re-parse: archived
  `project_json` into `ProjectModel`, one `FrameReader` per archived device from
  `ConnectionManager::buildFrameConfig`, chunks fed in `raw_id` order into
  `FrameBuilder::hotpathRxSourceFrame`, re-recorded by the **untouched** `Sessions::Export`
  into a temp DB (`DatabaseManager::setDbPathOverride`), with blocking `flushWorker()`
  every 4096 frames so the re-record never drops. Diff = per-uid lockstep walk ordered by
  `(timestamp_ns, reading_id)` (prefix-covered by the existing per-uid index; the integrity
  re-hash stays global `reading_id` order because it must mirror capture insertion order),
  bit-exact doubles (`std::bit_cast` compare; NaN folds to 0.0 in the digest since SQLite
  round-trips NaN as NULL), raw mismatch = parse stage,
  final-only = transform stage; virtual datasets and table-fed finals are classified, never
  compared. Verdicts: `reproduced` / `diverged` / `partial` / `not_verifiable` / `error`
  (JSON on stdout; process exit is binary 0/1). The only archive write is one appended
  `verifications` row (legacy archives get just that table created — never migrated).
- **Consumers.** `DatabaseManager::verifySession` spawns the child via QProcess (async,
  never blocks GUI); `latestVerification`/`latestVerdicts` read verdicts back (short-lived
  read-only connections); `sessions.verify` / `sessions.getVerification` are the API verbs;
  the Explorer shows a Verify action (SessionDetail) and a Verified column (SessionList).
  Count mismatches are reported as divergence annotated with the capture-time
  drop/overflow stats — never silently realigned. Known gap: `FrameConsumer::enqueueData`
  drops (consumer queue full at capture time) are invisible; only FrameReader-level
  drops/overflow are persisted.
- **Structured errors (spec 0047 addendum).** Every child-side failure carries `errorCode`
  (stable slug), `stage`, and `hint` in the report; `reparseSession()` failures are split
  (stored-project-invalid / stored-project-rejected / export-not-licensed via an explicit
  `SerialStudio::activated()` check / export-start-failed / feed-failed) and mapped to
  prose by `Verifier::reparseFailureText`. Parent-side, `childFailureReport` (file-local in
  `DatabaseManager.cpp`) synthesizes `child-spawn-failed` / `child-crashed` /
  `child-output-invalid` reports with the exit code and a stderr tail — no failure path
  concludes with an empty verdict map.

## Golden-Session Regression (spec 0047)

- **What it answers.** "Does my *edited* project still decode old telemetry the same?" —
  the inverse of 0044. `Sessions::Verifier` runs in `Mode::Regress`
  (`--regress-session <db> --regress-project <file> [--regress-session-id N]
  [--regress-keep-regen]`, `CLI::runSessionRegression()`, same bootstrap/teardown as
  verification; regression flow lives in `VerifierRegression.cpp`, a second TU of the same
  class).
- **Dual replay with injected provenance timestamps.** The archive replays twice through
  the untouched pipeline — baseline = archived `project_json`, candidate = the supplied
  file — each into its own temp re-record. Both feeds inject
  `CapturedData::timestamp = epoch + raw_index * kChunkStepNs` (1 ms; `FrameReader` spaces
  intra-chunk frames 1 ns apart), so `readings.timestamp_ns` decodes to a
  (chunk, rank) provenance key: `chunk = ns / kChunkStepNs + firstFrameChunk` (the
  first-drained-frame chunk is recorded per side). This survives frames dropped inside
  `FrameBuilder` and unarchived driver `frameStep`; ordinal pairing is explicitly
  non-conforming (one dropped frame would poison every later comparison). >10^6 frames
  from one chunk exceeds the rank budget → verdict `error`, never a silent misjoin.
  **Verify mode never injects** — its stamps, temp DB, and verdicts stay bit-identical to
  0044.
- **Diff & verdicts.** Per-uid merge-join on the provenance key across the two regen DBs:
  `structural-drift` (uid sets differ, or archived devices absent from a side's source
  list — excluded from that side's feed explicitly, never left to `buildFrameConfig()`'s
  silent `/* */` fallback) > `coverage-drift` (one-sided keys) > `value-drift` (bit-exact
  compare fails) > `identical`. Console-only archives are `not_verifiable`;
  virtual/table-fed datasets classified, never compared. **Control-script sessions compare
  normally** (unlike 0044 verify): both replays are script-free by construction —
  `ControlScript::shouldRun()` needs `setupExternalConnections()` (never wired headless)
  plus a live connection (never opened in the child), and `runRegression()` latches
  `ControlScript::shutdown()` as a hard guarantee; script-fed values ride the per-dataset
  classification. The report also carries `codeChanges` (textual control-script /
  frame-parser / transform comparison between archived and candidate projects). First
  divergences map back to the archived chunk's real `timestamp_ns`.
- **Ephemeral by contract.** Regression writes NOTHING to the archive (no verifications
  row; `appendVerificationRecord()` no-ops in Regress mode). Parent side
  (`DatabaseManager::regressSession`) serializes the current `ProjectModel` to a temp file
  when no candidate is given, shares the single child slot with verification (never
  concurrent), and publishes `regressionBusy` / `lastRegressionReport` /
  `regressionFinished` only in memory. `regressSessionsByTag` chains sessions sequentially
  (golden tag = plain session tag) and aggregates via `regressionSweepStatus()`. API:
  `sessions.regress` / `sessions.getRegression`; UI: SessionDetail "Check Project" action
  + drift panel, visually separate from the stored 0044 verdict.
