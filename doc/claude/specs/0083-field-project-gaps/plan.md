---
spec: 0083-field-project-gaps
phase: plan
status: approved     # draft -> approved (gate before /ss-tasks)
updated: 2026-09-11
---

# Plan 0083 — Close the gaps a real industrial deployment works around with scripts

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Read the relevant `doc/claude/` sub-docs and the *actual code*
> before writing this — a plan grounded in a stale mental model is worse than no plan.
> Gate: do not start `/ss-tasks` until a human marks this `approved`.
>
> The spec is an umbrella over six gaps. This plan designs all six as one body of work in the
> order the findings recommend, so tasks can land incrementally: G1 library + parameters,
> G2 tile identity, G3 importer merge, G4 Modbus importer v2, G5 write unit, G6 profiles.
> Every item is configuration, project-model, importer or driver-command work; nothing touches
> the per-frame path.

## Approach (one paragraph)

Six additive, backward-compatible extensions. **G1**: one optional project-level Lua chunk is
evaluated into the global table of every Lua transform state (frame lane per source, stream
lane per worker, editor dry-run) right before the per-dataset entries compile, and an optional
per-dataset parameter map is pushed as a `params` table into each entry's environment (JS gets
the same object as an IIFE argument). **G2**: a workspace tile carries `datasetUniqueId` beside
`groupUniqueId`; the per-type ordinal `relativeIndex` becomes a derived cache re-bound from
identity at load, after every group mutation and before every save, so every existing ordinal
consumer keeps working and older readers keep loading. **G3**: the importers keep building a
standalone project JSON; a new `ProjectLoader::mergeImportedProject()` remaps its ids and
appends sources, groups, tables and workspaces into the open document as one undo step.
**G4**: `RegisterEntry` learns unit id, bit index, access and word order; blocks group by unit,
the driver polls each block from its unit, and the generated Lua matches a reply by
`(unit, function, byteCount)` before touching the cursor; writable rows emit an Output group of
controls. **G5**: an odd-length write payload `[unit, addr_hi, addr_lo, words...]` names its
unit, which the SDK helpers grow an optional argument for. **G6**: a project-level
`workspaceProfiles` list names folder subsets; the active profile filters what the taskbar and
folder tree see, chosen by dialog, CLI or API.

## Affected subsystems & files

