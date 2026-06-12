#!/usr/bin/env python3
"""
Build a BM25 search index over Serial Studio's doc corpus.

Usage:  python3 build_search_index.py
Writes: search_index.json next to this file.

Corpus:
  - Every *.md under ai/skills/   (loaded skills)
  - Every *.md under ai/docs/     (scripting reference)
  - Every *.md under ai/templates/manifest descriptions
  - Every reference script under scripts/{painter,parser,transforms,output}/
    (the leading // header comment is the most useful chunk)
  - Every example .ssproj title + frameParser body in examples/

The index is keyword-y enough that BM25 works well; the corpus is small
enough (~few hundred chunks) that we don't need a vector DB.
"""

import json
import math
import re
import sys
from collections import Counter, defaultdict
from pathlib import Path

# ---- Paths --------------------------------------------------------------

SCRIPT = Path(__file__).resolve()
RCC = SCRIPT.parent.parent  # app/rcc
APP = RCC.parent  # app
ROOT = APP.parent  # repo root

OUT = SCRIPT.parent / "search_index.json"
SKILLS = SCRIPT.parent / "skills"
DOCS_DIR = SCRIPT.parent / "docs"
TEMPLATES = SCRIPT.parent / "templates"
SCRIPTS = RCC / "scripts"
EXAMPLES = ROOT / "examples"

# ---- Tokenization -------------------------------------------------------

STOP = {
    "the",
    "a",
    "an",
    "and",
    "or",
    "of",
    "to",
    "for",
    "in",
    "on",
    "at",
    "is",
    "are",
    "was",
    "were",
    "be",
    "been",
    "being",
    "this",
    "that",
    "these",
    "those",
    "it",
    "its",
    "as",
    "by",
    "with",
    "from",
    "into",
    "you",
    "your",
    "we",
    "our",
    "they",
    "their",
    "i",
    "if",
    "then",
    "do",
    "does",
    "did",
    "doing",
    "have",
    "has",
    "had",
    "but",
    "not",
    "no",
    "yes",
    "so",
    "such",
    "than",
    "when",
    "where",
    "what",
    "which",
    "who",
    "how",
    "why",
    "can",
    "could",
    "would",
    "should",
    "will",
    "may",
    "might",
    "must",
    "shall",
}

TOKEN_RE = re.compile(r"[A-Za-z][A-Za-z0-9_]+")


def tokenize(text: str):
    """Split into lowercased word tokens, drop stopwords + 1-char tokens."""
    out = []
    for m in TOKEN_RE.finditer(text):
        t = m.group(0).lower()
        if len(t) <= 1:
            continue
        if t in STOP:
            continue
        out.append(t)
    return out


# ---- Chunking -----------------------------------------------------------

CHUNK_TARGET = 600  # chars
CHUNK_MAX = 1000


def chunk_markdown(text: str):
    """Split markdown by double-newline paragraphs, then re-pack into chunks
    of roughly CHUNK_TARGET chars. Headings stay attached to the paragraph
    that follows them so retrieval surfaces the section title with the body.
    """
    paras = [p.strip() for p in re.split(r"\n\s*\n", text) if p.strip()]
    chunks = []
    cur = []
    cur_len = 0
    for p in paras:
        if cur_len + len(p) + 2 > CHUNK_MAX and cur:
            chunks.append("\n\n".join(cur))
            cur = []
            cur_len = 0
        cur.append(p)
        cur_len += len(p) + 2
        if cur_len >= CHUNK_TARGET:
            chunks.append("\n\n".join(cur))
            cur = []
            cur_len = 0
    if cur:
        chunks.append("\n\n".join(cur))
    return chunks


def chunk_script(path: Path):
    """For .js/.lua templates, the leading comment block is the most useful
    chunk. Take the first 1000 chars of the file (which should include the
    header comment) plus the first function definition."""
    text = path.read_text(errors="replace")
    return [text[:CHUNK_MAX]]


def first_function_or_decl(text: str, max_lines: int = 30):
    """Return the first ~30 lines so we capture function signatures."""
    lines = text.splitlines()[:max_lines]
    return "\n".join(lines)


# ---- Corpus collection --------------------------------------------------

corpus = []  # list of {id, source, title, body}


def add(source: str, title: str, body: str, doc_id: str):
    body = body.strip()
    if not body:
        return
    corpus.append(
        {
            "id": doc_id,
            "source": source,
            "title": title,
            "body": body,
        }
    )


def harvest_markdown(dir_path: Path, source: str):
    if not dir_path.exists():
        return
    for md in sorted(dir_path.glob("*.md")):
        text = md.read_text(errors="replace")
        title = md.stem
        for i, chunk in enumerate(chunk_markdown(text)):
            add(
                source=source,
                title=f"{title}#{i}",
                body=chunk,
                doc_id=f"{source}/{md.name}#{i}",
            )


def harvest_scripts(dir_path: Path, source: str):
    if not dir_path.exists():
        return
    for js in sorted(list(dir_path.glob("*.js")) + list(dir_path.glob("*.lua"))):
        for i, chunk in enumerate(chunk_script(js)):
            add(
                source=source,
                title=js.stem,
                body=chunk,
                doc_id=f"{source}/{js.name}#{i}",
            )


