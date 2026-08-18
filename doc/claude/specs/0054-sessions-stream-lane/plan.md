---
spec: 0054-sessions-stream-lane
phase: plan
status: approved
updated: 2026-08-15
---

# Plan 0054 — Session recording for stream-lane sources

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

Stream blocks ride a **third auxiliary queue into the existing session worker** rather than
into a new sibling sink. `Sessions::ExportWorker` already owns the one database connection, the
one `session_id`, the prepared statements and the batch transaction, and its `processData()`
override already drains two non-frame queues (`writeRawBytes`, `writeTableSnapshots`) on top of
the base frame drain; a third (`writeStreamBlocks`) is the same shape as the problem `raw_bytes`
already solved — a different payload landing in the same database. Blocks persist to a new
`stream_blocks` table as a packed little-endian `float64` array plus the metadata needed to
reconstruct each sample's time, so one row covers a whole block instead of hundreds of readings
rows. Replay expands blocks back into the typed columnar path the MDF4 player already uses.
Separately, the pre-session project restore in `Sessions::Player::closeFile()` moves off the
caller's stack onto a queued invoke, with a token so a restore in flight cannot clobber a
session opened right after.

## Known sharp edges — read before implementing

The four things most likely to bite between approval and done. Each is expanded in its own
section below; they are collected here so none of them is a surprise at review time.

1. **The hotpath section's real answer is the cached flag, not the hotpath.** This design
   touches no hotpath code at all. But `refreshStreamExportFlags()` is squarely in the
   cached-flag silent-breakage class: session-enabled has to be OR'd into it **and**
   `Sessions::Export::enabledChanged` wired to it. Miss either half and the original bug comes
   back with more code behind it — recording on, no blocks built, a file that opens fine and
   contains nothing. AC1 mandates testing with every other sink disabled specifically to catch
   that. Treat these as one task, never two.

2. **Size, given full precision.** `float64` puts a 60 s 48 kHz mono capture at 23 MB of
   payload — comfortably inside AC3's 50 MB, and still ~4.6× smaller than the equivalent
   `readings` rows before indexes. The blob is explicit little-endian, not host order, so a
   database moved between machines fails loudly rather than misdecoding.

3. **The schema bump is 1 → 2, additive only** — `CREATE ... IF NOT EXISTS` throughout, no
   drops and no retypes — so v1 files open and replay unchanged. If any step of implementation
   finds itself wanting to rewrite an existing table, stop: that breaks R8/AC7.

4. **The one genuinely uncertain piece is the deferred restore racing `closeFile()` →
   `openFile()` in the same turn.** The pending-restore token is the planned defence, but it
   wants a test rather than trust — see the close/reopen case in the test plan. This is the
   only part of the design where I would not be surprised to be wrong.

## Affected subsystems & files

| File | Change |
|------|--------|
| `app/src/Sessions/Export.h` | `Export`: stream-block queue + `ingestStreamBlock` slot + `streamSink`-style accessor. `ExportWorker`: queue pointer, `writeStreamBlocks()`, `m_streamBlockQuery`, `m_streamHash` |
| `app/src/Sessions/Export.cpp` | Drain stream queue in `processData()`; insert rows; prepare the statement in `prepareHotpathQueries()`; feed the reproducibility hash; reset state in `closeResources()` |
| `app/src/Sessions/DatabaseManager.h` | `kUserVersion` 1 → 2; declare the new table creator + explorer query |
| `app/src/Sessions/DatabaseManager.cpp` | `createSchemaStreamTables()` (called from `createSchema`); per-source stream stats query for the explorer |
| `app/src/Sessions/Player.h` / `.cpp` | Defer `restorePreSessionState()` (both call sites) behind a pending-restore token; read `stream_blocks` on load |
| `app/src/Sessions/PlayerLoaderWorker.h` / `.cpp` | Decode `stream_blocks` into columnar per-channel vectors for typed replay |
| `app/src/IO/ConnectionManager.cpp` | `wireStreamWorkerSinks()`: connect `blockReady` → session sink. `refreshStreamExportFlags()`: OR in session export enabled |
| `app/src/IO/ConnectionManager.cpp` (setup) | Connect `Sessions::Export::enabledChanged` → `refreshStreamExportFlags` |
| `app/qml/DatabaseExplorer/` | Surface per-source stream sample counts and time span (R11) |
| `doc/claude/architecture/export.md` | Document the fourth queue, the new table, and the replay path |

## Architecture & data flow

Recording:

```
StreamProcessor::blockReady (stream worker thread)
  --Qt::QueuedConnection--> Sessions::Export::ingestStreamBlock (GUI thread)
  --SPSC enqueue--> ExportWorker::processData (DB worker thread)
  --> writeStreamBlocks() --> INSERT INTO stream_blocks (batched in the existing transaction)
```

