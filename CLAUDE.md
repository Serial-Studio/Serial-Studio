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
- **Never compile.** Never invoke `cmake`/`jom`/`clang`/the compiler — the developer builds
  the app themselves. Verify structure by reading and with `scripts/code-verify.py`.
- **Runtime experiments are sanctioned.** Ground truth beats on-paper reasoning: drive a
  *running* app through the API server (`localhost:7777`, `tests/utils/api_client.py`) to
  probe hypotheses, not only to verify fixes; run `ctest` and already-built binaries
  (`--selftest`, `--benchmark-hotpath`) against an existing build dir; prototype ideas as
  throwaway Node/Python sims in the scratchpad. Launching the GUI app and compiling stay
  the user's.
- **Sample the running app before theorizing about a GUI stall.** Never reason a freeze,
  stutter, or ignored resize out of the source — `sample <pid>` in the background while the user
  reproduces. Recipe, known stack signatures, and the 2026-08-13 incident:
  [doc/claude/common-mistakes.md](doc/claude/common-mistakes.md) "Diagnosing a GUI Stall".
- **Update CLAUDE.md** for any architectural change that future me would otherwise miss.
- **`scripts/` is the style contract.** When in doubt, run it; don't restate it here.

## Context Canary — Last Line of Every Response

End every response — including one-word answers — with this exact line, reproduced
from memory:

`canary: qt 6.11.2 | cpp20 | hotpath 256k (native 1024k, js 64k) | queue 65536 | api 7777 | style 100/2`

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
  after. Operationalized as `/ss-spec`. What enforces it is the review conversation, not the
  git record: spec, plan and implementation land squashed in one commit, so history can
  neither confirm nor refute that the plan came first. Never cite the archive as proof of
  gating, and don't assume a spec dir is complete — some carry a plan or findings only.
- **Self-review before handoff.** Re-read the diff: is this *what was asked, and only that*?

## Scripts

All scripts in `scripts/` are CWD-independent and write LF endings; run from anywhere. The
per-script table lives in [doc/claude/scripts.md](doc/claude/scripts.md). What binds you:
**run `sanitize-commit.py` before every commit** (sanitize only — it never commits or
pushes); `code-verify.py` is the structural linter (`--fix` rewrites, `--check` regenerates
`.code-report`; errors block CI). Suppression: `// code-verify off` / `// code-verify on`
(C++/QML), `<!-- doc-verify off -->` / `<!-- doc-verify on -->` (Markdown) — suppressions
are a code-review trigger. `.code-report` / `.doc-report` / `.claim-report` are the cleanup
checklists; advisories are baseline debt, new code still clears them.

Three gates ratchet *growth* against a checked-in baseline instead of capping absolute size,
and `--accept` re-seeds each: `code-verify.py --singleton-census`, `code-verify.py
--tu-census` (excess over 1500 lines), and `claim-verify.py`, which resolves every path,
symbol and pinned constant in the AI-facing docs — the canary's values included — against the
tree. Baselines, fences and anchor mechanics: [doc/claude/scripts.md](doc/claude/scripts.md).

## Tests

Python/pytest suite under `tests/`. Full catalog — per-file coverage, fixtures, markers, the
delay/operation-mode tables, the C++ ctest tier, the `--selftest` in-app tier —
lives in [tests/README.md](tests/README.md); read it before writing a test. What binds you:

- **You don't build or launch the app, but you may drive a running one.** Integration,
  security, and performance tests drive a running Serial Studio over TCP — they need the app
  up with **Preferences → API & Plugins → Enable API Server** (`localhost:7777`). The user
  launches it; once up, you may run those tests and poke the API directly
  (`tests/utils/api_client.py`) to test hypotheses. Probe first: `nc -z 127.0.0.1 7777`.
- **`tests/scripts/` is the exception you *can* run** — pure JS frame-parser unit tests, fresh
  Node.js subprocess per case, no Qt, no app. `pip install -r tests/requirements.txt` once;
  `pytest.ini` registers all markers and a 30 s per-test timeout.
- **C++ units under `app/tests/`** (spec 0032) run via `ctest` (no CMake presets — the repo
  uses hand-written `cmake -B ... -D...` configures; CI inlines the unit-tier flags) — the
  maintainer builds; you may run `ctest` against an existing build dir (never configure or
  compile). **`--selftest`** suites run inside
  `CLI::process()` **before** the composition root: never touch an application singleton there.
