---
spec: 0044-session-reproducibility
phase: plan
status: approved     # draft -> approved (gate before /ss-tasks)
updated: 2026-08-05
---

# Plan 0044 — Session Reproducibility Verification

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Read the relevant `doc/claude/` sub-docs and the *actual code*
> before writing this — a plan grounded in a stale mental model is worse than no plan.
> Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

Verification runs as a **child process of the app's own binary** (`--verify-session`), the
same headless pattern `--benchmark-hotpath` already uses (`Misc/CLI.cpp` builds the pinned
composition root via `ModuleManager::instantiateCoreModules()`, then drives the real
pipeline in-process). The verifier loads the archived `project_json` through
`ProjectModel::loadFromJsonDocument`, builds one `IO::FrameReader` per archived device from
the production `FrameConfig`, feeds the archived `raw_bytes` chunks in `raw_id` order, and
lets the **unmodified** `Sessions::Export` sink re-record the regenerated session into a
temporary database — so the regenerated readings are written by the byte-identical
production path. The verdict is then a SQL sequence-diff (per `unique_id`, ordered by
`reading_id`) between archived and regenerated `readings`, with stage attribution (raw
columns mismatch = parse, final-only mismatch = transform). Capture side gains
worker-thread SHA-256 fingerprints, version stamps, and a reproducibility classification
written at session finalize; the archived DB gains an append-only `verifications` table.
Zero frame-path code changes; live-app isolation by construction (R1/AC6).

## Affected subsystems & files

| File | Change |
|------|--------|
| `app/src/Sessions/Export.h/.cpp` | ExportWorker: incremental SHA-256 over raw chunks (`writeRawBytes`) and readings rows (`bindAndInsertReading`), canonical byte serialization; finalize writes fingerprints + `app_version` + `capture_format` + repro classification + drop/overflow stats into the session row. Capture-time classification inputs gathered on the main thread at session open. |
| `app/src/Sessions/DatabaseManager.h/.cpp` | Additive schema migration (new `sessions` columns, `verifications` table, `PRAGMA user_version` stamp) following the existing `migrateColumnsTable` pattern; DB-path override hook for the verifier's temp target; read APIs for latest verdict (feeds Explorer model); `verifySession(sessionId)` QProcess launcher + result parsing. |
| `app/src/Sessions/Verifier.h/.cpp` (new) | The verification engine run inside the child process: open archive read-only, integrity re-hash, classification check, re-parse drive (readers + feed loop + export re-record), SQL diff, verdict JSON on stdout, append `verifications` row. Commercial license header. |
| `app/src/Misc/CLI.h/.cpp` | New options `--verify-session <db>`, `--verify-session-id <n>`, `--verify-keep-regen`; `runSessionVerification()` mirroring `runHotpathBenchmark()` (composition root, offscreen platform, exit code = verdict). |
| `app/src/IO/ConnectionManager.h` | Expose the existing `buildFrameConfig(int)` result for the verifier (public accessor or free helper — no logic change), so reader config is never re-derived. |
| `app/src/API/Handlers/SessionsHandler.h/.cpp` | `sessions.verify` verb (start + async completion per existing handler conventions), returning the same verdict JSON as the CLI. |
| `app/qml/DatabaseExplorer/SessionDetail.qml`, `SessionList.qml` | "Verify reproducibility" action; verdict badge (reproduced / diverged / not verifiable / never verified) + divergence detail display. |
| `tests/integration/test_session_verification.py` (new) | AC1–AC6 pytest coverage (needs running app + API server). |
| `tests/fixtures/sessions/` (new) | Checked-in pre-0044 legacy fixture DB (AC5) + tamper-copy helpers. |
| `doc/claude/architecture/export.md` | New "Reproducibility Verification" section; CLAUDE.md gets a one-line pointer only if review deems it architectural enough. |

## Architecture & data flow

