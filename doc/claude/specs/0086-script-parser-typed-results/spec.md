---
spec: 0086-script-parser-typed-results
title: Script parser lanes — typed results and scoped table capture
status: done          # closed 2026-09-12: maintainer ran the build/run gates and closed
created: 2026-09-11
author: Alex Spataru
---

# Spec 0086 — Script parser lanes: typed results, table capture only when used

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

Lua is the 256 kHz benchmark tier and JavaScript the 64 kHz tier. On 2026-09-11 a symbolized
profile of the Lua numeric run showed 76% of samples inside the allocator. Two causes, both
present in the JavaScript lane as well:

- **Every number makes a round trip through a string.** The parser script returns numbers.
  The engine converts each to a heap-allocated string, collects the strings into a
  heap-allocated list of lists, hands that to the frame builder, which parses every string
  back into a number. For an 8-channel frame that is about ten allocations and sixteen
  conversions per frame, then the same again to free it. In the profile: string construction
  26%, list growth 12%, teardown 4%.
- **Table capture is on for every script project.** The per-dataset capture into the shared
  data table (the state the table API, computed datasets and control scripts read) is enabled
  whenever a script parser engine exists, not when a script uses the table API. Every dataset
  then pays two hash lookups and two writes per frame, and the string write allocates
  (18% of the window) even though nothing reads it.

Users feel this as the ceiling on script-parsed projects: a Lua project parses at about a
third of the Native rate on the same data, and most of that gap is heap traffic rather than
script execution.

## Goals

- A Lua or JavaScript parser that returns numbers delivers them to the dashboard as numbers,
  with no string in between and no per-frame heap allocation on the conversion.
- A script project that does not use the table API pays nothing per frame for table capture.
- The Lua numeric and JavaScript numeric benchmark tiers gain measurably; the target is a
  number recorded in the plan from a before/after run, with 30% as the floor worth shipping.

## Non-Goals

- No change to the parser script API as seen by the user: scripts still return an array (or
  array of arrays) of values, and a script that returns strings keeps working.
- No change to the transform (per-dataset value script) lanes, the control script, or the
  Native parser.
- No change to what the table API exposes or to the semantics of computed datasets.
- No change to the Safe/Fast execution modes or the watchdog.
- No change to the frame-level display string: a numeric dataset still ends up with its
  display text exactly as today; only where that text is produced moves.

## Requirements

1. **R1 — Typed cells.** A parser script result is delivered to the frame builder as a
   sequence of cells that are each either a number or a string, preserving the type the
   script produced. A cell that was a number in the script is never formatted to text and
   re-parsed.
2. **R2 — Steady-state allocation-free.** Delivering the result of a script that returns
   only numbers, for a frame shape that has been seen before, performs no heap allocation
   after the script itself returns. Script execution is outside this requirement.
3. **R3 — Text cells are equivalent.** A cell the script produced as a string is treated
   exactly as the same text arriving from the Native parser: same numeric detection, same
   display text, same raw value.
4. **R4 — Multi-frame results keep working.** A script that returns an array of arrays still
   yields one frame per inner array, in order, with the same cap on element counts as today.
5. **R5 — Number formatting is unchanged where text is required.** When a numeric cell must
   become display text (dashboard value, API frame, exports), the text is identical to the
   text produced today for the same number.
6. **R6 — Table capture only when used.** Per-dataset table capture is enabled only when at
   least one script in the project (parser, transform, or control) references the table API,
   or an external table user exists (API server datatable verbs, in-app table views). A
   parser-only project with no such reference performs no capture writes per frame.
7. **R7 — Capture cannot be stale.** Editing any script so that it starts or stops
   referencing the table API, or opening or closing an external table user, re-derives the
   capture state before the next frame. A script that starts reading the table sees values
   from its first read.
8. **R8 — Capture writes do not allocate.** When capture is on, writing a dataset's value into
   the table performs no heap allocation in steady state, string values included.

## Acceptance Criteria

- [x] **AC1** — `--benchmark-hotpath` (with spec 0084) reports zero allocations per frame for
  Lua numeric and JavaScript numeric, and their frames-per-second rise by at least the floor
  recorded in the plan; the mixed tiers do not regress.
- [x] **AC2** — The `tests/scripts/` parser units pass unchanged: every existing script
  fixture produces the same dataset values and display texts.
- [x] **AC3** — A new unit drives a Lua and a JavaScript parser returning mixed number and
  string cells, and asserts numeric detection, display text and raw value per cell match the
  Native parser fed the equivalent delimited text.
- [x] **AC4** — A new unit drives a parser returning an array of arrays and asserts frame
  count, order and per-frame values are unchanged from today's behavior.
- [x] **AC5** — Integration: a project whose parser does not use the table API shows no
  table writes over a ten-second run (observable via the table API's write clock through the
  API server), while the same project with a control script that reads the table shows
  writes from the first frame.
- [x] **AC6** — Integration: editing a transform to add a table read while connected makes
  the transform see live values within one frame; removing it stops the writes.
- [x] **AC7** — The export-fidelity and replay integration tests pass unchanged.

## Constraints & Invariants

- Script engines stay on the pipeline thread and are never used from another thread; the
  typed result crosses no thread boundary.
- No Lua or QJSEngine call may run inside a routed lambda; the existing routing rule for the
  table store stands.
- The per-frame path stays free of message-bus traffic, signals and locks.
- The result cap on elements per frame and frames per result is unchanged.
- Numeric detection must stay bit-identical to the Native lane's for text cells; a value that
  the Native parser treats as non-numeric must stay non-numeric here.
- Must not regress the Native tiers or the stream lane.
- Change-driven transform skipping (spec 0083 era) must keep its semantics: "changed" still
  means value change, so the capture write path must keep its identical-value no-op.

## Open Questions

- How "references the table API" is detected (a compile-time scan of the script source for
  the API's global names, versus a runtime first-touch flag) is a plan decision, but it sets
  the user-visible rule for R6 and R7: a scan is conservative (a string literal mentioning
  the name enables capture), a runtime flag has a one-frame gap on first use. Recommended:
  the scan, since over-enabling costs a few percent and under-enabling loses data.
- Should the display text for a numeric cell be produced lazily (only when a consumer asks)
  or eagerly at the point where today's text would have existed? Eager keeps R5 trivially
  true and keeps spec 0055's per-sample display-text contract untouched; lazy is the bigger
  win and belongs with the future display-text spec. Recommended: eager here.
- JavaScript engines return values through a generic variant. Whether the engine binding can
  hand back a number without an intermediate variant allocation needs a measurement in the
  plan; if it cannot, R2 is met for Lua first and the JavaScript floor is recorded separately.