- The C++ hotpath has no pytest path — throughput is the user-run `--benchmark-hotpath` gate
  (see Threading & Hotpath), piece correctness the ctest tier; `ci.yml` runs both.

```bash
pytest tests/scripts/ -v                  # JS-parser units (Node.js only) — safe for you
pytest tests/integration/ -v              # all integration (needs running app)
pytest tests/ -m "not destructive" -v     # skip server-crashing tests
```

## Project Overview

Serial Studio: cross-platform telemetry dashboard, Qt 6.11.2 + C++20. Data sources: UART,
TCP/UDP/WebSocket/HTTP (one Network driver, spec 0068), BLE, Audio, Modbus, CAN Bus (Qt
plugins plus direct gs_usb/SLCAN/Seeed backends; J1939 TP + ISO-TP reassembly, spec 0073),
MQTT (Sparkplug B both directions, specs 0073/0074: host-application subscribe with project
generation, edge-node publish with stable aliases), OPC UA (embedded open62541 + mbedTLS,
six security policies None through Aes256_Sha256_RsaPss, spec 0067), Siemens S7comm
(S7-300/400/1200/1500, in-house stack), EtherNet/IP (libplctag), IEC 60870-5-104 (in-house
stack; all three PLC/telecontrol drivers are read-only clients, spec 0073), USB (libusb),
HID (hidapi), Process I/O. 15+ visualization
widgets, 5 output (control) widgets, 256 kHz+ data rate (CI-gated; see below).
Frame parsers in JavaScript (`QJSEngine`), Lua (embedded LuaJIT 2.1, 5.1 + shims; per-project
Safe/Fast execution mode — spec 0051), or Built-In ("Native"
in all internal identifiers — `SerialStudio::Native`, `CFrameParser`, `NativeTemplate`; only
user-facing strings/docs say Built-In. Parametrized C++ templates configured via a JSON
descriptor, no user code). Per-dataset value transforms in JS or Lua. Pro features: Output
widgets, Modbus, CAN Bus, OPC UA, S7comm, EtherNet/IP, IEC 60870-5-104, Sparkplug B, MDF4,
3D, ImageView, Waterfall, file-transfer protocols (X/Y/ZMODEM), Modbus map importer,
Historian (per-session SQLite recording; "Session Database" pre-2026-08), InfluxDB 2.x sink
(`app/src/InfluxDB/`, line protocol per published block).
User-facing renames of 2026-08-19 — internal identifiers unchanged: Historian (was Session
Database; `Sessions::` namespace, `sessions.*` API, "Session Databases" folder stay), Variables
(was Shared Memory; registers → variables in UI/docs, `RegisterDef`/`registers` JSON stay),
Computed Dataset (was Virtual Dataset; `virtual_` field + `"Virtual"` JSON key stay), Canvas
Widget (was Painter; `"painter"` widget key, `project.painter.*` API, Painter* classes stay).
In doc/help the hotpath is called the "acquisition pipeline" — code, benchmarks, and internal
docs keep "hotpath".

## Sub-Documentation

Deep subsystem detail and the silent-breakage lookup live in `doc/claude/`. Read the
relevant doc **in full** before working in that area — the inline blocks above and below are
pointers, not substitutes.

| Document | When to read it |
|----------|-----------------|
| [architecture.md](doc/claude/architecture.md) | Before touching any subsystem: the index into `doc/claude/architecture/` — dataflow (hotpath), startup, io, project, scripting, dashboard, kernels, export, mirror, commands-icons. |
| [common-mistakes.md](doc/claude/common-mistakes.md) | The silent-breakage lookup: gotchas the linter can't catch (timestamp capture, queued-vs-direct hotpath, `operator[]` inserts, macOS file-dialog reentrancy, the GUI-stall sampling recipe). |
| [code-style.md](doc/claude/code-style.md) | Full style spec + NASA Power of Ten. The Code Style block below is the inline essentials. |
| [trust-contract.md](doc/claude/trust-contract.md) | Full text of the Trust Contract above, with the incidents behind each rule. |
| [directory-map.md](doc/claude/directory-map.md) | The `app/src` / `app/qml` / `lib` tree, one line of role per subsystem. |
| [scripts.md](doc/claude/scripts.md) | The per-script table for `scripts/`. |
| [working-relationship.md](doc/claude/working-relationship.md) | How to collaborate here: recommend don't enumerate, push back when a choice will cost, ground truth outranks on-paper reasoning. Read once per session. |
| [j-space.md](doc/claude/j-space.md) | The verbalization discipline and its grounding. Read when tuning any AI-facing doc or skill. |
| [repo-skills.md](doc/claude/repo-skills.md) | The project `/`-skills catalog and when each fires. Most auto-activate; this is the deliberate-pick lookup. |
| [spec-driven.md](doc/claude/spec-driven.md) | Before any non-trivial or multi-file feature: the four gated phases, where artifacts live, when to skip. |

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
  (the human still gets one recommendation, per working-relationship.md). At least one
  alternative must violate an assumed constraint — then check whether that constraint
  actually binds: wrong answers usually hide in premises, not derivations. Wide design
  spaces may fan out a judge-panel workflow of independent attempts.