**Capture (existing threads, no new ones).** `ExportWorker` (worker thread) already sees
every raw chunk (`writeRawBytes`) and every readings row (`bindAndInsertReading`). Two
incremental `QCryptographicHash` (SHA-256) instances live in the worker: raw hash updates
per chunk in `raw_id` order (per-device interleave is fixed by insertion order, which the
hash follows); readings hash updates per row over a canonical byte layout — `unique_id`
(LE64), raw/final doubles as IEEE-754 bit patterns (LE64), raw/final strings as UTF-8
length-prefixed, `is_numeric` byte. `finalizeSession()` writes both digests plus
`app_version` (`APP_VERSION`), `capture_format` (new `kCaptureFormatVersion = 1`),
`repro_class` (JSON), and drop/overflow counters into the session row. Classification
inputs are snapshotted on the **main thread** when recording starts and handed to the
worker with the existing project-snapshot mutex pattern: control script running,
per-dataset transforms present, table-capture flag (`FrameBuilder` dataset capture) on,
per-dataset `is_virtual` (already in `columns`).

**Verification (child process).** Parent (`DatabaseManager::verifySession`) spawns
`QCoreApplication::applicationFilePath()` with `--verify-session <db> --verify-session-id
<n>`, reads stdout JSON. Child sequence:

1. CLI parses → offscreen platform → `instantiateCoreModules()` (benchmark precedent;
   licensing block first per spec 0042, so Pro entitlement resolves before anything reads it).
2. `Sessions::Verifier` opens the archive read-only: session row, `project_json`, `columns`
   (uid → source_id, is_virtual), fingerprints, classification.
3. **Integrity stage**: re-hash `raw_bytes` and `readings` with the same canonical code;
   mismatch vs stored digests → divergence attributed to *archive modification* (AC2).
   Legacy sessions (NULL digests) skip this stage and qualify the verdict.
4. **Classification stage**: `repro_class` says not mechanically verifiable → emit that
   verdict per Q1 (per-dataset roll-up: self-contained datasets still verify; virtual/
   table-fed ones report classification).
5. **Re-parse stage**: `ProjectModel::loadFromJsonDocument(project_json)`;
   `Sessions::Export::setSettingsPersistent(false)` + `setExportEnabled(true)` + temp-DB
   path override; per archived device one `IO::FrameReader` configured from
   `ConnectionManager::buildFrameConfig(deviceId)`; feed chunks in `raw_id` order
   (`IO::makeCapturedData`), drain each reader's queue into
   `FrameBuilder::hotpathRxFrame` / `hotpathRxSourceFrame(deviceId, ...)` exactly as
   `ConnectionManager::onFrameReady` routes per operation mode. Transforms compile from the
   loaded project as in a live session (no player open, dashboard drops frames headless —
   benchmark precedent).
6. Close the regenerated session (worker finalize), then **diff**: for each `unique_id`,
   `ROW_NUMBER() OVER (ORDER BY reading_id)` sequences from both DBs joined on
   (uid, seq); compare raw_numeric/raw_string and final_numeric/final_string exactly
   (SQLite REAL stores the IEEE double bit-exactly). Collect per-dataset mismatch counts +
   first mismatch (recorded ts, recorded vs regenerated values) + stage attribution (R4).
   Frame-count mismatch per uid short-circuits to a count-mismatch divergence annotated
   with the archived drop/overflow stats (see Risks).
7. Verdict JSON to stdout; append a `verifications` row into the archived DB (WAL +
   `busy_timeout` already standard); delete the temp regenerated DB unless
   `--verify-keep-regen`. Process exit code is binary (0 = reproduced, nonzero = anything
   else) because `CLI::ProcessResult` is binary and widening it would touch `main.cpp`;
   the fine-grained verdict (`reproduced` / `diverged` / `partial` / `not_verifiable` /
   `error`) lives in the JSON, which is what tests and lab CI consume. (Amended during
   /ss-implement, 2026-08-05.)

