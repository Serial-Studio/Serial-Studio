# CLAUDE.md

## Behavioral Rules

- **Read before writing.** Never edit a file you haven't read this session.
- **Read hotpath code in full** (`FrameBuilder`, `CircularBuffer`, `FrameReader`, `Dashboard`)
  before touching it. **Read `BluetoothLE.h/.cpp`** before writing any new driver — it's the
  canonical reference.
- **Read existing signal/slot wiring** in a file before adding or changing any.
- **Plan before multi-file changes** (>3 files): state the plan, get confirmation.
- **Edit, don't rewrite.** Targeted `Edit` calls; full rewrite only when asked or >70% changed.
- **No preamble, no trailing summary** — except a one-line statement of
  intent before non-trivial work, and one or two sentences naming what
  changed (and what's next) when you stop. Skip both on trivial edits.
- **Do not create markdown/doc files** unless asked. Share info conversationally.
- **Don't build or run the app.** Never invoke `cmake`/`jom`/`clang`/the compiler — the
  developer builds and runs it themselves. Verify changes by reading and with
  `scripts/code-verify.py`; leave compilation and runtime testing to the user.
- **Update CLAUDE.md** for any architectural change that future me would otherwise miss.
- **`scripts/` is the style contract.** When in doubt, run it; don't restate it here.

## Trust Contract

These rules are about predictability, not productivity — the difference
between a tool the user re-audits every time and a collaborator they rely
on. Capability without predictability gets disabled.

- **Stay in your lane.** Every file touched outside the explicit ask costs
  the reviewer an audit pass. Spot an adjacent fix? *Name it in chat*
  ("noticed X — want it in this pass?") rather than slipping it into the
  diff. Bundled scope creep erodes trust in every diff that follows.
- **Show the why, not the what.** Code shows *what*; a comment, chat reply,
  or commit message shows *why* — but only when the choice was non-obvious
  (one of two reasonable approaches, a workaround, a hidden invariant). One
  sentence. When the choice was obvious, say nothing.
- **State the plan before non-trivial work.** Not just multi-file: any
  change where a reasonable reviewer could prefer a different approach. Plan
  visible *before* execution is the contract — a summary after is not.
- **Self-review before handoff.** Before declaring a non-trivial change
  done, re-read the diff: is this *what was asked, and only that*? If you
  can't answer yes, say so before claiming completion.

## Scripts

All scripts in `scripts/` are CWD-independent and write LF endings on every platform. Safe
to run from any directory.

| Script | Role |
|--------|------|
| `sanitize-commit.py` | Top-level driver: chmod (POSIX) → expand-doxygen → clang-format → code-verify --fix → clang-format → code-verify --check → documentation-verify → search-index rebuild → changed-file summary. Sanitize only — it never commits or pushes. **Run before every commit.** |
| `code-verify.py` | Structural + tone linter for C++/QML/H. `--fix` rewrites in place; `--check` regenerates `.code-report`. Errors block CI; advisories are baseline-debt cleanup. |
| `documentation-verify.py` | Markdown linter for AI-narration / marketing copy. Read-only; writes `.doc-report`. Targets `README.md`, `AGENTS.md`, `doc/help/**`, `examples/**/README.md` (CLAUDE.md is exempt). |
| `expand-doxygen.py` | Rewrites single-line `/** text */` into the canonical 3-line block. |

Suppression: wrap a region in `// code-verify off` / `// code-verify on` (C++ and QML);
`<!-- doc-verify off -->` / `<!-- doc-verify on -->` (Markdown). Suppressions are a
code-review trigger — fix root cause when possible.

`.code-report` and `.doc-report` are the cleanup checklists. If a rule appears as advisory,
that means the existing codebase has baseline debt — new code should still clear it.

## Repo Skills (Claude Code)

Project-scoped Agent Skills live in `.claude/skills/` and load automatically for anyone running
Claude Code in this repo (committed to git, no install step). Invoke with `/<name>` or let them
auto-trigger. Keep them current when the workflows they encode change.

| Skill | Use it when |
|-------|-------------|
| `ss-hotpath` | Editing/reviewing the data hotpath (`FrameReader`, `CircularBuffer`, `FrameBuilder`, `Dashboard` draw). Auto-activates on those paths. Encodes the SPSC/DirectConnection/slot-pool rules and the `--benchmark-hotpath` 256 kHz check. |
| `ss-new-driver` | Adding a new I/O driver / data source under `app/src/IO/Drivers/`. Encodes the `BluetoothLE` reference pattern and every registration touch-point. |
| `ss-verify` | Before committing, or any "lint / check conventions / sanitize" request. Wraps `code-verify.py` + `sanitize-commit.py`. |
| `qt-cpp-review` | Reviewing/auditing C++ (or "before I commit"). Runs `code-verify.py` as phase 1, then six parallel read-only agents (model contracts, ownership, thread-safety + hotpath, API correctness, error handling, perf). Adapted from The Qt Company's `qt-cpp-review`; read-only. |
| `ss-cpp-modern` | Authoring/refactoring non-trivial C++ and you want the idiomatic modern-C++20 shape (smart-pointer choice, RAII wrapper, concept-constrained template, atomics). Adapted from jeffallan's `cpp-pro`; defers style/build/sanitize to the scripts and the other skills. |
| `cpp-compiler-flags` | Reading/changing the cmake flag modules (`Optimization`/`Hardening`/`Sanitizers`/`MiMalloc`), tuning `-O`/`-march`/LTO/PGO, adding a per-toolchain branch, or explaining a flag. Encodes the repo's real flag layout + invariants (IEEE-stable math, Lua unwind tables, x86-64-v2, two-stage PGO) and a cross-compiler flag catalog. Read-only on the build; never invokes cmake. |

## Project Overview

Serial Studio: cross-platform telemetry dashboard, Qt 6.11.1 + C++20. Data sources: UART,
TCP/UDP, BLE, Audio, Modbus, CAN Bus, MQTT, USB (libusb), HID (hidapi), Process I/O. 15+
visualization widgets, 5 output (control) widgets, 256 kHz+ data rate (CI-gated; see below).
Frame parsers in JavaScript (`QJSEngine`), Lua 5.4 (embedded `lua54`), or Built-In ("Native"
in all internal identifiers — `SerialStudio::Native`, `CFrameParser`, `NativeTemplate`; only
user-facing strings/docs say Built-In. Parametrized C++ templates configured via a JSON
descriptor, no user code). Per-dataset value transforms in JS or Lua. Pro features: Output
widgets, Modbus, CAN Bus, MDF4, 3D, ImageView, Waterfall, file-transfer protocols (X/Y/ZMODEM),
Modbus map importer, Session Database.

## Sub-Documentation

Deep subsystem detail and the silent-breakage lookup live in `doc/claude/`. Read the
relevant doc in full before working in that area — the inline summary below is a pointer,
not a substitute.

| Document | When to read it |
|----------|-----------------|
| [doc/claude/architecture.md](doc/claude/architecture.md) | Before touching any subsystem: full data flow, timestamp ownership, threading rules, AppState, operation modes, IO/driver model, ProjectModel-Editor split, multi-source, JSON `Keys::`, frame parser (JS+Lua), value transforms, data tables, export/Sessions DB, and all plot/time-ring/sweep/waterfall/output-widget internals. |
| [doc/claude/common-mistakes.md](doc/claude/common-mistakes.md) | The silent-breakage lookup table — gotchas the linter can't catch (timestamp capture, queued-vs-direct hotpath, `operator[]` inserts, scope creep, macOS file-dialog reentrancy, etc.). |
| [doc/claude/code-style.md](doc/claude/code-style.md) | Full style spec + NASA Power of Ten: formatting, naming, control flow, C++ headers, signals/connections, comments & Doxygen, QML, performance, licensing. The Code Style block below is the inline essentials — read this for the complete rules. |
| [doc/claude/directory-map.md](doc/claude/directory-map.md) | The `app/src` / `app/qml` / `lib` tree with one-line role notes per subsystem. |

## Threading & Hotpath — Non-Negotiable

The rules most likely to cause silent breakage. Full detail (data flow, threading table,
benchmark mechanics) in [doc/claude/architecture.md](doc/claude/architecture.md); the
`ss-hotpath` skill auto-activates on these paths and re-states them.

- **`FrameReader` and `CircularBuffer` are main-thread / SPSC. Never add mutexes.** Recreate
  via `resetFrameReader()` / `reconfigure()`.
- **Hotpath signal hops must be `Qt::DirectConnection`.** Queued between two main-thread
  objects fills the 4096-slot queue at 10+ kHz and drops frames.
- **No allocation, no Frame copy on the dashboard path.** Draw the Dashboard frame from
  `FrameBuilder::acquireFrame()` (slot pool, aliasing shared_ptr — no per-frame control
  block), never a direct `make_shared<TimestampedFrame>`. The async-sink fan-out in
  `hotpathTxFrame` makes one detached copy on purpose (slow export path, gated on a sink
  being on) so a backlog can't pin the pool.
- **Native + PlainText parses through the span fast lane** (`trySpanLane` →
  `parseUtf8Spans` → `applyDatasetValuesSpans`): byte views + in-place QString writes
  (`assign_utf8_in_place`), zero steady-state allocation. The hotpath reads **cached**
  flags (`m_operationMode`, `m_playerOpen`, `m_anyAsyncSink`, `m_captureLatestFrame`,
  Dashboard `m_streamAvailable`) — a new input to any of them must wire its change signal
  to the cache refresh or frames/exports silently stop (see common-mistakes.md).
  `m_captureLatestFrame` (control script running or API server on) gates the latest-frame
  capture behind `io.getLatestFrame`: it retains one `CapturedDataPtr` per source (the
  FrameReader pool probe skips pinned slots) plus the channel tokens — keep it gated and
  allocation-free.
- **Source owns time.** Stamp at the driver boundary; never re-stamp in export/report
  workers (use `monotonicFrameNs(...)` as the safety net only).
- **JS scripts**: always `IScriptEngine::guardedCall()`, never `parseFunction.call()`.
  `setInterrupted(true)` only in `JsWatchdogThread.cpp`.
- **256 kHz is a CI gate, not a slogan.** `--benchmark-hotpath` (`Benchmark::HotpathBenchmark`)
  drives the real parse pipeline with seven gates tiered off `--min-fps` (default 256000), from
  data pipeline + Native numeric at 4x (1.024 MHz) down to JS mixed at 64 kHz (full tier table
  in the `ss-hotpath` skill); `test.yml` runs it per PR, `deploy.yml` gates the shipped PGO
  binary. Don't regress it.
- **Hotpath optimization macros live in `app/src/DataModel/HotpathOptimization.h`** (`SS_FORCE_INLINE`,
  `SS_FLATTEN`, `SS_HOT`/`SS_COLD`, `SS_RESTRICT`, `SS_ASSUME`, `SS_NO_UNROLL`, ...): cross-toolchain
  spellings with a `__clang__`-first cascade (clang-cl/IntelLLVM take the GNU branch). Annotate the
  `.h` declaration and `.cpp` definition in lockstep. Never add a fast-math / no-unwind / GCC
  `optimize("...")` macro (breaks the IEEE-stable + Lua-unwind invariants). `SS_ASSUME` must restate
  a guard that already ran, never a precondition on a parsed frame. The `datasets+publish` stage is
  ~70-80% of per-frame time — gate any change here with `--benchmark-hotpath`.

## Code Style — Essentials

`scripts/code-verify.py` is the contract — read its `--check` output, don't re-derive the
rules. Full spec (formatting, header order, comments/Doxygen, QML, performance, licensing,
naming table) and the NASA Power of Ten live in
[doc/claude/code-style.md](doc/claude/code-style.md). The handful you need *before* typing,
because they shape the code you write (not just how the linter rewrites it):

- **Format**: 100-col, 2-space indent, LF, pointer/ref binds to type (`int* p`). No braces on
  single-statement bodies; blank line after a brace-free body. Max 3 nesting levels (guard
  clauses); functions 40-80 lines, hard limit 100. Run `clang-format`.
- **Headers (.h)**: `Q_OBJECT` → `Q_PROPERTY` → `signals:` → ctor/deleted copy → `public:`
  (`instance()` first) → `public slots:` → `private slots:` → `private:`, Christmas-tree in
  each block. `[[nodiscard]]` on every non-void return. **Never `Q_INVOKABLE void`** (use
  `public slots:`). **No in-header member init** — ctor init list only.
- **Signals**: `Q_EMIT` not `emit`; lowercase `signals:`/`public slots:`; never
  `SIGNAL()`/`SLOT()`. Never `disconnect(nullptr)` as the slot — capture the `Connection`.
- **Comments**: code is the spec; label, don't narrate. **No comments inside a function body**
  (`cxx-inbody-comment`, advisory) — functions are short, so the one-line `/** @brief ... */`
  above the function plus self-explanatory code carry it; fold a load-bearing *why* into the
  `@brief`, or fence a genuinely-needed note with `// code-verify off`. `//---` concern-group
  banners live *between* functions, never inside one. No inline EOL comments, no AI narration.
  Don't fake the em-dash with ` -- ` — rewrite the sentence.
- **Naming**: `CamelCase` types, `camelCase` functions, `lower_case` locals + public members,
  `s_`/`m_`/`k`/`UPPER_CASE` for static/private/constexpr/macro (full table in the sub-doc).
- **Safety-critical (NASA Power of Ten)** — hotpath violations are blockers. The ones that
  bite: no alloc/Frame-copy on the dashboard path; fixed loop bounds + capped recursion;
  assertion density ≥2/function; `[[nodiscard]]` + return checks at every system boundary;
  zero warnings; no `reinterpret_cast`/`dynamic_cast` on the hotpath; SPDX header per file.
