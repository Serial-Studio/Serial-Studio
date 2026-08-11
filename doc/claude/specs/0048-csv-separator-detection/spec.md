---
spec: 0048-csv-separator-detection
title: CSV player separator auto-detection
status: in-progress  # draft -> approved -> in-progress -> done | shelved
created: 2026-08-09
author: Alex Spataru
---

# Spec 0048 — CSV player separator auto-detection

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

The CSV player assumes every file is comma-separated. Many real-world logger exports are
not: the motivating file (`~/Desktop/Mazda/Parking Acc.csv`, a Mazda OBD transmission
log) uses semicolons — header `time(ms);RPM(1/min);TSS(1/min);OSS(1/min);LOAD(%);
TFT(°C);VSS(km/h)`, 778 rows. European tooling (Excel in most EU locales, many
dataloggers) writes semicolon-separated CSV by default; tab-separated exports are also
common. Opening such a file today collapses each row into one cell: the first data cell
reads as `0;803;-;-;-;-;-`, which is neither a number nor a date-time, so the player
falls into the interval/date-time prompt and produces a meaningless single-channel
playback instead of the seven channels the file actually contains. The user gets no hint
that the separator was the problem.

## Goals

- Opening a semicolon- or tab-separated telemetry CSV in the player produces the same
  channel breakdown, timestamp detection, and playback experience as the equivalent
  comma-separated file — with no extra user action.
- The Mazda file plays back as seven named channels with the `time(ms)` column driving
  the timeline.
- Comma-separated files — including every CSV Serial Studio itself exports — behave
  exactly as before.

## Non-Goals

- No user-facing separator picker or per-file override UI; detection is automatic and
  silent. (A picker can be a later spec if detection ever guesses wrong in practice.)
- No locale-aware decimal handling: cells using decimal commas (`35,69`) are out of
  scope; numeric parsing rules stay as they are today.
- No change to CSV *export* — Serial Studio keeps writing RFC-4180 comma-separated
  files.
- No change to how session-database or MDF4 replay interprets its own synthesized rows.
- No re-detection mid-file: one separator decision per opened file.

## Requirements

1. **R1** — When the player opens a CSV file, it determines the separator automatically
   from the file's leading content, choosing among comma, semicolon, tab, and pipe.
2. **R2** — Comma remains the default and wins ties: any file that parses plausibly as
   comma-separated (including all files Serial Studio exports) is treated exactly as
   today.
3. **R3** — A semicolon-separated file with a numeric first column (the Mazda log) is
   detected without prompting: correct column count from the header, numeric-timestamp
   mode, one channel per remaining column, empty/`-` cells handled as missing values
   exactly as they are for comma files.
4. **R4** — Detection respects quoting: separator characters inside RFC-4180
   double-quoted cells do not count as separators, for every accepted separator.
5. **R5** — The detected separator applies to the entire playback pipeline for that
   file — header naming, timestamp detection, row splitting during playback, seeking,
   and QuickPlot-mode injection — so no stage silently reverts to comma.
6. **R6** — Files where no candidate separator ever appears keep today's behavior
   (single-column handling and the existing "insufficient data" / prompt flows); the
   feature never makes a currently-loadable file fail to load.
7. **R7** *(2026-08-10 addendum — found on the real Mazda file)* — When the numeric
   timestamp column's header names a time unit (`time(ms)`, `t [us]`, `time_ms`, ...),
   playback pacing and the displayed timestamp use that unit converted to seconds,
   silently. A header with NO recognizable unit asks the user (seconds preselected;
   Enter or cancel keeps the legacy seconds reading) — maintainer decision 2026-08-10:
   ask instead of assume, mirroring the existing interval/date-time prompt. The Mazda
   log (`time(ms)`, ~16 ms row spacing) plays back in real time, not one row per 16+ s.

## Acceptance Criteria

- [x] **AC1** — Integration test: a semicolon fixture derived from the Mazda log opens
      via the API, reports the expected channel count and header names, and plays back
      values matching the file (pytest, running app). *(2026-08-10: green)*
- [x] **AC2** — Integration test: the same data as a comma file and as a tab file
      produce identical channel structure and values as the semicolon file.
      *(2026-08-10: green)*
- [x] **AC3** — Regression: an existing Serial Studio-exported comma CSV (quoted cells
      included) round-trips through the player unchanged relative to current behavior.
      *(2026-08-10: green, incl. quoted/unquoted-semicolon misdetection regressions)*
- [x] **AC4** — Quoting: a fixture with separator characters inside quoted cells splits
      into the correct number of columns for each supported separator.
      *(2026-08-10: green; tab titles display whitespace-simplified)*
- [x] **AC5** — Maintainer observation: opening the actual Mazda file shows seven
      channels named from the header and a timeline driven by `time(ms)`, with no
      interval/date-time prompt. *(2026-08-10: verified over the API — 777 rows, six
      data channels + timeline column, no prompt)*
- [ ] **AC6** — Hotpath: `--benchmark-hotpath` gates stay green (replay row splitting is
      shared with paths the benchmark exercises). *(pending: maintainer run, or the CI
      per-push benchmark gate)*
- [ ] **AC7** — Unit scaling: a `time(ms)` fixture reports a wall-clock duration matching
      the data (pytest); pressing play on the real Mazda file visibly advances the
      dashboard in real time, and opening a unitless numeric CSV shows the unit prompt
      with seconds preselected (maintainer observations — the prompt cannot be driven
      over the API).

## Constraints & Invariants

- **Serial Studio's own exports must parse byte-for-byte as before** — the detector must
  be unable to change the interpretation of a well-formed comma file. This is the
  deciding constraint.
- Synthesized replay rows (session database, MDF4, multi-source replay) are always
  comma-joined internally; their parsing must not become dependent on any per-file
  detection state.
- No per-row work may be added to shared row-splitting code beyond what an extra
  separator parameter costs; the 256 kHz hotpath gate must not regress.
- Detection runs once per file open, on the foreground quick pass — no second scan of
  the whole file.
- No new dependencies.

## Open Questions

- Should detection failure or a surprising choice (e.g. pipe) be surfaced to the user
  anywhere (status bar, log), or stay fully silent? Recommendation: fully silent now;
  revisit only if a real file misdetects.
- Is pipe (`|`) worth including as a candidate, or restrict to comma/semicolon/tab?
  Recommendation: include it — cheap, and some CLI tools emit it.