| File | Change |
|------|--------|
| **G1 — library + parameters** | |
| `core/Core/DataModel/FrameKeys.h` | `Keys::TransformLibrary("transformLibrary")`, `Keys::TransformParams("transformParams")` |
| `core/Core/DataModel/Frame.h` | `Dataset::transformParams` (`QVariantMap`, after `transformCode`); `Frame::transformLibrary` (`QString`, beside `controlScriptCode`), serialize/read/`clear_frame` |
| `app/rcc/properties/dataset.json` | new `subEntities` entry `transformParams` (readHook `readTransformParams`, writeHook `writeTransformParams`, apiName `transformParams`); regenerate the four TUs |
| `core/Pipeline/DataModel/Project/PropertyHooks.{h,cpp}` | the two hooks: JSON object ↔ `QVariantMap`, number/string/bool only, other value types dropped with a warning |
| `scripts/generate-property-registry.py` | the `postRead` step whitelist (`readAlarmBands`, `readFrequencyMarkers`) and the write-hook emitter learn `readTransformParams` / `writeTransformParams` |
| `core/Pipeline/DataModel/Generated/*`, `core/Ui/ProjectEditor/Generated/DatasetForm.cpp`, `core/Api/API/Generated/DatasetApiFields.cpp`, `app/rcc/api/proto-fields.json`, `doc/grpc/serialstudio-typed.proto` | regenerated, never hand-edited |
| `core/Pipeline/DataModel/ProjectModel.h/.cpp` | `transformLibraryCode` Q_PROPERTY + `setTransformLibraryCode()` (undo scope, keystroke coalesce key like `setControlScriptCode`) |
| `core/Pipeline/DataModel/Project/ProjectLoader.cpp` | read `Keys::TransformLibrary` in `applyJsonDocumentCore` |
| `core/Pipeline/DataModel/Project/ProjectPersistence.cpp` (or wherever `serializeToJson` writes `controlScriptCode`) | write the key when non-empty |
| `core/Pipeline/DataModel/FrameBuilder/TransformCompiler.{h,cpp}` | `TransformEntry` gains `params`; `compileLua` evaluates the library chunk once after bootstrap; `compileLuaEntry` sets `params` in the env; `compileJs` passes params into the IIFE |
| `core/Core/IO/StreamConfig.h`, `core/Devices/IO/ConnectionManager/StreamConfigBuilder.cpp` | `StreamConfig::transformLibrary`, `StreamChannelConfig::transformParams`, filled where `luaFastMode` is copied today (`StreamConfigBuilder.cpp:108`) |
| `core/Pipeline/IO/StreamWorker.{h,cpp}` | `setupLuaState` evaluates the library; `compileLuaEntry`/`compileJsEntry` inject `params` |
| `core/Ui/ProjectEditor/Editors/DatasetTransformEditor.{h,cpp}` | `validateTransform`/test run the library chunk first; a "Parameters" key/value table in the dialog; a library-edit mode (Lua only, validation = chunk compiles) |
| `app/qml/ProjectEditor/Views/ProjectView.qml` | "Shared Lua Library…" button next to the Lua execution-mode toggle, opens the editor in library mode |
| `core/Api/API/Handlers/ControlScriptHandler.cpp` (sibling pattern) → new `core/Api/API/Handlers/TransformLibraryHandler.{h,cpp}` | `project.transformLibrary.get` / `.set` / `.dryRun` |
| `app/src/API/Handlers/*` (`CommandHandler::initializeHandlers()`) + `app/rcc/ai/command_safety.json` | register the handler; tier the three commands |
| `scripts/code_verify_rules.py` | `_PROJECT_KEY_LITERALS` gains the two literals |
| `doc/help/Dataset-Transforms.md`, `app/rcc/ai/skills/` (transform skill) | document `params` and the library |
| **G2 — tile identity** | |
| `core/Core/DataModel/FrameKeys.h` | `Keys::DatasetUniqueId("datasetUniqueId")` |
| `core/Core/DataModel/Frame.h` | `WidgetRef::datasetUniqueId = -1`; serialize when ≥ 0, read optional |
| `core/Pipeline/DataModel/Project/WorkspaceKeys.{h,cpp}` | `ResolvedWidget` gains `groupUniqueId`; new `rebindRef(ref, lookup)` that resolves by identity and returns the current ordinal |
| `core/Pipeline/DataModel/Project/ProjectWorkspaces.{h,cpp}` | `rebindWidgetRefs()` (identity → ordinal, legacy fill of `datasetUniqueId` from the ordinal); called from the loader, from `notifyWorkspaceListChanged`, on `groupsChanged`, and from `writeProjectFile()` beside `flushWorkspaceRegen()`; `addWidgetToWorkspace` gains `datasetUniqueId` |
| `core/Pipeline/DataModel/ProjectModel.h` | forward the new parameter |
| `core/Pipeline/DataModel/Project/ProjectLoader.cpp` | call `rebindWidgetRefs()` after `loadCustomWorkspaces` |
| `core/Pipeline/DataModel/Project/ProjectPersistence.cpp` | rebind before serialize |
| `core/Ui/UI/Taskbar/TaskbarWorkspaces.cpp` | `resolveRefWindowId`: ordinal first, identity scan of `widgetMap()` (via `getGroupWidget`/`getDatasetWidget` uniqueIds) when the ordinal's owner mismatches |
| `core/Ui/ProjectEditor/EditorSummaries.cpp`, `app/qml/ProjectEditor/Views/AddWidgetDialog.qml` | rows carry `datasetUniqueId`; existing-key strings include it |
| `core/Ui/ApiHandlers/WorkspacesHandler.cpp` | `project.workspace.addWidget` accepts `datasetId`; list/validate emit it; `relativeIndex` stays accepted and auto-derived |
| `core/Ui/Misc/Problems/ProjectCheckers.cpp` | `dangling-workspace-widget` also fires when `datasetUniqueId` is gone |
| `core/Pipeline/DataModel/Importers/ImporterCommon.h` | `buildImportedWorkspaces` fills `datasetUniqueId = -1` (group-scope only; no change in output) |
| `scripts/code_verify_rules.py` | the new literal |
| **G3 — importer merge** | |
| `core/Pipeline/DataModel/Project/ProjectLoader.{h,cpp}` | `mergeImportedProject(const QJsonObject&, const QString& folderTitle)`: remap `sourceId`, group/dataset `uniqueId`, table names, workspace ids and ref ids; append; file groups/tables/workspaces into one new folder each; one `ProjectUndoScope`; `setModified(true)`; emits the load signals; `importCompleted(true, path)` |
| `core/Pipeline/DataModel/ProjectModel.h` | `mergeImportedProject` forwarder |
| `core/Pipeline/DataModel/Importers/ImporterCommon.h` | `finalizeImportedProject` unchanged; new `importedIdRemap` helper (pure, unit-testable) |
| `core/Pipeline/DataModel/Importers/{ModbusMapImporter,DBCImporter,ProtoImporter}.{h,cpp}` | `confirmImport()` keeps "new project"; new slot `confirmMerge()` |
| `app/qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml`, `.../DBCPreviewDialog.qml`, `app/qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml` | second button "Add to Current Project", enabled only in ProjectFile mode with a loaded document |
| `core/Core/Bus/Messages.h`, `core/Devices/IO/Drivers/Modbus.cpp` | `ModbusRegisterGroupsLoaded` gains `append` (default false → current `clearRegisterGroups()` behaviour) |
| `app/tests/tst_project_merge.cpp`, `app/tests/CMakeLists.txt` | ctest over the remap helper |
| **G4 — Modbus importer v2** | |
| `core/Pipeline/DataModel/Importers/ModbusRegisterMap.{h,cpp}` | `RegisterEntry` gains `unitId` (0 = connection default), `bitIndex` (-1), `writable`, `wordOrder` (`abcd` default, `cdab`, `badc`, `dcba`); CSV columns `slave`/`unit_id`/`unitid`/`device`, `bit`, `rw`/`access`/`writable`, `order`/`word_order`/`byte_order`; XML attributes and JSON keys with the same names |
| `core/Pipeline/DataModel/Importers/ModbusMapImporter.{h,cpp}` | `RegisterBlock::slaveAddress`; `computeBlocks` groups by (unit, type, contiguity); `blockTitle` adds "Unit N" when the map has more than one unit; `loadRegisterGroups` + `buildProject` write `slave`; bit rows on register blocks → `type = "rbit"` LED datasets; `wordOrder` → entry `order`; writable rows → one `GroupType::Output` group "<map> Controls" with Slider/Toggle/TextField controls whose `transmitFunction` calls the SDK helper with the unit argument and whose state binds to the block table register (spec 0080 `OutputStateSource::Table`); Lua `parse()` matches `(unit, func, byteCount)` against `BLOCKS` before the cursor; `decode` gains `rbit` and word-order permutation |
| `doc/help/Drivers-Modbus.md`, `doc/help/Auto-Generating-Projects.md` | the four new columns, the reply-matching rule |
| `tests/scripts/test_modbus_lua_parser.py` | drives the generated Lua under `luajit` with a `tableSet` stub |
| **G5 — write unit** | |
| `core/Devices/IO/Drivers/Modbus.cpp` | `write()`: odd length ≥ 5 → byte 0 is the unit (1..247), remainder decoded as today |
| `app/rcc/api/prelude.js` (+ regenerate `SerialStudio.js` via `scripts/generate-sdk.py`) | optional trailing `unit` on `modbusWriteRegister`, `modbusWriteRegisters`, `modbusWriteCoil`, `modbusWriteFloat` |
| `app/rcc/scripts/output/modbus_write.js`, `core/Ui/AI/ContextBuilder.cpp`, `core/Api/API/Handlers/ProjectUpdateCommands.cpp`, `doc/help/Drivers-Modbus.md`, `doc/help/Output-Controls.md` | helper descriptions mention the unit |
| **G6 — workspace profiles** | |
| `core/Core/DataModel/FrameKeys.h` | `Keys::WorkspaceProfiles("workspaceProfiles")`, `Keys::ProfileId`, `Keys::FolderIds`, `Keys::WorkspaceIds` |
| `core/Core/DataModel/Frame.h` | `struct WorkspaceProfile { int profileId; QString title; std::vector<int> folderIds; std::vector<int> workspaceIds; }` + serialize/read |
| `core/Pipeline/DataModel/Project/ProjectWorkspaces.{h,cpp}` | owns `m_profiles`, `m_activeProfileId` (-1 = all), `visibleList()` / `visibleFolderIds()`; CRUD with undo scopes; `setActiveProfile()` (runtime-only, emits `activeWorkspacesChanged`) |
| `core/Pipeline/DataModel/ProjectModel.h/.cpp` | `activeWorkspaces()` returns the profile-filtered vector; `workspaceProfiles`, `activeProfileId` Q_PROPERTYs; `profileChoiceRequested()` signal |
| `core/Pipeline/DataModel/Project/ProjectLoader.cpp` | load/save profiles; after load, if ≥ 2 profiles and no pre-selected id, emit `profileChoiceRequested` |
| `core/Pipeline/DataModel/Project/ProjectFolders.cpp` | `deleteWorkspaceFolder` drops the id from every profile |
| `core/Ui/UI/Taskbar/TaskbarWorkspaces.cpp` | folder tree built from the visible folder ids |
| `app/qml/Dialogs/WorkspaceProfileDialog.qml` + `app/qml/ProjectEditor/Views/WorkspacesView.qml` | pick-at-load dialog; profile CRUD (name + checkable folder list) |
| `app/src/Misc/CLI.{h,cpp}` | `--profile <name>` applied before the project opens |
| `core/Ui/ApiHandlers/WorkspacesHandler.cpp` | `project.workspace.profile.list/add/update/remove/select` |
| `app/rcc/ai/command_safety.json`, `scripts/code_verify_rules.py` | tiers + key literals |
| `doc/help/Project-Editor.md`, `doc/help/Command-Line-Interface.md`, `doc/help/Operator-Deployments.md` | profiles |
| **Docs (all gaps)** | |
| `CLAUDE.md`, `doc/claude/architecture/scripting.md`, `doc/claude/architecture/project.md`, `doc/claude/architecture/dashboard.md` | one paragraph each: library/params compile-time contract, identity-then-ordinal ref rule, merge helper, profiles |

