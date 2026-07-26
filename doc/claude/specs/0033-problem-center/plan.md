---
spec: 0033-problem-center
phase: plan
status: draft        # draft -> approved (gate before /ss-tasks)
updated: 2026-07-25
---

# Plan 0033 — Problem center (project + link diagnostics)

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Read the relevant `doc/claude/` sub-docs and the *actual code*
> before writing this — a plan grounded in a stale mental model is worse than no plan.
> Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

Add one session-scoped singleton, `Misc::ProblemCenter`, that is simultaneously the collector
and the `QAbstractListModel` the UI binds to — one object, one registration, no separate model
class to keep in sync. Subsystems register checkers against it (`id`, trigger mask, callable);
the center runs the matching checkers on project-change signals, on the existing
`Misc::TimerEvents::timeout1Hz` tick, and on demand, replacing each checker's findings
wholesale so a fixed condition disappears by itself. Three built-in checker files (project
schema, link statistics, script errors) are the only things that know what a problem *is*; the
center knows nothing about them. Link and script findings are derived from **plain counters
polled at 1 Hz**, never from per-frame signals: the frame path gains four unconditional
integer increments and one first-failure-per-dataset string capture, and nothing else. The
panel is a standalone `SmartWindow` opened by one spec-0028 command bound in both the main
window and the project editor, a taskbar indicator mirrors the severity counts, and a
read-only `problems.*` API handler exposes the same list to MCP and the in-app assistant.

## Affected subsystems & files

