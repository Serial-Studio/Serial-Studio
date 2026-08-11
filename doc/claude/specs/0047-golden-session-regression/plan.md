---
spec: 0047-golden-session-regression
phase: plan
status: approved     # draft -> approved (gate before /ss-tasks)
updated: 2026-08-06
---

# Plan 0047 — Golden-Session Parser Regression

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Read the relevant `doc/claude/` sub-docs and the *actual code*
> before writing this — a plan grounded in a stale mental model is worse than no plan.
> Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

Regression rides the spec-0044 child-process verifier (`Sessions::Verifier`, launched via
`--verify-session`) with a second mode: **dual replay**. The child replays the archived
`raw_bytes` twice through the untouched production pipeline — once under the archived
`project_json` (baseline), once under the candidate project (current editor state serialized
by the parent, or an explicit file) — each into its own temp re-record via the unmodified
`Sessions::Export` sink. Both feeds inject **synthetic chunk-indexed timestamps**
(`CapturedData::timestamp = t0 + raw_index * 1 ms`, the `makeCapturedData` overload that
already takes a timestamp), so `readings.timestamp_ns` in both temp DBs becomes a
deterministic provenance key: chunk index + intra-chunk rank, identical on both sides by
construction, immune to frames dropped inside `FrameBuilder` and to driver `frameStep`
spreading (which raw archives do not store). The diff is then SQL over the two temp DBs per
`unique_id`, joined on that key, producing the four-dimension drift report and the
severity-ordered verdict. Zero archive writes (ephemeral results per spec Q2), zero capture
path changes, zero edits to hotpath files.

## Affected subsystems & files

| File | Change |
|------|--------|
| `app/src/Sessions/Verifier.h` | `Options` gains `mode` (Verify/Regress), `candidateProjectPath`; regression state (candidate fingerprint, per-chunk first-frame bookkeeping, drift report members); declarations for the regression stages. |
| `app/src/Sessions/Verifier.cpp` | `run()` branches to the regression flow; `reparseSession()`/`feedArchivedBytes()` parameterized (project JSON source, regen path, timestamp-injection flag); shared stages (openArchive, loadSession, classify) reused as-is. |
| `app/src/Sessions/VerifierRegression.cpp` (new) | Second TU of the same class (god-file split pattern): dual-replay orchestration, candidate load + fingerprint (SHA-256 of candidate JSON bytes), device-topology check, provenance-join SQL diff, verdict taxonomy + report JSON. Commercial license header. |
| `app/src/Misc/CLI.h/.cpp` | New options `--regress-session <db>`, `--regress-session-id <n>`, `--regress-project <file>`, `--regress-keep-regen`; `runSessionRegression()` mirroring `runSessionVerification()` (`CLI.cpp:363-392`). |
| `app/src/Sessions/DatabaseManager.h/.cpp` | `regressSession(sessionId, candidatePath)` QProcess launcher sharing the single `m_verifyProcess` slot (one child at a time, spec AC8); when `candidatePath` empty, serialize `ProjectModel::serializeToJson()` to a temp file on the main thread first; `regressionBusy`/`lastRegressionReport` Q_PROPERTY + `regressionFinished` signal (ephemeral, never written to DB — see Data model). |
| `app/src/API/Handlers/SessionsHandler.h/.cpp` | `sessions.regress` (params: `sessionId` or `tag`, optional `projectJson` string / `projectPath`), `sessions.getRegression` poll — async start + poll, mirroring `sessions.verify`/`getVerification` (`SessionsHandler.cpp:164-174`). Tag sweep loops matching sessions sequentially in the parent, aggregates summary. `#ifdef BUILD_COMMERCIAL`. |
| `app/qml/DatabaseExplorer/SessionDetail.qml` | "Check against current project" action beside the 0044 verify button (`:431`); drift report panel (verdict badge, per-dataset table, first divergences), visually distinct from the reproducibility verdict (`:362`). |
| `tests/integration/test_session_regression.py` (new) | AC1–AC8 pytest coverage (running app + API server). |
| `doc/claude/architecture/export.md` | "Golden-Session Regression" subsection under the 0044 verification section. |

Out of lane, named here: no changes to `Sessions/Export.*`, `IO/FrameReader.*`,
`DataModel/FrameBuilder.*`, schema, or capture path.

## Architecture & data flow

**Parent (live app).** `DatabaseManager::regressSession()`: if no explicit candidate file,
write `ProjectModel::serializeToJson()` to a scratch temp file (main thread — same data the
`project.exportJson` handler serves). Spawn own binary with `--regress-session <db>
--regress-session-id <n> --regress-project <file>`, parse stdout JSON, expose via
`lastRegressionReport` + `regressionFinished`. Busy-gating shares the 0044 process slot so
verification and regression never run concurrently (single `m_verifyProcess`).

