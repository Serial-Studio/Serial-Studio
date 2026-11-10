# Second-Order AI-Writing Tells — Structural Pass for the Manual

`scripts/documentation-verify.py` is the first-order pass: phrases, marketing vocabulary,
tutorial voice, meta-references, dash misuse. Most AI-generated prose passes that filter and
still reads like AI, because the tells that survive a vocabulary fix are structural and
rhythmic. Run this pass only after the linter is clean; a page is done only when both passes
are clean.

Adapted from claude-blog's `ai-slop-detection.md` (MIT), which in turn adapts the impeccable
plugin's two-tier slop methodology (Paul Bakaus, Apache 2.0) and applies here to manual prose
instead of blog posts. Blog-only checks (listicle intro bloat, TL;DR boxes, question-heading
quotas *in favor* of questions) were dropped or inverted — a manual's norms differ.

## The tells

Report each hit with file:line and a quoted example. Thresholds are per page unless stated.

1. **Hedge stacking — the worst offender in a manual.** "may", "might", "often", "typically",
   "generally", "usually", "tends to". A manual states what the software does; a hedge on
   checkable behavior means the writer didn't check the code. Flag any 20-word window with
   more than one hedge, and *any* hedge on behavior that `Grep`/`Read` could settle. Hedges
   are legitimate only for genuinely environment-dependent behavior (OS quirks, driver
   timing), and then the sentence should name the condition instead of hedging.

2. **Question-cadence headings.** Outside `FAQ.md`, manual headings are noun phrases
   ("Free drivers", "Platform notes"). Flag any question-mark heading outside FAQ content.

3. **Uniform section shape.** Every H2 section lands at the same length with the same
   internal structure (paragraph, table, list — repeat). Real reference sections vary: UART
   needs a page, Process I/O needs a paragraph. Flag when 4+ consecutive sections share
   length within ±20% and identical element order.

4. **Symmetric list bloat.** Every bullet in a list is the same length (word-count SD < 5)
   with the same syntactic skeleton. Real lists vary; some items need three words, one needs
   two sentences. Also flag the bold-lead-in monotony variant: every bullet opening
   `**Term:** explanation` down an entire page.

5. **Bullet-itis.** Prose ideas chopped into bullets that each hold one sentence, or every
   section ending in a bullet list. Tables are for enumerable parameters; bullets are for
   genuinely parallel items; everything else is prose.

6. **Three-clause sentence rhythm.** Most sentences shaped `[clause], [clause], [clause].`
   in a row — the metronomic cadence. Flag when more than half the sentences in any 200-word
   window match.

7. **Triad tic.** Qualities always arriving in threes ("fast, reliable, and flexible").
   Flag 3+ triads per page.

8. **Capsule transitions.** Paragraphs or sections opening with a bare transition word:
   "First," "Next," "Additionally," "Crucially," "Furthermore,". Real prose buries
   transitions inside sentences. Flag when more than a third of paragraph openers do this.

9. **"Here's" openers.** "Here's how...", "Here's what...". Once is fine; three per page is
   a fingerprint.

10. **Wrap-up questions and summary paragraphs.** "What does this mean for you?" or an
    "In summary..." paragraph restating the section. A manual section ends when the
    information ends. Flag every instance outside FAQ content.

11. **False-balance framing.** "While X, it's also worth noting Y" with no real contrast —
    even-handedness theater. Flag more than one per page.

12. **"The key insight is..." / "What's important here is..."** as sentence openers — the
    model telegraphing a summary. Cut the opener; let the sentence stand.

13. **Opening-word repetition.** The same first word starting a large share of sentences
    (usually "The", "You", "This"). Flag when the top three first-words cover more than a
    quarter of all sentence openings on the page.

14. **Sentence-length flatness.** Sentences within a paragraph all nearly the same length
    (SD < 4 words). Technical prose still breathes: a short declarative after a long
    qualified one.

15. **Announced structure.** A page or section that lists what it is about to cover before
    covering it ("This page covers X, Y and Z", "Below we walk through..."). The sidebar and
    headings already do that. Flag every instance. Exception: a one-line pointer to a
    different page.

16. **Legacy-contrast setup.** Opening a section by positioning Serial Studio against a
    strawman ("Unlike traditional serial monitors...", "Instead of manually parsing...").
    Describe what the feature does, not what it replaces. Flag every instance outside
    comparison pages.

17. **The tidy template.** Taken together: claim first, structure announced, contrast set up,
    confident institutional "we", summary close. One of these alone is normal manual shape.
    Three or more on one page means the page follows the default template, not the
    feature. Rebuild it around what the code actually does.

Tells 15-17 come from SlopShape (Madler, arXiv:2609.15369, 2026). Across five 2026 models it
separated AI blog text from human text at 97 macro-F1 using structural features alone, and the
score dropped only about one point after the text was reworded. That is why tells like these
outlast a vocabulary fix. Human pages were the structural outliers; AI pages clustered on a few
common shapes. "Claim first" is not a tell by itself in a manual, where stating behavior first
is correct.

## Running the pass

- Read the whole page first; the tells are page-level patterns, not line-level phrases.
- Prefer counting to impressions for the threshold-based tells (2, 3, 4, 6, 13, 14); quote
  one representative example per tell rather than every hit.
- When fixing, fix the structure, not the words — rewriting a hedge as a different hedge or
  re-shuffling a symmetric list keeps the tell. State what the code actually does, vary the
  shape, or delete.

## Report format

```
## Second-order pass: <page>

| # | Tell | Hits | Example (file:line) |
|---|------|------|---------------------|
| 1 | Hedge stacking | 3 | "typically may need..." (Drivers-Foo.md:42) |
| ... | ... | ... | ... |

Verdict: PASS / FAIL (fail on any hit for tells 1, 2, 10, 12, 15, 16; thresholds otherwise)
```