**ConsoleOnly sessions** have no interpretation (synthesized `project_json` is a stub, no
readings schema semantics): verification = raw-integrity stage only, verdict qualified.
**QuickPlot sessions** re-parse through their synthesized EndDelimiter/PlainText project.

## Hotpath & threading impact

- **Touches the hotpath?** **No.** No edits to `FrameReader`, `CircularBuffer`,
  `FrameBuilder`, `Dashboard`, or the span lane. The verifier *calls* the existing public
  entry points from its own process, same as `HotpathBenchmark`. Capture-side hashing runs
  entirely on the ExportWorker thread inside the existing batched-write loops (per-row cost
  is small next to the SQLite insert it accompanies; the exporter phase of the benchmark is
  an ungated readout, and AC8 confirms the gated runs are untouched).
- **New cross-thread signal/slot?** No new frame-path connections. One new worker→controller
  hand-off for classification/fingerprint metadata rides the existing project-snapshot
  mutex + queued-invoke pattern already used by `setSnapshotIntervalMs`/`m_projectSnapshot`.
- **New input to a cached hotpath flag?** None. Verification never flips `m_anyAsyncSink`
  in the live app; in the child process the existing `setExportEnabled(true)` path flips it
  exactly as a live recording does.
- **Timestamp ownership** — unchanged. Regenerated timestamps are synthetic and explicitly
  **not compared**; alignment is by per-uid sequence. Recorded timestamps are reported in
  divergence detail only.

## Data model & persistence

Additive migration in `DatabaseManager` (same pattern as `migrateColumnsTable`, so every
pre-0044 archive keeps opening):

- `sessions` new nullable columns: `raw_sha256 TEXT`, `readings_sha256 TEXT`,
  `app_version TEXT`, `capture_format INTEGER`, `repro_class TEXT` (JSON),
  `frames_dropped INTEGER`, `overflow_bytes INTEGER`. NULL = legacy capture (R8).
- New table `verifications (verification_id INTEGER PRIMARY KEY AUTOINCREMENT, session_id
  INTEGER NOT NULL REFERENCES sessions, verified_at TEXT NOT NULL, app_version TEXT NOT
  NULL, verdict TEXT NOT NULL, detail_json TEXT)` — append-only (R7).
- `PRAGMA user_version` stamped to `1` on create/migrate (file-level format version; the
  full format-freeze doc is spec follow-up per Non-Goals).
- No `.ssproj` / `Frame.h` `Keys::` changes. `project_json` is consumed as stored.

## API / SDK surface

- `sessions.verify` in `SessionsHandler` (`#ifdef BUILD_COMMERCIAL`, registered in
  `CommandHandler::initializeHandlers()` already): starts verification for a db path +
  session id, completion delivers the verdict JSON (async start + completion event/poll per
  the handler infra's existing long-operation convention — exact shape settled in tasks
  against that infra). Same JSON schema as the CLI stdout, so pytest and lab CI consume one
  format (R9).
- SDK/generated surfaces regenerate via the standard `sanitize-commit.py` pipeline.

## QML / UI

- `SessionDetail.qml`: "Verify reproducibility" button → `DatabaseManager.verifySession`;
  busy state while the child runs; verdict panel (verdict, verified_at, app version,
  per-dataset divergence list, classification reasons, legacy qualifier).
