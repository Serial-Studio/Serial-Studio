---
name: ss-ai-audit
description: >-
  Audit Serial Studio's AI-facing material (CLAUDE.md, doc/claude/**, .claude/skills/**, and
  optionally the in-app assistant corpus under app/rcc/) against code ground truth. Use when
  asked to "audit the skills/docs", "check the docs for drift", "verify CLAUDE.md is still
  true", or after a refactor that moved/renamed things (TU splits, workflow consolidation,
  symbol renames). Finds stale claims and fixes them with targeted edits; docs-only, never
  touches code.
argument-hint: "[scope: skills | docs | all | <path>]"
---

# Serial Studio — AI-facing doc audit

AI-facing docs steer future edits only while they are true; a stale claim is worse than no
claim because it gets verbalized with confidence at the point of action. Code evolves, docs
don't — this skill is the periodic re-sync. Two sweeps have been needed already (2026-06
corpus sweep, 2026-07 skills audit), so treat drift as expected, not exceptional.

## Procedure

0. **Run the mechanical pass first.** `python3 scripts/claim-verify.py` resolves every
   backticked repo path, `file:line` citation, Markdown link, `Class::method`, bare camelCase
   identifier and pinned constant in the AI-facing tier against the tree, and writes
   `.claim-report`. It is a gate in CI and in `sanitize-commit.py`, baselined by
   `scripts/claim-baseline.json`, so anything it can catch should already be caught. Start
   from its advisories (identifiers that vanished are the richest seam) and spend the manual
   pass on what it structurally cannot check: whether a *paragraph* still describes the
   mechanism, whether a step list is complete, whether a rule still binds.

1. **Enumerate the claim-bearing files** in scope: `CLAUDE.md`, `doc/claude/**` (including
   `architecture/`), `.claude/skills/*/SKILL.md` + `references/`, and — if asked — the in-app
   assistant material under `app/rcc/`.

2. **Extract the verifiable claims.** A claim is anything checkable against the repo: a file
   path, a symbol (`Class::method`), a numeric constant, a CLI flag or default, a workflow
   file name, a pipeline step list, an enum value list, a registration touch-point list, a
   "this file contains X" example. Skip opinions and rules — audit facts.

3. **Verify each claim against ground truth** with `Grep`/`Read` — the code is the truth, the
   doc is the suspect. For volume, fan out parallel read-only subagents, one per skill or
   subsystem, each with an explicit numbered claim list and a
   VERIFIED / WRONG (+ correct fact) / NOT FOUND verdict format with `file:line` evidence.
   Keep the missions disjoint (`doc/claude/j-space.md`, named lenses).

4. **Fix with targeted edits — and fix every mirror.** Wrong facts propagate: the same claim
   typically lives in CLAUDE.md, a `doc/claude/` sub-doc, and one or more skills. After
   correcting one instance, grep the wrong literal across all AI-facing files and fix every
   copy in the same pass. Before each fix, state the ground-truth evidence (`file:line`) in
   chat — a correction you can't evidence is a new guess.

5. **Report** what was verified clean vs corrected, with the evidence. Do not silently fix.

## Known drift magnets (check these first)

- **Moved implementations** — TU splits and renames leave docs pointing at the old file
  (e.g. `ProjectEditor.cpp` → `Project/ProjectEditorShared.h` + `ProjectEditorForms.cpp`).
- **CI workflow names** — jobs get consolidated/renamed (`test.yml`/`deploy.yml` → `ci.yml`).
<!-- claim-verify off -->
- **Interface vs concrete attribution** — a method documented on the interface that only the
  concrete class carries (`IScriptEngine::guardedCall` → `JsScriptEngine::guardedCall`).
<!-- claim-verify on -->
- **Look-alike constants** — two real numbers conflated (65536 enqueue queue vs 4096
  `kCapturedPoolSize`). Verify which structure the number belongs to, not just that it
  appears in code.
- **Pipeline/checklist step lists** — scripts grow steps (`black`, `generate-sdk`) that the
  doc's enumeration silently omits.
- **"Known bug" examples** — landmines get fixed; a doc citing one as *present* is stale
  (reframe as "was fixed; flag reintroduction"), and "e.g. in file X" sightings get cleaned
  up (`Q_FOREACH` in `BluetoothLE.cpp`).
- **Registration touch-point lists** — new switches/resources appear (icon `.qrc` entries,
  extra `BusType` switches); re-grep a recently added enum value and mirror every hit.

## Rules

- **Docs-only.** Never "fix" code to match a doc — if the code looks wrong, say so in chat;
  the doc records reality, the maintainer changes reality.
- Every correction carries `file:line` ground-truth evidence.
- Stay in the AI-facing lane; user-facing docs (`README.md`, `doc/help/**`) have their own
  linter (`documentation-verify.py`) and tone rules — don't restyle them from here.
- When a skill's facts change, check `doc/claude/repo-skills.md` and CLAUDE.md's pointers to
  it still describe it correctly.