**Child sequence** (extends `Verifier::run()`):

1. CLI parses → offscreen → `instantiateCoreModules()` (licensing first, spec 0042) —
   identical bootstrap to `runSessionVerification()`.
2. Shared stages: `openArchive()` (read-only), `loadSession()` (project JSON, classification,
   fingerprints), `classifySession()`. Control-script sessions → `not_verifiable`, same rule
   as 0044. Candidate JSON read + SHA-256 fingerprint; parse failure → `error`.
3. **Topology check** (spec Q3): archived device ids (`SELECT DISTINCT device_id FROM
   raw_bytes`) vs the project's source ids, evaluated per side. An archived device absent
   from a side's source list is **excluded from that side's feed** and reported as
   source-removed structural drift — never left to `buildFrameConfig()`'s silent `/* */`
   fallback (`ConnectionManager.cpp:1817-1822`). Single-source projects keep device 0
   semantics.
4. **Baseline replay**: `reparseSession(archived project_json, regen-A path, inject=true)`.
   **Candidate replay**: reset pipeline state the same way a project switch does
   (`loadFromJsonDocument` + `FrameParser::readCode()` + `syncFromProjectModel()`, fresh
   `FrameReader`s), then `reparseSession(candidate JSON, regen-B path, inject=true)`.
   Injection: `feedArchivedBytes()` gains the flag; when set, chunk *k* feeds
   `makeCapturedData(bytes, kEpoch + k * kChunkStepNs)` with `kChunkStepNs = 1 ms`
   (FrameReader assigns frame `ts = chunk ts + rank * 1 ns` — `FrameReader.cpp:644-646` —
   and `monotonicFrameNs` bumps collisions by 1 ns, so ranks stay < 1 ms for any real
   chunk). The verifier records each side's first-drained-frame chunk index; stored
   `timestamp_ns` is baseline-relative, so `chunk = floor(ns / kChunkStepNs) +
   firstFrameChunk`, `rank = ns % kChunkStepNs` — the provenance key.
5. **Diff** (regen-A vs regen-B, per `unique_id` from each side's `columns` table):
   - *structural*: uid present in only one side's columns (identity = `uniqueId`, spec Q1)
     or whole-source exclusions from step 3;
   - *coverage*: provenance keys present on one side only (missing/added counts);
   - *value*: joined keys with `bitEqual` raw/final mismatch (same comparison core as
     `diffDataset`), tracking changed count, max |delta| over numeric finals, first
     divergence — reported with the **archived chunk's real `timestamp_ns`** (mapped back
     via chunk index) as the capture time;
   - virtual/table-fed datasets: classified, never compared (0044 rule).
6. Verdict = worst present class (`structural > coverage > value > identical`), qualified by
   0044 reproduction status: the report embeds `baselineReproduction` = the stored latest
   0044 verdict if any, plus `legacyCapture`. JSON to stdout; **no archive write** (no
   `appendVerificationRecord()` in regression mode); both temp DBs deleted unless
   `--regress-keep-regen`.

**Tag sweep (R9).** Parent-side: `sessions.regress {tag}` resolves session ids via the
existing tags tables (`session_tags`, `DatabaseWorker.cpp:935`), runs children sequentially
(one process slot), aggregates `{passed, drifted, notVerifiable, failed}` + per-session
reports in the poll response.

## Hotpath & threading impact

- **Touches the hotpath?** **No.** No edits to `FrameReader`, `CircularBuffer`,
  `FrameBuilder`, `Dashboard`, or the span lane. The child calls existing public entry
  points, same as 0044/benchmark. Timestamp injection uses the existing
  `makeCapturedData(data, timestamp, ...)` overload (`HAL_Driver.h:83`) — no signature or
  behavior change to any hotpath type. Live app impact: one QProcess spawn + temp-file
  write, UI-triggered only.
- **New cross-thread signal/slot?** No new frame-path connections. Parent-side
  `regressionFinished` is a main-thread QProcess-completion signal, same shape as
  `verificationFinished`.
- **New input to a cached hotpath flag?** None. The child flips `setExportEnabled` exactly
  as 0044 already does; the live app's flags are untouched.
- **Timestamp ownership** — preserved in spirit and letter: live capture still stamps at the
  driver boundary; in the child the *archive* is the source, and injection happens at the
  same boundary (`CapturedData` construction before `FrameReader::processData`). Nothing
  re-stamps downstream. 0044 verify mode keeps its current non-injected stamps —
  byte-identical behavior (spec constraint "0044 untouched").

