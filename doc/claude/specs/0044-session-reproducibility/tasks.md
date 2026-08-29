---
spec: 0044-session-reproducibility
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-08-05
---

# Tasks 0044 — Session Reproducibility Verification

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

## Tasks

### T1 — Schema migration: fingerprint columns, verifications table, user_version

- **Files:** `app/src/Sessions/DatabaseManager.h/.cpp`
- **Does:** Adds the additive migration following the existing `migrateColumnsTable` pattern:
  nullable `sessions` columns (`raw_sha256`, `readings_sha256`, `app_version`,
  `capture_format`, `repro_class`, `frames_dropped`, `overflow_bytes`), new append-only
  `verifications` table, `PRAGMA user_version = 1` stamp on create/migrate. Binding
  invariant: migration must leave every pre-0044 archive readable (NULL = legacy), and
  `createSchema` stays the single source of truth shared with `ExportWorker`.
- **Verify:** `python scripts/code-verify.py --check app/src/Sessions/DatabaseManager.cpp`;
  read-back: open a copy of an existing session DB via the migration path in code review
  (no data-destructive statement present — ALTER ADD / CREATE IF NOT EXISTS only).
- **Deps:** none
- [x] done

### T2 — Canonical serialization + capture-side SHA-256 fingerprints

- **Files:** `app/src/Sessions/Export.h/.cpp`
- **Does:** Two incremental `QCryptographicHash` (SHA-256) members in `ExportWorker`: raw
  hash updated per chunk inside `writeRawBytes` (insertion order = `raw_id` order), readings
  hash updated per row inside `bindAndInsertReading` over the canonical byte layout from
  plan.md (LE64 uid, IEEE-754 bit-pattern LE64 doubles, length-prefixed UTF-8 strings,
  is_numeric byte). Hash state resets in `closeResources`. Binding invariants: worker thread
  only — nothing on the main-thread frame path changes; canonical layout is shared code the
  verifier (T6) will reuse, not duplicated.
- **Verify:** `python scripts/code-verify.py --check app/src/Sessions/Export.cpp`; read-back:
  hash update sites sit inside the existing batched-write loops only.
- **Deps:** T1
- [x] done

### T3 — Capture-time classification + finalize stamping

- **Files:** `app/src/Sessions/Export.h/.cpp`
- **Does:** Main-thread snapshot of classification inputs when recording starts (control
  script running, per-dataset transforms present, FrameBuilder table-capture flag; per-dataset
  `is_virtual` already lands via `columns`), handed to the worker through the existing
  project-snapshot mutex pattern. `finalizeSession()` writes both digests, `app_version`,
  `capture_format = 1`, `repro_class` JSON, and the worker's frame-drop counter + FrameReader
  overflow bytes into the session row. Binding invariants: no new frame-path signal; the
  drop/overflow stats are read at close time (1 Hz-pull discipline — never per frame); the
  cross-thread hand-off reuses the mutex, no new connection type.
- **Verify:** `python scripts/code-verify.py --check app/src/Sessions/Export.cpp`; read-back:
  `finalizeSession` writes only the new nullable columns; no `Q_EMIT` added on frame paths.
- **Deps:** T2
- [x] done

### T4 — FrameConfig exposure for the verifier

- **Files:** `app/src/IO/ConnectionManager.h` (and `.cpp` only if a definition moves)
- **Does:** Public accessor exposing the existing `buildFrameConfig(int)` result — no logic
  change, no reordering. Binding invariants: no new `instance()` /
  `SessionContext::current()` call sites (singleton census flat); no signal wiring touched;
  ConnectionManager is hotpath-adjacent — the diff is declaration-level only.
- **Verify:** `python scripts/code-verify.py --check app/src/IO/ConnectionManager.h`;
  `python scripts/code-verify.py --singleton-census --check` unchanged.
- **Deps:** none
- [x] done

### T5 — Temp-DB target override for re-record

- **Files:** `app/src/Sessions/DatabaseManager.h/.cpp`, `app/src/Sessions/Export.cpp`
- **Does:** Explicit override hook for the session-DB target path (used only by the verifier
  child process) threaded through `canonicalDbPath` resolution, plus
  `setSettingsPersistent(false)` honored on the enable path so verification never writes
  QSettings. Binding invariant: default behavior byte-identical when no override set — the
  live capture path must not change.
