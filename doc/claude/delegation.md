# Delegation

The maintainer set the default on 2026-11-12: the main session writes the code, subagents read.
Bulk agent packages produced the two worst incidents here (the 2026-09-03 licensing inversion and
the spec-0069 generator), and every line an agent adds is a line the maintainer has to audit.

- **The main session writes the code, by hand.** It reads the file, makes the design decision
  and types the edit. Hotpath edits and protected surfaces never leave it.
- **Subagents search and review, read-only.** Multi-file exploration runs in search subagents;
  the main context receives the answer, not the excerpts. A line number a subagent reports is a
  hint: re-read the slice before editing it. A whole-file read over 800 lines is refused and
  redirected there, the read-in-full hotpath files excepted (`SS_READ_GATE_LINES` retunes it).
  Enforced: hook:large-read-gate.py.
- **Executor fan-out only on request.** When the maintainer asks for it in that message, each
  agent gets disjoint files and a half-page brief: the files, the invariants that bind, one
  acceptance check, and a donor file whose pattern the output matches. Agents never invent a
  generator, never touch a protected surface and never commit; the main session integrates the
  shared files and reviews every diff.
- **Reports are budgeted.** Subagents return conclusions, never file dumps. Measured: hard
  output caps cut advice volume ~7x with no detectable quality loss.
