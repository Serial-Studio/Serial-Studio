#!/usr/bin/env python3
"""Serial Studio documentation linter for Markdown.

Targets the prose under `doc/help/`, the top-level `README.md` and
`AGENTS.md`, and the per-example `README.md` files under `examples/`.

The rules below catch the AI-narration / marketing-copy patterns that
keep leaking into the docs. Documentation rewrites are judgement calls,
so this script never fixes anything in place -- it flags every hit and
writes a `.doc-report` at the repo root for a follow-up human or LLM
pass.

Usage:
    python3 scripts/documentation-verify.py                    # scan defaults
    python3 scripts/documentation-verify.py doc/help           # explicit dir
    python3 scripts/documentation-verify.py doc/help/Foo.md    # single file
    python3 scripts/documentation-verify.py --no-report ...    # don't write .doc-report

Exit codes:
    0 - clean
    1 - findings present (advisory only; CI choice whether to gate on this)
    2 - argument error

Skipped automatically:
    - Fenced code blocks (``` ... ```)
    - Indented code blocks (lines prefixed with 4+ spaces inside lists)
    - HTML comments (<!-- ... -->)
    - Inline code spans (`foo`)
    - Link URLs and image paths
    - YAML / TOML front matter at the top of the file

Findings are grouped by `kind`:
    ai-marketing-phrase    -- "escape hatch", "magic", "seamless", ...
    ai-tutorial-voice      -- "we'll", "let's", "buckle up", ...
    ai-conversational      -- "If you've ever", "you already know", ...
    ai-editorializing      -- "powerful", "elegant", "polished", ...
    ai-filler              -- "essentially", "just", "simply", "really", ...
    ai-superlative         -- "blazing fast", "lightning", "world-class", ...
    ai-meta-reference      -- "in this guide", "this section will", ...
    style-shouting         -- trailing "!" in body prose
    style-non-ascii        -- curly quotes, em-dash, en-dash, arrows
    style-emdash-density   -- > 8 em-dashes per file (rhythm tic)
    style-hr-separator     -- `---` / `***` / `___` horizontal rule used as section divider

Wrap a region with `<!-- doc-verify off -->` / `<!-- doc-verify on -->`
to disable scanning between the markers.
"""

from __future__ import annotations

import argparse
import os
import re
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable

# ---------------------------------------------------------------------------
# Rules
# ---------------------------------------------------------------------------

# Each rule is (kind, compiled-regex, message-template). The regex is
# matched against the *prose* version of each line (after code spans,
# URLs, and inline code have been masked). Matching is case-insensitive
# unless the pattern uses an explicit `(?-i:...)` group.


def _ci(pattern: str) -> re.Pattern:
    return re.compile(pattern, re.IGNORECASE)


