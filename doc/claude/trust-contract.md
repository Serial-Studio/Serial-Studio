# Trust Contract — Full Text

> Summarized inline in CLAUDE.md; this file holds the complete rules and the incidents that
> produced them. Read once per session if you have not internalized it. Companion reading:
> [working-relationship.md](working-relationship.md).

These rules are about predictability, not productivity — the difference
between a tool the user re-audits every time and a collaborator they rely
on. Capability without predictability gets disabled.

- **Never touch, revert, or restore files outside your own edits — the one
  rule whose violation loses real work.** A working-tree file *you* did not
  edit this session is the user's in-progress work. NEVER
  `git checkout`/`restore`/`reset`/`stash`/`clean` it, overwrite it, or
  "clean it up" — not even when it looks like noise, a generated artifact,
  or stray subagent output. Session-start `git status` is a snapshot, not a
  baseline to restore to. If such a file is in your way or seems wrong,
  *stop and say so in chat* — quote the path, say you did not touch it,
  ask. Restoring even derived artifacts (`.ts`/`.qm`, build output) needs
  explicit per-file permission — you cannot prove the user wasn't mid-edit.
  When unsure whether a file is yours: it is not. This has bitten before (a
  subagent regenerating translation files; a reflexive restore nearly
  discarding hours of uncommitted work) — absolute, not advisory.
- **Stay in your lane.** Every file touched outside the explicit ask costs
  the reviewer an audit pass. Spot an adjacent fix? *Name it in chat*
  ("noticed X — want it in this pass?") rather than slipping it into the
  diff. Bundled scope creep erodes trust in every diff that follows.
- **Show the why, not the what.** Code shows *what*; a comment, chat reply,
  or commit message shows *why* — but only when the choice was non-obvious
  (one of two reasonable approaches, a workaround, a hidden invariant). One
  sentence. When the choice was obvious, say nothing.
- **State the plan before non-trivial work.** Any change where a reasonable
  reviewer could prefer a different approach: plan visible *before*
  execution is the contract — a summary after is not. Operationalized as
  spec-driven development: non-trivial or multi-file work MUST start with
  `/ss-spec`; no implementation lands before an approved `plan.md`. Trivial
  one-liners exempt. See [spec-driven.md](spec-driven.md).
- **Self-review before handoff.** Before declaring a non-trivial change
  done, re-read the diff: is this *what was asked, and only that*? If you
  can't answer yes, say so before claiming completion.
