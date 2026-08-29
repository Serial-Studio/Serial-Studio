---
spec: 0060-compiled-expression-transforms
title: Compiled expression transforms
status: done          # closed 2026-08-20
created: 2026-08-16
author: Claude (overnight run, unattended; planned + implemented 2026-08-17 on maintainer request)
---

# Spec 0060 — Compiled expression transforms

> **Phase 1 of 4 — the WHAT and the WHY.** Written 2026-08-16 as specify-only; on 2026-08-17 the
> maintainer asked for B to be planned and implemented — see `plan.md` (purpose-built evaluator,
> nothing vendored).

## Problem / Motivation

Per-dataset transforms today are JS or Lua functions (`transformCode` / `transformLanguage`
on the dataset). That is the right tool for logic, but the common case is arithmetic:
`v * 0.0625 - 40`, `sqrt(a*a + b*b)`, `(v - prev) / dt`. On the frame lane a script call per
dataset per frame is already the dominant cost the parse-budget governor exists to manage; on
the stream lane (`IO::StreamWorker`, 44.1 kHz and up) a script engine invocation per sample is
not viable at all, which is why the stream path offers `transform_block` and otherwise falls
back to raw. Cross-dataset math ("dataset A minus dataset B") has no cheap path anywhere: it
needs the data-table API and a marshal. A compiled arithmetic expression, evaluated as a small
bytecode or tree walk with no allocation and no engine, would run per sample on the stream
worker and per frame on the pipeline at a fraction of the cost, and could reference sibling
datasets by name inside the same source.

## Goals

- A third transform mode alongside JS and Lua: a single arithmetic expression string with
  variables `v` (this sample), `t` (sample time in seconds, source clock), `n` (sample index
  since connect), sibling datasets by name (`{Temperature}` or a bare identifier resolved at
  compile time), and a history accessor `sample(dataset, k)` returning the k-th previous sample
  of a dataset (bounded k).
- Compile once at project load / edit; evaluate per sample with zero allocation and no engine
  call; usable on both lanes.
- Compile errors are surfaced at edit time (editor + Problem Center), never per frame.
- Deterministic IEEE double semantics identical on both lanes (no fast-math).

## Non-Goals

- Control flow, strings, tables, side effects, `apiCall`. Anything beyond arithmetic +
  functions stays in JS/Lua.
- Replacing JS/Lua transforms or migrating existing projects.
- Vendoring anything tonight (see the evaluation below; the decision is Alex's).

## Requirements

1. **R1** — `transformLanguage` gains a third value (`Expression`); `transformCode` holds the
   expression text; project JSON round-trips unchanged for the other two modes.
2. **R2** — Grammar: `+ - * / % ^`, unary minus, parentheses, comparisons yielding 0/1,
   `? :`, and a fixed function set (`abs min max clamp floor ceil round sqrt cbrt exp log
   log10 log2 pow sin cos tan asin acos atan atan2 sinh cosh tanh hypot lerp deg rad`),
   constants `pi e nan inf`; identifiers resolve to `v t n dt` or a sibling dataset name.
3. **R3** — `sample(name, k)` returns the k-th previous published value of a dataset of the
   same source (k in `[0, kMaxHistory]`, `kMaxHistory` = 256); before enough history it
   returns NaN.
4. **R4** — Cross-dataset references read the *current frame's* already-parsed value on the
   frame lane and the *same block's* sample on the stream lane; a reference to a dataset that
   is not part of the same source is a compile error.
5. **R5** — Evaluation allocates nothing, calls no engine, and is bounded by the compiled
   program length; a program is capped at a fixed node count.
6. **R6** — Errors: parse/reference errors are edit-time findings; runtime is total (NaN
   propagates), no per-frame diagnostics.
7. **R7** — `--benchmark-hotpath` gains an ungated "Expression numeric" readout so the win is
   measured, not asserted.

## Acceptance Criteria

- [x] **AC1** — `v * 0.0625 - 40` on a 256 kHz Native numeric project costs less than 10 % of
      the equivalent Lua transform in the benchmark readout.
- [x] **AC2** — An audio stream (44.1 kHz) with `sqrt(v*v)` per sample never trips the stream
      worker's drop counter on the reference machine.
- [x] **AC3** — `sample(Temperature, 1)` on the frame lane returns the previous frame's value;
      on the stream lane the previous sample.
- [x] **AC4** — A typo (`sqtr(v)`) is a single edit-time finding; the dataset publishes raw.
- [x] **AC5** — Bit-identical results between the frame lane and the stream lane for the same
      expression and inputs.

## Constraints & Invariants

- Hotpath rules: no allocation, no locks, no per-sample cross-thread traffic; the evaluator
  state is per dataset and thread-owned by whichever lane runs it.
- IEEE-stable math flags stay (no fast-math anywhere; the cmake flag modules are out of lane).
- Any third-party evaluator must be isolated in exactly one TU behind our own interface, so
  its compile-time cost and its headers never leak into hotpath TUs.

## Candidate evaluation: exprtk vs purpose-built

**exprtk** (Arash Partow, MIT): ~39 kLOC single header, very complete (strings, vectors,
loops, user functions), fast after compile (tree of virtual nodes, extensive constant folding).
Real cost: 30-90 s compile of the one TU that includes it, tens of MB of object code in
debug, RTTI/exceptions expected, a symbol-table model that would need adapters for our
by-name dataset references and the `sample()` history accessor, and far more surface than
R2 asks for (loops and strings are exactly what we do not want to expose). Its evaluation is
virtual-call per node, fine on the frame lane, adequate on the stream lane. Licence is MIT,
compatible with both builds. Vendoring is a `lib/` change (out of scope tonight).

**Purpose-built evaluator** (~600-900 lines: tokenizer, Pratt parser to a flat postfix
program of `{op, arg}` nodes, a stack machine of fixed depth): compiles in a second, no
RTTI/exceptions, our exact grammar, our exact error messages, trivial to unit-test bit-for-bit
against a reference walker, `sample()` and by-name references are first-class rather than
adapters, and constant folding of the common linear case (`a*v + b`) collapses to two
multiplies. Cost: we write and own it; no vectors/strings/loops (which is a feature here).

**Recommendation: purpose-built.** The grammar in R2 is small by design; the whole value of
the feature is a per-sample cost near the noise floor and behaviour we can prove bit-identical
across lanes (AC5). exprtk's completeness is weight we would carry and hide, its compile time
is paid on every CI run, and its adapter layer for history/by-name references would be about as
large as the evaluator itself. Revisit if R2 grows past arithmetic.

## Open Questions for Alex

- Name of the mode as shown to users ("Formula"? "Expression"?), and whether the editor gets a
  dedicated one-line field instead of the code editor.
- Should `sample()` history be per-dataset rings on both lanes (memory: `kMaxHistory` doubles
  per referenced dataset), or only where referenced (compile-time discovery)?
- Pro-gate or free? (Cross-dataset math is a Pro-flavoured capability; the arithmetic itself
  is a basic one.)
- Do we want the ungated benchmark readout in `ci.yml` from day one?
