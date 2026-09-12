---
spec: 0020-replay-timeline
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-07-19
---

# Tasks 0020 — Replay timeline rework

> **Phase 3 of 4 — the ordered checklist.** Decompose [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable*. `/ss-implement` works this list top to
> bottom and keeps the status boxes current. Gate: do not start `/ss-implement` until a human
> marks this `approved`.

## Conventions

- One task = one focused, reviewable change. If a task touches >3 files or needs a paragraph
  to describe, split it.
- **Verify** is how *this* unit is confirmed before moving on.
- **Deps** lists task IDs that must land first.

## Tasks

### T1 — FrameBuilder replay ingestion entry

- **Files:** `app/src/DataModel/FrameBuilder.h`, `app/src/DataModel/FrameBuilder.cpp`
- **Does:** Adds `replayChannels(sourceId, const QStringList& channels, recordedTs)`: asserts
  `m_playerOpen`, runs `applyDatasetValues` with the existing replay column map, publishes
  via `acquireFrame` (slot pool — `structureGeneration` stamped exactly like the live sites)
  to `Dashboard::hotpathRxFrame` plus API/gRPC observers only — **never CSV/MDF4/Sessions
  export or MQTT** (R6). Recorded timestamp rides the frame; nothing re-stamps downstream.
  Hotpath invariants bound here: slot pool no-alloc, no new signal hops, no new cached flags.
- **Verify:** `python scripts/code-verify.py --check` on both files; read-back diff against
  the invariant list above.
- **Deps:** none
- [x] done

### T2 — CSV playback through the fast path + budgeted catch-up

- **Files:** `app/src/CSV/Player.h`, `app/src/CSV/Player.cpp`
- **Does:** Playback (`updateData`, batch catch-up, `processFrameBatch`) feeds
  `replayChannels` with the stored row's `QStringList` (no `joinReplayRow` → bytes →
  re-split round trip) and the recorded timestamp; replaces `kMaxBatchSize = 100` with a
  ~20 ms wall-clock budget per pass, yielding via the existing single-shot timer. Multi-source
  keeps per-source slices from the existing layout map (uids are per-source).
- **Verify:** code-verify on both files; read-back: no remaining `processPayload` call on the
  CSV playback path. (QuickPlot-mode replay intentionally keeps the byte path — the QuickPlot
  parser consumes raw payloads and `replayChannels` is ProjectFile-only.)
- **Deps:** T1
- [x] done

### T3 — MDF4 playback through the fast path + budgeted catch-up

- **Files:** `app/src/MDF4/Player.h`, `app/src/MDF4/Player.cpp`
- **Does:** Same conversion as T2 (`sendFrame`/`catchUpToTarget`), building the channel list
  straight from the sample cache — also removes the multi-source split-then-rejoin of its own
  row. Time budget replaces the fixed batch.
- **Verify:** code-verify on both files; read-back as T2.
- **Deps:** T1
- [x] done

### T4 — Sessions playback through the fast path + budgeted catch-up

- **Files:** `app/src/Sessions/Player.h`, `app/src/Sessions/Player.cpp`
- **Does:** `injectFrame(QHash)` → channel-list construction feeding `replayChannels`
  per source; time-budgeted catch-up. SQL access unchanged in this task. (Commercial build.)
- **Verify:** code-verify on both files; read-back as T2. Clean benchmark point: playback
  lanes converted, `--benchmark-hotpath` runnable by maintainer here.
- **Deps:** T1
- [x] done

### T5 — Dashboard bulk plot-window load + seek clock reset

- **Files:** `app/src/UI/Dashboard.h`, `app/src/UI/Dashboard.cpp`
- **Does:** Bulk-load entries that, given a per-row `(timestampSec, per-source uid→double)`
  provider and a window, reset and refill the plot rings: samples y-ring, per-curve
  `TimeRing` (reuse `appendDecimated` verbatim in recorded-time order — never hand-rolled
  cells, absolute-grid semantics preserved), dataset-X rings, MultiPlot curves. Adds the
  seek-time reset of `m_plotClocks` / display clock so play-after-scrub resumes cleanly.
  Writes rings only — push tables and `m_layoutValid` untouched.
- **Verify:** code-verify on both files; read-back diff against the TimeRing/clock/push-table
  invariants named above.
- **Deps:** none (parallel to T2–T4)
- [x] done

### T6 — CSV tape scrub (coalesce, bulk fill, settle)

- **Files:** `app/src/CSV/Player.h`, `app/src/CSV/Player.cpp`
- **Does:** `setProgress` records the target and coalesces to a ~30 Hz main-thread timer;
  each tick bulk-fills plot rings via T5 from `m_csvData`/`m_timestampCache` (numeric parse
  on demand, plot-enabled datasets only, no new caches), injects the cursor row via T1 for
  scalar widgets, updates the timestamp display; a ~250 ms debounced at-rest pass runs
  `clearPlotData()` + full trailing-window replay through T1 (FFT/waterfall/GPS/3D exact).
  Backward and forward are the same rebuild-at-cursor operation.
