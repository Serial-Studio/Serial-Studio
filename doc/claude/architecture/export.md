# Architecture — Export & Sessions DB

> Part of the architecture corpus ([index](../architecture.md)). Read this file in full
> before touching CSV/MDF4 export, the Sessions database, or replay.

## Export Architecture & Sessions DB (Pro)

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
- **Typed stream sinks (spec 0051 M5)**: a stream-lane source never reaches the frame
  exporters. `CSV::StreamExport` / `MDF4::StreamExport` (each a `FrameConsumer<StreamBlockItemPtr>`
  with its own worker thread) write **one file per stream source**
  (`*_stream_sourceN.csv` / `.mf4`) carrying the **full-rate post-transform** samples, with
  per-sample times derived as `t0 + i * dt` — never the `monotonicFrameNs` bump, which exists
  only to break same-ns collisions on the frame lane. Their enable state follows the parent
  exporter (`setExportEnabled`), they close with it, and they stop in
  `stopFrameConsumerWorkers()`. The single-producer invariant is kept by fan-in: every
  `StreamProcessor::blockReady` is queued to the GUI-affine `ingestBlock`, so each sink's SPSC
  queue still has exactly one producer no matter how many stream workers exist. Block payloads
  are only built while some sink (CSV, MDF4 or an API subscriber) is live.
- **Session DB lives in `app/src/Sessions/`** (NOT `app/src/SQLite/`):
  - `Sessions::DatabaseManager` — singleton owning the open `.db`; backs `app/qml/DatabaseExplorer/`.
  - `Sessions::Export` (`Sessions/Export.h/.cpp`): `FrameConsumer`-based; tables
    `sessions/columns/readings/raw_bytes/table_snapshots`; second lock-free queue for raw
    bytes via `ConnectionManager::onRawDataReceived`. WAL mode, batch transactions.
  - `Sessions::Player`: replays a stored session through the FrameBuilder pipeline using the
    **final** (post-transform) reading columns, with a uid->cell replay column map installed via
    `FrameBuilder::setReplayColumnMap` (same mechanism as MDF4). **All three players count as
    final-value players** (`SerialStudio::isFinalValuePlayerOpen`), so per-dataset transforms
    never re-run during playback — they read live inputs (data tables) that don't exist then.
    Raw columns are only a fallback for pre-final-column session files.
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
    `publishReplayFrame`: dashboard + read-only observers (API/gRPC, only with a client
    connected) and **never a recording sink** (CSV/MDF4/Sessions export, MQTT) — replay
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

## Reproducibility Verification (spec 0044)

- **Capture-side fingerprints.** `ExportWorker` keeps two incremental SHA-256 hashes on the
  worker thread: raw chunks (in `writeRawBytes`, insertion = `raw_id` order) and readings
  rows (in `bindAndInsertReading`), over the canonical byte layout in
  `Sessions::hashRawChunk` / `hashReadingRow` (`Export.h` free functions — the verifier
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
