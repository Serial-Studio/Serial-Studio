---
spec: 0064-export-replay-fidelity
title: Export and Replay Fidelity
status: done          # closed 2026-08-20
created: 2026-08-18
author: Alex Spataru
---

# Spec 0064 — Export and Replay Fidelity

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

A production capture of the BADAQ project (109 groups, 635 datasets, three sources: one CAN/UDP
source whose 631 datasets are script-driven and table-fed, plus two 48 kHz audio sources with four
datasets between them) records almost nothing. Of 635 datasets, exactly **four** — the audio ones —
reach any recording sink. This is true of all three sinks simultaneously, measured on the files
themselves:

- The exported CSV has 636 columns and only 4 are ever non-empty.
- The exported MDF4 has 109 channel groups and 107 of them contain zero cycles.
- The session database records blocks for 4 unique dataset ids; its readings table is empty,
  while its captured raw bytes and table snapshots prove the source was alive and parsing.

The same project exported correctly one month earlier: a 2026-07-18 CSV of this project has 571 of
572 columns populated. The behavior changed with the pooled block-lane unification that landed on
2026-08-18 (`f4e26ef04`), which reworked how a synthetic, script-requested refresh is published.
That refresh is the only publication path a table-fed virtual dataset has — the project's control
script requests one per received frame specifically so those datasets render *and* record.

Replay is broken in three separate ways on top of that. A Serial Studio-generated CSV of this
project cannot be opened for replay at all: the player refuses its own file's elapsed-time column
and falls back to prompting the user to nominate a time column or a fixed interval. The cause is
that the exporter writes a **negative** first elapsed value — it measures elapsed time from the
first block it happens to see rather than from the earliest sample in the recording, and with two
sources the first block seen is 32 ms later than the earliest sample. MDF4 and session replay both
load and run, but the dashboard stays empty because the recordings themselves are empty for all but
the four dense-stream datasets.

The generated session report shows the same hole from a different angle. It lists every dataset the
recording declares — all 635 — but plots only the four dense-stream ones. A group such as APS500
appears in the report with no plot data behind it, because the report is a downstream reader of the
same recorded samples that were never written. Anyone reading the report sees a complete-looking
inventory of channels and an almost-empty set of charts, which is worse than an obviously truncated
report: it invites the reader to conclude the instrument was silent.

The user-visible result is that a long, expensive test capture is unrecoverable: gigabytes on disk,
almost no data in them, no way to replay what little is there, and a report that misrepresents the
run. Every symptom is a data-fidelity failure, so this spec treats them as one pass.

## Goals

- Every dataset that renders on the live dashboard is also written to every enabled recording sink,
  including datasets whose values are produced by a script or read from a data table rather than
  parsed directly out of a frame.
- A recording made by Serial Studio can always be replayed by Serial Studio without the user being
  asked to describe the file's own time column.
- Replaying a CSV, MDF4, or session recording of a project restores the dashboard to the same set
  of populated datasets that were live when it was recorded.
- Elapsed time in an exported recording starts at zero and increases, so it is meaningful to a
  human reading the file and to any third-party tool that opens it.
- A generated session report plots every dataset it lists, so a reader can trust that an empty chart
  means an idle channel rather than a lost one.
- The failure modes above are locked by automated regression coverage at three tiers, so a future
  change to the publication path cannot silently empty a recording again.

## Non-Goals

- **Reducing recording file size or row rate.** The 636-column-wide sparse row emitted per distinct
  sample instant costs roughly 60 MB/s on this project, and two independent 48 kHz sources produce
  ~93k rows/s rather than ~48k. That is a real problem and it is deliberately out of scope here;
  it gets its own spec. This pass is correctness only, and correctness will make the files
  *larger*, not smaller.
- **Changing the MDF4 channel layout.** MDF4 keeps writing a raw-value companion channel for every
  dataset and a time channel per group. Raw values stay recoverable from every recording.
- **Introducing a new on-disk export format** — narrow/long CSV, per-source files, or any layout
  change that existing recordings or third-party tools would have to adapt to.
- **A user-facing way to choose which datasets get recorded.** Worth having, not this pass.
- **Changing the session database schema or its capture format version.** Existing session files
  must keep opening.

## Requirements

1. **R1** — Every dataset present in a project's live dashboard is recorded by every enabled sink,
   with no dependence on how its value was produced. A dataset whose value comes from a script or a
   data table is recorded exactly like one parsed straight from a frame.
2. **R2** — A recording that captures a source at all captures every one of that source's datasets.
   A recording must never contain a source's structure with no samples behind it while that source
   was live.
3. **R3** — Opening a Serial Studio-generated CSV for replay never prompts the user to identify a
   time column or supply a fixed interval. This holds for any elapsed value the exporter is capable
   of writing, including zero and negative values in files already on disk.
4. **R4** — The first row of a newly exported CSV has an elapsed value of zero or greater, measured
   from the earliest sample in the recording, and elapsed values do not decrease. This holds when
   several sources with independent clocks are recorded together.
5. **R5** — Replaying an MDF4 recording of a project populates the dashboard with every dataset the
   file contains, matched to the correct dataset, for both single-source and multi-source
   recordings.
6. **R6** — Replaying a session recording populates the dashboard with every dataset the recording
   contains, in the current capture format.
7. **R7** — A synthetic refresh that feeds the recording sinks reports itself as published, so the
   documented "republish only when a value changed, or on first publish" suppression behaves as
   specified instead of republishing unconditionally.