| File | Change |
|------|--------|
| `app/src/Misc/ProblemCenter.h` | **New.** `Misc::ProblemCenter : QAbstractListModel` singleton: `Severity` / `Trigger` enums, `Finding` struct, `registerChecker()`, roles, severity-count properties, `runNow()`. |
| `app/src/Misc/ProblemCenter.cpp` | **New.** Model implementation, checker storage + dispatch, per-checker slice replacement, diffing against the previous run, NotificationCenter summarization, `setupExternalConnections()`. |
| `app/src/Misc/Problems/ProjectCheckers.h/.cpp` | **New.** Project-schema checkers (R8 set) + `registerAll()`. |
| `app/src/Misc/Problems/LinkCheckers.h/.cpp` | **New.** Link-statistics checkers (R9 set), holds the previous 1 Hz counter snapshot. |
| `app/src/Misc/Problems/ScriptCheckers.h/.cpp` | **New.** Parser/transform error checkers (R10 set). |
| `app/src/Misc/ModuleManager.cpp` | Add `(void)Misc::ProblemCenter::instance();` to `instantiateCoreModules()`; add `ProblemCenter::instance().setupExternalConnections();` to `setupCrossModuleConnections()`; add `ctx->setContextProperty("Cpp_Misc_ProblemCenter", ...)` in `registerCoreContextProperties()`. |
| `app/src/IO/FrameReader.h/.cpp` | Add `m_bytesIn`, `m_framesExtracted`, `m_checksumErrors`, `m_totalOverflowBytes` (+ accessors, + reset); accumulate overflow before the existing `resetOverflowCount()` at `FrameReader.cpp:110`; throttle the currently-unthrottled per-failure checksum `qWarning` at `FrameReader.cpp:569` to the 5 s `noteDroppedFrame` pattern. |
| `app/src/IO/DeviceManager.h/.cpp` | Add `[[nodiscard]] FrameReader* frameReader() const noexcept` — `m_frameReader` is private today with no accessor, so link stats are unreachable. |
| `app/src/IO/ConnectionManager.h/.cpp` | Add `[[nodiscard]] LinkStats linkStats() const` aggregating over `m_devices` (called at 1 Hz only, never per frame). |
| `app/src/DataModel/Scripting/IScriptEngine.h` | Add `errorCount()`, `lastError()`, `consecutiveTimeouts()`, `disabled()`, `resetErrorStats()` to the interface (non-`QObject`, plain virtuals). |
| `app/src/DataModel/Scripting/JsScriptEngine.h/.cpp` | Implement the above; set `m_errorCount`/`m_lastError` in the existing error branches (`JsScriptEngine.cpp:339`, `:387`). |
| `app/src/DataModel/Scripting/LuaScriptEngine.h/.cpp` | Same, at `LuaScriptEngine.cpp:526` and `:597`. |
| `app/src/DataModel/Scripting/FrameParser.h/.cpp` | Add `[[nodiscard]] QList<ScriptStat> scriptStats() const` walking the per-source engines. |
| `app/src/DataModel/FrameBuilder.h/.cpp` | Add `m_transformErrors`, `m_lastTransformError`, `m_lastTransformDatasetUniqueId` + accessors; increment in the existing transform-error branches. |
| `app/src/API/Handlers/ProblemsHandler.h/.cpp` | **New.** GPL (dual-license SPDX) static-only handler: `problems.list`, `problems.run`, `problems.listCheckers`. |
| `app/src/API/CommandHandler.cpp` | One `#include` + one `Handlers::ProblemsHandler::registerCommands();` in the GPL block of `initializeHandlers()`. |
| `app/rcc/ai/command_safety.json` | Add the three command names to `"safe"` (every registered command must be in exactly one tier). |
| `app/src/AI/ToolDispatcher.cpp` | Add a `problems` entry to `scopeDescriptions()` — a new top-level scope otherwise gets an empty blurb in `meta.listCategories`. |
| `app/rcc/commands/app.json` | One manifest entry: `app.problems`, `contexts: ["app","dashboard","editor"]`, `icon: "notifications/warning"` (already ships all four tiers — no new SVG), `category: "tools"`. |
| `app/qml/Commands/AppCommandBindings.qml` | `"app.problems": root.cmdAppProblems` + the `QtObject`. |
| `app/qml/Commands/ProjectEditorCommandBindings.qml` | Same id, editor-side binding (mirrors `app.helpCenter` / `app.preferences`, which already carry `"editor"` in `contexts`). |
| `app/qml/Dialogs/ProblemCenter.qml` | **New.** `Widgets.SmartWindow` listing findings grouped by severity, with explanation, remedy, and an activate-to-jump action. |
| `app/qml/main.qml` | One `DialogLoader` + `function showProblemCenter()`. |
| `app/qml/MainWindow/Panes/Dashboard/Taskbar.qml` | Severity indicator following the MQTT indicator recipe (L942-1069): `Loader` → `Widgets.IconButton` → upward `Popup`, plus the rounded-`Label` badge recipe from `NotificationLog.qml` L140-156. |
| `app/CMakeLists.txt` | `SOURCES` / `HEADERS` entries for the six new C++ files + the handler; `QML_SOURCES` entry for `Dialogs/ProblemCenter.qml`. |
| `tests/integration/test_problem_center.py` | **New.** Maintainer-run acceptance tests (AC2-AC7). |
| `tests/scripts/test_problem_center_static.py` | **New.** Runnable static test: safety tiers, manifest entry, binding presence. |
| `doc/help/API-Reference.md` | New `### Problems Commands (3)` section. |
| `doc/claude/architecture/dataflow.md`, `doc/claude/architecture.md` | Record the 1 Hz diagnostics tick and the new hotpath counters. |
| `CLAUDE.md` | One line under the hotpath block: link/script counters are polled at 1 Hz, never signalled per frame. |

## Architecture & data flow

```
                          registerChecker(id, triggers, fn)
   ProjectCheckers ─┐
   LinkCheckers ────┼──────────►  Misc::ProblemCenter  (QAbstractListModel, main thread)
   ScriptCheckers ──┘                    │
   (R9 IO self-test later) ──────────────┘
                                         │  findings, replaced per checker
                                         ├──► QML panel      (Dialogs/ProblemCenter.qml)
                                         ├──► taskbar badge  (severity counts)
                                         ├──► NotificationCenter (new/escalated findings only)
                                         └──► API problems.list / problems.run

  triggers:
    ProjectChanged  ← ProjectModel::groupsChanged / frameDetectionChanged / jsonFileChanged
    LinkSample      ← Misc::TimerEvents::timeout1Hz
    OnDemand        ← problems.run, panel refresh button, (later) a failed connect attempt
```