## Architecture & data flow

**G1.** `TransformCompiler::compile()` already buckets entries by `(sourceId, language)`
(`TransformCompiler.cpp:236-268`). `compileLua` gains one step between the deadline arm and the
entry loop: load the library with `luaL_loadbufferx(..., "=library", "t")` and `lua_pcall` it in
the default (global) environment, so its top-level functions land in `_G`; every entry's env
table already has `__index = _G` (`compileLuaEntry`), so `lib.rtd(v, params)` resolves through
the existing fallthrough with no per-call cost. A library that fails to compile or run is
reported once via `noteTransformError(-1, msg)` and a `qWarning`, and the entries still compile
(they merely see no library). `compileLuaEntry` creates the env table, then pushes the
parameter map as a `params` table (numbers as `lua_pushnumber`, strings as `lua_pushlstring`,
bools as booleans) before the chunk runs, so `local k = params.k` at load time works and so
does `params.k` inside `transform()`. `compileJs` wraps as
`(function(params){%1; return transform...})(<json>)` with the object serialised through
`QJsonDocument::Compact`, so JS gets the same names. The stream lane mirrors both in
`StreamProcessor::setupLuaState` / `compileLuaEntry` / `compileJsEntry` from `StreamConfig`
(`StreamConfigBuilder.cpp` fills it from the project, where `luaFastMode` is copied today). The editor's `validateTransform` and the test run evaluate the library chunk on the
dry-run state first, so a transform that calls a library function validates. The Expression
lane is untouched (`params` is not an expression identifier; stated as out of scope).