# Marketing / blog cliches. These are the ones that scream "blog post"
# instead of "manual". A single hit per line is reported (not per phrase).
_MARKETING_PHRASES: list[tuple[re.Pattern, str]] = [
    (_ci(r"\bescape hatch\b"), "metaphor; describe the actual capability"),
    (_ci(r"\bin one breath\b"), "blog-style heading; rename to a topic"),
    # Match "magic" as marketing voice, but skip technical compound terms
    # like "magic bytes", "magic numbers", "magic value/word/string/cookie".
    (
        _ci(
            r"\b(?:the\s+)?(?:real\s+)?magic\b(?!\s+(?:byte|number|value|word|string|cookie|method|constant)s?\b)"
        ),
        "marketing word; describe the mechanism",
    ),
    (_ci(r"\bseamless(?:ly)?\b"), "marketing adjective; remove or restate"),
    (_ci(r"\beffortless(?:ly)?\b"), "marketing adjective; remove or restate"),
    (_ci(r"\bworld[-\s]class\b"), "marketing superlative; remove"),
    (_ci(r"\bbest[-\s]in[-\s]class\b"), "marketing superlative; remove"),
    (_ci(r"\bcutting[-\s]edge\b"), "marketing adjective; remove or restate"),
    (
        _ci(r"\bstate[-\s]of[-\s]the[-\s]art\b"),
        "marketing adjective; remove or restate",
    ),
    (_ci(r"\bindustry[-\s]leading\b"), "marketing adjective; remove"),
    (_ci(r"\bgame[-\s]chang(?:er|ing)\b"), "marketing cliche; remove or restate"),
    (_ci(r"\bfuture[-\s]proof(?:ed)?\b"), "marketing adjective; remove"),
    (_ci(r"\bturnkey\b"), "marketing adjective; remove"),
    (_ci(r"\bplug[-\s]and[-\s]play\b"), "marketing cliche; restate"),
    (_ci(r"\bbattle[-\s]tested\b"), "marketing cliche; restate"),
    (_ci(r"\bproduction[-\s]ready\b"), "marketing adjective; restate"),
    (_ci(r"\bhandcrafted\b"), "marketing adjective; remove"),
    (
        _ci(r"\b(?:beautifully|elegantly|cleanly)\s+(?:designed|crafted|built|done)\b"),
        "marketing phrasing; restate",
    ),
    (
        _ci(r"\bunder the hood\b"),
        "blog metaphor; replace with 'internally' or describe the mechanism",
    ),
    (
        _ci(r"\bbehind the scenes\b"),
        "blog metaphor; replace with 'internally' or describe the mechanism",
    ),
    (
        _ci(r"\bout of the box\b"),
        "marketing cliche; restate (e.g. 'by default', 'with no setup')",
    ),
    (
        _ci(r"\bship[s]?\s+with\b"),
        "informal; consider 'is bundled with' or 'is included with'",
    ),
    (_ci(r"\bgives you\b"), "blog phrasing; consider 'provides' or restate"),
    (_ci(r"\bhands you\b"), "blog phrasing; consider 'provides' or restate"),
    (_ci(r"\blands on\b"), "blog phrasing; describe the actual write/render path"),
    (
        _ci(r"\bthe trick is\b"),
        "tutorial-blog phrasing; describe the technique directly",
    ),
    (_ci(r"\b(?:the\s+)?secret sauce\b"), "marketing cliche; remove"),
    (_ci(r"\bpoor[-\s]man'?s\b"), "informal phrase; restate as a neutral description"),
    (
        _ci(r"\b(?:looks|reads)\s+(?:great|amazing|gorgeous|stunning)\b"),
        "subjective claim; remove or describe what changed",
    ),
    (
        _ci(r"\b(?:looks|reads)\s+(?:as|like)\s+\""),
        "marketing simile; describe directly",
    ),
    (_ci(r"\bscreenshots?\s+well\b"), "marketing claim; remove"),
    (_ci(r"\bdesigned to\b"), "marketing voice; describe what it actually does"),
    (_ci(r"\bbuilt for\b"), "marketing voice; describe what it actually does"),
    (
        _ci(r"\bdeliberate(?:ly)?\b"),
        "editorializing; describe the actual design constraint",
    ),
    (_ci(r"\bpolished\b"), "subjective; remove or restate"),
    (_ci(r"\bprofessional[- ]grade\b"), "marketing claim; remove"),
    (_ci(r"\bblazing\s+fast\b"), "marketing superlative; cite an actual figure"),
    (_ci(r"\blightning[-\s]fast\b"), "marketing superlative; cite an actual figure"),
    (_ci(r"\bbuttery\s+smooth\b"), "marketing cliche; remove"),
    (
        _ci(r"\b(?:fully|completely)\s+(?:integrated|customizable)\b"),
        "marketing intensifier; drop the adverb",
    ),
    (_ci(r"\bunleash\b"), "marketing verb; restate"),
    (_ci(r"\bsupercharge\b"), "marketing verb; restate"),
    (_ci(r"\bempower(?:s|ed|ing)?\b"), "marketing verb; restate"),
    (
        _ci(r"\b(?:rich|robust|comprehensive)\s+(?:set|suite|api|features?)\b"),
        "marketing adjective; describe what is included",
    ),
    (_ci(r"\bship\s+it\b"), "informal; remove"),
]

