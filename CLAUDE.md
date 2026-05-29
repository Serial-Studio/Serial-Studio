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

These rules govern the human-agent relationship in this repo. They are about
predictability, not productivity — the difference between a tool the user
has to re-audit every time and a collaborator they can rely on. Capability
without predictability gets disabled.

- **Stay in your lane.** Every file you touch outside the explicit ask costs
  the reviewer an audit pass. If you spot an adjacent fix, *name it in chat*
  ("noticed X — want it in this pass?") instead of slipping it into the
  diff. Bundled scope creep — even when each individual change is correct
  — erodes trust in every diff that follows.
- **Show the why, not the what.** Code shows *what*. A short comment, the
  chat reply, or the commit message shows *why* — but only when the choice
  was non-obvious (picked one of two reasonable approaches, worked around a
  known bug, satisfied a hidden invariant). One sentence is enough. When
  the choice was obvious, say nothing — silence is the default.
- **State the plan before non-trivial work.** Not just multi-file: any
  change where a reasonable reviewer could prefer a different approach.
  State the plan, get a nod, then execute. "Plan visible before execution"
  is the contract — a summary *after* the fact is not a substitute.
- **Self-review before handoff.** Before declaring a non-trivial change
  done, re-read the diff and ask: is what I did *what was asked, and only
  that*? If you can't answer yes, say so before claiming completion.

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

## Project Overview

Serial Studio: cross-platform telemetry dashboard, Qt 6.11.1 + C++20. Data sources: UART,
TCP/UDP, BLE, Audio, Modbus, CAN Bus, MQTT, USB (libusb), HID (hidapi), Process I/O. 15+
visualization widgets, 6 output (control) widgets, 256 KHz+ target data rate. Frame parsers
in JavaScript (`QJSEngine`) or Lua 5.4 (embedded `lua54`). Per-dataset value transforms in
either language. Pro features: Output widgets, Modbus, CAN Bus, MDF4, 3D, ImageView,
Waterfall, file-transfer protocols (X/Y/ZMODEM), Modbus map importer, Session Database.

## Directory Structure

```
app/src/
├── IO/              ConnectionManager, DeviceManager, CircularBuffer, FrameReader, FrameConfig
│   ├── Drivers/     UART, Network, BluetoothLE, Audio, CANBus, HID, Modbus, Process, USB
│   └── FileTransmission/  Protocol base, XMODEM, YMODEM, ZMODEM, CRC utilities
├── DataModel/       Frame, FrameBuilder, FrameConsumer, DataTable(Store), ExportSchema,
│   │                ProjectModel, ProjectEditor, NotificationCenter
│   ├── Scripting/   IScriptEngine, FrameParser, JsScriptEngine, JsWatchdog,
│   │                LuaScriptEngine, LuaCompat, ScriptTemplates
│   ├── Editors/     JsCodeEditor, OutputCodeEditor, PainterCodeEditor,
│   │                DatasetTransformEditor, CodeFormatter
│   ├── Importers/   DBCImporter, ModbusMapImporter, ProtoImporter
│   └── Dialogs/     FrameParserTestDialog, TransmitTestDialog
├── UI/              Dashboard, Taskbar (workspaces), visualization + output widget types
│   └── Widgets/Output/  Button, Toggle, Slider, TextField, Panel (+ PanelLayout), Base
├── API/             TCP server port 7777 (MCP + legacy JSON-RPC), 25 handlers
├── Console/         Terminal + export
├── CSV/ MDF4/       File playback & export
├── Sessions/  (Pro) DatabaseManager + SQLite::Export + SQLite::Player
├── MQTT/            Publisher (FrameConsumer-based, threaded, rate-limited 1-30 Hz)
├── Licensing/       LemonSqueezy, Trial, MachineID, CommercialToken (FeatureTier)
├── Platform/        CSD, NativeWindow
├── Misc/            JsonValidator, ThemeManager, ModuleManager
├── AppState.h       Singleton: OperationMode, projectFilePath, FrameConfig
├── SerialStudio.h   Central enums (BusType, OperationMode, FrameDetection)
└── Concepts.h       C++20 concepts
app/qml/             DatabaseExplorer/, MainWindow/, ProjectEditor/, Widgets/, Dialogs/
lib/                 KissFFT, QCodeEditor, mdflib, OpenSSL, lua54, QuaZip, hidapi, QSimpleUpdater
```

## Sub-Documentation

Deep subsystem detail and the silent-breakage lookup live in `docs/claude/`. Read the
relevant doc in full before working in that area — the inline summary below is a pointer,
not a substitute.

| Document | When to read it |
|----------|-----------------|
| [docs/claude/architecture.md](docs/claude/architecture.md) | Before touching any subsystem: full data flow, timestamp ownership, threading rules, AppState, operation modes, IO/driver model, ProjectModel-Editor split, multi-source, JSON `Keys::`, frame parser (JS+Lua), value transforms, data tables, export/Sessions DB, and all plot/time-ring/sweep/waterfall/output-widget internals. |
| [docs/claude/common-mistakes.md](docs/claude/common-mistakes.md) | The silent-breakage lookup table — gotchas the linter can't catch (timestamp capture, queued-vs-direct hotpath, `operator[]` inserts, scope creep, macOS file-dialog reentrancy, etc.). |

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

## Code Style

`scripts/code-verify.py` enforces this — read its `--check` output, don't re-derive the rules.

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
`//---` banners for concern groups. In-body comments: one-line `//` section headers above a
logical block, only when not self-evident. **Forbidden**: inline EOL comments, multi-line
`//` prose, `/* ... */` inside function bodies, restating the code, AI narration ("we",
"Note that", tutorial voice, "this used to...", hedging, bare `TODO`).

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