The GUI-affine `ingestStreamBlock` is what preserves the single-producer invariant: every
stream worker's `blockReady` fans in through one GUI-thread slot, so the SPSC queue still has
exactly one producer no matter how many stream sources exist. This is the identical fan-in
CSV and MDF4 already use (`doc/claude/architecture/export.md`, "Typed stream sinks").

Column metadata needs no new work: the synthesized structure frame that today produces the
lone `readings` row already writes the stream datasets into `columns` (the failing capture had
`columns|1` for its one audio channel), so titles, units and widget survive unchanged.

Replay **indexes up front and fetches sample blobs on demand** — it never materializes a
session's samples in memory. `PlayerLoaderWorker` loads only block *metadata*
(`stream_block_id`, `source_id`, `unique_id`, `t0_ns`, `dt_ns`, `frames`) ordered by time; the
`samples` blob stays on disk until playback asks for it, one block at a time, through a
prepared select by rowid.

**Decoded blocks re-enter through the stream lane, not the frame lane** (revised 2026-08-15
after the first implementation replayed nothing). The frame-lane route was wrong twice over:
`FrameBuilder::replayChannelsTyped` asserts `m_operationMode == ProjectFile` and early-returns
on `m_frame.groups.empty()`, so in QuickPlot — where the structure lives in `m_quickPlotFrame` —
every call is a silent no-op; and even in ProjectFile it would mean one synthesized frame **per
sample**, 48,000/s for audio, which is precisely the cost the stream lane exists to avoid.

Instead the player pushes each decoded block as an `IO::SampleBlock` into a replay-owned
`IO::StreamWorker`. Everything downstream is then identical to a live capture: the processor
runs the transforms, envelope and FFT reduction, publishes one bounded display update per
block, and `Dashboard::drainStreamWorkers()` — which iterates `ConnectionManager::streamWorkers()`
— picks it up with no knowledge that the source is a recording. Widgets decimate at draw time
as they already do. No frames are synthesized, nothing is decimated on the way in, and full
fidelity is preserved end to end. Pacing falls out of the player's existing clock, which steps
over block start times at block rate.

This mirrors spec 0022, where the CSV player mmaps and indexes rather than materializing and
the MDF4 player decodes columnar data on its worker. Loading whole sessions would have put a
hard ceiling on recording length that nothing in the spec warns about: at the AC3 measurement
of ~23 MB per minute per channel, an hour-long capture is ~1.4 GB resident. The index is tens
of bytes per block, so it stays bounded no matter how long the session runs.

**Sustained-load target:** 120 Hz continuous logging with every exporter enabled at once. The
design keeps that stable by construction rather than by tuning — one row per block (not per
sample) bounds insert count, the existing batch transaction bounds commit frequency, replay
holds one decoded block at a time, and nothing here adds a per-sample cross-thread hop. If a
future change needs a rate cap or a decimation step to hold this target, that is a signal the
per-block invariant was broken somewhere upstream, not a licence to add one.

Teardown: `closeFile()` stops posting the restore inline. It sets a pending token and posts
`restorePreSessionState()` via `QMetaObject::invokeMethod(this, ..., Qt::QueuedConnection)`;
`openFile()` clears the token so a restore queued by a previous close cannot land on top of a
freshly opened session. Posting to `this` means Qt drops the call if the player dies first.

## Hotpath & threading impact

- **Touches the hotpath?** **No.** Nothing in `FrameReader`, `CircularBuffer`, the span fast
  lane, or the Dashboard draw path changes. The stream lane is already off the frame hotpath by
  construction; this adds a consumer at its block-rate output. `--benchmark-hotpath` is still
  run (AC10) to prove the frame lane did not regress, not because this design touches it.
- **New cross-thread signal/slot?** **Yes** — one: `StreamProcessor::blockReady` →
  `Sessions::Export::ingestStreamBlock`, `Qt::QueuedConnection`, block rate (~100/s), never
  per sample. Identical in type and cadence to the existing CSV/MDF4/API connections made in
  the same function. From there the payload crosses to the DB worker through the existing
  lock-free SPSC queue, not a signal.
- **New input to a cached hotpath flag?** **Yes — and this is the requirement most likely to
  be silently missed.** `refreshStreamExportFlags()` computes whether stream workers build
  export payloads at all; it currently ORs CSV, the API subscription, MDF4 and audio recording.
  Session export must be OR'd in **and** `Sessions::Export::enabledChanged` must be connected
  to `refreshStreamExportFlags` alongside the existing four. Miss either half and R4 fails
  exactly as the original bug did: recording enabled, no blocks produced, an empty file that
  looks valid. This is the `common-mistakes.md` cached-flag class.
