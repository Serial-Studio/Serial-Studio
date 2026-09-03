# WP-I handoff — one implementation per concern (2026-09-02)

All eleven tasks are done and ticked. Worked on the merged integration tree (WP0 + WP-A..H).
Every static gate is green and the three censuses were re-seeded at the end (coordinator item 11).

## Gate state at handoff

| Gate | Result (was, at WP-I start) |
|------|------|
| `code-verify.py --check` | 0 errors, 159 advisory (24) |
| `--tu-census --check` | 2775 excess (3386), worst 2983 (3269) — re-seeded |
| `--singleton-census --check` | 1550 (1571), static-cache 1069 (1088) — re-seeded, did not grow |
| `--dup-census --check` | 1639 shared windows over 20 pairs (2444 over 33) — re-seeded |
| `claim-verify.py --quiet` | 9 errors, **0 new** (the startup.md ordered anchor WP-J owns) |
| `registry-verify.py` | CLEAN |
| `generate-property-registry.py --check` | up to date |
| `pytest tests/scripts scripts/tests` | 529 passed (525) |
| `pytest tests --collect-only` | 1764 (1762) |

The 159 advisories are 149 newly-visible `id-placement` (see coordinator item 8), 9
`cxx-tu-too-long` and 1 pre-existing `doc-verbose-brief` in `PainterGeometry.h`. No advisory sits
in a file this package created.

## New file pairs

| Path | Role |
|------|------|
| `app/src/IO/Drivers/PolledPlcWorkerBase.{h,cpp}` | Protocol-independent half of a polled-PLC worker: abort latch, poll timer, change-latch table, OpcUaWire delta encoder, the three pulled counters, report-once link loss, one-shot dial verdict. |
| `app/src/AI/Providers/OpenAICompatibleProvider.{h,cpp}` | One adapter for every vendor speaking OpenAI Chat Completions with a bearer key; `OpenAICompatibleVendor` is the table row. |
| `app/src/DataModel/ReplayPlaybackEngine.{h,cpp}` | Scrub timer chain, playback epoch, steady-clock anchor, catch-up fill gate, seek-window walk, `formatTimestamp`. |
| `app/src/DataModel/ExportStructure.{h,cpp}` | Template-frame adoption (both lanes), `sanitizeTitle`, `sessionDir`. |
| `app/src/DataModel/FrameBuilder/BlockPublisher.{h,cpp}` | The publish fan-out: dashboard hop, cached any-async-sink flag, the ONE trimmed copy every sink shares. Sinks injected by reference. |
| `app/src/DataModel/FrameBuilder/ReplayIngest.{h,cpp}` | The replay column map and the two replay cell writers, with their shared value tail. |
| `app/src/IO/ConnectionManager/DeviceIoRouter.{h,cpp}` | What crosses the device link and how it is framed: delimiters + checksum, inbound payload/device taps, outbound writes with reply capture. |
| `app/src/IO/ConnectionManager/DeviceTableQuery.{h,cpp}` | Every read over the live device table (open counts, link state, `LinkStats`, config verdict, id lookups). Read-only by construction. |
| `app/src/AI/Conversation/ToolTurnRunner.{h,cpp}` | The tool half of a turn: pending Confirm table, outstanding-result counter, ToolCallCard payloads. |
| `app/src/UI/Widgets/Plot3D/Plot3DNodes.h` (+ rewritten `.cpp`) | The 3D plot's scene-graph node slots, texture-upload handshake and eye material. |
| `app/src/UI/Widgets/PlotBase.{h,cpp}` | Interpolation mode, visible-X window, log-X scratch and sweep config shared by Plot/MultiPlot/FFTPlot. |
| `app/src/Misc/ContextRegistry.{h,cpp}` | The one QML-globals table plus the collect-then-apply loop. |
| `app/src/Sessions/ReportOptionsModel.{h,cpp}` | The report dialog's dataset picker as a real `QAbstractListModel`. |
| `app/qml/Dialogs/ExtensionManager/{Grid,Detail,Repos}Page.qml` | The three pages of the extension manager. |
| `app/qml/AI/AssistantPanel/{AssistantTopBar,AssistantConversation,AssistantComposer}.qml` | The assistant panel's three sections. |
| `app/qml/Widgets/Dashboard/InstrumentBase.qml` | Model-agnostic instrument chrome: theme stops + tick math. |
| `app/qml/Widgets/Dashboard/SwipePages.qml` | Per-widget swipe-page persistence. |
| `app/qml/ProjectEditor/Views/CodeEditorMenu.qml` | The embedded code editors' right-click menu. |
| `app/qml/MainWindow/Panes/SetupPanes/Drivers/DriverTagPickerDialog.qml` | The frameless-window chrome every driver picker/preview dialog wears. |