**Core types** (`ProblemCenter.h`):

```cpp
enum Severity : int { Info = 0, Warning = 1, Error = 2 };            // == NotificationCenter::Level
enum Trigger  : quint8 { NoTrigger = 0, ProjectChanged = 1, LinkSample = 2, OnDemand = 4 };

struct Finding {
  Severity severity;
  int entityUniqueId;      // -1 when the finding has no project entity
  QString checkerId;       // owning checker
  QString code;            // stable sub-id, e.g. "duplicate-frame-index"
  QString title;           // one line, translated
  QString explanation;     // one or two sentences naming the concrete cause
  QString remedy;          // optional, "what to do", translated
  QString jump;            // "" | "dataset" | "group" | "action" | "source" | "settings/<page>"
};

using Checker = std::function<void(QList<Finding>&)>;
void registerChecker(const QString& id, quint8 triggers, Checker fn);
```

`registerChecker` stores `{id, triggers, fn}` in a `std::vector`. A run with trigger `T`
iterates the vector, calls every checker whose mask contains `T` into a scratch list, and
replaces that checker's contiguous slice of `m_findings`. The whole list is rebuilt in one
`beginResetModel`/`endResetModel` pair when the flattened result differs from the previous
one, and **not touched at all when it is identical** — a 1 Hz reset of an unchanged model
would repaint the panel every second. Equality is a field-wise compare of the flattened
vector; the list is bounded (see Risks) so the compare is cheap.

**Notification bridge.** After a run, findings present now but absent from the previous run
post to `NotificationCenter` on channel `"Problems"` at their own level; findings that
disappeared post nothing (the panel and badge already show the change). The center holds the
previous run's key set (`checkerId + code + entityUniqueId`) for the diff.

**Jump-to-entity.** `Q_INVOKABLE bool activate(int row)` resolves the finding's `jump` kind:
a project entity opens the project editor (`app.showProjectEditor()`) and asks
`ProjectEditor` to select the entity by `uniqueId`; a settings target opens the matching
dialog. The center itself does **not** call into the editor — it emits
`jumpRequested(QString kind, int uniqueId)` and QML performs the navigation, keeping the
center free of UI dependencies and keeping its constructor and its runtime both inert with
respect to the editor.

**Composition-root placement.** `ProblemCenter`'s constructor initializes members only: no
`instance()` call on any other module, no `connect`, no QSettings read, no timer. It is a
**leaf node with zero outgoing constructor edges**, so re-running the spec-0001 ctor-edge
proof is a formality — the node cannot participate in a cycle. It is added to
`instantiateCoreModules()` immediately after `DataModel::NotificationCenter::instance()`
(the two diagnostics services sit together) and must be in that list at all, rather than
lazily constructed, because `AppState::restoreLastProject()` runs at the end of
`setupCrossModuleConnections()` and its project-load signal must find the checkers already
registered. All wiring — the ProjectModel connections, the `timeout1Hz` connection, and
`ProjectCheckers/LinkCheckers/ScriptCheckers::registerAll()` — happens in
`setupExternalConnections()`, which `setupCrossModuleConnections()` calls after every module
exists.

## Hotpath & threading impact

