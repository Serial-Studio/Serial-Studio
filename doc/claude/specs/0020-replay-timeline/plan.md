---
spec: 0020-replay-timeline
phase: plan
status: approved     # draft -> approved (gate before /ss-tasks)
updated: 2026-07-19
---

# Plan 0020 — Replay timeline rework

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

Two cooperating lanes replace today's one-size-fits-all injection. **(1) A replay ingestion
fast path** in `FrameBuilder` accepts a player's already-split channel row (`QStringList` +
the recorded timestamp) directly — eliminating today's join-to-bytes → re-split round trip —
and publishes to the dashboard and read-only API observers but never to recording sinks
(CSV/MDF4/Sessions export, MQTT publish), satisfying R6 by construction. Paced playback and
catch-up use this path with a **time-budgeted batch loop** (~20 ms per event-loop pass)
instead of the fixed 100-row batch, so heavy recordings keep pace losslessly and the GUI
stays responsive. **(2) A bulk seek lane** for scrubbing: while the slider is dragged
(coalesced to ~30 Hz), `Dashboard` bulk-loads only the plot rings — numeric-parsing the
window's rows straight out of the player's existing in-memory storage for plot-enabled
datasets only (~ms per tick at the field project scale), plus one fast-path inject of the cursor frame
for scalar widgets — and a debounced at-rest pass replays the full trailing window through
lane (1) so FFT/waterfall/GPS/3D end exact (spec Q3). Backward and forward scrub are the
same operation (rebuild window ending at cursor), which is what makes tape semantics hold by
construction. This shape was chosen over "budgeted pipeline-only" (misses the 50 ms drag
budget at the field project scale) and "precomputed numeric matrix" (violates the memory bound).

## Affected subsystems & files

| File | Change |
|------|--------|
| `app/src/DataModel/FrameBuilder.h/.cpp` | New replay ingestion entry `replayChannels(sourceId, channels, timestamp)` (name final at implement): applies the replay column map, publishes via `acquireFrame` to Dashboard + API/gRPC observers only; reuses `m_replayColumnMap`, `m_playerOpen`. |
| `app/src/UI/Dashboard.h/.cpp` | Bulk plot-window load entries (samples y-ring, per-curve `TimeRing`, dataset-X ring, MultiPlot curves) fed by a per-row `(timestampSec, uid→double)` provider; seek-time plot-clock reset; interplay with `clearPlotData()`. |
| `app/src/CSV/Player.h/.cpp` | Scrub coalescing timer (~30 Hz), drag bulk fill from `m_csvData`/`m_timestampCache`, debounced at-rest window replay, playback/catch-up switched to the fast path with a time-budgeted batch loop. |
| `app/src/MDF4/Player.h/.cpp` | Same as CSV, fed from the MDF4 sample cache. |
| `app/src/Sessions/Player.h/.cpp` | Same, fed by a windowed SQL range query over `readings` (covering index `(session_id, unique_id, timestamp_ns)` exists; ties broken by `reading_id`). |
| `app/qml/Dialogs/CsvPlayer.qml`, `Mdf4Player.qml`, `SqlitePlayer.qml` | Expected unchanged (`onMoved` already fires continuously); touched only if a pressed/released hint is needed for the settle pass. |
| `tests/integration/test_replay_timeline.py` (new) | AC1 (tape accuracy), AC3 (pace + losslessness), AC5 (no re-record). |
| `doc/claude/architecture/export.md`, `dataflow.md` | Post-implement doc sync: replay ingestion path + publish-target policy. |

## Architecture & data flow

**Playback / at-rest rebuild (lane 1):**
`Player (main thread) → FrameBuilder::replayChannels(sourceId, QStringList, recordedTs)`
→ `applyDatasetValues` with the existing replay column map (transforms stay gated off)
→ `acquireFrame(frame, recordedTs)` (slot pool, `structureGeneration` stamped as today)
→ `Dashboard::hotpathRxFrame` + API server / gRPC observer fan-out (detached copy, as the
async-sink fan-out does today) — **recording sinks are not called**. All direct calls on the
main thread; no signals added. The players' `updateData` loops keep their timestamp-driven
scheduling but replace `kMaxBatchSize = 100` with a wall-clock budget (process rows until
~20 ms spent, then yield via the existing single-shot timer). Recorded timestamps ride the
frame (source-owns-time); nothing downstream re-stamps.

**Drag scrub (lane 2):**
Slider `onMoved` → player `setProgress` records the target and (re)starts a ~30 Hz coalescing
timer. Each coalesced tick: (a) `Dashboard` bulk-load — for each plot widget, reset its ring
and re-append the window's `(t, v)` pairs via the same `appendDecimated` / ring-append calls
the ingest path uses (absolute-grid TimeRing semantics preserved by reusing the exact append
primitive), values numeric-parsed on the fly from the player's row storage via the export
schema's uid→column map; (b) one lane-1 inject of the cursor row so gauges/bars/LEDs/DataGrid
track the cursor; (c) timestamp display update. A debounced (~250 ms) at-rest pass replays
the full trailing window through lane 1 (`clearPlotData()` first), making every widget —
including FFT/waterfall/GPS/3D push tables — exact at the resting position. Plot clocks and
the display clock reset at each seek so a subsequent play resumes cleanly from the recorded
timeline.

## Hotpath & threading impact

- **Touches the hotpath?** Yes — `FrameBuilder` (new entry point) and `Dashboard` (bulk ring
  load). The live-device lanes are untouched: no changes to `FrameReader`, `CircularBuffer`,
  the span fast lane, or `hotpathTxFrame`'s live fan-out. The new entry reuses
  `acquireFrame` (slot pool — no per-frame allocation, `structureGeneration` stamped) and
  calls `Dashboard::hotpathRxFrame` directly (no new signal hops). Bulk ring loads run at UI
  cadence on the main thread, bounded by window × plot-enabled datasets. `ss-hotpath`
  invoked; `--benchmark-hotpath` all-tiers re-run is AC4.
