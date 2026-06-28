---
name: ss-tasks
description: >-
  Phase 3 of Serial Studio's spec-driven workflow: decompose an approved plan.md into an
  ordered, individually-verifiable task checklist (tasks.md). Use after /ss-plan is approved,
  or "break this into tasks", "task it out". Gates on human approval before /ss-implement.
argument-hint: "[spec number or slug]"
---

# Serial Studio — /ss-tasks (phase 3 of 4)

Decompose the approved `plan.md` into an ordered checklist of small, individually-verifiable
units. Each task is a coherent diff a reviewer could read on its own. See
[doc/claude/spec-driven.md](../../../doc/claude/spec-driven.md) for the gate discipline.

## Preconditions

- `doc/claude/specs/NNNN-slug/plan.md` exists and its `status:` is `approved`.

## Procedure

1. **Read `spec.md` and `plan.md`.** The tasks must cover every file in the plan's affected-
   files table and every acceptance criterion in the spec — nothing more, nothing less.

2. **Copy the template and write the tasks:**

   ```bash
   cp doc/claude/specs/templates/tasks.md doc/claude/specs/NNNN-slug/tasks.md
   ```

3. **Decompose well:**
   - One task = one focused change. If a task touches more than ~3 files or needs a paragraph
     to describe, split it.
   - **Order** so the tree stays coherent: data model / `Keys::` and enums before the code that
     reads them; a driver class before its registration touch-points; UI after the model that
     feeds it.
   - Each task names its **Files**, what it **Does**, how to **Verify** it (usually
     `python scripts/code-verify.py --check <files>` plus a test or read-back), and its
     **Deps**.
   - Sequence hotpath-sensitive work so `--benchmark-hotpath` can be run at a clean point.

4. **Fill the Definition of Done** — the whole-feature gate (code-verify clean, `qt-cpp-review`
   run, hotpath not regressed, `pytest` targets identified, `sanitize-commit` run, diff in
   scope, spec marked `done`).

## Gate

Stop after writing `tasks.md`. Present the checklist for review. **Do not run `/ss-implement`
until the maintainer approves it.** Reordering or splitting tasks now is free; discovering a
missing task mid-build is not.

## Rules

- Tasks describe work; they do not perform it. No code in this phase.
- Every acceptance criterion in `spec.md` must be reachable by completing the listed tasks. If
  one is not, the plan is incomplete — go back to `/ss-plan`.
- Keep the checklist honest: it becomes the live progress record that `/ss-implement` updates.