## Data model & persistence

**None.** No schema changes, no new tables, no new `Frame.h` keys. Regression results are
ephemeral (spec Q2): child stdout → parent memory → API poll / QML property. The archive is
opened read-only and — unlike 0044, which appends a `verifications` row — regression writes
**nothing** to it. Temp artifacts: candidate JSON temp file (parent) and two regen DBs
(child), all deleted on completion unless `--regress-keep-regen`.

## API / SDK surface

- `sessions.regress` — params: `sessionId` (int) **or** `tag` (string, sweep), optional
  `projectJson` (string, written to temp file by the handler) or `projectPath` (string);
  default candidate = current editor project. Errors if a verification/regression is
  already running (shared slot) or no database is open.
- `sessions.getRegression` — poll: `{running, report}` (single) or `{running, summary,
  reports[]}` (sweep). Same JSON as CLI stdout so pytest/CI consume one format (R8).
- Both registered in `SessionsHandler` under `#ifdef BUILD_COMMERCIAL`; SDK/generated
  surfaces regenerate via the standard `sanitize-commit.py` pipeline.

## QML / UI

- `SessionDetail.qml`: "Check against current project" button (disabled while
  `verificationBusy || regressionBusy`); result panel titled as drift **vs the current
  project** — verdict badge (identical / value drift / coverage drift / structural drift /
  not verifiable / error), per-dataset drift table, first-divergence rows, candidate name +
  fingerprint, `baselineReproduction` qualifier. Placement and wording keep it visually
  separate from the stored 0044 reproducibility verdict (spec R10); truth-in-labeling
  strings state the comparison is against a transient candidate and is not stored.