- **Externalize long state.** Write intermediate state into durable artifacts (spec/plan/
  tasks files, a chat checklist); re-name only what binds the current edit.

## Threading & Hotpath — Non-Negotiable

The rules most likely to cause silent breakage. Full detail (data flow, threading table,
cached flags, benchmark mechanics) in
[doc/claude/architecture/dataflow.md](doc/claude/architecture/dataflow.md); the
`ss-hotpath` skill auto-activates on these paths and re-states them, including the spec-0055
block caps, the time-ring/plot-clock rules, and the kernel macros.

- **The frame pipeline runs on `IO::PipelineHost`'s processing thread (spec 0051 M3), not the
  GUI thread.** FrameReaders, `FrameParser` and `FrameBuilder` all live there; the GUI drains
  finished pooled frames from an SPSC ring in `Dashboard::onDisplayTick`. Public
  FrameBuilder/FrameParser mutators SELF-MARSHAL — GUI→pipeline waits go through
  `IO::PipelineHost::runOnObjectThread` (event-loop-backed), pipeline→GUI reads through
  `runOnGuiThreadBlocking`. Never add a plain `BlockingQueuedConnection` from the GUI into the
  pipeline.
- **`runOnObjectThread` is for command-rate waits, never for a GUI-thread script API.** The wait
  spins a nested `QEventLoop` that fires the display tick and re-enters the calling script.
  `DataModel::readTableView` / `writeTableStore` (`DataTable.h`) hold the ONE routing rule, and
  both the JS bridge and the Lua closures go through them. **No `lua_*` call may run inside a
  routed lambda.** Any new GUI-callable script API follows the same rule — see
  [doc/claude/architecture/scripting.md](doc/claude/architecture/scripting.md) "Data Tables".
- **`FrameReader` and `CircularBuffer` are pipeline-thread / SPSC. Never add mutexes.** Recreate
  via `resetFrameReader()` / `reconfigure()`.
- **In-pipeline signal hops must be `Qt::DirectConnection`.** Queued between two
  pipeline-thread objects fills the 65536-slot queue at 10+ kHz and drops frames. GUI↔pipeline
  traffic is chunk/command/tick rate only — never a per-frame queued emission.
- **No allocation on the publish path.** Blocks come from a pool (`claimBlockSlot`,
  `use_count()==1` probe, aliasing shared_ptr) with columns sized once at bind. The one copy is
  `clone_block_trimmed` for the async sinks — deliberate and gated on a sink being on.
- **One publication payload, one ingestion path (spec 0055).** Nothing publishes a
  `TimestampedFrame` any more: `FrameBuilder` stages parsed rows into a pooled
  `DataModel::DataBlock` and flushes on the display tick or a sample cap (`kFrameBlockSampleCap`
  64, `kStreamBlockSampleCap` 4096). Dense sources do per-sample work on their own
  `IO::StreamWorker` thread but emit `blockReady` **queued to the pipeline thread**, the SINGLE
  producer for every sink. Structure travels separately as a `StructureSnapshot`. Never add a
  rate cap or a per-view reduction; an overrun drops whole blocks and counts them.
- **Two republish lanes, one obligation each (spec 0064).** A table-fed virtual dataset publishes
  ONLY through the synthetic refresh, and it has a sink-fed lane (`dashboardTick()`) and a masked
  lane (`reprocessFrames()`, `refreshStreamDrivenFrames()` on every UI tick while a stream source
  produces). They must never share one "already republished" mark: the masked lane consumes the
  change-driven transform clock, so sharing it makes the export lane see `changed == false` and
  skip — dashboard live, every recording empty. `DataModel::RepublishGate` keeps them apart; any
  lane seeing a change marks the sinks dirty, only an export publish clears it. Never gate an
  export publish on "did THIS pass see a change".