**G2.** Two facts already exist: `WorkspaceKeys::buildResolvedWidgetLookup` computes the
per-type ordinal of every eligible widget from the project (`WorkspaceKeys.cpp`), and the
`Dashboard` can answer "which group/dataset owns window N" through `getGroupWidget(type, idx)`
/ `getDatasetWidget(type, idx)`. The design makes identity the truth and the ordinal a cache:
`ProjectWorkspaces::rebindWidgetRefs()` walks every ref, finds the lookup entry whose
`(widgetType, groupUniqueId, datasetUniqueId)` matches, and overwrites `relativeIndex` with
that entry's ordinal. A legacy ref (`datasetUniqueId == -1` on a dataset-scope type) is first
resolved the old way, by `(widgetType, groupUniqueId, relativeIndex)`, and its
`datasetUniqueId` is filled in from the hit. Rebind runs after `loadCustomWorkspaces`, inside
`notifyWorkspaceListChanged`, on `groupsChanged`, and in `writeProjectFile()` next to
`flushWorkspaceRegen()`, so a file edited by a generator opens correctly and a saved file is
always self-consistent for older readers. `resolveRefWindowId` keeps its ordinal fast path and
adds the identity scan as the fallback when the owner check fails, which covers the window
between a group mutation and the queued `groupsChanged` rebind.

**G3.** `mergeImportedProject(imported, folderTitle)` runs on the GUI thread under one
`ProjectUndoScope`: compute `sourceOffset = max(sourceId)+1`, allocate group uniqueIds from
`nextUniqueId` (the helper returns the old→new map so workspace refs and `xAxisId` /
`waterfallYAxis` are remapped with it), suffix colliding table names with the folder title,
offset workspace ids past the current maximum user id and remap their refs, then append
sources, groups, tables and workspaces to the live vectors, file them into a fresh group
folder, table folder and workspace folder named `folderTitle`, call `setModified(true)` and
`emitProjectLoadedSignals(false)`-equivalent structural signals. For Modbus the importer
publishes `ModbusRegisterGroupsLoaded{append = true}` so the driver keeps the groups it has.
Merge is refused (message box) when no document is open or the mode is not ProjectFile; in a
GPL build it is refused outright because the result is multi-source.

**G4.** `computeBlocks` sorts by `(unitId, registerType, address)` and starts a new block on
any unit or type change; `RegisterBlock::slaveAddress` flows to `loadRegisterGroups` (bus
message) and `buildProject` (`registerGroups[].slave`, the key `ModbusRegisterGroups` already
persists). The generated Lua declares every block with `unit`, `func`, `bytes` and picks the
block whose triple equals `(frame[1], frame[2], frame[3])`; only when two blocks share a
triple does it fall back to the cursor among those candidates, so one dropped reply no longer
shifts every later word. Bit rows on register blocks emit `type = "rbit", offset = word,
bit = n`; `decode` reads the word and masks. Word order is a per-entry byte permutation
applied before the numeric read. Writable rows build an `Output` group whose controls call
`modbusWriteRegister(addr, value, unit)` (Slider / TextField) or `modbusWriteCoil(addr, on,
unit)` (Toggle for coils, `rbit` rows use a read-modify-write helper: read the word from the
table, set the bit, write it), with `stateSource = Table` bound to the block's table register
so the control shows the plant's value (spec 0080).

