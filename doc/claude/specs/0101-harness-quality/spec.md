---
spec: 0101-harness-quality
title: Harness quality: tested hooks, honest evidence, bounded context, linted hotpath invariants
status: done         # draft -> approved -> in-progress -> done | shelved
created: 2026-10-17
author: Alex Spataru
---

# Spec 0101: Harness Quality

> **Phase 1 of 4, the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

The repo's AI harness turns its behavioral rules into mechanisms: six Stop/PreToolUse/
PostToolUse hooks (dirty-file guard, commit-sanitize gate, verify-on-save, syntax-on-save,
context canary, done-claim check), a deny-listed permission set, growth ratchets, and a
doc-to-tree claim verifier. On 2026-10-17 an outside project (PrimeFoldTools/Andon, PR 13)
adopted the done-claim check's transcript-reading design after six review rounds, which is
external evidence the architecture is sound. The same comparison exposed the debt underneath
it, all of it in the second layer:

- **The hooks have no tests.** Every one is fail-open by design, so a regression in any hook
  is a silent pass forever. The one hook with the same job upstream carries 292 tests and a
  mutation script; ours carry none. This is the exact defect class the done-claim check
  exists to catch, applied to the check itself.
- **The done-claim check credits runs it cannot see.** It accepts any verification command
  whose tool result is not flagged as an error. In this repo's own 28 session transcripts,
  all three markers of a result that reports success without succeeding appear (a
  backgrounded command, a tool timeout, a non-zero exit reinterpreted as success), and 42 of
  76 recorded pytest/ctest invocations were piped into `head`/`tail`, which hides the suite's
  exit status from the harness. A failing suite piped into `head` clears a done-claim today.
- **The canary measures less than it claims.** CLAUDE.md is in context every turn, so a
  "from memory" reproduction is a reproduction from the system prompt. The probe
  still catches compaction and attention loss, but its documentation overstates it, and a
  wrong description of a diagnostic is itself a trust defect.
- **The per-turn instruction load has outgrown its budget.** CLAUDE.md is about 5,000 words
  loaded on every turn; the sub-docs it demands be read "in full" total about 54,000, with
  one subsystem doc alone at 9,400. A session that honestly obeys two such demands spends a
  large share of its working context before touching code, and nothing ratchets the growth.
- **Four hotpath invariants are still prose-only.** CLAUDE.md marks them `Debt: no lint yet`:
  no mutex in the SPSC frame pieces, direct connections between pipeline-thread objects,
  source-owns-time (no re-stamping downstream), and the guarded-call rule for JS. These are
  the rules whose silent violation costs the 256 kHz gate, and they rely on the model
  having read the sentence.

## Goals

- Every hook has a test that drives it through its real entry point with a fixture event and
  transcript, covering the fire case, the stay-silent case, and the fail-open case.
- A done-claim is credited only by a verification run whose recorded result the harness can
  vouch for: finished, not backgrounded, not timed out, not reinterpreted, and invoked in a
  form whose exit status is the verifier's own.
- The canary's documented claim matches what the probe detects.
- The per-turn instruction load is bounded by a checked-in budget that CI ratchets, and
  "read in full" demands are replaced by section-scoped pointers so an edit loads the section
  that binds it.
- The four prose-only hotpath invariants have a mechanical check, each with a known-good and
  a known-bad fixture, and CLAUDE.md no longer says `no lint yet` for them.

## Non-Goals