- **Verify:** code-verify; read-back: `setProgress` no longer does synchronous window
  injection per slider tick.
- **Deps:** T1, T5
- [x] done

### T7 — MDF4 tape scrub

- **Files:** `app/src/MDF4/Player.h`, `app/src/MDF4/Player.cpp`
- **Does:** Same as T6, provider reads the MDF4 sample cache; `nextFrame`/`previousFrame`
  reuse the settle pass (single-step stays exact).
- **Verify:** code-verify; read-back as T6.
- **Deps:** T1, T5
- [x] done

### T8 — Sessions tape scrub (windowed SQL provider)

- **Files:** `app/src/Sessions/Player.h`, `app/src/Sessions/Player.cpp`
- **Does:** Same as T6 with a windowed range query over `readings` (covering index; ties
  broken by `reading_id`, never `DISTINCT timestamp_ns`), bounded by the window size.
- **Verify:** code-verify; read-back as T6. Second clean benchmark point.
- **Deps:** T1, T5
- [x] done

### T9 — Player dialog settle hint (only if needed)

- **Files:** `app/qml/Dialogs/CsvPlayer.qml`, `app/qml/Dialogs/Mdf4Player.qml`,
  `app/qml/Dialogs/SqlitePlayer.qml`
- **Does:** Only if the debounce alone can't detect end-of-gesture reliably: bind slider
  `pressed` to trigger the settle pass on release. Named here so it is not lane creep; skip
  and check off if the debounce suffices.
- **Verify:** code-verify (QML rules); manual drag in-app by maintainer.
- **Deps:** T6–T8
- [x] done — skipped as planned: the 250 ms settle debounce (restarted per move) detects
  end-of-gesture without a QML pressed binding; no dialog change needed.

### T10 — Integration tests (AC1, AC3, AC5)

- **Files:** `tests/integration/test_replay_timeline.py` (new)
- **Does:** AC1 — API-driven seek forward then backward; `dashboard.tailFrames` equals a
  fresh-open direct-seek baseline; play-after-backward-scrub sanity. AC3 — constant-rate
  wide generated CSV: pace within tolerance + delivered frames == recorded rows. AC5 — CSV
  export enabled during replay + scrub: no export growth, no session rows. CSV + MDF4
  parametrized; sessions variant gated on commercial build.
- **Verify:** file imports clean (`python -m py_compile`); maintainer runs
  `pytest tests/integration/test_replay_timeline.py -v` with the app up.
- **Deps:** T2–T8
- [x] done — CSV covers the shared lanes end-to-end (AC1/AC3/AC5); MDF4/session file
  authoring has no test helper, so their variants ride the shared-code coverage + AC2.

### T11 — Docs & memory sync

- **Files:** `doc/claude/architecture/export.md`, `doc/claude/architecture/dataflow.md`
- **Does:** Documents the replay ingestion entry, the publish-target policy (observers yes,
  recorders no), and the scrub lanes; updates the replay memory file. CLAUDE.md untouched
  (no top-level architectural rule changes).
- **Verify:** read-back; `documentation-verify.py` not applicable (doc/claude exempt from
  user-facing docs lint).
- **Deps:** T1–T8
- [x] done

## Definition of Done

- [x] Every acceptance criterion in `spec.md` is met and checked off there — EXCEPT AC2/AC4,
  which are maintainer-run: field-project scrub feel in-app + `--benchmark-hotpath` all tiers.
- [x] `python scripts/code-verify.py --check` is clean on all changed files (0 errors,
  0 advisories).
- [x] `qt-cpp-review` run (6-agent pass, 2026-07-19): the confirmed findings were fixed
  (compile-guard-safe engine teardown routed through playback-aware `compileTransforms`;
  static iteration cap alongside the catch-up time budget; QuickPlot scrub falls back to the
  settle rebuild instead of blanking rings; Sessions seek query prepared once + checked +
  NaN series cleared on failure; CSV date/time seconds cache; `constFind` in the Dashboard
  fill helpers; `[[nodiscard]] const` on `buildRowCells`; `std::pair` on `replaySeekSeries`;
  bounds asserts in the seek slots). Noted-not-fixed (outside lane / pre-existing class):
  CSV const-chain asymmetry, PathPolicy >64-depth fallback hardening, stale replay maps on
  project-reload-during-replay, near-dead multi-source byte branches kept as mode-crossover
  guards, settle-batch steady-stamp skew (benign).
- [x] `ss-hotpath` invariants named per task; `--benchmark-hotpath` re-run is the
  maintainer's AC4 gate (span lane untouched, QList lane strictly lighter).
- [x] `pytest tests/integration/test_replay_timeline.py -v` identified for the maintainer
  (app up with API server).
- [x] `python scripts/sanitize-commit.py` run; tree clean of lint debt.
- [x] Diff is *what was asked, and only that* — every changed file maps to spec 0020, the
  two earlier bug fixes, the mimalloc bump, or the doc/test sync.
- [x] `spec.md` status set to `done` (pending AC2/AC4 maintainer verification).