**Deleted:** `app/src/AI/Providers/{DeepSeek,Groq,Mistral,OpenRouter}Provider.{h,cpp}` — eight
files, replaced by four static vendor tables.

## Facade sizes

| Facade | Before | After | Under 1500? |
|--------|--------|-------|-------------|
| `AI/Conversation.cpp` | 1569 | **1483** | yes |
| `IO/ConnectionManager.cpp` | 1746 | **1589** | no — see below |
| `DataModel/FrameBuilder.cpp` | 3269 | **2983** | no — see below |

**ConnectionManager, the 89 lines that cannot come out.** `DeviceIoRouter` and
`DeviceTableQuery` took 157 lines. What remains is connect/disconnect orchestration, and the
blockers are specific symbols, not judgement:

- `onDriverOpenFinished` calls `QObject::sender()` to resolve which driver reported. That cannot
  leave the QObject.
- `notifyConnectedStateChanged`, `disconnectDevice(int)`, `disconnectDevice(HAL_Driver*)`,
  `setBusType` and `rebuildDevices` contain `Q_EMIT connectedChanged` / `connectingChanged` /
  `busTypeChanged` / `sessionClosed`. Q_EMIT requires the QObject.
- `wireUiDriver`, `wireDevice`, `wireStreamLifecycle`, `setupExternalConnections`,
  `buildDeviceForSource`, `dropUnavailablePrimaryDevice` and `setBusType` call `connect(...)`
  with `this` as receiver/context; the connection's lifetime is the facade's.
- The remaining 169-line block is `uart()`, `network()`, `bluetoothLE()`, `audio()`, `canBus()`,
  `hid()`, `modbus()`, `opcUa()`, `process()`, `s7()`, `ethernetIp()`, `iec104()`, `usb()`,
  `mqtt()`, `activeUiDriver()`, `uiDriverForBusType()` — the facade's own Q_INVOKABLE QML API,
  which you allowed to stay, and whose definitions cannot move to a second TU without creating the
  headerless continuation that "one class = one .h/.cpp pair" bans.

**FrameBuilder, the 1483 lines that cannot come out under this task's scope.** `BlockPublisher`,
`ReplayIngest` and the `parseProjectFrame` collapse took 286 lines. The rest is two clusters you
did not authorise me to extract and that I would not extract blind:

- The dataset-apply cluster (`decodeProjectChannels`, `applyDatasetValue`,
  `applyDatasetValueSpan`, `beginDatasetPass`, `endDatasetPass`, `reprocessDatasetValues`, the two
  `applyDatasetValuesSpans` overloads, `parseQuickPlotFrame`) — 683 lines, and
  `applyDatasetValueSpan` is `SS_HOT` on the 1.024 MHz Native tier.
- The transform dispatch (`applyTransform`, `applyTransformLua/Expr/Js`, `compileTransforms`,
  the engine lifecycle) — 318 lines holding the `m_compileGuard` re-entry contract.

Of the plan's "four apply tails", two are collapsed (both replay tails now share
`ReplayIngest::publishDatasetValue`). The two live tails (`applyDatasetValue`,
`applyDatasetValueSpan`) are the per-cell hot loop; sharing a tail there costs a call unless
force-inlined, and the only check is `--benchmark-hotpath`, which you run.

## Tests added / changed

| Suite | Covers |
|-------|--------|
| `app/tests/tst_ethernetip_worker.cpp` (new) | `PolledPlcWorkerBase` via a scripted stub: unchanged values cost no wire entry, only dirty channels reach the frame, dirty marks are consumed by the publish, counters accumulate, link loss reported once, dial verdict emitted once per attempt with its reason, abort latch holds until cleared. |
| `app/tests/tst_replay_playback_engine.cpp` (new) | `ReplayPlaybackEngine`: window covers the plot range, `points()` is a floor, an untimed row ends the walk, the epoch retires a superseded chain, replay time is the recording's time, the catch-up fill is gated, a scrub arms one tick then one settle, the label never goes negative. |
| `app/tests/tst_export_structure.cpp` (new) | `ExportStructure`: an empty frame never wipes an adopted template, a published structure only fills an empty slot, an open file keeps its schema, a title cannot escape the workspace, a title that scrubs away falls back. |
| `scripts/tests/fixtures/id-placement/{bad,good}.qml` (new) | The `id-placement` rule, so it cannot go dead again; its `UNFIXTURED` entry is removed. |
| `tests/scripts/test_widget_manifests.py` | Context-name test re-pointed at `registry.add`; two new tests for stale-table drift and for `hostContextNames()` forwarding. |
| `tests/scripts/test_cpp_regressions.py` | Three pins re-targeted to the code's new homes: the dashboard hop now pins `BlockPublisher.cpp`, the reply-error confirmation clear pins `m_tools.clearPending()`, and the source-owns-time pin is whitespace-insensitive (clang-format re-aligned the assignment when the two `parseProjectFrame`s collapsed). |