- No SessionList change (0044 verdict badge stays the only per-row badge; regression is
  ephemeral).

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Diff baseline | (A) single replay vs recorded readings, reconstructed timestamps; (B) chunk provenance recorded at capture; (C) dual replay, injected provenance keys | **C** (maintainer-approved, spec R4 amended) — A must reproduce baseline-relative monotonicized stamps and driver `frameStep` the archive does not store (audio-class sources cannot match bit-exact; silent misalignment is this spec's kill target); B violates the no-capture-change non-goal and would not help existing archives anyway. C is deterministic by construction and equals the recording whenever 0044 reproduces. |
| Provenance key transport | injected chunk-indexed timestamps through the untouched pipeline; verifier-side frame ledger at the drain loop; extra column in the temp re-record | **Injection** — survives frames dropped inside `FrameBuilder` (a ledger at the drain point cannot see those drops and would silently misalign); an extra column would fork `Sessions::Export`. One flag-gated line in the feed loop. |
| Where regression runs | extend `Verifier` (new TU, shared stages); separate `Regressor` class; in-process | **Extend Verifier** — openArchive/loadSession/classify/feed are shared verbatim; a sibling class duplicates them and drifts. New `VerifierRegression.cpp` TU keeps file sizes sane. In-process was rejected by 0044 already (destroys live state). |
| Concurrency | share the 0044 process slot; independent parallel children | **Shared slot** — two children re-recording through one temp-DB override pattern invite path collisions; sequential is plenty for a lab sweep and satisfies AC8 trivially. |
| Tag sweep location | parent loops children; one child does the whole sweep | **Parent loops** — child stays single-session/simple, poll surface reports incremental progress, a crashed session run fails one entry instead of the sweep. |
| Candidate hand-off | temp file path; JSON on argv; stdin | **Temp file** — argv has platform length limits and quoting hazards for multi-MB projects; stdin complicates the existing QProcess wiring. File is the 0044-era pattern (db path on argv). |

## Risks & mitigations

- **Chunk-rank overflow**: > 10^6 frames parsed from a single chunk would spill into the
  next chunk's key space. `kChunkStepNs = 1 ms` at 1 ns/rank allows 10^6; the verifier
  counts drained frames per chunk and downgrades to `error` (inconclusive) if any chunk
  exceeds the budget — never a silent misjoin.
- **Pipeline state bleed between replays**: transforms/parsers from replay A surviving into
  replay B. Mitigation: candidate load runs the full project-switch path
  (`loadFromJsonDocument` → `readCode()` → `syncFromProjectModel()`), fresh `FrameReader`
  map, `resetMonotonicClock` implied by the new Export session; AC1 (identical project ⇒
  identical verdict) is the regression test for bleed, and a self-check diff of A vs A can
  be enabled in tests via env if needed.
- **`buildFrameConfig` silent fallback for removed sources** — pre-checked topology (step
  3) excludes those devices explicitly; grounded at `ConnectionManager.cpp:1817-1822`.
- **QuickPlot/ConsoleOnly-archived sessions**: `buildFrameConfig` short-circuits on the
  *live* operation mode (`AppState`), and the child pins `ProjectFile` mode as 0044 does;
  console-only archives (no groups) report `not_verifiable` for regression, same guard as
  0044's `sessionIsConsoleOnly()`.
- **Large sessions, double replay**: wall-clock doubles vs 0044 verify. Acceptable
  (offline, async, poll-based); report includes per-side replay row counts so a truncated
  run is visible. No parallelism inside the child (thread-affinity of the pipeline).
- **Windows GUI-subsystem stdout** — same piped-QProcess pattern 0044 already validated on
  CI.
- **Singleton census** — no new `instance()`/`SessionContext::current()` call sites beyond
  the existing Verifier pattern; census must stay flat (`code-verify.py --singleton-census
  --check`).
- **Licensing** — regression is Pro-gated at CLI/handler/UI entries, `BUILD_COMMERCIAL`
  compiles it out of GPL builds, same as 0044.

## Addendum (2026-08-06, maintainer-requested at plan approval): actionable error reporting

The 0044 verification error surface is opaque and regression inherits it, so both get the
same structured treatment in this pass:

- **Child-side (`Verifier`)**: every `fail()` site emits, alongside the human sentence, a
  stable `errorCode` slug, the `stage` that failed (`open-archive` / `load-session` /
  `integrity` / `reparse` / `diff` / `regress-*`), and a `hint` naming the likely fix. The
  conflated `reparseSession()` failure splits into distinct causes: invalid stored project
  JSON, project load rejected, export unavailable because the build/machine is not
  Pro-entitled (checked explicitly so licensing reads as licensing, not as a generic
  failure).
- **Parent-side (`DatabaseManager`)**: child spawn failure and child crash/non-JSON stdout
  no longer conclude with an empty map — the parent synthesizes a structured report
  (`errorCode`: `child-spawn-failed` / `child-crashed` / `child-output-invalid`, exit code,
  and the tail of captured stderr) so the UI and API always have a why. Applies to both the
  verify and regress launch paths.
- **UI (`SessionDetail.qml`)**: the error state renders the reason sentence and the hint,
  not just the word "error"; same panel component serves both passes.
- **API**: `sessions.getVerification` / `sessions.getRegression` carry the structured
  fields automatically (they relay the report JSON).
- **Test**: pytest asserts the structured fields on a forced failure (nonexistent archive
  path → `errorCode: open-archive-failed` with hint; unparseable candidate JSON →
  `regress-candidate-invalid`).

Lane note: this touches the 0044 error paths inside the same files already in the table
above plus the verify-launch half of `DatabaseManager.cpp`; no new files.

## Test & verification plan

- **Unit (I can run):** none — no JS-parser semantics change (`tests/scripts/` untouched).
- **Integration (maintainer runs app + API server):**
  `pytest tests/integration/test_session_regression.py -v`
  - **AC1** identity: capture synthetic session, `sessions.regress` with untouched project →
    `identical`, all-zero drift figures.
  - **AC2** value drift: change one transform constant → `value drift`, exact changed count,
    correct first-divergence pair + max delta, other datasets zero.
  - **AC3** coverage drift: candidate parser variant rejecting a known frame subset →
    `coverage drift`, missing count exact, **zero values changed** (the sim's ordinal-pairing
    failure must not reproduce).
  - **AC4** structural drift: candidate adds one dataset, removes another → both listed,
    survivors zero-drift.
  - **AC5** explicit candidate file via `projectPath`/`projectJson` → report names + hashes
    the candidate.
  - **AC6** classification: table-fed dataset → classified not compared; legacy fixture
    (0044's) → legacy-qualified verdict.
  - **AC7** tag sweep: three tagged sessions, drifting candidate → aggregate matches
    per-session verdicts.
  - **AC8** (destructive-marked): regression during live capture → capture unaffected;
    `sessions.verify` while regression runs → clean busy error.
- **UI (maintainer, AC9):** edit transform → run from SessionDetail → read report → revert →
  rerun → `identical`.
- **Hotpath (AC10):** `--benchmark-hotpath` full run — expected bit-identical flags/paths;
  no live-app code executes per frame.
- **Static:** `python scripts/code-verify.py --check` on every touched file; `qt-cpp-review`
  before handoff; `python scripts/sanitize-commit.py` before commit.