# Tutorial / chatty / conversational tone. Reasonable in a blog post,
# wrong in a manual.
_TUTORIAL_VOICE: list[tuple[re.Pattern, str]] = [
    (_ci(r"\bwe'(?:ll|re|ve)\b"), "tutorial voice; rewrite without 'we'"),
    (
        _ci(
            r"\b(?:we|let'?s)\s+(?:take\s+a\s+look|dive\s+in|get\s+started|walk\s+through)\b"
        ),
        "tutorial voice; describe directly",
    ),
    (_ci(r"\b(?:buckle|strap|hold)\s+(?:up|on)\b"), "tutorial voice; remove"),
    (
        _ci(r"\bin\s+this\s+(?:guide|article|post|tutorial|section\s+we)\b"),
        "blog meta-reference; remove or restate",
    ),
    (_ci(r"\bas\s+(?:we|you)\s+(?:can\s+)?see\b"), "tutorial voice; remove"),
    (
        _ci(r"\bas\s+(?:we|you)'(?:ll|ve)\s+(?:see|noticed)\b"),
        "tutorial voice; remove or describe directly",
    ),
    (
        _ci(r"\bnow\s+(?:we|let'?s|it'?s\s+time)\b"),
        "tutorial voice; describe the next step directly",
    ),
    (_ci(r"\bfirst\s+we\b"), "tutorial voice; use imperative or numbered step"),
    (_ci(r"\bnext\s+up\b"), "tutorial filler; remove or use a heading"),
    (
        _ci(r"\bfor\s+the\s+rest\s+of\s+(?:this|the)\s+(?:guide|article|section)\b"),
        "blog meta-reference; remove",
    ),
    (_ci(r"\bthat'?s\s+(?:it|the\s+whole\s+)"), "informal; remove"),
    (_ci(r"\bthat'?s\s+the\s+whole\s+widget\b"), "informal; describe the result"),
    (_ci(r"\bthe\s+rest\s+is\s+just\b"), "informal; describe the rest"),
    (
        _ci(r"\byou'?ll\s+(?:want|need|love)\b"),
        "tutorial voice; use imperative or describe",
    ),
    (_ci(r"\byou\s+already\s+know\b"), "blog phrasing; remove"),
    (
        _ci(r"\bif\s+you'?ve\s+(?:ever|drawn|used|written)\b"),
        "blog phrasing; describe directly",
    ),
    (_ci(r"\btrust\s+(?:us|me)\b"), "informal; remove"),
    (_ci(r"\btake\s+(?:our|my)\s+word\b"), "informal; remove"),
]

# Conversational asides and rhetorical hedging.
_CONVERSATIONAL: list[tuple[re.Pattern, str]] = [
    (_ci(r"\bobviously\b"), "patronising hedge; remove"),
    (_ci(r"\bof\s+course\b"), "patronising hedge; remove"),
    (_ci(r"\bclearly\b"), "patronising hedge; remove"),
    (_ci(r"\bnaturally\b"), "patronising hedge; remove"),
    (_ci(r"\bneedless\s+to\s+say\b"), "filler; remove"),
    (_ci(r"\bit\s+goes\s+without\s+saying\b"), "filler; remove"),
    (_ci(r"\bat\s+the\s+end\s+of\s+the\s+day\b"), "blog filler; remove"),
    (
        _ci(r"\b(?:long\s+story\s+short|tldr|tl;dr)\b"),
        "blog filler; rewrite into a heading",
    ),
    (_ci(r"\bthe\s+thing\s+is\b"), "blog filler; remove"),
    (
        _ci(r"\bhere'?s\s+the\s+(?:thing|deal|kicker|catch)\b"),
        "blog filler; describe directly",
    ),
    (_ci(r"\bspoiler\s*(?:alert)?\b"), "blog filler; remove"),
    (_ci(r"\bhot\s+take\b"), "blog filler; remove"),
    (_ci(r"\bin\s+a\s+nutshell\b"), "blog filler; replace with 'in summary' or remove"),
    (_ci(r"\b(?:like\s+)?we\s+said\b"), "blog reference; remove"),
    (_ci(r"\bas\s+(?:we|i)\s+mentioned\b"), "blog reference; remove"),
]

# Editorializing adjectives. These are subjective claims masquerading as
# facts. Most of them belong in a marketing page, not a manual.
_EDITORIALIZING: list[tuple[re.Pattern, str]] = [
    (_ci(r"\bpowerful\b"), "subjective; describe what it does"),
    (
        _ci(r"\b(?:incredibly|amazingly|surprisingly)\s+\w+"),
        "subjective intensifier; remove",
    ),
    (_ci(r"\b(?:beautiful|elegant|gorgeous)\b"), "subjective; remove or describe"),
    (_ci(r"\bdelightful\b"), "subjective; remove"),
    (_ci(r"\bintuitive(?:ly)?\b"), "subjective; remove or describe the affordance"),
    (
        _ci(r"\beasy\s+to\s+(?:use|understand|learn)\b"),
        "subjective; describe what makes it so",
    ),
    (_ci(r"\buser[-\s]friendly\b"), "subjective; describe the affordance"),
    (
        _ci(r"\bsimple\s+(?:and\s+)?(?:clean|elegant|powerful)\b"),
        "subjective stack; remove or restate",
    ),
    (_ci(r"\b(?:a|the)\s+breeze\b"), "informal; remove"),
    (
        _ci(r"\b(?:works|just\s+works)\s+(?:like\s+a\s+charm|magic|beautifully)\b"),
        "marketing phrasing; remove",
    ),
    (_ci(r"\bjust\s+works\b"), "marketing phrasing; describe the behaviour"),
    (_ci(r"\bbest\s+practice[s]?\b"), "vague; cite the specific rule"),
    (_ci(r"\b(?:the\s+)?right\s+thing\b"), "vague; cite the actual choice"),
]

