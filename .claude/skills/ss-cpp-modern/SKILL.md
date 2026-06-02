---
name: ss-cpp-modern
description: >-
  Modern C++20 authoring guidance for Serial Studio (Qt 6.11, C++20): concepts, ranges,
  move/RAII, std smart pointers, constexpr, lock-free SPSC atomics. Use when writing or
  refactoring non-trivial C++ here and you want the idiomatic modern-C++ shape — picking a
  smart pointer, designing an RAII wrapper, a concept-constrained template, or a hotpath data
  structure. Defers all style/build/sanitize/test rules to CLAUDE.md, scripts/, and the
  ss-hotpath / ss-verify / qt-cpp-review skills — it does NOT build, sanitize, or run anything.
---

# Serial Studio — modern C++20 authoring

Adapted from jeffallan's `cpp-pro` skill, trimmed to what this repo actually uses and
realigned to its rules. This is *authoring* guidance — the idiomatic C++20 shape of a thing.
It does not own style, build, or verification:

- **Style / formatting / structure**: `scripts/code-verify.py` is the contract. See [[ss-verify]].
  Don't re-derive its rules; run it. (100-col, 2-space, LF, `m_`/`s_`/`k` naming, header member
  order, `Q_EMIT`, no in-header init, `[[nodiscard]]`, ASCII-only.)
- **Hotpath / threading / Power of Ten**: see [[ss-hotpath]] before touching the data path.
- **Correctness review**: see [[qt-cpp-review]] after writing.
- **Building / sanitizers / running**: **you don't.** The developer builds the Pro edition and
  runs sanitizers (`scripts/`) themselves. Never invoke `cmake`/`jom`/`clang`/the compiler.

## House baselines (this repo, not generic C++)

These override the generic "C++ Pro" advice you may know:

- **`std` smart pointers, not Qt ones.** `std::unique_ptr` / `std::shared_ptr` / `std::weak_ptr`
  are house style (e.g. `TimestampedFramePtr = std::shared_ptr<TimestampedFrame>`). Do not reach
  for `QScopedPointer` / `QSharedPointer`.
- **No raw `new`/`delete`** outside a QObject parent-owned tree or a documented RAII wrapper.
  QObject children are parent-owned; everything else is a smart pointer or a value.
- **`static_cast` and friends, never C-style casts.** No `reinterpret_cast`/`dynamic_cast` on
  the hotpath (Power of Ten).
- **`auto` with judgement, not reflexively.** Use it when the type is obvious from the
  initializer (`auto it = map.find(...)`); spell the type out when it aids the reader. The repo
  favors readable concrete types in signatures.
- **Const-correct always.** `[[nodiscard]]` on every non-void return in a header (linter
  enforces). Methods that don't mutate are `const`.
- **Exceptions vs error codes**: Lua/JS host boundaries force unwind tables + try/catch +
  `lua_atpanic` (see the Lua exception-safety setup); elsewhere prefer status returns /
  `std::optional` / signals consistently within a subsystem (ERR-12). Don't mix patterns in one
  class.

## When to reach for what

### Smart pointer choice
- `std::unique_ptr<T>` — sole ownership; the default. `const std::unique_ptr<T>` for a member
  that is created once and lives for the owner's lifetime (scoped, non-reseatable).
- `std::shared_ptr<T>` — genuine shared ownership only (e.g. a frame handed to several async
  sinks). On the dashboard hotpath, frames come from `FrameBuilder::acquireFrame()` (slot pool),
  not fresh `make_shared` — see [[ss-hotpath]] SS-HOT-3.
- `std::weak_ptr<T>` — break a shared cycle / observe without owning.
- A QObject with a parent — parent-owned; no smart pointer needed (and don't add one).

### Concepts (C++20) over SFINAE
Constrain templates with a named `concept` instead of `std::enable_if`. A concept reads as a
self-documenting requirement and gives better diagnostics. Prefer requires-clauses on
overloaded templates; reserve `static_assert` for hard mandates that should never be a silent
overload-resolution miss. Keep concept names domain-specific.

### Ranges / views (C++20)
Lazy `std::views::filter | transform | take` pipelines replace hand-rolled loops for
transformation chains — but they are **off the hotpath** by default. The frame extractors and
per-dataset transforms are hand-tuned, fixed-bound loops (Power of Ten SS-POT-2); do not
introduce range pipelines there without benchmarking (`--benchmark-hotpath`).

### Move semantics & RAII
- Define the special members deliberately: a type owning a resource is either `Q_DISABLE_COPY`
  + movable (move-and-swap assignment) or a value type with the rule of zero. Document the
  moved-from state.
- `return std::move(local)` defeats NRVO — return the named local directly. A `const` local
  also blocks the implicit move on return.
- RAII wrap every raw OS/library handle (file, socket, libusb/hidapi handle) so cleanup is
  exception-safe and ordering is explicit. The driver destructors here are order-sensitive
  (join the worker thread before tearing down the resource it touches — SS-DRV-1).

### constexpr / compile-time
Prefer `constexpr` (and `kCamelCase` constants) over macros and runtime initialization for
fixed data. Don't use a dynamically-sized container for statically-sized data (VAR-4) — use
`std::array`.

### Lock-free / atomics (the SPSC ring)
`CircularBuffer` + `FrameReader` are a single-producer/single-consumer, main-thread design —
**no mutex** (SS-HOT-1). When you genuinely need cross-thread coordination elsewhere, use
`std::atomic` with explicit memory ordering (`acquire`/`release` to publish-then-observe), not
ad-hoc volatile or relaxed-everywhere. Shared counters across threads are `std::atomic` or
mutex-guarded (THR-5). The JS watchdog's interrupt flag is the canonical atomic-flag pattern.

## Reference

- `references/cpp20-idioms.md` — concept, ranges, move, RAII, and atomic snippets written in
  this repo's naming/style (not the generic upstream examples).

## Output expectations

When you implement, follow CLAUDE.md's handoff rules: one-line statement of intent before
non-trivial work, a one/two-sentence summary of what changed when you stop, no doc files unless
asked. Provide the header + implementation edits; mention CMake list additions in chat if a new
translation unit is needed (the developer wires and builds it). Do not paste whole-file
rewrites — targeted edits only.
