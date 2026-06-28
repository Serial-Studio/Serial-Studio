---
name: ss-plan
description: >-
  Phase 2 of Serial Studio's spec-driven workflow: turn an approved spec.md into a technical
  design (plan.md) — files, data flow, hotpath/threading impact, tradeoffs, risks, test plan.
  Use after /ss-spec is approved, or "plan this spec", "design the implementation". Reads the
  doc/claude sub-docs and the real code; gates on human approval before /ss-tasks.
argument-hint: "[spec number or slug]"
---

# Serial Studio — /ss-plan (phase 2 of 4)

Turn the approved `spec.md` into a concrete technical design. This is the **HOW**: which files,
which subsystems, how data and threads move, and which design was chosen over which
alternatives. See [doc/claude/spec-driven.md](../../../doc/claude/spec-driven.md) for the gate
discipline.

## Preconditions

- `doc/claude/specs/NNNN-slug/spec.md` exists and its `status:` is `approved`. If it is still
  `draft`, stop — the spec must be approved first (`/ss-spec`).

## Procedure

1. **Read the spec in full.** Every requirement and acceptance criterion is a target the plan
   must hit.

2. **Read the ground truth — do not design from memory.** Read the relevant `doc/claude/`
   sub-doc(s) *and the actual code* for the area:
   - [architecture.md](../../../doc/claude/architecture.md) — data flow, threading, AppState,
     operation modes, ProjectModel/Editor split, multi-source, `Keys::`, parsers, export/DB.
   - [common-mistakes.md](../../../doc/claude/common-mistakes.md) — the silent-breakage classes
     this change is exposed to.
   - [code-style.md](../../../doc/claude/code-style.md) and
     [directory-map.md](../../../doc/claude/directory-map.md) as needed.
   - If the change touches the hotpath, invoke **`ss-hotpath`** and read `FrameBuilder` /
     `CircularBuffer` / `FrameReader` / `Dashboard` in full per the CLAUDE.md rule.
   - For a new driver, follow **`ss-new-driver`** and grep an existing `BusType` value to find
     every registration touch-point.

3. **Copy the template and write the plan:**

   ```bash
   cp doc/claude/specs/templates/plan.md doc/claude/specs/NNNN-slug/plan.md
   ```

   Fill every section. The **Hotpath & threading impact** section is required — answer it
   explicitly even when the answer is "none". List concrete file paths in **Affected
   subsystems & files**, confirmed by grep, not guessed.

4. **Surface tradeoffs as decisions, up front.** Where two reasonable designs diverge on
   something that matters (perf vs simplicity, fidelity vs readability, scope), put the choice
   in the Tradeoffs table with a recommendation and the one-line why. Recommend, do not
   enumerate. Pull the deciding constraint out of the spec rather than discovering it later.

5. **Map every acceptance criterion to a check** in the Test & verification plan — `pytest`
   files, `tests/scripts/` units, `--benchmark-hotpath`, or a maintainer observation.

## Gate

Stop after writing `plan.md`. Present the design — especially the tradeoffs and the hotpath
answer — for review. **Do not run `/ss-tasks` until the maintainer approves it.** A plan visible
before execution is the contract; this is where a different approach gets chosen, cheaply.

## Rules

- No code yet. The plan describes the change; it does not make it.
- If planning surfaces a question that changes the spec, go back and amend `spec.md` (and
  re-confirm) rather than silently diverging.
- Honor the Trust Contract: the plan's file list *is* the lane — anything outside it must be
  named in chat, not slipped in during `/ss-implement`.
