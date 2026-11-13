---
name: ss-spec
description: >-
  Step 1 of Serial Studio's spec-driven workflow: capture WHAT a feature must do, WHY, and the
  design that delivers it, in one document. Use to start any debatable feature or anything on
  the hotpath, licensing or API auth — "spec this out", "write a spec", "start a feature",
  "plan this". Produces doc/claude/specs/NNNN-slug/spec.md and gates on the one human approval
  before /ss-implement.
argument-hint: "[feature name or one-line description]"
---

# Serial Studio — /ss-spec (step 1 of 2)

One document, one approval. The first half says **what and why** in terms a reviewer can read
without the codebase; the **Design** half says how. See
[doc/claude/spec-driven.md](../../../doc/claude/spec-driven.md).

## Procedure

1. **Pick the number and slug.** The next number is `max(existing) + 1`, zero-padded to four
   digits:

   ```bash
   ls -d doc/claude/specs/[0-9][0-9][0-9][0-9]-* 2>/dev/null \
     | sed -E 's#.*/([0-9]{4}).*#\1#' | sort -n | tail -1
   ```

   Choose a short kebab-case slug from the feature name (`0007-canbus-filter`).

2. **Ground yourself in real behavior** (a report, a screenshot, a measured limit), then state
   the problem and the one deciding constraint in a single chat sentence *before* drafting — a
   problem you can name is one the requirements will actually serve (`doc/claude/j-space.md`).

3. **Create the directory and copy the template:**

   ```bash
   mkdir -p doc/claude/specs/NNNN-slug
   cp doc/claude/specs/templates/spec.md doc/claude/specs/NNNN-slug/spec.md
   ```

   Fill in the frontmatter (`spec`, `title`, `created`, `author: Alex Spataru`, `status:
   draft`).

4. **Write the what and why:** Problem/Motivation, Goals, Non-Goals, Requirements (numbered,
   testable, no file paths or class names), Acceptance Criteria (each tied to a real check),
   Constraints & Invariants.

5. **Run the premise check.** Name the one measurement that would kill the spec (the coupling
   figure that retired spec 0069, the local benchmark that would have stopped spec 0090 before
   eleven commits), run it time-boxed to about an hour in the scratchpad or against the running
   app, and record the question, the number and the verdict. If the premise cannot be measured,
   say so there in one sentence; the maintainer decides whether to proceed on reasoning alone.

6. **Read the ground truth before designing — never from memory.** The
   [architecture.md](../../../doc/claude/architecture.md) file(s) for the touched subsystems,
   the rows of [common-mistakes.md](../../../doc/claude/common-mistakes.md) the change is
   exposed to, and the actual code. Hotpath: invoke **`ss-hotpath`** and read `FrameBuilder` /
   `CircularBuffer` / `FrameReader` / `Dashboard` in full. New driver: follow
   **`ss-new-driver`**.

7. **Diverge, then write the Design.** Sketch 2-3 *named* candidate approaches in chat (a name
   plus one line each; at least one breaks an assumed constraint), recommend one with the
   one-line why, then fill the Design sections. **Hotpath & threading impact** is required even
   when the answer is "none". List concrete file paths confirmed by grep. Put each real tradeoff
   in the table with a pick, and map every acceptance criterion to a check.

8. **Resolve ambiguity before finishing.** Where a call is genuinely the maintainer's (scope,
   tier gating, UX shape), use `AskUserQuestion` with a recommendation — do not guess and bury
   the assumption. Unresolved items stay in Open Questions.

## Gate

Stop after writing `spec.md`. Present it — especially the tradeoffs and the hotpath answer —
and **do not run `/ss-implement` until the maintainer approves it** (set `status: approved`).
This is the only gate, so everything a reviewer might veto has to be in this file.

## Rules

- This file is the only thing you create. No code, no scaffolding yet.
- The Premise Check section never keeps its template text.
- The spec's file list *is* the lane for `/ss-implement`.
- A change to the spec after approval goes through the advisor, then back to `draft` for
  re-confirmation.