**G5.** `Modbus::write` today rejects odd lengths (`data.length() % 2 != 0`), so an odd payload
is free to mean "unit-prefixed": byte 0 is the unit (1..247, else refused), the remainder is
the existing `[addr_hi, addr_lo, words...]`. `sendWriteRequest(unit, ...)` takes it. Every
existing payload is even and unchanged. The SDK helpers append the byte only when the third
argument is given.

**G6.** `WorkspaceProfile` is a plain project-level list. `activeWorkspaces()` (the one call the
taskbar and API read) returns `m_visible`, rebuilt whenever the list, folders or active profile
change: a workspace is visible when the active profile is -1, or its folder (or any ancestor)
is in `folderIds`, or its id is in `workspaceIds`. The auto workspaces (ids < `UserStart`)
are never filtered. The loader emits `profileChoiceRequested` when a project declares two or
more profiles and nothing pre-selected one; `MainWindow` shows the dialog; the CLI `--profile`
and the API `select` pre-select by title or id. The editor never filters: it always shows the
whole project.

## Hotpath & threading impact

- **Touches the hotpath?** No. `TransformCompiler` and `StreamProcessor::compileEngines` are
  compile-time (project load / connection open); the library evaluation and the `params` push
  happen once per compile, and `applyTransformLua` / `applyTransformJs` are not edited. The
  per-frame call sees a function whose upvalues or environment already hold the parameters.
  `Dataset` grows by one `QVariantMap` (a pointer-sized COW member), which the span fast lane
  never touches; `WidgetRef` and `Frame` growth are project-model only. No FrameReader,
  CircularBuffer, BlockStager or Dashboard ingest change; `--benchmark-hotpath` is run once by
  the maintainer as a regression check because `Dataset` changed size.
- **New cross-thread signal/slot?** No. The stream worker receives the library and the params
  through its existing config struct at worker build time. `profileChoiceRequested` and the
  merge signals are GUI-thread only.
- **New input to a cached hotpath flag?** No. `m_changeDriven`, `m_captureLatestFrame`,
  `m_anyAsyncSink`, `m_streamAvailable` are untouched; the library does not change which
  datasets have transforms.
- **Timestamp ownership** — unchanged; nothing here stamps or re-stamps.
- **`lua_*` in a routed lambda** — none added; the library chunk runs on the state's own
  thread inside the existing bootstrap/compile sequence.

## Data model & persistence

- **Project JSON, all optional, schema stays 3.** Every reader of an older file sees the same
  document it sees today; every writer of a newer file only adds keys an older Serial Studio
  ignores. `relativeIndex` keeps being written for G2, so older readers keep resolving tiles.
  - `transformLibrary` (project, string, omitted when empty).
  - `datasets[].transformParams` (object, omitted when empty; values number | string | bool).
  - `workspaces[].widgetRefs[].datasetUniqueId` (int, omitted when -1).
  - `workspaceProfiles` (array of `{profileId, title, folderIds[], workspaceIds[]}`).
  - `sources[].connection.registerGroups[].slave` (int, omitted when 0) — the key the driver
    already persists in QSettings.
- **`Keys::`** every literal is declared once in `FrameKeys.h` and added to
  `_PROJECT_KEY_LITERALS` in `scripts/code_verify_rules.py`.
- **Dataset property registry:** `transformParams` is a `subEntities` entry (like
  `alarmBands`), because the manifest's scalar types are int/double/bool/string. Read/write
  hooks live in `PropertyHooks.{h,cpp}`; the generator emits serialization, form and API
  appliers; `registry-verify.py` proves the hooks exist.
- **Migration:** G2's `rebindWidgetRefs()` fills `datasetUniqueId` on first load from the
  legacy ordinal and persists it on the next save. G4's importer CSV/XML/JSON without the new
  columns imports byte-identical to today (all new fields default off; `computeBlocks` with
  every `unitId == 0` sorts exactly as before).
- **No Sessions DB / MDF4 / CSV schema change.**

## API / SDK surface

- `project.transformLibrary.get` / `.set` / `.dryRun` (new `TransformLibraryHandler`, registered
  in `CommandHandler::initializeHandlers()`; `set` → `Modify`, `dryRun` → `Safe` in
  `command_safety.json`).
- `project.dataset.*` gains `transformParams` through the generated API fields (typed as an
  object in the schema projection; gRPC number appended, never moved).
