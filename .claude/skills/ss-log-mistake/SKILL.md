---
name: ss-log-mistake
description: >-
  Append a row to Serial Studio's mistakes ledger (doc/claude/common-mistakes.md) for a defect
  just caught, and decide whether the class can be made mechanical. Use when asked to "log this
  mistake", "add this to common-mistakes", "make sure this doesn't happen again", or right after
  the maintainer corrects a repeated error. A row closes with `Codified:` naming the lint rule,
  anchor, hook, or test that now catches it, or `not yet`.
argument-hint: "[what went wrong]"
---

# Serial Studio — /ss-log-mistake

Writing a mistake down is not the fix. The fix is the check that makes the class harder to
repeat; the row exists to say which check that is, or to admit there is none yet. Adapted from
the `/log-mistake` command and defect ledger in PrimeFoldTools/andon (MIT).

## Procedure

1. **Name the class before the row.** One chat sentence: the symptom, why it was silent, and
   which family it belongs to. Claim exceeded evidence (said done, had not checked). Decision
   outran reality (acted on a stale map: an old handoff, a cached agent result, memory instead
   of the tree). Rule was a convention, not enforcement (a comment said "keep in lockstep").
   Pressure replaced evidence (retracted a right answer when challenged). Naming the family is
   what lets you spot the next member before it lands (`doc/claude/j-space.md`,
   verbalize-to-load).

2. **Check it is not already there.** Grep `doc/claude/common-mistakes.md` and CLAUDE.md for
   the symbol, path, or phrase. The ledger never restates a CLAUDE.md rule: if the mistake is a
   plain rule violation, the row records only the silent-breakage mechanism the rule does not
   explain, and points at the rule. If a row already covers the class, sharpen that row instead
   of adding a near-duplicate.

3. **Pick the section** by subsystem. Workflow and collaboration mistakes go under
   "Process & Trust"; the section list is the table of contents at the top of the file.

4. **Write the row in the existing two-column shape.**
   - *Mistake*: the concrete wrong action, phrased so a reviewer would recognize it in a diff or
     a chat reply. Not "was careless": name the call, the file, the claim.
   - *Fix*: the correct action, then the one-clause *why it is silent* (what the compiler, the
     linter, or the reviewer cannot see). Dates and commit hashes when the row records a real
     incident. End with `Codified:` (step 5).

5. **Ask the mechanical question, per tier.** Could the class be caught by:
   - a `scripts/code-verify.py` rule (a structural pattern in C++/QML/Python),
   - a `scripts/claim-verify.py` anchor in `scripts/doc-anchors.json` (a value quoted in two
     places that must agree),
   - a hook in `.claude/hooks/` (a per-turn or per-tool-call behavior),
   - `scripts/layer-verify.py` or `scripts/registry-verify.py` (a dependency edge, a command or
     icon registration),
   - a ctest or pytest (a runtime property)?

   If one fits, say which in chat and offer to build it as its own pass; the row lands now with
   `Codified: not yet` and is updated when the check ships. Bundling the countermeasure into
   the same diff as whatever was being fixed is scope creep (trust contract, "stay in your
   lane"). If no tier fits, say why in one clause and leave `not yet`: an honest `not yet` is
   the queue the next hook comes from. `scripts/ledger-verify.py --check` (sanitize and CI)
   refuses a tree whose open-row count grew, so a `not yet` row lands only by closing another
   row with a real check, or by `--accept` after the maintainer's deliberate decision.

6. **Handoff.** Quote the finished row in chat. Nothing else in the file changes.

## Don't

- Add a row for a one-off typo or a mistake the linter already reports; the ledger is for
  silent breakage.
- Write "remember to" in the Fix column. If the fix is a reminder, it belongs in step 5 as a
  candidate hook, not in the row.
- Mark `Codified:` with a check that does not exist in the tree yet.
