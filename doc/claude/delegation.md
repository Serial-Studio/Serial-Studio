# Delegation & Cost — Advisor Economics

Quality concentrates in planning and review; bulk token generation doesn't need the strong
model. Anthropic's advisor-tool measurements and the spec-0069 post-mortem agree: the main session
advises, cheaper executors type.

- **The main session plans, reviews, and integrates; mechanical execution is delegated.**
  Applying an approved brief, repetitive multi-file sweeps, test-writing against a fixed
  pattern: run them in subagents with a cheaper model override (Sonnet by default, Haiku
  for purely mechanical work). Design decisions, hotpath edits, and anything on a
  protected surface stay in the main session. An executor brief names a donor file whose
  pattern the output matches; a brief without one produces code that fits nothing.
- **Two high-tier touchpoints per task, not continuous involvement.** One after
  orientation but before committing to an approach (the `/ss-plan` gate), one before
  declaring done (the self-review gate). Between them the strong model directs; it
  doesn't type.
- **Briefs and reports are budgeted.** A per-component brief fits half a page: files,
  the invariants that bind, one acceptance check. Subagents return conclusions, never
  file dumps. Measured: hard output caps cut advice volume ~7x with no detectable quality loss, so cap first
  and raise only on evidence.
- **Searches fan out, conclusions come back.** Multi-file exploration runs in read-only
  search subagents; the main context receives the answer, not the excerpts, which also keeps the cached
  prompt prefix stable. A line number a subagent reports is a hint: re-read the slice
  before editing it. A whole-file read over 800 lines is refused and redirected there,
  the read-in-full hotpath files excepted (`SS_READ_GATE_LINES` retunes it). Enforced:
  hook:large-read-gate.py.
- **No process tax on trivial work.** A mandated consult/plan step on tasks whose first action needs no
  planning measured net-zero quality gain at pure cost. Keep the "skip for trivial" escape
  hatches; never write a rule that both mandates and restrains one mechanism.
