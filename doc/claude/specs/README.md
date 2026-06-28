# Specs

Spec-driven development artifacts. Each feature gets a numbered directory holding the three
phase documents the workflow produces. The workflow itself — phases, gates, and how it ties
into the hotpath / verify / trust rules — is documented in
[`../spec-driven.md`](../spec-driven.md). The skills that drive it are `/ss-spec`,
`/ss-plan`, `/ss-tasks`, `/ss-implement`.

## Layout

```
doc/claude/specs/
  README.md            <- this file
  templates/           <- skeletons copied by the skills (do not edit per-feature)
    spec.md
    plan.md
    tasks.md
  NNNN-short-slug/      <- one directory per feature
    spec.md            <- /ss-spec     WHAT & WHY, acceptance criteria, non-goals
    plan.md            <- /ss-plan     files, data flow, hotpath/threading, tradeoffs, risks
    tasks.md           <- /ss-tasks    ordered, individually-verifiable checklist
```

## Numbering

Zero-padded, four digits, monotonically increasing — `0001`, `0002`, ... The next number is
`max(existing) + 1`; `/ss-spec` computes it. Numbers are never reused, even for a shelved
spec. The `short-slug` is a few kebab-case words naming the feature (`0007-canbus-filter`).

## Status lifecycle

Tracked in the `status:` frontmatter of `spec.md` (the per-phase docs carry their own gate
status):

- `draft` — being written; not yet a contract.
- `approved` — the maintainer accepted the spec; planning may begin.
- `in-progress` — implementation underway (`/ss-implement` sets this).
- `done` — all acceptance criteria met and verified.
- `shelved` — abandoned or deferred; kept for the record, number not reused.

## Why these live in git

A spec is a reviewable, durable artifact, not scratch. Tracking it next to the architecture
sub-docs means a plan can be vetoed before code exists, a half-finished feature can be resumed
by reading three files instead of reconstructing a chat, and a PR can point at the spec it
implements.
