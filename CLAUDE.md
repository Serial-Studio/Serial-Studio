# CLAUDE.md

## Behavioral Rules

- **Read before writing.** Never edit a file you haven't read this session.
- **Read hotpath code in full** (`FrameBuilder`, `CircularBuffer`, `FrameReader`, `Dashboard`)
  before touching it. **Read `BluetoothLE.h/.cpp`** before writing any new driver — it's the
  canonical reference.
- **Read existing signal/slot wiring** in a file before adding or changing any.
- **Plan before multi-file changes** (>3 files): state the plan, get confirmation. Non-trivial
  or multi-file work runs through spec-driven development (`/ss-spec` → `/ss-plan` → `/ss-tasks`
  → `/ss-implement`); see [doc/claude/spec-driven.md](doc/claude/spec-driven.md).
- **Edit, don't rewrite.** Targeted `Edit` calls; full rewrite only when asked or >70% changed.
- **No preamble, no trailing summary** — except a one-line statement of
  intent before non-trivial work, and one or two sentences naming what
  changed (and what's next) when you stop. Skip both on trivial edits.
  (The Context Canary line below is exempt — it is mandatory on every response.)
- **Do not create markdown/doc files** unless asked. Share info conversationally.
- **Don't build or run the app.** Never invoke `cmake`/`jom`/`clang`/the compiler — the
  developer builds and runs it themselves. Verify changes by reading and with
  `scripts/code-verify.py`; leave compilation and runtime testing to the user.
- **Update CLAUDE.md** for any architectural change that future me would otherwise miss.
- **`scripts/` is the style contract.** When in doubt, run it; don't restate it here.

## Context Canary — Last Line of Every Response

End every response — including one-word answers — with this exact line, reproduced
from memory:

`canary: qt 6.11.1 | cpp20 | hotpath 256k (native 1024k, js 64k) | queue 65536 | api 7777 | style 100/2`

It is a context-health probe in plain ASCII. Each value is a fact the repo's rules
depend on, so a wrong or missing value shows *which* fact was lost — and retyping the
line re-anchors those constants every turn (see the J-Space discipline below). Keep it
unobtrusive: one plain-text line at the very end, nothing else on the line.

- **From memory only.** Never Read/Grep this file or anything else to reconstruct the
  line — a looked-up canary defeats the measurement. If you cannot reproduce it
  confidently, write `canary: lost` instead of guessing: that is the signal firing.
- **Verbatim.** Same values, same order, every turn. Do not paraphrase, reformat,
  extend, or "improve" it.
- **For the developer:** any mutated value, missing segment, or vanished canary means
  the context window is degraded and the session is about to spiral — treat recent
  output as suspect, checkpoint the current step, and `/compact` or restart before
  continuing non-trivial work.

## Trust Contract

These rules are about predictability, not productivity — the difference
between a tool the user re-audits every time and a collaborator they rely
on. Capability without predictability gets disabled.

- **Never touch, revert, or restore files outside your own edits — the one
  rule whose violation loses real work.** A working-tree file *you* did not
  edit this session is the user's in-progress work. NEVER
  `git checkout`/`restore`/`reset`/`stash`/`clean` it, overwrite it, or
  "clean it up" — not even when it looks like noise, a generated artifact,
  or stray subagent output. Session-start `git status` is a snapshot, not a
  baseline to restore to. If such a file is in your way or seems wrong,
  *stop and say so in chat* — quote the path, say you did not touch it,
  ask. Restoring even derived artifacts (`.ts`/`.qm`, build output) needs
  explicit per-file permission — you cannot prove the user wasn't mid-edit.
  When unsure whether a file is yours: it is not. This has bitten before (a
  subagent regenerating translation files; a reflexive restore nearly
  discarding hours of uncommitted work) — absolute, not advisory.
- **Stay in your lane.** Every file touched outside the explicit ask costs
  the reviewer an audit pass. Spot an adjacent fix? *Name it in chat*
  ("noticed X — want it in this pass?") rather than slipping it into the
  diff. Bundled scope creep erodes trust in every diff that follows.
- **Show the why, not the what.** Code shows *what*; a comment, chat reply,
  or commit message shows *why* — but only when the choice was non-obvious
  (one of two reasonable approaches, a workaround, a hidden invariant). One
  sentence. When the choice was obvious, say nothing.
- **State the plan before non-trivial work.** Any change where a reasonable
  reviewer could prefer a different approach: plan visible *before*
  execution is the contract — a summary after is not. Operationalized as
  spec-driven development: non-trivial or multi-file work MUST start with
  `/ss-spec`; no implementation lands before an approved `plan.md`. Trivial
  one-liners exempt. See [doc/claude/spec-driven.md](doc/claude/spec-driven.md).
- **Self-review before handoff.** Before declaring a non-trivial change
  done, re-read the diff: is this *what was asked, and only that*? If you
  can't answer yes, say so before claiming completion.

## Scripts

All scripts in `scripts/` are CWD-independent and write LF endings on every platform. Safe
to run from any directory.

| Script | Role |
|--------|------|
| `sanitize-commit.py` | Top-level driver: chmod (POSIX) → expand-doxygen → clang-format → code-verify --fix → clang-format → code-verify --check → singleton-census gate (blocking) → black → documentation-verify → generate-sdk → generate-command-strings → generate-property-registry (regen + --check + --check-snapshot) → registry-verify → search-index rebuild → changed-file summary. Sanitize only — it never commits or pushes. **Run before every commit.** |
| `code-verify.py` | Structural + tone linter for C++/QML/H. `--fix` rewrites in place; `--check` regenerates `.code-report`. Errors block CI; advisories are baseline-debt cleanup. |
| `documentation-verify.py` | Markdown linter for AI-narration / marketing copy. Read-only; writes `.doc-report`. Targets `README.md`, `AGENTS.md`, `doc/help/**`, `examples/**/README.md` (CLAUDE.md is exempt). |
| `expand-doxygen.py` | Rewrites single-line `/** text */` into the canonical 3-line block. |
| `tu-cutter.py` | Deterministic TU splitter for god-class .cpp files; refuses to cut unless the block parse reconstructs the original exactly (spec 0002 holds the manifests + plan). |
| `registry-verify.py` | Spec-0028/0036 registry lint: icon tree + command manifests + commercial-guard scan of `app/qml/Commands/` + QML icon render-size + property-manifest rules. Run after touching icons, manifests, or bindings; gated in `sanitize-commit.py`. |
| `generate-command-strings.py` | Manifests -> `app/src/UI/CommandStrings.cpp` (lupdate stub, "Commands" context). Hooked into sanitize-commit; `--check` gates drift. |
| `generate-legacy-icons.py` | icon-map.csv -> `Misc::legacyIconPath()` table mapping pre-0028 icon URLs persisted in user project files. Rerun only if the migration manifest changes. |

Suppression: wrap a region in `// code-verify off` / `// code-verify on` (C++ and QML);
`<!-- doc-verify off -->` / `<!-- doc-verify on -->` (Markdown). Suppressions are a
code-review trigger — fix root cause when possible.

`.code-report` and `.doc-report` are the cleanup checklists. If a rule appears as advisory,
that means the existing codebase has baseline debt — new code should still clear it.

## Tests

Python/pytest suite under `tests/`. Full catalog — per-file coverage, fixtures, markers, the
delay/operation-mode tables, the C++ ctest tier + presets, the `--selftest` in-app tier —
lives in [tests/README.md](tests/README.md); read it before writing a test. What binds you:

- **You don't build or run the app, so you can't run the live-API tests.** Integration,
  security, and performance tests drive a running Serial Studio over TCP — they need the app
  up with **Settings → Miscellaneous → Enable API Server** (`localhost:7777`). The user runs
  those.
- **`tests/scripts/` is the exception you *can* run** — pure JS frame-parser unit tests, fresh
  Node.js subprocess per case, no Qt, no app. `pip install -r tests/requirements.txt` once;
  `pytest.ini` registers all markers and a 30 s per-test timeout.
- **C++ units under `app/tests/`** (spec 0032) run via `ctest` and the `CMakePresets.json`
  presets — the maintainer's step, not yours. **`--selftest`** suites run inside
  `CLI::process()` **before** the composition root: never touch an application singleton there.
- The C++ hotpath has no pytest path — throughput is the user-run `--benchmark-hotpath` gate
  (see Threading & Hotpath), piece correctness the ctest tier; `ci.yml` runs both.

```bash
pytest tests/scripts/ -v                  # JS-parser units (Node.js only) — safe for you
pytest tests/integration/ -v              # all integration (needs running app)
pytest tests/ -m "not destructive" -v     # skip server-crashing tests
```

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
| [doc/claude/architecture.md](doc/claude/architecture.md) | Before touching any subsystem: the index into the per-subsystem `doc/claude/architecture/` files — dataflow (hotpath), startup, io, project, scripting, dashboard, export. Read the file(s) for the touched subsystem in full; the index maps what lives where. |
| [doc/claude/common-mistakes.md](doc/claude/common-mistakes.md) | The silent-breakage lookup table — gotchas the linter can't catch (timestamp capture, queued-vs-direct hotpath, `operator[]` inserts, scope creep, macOS file-dialog reentrancy, etc.). |
| [doc/claude/code-style.md](doc/claude/code-style.md) | Full style spec + NASA Power of Ten: formatting, naming, control flow, C++ headers, signals/connections, comments & Doxygen, QML, performance, licensing. The Code Style block below is the inline essentials — read this for the complete rules. |
| [doc/claude/directory-map.md](doc/claude/directory-map.md) | The `app/src` / `app/qml` / `lib` tree with one-line role notes per subsystem. |
| [doc/claude/working-relationship.md](doc/claude/working-relationship.md) | How to collaborate here: recommend don't enumerate, push back when a choice will cost, ground truth outranks on-paper reasoning, surface tradeoffs as decisions, engage the "why." Read once per session if you haven't internalized it. |
| [doc/claude/j-space.md](doc/claude/j-space.md) | The verbalization discipline and its grounding (the Transformer Circuits global-workspace paper): why naming the binding constraints right before an edit works, the six disciplines, and where each is wired into the skills. Read when tuning any AI-facing doc or skill. |
| [doc/claude/repo-skills.md](doc/claude/repo-skills.md) | The project-scoped `/`-skills catalog (`ss-hotpath`, `ss-new-driver`, `ss-verify`, `qt-cpp-review`, `ss-cpp-modern`, `cpp-compiler-flags`, `ss-docs`, and the `ss-spec`/`ss-plan`/`ss-tasks`/`ss-implement` workflow) and when each fires. Most auto-activate; this is the lookup when picking one deliberately. |
| [doc/claude/spec-driven.md](doc/claude/spec-driven.md) | Before any non-trivial or multi-file feature: the default workflow. The four gated phases (`/ss-spec` → `/ss-plan` → `/ss-tasks` → `/ss-implement`), where artifacts live (`doc/claude/specs/NNNN-slug/`), the gate discipline, when to skip, and how it composes with the hotpath/verify/trust rules. |

## J-Space Discipline — Verbalize the Binding Constraints

Deliberate reasoning runs on a small set of verbalized concepts; familiar-shaped work runs
on autopilot and bypasses it ([doc/claude/j-space.md](doc/claude/j-space.md)). The repo's
rules only steer an edit if *named at the point of action*, so:

- **Name before acting.** Before any edit on a protected path (hotpath, ctor closure,
  signal wiring, cmake flag modules), state in chat the 3-5 invariants that bind *this*
  change — in your own words, not a doc citation.
- **Few, late, specific.** Only the constraints that bind the change at hand; never recite
  whole rule files.
- **Counterfactual check at handoff.** Which rule does this diff most risk violating, and
  what concrete evidence says it doesn't? Name both.
- **Diverge by naming.** Design/review work sketches named alternatives before recommending
  (the human still gets one recommendation, per working-relationship.md).
- **Externalize long state.** Write intermediate state into durable artifacts (spec/plan/
  tasks files, a chat checklist); re-name only what binds the current edit.

## Threading & Hotpath — Non-Negotiable

The rules most likely to cause silent breakage. Full detail (data flow, threading table,
cached flags, benchmark mechanics) in
[doc/claude/architecture/dataflow.md](doc/claude/architecture/dataflow.md); the
`ss-hotpath` skill auto-activates on these paths and re-states them.

- **`FrameReader` and `CircularBuffer` are main-thread / SPSC. Never add mutexes.** Recreate
  via `resetFrameReader()` / `reconfigure()`.
- **Hotpath signal hops must be `Qt::DirectConnection`.** Queued between two main-thread
  objects fills the 65536-slot queue at 10+ kHz and drops frames.
- **No allocation, no Frame copy on the dashboard path.** Draw the Dashboard frame from
  `FrameBuilder::acquireFrame()` (slot pool, aliasing shared_ptr), never a direct
  `make_shared<TimestampedFrame>`. The `hotpathTxFrame` async-sink fan-out makes one detached
  copy on purpose (slow export path, gated on a sink being on) so a backlog can't pin the pool.
- **Native + PlainText parses through the span fast lane** (`trySpanLane` →
  `parseUtf8Spans` → `applyDatasetValuesSpans`): byte views + in-place QString writes,
  zero steady-state allocation. The hotpath reads **cached** flags (`m_operationMode`,
  `m_playerOpen`, `m_anyAsyncSink`, `m_captureLatestFrame`, `m_changeDriven`, Dashboard
  `m_streamAvailable`) — a new input to any of them must wire its change signal to the cache
  refresh or frames/exports silently stop. Flag mechanics: dataflow.md "Cached Hotpath
  Flags", read before touching any of them. `streamAvailable()` also reads the spec-0040
  mirror flag (`API::MirrorSession::mirroring()`, a plain module-static bool — never a
  construction; see [doc/claude/architecture/mirror.md](doc/claude/architecture/mirror.md)).
- **Source owns time.** Stamp at the driver boundary; never re-stamp in export/report
  workers (use `monotonicFrameNs(...)` as the safety net only).
- **Driver opens are synchronous.** `DeviceManager::open()` calls `HAL_Driver::open(mode)`
  directly; there is no async-open hook on `HAL_Driver` and no per-device task runner (the
  spec-0034 `IO::ConnectionFlows` layer was removed 2026-07-30). Drop recovery is per-driver
  again: UART polls `m_pendingReconnect` off the 1 Hz tick, MQTT/Modbus/Network schedule their
  own. The task-tree engine `app/src/Async/` (`TaskTree`, `RetryPolicy`, `AsyncClock`) stays,
  used only by `MQTT::Publisher` and the spec-0035 diagnostics probes — thread-affine, no
  mutex, no new thread, nothing per frame; retry/backoff still declared **once** in
  `RetryPolicy::initialConnect()` / `autoReconnect()`.
  See [doc/claude/architecture/io.md](doc/claude/architecture/io.md).
- **Diagnostics are pulled, never pushed (specs 0033/0035).** `FrameReader` / `FrameBuilder`
  counters are plain `quint64` increments polled on the 1 Hz tick — never signal, allocate,
  or lock per frame. A recreated `FrameReader` zeroes them: consumers work on deltas, treat a
  decrease as a reset. `Misc::ConnectionDiagnostics` reports through `Misc::ProblemCenter`
  under the same rule: checkers return synchronously, nothing per frame.
- **JS scripts**: always `JsScriptEngine::guardedCall()`, never `parseFunction.call()`.
  `setInterrupted(true)` only in `JsWatchdogThread.cpp`.
- **256 kHz is a CI gate, not a slogan.** `--benchmark-hotpath` drives the real parse pipeline
  with nine gates tiered off `--min-fps` (default 256000), from Native numeric at 4x
  (1.024 MHz) down to JS mixed at 64 kHz, plus 0.5x consumer-path floors (full tier table in
  the `ss-hotpath` skill); `ci.yml` runs it per push/PR as a hard gate on the PGO-optimized
  binary. Don't regress it.
- **Portable SIMD kernels live in `app/src/DSPSimd.h`** (`namespace DSP`, spec 0021): x86-64-v2
  + NEON lanes + reference scalar fallback, per-lane bit-exact versus the scalar loop (full
  contract in the header). New bulk loops reuse these — never inline intrinsics at call sites.
- **Hotpath optimization macros live in `app/src/DataModel/HotpathOptimization.h`**
  (`SS_FORCE_INLINE`, `SS_FLATTEN`, `SS_HOT`/`SS_COLD`, `SS_RESTRICT`, `SS_ASSUME`, ...); the
  header documents the toolchain cascade. Annotate `.h` declaration and `.cpp` definition in
  lockstep. Never add a fast-math / no-unwind / GCC `optimize("...")` macro (breaks the
  IEEE-stable + Lua-unwind invariants). `SS_ASSUME` must restate a guard that already ran,
  never a precondition on a parsed frame. `datasets+publish` is ~70-80% of per-frame time —
  gate any change with `--benchmark-hotpath`.

## Startup & Composition Root — Non-Negotiable

- **`ModuleManager::instantiateCoreModules()` pins singleton construction order** (ProjectModel
  before AppState, Dashboard last). Never reorder or add entries without re-running the ctor-edge
  proof in [doc/claude/specs/0001-composition-root/](doc/claude/specs/0001-composition-root/).
- **ProjectModel's ctor closure is a protected surface** (`newJsonFile`, `watchProjectFile`,
  `scheduleAutoSave`, the `ControlScript::setCode` chain): it runs before AppState/Dashboard
  exist; calling their `instance()` there recurses the Meyers guard and aborts — shipped and
  crashed once (2026-07-07). Gate new code on `m_initialized`
  (see [doc/claude/architecture/startup.md](doc/claude/architecture/startup.md)).
