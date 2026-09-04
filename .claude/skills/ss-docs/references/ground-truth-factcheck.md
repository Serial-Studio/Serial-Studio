# Ground-Truth Factcheck — Verify Manual Claims Against Code

Every checkable claim in a doc/help entry is verified against the repository before it ships
or passes review. The code is the truth; the doc is the suspect. This is the manual-facing
sibling of `ss-ai-audit` (which audits the AI-facing docs with the same stance).

Adapted from claude-blog's `blog-factcheck` skill (MIT): its claim-extraction table and
verdict scoring, with WebFetch-against-cited-URLs replaced by `Grep`/`Read` against `core/`,
`app/src`, `app/qml`, and `doc/help/help.json` — a manual's sources are not on the web.

## What counts as a claim

Extract anything a `Grep`/`Read` could settle:

| Kind | Example | Typical ground truth |
|------|---------|----------------------|
| Default value | "Baud rate default: 9600" | Driver ctor init list / member init in `core/Devices/IO/Drivers/` |
| Range / options | "Data bits: 5, 6, 7, 8" | The enum, combo-box model, or validator that feeds the UI |
| UI label / menu path | "Settings → Miscellaneous → Enable API Server" | `app/qml/**` strings; the label must match verbatim |
| Edition gating | "Requires a Pro license" | `SerialStudio::activated()` / `commercialCfg()` call sites guarding the feature |
| Behavior | "Reconnects automatically on disconnect" | The slot/handler implementing it |
| File / format | "Exports MDF4" / port name examples | The exporter/driver code; platform-specific literals |
| CLI flag | "`--benchmark-hotpath`" | Grep the exact flag string in `app/src` / `core/` |
| Cross-reference | `[UART](Drivers-UART.md)` | Target file exists; entry registered in `help.json` |

Skip opinions, theory primers, and protocol background — audit facts, not prose.

## Procedure

1. Build the claims list for the page(s) in scope: claim text, kind, location (heading or
   line).
2. For each claim, find the ground truth with `Grep` (exact user-facing string first, then
   the symbol) and `Read` the hit in context. `tests/` can corroborate behavior claims but
   never replaces the implementation as the source.
3. Assign a verdict. Never infer a verdict from another Markdown file — docs cross-copy each
   other's mistakes, which is how a wrong claim survives review.
4. For volume (a whole section of the manual), fan out parallel read-only subagents, one per
   page, each with its explicit numbered claim list and the verdict format below.

## Verdicts

| Verdict | Meaning | Required evidence |
|---------|---------|-------------------|
| VERIFIED | Code matches the claim | `file:line` |
| WRONG | Code contradicts the claim | `file:line` + the correct fact |
| NOT FOUND | Ground truth could not be located | The searches tried; flag for the maintainer, do not guess |

A paraphrase that changes meaning ("about 10 kHz" for a 256 kHz gate) is WRONG, not close.
A claim whose ground truth is genuinely runtime-dependent (OS behavior, hardware timing)
gets NOT FOUND with a note, not a hedge in the doc.

## Report format

```
## Factcheck: <page>

Claims: <N> — Verified: <n> | Wrong: <n> | Not found: <n>

| # | Claim | Kind | Verdict | Evidence |
|---|-------|------|---------|----------|
| 1 | "Baud default 9600" | default | VERIFIED | UART.cpp:88 |
| 2 | "DTR default Off" | default | WRONG — default is On | UART.cpp:92 |
| 3 | "reconnect backoff 2 s" | behavior | NOT FOUND | grepped "reconnect", "backoff" |
```

## Rules

- **Docs-only.** If the code looks wrong (a default that contradicts the UI text, a gate
  missing on a Pro feature), say so in chat — never edit code from a docs task, and never
  document intent instead of the code's actual behavior.
- Every WRONG fix carries its `file:line` evidence in chat before the edit; a correction you
  cannot evidence is a new guess.
- When a fix changes a fact that other pages repeat, grep the wrong literal across
  `doc/help/**` and `README.md` and fix every mirror in the same pass.
