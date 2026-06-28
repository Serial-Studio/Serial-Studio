---
spec: NNNN-short-slug
title: <Human-readable feature name>
status: draft        # draft -> approved -> in-progress -> done | shelved
created: YYYY-MM-DD
author: <name>
---

# Spec NNNN — <Feature name>

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

<Why does this need to exist? What is broken, missing, or painful today, and who feels it?
Ground this in real behavior — a user report, a screenshot, a measured limit, a recurring
support question — not a hypothetical. Ground truth outranks on-paper reasoning. One or two
paragraphs.>

## Goals

<What does success look like, in observable terms? Each bullet is a single outcome a user
or maintainer could confirm.>

-

## Non-Goals

<Explicit scope boundaries — what this feature deliberately does NOT do, so the plan does
not over-build and review does not expand it. Naming a non-goal here is cheaper than cutting
it later.>

-

## Requirements

<Numbered, testable statements of user-facing behavior. Each must be verifiable by someone
who cannot see the code. Prefer "the dashboard shows X when Y" over "add a handler for X".>

1. **R1** —
2. **R2** —

## Acceptance Criteria

<How we will know each requirement is met. Tie each to a check that can actually run where
one exists: a `pytest` integration test, a `tests/scripts/` JS unit, the `--benchmark-hotpath`
gate, or a specific observation the maintainer will make in the running app. These become the
test plan in `plan.md`.>

- [ ] **AC1** —
- [ ] **AC2** —

## Constraints & Invariants

<Anything the implementation must not break, stated as a constraint rather than a design.
Examples: "must not regress the 256 kHz hotpath gate", "must work in QuickPlot and
ProjectFile modes", "Pro-only feature, gate behind BUILD_COMMERCIAL", "no new dependency".
Pull the deciding constraint out here, up front.>

-

## Open Questions

<Anything that blocks a confident plan. Resolve these with the maintainer before `/ss-plan`
rather than guessing — a wrong assumption here propagates through every later phase.>

-