- **Time rings are sized from a rate, never a sample count alone**, and a ring's clock never
  rewinds; `m_plotClocks` and `m_plotDisplayTimeSec` are ONE state (`resetPlotClocks()`), never
  cleared one without the other. Full rules + both 2026-08 incidents:
  [doc/claude/architecture/dashboard.md](doc/claude/architecture/dashboard.md) "Time-Ring Sizing".
- **Native + PlainText parses through the span fast lane** (`trySpanLane` →
  `parseUtf8Spans` → `applyDatasetValuesSpans`): byte views + in-place QString writes,
  zero steady-state allocation. The hotpath reads **cached** flags (`m_operationMode`,
  `m_playerOpen`, `m_anyAsyncSink`, `m_captureLatestFrame`, `m_changeDriven`, Dashboard
  `m_streamAvailable`) — a new input to any of them must wire its change signal to the cache
  refresh or frames/exports silently stop. Flag mechanics: dataflow.md "Cached Hotpath
  Flags", read before touching any of them. `streamAvailable()` also reads the spec-0040
  mirror flag (`API::MirrorSession::mirroring()`, a plain module-static bool — never a
  construction).
- **Source owns time.** Stamp at the driver boundary; never re-stamp in export/report
  workers (use `monotonicFrameNs(...)` as the safety net only).
- **Driver opens are synchronous calls; several drivers dial asynchronously behind them.**
  `DeviceManager::open()` calls `HAL_Driver::open(mode)` directly — no async-open hook, no task
  runner. In-flight dials report via `HAL_Driver::isConnecting()`; the async dial verdict has ONE
  owner, `HAL_Driver::openFinished(ok, reason)`, emitted exactly once per attempt — a driver that
  reports only success wedges the connect button. NO reopen-on-config-edit machinery exists. Full
  doctrine (probe sockets, drop recovery, the `app/src/Async/` task tree):
  [doc/claude/architecture/io.md](doc/claude/architecture/io.md) "Opening a Link".
- **Diagnostics are pulled, never pushed (specs 0033/0035).** `FrameReader` / `FrameBuilder`
  counters are plain `quint64` increments polled on the 1 Hz tick — never signal, allocate,
  or lock per frame. A recreated `FrameReader` zeroes them: consumers work on deltas.
- **JS scripts**: always `JsScriptEngine::guardedCall()`, never `parseFunction.call()`.
  `setInterrupted(true)` only in `JsWatchdogThread.cpp`.
- **256 kHz is a CI gate, not a slogan.** `--benchmark-hotpath` drives the real parse pipeline
  with nine gates tiered off `--min-fps` (default 256000), from Native numeric at 4x
  (1.024 MHz) down to JS mixed at 64 kHz, plus 0.5x consumer-path floors (full tier table in
  the `ss-hotpath` skill); `ci.yml` runs it per push/PR as a hard gate on the PGO-optimized
  binary. Don't regress it. `datasets+publish` is ~70-80% of per-frame time.
- **Reuse the kernels; never inline intrinsics or invent a macro.** `app/src/DSPSimd.h`
  (spec 0021, bit-exact per lane) and `app/src/DataModel/HotpathOptimization.h`
  (`SS_FORCE_INLINE`, `SS_ASSUME`, ...; never fast-math / no-unwind / GCC `optimize("...")`):
  [doc/claude/architecture/kernels.md](doc/claude/architecture/kernels.md).

## Startup & Composition Root — Non-Negotiable

Full contract, including the ctor-edge proof and the licensing consumer inventory:
[doc/claude/architecture/startup.md](doc/claude/architecture/startup.md).

- **`ModuleManager::instantiateCoreModules()` pins singleton construction order** (ProjectModel
  before AppState, Dashboard last). Never reorder or add entries without re-running the ctor-edge
  proof in [doc/claude/specs/0001-composition-root/](doc/claude/specs/0001-composition-root/).
- **ProjectModel's ctor closure is a protected surface** (`newJsonFile`, `watchProjectFile`,
  `scheduleAutoSave`, the `ControlScript::setCode` chain): it runs before AppState/Dashboard
  exist; calling their `instance()` there recurses the Meyers guard and aborts — shipped and
  crashed once (2026-07-07). Gate new code on `m_initialized`.