- `project.workspace.addWidget` accepts `datasetId`; `list` / `validate` emit it.
- `project.workspace.profile.list/add/update/remove/select`.
- `io.modbus.addRegisterGroup` already accepts `slaveAddress`; unchanged.
- SDK prelude: optional `unit` argument on the four `modbusWrite*` helpers; `SerialStudio.js`
  regenerated by `scripts/generate-sdk.py`.
- `--profile <name>` CLI option.
- Pro gating unchanged: importers and Output widgets stay behind `BUILD_COMMERCIAL`; merge is
  refused in GPL builds because a merged project has two sources.

## QML / UI

- **ProjectView.qml:** a "Shared Lua Library…" button beside the Lua execution-mode row opens
  `DatasetTransformEditor` in library mode (language locked to Lua, validation = the chunk
  compiles and runs; no `transform()` required). `DatasetTransformEditor` gets a small
  "Parameters" key/value table (add/remove rows; numeric strings stored as numbers, `true` /
  `false` as bools, everything else as strings) shown only in dataset mode.
- **Preview dialogs (Modbus, DBC, Proto):** a second footer button "Add to Current Project",
  enabled when `Cpp_JSON_ProjectModel` has a document and the mode is ProjectFile.
- **AddWidgetDialog.qml:** rows carry `datasetUniqueId`; the duplicate key string becomes
  `type:group:dataset:rel`.
- **WorkspacesView.qml:** a "Profiles" section (list + name field + checkable folder tree).
- **WorkspaceProfileDialog.qml:** pick-at-load list, remembered per project path in
  `QSettings` so the operator is not asked every launch; a "Show all" entry.
- Combo/list restore races: profile and parameter widgets read from the model only while
  unfocused, per the TableDelegate rule.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Library scope (spec open question) | (a) one chunk per project; (b) one per source | **(a).** It is what the field project needs, one editor, one API verb; evaluated into every source's state so per-source semantics fall out anyway. Per-source can be added later as a second key without breaking (a). |
| How the library reaches the transform | (a) prelude chunk into `_G`, env `__index` fallthrough; (b) a `require`-style module table; (c) concatenate the library in front of every transform chunk | **(a).** Zero per-call cost, zero change to entry isolation (entries still get their own env), no package machinery (non-goal). (c) would recompile 730 copies. |
| Parameter typing (spec open question) | (a) free-form string map; (b) JSON scalars number/string/bool; (c) typed schema per dataset | **(b).** Transforms want numbers without `tonumber`; JSON already carries the types; a schema is speculative. Table names are strings, so "table name" needs no special type. |
| Parameters on which lanes | Lua only; Lua + JS; all three | **Lua + JS.** Same object, two injection points. Expression has a fixed grammar; adding `params` there is a separate spec. |
| Tile identity | (a) identity is truth, ordinal is a rebound cache; (b) resolve by identity only, stop writing `relativeIndex`; (c) extend the existing shift/anchor arithmetic to inserts | **(a).** Older readers keep working (spec constraint), every ordinal consumer stays untouched, and a file written by a generator resolves at load. (b) breaks older readers; (c) cannot see edits made outside the app. |
| Merge mechanism | (a) JSON-level remap + append in `ProjectLoader`; (b) drive `ProjectModel` mutators one by one | **(a).** One undo step, one signal burst, one helper shared by three importers, and the remap is a pure function a ctest can pin. |
| Unit id column name (spec constraint) | `slave` / `unit_id` / `unitid` / `device` | **All four accepted; `slave` documented first.** `unit` is already the units column. |
| Word order vocabulary | `big`/`little`; `abcd`/`cdab`/`badc`/`dcba` | **The four-letter forms.** Unambiguous for 32- and 64-bit values; what every Modbus tool uses. |
| Write unit carrier | (a) odd-length payload prefix; (b) a per-control `unitId` property | **(a).** Reaches scripts, macros, `io.write` and the API with no schema change; odd length is provably unused today. |
| Profile semantics | (a) folder subset per profile; (b) a flag per folder | **(a) plus optional explicit workspace ids.** Overlapping profiles need (a); explicit ids cover top-level workspaces not in a folder. |
| Which profile at load | always ask; ask once and remember per path; CLI/API only | **Ask once, remember per path, `--profile` and API override.** An operator deployment opens the same file daily. |
| Writable rows → controls | skip (R7 is only a data-model change); emit controls with state feedback | **Emit controls bound to the block table.** AC6 needs a press to change the register; spec 0080 already gives the read-back for free. |

## Risks & mitigations

- **Generated-artifact drift** (`common-mistakes.md` ProjectModel row): `transformParams` goes
  through the manifest; `generate-property-registry.py --check` and `registry-verify.py` run
  per task; no hand edit of `Generated/`.
