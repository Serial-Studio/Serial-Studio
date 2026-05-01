"""Static-analysis rules for scripts/code-verify.py.

Adds CLAUDE.md-derived semantic checks on top of the formatter's existing
style rules. C++ rules use tree-sitter's C++ grammar to walk a real AST;
QML rules stay line-based on top of the tokenizer that already lives in
code-verify.py.

Each rule returns a list of (line, kind, message) tuples. The driver in
code-verify.py wraps them as Violations and routes them through the
existing flag-only / auto-fixable pipeline. Every rule here is flag-only.

Tree-sitter is the only new dependency. The module degrades gracefully:
when tree-sitter or tree-sitter-cpp aren't importable, C++ semantic
checks are silently skipped and the formatter still runs its line-based
rules. The CI install pins both in tests/requirements.txt.

`code-verify off / on` fences mask every rule here too, same as the
existing rules — the driver passes the fence mask in.
"""
from __future__ import annotations

import re
from dataclasses import dataclass
from pathlib import Path

try:
    import tree_sitter
    import tree_sitter_cpp
    _CPP_LANG = tree_sitter.Language(tree_sitter_cpp.language())
    _CPP_PARSER = tree_sitter.Parser(_CPP_LANG)
    HAS_TREE_SITTER = True
except Exception:
    HAS_TREE_SITTER = False
    _CPP_LANG = None
    _CPP_PARSER = None


# ---------------------------------------------------------------------------
# Public types
# ---------------------------------------------------------------------------

@dataclass(frozen=True)
class Finding:
    line: int
    kind: str
    message: str


# ---------------------------------------------------------------------------
# Hotpath method names (CLAUDE.md: never allocate on the dashboard path)
# ---------------------------------------------------------------------------

# Methods named here are walked for new/make_shared/append calls. The names
# come straight from CLAUDE.md's "Threading Rules" / "Hotpath" sections.
_HOTPATH_METHODS = frozenset({
    "hotpathRxFrame",
    "hotpathRxSourceFrame",
    "processData",
    "onReadyRead",
    "onFrameReady",
    "onRawDataReceived",
    "appendChunk",
    "frameTimestamp",
    "applyTransform",
    "parseProjectFrame",
    "updateData",
    "updateLineSeries",
    "pushSample",
})

# Calls / patterns banned on the hotpath. Each entry is (regex, message).
_HOTPATH_BANNED_CALLS = [
    (re.compile(r"\bnew\s+[A-Za-z_]"), "`new` allocation on hotpath"),
    (re.compile(r"\bstd::make_shared\b"), "`std::make_shared` allocation on hotpath"),
    (re.compile(r"\bstd::make_unique\b"), "`std::make_unique` allocation on hotpath"),
    (re.compile(r"\.append\("), "`.append(` (likely Qt container resize) on hotpath"),
    (re.compile(r"\.push_back\("), "`.push_back(` on hotpath (pre-reserve at init)"),
    (re.compile(r"\bemit\b"), "bare `emit` on hotpath -- use `Q_EMIT`"),
]


# ---------------------------------------------------------------------------
# Hardcoded JSON keys (CLAUDE.md: use Keys::* namespace)
# ---------------------------------------------------------------------------

# Mirrors the inline constexpr declarations at the top of Frame.h. When one
# of these strings appears as a literal in a .cpp/.h that ALSO uses ss_jsr
# or QJsonObject, it's almost certainly a writer/reader bypassing Keys::*.
_PROJECT_KEY_LITERALS = frozenset({
    "busType", "frameStart", "frameEnd", "frameDetection", "frameParserCode",
    "frameParserLanguage", "decoderMethod", "checksumAlgorithm",
    "hexadecimalDelimiters", "schemaVersion", "writerVersion",
    "writerVersionAtCreation", "sourceId",
    "datasetId", "uniqueId",
})


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def _line_of(node) -> int:
    """1-based start line of a tree-sitter node."""
    return node.start_point[0] + 1


def _node_text(node, src: bytes) -> str:
    return src[node.start_byte:node.end_byte].decode("utf-8", errors="replace")


def _walk(node):
    """Yield node and all descendants in depth-first order."""
    yield node
    for child in node.children:
        yield from _walk(child)


