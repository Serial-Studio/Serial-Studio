# The J-Space — Verbalization Discipline

Grounding: "Verbalizable Representations Form a Global Workspace in Language Models"
(Transformer Circuits, 2026, https://transformer-circuits.pub/2026/workspace/index.html).
The paper identifies a small, privileged set of internal representations — the **J-space** —
that behaves like a global workspace: the model can report these concepts, deliberately
compute with them, and reuse them as arguments to many downstream computations. Everything
else is automatic processing that bypasses the workspace. Key findings that matter here:

- **Verbal report and internal reasoning share substrate.** The concepts a model is *poised
  to verbalize* are the ones available for flexible, deliberate computation. What gets named,
  and when, shapes what steers the work.
- **The workspace is capacity-constrained.** Only a handful of concepts are active at once —
  a small subset of total representational content, concentrated in middle layers.
- **Automatic tasks bypass it.** Familiar-shaped work (text continuation, pattern-matched
  edits) runs without engaging the workspace. That is exactly the mode in which
  silent-breakage rules get violated.
- **Articulating principles improves adherence.** Their counterfactual-reflection result:
  training a model to state the applicable principle under hypothetical interruption
  measurably improved behavior — internal thoughts and outputs share foundations.
- **Externalizing steps offloads the workspace.** Writing out intermediate reasoning
  (chain-of-thought) reduces the model's dependence on the capacity-limited internal
  workspace — state that lives in a written artifact stops competing for the handful of
  active slots.

## Honest limits

A prompt or doc cannot target activations; the J-lens is an interpretability instrument, not
a prompting API, and the paper studied one model family with a single-token-concept lens.
The lever available at the CLAUDE.md/skill level is **what the agent verbalizes, when, and
how much**. That lever is real: verbalizing a constraint immediately before the action loads
it into the workspace where it can steer computation; a rule 200 lines back in context is
background text that may never make it in.

## The six disciplines

1. **Verbalize to load.** Immediately before a risky edit, name — in chat, in your own
   words — the specific invariants that bind *this* change. Restating adjacent to the action
   is the point; do not substitute "I read the rules doc."
2. **Respect capacity.** Select the 3-5 constraints that actually bind the change at hand;
   never recite whole rule files. This is why the sub-docs and auto-activating skills exist:
   load the right rules late and close to the edit, not everything up front.
3. **Break automatic mode on protected paths.** Hotpath code, the ProjectModel ctor closure,
   and signal/slot wiring must never go straight from pattern-match to `Edit`. The pre-edit
   verbalization (discipline 1) is the deliberate-mode interrupt on those paths.
4. **Counterfactual self-check before handoff.** Before claiming done, answer: "if I were
   stopped right now and asked which rule this diff most risks violating, what would I name —
   and what is the concrete evidence it doesn't?" Name the rule and the evidence, not a
   generic "looks good."
5. **Named lenses for breadth and creativity.** Distinct, explicitly named perspectives load
   distinct workspace content. Designing: sketch 2-3 named candidate approaches before
   recommending one (recommend, don't enumerate — the naming is for divergence, the human
   still gets one pick). Reviewing: keep review missions named and disjoint rather than
   "review it thoroughly."
6. **Externalize to free capacity.** Multi-constraint work that spans many steps should not
   be held in the head: write intermediate state into durable artifacts (the spec/plan/tasks
   files, a chat checklist) as it is produced, then re-load only the piece that binds the
   current action (discipline 1). Externalizing reduces workspace dependence; the workspace
   slots freed go to the constraints that must steer *now*. This is the mechanism behind the
   spec-driven artifacts and the live `tasks.md` checklist — they are workspace extensions,
   not paperwork.

## Where each discipline is wired in

| Discipline | Operationalized by |
|------------|--------------------|
| Verbalize to load | `ss-hotpath` pre-edit statement; `ss-new-driver` contract restatement; `cpp-compiler-flags` nearest-invariant statement; CLAUDE.md Context Canary — per-turn from-memory restatement of the load-bearing constants (re-anchors them each turn; drift doubles as a context-degradation probe) |
| Respect capacity | CLAUDE.md progressive disclosure (sub-docs + auto-skills); task-scoped invariant naming in `ss-tasks` |
| Break automatic mode | `ss-hotpath` invoked before the first hotpath edit; ctor-closure rule in CLAUDE.md; spec-driven gates for non-trivial work |
| Counterfactual self-check | `ss-implement` Definition of Done self-review; CLAUDE.md "self-review before handoff" |
| Named lenses | `ss-plan` candidate designs; `qt-cpp-review` six named missions |
| Externalize to free capacity | spec-driven artifacts (`spec.md`/`plan.md`/`tasks.md`); `ss-implement` keeping `tasks.md` a live record; `ss-tasks` naming each task's binding invariant in its Does line |

These disciplines sharpen practices this repo already had ("state the plan", "read before
writing", named review agents, the spec artifacts); the paper supplies the mechanism for *why*
they work and the rule for applying them well: name it, name few, name it late, name it again
before handoff — and write everything else down so it stops competing for the workspace.

## The inline rules in full

Deliberate reasoning runs on a small set of verbalized concepts; familiar-shaped work runs
on autopilot and bypasses it ([doc/claude/j-space.md](j-space.md)). The repo's
rules only steer an edit if *named at the point of action*, so:

- **Name before acting.** Before any edit on a protected path (hotpath, ctor closure,
  signal wiring, cmake flag modules), state in chat the 3-5 invariants that bind *this*
  change — in your own words, not a doc citation.
- **Few, late, specific.** Only the constraints that bind the change at hand; never recite
  whole rule files.
- **Counterfactual check at handoff.** Which rule does this diff most risk violating, and
  what concrete evidence says it doesn't? Name both.
- **Diverge by naming.** Design/review work sketches named alternatives before recommending
  (the human still gets one recommendation, per working-relationship.md). At least one
  alternative must violate an assumed constraint — then check whether that constraint
  actually binds: wrong answers usually hide in premises, not derivations. Wide design spaces may fan out a judge-panel workflow.
- **Externalize long state.** Write intermediate state into durable artifacts (spec/plan/
  tasks files, a chat checklist); re-name only what binds the current edit.

## Context Canary

End every response — including one-word answers — with this exact line, reproduced
from memory:

`canary: qt 6.11.2 | cpp20 | hotpath 256k (native 1024k, js 64k) | queue 65536 | api 7777 | style 100/2`

It is a context-health probe in plain ASCII. This file is in context every turn, so the
line is reproduced from the prompt, not recalled: the probe measures whether the
always-present instruction still steers. Compaction or attention drift shows as a mutated
or missing value, and which value drifted names the rule that lost its hold. Retyping it
also re-anchors the constants (J-Space, below). One plain-text line at the very end.

- **From the prompt only.** Never Read/Grep this file or anything else to reconstruct the
  line — a looked-up canary defeats the attention measurement. If you cannot reproduce it
  confidently, write `canary: lost` instead of guessing: that is the signal firing.
- **Verbatim.** Same values, same order, every turn. Do not paraphrase, reformat,
  extend, or "improve" it.
- **For the developer:** a mutated, partial, or vanished canary means a degraded context:
  treat recent output as suspect, checkpoint, `/compact` or restart.