- **A ctor-edge proof dies when ctor-reachable code changes.** Any edit inside that closure
  re-triggers the check, no matter how unrelated the edit looks.
- **`SessionContext` (spec 0039) owns the eight core modules** as `unique_ptr` slots adopted
  inside `instantiateCoreModules()`. Ctor/dtor stay empty (a constructing ctor re-enters the
  Meyers guard and aborts); adopted addresses never change; `shutdown()` (from `main.cpp`
  while `qApp` is alive) releases in exact reverse pinned order. **Never call
  `SessionContext::current()` from a method body** — composition root and `instance()`
  forwarders only; the singleton census (`code-verify.py --singleton-census --check`) fails
  on any increase. Full contract:
  [doc/claude/architecture/startup.md](doc/claude/architecture/startup.md).
- **License-gated state must exist before `restoreLastProject()` or re-derive on
  `activatedChanged`.** The licensing block (MachineID, LemonSqueezy, OfflineLicense, Trial)
  is the FIRST thing `instantiateCoreModules()` builds after Translator (spec 0042): their
  ctors install the CommercialToken, so entitlement is final-for-startup before any consumer
  constructs. Anything baking `proWidgetsEnabled()` into derived state at load time still
  needs a `LemonSqueezy::activatedChanged` hook (Trial/Offline transitions funnel into it;
  consumer inventory: `doc/claude/specs/0042-license-token-hardening/consumers.md`), or
  late/async activation ships fallback widgets (2026-07-09: Plot3D degraded to MultiPlot).

