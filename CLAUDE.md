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
- **Work on master, in the main checkout.** No branches and no worktrees, yours or a subagent's
  (`isolation: "worktree"`): the maintainer builds and tests what master holds, and
  `.claude/settings.json` denies `git branch`, `git worktree add` and `EnterWorktree`. Commit as
  soon as a change verifies: ONE commit per change (spec, code, tests, docs together), one
  `git add` plus `git commit -m` with the `Co-Authored-By` trailer. The commit hooks lint the
  staged files; a whole-tree `sanitize-commit.py` is for generator inputs and before a push,
  never a per-commit tax.
- **No preamble, no trailing summary** — except a one-line statement of
  intent before non-trivial work, and one or two sentences naming what
  changed (and what's next) when you stop. Skip both on trivial edits.
  (The Context Canary line below is exempt — it is mandatory on every response.)
- **Do not create markdown/doc files** unless asked. Share info conversationally.
- **Never build — but always syntax-check.** Never invoke `cmake`/`jom` or produce build
  artifacts — the developer builds the app themselves. The ONE sanctioned compiler use
  (2026-10-04) is `scripts/syntax-check.py` (clang `-fsyntax-only` via the newest compile
  database under `build/`): run it on every C++ file you edit, every time — lint checks shape, only the compiler checks truth, and every
  AI batch-edit failure of 2026-10 was compiler-class and lint-invisible. A hook runs it
  per edit; a SKIP (stale database) means the claim "it compiles" is unverified — say so.
- **Runtime experiments are sanctioned.** Ground truth beats on-paper reasoning: drive a
  *running* app through the API server (`localhost:7777`, `tests/utils/api_client.py`) to
  probe hypotheses, not only to verify fixes; run `ctest` and already-built binaries
  (`--selftest`, `--benchmark-hotpath`) against an existing build dir; prototype ideas as
  throwaway Node/Python sims in the scratchpad. Launching the GUI app and compiling stay
  the user's.
- **Sample the running app before theorizing about a GUI stall.** Never reason a freeze, stutter or ignored resize out of the source: `sample <pid>` in the
  background while the user reproduces. Recipe, stack signatures and the 2026-08-13 incident:
  [doc/claude/common-mistakes.md](doc/claude/common-mistakes.md) "Diagnosing a GUI Stall".
- **Update CLAUDE.md** for any architectural change that future me would otherwise miss.
- **`scripts/` is the style contract.** When in doubt, run it; don't restate it here.

## Context Canary — Last Line of Every Response

End every response — including one-word answers — with this exact line, reproduced
from memory:

`canary: qt 6.11.2 | cpp20 | hotpath 256k (native 1024k, js 64k) | queue 65536 | api 7777 | style 100/2`

One plain-text line at the very end.

- **From the prompt only.** Never Read/Grep this file or anything else to reconstruct the
  line. If you cannot reproduce it confidently, write `canary: lost` instead of guessing.
- **Verbatim.** Same values, same order, every turn.

What it measures and what a mutated canary means: [j-space.md](doc/claude/j-space.md).

## Trust Contract

Predictability, not productivity — the difference between a tool the user re-audits every
time and a collaborator they rely on. Capability without predictability gets disabled. Full
text and the incidents behind each rule:
[doc/claude/trust-contract.md](doc/claude/trust-contract.md).

- **Never touch, revert, or restore files outside your own edits — the one rule whose
  violation loses real work.** A working-tree file *you* did not edit this session is the
  user's in-progress work: never `git checkout`/`restore`/`reset`/`stash`/`clean` it,
  overwrite it, or "clean it up". Derived artifacts (`.ts`/`.qm`, build output) included.
  In the way or seems wrong? Quote the path in chat and ask. When unsure whether a file is
  yours: it is not.
- **Stay in your lane.** Spot an adjacent fix? Name it in chat ("noticed X — want it in this
  pass?") rather than slipping it into the diff.
- **Show the why, not the what.** Comment/reply/commit message explains *why*, and only when
  the choice was non-obvious. One sentence.
- **State the plan before non-trivial work** — visible *before* execution, not a summary
  after. Operationalized as `/ss-spec`, whose premise check measures the one number that
  would kill the spec. `scripts/commit-shape.py` refuses a protected surface staged with other
  code and bulk without a `Mechanical:` trailer. Never cite the archive as proof of gating.
- **Self-review before handoff.** Re-read the diff: is this *what was asked, and only that*?
- **Security-sensitive surfaces are decision-visible.** `app/src/Licensing/`,
  `core/Core/Licensing/`, `MachineID`, and the API Server auth/consent files: any
  *behavioral* change there is named in chat with its failure modes BEFORE it lands, and
  never rides inside a bulk/agent package (the 2026-09-03 stored-fingerprint inversion
  shipped a seat-cloning hole into master for a month exactly that way). The checklist is
  [doc/claude/specs/0042-license-token-hardening/consumers.md](doc/claude/specs/0042-license-token-hardening/consumers.md).

## Delegation & Cost — Advisor Economics

Quality concentrates in planning and review: the main session advises, cheaper executors type.
The full rules and their measurements: [doc/claude/delegation.md](doc/claude/delegation.md).

- **The main session plans, reviews, and integrates; mechanical execution is delegated** to
  subagents on a cheaper model (Sonnet by default, Haiku for purely mechanical work). Design
  decisions, hotpath edits, and anything on a protected surface stay in the main session. An
  executor brief names a donor file whose pattern the output matches.
- **Searches fan out, conclusions come back.** A line number a subagent reports is a hint:
  re-read the slice before editing it. A whole-file read over 800 lines is refused and redirected,
  the read-in-full hotpath files excepted. Enforced: hook:large-read-gate.py.
- **No process tax on trivial work.** Keep the "skip for trivial" escape hatches.

## Scripts

All scripts in `scripts/` are CWD-independent and write LF endings; run from anywhere. The
per-script table lives in [doc/claude/scripts.md](doc/claude/scripts.md). What binds you:
the commit hooks run `code-verify.py --check` and the pinned clang-format over the staged
files, so a commit needs no pre-run; **run `sanitize-commit.py`** (sanitize only — it never
commits or pushes) when you touched a generator input (`api-schema.json`, `dataset.json`, a
command manifest, `command_safety.json`, doc/help) and before a push, never per commit;
`code-verify.py` is the structural linter (`--fix` rewrites, `--check` regenerates
`.code-report`; errors block CI). Suppression: `// code-verify off` / `// code-verify on`
(C++/QML), `<!-- doc-verify off -->` / `<!-- doc-verify on -->` (Markdown) — suppressions
are a code-review trigger.

Growth ratchets (the singleton and TU censuses, `claim-verify.py`, `ledger-verify.py`,
`layer-verify.py`) re-seed with `--accept`; the gate list, baselines, fences and anchor
mechanics live in [doc/claude/scripts.md](doc/claude/scripts.md).

## Tests

Python/pytest suite under `tests/`. The full catalog and these rules in full live in
[tests/README.md](tests/README.md); read it before writing a test. What binds you:

- **You don't build or launch the app, but you may drive a running one**, once the user has it
  up with the API Server enabled (`localhost:7777`). Probe first: `nc -z 127.0.0.1 7777`.
- **`tests/scripts/` and `scripts/tests/` are yours to run**: JS frame-parser units with no app,
  and the tooling tier.
- **C++ units under `app/tests/`** run via `ctest` against an existing build dir (never configure
  or compile). **`--selftest`** suites run **before** the composition root: never touch an
  application singleton there.

```bash
pytest tests/scripts/ -v                  # JS-parser units (Node.js only) — safe for you
pytest tests/integration/ -v              # all integration (needs running app)
pytest tests/ -m "not destructive" -v     # skip server-crashing tests
```

## Project Overview

Serial Studio: cross-platform telemetry dashboard, Qt 6.11.2 + C++20, 15+ visualization
widgets, 5 output (control) widgets, 256 kHz+ data rate (CI-gated; see below). Frame parsers in
JavaScript, Lua, or Built-In ("Native" in all internal identifiers; only user-facing strings and
docs say Built-In). Other user-facing names differ from identifiers that stay as they are:
Historian (`Sessions::`), Variables (`registers`), Computed Dataset (`virtual_`), Canvas Widget
(`painter`), and "acquisition pipeline" in doc/help for the hotpath.

The code base is seven STRICT static libraries under `core/` plus the composition root in
`app/src`; a library includes and links only the layers below it. Cross-library talk goes over
`Core::Bus::MessageBus`, the interfaces the root binds, or the three root-bound module sets, never
an `instance()` reach into another library. Product detail and the full text:
[directory-map.md](doc/claude/directory-map.md) "Product Overview".

## Sub-Documentation

Deep subsystem detail and the silent-breakage lookup live in `doc/claude/`; read the
section a pointer names before working there. The inline blocks are pointers, not substitutes.

| Document | When to read it |
|----------|-----------------|
| [architecture.md](doc/claude/architecture.md) | Before touching any subsystem: the index into `doc/claude/architecture/`. |
| [architecture/ai.md](doc/claude/architecture/ai.md) | Before touching `core/Ui/AI/`, `app/rcc/ai/` or `app/qml/AI/`. |
| [common-mistakes.md](doc/claude/common-mistakes.md) | The silent-breakage lookup: gotchas the linter can't catch. |
| [code-style.md](doc/claude/code-style.md) | Full style spec + NASA Power of Ten. |
| [trust-contract.md](doc/claude/trust-contract.md) | Full text of the Trust Contract, with the incidents behind each rule. |
| [directory-map.md](doc/claude/directory-map.md) | The source tree, one line of role per subsystem. |
| [scripts.md](doc/claude/scripts.md) | The per-script table for `scripts/`. |
| [working-relationship.md](doc/claude/working-relationship.md) | How to collaborate here. Read once per session. |
| [j-space.md](doc/claude/j-space.md) | The verbalization discipline. Read when tuning any AI-facing doc or skill. |
| [repo-skills.md](doc/claude/repo-skills.md) | The project `/`-skills catalog and when each fires. |
| [spec-driven.md](doc/claude/spec-driven.md) | Before any non-trivial or multi-file feature. |
| [TODO.md](doc/claude/TODO.md) | Open work across sessions. Read it at the start of a task; add what you defer, delete what you finish in the same commit. |

## J-Space Discipline — Verbalize the Binding Constraints

The repo's rules only steer an edit if *named at the point of action*
([doc/claude/j-space.md](doc/claude/j-space.md)), so:

- **Name before acting.** Before any edit on a protected path (hotpath, ctor closure,
  signal wiring, cmake flag modules), state in chat the 3-5 invariants that bind *this*
  change — in your own words, not a doc citation.
- **Counterfactual check at handoff.** Which rule does this diff most risk violating, and
  what concrete evidence says it doesn't? Name both.
- **Diverge by naming.** Design/review work sketches named alternatives before one
  recommendation; at least one must violate an assumed constraint.

## Threading & Hotpath — Non-Negotiable

The rules most likely to cause silent breakage. Every rule in full:
[doc/claude/hotpath-rules.md](doc/claude/hotpath-rules.md); data flow, threading table and
benchmark mechanics: [doc/claude/architecture/dataflow.md](doc/claude/architecture/dataflow.md).
Invoke the `ss-hotpath` skill before editing these paths (nothing loads it for you).

- **The frame pipeline runs on `IO::PipelineHost`'s processing thread (spec 0051 M3), not the
  GUI thread.** Never add a plain `BlockingQueuedConnection` from the GUI into the pipeline.
- **`runOnObjectThread` is for command-rate waits, never for a GUI-thread script API.** **No
  `lua_*` call may run inside a routed lambda.**
- **`FrameReader` and `CircularBuffer` are pipeline-thread / SPSC. Never add mutexes.** Enforced:
  code-verify:spsc-lock.
- **In-pipeline signal hops must be `Qt::DirectConnection`.** GUI↔pipeline traffic is
  chunk/command/tick rate only. Enforced: code-verify:pipeline-queued-hop.
- **No allocation on the publish path.** Blocks come from a pool.
- **One publication payload, one ingestion path (spec 0055).** Blocks flush on the display tick or
  a sample cap (`kFrameBlockSampleCap` 64, `kStreamBlockSampleCap` 4096). Never add a rate cap or a
  per-view reduction.
- **The two republish lanes never share one "already republished" mark (spec 0064).** Enforced:
  compile:DataModel::RepublishLane, ctest:tst_republish_lanes.
- **The session boundary is the sink contract (spec 0075).** The builder flushes every open block
  BEFORE emitting.
- **Time rings are sized from a rate, never a sample count, and a ring's clock never rewinds.**
  Enforced: compile:DSP::RingCapacity, compile:UI::PlotClockState, ctest:tst_time_ring_sizing,
  ctest:tst_ring_exactness, code-verify:lock-memory-scope.
- **Native + PlainText parses through the span fast lane**, zero steady-state allocation. Hotpath
  flags are `DataModel::Cached<T>`; a new cache is never a plain `bool`. Enforced:
  ctest:tst_cached_flags, anchor:cached-hotpath-flags.
- **Source owns time.** Stamp at the driver boundary; never re-stamp in export/report workers.
  Enforced: code-verify:--clock-census.
- **Driver opens are synchronous calls; most drivers dial asynchronously behind them.** An async
  dial's verdict has ONE owner, `HAL_Driver::openFinished(ok, reason)`, emitted exactly once per
  attempt.
- **Diagnostics are pulled, never pushed (specs 0033/0035).**
- **JS scripts**: always `JsScriptEngine::guardedCall()`, never `parseFunction.call()`.
  `setInterrupted(true)` only in `JsWatchdogThread.cpp`. Enforced: code-verify:js-raw-call,
  code-verify:js-interrupt-site.
- **256 kHz is a CI gate, not a slogan.** `--benchmark-hotpath` is a hard gate in `ci.yml`; don't
  regress it. Enforced: anchor:hotpath-min-fps.
- **The message bus is not a hotpath primitive** (spec 0076). Enforced: code-verify:bus-on-hotpath.
- **Reuse the kernels; never inline intrinsics or invent a macro** (`core/Core/DSPSimd.h`,
  `core/Core/HotpathOptimization.h`); never fast-math, no-unwind or GCC `optimize("...")`.

## Startup & Composition Root — Non-Negotiable

Full contract, including the ctor-edge proof and the licensing consumer inventory:
[doc/claude/architecture/startup.md](doc/claude/architecture/startup.md).

- **`ModuleManager::instantiateCoreModules()` pins singleton construction order** (ProjectModel
  before AppState, Dashboard last); never reorder or add entries without re-running the ctor-edge
  proof in [doc/claude/specs/0001-composition-root/](doc/claude/specs/0001-composition-root/).
  Enforced: anchor:composition-root-order.
- **No new function-static singleton reach in a module ctor closure** — the one residual hazard
  now that an early reach into an adopted module fails by name. Enforced:
  compile:Core::ModuleConstruction, script:scripts/code-verify.py#--singleton-census.
- **`SessionContext` (spec 0039) owns the nine core modules; never call
  `SessionContext::current()` from a method body**, and no library includes `SessionContext.h`.
  Enforced: code-verify:arch-session-adopt-site, script:scripts/code-verify.py#--singleton-census.
- **Every composition root goes through `ModuleManager::composeSession(BindMode)`**; an unbound
  root publishes through a null host and a sink left off its list records a valid-looking empty
  file. Enforced: compile:Misc::ModuleManager::composeSession.
- **License-gated state must exist before `restoreLastProject()` or re-derive on
  `activatedChanged`.** The licensing block is the FIRST thing `instantiateCoreModules()` builds
  after Translator (spec 0042). `activatedChanged` fires only on real token-validity transitions.
  Anything baking `SerialStudio::activated()` into derived state at load time still needs a
  `LemonSqueezy::activatedChanged` hook, or late/async activation ships fallback widgets
  (2026-07-09: Plot3D degraded to MultiPlot).

## Subsystem Contracts — Read Before Touching

Each row is a trigger, not a summary: read the named section before editing that area. What
breaks silently in each one: [doc/claude/subsystem-contracts.md](doc/claude/subsystem-contracts.md).

| Touching | Read |
|----------|------|
| Any `ProjectModel` mutator, undo/redo (spec 0031) | [project.md](doc/claude/architecture/project.md) "Undo History" |
| A dataset property or any API-surface generator (specs 0036, 0037) | [project.md](doc/claude/architecture/project.md) "Dataset Property Registry", `project.md` "Generated API Surfaces" |
| A toolbar button, palette entry, context menu, shortcut, or fixed icon (specs 0028/0063) | [commands-icons.md](doc/claude/architecture/commands-icons.md) "Icons", `commands-icons.md` "Commands" |
| Installable widgets, `UI::WidgetExtensions` (spec 0038) | [dashboard.md](doc/claude/architecture/dashboard.md) "Widget Extensions" |
| Alarm bands: `UI::AlarmMonitor` trackers, the annunciator's band points, the Project Editor's Alarm Bands buttons, the `suppressAlarms` or `alarmsWhileVisible` option (specs 0093/0105) | [dashboard.md](doc/claude/architecture/dashboard.md) "Alarm Bands" |
| `core/Ui/Console/Annotations.*`, `ConsoleAnnotations.qml` (spec 0059) | [dashboard.md](doc/claude/architecture/dashboard.md) "Frame annotation layer" |
| `core/Api/API/Mirror/`, `streamAvailable()` (spec 0040) | [mirror.md](doc/claude/architecture/mirror.md) "Wire protocol" |
| Chrome motion: a popup, menu, dialog or button animation in `app/qml` | [PopupEnter.qml](app/qml/Widgets/PopupEnter.qml), [PopupExit.qml](app/qml/Widgets/PopupExit.qml) |
| HDR output: `HdrSurface`/`HdrBoost`, `Misc::HdrOutput`, an emissive material, a shader under `app/shaders/`, or any `MultiEffect`/`layer` near an emissive element (spec 0089) | [dashboard.md](doc/claude/architecture/dashboard.md) "HDR Output" |
| An embedded code editor's render cadence | [scripting.md](doc/claude/architecture/scripting.md) "Embedded Code Editors" |
| The AI assistant: a tool tier, the checkpoint timer, a provider, the tool surface, the sandbox roots | [ai.md](doc/claude/architecture/ai.md) "The Tier Model", `ai.md` "Checkpoint Semantics" |
| Locating a god object's concerns (`ProjectModel`, `ProjectHandler`, `FrameBuilder`, `Dashboard`) | [directory-map.md](doc/claude/directory-map.md) |
| The connect gate, a dataset transform or user-table mutator, or any new way to author either (spec 0094) | [io.md](doc/claude/architecture/io.md) "Opening a Link" |
| Anything that switches the operation mode: a project load, save or generation, a file drop, an example download, the Project Editor opening (spec 0100) | [AppState.h](core/Pipeline/AppState.h) |
| A new API command, one that runs or installs code, loads a project or launches a process, or one that re-dispatches other commands (spec 0098) | [CommandCapabilities.h](core/Api/API/CommandCapabilities.h), [CommandRegistry.h](core/Api/API/CommandRegistry.h), [LaunchAuthorizer.h](core/Core/IO/LaunchAuthorizer.h) |
| A new API command, or anything that serializes the project or a live frame outward (spec 0096) | [io.md](doc/claude/architecture/io.md) "Sealed Sessions" |
| Anything under `core/` or a source that could move there (spec 0076) | [directory-map.md](doc/claude/directory-map.md) "core/" |
| The driving surface (spec 0101): `core/Ui/UI/Automation/`, `AutomationHandler`, an `objectName` under `app/qml`, or the `DrivesUi` consent | [automation.md](doc/claude/architecture/automation.md) |
| Quick Plot parsing: `LineFormat`, `QuickPlotFormatState`, `QuickPlotBuilder`, `line_values`, Save as Project (spec 0106) | [dataflow.md](doc/claude/architecture/dataflow.md) "The Quick Plot Lane" |
| A `#ss` declaration, `declaration.json`, a `wire` flag or `firmware/` (spec 0111) | [dataflow.md](doc/claude/architecture/dataflow.md) "Device Declarations" |

## Code Style — Essentials

`scripts/code-verify.py` is the contract — read its `--check` output, don't re-derive the
rules. Full spec and the NASA Power of Ten live in
[doc/claude/code-style.md](doc/claude/code-style.md). The handful you need *before* typing:

- **Format**: 100-col, 2-space indent, LF, pointer/ref binds to type (`int* p`). No braces on
  single-statement bodies; blank line after a brace-free body. Max 3 nesting levels (guard
  clauses); functions 40-80 lines, hard limit 100; translation units under 1500 lines
  (`cxx-tu-too-long` — the function cap alone let god TUs accrete a method per feature).
  Run `clang-format`.
- **Headers (.h)**: `Q_OBJECT` → `Q_PROPERTY` → `signals:` → ctor/deleted copy → `public:`
  (`instance()` first) → `public slots:` → `private slots:` → `private:`, Christmas-tree in
  each block. `[[nodiscard]]` on every non-void return. **Never `Q_INVOKABLE void`** (use
  `public slots:`). **No in-header member init** — ctor init list only.
- **Signals**: `Q_EMIT` not `emit`; lowercase `signals:`/`public slots:`; never
  `SIGNAL()`/`SLOT()`. Never `disconnect(nullptr)` as the slot — capture the `Connection`.
- **Comments**: label, don't narrate. **No comments inside a function body** (`cxx-inbody-comment`):
  fold a load-bearing *why* into the one-line `/** @brief ... */`. `//---` banners live *between*
  functions. No inline EOL comments, no AI narration.
- **Naming**: `CamelCase` types, `camelCase` functions, `lower_case` locals + public members,
  `s_`/`m_`/`k`/`UPPER_CASE` for static/private/constexpr/macro (full table in the sub-doc).
- **Safety-critical (NASA Power of Ten)** — hotpath violations are blockers: no alloc/Frame-copy
  on the dashboard path; fixed loop bounds; assertion density ≥2/function; `[[nodiscard]]` + return
  checks at every system boundary; zero warnings; no `reinterpret_cast`/`dynamic_cast` on the
  hotpath; SPDX header per file (first-party is `GPL-3.0-or-later`; `reuse lint` gates CI).
- **Assertions are `SS_ASSERT(cond, action)`** (`core/Core/SSAssert.h`), not `Q_ASSERT`: the
  condition evaluates in **every** build; the action is one side-effect-complete statement, never
  `continue`/`break`. On per-frame/per-cell kernels use `SS_ASSERT_HOTPATH(cond)`.
