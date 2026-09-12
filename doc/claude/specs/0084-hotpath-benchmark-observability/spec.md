---
spec: 0084-hotpath-benchmark-observability
title: Hotpath benchmark observability — allocation gate, width knob, release symbols
status: done          # closed 2026-09-12: maintainer ran the build/run gates and closed
created: 2026-09-11
author: Alex Spataru
---

# Spec 0084 — Hotpath benchmark observability

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

The hotpath benchmark reports frames per second per tier and a three-way stage split for the
Native lane. That is enough to catch a throughput regression and nothing else. On 2026-09-11 a
profiling pass over the shipped 4.1.0 binary found two things the benchmark could not have
shown:

- The Native publish path allocates once per flushed block (a map node freed and re-created on
  every open/close), in a lane whose rule is "no allocation on the publish path". The frames
  per second number never moved, so nothing flagged it.
- The Lua and JavaScript parser lanes spend the majority of their time in the allocator
  (76% of samples in the Lua numeric window of a symbolized build). The benchmark reports
  those lanes as passing their tiers, which they do, while hiding that most of the budget is
  heap traffic rather than parsing.

Two more gaps made the investigation harder than it should have been:

- The benchmark project is fixed at 8 channels. A per-dataset cost that scales with project
  width (the field report in spec 0083 involved a 635-dataset project) is invisible at 8, and
  there is no way to run the same tiers wider without editing source.
- The shipped binary is stripped and no symbolized optimized build is kept, so attributing
  cost to functions required a separate debug build. Function-level numbers from a debug build
  are shape-only; the optimized binary is what ships and what CI gates.

## Goals

- A CI run of the benchmark fails when a Native lane allocates on the per-frame path, and
  prints the allocation count per frame for every lane so a script-lane regression is visible
  in the log even before it is gated.
- The same benchmark tiers can be run at a chosen project width from the command line, so a
  per-dataset cost can be measured at 8, 64, 256 or 635 datasets without a code change.
- A profiler attached to the release binary resolves function names, with no change to what
  end users download.

## Non-Goals

- No change to the tier thresholds, the 256 kHz gate, or which lanes are gated on throughput.
- No allocation ceiling on the script lanes in this spec; they are report-only until spec 0086
  removes their per-frame allocations, at which point a floor is set there.
- No new profiling tool or in-app profiler UI. The goal is to make the existing external
  profilers useful, not to replace them.
- No change to the in-app benchmark dialog beyond whatever the shared report gains for free.

## Requirements

1. **R1 — Allocation count per run.** Every benchmark run row reports the number of heap
   allocations that happened on the benchmark thread during the timed loop, expressed per
   parsed frame, alongside frames per second.
2. **R2 — Native allocation gate.** The Native numeric and Native mixed runs fail the benchmark
   (nonzero exit, `HOTPATH_PASS=0`) when their per-frame allocation count is above zero. The
   report names the failing run and the count.
3. **R3 — Script lanes report only.** Lua and JavaScript runs print their per-frame allocation
   count and never fail on it under this spec.
4. **R4 — The count excludes setup.** Project loading, chunk construction and engine
   compilation happen before the timed loop and do not contribute to the count, so a lane
   that is allocation-free per frame reads exactly zero.
5. **R5 — Width knob.** A command-line option sets the number of numeric channels in the
   synthetic benchmark project. The default stays 8 so existing CI numbers and the PGO
   training run are unchanged. Every parser tier and the exporter floor honor the knob,
   including the string-column mixed runs, whose string columns are appended after the numeric
   ones as today. The dashboard rows keep their fixed all-widget project: they measure widget
   ingest, not width. Above the Native span lane's field cap the report says which lane the
   Native rows took, so a wide run is never read as a span-lane number.
6. **R6 — Width in the report.** The report header and the machine-readable summary line state
   the channel count the run used, so two reports at different widths cannot be confused.
7. **R7 — Width is not gated.** Runs at a non-default width print their tier target for
   reference but do not fail on it; the tier thresholds were calibrated at 8 channels.
8. **R8 — Release symbols kept out of band.** Release and CI builds produce a separate symbol
   artifact for the application binary, the shipped binary stays stripped, and CI uploads the
   symbol artifact next to the binary so a profile taken on a released build can be
   symbolicated after the fact.

## Acceptance Criteria

- [x] **AC1** — `--benchmark-hotpath` output contains an allocations-per-frame column and a
  matching `HOTPATH_<LANE>_ALLOC_PER_FRAME=` value for every gated run.
- [x] **AC2** — With the current tree, the Native numeric row reports a nonzero per-frame count
  (the open-block map churn) and the benchmark exits nonzero; after spec 0085 lands it reports
  zero and passes. This ordering is the proof the gate measures the real thing.
- [x] **AC3** — A Lua run that allocates prints its count and the benchmark still passes when
  every throughput tier is met.
- [x] **AC4** — Running with the width option at 256 channels produces a report whose header
  and summary line say 256, whose Native rows still complete, and whose exit code ignores the
  tier targets.
- [x] **AC5** — A `--min-fps 1` PGO training run with no width option behaves exactly as today
  (same project shape, same phases).
- [x] **AC6** — A release CI job publishes a symbol artifact for the application; loading it
  into a profiler over a trace of the shipped binary resolves hotpath function names.
- [x] **AC7** — The shipped binary size and the strip status are unchanged (verified by the
  existing packaging step, nothing new to the user).

## Constraints & Invariants

- Counting allocations must not itself allocate or perturb the timed loop measurably; the
  frames-per-second numbers for the existing tiers stay within run-to-run noise of today's.
- The allocation count is attributable to the benchmark thread for the gated rows. The
  allocator's statistics are heap-wide: the benchmark thread's counters merge in on every read,
  another thread's only when that thread exits or reads statistics itself. The Native rows run
  first, before any script engine, watchdog or exporter worker exists, so no thread can end
  inside their windows and their count is the benchmark thread's alone. Qt event-loop pumping
  is excluded explicitly. (Amended 2026-09-11 after the plan's open question was resolved
  against the pinned allocator's API.)
- The default report format stays backward compatible: every existing `HOTPATH_*` key keeps
  its meaning, new keys are additive, and `ci.yml` parses the same lines it parses today.
- The width knob changes only the synthetic project; the tier definitions, the sample floor
  and the wall-clock window are unaffected.
- The symbol artifact is a build product and never enters the repo; it must not change what
  `sanitize-commit.py` or `code-verify.py` see.
- Works on all three CI platforms. Where a platform cannot count allocations on the benchmark
  thread without a debug allocator, the column reads "n/a" and the gate is skipped there
  rather than failing.

## Open Questions

- Whether the allocation counter should come from the bundled allocator's own statistics or
  from a counting hook around the timed loop is a plan decision, but it has one spec-level
  consequence: if only the allocator statistics are available, the count is process-wide, not
  per thread, and the "benchmark thread only" constraint needs restating. Resolve in the plan
  with a measurement, then amend this spec if needed.
- Should the width knob also be exposed in the in-app benchmark dialog? Recommended no for
  now; the dialog exists for the user's machine, the knob exists for investigation.
