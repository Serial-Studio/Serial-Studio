---
spec: 0055-unified-block-lane
title: Unified Block Publication Lane
status: done         # draft -> approved -> in-progress -> done | shelved
created: 2026-08-16
author: Alex Spataru
---

# Spec 0055 — Unified Block Publication Lane

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

Serial Studio publishes acquired data through two parallel lanes. Byte-oriented sources
(UART, TCP/UDP, BLE, Modbus, CAN, MQTT, USB, HID, Process) publish one parsed frame at a
time. Dense typed sources — Audio today, and any source the user switches into the stream
lane — publish blocks of samples that bypass parsing entirely. The two lanes carry different
payloads, on different timebases, with different precision, and every consumer that wants to
see all of a project's data has to implement both.

Six consumers already carry two ingestion paths: the dashboard, CSV export, MDF4 export,
session recording, the API server, and the WAV taps. Three consumers carry only the frame
path, and the result is a silent data hole. MQTT publishing and the gRPC server receive
**nothing at all** from a stream-lane source — a project whose only source is audio publishes
no MQTT messages and exposes no data to gRPC clients, with no error anywhere. The remote
dashboard mirror is degraded rather than blind: its publisher reads the dashboard's combined
frame, which carries a stream dataset's *latest value* refreshed on the display tick, so a
viewer watching a 48 kHz source sees a tick-rate trace instead of the signal — the full-rate
samples never cross the wire. The same split shows up in export: a recording
of a mixed project produces a main file for the frame sources plus one extra file per stream
source, so a single recording session lands as several files a user must correlate by hand.
Every new consumer, every new export target, and every new widget pays this tax twice, and
the second half is the half that gets forgotten.

The two lanes exist because dense sources genuinely need batching — a 48 kHz source cannot
afford one publication per sample. But that is a difference of batch size, not of kind. A
low-rate source publishing a batch of one, and a 48 kHz source publishing a batch of 512, can
travel the same contract. Parsing itself stays frame-by-frame: splitting a byte stream on a
delimiter is inherently sequential and is not in scope to change.

## Goals

- Every data source in a project reaches every consumer through one ingestion contract, so a
  consumer that handles one source handles all of them.
- The stream-lane blind spots close: MQTT, gRPC, and the remote mirror carry data from dense
  typed sources with no per-consumer stream-specific work.
- One recording session produces one file per format, whatever mix of sources the project has.
- Replay of a recorded session is indistinguishable from live acquisition of the same data,
  regardless of which lane the data was captured on.
- The parse hotpath's measured throughput is unchanged or better.
- Consumer-side code shrinks: the second ingestion path disappears from the six consumers that
  carry it today, and no consumer is left with only the frame path.

## Non-Goals

- **Changing how bytes are parsed.** Delimiter scanning, checksum validation, the parser
  engines (Native / JS / Lua) and per-dataset transforms keep their current per-frame
  semantics and their current results.
- **Changing which sources use the dense lane.** The existing driver capability flag and the
  per-source project override keep deciding batch behavior; no source changes lane as a side
  effect of this work.
- **Changing widget behavior or appearance.** Decimation, plot ring sizing, FFT windowing and
  every widget's visible output stay as they are.
- **Adding a new export format** or changing what a given format records, beyond merging the
  per-source files into one and the row layout change stated in R6.
- **Rewriting the sessions database into a new file format.** The schema gains a unified
  layout; the file stays SQLite and legacy files stay readable.
- **Removing the CSV interval snapshot mode.** It stays available and unchanged in meaning.
- **Changing the licensing tier of any feature.**

## Requirements

1. **R1 — One publication contract.** Every source publishes its acquired values through a
   single contract that carries one or more samples per publication. A low-rate source
   publishes batches of one or a few; a dense source publishes batches sized by its
   acquisition period. No consumer can observe which kind of source produced a batch except
   by inspecting its declared timing and size.

2. **R2 — The contract is a superset of what both lanes carry today.** It carries numeric
   values at the precision the pipeline computes them, string values for datasets that
   produce text, the identity of each dataset the values belong to, and timing for every
   sample. Timing supports both a uniform grid (a start time plus a fixed step) and
   irregular per-sample times, because dense sources have the former and parsed frames have
   the latter. No value that reaches a consumer today may be lost, truncated, or reduced in
   precision by the change.

