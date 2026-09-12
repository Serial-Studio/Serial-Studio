---
spec: 0085-native-publish-allocation-free
title: Native publish path — no per-block allocation, no duplicate raw copies
status: done          # closed 2026-09-12: maintainer ran the build/run gates and closed
created: 2026-09-11
author: Alex Spataru
---

# Spec 0085 — Native publish path: allocation-free, raw values only when they differ

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

The Native numeric lane spends 265 of its 305 ns per frame after tokenizing (shipped 4.1.0
binary, M2 Pro, 8 channels). Hardware counters show that time is not memory-bound: the core
retires close to six instructions per cycle, back-end stalls are under 9%, and the 60 L1D
misses per frame hit L2 and are hidden. The cost is instruction count, roughly 800
instructions per dataset for "parse a number, store it, stage it". A symbolized profile over
the same window attributes it to two things the current design does that carry no information:

- **Every dataset writes four strings per frame, two of them duplicates.** The parsed text is
  written to the dataset's value, copied to its raw value, then both are copied again into the
  staged block's text and raw-text columns. When no transform ran, raw equals final by
  definition. String writes (resize, widen, copy) are about 12% of the Native window.
- **Every block flush frees and re-allocates a map node.** The open-block bookkeeping erases
  its entry on flush and inserts a fresh one on the next frame, so each 64-sample block costs
  one free and one malloc on the pipeline thread. Allocator frames are about 5% of the window.
  The publish path rule is "no allocation"; this is a per-block violation that the throughput
  gate never noticed.

Both are measurable with spec 0084's allocation column, which is why that spec comes first.

## Goals

- The Native numeric and Native mixed benchmark runs report zero allocations per frame.
- A dataset without a transform pays for one text write and one numeric store per frame at
  the publish stage, not two of each.
- Every consumer that reads pre-transform values (MDF4 raw channels, the Historian block
  table, the parse-versus-transform classification) sees exactly the values it sees today.

## Non-Goals

- No change to the per-sample display-string contract (spec 0055 D6): every sample of a
  frame-lane block still carries its display text. That contract is the bigger prize and gets
  its own spec once spec 0084's width numbers exist to size it.
- No change to the stream lane, which never carries raw values; its consumers already take the
  final-value fallback this spec extends to the frame lane.
- No change to the block cap, the pool size, or the flush-on-tick behavior.
- No change to what the dashboard, the API wire, gRPC or MQTT publish.

## Requirements

1. **R1 — No per-block allocation.** Opening, filling and flushing a block on the frame lane
   performs no heap allocation in steady state. Steady state begins after the first block for
   a given source under a given project structure; a structure change may allocate once.
2. **R2 — Raw values are carried only when they can differ.** A block column carries a
   pre-transform value and text only for datasets that have a transform. For every other
   dataset the column marks raw as absent, and the pre-transform copies at the dataset level
   are skipped as well.
3. **R3 — Consumers fall back to the final value.** Any consumer of pre-transform data treats
   an absent raw column as "raw equals final" and produces byte-identical output to today for a
   project without transforms, and identical output to today for a project with transforms.
4. **R4 — Transform edits re-derive the layout.** Adding or removing a transform on a dataset,
   or changing the project structure, updates which columns carry raw data before the next
   block is staged. No block ever mixes the two layouts.
5. **R5 — Masked and unmasked blocks keep their separation.** The rule that a block staged
   under one sink-mask state never continues under the other is unchanged.
6. **R6 — Behavior under pool exhaustion is unchanged.** When every block slot is held by a
   consumer, staging still drops and counts exactly as today.

## Acceptance Criteria

- [x] **AC1** — `--benchmark-hotpath` (with spec 0084) reports zero allocations per frame for
  Native numeric and Native mixed, and the benchmark passes.
- [x] **AC2** — The Native numeric frames-per-second number does not decrease; the stage
  report's datasets+publish figure decreases measurably (target: at least 8% off the 265 ns
  baseline on the same machine, recorded in the plan).
- [x] **AC3** — The existing staging unit test is extended to assert, for a two-source project
  where one dataset has a transform: the transformed dataset's column carries raw values, the
  others do not, and flushing twenty blocks per source performs zero allocations after the
  first.
- [x] **AC4** — The export-fidelity integration test and the CSV/MDF4 export tests pass
  unchanged; an MDF4 recording of a transform-free project has the same raw channels with the
  same samples as a recording made before this change.
- [x] **AC5** — A Historian session recorded with a transformed dataset and an untransformed
  one reads back the transformed dataset's raw values and the untransformed dataset's final
  values in the raw slot, matching a pre-change recording (the stored row may omit the raw
  blob; the readout is what must match).
- [x] **AC6** — Adding a transform to a dataset while connected, then removing it, produces
  blocks whose raw presence follows the transform, with no dropped or mixed block (verified by
  the API stream subscriber seeing continuous block numbers).

## Constraints & Invariants

- The pipeline thread is the only writer of staging state; no mutex or atomic is introduced.
- Block slots keep their aliasing shared-pointer hand-out; consumers never receive a copy of
  the pooled slot where they receive the slot today.
- Column order in a block stays export-schema order; consumers index columns positionally.
- The frame lane sample cap (64) and the display-text-per-sample rule are untouched.
- No new dependency; no change to the block's serialized forms (MDF4, Historian, API).
- Must not regress any of the nine benchmark tiers.

## Open Questions

- Should "has a transform" be the only condition for carrying raw data, or should a computed
  dataset (no parsed input, value produced by script) also count? Recommended: yes, any
  dataset whose final value is script-produced carries raw, so the classification consumer
  keeps its "parsed vs produced" distinction.
- Whether the API's per-frame serialization exposes raw values today, and therefore whether
  an API client could observe the change. Believed no; the plan verifies against the API
  schema and the mirror fixtures.
