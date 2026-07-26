---
spec: 0033-problem-center
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement); all 19 tasks complete
updated: 2026-07-25
---

# Tasks 0033 — Problem center (project + link diagnostics)

> **Phase 3 of 4 — the ordered checklist.** Decompose [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable* — each one a coherent diff a reviewer
> could read in isolation. `/ss-implement` works this list top to bottom and keeps the status
> boxes current. Gate: do not start `/ss-implement` until a human marks this `approved`.

## Conventions

- One task = one focused, reviewable change. If a task touches >3 files or needs a paragraph
  to describe, split it.
- **Verify** is how *this* unit is confirmed before moving on — usually
  `python scripts/code-verify.py --check <files>`, plus a test or a read-back where one fits.
- **Deps** lists task IDs that must land first.
- Order so the tree compiles (conceptually) after each task where practical.
- The agent does **not** build, run the app, or run the maintainer-only steps (T18's
  `--dump-api-schema`, `--benchmark-hotpath`, and the live-API pytest files).

## Tasks

### T1 — ProblemCenter core type + model

- **Files:** `app/src/Misc/ProblemCenter.h`, `app/src/Misc/ProblemCenter.cpp`
- **Does:** Adds the `Misc::ProblemCenter : QAbstractListModel` singleton — `Severity` and
  `Trigger` enums, the `Finding` struct, `registerChecker(id, triggers, fn)`, the roles enum,
  `errorCount`/`warningCount`/`infoCount`/`totalCount` Q_PROPERTYs, `runNow()`, `activate(row)`
  emitting `jumpRequested`, and the per-checker slice replacement with the equality compare
  that skips the model reset when nothing changed. The constructor is **inert**: member
  initialization only, no `instance()` call, no `connect`, no timer, no QSettings — with a
  header comment saying so and why (spec 0001 ctor-edge proof).
- **Verify:** `python scripts/code-verify.py --check app/src/Misc/ProblemCenter.h
  app/src/Misc/ProblemCenter.cpp`; re-read the constructor and confirm zero outgoing calls.
- **Deps:** none
- [x] done — `Misc::ProblemCenter` added; ctor is member-init only (zero outgoing edges), model
      resets only when the flattened list differs, one aggregated notification per run with new
      findings.

### T2 — Wire ProblemCenter into the composition root

- **Files:** `app/src/Misc/ModuleManager.cpp`, `app/CMakeLists.txt`
- **Does:** Adds `(void)Misc::ProblemCenter::instance();` to `instantiateCoreModules()`
  immediately after `NotificationCenter`; calls `Misc::ProblemCenter::instance()
  .setupExternalConnections();` in `setupCrossModuleConnections()` **before**
  `appState->restoreLastProject()`; registers `Cpp_Misc_ProblemCenter` in
  `registerCoreContextProperties()`; adds the two new files to `SOURCES`/`HEADERS`.
  `setupExternalConnections()` connects the ProjectModel change signals and
  `Misc::TimerEvents::timeout1Hz`, and posts run summaries to `NotificationCenter`.
- **Verify:** `python scripts/code-verify.py --check app/src/Misc/ModuleManager.cpp`; read back
  the ordering — registration must precede `restoreLastProject()`. State in chat that the
  spec-0001 ctor-edge proof was re-run and the new node has zero outgoing constructor edges.
- **Deps:** T1
- [x] done (ModuleManager part) — instantiated right after `NotificationCenter`,
      `setupExternalConnections()` called before `restoreLastProject()`, `Cpp_Misc_ProblemCenter`
      registered. `app/CMakeLists.txt` entries are owned by the coordinator (listed in handoff).

### T3 — Project-schema checkers

- **Files:** `app/src/Misc/Problems/ProjectCheckers.h`, `app/src/Misc/Problems/ProjectCheckers.cpp`,
  `app/CMakeLists.txt`
- **Does:** Implements and registers the spec R8 checks: duplicate frame indices **per source**;
  dataset index unreachable for the configured parser; groups with no datasets and no output
  widgets; dangling references (`xAxisId`, `waterfallYAxis`, workspace `WidgetRef`, action and
  output-widget `sourceId`); inverted/degenerate ranges (`pltMin/pltMax`, `wgtMin/wgtMax`,
  `fftMin/fftMax`, `ledHigh` outside range, alarm bands outside range); duplicate dataset
  aliases. Each finding sets `entityUniqueId` and `jump`. Returns nothing when there is no
  project document (QuickPlot / Console-only). Caps output at 50 findings with a trailing
  "and N more".
- **Verify:** `python scripts/code-verify.py --check` on both files; read back that every
  index/reference check is scoped by `sourceId`.
- **Deps:** T1, T2
- [x] done — five checkers (`project.frame-index`, `.empty-group`, `.reference`, `.numeric-range`,
      `.alias`) registered on `ProjectChanged | OnDemand`; index and reference checks keyed by
      `(sourceId, index)`; every check returns early unless a ProjectFile document with groups
      exists; each slice capped at 50 with a trailing "and N more". `app/CMakeLists.txt` entries
      are owned by the coordinator (listed in handoff).

### T4 — FrameReader diagnostic counters

- **Files:** `app/src/IO/FrameReader.h`, `app/src/IO/FrameReader.cpp`
- **Does:** Adds plain `quint64 m_bytesIn`, `m_framesExtracted`, `m_checksumErrors`,
  `m_totalOverflowBytes` with `[[nodiscard]]` accessors next to `droppedFrameCount()` and a
  combined reset. Increments them at the existing sites: chunk size in `processData`, frame
  accounting next to `noteDroppedFrame`, the `ValidationStatus::ChecksumError` branch, and the
  overflow accumulation immediately before the existing `resetOverflowCount()` (which today
  destroys the number). Also throttles the currently-unthrottled per-failure checksum
  `qWarning` to the 5 s pattern `noteDroppedFrame` already uses.
- **Verify:** `python scripts/code-verify.py --check app/src/IO/FrameReader.h
  app/src/IO/FrameReader.cpp`; read back that no atomic, mutex, signal, or allocation was
  added and that every increment sits inside a branch that already existed.
- **Deps:** none
- [x] done — four plain `quint64` counters + `resetDiagnosticCounters()`; overflow accumulates
      before the existing `resetOverflowCount()`; checksum dump throttled to 5 s. No atomic,
      mutex, signal or allocation added.

### T5 — Make link statistics reachable

- **Files:** `app/src/IO/DeviceManager.h`, `app/src/IO/ConnectionManager.h`,
  `app/src/IO/ConnectionManager.cpp`
- **Does:** Adds `[[nodiscard]] IO::FrameReader* frameReader() const noexcept` to
  `DeviceManager` (`m_frameReader` is private with no accessor today) and a
  `[[nodiscard]] LinkStats linkStats() const` on `ConnectionManager` that sums the per-device
  counters. `LinkStats` is a small POD (bytes in, frames extracted, dropped, checksum errors,
  overflow bytes). Called at 1 Hz only — no caching, no signal.
- **Verify:** `python scripts/code-verify.py --check` on the three files; confirm no new
  connection and no call site on the frame path.
- **Deps:** T4
- [x] done — `DeviceManager::frameReader()` (inline, null between reconfigure and open) and
      `IO::LinkStats` + `ConnectionManager::linkStats()` summing over `m_devices`. No caching,
      no signal, no frame-path call site.

### T6 — Link checkers

- **Files:** `app/src/Misc/Problems/LinkCheckers.h`, `app/src/Misc/Problems/LinkCheckers.cpp`,
  `app/CMakeLists.txt`
- **Does:** Implements the spec R9 checks against a **previous-sample delta**, not absolute
  totals (a reconnect recreates `FrameReader` and resets the counters — a decrease means
  reset, not negative rate): bytes received but no frames extracted over a sustained window;
  frames extracted but none parsed; checksum failure rate above threshold; frame-queue drops;
  ring-buffer overruns. Suppresses itself while a player is open (replay bypasses
  `FrameReader`). Finding text is bucketed so it stays stable while the condition is stable.
- **Verify:** `python scripts/code-verify.py --check` on both files; read back the
  decrease-means-reset handling and the player suppression.
- **Deps:** T1, T2, T5
- [x] done — one `link.statistics` checker on `LinkSample | OnDemand`; any counter decrease (or a
      `parsedFrameCount()` decrease) means "reader recreated" and rebases instead of subtracting;
      suppressed by `SerialStudio::isAnyPlayerOpen()` or a closed link; the sampler advances at
      most once per 500 ms so an on-demand re-run cannot collapse the sustained windows; all
      counts reported as decade buckets and the checksum rate as a coarse band.
      `app/CMakeLists.txt` entries are owned by the coordinator (listed in handoff).

### T7 — Script engine error counters

- **Files:** `app/src/DataModel/Scripting/IScriptEngine.h`,
  `app/src/DataModel/Scripting/JsScriptEngine.{h,cpp}`,
  `app/src/DataModel/Scripting/LuaScriptEngine.{h,cpp}`
- **Does:** Adds `errorCount()`, `lastError()`, `consecutiveTimeouts()`, `disabled()`,
  `resetErrorStats()` to the `IScriptEngine` interface (plain virtuals — the interface stays a
  non-`QObject`) and implements them in both engines, setting `m_errorCount` / `m_lastError`
  in the error branches that already build the message string. Native/`CFrameParser` returns
  zero and an empty string.
- **Verify:** `python scripts/code-verify.py --check` on all five files; confirm no `Q_OBJECT`
  was added to the interface and no allocation was introduced on a success path.
- **Deps:** none
- [x] done — defaulted virtuals on the non-`QObject` interface (Native inherits zero/empty);
      JS + Lua implement them and record in their existing error branches. `CFrameParser::
      lastError()` gained `override` (it already matched the new virtual's signature).

### T8 — FrameParser and FrameBuilder script statistics

- **Files:** `app/src/DataModel/Scripting/FrameParser.{h,cpp}`,
  `app/src/DataModel/FrameBuilder.{h,cpp}`
- **Does:** Adds `[[nodiscard]] QList<ScriptStat> scriptStats() const` to `FrameParser`, walking
  the per-source engines. Adds `m_transformErrors`, `m_lastTransformError`,
  `m_lastTransformDatasetUniqueId` to `FrameBuilder` with accessors; the counter increments in
  the existing transform error branches and the message string is captured **only when the
  failing dataset differs from the last recorded one**, so a dataset that throws every frame
  allocates once, not per frame.
- **Verify:** `python scripts/code-verify.py --check` on the four files; read back the
  first-failure-per-dataset guard.
- **Deps:** T7
- [x] done — `FrameParser::scriptStats()` walks the per-source engines; `FrameBuilder::
      noteTransformError()` increments a plain counter and assigns the message only when the
      failing dataset differs from the last recorded one. Stats reset with the engines.

### T9 — Script checkers

- **Files:** `app/src/Misc/Problems/ScriptCheckers.h`, `app/src/Misc/Problems/ScriptCheckers.cpp`,
  `app/CMakeLists.txt`
- **Does:** Implements the spec R10 checks: parser errors per source with count and last
  message; a disabled engine (consecutive-timeout cutoff reached); transform errors with count
  and last message. Bucketed counts, stable text.
- **Verify:** `python scripts/code-verify.py --check` on both files.
- **Deps:** T1, T2, T8
- [x] done — `script.parser` (per-source: watchdog-disabled engine as Error, repeated failures as
      Warning) and `script.transform` (failing per-dataset transform, jumps to the dataset), both
      on `LinkSample | OnDemand`; counts bucketed by decade, retained error text passed through.
      `app/CMakeLists.txt` entries are owned by the coordinator (listed in handoff).

### T10 — Problems API handler

- **Files:** `app/src/API/Handlers/ProblemsHandler.h`, `app/src/API/Handlers/ProblemsHandler.cpp`,
  `app/CMakeLists.txt`
- **Does:** Adds the GPL (dual-license SPDX) static-only handler with `problems.list`
  (`severity?`, `checkerId?`, `limit?` default 50 max 200), `problems.run`, and
  `problems.listCheckers`, built with `API/SchemaBuilder.h` and following `ScriptsHandler`'s
  shape, including the trailing `hint` string convention.
- **Verify:** `python scripts/code-verify.py --check` on both files; confirm the schemas use
  `SchemaBuilder` helpers and the class has no state.
- **Deps:** T1
- [x] done — stateless `API::Handlers::ProblemsHandler` with `problems.list` (severity/checkerId/
      limit, default 50, capped at 200), `problems.run` and `problems.listCheckers`; schemas built
      with `SchemaBuilder`, shared payload helper, trailing `hint` on every result.

### T11 — Register the handler

- **Files:** `app/src/API/CommandHandler.cpp`
- **Does:** Adds the `#include` and `Handlers::ProblemsHandler::registerCommands();` in the GPL
  block of `initializeHandlers()`. Does **not** add any name to `destructiveCommandSet()`.
- **Verify:** `python scripts/code-verify.py --check app/src/API/CommandHandler.cpp`; read back
  that the call sits before the `#ifdef BUILD_COMMERCIAL` block.
- **Deps:** T10
- [x] done — `#include` sorted before `ProjectHandler.h`, `registerCommands()` called after
      `AssistantHandler` and before the `#ifdef BUILD_COMMERCIAL` block; `destructiveCommandSet()`
      untouched.

### T12 — Assistant safety tiers and scope description

- **Files:** `app/rcc/ai/command_safety.json`, `app/src/AI/ToolDispatcher.cpp`
- **Does:** Adds the three command names to the `"safe"` array (alphabetically) — an untagged
  name resolves to `Confirm` and would prompt the user on a read-only call — and adds a
  `problems` entry to `scopeDescriptions()`, since this is a new top-level scope.
- **Verify:** `python scripts/code-verify.py --check app/src/AI/ToolDispatcher.cpp`;
  `python -c "import json; json.load(open('app/rcc/ai/command_safety.json'))"`.
- **Deps:** T10
- [x] done — all three names added to the `"safe"` tier (and to no other); `problems` scope blurb
      added to `scopeDescriptions()` next to `notifications`. Manifest still parses as JSON.

### T13 — Runnable static test

- **Files:** `tests/scripts/test_problem_center_static.py`
- **Does:** Asserts the three commands are in `command_safety.json`'s `"safe"` tier and in no
  other tier, and that `ToolDispatcher.cpp` carries a `problems` scope description. Mirrors
  `tests/scripts/test_ai_assistant_static.py`. (The manifest/binding assertions are added to
  this file by T15, once those files exist.)
- **Verify:** `pytest tests/scripts/test_problem_center_static.py -v` — the agent runs this.
- **Deps:** T12
- [x] done — 7 tests, all green: safe tier, single-tier, C++ registration, GPL-block ordering,
      no `BUILD_COMMERCIAL` guard, scope description, and not in `destructiveCommandSet()`.
      T15 still owes this file the manifest/binding assertions.

### T14 — Problem center window

- **Files:** `app/qml/Dialogs/ProblemCenter.qml`, `app/qml/main.qml`, `app/CMakeLists.txt`
- **Does:** Adds the `Widgets.SmartWindow { category: "Problems" }` panel binding a `ListView`
  directly to `Cpp_Misc_ProblemCenter` (severity icon, title, explanation, remedy, "Go to"
  when `jump !== ""`, severity filter, refresh button, empty state), the `DialogLoader` and
  `app.showProblemCenter()` in `main.qml`, and the `QML_SOURCES` entry. Icon requests are
  16 px in 16 px render slots.
- **Verify:** `python scripts/code-verify.py --check app/qml/Dialogs/ProblemCenter.qml
  app/qml/main.qml`; `python scripts/registry-verify.py` (icon render-size lint).
- **Deps:** T1, T2
- [x] done — `Dialogs/ProblemCenter.qml` is a `Widgets.SmartWindow { category: "ProblemCenter" }`
      (severity icons, explanation, remedy, "Go To", severity filter, Refresh, empty state, last-run
      footer) hosted by a `DialogLoader` in `main.qml` with `app.showProblemCenter()`; the loader is
      also closed on quit. `app/CMakeLists.txt` entry is owned by the coordinator (listed in the
      handoff). Icon requests are 16 px in 16 px slots (48 px in the 48 px empty-state slot).

### T15 — Command manifest entry and bindings

- **Files:** `app/rcc/commands/app.json`, `app/qml/Commands/AppCommandBindings.qml`,
  `app/qml/Commands/ProjectEditorCommandBindings.qml`,
  `tests/scripts/test_problem_center_static.py`
- **Does:** Adds the `app.problems` manifest entry (`kind: "action"`,
  `contexts: ["app","dashboard","editor"]`, `category: "tools"`,
  `icon: "notifications/warning"` — already ships all four tiers, so no new SVG and no
  `rcc.qrc` change) and the matching `map` line + `QtObject` in both binding files, mirroring
  `app.helpCenter`. No layout-manifest edit; no `Cpp_CommercialBuild` guard. Extends
  `tests/scripts/test_problem_center_static.py` with the manifest-entry and binding-map
  assertions.
- **Verify:** `python scripts/registry-verify.py`;
  `python scripts/generate-command-strings.py --check`;
  `pytest tests/scripts/test_problem_center_static.py -v`.
- **Deps:** T13, T14
- [x] done — `app.problems` added to `app/rcc/commands/app.json` (action, tools, order 10,
      `notifications/warning`, three contexts) with bindings in `AppCommandBindings.qml` (serves the
      app and dashboard palettes) and `ProjectEditorCommandBindings.qml`. No layout-manifest edit, no
      commercial guard, no new SVG. `generate-command-strings.py` re-run; the regenerated
      `CommandStrings.cpp` also picks up the pending spec-0031 Undo/Redo strings and drops a stale
      "Recover" entry. Static test extended (10 tests green).

### T16 — Taskbar severity indicator

- **Files:** `app/qml/MainWindow/Panes/Dashboard/Taskbar.qml`
- **Does:** Adds the indicator next to the MQTT one following the same
  `Loader` → `Widgets.IconButton` → upward `Popup` recipe, with the rounded-`Label` badge from
  `NotificationLog.qml`, `visible: Cpp_Misc_ProblemCenter.totalCount > 0`, opacity keyed to the
  highest present severity, and an "Open problem center" button.
- **Verify:** `python scripts/code-verify.py --check app/qml/MainWindow/Panes/Dashboard/Taskbar.qml`;
  `python scripts/registry-verify.py` (render-size lint).
- **Deps:** T14
- [x] done — indicator sits next to the MQTT one; no `Loader` wrapper because the feature is GPL
      (nothing to gate on `Cpp_CommercialBuild`). Icon tracks the highest present severity, badge
      carries the error count, upward `Popup` shows the three counts plus "Open Problem Center".

### T17 — Jump-to-entity navigation

- **Files:** `app/qml/Dialogs/ProblemCenter.qml`, `app/qml/main.qml`
- **Does:** Handles `Cpp_Misc_ProblemCenter.jumpRequested(kind, uniqueId)` in QML: opens the
  project editor via `app.showProjectEditor()` and selects the entity by `uniqueId`, or opens
  the matching settings dialog for a `settings/<page>` target. The C++ side never calls into
  the editor.
- **Verify:** `python scripts/code-verify.py --check` on both files; read back that no C++ file
  gained an editor dependency.
- **Deps:** T3, T14
- [x] done — `main.qml` handles `onJumpRequested` and routes to the editor:
      `settings/<page>` opens preferences, otherwise `showProjectEditor()` then
      `selectSource`/`selectAction`/`selectGroup`, or `selectDatasetByUniqueId()` which maps a
      dataset uniqueId to the `(groupId, datasetId)` pair `selectDataset()` takes. **Checker
      contract for T3:** `entityUniqueId` is a `sourceId` for `jump == "source"`, an `actionId` for
      `"action"`, a positional `groupId` for `"group"`, and a dataset `uniqueId` for `"dataset"`.
      No C++ file gained an editor dependency.

### T18 — Integration tests

- **Files:** `tests/integration/test_problem_center.py`
- **Does:** Writes the eight acceptance tests named in `plan.md` (AC2-AC7) using the
  `api_client` / `device_simulator` / `clean_state` fixtures and the `tests/README.md` delay
  table; link tests wait at least two 1 Hz ticks before asserting. The **maintainer** runs
  them against a live app.
- **Verify:** file written and self-consistent; listed for the maintainer in the handoff. The
  agent does not run it.
- **Deps:** T3, T6, T9, T11
- [x] done — 8 tests, one per plan.md name, `black`-formatted and `py_compile`-clean. Markers:
      `project` on the four project/API tests, `network` + `slow` on the three link/script tests
      (`integration` is added automatically by `conftest.pytest_collection_modifyitems`). The link
      tests stream in ~1 s rounds and call `problems.run` between rounds instead of sleeping
      blind, so three sustained samples accumulate well inside the 30 s per-test timeout.
      Two deviations from `spec.md`, both following `plan.md`: `duplicate-frame-index` is asserted
      as **warning** (plan tradeoff — two datasets legitimately share an index), and the checksum
      finding is cleared by **reopening the link** rather than by diluting the rate, because the
      checker accumulates totals for the life of the reader. **Maintainer runs this file.**

### T19 — Documentation

- **Files:** `doc/help/API-Reference.md`, `doc/claude/architecture/dataflow.md`, `CLAUDE.md`
- **Does:** Adds a `### Problems Commands (3)` section to the API reference in the existing
  per-command format; records the 1 Hz diagnostics tick and the new `FrameReader` /
  `FrameBuilder` counters in `dataflow.md`; adds one line to `CLAUDE.md`'s hotpath block
  stating that link and script diagnostics are polled at 1 Hz and must never signal per frame.
- **Verify:** `python scripts/documentation-verify.py`; read the `.doc-report`.
- **Deps:** T11, T6, T9
- [x] done — `doc/help/API-Reference.md` gained `### Problems Commands (3)` (finding-field table +
      per-command entries, placed after the workspace section, GPL count list updated);
      `doc/claude/architecture/dataflow.md` gained "Diagnostic Counters — Pulled at 1 Hz
      (spec 0033)" between the cached-flags and replay sections; `CLAUDE.md` gained one hotpath
      bullet. Also (outside the task's file list, called out here): `tests/README.md` registers
      `test_problem_center.py` in the file table and the tree, and one stale value in
      `doc/claude/architecture/project.md` was corrected — `kSchemaVersion` reads 3
      (`Frame.h:868`), not 1. `python3 scripts/documentation-verify.py`: 121 files, 0 findings.

## Definition of Done

<The whole-feature gate, checked once every task is complete.>

- [ ] Every acceptance criterion in `spec.md` is met and checked off there (AC8/AC9 pending
      maintainer observation and `--benchmark-hotpath`). — AC1 and AC10 checked; AC2-AC7 are
      code-complete and await the maintainer's run of the integration file.
- [x] `python scripts/code-verify.py --check` is clean on all changed files (no new errors;
      no new advisories beyond the known `arch-singleton-instance` ratchet). — 0 errors across
      the problem-center C++/QML files; only `arch-singleton-instance` advisories, as predicted
      in `plan.md`'s risk list.
- [x] `python scripts/registry-verify.py` clean — manifest schema, ids, icon resolution,
      shortcut uniqueness, commercial-guard scan, and QML icon render-size lint. — `CLEAN`.
- [x] `python scripts/generate-command-strings.py --check` clean (no manifest/strings drift).
- [x] `pytest tests/scripts/test_problem_center_static.py -v` green (agent runs this). —
      10 passed.
- [ ] `qt-cpp-review` run on the C++ diff; findings addressed or noted.
- [ ] `ss-hotpath` re-read before T4 and T8; `--benchmark-hotpath` run by the maintainer with
      no regression on any of the nine gated tiers.
- [x] Spec-0001 ctor-edge proof re-run and recorded: `Misc::ProblemCenter` has zero outgoing
      constructor edges. — recorded in T1/T2; the constructor is member-init only.
- [x] `pytest tests/integration/test_problem_center.py` handed to the maintainer with the
      run instructions (app up, API server on 7777).
- [ ] `python scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [x] Maintainer-only follow-up flagged in the handoff: `SerialStudio --dump-api-schema
      app/rcc/api/api-schema.json` then re-run `sanitize-commit.py`, so the JS/Lua SDK picks
      up the three new commands.
- [ ] Diff is *what was asked, and only that* — no scope creep, no foreign files touched. The
      one deliberate adjacent fix (throttling the unthrottled checksum `qWarning`) is named
      in the commit message with its reason. — final read-through owed at commit time.
- [ ] `spec.md` status set to `done`. — stays `in-progress` until maintainer acceptance.