- **Timestamp ownership.** The source stamps at the driver boundary. Blocks carry `t0` and
  `dt` from `StreamBlockItem`; the writer stores them relative to the existing session
  `m_steadyBaseline` and derives per-sample times as `t0 + i * dt`. Nothing re-stamps —
  `monotonicFrameNs` is not used on this path.

## Data model & persistence

New table, created by `createSchemaStreamTables()` from the existing `createSchema()`:

```sql
CREATE TABLE IF NOT EXISTS stream_blocks (
  stream_block_id INTEGER PRIMARY KEY AUTOINCREMENT,
  session_id      INTEGER NOT NULL,
  source_id       INTEGER NOT NULL,
  unique_id       INTEGER NOT NULL,
  block_number    INTEGER NOT NULL,
  t0_ns           INTEGER NOT NULL,
  dt_ns           INTEGER NOT NULL,
  frames          INTEGER NOT NULL,
  samples         BLOB NOT NULL
);
CREATE INDEX IF NOT EXISTS idx_stream_blocks_session_uid_t0
  ON stream_blocks (session_id, unique_id, t0_ns);
```

`samples` is `frames * 8` bytes: IEEE-754 `float64`, **little-endian, written explicitly**
rather than in host order, because session files move between machines. `float64` is the
spec's full-internal-precision constraint — `StreamBlockItem::channels` is
`std::vector<std::vector<double>>`, so a recorded sample and the pipeline's value compare
equal, which is what lets AC2 and AC5 demand exact equality with no tolerance.

Size check against AC3: 48 kHz × 60 s × 8 B = 23 MB of sample payload, plus roughly one row of
metadata per ~10 ms block. Comfortably under the 50 MB budget, and ~4.6× smaller than the
equivalent `readings` rows would be before indexes.

`kUserVersion` goes 1 → 2. Migration is additive only: every statement is
`CREATE TABLE/INDEX IF NOT EXISTS`, no column is dropped or retyped, so a v1 database opens,
gains the empty table, and replays exactly as before (R8/AC7). Nothing reads `user_version` to
gate behavior; it is a marker.

Reproducibility (R6): `ExportWorker` hashes each readings row into `m_readingsHash` via
`hashReadingRow`. Stream blocks get the analogous `hashStreamBlock` contribution — block
metadata plus the sample bytes — folded into a `m_streamHash` recorded alongside the existing
raw and readings digests, so a stream session verifies on content rather than trivially
verifying because it contained nothing.

## API / SDK surface

None. No new API handler, no `EnumLabels` entry, no SDK change. Existing session commands keep
working unchanged; nothing about the session command surface is rate- or payload-dependent.

## QML / UI

`app/qml/DatabaseExplorer/` gains a read-only presentation of stream content per source:
sample count and time span, fed by a new `DatabaseManager` aggregate query
(`SUM(frames)`, `MIN(t0_ns)`, `MAX(t0_ns + frames * dt_ns)` grouped by `source_id`,
`unique_id`). Deliberately no waveform preview and no in-explorer plotting — the spec scopes
this to "confirm the capture worked without replaying it" (R11), which counts and a span
satisfy.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Where the sink lives | Sibling `Sessions::StreamExport` with its own worker (mirrors CSV/MDF4) · fourth queue on the existing worker · expand blocks to frames through the existing frame queue | **Fourth queue.** The sibling's independence buys nothing when both writers target one file — it would mean two SQLite connections, a cross-worker `session_id` handoff and two transactions contending under WAL. Blocks-to-frames needs no schema at all and satisfies R1/R3/R5 trivially, which is exactly why it is tempting, but it re-creates the 48k-frames/s per-sample cost the stream lane exists to remove and violates the per-block invariant. |
| Sample encoding | `float64` · `float32` · text | **`float64`.** The maintainer's explicit call: precision is the reason export exists. `float32` would halve the file and match what the API stream sends, but forces AC2/AC5 onto a tolerance instead of exact equality. |
| Byte order in the blob | Host order · explicit little-endian | **Explicit LE.** Session databases are moved between machines; host order would make a file silently misdecode rather than fail loudly. |
| Replay mechanism | New block-replay path · reuse `replayChannelsTyped` | **Reuse.** The MDF4 player already decodes columnar per-channel doubles on a loader thread and injects through this entry point; a second mechanism would duplicate the generation-stamp/cancel-join machinery that path already gets right. |
| Replay memory model | Materialize all samples at open · index up front, blobs on demand | **Index + on-demand.** Materializing is simpler but caps session length invisibly (~1.4 GB/hour/channel at the AC3 rate). On-demand matches how the CSV and MDF4 players already behave (spec 0022) and keeps memory flat regardless of recording length, which is what the 120 Hz continuous-logging target requires. |
| Replay lane | Frame lane (`replayChannelsTyped`) · stream lane (feed a replay `StreamWorker`) | **Stream lane.** The frame route is a no-op in QuickPlot (asserts ProjectFile, early-returns on an empty `m_frame`) and costs one synthesized frame per sample — 48k/s — everywhere else. Feeding a replay-owned `StreamWorker` reuses the entire live display path unchanged, so recorded data renders exactly as captured with no decimation on the way in. |
| Teardown fix | Defer the restore · skip it when closing · stop reloading the project | **Defer** (the maintainer's call in the spec). Smallest change that removes the entire re-entrancy class rather than the one observed path. |

## Risks & mitigations

- **The cached-flag miss (highest risk).** Adding the sink but not wiring session-enabled into
  `refreshStreamExportFlags` reproduces the original bug with more code. AC1 mandates testing
  with every other sink disabled, which is the only configuration that catches it.
- **Deferred restore racing a new session.** `closeFile()` immediately followed by `openFile()`
  could let the queued restore land after the new session opened. Mitigated by the pending-
  restore token cleared in `openFile()`; needs a test that closes and reopens in one turn.
- **Blob decode on a truncated or foreign file.** A `samples` blob whose length is not
  `frames * 8` must be rejected at load with a warning, not decoded past its end.
- **Session size on long captures.** 23 MB/minute/channel is bounded and documented, but a
  multi-hour capture is large *on disk*. Out of scope per the spec's non-goals (no retention
  policy); worth stating in the docs so it is a known property rather than a surprise. Memory
  is not exposed to this, because replay indexes rather than materializes.
