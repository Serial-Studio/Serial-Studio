---
spec: 0054-sessions-stream-lane
title: Session recording for stream-lane sources
status: done
created: 2026-08-15
author: Alex Spataru
---

# Spec 0054 — Session recording for stream-lane sources

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

The session database silently records nothing for dense typed sources. Recording an audio
Quick Plot session on 2026-08-15 produced a database containing exactly one reading:

```
sessions|1
columns|1
readings|1
```

The single row is the synthesized structure frame published at connect. Every actual audio
sample — 48,000 per second — is absent. A simultaneous CSV export of the same capture wrote
4.3 million rows, so the data was flowing; the session database is the only export sink that
never receives it.

The cause is a gap left by the typed stream lane. When dense sources were moved off the frame
lane, the four other sinks (CSV, MDF4, the API stream subscription, and audio recording) each
gained a path for the new per-block payloads. Session recording did not. A user who enables
session recording for an audio source therefore gets a file that looks valid — it opens, it
lists a session, it replays — and contains none of their data. That is worse than an error,
because the loss is only discoverable after the capture is gone. It also makes the session
database unusable for the reproducibility guarantees it was built to provide, since a session
that recorded nothing trivially "verifies".

The same subsystem carries a second defect that is already crashing the app. Closing a session
restores the project that was loaded before the session opened, and it does that work inline.
When the session is closed by closing the window, that restoration runs *inside* window
destruction: the project reload rebuilds devices, which waits on the frame pipeline, which
spins a nested event loop, which re-enters the platform event dispatcher while the native
window is halfway through tearing down. The observed crash is the window manager updating
mouse-tracking regions against a view hierarchy that no longer exists. Both problems live in
the session subsystem and both are exercised by the same user action — record a session, then
close the app — so they are specified together.

## Goals

- A stream-lane source recorded to the session database produces a faithful, complete record
  of every sample it captured.
- Session recording works when it is the only export sink enabled — no other export needs to
  be switched on for data to arrive.
- Replaying such a session reproduces the recorded signal at its original rate and timing.
- Session databases stay a practical size for sample-rate data, so a multi-minute capture is
  a file a user can keep, move, and open.
- Closing a session — including by closing the application window — never crashes and always
  leaves the application in the state it had before the session was opened.

## Non-Goals

- Changing how frame-lane sources are recorded. Their behavior, schema, and file size stay
  exactly as they are.
- Recording raw device bytes for stream sources. Dense typed sources have no frame-delimited
  byte stream to capture.
- Adding compression, downsampling, or retention policies to the session database.
- Reworking the database explorer UI beyond what R11 needs: showing that stream data exists,
  how much of it, and over what span. No waveform preview, no in-explorer plotting.
- Moving the CSV and MDF4 players onto the stream lane. Deferred to its own spec (2026-08-16)
  alongside the wider "stream sources only" direction; this spec covers the session player.
- Correlating or resampling sources onto a common clock beyond the shared session start
  reference R12 requires. Each source keeps its own rate.
- Changing which sources take the stream lane, or the per-source lane override.
- Making new session files readable by older builds.

## Requirements

1. **R1** — A stream-lane source that is recorded produces, in the session database, every
   sample it captured for every enabled dataset, with no gaps and no duplicates.
2. **R2** — Recorded stream samples are stored grouped by acquisition block rather than one
   database row per sample, so database growth and write cost scale with the number of blocks
   rather than the number of samples.
3. **R3** — Each recorded sample is recoverable with its original acquisition time, derived
   from the block's start time and sample interval.
4. **R4** — Enabling session recording is on its own sufficient to record a stream source.
   With every other export sink disabled, the capture still lands in the database.
5. **R5** — Replaying a session that contains stream data drives the dashboard with the
   recorded samples at their original rate, producing the same signal that was captured.
6. **R6** — Reproducibility verification reports a match for a session containing stream data.
7. **R7** — A session that contains no stream sources is recorded, replayed, and verified
   exactly as it is today, with no change in file size or behavior.
8. **R8** — Session databases written by earlier builds still open, replay, and verify.
9. **R9** — Closing a session never performs project restoration while a window is being
   destroyed.
10. **R10** — Closing a session leaves the previously loaded project and operation mode
    restored, regardless of whether the session was closed from the UI or by closing the
    window.
11. **R11** — A recorded stream source is visible as recorded data in the database explorer,
    with enough detail — which sources, how many samples, over what time span — for a user to
    confirm a capture succeeded without replaying it.