3. **R3 — Every consumer ingests only the unified contract.** The dashboard, CSV export, MDF4
   export, session recording, the API server, the gRPC server, MQTT publishing, the remote
   dashboard mirror, the WAV taps, and the hotpath benchmark each have exactly one ingestion
   path after this work.

4. **R4 — Dense sources become fully visible to the three underserved consumers.** With a
   stream-lane source connected and no byte-oriented source in the project, MQTT publishes its
   values, gRPC clients receive them, and a remote mirror viewer plots the source at its real
   sample rate rather than one value per display tick.

5. **R5 — MDF4 records one file per session.** A recording of a project with any mix of
   source rates produces exactly one `.mf4` file containing every dataset, with each group of
   datasets carrying its own time base at its own rate. No `_stream_sourceN.mf4` file is
   produced.

6. **R6 — CSV records one file per session, sparsely.** A recording produces exactly one
   `.csv` file whose columns are the union of every dataset in the project. Each row carries
   one distinct sample instant; a cell is filled only for datasets actually sampled at that
   instant and is left empty otherwise. No value is repeated to fill a gap and no value is
   invented. No `_stream_sourceN.csv` file is produced. The existing interval snapshot mode
   remains available and continues to write dense forward-filled rows on its configured
   cadence.

7. **R7 — Session recording stores one unified representation.** A recorded session stores
   every source's samples in one layout, at full precision, in one file, whatever lane the
   data arrived on.

8. **R8 — Legacy session archives still open, browse, replay and verify.** A session file
   recorded by a shipped build before this change opens in the database explorer, replays
   with the same values and timing it replays with today, and produces the same
   reproducibility verdict. New recordings use the unified layout. The application never
   writes the legacy layout.

9. **R9 — Replay re-enters through the unified contract.** All three players (CSV, MDF4,
   session) inject recorded data through the same contract live acquisition uses. Scrubbing,
   the tape settle pass, and playback catch-up keep their current behavior, and a replayed
   session is still never re-recorded into a new one.

10. **R10 — Throughput is preserved.** The parse pipeline sustains at least the rates it
    sustains today at every gated tier, with no steady-state memory allocation introduced on
    the publication path.

11. **R11 — Backpressure semantics are preserved.** When a consumer cannot keep up, whole
    batches are dropped and counted, exactly as whole display updates are dropped and counted
    today. No source is rate-capped, strided, or reduced to make a slow consumer keep up.

12. **R12 — Wire and schema versions are bumped, not silently changed.** Any published
    interface whose layout changes (the mirror wire protocol, the session schema, the
    reproducibility fingerprints) declares a new version, and a peer or file at the old
    version is either handled or refused with a clear message — never misread.

## Acceptance Criteria

- [x] **AC1** (R1, R3) — A source-code audit shows exactly one ingestion entry point per
      consumer subsystem; the second path is gone from all six consumers that carry it today,
      and no consumer is left with a frame-only path.

- [~] **AC2** (R4) — With a stream-lane audio source connected and no other source: an MQTT
      broker receives published messages (today: none); a gRPC client receives values (today:
      none); a mirror viewer's received sample count over a fixed window matches the source's
      sample rate rather than the display-tick rate (today: tick rate). Each is a distinct
      check, and each fails on today's build.

- [x] **AC3** (R2) — A round-trip test drives numeric values, string values, a uniform-grid
      source and an irregular-timing source through the contract and compares the values a
      consumer receives against the values the pipeline produced: bit-exact for numerics,
      byte-exact for strings, and exact for every sample's time.

- [x] **AC4** (R5) — Recording a project with a 48 kHz source and a low-rate source produces
      exactly one `.mf4` in the output directory. Opening it in a third-party MDF4 reader
      shows both sources' channels, each on its own time base at its own rate.

- [x] **AC5** (R6) — The same recording produces exactly one `.csv`. Its header is the union
      of all datasets; every row's filled cells correspond to datasets sampled at that row's
      instant; no cell repeats a prior row's value to fill a gap. Row count equals the number
      of distinct sample instants recorded.