# Empty filler words. Manuals are tighter without them.
_FILLER: list[tuple[re.Pattern, str]] = [
    (_ci(r"\bessentially\b"), "filler; remove or rewrite"),
    (_ci(r"\bbasically\b"), "filler; remove or rewrite"),
    (_ci(r"\bsimply\b"), "filler; remove"),
    (
        _ci(r"\bjust\s+(?:add|use|set|click|run|call|write|do|make)\b"),
        "minimising 'just'; remove",
    ),
    (
        _ci(r"\bactually\b"),
        "filler; remove unless contrasting with a stated misconception",
    ),
    (_ci(r"\breally\b"), "filler intensifier; remove"),
    (
        _ci(r"\bvery\s+(?:fast|easy|simple|good|nice|useful|important)\b"),
        "filler intensifier; describe directly",
    ),
    (_ci(r"\bquite\b"), "filler hedge; remove"),
    (
        _ci(r"\bpretty\s+(?:much|simple|easy|fast|good|straightforward)\b"),
        "filler hedge; remove or quantify",
    ),
    (_ci(r"\bkind\s+of\b"), "filler hedge; remove"),
    (_ci(r"\bsort\s+of\b"), "filler hedge; remove"),
    (_ci(r"\bin\s+order\s+to\b"), "verbose; replace with 'to'"),
    (_ci(r"\bmake\s+sure\s+to\b"), "verbose; replace with imperative"),
    (
        _ci(r"\ba\s+(?:lot|bunch|ton)\s+of\b"),
        "informal quantifier; cite a number or use 'many'",
    ),
    (_ci(r"\btons\s+of\b"), "informal quantifier; cite a number or use 'many'"),
    (_ci(r"\bawesome\b"), "filler adjective; remove"),
    (_ci(r"\bnice\b"), "filler adjective; remove or describe"),
]

# Meta-references. The doc shouldn't talk about itself.
_META: list[tuple[re.Pattern, str]] = [
    (
        _ci(
            r"\bthis\s+(?:section|chapter|guide|document|article)\s+(?:will|covers|explains|describes|walks)\b"
        ),
        "self-reference; describe the topic directly",
    ),
    (
        _ci(r"\bin\s+the\s+(?:next|previous)\s+(?:section|chapter|paragraph)\b"),
        "rot reference; link to the section by name",
    ),
    (_ci(r"\bsee\s+below\b"), "rot reference; link to the section"),
    (_ci(r"\bsee\s+above\b"), "rot reference; link to the section"),
    (
        _ci(r"\bas\s+(?:we|you)\s+saw\s+(?:above|earlier)\b"),
        "rot reference; remove or link",
    ),
]


# Combined rule table (kind, list of (pattern, message)).
_RULES: list[tuple[str, list[tuple[re.Pattern, str]]]] = [
    ("ai-marketing-phrase", _MARKETING_PHRASES),
    ("ai-tutorial-voice", _TUTORIAL_VOICE),
    ("ai-conversational", _CONVERSATIONAL),
    ("ai-editorializing", _EDITORIALIZING),
    ("ai-filler", _FILLER),
    ("ai-meta-reference", _META),
]


# ---------------------------------------------------------------------------
# Style rules (independent of phrase scanning)
# ---------------------------------------------------------------------------

# Trailing exclamation in body prose (not in headings -- headings are
# allowed a single closing punctuation that is rarely "!").
_SHOUTING = re.compile(r"(?<![A-Z0-9!])!(?:\s|$|[)\]])")

# Horizontal-rule separator on its own line: three or more dashes,
# asterisks, or underscores. CommonMark also allows internal spaces
# (e.g. `- - -`); we match those too. Front matter is already stripped
# upstream, so any HR seen here is a body-prose separator.
_HR_SEPARATOR = re.compile(r"^\s{0,3}(?:(?:-\s*){3,}|(?:\*\s*){3,}|(?:_\s*){3,})\s*$")

# Non-ASCII typography that should be ASCII in source: em dash, en dash,
# horizontal ellipsis, smart quotes, arrows, micro / degree / multiply
# / division glyphs, non-breaking space.
_NON_ASCII_GLYPHS = {
    "—": "em-dash (use --)",
    "–": "en-dash (use - or numeric range)",
    "…": "ellipsis (use ...)",
    "“": 'left smart quote (use ")',
    "”": 'right smart quote (use ")',
    "‘": "left smart quote (use ')",
    "’": "right smart quote (use ')",
    "→": "right arrow (use -> or 'to')",
    "←": "left arrow (use <- or 'from')",
    "×": "multiplication sign (use x or *)",
    "÷": "division sign (use /)",
    "µ": "micro sign (use 'micro' or 'u')",
    "°": "degree sign (use ' deg' or 'degrees')",
    " ": "non-breaking space (use a normal space)",
    " ": "narrow no-break space (use a normal space)",
}