## Project Layout — the god files are split

`ProjectModel` / `ProjectEditor` implementations live across per-concern TUs in
`app/src/DataModel/Project/`; `ProjectHandler` across `API/Handlers/ProjectHandler{File,
Entities,Parser,Batch}.cpp` (registration stays in `ProjectHandler.cpp`). Facade headers
unchanged — QML/API contracts intact. Map in
[doc/claude/directory-map.md](doc/claude/directory-map.md); splitter: `scripts/tu-cutter.py`.

## Project Undo History (spec 0031)

Full detail: [doc/claude/architecture/project.md](doc/claude/architecture/project.md)
"Undo History" (read before touching any `ProjectModel` mutator).

- **`DataModel::ProjectHistory`**: scoped whole-document mementos, two-phase —
  `ProjectUndoScope` **stages** a snapshot, the first `setModified(true)` **commits** it. A
  guard-returning slot or a mutator that never calls `setModified(true)` silently records
  nothing.
- **Every new document-mutating `ProjectModel` slot opens a `ProjectUndoScope`**
  (`undo-scope-missing` lint; Editor TUs excluded, workspace CRUD + presentation setters
  whitelisted). Composites wrap in one `ProjectUndoFrame`; dialog-showing slots open their
  scope *after* the dialog.
- **The apply path never emits `jsonFileChanged`**; `project.undo`/`project.redo` never error
  on empty history.