12. **R12** — In a session containing both frame-lane and stream-lane sources, every source
    replays at its own recorded rate, and all sources' timestamps are expressed against the
    same session start reference, so they are directly comparable on a single time axis.

## Acceptance Criteria

- [ ] **AC1** — Record 60 s of a 48 kHz audio source in Quick Plot with session recording on
      and CSV, MDF4, and API export all off. The resulting database contains recorded stream
      data covering the full 60 s, and the sample count matches the source's rate to within
      one block.
- [ ] **AC2** — For the AC1 capture, run the same capture simultaneously to CSV. Every recorded
      sample corresponds to a CSV row, in the same order, with none dropped or duplicated. CSV
      is the lossy side of this comparison — it writes ten significant digits while the database
      keeps full precision — so the exact check is that re-formatting the recorded sample at
      CSV's precision reproduces the CSV text. The database is never the side that rounds.
- [ ] **AC3** — The AC1 database is under 50 MB.
- [ ] **AC4** — Replay the AC1 session. The dashboard plots the recorded waveform, playback
      duration matches the original capture, and the reported timestamps advance at the
      original rate.
- [ ] **AC5** — The reproducibility verifier reports a match for the AC1 session.
- [ ] **AC6** — Record and replay a frame-lane (non-stream) session. Database size, contents,
      and replay behavior are unchanged from a build without this feature.
- [ ] **AC7** — A session database captured with the current shipped build opens, replays, and
      verifies without error.
- [ ] **AC8** — With a session player open, close the application window 20 times in a row
      across fresh launches. No crash.
- [ ] **AC9** — Load a project, open and then close a session via the UI: the original project
      is loaded and the operation mode restored. Repeat, closing via the window close button:
      same end state.
- [ ] **AC10** — `--benchmark-hotpath` passes every gated tier at its default thresholds, with
      no regression against the current baseline.
- [ ] **AC11** — The existing `pytest` session and integration suites pass.
- [ ] **AC12** — After the AC1 capture, the database explorer shows the stream source with its
      sample count and time span, without the session being replayed.
- [ ] **AC13** — Record a session with one frame-lane and one stream-lane source. Both replay
      at their recorded rates, and a value known to have been captured at a given moment
      reports the same session-relative time in both sources' recorded timestamps.

## Constraints & Invariants

- **Nothing crosses a thread boundary per sample.** Stream data moves in blocks only; this is
  the invariant that makes the stream lane viable at 48 kHz and it must not be weakened to
  make recording easier.
- **The source owns time.** Per-sample times derive from the block's start time and sample
  interval. Recording must never re-stamp samples with its own clock.
- **Storage is per block, not per sample** — the decision taken for R2. A block carries its
  samples as a packed array alongside the metadata needed to reconstruct each sample's time.
- **Samples are stored at full internal precision.** Recording must not narrow, quantize, or
  otherwise lossily encode a sample on its way to disk. Export exists to preserve the captured
  signal exactly; a recorded value and the value the pipeline produced compare equal, which is
  what lets AC2 and AC5 demand exact equality instead of a tolerance.
- **No database work on the GUI thread.** Session writes stay on their own thread, as today.
- **Must not regress the 256 kHz hotpath gate**, and must add nothing to the frame lane's
  per-frame cost for sessions that have no stream sources.
- **Pro-only**, following the existing session recording gating.
- **Reading older session files keeps working.** Writing files that older builds cannot read
  is acceptable (declared a non-goal); silently corrupting or failing to open an older file is
  not.
- **No nested event loop may run while a window is being destroyed**, and no new blocking wait
  may be introduced on the path from closing a session to restoring project state.
- **No new third-party dependency.**

## Open Questions

All three questions raised in the first draft are resolved and folded into the sections above:

- **Sample precision on disk** — resolved: full internal precision, no lossy encoding. Reducing
  precision would defeat the purpose of exporting. AC2 and AC5 therefore demand exact equality.
- **Visibility in the database explorer** — resolved as R11. A capture the user cannot confirm
  without replaying it is how this bug stayed invisible for a whole release; the explorer has
  to show that stream data landed. Scoped to counts and time span, not a waveform preview.
- **Mixed sessions** — resolved as R12. Each source replays at its own recorded rate and no
  resampling or correlation machinery is introduced; the only guarantee is a shared session
  start reference, so timestamps from both lanes are comparable on one axis. This is what
  "source owns time" already implies, made explicit rather than left to the plan.

None remaining. Ready for review.