def _find_field_decl_default(class_node, src: bytes):
    """Yield (line, name) for every `field_declaration` in `class_node` that
    has a `default_value` (the `int m_foo = 0;` pattern that CLAUDE.md
    forbids in headers)."""
    body = class_node.child_by_field_name("body")
    if body is None:
        return
    for child in body.children:
        if child.type != "field_declaration":
            continue
        # field_declaration nodes carry their default_value as a named field.
        default = child.child_by_field_name("default_value")
        if default is None:
            continue
        decl = child.child_by_field_name("declarator")
        if decl is None:
            continue
        # Skip static member initializers (those are valid C++17 inline init,
        # not the in-header-default the rule cares about).
        # tree-sitter exposes `static` via storage_class_specifier siblings.
        is_static = any(
            c.type == "storage_class_specifier" and _node_text(c, src) == "static"
            for c in child.children
        )
        if is_static:
            continue
        # Skip constexpr — those have to be initialized in the header.
        is_constexpr = any(
            c.type == "type_qualifier" and _node_text(c, src) == "constexpr"
            for c in child.children
        )
        if is_constexpr:
            continue
        name = _node_text(decl, src).strip()
        yield (_line_of(child), name)


def _is_void_return(func_node, src: bytes) -> bool:
    """True when a function_definition / field_declaration returns void."""
    type_node = func_node.child_by_field_name("type")
    if type_node is None:
        return True  # constructors / destructors -- treat as "no return"
    return _node_text(type_node, src).strip() == "void"


def _has_attribute(node, src: bytes, attr_name: str) -> bool:
    """True when the declaration carries `[[<attr_name>]]` either before
    the type or as a sibling attribute_specifier."""
    # Attributes are parsed as attribute_declaration siblings before the
    # declaration's type, OR as attribute children inside the declarator.
    # Walk a small radius around the node to find them.
    text_before = src[max(0, node.start_byte - 200):node.start_byte].decode(
        "utf-8", errors="replace"
    )
    if f"[[{attr_name}]]" in text_before or f"[[ {attr_name} ]]" in text_before:
        return True
    for child in node.children:
        if "attribute" in child.type and attr_name in _node_text(child, src):
            return True
    return False


def _function_body_lines(body_node) -> int:
    """Number of source lines a `compound_statement` spans (end - start + 1).
    Returns 0 when the node is missing."""
    if body_node is None:
        return 0
    return body_node.end_point[0] - body_node.start_point[0] + 1


def _max_nesting_depth(body_node) -> int:
    """Maximum control-flow nesting depth inside a compound_statement.
    Counts if/else/for/while/do/switch/try blocks. Lambdas reset the
    counter -- their body is a separate scope."""
    if body_node is None:
        return 0

    NESTERS = {
        "if_statement", "for_statement", "for_range_loop",
        "while_statement", "do_statement", "switch_statement",
        "try_statement",
    }
    LAMBDA = "lambda_expression"

    def walk(n, depth):
        best = depth
        for c in n.children:
            if c.type == LAMBDA:
                continue  # lambda body is its own scope
            if c.type in NESTERS:
                best = max(best, walk(c, depth + 1))
            else:
                best = max(best, walk(c, depth))
        return best

    return walk(body_node, 0)


def _previous_doxygen_brief(node, src: bytes) -> bool:
    """True when the lines immediately before `node` form a /** @brief ... */
    block. The block may sit anywhere from 0 to ~3 blank/decorator lines
    above the declaration."""
    start_line = node.start_point[0]
    if start_line == 0:
        return False

    # Walk backward from the line just above the declaration. We want to
    # find a closing `*/` first, then verify that the matching `/**` is
    # within reach AND mentions @brief. This handles long doxygen blocks
    # (DSP.h's FixedQueue carries a 16-line @brief) without hand-tuning
    # a fixed lookback window.
    src_text = src.decode("utf-8", errors="replace")
    lines = src_text.split("\n")
    cur = start_line - 1
    # Tolerate up to 8 lines of pre-declaration noise between the doxygen
    # close and the declaration: blanks, banner comments, `template<...>`
    # clauses, `requires ...` constraints, attributes (`[[nodiscard]]`),
    # access specifiers carried over from a class body.
    skip = 0
    while cur >= 0 and skip < 8:
        s = lines[cur].strip()
        if (not s or s.startswith("//") or s.startswith("template")
                or s.startswith("requires") or s.startswith("[[")
                or s.startswith("public:") or s.startswith("private:")
                or s.startswith("protected:")):
            cur -= 1
            skip += 1
            continue
        break
    if cur < 0:
        return False
    if not lines[cur].rstrip().endswith("*/"):
        return False
    # Walk further back until we find `/**` -- look at most 60 lines.
    open_idx = -1
    for j in range(cur, max(0, cur - 60) - 1, -1):
        if "/**" in lines[j]:
            open_idx = j
            break
    if open_idx < 0:
        return False
    block = "\n".join(lines[open_idx:cur + 1])
    return "@brief" in block