## Dataset Property Registry & Generated API Surfaces (specs 0036, 0037)

Full detail: [doc/claude/architecture/project.md](doc/claude/architecture/project.md)
(read before adding a dataset property or touching any API-surface generator).

- **One declaration**: `app/rcc/properties/dataset.json`. `generate-property-registry.py`
  emits six checked-in artifacts from it (C++ TUs under `*/Generated/`, the gRPC field-number
  ledger `proto-fields.json`, the typed `.proto`). **Never hand-edit a generated file**; edit
  the manifest and rerun.
- **`schema_props_for()` is the only definition of a property's API schema.** Never re-derive
  it anywhere else.
- **gRPC field numbers are append-only released state.** Removing a parameter retires its
  number to `reserved`; moving one is `code-verify`'s `proto-field-renumbered` error.
  `api-schema.json` is maintainer-dumped; `--check-snapshot` warns locally, **fails in CI**.
- **Gates** (`generate-property-registry.py --check`/`--check-snapshot`, `generate-sdk.py
  --check`, `generate-command-strings.py --check`, `registry-verify.py`, `code-verify.py`)
  all run in `sanitize-commit.py` and the CI `lint` job.

## Icon & Command Registry (spec 0028)

Full detail + recipes: [doc/claude/architecture/commands-icons.md](doc/claude/architecture/commands-icons.md)
(read before adding a toolbar button, palette entry, menu item, shortcut, or fixed icon).