# How many em-dashes in a single file before we flag the density itself.
# A few are fine for parentheticals; ten or more is a rhythm tic.
_EMDASH_DENSITY_THRESHOLD = 8


# ---------------------------------------------------------------------------
# Markdown masking
# ---------------------------------------------------------------------------

# Replace inline-code spans, link URLs, and image targets with spaces of
# the same length so column offsets in the original line are preserved.
_INLINE_CODE_RE = re.compile(r"`+[^`\n]*?`+")
_AUTOLINK_RE = re.compile(r"<[a-zA-Z][^>\s]*://[^>\s]*>")
_LINK_URL_RE = re.compile(r"\]\([^)\n]*\)")
_LINK_REF_RE = re.compile(r"\]\[[^\]\n]*\]")
_IMG_URL_RE = re.compile(r"!\[[^\]\n]*\]\([^)\n]*\)")
_HTML_TAG_RE = re.compile(r"</?[A-Za-z][^>\n]*>")
# Top-of-file YAML or TOML front matter.
_FRONT_MATTER = re.compile(r"\A(?:---|\+\+\+).*?\n(?:---|\+\+\+)\n", re.DOTALL)


_TOC_LINE_RE = re.compile(
    r"""^\s*
        (?:[-*+]|\d+\.)        # bullet or numbered list marker
        \s+
        (?:\[[^\]]+\]\([^)]+\)\s*)+    # one or more inline links, possibly separated by spaces
        \.?                    # optional trailing period
        \s*$""",
    re.VERBOSE,
)

_BOLD_HEADING_RE = re.compile(r"^\s*\*\*[^*]+(?::|\.|\?|!)?\*\*\s*:?\s*$")


def _is_toc_line(line: str) -> bool:
    return bool(_TOC_LINE_RE.match(line))


def _is_bold_heading(line: str) -> bool:
    return bool(_BOLD_HEADING_RE.match(line))


def _mask(line: str) -> str:
    """Return `line` with inline code spans, URLs, image sources, HTML
    tags, and link references masked to spaces. Preserves length so the
    column offsets returned by re.search() still index into the original
    line."""

    def blank(m: re.Match) -> str:
        return " " * (m.end() - m.start())

    masked = line
    for pat in (
        _INLINE_CODE_RE,
        _IMG_URL_RE,
        _LINK_URL_RE,
        _LINK_REF_RE,
        _AUTOLINK_RE,
        _HTML_TAG_RE,
    ):
        masked = pat.sub(blank, masked)
    return masked


# ---------------------------------------------------------------------------
# File walker
# ---------------------------------------------------------------------------


@dataclass
class Finding:
    path: Path
    line: int
    col: int
    kind: str
    message: str
    excerpt: str


_FENCE_RE = re.compile(r"^(\s*)(```+|~~~+)")


def _strip_code_blocks(text: str) -> list[tuple[int, str]]:
    """Return list of (1-based line number, original line) for prose
    lines only -- fenced code blocks and top-of-file front matter are
    dropped. HTML comments are PRESERVED so the disable-fence markers
    `<!-- doc-verify off -->` / `<!-- doc-verify on -->` can be seen by
    the next stage; the prose-rule loop separately ignores HTML-comment
    lines that aren't disable markers."""
    text = _FRONT_MATTER.sub("", text)

    out: list[tuple[int, str]] = []
    in_fence = False
    fence_marker = ""

    for idx, line in enumerate(text.splitlines(), start=1):
        # Fenced code blocks.
        m = _FENCE_RE.match(line)
        if m:
            marker = m.group(2)
            if not in_fence:
                in_fence = True
                fence_marker = marker[:3]  # match opening backtick or tilde count
                continue
            if marker.startswith(fence_marker):
                in_fence = False
                continue

        if in_fence:
            continue

        out.append((idx, line))
    return out


def _check_disable_fences(prose_lines: list[tuple[int, str]]) -> list[tuple[int, str]]:
    """Filter out lines between `<!-- doc-verify off -->` and
    `<!-- doc-verify on -->` markers."""
    enabled = True
    out: list[tuple[int, str]] = []
    for lineno, line in prose_lines:
        # Toggle markers
        if "<!-- doc-verify off -->" in line:
            enabled = False
            continue
        if "<!-- doc-verify on -->" in line:
            enabled = True
            continue
        if enabled:
            out.append((lineno, line))
    return out