- **Undo capture without `setModified(true)`**: every new mutator (`setTransformLibraryCode`,
  profile CRUD, `mergeImportedProject`, `addWidgetToWorkspace` extension) opens a
  `ProjectUndoScope` and sets modified; `undo-scope-missing` lint is the check. Workspace-only
  edits stay in the whitelist by spec 0031.
- **Rebind order vs. deferred regen**: `rebindWidgetRefs()` must run after
  `flushWorkspaceRegen()` in `writeProjectFile()`, or a save could rebind against a stale
  workspace list. Named in the task and pinned by the AC2 test (save → reload → tiles intact).
- **Library errors masking transform errors**: the library failure is reported under id -1 so
  the 1 Hz diagnostics show it; the entries still compile so a typo in a helper does not blank
  the dashboard, only the datasets that call it (they fall back to raw, as today).
- **Lua `lua_atpanic` during library eval**: the chunk is loaded and run with `lua_pcall` on
  the state after the protected bootstrap; the deadline hook is enabled, so an infinite loop in
  a library body times out like a transform.
- **Stream lane config plumbing**: the worker must receive the library string even when the
  project has no stream datasets with Lua transforms; cheap (one QString copy at worker build).
- **Merge id collisions**: `nextUniqueId` allocation for groups, workspace ids past the current
  max user id, table names suffixed; the ctest enumerates a project already holding each kind.
- **`registerGroups` persistence vs. merge**: the Modbus driver replaces its group list on the
  bus message today; `append` is explicit so the "new project" path is byte-identical.
- **Modbus `write()` contract**: odd length is rejected today, verified by reading
  `Modbus.cpp:288-291`; the coil helper still writes a holding register (pre-existing, not
  changed here, noted in chat).
- **`%n` / `.arg()` translations**: new user-facing strings use numbered placeholders.
- **macOS file-dialog reentrancy**: the merge path keeps `QMetaObject::invokeMethod(...,
  QueuedConnection)` around any model mutation launched from a dialog slot.
- **Profiles hide the active workspace**: selecting a profile whose set excludes the current
  taskbar tab falls back to the first visible workspace.

## Addendum A — JavaScript library + Project Scripts node

**Data.** `Keys::TransformLibraryJs` (`"transformLibraryJs"`), `Frame::transformLibraryJs`
(read/write/reset next to `transformLibrary`), `ProjectModel::transformLibraryJsCode` property +
`setTransformLibraryJsCode` (undo scope key `transform-library-js`) + `transformLibraryJsChanged`,
loader/persistence, `newJsonFile` clears. Carried like the Lua one: `FrameBuilder::ProjectSnapshot`,
`Core::Bus::ProjectStructureSnapshot` (appended LAST, after `transformLibrary`),
`StreamConfig::transformLibraryJs` via `StreamConfigBuilder`.

**Engines.** Frame lane: `TransformCompiler::compileJsLibrary(js, engine, code)` runs before the
entries in `compileJs`; the `JsWatchdog` is created first and armed around the evaluate so a
runaway top-level loop is interrupted; a failure notes `kTransformLibraryJsErrorId` (-3). Stream
lane: `StreamProcessor::setupJsEngine` evaluates `m_config.transformLibraryJs` under
`m_jsWatchdog` arm/disarm after the table prelude. `ExternalWiring::watchProjectScripting` adds
`transformLibraryJsChanged -> syncFromProjectModel`. `ScriptCheckers::scriptDatasetLabel` names -3.
No per-sample cost: globals resolve through the engine's global object exactly as today.

**Editor.** `TransformLibraryEditor` gains `Q_PROPERTY(bool lua)`: picks model getter/setter/
signal, highlighter (`setScriptLanguage`), formatter language, starter comment, validate engine.
One `TransformLibraryView.qml` serves both, keyed on `currentView` (`TransformLibraryView` = Lua,
`JsLibraryView` = JS). `DatasetTransformEditor` receives both libraries
(`displayDialog(..., luaLibrary, jsLibrary)`, `setLibraryCode(lua, js)`); the JS validate/test
paths evaluate the JS library first; the toolbar button reads "Open Lua Library" / "Open
JavaScript Library" and emits `libraryEditRequested(int language)`.

**Tree.** `EntityKind` appends `KindScriptsRoot`, `KindJsLibrary`. `EditorTree::appendScriptsTree
(root, expandedStates)` builds the **Project Scripts** node (icon `editor/code`) with Control Loop,
Lua Library, JavaScript Library as children, `restoreExpandedStateMap` on `<root>/Project Scripts`,
search filter keeps the node when the title or any child matches. `m_scriptsRootItem`,
`m_jsLibraryItem`; `EditorSelection` routes them to `ProjectScriptsView` / `JsLibraryView`; nav
history resolves both. New `ProjectScriptsView.qml`: three rows (name, one-line purpose, Open).

