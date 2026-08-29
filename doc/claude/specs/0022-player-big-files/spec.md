---
spec: 0022-player-big-files
title: Big-file CSV & MDF4 replay — stream and index instead of full materialization
status: done         # draft -> approved -> in-progress -> done | shelved
created: 2026-07-19
author: Alex Spataru
---

# Spec 0022 — Big-file CSV & MDF4 replay

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

The maintainer observes that the CSV and MDF4 players struggle far more than the session
player on big recordings. The cause is architectural: both file players materialize the
entire recording in memory at open time — the CSV player parses every row of the file into
one heap-allocated string object per cell (a 1 GB CSV becomes several GB of RAM and millions
of allocations, behind a hard 10-million-row cap that silently truncates anything larger),
and the MDF4 player decodes every channel of the whole file before the first frame plays.
Both do this work on the thread that opens the file, so a big file freezes the UI for the
whole parse. The session player proves the alternative: it keeps the recording on disk and
reads indexed windows on demand, which is why it scales to long sessions the file players
cannot handle.

A second, related cost lives in the replay hand-off: every player converts all values to
per-cell text before injecting them, so the MDF4 player formats binary numeric samples into
strings row by row during playback, and downstream consumers parse many of those strings
straight back into numbers. For numeric-heavy recordings this round trip is pure overhead
on every played frame.

## Goals

- Opening a CSV or MDF4 recording of any size returns control to the UI immediately;
  indexing/decoding continues in the background with visible progress.
- Playback and timeline scrubbing become available as soon as the indexed region covers the
  playhead, not only after the whole file is processed.
- CSV playback memory is bounded by a playback window, independent of file size; the
  10-million-row truncation cap is gone.
- MDF4 numeric channels are stored compactly (fixed bytes per sample, no per-sample string
  objects); decode happens off the UI thread.
- Numeric channels flow through replay as numbers end-to-end — no format-to-text /
  parse-from-text round trip per frame.
- Playback behavior on a file that both old and new code can handle is indistinguishable:
  same frames, same order, same recorded-timestamp pacing, same scrub feel.

## Non-Goals

- No change to the session player (it is the reference behavior, not a target).
- No change to any recording or export format (CSV schema, MDF4 layout, session DB).
- No new CSV dialect support (e.g. quoted embedded newlines that the current reader does not
  handle); parsing accepts exactly what the current reader accepts.
- No change to live acquisition, the parse hotpath, or recording sinks.
- No redesign of the player UI beyond surfacing background-indexing progress.
- No attempt at windowed (bounded-memory) MDF4 decode in this spec; MDF4 keeps a full
  in-memory decode, just compact and off-thread (per maintainer decision 2026-07-19).

## Requirements

1. **R1 — Instant open.** Opening a CSV or MDF4 file of any size leaves the UI responsive
   within perceptual immediacy (no multi-second freeze); a progress indication shows
   background indexing/decode advancing to completion.
2. **R2 — Early playback.** Play, pause, frame stepping, and scrubbing work as soon as the
   indexed region covers the requested position. Scrubbing beyond the indexed frontier
   clamps to the frontier and continues once indexing catches up.
3. **R3 — Bounded CSV memory.** During CSV playback, player-attributable memory stays
   bounded regardless of file size (target: a 1 GB, 10M-row file adds well under 500 MB
   beyond the OS file cache; today it adds multiple GB).
4. **R4 — No row cap.** CSV files larger than 10M rows play to the end; the current silent
   truncation is removed. Any remaining practical bound must be stated and reported to the
   user, never silent.
5. **R5 — Compact MDF4 storage.** Numeric MDF4 channels are held at fixed bytes per sample;
   per-sample string storage remains only for genuinely textual channels.
6. **R6 — Off-thread decode.** MDF4 decode and CSV indexing never run on the UI thread;
   closing the player (or the app) during background work cancels it cleanly.
7. **R7 — Numeric replay lane.** During replay, numeric channels reach dashboards without a
   per-frame number-to-text-to-number round trip. Textually recorded values (CSV cells,
   string channels) still display exactly as recorded — the numeric lane must never alter
   what the user sees.
8. **R8 — Behavior parity.** For a file within today's limits, the new players produce the
   same played frames, in the same order, at the same recorded-timestamp pacing, with the
   same scrub semantics (bulk window fill + settle replay), the same transform-skip rule,
   and recording sinks still never fed during replay.
9. **R9 — Timestamp flows unchanged.** Recorded timestamps remain the time source during
   replay; the timestamp-less-CSV interval prompt flow keeps working.
10. **R10 — Mode coverage.** Both QuickPlot and ProjectFile replay modes, single- and
    multi-source recordings, keep working in both players.

## Acceptance Criteria

- [x] **AC1** — Maintainer observation: opening a ≥1 GB CSV (≥10M rows) shows a responsive
  UI immediately, playback available within ~2 s, background index completing with visible
  progress; memory monitor confirms R3's bound. Same observation for a large MDF4 file.
- [x] **AC2** — A CSV with more than 10M rows plays past row 10,000,001 (currently
  impossible); the last row of the file is reachable by scrubbing to the end.
- [x] **AC3** — Parity check on a reference recording: with the same project and a mid-size
  CSV and MDF4 file, dashboards show identical values and the timeline behaves identically
  before/after the change (maintainer A/B, plus existing replay integration tests passing
  unmodified where they exist).
- [x] **AC4** — Textual parity: a CSV containing text cells and a recording with string
  channels display those values verbatim during replay (spot-checked against the raw file).
- [x] **AC5** — `--benchmark-hotpath` passes at its current gates (live path untouched).
- [x] **AC6** — Kill test: closing the player and quitting the app mid-index/mid-decode
  neither crashes nor leaves the UI wedged (maintainer observation, repeated).
- [x] **AC7** — Scrub-to-frontier: during background indexing, dragging the timeline past
  the indexed region clamps and then advances as indexing completes (maintainer
  observation).

## Constraints & Invariants

- **Replay semantics from spec 0020 are the contract**: recorded timestamps ride the frames,
  scrub = bulk window fill + debounced settle replay, transforms stay torn down during
  replay, recording sinks never see replayed frames. This restructure changes where the data
  lives and how it is typed, never these behaviors.
- **The live 256 kHz parse hotpath must be untouched or provably unaffected** (CI gate).
- **Displayed values are sacred**: replay must never reformat a recorded textual value.
  Numeric-lane values must render identically to today's replay of the same file.
- **No new third-party dependencies**; platform file-mapping and existing vendored parsers
  are fair game.
- **Tier gating unchanged**: MDF4 replay stays Pro; CSV replay stays free.
- **UI-thread discipline**: background work communicates through the established
  main-thread-safe patterns; no new locking on any path the live pipeline shares.
- **Cancellation safety** (R6) extends to reopening a different file while background work
  on the previous one is still running.

## Open Questions

- Exact numeric targets in R3/AC1 (memory bound, seconds-to-playable) should be confirmed
  on the maintainer's hardware once a prototype exists; the spec's numbers are intent,
  not contractual until then.
- Whether MDF4 progress can be meaningfully fractional (mdflib reads whole data groups) or
  is better presented per-group/indeterminate — resolve in plan with mdflib's actual read
  granularity.
