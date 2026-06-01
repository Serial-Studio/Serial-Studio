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
| `sanitize-commit.py` | Top-level driver: chmod (POSIX) → expand-doxygen → clang-format → code-verify --fix → clang-format → code-verify --check → documentation-verify → search-index rebuild → conventional-commit prompt. **Run before every commit.** |
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

## Project Overview

Serial Studio: cross-platform telemetry dashboard, Qt 6.11.1 + C++20. Data sources: UART,
TCP/UDP, BLE, Audio, Modbus, CAN Bus, MQTT, USB (libusb), HID (hidapi), Process I/O. 15+
visualization widgets, 5 output (control) widgets, 256 KHz+ data rate (CI-gated; see below). Frame parsers
in JavaScript (`QJSEngine`) or Lua 5.4 (embedded `lua54`). Per-dataset value transforms in
either language. Pro features: Output widgets, Modbus, CAN Bus, MDF4, 3D, ImageView,
Waterfall, file-transfer protocols (X/Y/ZMODEM), Modbus map importer, Session Database.

## Sub-Documentation

Deep subsystem detail and the silent-breakage lookup live in `docs/claude/`. Read the
relevant doc in full before working in that area — the inline summary below is a pointer,
not a substitute.

| Document | When to read it |
|----------|-----------------|
| [docs/claude/architecture.md](docs/claude/architecture.md) | Before touching any subsystem: full data flow, timestamp ownership, threading rules, AppState, operation modes, IO/driver model, ProjectModel-Editor split, multi-source, JSON `Keys::`, frame parser (JS+Lua), value transforms, data tables, export/Sessions DB, and all plot/time-ring/sweep/waterfall/output-widget internals. |
| [docs/claude/common-mistakes.md](docs/claude/common-mistakes.md) | The silent-breakage lookup table — gotchas the linter can't catch (timestamp capture, queued-vs-direct hotpath, `operator[]` inserts, scope creep, macOS file-dialog reentrancy, etc.). |
| [docs/claude/code-style.md](docs/claude/code-style.md) | Full style spec + NASA Power of Ten: formatting, naming, control flow, C++ headers, signals/connections, comments & Doxygen, QML, performance, licensing. The Code Style block below is the inline essentials — read this for the complete rules. |
| [docs/claude/directory-map.md](docs/claude/directory-map.md) | The `app/src` / `app/qml` / `lib` tree with one-line role notes per subsystem. |

## Threading & Hotpath — Non-Negotiable

The rules most likely to cause silent breakage. Full detail in
[docs/claude/architecture.md](docs/claude/architecture.md).

- **Data flow**: Driver → `FrameReader::processData` (main) → `DeviceManager::onReadyRead`
  → `ConnectionManager::onFrameReady` → `FrameBuilder` → shared `TimestampedFramePtr` to
  Dashboard / CSV / MDF4 / API / Sessions.
- **`FrameReader` and `CircularBuffer` are main-thread / SPSC. Never add mutexes.** Recreate
  via `resetFrameReader()` / `reconfigure()`.
- **Hotpath signal hops must be `Qt::DirectConnection`.** Queued between two main-thread
  objects fills the 4096-slot queue at 10+ kHz and drops frames.
- **No allocation, no Frame copy on the dashboard path.** Draw frames from
  `FrameBuilder::acquireFrame()` (slot pool) — never direct `make_shared<TimestampedFrame>`.
- **Source owns time.** Stamp at the driver boundary; never re-stamp in export/report
  workers (use `monotonicFrameNs(...)` as the safety net only).
- **JS scripts**: always `IScriptEngine::guardedCall()`, never `parseFunction.call()`.
  `setInterrupted(true)` only in `JsWatchdogThread.cpp`.