**Naming note:** `tasks.md` calls the engine suite `tst_replay_seek_engine`, but that name was
already taken in this tree by WP-E's suite for `UI::ReplaySeekEngine` (a different class). Mine is
`tst_replay_playback_engine`. `tst_sessions_player_epoch` and `tst_csv_player_catchup` were not
written separately: the epoch and the catch-up gate now live in `ReplayPlaybackEngine` and are
covered there. `test_replay_timeline.py` was not written (needs a running app).

## Coordinator items

1. **`tst_ethernetip_worker` against `PolledPlcWorkerBase`** — done.
2. **Wildcard disconnects** — four cleared (`S7.cpp`, `EthernetIp.cpp`, `MDF4/Player.cpp`, and
   `DeviceManager.cpp`, which WP-C had already fixed without lowering the baseline). The baseline
   in `code_verify_rules.py` is now 15 sites in 12 files, down from 19 in 16. The remaining twelve
   files are outside every WP-I task's file list; `OpcUaSubscriptions::unbindSession` is one.
3. **Four Lua hook implementations onto `LuaDeadlineHook`** — not done. The mechanical part is
   easy (`bind()` + `enable()` replacing each `lua_sethook`), but `bind()` calls
   `lua_newuserdata`, which its own doxygen says must run inside the caller's protected bootstrap;
   in `LuaScriptEngine::createState` the `lua_sethook` happens *after* `guardedPcall` returns, so
   adopting the shared hook means moving the bind into `bootstrapEngineState`. It also changes the
   timeout error string, which `tst_lua_deadline_hook` and the Lua compat suites may pin. Four
   engines on the per-frame parse path, no compiler: deferred with this reason.
4. **`ExtensionRowsModel` split (F14)** — not done; `ExtensionData` was not in any WP-I task's
   file list and `PlotBase`/`Plot3DNodes` consumed T7's budget.
5. **`wireAsyncSinkHooks` singleton advisories** — done; sinks arrive as an injected `AsyncSinks`
   struct resolved in `setupExternalConnections` (a sanctioned site for the lint), and
   `BlockPublisher` then takes them as `BlockPublisher::Sinks`.
6. **`DashboardIngest` owned state** — left as-is (explicitly optional).
7. **`Terminal.cpp` 1988 lines** — left as-is (explicitly a follow-up).
8. **Dead `id-placement` rule** — fixed and fixtured. `_check_shallow_id` broke out of its walk at
   the first content line, so `shallow_id_idx` could only ever equal `first_content_idx` and the
   branch was unreachable. The walk now continues at shallow depth until it finds an `id:` or the
   body ends. That surfaced **149 real sites**, so the kind moved to the advisory set
   (`id-blank-line`, the auto-fixable half, stays blocking) — promoting it would have failed CI on
   files no package touched. The 149 are now the cleanup checklist in `.code-report`.
9. **Dup-window rule catches no C++ pair** — confirmed. The S7/EIP pair never reached the
   40-window threshold (identifiers differ), so WPI-T1 removing ~250 lines of clone did not move
   the census. All 20 remaining pairs are QML.
10. Left open as instructed.
11. **Re-seeded** all three censuses; the singleton total shrank and did not grow.

## Patches for the coordinator

None — everything was inside my task file lists. Two things need your decision:

- **Translation catalogs.** `app/translations/ts/*.ts` still carry `<location>` entries for the
  four deleted provider files, and five `tr()` strings changed: the per-vendor "No DeepSeek API
  key set..." strings became one `"No %1 API key set..."`, and the two MDF4 Pro-gate strings were
  reworded for trial parity. Derived artifacts are yours; they are stale until `lupdate` runs.
- **`--benchmark-hotpath`.** `FrameBuilder.cpp` changed (dead-code removal, the `AsyncSinks`
  wiring, the `BlockPublisher`/`ReplayIngest` extraction and the `parseProjectFrame` collapse).
  Nothing on the per-frame path changed shape, but the gate is maintainer-run.

