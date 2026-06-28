---
spec: NNNN-short-slug
phase: tasks
status: draft        # draft -> approved (gate before /ss-implement)
updated: YYYY-MM-DD
---

# Tasks NNNN — <Feature name>

> **Phase 3 of 4 — the ordered checklist.** Decompose [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable* — each one a coherent diff a reviewer
> could read in isolation. `/ss-implement` works this list top to bottom and keeps the status
> boxes current. Gate: do not start `/ss-implement` until a human marks this `approved`.

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
- [ ] Relevant `pytest` tests identified for the maintainer to run (listed in `plan.md`).
- [ ] `python scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [ ] Diff is *what was asked, and only that* — no scope creep, no foreign files touched.
- [ ] `spec.md` status set to `done`.
