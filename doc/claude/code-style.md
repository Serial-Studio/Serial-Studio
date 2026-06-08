# Code Style & Safety-Critical Rules

`scripts/code-verify.py` enforces this — read its `--check` output, don't re-derive the rules.
The compressed essentials live inline in `CLAUDE.md`; this doc is the full specification.

## Code Style

### Formatting

- 100-column limit, 2-space indent. Pointer/ref binds to type (`int* p`, `const Foo& r`).
- Braces: function bodies on new line, control statements on same line.
- `RemoveBracesLLVM` strips braces from single-statement bodies. `BinPackArguments` /
  `BinPackParameters` = false (one per line when wrapping).
- LF endings everywhere. ASCII-only in user-facing Markdown (CLAUDE.md and code comments
  exempt). Run `clang-format`.

### Naming

| Kind | Convention | Example |
|------|------------|---------|
| Classes / Enums | `CamelCase` | `FrameReader`, `BusType` |
| Functions | `camelCase` | `hotpathRxFrame` |
| Locals / params | `lower_case` | `frame_data` |
| Static vars | `s_lower_case` | `s_devices` |
| Private members | `m_camelCase` | `m_deviceIndex` |
| Public/protected members | `lower_case` | `sourceId` |
| Constants / constexpr | `kCamelCase` | `kMaxBufferSize` |
| Macros | `UPPER_CASE` | `BUILD_COMMERCIAL` |

### Control Flow

- **Max 3 nesting levels.** Early returns, early continues, extract functions.
- **No braces on single-statement bodies.** Blank line after a brace-free body on its own line.
- **Guard clauses** over nested error handling.
- **Functions: 40-80 lines target**, hard limit 100. Split bigger work.

```cpp
if (!frame.isValid())
  return;

for (const auto& g : frame.groups())
{
  if (!g.isEnabled())
    continue;

  processGroup(g);
}
```

### C++ Headers

Reference: `app/src/IO/Drivers/BluetoothLE.h`. Order: `Q_OBJECT` → `Q_PROPERTY` block
(clang-format off, one attribute per line) → `signals:` → private ctor + deleted copy/move
(singletons) → `public:` (`instance()` first, then `[[nodiscard]]` getters) → `public slots:`
→ `private slots:` → `private:` helpers → `private:` members.

- `[[nodiscard]]` on every non-void return.
- **Never `Q_INVOKABLE void`** — `public slots:`. `Q_INVOKABLE` is for non-void returns only.
- Christmas-tree ordering (shortest-to-longest line) within each block.
- `noexcept` on trivial const getters that only read members.
- **No in-header member init** (`int m_foo = 0;` is forbidden). Use the ctor init list.

### Signals & Connections

- `Q_EMIT`, never bare `emit`.
- `signals:` / `public slots:` / `private slots:` — never `Q_SIGNALS:` / `public Q_SLOTS:`.
- Short `connect()` on one line; long form one arg per line. Never `SIGNAL()` / `SLOT()`.
- Never `disconnect(nullptr)` as the slot — capture the `QMetaObject::Connection` and
  disconnect that.
- Never call `parseFunction.call()` directly on a `QJSEngine` parser — always
  `IScriptEngine::guardedCall()`.

### Comments & Doxygen

**Code is the spec. Comments label sections; they don't narrate.**

**Headers (.h)** — only two kinds of comments allowed:
1. SPDX banner at top.
2. `/** @brief ... */` directly above every type-level definition: classes, structs, `enum` /
   `enum class`, top-level `typedef`, top-level `using`-aliases. One `@brief` per definition
   — helper structs and payload typedefs need their own, not just the primary class.

No function doxygen above member declarations, no trailing `/**< ... */`, no multi-tag
verbose blocks, no inline `//`. Names + types are the documentation. Exempt from `@brief`:
forward declarations, nested types inside a class body, `using Base::Base;` imports, type
aliases declared inside a function body.

**Source (.cpp)** — every function definition gets a one-line `/** @brief ... */` directly
above it. Ctors, dtors, slots, helpers, every one. No `@param`/`@return`/`@note`. Use 98-dash
`//---` banners for concern groups *between* functions. **No comments inside a function body.**
Functions are capped at 100 lines, so the `@brief` above the function plus self-explanatory code
carry it: a comment that restates or narrates the next line gets deleted, and a load-bearing
*why* is folded up into the `@brief` (lengthen the brief if needed — the brief is the right home
for the why). A genuinely-needed in-body note (a literal lookup table, a derivation, a citation)
stays behind a reviewed `// code-verify off` / `on` fence. `code-verify.py` flags every in-body
comment as `cxx-inbody-comment` (advisory; tree-sitter-located, so the `@brief` above the
function is never caught, and tooling pragmas like `// clang-format`/`// NOLINT`/`// fallthrough`
are skipped). **Forbidden**: inline EOL comments, multi-line `//` prose, `/* ... */` inside
function bodies, restating the code, AI narration ("we", "Note that", tutorial voice, "this used
to...", hedging, bare `TODO`).

**Don't fake the em-dash.** Source and user-facing Markdown are ASCII-only, so the em-dash
glyph (U+2014) is out — but the fix is to *rewrite the sentence*, not to swap in a spaced
double-hyphen ` -- `. `--` as a sentence dash is a mechanical glyph trade that reads like a
robot did the edit; recast with a comma, period, or parentheses instead. The point of
the rule is human, considered prose, not one dash glyph for another. `code-verify.py` flags it
in comments (`comment-dash-substitute`, advisory) and `documentation-verify.py` in docs
(`style-dash-substitute`); `i--`, `--i`, and `//---` banners don't match (the rule needs a
space on both sides). The whole codebase carries baseline `--` debt, so both ship as advisory
— new prose should still clear them.