- **New cross-thread signal/slot?** No. Players, FrameBuilder, and Dashboard are all
  main-thread; new calls are direct function calls. The two new QTimers (coalesce, settle)
  are main-thread single-shots.
- **New input to a cached hotpath flag?** No new flags and no new inputs: `m_playerOpen`,
  `m_streamAvailable`, and the capture/watchdog gating from 2026-07-18 are reused as-is.
  The replay entry asserts `m_playerOpen` instead of re-deriving it.
- **Timestamp ownership** — recorded timestamps are stamped by the player (the recording is
  the source) at the entry boundary; Dashboard/observers never re-stamp. This also fixes
  today's wall-clock stamping of replayed frames.

## Data model & persistence

None. No `Keys::` additions, no schema or writer-version changes, no widgetSettings changes.
Session DB is read with an additional windowed query shape only.

## API / SDK surface

No new commands. Existing player handlers (`CSVPlayerHandler`, `MDF4PlayerHandler`, session
player commands) keep their surface; `setProgress` semantics (coalesced scrub) are
behavior-compatible. API frame-stream observers continue to receive replayed frames
(tradeoff below); recording sinks stop receiving them (R6).

## QML / UI

Expected none (sliders already emit continuous `onMoved`). If the settle pass needs an
explicit end-of-gesture signal, the three player dialogs gain a `pressed`-binding one-liner —
named here so it is not lane creep.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Drag-scrub mechanism | (a) budgeted pipeline-only; (b) bulk plot-ring fill + at-rest pipeline pass; (c) precomputed numeric matrix | **(b)** — only shape that meets the 50 ms drag budget at the field project scale without decimation (a: ~100 ms+/window even after speedups) and without unbounded memory (c: ~GBs at 10M rows). |
| Replay publish targets | (a) dashboard only; (b) dashboard + read-only observers (API/gRPC), no recorders/MQTT; (c) full fan-out with per-sink replay gates | **(b)** — R6 by construction while SDK/API consumers watching a replay keep working; (c) spreads the gate across five sinks and invites silent re-record regressions. |
| At-rest rebuild scope | (a) plots only; (b) full window through lane 1 | **(b)** — spec Q3 requires FFT/waterfall/GPS/3D correctness at rest; one ~100–200 ms pass per gesture is within intent (budget applies to the drag). |
| Catch-up batching | (a) bigger fixed batch; (b) wall-clock-budgeted batch (~20 ms/pass) | **(b)** — a fixed batch either starves throughput or blocks the GUI; a time budget adapts to project width and machine speed, and stretching (spec Q2) falls out naturally. |
| Scrub data source | (a) new per-recording numeric cache; (b) read the players' existing row storage on the fly | **(b)** — zero added memory (spec constraint); toDouble-on-demand is ~ns-scale per cell. |

## Risks & mitigations

- **TimeRing envelope semantics** (absolute grid, phase-stable lattice — see
  [architecture/dashboard.md](../../architecture/dashboard.md) and the time-ring memory):
  bulk fill must reuse `appendDecimated` verbatim on a reset ring, never hand-roll cells —
  mitigated by feeding the exact ingest primitive in recorded-time order.
- **Plot/display clock continuity** (`m_plotClocks`, `m_plotDisplayTimeSec` never run
  backwards): a backward seek must reset per-source clocks or resumed playback compresses
  onto one decimator cell — explicit clock-reset step at seek, covered by AC1's
  scrub-then-play assertion in the test.
- **uniqueIds are NOT unique across sources** (per-source plot-clock memory, 2026-06):
  all uid→column maps stay per-source (`m_replayColumnMap` already is; the bulk-fill
  provider mirrors it).
- **Push-table staleness** (`m_layoutValid` contract): bulk loads write rings only and
  never touch push tables; any layout change still routes through reconfigure.
- **Silent re-record** (R6): lane 1 never calls recording sinks; AC5 asserts it from the
  outside. Sessions `table_snapshots` capture is 1 Hz main-thread — confirm it no-ops
  while replaying (player open ⇒ capture flag off since 2026-07-18).
- **Sessions windowed query cost** on multi-GB sessions: bounded by window size and served
  by the covering index; ties broken with `reading_id` (never `DISTINCT timestamp_ns`).
- **Live hotpath regression**: no live-lane edits, but AC4 (`--benchmark-hotpath`, all nine
  tiers) is the backstop.

## Test & verification plan

- **Unit (I can run):** none applicable (no JS-parser change).
- **Integration (maintainer runs, app up with API server):**
  - `tests/integration/test_replay_timeline.py` (new):
    - AC1 — open recording via API, seek P1 → P2 < P1, `dashboard.tailFrames` equals a
      fresh-open direct-seek-to-P2 baseline; then play-after-backward-scrub sanity.
      Parametrized CSV + MDF4; sessions variant runs on commercial builds.
    - AC3 — generated constant-rate wide CSV; assert timestamp pace within tolerance and
      dashboard-delivered frame count == recorded rows (lossless).
    - AC5 — with CSV export enabled, replay + scrub; assert no export file growth and no
      new session rows.
- **Maintainer observation:** AC2 — The field project file + a real capture: both-direction live
  scrubbing, no multi-second freeze, event findable by eye.
- **Hotpath:** AC4 — `--benchmark-hotpath` all gated tiers on the built binary.
- **Static:** `python scripts/code-verify.py --check` on all touched files;
  `qt-cpp-review` before handoff; `python scripts/sanitize-commit.py` before commit.