8. **R8** — Replaying any recording never re-records it. Replay populates the dashboard and the
   read-only observers and produces no new file, session, or published message.
9. **R9** — Recordings written before this change still open and replay. No recording made by a
   released build becomes unreadable.
10. **R10** — A generated session report plots every dataset it lists. Any dataset the report
    enumerates in its channel inventory has plot data and summary statistics behind it, for
    script-driven and table-fed datasets exactly as for dense-stream ones. A report must never
    present a populated channel list against charts drawn from only a subset of the sources.

## Acceptance Criteria

- [ ] **AC1** (R1, R2) — A new C++ ctest round-trip suite builds a synthetic multi-source project
      containing script-driven/table-fed datasets alongside dense-stream datasets, drives a
      recording through the publication path, and asserts that every dataset appears in the written
      CSV and MDF4 with the expected sample count. The suite fails on today's code.
- [ ] **AC2** (R3, R4) — The same ctest suite reads the CSV it just wrote back through the replay
      timestamp detection and asserts it is accepted as a numeric elapsed column with no prompt,
      and that the first elapsed value is >= 0 and the column is non-decreasing. A separate case
      feeds a file whose first elapsed value is negative (matching the recordings already on disk)
      and asserts it is likewise accepted.
- [ ] **AC3** (R5) — A ctest case writes an MDF4 recording of a multi-source project, reads it back
      through the replay channel mapping, and asserts each channel's values land on the dataset they
      were recorded from. A case with a project whose dataset ordering differs from its identifier
      ordering is included, because that is where the BADAQ project diverges.
- [ ] **AC4** (R6) — A ctest or pytest case records a session in the current capture format,
      reopens it, and asserts the recording contains blocks for every dataset and that replay
      restores them. Recording only the dense-stream datasets fails the check.
- [ ] **AC5** (R1, R2, R8) — A pytest integration test drives a running app over the API: loads a
      project with script/table-fed datasets, records, stops, reopens each recording, and asserts
      the dashboard reports the same populated dataset set as the live capture and that no new
      recording file or session row was created during replay.
- [ ] **AC6** (R7) — A ctest case asserts that a synthetic refresh which feeds the sinks is reported
      as published, and that a second refresh with no changed value is suppressed.
- [ ] **AC7** (R9) — The replay suites open at least one fixture recording produced by the current
      released format and assert it still loads and replays.
- [ ] **AC8** (R10) — A test generates a report from a recorded session containing both
      script/table-fed and dense-stream datasets, and asserts that every dataset the report lists
      also carries plot data and summary statistics. A report that lists a dataset with no samples
      behind it fails the check.
- [ ] **AC9** — Maintainer check on the real BADAQ project: record a short capture, then replay the
      CSV, the MDF4, and the session recording, and generate the session report. Each replay opens
      without a time-column prompt and shows a populated dashboard; the report plots the APS500 and
      CAN groups, not only the vibration channels.
- [ ] **AC10** — `--benchmark-hotpath` still clears every tier at its default rate on the
      PGO-optimized binary. No gate regresses.

## Constraints & Invariants

- **One publication payload, one ingestion path.** Whatever the fix is, it must not reintroduce a
  second publication payload type or a second producer for the sinks. The single-producer rule for
  every sink stands.
- **No allocation on the publish path**, and no per-frame queued emission between the pipeline and
  the GUI. A fix that restores fidelity by copying per frame is not acceptable.
- **No rate cap and no per-view reduction** may be introduced to make the numbers smaller. Dropping
  data to fix a data-loss bug is not a fix. Overruns still drop whole blocks and still count them.
- **The 256 kHz hotpath gate is a hard CI gate** and must not regress.
- **Replay must stay masked from the recording sinks.** Restoring fidelity must not open a path
  where replaying a file re-records it.
- **Both operation modes.** The behavior must hold in ProjectFile mode and must not break QuickPlot
  replay, which works correctly today and is the control case.
- **No on-disk format change** to CSV, MDF4, or the session database, and no capture-format version
  bump. Files already on disk must keep opening.
- **The exporter, not the reader, owns time.** Timestamps are stamped at the source boundary; the
  fix for negative elapsed values corrects which sample the recording is measured *from*, and must
  not re-stamp samples in an export worker.
- **Tests must run where CI can run them.** The C++ tier runs headless under ctest with no device
  and no GUI; the pytest integration tier may require a running app, and must be marked so it is
  skipped when one is not available.

## Open Questions

- The precise step at which a synthetic refresh's block is dropped before the async sinks has not
  been isolated. Static reading narrowed it to the republish/stage/flush path introduced by
  `f4e26ef04` and left two candidates: the cached "any async sink" flag read on the publish path,
  and the column binding used for a republished frame. This must be confirmed against a running app
  before `/ss-plan` commits to a fix, per the repo's ground-truth-over-on-paper-reasoning rule.
  **Resolution owner: maintainer runs the app with the BADAQ project so the drop point can be
  instrumented directly.**
- Recordings already on disk (the BADAQ CSV, MDF4, and session files) contain only four datasets and
  cannot be repaired by this change. Should the spec require any user-visible signal when a
  recording is opened that contains structure for sources with no samples — a warning on open —
  or is silent replay of a sparse recording acceptable? **Recommendation: out of scope; a warning
  belongs with the size/robustness spec.**
- Whether the ctest round-trip suite should also assert byte-level stability of the exported CSV
  header and MDF4 channel naming, which would catch accidental schema reordering but would need
  regenerating whenever a column label legitimately changes. **Recommendation: assert ordering and
  membership, not bytes.**