## Behaviour deltas, deliberate and named

Every other change is a pure move. These are the exceptions:

- **S7's poll tick gained an abort check.** `PolledPlcWorkerBase::onPollTimer` guards on
  `!open || aborted()`; EtherNet/IP already did both, S7 only checked `open`. The added check can
  only make a teardown unwind earlier.
- **`ExportStructure::sanitizeTitle` takes a fallback.** The three copies disagreed: CSV and
  Sessions fall back to `"Untitled"`, MDF4 to `"SerialStudio"`. It is a parameter, so no lane's
  folder name changes.
- **`ReplayPlaybackEngine::formatTimestamp` clamps at zero.** MDF4's copy clamped, CSV's and
  Sessions' did not. For the non-negative offsets all three actually produce, output is identical.
- **MDF4's `setProgress` rounding is NOT unified.** It uses `qBound(0, progress * count, count-1)`
  where CSV and Sessions use `qMin(count-1, qCeil(count * progress))`. No finding covers it and
  unifying would move the seek cursor by a row, so each player keeps its own clamp.
- **Four QML globals dropped**: `Cpp_BuildDate`, `Cpp_BuildTime`, `Cpp_AppOrganizationDomain`,
  `Cpp_Audio_Export`. All four were grep-verified absent from `app/qml`, `app/src` and `app/rcc`.
  `Widgets::AudioExport::setupExternalConnections()` still runs; only the context property is gone.
- **The four code-editor context menus are now one.** `PainterCodeDialog`'s menu had a different
  item ORDER (cut/copy/paste before undo/redo) and `OutputWidgetView`'s had no enable guards. Both
  now use the union menu, which is what G5 asked for; it is a visible normalisation.

## Invariants found that the plan did not state

- **`m_maskSinks` is bound by reference into `BlockStager`.** `BlockPublisher` therefore takes it
  as `const bool&` and the bool stays a FrameBuilder member; moving the storage would silently
  break the stager's mask.
- **`ReplayIngest`'s cell writers are non-virtual on purpose.** They run per dataset per replayed
  row; a host-interface with virtual dispatch (the `BlockStagerHost` pattern) would put a vtable
  hop in that loop, so they take the three facade members by reference instead.
- **`Plot3DNodes` cannot own `m_channelIsolation`.** `Plot3D::eyeColor` reads it too, so the node
  class takes a single resolved `stereo` flag via `setStereo()` and the facade keeps the input.
- **`PlotBase` is composed, not a QQuickItem base.** A base class holding `interpolationModeChanged`
  would move the NOTIFY signal out of each widget's own metaobject; composition keeps all three
  Q_PROPERTY surfaces byte-identical to what QML sees today.
- **`registry.add(name, ptr)` is unambiguous.** `QVariant` has an implicit `QVariant(bool)` and a
  pointer converts to `bool`, but that is a user-defined conversion sequence and loses to the
  `QObject*` overload's standard pointer conversion.
- **The three ExtensionManager pages read exactly three root flags and write none**, which is why
  they became plain `required property bool` inputs rather than taking a dialog handle.

## Docs my moves invalidated (for WP-J)

- **`doc/claude/architecture/dataflow.md`** — three concrete breaks: the flow diagram names
  `FrameBuilder::publishBlock` (L36) and "the SAME publishBlock tail" (L45); publishing is now
  `BlockPublisher::publish`, reached through `FrameBuilder`'s `m_publisher`. The cached-flag list
  (L205) names `refreshAnyAsyncSink`, now `BlockPublisher::refreshSinkFlag`. The diagnostics
  paragraph (L248) says `ConnectionManager::linkStats()` sums the per-device counters; it now
  forwards to `DeviceTableQuery::linkStats()`, and `IO::LinkStats` moved from
  `ConnectionManager.h` to `ConnectionManager/DeviceTableQuery.h`. Also: `kFramePoolBudgetBytes` /
  `refreshFramePoolBudget` / `FramePoolPolicy::applyMemoryBudget` are gone, and
  `FrameBuilder::frameChanged` no longer exists (A6/A14). A6's own text is now half-true — the
  budget went, the slots stayed as the per-source flat-table cache.
- **`doc/claude/architecture/dashboard.md:530`** — "`hostContextNames()` is a hand-kept mirror of
  ModuleManager that `registry-verify.py` lints" is wrong: it returns
  `Misc::ContextRegistry::objectNames()`, and the lint now compares that table against
  `registry.add(...)` in both directions and asserts the forwarding.
