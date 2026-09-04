# Manual Heuristics — Ordinal Rubric (0 to 4) for doc/help Entries

Ten heuristics for scoring a manual page, each 0-4 with a P0-P3 severity tag. Use in the
review workflow after the linter and the second-order pass; the rubric catches what neither
does (a phrase-clean, structurally-varied page that still documents the wrong default).

Adapted from claude-blog's `editorial-heuristics.md` (MIT), itself an adaptation of Nielsen's
10 Usability Heuristics (NN/g, 1994 rev. 2020) via the impeccable plugin (Apache 2.0). Blog
concerns (SEO metadata, author bio, E-E-A-T, FAQ schema) are replaced with manual concerns
(ground truth, edition gating, help.json graph).

## Scale and severity

| Score | Meaning | Severity |
|---|---|---|
| 0 | Absent or actively wrong | P0 — blocking (wrong fact, misleading page) |
| 1 | Major gaps | P1 — fix before handoff |
| 2 | Mixed | P2 — fix soon |
| 3 | Good, minor gaps | P3 — polish |
| 4 | Excellent (rare) | none |

Any 0 or 1 generates at least one P0/P1 in the report. A WRONG ground-truth claim is P0
regardless of the other nine scores.

## The ten heuristics

1. **Visibility of intent.** First paragraph says what the feature is, whether it is Pro or
   free, and where it lives in the UI. Reader knows in five seconds whether this page answers
   their question.

2. **Heading-content match.** A heading is a contract; the section delivers it in its first
   100 words, in the order the heading implies. No bait-and-switch, no burying.

3. **Reader control and exit.** Sections are self-contained enough to enter cold; related
   pages are linked where the reader needs them (not only in a footer); the page sits in the
   right `help.json` section so navigation finds it.

4. **Terminology consistency.** The UI string is the term, used identically throughout and
   matching `Glossary.md` — no rotating "frame parser" / "parser script" / "JS parser" for
   variety, no drift from what the app actually displays.

5. **Ground-truth prevention.** Every default, range, UI label, shortcut, file format, and
   gating claim is verifiable against `core/` / `app/src` / `app/qml` — and was verified (see
   ground-truth-factcheck.md). "The app handles this automatically" without saying what it
   does is a 1; a wrong default is a 0.

6. **Recognition over recall.** The reader never needs an unnamed other page's context to
   follow this one: jargon is defined inline or linked to the Glossary; prerequisites are
   named and linked; comparisons use tables, not buried prose.

7. **Skimmer vs deep-reader flexibility.** Configuration is in tables, procedures are in
   numbered steps, theory is in prose — each mode of reading is served by the right shape,
   and a skimmer can extract the parameter they need without reading the theory.

8. **Information density.** Every paragraph earns its place: no throat-clearing intro before
   the first substantive claim, no summary paragraph restating the section, no padding to
   make a thin feature look documented. Page length matches feature scope.

9. **Failure recovery.** The page anticipates the reader whose setup doesn't work: platform
   notes where behavior differs, symptoms and causes for common failures, a link to
   `Troubleshooting.md` when deeper diagnosis lives there.

10. **Embedded in the manual graph.** Registered in `help.json`; cross-links run both
    directions (this page links relatives, relatives link back); terms it introduces exist
    in `Glossary.md`; no orphan pages.

## Report format

```
## Manual heuristics: <page>

| # | Heuristic | Score | Severity | Note |
|---|-----------|-------|----------|------|
| 1 | Visibility of intent | 3 | P3 | Pro gating stated late |
| 5 | Ground-truth prevention | 0 | P0 | Baud default documented as 115200; code says 9600 |
| ... | ... | ... | ... | ... |

### Prioritized fixes
- P0: ...
- P1: ...
- P2: ...
- P3: ...
```

Cross-check against the factcheck verdicts: a page scoring 3+ across the board while carrying
a WRONG claim means heuristic 5 was scored without running the factcheck — run it.