def harvest_templates_manifest():
    manifest_path = TEMPLATES / "manifest.json"
    if not manifest_path.exists():
        return
    data = json.loads(manifest_path.read_text())
    for entry in data.get("templates", []):
        body = f"Template: {entry.get('title','')}\n\n{entry.get('description','')}"
        add(
            source="template",
            title=entry.get("id", ""),
            body=body,
            doc_id=f"template/{entry.get('id')}",
        )


def harvest_example_readmes():
    if not EXAMPLES.exists():
        return
    for md in sorted(EXAMPLES.rglob("README.md")):
        text = md.read_text(errors="replace")
        title = md.parent.name
        for i, chunk in enumerate(chunk_markdown(text)):
            add(
                source="example",
                title=f"{title} README#{i}",
                body=chunk,
                doc_id=f"example/{md.relative_to(ROOT)}#{i}",
            )


def harvest_examples():
    if not EXAMPLES.exists():
        return
    for proj in sorted(EXAMPLES.rglob("*.ssproj")):
        try:
            data = json.loads(proj.read_text())
        except Exception:
            continue
        title = data.get("title", proj.stem)
        parser = data.get("frameParser", "")
        body_parts = [f"Example project: {title} ({proj.relative_to(ROOT)})"]
        # Group + dataset titles tell the model what shape this project is
        for g in data.get("groups", []):
            gw = g.get("widget", "")
            gtitle = g.get("title", "")
            ds_titles = ", ".join(d.get("title", "") for d in g.get("datasets", []))
            body_parts.append(f"Group '{gtitle}' (widget={gw}): {ds_titles}")
        if parser:
            body_parts.append("Frame parser:\n" + first_function_or_decl(parser))
        add(
            source="example",
            title=title,
            body="\n\n".join(body_parts),
            doc_id=f"example/{proj.relative_to(ROOT)}",
        )
        for i, src in enumerate(data.get("sources", [])):
            src_parser = src.get("frameParserCode", "")
            if not src_parser:
                continue
            add(
                source="example",
                title=f"{title} (source {i})",
                body=f"Example project: {title} ({proj.relative_to(ROOT)}), source {i}\n\n"
                + "Frame parser:\n"
                + first_function_or_decl(src_parser),
                doc_id=f"example/{proj.relative_to(ROOT)}#source{i}",
            )


print("[index] harvesting corpus...", file=sys.stderr)
harvest_markdown(SKILLS, "skill")
harvest_markdown(DOCS_DIR, "doc")
harvest_templates_manifest()
harvest_scripts(SCRIPTS / "painter", "script:painter")
harvest_scripts(SCRIPTS / "parser/js", "script:parser_js")
harvest_scripts(SCRIPTS / "parser/lua", "script:parser_lua")
harvest_scripts(SCRIPTS / "transforms/js", "script:transform_js")
harvest_scripts(SCRIPTS / "transforms/lua", "script:transform_lua")
harvest_scripts(SCRIPTS / "output", "script:output_widget")
harvest_scripts(SCRIPTS / "mqtt/js", "script:mqtt_js")
harvest_scripts(SCRIPTS / "mqtt/lua", "script:mqtt_lua")
harvest_scripts(SCRIPTS / "control", "script:control")
harvest_markdown(SCRIPTS / "native", "doc:native_template")
harvest_examples()
harvest_example_readmes()

print(f"[index] corpus has {len(corpus)} chunks", file=sys.stderr)

# ---- BM25 stats ---------------------------------------------------------

doc_tokens = []  # list of token list per doc
doc_freq = Counter()  # term -> number of docs that contain it
for entry in corpus:
    toks = tokenize(entry["title"]) + tokenize(entry["body"])
    doc_tokens.append(toks)
    for t in set(toks):
        doc_freq[t] += 1

N = len(corpus)
avgdl = sum(len(t) for t in doc_tokens) / max(1, N)

# IDF using BM25 form
idf = {}
for term, df in doc_freq.items():
    idf[term] = math.log(1 + (N - df + 0.5) / (df + 0.5))

# Per-doc term frequency map (compact keys)
docs_out = []
for entry, toks in zip(corpus, doc_tokens):
    tf = Counter(toks)
    # Keep only top-50 terms by frequency to keep index size sane.
    most = dict(tf.most_common(50))
    docs_out.append(
        {
            "id": entry["id"],
            "source": entry["source"],
            "title": entry["title"],
            "body": entry["body"],
            "len": len(toks),
            "tf": most,
        }
    )

# ---- Write --------------------------------------------------------------

index = {
    "version": 1,
    "k1": 1.5,
    "b": 0.75,
    "N": N,
    "avgdl": avgdl,
    "idf": idf,  # term -> idf weight
    "docs": docs_out,
}

OUT.parent.mkdir(parents=True, exist_ok=True)
OUT.write_bytes(json.dumps(index, separators=(",", ":")).encode("utf-8"))
print(f"[index] wrote {OUT} ({OUT.stat().st_size // 1024} KiB)", file=sys.stderr)