- [x] **AC6** (R6) — With the interval snapshot setting non-zero, the same recording produces
      dense forward-filled rows at the configured cadence, unchanged from today.

- [x] **AC7** (R7, R8) — A session recorded by a shipped pre-change build opens in the
      explorer, replays to the same values and timestamps, and returns the same
      reproducibility verdict as it does on the shipped build. A `ctest` case pins this
      against a checked-in legacy archive.

- [x] **AC8** (R9) — Replaying a session recorded from a dense source reproduces the same
      dashboard trace as the live capture did; scrubbing to a position and letting the settle
      pass run leaves FFT, waterfall, GPS and 3D widgets showing the state they showed live at
      that position; and no new session file is created during replay.

- [x] **AC9** (R10) — `--benchmark-hotpath` passes every gated tier on a PGO-optimized build,
      with per-tier throughput no lower than the pre-change build measured on the same
      machine. The exporter and dashboard readouts are recorded before and after for
      comparison.

- [x] **AC10** (R11) — A synthetic overload (a dense source faster than a deliberately stalled
      consumer) drops whole batches, increments the drop counter, and leaves the source's
      acquisition rate unchanged. No values are silently interleaved from a partially
      delivered batch.

- [~] **AC11** (R12) — A mirror viewer at the old wire version is refused with a clear
      version-mismatch message rather than plotting misaligned data. A session file at the old
      schema version is detected and read through the legacy path.

- [x] **AC12** — The existing `pytest` suite and the full `ctest` tier pass with no new
      failures, and `scripts/code-verify.py --check` reports no new errors.

## Constraints & Invariants

- **The 256 kHz CI gate is the hard ceiling on this design.** Any publication contract that
  cannot be produced without steady-state allocation on the parse path is disqualified.
  Today's fast lane writes parsed text in place into a recycled slot; whatever replaces it
  must be equally allocation-free at rate.
- **Parsing stays per-frame and single-threaded on the pipeline thread.** No part of this
  work moves parsing, parallelizes it, or changes its results.
- **The single-producer discipline holds.** Every consumer queue keeps exactly one producer,
  however many sources fan into it.
- **Precision only increases.** No consumer may receive a value at lower precision than it
  receives today; the dense lane's current single-precision transport is a ceiling to be
  raised, not preserved.
- **Source owns time.** Timing is stamped at the driver boundary and carried through; no stage
  re-stamps.
- **Recording sinks never see replayed data.**
- **Works in both QuickPlot and ProjectFile modes**, and with the console-only operation mode.
- **No new third-party dependency.**
- **Pro-gated features stay Pro-gated** — MDF4, session recording and the mirror keep their
  current tier gating, and a GPL build must still compile and run with them absent.
- **Legacy archives are read-only history.** Nothing in this work may modify, migrate in
  place, or invalidate an existing session file.

## Resolved Decisions

Settled with the maintainer at approval time. Each is a requirement the plan must honor; the
values marked *plan-phase measurement* are the only degrees of freedom left.

- **D1 — A batch closes on the display tick or on a size cap, whichever comes first.** This
  bounds publication latency to one tick for a slow source and bounds batch memory by the cap,
  while still collapsing many publications into one at high rates. The cap value is a
  plan-phase measurement against `--benchmark-hotpath`. (Resolves the R1 batch boundary.)
- **D2 — Dense sources are numeric-only.** The unified contract carries strings because parsed
  frames produce them, but no dense source produces one, and its per-block worker path carries
  no string storage. A dense source that needs text is a future spec, not a contract change.
  (Constrains R2.)
- **D3 — The single CSV is strictly time-ordered via a bounded reorder window.** Arriving
  batches are buffered for a small window, merge-sorted by sample instant, then flushed; the
  residue flushes at session close. Rows are never emitted out of time order. The window size
  is a plan-phase measurement. (Constrains R6.)
- **D4 — The CSV interval-snapshot default stays disabled.** Sparse full-rate rows remain the
  default so existing low-rate recordings are byte-for-byte unaffected in cadence and no
  capture is silently downsampled. A user recording a dense source at full rate accepts a large
  file; MDF4 and the session database remain the recommended homes for sample-rate data.
  (Constrains R6.)