**API.** `TransformLibraryHandler`: optional `language` param on all three commands (`lua`
default; `js`/`javascript`); `get` echoes `language`; JS `dryRun` uses `ScriptDryRun` JS evaluate.

**Files (new):** `app/qml/ProjectEditor/Views/ProjectScriptsView.qml`. **Files (touched):**
`FrameKeys.h`, `Frame.h`, `Messages.h`, `StreamConfig.h`, `StreamConfigBuilder.cpp`,
`ProjectModel.{h,cpp}`, `ProjectLoader.cpp`, `ProjectPersistence.cpp`, `EntityKinds.h`,
`FrameBuilder.{h,cpp}`, `FrameBuilder/ExternalWiring.cpp`, `FrameBuilder/TransformCompiler.{h,cpp}`,
`IO/StreamWorker.cpp`, `ScriptCheckers.cpp`, `TransformLibraryHandler.cpp`,
`TransformLibraryEditor.{h,cpp}`, `DatasetTransformEditor.{h,cpp}`, `EditorForms.cpp`,
`EditorTree.{h,cpp}`, `EditorSelection.{h,cpp}`, `EditorSummaries.{h,cpp}`, `ProjectEditor.{h,cpp}`,
`ProjectEditor.qml`, `TransformLibraryView.qml`, `app/CMakeLists.txt`, docs (`Dataset-Transforms.md`,
`Project-Editor.md`, `API-Reference.md`, `transforms.md` skill, `scripting.md`, `CLAUDE.md`),
`tests/integration/test_dataset_transforms.py`.

**Verification.** AC9/AC10: `pytest tests/integration/test_dataset_transforms.py` (new
`test_js_library_function_shared_with_params`, `test_js_library_dry_run_reports_errors`,
round-trip extended). AC11: maintainer observation. Hotpath: compile-time only, no benchmark
change beyond the earlier one-off.

## Test & verification plan

- **Unit (you can run):**
  - `tests/scripts/test_modbus_lua_parser.py` (new): renders the generated Lua for a two-unit
    map by calling the importer's codegen through a fixture (`tests/fixtures/modbus/two_units.csv`
    and the expected Lua checked in), runs it under `luajit` with `tableSet`/`bit` stubs, feeds a
    captured reply sequence with one reply dropped, asserts every register lands in the right
    table (**AC4**); also an `rbit` row and a `cdab` float.
  - `pytest tests/scripts/` stays green (existing parser tests untouched).
- **C++ ctest (maintainer builds, agent runs against the build dir):**
  - `tst_project_merge`: `importedIdRemap` over a project holding sources, groups, tables,
    workspaces; asserts no id collision and every ref remapped (**AC3** structural half).
  - `tst_workspace_rebind` (pure over `WorkspaceKeys`): insert a group before a referenced one,
    rebind, ordinal moved, identity kept; legacy ref gets `datasetUniqueId` filled (**AC2**
    structural half).
- **Integration (maintainer runs; app up with API server):**
  - `tests/integration/test_dataset_transforms.py`: new cases — Lua library function called from
    two datasets with different `transformParams` yields the expected values; JS `params`; a
    library syntax error leaves untransformed datasets intact (**AC1** mechanism).
  - `tests/integration/test_workspace_identity.py` (new): load fixture, add a tile for group B's
    plot, insert group A at index 0 via `project.group.add` + reorder, `project.workspace.list`
    still resolves the same dataset uniqueId; save, reload, same (**AC2**).
  - `tests/integration/test_modbus_groups.py`: extend the socket server to answer as unit 1 and
    unit 2; assert `io.write` of `[2, addr_hi, addr_lo, v_hi, v_lo]` arrives with unit id 2
    while the connection's unit is 1 (**AC7**); even payloads still target unit 1.
  - `tests/integration/test_project_editor.py`: profile CRUD + `select` filters
    `project.workspace.list` (**AC8** mechanism).
- **Maintainer observation:**
  - **AC1** field-project sensor channels rewritten as `return lib.rtd(v, params)`; compare
    `dashboard.getData` against the pre-change file on the simulator.
  - **AC3** import the load-bank map into the open field project; existing groups/tables/
    workspaces untouched (diff the saved file minus the new folder).
  - **AC5/AC6** Modbus PLC Simulator example: LED follows a bit row; the generated control
    changes a register.
  - **AC8** open the field project with one engine profile; only that engine's workspaces show.
- **Hotpath:** `--benchmark-hotpath` once after G1 lands (Dataset size changed); no gate
  expected to move.
- **Static:** `python3 scripts/code-verify.py --check <files>` per task;
  `scripts/generate-property-registry.py --check`, `scripts/registry-verify.py`,
  `scripts/generate-sdk.py --check`, `scripts/claim-verify.py`, `scripts/layer-verify.py`;
  `qt-cpp-review` on the C++ diff before handoff; `python3 scripts/sanitize-commit.py` before
  commit.