- **`doc/claude/architecture/scripting.md:282`** — "publishing *through* `publishBlock`" should
  name `BlockPublisher::publish`; the republish-lane rule itself is unchanged.
- **`doc/claude/architecture/io.md`** — S7 and EtherNet/IP now share `PolledPlcWorkerBase`; the
  per-driver worker description should name the base and say what stays per-driver (S7: the ISO
  handshake, chunk plan, `m_lastFault`/`m_itemErrors`; EIP: the libplctag seam and the dead-tick
  watchdog). The "Opening a Link" teardown paragraph no longer describes a wildcard disconnect for
  either driver. ConnectionManager's byte path is now `DeviceIoRouter`.
- **`doc/claude/architecture/export.md`** — the three exporters share `DataModel::ExportStructure`
  for template adoption and workspace paths; the Sessions ConsoleOnly raw-DB branch is gone (B13),
  and with it the worker's `operationMode` parameter.
- **`doc/claude/architecture/ai.md` (WP-J is writing it)** — the provider roster is Anthropic,
  OpenAI, Gemini, Local and `OpenAICompatibleProvider` x4 (DeepSeek, Groq, Mistral, OpenRouter as
  vendor tables). Do not name the four deleted classes. The turn's tool state lives in
  `AI::ToolTurnRunner`, not in `Conversation`.
- **`doc/claude/architecture/startup.md`** — I added one INV-4 sentence (K7); the pinned-order fix
  and the rest of K8 are still yours.
- **`doc/claude/scripts.md`** — three lint changes: the context-name lint now reads
  `ContextRegistry`; `id-placement` is live and advisory with 149 baseline sites; the
  wildcard-disconnect baseline is 15 sites in 12 files, not 19 in 16.
- **`tests/README.md`** — three new C++ suites (`tst_ethernetip_worker`,
  `tst_replay_playback_engine`, `tst_export_structure`), the `id-placement` fixture pair, and a
  note that `tst_replay_seek_engine` (WP-E, `UI::ReplaySeekEngine`) and
  `tst_replay_playback_engine` (WP-I, `DataModel::ReplayPlaybackEngine`) are different classes.
- **`doc/claude/specs/0038-widget-extensions/tasks.md`** mentions `hostContextNames()`; the spec
  archive is historical, so probably leave it, but the T17 acceptance criterion now reads oddly.

## Counterfactual self-check

**Which rule does this diff most risk violating?** "Every refactor is behaviour-preserving," on the
QML inheritance changes in T9/T10 — nine widget and dialog files changed root type or lost a
declaration block, and none of it can be rendered here.

**What evidence says it does not?** Four things. First, every moved block was proved identical
before moving, not assumed: the four instrument chrome blocks hash to one value
(`e1d68cb7…`), `evenTickValues` and `niceTickValues` are byte-identical between Meter and Gauge,
and the six driver dialogs' chrome differs only in the `title:` line, which stays in each derived
file. Second, the inheritance direction preserves declaration order, which is what QML stacking
depends on: `InstrumentBase` and `DriverTagPickerDialog` declare their children first, exactly
where each widget's own copy sat, so content still stacks above the chrome. Third, the extracted
pages and sections were checked for unresolved ids before the cut — the three ExtensionManager
pages reference no outer id but `root`, the three AssistantPanel sections reference only `root`,
`memoryManager` and `composer`, and each of those became an explicit property or signal. Fourth,
the dup census fell 2444 → 1639 across 33 → 20 pairs, which is the mechanical confirmation that
the blocks really were clones and really are gone.

The second-riskiest is the `FrameBuilder` publish extraction, because a mistake there is a silent
recording failure rather than a crash. Evidence: `BlockPublisher::publish` is the original body
with `m_sinks.x->` substituted for the `static auto& x` caches, the mask branch and the
`!m_anyAsyncSink` early-out kept in the same order; the one `clone_block_trimmed` per fan-out is
still one, and the masked branch still allocates only when a read-only observer is attached;
`m_maskSinks` is bound by reference so `BlockStager` and `BlockPublisher` observe the same bool;
and `structureGeneration` stamping was never in `publishBlock` (it is the stager's and
`trySpanLane`'s), so no stamp site moved. `tests/scripts/test_cpp_regressions.py` now pins the
dashboard hop in its new home.

I also caught and fixed one defect of my own during T6: my first cut of `Conversation::cancel()`
and `onReplyError()` called `m_tools.pendingInFamily(QString())`, which returns empty because the
family lookup early-returns on an empty string — it would have silently stopped denying pending
tool cards on cancel. `ToolTurnRunner::pendingIds()` replaced it at both sites.