### QML

- **Christmas-tree property order** by **total rendered line length** (shortest first).
  `id` first, blank line after.
- **Typography**: `font: Cpp_Misc_CommonFonts.uiFont` etc. Individual `font.*` sub-properties
  only in dashboard widgets that compute dynamic pixel sizes (zoom-dependent).
- **Reactive bindings**: `Q_PROPERTY` + `NOTIFY`. No comma-expression hacks.
- **Enums**: `SerialStudio.BusType`, `ProjectModel.SomeEnum`. Never hardcoded integers.
- **No inline `//` comments mid-statement**; section headers on their own line only.

Font helpers: `uiFont`, `boldUiFont`, `monoFont`, `customUiFont(fraction, bold)`,
`customMonoFont(fraction, bold)`, `widgetFont(fraction, bold)`. Scales: `kScaleSmall=0.85`,
`kScaleNormal=1.0`, `kScaleLarge=1.25`, `kScaleExtraLarge=1.50`.

### Performance

- Hotpaths: zero-copy const refs, `[[likely]]`/`[[unlikely]]`, static-cached singletons.
- **Never allocate on the dashboard path. Never copy a Frame.**
- KMP for single-delimiter; `CircularBuffer::findFirstOfPatterns()` for multi (single-pass,
  stack array ≤8, no heap).
- `constexpr` CRC tables. Profile first.

### Licensing

SPDX headers required: `GPL-3.0-only`, `LicenseRef-SerialStudio-Commercial`, or both.
Validate at system boundaries only (API input, file I/O, network). Trust internal data.

## Safety-Critical Code — NASA Power of Ten

Mission-critical telemetry. Hotpath violations are blockers.

1. **No `goto`/`setjmp`/`longjmp`.** No unbounded recursion — every recursive function has a
   hard depth cap (`FrameParser::parseMultiFrame` ≤2, `JsonValidator` ≤128,
   `Taskbar::findItemByWindowId` ≤3, `ConversionUtils` ≤64).
2. **Loops have fixed upper bounds.** External-data loops use explicit `kMaxIterations`.
   `while(true)` only with a provable termination invariant — document it.
3. **No allocation after init on the hotpath.** No `new`/`make_shared`/`.append()` on the
   dashboard path. `FrameBuilder::acquireFrame()` draws each `TimestampedFramePtr` from a
   fixed-size slot pool (`kFramePoolSize = 1024`); the slot is recycled when the last
   consumer drops the shared_ptr (custom deleter flips `inUse` to false). Don't bypass the
   pool with a direct `std::make_shared<TimestampedFrame>(...)` on the hotpath — that
   re-introduces a per-frame heap alloc. Pool exhaustion logs once and falls back to
   `make_shared` so the producer never blocks. The perf-* advisories (`code-verify.py`)
   catch accidental hot-path allocation, regex construction, locking, logging, throwing,
   large by-value params, `shared_ptr` by-value, runtime divide/modulo, `pow()`,
   `dynamic_cast`, virtual calls, large stack buffers, false-sharing, recursion in hot loops.
4. **Functions 40-80 lines, hard limit 100.** Nesting ≤3. Split into helpers.
5. **Assertion density ≥2 per function.** Pre/post-conditions + invariants. `Q_ASSERT` for
   debug; `if (!cond) return;` for release safety. No `assert(true)`.
6. **Smallest scope.** Declare at first use. No function-top var blocks. Anonymous namespaces
   only for true file-locals.
7. **Check return values at system boundaries** (driver/file/network/API). `[[nodiscard]]`
   everywhere. `try_enqueue()` failures must be logged. JS calls go through
   `IScriptEngine::guardedCall()`, never direct.
8. **Minimal preprocessor.** Only `#include`, `#pragma once`, `#ifdef BUILD_COMMERCIAL` /
   `ENABLE_GRPC`, platform guards. No token pasting, no variadic macros.
9. **No `reinterpret_cast`** except byte-level access (`const uint8_t*`). Prefer
   `std::bit_cast`. No raw function pointers. No `dynamic_cast` on the hotpath — refactor
   to a tag or invariant-checked `static_cast`.
10. **Zero warnings.** `-Wall -Wextra -Wpedantic`, `ENABLE_HARDENING` for production. Fix
    root cause; never suppress without justification.
