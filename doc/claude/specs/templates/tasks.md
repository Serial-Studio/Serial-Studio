---
spec: NNNN-short-slug
phase: tasks
updated: YYYY-MM-DD
---

# Tasks NNNN — <Feature name>

> **The ordered checklist.** `/ss-implement` decomposes the Design in [`spec.md`](./spec.md)
> into units that are small, ordered, and *individually verifiable*, works the list top to
> bottom and keeps the boxes current. No approval gate: the spec was the gate.

## Conventions

- One task = one focused, reviewable change. If a task touches >3 files or needs a paragraph
  to describe, split it.
- **Verify** is how *this* unit is confirmed before moving on — usually
  `python scripts/code-verify.py --check <files>`, plus a test or a read-back where one fits.
- **Deps** lists task IDs that must land first.
- Order so the tree compiles (conceptually) after each task where practical.

## Tasks

### T1 — <short title>

- **Files:** `app/src/...`
- **Does:** <one or two sentences>
- **Verify:** <command / test / observation>
- **Deps:** none
- [ ] done

### T2 — <short title>

- **Files:**
- **Does:**
- **Verify:**
- **Deps:** T1
- [ ] done

## Definition of Done

<The whole-feature gate, checked once every task is complete.>

- [ ] Every acceptance criterion in `spec.md` is met and checked off there.
- [ ] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [ ] `qt-cpp-review` run on the C++ diff; findings addressed or noted.
- [ ] `ss-hotpath` checks pass / `--benchmark-hotpath` not regressed (if hotpath touched).
- [ ] Relevant `pytest` tests identified for the maintainer to run (listed in `spec.md`).
- [ ] `python scripts/sanitize-commit.py` run, only if a generator input changed.
- [ ] Diff is *what was asked, and only that* — no scope creep, no foreign files touched.
- [ ] `spec.md` status set to `done`.