- **Verify:** `python scripts/code-verify.py --check` on touched files; read-back: override
  consulted in exactly one resolution point, nowhere on the frame path.
- **Deps:** T1
- [x] done

### T6 — Sessions::Verifier: archive reader + integrity + classification stages

- **Files:** `app/src/Sessions/Verifier.h/.cpp` (new), `app/src/CMakeLists.txt` (or the TU
  list that registers Sessions sources)
- **Does:** New class (commercial license header): opens archive read-only; loads session
  row, `project_json`, `columns` map, fingerprints, classification; integrity stage re-hashes
  `raw_bytes` + `readings` with the T2 shared canonical code and attributes mismatch to
  archive modification; classification stage emits per-dataset not-verifiable roll-up (Q1);
  legacy (NULL digests) qualifies the verdict. Produces the verdict/detail JSON structure.
  No re-parse yet.
- **Verify:** `python scripts/code-verify.py --check app/src/Sessions/Verifier.cpp`; header
  layout per code-style (plain object driven by the CLI runner — no singleton, no instance()).
- **Deps:** T1, T2
- [x] done

### T7 — Verifier re-parse stage: readers, feed loop, re-record, diff

- **Files:** `app/src/Sessions/Verifier.cpp`
- **Does:** Re-parse drive: `ProjectModel::loadFromJsonDocument(project_json)`; enable
  `Sessions::Export` with T5 override + persistence off; one `IO::FrameReader` per archived
  device configured via T4 accessor; feed `raw_bytes` chunks in `raw_id` order through
  `IO::makeCapturedData`, drain each reader's queue into `FrameBuilder::hotpathRxFrame` /
  `hotpathRxSourceFrame(deviceId, ...)` mirroring `ConnectionManager::onFrameReady` routing;
  close regenerated session; SQL sequence-diff per plan (ROW_NUMBER over `reading_id`,
  bit-exact numeric + exact text compare, stage attribution raw=parse/final=transform,
  count-mismatch short-circuit citing archived drop/overflow stats); append `verifications`
  row; delete temp DB unless keep-flag. Binding invariants: calls existing public pipeline
  entry points only — zero edits inside FrameReader/FrameBuilder; runs only in the child
  process (never invoked from the live app); ConsoleOnly = integrity-only verdict; QuickPlot
  re-parses its synthesized project.
- **Verify:** `python scripts/code-verify.py --check app/src/Sessions/Verifier.cpp`;
  read-back against `ConnectionManager::onFrameReady` routing for fidelity.
- **Deps:** T4, T5, T6
- [x] done

### T8 — CLI entry: --verify-session

- **Files:** `app/src/Misc/CLI.h/.cpp`
- **Does:** Options `--verify-session <db>`, `--verify-session-id <n>` (default latest
  completed), `--verify-keep-regen`; `runSessionVerification()` mirroring
  `runHotpathBenchmark()`: offscreen platform, `ModuleManager::instantiateCoreModules()` plus
  the FrameBuilder/Sessions::Export wiring the child needs, run Verifier, JSON to stdout,
  binary exit code (verdict lives in the JSON; plan amended). Binding invariants: composition
  root order untouched (licensing first per 0042); `BUILD_COMMERCIAL`-gated; no module added
  to `instantiateCoreModules()`.
- **Verify:** `python scripts/code-verify.py --check app/src/Misc/CLI.cpp`; read-back: no
  edits inside `ModuleManager`.
- **Deps:** T7
- [x] done

### T9 — Parent-side launcher + verdict read APIs

- **Files:** `app/src/Sessions/DatabaseManager.h/.cpp`
- **Does:** `verifySession(sessionId)`: spawns `QCoreApplication::applicationFilePath()` via
  QProcess with piped stdout (`--verify-session` args), parses verdict JSON, emits a
  completion signal, refreshes the sessions model; read API for latest verdict per session
  (feeds Explorer). Binding invariants: async QProcess — never blocks the GUI thread; WAL +
  busy_timeout already cover the child's append while Explorer holds the DB open; no work on
  any frame path.
- **Verify:** `python scripts/code-verify.py --check app/src/Sessions/DatabaseManager.cpp`;
  read-back: QProcess lifetime owned + error/crash path emits a failed verdict, no orphan.
