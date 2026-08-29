---
spec: 0023-export-pacing
title: Scheduled dashboard ticks and CSV interval-snapshot logging
status: done          # closed 2026-08-20
created: 2026-07-20
author: Claude (with Alex Spataru)
---

# Spec 0023 — Scheduled dashboard ticks and CSV interval-snapshot logging

> **Phase 1 of 4 — the WHAT and the WHY.** Gate: approved by the maintainer in chat on
> 2026-07-20 (design direction selected interactively; spec and plan reviewed together).

> **Post-implementation note (2026-07-20):** R1/R2 (deferred + coalesced `dashboardTick()`,
> `scheduled` response) were **reverted** after landing. Coalescing to one republish per
> UI-timer window decimated smooth per-frame control-script curves (a Lorenz attractor
> rendered jagged). `dashboardTick()` is synchronous again and reports `published`. The CSV
> interval-snapshot logging (R3-R5) stays as the export-rate bound.

## Problem / Motivation

The BADAQ test-cell project (two sources: a 500 kbit/s CAN bus feeding ~582 table-driven
datasets, plus a 48 kHz audio source for IEPE vibration) produced a **2+ GB CSV in ~5
seconds**, mostly empty or repeated cells. Ground truth from the code:

- Every published frame becomes one CSV row (`CSV::ExportWorker::processItems`), and the
  audio driver publishes **one frame per sample** — 48 000 rows/s — with no coalescing
  anywhere on the export side (the dashboard survives only because repaints are gated by
  the 60 Hz UI timer).
- The CSV schema is the union of all datasets across all sources (~583 columns), but each
  frame is per-source, so every audio row carries 582 forward-filled/empty CAN cells.
- A control script calling `dashboardTick()` per received frame adds up to ~1 000 more
  rows/s, each call doing a full synchronous republish through a
  `BlockingQueuedConnection` round-trip.

CSV is the wrong container for sample-rate vibration data (that belongs in MDF4 / the
session DB, which are sparse); what a test cell needs from CSV is a **paced trend log**.
And `dashboardTick()` needs to be safe to call at arbitrary rates from control scripts.

## Goals

- A control script may call `dashboardTick()` per received frame at any rate without
  multiplying export rows or blocking its loop on a full synchronous republish; republish
  work is bounded by the UI refresh rate.
- CSV export can operate as a fixed-interval snapshot logger: one forward-filled row of
  the full schema every N ms, regardless of frame rate.
- Default behavior is unchanged: per-frame CSV logging remains the out-of-the-box mode,
  and projects without control scripts see no behavioral difference except tick pacing.
- The BADAQ project records a usable CSV trend log while audio-rate data still reaches
  MDF4/session sinks at full rate, and its CAN watchdog + render tick stay driven by the
  CAN source rather than the 48 kHz audio stream.

## Non-Goals

- No per-source CSV files (rejected in favor of interval mode; may become a later spec).
- No pacing/decimation for MDF4, Sessions, MQTT, or API sinks — full-rate recording there
  is correct for vibration work.
- No change to `dashboard.reprocess` / `refreshDashboard()` — it stays synchronous and
  remains the escape hatch for scripts that need tick-then-read-back semantics.
- No CSV file rotation by size/time (worth a separate spec if wanted).
- No project-file (.ssproj) key for the CSV interval — it is an application setting, like
  `CSVExport` itself.

## Requirements

1. **R1** — Calling `dashboard.tick` (SDK `dashboardTick()`) schedules a coalesced
   republish that runs in the next UI-timer window; N calls inside one window produce at
   most one republish. The republish still feeds export sinks (`feedExports = true`).
2. **R2** — The `dashboard.tick` API responds immediately with `scheduled: true/false`
   (false only when ProjectFile mode / a loaded project is missing) instead of blocking
   until the republish completes.
3. **R3** — A new CSV export interval setting (`0` = per-frame, default; `> 0` =
   snapshot every N ms) is persisted in QSettings, settable from the Setup pane UI and
   the `csvExport.*` API, and applied live without restarting the recording.
4. **R4** — In interval mode, arriving frames only update the worker's last-values map;
   rows are written by the worker's snapshot timer with a monotonic elapsed-seconds
   timestamp, after draining any queued frames so cells are never staler than the queue.
5. **R5** — In interval mode no row is written before the first frame of the session
   (file creation stays lazy, on first data).

## Acceptance Criteria

- [x] **AC1** — With CSV interval = 100 ms and the BADAQ project running (CAN + audio),
  the CSV grows at ~10 rows/s x ~583 columns (order of 100 KB/min, not GB/5 s), while
  MDF4/Sessions still record per-sample. (Maintainer observes in the running app.)
- [x] **AC2** — A control script calling `dashboardTick()` in a tight loop produces at
  most `uiRefreshRate` export rows/s on the tick path, and the script loop is not blocked
  by republish work. (Maintainer observes; `tests/integration/` control-script suite.)
- [x] **AC3** — With interval = 0 the CSV byte stream for a single-source project is
  identical to today's output. (Existing `pytest` CSV export integration tests pass
  unchanged.)
- [x] **AC4** — `--benchmark-hotpath` gates pass unchanged (the per-frame CSV path gains
  only a cached-mode branch; the parse pipeline is untouched).
- [x] **AC5** — `csvExport.getStatus` reports the interval; `csvExport.setInterval`
  changes it live; values persist across restart.

## Constraints & Invariants

- Must not regress the 256 kHz hotpath CI gate or the Lua exporter/dashboard 0.5x floor
  rows.
- No allocation and no Frame copies added to the dashboard path; the async-sink detached
  copy remains the only per-frame copy.
- `FrameReader`/`CircularBuffer` untouched; no new mutexes; all new signal hops between
  main-thread objects are direct (`Qt::AutoConnection` same-thread or explicit
  `Qt::DirectConnection`); worker-side state is touched only from the worker thread via
  queued invokes.
- Source owns time: snapshot rows use the worker's monotonic clock against the session
  reference timestamp; nothing re-stamps frames.
- GPL feature surface: CSV export and `dashboard.tick` are not commercial-gated; no
  `BUILD_COMMERCIAL` changes.

## Open Questions

None — CSV strategy (interval snapshot vs per-source files) and tick export fan-out
(coalesced but kept) were decided with the maintainer on 2026-07-20. The BADAQ script
mitigation was chosen as "gate the per-frame tick on the audio source", but
implementation review found that gating the tick off would freeze and stop recording the
CAN datasets: the BADAQ CAN parser writes only data tables and returns no channels, so
the table-driven CAN datasets render/record exclusively via the tick. The mitigation was
corrected to pin the watchdog and tick trigger to the CAN source (sourceId 0) — which
achieves the actual goal (decoupling the tick from the audio firehose) without the
freeze — and to keep ticking every CAN frame, relying on the coalescing (this spec) and
CSV interval mode to bound output. Recorded in the tradeoff table in plan.md.