- **256 kHz is a CI gate, not a slogan.** `--benchmark-hotpath` (`Misc::HotpathBenchmark`) drives the
  real parse pipeline in-process — `FrameReader` extraction → `FrameBuilder` → the **Lua** frame parser
  → per-dataset transforms → Dashboard — against a project loaded programmatically via
  `ProjectModel::loadFromJsonDocument`. It exits nonzero when the **Lua** run sustains below `--min-fps`
  (default 256000), then runs an ungated **JS** pipeline and an ungated **Lua + all exporters live**
  pipeline (CSV/MDF4/Sessions/API/gRPC) for PGO training + an exporter-slowdown readout. The gated runs
  disable the `FrameBuilder` parse-budget guard (an interactive 80%-duty throttle that a 100%-duty
  benchmark would trip every window) via `setParseBudgetEnabled(false)` and run **no** exporters, so the
  gate measures pure parse capacity; the exporter phase is deliberately *not* gated (its `FrameConsumer`
  worker threads can't drain faster than a flat-out producer, so the 1024-slot pool exhausts into the
  heap-fallback path — that penalty is the point of the readout). Each run lasts until both the
  `--benchmark-frames` floor (default 1M) and the `--benchmark-seconds` window (default 10) are met.
  Throughput = `FrameBuilder::parsedFrameCount()` / elapsed. `test.yml` runs it per PR; `deploy.yml`
  gates the shipped PGO binary on it. Don't regress the parse hotpath.

## Code Style — Essentials

`scripts/code-verify.py` enforces this — read its `--check` output, don't re-derive the rules.
Full spec (formatting, headers, comments/Doxygen, QML, performance, licensing) and the NASA
Power of Ten in [docs/claude/code-style.md](docs/claude/code-style.md). The hard rules you
need *before* writing:

- **Format**: 100-col, 2-space indent, LF, pointer/ref binds to type (`int* p`). Run
  `clang-format`. No braces on single-statement bodies; blank line after a brace-free body.
- **Control flow**: max 3 nesting levels (guard clauses, early return/continue). Functions
  40-80 lines, hard limit 100.
- **Headers (.h)**: order `Q_OBJECT` → `Q_PROPERTY` → `signals:` → ctor/deleted copy → `public:`
  (`instance()` first) → `public slots:` → `private slots:` → `private:`. `[[nodiscard]]` on every
  non-void return. **Never `Q_INVOKABLE void`** (use `public slots:`). **No in-header member
  init** — use the ctor init list. Christmas-tree (shortest-line-first) within each block.
- **Signals**: `Q_EMIT` not `emit`; lowercase `signals:`/`public slots:`. Never `SIGNAL()`/`SLOT()`.
  Never `disconnect(nullptr)` as the slot — capture and disconnect the `QMetaObject::Connection`.
- **Comments**: code is the spec; label, don't narrate. `.h` allows only the SPDX banner and a
  one-line `/** @brief */` per type. `.cpp` gets a one-line `@brief` per function definition. No
  inline EOL comments, no AI narration. Don't fake the em-dash with ` -- ` — rewrite the sentence.
- **QML**: Christmas-tree property order (`id` first); enums via `SerialStudio.BusType` /
  `ProjectModel.SomeEnum`, never integers; `font: Cpp_Misc_CommonFonts.uiFont`.
- **Naming**: `CamelCase` types/enums, `camelCase` functions, `lower_case`
  locals + public/protected members, `s_lower_case` statics, `m_camelCase`
  private members, `kCamelCase` constants/constexpr, `UPPER_CASE` macros.
  Full table → [docs/claude/code-style.md](docs/claude/code-style.md).

- **Safety-critical (NASA Power of Ten)** — hotpath violations are blockers; full list in the
  sub-doc. The ones that bite: no allocation/Frame-copy on the dashboard path (use
  `FrameBuilder::acquireFrame()` slot pool); fixed loop bounds + capped recursion; assertion
  density ≥2 per function; `[[nodiscard]]` + return checks at every system boundary; zero
  warnings; no `reinterpret_cast`/`dynamic_cast` on the hotpath; SPDX header on every file.