def scan_file(path: Path) -> list[Finding]:
    """Scan `path` and return a list of findings."""
    try:
        text = path.read_text(encoding="utf-8")
    except (OSError, UnicodeDecodeError) as exc:
        return [Finding(path, 0, 0, "io-error", f"could not read: {exc}", "")]

    findings: list[Finding] = []
    prose = _check_disable_fences(_strip_code_blocks(text))

    # Per-line phrase rules.
    in_html_comment = False
    prev_raw = ""
    for lineno, raw in prose:
        masked = _mask(raw)
        body = masked.lstrip()

        # Skip HTML comment regions. `_check_disable_fences` already
        # consumed `<!-- doc-verify off/on -->` markers; everything
        # else inside `<!-- ... -->` is non-prose metadata.
        if in_html_comment:
            if "-->" in raw:
                in_html_comment = False
            prev_raw = raw
            continue
        if body.startswith("<!--"):
            if "-->" not in raw:
                in_html_comment = True
            prev_raw = raw
            continue

        # Style: horizontal-rule separator used as a section divider.
        # Only flag when the previous prose line is blank, to avoid
        # mis-flagging a setext h2 underline (`Heading\n---`).
        if _HR_SEPARATOR.match(raw) and prev_raw.strip() == "":
            findings.append(
                Finding(
                    path=path,
                    line=lineno,
                    col=1,
                    kind="style-hr-separator",
                    message=(
                        "horizontal-rule separator; section headings "
                        "already provide structure -- remove the rule"
                    ),
                    excerpt=raw.strip()[:140],
                )
            )
            prev_raw = raw
            continue

        # Skip headings -- the marketing tone is more tolerable in a
        # heading and rules trigger too often there. Body prose is what
        # this script cares about.
        if body.startswith("#"):
            prev_raw = raw
            continue

        # Skip table-of-contents lines: a list bullet whose only
        # content is one or more links. A real TOC entry is structural
        # and the anchor text often duplicates the section heading we
        # already chose to skip.
        if _is_toc_line(raw):
            prev_raw = raw
            continue

        # Skip bold-as-heading lines: a line that is entirely wrapped
        # in `**...**` (with optional trailing colon) is acting as a
        # heading. Body prose containing inline `**bold**` is unaffected.
        if _is_bold_heading(raw):
            prev_raw = raw
            continue

        for kind, rules in _RULES:
            for pat, msg in rules:
                m = pat.search(masked)
                if m:
                    findings.append(
                        Finding(
                            path=path,
                            line=lineno,
                            col=m.start() + 1,
                            kind=kind,
                            message=f'"{m.group(0)}": {msg}',
                            excerpt=raw.strip()[:140],
                        )
                    )
                    # One hit per (line, kind) keeps reports readable.
                    break

        # Style: trailing exclamation in body prose. Skip code fences,
        # already handled. Allow "!important" in CSS-ish contexts and
        # avoid flagging `!=` or `!==`. Also allow "!" inside parens that
        # look like aside punctuation: `(yes!)` is rare but tolerated.
        for m in _SHOUTING.finditer(masked):
            # Reject obvious false positives: !=, !==, !$, !-style URLs.
            ch_before = masked[m.start() - 1] if m.start() > 0 else ""
            ch_after = masked[m.start() + 1] if m.start() + 1 < len(masked) else ""
            if ch_after in ("=", "!", "$"):
                continue
            findings.append(
                Finding(
                    path=path,
                    line=lineno,
                    col=m.start() + 1,
                    kind="style-shouting",
                    message="trailing '!' in body prose; manuals don't shout",
                    excerpt=raw.strip()[:140],
                )
            )
            break  # one per line

        # Style: non-ASCII typography. Scan the *unmasked* line so
        # glyphs inside `inline code` are skipped (those are wiped to
        # spaces by _mask). But we want to skip code/URL spans, so
        # iterate the masked-but-restored characters.
        for col, ch in enumerate(raw):
            if masked[col] == " " and ch != " ":
                continue  # masked-out code or URL
            if ch in _NON_ASCII_GLYPHS:
                findings.append(
                    Finding(
                        path=path,
                        line=lineno,
                        col=col + 1,
                        kind="style-non-ascii",
                        message=_NON_ASCII_GLYPHS[ch],
                        excerpt=raw.strip()[:140],
                    )
                )
                break  # one per line

        prev_raw = raw

    # Whole-file rules.

    # Em-dash density. Count actual em dash characters (U+2014) AND the
    # ASCII spaced double-hyphen ` -- ` we use as the substitute, since
    # the failure mode is "every other paragraph has a parenthetical
    # rhythm tic" regardless of the glyph used.
    em_count = 0
    for _, raw in prose:
        masked = _mask(raw)
        em_count += masked.count("—")
        em_count += len(re.findall(r"(?<=\S)\s--\s(?=\S)", masked))
    if em_count > _EMDASH_DENSITY_THRESHOLD:
        findings.append(
            Finding(
                path=path,
                line=1,
                col=1,
                kind="style-emdash-density",
                message=(
                    f"{em_count} em-dash / spaced-double-hyphen occurrences; "
                    "many of these are likely rhythm tics. Replace "
                    "parentheticals with parentheses, periods, or "
                    "restructure the sentence."
                ),
                excerpt="",
            )
        )

    return findings