- **A ctor-edge proof dies when ctor-reachable code changes.** Any edit inside that closure
  re-triggers the check, no matter how unrelated the edit looks.
- **`SessionContext` (spec 0039) owns the nine core modules** as `unique_ptr` slots adopted
  inside `instantiateCoreModules()`. Ctor/dtor stay empty; adopted addresses never change;
  `shutdown()` releases in exact reverse pinned order, after the pipeline thread and every stream
  worker join in `stopFrameConsumerWorkers()`. **Never call `SessionContext::current()` from a
  method body** — composition root and `instance()` forwarders only; the singleton census
  (`code-verify.py --singleton-census --check`) fails on any increase.
- **License-gated state must exist before `restoreLastProject()` or re-derive on
  `activatedChanged`.** The licensing block is the FIRST thing `instantiateCoreModules()` builds
  after Translator (spec 0042). `activatedChanged` fires only on real token-validity transitions.
  Anything baking `SerialStudio::activated()` into derived state at load time still needs a
  `LemonSqueezy::activatedChanged` hook, or late/async activation ships fallback widgets
  (2026-07-09: Plot3D degraded to MultiPlot).

## Subsystem Contracts — Read Before Touching

Each row is a trigger, not a summary: read the linked doc **in full** before editing that
area. The hazard column names what breaks silently — the doc holds the rule.

| Touching | Read | Hazard |
|----------|------|--------|
| Any `ProjectModel` mutator, undo/redo (spec 0031) | [project.md](doc/claude/architecture/project.md) "Undo History" | Mementos are two-phase: `ProjectUndoScope` stages, the first `setModified(true)` commits — omit it and nothing is recorded. Lint: `undo-scope-missing`. |
| A dataset property or any API-surface generator (specs 0036, 0037) | [project.md](doc/claude/architecture/project.md) | `app/rcc/properties/dataset.json` generates six checked-in artifacts — **never hand-edit a generated file**. gRPC field numbers are append-only; a moved one fails CI. |
| A toolbar button, palette entry, context menu, shortcut, or fixed icon (specs 0028/0063) | [commands-icons.md](doc/claude/architecture/commands-icons.md) | Icons resolve ONLY via `Misc::IconRegistry` — never hardcode `qrc:/icons/...`. New command = one manifest entry + one bindings entry; run `scripts/registry-verify.py`. Project Editor context menus are registry-driven too (`editor-menus.json` + `ProjectEditorMenuBindings.qml` + `CommandMenu.qml`) — never hand-write a `Menu` there. |
| Installable widgets, `UI::WidgetExtensions` (spec 0038) | [dashboard.md](doc/claude/architecture/dashboard.md) | Packages resolve to `DashboardExtension = 100`, persist as `"ext:<id>"`; `readsStringValues` is what registers one in `string_targets`. Trust model is consent, not containment — never call an extension sandboxed. |
| `app/src/Console/Annotations.*`, `ConsoleAnnotations.qml` (spec 0059) | [dashboard.md](doc/claude/architecture/dashboard.md) "Frame annotation layer" | `annotate()` stages, `commitPending()` publishes per tick — reading `count()` right after needs a commit. `reset()` clears the model *before* re-reading the offset. |
| `app/src/API/Mirror/`, `streamAvailable()` (spec 0040) | [mirror.md](doc/claude/architecture/mirror.md) | Dataset ordering or `wireUniqueId` changes are wire breaks: bump `kWireVersion`, regenerate `tests/fixtures/mirror/`. Viewer frames never reach the export fan-out. |
| An embedded code editor's render cadence | [scripting.md](doc/claude/architecture/scripting.md) "Embedded Code Editors" | Never give a main-window-embedded editor an unconditional per-tick `grab()` — cost 13% of the GUI thread (2026-08-17). |
| Locating a god object's concerns (`ProjectModel`, `ProjectHandler`, `FrameBuilder`, `Dashboard`) | [directory-map.md](doc/claude/directory-map.md) | Spec 0070 re-formed the god objects into facades owning real sub-object classes (one class = one .h/.cpp, in a sibling dir named after the facade). Never split one class across TUs; `tu-cutter.py` is retired for class work. |

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
  per-frame/per-cell kernels use `SS_ASSERT_HOTPATH(cond)` instead — see
  [doc/claude/architecture/kernels.md](doc/claude/architecture/kernels.md).
