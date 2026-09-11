---
spec: 0082-render-parallelism
title: Render Parallelism
status: done         # draft -> approved -> in-progress -> done | shelved (Half A done; Half B shelved per R5, 2026-09-11)
created: 2026-09-11
author: Alex Spataru
---

# Spec 0082 — Render Parallelism

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

Spec 0081 gave the kernels wider vectors. Two other forms of parallelism a modern core offers
are still unused on the render side, and both are grounded in the code as it stands today.

**Instruction-level parallelism.** The minimum, maximum and finite-point reductions accumulate
into a single register: every iteration waits for the previous compare to finish before it can
start its own. On current cores a vector min or max has a latency of about four cycles but a
throughput of two per cycle, so a single chain runs at roughly one eighth of what the execution
units can sustain. These reductions run on every display tick for every plot that autoscales,
over rings that hold up to the full time window, and the AVX2 lane inherited the same one-chain
shape, so the wider vectors only moved the ceiling, not the dependency.

**Thread-level parallelism.** Every FFT and waterfall widget performs its whole transform on the
GUI thread inside the display tick: windowing, the FFT itself (up to 65536 bins for a
waterfall), smoothing, decibel conversion and row painting. Fourteen widget types subscribe to
that tick, and a dashboard with several spectral widgets serialises all of their transforms on
the one thread that also has to lay out and paint the window. The maintainer's four-waterfall
project is the case spec 0075 R15 was written from; the texture side of that cost was fixed,
the transform side was not. No measurement of the transform share of the GUI tick exists yet,
which is why the first requirement below is to obtain one before any thread is added.

The acquisition pipeline is out of scope on purpose: frames within one source are sequential by
design, the pipeline thread is the single producer for every sink, and the dense sources already
have one worker thread each. Multicore gains, if any, are on the render side.

## Goals

- The reduction kernels run several independent accumulators so the execution units are not
  idle behind one dependency chain, with output bit-identical to today's kernels.
- The GUI thread's share of spectral transform work is measured on a real heavy project before
  any change to where that work runs, and the decision to move it is made from that number.
- If the measurement warrants it, spectral transforms and row staging run on worker threads
  bounded by the machine's core count, and the GUI thread only adopts finished rows, with no
  visible change in cadence, order or content.
- A machine with few cores, or a pool that cannot be created, behaves exactly as today.

## Non-Goals

- No change to the acquisition pipeline's threading, the single-producer publish path, the
  stream workers or the sink fan-out.
- No parallel frame parsing. Frames within a source stay sequential.
- No GPU compute for the transforms.
- No change to any displayed, exported or recorded value. Reordering that changes a float
  result is forbidden; only order-independent operations may gain accumulators.
- No new dependency. Whatever threading is used comes from the toolkit already linked.
- No change to the idle behaviour spec 0075 R15.1 defines and the waterfall fix of 2026-09-11
  restored: a widget whose ring received no samples does no transform and writes no row.

## Requirements

1. **R1** — The minimum, maximum, combined min/max and finite-point min/max reductions use at
   least two independent accumulators per lane on every vector level (SSE4, AVX2, NEON), merged
   once at the end with the same operand order the single-chain version used.
2. **R2** — Every reduction produces a result bit-identical to the scalar reference for every
   input the existing bit-exact suite covers, at every runtime level. The documented signed-zero
   exception stays the only tolerated divergence.
3. **R3** — A reduction over a long span (at least 64k elements) is measurably faster than the
   single-chain version on the same machine and level, reported as a ratio by a repeatable
   micro-benchmark the maintainer can run.
4. **R4** — Before any thread is added, the maintainer measures the GUI thread on the
   four-waterfall project with all waterfalls and FFT plots live: the fraction of a display tick
   spent in transform and row staging, and the resulting tick cadence. The number and the
   method are recorded in the plan.
5. **R5** — Off-thread staging is built only if R4 shows transform work above an agreed share of
   the tick budget (proposed: one fifth). Below that share the multicore half of this spec is
   shelved with the measurement as the reason.
6. **R6** — When built, each spectral widget's transform and row staging run on a worker; the
   GUI thread only adopts the finished row on the next display tick. A tick that finds no
   finished row draws what it has; rows are never dropped, reordered or duplicated.