# ---------------------------------------------------------------------------
# C++ rules (tree-sitter)
# ---------------------------------------------------------------------------

def _cpp_rules(src: bytes, path: Path, fence_mask: list[bool]) -> list[Finding]:
    """Run tree-sitter-backed C++/Qt rules on `src`. Returns one Finding per
    issue. `fence_mask[i]` is True when source line `i+1` sits inside a
    `// code-verify off` / `// code-verify on` fence and must be skipped.

    The rules implemented here mirror CLAUDE.md's "Common Mistakes" table
    and the NASA Power-of-Ten section:

        qt-bare-emit             bare `emit` outside strings/comments
        qt-uppercase-signal-slot Q_SIGNALS: / Q_SLOTS: section labels
        qt-invokable-void        Q_INVOKABLE void f() (use `public slots:`)
        qt-header-member-init    int m_foo = 0; in a header class body
        qt-disconnect-nullptr    disconnect(..., nullptr) as the slot arg
        qt-direct-jsengine-call  parseFunction.call(...) without guardedCall
        cxx-function-too-long    function definition over 100 lines
        cxx-nesting-too-deep     control-flow nesting > 3 levels
        cxx-goto-or-jmp          goto / setjmp / longjmp
        doc-missing-brief-cpp    .cpp function definition without /** @brief */
        doc-missing-brief-h      header type-level definition without /** @brief */
        hotpath-allocation       allocation/append on a known hotpath method
        keys-hardcoded-literal   raw "busType" etc. literal where Keys:: belongs
    """
    if not HAS_TREE_SITTER:
        return []

    out: list[Finding] = []
    tree = _CPP_PARSER.parse(src)
    root = tree.root_node
    is_header = path.suffix in {".h", ".hpp", ".hxx"}

    def fenced(line: int) -> bool:
        idx = line - 1
        return 0 <= idx < len(fence_mask) and fence_mask[idx]

    # ---- qt-bare-emit / qt-uppercase-signal-slot / cxx-goto / Q_INVOKABLE void
    # The fastest pass that catches text-level Qt mistakes is a token sweep
    # that respects strings/comments. We use a tiny per-line scan that
    # strips comments and string literals first.
    src_text = src.decode("utf-8", errors="replace")
    lines = src_text.split("\n")
    for i, raw in enumerate(lines, start=1):
        if fenced(i):
            continue
        scrubbed = _strip_strings_and_line_comments(raw)
        # Bare `emit` -- token must be word-isolated.
        if re.search(r"\bemit\b\s+[A-Za-z_]", scrubbed):
            out.append(Finding(i, "qt-bare-emit",
                               "use `Q_EMIT` instead of bare `emit`"))
        if re.search(r"\bQ_SIGNALS\s*:", scrubbed):
            out.append(Finding(i, "qt-uppercase-signal-slot",
                               "use lowercase `signals:` (CLAUDE.md)"))
        if re.search(r"\b(?:public|protected|private)?\s*Q_SLOTS\s*:", scrubbed):
            out.append(Finding(i, "qt-uppercase-signal-slot",
                               "use lowercase `slots:` (CLAUDE.md)"))
        if re.search(r"\bgoto\b", scrubbed):
            out.append(Finding(i, "cxx-goto-or-jmp",
                               "`goto` is banned (NASA Power of Ten rule 1)"))
        if re.search(r"\b(?:setjmp|longjmp)\s*\(", scrubbed):
            out.append(Finding(i, "cxx-goto-or-jmp",
                               "`setjmp`/`longjmp` are banned (NASA P10 rule 1)"))

    # Q_INVOKABLE void on the same source line is unambiguous; spread cases
    # are rare enough that we accept the false-negative.
    invokable_void_re = re.compile(r"\bQ_INVOKABLE\b\s+void\b")
    for i, raw in enumerate(lines, start=1):
        if fenced(i):
            continue
        scrubbed = _strip_strings_and_line_comments(raw)
        if invokable_void_re.search(scrubbed):
            out.append(Finding(i, "qt-invokable-void",
                               "`Q_INVOKABLE void` -- move to `public slots:`"))

    # disconnect(<conn>, nullptr) -- the 2-arg form where nullptr is in
    # the slot slot. CLAUDE.md flags this because it disconnects every
    # slot the connection was paired with. The 4-arg wildcard form
    # `disconnect(sender, nullptr, receiver, nullptr)` is idiomatic Qt
    # ("disconnect every signal-slot pair between sender and receiver")
    # and explicitly NOT what the rule cares about.
    for i, raw in enumerate(lines, start=1):
        if fenced(i):
            continue
        scrubbed = _strip_strings_and_line_comments(raw)
        # Match: disconnect ( <one-arg> , nullptr )  -- exactly two args.
        # The first arg can be a chain like `obj->signal`, but must not
        # contain a comma (we only want the 2-arg form).
        if re.search(r"\bdisconnect\s*\(\s*[^,()]+\s*,\s*nullptr\s*\)",
                     scrubbed):
            out.append(Finding(i, "qt-disconnect-nullptr",
                               "`disconnect(<conn>, nullptr)` disconnects ALL slots from the "
                               "connection; capture the QMetaObject::Connection and disconnect that"))

    # parseFunction.call(...) is the JS engine path that must go through
    # IScriptEngine::guardedCall for the runtime watchdog.
    for i, raw in enumerate(lines, start=1):
        if fenced(i):
            continue
        scrubbed = _strip_strings_and_line_comments(raw)
        if re.search(r"\bparseFunction\.call\s*\(", scrubbed):
            out.append(Finding(i, "qt-direct-jsengine-call",
                               "`parseFunction.call()` bypasses the runtime watchdog -- "
                               "route through `IScriptEngine::guardedCall()`"))

    # ---- AST-driven rules
    # Walk every function_definition: function-too-long, nesting-too-deep,
    # missing @brief in .cpp, hotpath allocations.
    for n in _walk(root):
        if n.type == "function_definition":
            line = _line_of(n)
            if fenced(line):
                continue
            body = n.child_by_field_name("body")
            length = _function_body_lines(body)
            if length > 100:
                out.append(Finding(line, "cxx-function-too-long",
                                   f"function spans {length} lines (>100; "
                                   f"NASA P10 rule 4 caps at 100)"))
            depth = _max_nesting_depth(body)
            if depth > 3:
                out.append(Finding(line, "cxx-nesting-too-deep",
                                   f"control-flow nesting depth {depth} (>3; "
                                   f"CLAUDE.md caps at 3)"))

            if not is_header and not _previous_doxygen_brief(n, src):
                fname = _function_name(n, src)
                # Skip lambdas (no name), tree-sitter macro mis-parses
                # (single-letter / lowercase keywords), and inline-defined
                # methods nested inside another function definition.
                if not fname:
                    continue
                if _has_function_ancestor(n):
                    continue
                # Anonymous-namespace static helpers are still public-facing
                # in this codebase; the rule applies. But generated/extern-C
                # main entry points aren't worth flagging.
                if fname == "main" and not _has_function_ancestor(n):
                    continue
                out.append(Finding(
                    line, "doc-missing-brief-cpp",
                    f"`{fname}` lacks a preceding `/** @brief ... */`"))

            # Hotpath allocations.
            fname = _function_name(n, src)
            if fname in _HOTPATH_METHODS and body is not None:
                body_text = _node_text(body, src)
                body_start = body.start_point[0] + 1
                for j, body_line in enumerate(body_text.split("\n")):
                    abs_line = body_start + j
                    if fenced(abs_line):
                        continue
                    scrubbed = _strip_strings_and_line_comments(body_line)
                    for pat, msg in _HOTPATH_BANNED_CALLS:
                        if pat.search(scrubbed):
                            out.append(Finding(
                                abs_line, "hotpath-allocation",
                                f"{msg} (in `{fname}`)"))
                            break

    # ---- Header-only rules: in-header member init, @brief on type-level defs
    if is_header:
        # In-header member init -- only QObject classes (`Q_OBJECT` macro
        # in the body) per CLAUDE.md. Plain POD structs / config bags are
        # idiomatic with `int x = 0;` and are NOT what the rule targets.
        for n in _walk(root):
            if n.type != "class_specifier":
                continue
            cls_line = _line_of(n)
            if fenced(cls_line):
                continue
            body = n.child_by_field_name("body")
            if body is None:
                continue
            body_text = _node_text(body, src)
            if "Q_OBJECT" not in body_text and "Q_GADGET" not in body_text:
                continue
            for line, name in _find_field_decl_default(n, src):
                if fenced(line):
                    continue
                # Only complain about `m_<name>` style members (CLAUDE.md
                # convention: private members are `m_camelCase`). Public
                # config fields without the prefix aren't covered.
                last_ident = name.split()[-1].lstrip("&*")
                if not last_ident.startswith("m_"):
                    continue
                out.append(Finding(
                    line, "qt-header-member-init",
                    f"in-header default init `{last_ident}` -- "
                    f"initialize in the constructor member init list"))

        # Type-level @brief: every class/struct/enum/typedef/using-alias at
        # namespace scope needs a /** @brief */ banner, per CLAUDE.md.
        for n in _walk(root):
            if n.type not in {
                "class_specifier", "struct_specifier",
                "enum_specifier", "type_definition", "alias_declaration",
            }:
                continue
            # Skip nested types (parent is field_declaration_list / class
            # body) -- the enclosing class's @brief covers them.
            if _is_nested_type(n):
                continue
            # Skip forward declarations (no body / definition).
            if n.type in ("class_specifier", "struct_specifier",
                          "enum_specifier"):
                body_field = n.child_by_field_name("body")
                if body_field is None:
                    continue
            # Skip type aliases inside function bodies / templates (not
            # namespace-scope, no @brief required).
            if _has_function_ancestor(n):
                continue
            line = _line_of(n)
            if fenced(line):
                continue
            if not _previous_doxygen_brief(n, src):
                name = _type_name(n, src)
                if name:
                    out.append(Finding(
                        line, "doc-missing-brief-h",
                        f"`{name}` lacks a preceding `/** @brief ... */`"))

    # ---- nodiscard on const getters in headers. CLAUDE.md says
    # "[[nodiscard]] on every non-void return". We narrow to the case
    # most likely to be a real getter (and least likely to be noise):
    # const member functions returning non-void inside a class body.
    # Setters, signals, Q_INVOKABLE methods, operators and constructors
    # are explicitly skipped.
    if is_header:
        for n in _walk(root):
            if n.type != "field_declaration":
                continue
            line = _line_of(n)
            if fenced(line):
                continue
            decl = n.child_by_field_name("declarator")
            if decl is None or decl.type != "function_declarator":
                continue
            type_node = n.child_by_field_name("type")
            if type_node is None:
                continue
            if _node_text(type_node, src).strip() == "void":
                continue
            if _in_signals_section(n, src):
                continue
            if _has_attribute(n, src, "nodiscard"):
                continue
            # Const member functions only. tree-sitter exposes the trailing
            # const as a `type_qualifier` inside the function_declarator.
            is_const = any(
                c.type == "type_qualifier" and _node_text(c, src) == "const"
                for c in decl.children
            )
            if not is_const:
                continue
            # Skip Q_INVOKABLE -- those are runtime-callable, not getters
            # with a strong nodiscard contract.
            line_text = src.decode("utf-8", errors="replace").split("\n")[line - 1]
            if "Q_INVOKABLE" in line_text or "Q_PROPERTY" in line_text:
                continue
            name = _node_text(decl, src).split("(", 1)[0].strip()
            if not name or name.startswith("operator"):
                continue
            # Skip setters that happen to be const (extremely rare, but
            # leaves a clean exit condition).
            if name.startswith("set") and len(name) > 3 and name[3].isupper():
                continue
            out.append(Finding(
                line, "qt-missing-nodiscard",
                f"non-void const getter `{name}` lacks `[[nodiscard]]`"))

    # ---- Hardcoded JSON keys: only flag inside writer/reader functions,
    # detected by presence of `ss_jsr(` OR `QJsonObject` OR `QJsonValue` in
    # the file. Frame.h itself is the source of truth and exempt.
    if path.name not in {"Frame.h", "Frame.cpp"}:
        text = src_text
        if "ss_jsr(" in text or "QJsonObject" in text or "QJsonValue" in text:
            for i, raw in enumerate(lines, start=1):
                if fenced(i):
                    continue
                scrubbed = _strip_line_comments(raw)
                # Match `"<key>"` literal where <key> is in the curated list.
                for m in re.finditer(r'"([A-Za-z_][A-Za-z0-9_]*)"', scrubbed):
                    if m.group(1) in _PROJECT_KEY_LITERALS:
                        out.append(Finding(
                            i, "keys-hardcoded-literal",
                            f"hardcoded JSON key `\"{m.group(1)}\"` -- "
                            f"use `Keys::{_camel(m.group(1))}` from Frame.h"))
                        break  # one finding per line is plenty
    return out