- **Touches the hotpath? Yes, minimally — four unconditional integer increments and one
  guarded string capture.** The touched code is `FrameReader::processData` /
  `enqueueCaptured` / `checksum` and the `FrameBuilder` transform error branch, all
  main-thread. Specifically:
  - `m_bytesIn += data.size()` once per `processData` call (per chunk, not per frame).
  - `++m_framesExtracted` next to the existing `++m_droppedFrames` accounting.
  - `++m_checksumErrors` inside the existing `ValidationStatus::ChecksumError` branch, which
    is already the slow/failing path.
  - `m_totalOverflowBytes += overflow` at `FrameReader.cpp:109`, inside the existing
    `if (overflow > 0)` guard, immediately before the existing `resetOverflowCount()` that
    currently destroys the number.
  - `++m_transformErrors` in the transform error branch; `m_lastTransformError` is assigned
    **only when the failing dataset differs from the last recorded one**, so a dataset that
    throws every frame allocates once, not per frame.
  All counters are plain `quint64` members on main-thread objects — **no atomics, no
  mutexes, no `Q_EMIT`, no allocation** on any success path. `CircularBuffer`'s existing
  `std::atomic<qsizetype> m_overflowCount` (relaxed, SPSC) is untouched and remains the only
  atomic in that path.
- **New cross-thread signal/slot? No.** The only new connections are
  `TimerEvents::timeout1Hz → ProblemCenter::onLinkSample` and three ProjectModel change
  signals → `ProblemCenter::onProjectChanged`, all between main-thread objects, all
  `AutoConnection` resolving to direct. None of them is on the frame path, so the
  "hotpath hops must be Direct" rule is satisfied trivially rather than by exception.
- **No new per-frame signal traffic.** This is the load-bearing decision: the 65536-slot
  queue argument means link diagnostics must never emit per frame. Counters are *pulled* at
  1 Hz by `ConnectionManager::linkStats()` / `FrameParser::scriptStats()` /
  `FrameBuilder::parsedFrameCount()`, and the model only resets when the flattened finding
  list actually changed — the `MQTT::Publisher::emitStatsIfChanged` coalescing pattern
  (`Publisher.cpp:1880`), applied to a 1 Hz tick instead of 500 ms.
- **New input to a cached hotpath flag? No.** Nothing here reads or feeds `m_operationMode`,
  `m_playerOpen`, `m_anyAsyncSink`, `m_captureLatestFrame`, `m_changeDriven`, or Dashboard
  `m_streamAvailable`. No checker may gate frame processing; a checker that wanted to would
  be rejected in review.
- **Timestamp ownership** — unchanged. Findings carry a wall-clock discovery time from the
  collector for display ordering only; nothing re-stamps a frame, and no counter is derived
  from `CapturedData::timestamp`.
