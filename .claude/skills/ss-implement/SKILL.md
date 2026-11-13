---
name: ss-implement
description: >-
  Step 2 of Serial Studio's spec-driven workflow: turn an approved spec.md into a task
  checklist (tasks.md) and execute it top to bottom, verifying each task. Use after /ss-spec is
  approved, or "implement the spec", "build it", "work the tasks". Honors the hotpath, verify,
  and trust rules; leaves the verified change in the tree for the maintainer.
argument-hint: "[spec number or slug]"
---

# Serial Studio — /ss-implement (step 2 of 2)

Write the checklist, then execute it one task at a time, verifying as you go. This is the only
step that writes code. See [doc/claude/spec-driven.md](../../../doc/claude/spec-driven.md).

## Preconditions

- `doc/claude/specs/NNNN-slug/spec.md` exists and its `status:` is `approved` or `in-progress`.
  Read it in full (and `plan.md`, if an older spec has one) before touching code. A spec with
  neither a Design section nor a `plan.md` goes back to `/ss-spec`.
- Set `spec.md` `status: in-progress`.

## Write the checklist (no gate)

If the spec directory already holds a `tasks.md`, resume it at the first unticked task.
Otherwise copy `doc/claude/specs/templates/tasks.md` there and decompose the Design:

- One task = one focused change; more than ~3 files or a paragraph of description means split.
- Order so the tree stays coherent: data model, `Keys::` and enums before the code that reads
  them; a driver class before its registration touch-points; UI after the model that feeds it.
- Each task names its **Files**, what it **Does**, how to **Verify** it and its **Deps**. A
  task on a silent-breakage class (hotpath, ctor closure, signal wiring, cached hotpath flags)
  names the binding invariant in its Does line.
- The tasks cover every file in the spec's Affected files table and every acceptance criterion,
  nothing more. If they cannot, the spec is incomplete: stop and say so.

Show the task titles in chat in one short list and start; do not wait for approval.

## Per-task loop

1. **Read before writing.** Read the target files in full this session — hotpath files
   (`FrameBuilder`, `CircularBuffer`, `FrameReader`, `Dashboard`) and existing signal/slot
   wiring are mandatory full reads (CLAUDE.md). Invoke **`ss-hotpath`** on any hotpath task,
   **`ss-cpp-modern`** when shaping non-trivial new C++.
2. **Name the binding invariants** before the first `Edit`: restate in chat, in your own
   words, the invariant(s) the task's Does line carries (plus any the read surfaced). A
   constraint steers the edit only when named at the point of action (`doc/claude/j-space.md`).
3. **Make the change** by hand with targeted `Edit` calls — edit, don't rewrite, never a
   generator. Follow [code-style.md](../../../doc/claude/code-style.md).
4. **Verify the task:** `python scripts/code-verify.py --check <changed files>` and
   `python scripts/syntax-check.py <changed C++ files>`. Resolve new errors before moving on.
   Run the task's stated check (a `tests/scripts/` JS unit you *can* run, or a read-back).
5. **Mark it done** — tick the task's box in `tasks.md` so the file stays a live record.

Stay strictly inside the spec's file list. If a needed change falls outside it, **stop and name
it in chat** ("the spec didn't cover X — add it?") rather than widening the diff. Never touch,
revert, or restore a working-tree file you did not edit this session.

## Finish

When every task is done, run the Definition of Done in `tasks.md`:

1. **Static review:** invoke **`qt-cpp-review`** on the C++ diff; address or note findings.
2. **Hotpath:** if touched, confirm the `--benchmark-hotpath` plan with the maintainer.
3. **Self-review the diff, counterfactually:** is this *what was asked, and only that*? Then
   answer out loud: "which rule does this diff most risk violating, and what is the concrete
   evidence it doesn't?" If either answer is weak, say so before claiming completion.
4. **Sanitize only if a generator input changed** (`api-schema.json`, `dataset.json`, a
   command manifest, `command_safety.json`, doc/help), and only when none of the maintainer's
   own files are dirty: `python scripts/sanitize-commit.py`.
5. **Hand off.** Name the changed files, the `git diff --numstat` net total beside the spec's
   approved total, and the `pytest` targets the maintainer runs against a live app. Over 1.5
   times the approved total: stop, amend `spec.md` and set it back to `draft`. Leave the edits in the tree; commit only when the maintainer asks, as one commit
   of your own paths (CLAUDE.md "Commit only when asked").
6. Set `spec.md` `status: done`.

## Rules

- **Never build or run the app; never push unasked.**
- The spec is the contract: every acceptance criterion ends up satisfied and checked off in
  `spec.md`. If reality diverges from the design mid-build, stop, amend `spec.md` and get it
  re-confirmed rather than silently improvising.
- Keep `tasks.md` honest as you go — a half-finished feature should be resumable by reading the
  spec files alone.