def _function_name(func_node, src: bytes) -> str:
    """Return the function's name from a function_definition node, or "".
    Walks declarator -> identifier / qualified_identifier / field_identifier."""
    decl = func_node.child_by_field_name("declarator")
    while decl is not None and decl.type == "function_declarator":
        decl = decl.child_by_field_name("declarator")
    if decl is None:
        return ""
    if decl.type in ("identifier", "field_identifier", "destructor_name"):
        return _node_text(decl, src)
    if decl.type == "qualified_identifier":
        # last segment is the actual function name.
        last = None
        for child in decl.children:
            if child.type in ("identifier", "field_identifier",
                              "destructor_name"):
                last = child
        if last is not None:
            return _node_text(last, src)
    if decl.type == "operator_name":
        return _node_text(decl, src)
    return ""


def _type_name(node, src: bytes) -> str:
    """Return the named type's identifier, or "" for anonymous declarations."""
    name = node.child_by_field_name("name")
    if name is not None:
        return _node_text(name, src)
    # alias_declaration has no `name` field but its first identifier is the
    # alias being declared.
    for child in node.children:
        if child.type in ("type_identifier", "identifier"):
            return _node_text(child, src)
    return ""


def _is_nested_type(node) -> bool:
    """True when `node` lives inside a class/struct body."""
    parent = node.parent
    while parent is not None:
        if parent.type == "field_declaration_list":
            return True
        if parent.type == "translation_unit":
            return False
        parent = parent.parent
    return False


