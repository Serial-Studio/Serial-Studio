---
spec: 0020-replay-timeline
title: Replay timeline rework — tape-style scrubbing and lossless catch-up
status: done         # draft -> approved -> in-progress -> done | shelved
# AC1/AC3/AC5 covered by tests/integration/test_replay_timeline.py (maintainer runs);
# AC2 (BADAQ scrub observation) and AC4 (--benchmark-hotpath all tiers) verified by the
# maintainer on the next build.
created: 2026-07-19
author: Alex Spataru
---

# Spec 0020 — Replay timeline rework

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

Replaying a recording is how users find events after the fact, and both halves of that
workflow degrade badly on real projects. Ground truth: BADAQ.ssproj (88 groups, 571
datasets, one recorded row per CAN message) replayed from CSV or MDF4 on 2026-07-18.

1. **Scrubbing is unusable.** Earlier releases let the user drag the timeline and watch
   the plots "play" under the cursor — the primary way to spot an event visually and jump
   to it. Today every slider movement discards all plot data and synchronously rebuilds a
   full plot window through the general parse pipeline; on BADAQ-scale projects one tick
   costs hundreds of full-pipeline frame injections, the UI freezes for seconds, and the
   live-scrub feel is gone entirely.

2. **Playback cannot keep pace.** When the recording's frame rate exceeds what the replay
   pipeline can process, the catch-up loop replays every intermediate row in bounded
   batches and falls further behind forever: a recording plays in permanent slow motion.
   Session replay only feels better because its frame count collapses same-millisecond
   bursts, not because its pipeline is faster.

The maintainer's direction (2026-07-19): scrubbing must behave **like a tape** — fast in
both directions, with plots always reflecting the cursor position (scrubbing backward
visibly rewinds points) — and catch-up must become fast enough to stay lossless; skipping
recorded data is not acceptable.

## Goals

- Dragging the replay timeline updates the visible plots continuously, in both directions,
  smoothly enough to visually locate an event on a BADAQ-scale project.
- At any resting timeline position, every plot shows exactly the recorded samples of its
  trailing window ending at that position — identical whether the position was reached by
  scrubbing forward, scrubbing backward, or playing.
- Normal playback holds the recording's own pace on BADAQ-scale projects without dropping
  or skipping any recorded row, and recovers losslessly from transient stalls.
- All three replay sources (CSV, MDF4, session database) behave the same way.

## Non-Goals

- No change to live-device streaming behavior, dashboards, or exports outside replay.
- No decimation, sampling, or skip-ahead of recorded data anywhere in replay (explicitly
  rejected by the maintainer).
- No new playback features (speed multipliers, loop regions, bookmarks/event markers) —
  this rework restores and hardens what existed.
- No change to recording/export formats or their writers.
- No redesign of the player dialogs beyond what scrubbing behavior requires.

## Requirements

1. **R1 — Position accuracy (tape semantics).** When the timeline rests at position P,
   every plot shows exactly the recorded samples in its trailing window ending at P, with
   no leftover samples from positions after P (backward scrub rewinds) and no gaps from
   positions before the window. This holds regardless of how P was reached.
2. **R2 — Live scrub feedback.** While the user drags the timeline on a BADAQ-scale
   project, plots visibly update at an interactive rate (multiple distinct updates per
   second) and the UI never hard-freezes; non-plot widgets (gauges, bars, LEDs) track the
   cursor's current frame.
3. **R3 — Paced playback.** Playing a BADAQ-scale recording advances the displayed
   timestamp at the recording's own rate: after N seconds of wall-clock playback the
   timeline position is within a small fixed tolerance of N seconds of recorded time,
   with no unbounded drift.
4. **R4 — Losslessness.** Over a completed playback run, every recorded row is processed
   and delivered to the dashboard exactly once — none skipped, none duplicated — even when
   transient stalls force catch-up.
5. **R5 — Parity across players.** R1–R4 hold identically for CSV, MDF4, and session
   replay (session replay in commercial builds).
6. **R6 — Replay never re-records.** Scrubbing and playback must not append to any export
   sink (CSV/MDF4/session/MQTT publish) — jogging a tape must not create a new recording.

## Acceptance Criteria

- [x] **AC1** — Integration test (API-driven player): open a known recording, seek forward
  to P1, then backward to P2 < P1; `dashboard.tailFrames` returns exactly the recorded
  trailing-window samples for P2, matching a fresh open + direct seek to P2. Covers CSV and
  MDF4; session replay verified by the same script on a commercial build.
- [x] **AC2** — Maintainer observation on BADAQ.ssproj + a real capture: dragging the
  timeline in both directions shows plots updating live with no multi-second freeze; an
  event visible in the recording can be located by scrubbing alone.
- [x] **AC3** — Integration test: replay a generated constant-rate recording of
  BADAQ-scale width; assert the reported timestamp tracks wall-clock within tolerance and
  the number of frames delivered to the dashboard equals the number of recorded rows
  (losslessness + pace together).
- [x] **AC4** — `--benchmark-hotpath` still passes every gated tier (the live parse path
  must not regress in service of replay).
- [x] **AC5** — Integration test: with CSV export enabled-then-replaying and during
  scrubbing, no export file grows and no session rows are written.

## Constraints & Invariants

- The live-device hotpath keeps its throughput gates (256 kHz reference, all nine
  benchmark tiers); replay-side speedups must not add work, allocation, or locking to the
  live lanes.
- The cached-hotpath-flag discipline holds: any new input that influences a cached flag
  wires its change signal to the cache refresh.
- Transforms, the frame-parser script, the control script, and the script watchdog remain
  inert during replay (state restored on player close), as fixed on 2026-07-18.
- Recorded values are final values; replay must never re-run dataset transforms.
- Feature availability is unchanged: CSV replay stays GPL, MDF4/session replay stay
  commercial-gated.
- Any per-recording precomputation must be memory-bounded and must not meaningfully slow
  file open (recordings up to the existing 10M-row cap must remain openable).
- Plot window capacity semantics (`points` per dataset) are unchanged.

## Open Questions

All resolved by the maintainer, 2026-07-19:

- **Q1 — Scrub stall budget:** approved as proposed — no single UI stall above ~50 ms
  during a drag gesture on the BADAQ reference recording.
- **Q2 — Permanently underpowered machines:** confirmed — playback stretches time
  (lossless) when hardware cannot hold the recording rate; skipping stays forbidden.
- **Q3 — Widgets beyond plots during drag:** confirmed — at-rest correctness is
  sufficient for FFT/waterfall/GPS/3D; plots and scalar widgets update live.
