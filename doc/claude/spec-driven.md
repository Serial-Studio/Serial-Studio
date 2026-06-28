# Spec-Driven Development

The default workflow for non-trivial work in this repo. Instead of steering a feature through
an open-ended chat ("prompt engineering"), the intent is captured as reviewable artifacts that
gate each other: **spec -> plan -> tasks -> implementation**. Each phase produces a document a
human approves before the next begins. The contract moves from "did the agent guess right" to
"did we agree on this, in writing, before code existed".

This is not a generic framework bolted on top of the repo. It reuses what is already here — the
`doc/claude/` sub-docs as the knowledge base the plan reads, the `scripts/` linters as the
verification gate, the existing skills (`ss-hotpath`, `ss-new-driver`, `ss-verify`,
`qt-cpp-review`, `ss-cpp-modern`, `cpp-compiler-flags`), and the Trust Contract from CLAUDE.md.
The workflow just sequences them.

## Why spec-driven over prompt engineering

- **The design is reviewable before it is built.** A wrong approach gets vetoed in `plan.md`,
  when changing it is free, not in a 600-line diff.
- **Tradeoffs surface up front, as decisions.** The working-relationship rule ("surface
  tradeoffs as decisions, not after-the-fact notes") is built into the plan template.
- **The hotpath and silent-breakage classes get a required, explicit answer** every time —
  not an afterthought discovered when frames stop flowing.
- **Work is resumable.** A half-finished feature is three files (`spec`/`plan`/`tasks`), not a
  lost chat. Context summarization, a new session, or a different person can pick it up.
- **A spec is a durable artifact.** It travels with the code, can be linked from a PR, and
  records *why* — not just *what* — for future-you.

## The four phases

| Phase | Skill | Produces | Gate |
|-------|-------|----------|------|
| 1. Specify | `/ss-spec` | `spec.md` — WHAT & WHY: problem, goals, non-goals, numbered requirements, acceptance criteria, constraints | Maintainer marks spec `approved` |
| 2. Plan | `/ss-plan` | `plan.md` — HOW: affected files, data flow, hotpath/threading impact, data model, API/QML, tradeoffs, risks, test plan | Maintainer approves the design |
| 3. Tasks | `/ss-tasks` | `tasks.md` — ordered, individually-verifiable checklist + Definition of Done | Maintainer approves the breakdown |
| 4. Implement | `/ss-implement` | code, task by task; `tasks.md` kept current | Definition of Done met; self-review; sanitize |

Artifacts live in `doc/claude/specs/NNNN-slug/` (see
[specs/README.md](./specs/README.md) for numbering and the `status:` lifecycle). The templates
the skills copy from are in `doc/claude/specs/templates/`.

## Gate discipline

The gates are the point. **Do not run the next phase until a human has approved the current
one.** The agent writes a phase, stops, and presents it; the maintainer reviews and either
sends it back or approves. Skipping ahead — planning an unreviewed spec, coding an unreviewed
plan — defeats the workflow and reintroduces exactly the "guessed wrong, found out late"
failure mode it exists to prevent.

Each phase reads the previous artifact as its source of truth. If a later phase reveals that an
earlier one was wrong (planning exposes a bad requirement, implementation exposes a missing
task), **go back, amend the earlier document, and re-confirm** — do not silently diverge. The
artifacts must stay true; a stale spec is worse than none.

## When to use it — and when to skip

Spec-driven is the **default for non-trivial or multi-file work** (this folds in, and now
operationalizes, the CLAUDE.md "plan before multi-file changes" and "state the plan before
non-trivial work" rules). Reach for it when a reasonable reviewer could prefer a different
approach: a new driver, a new widget, a schema change, an API surface, anything touching the
hotpath.

**Skip it for the genuinely trivial** — a typo, a one-line fix, a rename, a comment, a doc
tweak. Forcing a four-phase ceremony onto a one-liner is its own kind of waste. The boundary is
judgment, same as before; the rule of thumb is the existing one: if the change is small,
obvious, and a reviewer could not reasonably prefer a different approach, just make it.

## How it composes with the rest of the repo

- **Hotpath** — `/ss-plan` requires an explicit hotpath/threading answer and pulls in
  `ss-hotpath`; `/ss-implement` reads the hotpath files in full and routes `--benchmark-hotpath`
  to the maintainer.
- **Verify** — `/ss-implement` runs `code-verify --check` per task and `sanitize-commit.py` at
  the end (via `ss-verify`); `qt-cpp-review` runs on the C++ diff before handoff.
- **Trust Contract** — the plan's file list *is* the lane. Anything outside it is named in chat,
  not slipped into the diff. Foreign working-tree files are never touched. Nothing is committed
  without explicit per-turn permission. A self-review ("what was asked, and only that") is part
  of the Definition of Done.
- **Tests** — acceptance criteria map to concrete checks: `tests/scripts/` JS units the agent
  can run, and `pytest` integration/security/perf suites the maintainer runs against a live
  app.

## One-line mental model

`/ss-spec` agrees on the problem. `/ss-plan` agrees on the solution. `/ss-tasks` agrees on the
steps. `/ss-implement` executes, verifies, and proves it — with a human gate between each.
