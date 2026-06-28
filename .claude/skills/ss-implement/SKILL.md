---
name: ss-implement
description: >-
  Phase 4 of Serial Studio's spec-driven workflow: execute an approved tasks.md top to bottom,
  verifying each task and keeping the checklist current. Use after /ss-tasks is approved, or
  "implement the spec", "build it", "work the tasks". Honors the hotpath, verify, and trust
  rules; never commits without explicit permission.
argument-hint: "[spec number or slug]"
---

# Serial Studio — /ss-implement (phase 4 of 4)

Execute the approved `tasks.md` one task at a time, verifying as you go. This is the only phase
that writes code. See [doc/claude/spec-driven.md](../../../doc/claude/spec-driven.md).

## Preconditions

- `doc/claude/specs/NNNN-slug/tasks.md` exists and its `status:` is `approved`.
- Read `spec.md`, `plan.md`, and `tasks.md` in full before touching code.
- Set `spec.md` `status: in-progress`.

## Per-task loop

For each task in order:

1. **Read before writing.** Read the target files in full this session — hotpath files
   (`FrameBuilder`, `CircularBuffer`, `FrameReader`, `Dashboard`) and existing signal/slot
   wiring are mandatory full reads (CLAUDE.md). Invoke **`ss-hotpath`** on any hotpath task,
   **`ss-cpp-modern`** when shaping non-trivial new C++.
2. **Make the change** with targeted `Edit` calls — edit, don't rewrite. Follow
   [code-style.md](../../../doc/claude/code-style.md): header ordering, `[[nodiscard]]`, no
   in-header init, `Q_EMIT`, no in-body comments, 100-col, ASCII-only in source.
3. **Verify the task:** `python scripts/code-verify.py --check <changed files>`. Resolve new
   errors before moving on (advisories are baseline debt, but new code clears them). Run the
   task's stated check (a `tests/scripts/` JS unit you *can* run, or a read-back).
4. **Mark it done** — tick the task's box in `tasks.md` so the file stays a live record.

Stay strictly inside the plan's file list. If a needed change falls outside it, **stop and name
it in chat** ("the plan didn't cover X — add it?") rather than widening the diff. Never touch,
revert, or restore a working-tree file you did not edit this session.

## Finish

When every task is done, run the Definition of Done in `tasks.md`:

1. **Static review:** invoke **`qt-cpp-review`** on the C++ diff; address or note findings.
2. **Hotpath:** if touched, confirm the `--benchmark-hotpath` plan with the maintainer (you
   cannot run it — you don't build the app).
3. **Self-review the diff:** is this *what was asked, and only that*? If not, say so before
   claiming completion.
4. **Sanitize:** run `python scripts/sanitize-commit.py` (it sanitizes only — never commits).
5. **Identify the `pytest` targets** from `plan.md` for the maintainer to run (the app must be
   up with the API server enabled).
6. Set `spec.md` `status: done`.

## Rules

- **Never build or run the app**, and **never commit or push without explicit per-turn
  permission** — earlier authorizations do not carry over.
- The spec is the contract: every acceptance criterion must end up satisfied and checked off in
  `spec.md`. If reality diverges from the plan mid-build, stop and amend `plan.md`/`tasks.md`
  (re-confirm) rather than silently improvising.
- Keep `tasks.md` honest as you go — a half-finished feature should be resumable by reading the
  three spec files alone.