- **Benchmark plan.** `--benchmark-hotpath` before and after, comparing all nine gated tiers;
  the `datasets+publish` stage is 70-80% of per-frame time and the transform-error counter
  lives there, so the Lua-mixed and JS-mixed rows are the ones to watch. Expected delta:
  none measurable (the increments are on data already in L1 and the error branch is not
  taken in the benchmark's workload).

## Data model & persistence

Nothing persists. No `Frame.h` `Keys::` additions, no schema version bump, no writer-version
change, no `widgetSettings` blob, no Sessions DB table, no migration. The finding list is
rebuilt from scratch on every trigger and dies with the process. The one piece of persisted
state is the panel window's geometry, handled by the existing `SmartWindow` `category`
mechanism, and (optionally) the taskbar indicator's pinned state through the existing
`TaskbarSettings` list.

Project-file bytes are unchanged, which also means the rolling-backup SHA-1 arbiter and the
on-disk file-watcher hash are unaffected — the problem center never triggers a save, a
`setModified(true)`, or a `contentTouched`.

## API / SDK surface

New GPL handler `app/src/API/Handlers/ProblemsHandler.{h,cpp}`, static-only, following
`ScriptsHandler` (the closest read-only catalog precedent) and using `API/SchemaBuilder.h`:

| Command | Params | Returns |
|---------|--------|---------|
| `problems.list` | `severity?` (`info`/`warning`/`error`), `checkerId?`, `limit?` (default 50, max 200) | `{findings[], counts{info,warning,error}, total, hint}` — each finding `{severity, checkerId, code, title, explanation, remedy, entityUniqueId, jump}` |
| `problems.run` | none | re-runs every checker (all triggers) and returns the same shape as `list` |
| `problems.listCheckers` | none | `{checkers[{id, triggers[]}], total}` — lets an agent (and R9) see what is registered |

- Registered in the GPL block of `CommandHandler::initializeHandlers()`; **not** added to
  `destructiveCommandSet()` (nothing mutates, no backup snapshot, no undo frame).
- `app/rcc/ai/command_safety.json`: all three go in `"safe"` — `AI::CommandRegistry::safetyOf`
  returns `Confirm` for any untagged name, which would make the assistant prompt on a
  read-only call.
- `app/src/AI/ToolDispatcher.cpp::scopeDescriptions()` gains a `problems` blurb; `problems` is
  the 20th top-level scope and would otherwise show an empty description in
  `meta.listCategories`.
- MCP tool schemas and the gRPC `.proto` are generated from `API::CommandRegistry` at
  runtime — **no regeneration needed** for either.
- `api-schema.json` and the JS/Lua SDK only pick the commands up after the maintainer runs
  `SerialStudio --dump-api-schema app/rcc/api/api-schema.json` followed by
  `scripts/sanitize-commit.py`. That is a maintainer step, called out in `tasks.md` and not
  attempted by the implementing agent.
- Script `apiCall` reach comes for free through the registry; the commands are read-only, so
  the "destructive verbs mid-parse" teardown hazard does not apply.

## QML / UI

**Panel: `app/qml/Dialogs/ProblemCenter.qml`, a `Widgets.SmartWindow { category: "Problems" }`
hosted by a `DialogLoader` in `main.qml`, opened by `app.showProblemCenter()`.** It binds a
`ListView` directly to `Cpp_Misc_ProblemCenter` (the singleton *is* the model), with a
severity icon from `Cpp_Misc_IconRegistry.icon("notifications", <level>, 16)`, the title, the
explanation, an optional remedy line, and a "Go to" action visible when `jump !== ""`. A
severity filter row and a Refresh button (calling `runNow()`) sit in the header. Empty state:
"No problems detected" with the last-run time.

**Taskbar indicator**: a `Loader` in `MainWindow/Panes/Dashboard/Taskbar.qml` placed next to
the MQTT indicator, `visible: Cpp_Misc_ProblemCenter.totalCount > 0`, icon opacity keyed to
the highest present severity, an unread-style rounded `Label` badge carrying the error count
(the `NotificationLog.qml` L140-156 recipe), and an upward `Popup` summarizing counts with an
"Open problem center" button. Icon requests are 16 px inside a 16 px render slot so
`registry-verify.py`'s render-size lint passes.

**Command (spec 0028)**: one manifest entry `app.problems` in `app/rcc/commands/app.json`
with `contexts: ["app","dashboard","editor"]` and `category: "tools"` — the exact shape
`app.helpCenter` and `app.preferences` already use — plus one binding in
`AppCommandBindings.qml` (serves the app and dashboard palettes) and one in
`ProjectEditorCommandBindings.qml` (serves the editor palette). No layout-manifest edit in
v1: the command reaches users through the palette and the Start menu without disturbing the
ribbon. `icon: "notifications/warning"` reuses an icon that already ships all four tiers, so
no new SVG and no `rcc.qrc` change. Translations are generated by
`generate-command-strings.py` inside `sanitize-commit.py`. No `Cpp_CommercialBuild` guard —
the feature is GPL.

**ComboBox restore races**: none — the panel has no persisted-index combo boxes. **Font
auto-scale**: the panel uses `Cpp_Misc_CommonFonts` like every other dialog.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Panel shape | (a) standalone `SmartWindow` via `DialogLoader`; (b) dashboard tool window like `NotificationLog`; (c) bottom panel inside the project editor; (d) third main-window column | **(a)**. `SerialStudio::DashboardNotificationLog` and its siblings sit inside the `#ifdef BUILD_COMMERCIAL` block of the `DashboardWidget` enum, so (b) cannot ship GPL without restructuring that enum. (c) reaches only project findings and only while the editor is open — link findings matter most when the editor is closed. (d) disturbs the `MainWindow.qml:586` minimum-width math. (a) is one QML file, works in QuickPlot and Console-only modes, and is reachable identically from both windows. |
| Collector vs model | separate `ProblemCenter` + `ProblemModel`; one class that is both | **One class.** A separate model would need a full change-propagation contract between two objects for zero benefit; `NotificationCenter`'s split (plain QObject + a QML-side `ListModel`) exists because notifications are append-only events, which findings are not. |
| Finding lifetime | append events and expire them; replace per checker each run | **Replace per checker.** R4 requires "fixed problems disappear by themselves". Event semantics would need TTLs and resolution matching — exactly the complexity `NotificationCenter` already carries and the reason it is the wrong model here. |
| Link stats transport | per-frame signal / callback into the center; counters polled on a tick | **Polled at 1 Hz** via `TimerEvents::timeout1Hz`. A per-frame hop between two main-thread objects is precisely the queued-connection failure mode the hotpath rules forbid, and even a direct hop adds a call per frame for data nobody reads more than once a second. |
| Tick source | new `QTimer` in the center; existing `timeout1Hz` | **Existing `timeout1Hz`.** It already drives `FrameBuilder` GC, `FrameParser::collectGarbage`, `Sessions::Export` snapshots, and four driver refreshes; adding a fifth subscriber costs nothing and keeps timer policy in one place. |
| Script error plumbing | promote `IScriptEngine` to `QObject` and emit per error; plain counters + `lastError` polled | **Counters.** `IScriptEngine` is deliberately not a `QObject` (it is on the parse path); a signal per failing frame is the same per-frame-traffic mistake. |
| Checksum log | leave the per-failure `qWarning`; throttle it | **Throttle to 5 s** (the `noteDroppedFrame` pattern). It is currently unthrottled and dumps up to 128 bytes of hex per failing frame, and `ModuleManager`'s message handler forwards warnings into `NotificationCenter` — a corrupt link floods both. The counter this spec adds carries the diagnostic value the log was providing, so throttling loses nothing. Same lines, same commit; called out here so it is not read as scope creep. |
| Constructor placement | lazy `instance()` on first use; entry in `instantiateCoreModules()` | **Entry in the list, with an inert ctor.** `restoreLastProject()` fires at the end of `setupCrossModuleConnections()` and must find checkers registered. The inert ctor (zero outgoing edges) makes the spec-0001 proof re-run trivial. |
| API scope name | `problems.*`; `diagnostics.*` | **`problems.*`**, matching the roadmap wording and the panel name. Flagged in `spec.md` Open Questions because roadmap R9 uses "connection diagnostics"; if the maintainer prefers `diagnostics.*`, rename before implementation, not after. |
| Duplicate frame index severity | error; warning; information | **Warning**, with the explanation naming both datasets. Two datasets legitimately share an index when one value drives two widgets, so `Error` would produce false positives on correct projects. Open Question in `spec.md`. |

## Risks & mitigations

- **Silent breakage: `FrameReader` is recreated, not reused.** `resetFrameReader()` /
  `DeviceManager::reconfigure()` construct a fresh reader on every connect and on config
  changes, so the new counters restart at zero. The link checkers must therefore work on
  **deltas against the previous 1 Hz snapshot and treat a decrease as a reset**, not on
  absolute totals, or every reconnect fabricates a huge negative rate. This is the single
  most likely bug in the feature.
- **A 1 Hz `beginResetModel` would repaint the panel every second.** Mitigated by the
  equality compare before touching the model; a checker that returns a freshly-formatted
  string containing a live counter (e.g. "1234 frames dropped") would defeat it. Rule for
  checker authors, stated in the checker header: **the finding text must be stable while the
  condition is stable** — put changing numbers in bucketed form ("more than 1000 frames
  dropped"), not exact live counts.
- **Notification flood on project load.** A project with fifteen warnings would post fifteen
  events. Mitigated by posting one aggregated event per run ("3 problems found in this
  project") rather than one per finding, with the panel holding the detail. Confirmed as an
  Open Question in `spec.md`; the plan's default is the aggregate.
- **Unbounded finding lists.** A broken project could produce a finding per dataset. Each
  checker caps its own output (proposed: 50 findings per checker, with a trailing
  "and N more" finding), which also bounds the equality compare.
- **`arch-singleton-instance` advisory count rises.** The new files add `instance()` call
  sites; the rule is advisory with ~1,850 existing sites and is a ratchet, not a gate. The
  checkers take their dependencies as locals inside the run function rather than caching
  singleton references as members, which keeps the ctor inert.
- **Ctor-edge proof.** Any future edit that adds a call into another module from
  `ProblemCenter`'s constructor re-triggers the spec-0001 proof. The header carries a comment
  stating the constructor is intentionally inert.
- **Operation-mode blind spots.** In QuickPlot and Console-only there is no project document;
  the project checkers must return no findings rather than reporting an empty project as
  broken. Explicit guard on `ProjectModel::groupCount()` plus the operation mode.
- **Multi-source projects.** Frame-index and reference checks are **per source** — two
  datasets with index 3 in different sources are correct. Getting this wrong turns every
  multi-source project into a wall of false positives.
- **Player/replay mode.** While a player is open, transform engines are destroyed and frames
  arrive through `replayChannels`, bypassing `FrameReader` entirely. Link checkers must
  suppress themselves while `m_playerOpen`-equivalent player state is active, or replay
  reports "bytes received, no frames extracted" forever. (Read, do not write, the player's
  public open state — no new input to a cached flag.)
- **Pro-gated subsystems.** A checker that inspects a commercial subsystem compiles out with
  it (`#ifdef BUILD_COMMERCIAL` around that checker's registration only), never around the
  center, the model, the panel, or the handler.

## Test & verification plan

- **Unit (agent can run):** `pytest tests/scripts/test_problem_center_static.py` — asserts the
  three command names appear in `command_safety.json`'s `"safe"` array and in no other tier,
  that `app.problems` exists in `app/rcc/commands/app.json` with the three contexts, and that
  both binding files map the id. Mirrors `tests/scripts/test_ai_assistant_static.py`.
- **Integration (maintainer runs, app up with the API server on 7777):**
  `pytest tests/integration/test_problem_center.py -v` —
  `test_duplicate_frame_index_reported_once` (AC2),
  `test_finding_clears_when_condition_fixed` (AC2),
  `test_dangling_xaxis_reference_has_jump_target` (AC3),
  `test_empty_group_and_inverted_range_are_warnings` (AC3),
  `test_delimiter_mismatch_reports_no_frames_extracted` (AC4, TCP loopback),
  `test_checksum_failure_rate_reported_and_cleared` (AC5),
  `test_failing_transform_reports_error_text_and_count` (AC6),
  `test_problems_run_refreshes_list` (AC7).
  All use the `api_client` / `device_simulator` / `clean_state` fixtures and the delay table
  in `tests/README.md`; link tests need at least two 1 Hz ticks, so they wait ≥ 2.5 s before
  asserting.
- **Hotpath:** `--benchmark-hotpath` before and after, all nine gated tiers, maintainer runs.
  Any regression on the Lua-mixed / JS-mixed rows points at the transform-error counter.
- **Static:** `python scripts/code-verify.py --check` on every changed C++/QML file;
  `python scripts/registry-verify.py` after the manifest and binding edits;
  `qt-cpp-review` on the C++ diff before handoff; `python scripts/sanitize-commit.py` before
  commit (it also regenerates `CommandStrings.cpp` and re-runs `registry-verify.py`).
- **Maintainer observation (AC8):** palette entry present in both windows, indicator badge
  counts correct, jump lands on the right entity, one aggregated notification per run with
  new findings and none on an unchanged re-run.
