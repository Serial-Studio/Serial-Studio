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
_HOTPATH_METHODS = frozenset(
    {
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
    }
)

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
# CPU-microarchitecture / performance rules
# ---------------------------------------------------------------------------
#
# These rules apply knowledge of how compiled C++ behaves at the assembly /
# register / branch-predictor / cache level. The cycle counts in the rule
# messages are representative for current Intel (Skylake-derived) and ARM
# (Cortex-A7x/A78) microarchitectures; exact numbers vary with the target.
# All rules ship as advisory -- the goal is a checklist for a follow-up
# human / LLM pass, not a CI gate.

# Heavy types -- known to be expensive to copy by value. Even implicitly
# shared Qt containers (QString/QByteArray/QList/...) pay an atomic refcount
# bump on the COW pointer, which is a `lock`-prefix instruction on x86 or an
# `ldxr/stxr` loop on ARM without LSE. std:: containers do a full deep copy.
_HEAVY_TYPES = frozenset(
    {
        "QString",
        "QByteArray",
        "QStringList",
        "QVariant",
        "QVariantMap",
        "QVariantList",
        "QVariantHash",
        "QList",
        "QVector",
        "QMap",
        "QHash",
        "QSet",
        "QQueue",
        "QStack",
        "QJsonObject",
        "QJsonArray",
        "QJsonDocument",
        "QJsonValue",
        "QImage",
        "QPixmap",
        "QPolygon",
        "QPolygonF",
        "QPainterPath",
        "QBitArray",
        "QDateTime",
        "std::string",
        "std::wstring",
        "std::vector",
        "std::map",
        "std::unordered_map",
        "std::list",
        "std::deque",
        "std::set",
        "std::unordered_set",
        "std::multimap",
        "std::unordered_multimap",
    }
)

_REFCOUNTED_TYPES = frozenset(
    {
        "std::shared_ptr",
        "QSharedPointer",
        "QSharedDataPointer",
        "QExplicitlySharedDataPointer",
        "boost::shared_ptr",
    }
)


# File-wide perf patterns: scanned inside every function body, not just
# hotpath methods. Cost matters everywhere these appear; the user can
# wrap a region in `// code-verify off` when the slow path is intentional
# (init code that builds a regex once, error path that throws, etc.).
_PERF_BODY_PATTERNS = [
    # `/ <floating-literal>` -- compilers do NOT fold `a / 2.5` to
    # `a * 0.4` without `-ffast-math` (would lose 1 ULP for non-exact
    # reciprocals). Multiplying by a precomputed reciprocal is ~3 cyc
    # vs ~12-22 cyc for divsd.
    (
        re.compile(
            r"(?<![*/=<>!&|^])/\s*(?:\d+\.\d*|\.\d+|\d+\.\d*[eE][+-]?\d+)" r"[fFlL]?"
        ),
        "perf-divide-by-float-literal",
        "`/` with a floating-point literal -- compilers don't fold to "
        "reciprocal multiply (would lose IEEE accuracy without -ffast-math). "
        "Precompute `constexpr double kInvX = 1.0 / X;` and multiply (~3 cyc "
        "mulsd vs ~12-22 cyc divsd).",
    ),
    # `pow(x, N)` -- libm transcendental, goes through `exp(log(x) * y)`.
    # 40+ cyc on Intel, similar on ARM. Caller-saved FPU/SIMD state gets
    # clobbered too.
    (
        re.compile(r"\b(?:std::)?pow\s*\("),
        "perf-pow-call",
        "`pow(...)` -- libm transcendental via `exp(log(x) * y)` (40+ cyc on "
        "Intel/ARM) and clobbers caller-saved FPU/SIMD state. For small "
        "integer exponents write the multiply (`x*x`, `x*x*x`); for "
        "`pow(x, 0.5)` use `std::sqrt(x)`; for `pow(2.0, n)` use "
        "`std::ldexp(1.0, n)` (single mantissa-shift insn).",
    ),
    # `dynamic_cast<T>` -- walks the inheritance graph via RTTI typeinfo
    # string comparisons; 50-200+ cyc worst case and a function call.
    (
        re.compile(r"\bdynamic_cast\s*<"),
        "perf-dynamic-cast",
        "`dynamic_cast<...>` -- walks the inheritance graph via RTTI typeinfo "
        "string compares (50-200+ cyc worst case, runtime call). Use a "
        "discriminating enum + `static_cast`, or pre-resolve the cast once "
        "(store the typed pointer at object init).",
    ),
    # malloc / free family -- same arena-mutex cost as `new`/`delete`,
    # just less visible. Both Linux glibc and Windows HeapAlloc serialize
    # on a per-arena mutex; on contended workloads this is a real cost.
    (
        re.compile(
            r"\b(?:malloc|calloc|realloc|free|aligned_alloc|posix_memalign)" r"\s*\("
        ),
        "perf-malloc-family",
        "C heap call -- malloc/free contend on a per-arena mutex (glibc, "
        "RtlHeap) and aren't pipelineable. In a hot loop, reuse a "
        "pre-reserved buffer or a small-object pool.",
    ),
    # `QRegularExpression(...)` constructor -- compiles the regex to a
    # state machine, heap-allocates capture tables. If invoked in a loop,
    # the regex gets recompiled every iteration.
    (
        re.compile(r"\bQRegularExpression\s*\([^)]"),
        "perf-regex-construct",
        "`QRegularExpression(...)` constructor -- compiles a DFA/NFA state "
        "machine and heap-allocates capture state. Build the regex once "
        "(file-scope `static const`, or a class member init) and reuse the "
        "`.match(...)` path each iteration.",
    ),
    # `.arg(...).arg(...)` chains -- each call returns a new QString
    # (heap alloc + copy). Two .arg()s = two allocs. Pass all args in
    # one call (`s.arg(a, b, c)`) or use QStringBuilder (`%` operator
    # with `<QStringBuilder>` included).
    (
        re.compile(r"\.arg\s*\([^()]*\)\s*\.arg\s*\("),
        "perf-arg-chain",
        "`.arg(...).arg(...)` chain -- each call allocates a fresh QString "
        "(heap + memcpy). Combine into one call (`.arg(a, b, c)`) or include "
        "`<QStringBuilder>` and use the `%` operator (single allocation, "
        "sized exactly).",
    ),
]


# Hotpath-only perf patterns: too noisy to flag file-wide in this codebase
# (qDebug and QString allocation are pervasive in setup/teardown/error
# paths and aren't wrong there). The hotpath methods listed in
# `_HOTPATH_METHODS` run at kHz+ rates -- THAT'S where the cost bites.
_HOTPATH_PERF_PATTERNS = [
    # QString / QByteArray construction with a literal -- each call hits
    # the heap allocator (malloc on Linux, RtlAllocateHeap on Windows),
    # contended on the arena mutex, not pipelineable. Cache the result
    # at init or hoist into a file-scope `static const`.
    #
    # `QStringLiteral("...")` is deliberately NOT flagged: by design it
    # constant-folds into a static read-only QString with zero heap touch
    # (that's why Qt has it). The other entries are the genuine heap-
    # allocating constructors/conversions.
    (
        re.compile(
            r"\bQString\s*\(\s*[\"R]"
            r"|\bQByteArray\s*\(\s*[\"R]"
            r"|\.toUtf8\s*\(\s*\)"
            r"|\.toStdString\s*\(\s*\)"
            r"|\.toLatin1\s*\(\s*\)"
            r"|\.toLocal8Bit\s*\(\s*\)"
            r"|\bQString::fromUtf8\s*\("
            r"|\bQString::fromLatin1\s*\("
        ),
        "perf-string-alloc-hotpath",
        "string construction/conversion on the hotpath -- heap allocation + "
        "memcpy. malloc contends on a per-arena mutex; the new buffer "
        "pollutes L1 (32-48 KB). Cache the QString at init, or use a "
        "fixed stack buffer for transient formatting.",
    ),
    # qDebug / qWarning -- builds a QDebug stream object, takes the global
    # message-handler mutex, formats and writes. Even filtered-out
    # categories pay the format cost because `<<` is eager. Hundreds of
    # cycles minimum per call; thousands when the handler dispatches to
    # a Console widget that re-enters the event loop.
    (
        re.compile(r"\bq(?:Debug|Info|Warning|Critical|Fatal)\s*\("),
        "perf-log-on-hotpath",
        "Qt logging call on the hotpath -- builds a QDebug stream, takes "
        "the global message-handler mutex, formats and writes. `<<` is "
        "eager: even filtered-out categories pay the format cost. Gate "
        "behind `#ifdef SERIAL_STUDIO_DEBUG` or move to a sampled counter.",
    ),
    # `throw` on the hotpath -- exception throw runs the personality "
    # routine, walks DWARF / SEH unwind tables (1000s of cycles per
    # frame), mispredicts every catch on the way out, trashes the
    # return-address stack. `noexcept` callers crash hard.
    (
        re.compile(r"\bthrow\s+\w"),
        "perf-throw-on-hotpath",
        "`throw` on the hotpath -- stack unwinding via DWARF/SEH personality "
        "routines (1000s of cycles), mispredicts every catch frame, trashes "
        "the return-address stack predictor. Return an error code, an "
        "`std::expected`-style variant, or a sentinel value instead.",
    ),
    # Mutex / lock-guard acquisition on the hotpath -- ~20 cyc lock-prefix
    # RMW on x86, ldaxr+stxr+DMB on ARM, serializes the store buffer, and
    # contended bouncing thrashes the L1 line. Outside the kHz frame path
    # the cost is irrelevant; locks are the right answer for once-per-event
    # state mutation. Only flag inside known-hot methods.
    (
        re.compile(
            r"\b(?:QMutexLocker|QReadLocker|QWriteLocker|QRecursiveMutex"
            r"|std::lock_guard|std::unique_lock|std::scoped_lock"
            r"|std::shared_lock)\b"
        ),
        "perf-lock-acquire",
        "lock acquisition on the hotpath -- atomic RMW with full memory "
        "barrier (~20 cyc x86 `lock`-prefix, ldaxr+stxr+DMB on ARM), "
        "serializes the store buffer; contended bouncing thrashes the L1 "
        "line. Prefer thread-local / SPSC / per-core state, or a relaxed "
        "`std::atomic` when the invariant fits a single word.",
    ),
    # Bare mutex.lock() / lockForRead() calls -- same physical cost.
    (
        re.compile(r"\b\w+\.(?:lock|try_lock|lockForRead|lockForWrite|tryLock)\s*\("),
        "perf-lock-acquire",
        "explicit `.lock()`/`.try_lock()`/`.lockForRead()` call on the "
        "hotpath -- same `lock`-prefix RMW cost as the locker types.",
    ),
    # Integer / float division by a non-literal divisor on the hotpath.
    # `idiv`/`udiv` is the slowest ALU op (20-40 cyc Skylake/Zen, not
    # pipelined; 12-40 cyc Cortex-A78). When the divisor is constexpr the
    # compiler emits a magic-number multiply; the hotpath cost only bites
    # when the divisor is a true runtime variable. `sizeof(...)` is
    # compile-time and skipped via lookahead. Reciprocal-cache lines
    # (`auto inv = 1.0 / x`) are skipped via _is_reciprocal_cache_line.
    (
        re.compile(r"(?<![*/=<>!&|^])/\s*(?!/)(?!sizeof\b)[A-Za-z_]\w*"),
        "perf-divide-runtime-divisor",
        "`/` with a non-literal divisor on the hotpath -- division is the "
        "slowest ALU op (divsd ~11-22 cyc Skylake, fdiv ~10-40 cyc Cortex-A78; "
        "idiv 20-40 cyc, not pipelined). Cache the reciprocal once "
        "(`r = 1.0 / d`) and multiply in the loop, or use a bit-shift for "
        "power-of-two integer cases.",
    ),
    # Modulo by a non-literal divisor on the hotpath. Same idiv cost as
    # integer divide; power-of-two N can be replaced with `& (N - 1)`.
    (
        re.compile(r"(?<![%=*/+\-<>!&|^])%\s*[A-Za-z_]\w*"),
        "perf-modulo-runtime-divisor",
        "`%` with a non-literal divisor on the hotpath -- emits `idiv`/`udiv` "
        "(20-40 cyc x86, 12-40 cyc ARM). For power-of-two N use `& (N - 1)` "
        "(single-cycle `and`); for runtime divisors hoist out of the loop or "
        "use a libdivide-style precomputed magic-number multiply.",
    ),
]