- `SessionList.qml`: latest-verdict badge per session (from the `verifications` read API).
- Wording per spec Truth-in-labeling: fixed strings state what the check proves and what it
  does not (no determinism guarantee, not a safety function).

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Where verification runs | (A) child process, own composition root; (B) in-process in the live app; (C) standalone re-parser | **A** — B destroys the user's live session state and adds a hotpath branch (violates R1); C is a checker fork that drifts (violates the deciding constraint). A reuses the benchmark's proven headless pattern. |
| How regenerated values are captured | temp-DB re-record via untouched `Sessions::Export` + SQL diff; in-memory comparator sink | **Temp-DB re-record** — zero new frame-path code, write path byte-identical to production, regenerated DB is a durable debug artifact (`--verify-keep-regen`). In-memory saves disk but needs a new branch in `hotpathTxFrame`. |
| Fingerprint algorithm | SHA-256 (`QCryptographicHash`); xxHash/FNV | **SHA-256** — tamper-evident and platform-stable; runs on the worker thread where speed is a non-issue. |
| Row alignment | per-uid sequence (`ROW_NUMBER` over `reading_id`); timestamp matching | **Sequence** — regenerated timestamps are synthetic; recorded `timestamp_ns` is monotonicized per frame, so ordinal position is the only stable join key. |
| Verdict storage | `verifications` table in the archived DB; sidecar file | **In-DB** — travels with the evidence, survives file moves; spec explicitly allows appended verification records. |
| Classification depth | capture-time flags (control script, transforms + table-capture, per-dataset `is_virtual`); static analysis of script code | **Flags** — cheap, honest, already-available signals; script analysis is over-engineering with false confidence. |

## Risks & mitigations

- **Capture-side frame drops break sequence alignment** (FrameConsumer queue overflow at
  high rate: recorded stream is a subset of regenerated). Mitigation: persist the worker's
  drop counter + FrameReader overflow bytes at finalize; on count mismatch the verdict says
  *diverged: count mismatch* and cites those stats so a lossy capture is distinguishable
  from an interpretation change. No fuzzy realignment (honesty over green checkmarks).
- **`buildFrameConfig` exposure** — accessor only, no logic move; no new `instance()` /
  `SessionContext::current()` call sites (singleton census stays flat).
- **Child-process composition root drift** — verifier adds no modules and never reorders
  `instantiateCoreModules()`; same contract the benchmark already relies on.
- **Windows GUI-subsystem stdout** — QProcess pipes handles directly (not console
  attachment), so the `/SUBSYSTEM:WINDOWS` CI gotcha does not apply to the parent-spawned
  child; the pytest CLI tests run through the same piped spawn. Verified in tasks on CI.
- **Concurrent append while Explorer has the archive open** — WAL + `busy_timeout=5000` is
  already the project standard; append happens once at verdict time.
- **`operator[]` inserts / setter-guard classes** (common-mistakes) apply to the new
  DatabaseManager/QML model code — normal review discipline.
- **Licensing in the child** — composition root builds licensing first (spec 0042);
  verification is Pro-gated at the CLI/handler/UI entries, and a GPL build compiles none of
  it (`BUILD_COMMERCIAL`).

## Test & verification plan

- **Unit (I can run):** none — no JS-parser semantics change (`tests/scripts/` untouched).
- **Integration (maintainer runs, app up + API server):**
  `pytest tests/integration/test_session_verification.py -v`
  - AC1: record synthetic session (Native + JS variants) via API-driven capture, run
    `sessions.verify`, assert verdict `reproduced`, zero divergences.
  - AC2: copy fixture, flip one `readings` row via sqlite3, assert `diverged`, dataset
    named, count 1, exact value pair, attribution `archive-modified`.
  - AC3: copy fixture, edit a transform constant inside stored `project_json`, assert
    `diverged` with attribution `interpretation` (integrity hashes exclude `project_json`
    tamper → readings hash still matches, so divergence lands on the transform stage).
  - AC4: record with a table-fed (virtual) dataset, assert classification verdict, not
    `reproduced`.
  - AC5: checked-in legacy fixture verifies with qualified legacy verdict, exit 0 path, no
    crash.
  - AC6 (destructive-marked): start live capture, run verification concurrently, assert
    capture rows keep flowing and both DBs stay intact; maintainer eyeballs dashboard.
  - AC7: restart app (maintainer), verdict badge persists (read from `verifications`).
- **Hotpath:** `--benchmark-hotpath` full run (AC8) — expected unchanged; exporter-phase
  readout may shift within noise from worker-thread hashing (ungated by design).
- **Static:** `python scripts/code-verify.py --check` on every touched file;
  `qt-cpp-review` before handoff; `python scripts/sanitize-commit.py` before commit.
