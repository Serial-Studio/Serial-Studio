---
name: ss-verify
description: >-
  Run Serial Studio's structural/style linter and commit pipeline. Use before committing, when
  asked to "verify conventions", "check code style", "lint", "run code-verify", or "sanitize the
  commit". Wraps scripts/code-verify.py and scripts/sanitize-commit.py — the repo's style contract.
allowed-tools: Bash(python scripts/code-verify.py:*), Bash(python scripts/sanitize-commit.py:*)
---

# Serial Studio — verify & sanitize

The scripts in `scripts/` are the style contract. Don't re-derive the rules from CLAUDE.md —
run the linter and read its output.

## Quick check (read-only)

Lint just the files you changed (fast, no writes):

```
python scripts/code-verify.py --check <files...>
```

`--check` reports issues and regenerates `.code-report`. **Errors block CI; advisories are
baseline debt — new code must still clear them.** Hotpath violations (see [ss-hotpath]) are
always blockers. To auto-fix formatting in place: `python scripts/code-verify.py --fix <files...>`.

## Full pre-commit pipeline

Before any commit, run the top-level driver, which chains: clang-format → code-verify (--fix
then --check) → documentation-verify → search-index rebuild → conventional-commit prompt:

```
python scripts/sanitize-commit.py
```

## Reminders

- Commit messages are Conventional Commits (`feat|fix|chore|docs|style|refactor|perf|test(scope): …`).
- Markdown narration/marketing copy is linted by `scripts/documentation-verify.py` (writes
  `.doc-report`); CLAUDE.md is exempt.
- Only commit or push when the developer explicitly asks.
