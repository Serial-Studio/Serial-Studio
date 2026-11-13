# Specs

Spec-driven development artifacts. Each feature gets a numbered directory holding the two
documents the workflow produces. The workflow itself is documented in
[`../spec-driven.md`](../spec-driven.md). The skills that drive it are `/ss-spec` and
`/ss-implement`.

## Layout

```
doc/claude/specs/
  README.md            <- this file
  templates/           <- skeletons copied by the skills (do not edit per-feature)
    spec.md
    tasks.md
  NNNN-short-slug/      <- one directory per feature
    spec.md            <- /ss-spec       what, why, acceptance criteria, and the design
    tasks.md           <- /ss-implement  ordered, individually-verifiable checklist
    plan.md            <- specs before 2026-11-12 only: the design, as its own file
```

## Numbering

Zero-padded, four digits, monotonically increasing — `0001`, `0002`, ... The next number is
`max(existing) + 1`; `/ss-spec` computes it. Numbers are never reused, even for a shelved
spec. The `short-slug` is a few kebab-case words naming the feature (`0007-canbus-filter`).

## Status lifecycle

Tracked in the `status:` frontmatter of `spec.md`:

- `draft` — being written; not yet a contract.
- `approved` — the maintainer accepted the spec and its design; implementation may begin.
- `in-progress` — implementation underway (`/ss-implement` sets this).
- `done` — all acceptance criteria met and verified.
- `shelved` — abandoned or deferred; kept for the record, number not reused.

## Why these live in git

A spec is a reviewable, durable artifact, not scratch. Tracking it next to the architecture
sub-docs means a design can be vetoed before code exists, a half-finished feature can be resumed
by reading two files instead of reconstructing a chat, and a PR can point at the spec it
implements.