# Header line-pattern: a hotpath method declared `virtual`. Every call
# site emits an indirect branch through the vtable; the predictor learns
# monomorphic sites but can't inline, and polymorphic sites mispredict
# (15-20 cycle bubble on x86, similar on ARM). `final` partially helps
# when the dynamic type is known.
_VIRTUAL_HOTPATH_RE = re.compile(
    r"\bvirtual\b[^;{]*\b("
    + "|".join(sorted(re.escape(n) for n in _HOTPATH_METHODS))
    + r")\s*\("
)

# Generic atomic-type detector used by the false-sharing rule. Catches
# `std::atomic<T>`, `std::atomic_int`, `std::atomic_flag`, and the Qt
# `QAtomicInt`/`QAtomicPointer<T>`/`QAtomicInteger<T>` family.
_ATOMIC_DECL_RE = re.compile(
    r"\b(?:std::atomic(?:_[a-z0-9_]+)?\s*(?:<|\s+m?_?\w)"
    r"|std::atomic_flag\b"
    r"|QAtomic(?:Int|Pointer|Integer)\b)"
)

# Local fixed-size array declaration with a numeric size, e.g.
# `char buf[8192];`, `double samples[2048] = {};`.
_STACK_ARRAY_RE = re.compile(
    r"\b(?:char|signed\s+char|unsigned\s+char|int8_t|uint8_t|int|short|long"
    r"|size_t|ptrdiff_t|int16_t|int32_t|int64_t|uint16_t|uint32_t|uint64_t"
    r"|float|double|wchar_t|qint8|qint16|qint32|qint64|quint8|quint16"
    r"|quint32|quint64|qreal)\s+"
    r"(?:const\s+)?"
    r"\w+\s*\[\s*(\d+)\s*\]\s*[;={,]"
)


def _walk_to_function_declarator(decl):
    """Drill through pointer/reference declarators to the innermost
    function_declarator. Returns None when the chain doesn't lead to one."""
    seen = 0
    while decl is not None and seen < 16:
        if decl.type == "function_declarator":
            return decl
        nested = decl.child_by_field_name("declarator")
        if nested is None:
            return None
        decl = nested
        seen += 1
    return None


def _sink_param_names(func_node, src: bytes) -> set:
    """Return parameter names that this function treats as sinks.

    A sink parameter is one where pass-by-value + move is the right call
    instead of pass-by-const-ref:
      - The body (or a constructor's field initializer list) calls
        `std::move(<param>)`.
      - The body ends with `return <param>;` AFTER mutating it -- the
        param is the function's return value, so the implicit move on
        `return` makes the by-value form at least as cheap as
        `const T&` + explicit copy."""
    names: set = set()
    body = func_node.child_by_field_name("body")
    if body is not None:
        body_text = _node_text(body, src)
        for m in re.finditer(r"\bstd::move\s*\(\s*([A-Za-z_]\w*)\s*\)", body_text):
            names.add(m.group(1))
        # `return <name>;` at any point in the body: param flows out as
        # the return value, which the compiler implicitly moves from.
        for m in re.finditer(r"\breturn\s+([A-Za-z_]\w*)\s*;", body_text):
            names.add(m.group(1))
    for c in func_node.children:
        if c.type != "field_initializer_list":
            continue
        for m in re.finditer(
            r"\bstd::move\s*\(\s*([A-Za-z_]\w*)\s*\)", _node_text(c, src)
        ):
            names.add(m.group(1))
    return names


def _parameter_perf_findings(func_node, src: bytes, fenced) -> list:
    """Flag heavy types or refcounted smart pointers passed by value.
    Applies universally -- by-value copies are wasteful regardless of
    whether the function is in the hotpath list.

    Sink parameters (those `std::move`'d in the body) are skipped: the
    by-value + move idiom is correct C++ for a function that conditionally
    keeps a local copy of the argument."""
    findings: list = []
    decl = func_node.child_by_field_name("declarator")
    fdecl = _walk_to_function_declarator(decl)
    if fdecl is None:
        return findings
    params = fdecl.child_by_field_name("parameters")
    if params is None:
        return findings
    sink_params = _sink_param_names(func_node, src)
    for param in params.children:
        if param.type != "parameter_declaration":
            continue
        param_type = param.child_by_field_name("type")
        param_decl = param.child_by_field_name("declarator")
        if param_type is None:
            continue
        if param_decl is not None and param_decl.type in (
            "pointer_declarator",
            "reference_declarator",
            "abstract_pointer_declarator",
            "abstract_reference_declarator",
            "rvalue_reference_declarator",
            "abstract_rvalue_reference_declarator",
        ):
            continue
        # Resolve the parameter's identifier name (if it has one) so we
        # can suppress sink-param idioms.
        pname = None
        if param_decl is not None:
            cur = param_decl
            while cur is not None and cur.type != "identifier":
                next_cur = None
                for c in cur.children:
                    if c.type == "identifier":
                        next_cur = c
                        break
                if next_cur is None:
                    for c in cur.children:
                        if hasattr(c, "children") and c.children:
                            next_cur = c
                            break
                cur = next_cur
            if cur is not None and cur.type == "identifier":
                pname = _node_text(cur, src)
        if pname is not None and pname in sink_params:
            continue
        type_text = _node_text(param_type, src).strip()
        base = type_text
        for q in ("const ", "constexpr ", "volatile ", "mutable ", "register "):
            while base.startswith(q):
                base = base[len(q) :].lstrip()
        cuts = [i for i in (base.find("<"), base.find(" ")) if i >= 0]
        if cuts:
            base = base[: min(cuts)]
        line = _line_of(param)
        if fenced(line):
            continue
        if base in _HEAVY_TYPES:
            findings.append(
                Finding(
                    line,
                    "perf-large-by-value-param",
                    f"`{type_text}` passed by value -- forces a copy in the "
                    f"prologue (atomic ref-bump for Qt COW types: `lock`-prefix "
                    f"on x86, ldxr+stxr on ARM without LSE; full deep memcpy "
                    f"for std:: containers). Pass `const {base}&` and copy "
                    f"only when you genuinely keep a local copy.",
                )
            )
        elif base in _REFCOUNTED_TYPES:
            findings.append(
                Finding(
                    line,
                    "perf-shared-ptr-by-value",
                    f"`{type_text}` by value -- two atomic refcount ops per "
                    f"call (`lock add`/`lock sub` on x86, ~20 cyc each; "
                    f"ldxr/stxr loop on ARM without LSE/v8.1 atomics). "
                    f"Pass `const {base}<...>&` and copy only when you "
                    f"actually store the pointer.",
                )
            )
    return findings


def _init_only_decl_line_span(body, src: bytes) -> set:
    """Return the set of 1-based line numbers that belong to a declaration
    whose initializer runs at most once: `constexpr` (compile-time folded)
    or `static const`/`static constexpr` (function-local one-shot init).

    Runtime-cost rules (divide, modulo, regex-construct, ...) reason about
    the per-call cost of code inside a function body. These declarations
    are not on the per-call path -- the optimizer folds `constexpr` and the
    runtime evaluates `static const` exactly once -- so the rules must not
    fire on their initializer lines, no matter how many physical lines the
    initializer spans."""
    if body is None:
        return set()
    exempt: set = set()
    for node in _walk(body):
        if node.type != "declaration":
            continue
        specifiers = []
        for c in node.children:
            if c.type == "storage_class_specifier":
                specifiers.append(_node_text(c, src))
            elif c.type == "type_qualifier":
                specifiers.append(_node_text(c, src))
        spec_set = set(s.strip() for s in specifiers)
        is_constexpr = "constexpr" in spec_set
        is_static_const = "static" in spec_set and "const" in spec_set
        if not (is_constexpr or is_static_const):
            continue
        first = node.start_point[0] + 1
        last = node.end_point[0] + 1
        for ln in range(first, last + 1):
            exempt.add(ln)
    return exempt


def _cold_branch_line_span(body, src: bytes) -> set:
    """Return the set of 1-based line numbers inside cold-path branches:
    `[[unlikely]]`-attributed statements and `catch_clause` bodies.

    Both are reached only on error / overflow / exception, not on the
    steady-state hotpath that the perf rules are designed to flag.
    `qWarning(...)` inside an overflow branch or a catch block is correct
    code, not a hotpath log call."""
    if body is None:
        return set()
    exempt: set = set()
    for node in _walk(body):
        if node.type == "catch_clause":
            cs = node.child_by_field_name("body")
            if cs is None:
                for c in node.children:
                    if c.type == "compound_statement":
                        cs = c
                        break
            if cs is not None:
                for ln in range(cs.start_point[0] + 1, cs.end_point[0] + 2):
                    exempt.add(ln)
            continue
        if node.type == "attributed_statement":
            attr_text = _node_text(node, src)[:64]
            if "[[unlikely]]" not in attr_text and "[[gnu::unlikely]]" not in attr_text:
                continue
            for c in node.children:
                if c.type == "attribute_declaration":
                    continue
                for ln in range(c.start_point[0] + 1, c.end_point[0] + 2):
                    exempt.add(ln)
    return exempt


_RECIPROCAL_CACHE_RE = re.compile(r"\b1(?:\.0+f?|\.0+L?|\.0+|\b)\s*/\s*[A-Za-z_(]")
_DIV_OR_MOD_DIVISOR_RE = re.compile(
    r"(?<![*/=<>!&|^])[/%]\s*(?!/)(?!sizeof\b)([A-Za-z_]\w*)"
)


def _is_reciprocal_cache_line(scrubbed: str) -> bool:
    """True when the line is a reciprocal-cache declaration like
    `const float inv = 1.0f / x;` or `auto r = 1.0 / qMax(...);`.
    These are the rule's RECOMMENDED fix for runtime-divisor cost --
    cache the reciprocal once, multiply in the loop -- so flagging them
    is exactly backwards. We detect by the literal-1 numerator pattern."""
    return bool(_RECIPROCAL_CACHE_RE.search(scrubbed))


# Well-known math/system identifiers that resolve to compile-time constants
# even though they're macros (M_PI family) or constants the compiler
# substitutes via the standard library headers. Treating these as
# compile-time means the divisor / modulo rules don't fire on them.
_KNOWN_COMPILE_TIME_NAMES = frozenset(
    {
        "M_PI",
        "M_PI_2",
        "M_PI_4",
        "M_1_PI",
        "M_2_PI",
        "M_2_SQRTPI",
        "M_E",
        "M_LOG2E",
        "M_LOG10E",
        "M_LN2",
        "M_LN10",
        "M_SQRT2",
        "M_SQRT1_2",
        "INT8_MAX",
        "INT16_MAX",
        "INT32_MAX",
        "INT64_MAX",
        "UINT8_MAX",
        "UINT16_MAX",
        "UINT32_MAX",
        "UINT64_MAX",
        "CHAR_BIT",
        "CHAR_MAX",
        "CHAR_MIN",
    }
)


def _compile_time_constants_in_scope(body, src: bytes) -> set:
    """Walk the function body for `constexpr` declarations and return the
    set of their identifier names, plus a fixed set of well-known math/
    system macros that resolve to compile-time constants. The divisor /
    modulo rules can then skip lines whose divisor resolves to one of
    these -- the compiler folds them into a multiply-by-magic-number, no
    idiv at runtime."""
    if body is None:
        return set(_KNOWN_COMPILE_TIME_NAMES)
    names: set = set(_KNOWN_COMPILE_TIME_NAMES)
    for node in _walk(body):
        if node.type != "declaration":
            continue
        is_constexpr = any(
            c.type == "type_qualifier" and _node_text(c, src).strip() == "constexpr"
            for c in node.children
        )
        if not is_constexpr:
            continue
        for c in node.children:
            if c.type != "init_declarator":
                continue
            decl = c.child_by_field_name("declarator")
            if decl is None:
                continue
            cur = decl
            while cur is not None and cur.type != "identifier":
                next_cur = None
                for cc in cur.children:
                    if cc.type == "identifier":
                        next_cur = cc
                        break
                cur = next_cur
            if cur is not None and cur.type == "identifier":
                names.add(_node_text(cur, src))
    return names


