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

Before any commit, run the top-level driver, which chains: chmod (POSIX) → expand-doxygen →
clang-format → code-verify --fix → clang-format → code-verify --check → singleton-census gate
(spec 0039, blocking) → tu-census gate (blocking) → black (Python) → documentation-verify →
claim-verify gate (blocking) → generate-sdk (regenerates
`SerialStudio.js`/`.lua` from `api-schema.json`) → generate-command-strings →
generate-property-registry (regen + --check + --check-snapshot, specs 0036/0037) →
registry-verify → search-index rebuild → changed-file summary. It only sanitizes the
working tree — committing and pushing stay with the developer:

```
python scripts/sanitize-commit.py
```

## Fixing what the linter reports

Mechanical findings (formatting, naming, header order, a missing `[[nodiscard]]`) need no
ceremony — just fix them. When the fix is anything more — a suppression, a restructure, a
signature change — first name the rule being violated and its cause in one sentence, then fix
the cause, not the symptom (`doc/claude/j-space.md`, verbalize-to-load). Pattern-matched
fixes silence the linter line while leaving the underlying issue (a suppression comment where
a restructure was due, a renamed symbol that still leaks the design problem).

## Reminders

- Commit messages are Conventional Commits (`feat|fix|chore|docs|style|refactor|perf|test(scope): …`).
- Markdown narration/marketing copy is linted by `scripts/documentation-verify.py` (writes
  `.doc-report`); CLAUDE.md is exempt.
- Only commit or push when the developer explicitly asks.