# ---------------------------------------------------------------------------
# Targets
# ---------------------------------------------------------------------------


def default_targets(repo_root: Path) -> list[Path]:
    """The default scan set: doc/help, README.md, AGENTS.md, and every
    Markdown file under examples/."""
    out: list[Path] = []
    help_dir = repo_root / "doc" / "help"
    if help_dir.is_dir():
        out.append(help_dir)
    for top in ("README.md", "AGENTS.md"):
        p = repo_root / top
        if p.is_file():
            out.append(p)
    examples = repo_root / "examples"
    if examples.is_dir():
        out.append(examples)
    return out


def iter_markdown_files(targets: list[Path]) -> Iterable[Path]:
    """Yield every `.md` file under `targets`, deduplicated and sorted."""
    seen: set[Path] = set()
    for t in targets:
        if t.is_file():
            if t.suffix.lower() == ".md":
                seen.add(t.resolve())
        elif t.is_dir():
            for root, _, files in os.walk(t):
                # Skip vendored / build trees.
                parts = Path(root).parts
                if any(
                    p in {"node_modules", "build", ".git", "venv", "__pycache__"}
                    for p in parts
                ):
                    continue
                for name in files:
                    if name.lower().endswith(".md"):
                        seen.add(Path(root, name).resolve())
    return sorted(seen)


# ---------------------------------------------------------------------------
# Report
# ---------------------------------------------------------------------------

_REPORT_HEADER = """\
# Documentation Quality Report

Generated by `scripts/documentation-verify.py`. Each entry below was
flagged by a heuristic that often signals AI-narration or marketing-copy
phrasing in a technical manual. The script does not auto-fix anything;
documentation rewrites are judgement calls and belong to a human or an
LLM that has read the surrounding paragraph.

## Why these rules exist (read this first)

Readers reach for the manual when they're stuck. They came here for the
answer, not for our narrative voice. The patterns these rules catch are
the ones that most reliably distinguish a sales page or a tutorial from
a reference manual:

- Marketing voice ("powerful", "seamless", "world-class", "blazing
  fast") tells the reader they're in the wrong kind of document. A
  manual that sounds like a product page burns its own credibility.
- Tutorial voice ("we'll", "let's", "now we'll see") forces the reader
  into the writer's narrative when they wanted a direct answer. The
  reader isn't on a journey with us; they're trying to ship something.
- Conversational asides ("obviously", "of course", "needless to say")
  signal that the writer is more invested in sounding clever than in
  being useful, and patronise readers who didn't already know.
- Filler ("essentially", "basically", "simply", "just") promises
  shortness it doesn't deliver. Manuals are tighter without it.
- Meta-references ("this section will", "see above") rot the moment
  the doc is reorganised; link to the section by name instead.

A finding is a *place to look*, not a change to apply blindly. The
matched phrase is sometimes correct in context — a quoted sentence
might legitimately say "powerful" if the surrounding text is reporting
someone else's claim. Read the paragraph; decide; rewrite only when
the rewrite is genuinely shorter or clearer.

## How to read this report

For each finding the report shows the file, line, column, the matched
phrase, and a one-line note suggesting the fix. The matched phrase is
not always wrong in context (a quoted sentence might legitimately use
"powerful" if the surrounding text is reporting someone else's claim);
the report is a list of *places to look*, not a list of changes to
apply blindly.

## Rules

These rules encode the writing principles we want the docs to follow:

- **No marketing copy.** Words like "magic", "seamless", "world-class",
  "blazing fast", "escape hatch", "polished" are blog or sales voice,
  not manual voice. Either remove the word or describe what the system
  actually does.
- **No tutorial voice.** "We'll", "let's", "buckle up", "as you can
  see", "now we'll" -- a manual addresses the reader directly with
  imperatives or describes the system in the third person, not in a
  collaborative tour-guide register.
- **No conversational asides.** "Obviously", "of course", "needless to
  say", "the thing is", "trust me" -- patronising or filler. Drop them.
- **No editorializing.** "Powerful", "elegant", "intuitive",
  "user-friendly" are subjective claims. Describe the affordance
  instead: "exposes a JavaScript callback", "the dialog has a search
  field", etc.
- **No filler.** "Essentially", "basically", "simply", "just",
  "actually", "really", "very", "quite", "kind of", "in order to" --
  every one of these can be deleted or replaced with a stronger word
  without losing meaning.
- **No meta-references.** "This section will", "in the next chapter",
  "see above", "as we mentioned earlier" -- rot the moment the doc is
  reorganized. Link to the section by name.
- **No shouting.** Trailing `!` in body prose. Manuals are declarative.
- **ASCII typography.** Em dashes, en dashes, smart quotes, arrows,
  micro / degree glyphs all break older toolchains and read as escape
  goo in legacy editors. Use `--`, `-`, `"`, `->`, `degrees`, etc. The
  same rule the C++/QML linter applies in source.
- **Em-dash density.** A few em dashes (or `--` substitutes) are fine
  for parentheticals. Past 8 in a single file the dashes start to read
  as a writing tic; replace some with parentheses, commas, or split
  sentences.
- **No horizontal-rule separators.** `---`, `***`, `___` on their own
  line are visual noise; section headings already provide structure.
  Setext-style heading underlines (a `---` directly under a heading
  line) are not flagged.

## Skipped

- Fenced code blocks (`````` ... ``````)
- Inline code spans (`` `foo` ``)
- HTML comments (`<!-- ... -->`)
- Link URLs and image paths
- YAML / TOML front matter
- Headings (`#` lines) -- the rules are tuned for body prose
- Regions wrapped in `<!-- doc-verify off -->` / `<!-- doc-verify on -->`

## Findings
"""