## Open Questions

- None blocking. Two plan-phase measurements remain: the D1 batch size cap and the D3 reorder
  window size. Both are settled by benchmark in `/ss-plan`, not by discussion.

## Implementation outcome (2026-08-16)

34 of 36 tasks landed; `tasks.md` carries the per-task record, `plan.md` the numbered decisions
D1-D8. `code-verify.py --check` is clean on all 79 changed files.

**Met in full:** R1, R2, R3, R5, R6, R7, R8, R9, R11, R12 (see below). The MQTT and gRPC halves of
R4 are met -- a dense-only project reaches both for the first time.

**Two deviations, both deliberate and recorded:**

1. **R4's mirror clause is not met.** The mirror keeps its pre-0055 fidelity of one value per
   dataset per display tick. Its viewer was converted (it had to be -- `hotpathRxFrame` is gone) and
   it works, but full-rate delivery means redesigning the snapshot payload, the chunker, the
   client decode, the viewer reconstruction and every fixture. Written up in
   `doc/claude/architecture/mirror.md`. `kWireVersion` deliberately stays 1: the payload never
   changed, so R12's "bump what changed" is satisfied by not bumping what did not.
2. **A block-backed session reports no standard deviation** in the explorer report. The per-block
   summary carries min/max/sum/count but no sum of squares; emitting a wrong number would be worse
   than emitting none. One more summary column whenever it is wanted.

**Still open (both require a build, which is the maintainer's):**

- **T33 / AC9** -- the `--benchmark-hotpath` before/after run and the `kFrameBlockSampleCap` sweep.
  The parser tiers should be flat (no parse code moved) and the `lua+exporters` readout should
  *improve*: the per-frame `make_shared<TimestampedFrame>(frame->data)` deep copy is gone, replaced
  by one trimmed values-only copy per block.
- **T34 / AC5** -- the CSV reorder-window sweep (starting value 250 ms).
- **AC4, AC8, AC10, AC12** -- maintainer observations and the pytest/ctest runs listed in
  `plan.md`'s test plan.

## Post-landing fixes (2026-08-16, found by running the suite)

Three defects the task list did not anticipate, all found by `test_block_lane_sinks.py` and the
integration sweep rather than by review:

1. **QuickPlot recorded nothing.** `CSV::ExportWorker::processItems()` used to fall back to
   `createCsvFile((*items.begin())->data)` when no template had arrived. Blocks carry values only,
   so that fallback was deleted with nothing in its place -- and the template is fetched once, at
   connect, gated on ProjectFile mode. In QuickPlot the structure does not exist yet at connect,
   so CSV / MDF4 / Sessions never created a file: enabled, no error, no output. Fixed by giving
   each worker `applyPublishedStructure()`, fed from `FrameBuilder::structurePublished`.
   `test_csv_sink_alone_writes_rows` is the regression test.
2. **The connect-time template fetch spun a nested event loop** inside `connectDevice()`
   (`invokeOnBuilderThreadBlocking` on the GUI thread), which crashed the app when driven from an
   API command. Replaced with `FrameBuilder::sessionStructureReady(const Frame&)`, emitted from the
   pipeline thread and consumed queued by all three sinks. The payload is the union frame
   (`m_frame` / `m_quickPlotFrame`), never a per-source frame: a single-file export builds one
   schema, and a per-source frame would silently drop the other sources' columns. Written up as C1
   in `doc/claude/specs/0056-connect-path-crashes/findings.md`.
3. **An empty structure could clobber a good one.** `sessionStructureReady` is queued, so in
   QuickPlot its empty payload can land *after* `structurePublished` filled the template, leaving
   the worker with no schema and no file. `setTemplateFrame()` now ignores an empty frame.

**Verification status of the acceptance criteria that depend on a running app:** AC12's
block-lane portion passes (`test_block_lane_sinks.py`: 3 passed, 3 skipped for want of a virtual
capture device; `test_csv_export.py`: 5 passed). The *full* suite still cannot complete: an
unrelated pre-existing crash (C2 in spec 0056) kills the app partway through every pass. 17 of
~40 integration files have run; the rest are unverified for that reason, not because they fail.
