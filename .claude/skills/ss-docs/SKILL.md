---
name: ss-docs
description: >-
  Write, update, or review Serial Studio's user-facing documentation: the doc/help manual,
  README.md, and examples READMEs. Use when asked to "update the documentation", "update the
  docs", "write a help entry", "review the documentation", "edit doc/help", "document <feature>",
  or when touching any Markdown under doc/help/. Grounds every factual claim in code, mirrors
  the manual's voice and structure, registers new pages in help.json, and runs a two-tier
  AI-writing-tell check (documentation-verify.py first, structural pass second) before handoff.
argument-hint: "[update <file> | review <file|dir> | new <topic>]"
---

# Serial Studio — user-facing docs (write, update, review)

The manual competes with the code for the user's trust; a wrong default or a blog-voiced
paragraph loses it. This skill covers `doc/help/**`, `README.md`, and `examples/**/README.md`.
The AI-facing material (CLAUDE.md, `doc/claude/**`, the skills) belongs to `ss-ai-audit` —
don't restyle it from here.

Adapted from AgriciDaniel/claude-blog (MIT): its editorial machinery, rescoped from SEO-blog
writing to a technical manual. SEO/GEO advice (question headings, statistics-per-word quotas,
summary boxes, keyword placement) was deliberately dropped — a manual is not a blog post.
E-E-A-T survives the translation: [references/eeat-manual.md](references/eeat-manual.md)
carries the experience/expertise/authority/trust signals in manual form, trust evaluated
first.

## Invariants — name the ones that bind before editing

Per the J-space discipline, state 3-5 of these in chat, in your own words, right before the
edit:

1. **The code is the ground truth.** Every default value, range, UI label, menu path, Pro
   gate, and behavior claim is verified with `Grep`/`Read` against `core/` / `app/src` /
   `app/qml` before it is written — never copied from another doc or from memory. Procedure
   and verdict format: [references/ground-truth-factcheck.md](references/ground-truth-factcheck.md).
2. **Manual voice, not blog voice.** Declarative present tense, no tutorial "we/let's", no
   marketing adjectives, no meta-references ("in this guide"). `scripts/documentation-verify.py`
   is the phrase-level contract — run it, don't re-derive it.
3. **Linter-clean is necessary, not sufficient.** After the linter passes, run the structural
   pass in [references/second-order-tells.md](references/second-order-tells.md) — the tells
   that survive a vocabulary fix.
4. **Structure mirrors the neighbors.** Read 1-2 sibling entries from the same `help.json`
   section before writing; match their shape (overview first, configuration tables, numbered
   quick starts, platform notes).
5. **New pages register in `doc/help/help.json`** (`id`, `title`, `section`, `file`) and
   cross-link in both directions — the page links its relatives, and at least one existing
   page links back.

## Update workflow

1. **Read the target entry in full**, plus 1-2 siblings from the same `help.json` section.
2. **Extract the claims your edit touches** and verify each against code
   (ground-truth-factcheck.md). A claim you cannot evidence does not go in the doc.
3. **Name the binding invariants in chat** (3-5, from the list above).
4. **Edit, don't rewrite.** Targeted edits that keep the entry's existing structure and any
   passing prose. Keep terminology aligned with the UI strings and `Glossary.md`.
5. **Gate before handoff:**
   - `python scripts/documentation-verify.py <files>` — fix every finding or justify a
     `<!-- doc-verify off -->` fence (fences are a review trigger).
   - Second-order structural pass per second-order-tells.md.
6. **Counterfactual check:** name the claim in your diff most at risk of being wrong and the
   `file:line` evidence that it isn't.

## Review workflow (read-only)

When asked to review rather than change, report — do not fix unless asked.

1. **First-order:** run `python scripts/documentation-verify.py <scope>` and summarize its
   findings; don't duplicate them by hand.
2. **Second-order:** structural pass per second-order-tells.md, each hit with file:line and a
   quoted example.
3. **Ground-truth factcheck:** extract the checkable claims, verify against code, report
   VERIFIED / WRONG (+ correct fact) / NOT FOUND verdicts with `file:line` evidence.
4. **Heuristics rubric:** score the entry 0-4 on the ten manual heuristics in
   [references/manual-heuristics.md](references/manual-heuristics.md); tag findings P0-P3.
5. **E-E-A-T pass:** trust first (factcheck verdicts, gating disclosure, scope honesty — any
   failure is P0), then experience/expertise/authority per
   [references/eeat-manual.md](references/eeat-manual.md). A page with no worked example, no
   concrete value, and no edge case is a UI screenshot in prose — P1.
6. **Report prioritized P0 → P3.** A WRONG factual claim is always P0. Never silently fix.

## New-entry checklist

1. Pick the `help.json` section it belongs to; read that section's entries for shape.
2. Draft with the shape the section uses: `# Title` (sentence case), `## Overview` stating
   what the feature is and its edition gating (Pro or free), configuration tables, numbered
   quick starts, platform notes where behavior differs.
3. Every factual claim carries ground-truth evidence you actually read this session.
4. Write the experience signals in, don't bolt them on: at least one worked example with
   real output, real platform values, the feature's honest limits (eeat-manual.md).
5. Register in `help.json`; add cross-links both directions; add new terms to `Glossary.md`
   if the manual defines them nowhere else.
6. Run both gate tiers (update workflow step 5).

## Rules

- **Docs-only.** If the code looks wrong while fact-checking, say so in chat — never change
  code from this skill, and never document the intended behavior instead of the actual one.
- Em dashes are allowed sparingly; the linter's density and ` -- `-substitute rules are the
  contract (the upstream blog plugin's "zero em dashes" rule does not apply here).
- `sanitize-commit.py` runs `documentation-verify.py` as part of the pre-commit pipeline;
  passing the gate here means no surprises there.
- Keep the manual's terminology stable: the UI string is the term ("Project Editor", "frame
  parser", "Setup panel") — no synonym rotation for variety.