- A blocking mode for the done-claim check. The verdict stays a warning; a Stop block
  re-invokes the model and spends the tokens the hook exists to avoid (settled by the canary
  hook's design).
- Tying a verification run to the claim's subject. Any honest, visible run of a repo
  verification tool still counts; subject matching is out of scope.
- Detecting a forged transcript, an aliased command, or a fake verifier earlier on PATH.
  The harness-written record is the trust boundary; we do not go below it.
- Rewriting the sub-docs' content. Trimming is structural (scoping and budgeting), not an
  editorial pass on architecture prose.
- Changing what the canary line contains or how often it is emitted.
- Replacing the fail-open posture. Hooks still never block real work on an internal error.

## Requirements

1. **R1. Hook test tier.** Each of the six hooks has an automated test that executes the
   hook as a subprocess with a synthetic hook event and, where the hook reads one, a
   synthetic transcript. For each hook the suite covers: (a) the condition that must produce
   output or a decision, (b) the condition that must stay silent, (c) an internal failure
   (malformed event, missing transcript) that must exit zero with no output. The tests run
   with no app, no Qt and no network.
2. **R2. Mutation proof for the done-claim check.** Each guard in the done-claim check has
   a documented mutation (a one-line breakage) and a named test that fails under it. A test
   that still passes against a hook that fires on everything, or on nothing, does not count.
3. **R3. Recorded-result markers.** The done-claim check does not credit a verification
   run whose recorded tool result carries a backgrounded, timed-out, or reinterpreted-exit
   marker, or whose result is an error, interrupted, or absent.
4. **R4. Exit-status honesty.** The done-claim check credits a verification command only
   when its command line is in a form whose exit status is the verifier's own. A pipe into
   another program without pipefail, a trailing success coercion, a chained `; echo $?`, a
   comment, a leading `exit`/`trap`, a no-run or collection-only flag, and an unquoted glob
   are refused. Refusal withholds credit; it never produces a false warning on a turn that
   made no claim or no edit.
5. **R5. Session binding.** A verification run counts only from the current session's
   transcript, since the last human prompt, on the main thread (not a subagent).
6. **R6. Creditable forms are documented.** The accepted command forms are listed where the
   model reads them, so a session knows how to run tests so they register. The list is
   verified against the tree the same way the canary's values are.
7. **R7. Honest canary description.** The canary's description states what the probe
   detects (loss of attention to the always-present instruction, heavy compaction) and what
   it does not (recall independent of the system prompt). The hook's behavior is unchanged.
8. **R8. Instruction-load budget.** The size of the always-loaded instruction file is
   measured by a repo script and compared to a checked-in baseline; growth beyond the
   baseline fails CI, and an explicit accept re-seeds it, following the existing census
   ratchets.
9. **R9. Section-scoped reads.** Every "read in full" demand for a sub-doc in the
   always-loaded instruction file is replaced by a pointer to a named section, and every
   such section exists under that name. The claim verifier resolves these section anchors
   against the tree.
10. **R10. SPSC no-mutex lint.** A mutex, lock guard, or atomic-with-lock introduced into
    the SPSC frame pieces fails the structural linter, with a known-bad fixture that fails and
    a known-good fixture that passes.
11. **R11. Pipeline direct-connection lint.** A queued or auto connection between two
    objects that both live on the pipeline thread fails the structural linter. The check may
    be conservative (flag only what it can prove), and a documented suppression exists for
    the intended chunk/command/tick-rate hops.
12. **R12. Source-owns-time lint.** A monotonic or wall clock read inside an export, report
    or recording worker fails the structural linter, with the named safety-net helper as the
    single allowed exception.
13. **R13. Guarded-call lint.** A direct invocation of a JS function value outside the
    guarded-call helper, or a watchdog interrupt set outside the watchdog thread, fails the
    structural linter.
14. **R14. Debt markers retired.** CLAUDE.md's four `Debt: no lint yet` markers are replaced
    by the `Enforced:` form the rest of the file uses, each naming its check.

## Acceptance Criteria

- [x] **AC1.** A new pytest module in the tooling tier (`scripts/tests/`, already run by the CI
  lint job) runs all hook tests with no app, no Qt and no network, and passes in CI.
- [x] **AC2.** A mutation script lists every guard of the done-claim check, applies each
  breakage in turn, and reports the named test that failed; the run ends with N of N killed.
- [x] **AC3.** Fixture transcripts containing each of the three success-masking markers, and
  one with a pytest run piped into `head`, produce the warning; the same transcripts with a
  plain passing run stay silent.
- [x] **AC4.** Fixture transcripts from a different session id or a subagent lane do not
  credit a claim in the current session.
- [x] **AC5.** The claim verifier passes with the new section anchors and the creditable
  forms list, and fails when a pointed-to section heading is renamed.
- [x] **AC6.** The instruction-load budget script fails on a file grown past its baseline
  and passes after an explicit accept; CI runs it.
- [x] **AC7.** CLAUDE.md contains zero occurrences of "in full" as a sub-doc read demand (the
  hotpath code read-in-full rule stays) and zero occurrences of `no lint yet`.
- [x] **AC8.** For each of R10 to R13, the structural linter's self-test has one fixture that
  fails and one that passes, and `--check` on the current tree reports no new errors (any
  existing violation is either fixed in this spec or recorded as baseline debt with a
  suppression and a ledger row).
- [x] **AC9.** `--benchmark-hotpath` is unchanged: this spec touches no runtime code except
  where a lint from R10 to R13 exposes a real violation, and each such fix is named.
- [x] **AC10.** The canary hook's test shows identical verdicts before and after this spec
  on the same fixture transcripts.

## Constraints & Invariants

- **Hooks never spend model tokens on a healthy turn and never block on an internal error.**
  Warn-only and fail-open are preserved; the tests assert both.
- **Refusal is one-directional.** A stricter evidence grammar may withhold credit from an
  honest run; it must never emit a warning on a turn that made no edits or no claim.
- **Ratchets, not caps.** The instruction-load budget follows the existing census pattern:
  baseline checked in, growth fails, explicit accept re-seeds.
- **Lints are conservative.** Each new structural rule flags only what it can prove from the
  text; a documented suppression exists and is a code-review trigger, like the others.
- **No new runtime dependency**, no new build step; everything runs in Python 3 already
  required by the scripts.
- **The canary line, its values, and its cadence do not change.** Only its description does.
- **Hotpath code changes only on evidence.** A lint finding is fixed only when the finding is
  a real violation; a false positive is fixed in the lint, never worked around in the code.
- **Protected surfaces untouched.** Nothing here edits licensing, machine-id, or API auth
  code.

## Open Questions

- Resolved in plan: hook tests live in `scripts/tests/`, the tier CI already runs for the
  linter's own fixtures, so no new wiring.
- Resolved in plan: R11 takes a documented identifier list as its input (the pipeline-resident
  objects), flags only a connect whose both endpoints are on that list, and is conservative by
  construction.
- Resolved in plan: R8 gates the always-loaded file and reports the one-hop sub-doc total as an
  advisory line in the same baseline.