7. **R7** — The worker count is bounded by the machine's core count minus the cores the pipeline
   thread and any dense stream workers need, never below one, and the pipeline thread's
   scheduling priority is unaffected.
8. **R8** — A widget that is hidden, reconfigured or destroyed never receives a late result;
   a result for a stale configuration is discarded.
9. **R9** — With workers active, every tick of a live spectral widget is bit-identical to the
   same tick staged on the GUI thread: same row, same smoothing, same marker states.
10. **R10** — The staging path allocates nothing per tick beyond what it allocates today; buffers
    are owned per widget and reused.
11. **R11** — If no worker can be created, or the platform exposes a single core, staging runs
    on the GUI thread exactly as today, with no user-visible difference and one log line.

## Acceptance Criteria

- [x] **AC1** (R1, R2) — The bit-exact ctest suite passes unchanged at every runtime level with
      the multi-accumulator reductions in place; the suite's length table already crosses the
      accumulator boundaries (lengths 1..33, 63..65, 255, 1024).
- [x] **AC2** (R3) — The micro-benchmark reports the single-chain versus multi-accumulator
      ratio on the maintainer's machine for SSE4 and AVX2; the ratio exceeds 1.5 on at least
      one level, or the change is reverted with the numbers recorded.
- [x] **AC3** (R4) — A sampled profile of the running app on the four-waterfall project, taken
      by the maintainer, is summarised in the plan with the transform share of the GUI tick.
- [x] **AC4** (R5) — The plan states the go or no-go for off-thread staging with the R4 number.
- [ ] (shelved, R5 no-go) **AC5** (R6, R9) — Maintainer observation: the same project staged on workers and on the
      GUI thread produces identical waterfall images after a fixed number of ticks (screenshot
      or texture dump diff), and the FFT plot's peak markers agree.
- [ ] (shelved, R5 no-go) **AC6** (R7) — The pipeline thread's priority band is unchanged in the thread listing
      with workers running; the worker count equals the documented bound.
- [ ] (shelved, R5 no-go) **AC7** (R8) — Toggling a spectral widget hidden and visible, changing its FFT size, and
      closing the project while workers are busy produces no crash, no stale row and no
      warning, over a repeated loop the maintainer runs.
- [ ] (shelved, R5 no-go) **AC8** (R10) — A steady-state run with workers active shows a flat working set and no
      per-tick allocation in the staging path (the maintainer's existing page-fault script).
- [ ] (shelved, R5 no-go) **AC9** (R11) — Forcing the pool unavailable in a build switch produces the GUI-thread
      path, identical output and the one log line.
- [x] **AC10** — `--benchmark-hotpath` gates are unaffected in every tier: none of this touches
      the acquisition pipeline.

## Constraints & Invariants

- **Bit-exactness is the deciding constraint.** Multiple accumulators are legal only for
  operations whose result does not depend on association order: min and max qualify, and NaN
  semantics survive because a NaN element never wins in any accumulator and a NaN seed poisons
  every accumulator the same way. Sums, products and the windowing chain are excluded.
- **The acquisition pipeline is untouched.** No new thread may share a core with the pipeline
  thread by priority, no publish-path object is read from a worker, and the message bus stays off
  every per-tick path.
- **Source owns time.** A row's position in the waterfall is decided by when its samples
  arrived, not by when a worker finished; late results adopt in arrival order.
- **Idle stays idle.** A widget with no new samples does no work on any thread.
- **Widget lifetime is the GUI thread's.** Workers hold no reference that can outlive the
  widget; cancellation is a state check on adoption, never a blocking join on the GUI thread.
- **No allocation per tick, no lock on the draw path.** Hand-off between worker and GUI thread
  is a single-producer, single-consumer slot per widget.
- **The measurement comes first.** The multicore half is conditional on R4; a plan that skips
  the measurement is not acceptable.

## Open Questions

- **Threshold for R5.** One fifth of the tick budget is proposed. The maintainer may prefer an
  absolute figure (milliseconds per tick on the reference machine).
- **Scope of the worker path.** FFT and waterfall transforms are the named candidates. Plot and
  multiplot downsampling also run per widget per tick; whether they join depends on the same
  measurement showing them as a material share.
- **Where the micro-benchmark for R3 lives.** A ctest that prints timings without gating, or a
  new tier of the hotpath benchmark. The former keeps the benchmark's gate list untouched.
