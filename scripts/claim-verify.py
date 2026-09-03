#!/usr/bin/env python3
"""Serial Studio AI-facing documentation claim checker.

The repo's AI-facing prose (`CLAUDE.md`, `doc/claude/**`, `.claude/skills/**`)
is where the system's understanding lives: ~104k tokens of it steer every edit
an assistant makes. It is also the tier that rots silently. `code-verify.py`,
`ctest` and `--benchmark-hotpath` are executable memory -- they run against
real code and go red. Prose memory has nothing but a periodic read.

This script moves the mechanically-checkable part of that prose into the
executable tier. It does not judge writing; it checks the claims that have a
ground truth in the tree:

    link-target-missing  a Markdown link to a repo path that does not resolve
    path-missing         a backticked repo path (app/..., scripts/..., ...)
                         that does not exist
    line-out-of-range    a `file:line` citation past the end of the file
    symbol-missing       a backticked `Class::method` whose final identifier
                         appears nowhere in first-party C++/QML/JSON (error)
    identifier-missing   a backticked bare camelCase name (a function or member
                         the doc is pointing at) that no longer exists (advisory)
    symbol-moved         `Class::method` where both halves exist but never
                         together -- a likely re-home (advisory)
    anchor-drift         a constant in scripts/doc-anchors.json whose code-side
                         pattern no longer matches, or whose doc-side literal
                         has gone missing

`doc/claude/specs/**` is deliberately out of scope: a spec is a dated record of
what was decided, not a live claim about the tree, and files legitimately move
after one lands.

Wrap a region with `<!-- claim-verify off -->` / `<!-- claim-verify on -->` to
exempt it. Fenced code blocks are skipped wholesale -- they hold examples,
including paths that are meant to be illustrative.

Usage:
    python3 scripts/claim-verify.py                 # scan the AI-facing tier
    python3 scripts/claim-verify.py CLAUDE.md       # explicit file or dir
    python3 scripts/claim-verify.py --no-report     # don't write .claim-report

Exit codes:
    0 - clean (advisories alone don't fail)
    1 - errors found
    2 - argument error
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
ANCHORS_PATH = Path(__file__).with_name("doc-anchors.json")
REPORT_PATH = REPO_ROOT / ".claim-report"
BASELINE_PATH = Path(__file__).with_name("claim-baseline.json")

# The always-loaded tier. `doc/claude/specs/**` is excluded on purpose (see the
# module docstring); so is any build output that wanders into the tree.
DOC_TARGETS = ("CLAUDE.md", "doc/claude", ".claude/skills")
DOC_EXCLUDE_DIRS = frozenset({"specs", "build", "node_modules", "__pycache__"})

# Trees searched for C++/QML symbols. `lib/` is vendored, so a symbol that only
# resolves there is not a claim about first-party code.
SOURCE_TREES = ("app/src", "app/qml")
SOURCE_SUFFIXES = (".cpp", ".h", ".qml")

# Schema and manifest keys (`reservedId`, `jsonKey`, `formIdOrder`) are named in the
# docs exactly like C++ members, and they are just as checkable.
RESOURCE_TREES = ("app/rcc",)
RESOURCE_SUFFIXES = (".json",)
SOURCE_EXCLUDE_DIRS = frozenset({"ThirdParty"})

# A backticked span is treated as a repo path only when it starts with one of
# these. Anything else (`.ts`, `foo/bar` in prose, a shell fragment) is left
# alone -- a false path finding costs more than the miss.
PATH_PREFIXES = (
    "app/",
    "lib/",
    "doc/",
    "cmake/",
    "scripts/",
    "tests/",
    "examples/",
    ".claude/",
    ".github/",
)

# Namespaces whose members live outside this repo.
FOREIGN_NAMESPACES = frozenset(
    {
        "std",
        "Qt",
        "QQmlEngine",
        "QJSEngine",
        "QMetaObject",
        "QChar",
        "QRegularExpression",
        "QPainter",
        "QHash",
        "QMap",
        "QList",
        "QVector",
        "QJSValue",
        "QQuickItem",
        "QQuickWindow",
        "QWidget",
        "QPlainTextEdit",
        "QCodeEditor",
        "QLineNumberArea",
        "QString",
        "QByteArray",
        "QDateTime",
        "QDir",
        "QFile",
        "QObject",
        "QVariant",
        "QColor",
        "QImage",
        "QUrl",
        "QProcess",
        "QSettings",
        "QCoreApplication",
        "QGuiApplication",
        "QApplication",
        "QTest",
        "QThread",
        "QTimer",
        "QLocale",
        "QSslSocket",
        "QAbstractSocket",
        "QSerialPort",
        "QSerialPortInfo",
        "QBluetoothDeviceInfo",
        "QLowEnergyController",
        "QAudioFormat",
        "QModbusDataUnit",
        "QCanBusDevice",
        "QMqttClient",
        "UA_Client",
        "lua_State",
        "chrono",
        "filesystem",
        "ranges",
        "views",
        "this_thread",
        "memory_order",
    }
)

# Bare camelCase names the docs legitimately write without the repo calling
# them: Qt API named in review guidance, and the naming-convention placeholders
# the style docs use as examples.
FOREIGN_IDENTS = frozenset(
    {
        "camelCase",
        "charCodeAt",
        "quitNow",
        "activatedAmbiguously",
        "renderSceneGraph",
        "kCamelCase",
        "m_camelCase",
        "lowerCase",
        "beginMoveRows",
        "beginInsertRows",
        "beginRemoveRows",
        "layoutChanged",
        "dataChanged",
        "qDeleteAll",
        "deleteLater",
        "saveToFile",
        "timeOut",
        "readyRead",
        "aboutToQuit",
    }
)

FENCE_RE = re.compile(r"^\s*(```|~~~)")
OFF_RE = re.compile(r"<!--\s*claim-verify\s+off\s*-->")
ON_RE = re.compile(r"<!--\s*claim-verify\s+on\s*-->")
CODE_SPAN_RE = re.compile(r"`([^`\n]+)`")
LINK_RE = re.compile(r"\[[^\]]*\]\(([^)\s]+)(?:\s+\"[^\"]*\")?\)")
SYMBOL_RE = re.compile(r"^(?:[A-Z][A-Za-z0-9_]*::)+[A-Za-z_][A-Za-z0-9_]*(?:\(\))?$")
IDENT_RE = re.compile(r"[A-Za-z_][A-Za-z0-9_]*")
DECL_RE = re.compile(r"\b(?:class|struct)\s+([A-Z][A-Za-z0-9_]*)\s*(?=[{:])")

# A backticked bare camelCase word is nearly always a function or member the
# docs are naming. Requiring an interior capital is what keeps ordinary prose
# in backticks (`changed`, `true`) out of the check.
CAMEL_RE = re.compile(r"^[a-z][A-Za-z0-9_]*[A-Z][A-Za-z0-9_]*(?:\(\))?$")
LINE_CITE_RE = re.compile(r"^(?P<path>[^:\s]+):(?P<line>\d+)$")
BRACE_RE = re.compile(r"\{([^{}]*)\}")
DUAL_SUFFIX_RE = re.compile(
    r"^(?P<stem>.+)\.(?P<first>\w+)\s*[/|]\s*\.?(?P<second>\w+)$"
)

# Documentation writes templates as `app/src/IO/Drivers/<Name>.h` and
# `doc/claude/specs/NNNN-slug/spec.md`. Those are shapes, not paths.
PLACEHOLDER_RE = re.compile(r"<[^>]*>|NNNN")


class Finding:
    """One failed claim, with the file that made it."""

    def __init__(self, path: Path, line: int, kind: str, message: str, error: bool):
        self.path = path
        self.line = line
        self.kind = kind
        self.message = message
        self.error = error

    def key(self) -> str:
        """Identity that survives edits elsewhere in the file (no line number)."""
        rel = self.path.resolve().relative_to(REPO_ROOT).as_posix()
        return f"{rel}\t{self.kind}\t{self.message}"

    def render(self) -> str:
        rel = self.path.resolve().relative_to(REPO_ROOT).as_posix()
        tier = "error" if self.error else "advisory"
        return f"{rel}:{self.line}: {tier}: {self.kind}: {self.message}"


def iter_doc_files(targets: list[Path]):
    """Yield every Markdown file in the AI-facing tier under `targets`."""
    for target in targets:
        if target.is_file():
            if target.suffix == ".md":
                yield target
            continue
        for path in sorted(target.rglob("*.md")):
            if DOC_EXCLUDE_DIRS.intersection(path.parts):
                continue
            yield path


def _visible_lines(lines: list[str]) -> list[bool]:
    """Mask out fenced code blocks and claim-verify-off regions."""
    visible = [True] * len(lines)
    in_fence = False
    suppressed = False
    for i, raw in enumerate(lines):
        if OFF_RE.search(raw):
            suppressed = True
        if FENCE_RE.match(raw):
            in_fence = not in_fence
            visible[i] = False
            continue
        visible[i] = not in_fence and not suppressed
        if ON_RE.search(raw):
            suppressed = False
    return visible


class SourceIndex:
    """First-party C++/QML, indexed three ways for the symbol checks.

    `blob` answers "is this exact qualified name written anywhere"; `idents`
    answers "does this identifier exist at all"; `owners` maps a declared type
    to every identifier in the files that declare it, which is what separates
    a method that moved from a data member the docs simply never wrote
    qualified (`Dataset::xAxisId` is real prose about a real member, and
    flagging it would train everyone to ignore this report)."""

    def __init__(self, blob: str, idents: set[str], owners: dict[str, set[str]]):
        self.blob = blob
        self.idents = idents
        self.owners = owners


def build_source_index() -> SourceIndex:
    """Read first-party C++/QML once and build the three symbol indexes."""
    chunks: list[str] = []
    owners: dict[str, set[str]] = {}

    for tree in SOURCE_TREES:
        base = REPO_ROOT / tree
        if not base.is_dir():
            continue
        for path in sorted(base.rglob("*")):
            if path.suffix not in SOURCE_SUFFIXES:
                continue
            if SOURCE_EXCLUDE_DIRS.intersection(path.parts):
                continue
            try:
                text = path.read_text(encoding="utf-8", errors="replace")
            except OSError:
                continue

            chunks.append(text)
            file_idents = set(IDENT_RE.findall(text))
            for name in set(DECL_RE.findall(text)) | {path.stem}:
                owners.setdefault(name, set()).update(file_idents)

    for tree in RESOURCE_TREES:
        base = REPO_ROOT / tree
        if not base.is_dir():
            continue
        for path in sorted(base.rglob("*")):
            if path.suffix not in RESOURCE_SUFFIXES:
                continue
            try:
                chunks.append(path.read_text(encoding="utf-8", errors="replace"))
            except OSError:
                continue

    blob = "\n".join(chunks)
    return SourceIndex(blob, set(IDENT_RE.findall(blob)), owners)


def _path_token(span: str) -> str | None:
    """Extract the repo path a code span starts with, if it is one."""
    token = span.strip().split()[0] if span.strip() else ""
    token = token.rstrip(".,;:")
    if not token.startswith(PATH_PREFIXES):
        return None
    return token


def _expand(token: str) -> list[str]:
    """Expand the shorthands the docs use for sibling files.

    `Foo.{h,cpp}` and `Foo.h/.cpp` both name two real files; a reader parses
    them without thinking, and flagging them as one missing path would train
    everyone to ignore the report."""
    dual = DUAL_SUFFIX_RE.match(token)
    if dual is not None:
        stem = dual.group("stem")
        return [f"{stem}.{dual.group('first')}", f"{stem}.{dual.group('second')}"]

    match = BRACE_RE.search(token)
    if match is None:
        return [token]

    head, tail = token[: match.start()], token[match.end() :]
    return [f"{head}{option}{tail}" for option in match.group(1).split(",") if option]


def _resolve(token: str) -> bool:
    """True when a repo-relative path (possibly a glob) matches something."""
    for candidate in _expand(token):
        if any(ch in candidate for ch in "*?["):
            if not any(REPO_ROOT.glob(candidate)):
                return False
        elif not (REPO_ROOT / candidate).exists():
            return False
    return True


def check_paths(path: Path, lines: list[str], visible: list[bool]) -> list[Finding]:
    """Verify every backticked repo path and `file:line` citation resolves."""
    out: list[Finding] = []
    for i, raw in enumerate(lines):
        if not visible[i]:
            continue
        for span in CODE_SPAN_RE.findall(raw):
            token = _path_token(span)
            if token is None or PLACEHOLDER_RE.search(token):
                continue

            cite = LINE_CITE_RE.match(token)
            if cite is not None:
                target = REPO_ROOT / cite.group("path")
                if not target.is_file():
                    out.append(
                        Finding(
                            path,
                            i + 1,
                            "path-missing",
                            f"`{cite.group('path')}` does not exist",
                            True,
                        )
                    )
                    continue
                count = len(
                    target.read_text(encoding="utf-8", errors="replace").splitlines()
                )
                if int(cite.group("line")) > count:
                    out.append(
                        Finding(
                            path,
                            i + 1,
                            "line-out-of-range",
                            f"`{token}` cites past the end of the file ({count} lines)",
                            True,
                        )
                    )
                continue

            if not _resolve(token):
                out.append(
                    Finding(
                        path, i + 1, "path-missing", f"`{token}` does not exist", True
                    )
                )
    return out


def check_links(path: Path, lines: list[str], visible: list[bool]) -> list[Finding]:
    """Verify every relative Markdown link target exists on disk."""
    out: list[Finding] = []
    for i, raw in enumerate(lines):
        if not visible[i]:
            continue
        spans = CODE_SPAN_RE.findall(raw)
        for target in LINK_RE.findall(raw):
            if any(target in span for span in spans):
                continue
            if target.startswith(("http://", "https://", "mailto:", "#")):
                continue
            clean = target.split("#", 1)[0]
            if not clean:
                continue
            if (path.parent / clean).exists():
                continue
            out.append(
                Finding(
                    path,
                    i + 1,
                    "link-target-missing",
                    f"`{clean}` does not resolve from {path.parent.name}/",
                    True,
                )
            )
    return out


def check_symbols(
    path: Path, lines: list[str], visible: list[bool], index: SourceIndex
) -> list[Finding]:
    """Verify every backticked `Class::method` still exists in first-party code."""
    out: list[Finding] = []
    for i, raw in enumerate(lines):
        if not visible[i]:
            continue
        for span in CODE_SPAN_RE.findall(raw):
            text = span.strip()
            if CAMEL_RE.match(text):
                bare = text[:-2] if text.endswith("()") else text
                if bare not in index.idents and bare not in FOREIGN_IDENTS:
                    out.append(
                        Finding(
                            path,
                            i + 1,
                            "identifier-missing",
                            f"`{text}` appears nowhere under app/src or app/qml",
                            False,
                        )
                    )
                continue

            if not SYMBOL_RE.match(text):
                continue

            qualified = text[:-2] if text.endswith("()") else text
            parts = qualified.split("::")
            if parts[0] in FOREIGN_NAMESPACES:
                continue

            member = parts[-1]
            owner = parts[-2]
            if qualified in index.blob or f"{owner}::{member}" in index.blob:
                continue

            if member not in index.idents:
                out.append(
                    Finding(
                        path,
                        i + 1,
                        "symbol-missing",
                        f"`{text}`: `{member}` appears nowhere under app/src or app/qml",
                        True,
                    )
                )
                continue

            declared = index.owners.get(owner)
            if declared is not None and member not in declared:
                out.append(
                    Finding(
                        path,
                        i + 1,
                        "symbol-moved",
                        f"`{text}`: `{member}` exists, but not in any file declaring "
                        f"`{owner}`",
                        False,
                    )
                )
    return out


def _out_of_order(placed: list) -> list:
    """The entries to move: everything outside the longest already-correct run.

    Reporting every entry whose position disagrees with a running maximum turns one displaced
    block into a finding per entry after it. The complement of the longest increasing
    subsequence is the minimal set that actually has to move.
    """
    if len(placed) < 2:
        return []

    best = [1] * len(placed)
    prev = [-1] * len(placed)
    for i in range(len(placed)):
        for j in range(i):
            if placed[j][1] < placed[i][1] and best[j] + 1 > best[i]:
                best[i] = best[j] + 1
                prev[i] = j

    end = max(range(len(placed)), key=lambda i: best[i])
    keep = set()
    while end != -1:
        keep.add(end)
        end = prev[end]

    return [symbol for i, (symbol, _) in enumerate(placed) if i not in keep]


def _ordered_doc_index(body: str, symbol: str) -> int:
    """First offset in `body` where the doc names `symbol`, however it qualifies it.

    The docs write the same entity as `Translator`, `Misc::ProblemCenter` or
    `IO::PipelineHost` depending on how ambiguous the bare name is, so the match is on the
    last `::` segment inside a backticked token."""
    simple = symbol.split("::")[-1]
    match = re.search(r"`[A-Za-z_:]*\b" + re.escape(simple) + r"`", body)
    return match.start() if match else -1


def _check_ordered_anchor(anchor: dict) -> list[Finding]:
    """Pin a doc's ORDERED list against the order the code actually uses.

    A `must_contain` anchor proves a name is mentioned somewhere; it cannot prove a sequence.
    Construction order IS the contract for the composition root -- CLAUDE.md forbids reordering
    `instantiateCoreModules()` without re-running the ctor-edge proof -- so a doc that lists the
    same names in a different order is worse than no list: it reads as authoritative. This
    anchor extracts one capture per entry from the source, in source order, and requires the doc
    to name them in that same order.
    """
    out: list[Finding] = []
    name = anchor["name"]
    source = REPO_ROOT / anchor["source"]
    if not source.is_file():
        return [
            Finding(
                ANCHORS_PATH,
                1,
                "anchor-drift",
                f"{name}: source `{anchor['source']}` does not exist",
                True,
            )
        ]

    text = source.read_text(encoding="utf-8", errors="replace")
    scope = anchor.get("source_scope")
    if scope:
        window = re.search(scope, text)
        if window is None:
            return [
                Finding(
                    ANCHORS_PATH,
                    1,
                    "anchor-drift",
                    f"{name}: `{anchor['source']}` no longer matches the source scope "
                    f"/{scope}/ -- the sequence this anchor pins has moved",
                    True,
                )
            ]
        text = window.group(0)

    symbols = [m.group(1) for m in re.finditer(anchor["source_pattern"], text)]
    if len(symbols) < 2:
        return [
            Finding(
                ANCHORS_PATH,
                1,
                "anchor-drift",
                f"{name}: `{anchor['source_pattern']}` captured {len(symbols)} entries; "
                f"an ordered anchor needs at least two",
                True,
            )
        ]

    for doc in anchor.get("docs", []):
        doc_path = REPO_ROOT / doc["path"]
        if not doc_path.is_file():
            out.append(
                Finding(
                    ANCHORS_PATH,
                    1,
                    "anchor-drift",
                    f"{name}: doc `{doc['path']}` does not exist",
                    True,
                )
            )
            continue

        body = doc_path.read_text(encoding="utf-8", errors="replace")
        doc_scope = doc.get("scope")
        if doc_scope:
            window = re.search(doc_scope, body, re.S)
            if window is None:
                out.append(
                    Finding(
                        doc_path,
                        1,
                        "anchor-drift",
                        f"{name}: the section this anchor pins (/{doc_scope}/) is gone",
                        True,
                    )
                )
                continue
            body = window.group(0)

        placed: list[tuple[str, int]] = []
        for symbol in symbols:
            index = _ordered_doc_index(body, symbol)
            if index < 0:
                out.append(
                    Finding(
                        doc_path,
                        1,
                        "anchor-drift",
                        f"{name}: `{symbol}` is in {anchor['source']} but not in this list",
                        True,
                    )
                )
                continue
            placed.append((symbol, index))

        for symbol in _out_of_order(placed):
            out.append(
                Finding(
                    doc_path,
                    1,
                    "anchor-drift",
                    f"{name}: `{symbol}` sits in the wrong place in this list -- "
                    f"{anchor['source']} constructs it somewhere else in the sequence",
                    True,
                )
            )

    return out


def check_anchors() -> list[Finding]:
    """Verify the constants the docs quote still read that way in the code.

    Each anchor binds both sides: the code-side regex must still match in the
    file that owns the value, and the doc-side literal must still appear in the
    file that quotes it. Either half going stale is the finding."""
    out: list[Finding] = []
    if not ANCHORS_PATH.is_file():
        return out

    spec = json.loads(ANCHORS_PATH.read_text(encoding="utf-8"))
    for anchor in spec.get("anchors", []):
        if anchor.get("kind") == "ordered":
            out.extend(_check_ordered_anchor(anchor))
            continue

        name = anchor["name"]
        source = REPO_ROOT / anchor["source"]
        if not source.is_file():
            out.append(
                Finding(
                    ANCHORS_PATH,
                    1,
                    "anchor-drift",
                    f"{name}: source `{anchor['source']}` does not exist",
                    True,
                )
            )
            continue

        text = source.read_text(encoding="utf-8", errors="replace")
        if re.search(anchor["source_pattern"], text) is None:
            out.append(
                Finding(
                    ANCHORS_PATH,
                    1,
                    "anchor-drift",
                    f"{name}: `{anchor['source']}` no longer matches "
                    f"/{anchor['source_pattern']}/ -- the value moved or changed, "
                    f"so every doc quoting it is now suspect",
                    True,
                )
            )
            continue

        for doc in anchor.get("docs", []):
            doc_path = REPO_ROOT / doc["path"]
            if not doc_path.is_file():
                out.append(
                    Finding(
                        ANCHORS_PATH,
                        1,
                        "anchor-drift",
                        f"{name}: doc `{doc['path']}` does not exist",
                        True,
                    )
                )
                continue
            body = doc_path.read_text(encoding="utf-8", errors="replace")
            if doc["must_contain"] not in body:
                out.append(
                    Finding(
                        doc_path,
                        1,
                        "anchor-drift",
                        f"{name}: expected \"{doc['must_contain']}\" to appear here",
                        True,
                    )
                )
    return out


def load_baseline() -> set[str]:
    """Read the accepted-drift baseline, if one has been seeded."""
    if not BASELINE_PATH.is_file():
        return set()
    spec = json.loads(BASELINE_PATH.read_text(encoding="utf-8"))
    return set(spec.get("accepted", []))


def write_baseline(findings: list[Finding]) -> None:
    """Freeze today's errors so the gate can block tomorrow's."""
    spec = {
        "purpose": (
            "Claims scripts/claim-verify.py flags today and that nobody has "
            "corrected yet. The gate fails on any error NOT listed here, so new "
            "drift blocks immediately while this backlog is worked down. Shrink "
            "it by fixing the doc, then re-run with --accept."
        ),
        "regenerate": "python scripts/claim-verify.py --accept",
        "accepted": sorted(f.key() for f in findings if f.error),
    }
    BASELINE_PATH.write_text(
        json.dumps(spec, indent=2, ensure_ascii=False) + "\n",
        encoding="utf-8",
        newline="\n",
    )


def write_report(findings: list[Finding]) -> None:
    """Group the findings by rule into `.claim-report` at the repo root."""
    header = (
        "# AI Documentation Claim Report\n\n"
        "Generated by `scripts/claim-verify.py`. Every entry is a claim in the\n"
        "always-loaded AI-facing tier (`CLAUDE.md`, `doc/claude/**`,\n"
        "`.claude/skills/**`) that no longer matches the tree. Errors block CI;\n"
        "advisories are judgement calls. Exempt a region with\n"
        "`<!-- claim-verify off -->` / `<!-- claim-verify on -->`.\n\n"
    )

    if not findings:
        REPORT_PATH.write_text(
            header + "No stale claims found.\n", encoding="utf-8", newline="\n"
        )
        return

    by_kind: dict[str, list[Finding]] = {}
    for finding in findings:
        by_kind.setdefault(finding.kind, []).append(finding)

    body = []
    for kind in sorted(by_kind):
        rows = by_kind[kind]
        body.append(f"## `{kind}` ({len(rows)})\n")
        for finding in rows:
            body.append(f"- {finding.render()}")
        body.append("")

    REPORT_PATH.write_text(
        header + "\n".join(body) + "\n", encoding="utf-8", newline="\n"
    )


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--quiet", action="store_true", help="print the summary only")
    parser.add_argument(
        "--accept",
        action="store_true",
        help="re-baseline scripts/claim-baseline.json to today's errors",
    )
    parser.add_argument(
        "--no-report", action="store_true", help="skip writing .claim-report"
    )
    parser.add_argument(
        "paths",
        nargs="*",
        type=Path,
        help="files or directories (default: the AI tier)",
    )
    args = parser.parse_args(argv)

    if args.paths:
        targets = [p if p.is_absolute() else REPO_ROOT / p for p in args.paths]
        missing = [t for t in targets if not t.exists()]
        if missing:
            print(f"no such path: {missing[0]}", file=sys.stderr)
            return 2
    else:
        targets = [REPO_ROOT / t for t in DOC_TARGETS]
        targets = [t for t in targets if t.exists()]

    docs = sorted(set(iter_doc_files(targets)))
    if not docs:
        print("no Markdown files found", file=sys.stderr)
        return 2

    index = build_source_index()

    findings: list[Finding] = []
    for doc in docs:
        lines = doc.read_text(encoding="utf-8", errors="replace").splitlines()
        visible = _visible_lines(lines)
        findings.extend(check_paths(doc, lines, visible))
        findings.extend(check_links(doc, lines, visible))
        findings.extend(check_symbols(doc, lines, visible, index))

    findings.extend(check_anchors())
    findings.sort(key=lambda f: (f.path.as_posix(), f.line, f.kind))

    if args.accept:
        write_baseline(findings)
        write_report(findings)
        accepted = sum(1 for f in findings if f.error)
        print(f"baseline written to {BASELINE_PATH} ({accepted} accepted)")
        return 0

    baseline = load_baseline()
    fresh = [f for f in findings if f.error and f.key() not in baseline]

    if not args.quiet:
        for finding in findings:
            print(finding.render())

    if not args.no_report:
        write_report(findings)

    errors = sum(1 for f in findings if f.error)
    advisories = len(findings) - errors
    print(
        f"claim-verify: {len(docs)} docs, {errors} error(s) "
        f"({len(fresh)} new), {advisories} advisory(ies)"
    )

    if fresh:
        print("\nnew stale claims (not in the baseline):", file=sys.stderr)
        for finding in fresh:
            print(f"  {finding.render()}", file=sys.stderr)
        print(
            "\nFix the doc against the tree -- the code is the truth, the doc is "
            "the suspect. If the claim is right and the checker is wrong, fence it "
            "with <!-- claim-verify off --> or re-baseline with "
            "python scripts/claim-verify.py --accept",
            file=sys.stderr,
        )
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