def _has_function_ancestor(node) -> bool:
    """True when `node` is nested inside another function_definition --
    typically a lambda body or an inline helper. Such nodes inherit the
    enclosing function's @brief."""
    parent = node.parent
    while parent is not None:
        if parent.type in ("function_definition", "lambda_expression"):
            return True
        parent = parent.parent
    return False


def _in_signals_section(node, src: bytes) -> bool:
    """Heuristic: walk backwards to the previous access_specifier inside the
    same field_declaration_list. Returns True when that specifier is
    `signals` or `Q_SIGNALS`."""
    parent = node.parent
    if parent is None or parent.type != "field_declaration_list":
        return False
    cur = node
    while cur is not None:
        prev = cur.prev_sibling
        if prev is None:
            return False
        if prev.type == "access_specifier":
            spec = _node_text(prev, src).strip().rstrip(":").strip()
            return spec in ("signals", "Q_SIGNALS")
        cur = prev
    return False


# ---------------------------------------------------------------------------
# Line-text helpers (string/comment stripping)
# ---------------------------------------------------------------------------

_STRING_RE = re.compile(r'"(?:\\.|[^"\\])*"')
_CHAR_RE = re.compile(r"'(?:\\.|[^'\\])'")


def _strip_strings_and_line_comments(line: str) -> str:
    """Replace string / char literals and `//` line comments with spaces of
    the same length. Used before token-level regex scans so a `// emit` in
    a comment can't trigger qt-bare-emit. Block comments (`/* */`) are
    left alone -- multi-line comment handling lives in the formatter."""
    # Doxygen / block-comment continuation lines: ` * emit ...` is prose,
    # never code, even though it isn't a `//` line comment. Strip the
    # whole line when it starts with optional whitespace + `*`.
    if re.match(r"^\s*\*(?!\w)", line):
        return " " * len(line)
    line = _STRING_RE.sub(lambda m: " " * len(m.group(0)), line)
    line = _CHAR_RE.sub(lambda m: " " * len(m.group(0)), line)
    idx = line.find("//")
    if idx >= 0:
        line = line[:idx] + " " * (len(line) - idx)
    return line