def _scan_body_lines(body, src: bytes, fname: str, fenced, patterns) -> list:
    """Run a list of `(regex, kind, message)` triples over each line of a
    function body. First-match wins per line so a single problematic
    expression doesn't fire every pattern.

    Lines skipped (all driven by AST walks for multi-line statements):
      - `constexpr` / `static const` declarations -- init code, not per-call
      - `[[unlikely]]`-attributed substatement bodies -- cold path
      - `catch_clause` bodies -- error / exception path, not steady-state

    Lines skipped per-pattern: reciprocal-cache declarations
    (`const T inv = 1.0 / x;`) bypass the divide rule -- they ARE the
    recommended fix for runtime-divisor cost."""
    if body is None:
        return []
    findings: list = []
    body_text = _node_text(body, src)
    body_start = body.start_point[0] + 1
    exempt_lines = _init_only_decl_line_span(body, src) | _cold_branch_line_span(
        body, src
    )
    constexpr_names = _compile_time_constants_in_scope(body, src)
    for j, line in enumerate(body_text.split("\n")):
        abs_line = body_start + j
        if fenced(abs_line) or abs_line in exempt_lines:
            continue
        scrubbed = _strip_strings_and_line_comments(line)
        is_recip_cache = _is_reciprocal_cache_line(scrubbed)
        # If every divisor / modulo on this line resolves to a constexpr
        # name we know is in scope, the compiler folds them. Skip the
        # divide/modulo runtime rules for this line.
        divisors = _DIV_OR_MOD_DIVISOR_RE.findall(scrubbed)
        all_compile_time = bool(divisors) and all(
            d in constexpr_names for d in divisors
        )
        for pat, kind, msg in patterns:
            if is_recip_cache and kind == "perf-divide-runtime-divisor":
                continue
            if all_compile_time and kind in (
                "perf-divide-runtime-divisor",
                "perf-modulo-runtime-divisor",
            ):
                continue
            if pat.search(scrubbed):
                findings.append(Finding(abs_line, kind, msg))
                break
    return findings


def _recursion_findings(func_node, fname: str, body, src: bytes, fenced) -> list:
    """Flag direct self-recursion in a hotpath method. Recursion at kHz
    rates blows the i-cache (200+ cyc for an L2 miss), trashes the RAS
    predictor (mispredict on every return), and prevents inlining.

    Only flags **stack** recursion. The following are NOT recursion and
    are skipped:
      - Calls inside a `lambda_expression` body (deferred to whichever
        executor consumes the lambda; doesn't grow the stack here).
      - Qualified calls like `Base::fname(...)` or `Foo::fname(...)`
        (statically dispatched to a different function).
      - Method calls on a different object (`other.fname(...)`,
        `other->fname(...)`); only bare `fname(...)` and `this->fname(...)`
        are real self-calls."""
    if body is None or not fname or fname not in _HOTPATH_METHODS:
        return []
    findings: list = []
    seen_lines: set = set()
    for node in _walk(body):
        if node.type != "call_expression":
            continue
        # Skip calls that live inside a lambda body -- those execute when
        # the lambda runs, not on this call's stack frame.
        cur = node.parent
        in_lambda = False
        while cur is not None and cur is not body:
            if cur.type == "lambda_expression":
                in_lambda = True
                break
            cur = cur.parent
        if in_lambda:
            continue
        callee = node.child_by_field_name("function")
        if callee is None:
            continue
        # Recognise: `fname(...)` (identifier) or `this->fname(...)` (field_expression
        # whose object is `this`). Reject `Foo::fname`, `obj.fname`, `obj->fname`.
        is_self = False
        if callee.type == "identifier":
            is_self = _node_text(callee, src) == fname
        elif callee.type == "field_expression":
            obj = callee.child_by_field_name("argument")
            field = callee.child_by_field_name("field")
            if (
                obj is not None
                and field is not None
                and obj.type == "this"
                and _node_text(field, src) == fname
            ):
                is_self = True
        if not is_self:
            continue
        line = node.start_point[0] + 1
        if fenced(line) or line in seen_lines:
            continue
        seen_lines.add(line)
        findings.append(
            Finding(
                line,
                "perf-recursive-hotpath",
                f"hotpath `{fname}` calls itself -- recursion on a kHz "
                f"frame loop blows the i-cache (200+ cyc per L2 miss), "
                f"mispredicts the RAS on every return, and prevents the "
                f"compiler from inlining. Rewrite iteratively (explicit "
                f"work-list / std::stack).",
            )
        )
    return findings


_RUN_LOOP_COND_RE = re.compile(r"\bm_\w+\s*(?:\.load\s*\(|\)\s*\.\s*load\s*\()")


# Qt event-handler suffixes. These methods fire only on user input, geometry
# changes, focus changes, etc. -- cold paths by construction. Any per-call
# divide/modulo/pow inside them is irrelevant to the steady-state hotpath.
_QT_EVENT_HANDLER_SUFFIXES = (
    "Event",  # mousePressEvent, wheelEvent, keyPressEvent, paintEvent...
    "EventFilter",  # eventFilter override
    "ChangeEvent",  # geometryChange, focusChange, etc.
)


def _is_qt_event_handler(fname: str) -> bool:
    """True when @a fname matches Qt's event-handler naming convention.

    Qt's QObject / QWidget / QQuickItem event handlers all end in `Event`
    (`mousePressEvent`, `wheelEvent`, `paintEvent`, `geometryChange`,
    `eventFilter`, ...). They are dispatched once per user gesture or
    window event -- nowhere near the kHz frame rate the perf rules
    target."""
    if not fname:
        return False
    for suffix in _QT_EVENT_HANDLER_SUFFIXES:
        if fname.endswith(suffix) and len(fname) > len(suffix):
            return True
    return False


# Method-name patterns for the QQuickPaintedItem / QPainter render path. These
# functions are called at most at the screen refresh rate (~60 Hz) -- two
# orders of magnitude below the kHz frame loop the perf rules target.
_PAINT_METHOD_NAMES = frozenset(
    {
        "paint",
        "render",
    }
)
_PAINT_METHOD_PREFIXES = ("draw", "render", "paint")


def _is_paint_method(fname: str) -> bool:
    """True when @a fname is a paint / render method (`paint`, `paintEvent`,
    `drawXAxis`, `drawGrid`, `renderTile`, `paintBackground`, ...).

    Paint callbacks fire at the screen refresh rate at most. Compared to
    the kHz frame hotpath, the per-call cost of one or two divides is
    invisible. Locks, divides, etc. on these paths are not the rule's
    target."""
    if not fname:
        return False
    if fname in _PAINT_METHOD_NAMES:
        return True
    for prefix in _PAINT_METHOD_PREFIXES:
        if fname.startswith(prefix) and len(fname) > len(prefix):
            # Next char must be uppercase to avoid false positives like
            # `drained` or `paintbrush`.
            tail = fname[len(prefix)]
            if tail.isupper():
                return True
    return False


def _is_constexpr_or_consteval(func_node, src: bytes) -> bool:
    """True when the function carries `constexpr`, `consteval`, or `constinit`
    among its specifiers. Such functions are compile-time-evaluable (or are
    only meaningful at compile time), so runtime-cost rules don't apply to
    their bodies: any non-literal divisor / modulo / pow / etc. that survives
    constant folding does so because the function is being called with
    runtime arguments at a single specific site that the user is choosing
    to keep generic."""
    for c in func_node.children:
        if c.type in ("type_qualifier", "storage_class_specifier"):
            if _node_text(c, src).strip() in ("constexpr", "consteval", "constinit"):
                return True
    return False


def _is_long_running_loop_function(body, src: bytes) -> bool:
    """Heuristic: the function's body contains a `while`/`for`/`do` loop
    whose condition reads a member atomic flag (e.g. `m_running.load()`),
    at any nesting level inside the body. That's the canonical
    thread-entry / event-loop pattern in this codebase -- the function is
    called once per thread start, not per frame, so a 4 KB stack buffer
    in front of the loop amortizes.

    Descends through `preproc_ifdef` and similar nesting so a function
    body that's entirely wrapped in `#ifdef Q_OS_WIN` still matches."""
    if body is None:
        return False
    for node in _walk(body):
        if node.type not in ("while_statement", "for_statement", "do_statement"):
            continue
        cond = node.child_by_field_name("condition")
        if cond is None:
            for c in node.children:
                if c.type == "condition_clause" or c.type == "parenthesized_expression":
                    cond = c
                    break
        if cond is None:
            continue
        if _RUN_LOOP_COND_RE.search(_node_text(cond, src)):
            return True
    return False


def _large_stack_buffer_findings(body, src: bytes, fenced) -> list:
    """Flag local fixed-size arrays > ~4 KB. Stack-frame setup cost,
    pollutes L1 (32-48 KB) when the function recurses or is called in a
    hot loop with other state already on the stack, and on deep call
    paths risks overflow. The 1024-element threshold catches `double[512]`
    (4 KB), `int[1024]` (4 KB), `char[4096]` (4 KB) and similar.

    Functions that ARE the long-running loop (e.g. `pipeReadLoopWin`,
    detected by a top-level `while (m_*.load())`) are exempted: the stack
    frame is set up once per thread start, the buffer is reused every
    iteration, and the cost amortizes."""
    if body is None:
        return []
    if _is_long_running_loop_function(body, src):
        return []
    findings: list = []
    body_text = _node_text(body, src)
    body_start = body.start_point[0] + 1
    for j, line in enumerate(body_text.split("\n")):
        abs_line = body_start + j
        if fenced(abs_line):
            continue
        scrubbed = _strip_strings_and_line_comments(line)
        m = _STACK_ARRAY_RE.search(scrubbed)
        if m is None:
            continue
        try:
            n = int(m.group(1))
        except ValueError:
            continue
        if n < 1024:
            continue
        findings.append(
            Finding(
                abs_line,
                "perf-large-stack-buffer",
                f"local array of {n} elements on the stack -- frame-setup "
                f"cost, pollutes L1 (32-48 KB) when called in a hot loop, "
                f"risks overflow on deep call paths (Windows default 1 MB, "
                f"Linux 8 MB). Promote to a member, thread_local, or a "
                f"pre-reserved buffer.",
            )
        )
    return findings


def _adjacent_atomic_findings(class_node, src: bytes, fenced) -> list:
    """Two `std::atomic<>` (or QAtomic*) members within a few lines of
    each other almost certainly share a 64-byte cache line. When two
    cores write to atomics that share a line, MESI/MOESI invalidations
    bounce the line across cores -- a 50-200x slowdown vs the uncontended
    case (false sharing). The fix is `alignas(64)` (or
    `std::hardware_destructive_interference_size`) on each, or explicit
    `char _pad[64];` padding.

    Pointer-to-atomic fields are skipped: the pointer itself is set once
    at construction, and the actual atomics live behind the indirection
    in some other object whose layout we can't reason about from here."""
    findings: list = []
    body = class_node.child_by_field_name("body")
    if body is None:
        return findings
    prev_line = -100
    for child in body.children:
        if child.type != "field_declaration":
            continue
        text = _node_text(child, src)
        if not _ATOMIC_DECL_RE.search(text):
            prev_line = -100
            continue
        # Skip pointer-to-atomic and reference-to-atomic fields: the
        # atomic that would suffer false sharing lives elsewhere.
        if re.search(r"atomic\w*\s*(?:<[^>]*>)?\s*[*&]", text):
            prev_line = -100
            continue
        if "alignas" in text:
            prev_line = _line_of(child)
            continue
        line = _line_of(child)
        if not fenced(line) and 0 < line - prev_line <= 4:
            findings.append(
                Finding(
                    line,
                    "perf-false-sharing-risk",
                    "adjacent atomic members will share a cache line "
                    "(64 B Intel/AArch64, up to 128 B on Apple Silicon "
                    "M-series via the 128 B speculative line). Cross-core "
                    "writes thrash MESI/MOESI invalidations (50-200x slowdown "
                    "vs uncontended). Add `alignas(64)` / "
                    "`alignas(std::hardware_destructive_interference_size)` "
                    "or insert `char _pad[64 - sizeof(prev)];` between them.",
                )
            )
        prev_line = line
    return findings


def _virtual_hotpath_findings(src_text: str, path: Path, fenced) -> list:
    """Header line-scan: a hotpath method declared `virtual`. Every call
    site emits a vtable load + indirect branch (5-10 cyc best case, 15-20
    cyc misprediction penalty on polymorphic sites) and the compiler
    can't inline through it. If there's only one implementation, mark
    `final` (devirtualizes when the dynamic type is statically known)
    or drop `virtual` entirely.

    Skipped when the method is taken as a Qt member-function pointer
    (`&Class::method`) anywhere in the header or its paired `.cpp` --
    that means the method is dispatched through Qt's metacall (signal-slot
    or `QMetaObject::invokeMethod`), which can't be devirtualized regardless
    of `final`."""
    findings: list = []
    metacall_names = _qt_metacall_referenced_methods(src_text, path)
    for i, raw in enumerate(src_text.split("\n"), start=1):
        if fenced(i):
            continue
        scrubbed = _strip_strings_and_line_comments(raw)
        if "virtual" not in scrubbed:
            continue
        m = _VIRTUAL_HOTPATH_RE.search(scrubbed)
        if m is None:
            continue
        name = m.group(1)
        if name in metacall_names:
            continue
        findings.append(
            Finding(
                i,
                "perf-virtual-hotpath",
                f"hotpath method `{name}` declared `virtual` -- every call "
                f"site emits a vtable load + indirect branch (5-10 cyc best "
                f"case, 15-20 cyc misprediction on polymorphic sites). The "
                f"compiler can't inline through the indirect call. If only "
                f"one implementation exists, drop `virtual` or mark `final` "
                f"so the compiler can devirtualize.",
            )
        )
    return findings


