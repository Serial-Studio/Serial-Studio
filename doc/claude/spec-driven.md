# Spec-Driven Development

How debatable work gets agreed before code exists: one document, one approval.

## When to use it — and when to skip

- **Skip it** when the change is small or obvious and a reviewer could not reasonably prefer a
  different approach: state the plan in a line or two of chat and make the change.
- **Use it** when a reviewer could prefer another approach (a new driver, a new widget, a schema
  change, an API surface), and always when the change touches the hotpath, licensing or the API
  auth and consent files.

## The two steps

| Step | Skill | Produces | Gate |
|------|-------|----------|------|
| 1. Specify | `/ss-spec` | `spec.md`: the problem, goals, non-goals, numbered requirements, acceptance criteria, the premise check, and the design (files, data flow, hotpath and threading answer, tradeoffs, risks, tests) | The maintainer marks it `approved`. The only gate. |
| 2. Implement | `/ss-implement` | `tasks.md` (an ordered checklist, written without a gate and kept current), then the code, task by task | Definition of Done met; self-review |

Artifacts live in `doc/claude/specs/NNNN-slug/` (see [specs/README.md](./specs/README.md) for
numbering and the `status:` lifecycle). Specs written before 2026-11-12 keep their separate
`plan.md`; read it as the design.

If implementation shows the approved spec was wrong, stop, amend `spec.md`, set it back to
`draft` and get it re-confirmed. That is the only second gate; do not silently diverge.

## Before the spec — explore (ungated)

Throwaway prototyping is sanctioned: scratchpad sims (Node/Python), API experiments against a
running app (`localhost:7777`), quick sketches of shapes that will be rejected. Nothing from it
lands in the repo except the premise check: the one measurement that would have killed the spec,
with its number, in the spec's own section. Spec 0069 (TU decomposition) was retired after five
failed builds for want of a coupling figure, and spec 0090 (BOLT) was reverted after eleven
commits for want of one local benchmark; both numbers took an hour.

## Landing

A feature is ONE commit: `spec.md`, `tasks.md`, the code, its tests and docs together, made when
the maintainer asks for it (CLAUDE.md "Commit only when asked"). `scripts/commit-shape.py`,
which `.claude/hooks/commit-shape-gate.py` runs on every commit Claude issues, keeps a
security-sensitive surface in a commit of its own and caps unexplained commits at 60 paths (a
`Mechanical:` trailer naming the generator lifts the cap), because both 2026 user-facing
failures were a change inside a package too large to read. The archive cannot prove a spec was
*approved* before the code; the review conversation does that, so never cite the archive as
proof of gating.

## How it composes with the rest of the repo

- **Hotpath** — the spec's design requires an explicit hotpath/threading answer and pulls in
  `ss-hotpath`; `/ss-implement` reads the hotpath files in full and routes `--benchmark-hotpath`
  to the maintainer.
- **Verify** — `/ss-implement` runs `code-verify --check` per task; `sanitize-commit.py` runs
  only when a generator input changed; `qt-cpp-review` runs on the C++ diff before handoff.
- **Trust Contract** — the spec's file list *is* the lane. Anything outside it is named in chat,
  not slipped into the diff. Foreign working-tree files are never touched.
- **Tests** — acceptance criteria map to concrete checks: `tests/scripts/` JS units the agent
  can run, and `pytest` integration/security/perf suites the maintainer runs against a live
  app.