def _strip_line_comments(line: str) -> str:
    """Replace only `//` line comments with spaces -- string literals remain
    visible. Used when the rule cares about the contents of strings."""
    idx = line.find("//")
    if idx >= 0:
        return line[:idx] + " " * (len(line) - idx)
    return line


def _camel(snake: str) -> str:
    """Map a JSON key to its Frame.h Keys:: identifier (PascalCase). The
    constants in Frame.h are PascalCase versions of the camelCase JSON
    keys -- e.g. `frameStart` -> `FrameStart`."""
    return snake[:1].upper() + snake[1:]


# ---------------------------------------------------------------------------
# QML semantic rules (line-based -- no tree-sitter for QML)
# ---------------------------------------------------------------------------

# Files that are allowed to use direct font.pixelSize/font.family. These are
# the dashboard widgets that compute zoom-dependent font sizes; the
# `font:` helpers don't accept dynamic sizes.
_FONT_PIXEL_OK_FILES = frozenset({
    "FFTPlot.qml", "Plot.qml", "MultiPlot.qml", "Bar.qml",
    "Gauge.qml", "Compass.qml", "DataGrid.qml", "Accelerometer.qml",
    "Gyroscope.qml", "GPS.qml", "LEDPanel.qml", "Plot3D.qml",
    "Waterfall.qml", "ImageView.qml", "Terminal.qml", "PlotWidget.qml",
    "VisualRange.qml", "NotificationLog.qml",
    # Project-editor visualizations that compute zoom-dependent font sizes
    # are also exempt -- the `font:` helpers don't accept a dynamic
    # pixelSize, and these views need it for Ctrl+Wheel zoom.
    "FlowDiagram.qml",
})


