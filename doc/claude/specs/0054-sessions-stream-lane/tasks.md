---
spec: 0054-sessions-stream-lane
phase: tasks
status: approved
updated: 2026-08-16
---

# Tasks 0054 — Session recording for stream-lane sources

> **Phase 3 of 4 — the ordered checklist.** Decompose [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable*. Gate: do not start `/ss-implement` until
> a human marks this `approved`.

## Conventions

- One task = one focused, reviewable change.
- **Verify** is how *this* unit is confirmed before moving on.
- **Deps** lists task IDs that must land first.
- T1 is deliberately first and stands alone: it fixes a shipped crash and shares no code with
  the recording work, so it can be reviewed and landed without waiting for the rest.

## Tasks

### T1 — Defer the pre-session project restore off the teardown stack

- **Files:** `app/src/Sessions/Player.h`, `app/src/Sessions/Player.cpp`
- **Does:** Replace the two inline `restorePreSessionState()` calls (`closeFile()` and the
  load-failure path) with a queued invoke on `this`, guarded by a pending-restore token that
  `openFile()` clears. **Binding invariant: no nested event loop may run while a window is
  being destroyed** — the restore reloads the project, which rebuilds devices and waits on the
  pipeline; running that inline from a QML handler fired during `QWindowPrivate::destroy()` is
  what re-entered AppKit and crashed. Posting to `this` also lets Qt drop the call if the
  player dies first.
- **Verify:** `python scripts/code-verify.py --check app/src/Sessions/Player.h
  app/src/Sessions/Player.cpp`; maintainer closes the window with a player open (AC8) and
  confirms project + operation mode restored via both close paths (AC9).
- **Deps:** none
- [x] done

### T2 — Add the `stream_blocks` table and bump the schema version

- **Files:** `app/src/Sessions/DatabaseManager.h`, `app/src/Sessions/DatabaseManager.cpp`
- **Does:** Add `createSchemaStreamTables()` (called from `createSchema()`) creating
  `stream_blocks` and its `(session_id, unique_id, t0_ns)` index, and bump `kUserVersion`
  1 → 2. **Binding invariant: additive migration only** — `CREATE ... IF NOT EXISTS`
  throughout, no table dropped or retyped, so a v1 database opens, gains the empty table, and
  replays unchanged (R8/AC7).
- **Verify:** `python scripts/code-verify.py --check` on both files; open a pre-existing v1
  session database and confirm it still opens and lists its session.
- **Deps:** none
- [x] done

### T3 — Persist stream blocks through a third auxiliary queue

- **Files:** `app/src/Sessions/Export.h`, `app/src/Sessions/Export.cpp`
- **Does:** Add the stream-block SPSC queue, the GUI-affine `ingestStreamBlock` slot, the
  prepared insert in `prepareHotpathQueries()`, and `writeStreamBlocks()` drained from
  `processData()` alongside `writeRawBytes()` / `writeTableSnapshots()`. Samples are packed
  **explicit little-endian `float64`**, `frames * 8` bytes. **Binding invariants: the
  single-producer SPSC rule** — every stream worker's `blockReady` fans in through the one
  GUI-thread slot, so the queue keeps exactly one producer regardless of source count — and
  **source owns time**: store `t0`/`dt` from the block relative to the existing session
  baseline and derive per-sample times as `t0 + i * dt`; never re-stamp, never call
  `monotonicFrameNs` here.
- **Verify:** `python scripts/code-verify.py --check` on both files; read back that a recorded
  block's blob length equals `frames * 8`.
- **Deps:** T2
- [x] done

### T4 — Fold stream blocks into the reproducibility digest

- **Files:** `app/src/Sessions/Export.h`, `app/src/Sessions/Export.cpp`,
  `app/src/Sessions/DatabaseManager.cpp`
- **Does:** Add `hashStreamBlock` (block metadata + sample bytes) and an `m_streamHash`
  recorded alongside the existing raw and readings digests, reset in `closeResources()` with
  them. Without this a stream session verifies trivially — the exact failure mode the spec
  exists to remove. The digest needs a `stream_sha256` column on `sessions`, added as one entry
  in the existing additive `migrateSessionsTable` column list (scope note: this is the one line
  of T4 that lands outside its originally listed files, still inside the plan's file list).
- **Verify:** `python scripts/code-verify.py --check` on all three files; two identical recorded
  sessions produce equal digests, and a session with one altered sample does not.
- **Deps:** T3
- [x] done

### T5 — Wire the session sink into the stream fan-out *and* the export-active gate

- **Files:** `app/src/IO/ConnectionManager.cpp`
- **Does:** In `wireStreamWorkerSinks()` connect `StreamProcessor::blockReady` →
  `Sessions::Export::ingestStreamBlock` (`Qt::QueuedConnection`, block rate); in
  `refreshStreamExportFlags()` OR in session-export-enabled; and connect
  `Sessions::Export::enabledChanged` → `refreshStreamExportFlags` beside the existing four.
  **Binding invariant: cached hotpath flag — these three edits are ONE task and must never be
  split.** `refreshStreamExportFlags()` decides whether stream workers build export payloads
  at all; connect the sink without feeding the gate and blocks are never produced, feed the
  gate without the change signal and it never re-evaluates. Either half alone reproduces the
  original bug behind more code.
- **Verify:** `python scripts/code-verify.py --check app/src/IO/ConnectionManager.cpp`; with
  CSV, MDF4 and API export all **disabled** and only session recording on, confirm
  `stream.getSources` shows blocks processed and rows land in `stream_blocks` (AC1).
- **Deps:** T3
- [x] done

### T6 — Decode `stream_blocks` on the session loader thread

- **Files:** `app/src/Sessions/PlayerLoaderWorker.h`, `app/src/Sessions/PlayerLoaderWorker.cpp`
- **Does:** Load the session's stream-block **index only** — `stream_block_id`, `source_id`,
  `unique_id`, `t0_ns`, `dt_ns`, `frames`, ordered by time — leaving `samples` blobs on disk.
  **Binding invariant: never materialize a session's samples**; at ~23 MB/minute/channel an
  hour-long capture would be ~1.4 GB resident, and the 120 Hz continuous target requires memory
  flat in session length. Also fix the empty-session guard in `openAndLoad` to "no timestamps
  **and** no stream blocks", or a pure-stream session refuses to open.
- **Verify:** `python scripts/code-verify.py --check` on both files; open a recorded session and
  assert the index block count matches `SELECT COUNT(*) FROM stream_blocks` with no sample
  memory held.
- **Deps:** T3
- [x] done

### T7 — Fetch stream blobs on demand (superseded in part by T14)

- **Files:** `app/src/Sessions/Player.h`, `app/src/Sessions/Player.cpp`
- **Does:** Fetch one block's `samples` blob on demand by rowid through a prepared select,
  decode it little-endian, and inject via `FrameBuilder::replayChannelsTyped` at block cadence —
  the typed path. **Superseded by T14:** that injection proved to be a no-op in QuickPlot, so
  the decoded block now enters through the stream lane instead; the on-demand fetch, the
  one-block-at-a-time rule and the blob validation below all still stand. Hold
  **one decoded block at a time**; reject a blob whose length is not `frames * 8` with a warning
  rather than decoding past its end. Sessions with no stream blocks keep their current behavior
  exactly (R7).
- **Verify:** `python scripts/code-verify.py --check` on both files; replay a recorded audio
  session and confirm the dashboard plots the waveform for the full capture duration (AC4).
- **Deps:** T1, T6
- [x] done

### T8 — Per-source stream stats query for the explorer

- **Files:** `app/src/Sessions/DatabaseWorker.h`, `app/src/Sessions/DatabaseWorker.cpp`,
  `app/src/Sessions/DatabaseManager.h`, `app/src/Sessions/DatabaseManager.cpp`
- **Does:** Add the aggregate used by R11 — `SUM(frames)`, `MIN(t0_ns)`,
  `MAX(t0_ns + frames * dt_ns)` grouped by `source_id`, `unique_id` — returning sample count
  and time span per stream source. **Scope note:** the query lives in `DatabaseWorker`, not
  `DatabaseManager` as originally listed, because session DB access must not run on the GUI
  thread; the manager only forwards the request and re-emits the result.
- **Verify:** `python scripts/code-verify.py --check` on all four files; run the query against a
  recorded session and check the span matches the capture duration.
- **Deps:** T3
- [x] done

### T9 — Surface stream content in the database explorer

- **Files:** `app/qml/DatabaseExplorer/`
- **Does:** Show sample count and time span per stream source so a capture can be confirmed
  without replaying it. Counts and span only — no waveform preview, no in-explorer plotting
  (explicit spec non-goal).
- **Verify:** `python scripts/code-verify.py --check` on changed QML; maintainer opens a
  recorded session in the explorer and sees the stream source listed with its counts (AC12).
- **Deps:** T8
- [x] done

### T10 — C++ unit coverage

- **Files:** `app/src/Sessions/StreamBlockCodec.h`, `app/tests/tst_stream_block_codec.cpp`,
  `app/tests/CMakeLists.txt`
- **Does:** Covers the wire format: bit-exact round-trip (including infinities and NaN, which
  must not be canonicalised), `frames * 8` blob length, explicit little-endian encoding, and
  rejection of truncated / misaligned / frame-count-mismatched blobs. **Scope note:** the codec
  was extracted to a header so it is reachable from a test at all — the encoder previously lived
  in `Export.cpp` and the decoder in `Player.cpp` with no shared definition, which is precisely
  the pair that drifts. **Deferred to T11 / maintainer:** the v1 schema migration and the
  close/reopen race need a live database and the composition root, so they cannot be unit tests
  without linking most of the app; they move to the integration tier and the AC8/AC9 checks.
- **Verify:** `ctest -R tst_stream_block_codec` against an existing build directory (maintainer
  builds first); `python scripts/code-verify.py --check` on all three files.
- **Deps:** T1, T2, T3, T6
- [x] done

### T11 — Integration coverage over the API

- **Files:** `tests/integration/`
- **Does:** Add the session-stream cases driven over `localhost:7777`: AC1 (record with every
  other sink disabled), AC2 (simultaneous CSV, exact per-sample equality — no tolerance), AC3
  (database under 50 MB), AC5 (verifier reports a match), AC6 (frame-lane session unchanged),
  AC13 (mixed frame + stream session, session-relative times agree).
  Adds `tests/integration/test_session_stream_lane.py`, plus AC7 (a synthesized pre-0054
  database still opens) which moved here from T10. Audio cases self-skip without an input
  device and deliberately need no signal — silence still produces samples, and every assertion
  is about counts, shape and exactness rather than content.
- **Verify:** `pytest tests/integration/test_session_stream_lane.py -v` with the app running and
  the API server enabled.
- **Deps:** T5, T7
- [x] done

### T12 — Document the fourth queue and the replay path

- **Files:** `doc/claude/architecture/export.md`
- **Does:** Record that Sessions takes stream blocks on a third auxiliary queue into the
  existing worker (and why, versus the sibling-sink shape CSV/MDF4 use), the `stream_blocks`
  table and its LE `float64` blob, the `user_version` 2 bump, and the size property
  (~23 MB/minute/channel) so it is known rather than discovered.
- **Verify:** `python scripts/code-verify.py --check doc/claude/architecture/export.md`
- **Deps:** T3, T7
- [x] done

### T13 — Replay-owned stream workers

- **Files:** `app/src/IO/ConnectionManager.h`, `app/src/IO/ConnectionManager.cpp`
- **Does:** Let a player create and tear down `IO::StreamWorker`s for a recording's stream
  sources (source id, channel count, sample rate, dataset configs derived from the session's
  columns) alongside the device-built ones, so `Dashboard::drainStreamWorkers()` picks them up
  with no change. **Binding invariants:** workers join before teardown exactly as the
  device-built ones do; replay workers must never be fed by `refreshStreamExportFlags()` into
  re-exporting what is already recorded.
  Adds `app/src/IO/ReplayStreamSource.h`, a stand-in driver so `StreamWorker` binds to a real
  signal source (its ctor asserts a non-null driver) with **zero changes to StreamWorker**.
  Export sinks are deliberately not wired for replay workers: a replayed session must not be
  re-recorded into a new one.
- **Verify:** `python scripts/code-verify.py --check` on both files; a replayed session shows a
  live stream source in `stream.getSources`.
- **Deps:** T7
- [x] done

### T14 — Sessions player feeds blocks into the stream lane

- **Files:** `app/src/Sessions/Player.h`, `app/src/Sessions/Player.cpp`
- **Does:** Replace the `replayChannelsTyped` injection with pushing each decoded block as an
  `IO::SampleBlock` to the replay worker's `StreamProcessor::onSampleBlock`. **Binding
  invariant: source owns time** — the block carries its recorded `t0`/`dt`; the player paces,
  it does not re-stamp. Drop the now-dead typed-cell path.
  `Dashboard::streamAvailable()` needs no new input: `Sessions::Player::isOpen()` is already
  one of its wired inputs, so the cached flag lights up when the session opens.
- **Verify:** replay an audio QuickPlot session; the waveform renders for the full capture
  duration at the recorded rate (AC4).
- **Deps:** T13
- [x] done

### T15 — CSV and MDF4 players use the same lane

- **Files:** `app/src/CSV/Player.cpp`, `app/src/MDF4/Player.cpp`
- **Does:** Route their stream-lane data through the same replay worker path rather than
  per-row frame injection, so all three players behave identically. Frame-lane content in those
  files keeps its current path untouched.
- **Verify:** replay a `*_stream_source*.csv` and a stream `.mf4`; both render as the session
  player does.
- **Deps:** T14
- **Status:** DEFERRED out of spec 0054 by the maintainer (2026-08-16). The CSV and MDF4
  players replay their own file formats, which is separate surface from the session database;
  it gets its own spec alongside the wider "stream sources only" direction.
- [ ] done (deferred)

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` is met and checked off there.
- [ ] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [ ] `python scripts/code-verify.py --singleton-census --check` shows no growth.
- [ ] `qt-cpp-review` run on the C++ diff; findings addressed or noted.
- [ ] `--benchmark-hotpath` passes every gated tier at default thresholds (AC10) — this design
      touches no hotpath code, so any movement is a signal to stop and investigate.
- [ ] `pytest tests/integration/ -v` and the session suites identified for the maintainer.
- [ ] `python scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [ ] Diff is *what was asked, and only that* — in particular `app/src/IO/StreamWorker.{h,cpp}`
      is **not** touched (concurrent display-path work lives there).
- [ ] `spec.md` status set to `done`.