- **Icons**: `qrc:/icons/<category>/<tier>/<name>.svg` (tiers 16/24/32/48; `buttons/`
  exempt). Resolve via `Misc::IconRegistry` — QML `Cpp_Misc_IconRegistry.icon(cat, name, px)`
  / `iconById("cat/name", px)`, C++ `iconPath()` for QPixmap/QIcon. Nearest tier
  at-or-above px; unknown ids warn once and serve `system/16/missing.svg`. Never hardcode a
  path. Old URLs in saved projects remap via `Misc::legacyIconPath()`.
- **Commands**: metadata declared once in `app/rcc/commands/*.json` (+ layout manifests in
  `layouts/`), behavior bound per context in `app/qml/Commands/*CommandBindings.qml`, joined
  by `CommandModel.qml`, rendered by `Widgets/CommandToolbar.qml`; loaded by
  `UI::CommandRegistry` (`Cpp_UI_CommandRegistry`). **New command = one manifest entry + one
  bindings entry**; palette, Start menu, toolbars, shortcuts follow. A command shows in a
  palette only if its `contexts` includes that palette AND the context's model has a binding
  for it (binding-set order `[app,dashboard]` main / `[dashboard,app]` dashboard). Commercial
  bindings need a `Cpp_CommercialBuild` guard; run `scripts/registry-verify.py`.

