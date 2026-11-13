---
spec: NNNN-short-slug
title: <Human-readable feature name>
status: draft        # draft -> approved -> in-progress -> done | shelved
created: YYYY-MM-DD
author: <name>
---

# Spec NNNN — <Feature name>

> **Step 1 of 2 — what, why, and the design.** Everything above **Design** is readable without
> the codebase: no file paths, no class names, no signal wiring. Gate: do not start
> `/ss-implement` until a human marks this `approved`. It is the only gate.

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
gate, or a specific observation the maintainer will make in the running app.>

- [ ] **AC1** —
- [ ] **AC2** —

## Constraints & Invariants

<Anything the implementation must not break, stated as a constraint rather than a design.
Examples: "must not regress the 256 kHz hotpath gate", "must work in QuickPlot and
ProjectFile modes", "Pro-only feature, gate behind BUILD_COMMERCIAL", "no new dependency".
Pull the deciding constraint out here, up front.>

-

## Premise Check

<The one measurement that would kill this spec, run before designing: the question, the
number, the verdict. Spec 0069 had no coupling figure and was retired after five failed builds;
spec 0090 had no local benchmark and was reverted after eleven commits. Time-box it to about an
hour. If the premise cannot be measured, say so in one sentence and let the maintainer decide.>

- **Question:**
- **Measured:**
- **Verdict:**

## Design

<Written after reading the relevant `doc/claude/` sub-docs and the *actual code*. Delete a
subsection that does not apply, except Hotpath & threading.>

### Approach

<The chosen design in 3-5 sentences: what gets built, where it plugs in, and why this shape
over the alternatives.>

### Affected files

<Every file you expect to create or edit, confirmed by grep. This list is the lane, and the
total is the size the maintainer approves: estimate net lines per file.>

| File | Change | ± lines |
|------|--------|---------|
| `core/...` | | |
| **Total** | code / tests / docs | |

### Hotpath & threading impact

<REQUIRED. Answer explicitly even if the answer is "none".>

- **Touches the hotpath?** (`FrameReader` / `CircularBuffer` / `FrameBuilder` / `Dashboard`
  draw / span fast lane) — yes/no. If yes: how the SPSC / `Qt::DirectConnection` / no-alloc /
  cached-flag rules are preserved, plus the `--benchmark-hotpath` plan.
- **New cross-thread signal/slot?** — yes/no; connection type and why.
- **New input to a cached hotpath flag?** — if yes, where its change signal wires into the
  cache refresh.
- **Timestamp ownership** — the source stamps at the driver boundary; nothing re-stamps.

### Data model, API and UI

<`Keys::` additions, schema or project-JSON changes and their migration story; new or changed
API commands and their classifications; new QML components and the model that feeds them.>

### Tradeoffs

<The decisions a reviewer might make differently. Recommend, do not enumerate, but record what
was rejected so review does not relitigate it.>

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| | | |

### Risks

<What could regress, and the silent-breakage classes from `common-mistakes.md` this change is
exposed to.>

-

### Tests

<One check per acceptance criterion: `tests/scripts/` units you can run, `pytest` files the
maintainer runs against a live app, `--benchmark-hotpath` if the hotpath is touched.>

-

## Open Questions

<Anything that blocks a confident design. Resolve these with the maintainer before asking for
approval rather than guessing.>

-
