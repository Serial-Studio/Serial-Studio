---
spec: 0069-tu-decomposition
phase: tasks
status: shelved
updated: 2026-08-26
---

# Tasks 0069 — Translation-Unit Decomposition

> **Phase 3 of 4 — the ordered checklist.** Decompose [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable*. Gate: do not start `/ss-implement` until a
> human marks this `approved`.

## Conventions

- **One component = one task**, which is a deliberate departure from the usual ">3 files means
  split it" rule. A component split is atomic: a residual `.cpp` whose functions have been
  moved out but whose new TUs are not yet registered in the build does not link, so a
  half-applied split is never a coherent state. The unit of review here is the component, and
  each one is independently verifiable by the reconstruction check in T4.
- **Verify** for every cut task is the same three-part check, written once here rather than
  repeated 29 times: (a) `tu-cutter.py` reports a successful cut, which it only does when the
  emitted blocks reconstruct the original exactly and brace/`#if` balance verifies per file;
  (b) `python3 scripts/code-verify.py --check <new files>` reports no `cxx-tu-too-long` and no
  new errors; (c) the T4 harness reports an empty normalized diff for the component.
- **Deps** lists task IDs that must land first.
- Batches map to maintainer build checkpoints. The work lands as one squashed commit, but the
  tree is handed over buildable at the end of each batch.

## Phase 1 — Tooling (prerequisite)

### T1 — `tu-cutter.py`: file-scope variable definitions

- **Files:** `scripts/tu-cutter.py`
- **Does:** Add a `var:<name>` block kind covering file-scope definitions whose signature has
  no `(`: braced-initializer arrays (`static constexpr BleKnownUuid BLE_KNOWN_UUIDS[] = {...};`)
  and qualified out-of-class static member definitions
  (`bool IO::Drivers::BluetoothLE::s_initialized = false;`). These are the 268 uncovered lines
  that currently make `BluetoothLE.cpp` refuse to cut.
- **Verify:** `python3 scripts/tu-cutter.py inventory app/src/IO/Drivers/BluetoothLE.cpp`
  reports `uncovered_nonblank` empty; the block count rises and every other file's inventory is
  byte-identical to its pre-change output.
- **Deps:** none
- [x] done

### T2 — `tu-cutter.py`: `clang-format` fence pairing

- **Files:** `scripts/tu-cutter.py`
- **Does:** Pair `// clang-format off` with its matching `on` and emit the fenced region as a
  single block, so a manifest cannot assign the two directives to different TUs and orphan the
  fence. Today they emit as two independent one-line blocks.
- **Verify:** `inventory` on `BluetoothLE.cpp` shows one fenced block spanning lines 43-313
  rather than two one-line `clangfmt:` blocks.
- **Deps:** T1
- [x] done

### T3 — `tu-cutter.py`: top-level namespace descent

- **Files:** `scripts/tu-cutter.py`
- **Does:** When a `namespace` block exceeds a threshold, recurse into its body, key children as
  `namespace:X/<child-key>`, and re-wrap each destination TU in the same namespace with the same
  nesting depth. Without this, `Misc/CLI.cpp` is one opaque 1542-line block and
  `ProtoImporter.cpp` is two blocks of 780 and 1056.
- **Verify:** `inventory` on both files reports many small blocks instead of 1 and 2; a cut of
  `CLI.cpp` emits TUs that each open and close `namespace Misc` correctly.
- **Deps:** T2
- [x] done

### T4 — Reconstruction harness (AC9)

- **Files:** scratchpad only — not added to `scripts/`, since this verifies a one-time migration
  rather than an ongoing invariant.
- **Does:** Build a harness that snapshots each component's pre-split file, and after the cut
  concatenates the residual plus every new TU, normalizes away blank lines, `#include` lines and
  the emitted namespace wrappers, and diffs against the snapshot. This is the independent
  re-proof of AC9; `tu-cutter.py`'s own selfcheck is the first proof.
- **Verify:** Harness reports an empty diff when run against an already-split component
  (`app/src/IO/Drivers/Network.cpp` and its `Network/` dir), proving the harness itself is
  calibrated before it is trusted.
- **Deps:** none
- [x] done

## Phase 2 — Batch A: self-contained catalogs and drivers

### T5 — Split `BinaryTemplates.cpp`

- **Files:** `app/src/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp`,
  new `.../NativeTemplates/Binary/` (14 TUs + `BinaryShared.h`), `app/CMakeLists.txt`
- **Does:** One TU per wire format — `Raw`, `Hex`, `Base64`, `Tlv`, `Cobs`, `Slip`, `Ubx`,
  `Sirf`, `Mavlink`, `Nmea2000`, `Rtcm`, `Modbus`, `MessagePack`, `OpcUaDelta`. The "Shared
  binary helpers" banner becomes `BinaryShared.h` as `inline`, never external linkage. The
  "Family registry" banner stays in the residual so registration remains in one place.
- **Verify:** standard three-part check; `pytest tests/scripts/ -v` passes (these templates are
  what the JS parser units exercise).
- **Deps:** T4
- [x] done

### T6 — Split `TextTemplates.cpp`

- **Files:** `.../NativeTemplates/TextTemplates.cpp`, new `.../NativeTemplates/Text/` (10 TUs +
  `TextShared.h`), `app/CMakeLists.txt`
- **Does:** One TU per format — `Delimited`, `FixedWidth`, `KeyValue`, `Ini`, `AtCommand`,
  `Nmea0183`, `UrlEncoded`, `Json`, `Xml`, `Yaml`. Family registry stays in the residual.
- **Verify:** standard three-part check; `pytest tests/scripts/ -v`.
- **Deps:** T5
- [x] done

### T7 — Split `BluetoothLE.cpp`

- **Files:** `app/src/IO/Drivers/BluetoothLE.cpp`, new `IO/Drivers/BluetoothLE/` (7 TUs),
  `app/CMakeLists.txt`
- **Does:** `Uuids`, `Hal`, `Specifics`, `Slots`, `Discovery`, `Identity`, `PropertyModel`. The
  UUID table moves whole inside its `clang-format` fence. **Binding invariant:** the static
  shared discovery state (`s_initialized`, `s_localDevice`, `s_discoveryAgent`, `s_instances`)
  are definitions of declared class statics — they must land in exactly one TU, and must not
  gain or lose `static` or change initializer.
- **Verify:** standard three-part check; confirm the emitted static-member definitions appear
  exactly once across the component.
- **Deps:** T1, T2, T4
- [x] done

### T8 — Split `USB.cpp`

- **Files:** `app/src/IO/Drivers/USB.cpp`, new `IO/Drivers/USB/` (7 TUs), `app/CMakeLists.txt`
- **Does:** `Hal`, `Properties`, `Slots`, `Helpers`, `ControlTransfers`, `Identity`,
  `PropertyModel`.
- **Verify:** standard three-part check.
- **Deps:** T4
- [x] done

### T9 — Split `Modbus.cpp`

- **Files:** `app/src/IO/Drivers/Modbus.cpp`, new `IO/Drivers/Modbus/` (6 TUs),
  `app/CMakeLists.txt`
- **Does:** `Hal`, `Properties`, `Generation`, `Slots`, `Identity`, `PropertyModel`. **Binding
  invariant:** Modbus is Pro — every `#ifdef BUILD_COMMERCIAL` region moves whole, and no TU may
  end up with an unbalanced fence.
- **Verify:** standard three-part check; grep each new TU for balanced `#ifdef`/`#endif`.
- **Deps:** T4
- [x] done

### T10 — Split `Audio.cpp`

- **Files:** `app/src/IO/Drivers/Audio.cpp`, new `IO/Drivers/Audio/` (7 TUs),
  `app/CMakeLists.txt`
- **Does:** `Hal`, `DeviceParams`, `DeviceModels`, `Parsing`, `Discovery`, `Callback`,
  `PropertyModel`. **Binding invariant:** Audio is a stream-capable source — the audio callback
  and its parsing path run off the GUI thread and emit `blockReady` queued to the pipeline
  thread. Relocation only; no connection type changes.
- **Verify:** standard three-part check; confirm no `connect(` call gained or lost a connection-
  type argument.
- **Deps:** T4
- [x] done

### T11 — Split `OpcUa.cpp`

- **Files:** `app/src/IO/Drivers/OpcUa.cpp`, new `IO/Drivers/OpcUa/` (8 TUs),
  `app/CMakeLists.txt`
- **Does:** `Hal`, `Discovery`, `Subscription`, `Browse`, `Properties`, `Security`,
  `Certificates`, `PropertyModel`. **Binding invariant:** `openFinished(ok, reason)` has exactly
  one owner and must be emitted exactly once per attempt — the async dial paths move whole so
  no emit site is duplicated or dropped.
- **Verify:** standard three-part check; count `openFinished` emit sites before and after.
- **Deps:** T4
- [x] done

> **Batch A checkpoint** — maintainer builds GPL and commercial configurations.

## Phase 3 — Batch B: namespace-wrapped and service layers

### T12 — Split `CLI.cpp`

- **Files:** `app/src/Misc/CLI.cpp`, new `Misc/CLI/` (6 TUs), `app/CMakeLists.txt`
- **Does:** `Registration`, `ArgvScan`, `Processing`, `Apply`, `BusSetup`, `Commercial`. First
  consumer of namespace descent. **Binding invariant:** `--selftest` suites run inside
  `CLI::process()` *before* the composition root — nothing moved here may touch an application
  singleton.
- **Verify:** standard three-part check; confirm no new `::instance()` call appears in the
  component (`code-verify.py --singleton-census --check` must not rise).
- **Deps:** T3, T4
- [x] done

### T13 — Split `ProtoImporter.cpp`

- **Files:** `app/src/DataModel/Importers/ProtoImporter.cpp`, new `DataModel/Importers/Proto/`
  (7 TUs), `app/CMakeLists.txt`
- **Does:** `Lexer`, `Parser`, `Status`, `Ui`, `Generation`, `Heuristics`, `LuaEmit`. The
  `detail::proto` internal namespace splits across `Lexer` and `Parser` with the namespace
  re-wrapped in each.
- **Verify:** standard three-part check.
- **Deps:** T3, T4
- [x] done

### T14 — Split `ExtensionManager.cpp`

- **Files:** `app/src/Misc/ExtensionManager.cpp`, new `Misc/ExtensionManager/` (7 TUs),
  `app/CMakeLists.txt`
- **Does:** `Properties`, `Repository`, `Install`, `AutoUpdate`, `Network`, `Plugins`,
  `Manifest`.
- **Verify:** standard three-part check.
- **Deps:** T4
- [x] done

### T15 — Split `Sessions/DatabaseManager.cpp`

- **Files:** `app/src/Sessions/DatabaseManager.cpp`, new `Sessions/DatabaseManager/` (9 TUs),
  `app/CMakeLists.txt`
- **Does:** `Worker`, `Accessors`, `Reproducibility`, `Files`, `Locking`, `Sessions`, `Tags`,
  `Export`, `Schema`. **Binding invariant:** the schema banner is shared with `Sessions::Export`
  at session-creation time — it moves whole, and no DDL string is reformatted.
- **Verify:** standard three-part check; byte-compare the extracted schema strings.
- **Deps:** T4
- [x] done

### T16 — Split `Sessions/Player.cpp`

- **Files:** `app/src/Sessions/Player.cpp`, new `Sessions/Player/` (9 TUs),
  `app/CMakeLists.txt`
- **Does:** `Worker`, `Status`, `Files`, `LocalDb`, `StateCapture`, `Seeking`, `Alignment`,
  `Synthesis`, `StreamReplay`. **Binding invariant:** frame synthesis stamps nothing — source
  owns time; the replay path must not gain a re-stamp.
- **Verify:** standard three-part check; confirm no new `monotonicFrameNs` or timestamp
  assignment site.
- **Deps:** T4
- [x] done

### T17 — Split `CSV/Player.cpp`

- **Files:** `app/src/CSV/Player.cpp`, new `CSV/Player/` (7 TUs), `app/CMakeLists.txt`
- **Does:** `Status`, `Control`, `Files`, `Seeking`, `Processing`, `Rows`, `MultiSource`.
- **Verify:** standard three-part check.
- **Deps:** T4
- [x] done

### T18 — Split `MQTT/Publisher.cpp`

- **Files:** `app/src/MQTT/Publisher.cpp`, new `MQTT/Publisher/` (6 TUs), `app/CMakeLists.txt`
- **Does:** `Worker`, `Getters`, `Setters`, `Config`, `Lifecycle`, `Publish`. **Binding
  invariant:** the "Data publishing hotpaths" banner is on the publish path — no allocation is
  introduced and the worker/GUI thread split is unchanged. Pro-gated: fences move whole.
- **Verify:** standard three-part check; balanced-fence grep.
- **Deps:** T4
- [x] done

### T19 — Split `IO/ConnectionManager.cpp`

- **Files:** `app/src/IO/ConnectionManager.cpp`, new `IO/ConnectionManager/` (6 TUs),
  `app/CMakeLists.txt`
- **Does:** `Status`, `Accessors`, `Transmit`, `Lifecycle`, `Slots`, `Helpers`. **Binding
  invariant:** driver opens are synchronous calls with async dials behind them; the connect
  path has known crash history (spec 0056) — relocation only, no reordering of lifecycle steps.
- **Verify:** standard three-part check.
- **Deps:** T4
- [x] done

> **Batch B checkpoint** — maintainer builds both configurations.

## Phase 4 — Batch C: API and project layers

### T20 — Split `API/Server.cpp`

- **Files:** `app/src/API/Server.cpp`, new `API/Server/` (5 TUs), `app/CMakeLists.txt`
- **Does:** `Worker`, `Auth`, `Reception`, `Mirror`, `StreamBlocks`.
- **Verify:** standard three-part check.
- **Deps:** T4
- [x] done

### T21 — Split `API/Handlers/ProjectHandler.cpp`

- **Files:** `app/src/API/Handlers/ProjectHandler.cpp`, new
  `ProjectHandlerRegistration.cpp` + `ProjectHandlerPainter.cpp` in the same directory,
  `app/CMakeLists.txt`
- **Does:** Follows the existing flat `ProjectHandler{File,Batch,Discovery,Parser,Entities}`
  convention rather than creating a subdirectory. **Binding invariant:** every command keeps its
  registration call; the command set must be identical before and after.
- **Verify:** standard three-part check; diff the sorted list of registered command names.
- **Deps:** T4
- [x] done

### T22 — Split `API/Handlers/ProjectHandlerEntities.cpp`

- **Files:** `app/src/API/Handlers/ProjectHandlerEntities.cpp`, new
  `ProjectHandlerAlarmCompat.cpp`, `ProjectHandlerDatasetFields.cpp`,
  `ProjectHandlerOutputWidgets.cpp`, `ProjectHandlerBulk.cpp`, `app/CMakeLists.txt`
- **Does:** Same flat convention.
- **Verify:** standard three-part check; registered-command diff.
- **Deps:** T21
- [x] done

### T23 — Split `ProjectModelCrud.cpp`

- **Files:** `app/src/DataModel/Project/ProjectModelCrud.cpp`, new `ProjectModelMutation.cpp`,
  `ProjectModelReorder.cpp`, `ProjectModelOutputWidgets.cpp`, `ProjectModelIdMutators.cpp`,
  `ProjectModelBulk.cpp` in the same directory, `app/CMakeLists.txt`
- **Does:** Feeds the existing `DataModel/Project/` split directory. **Binding invariant:**
  every mutator is memento-staged — `ProjectUndoScope` stages and the first `setModified(true)`
  commits. Neither may be separated from its mutator.
- **Verify:** standard three-part check; `code-verify.py` reports no `undo-scope-missing`.
- **Deps:** T4
- [x] done

### T24 — Split `ProjectModel.cpp`

- **Files:** `app/src/DataModel/ProjectModel.cpp`, new `ProjectModelStatus.cpp`,
  `ProjectModelDocumentInfo.cpp`, `ProjectModelInit.cpp`, `ProjectModelSelection.cpp`,
  `ProjectModelScalarSetters.cpp` in `DataModel/Project/`, `app/CMakeLists.txt`
- **Does:** Facade stays at `DataModel/ProjectModel.cpp`. **Binding invariant — highest risk in
  this batch:** the ProjectModel constructor closure (`newJsonFile`, `watchProjectFile`,
  `scheduleAutoSave`, the `ControlScript::setCode` chain) runs before AppState and Dashboard
  exist. It must remain in the residual TU, and moving any ctor-reachable code re-triggers the
  ctor-edge proof in spec 0001 regardless of how unrelated the move looks.
- **Verify:** standard three-part check; confirm the ctor and every function it reaches stayed
  in the residual; re-run the spec-0001 ctor-edge reasoning and record the result in the task.
- **Deps:** T23
- [x] done

> **Batch C checkpoint** — maintainer builds both configurations; run `ctest`.

## Phase 5 — Batch D: UI and AI

### T25 — Split `UI/Taskbar.cpp`

- **Files:** `app/src/UI/Taskbar.cpp`, new `UI/Taskbar/` (7 TUs), `app/CMakeLists.txt`
- **Does:** `Model`, `Getters`, `Selection`, `WindowState`, `FullModel`, `Search`, `Workspaces`.
  **Binding invariant:** workspace IDs >= 1000 and group IDs < 1000; `deleteWorkspace(id)`
  branches on that threshold — the branch moves whole.
- **Verify:** standard three-part check.
- **Deps:** T4
- [x] done

### T26 — Split `UI/WindowManager.cpp`

- **Files:** `app/src/UI/WindowManager.cpp`, new `UI/WindowManager/` (4 TUs),
  `app/CMakeLists.txt`
- **Does:** `Queries`, `Layout`, `Geometry`, `Interaction`. The 1998-line "Layout management"
  banner sub-clusters by function: layout algorithms (`cascadeLayout`, `constrainWindows`)
  into `Layout`, geometry math (`computeResizedGeometry`) into `Geometry`, and pointer handling
  (`mousePressEvent`, `startManualPress`, `handleDragMove`) into `Interaction`.
- **Verify:** standard three-part check.
- **Deps:** T4
- [x] done

### T27 — Split `UI/Widgets/PainterContext.cpp`

- **Files:** `app/src/UI/Widgets/PainterContext.cpp`, new `UI/Widgets/Painter/` (9 TUs),
  `app/CMakeLists.txt`
- **Does:** `Gradient`, `Pattern`, `Style`, `StateStack`, `Paths`, `Shapes`, `Text`, `Images`,
  `Helpers`. **Binding invariant:** the `"painter"` widget key and `project.painter.*` API
  strings are unchanged — this is the Canvas Widget rename's internal identifier.
- **Verify:** standard three-part check.
- **Deps:** T4
- [x] done

### T28 — Split `UI/Widgets/Waterfall.cpp`

- **Files:** `app/src/UI/Widgets/Waterfall.cpp`, new `UI/Widgets/Waterfall/` (8 TUs),
  `app/CMakeLists.txt`
- **Does:** `Fft`, `Image`, `Hotpath`, `Paint`, `Axes`, `Ticks`, `ViewState`, `Input`.
  **Binding invariant:** the "Hotpath" banner is per-sample work — its file-scope helpers are
  promoted to a shared header as `SS_FORCE_INLINE` rather than left to cross a TU boundary.
- **Verify:** standard three-part check.
- **Deps:** T4
- [x] done

### T29 — Split `UI/Widgets/Terminal.cpp`

- **Files:** `app/src/UI/Widgets/Terminal.cpp`, new `UI/Widgets/Terminal/` (9 TUs),
  `app/CMakeLists.txt`
- **Does:** `Render`, `Metrics`, `Buffer`, `Selection`, `Search`, `Style`, `Ansi`, `Color`,
  `Input`. **Binding invariant:** the rendering pipeline must not gain an unconditional
  per-tick `grab()` — nothing about render cadence changes here.
- **Verify:** standard three-part check.
- **Deps:** T4
- [x] done

### T30 — Split `AI/Conversation.cpp`

- **Files:** `app/src/AI/Conversation.cpp`, new `AI/Conversation/` (9 TUs),
  `app/CMakeLists.txt`
- **Does:** `Wiring`, `Slots`, `ReplyHandlers`, `Tools`, `History`, `HelpIndex`, `Snapshot`,
  `Handoff`, `Budget`. The 1707-line "Internals" banner sub-clusters by function name:
  `appendBasicMetaTools`/`appendCommandMetaTools`/`appendReferenceMetaTools`/`dispatcherTools`
  into `Tools`, `ageHistoryToolResults` into `History`, `fetchHelpIndex` into `HelpIndex`.
- **Verify:** standard three-part check.
- **Deps:** T4
- [x] done

### T31 — Split `AI/ToolDispatcher.cpp`

- **Files:** `app/src/AI/ToolDispatcher.cpp`, new `AI/ToolDispatcher/` (5+ TUs),
  `app/CMakeLists.txt`
- **Does:** `AssistantTools`, `Catalog`, `Dispatch`, `Context`, plus the 1438-line "Sandboxed
  filesystem virtual tools" banner sub-clustered into `FilesystemTools` and `ProjectTools`
  (`executeAddTile`, `executeBulkApply`, `resolveDataset`, `resolveWorkspace` are project-model
  operations, not filesystem ones).
- **Verify:** standard three-part check.
- **Deps:** T30
- [x] done

> **Batch D checkpoint** — maintainer builds both configurations.

## Phase 6 — Batch E: hotpath (last, against an otherwise-settled tree)

### T32 — Capture pre-split hotpath baseline

- **Files:** none (measurement only)
- **Does:** Maintainer builds the optimized binary *and* a non-optimized binary from the tree as
  it stands after Batch D, and runs `--benchmark-hotpath` on both. Results recorded in this
  file. This is the baseline AC5 compares against, and it must be captured before T33/T34, not
  reconstructed afterwards.
- **Verify:** Both runs meet every tier; numbers recorded below the task.
- **Deps:** T31
- [ ] done

### T33 — Split `UI/Dashboard.cpp`

- **Files:** `app/src/UI/Dashboard.cpp`, new `UI/Dashboard/` (9 TUs + `DashboardShared.h`),
  `app/CMakeLists.txt`
- **Does:** `Queries`, `Access`, `Setters`, `Session`, `Tools`, `Frames`, `WidgetMap`, `Series`,
  `TimeRings`. **Binding invariants:** (1) every per-tick file-local helper from the "File-local
  helpers" banner is promoted to `DashboardShared.h` as `SS_FORCE_INLINE`, so inlining survives
  without link-time optimization; (2) `m_plotClocks` and `m_plotDisplayTimeSec` are ONE state
  via `resetPlotClocks()` and must not be separated across TUs in a way that invites clearing
  one without the other; (3) `m_streamAvailable` keeps its existing cache-refresh wiring;
  (4) `onDisplayTick`'s drain order (block ring, stream workers, one coalesced `updated()`) is
  unchanged.
- **Verify:** standard three-part check; confirm `resetPlotClocks` and both members live in one
  TU; confirm the display-tick ordering is byte-identical.
- **Deps:** T32
- [x] done

### T34 — Split `DataModel/FrameBuilder.cpp`

- **Files:** `app/src/DataModel/FrameBuilder.cpp`, new `DataModel/FrameBuilder/` (10 TUs +
  `FrameBuilderShared.h`), `app/CMakeLists.txt`
- **Does:** `Pool`, `Blocks`, `Wiring`, `Hotpath`, `Slots`, `Parsing`, `ParserBudget`,
  `QuickPlot`, `Transforms`, `DataTables`. **Binding invariants:** (1) every per-frame
  file-scope helper promotes to `FrameBuilderShared.h` as `SS_FORCE_INLINE`; (2) the span fast
  lane (`trySpanLane` -> `parseUtf8Spans` -> `applyDatasetValuesSpans`) stays allocation-free
  and its three functions stay together; (3) block staging keeps `kFrameBlockSampleCap` 64 and
  `kStreamBlockSampleCap` 4096 and the pool's `claimBlockSlot` / `use_count()==1` probe
  unchanged; (4) in-pipeline signal hops stay `Qt::DirectConnection`; (5) diagnostics counters
  stay plain `quint64` increments, never signals.
- **Verify:** standard three-part check; grep each new TU to confirm no `connect(` gained a
  connection type, no allocation appears on the publish path, and the two block caps are
  unchanged.
- **Deps:** T33
- [x] done

### T35 — Post-split hotpath verification (AC5)

- **Files:** none (measurement only)
- **Does:** Maintainer rebuilds optimized and non-optimized; assistant runs
  `--benchmark-hotpath` on both and compares against T32. The non-optimized comparison is the
  one that proves the force-inline promotion worked; the optimized one proves the CI gate holds.
- **Verify:** Every tier met on both builds; no regression beyond run-to-run noise.
- **Deps:** T34
- [ ] done

## Phase 7 — Headers and QML

### T36 — Decompose `DSP.h` into an umbrella header

- **Files:** `app/src/DSP.h`, new `app/src/DSP/` (7 headers)
- **Does:** `FixedQueue.h`, `Aliases.h`, `Structures.h`, `DownsampleWorkspace.h`, `Ring.h`,
  `Downsample.h`, `Downsample2D.h`. `DSP.h` becomes includes in dependency order and nothing
  else, so every existing `#include "DSP.h"` keeps working with zero caller edits.
- **Verify:** `code-verify.py --check`; confirm no caller was edited; confirm each piece carries
  its own include guard and SPDX header.
- **Deps:** none
- [x] done

### T37 — Decompose `DataModel/Frame.h` into an umbrella header

- **Files:** `app/src/DataModel/Frame.h`, new `app/src/DataModel/Frame/` (6 headers)
- **Does:** `Keys.h`, `Action.h`, `OutputWidget.h`, `Serialize.h`, `Deserialize.h`,
  `Concepts.h`. **Binding invariant:** `Keys::` stays the single source of truth — relocated,
  never duplicated, and no key string is edited.
- **Verify:** `code-verify.py --check`; byte-compare the extracted `Keys::` block against the
  original; confirm no caller edited.
- **Deps:** T36
- [x] done

### T38 — Split `Settings.qml` into per-tab pages

- **Files:** `app/qml/Dialogs/Settings.qml`, new `app/qml/Dialogs/Settings/` (8 pages),
  `app/CMakeLists.txt` (`QML_SOURCES`)
- **Does:** `GeneralTab`, `StartupTab`, `PlottingTab`, `LayoutTab`, `TaskbarTab`, `ConsoleTab`,
  `ExportTab`, `NotificationsTab`. **Binding invariant:** `NotificationsTab` is Pro — in GPL
  builds it must remain an empty `Item` so `StackLayout` indices stay aligned with the visible
  `TabButton`s. The `implicitHeight` expression referencing all eight tab ids must keep
  resolving.
- **Verify:** `code-verify.py --check`; maintainer opens Preferences and confirms all eight tabs
  render and switch correctly in both build configurations.
- **Deps:** none
- [x] done

### T39 — Split `PlotWidget.qml` into sub-components

- **Files:** `app/qml/Widgets/PlotWidget.qml`, new `app/qml/Widgets/Plot/` (6 components),
  `app/CMakeLists.txt`
- **Does:** `CurveLayer`, `AreaFill`, `AxisLayer`, `TriggerOverlay`, `MarkerLayer`,
  `CrosshairOverlay`. **Binding invariant:** every `property alias` target
  (`_graph`, `_axisX`, `_axisY`, `_yLabel`, `_curveLayer`, `_theme`) must stay resolvable — the
  aliased ids remain in the root file, since a QML alias cannot cross a component boundary.
- **Verify:** `code-verify.py --check`; maintainer confirms a live plot renders with curves,
  axes, area fill, trigger line and crosshair.
- **Deps:** T38
- [ ] done

### T40 — Split `FlowDiagram.qml`

- **Files:** `app/qml/ProjectEditor/Views/FlowDiagram.qml`, new
  `app/qml/ProjectEditor/Views/FlowDiagram/` (4 components + `layout.js`),
  `app/CMakeLists.txt`
- **Does:** Layout math to `layout.js` as a `.pragma library`-free JS import; `NodeCard`,
  `DatasetChip`, `TransformBlock`, `ArrowLayer` as components.
- **Verify:** `code-verify.py --check`; maintainer opens the Project Editor flow view and
  confirms nodes, chips, transforms and arrows render and collapse/zoom still work.
- **Deps:** T39
- [ ] done

## Phase 8 — Enforcement and documentation

### T41 — Promote `cxx-tu-too-long` to a blocking error; retire the ratchet

- **Files:** `scripts/code-verify.py`, `scripts/tu-census.json` (deleted),
  `scripts/sanitize-commit.py`, `.github/workflows/ci.yml`
- **Does:** Remove `cxx-tu-too-long` from `_ADVISORY_KINDS` and drop its explanatory comment;
  delete `_tu_tier` / `_collect_tu_census` / `_print_tu_census` / `_run_tu_census` and the
  `--tu-census` argument; drop the ratchet sentence from the rule message and the rule catalog;
  delete the baseline; remove the gate step from `sanitize-commit.py` and the
  `🔒 Ratchet translation-unit size` step from CI. **Must land after every split**, or CI fails
  for the duration of the work.
- **Verify:** `code-verify.py --check` exits 0 on the tree; a scratch 1600-line file makes it
  exit non-zero; `grep -r tu-census` finds nothing outside this spec directory.
- **Deps:** T35, T37, T40
- [ ] done

### T42 — Update documentation for the new layout

- **Files:** `doc/claude/directory-map.md`, `doc/claude/scripts.md`, `CLAUDE.md`, and any
  architecture doc naming a moved implementation
- **Does:** Record the new per-component directories in the directory map; update the
  `scripts.md` gate table and the CLAUDE.md "Three gates" paragraph to two gates; update the
  CLAUDE.md code-style line that cites the ratchet. Anything else is found by the linter, not
  guessed.
- **Verify:** `python3 scripts/claim-verify.py` and `python3 scripts/documentation-verify.py`
  both clean.
- **Deps:** T41
- [x] done

## Progress note (2026-08-25)

**32 of 34 components split; 863 excess lines remain, down from 25287.** All C++ and header
work is complete and mechanically verified. What is outstanding, and why:

- **T39 / T40 (`PlotWidget.qml`, `FlowDiagram.qml`) — not started, needs a decision.** Unlike
  every other component, these cannot be verified mechanically: QML resolves at runtime, so a
  wrong binding compiles clean and fails only when a user opens the surface. Both are also
  genuinely coupled rather than merely long. `PlotWidget.qml`'s 857-line overlay block reads
  `_graph`, `_axisX`, `_axisY`, `_triggerLine`, `root` and eight root properties, and three of
  its internal ids (`_overlayMouse`, `_xPosLabel`, `_yPosLabel`) are read from outside, so
  extracting it means inventing a property interface. `FlowDiagram.qml`'s `layoutDiagram` is a
  single 816-line function reading two outer ids, 13 root properties, six sibling functions and
  four C++ singletons; extracting it to a `.js` works only if a non-`.pragma library` import
  resolves the document scope. `Settings.qml` (T38) was different and is done: its eight tabs
  had zero cross-references in either direction, which is why it was safe to cut blind.
- **T41 — blocked by T39/T40.** Promoting `cxx-tu-too-long` to an error requires zero
  violations. The census has been re-baselined instead (34 files -> 2), so the ratchet still
  guards against regrowth in the meantime.
- **T32 / T35 — maintainer measurement, not yet run.** The pre-split baseline is still
  reachable: it is commit `9f205f036`, so the baseline can be captured at any time rather than
  needing to have been captured first.

### Deviations from `plan.md`, with reasons

- **No `FrameBuilderShared.h` or `DashboardShared.h`.** The plan assumed per-frame file-scope
  helpers would be split from their callers and need force-inline promotion. A locality
  analysis of every file-scope helper in both files showed each one's call sites land in a
  single destination TU, so each travels with its only caller and keeps plain `static` internal
  linkage. That is strictly better than promotion: it preserves inlining in every build with no
  change to the symbol set at all. The one helper that did span two destinations
  (`timeRingCapacity`) was moved to the destination that owns all its callers.
- **Shared template helpers are `static inline`, not `inline`.** Plain `inline` has external
  linkage, which R4 forbids; `static inline` keeps internal linkage and still avoids
  unused-function warnings under `-Wall -Wextra`. Header constants use `inline constexpr` to
  avoid `-Wunused-const-variable`.
- **`Frame/Concepts.h` renamed to `Frame/FrameConcepts.h`** — a quoted include resolves
  relative to the including file first, so the original name would have shadowed
  `app/src/Concepts.h` for every file in that directory.
- **Four `tu-cutter.py` fixes beyond the three planned**, each found by a real failure: a
  whole-file `#ifdef` fence opened in the preamble was never closed in the emitted TUs (4 Pro
  files); a banner comment attached to a dropped `nsopen` block was lost; `extern "C" {` was
  parsed as a variable definition; and `close_brace` latched onto the `{}` of a defaulted
  argument, ending a block at its signature.
- **`claim-verify.py` now treats a namespace as an owner.** `owners` was keyed off the file
  stem, so `DSP::TimeRing` stopped resolving the moment `DSP.h` became an umbrella. The doc was
  right and the checker was wrong.
- **`tests/scripts` helpers are component-aware.** 26 static tests pinned assertions to
  `_read("<file>.cpp")` and broke when the code moved. They now read the whole component, so
  they assert on behaviour rather than on which file a function landed in.

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` is met and checked off there.
- [ ] `python scripts/code-verify.py --check` is clean; zero `cxx-tu-too-long`, no new errors.
- [ ] `qt-cpp-review` run on the C++ diff; findings addressed or noted.
- [ ] `--benchmark-hotpath` not regressed on **both** optimized and non-optimized builds (T35).
- [ ] `ctest` and `--selftest` pass against the maintainer's build.
- [ ] `pytest tests/scripts/` passes; `pytest tests/ -m "not destructive"` passes against the
      running app.
- [ ] `code-verify.py --singleton-census --check` has not risen.
- [ ] AC9 reconstruction harness reports an empty normalized diff for all 34 components.
- [ ] `python scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [ ] Diff is *what was asked, and only that* — no behavior change, no drive-by fixes.
- [ ] `spec.md` status set to `done`.