## Widget Extensions (spec 0038)

Installable dashboard widgets (`UI::WidgetExtensions`): third-party packages resolve to
`DashboardExtension = 100` and persist under `"ext:<id>"`, bundled conversions keep their builtin
enum via `replaces`, `readsStringValues` is what registers a package in `string_targets`, and the
trust model is consent, not containment — never call an extension sandboxed. Read
[doc/claude/architecture/dashboard.md](doc/claude/architecture/dashboard.md) before touching it.

## Remote Dashboard Mirror (spec 0040)

One instance streams its dashboard to another over the API socket (NDJSON, top-level
`"mirror"` key; FNV-1a-64 layout hash guards positional snapshots — any change to dataset
ordering or `wireUniqueId` is a wire break: bump `kWireVersion`, regenerate
`tests/fixtures/mirror/`). Publisher wakes on the display tick only while subscribed (never
the frame path); viewer injects via `Dashboard::hotpathRxFrame` with
`structureGeneration >= 1<<48`, never reaches the export fan-out.
`ConnectionState::streamFrames` defaults `true`; only `mirror.subscribe` may flip it. Read
[doc/claude/architecture/mirror.md](doc/claude/architecture/mirror.md) before touching
`app/src/API/Mirror/` or `streamAvailable()`.

## Code Style — Essentials

`scripts/code-verify.py` is the contract — read its `--check` output, don't re-derive the
rules. Full spec and the NASA Power of Ten live in
[doc/claude/code-style.md](doc/claude/code-style.md). The handful you need *before* typing:

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
  zero warnings; no `reinterpret_cast`/`dynamic_cast` on the hotpath; SPDX header per file —
  first-party is `GPL-3.0-or-later` (relicensed from `-only`, 2026-07). The repo is
  REUSE-compliant: `REUSE.toml` + `LICENSES/`, `reuse lint` gates in CI.
- **Assertions are `SS_ASSERT(cond, action)`** (`app/src/SSAssert.h`), not `Q_ASSERT`: the
  condition evaluates in **every** build; debug aborts, release reports once per site and
  runs the recovery `action` instead of the guarded code. Condition side-effect-free and
  cheap; action side-effect-complete, single statement, no top-level comma, never
  `continue`/`break` (use `SS_ASSERT_LOG(cond); if (!(cond)) continue;`). On the
  per-frame/per-cell kernels use `SS_ASSERT_HOTPATH(cond)` instead — compiles out of release
  (even the pass-path branch is measurable at rate; the 2026-07 wholesale swap cost ~5%
  throughput), admissible only where the condition restates a guard that provably already
  ran, never on device bytes; the blocking `hotpath-assert-scope` lint pins it to the
  hotpath TUs. `SS_ASSUME` stays the zero-branch kernel spelling.