def _qml_rules(src: str, path: Path, fence_mask: list[bool]) -> list[Finding]:
    """QML semantic rules. Line-based: tree-sitter doesn't have a published
    QML grammar, and the existing tokenizer in code-verify.py already
    tracks the structure we need."""
    out: list[Finding] = []
    if path.suffix != ".qml":
        return out

    file_allows_pixel = path.name in _FONT_PIXEL_OK_FILES

    for i, raw in enumerate(src.split("\n"), start=1):
        if i - 1 < len(fence_mask) and fence_mask[i - 1]:
            continue
        # Strip trailing comment -- still a flat scan, but enough to avoid
        # complaining about comment text.
        line = _strip_line_comments(raw)

        # qml-font-pixel: `font.pixelSize:` / `font.family:` outside the
        # dashboard whitelist. JS function bodies inside QML still match,
        # but those very rarely set fonts.
        if not file_allows_pixel:
            if re.search(r"\bfont\.(pixelSize|pointSize|family|bold|italic"
                         r"|weight|capitalization)\s*:", line):
                out.append(Finding(
                    i, "qml-font-pixel",
                    "use `font: Cpp_Misc_CommonFonts.uiFont` (or another "
                    "helper) instead of individual `font.*` sub-properties"))

        # qml-bus-type-int: `busType: <integer>` or `BusType: <int>`. The
        # property name on a `Source` is `busType`; the C++ enum is
        # `SerialStudio.BusType.UART`. Reject literal ints.
        m = re.search(r"\bbusType\s*:\s*(-?\d+)\b", line)
        if m:
            out.append(Finding(
                i, "qml-bus-type-int",
                f"hardcoded busType `{m.group(1)}` -- use "
                f"`SerialStudio.BusType.<NAME>`"))

    return out


# ---------------------------------------------------------------------------
# Public entry point
# ---------------------------------------------------------------------------

def analyze(path: Path, src_text: str, fence_mask: list[bool]) -> list[Finding]:
    """Run every applicable rule against `src_text` for `path`. The driver
    in code-verify.py wraps each Finding as a Violation."""
    suffix = path.suffix.lower()
    if suffix in (".cpp", ".cc", ".cxx", ".c", ".h", ".hpp", ".hxx", ".mm"):
        return _cpp_rules(src_text.encode("utf-8"), path, fence_mask)
    if suffix == ".qml":
        return _qml_rules(src_text, path, fence_mask)
    return []