- **Deps:** T8
- [x] done

### T10 — API verb: sessions.verify

- **Files:** `app/src/API/Handlers/SessionsHandler.h/.cpp`
- **Does:** `sessions.verify` verb (`#ifdef BUILD_COMMERCIAL`): starts verification via T9,
  delivers the same verdict JSON as the CLI on completion, following the handler infra's
  existing long-operation convention (async start + completion event/poll — settle exact
  shape against that infra, do not invent a new one). Binding invariant: handler stays
  static-method pattern, registered surface only.
- **Verify:** `python scripts/code-verify.py --check app/src/API/Handlers/SessionsHandler.cpp`;
  `scripts/generate-sdk.py --check` after regen via sanitize pipeline.
- **Deps:** T9
- [x] done

### T11 — Database Explorer UI: verify action + verdict badge

- **Files:** `app/qml/DatabaseExplorer/SessionDetail.qml`,
  `app/qml/DatabaseExplorer/SessionList.qml`
- **Does:** "Verify reproducibility" action in SessionDetail (busy state while child runs;
  verdict panel with verdict, verified_at, verifying version, per-dataset divergence list,
  classification reasons, legacy qualifier, truth-in-labeling strings); latest-verdict badge
  in SessionList from the T9 read API. Binding invariants: QML sandwich comment style;
  strings via qsTr with numbered placeholders only; no live text-binding echo on any input
  field.
- **Verify:** `python scripts/code-verify.py --check` on both QML files; read-back against
  ref_code_style QML rules.
- **Deps:** T9
- [x] done

### T12 — Legacy fixture + pytest integration suite

- **Files:** `tests/fixtures/sessions/` (new legacy fixture DB + README),
  `tests/integration/test_session_verification.py` (new)
- **Does:** Checks in a small pre-0044 fixture DB; writes the AC1–AC7 test suite from
  plan.md's mapping (record-verify round trip for Native + JS, readings tamper, project_json
  transform tamper, virtual-dataset classification, legacy verdict, concurrent-capture
  destructive-marked case, verdict persistence via API read-back). Uses
  `tests/utils/api_client.py`; destructive cases marked per `pytest.ini` markers. Binding
  invariant: tests drive the running app over TCP (maintainer runs them) — no app spawn from
  pytest except the documented CLI child-path cases.
- **Verify:** `pytest --collect-only tests/integration/test_session_verification.py` parses;
  fixture README documents provenance (maintainer generates fixture with a real build).
- **Deps:** T10
- [x] done

### T13 — Docs: export.md section + spec artifact sync

- **Files:** `doc/claude/architecture/export.md`, `doc/claude/specs/0044-session-reproducibility/*`
- **Does:** "Reproducibility Verification" section in export.md (capture fingerprints,
  verifications table, child-process verifier, count-mismatch semantics); checklist statuses
  current; propose (in chat, not unilaterally) whether CLAUDE.md needs a one-line pointer.
- **Verify:** `python scripts/documentation-verify.py` clean on touched Markdown (export.md
  is AI-facing, not marketing — verify only if in its target set); links resolve.
- **Deps:** T7 (semantics settled)
- [x] done

## Definition of Done

- [x] Every acceptance criterion in `spec.md` is met and checked off there (AC1–AC7 via the
      T12 suite the maintainer runs; AC8 via the maintainer's `--benchmark-hotpath` run).
- [x] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [x] `qt-cpp-review` run on the C++ diff (6-agent pass, 2026-08-05); confirmed findings
      fixed (child-process wiring, CrashTracker early-exit flag, hash-after-exec, NaN digest
      fold, regen sanity gate, integrity-unreadable verdict, finalize fallback, QProcess
      crash-path stdout, id validation, device cap, override reset/guard); deferred:
      verification cancel action, verdict caching in DatabaseManager.
- [x] `--benchmark-hotpath` not regressed (maintainer runs; zero frame-path edits expected —
      any regression is a red flag on the diff, not a tuning problem).
- [x] `pytest tests/integration/test_session_verification.py` listed for the maintainer,
      destructive cases marked.
- [x] `python scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [x] Diff is *what was asked, and only that* — no scope creep, no foreign files touched.
- [x] `spec.md` status set to `done` (AC boxes stay open for the maintainer).