_QT_METACALL_REF_RE = re.compile(r"&\s*\w+(?:\s*::\s*\w+)*\s*::\s*(\w+)\b")


def _qt_metacall_referenced_methods(src_text: str, path: Path) -> set:
    """Return the set of method names that appear as `&Class::method` in
    the given header AND in its paired `.cpp` (if any). Such references
    feed `connect(...)`, `QMetaObject::invokeMethod(...)`, or the Qt
    property system, all of which dispatch through `QMetaMethod::invoke`
    and can't be devirtualized -- so `virtual` on those methods isn't
    avoidable cost."""
    names: set = set()
    for m in _QT_METACALL_REF_RE.finditer(src_text):
        names.add(m.group(1))
    if path.suffix.lower() in (".h", ".hpp", ".hxx"):
        cpp = path.with_suffix(".cpp")
        if cpp.exists():
            try:
                cpp_text = cpp.read_text(encoding="utf-8", errors="replace")
                for m in _QT_METACALL_REF_RE.finditer(cpp_text):
                    names.add(m.group(1))
            except OSError:
                pass
    return names


# ---------------------------------------------------------------------------
# Hardcoded JSON keys (CLAUDE.md: use Keys::* namespace)
# ---------------------------------------------------------------------------

# Mirrors the inline constexpr declarations at the top of Frame.h. When one
# of these strings appears as a literal in a .cpp/.h that ALSO uses ss_jsr
# or QJsonObject, it's almost certainly a writer/reader bypassing Keys::*.
_PROJECT_KEY_LITERALS = frozenset(
    {
        "busType",
        "frameStart",
        "frameEnd",
        "frameDetection",
        "frameParserCode",
        "frameParserLanguage",
        "decoderMethod",
        "checksumAlgorithm",
        "hexadecimalDelimiters",
        "schemaVersion",
        "writerVersion",
        "writerVersionAtCreation",
        "sourceId",
        "datasetId",
        "uniqueId",
    }
)


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------


def _line_of(node) -> int:
    """1-based start line of a tree-sitter node."""
    return node.start_point[0] + 1