def write_report(report_path: Path, findings: list[Finding]) -> None:
    """Group findings by kind, then by file, and write `.doc-report`.
    Removes any stale report when there are no findings."""
    if not findings:
        if report_path.exists():
            report_path.unlink()
        return

    by_kind: dict[str, list[Finding]] = {}
    for f in findings:
        by_kind.setdefault(f.kind, []).append(f)

    out: list[str] = [_REPORT_HEADER]
    out.append(f"\n_Total findings: {len(findings)}_\n")

    for kind in sorted(by_kind):
        entries = by_kind[kind]
        out.append(f"\n### `{kind}` ({len(entries)})\n\n")
        # Group by file within each kind for readability.
        by_file: dict[Path, list[Finding]] = {}
        for f in entries:
            by_file.setdefault(f.path, []).append(f)
        for fpath in sorted(by_file):
            out.append(f"**`{fpath}`**\n\n")
            for f in by_file[fpath]:
                excerpt = f.excerpt.replace("`", "'")
                out.append(f"- L{f.line}:{f.col} -- {f.message}\n")
                if excerpt:
                    out.append(f"  > {excerpt}\n")
            out.append("\n")

    out.append("\n---\n")
    report_path.write_text("".join(out), encoding="utf-8", newline="\n")


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(
        description="Markdown linter for AI-narration / marketing-copy patterns",
        epilog="With no arguments, scans doc/help, README.md, AGENTS.md, examples/.",
    )
    parser.add_argument(
        "paths",
        nargs="*",
        type=Path,
        help="files or directories to scan (default: repo trees)",
    )
    parser.add_argument(
        "--no-report",
        action="store_true",
        help="skip writing .doc-report at the repo root",
    )
    parser.add_argument(
        "--quiet",
        "-q",
        action="store_true",
        help="don't print per-finding lines (rely on .doc-report)",
    )

    args = parser.parse_args(argv)

    repo_root = Path(__file__).resolve().parent.parent
    if not args.paths:
        targets = default_targets(repo_root)
        if not targets:
            print("no default targets exist; pass paths explicitly", file=sys.stderr)
            return 2
        args.paths = targets

    files = list(iter_markdown_files(args.paths))
    if not files:
        print("no markdown files found", file=sys.stderr)
        return 2

    findings: list[Finding] = []
    for path in files:
        findings.extend(scan_file(path))

    if not args.quiet:
        for f in findings:
            print(f"{f.path}:{f.line}:{f.col}: {f.kind}: {f.message}")

    if not args.no_report:
        write_report(repo_root / ".doc-report", findings)

    by_kind: dict[str, int] = {}
    for f in findings:
        by_kind[f.kind] = by_kind.get(f.kind, 0) + 1

    print(f"\n{len(files)} files scanned, {len(findings)} findings", file=sys.stderr)
    if by_kind:
        for kind in sorted(by_kind):
            print(f"  {kind}: {by_kind[kind]}", file=sys.stderr)

    return 1 if findings else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
