---
name: ss-spec
description: >-
  Phase 1 of Serial Studio's spec-driven workflow: capture WHAT a feature must do and WHY,
  with no implementation detail. Use to start any non-trivial or multi-file feature — "spec
  this out", "write a spec", "start a feature", "let's spec-drive X". Produces
  doc/claude/specs/NNNN-slug/spec.md and gates on human approval before /ss-plan.
argument-hint: "[feature name or one-line description]"
---

# Serial Studio — /ss-spec (phase 1 of 4)

Capture the **WHAT** and the **WHY**. No file paths, class names, signal wiring, or algorithm
choices — those belong to `/ss-plan`. A spec that leaks implementation pre-commits the design
before it has been reviewed. See [doc/claude/spec-driven.md](../../../doc/claude/spec-driven.md)
for the full workflow and the gate discipline.

## Procedure

1. **Pick the number and slug.** The next number is `max(existing) + 1`, zero-padded to four
   digits:

   ```bash
   ls -d doc/claude/specs/[0-9][0-9][0-9][0-9]-* 2>/dev/null \
     | sed -E 's#.*/([0-9]{4}).*#\1#' | sort -n | tail -1
   ```

   If that prints nothing, start at `0001`. Choose a short kebab-case slug from the feature
   name (`0007-canbus-filter`).

2. **Ground yourself first.** Skim the relevant `doc/claude/` sub-doc(s) for the area so the
   spec's claims about current behavior are true — but keep the spec implementation-free.
   Ground in real behavior (a report, a screenshot, a measured limit), not a hypothesis.

3. **Create the directory and copy the template:**

   ```bash
   mkdir -p doc/claude/specs/NNNN-slug
   cp doc/claude/specs/templates/spec.md doc/claude/specs/NNNN-slug/spec.md
   ```

   Fill in the frontmatter (`spec`, `title`, `created`, `author: Alex Spataru`, `status:
   draft`).

4. **Write the spec** into every section: Problem/Motivation, Goals, Non-Goals, Requirements
   (numbered + testable), Acceptance Criteria (tied to a real check), Constraints & Invariants,
   Open Questions.

5. **Resolve ambiguity before finishing.** Where a requirement is genuinely the maintainer's
   call (scope, tier gating, UX shape), use `AskUserQuestion` with a recommendation — do not
   guess and bury the assumption. Unresolved items stay in Open Questions.

## Gate

Stop after writing `spec.md`. Present it for review and **do not run `/ss-plan` until the
maintainer approves it** (set `status: approved`). The spec is the contract every later phase
is measured against; an unreviewed spec poisons the plan and the build.

## Rules

- WHAT/WHY only. The first reviewer should be able to read it without seeing the codebase.
- Acceptance criteria must be verifiable — prefer ones that map to `pytest`, `tests/scripts/`,
  `--benchmark-hotpath`, or a concrete in-app observation.
- This file is the only thing you create. No code, no plan, no scaffolding yet.