- **A pure-stream session must not be rejected as empty.** `openAndLoad` currently fails a load
  outright when the timestamp index is empty; that guard has to become "no timestamps **and**
  no stream blocks", or a session whose data is entirely stream-lane refuses to open.
- **Per-block disk reads during playback.** On-demand fetching trades resident memory for a
  read per block. Bounded and sequential (blocks replay in time order, SQLite page cache
  covers the locality), but it is the cost being accepted for flat memory.
- **WAL growth under large blobs.** The existing batch transaction already bounds commit
  frequency; block rate (~100/s) is far below the frame rates this worker already handles.
- **`Sessions/Export.cpp` and `Player.cpp` are large TUs.** Keep additions in new
  concern-grouped functions rather than growing existing ones past the style limits.

## Test & verification plan

- **Unit (I can run):** none applicable — `tests/scripts/` covers JS frame parsers, which this
  does not touch.
- **C++ units (`ctest`, maintainer builds / I can run against an existing build dir):**
  - Blob round-trip: pack a known `double` array, read it back, assert bit-exact equality and
    correct per-sample timestamps.
  - Truncated / non-multiple-of-8 blob is rejected with a warning, not decoded past its end.
  - Byte order: a blob written on this machine decodes identically when read as explicit LE,
    guarding sharp edge 2.
  - **Close/reopen race (sharp edge 4):** `closeFile()` immediately followed by `openFile()` in
    the same turn must leave the newly opened session intact — the deferred restore must not
    land on top of it. This is the test that decides whether the pending-restore token is
    actually sufficient; if it fails, the token design is wrong, not the test.
  - Schema migration: open a fixture v1 database, assert it gains the empty table, still
    replays, and that no existing table was rewritten (sharp edge 3).
- **Integration (maintainer runs the app; I can drive it):** extend `tests/integration/` with a
  session-stream case driven over the API at `localhost:7777` —
  - AC1/AC4: enable session recording only, connect audio, record, assert block coverage.
  - AC2: simultaneous CSV export, compare every sample exactly.
  - AC3: assert the resulting database is under 50 MB.
  - AC5: run the reproducibility verifier, assert a match.
  - AC6/AC7: record/replay a frame-lane session and open a pre-existing v1 database.
  - AC12: query the explorer aggregate, assert counts and span without replaying.
  - AC13: mixed frame + stream session, assert session-relative times agree.
- **Maintainer observation:** AC8 (close the window 20 times with a player open, across fresh
  launches — this is the crash that motivated R9) and AC9 (project/mode restored via both close
  paths).
- **Hotpath:** `--benchmark-hotpath` at default thresholds for AC10 — proving no frame-lane
  regression, since this design does not touch it.
- **Static:** `python scripts/code-verify.py --check` on every touched file; `qt-cpp-review`
  before handoff; `python scripts/sanitize-commit.py` before commit.