def _node_text(node, src: bytes) -> str:
    return src[node.start_byte : node.end_byte].decode("utf-8", errors="replace")


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
    text_before = src[max(0, node.start_byte - 200) : node.start_byte].decode(
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
        "if_statement",
        "for_statement",
        "for_range_loop",
        "while_statement",
        "do_statement",
        "switch_statement",
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
        if (
            not s
            or s.startswith("//")
            or s.startswith("template")
            or s.startswith("requires")
            or s.startswith("[[")
            or s.startswith("public:")
            or s.startswith("private:")
            or s.startswith("protected:")
        ):
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
    block = "\n".join(lines[open_idx : cur + 1])
    return "@brief" in block


def _previous_doxygen_block_range(node, src: bytes):
    """Return `(open_idx, close_idx)` 0-based line indices for the doxygen
    block immediately above `node`, or `None` when no block is present. The
    walk mirrors `_previous_doxygen_brief` but exposes the block range so a
    caller can inspect its body (verbose-brief detection)."""
    start_line = node.start_point[0]
    if start_line == 0:
        return None
    src_text = src.decode("utf-8", errors="replace")
    lines = src_text.split("\n")
    cur = start_line - 1
    skip = 0
    while cur >= 0 and skip < 8:
        s = lines[cur].strip()
        if (
            not s
            or s.startswith("//")
            or s.startswith("template")
            or s.startswith("requires")
            or s.startswith("[[")
            or s.startswith("public:")
            or s.startswith("private:")
            or s.startswith("protected:")
        ):
            cur -= 1
            skip += 1
            continue
        break
    if cur < 0:
        return None
    if not lines[cur].rstrip().endswith("*/"):
        return None
    open_idx = -1
    for j in range(cur, max(0, cur - 60) - 1, -1):
        if "/**" in lines[j]:
            open_idx = j
            break
    if open_idx < 0:
        return None
    return (open_idx, cur)


_VERBOSE_DOXY_TAGS = (
    "@param",
    "@return",
    "@returns",
    "@retval",
    "@throws",
    "@throw",
    "@exception",
    "@see",
    "@sa",
    "@note",
    "@warning",
    "@todo",
    "@since",
    "@deprecated",
    "@pre",
    "@post",
    "@invariant",
    "@tparam",
    "@details",
)


def _verbose_doxygen_reason(block_lines: list[str]) -> str:
    """Inspect a `/** ... */` doxygen block and return a short reason string
    when it is "verbose" per CLAUDE.md (one-line `@brief` is the contract).
    Returns `""` when the block is acceptable (one-line `/** @brief ... */`
    or a clean multi-line that's just a wrapped `@brief` sentence).

    Verbose forms (any one trips the rule):
    - Carries a doxygen tag other than `@brief` (`@param`, `@return`,
      `@note`, `@see`, `@throws`, `@tparam`, `@retval`, ...).
    - Contains a blank doxygen continuation line (` *` with no body), which
      separates an extended description paragraph from the brief.
    - Spans more than 6 source lines. The `cxx-inbody-comment` policy folds a
      load-bearing in-body "why" up INTO the brief, so a brief that absorbs one
      or two of those is legitimately 4-6 lines (`/**`, three or four ` * ...`
      content lines, ` */`). Beyond 6 the block has stopped being a brief and
      should move to the commit message. (Was 4: tightened back when briefs were
      one-liners; relaxed once the brief became the home for the in-body why.)
    """
    if not block_lines:
        return ""
    body = "\n".join(block_lines)
    if "@brief" not in body:
        return ""
    for tag in _VERBOSE_DOXY_TAGS:
        if tag in body:
            return f"contains `{tag}` -- one-line `@brief` is the contract"
    for raw in block_lines:
        if raw.strip() == "*":
            return (
                "contains a blank `*` continuation -- extended "
                "description belongs in the commit message, not the "
                "header doxygen"
            )
    if len(block_lines) > 6:
        return (
            f"spans {len(block_lines)} lines -- a brief that has grown past "
            "six lines belongs in the commit message, not the doxygen"
        )
    return ""


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
        cxx-while-loop           `while (cond)` (use bounded `for` instead)
        doc-missing-brief-cpp    .cpp function definition without /** @brief */
        doc-missing-brief-h      header type-level definition without /** @brief */
        doc-verbose-brief        doxygen block carries @param/@return/blank-`*`
                                 paragraphs or wraps past 6 lines (a brief may
                                 absorb an in-body why, but stays <=6 lines)
        cxx-inbody-comment       comment inside a function body (functions are
                                 short; the @brief above + the code carry it)
        cxx-trivial-function     function with an empty / only-Q_UNUSED / single
                                 constant-return body (dead-code candidate)
        cxx-scattered-constant   file-scope static/constexpr/const declared after
                                 the first function (hoist to the top section)
        hotpath-allocation       allocation/append on a known hotpath method
        keys-hardcoded-literal   raw "busType" etc. literal where Keys:: belongs
        cxx-anonymous-namespace  helpers/types defined inside `namespace { ... }`
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
            out.append(
                Finding(i, "qt-bare-emit", "use `Q_EMIT` instead of bare `emit`")
            )
        if re.search(r"\bQ_SIGNALS\s*:", scrubbed):
            out.append(
                Finding(
                    i,
                    "qt-uppercase-signal-slot",
                    "use lowercase `signals:` (CLAUDE.md)",
                )
            )
        if re.search(r"\b(?:public|protected|private)?\s*Q_SLOTS\s*:", scrubbed):
            out.append(
                Finding(
                    i, "qt-uppercase-signal-slot", "use lowercase `slots:` (CLAUDE.md)"
                )
            )
        if re.search(r"\bgoto\b", scrubbed):
            out.append(
                Finding(
                    i, "cxx-goto-or-jmp", "`goto` is banned (NASA Power of Ten rule 1)"
                )
            )
        if re.search(r"\b(?:setjmp|longjmp)\s*\(", scrubbed):
            out.append(
                Finding(
                    i,
                    "cxx-goto-or-jmp",
                    "`setjmp`/`longjmp` are banned (NASA P10 rule 1)",
                )
            )
        # `while (...)` loop opener at the start of a stripped line. The
        # leading `^\s*` anchor excludes the trailing `} while (cond);` of
        # a do-while construct, which is allowed because the body always
        # runs at least once and bounded loops use this form. Bare `while`
        # is risky because the bound is implicit: nothing in the syntax
        # forces the author to articulate a max iteration count, which is
        # exactly the kind of bug that hid the WindowManager::tileGrid
        # infinite loop (int overflow turned `cols * rows < n` permanently
        # true and froze the GUI thread). Rewrite as
        # `for (int guard = 0; guard < kMaxIterations && cond; ++guard)`
        # so the bound is articulated and code review can audit it. Suppress
        # with `// code-verify off` when the bound is non-obvious but
        # provable by other invariants.
        #
        # Canonical bounded forms are whitelisted because flagging them
        # produces ~98% false-positive churn -- see the catalog in
        # memory/project_while_loop_patterns.md. The fire-anyway shape is
        # the WindowManager::tileGrid one: `cond` only references state
        # mutated by side effects in the body, with no visible iteration
        # cap, queue/iterator drain, atomic flag, or size/length bound.
        if re.match(r"^\s*while\s*\(", scrubbed):
            cond_text = _gather_while_condition(lines, i - 1)
            if cond_text is not None and not _while_condition_is_bounded(cond_text):
                out.append(
                    Finding(
                        i,
                        "cxx-while-loop",
                        "`while (...)` has no syntactically-visible upper bound -- "
                        "prefer `for (int i = 0; i < kMax && cond; ++i)` "
                        "(NASA P10 rule 2). Suppress with `// code-verify off` "
                        "when the bound is provably finite (queue drain, fixed "
                        "iterator walk, etc).",
                    )
                )

    # Q_INVOKABLE void on the same source line is unambiguous; spread cases
    # are rare enough that we accept the false-negative.
    invokable_void_re = re.compile(r"\bQ_INVOKABLE\b\s+void\b")
    for i, raw in enumerate(lines, start=1):
        if fenced(i):
            continue
        scrubbed = _strip_strings_and_line_comments(raw)
        if invokable_void_re.search(scrubbed):
            out.append(
                Finding(
                    i,
                    "qt-invokable-void",
                    "`Q_INVOKABLE void` -- move to `public slots:`",
                )
            )

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
        if re.search(r"\bdisconnect\s*\(\s*[^,()]+\s*,\s*nullptr\s*\)", scrubbed):
            out.append(
                Finding(
                    i,
                    "qt-disconnect-nullptr",
                    "`disconnect(<conn>, nullptr)` disconnects ALL slots from the "
                    "connection; capture the QMetaObject::Connection and disconnect that",
                )
            )

    # parseFunction.call(...) is the JS engine path that must go through
    # IScriptEngine::guardedCall for the runtime watchdog.
    for i, raw in enumerate(lines, start=1):
        if fenced(i):
            continue
        scrubbed = _strip_strings_and_line_comments(raw)
        if re.search(r"\bparseFunction\.call\s*\(", scrubbed):
            out.append(
                Finding(
                    i,
                    "qt-direct-jsengine-call",
                    "`parseFunction.call()` bypasses the runtime watchdog -- "
                    "route through `IScriptEngine::guardedCall()`",
                )
            )

    # ---- AST-driven rules
    # Walk every function_definition: function-too-long, nesting-too-deep,
    # missing @brief in .cpp, hotpath allocations, trivial dead-function bodies.
    contract_names = _header_contract_names(path, src_text, is_header)
    # Functions defined more than once in this translation unit are build
    # variants (one body per `#ifdef` branch); a trivial stub in one branch is
    # expected, not dead code. Count top-level definitions by name.
    func_name_counts: dict = {}
    for fn in _walk(root):
        if fn.type == "function_definition" and not _has_function_ancestor(fn):
            nm = _function_name(fn, src)
            if nm:
                func_name_counts[nm] = func_name_counts.get(nm, 0) + 1
    is_platform_file = bool(_PLATFORM_FILE_RE.search(path.name))
    for n in _walk(root):
        if n.type == "function_definition":
            line = _line_of(n)
            if fenced(line):
                continue
            body = n.child_by_field_name("body")
            length = _function_body_lines(body)
            if length > 100:
                out.append(
                    Finding(
                        line,
                        "cxx-function-too-long",
                        f"function spans {length} lines (>100; "
                        f"NASA P10 rule 4 caps at 100)",
                    )
                )
            depth = _max_nesting_depth(body)
            if depth > 3:
                out.append(
                    Finding(
                        line,
                        "cxx-nesting-too-deep",
                        f"control-flow nesting depth {depth} (>3; "
                        f"CLAUDE.md caps at 3)",
                    )
                )

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
                out.append(
                    Finding(
                        line,
                        "doc-missing-brief-cpp",
                        f"`{fname}` lacks a preceding `/** @brief ... */`",
                    )
                )

            # Verbose doxygen above a function definition. Applies to both
            # `.cpp` definitions and inline-defined methods in headers; the
            # contract per CLAUDE.md is a one-line `/** @brief ... */`.
            if not _has_function_ancestor(n):
                fname = _function_name(n, src)
                if fname:
                    rng = _previous_doxygen_block_range(n, src)
                    if rng is not None:
                        open_idx, close_idx = rng
                        block_lines = src.decode("utf-8", errors="replace").split("\n")[
                            open_idx : close_idx + 1
                        ]
                        reason = _verbose_doxygen_reason(block_lines)
                        if reason and not fenced(open_idx + 1):
                            out.append(
                                Finding(
                                    open_idx + 1,
                                    "doc-verbose-brief",
                                    f"verbose doxygen above `{fname}`: {reason}",
                                )
                            )

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
                            out.append(
                                Finding(
                                    abs_line,
                                    "hotpath-allocation",
                                    f"{msg} (in `{fname}`)",
                                )
                            )
                            break

            # CPU-microarchitecture / perf rules. _PERF_BODY_PATTERNS runs on
            # every function body (file-wide); _HOTPATH_PERF_PATTERNS adds
            # hotpath-only checks for patterns that would be too noisy across
            # the whole codebase (qDebug, QString allocation).
            #
            # `constexpr` / `consteval` functions are compile-time-evaluable
            # so runtime-cost reasoning doesn't apply to their bodies. Qt
            # event handlers (`*Event`, `eventFilter`, ...) and paint
            # callbacks (`paint`, `drawXxx`, `renderXxx`, ...) are bound
            # to the screen refresh rate -- two orders of magnitude below
            # the kHz frame hotpath -- so per-call divide/modulo cost
            # there is below the noise floor.
            if (
                not _is_constexpr_or_consteval(n, src)
                and not _is_qt_event_handler(fname)
                and not _is_paint_method(fname)
            ):
                out.extend(
                    _scan_body_lines(body, src, fname, fenced, _PERF_BODY_PATTERNS)
                )
                if fname in _HOTPATH_METHODS:
                    out.extend(
                        _scan_body_lines(
                            body, src, fname, fenced, _HOTPATH_PERF_PATTERNS
                        )
                    )
                    out.extend(_recursion_findings(n, fname, body, src, fenced))

            # Universal: function-signature scan for heavy/refcounted types
            # passed by value, and body scan for oversized stack arrays.
            out.extend(_parameter_perf_findings(n, src, fenced))
            out.extend(_large_stack_buffer_findings(body, src, fenced))

            # Trivial / dead-function candidate: empty body, only Q_UNUSED, or a
            # single constant return. Interface overrides (header `virtual` /
            # `override` / `final`), operators and `main` are excluded.
            if (
                not _has_function_ancestor(n)
                and not fenced(line)
                and not is_platform_file
            ):
                fname = _function_name(n, src)
                if (
                    fname
                    and fname != "main"
                    and not fname.startswith("operator")
                    and fname not in contract_names
                    and fname not in _CPP_NON_FUNCTION_NAMES
                    and not _DEFAULT_PROVIDER_RE.match(fname)
                    and func_name_counts.get(fname, 1) <= 1
                ):
                    reason = _function_trivial_reason(n, src)
                    if reason:
                        out.append(
                            Finding(
                                line,
                                "cxx-trivial-function",
                                f"`{fname}` has a trivial body ({reason}) -- likely "
                                f"dead. Interface overrides are already excluded; "
                                f"confirm no QML binding, Q_INVOKABLE/Q_PROPERTY use, "
                                f"or call site depends on it, then delete the function "
                                f"with its declaration and call sites. Not auto-removed "
                                f"-- removal ripples into headers, properties, and QML.",
                            )
                        )

    # ---- In-body comments. Every function here is <=100 lines (the
    # cxx-function-too-long cap), so the contract is one `/** @brief ... */`
    # above the function plus self-explanatory code -- no comments INSIDE the
    # body. The @brief sits outside the body node, so it is never flagged
    # (see _comment_in_function_body). Tooling pragmas (clang-format / NOLINT /
    # cppcheck-suppress / fallthrough) are directives and skipped; a genuinely
    # load-bearing note can stay behind a reviewed `// code-verify off` fence.
    for n in _walk(root):
        if n.type != "comment":
            continue
        if not _comment_in_function_body(n):
            continue
        line = _line_of(n)
        if fenced(line):
            continue
        if _is_inbody_pragma(_node_text(n, src)):
            continue
        out.append(
            Finding(
                line,
                "cxx-inbody-comment",
                "comment inside a function body -- functions here are short "
                "(<=100 lines) so the code reads on its own. Delete the "
                "comment, or fold a load-bearing 'why' into the one-line "
                "`/** @brief ... */` above the function. A genuinely-needed "
                "note can stay behind a reviewed `// code-verify off` fence.",
            )
        )

    # ---- Scattered file-scope constants/globals. Anything `static` /
    # `constexpr` / `const` declared at file or namespace scope AFTER the first
    # function definition should be hoisted into the top `// Constants` banner
    # so every constant lives in one place, not spread across the file.
    func_starts = [
        n.start_point[0]
        for n in _walk(root)
        if n.type == "function_definition" and not _has_function_ancestor(n)
    ]
    if path.suffix in _IMPL_SUFFIXES and func_starts:
        first_func_line = min(func_starts)
        for n in _walk(root):
            if n.type != "declaration" or n.start_point[0] <= first_func_line:
                continue
            if not _is_file_scope_decl(n) or not _is_constant_global_decl(n, src):
                continue
            line = _line_of(n)
            if fenced(line):
                continue
            name = _decl_first_name(n, src)
            if not _CONSTANT_NAME_RE.match(name):
                continue
            out.append(
                Finding(
                    line,
                    "cxx-scattered-constant",
                    f"file-scope constant/global `{name}` is declared after the "
                    f"first function (line {first_func_line + 1}) -- hoist it into "
                    f"the `// Constants` banner at the top of the file so every "
                    f"constant lives in one section, not scattered across the file.",
                )
            )

    # ---- Anonymous-namespace helpers (.cpp only). Anonymous namespaces hide
    # symbols from the linker but also from grep, doxygen, IDE call hierarchies,
    # and any reader trying to trace where a helper lives. CLAUDE.md prefers
    # named-namespace statics or class-private statics; flag every helper /
    # variable / type defined inside `namespace { ... }`. Headers don't get
    # this rule because anon namespaces in headers are a separate (worse) bug
    # caught by other tools.
    if not is_header:
        for n in _walk(root):
            if n.type != "namespace_definition":
                continue
            if not _is_anonymous_namespace(n):
                continue
            body = n.child_by_field_name("body")
            if body is None:
                continue
            for child in body.children:
                if child.type not in (
                    "function_definition",
                    "declaration",
                    "template_declaration",
                    "class_specifier",
                    "struct_specifier",
                    "enum_specifier",
                    "type_definition",
                    "alias_declaration",
                    "union_specifier",
                ):
                    continue
                line = _line_of(child)
                if fenced(line):
                    continue
                label = _anon_member_label(child, src)
                out.append(
                    Finding(
                        line,
                        "cxx-anonymous-namespace",
                        f"`{label}` defined inside an anonymous namespace -- "
                        f"hard to trace; prefer a class-private `static` "
                        f"(default), file-scope `static` for free helpers, or "
                        f"a named `detail` namespace for TU-private types",
                    )
                )

    # ---- Header-only: function doxygen blocks above non-type declarations.
    # CLAUDE.md "Headers (.h) -- strict rule": the only block-doc allowed in
    # a header is `/** @brief ... */` above a TYPE-LEVEL definition. Function
    # doxygen blocks above member-function decls are explicitly forbidden
    # ("No function doxygen, member-variable comments, signal/slot comments,
    # @param/@return/@note/@see, or inline `//`. Names + types are the
    # documentation."). The existing tools track type-level coverage; this
    # check finds the opposite: blocks that should be deleted.
    if is_header:
        src_text2 = src.decode("utf-8", errors="replace")
        lines2 = src_text2.split("\n")
        for n in _walk(root):
            if n.type != "field_declaration":
                continue
            line = _line_of(n)
            if fenced(line):
                continue
            decl = n.child_by_field_name("declarator")
            if decl is None:
                continue
            # Only complain when the declarator is a function declarator
            # (we don't want to flag plain member-variable doxygen here --
            # `qt-header-member-init` covers that for QObject classes, and
            # POD config-bag fields are intentionally exempt).
            if decl.type != "function_declarator":
                continue
            # Skip inline-defined methods -- those are function bodies, and
            # the doc-missing-brief-cpp rule handles their doxygen.
            #
            # An inline-defined method's `field_declaration` carries its body
            # as a `compound_statement` child of the function_declarator's
            # parent. Detect by walking the field_declaration for any
            # compound_statement descendant that's a body, not an init list.
            has_body = False
            for c in n.children:
                if c.type == "function_definition":
                    has_body = True
                    break
            if has_body:
                continue
            # Walk back from `line - 1` to find a closing `*/`. If found, find
            # its matching `/**` and check whether the block is multi-line.
            # A one-line `/** @brief ... */` block above a member function
            # is also banned but we report it the same way -- the message
            # tells the reader to delete the block.
            cur = line - 2
            skip = 0
            while cur >= 0 and skip < 6:
                s = lines2[cur].strip()
                if (
                    not s
                    or s.startswith("//")
                    or s.startswith("[[")
                    or s.startswith("template")
                    or s.startswith("requires")
                ):
                    cur -= 1
                    skip += 1
                    continue
                break
            if cur < 0:
                continue
            if not lines2[cur].rstrip().endswith("*/"):
                continue
            # Walk further back to find the opening `/**`.
            open_idx = -1
            for j in range(cur, max(0, cur - 60) - 1, -1):
                if "/**" in lines2[j]:
                    open_idx = j
                    break
            if open_idx < 0:
                continue
            block = "\n".join(lines2[open_idx : cur + 1])
            # If the block is just `/** @brief ... */` on one line above a
            # type-level def, the doc-missing-brief-h rule is happy. But
            # if it's above a function declaration, CLAUDE.md still says it
            # shouldn't be there. Report it with a kind that maps to advisory
            # (heuristic only, broad existing-code debt).
            out.append(
                Finding(
                    open_idx + 1,
                    "doc-header-function-block",
                    "header function declaration carries a doxygen block -- "
                    "headers should hold ONLY @brief banners above type-level "
                    'definitions; delete the block (CLAUDE.md "Headers (.h) -- '
                    'strict rule")',
                )
            )

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
                out.append(
                    Finding(
                        line,
                        "qt-header-member-init",
                        f"in-header default init `{last_ident}` -- "
                        f"initialize in the constructor member init list",
                    )
                )

        # Type-level @brief: every class/struct/enum/typedef/using-alias at
        # namespace scope needs a /** @brief */ banner, per CLAUDE.md.
        for n in _walk(root):
            if n.type not in {
                "class_specifier",
                "struct_specifier",
                "enum_specifier",
                "type_definition",
                "alias_declaration",
            }:
                continue
            # Skip nested types (parent is field_declaration_list / class
            # body) -- the enclosing class's @brief covers them.
            if _is_nested_type(n):
                continue
            # Skip forward declarations (no body / definition).
            if n.type in ("class_specifier", "struct_specifier", "enum_specifier"):
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
                    out.append(
                        Finding(
                            line,
                            "doc-missing-brief-h",
                            f"`{name}` lacks a preceding `/** @brief ... */`",
                        )
                    )
            else:
                # Verbose @brief on a type-level definition. Same rule:
                # one-line `/** @brief ... */`, no `@param`/`@note`/blank-`*`
                # paragraph splits / 7+ line prose.
                name = _type_name(n, src)
                if name:
                    rng = _previous_doxygen_block_range(n, src)
                    if rng is not None:
                        open_idx, close_idx = rng
                        block_lines = src.decode("utf-8", errors="replace").split("\n")[
                            open_idx : close_idx + 1
                        ]
                        reason = _verbose_doxygen_reason(block_lines)
                        if reason and not fenced(open_idx + 1):
                            out.append(
                                Finding(
                                    open_idx + 1,
                                    "doc-verbose-brief",
                                    f"verbose doxygen above `{name}`: {reason}",
                                )
                            )

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
            out.append(
                Finding(
                    line,
                    "qt-missing-nodiscard",
                    f"non-void const getter `{name}` lacks `[[nodiscard]]`",
                )
            )

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
                        out.append(
                            Finding(
                                i,
                                "keys-hardcoded-literal",
                                f'hardcoded JSON key `"{m.group(1)}"` -- '
                                f"use `Keys::{_camel(m.group(1))}` from Frame.h",
                            )
                        )
                        break  # one finding per line is plenty

    # ---- False-sharing risk: adjacent std::atomic / QAtomic members in a
    # class body. Applies to both headers and inline class definitions in
    # .cpp files.
    for n in _walk(root):
        if n.type not in ("class_specifier", "struct_specifier"):
            continue
        cls_line = _line_of(n)
        if fenced(cls_line):
            continue
        out.extend(_adjacent_atomic_findings(n, src, fenced))

    # ---- Hotpath methods declared virtual. Header-only; the declaration
    # site is where `virtual` is written. Inline-defined methods inside a
    # class body in a .cpp would also match if this pattern ever appears
    # there, which is rare.
    if is_header:
        out.extend(_virtual_hotpath_findings(src_text, path, fenced))

    return out


def _function_name(func_node, src: bytes) -> str:
    """Return the function's name from a function_definition node, or "".
    Walks declarator -> identifier / qualified_identifier / field_identifier.
    For nested `qualified_identifier` (e.g. `A::B::C::foo`), the rightmost
    segment is the actual function name."""
    decl = func_node.child_by_field_name("declarator")
    while decl is not None and decl.type == "function_declarator":
        decl = decl.child_by_field_name("declarator")
    if decl is None:
        return ""

    # Walk to the rightmost segment of any qualified-identifier chain.
    while decl is not None and decl.type == "qualified_identifier":
        nested = None
        for child in decl.children:
            if child.type in (
                "identifier",
                "field_identifier",
                "destructor_name",
                "qualified_identifier",
                "operator_name",
                "template_function",
            ):
                nested = child
        if nested is None:
            return ""
        decl = nested

    if decl.type in ("identifier", "field_identifier", "destructor_name"):
        return _node_text(decl, src)
    if decl.type == "operator_name":
        return _node_text(decl, src)
    if decl.type == "template_function":
        name = decl.child_by_field_name("name")
        if name is not None:
            return _node_text(name, src)
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


def _comment_in_function_body(node) -> bool:
    """True when a `comment` node sits INSIDE a function body or a
    constructor's member-initializer list.

    The discriminator against a function's own `/** @brief */` is that an
    in-body comment's ancestor chain passes through a `compound_statement`
    (the `{ ... }` body) or a `field_initializer_list` before reaching the
    enclosing `function_definition`. The doxygen banner above a definition
    is a sibling of that definition at namespace / class scope, so it never
    passes through the body and is never flagged. Verified empirically: on
    Taskbar.cpp this matches the body-range walk exactly (112/112) and skips
    every leading `@brief` block."""
    parent = node.parent
    seen = 0
    passed_body = False
    while parent is not None and seen < 64:
        if parent.type in ("compound_statement", "field_initializer_list"):
            passed_body = True
        if parent.type == "function_definition":
            return passed_body
        parent = parent.parent
        seen += 1
    return False


# ---------------------------------------------------------------------------
# Trivial / dead-function detection (cxx-trivial-function)
# ---------------------------------------------------------------------------
#
# Flags a function whose entire body is dead weight: empty (`void onX() {}`),
# only `Q_UNUSED(...)` no-ops, or a single constant return
# (`bool foo() { return true; }`, `QVariantList groupModel() { return {}; }`).
# These are candidates for removal -- advisory, because deleting one ripples
# into its declaration, any `Q_PROPERTY` block, QML bindings, and call sites,
# so it stays a human-approved step, not an auto-fix.
#
# Interface contracts are excluded: a method declared `virtual` / `override` /
# `final` in the (paired) header returns its constant by design -- the base
# type requires the override, and dropping it changes behavior rather than
# removing dead code. Constructors, destructors and conversion operators (no
# return type) are excluded too -- an empty ctor body is normal.

_TRIVIAL_RETURN_LITERAL_TYPES = frozenset(
    {
        "true",
        "false",
        "null",
        "nullptr",
        "number_literal",
        "string_literal",
        "char_literal",
        "concatenated_string",
        "user_defined_literal",
    }
)

# C++ keywords that tree-sitter occasionally surfaces as a `_function_name`
# when a macro-without-semicolon (`Q_OBJECT`, `Q_GADGET`) inside a class body
# derails the parse and the access-specifier / class header gets mistaken for
# a definition (e.g. `class R : public QObject { Q_OBJECT ... }` yields a
# phantom function named `public`). Never a real function name.
_CPP_NON_FUNCTION_NAMES = frozenset(
    {
        "public",
        "private",
        "protected",
        "class",
        "struct",
        "union",
        "enum",
        "namespace",
        "template",
        "typename",
        "return",
        "if",
        "else",
        "for",
        "while",
        "do",
        "switch",
        "case",
        "default",
        "try",
        "catch",
        "const",
        "constexpr",
        "static",
        "inline",
        "virtual",
        "explicit",
        "friend",
        "using",
        "typedef",
    }
)

_OVERRIDE_DECL_RE = re.compile(
    r"\b([A-Za-z_]\w*)\s*\([^;{]*?\)\s*(?:const\b\s*)?(?:noexcept\b[^;{]*?)?"
    r"(?:->[^;{]*?)?(?:\boverride\b|\bfinal\b)"
)
_VIRTUAL_DECL_RE = re.compile(r"\bvirtual\b[^;{=]*?\b([A-Za-z_]\w*)\s*\(")
# Q_PROPERTY READ/WRITE/RESET/MEMBER targets: a method/member the property
# system reads is QML-exposed. A trivial constant getter behind a Q_PROPERTY
# is the property's value (a capability flag, a fixed default), not dead code.
_QPROP_TARGET_RE = re.compile(r"\b(?:READ|WRITE|RESET|MEMBER)\s+([A-Za-z_]\w*)")
# Files holding platform-specific variants of a method: a trivial / empty body
# is the expected no-op stub for the platforms where the real implementation
# lives elsewhere (e.g. NativeWindow_CSD.cpp's stub vs NativeWindow_macOS.mm).
_PLATFORM_FILE_RE = re.compile(
    r"_(?:macOS|mac|cocoa|CSD|windows|win|winrt|linux|unix|x11|wayland"
    r"|android|ios|qnx|generic|stub)\.(?:cpp|mm|cc|cxx)$",
    re.IGNORECASE,
)
# A `defaultFoo()` accessor returns a fixed default value by convention; that
# is its whole job, not dead code.
_DEFAULT_PROVIDER_RE = re.compile(r"^default[A-Z0-9]")
_header_contract_cache: dict = {}


def _contract_names_from_text(text: str) -> set:
    """Names a trivial body is legitimate on: declared `virtual` / `override` /
    `final` (interface contract), or a `Q_PROPERTY` READ/WRITE/RESET/MEMBER
    target (QML-exposed -- the constant IS the property value, not dead code)."""
    names: set = set()
    for m in _OVERRIDE_DECL_RE.finditer(text):
        names.add(m.group(1))
    for m in _VIRTUAL_DECL_RE.finditer(text):
        names.add(m.group(1))
    for m in _QPROP_TARGET_RE.finditer(text):
        names.add(m.group(1))
    return names


def _header_contract_names(path, src_text: str, is_header: bool) -> set:
    """Override/virtual/final method names relevant to `path`: always the ones
    declared in the file itself -- this catches classes defined inline in a
    `.cpp` (e.g. the NativeTemplate implementations, whose `id()`/`name()`
    overrides live in the `.cpp`, not a header) -- unioned, for a `.cpp`, with
    those in the paired `.h`/`.hpp`/`.hxx`. A trivial body on one of these is
    an interface contract, not dead code."""
    names = _contract_names_from_text(src_text)
    if is_header:
        return names
    for suffix in (".h", ".hpp", ".hxx"):
        cand = path.with_suffix(suffix)
        if not cand.exists():
            continue
        key = str(cand)
        cached = _header_contract_cache.get(key)
        if cached is None:
            try:
                text = cand.read_text(encoding="utf-8", errors="replace")
            except OSError:
                text = ""
            cached = _contract_names_from_text(text)
            _header_contract_cache[key] = cached
        names = names | cached
        break
    return names


def _is_constant_return_expr(node, src: bytes) -> bool:
    """True when a return expression is a compile-time / default constant: a
    literal, `nullptr`, an empty `{}`, a zero-arg construction of a Type
    (`QVariantList()`), or a string-literal macro (`QStringLiteral("x")`)."""
    t = node.type
    if t in _TRIVIAL_RETURN_LITERAL_TYPES:
        return True
    if t == "initializer_list":
        return all(c.type in ("{", "}", ",") for c in node.children)
    if t == "unary_expression":
        arg = node.child_by_field_name("argument")
        return arg is not None and arg.type == "number_literal"
    if t == "call_expression":
        fn = node.child_by_field_name("function")
        args = node.child_by_field_name("arguments")
        if fn is None or args is None:
            return False
        arg_nodes = [c for c in args.children if c.type not in ("(", ")", ",")]
        fn_text = _node_text(fn, src)
        # Zero-arg construction of a CamelCase Type (`QVariantList()`,
        # `QString()`). Require a lowercase letter so ALL_CAPS macros
        # (`SS_LICENSE_GUARD()`, `Q_NULLPTR`) aren't mistaken for a default
        # value -- a macro can expand to anything, including a real check.
        if (
            not arg_nodes
            and fn.type == "identifier"
            and fn_text[:1].isupper()
            and any(c.islower() for c in fn_text)
        ):
            return True
        if fn_text in ("QStringLiteral", "QByteArrayLiteral", "QLatin1String"):
            return bool(arg_nodes) and all(
                a.type in _TRIVIAL_RETURN_LITERAL_TYPES for a in arg_nodes
            )
    return False


def _function_trivial_reason(func_node, src: bytes) -> str:
    """Return a short reason when @p func_node's body is trivial dead weight
    (empty, only Q_UNUSED no-ops, or a single constant return), else "".

    Constructors / destructors / conversion operators (no return type) return
    "" -- an empty ctor body is not dead code. The caller additionally skips
    interface overrides, operators, and `main`."""
    if func_node.child_by_field_name("type") is None:
        return ""
    body = func_node.child_by_field_name("body")
    if body is None or body.type != "compound_statement":
        return ""

    meaningful = [c for c in body.children if c.type not in ("{", "}", "comment")]
    if not meaningful:
        return "empty body"
    if all(_node_text(c, src).lstrip().startswith("Q_UNUSED") for c in meaningful):
        return "only Q_UNUSED no-ops"
    if len(meaningful) != 1 or meaningful[0].type != "return_statement":
        return ""

    expr = None
    for c in meaningful[0].children:
        if c.type not in ("return", ";"):
            expr = c
    if expr is None:
        return "returns nothing"
    if _is_constant_return_expr(expr, src):
        snippet = _node_text(expr, src).strip().replace("\n", " ")[:40]
        return f"only returns the constant `{snippet}`"
    return ""


# ---------------------------------------------------------------------------
# Scattered file-scope constants / globals (cxx-scattered-constant)
# ---------------------------------------------------------------------------
#
# File-scope `static` / `constexpr` / `const` declarations (and anonymous-
# namespace ones) belong together in a `// Constants` banner at the top of the
# file, not sprinkled between functions across thousands of lines. The rule
# flags any such declaration positioned AFTER the first function definition --
# those should be hoisted to the top section so every constant lives in one
# reviewable place. Declarations before the first function (the top section
# itself), class members, function-local statics, and `extern` declarations
# are not flagged.

_CONST_DECL_SPECIFIERS = frozenset({"static", "constexpr", "const", "constinit"})

# A file-scope constant / global in this codebase follows one of three naming
# conventions: `kCamelCase` (constexpr), `s_lower_case` (static file-local), or
# `UPPER_CASE`. Requiring the name to match keeps the scattered-constant rule
# from firing on locals that tree-sitter mis-attributes to file scope when a
# `#ifdef` splits a function body (e.g. `const auto& dy = ...` in a commercial
# branch) and on attribute identifiers like `nodiscard`.
_CONSTANT_NAME_RE = re.compile(r"^(?:k[A-Z]|s_[a-z_]|[A-Z][A-Z0-9_]+$)")

# Implementation-file suffixes for the "all constants in one top section" rule.
# Headers legitimately group constants next to the type they describe, so the
# rule skips them. `.mm` is excluded too: tree-sitter-cpp can't parse
# Objective-C++, so it mis-classifies function-local statics as file-scope
# (a false positive on Utilities_macOS.mm's `kOrder`).
_IMPL_SUFFIXES = (".cpp", ".cc", ".cxx")


def _is_file_scope_decl(node) -> bool:
    """True when `node` sits at translation-unit or namespace scope -- not
    inside a class body, a function body, or a lambda."""
    p = node.parent
    while p is not None:
        if p.type in (
            "function_definition",
            "lambda_expression",
            "field_declaration_list",
            "compound_statement",
        ):
            return False
        p = p.parent
    return True


def _is_constant_global_decl(node, src: bytes) -> bool:
    """True when `node` is a file-scope variable declaration carrying a
    `static` / `constexpr` / `const` / `constinit` specifier. Function forward
    declarations (`static void f();`) and `extern` declarations are excluded --
    only actual constant/global variables count."""
    if node.type != "declaration":
        return False
    specs = {
        _node_text(c, src).strip()
        for c in node.children
        if c.type in ("storage_class_specifier", "type_qualifier")
    }
    if "extern" in specs:
        return False
    if not (specs & _CONST_DECL_SPECIFIERS):
        return False
    child_types = [c.type for c in node.children]
    if "function_declarator" in child_types:
        return False
    return any(
        t
        in (
            "init_declarator",
            "identifier",
            "array_declarator",
            "pointer_declarator",
            "reference_declarator",
        )
        for t in child_types
    )


def _decl_first_name(node, src: bytes) -> str:
    """Best-effort name of the first variable declared in `node`."""
    for c in node.children:
        if c.type == "init_declarator":
            target = c.child_by_field_name("declarator") or c
            for sub in _walk(target):
                if sub.type == "identifier":
                    return _node_text(sub, src)
    for sub in _walk(node):
        if sub.type == "identifier":
            return _node_text(sub, src)
    return "<constant>"


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


def _is_anonymous_namespace(node) -> bool:
    """True when a namespace_definition has no name (`namespace { ... }`).
    The grammar exposes the name as a `namespace_identifier` or
    `nested_namespace_specifier` child; anonymous namespaces have neither."""
    for child in node.children:
        if child.type in ("namespace_identifier", "nested_namespace_specifier"):
            return False
    return True


def _anon_member_label(node, src: bytes) -> str:
    """Return a short human label for a top-level entity inside an anonymous
    namespace -- function name, type name, or first declared identifier --
    so the report can point at the actual symbol rather than just the line."""
    if node.type == "function_definition":
        name = _function_name(node, src)
        return name if name else "<function>"
    if node.type in (
        "class_specifier",
        "struct_specifier",
        "enum_specifier",
        "union_specifier",
        "type_definition",
        "alias_declaration",
    ):
        name = _type_name(node, src)
        return name if name else "<type>"
    if node.type == "template_declaration":
        for child in node.children:
            if child.type in (
                "function_definition",
                "class_specifier",
                "struct_specifier",
            ):
                return _anon_member_label(child, src)
        return "<template>"
    if node.type == "declaration":
        for child in _walk(node):
            if child.type in ("identifier", "field_identifier"):
                return _node_text(child, src)
    return "<entity>"


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


# Tooling pragmas a compiler or another linter reads. Deleting one changes
# behavior (clang-format regions, suppressed warnings, switch fallthrough),
# so the in-body-comment rule never flags them even though they live inside
# a function body.
_INBODY_PRAGMA_RE = re.compile(
    r"^(?:clang-format|code-(?:verify|format)|NOLINT\w*|cppcheck-suppress"
    r"|coverity|lint|fallthrough|FALLTHROUGH|@suppress)\b",
    re.IGNORECASE,
)


def _comment_pragma_payload(text: str) -> str:
    """Return the prose after a single comment's leading markers, for pragma
    detection. Handles `//`, `///`, `//!`, and `/*` / `*` block openers."""
    s = text.lstrip()
    if s.startswith("//"):
        j = 2
        while j < len(s) and s[j] == "/":
            j += 1
        if j < len(s) and s[j] == "!":
            j += 1
        return s[j:].lstrip()
    if s.startswith("/*"):
        return s[2:].lstrip().lstrip("*").lstrip()
    if s.startswith("*"):
        return s[1:].lstrip()
    return s


def _is_inbody_pragma(text: str) -> bool:
    """True when a comment carries a tooling pragma (clang-format / NOLINT /
    cppcheck-suppress / fallthrough / code-verify). Those are directives, not
    the narration the in-body rule targets, and removing them is not safe."""
    return bool(_INBODY_PRAGMA_RE.match(_comment_pragma_payload(text)))


# `while (...)` condition extraction + whitelist. Tree-sitter would give us
# the parsed condition cheaply, but the surrounding loop already operates on
# stripped text lines, so we re-balance parens by hand: walk forward from the
# opening `(` after `while`, counting depth across multi-line conditions, and
# return the inner text. Cheap, no second parse, no false multi-line miss.
def _gather_while_condition(lines: list[str], start_idx: int) -> str | None:
    """Return the text inside the `while ( ... )` opener that begins on
    `lines[start_idx]`, joining continuation lines until parens balance.
    Returns None if the opener cannot be parsed cleanly (e.g. embedded in a
    macro or split awkwardly across the file)."""
    first = _strip_strings_and_line_comments(lines[start_idx])
    m = re.search(r"\bwhile\s*\(", first)
    if not m:
        return None
    pos = m.end()
    depth = 1
    pieces: list[str] = []
    line_idx = start_idx
    cur = first[pos:]
    while line_idx < len(lines):
        for ch in cur:
            if ch == "(":
                depth += 1
            elif ch == ")":
                depth -= 1
                if depth == 0:
                    break
            pieces.append(ch)
        if depth == 0:
            break
        line_idx += 1
        if line_idx >= len(lines):
            return None
        cur = _strip_strings_and_line_comments(lines[line_idx])
        pieces.append(" ")
    if depth != 0:
        return None
    return "".join(pieces)


# Canonical bounded shapes drawn from the catalog in
# memory/project_while_loop_patterns.md. Each entry is a substring or regex
# fragment that, if present in the condition text, marks the loop as
# syntactically bounded. The list is conservative -- a few genuinely-unbounded
# loops will sneak through if their condition happens to mention one of these
# tokens, but the false-positive rate at advisory severity matters more than
# the rare false-negative.
_WHILE_BOUNDED_PATTERNS = [
    # Queue drains: `try_dequeue`, `try_pop`, `pop_front` returning bool.
    re.compile(r"\btry_(?:de)?queue\b"),
    re.compile(r"\btry_pop\b"),
    # Atomic worker-thread flag: `m_running.load()`, `m_*.load(...)`.
    re.compile(r"\.load\s*\("),
    # Qt iterator / stream walks. `.next()` and `->next()` both hit -- the
    # `[.>-]` cluster covers `obj.next()`, `ptr->next()`, and the rarer
    # `(*ptr).next()`.
    re.compile(r"(?:\.|->)\s*hasNext\s*\(\s*\)"),
    re.compile(r"(?:\.|->)\s*atEnd\s*\(\s*\)"),
    re.compile(r"(?:\.|->)\s*next\s*\(\s*\)"),
    re.compile(r"(?:\.|->)\s*framesAvailable\s*\(\s*\)"),
    # Explicit iteration cap: `i < kMax*`, `i < 8192`, `iterations < N`.
    # The `< <literal int>` form catches `fftSamples < 8192`, `wrapCount < 10`.
    re.compile(r"<\s*k[A-Z]\w*"),
    re.compile(r"<\s*\d+\b"),
    re.compile(r"\b(?:iterations?|wrapCount|guard|tries|attempts)\b\s*<"),
    # Length / size bound: `i < n`, `i < buf.size()`, `i < current_size`,
    # `start < line.length()`. The bare-identifier form covers member fields
    # that look like a size (current_size, m_size, end_, len, n, ...).
    re.compile(r"<\s*\w+\.(?:size|length|count)\s*\(\s*\)"),
    re.compile(r"<\s*\w+\s*->\s*(?:size|length|count)\s*\(\s*\)"),
    # Bare-ident size bound: only count when the LHS clause is a single
    # identifier (not an arithmetic expression). The leading lookbehind
    # rejects anchored mid-clause matches; the body matches `((cond &&|^)
    # <whitespace> <ident> < <size-ident>)`. The condition `cols * rows < n`
    # is the WindowManager::tileGrid shape that the rule exists to catch,
    # so it must NOT match here.
    re.compile(
        r"(?:^|&&|\|\||\()\s*\w+\s*<\s*(?:n|len|cnt|count|size|end|m|num\w*"
        r"|\w*_size|\w*_len|\w*Size|\w*Length|\w*Count|\w*End)\b"
    ),
    # Size-driven append/trim: `data.size() < kSize`, `vec.size() < maxX`,
    # `m_logEntries.size() > kMaxLogEntries`.
    re.compile(r"(?:\.|->)\s*size\s*\(\s*\)\s*[<>]"),
    re.compile(r"(?:\.|->)\s*length\s*\(\s*\)\s*[<>]"),
    re.compile(r"(?:\.|->)\s*count\s*\(\s*\)\s*[<>]"),
    # String trim helpers: `endsWith` / `startsWith` on a buffer that shrinks
    # inside the body (`.chop`, `.remove(0, n)`).
    re.compile(r"(?:\.|->)\s*endsWith\s*\("),
    re.compile(r"(?:\.|->)\s*startsWith\s*\("),
    # Parser sentinel walks: `m_cur.type != Tok::Eof` and the positive form
    # `m_cur.type == Tok::Foo` / `m_cur.type == Tok::Foo || ... == Tok::Bar`
    # where each `advance()` call inside the body necessarily either consumes
    # a token or reaches Eof, so the token stream bounds the loop.
    re.compile(r"\w+::Eof\b"),
    re.compile(r"\bTok::[A-Z]\w*"),
    # Container traversal: `m_pendingChunks.empty()` style draining.
    re.compile(r"!\s*\w+(?:\.|->)\s*empty\s*\(\s*\)"),
    # Strict-monotone counter heading toward zero -- `b >= 0`, `end > 0`,
    # `count > 1`, `wordStartX > 0`. The body strictly decrements; bound is
    # the initial value of the counter.
    re.compile(r"\b\w+\s*>=\s*0\b"),
    re.compile(r"\b\w+\s*>\s*\d+\b"),
    # Two indices converging: `s < e`, `e > s`, `lo < hi`. Each iteration
    # moves one toward the other; bound is `|e - s|`. Anchor to clause start
    # so `cols * rows < e` does not match.
    re.compile(r"(?:^|&&|\|\||\()\s*[a-z]\w{0,3}\s*[<>]\s*[a-z]\w{0,3}\b"),
    # Cancellation polling: `!context->IsCancelled()` against an external
    # cancel signal that's necessarily eventually true.
    re.compile(r"IsCancelled\s*\(\s*\)"),
    # `contains(x)` predicate over a finite set: caller mutates `x` inside
    # the body until the set rejects it. Bound is the set's size.
    re.compile(r"(?:\.|->)\s*contains\s*\("),
    # Name-collision finder: `hasName(unique)` / `hasFoo(x)` where the body
    # appends a suffix to the candidate. Bound is the namespace size.
    re.compile(r"\bhas[A-Z]\w*\s*\("),
]


def _while_condition_is_bounded(cond_text: str) -> bool:
    """True if `cond_text` matches one of the canonical bounded shapes.
    See `_WHILE_BOUNDED_PATTERNS` for the catalog -- the list is the source
    of truth, this function just runs the patterns in order."""
    for pat in _WHILE_BOUNDED_PATTERNS:
        if pat.search(cond_text):
            return True
    return False


def _camel(snake: str) -> str:
    """Map a JSON key to its Frame.h Keys:: identifier (PascalCase). The
    constants in Frame.h are PascalCase versions of the camelCase JSON
    keys -- e.g. `frameStart` -> `FrameStart`."""
    return snake[:1].upper() + snake[1:]


# ---------------------------------------------------------------------------
# Comment-narration rule (CLAUDE.md "Comments & Doxygen" tone bans)
# ---------------------------------------------------------------------------
#
# CLAUDE.md bans tutorial voice ("we", "let's"), throat-clearing ("Note that",
# "FYI"), rot-references ("this PR", "the recent fix"), caller references
# ("Used by X", "Called from Y"), hedging ("for now", "ideally"), and
# tutorial-restating ("This is a function that...", "Iterates over..."). The
# scanner runs over `//` line-comment text and ` * ` doxygen continuation
# text; @brief lines are excluded (a one-liner @brief that mentions "we" is
# rare but possible -- better than nuking 100% of legitimate phrasing).
#
# Third-party files keep their upstream prose unchanged. Detection by path
# (`ThirdParty/`, the SimpleCrypt vendored crypto helper, the Lemon Squeezy
# example template) avoids touching code we don't own.

_NARRATION_PATTERNS = [
    # Tutorial voice -- "we", "let's", "now we"
    (
        re.compile(r"\b(?:we|let's|let us|now we|first we)\b", re.IGNORECASE),
        "tutorial voice (`we`/`let's`) -- rewrite without the first person",
    ),
    # "This is a helper / function / class that..."
    (
        re.compile(
            r"\bThis is a (?:helper |small |simple |utility )?"
            r"(?:function|class|method|variable|helper|piece of code|wrapper|macro)\b"
        ),
        "tutorial voice (`This is a function/class/...`) -- the code already says that",
    ),
    # Throat-clearing prefixes
    (
        re.compile(r"\b(?:Note that|Please note|FYI)\b"),
        "throat-clearing (`Note that`/`FYI`) -- drop the prefix or drop the comment",
    ),
    # Caller references
    (
        re.compile(r"\b(?:Used by|Called from|Called by|Invoked by|invoked from)\b"),
        "caller-reference (`Used by`/`Called from`) -- those rot; if it's load-bearing, "
        "encode it as an invariant in the function instead",
    ),
    # Rot-references (phrasing tied to a transient state)
    (
        re.compile(
            r"\b(?:this PR|recent fix|the recent (?:fix|change)|"
            r"as mentioned above|see below|see above)\b",
            re.IGNORECASE,
        ),
        "rot-reference (`this PR`/`see below`) -- moves the moment it ships; put context "
        "in the commit message",
    ),
    # Restating the code
    (
        re.compile(r"\b(?:Loops over|Iterates over|Iterate through)\b"),
        "restating the code (`Loops over`/`Iterates over`) -- the loop already says that",
    ),
    # Hedging
    (
        re.compile(r"\b(?:For now,|for now,|For clarity,|for clarity,|ideally,)\b"),
        "hedging (`for now`/`ideally`) -- be definite or delete",
    ),
    # Filler adverbs
    (
        re.compile(r"\b(?:simply|basically)\b", re.IGNORECASE),
        "filler adverb (`simply`/`basically`) -- delete or rephrase",
    ),
]

# Files whose prose is upstream and not authored by this codebase.
_NARRATION_VENDORED_PATH_HINTS = (
    "/ThirdParty/",
    "/lemonsqueezy/",  # Pro license-server example template
    "/SimpleCrypt.",  # Andre Somers' vendored crypto helper
)


def _is_brief_line(text: str) -> bool:
    """True when this comment line carries the doxygen `@brief` marker.

    A one-line `@brief` that happens to contain a banned word is rare but
    legitimate (e.g., the @brief of a `sharedDatasetUnit` helper that returns
    "the unit shared by all datasets, or empty when they disagree" — the
    word `they` could trip a future tutorial-voice check). Excluding @brief
    lines keeps the false-positive rate low without making the rule toothless.
    """
    return "@brief" in text


def _is_vendored_path(path: Path) -> bool:
    """True when @p path points to a vendored / upstream-prose file. The
    check is path-based because vendored files don't carry a uniform marker
    in their text -- some have an SPDX banner, some have a license comment,
    some have neither."""
    s = str(path)
    return any(hint in s for hint in _NARRATION_VENDORED_PATH_HINTS)


# ---------------------------------------------------------------------------
# Qt-style preferences for console output and stream formatting
# ---------------------------------------------------------------------------
#
# CLAUDE.md does not call these out by name, but they are universally Qt-style
# violations: a Qt application that mixes `printf` / `std::cout` / `std::endl`
# with `qDebug` / `\n` is harder to retarget at the Qt logging-categories
# infrastructure, breaks message-handler routing, and pays a synchronous-flush
# cost on every emission. Two cases legitimately need raw stdio (the qDebug
# message handler itself, and Windows console attachment) -- those wrap the
# region in `// code-verify off` to silence the rule with intent.

_STDIO_PATTERNS = [
    (
        re.compile(r"\bstd::(?:cout|cerr|clog)\b"),
        "qt-prefer-qdebug",
        "`std::cout` / `std::cerr` -- prefer `qDebug()` / `qWarning()` "
        "(routes through the Qt message handler and the Console widget)",
    ),
    (
        re.compile(r"^\s*#include\s+<iostream>"),
        "qt-prefer-qdebug",
        "`<iostream>` -- prefer `<QDebug>` (Qt streams integrate with the message handler)",
    ),
    (
        re.compile(r"\bprintf\s*\("),
        "qt-prefer-qdebug",
        "`printf(...)` -- prefer `qDebug()` (Qt routes through the message handler "
        "and the Console widget)",
    ),
    (
        re.compile(r"\bstd::endl\b"),
        "qt-prefer-newline",
        "`std::endl` flushes the stream on every line -- use `'\\n'` "
        "(or Qt::endl when explicit flushing is intentional)",
    ),
]


def _stdio_findings(src_text: str, path: Path, fence_mask: list[bool]) -> list[Finding]:
    """Flag raw-stdio output (`std::cout`, `<iostream>`, `printf`, `std::endl`)
    in Qt source code. Strings and `//` line comments are masked first so
    `// printf("...")` in prose doesn't fire."""
    if path.suffix not in (".cpp", ".cc", ".cxx", ".mm", ".h", ".hpp", ".hxx"):
        return []
    if _is_vendored_path(path):
        return []
    out: list[Finding] = []
    for i, raw in enumerate(src_text.split("\n"), start=1):
        if i - 1 < len(fence_mask) and fence_mask[i - 1]:
            continue
        scrubbed = _strip_strings_and_line_comments(raw)
        for pat, kind, msg in _STDIO_PATTERNS:
            if pat.search(scrubbed):
                out.append(Finding(i, kind, msg))
                break
    return out


_TRAILING_DOXY_RE = re.compile(r"/\*\*<")


def _trailing_doxy_findings(
    src_text: str, path: Path, fence_mask: list[bool]
) -> list[Finding]:
    """Flag trailing-style doxygen `/**< ... */` member comments in headers.

    CLAUDE.md "Headers (.h) -- strict rule" forbids member-variable
    comments. Trailing `/**< description */` is the doxygen-specific form
    of that ban -- the underlying problem is the same: names + types are
    the documentation.
    """
    if path.suffix not in (".h", ".hpp", ".hxx"):
        return []
    if _is_vendored_path(path):
        return []
    out: list[Finding] = []
    for i, raw in enumerate(src_text.split("\n"), start=1):
        if i - 1 < len(fence_mask) and fence_mask[i - 1]:
            continue
        if _TRAILING_DOXY_RE.search(raw):
            out.append(
                Finding(
                    i,
                    "doc-trailing-member",
                    "header member-variable trailing doxygen `/**< ... */` -- "
                    'delete it (CLAUDE.md "No member-variable comments" rule)',
                )
            )
    return out


def _comment_narration_findings(
    src_text: str, path: Path, fence_mask: list[bool]
) -> list[Finding]:
    """Scan comment text in @p src_text and emit one finding per banned pattern.

    Walks every line: `// ...` line comments contribute their tail text, and
    ` * ...` lines inside a doxygen / block comment contribute their body
    (minus the leading `*`). String literals and code outside comments are
    not visited. @brief lines are skipped.
    """
    if _is_vendored_path(path):
        return []

    out: list[Finding] = []
    lines = src_text.split("\n")
    for i, raw in enumerate(lines, start=1):
        if i - 1 < len(fence_mask) and fence_mask[i - 1]:
            continue

        # `//` line comment -- everything after the marker is prose.
        m_line = re.search(r"//(.*)$", raw)
        # ` * ` doxygen continuation -- a line starting with whitespace + `*`
        # but NOT starting a `*/` close or a `/**` open is prose. The opener
        # `/**` line itself often contains @brief and runs through the
        # @brief filter below.
        m_doxy = re.match(r"^\s*\*\s?(.*)$", raw)

        if m_line:
            text = m_line.group(1)
        elif m_doxy and not m_doxy.group(1).startswith("/"):
            text = m_doxy.group(1)
        else:
            continue

        if _is_brief_line(text):
            continue

        for pat, msg in _NARRATION_PATTERNS:
            if pat.search(text):
                out.append(Finding(i, "comment-narration", msg))
                break  # one finding per line is plenty
    return out


# ---------------------------------------------------------------------------
# QML semantic rules (line-based -- no tree-sitter for QML)
# ---------------------------------------------------------------------------

# Files that are allowed to use direct font.pixelSize/font.family. These are
# the dashboard widgets that compute zoom-dependent font sizes; the
# `font:` helpers don't accept dynamic sizes.
_FONT_PIXEL_OK_FILES = frozenset(
    {
        "FFTPlot.qml",
        "Plot.qml",
        "MultiPlot.qml",
        "Bar.qml",
        "Clock.qml",
        "Gauge.qml",
        "Meter.qml",
        "Compass.qml",
        "DataGrid.qml",
        "Accelerometer.qml",
        "Gyroscope.qml",
        "GPS.qml",
        "LEDPanel.qml",
        "Plot3D.qml",
        "Waterfall.qml",
        "ImageView.qml",
        "Terminal.qml",
        "PlotWidget.qml",
        "VisualRange.qml",
        "NotificationLog.qml",
        # Project-editor visualizations that compute zoom-dependent font sizes
        # are also exempt -- the `font:` helpers don't accept a dynamic
        # pixelSize, and these views need it for Ctrl+Wheel zoom.
        "FlowDiagram.qml",
    }
)


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
            if re.search(
                r"\bfont\.(pixelSize|pointSize|family|bold|italic"
                r"|weight|capitalization)\s*:",
                line,
            ):
                out.append(
                    Finding(
                        i,
                        "qml-font-pixel",
                        "use `font: Cpp_Misc_CommonFonts.uiFont` (or another "
                        "helper) instead of individual `font.*` sub-properties",
                    )
                )

        # qml-bus-type-int: `busType: <integer>` or `BusType: <int>`. The
        # property name on a `Source` is `busType`; the C++ enum is
        # `SerialStudio.BusType.UART`. Reject literal ints.
        m = re.search(r"\bbusType\s*:\s*(-?\d+)\b", line)
        if m:
            out.append(
                Finding(
                    i,
                    "qml-bus-type-int",
                    f"hardcoded busType `{m.group(1)}` -- use "
                    f"`SerialStudio.BusType.<NAME>`",
                )
            )

    return out


# ---------------------------------------------------------------------------
# Public entry point
# ---------------------------------------------------------------------------


def analyze(path: Path, src_text: str, fence_mask: list[bool]) -> list[Finding]:
    """Run every applicable rule against `src_text` for `path`. The driver
    in code-verify.py wraps each Finding as a Violation."""
    suffix = path.suffix.lower()
    out: list[Finding] = []
    if suffix in (".cpp", ".cc", ".cxx", ".c", ".h", ".hpp", ".hxx", ".mm"):
        out.extend(_cpp_rules(src_text.encode("utf-8"), path, fence_mask))
        out.extend(_comment_narration_findings(src_text, path, fence_mask))
        out.extend(_trailing_doxy_findings(src_text, path, fence_mask))
        out.extend(_stdio_findings(src_text, path, fence_mask))
        return out
    if suffix == ".qml":
        out.extend(_qml_rules(src_text, path, fence_mask))
        out.extend(_comment_narration_findings(src_text, path, fence_mask))
        return out
    return out
