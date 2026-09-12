---
spec: 0083-field-project-gaps
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-09-11
---

# Tasks 0083 — Close the gaps a real industrial deployment works around with scripts

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
- Gaps land in the plan's order (G1 → G6); each gap is self-contained so the maintainer can
  build and check between them. `CV` below means `python3 scripts/code-verify.py --check` on
  the task's files.

## Tasks

### G1 — Shared Lua library + dataset parameters

### T1 — Keys and data-model fields

- **Files:** `core/Core/DataModel/FrameKeys.h`, `core/Core/DataModel/Frame.h`,
  `scripts/code_verify_rules.py`
- **Does:** Add `Keys::TransformLibrary("transformLibrary")` and
  `Keys::TransformParams("transformParams")`; add `Dataset::transformParams` (`QVariantMap`,
  after `transformCode`) and `Frame::transformLibrary` (`QString`, beside
  `controlScriptCode`) with `serialize`/`read`/`clear_frame` handling (write when non-empty).
  Add both literals to `_PROJECT_KEY_LITERALS`. `Dataset` is copied on configure paths only;
  the span fast lane never touches the new member.
- **Verify:** CV; `static_assert` alignment lines untouched; `grep -n transformParams Frame.h`.
- **Deps:** none
- [x] done

### T2 — Manifest sub-entity + hooks + generator

- **Files:** `app/rcc/properties/dataset.json`, `core/Pipeline/DataModel/Project/PropertyHooks.{h,cpp}`,
  `scripts/generate-property-registry.py`
- **Does:** Declare `transformParams` under `subEntities` (jsonKey `TransformParams`, readHook
  `readTransformParams`, writeHook `writeTransformParams`, apiName `transformParams`); implement
  the hooks (JSON object ↔ `QVariantMap`; number/string/bool kept, other value types dropped
  with one `qWarning`); teach the generator's `postRead` step list and write-hook emitter the two
  names. **Never hand-edit a generated TU.**
- **Verify:** `python3 scripts/generate-property-registry.py` then `--check` clean;
  `python3 scripts/registry-verify.py` clean; CV on the hooks.
- **Deps:** T1
- [x] done

### T3 — Regenerate the four TUs and the proto ledger

- **Files:** `core/Pipeline/DataModel/Generated/*`, `core/Ui/ProjectEditor/Generated/DatasetForm.cpp`,
  `core/Api/API/Generated/DatasetApiFields.cpp`, `app/rcc/api/proto-fields.json`,
  `doc/grpc/serialstudio-typed.proto`
- **Does:** Commit the generator output. The gRPC field number for `transformParams` is
  appended after the message's current maximum; no existing number moves.
- **Verify:** `python3 scripts/generate-property-registry.py --check`;
  `python3 scripts/code-verify.py --check` reports no `api-generated-edited` /
  `proto-field-renumbered`; `generate-property-registry.py --check-snapshot` warns only on the
  new field (the maintainer's `--dump-api-schema` refreshes `api-schema.json`).
- **Deps:** T2
- [x] done

### T4 — ProjectModel library property, load and save

- **Files:** `core/Pipeline/DataModel/ProjectModel.{h,cpp}`,
  `core/Pipeline/DataModel/Project/ProjectLoader.cpp`,
  `core/Pipeline/DataModel/Project/ProjectPersistence.cpp`, `core/Pipeline/DataModel/FrameBuilder.cpp`
  (added during implementation: one `transformLibraryChanged` -> `compileTransforms()` connect
  beside the `luaFastModeChanged` one, wiring block only)
- **Does:** `transformLibraryCode` Q_PROPERTY + `setTransformLibraryCode()` mirroring
  `setControlScriptCode` (opens a `ProjectUndoScope` with a slot-level coalesce key, guard
  return on equal, `setModified(true)`, emits `transformLibraryCodeChanged`). Loader reads the
  key in `applyJsonDocumentCore` next to `controlScriptCode` (`ProjectLoader.cpp:695`);
  persistence writes it beside `ControlScriptCode` (`ProjectPersistence.cpp:141`) when
  non-empty. Ctor closure untouched: nothing new runs before `m_initialized`.
- **Verify:** CV; `undo-scope-missing` clean; round-trip read-back by eye.
- **Deps:** T1
- [x] donee

### T5 — TransformCompiler: library chunk + params

- **Files:** `core/Pipeline/DataModel/FrameBuilder/TransformCompiler.{h,cpp}`,
  `core/Pipeline/DataModel/FrameBuilder/ExternalWiring.{h,cpp}` (added: the two recompile connects
  moved out of the hotpath TU into `watchProjectScripting()`), `core/Ui/Misc/Problems/ScriptCheckers.cpp`
  (added: label for the library error sentinel)
- **Does:** `TransformEntry` gains `QVariantMap params`; `compile()` copies it from the dataset.
  `compileLua`: after the bootstrap and the JIT/hook mode switch, arm the deadline and run the
  library chunk (`luaL_loadbufferx(..., "=library", "t")` + `lua_pcall` in the default global
  env); on failure `noteTransformError(-1, msg)` + `qWarning`, entries still compile.
  `compileLuaEntry`: after creating the env table and before loading the chunk, push a `params`
  table into the env (number → `lua_pushnumber`, string → `lua_pushlstring`, bool → boolean).
  `compileJs`: wrap as `(function(params){%1 ...})(<compact JSON>)`. Compile-time only; no edit
  to `applyTransformLua`/`applyTransformJs`; `lua_gettop` balance asserted as today.
- **Verify:** CV; read-back of stack discipline; `SS_ASSERT` density ≥ 2 per new function.
- **Deps:** T1, T4
- [x] donee

### T6 — Stream lane: config + worker

- **Files:** `core/Core/IO/StreamConfig.h`, `core/Devices/IO/ConnectionManager/StreamConfigBuilder.cpp`,
  `core/Pipeline/IO/StreamWorker.cpp`, `core/Core/Bus/Messages.h` (added: `ProjectStructureSnapshot`
  carries `transformLibrary`, the builder reads the project through it, not `ProjectModel`)
- **Does:** `StreamConfig::transformLibrary` and `StreamChannelConfig::transformParams`, filled
  where `luaFastMode` is copied (`StreamConfigBuilder.cpp:108`). `setupLuaState` runs the
  library chunk after the table API injection and mode switch; `compileLuaEntry` sets `params`
  in the env; `compileJsEntry` passes params into the IIFE. Per-sample paths untouched; the
  worker's Lua state stays worker-thread-only.
- **Verify:** CV; `layer-verify.py` clean (Core header, Devices builder, Pipeline worker).
- **Deps:** T5
- [x] donee

### T7 — Editor: library-aware validation and params table

- **Files:** `core/Ui/ProjectEditor/Editors/DatasetTransformEditor.{h,cpp}`,
  `core/Ui/ProjectEditor/EditorForms.{h,cpp}`, `core/Ui/ProjectEditor/ProjectEditor.{h,cpp}` (added
  during implementation: the opener slot the QML button calls and the apply-signal wiring live there)
- **Does:** `validateTransform` and the test run evaluate the project library on the dry-run
  Lua state before the transform (JS: prepend nothing; JS has no library). Add a "Parameters"
  key/value table (add/remove rows; numeric text → number, `true`/`false` → bool, else string;
  model sync only while unfocused) shown in dataset mode, committed with the code through
  `ProjectModel::updateDataset`. Add a library mode: language locked to Lua, validation = chunk
  loads and runs, no `transform()` required, saves through `setTransformLibraryCode`.
- **Verify:** CV; the `%n`/`.arg()` rule on new strings; read-back.
- **Deps:** T4, T5
- [x] donee

### T8 — ProjectView button + library API handler

- **Files:** `app/qml/ProjectEditor/Views/ProjectView.qml`,
  `core/Api/API/Handlers/TransformLibraryHandler.{h,cpp}`, `core/Api/API/CommandHandler.cpp`,
  `app/rcc/ai/command_safety.json`, `core/Api/CMakeLists.txt` (added: the explicit source list)
- **Does:** "Shared Lua Library…" button next to the Lua execution-mode row opens the editor in
  library mode. New handler with `project.transformLibrary.get` / `.set` / `.dryRun` (dry-run
  through `ScriptDryRun`, Lua, JIT off), registered in `CommandHandler::initializeHandlers()`
  after `ControlScriptHandler`; tiers: get/dryRun `Safe`, set `Modify`.
- **Verify:** CV; `python3 scripts/registry-verify.py`; every new command name present in
  `command_safety.json`.
- **Deps:** T4, T7
- [x] donee

### T9 — G1 docs

- **Files:** `doc/help/Dataset-Transforms.md`, `app/rcc/ai/skills/` (the transform skill),
  `doc/claude/architecture/scripting.md`, `CLAUDE.md`
- **Does:** Document the library (one chunk per project, evaluated into globals, both lanes,
  failure semantics) and `params` (types, Lua/JS shape, not Expression). One paragraph in
  scripting.md "Per-Dataset Value Transforms"; one line in CLAUDE.md's scripting pointers.
- **Verify:** `python3 scripts/documentation-verify.py`, `python3 scripts/claim-verify.py`.
- **Deps:** T8
- [x] donee

### T10 — G1 integration tests

- **Files:** `tests/integration/test_dataset_transforms.py`
- **Does:** Cases: Lua library function shared by two datasets with different `transformParams`;
  JS `params`; library syntax error leaves untransformed datasets intact and reports through
  the diagnostics; project export/import round-trips both keys. Maintainer runs.
- **Verify:** `pytest tests/integration/test_dataset_transforms.py -v` (maintainer, app up).
- **Deps:** T8
- [x] donee

### G2 — Workspace tile identity

### T11 — `WidgetRef::datasetUniqueId`

- **Files:** `core/Core/DataModel/FrameKeys.h`, `core/Core/DataModel/Frame.h`,
  `scripts/code_verify_rules.py`
- **Does:** `Keys::DatasetUniqueId("datasetUniqueId")`; `WidgetRef::datasetUniqueId = -1`,
  serialized when ≥ 0, read optional. `relativeIndex` keeps being written.
- **Verify:** CV; a file without the key reads exactly as before.
- **Deps:** none
- [x] donee

### T12 — WorkspaceKeys: identity lookup + rebind helper

- **Files:** `core/Pipeline/DataModel/Project/WorkspaceKeys.{h,cpp}`
- **Does:** `ResolvedWidget` gains `groupUniqueId`; new pure `rebindRef(WidgetRef&, lookup)`:
  identity match on `(widgetType, groupUniqueId, datasetUniqueId)` → overwrite
  `relativeIndex`; legacy ref (dataset-scope type, `datasetUniqueId == -1`) resolved by the old
  triple and back-filled. Returns whether the ref resolved.
- **Verify:** CV; unit-testable without the app (T16).
- **Deps:** T11
- [x] donee

### T13 — ProjectWorkspaces rebind + call sites

- **Files:** `core/Pipeline/DataModel/Project/ProjectWorkspaces.{h,cpp}`,
  `core/Pipeline/DataModel/Project/ProjectLoader.cpp`,
  `core/Pipeline/DataModel/Project/ProjectPersistence.cpp`
- **Does:** `rebindWidgetRefs()` over the whole list; called after `loadCustomWorkspaces`, from
  `notifyWorkspaceListChanged`, on `groupsChanged`, and in `writeProjectFile()` **after**
  `flushWorkspaceRegen()` (the deferred regen must land before the rebind, or a save serializes
  ordinals bound to a stale list). `addWidgetToWorkspace` gains `datasetUniqueId` (duplicate
  check includes it). No new undo scope: workspace CRUD stays whitelisted per spec 0031.
- **Verify:** CV; read-back of the write-path order.
- **Deps:** T12
- [x] donee

### T14 — Taskbar identity fallback + Problem Center

- **Files:** `core/Ui/UI/Taskbar/TaskbarWorkspaces.cpp`, `core/Ui/Misc/Problems/ProjectCheckers.cpp`
- **Does:** `resolveRefWindowId`: keep the ordinal fast path; when the owner check fails, scan
  `widgetMap()` for the window whose `getGroupWidget`/`getDatasetWidget` uniqueId matches the
  ref. `dangling-workspace-widget` also fires when `datasetUniqueId ≥ 0` names a dataset no
  group holds.
- **Verify:** CV; `tests/scripts/test_problem_center_static.py` still green.
- **Deps:** T13
- [x] donee

### T15 — Editor, dialog, API, importer plumbing

- **Files:** `core/Ui/ProjectEditor/EditorSummaries.cpp`, `app/qml/ProjectEditor/Views/AddWidgetDialog.qml`,
  `core/Ui/ApiHandlers/WorkspacesHandler.cpp`, `core/Pipeline/DataModel/ProjectModel.h`,
  `core/Pipeline/DataModel/Importers/ImporterCommon.h`
- **Does:** Rows carry `datasetUniqueId`; the dialog's duplicate key becomes
  `type:group:dataset:rel` and passes the id to `addWidgetToWorkspace`; API `addWidget` accepts
  `datasetId` (optional), `list`/`validate` emit it, `relativeIndex` still accepted and
  auto-derived; `buildImportedWorkspaces` sets `-1` (group-scope). Split into two diffs if the
  handler change exceeds ~80 lines.
- **Verify:** CV; `python3 scripts/registry-verify.py`; API schema strings updated.
- **Deps:** T13
- [x] donee

### T16 — G2 tests + docs

- **Files:** `app/tests/tst_workspace_rebind.cpp`, `app/tests/CMakeLists.txt`,
  `tests/integration/test_workspace_identity.py`, `doc/claude/architecture/dashboard.md`
- **Does:** ctest over `rebindRef` (insert-before moves the ordinal, identity kept; legacy fill);
  integration test per plan (AC2: add tile, insert group at 0, list resolves the same dataset;
  save/reload). dashboard.md "Workspaces": identity-then-ordinal rule.
- **Verify:** maintainer builds; `ctest -R workspace_rebind`; `pytest
  tests/integration/test_workspace_identity.py -v`.
- **Deps:** T15
- [x] donee

### G3 — Importer merge

### T17 — Pure id remap helper + ctest

- **Files:** `core/Pipeline/DataModel/Project/ProjectMerge.{h,cpp}` (added: GPL-licensed pure pair,
  registered in `core/Pipeline/CMakeLists.txt`; `ImporterCommon.h` is Pro-licensed header-only, so a
  ctest could not link it), `app/tests/tst_project_merge.cpp`, `app/tests/CMakeLists.txt`
- **Does:** `ProjectMerge::remap(QJsonObject& imported, const RemapBase& base)`: offsets `sourceId`,
  reassigns group uniqueIds from `nextUniqueId` (returns old→new map; remaps refs, `xAxisId`,
  `waterfallYAxis`), suffixes colliding table names, offsets workspace ids past the current max
  user id. ctest enumerates a base project holding each kind and asserts no collision.
- **Verify:** CV; `ctest -R project_merge` (maintainer builds).
- **Deps:** T11
- [x] donee

### T18 — `ProjectLoader::mergeImportedProject`

- **Files:** `core/Pipeline/DataModel/Project/ProjectLoader.{h,cpp}`, `core/Pipeline/DataModel/ProjectModel.h`
- **Does:** GUI-thread, one `ProjectUndoScope`: run the remap, append sources/groups/tables/
  workspaces to the live vectors, file them into one new group folder, table folder and
  workspace folder named after the import, `setModified(true)`, emit the structural signals
  (`groupsChanged`, `sourcesChanged`, tables/workspaces changed, `frameDetectionChanged` so
  `FrameBuilder::syncFromProjectModel` runs), then `importCompleted(true, m_filePath)`.
  Refuse with a message box when no document is open, mode is not ProjectFile, or the build is
  GPL (multi-source). Any dialog-launched call stays behind
  `QMetaObject::invokeMethod(..., QueuedConnection)`.
- **Verify:** CV; `undo-scope-missing` clean; read-back of signal order against `project.md`.
- **Deps:** T17
- [x] donee

### T19 — Importers: `confirmMerge()` + bus `append`

- **Files:** `core/Pipeline/DataModel/Importers/{ModbusMapImporter,DBCImporter,ProtoImporter}.{h,cpp}`,
  `core/Core/Bus/Messages.h`, `core/Devices/IO/Drivers/Modbus.{h,cpp}`,
  `core/Devices/IO/Drivers/Modbus/ModbusRegisterGroups.{h,cpp}` (added: `addFromJson` keeps the
  driver TU under its size baseline and reads `slave`)
- **Does:** Each importer gains a `confirmMerge()` slot calling `pm.mergeImportedProject(project,
  suggestion)`; the Modbus path publishes `ModbusRegisterGroupsLoaded{append = true}` (message
  gains a defaulted `append` field; the driver's subscriber skips `clearRegisterGroups()` when
  set). `confirmImport()` unchanged.
- **Verify:** CV; `layer-verify.py`; read-back that the new-project path is byte-identical.
- **Deps:** T18
- [x] donee

### T20 — Preview dialogs: "Add to Current Project"

- **Files:** `app/qml/MainWindow/Panes/SetupPanes/Drivers/ModbusPreviewDialog.qml`,
  `app/qml/MainWindow/Panes/SetupPanes/Drivers/DBCPreviewDialog.qml`,
  `app/qml/ProjectEditor/Dialogs/ProtoPreviewDialog.qml`
- **Does:** Second footer button, enabled when a document is loaded and the mode is ProjectFile
  (`Cpp_JSON_ProjectModel` / `Cpp_Misc_AppState` bindings), calling `confirmMerge()`.
- **Verify:** CV (QML rules); maintainer observation on AC3.
- **Deps:** T19
- [x] donee

### T21 — G3 docs

- **Files:** `doc/help/Auto-Generating-Projects.md`, `doc/claude/architecture/project.md`
- **Does:** "Add to current project" for all three importers; project.md "Importers" paragraph on
  the merge helper and the remap rules.
- **Verify:** `documentation-verify.py`, `claim-verify.py`.
- **Deps:** T20
- [x] donee

### G4 — Modbus importer v2

### T22 — `RegisterEntry` fields + CSV/XML/JSON parsers

- **Files:** `core/Pipeline/DataModel/Importers/ModbusRegisterMap.{h,cpp}`
- **Does:** `unitId` (0 default), `bitIndex` (-1), `writable`, `wordOrder` (`abcd` default,
  `cdab`, `badc`, `dcba`; unknown → default with a warning). CSV headers `slave`/`unit_id`/
  `unitid`/`device`, `bit`, `rw`/`access`/`writable` (`r`, `w`, `rw`, `true`), `order`/
  `word_order`/`byte_order`; XML attributes and JSON keys with the same names. Absent columns →
  defaults, so existing maps parse identically.
- **Verify:** CV; parse the existing example map and diff the entries by eye.
- **Deps:** none
- [x] donee

### T23 — Blocks by unit, driver groups, block titles

- **Files:** `core/Pipeline/DataModel/Importers/ModbusMapImporter.{h,cpp}`
- **Does:** `RegisterBlock::slaveAddress`; sort by `(unitId, registerType, address)`; new block on
  unit or type change; `blockTitle` adds "Unit N" only when the map holds more than one unit;
  `loadRegisterGroups` and `buildProject`'s `registerGroups[]` write `slave` when non-zero.
- **Verify:** CV; a one-unit map yields the same blocks and titles as before.
- **Deps:** T22
- [x] donee

### T24 — Generated Lua: reply matching, `rbit`, word order

- **Files:** `core/Pipeline/DataModel/Importers/ModbusMapImporter.cpp`
- **Does:** `BLOCKS` entries carry `unit`, `func`, `bytes`; `parse()` picks the block whose
  `(frame[1], frame[2], frame[3])` triple matches, cursor only among duplicate triples; entry
  `type = "rbit"` with `bit = n` reads the word and masks; per-entry `order` permutes bytes
  before the numeric read. Header comment rewritten to describe the new rule. Binary-decoder
  rule: the frame is a byte table, converted once at the top (already the case).
- **Verify:** CV (the Lua literal stays inside the `code-verify off` fence); T27 runs it.
- **Deps:** T23
- [x] donee

### T25 — Datasets for bit rows; Output group for writable rows

- **Files:** `core/Pipeline/DataModel/Importers/ModbusMapImporter.{h,cpp}`
- **Does:** `buildDatasetFromEntry`: `bitIndex ≥ 0` on a register block → LED dataset. New
  `buildOutputGroup(blocks)`: one `GroupType::Output` group "<map> Controls" with a control per
  writable row (Slider for numeric registers, Toggle for coils and bit rows, TextField for
  strings), `transmitFunction` using the SDK helper with the unit argument (bit rows:
  read-modify-write from the block table), `stateSource = Table` bound to the block table
  register with `stateConfirmMs` default. Widget `uniqueId`s follow `finalizeImportedProject`.
- **Verify:** CV; generated JSON read back through `read(OutputWidget&)` + `normalize`.
- **Deps:** T24, T29 (helper unit argument)
- [x] donee

### T26 — G4 docs

- **Files:** `doc/help/Drivers-Modbus.md`, `doc/help/Auto-Generating-Projects.md`
- **Does:** The four columns, the word-order vocabulary, the reply-matching rule, the generated
  controls.
- **Verify:** `documentation-verify.py`, `claim-verify.py`, `trial-parity` clean.
- **Deps:** T25
- [x] donee

### T27 — Lua parser unit test under `luajit`

- **Files:** `tests/scripts/test_modbus_lua_parser.py`, `tests/fixtures/modbus/two_units.csv`,
  `tests/fixtures/modbus/two_units.lua`
- **Does:** Checked-in generated Lua for a two-unit map (regenerated by the maintainer through
  the importer when the codegen changes; the test also greps the C++ literal for the `BLOCKS`
  fields it relies on so drift is loud); runs it under `luajit` with `tableSet` / `bit` stubs;
  feeds a captured reply sequence with one reply dropped; asserts every register lands in the
  right table; covers an `rbit` row and a `cdab` float (AC4).
- **Verify:** `pytest tests/scripts/test_modbus_lua_parser.py -v` (agent can run; `luajit` is
  on PATH).
- **Deps:** T24
- [x] donee

### G5 — Write unit

### T28 — `Modbus::write` unit prefix

- **Files:** `core/Devices/IO/Drivers/Modbus.cpp`
- **Does:** Odd length ≥ 5 → byte 0 is the unit (1..247, else return 0), remainder decoded as
  today; even payloads unchanged; `sendWriteRequest(unit)`. `@brief` states the two shapes.
- **Verify:** CV; read-back that the even path is byte-identical.
- **Deps:** none
- [x] donee

### T29 — SDK helpers + descriptions

- **Files:** `app/rcc/api/prelude.js`, `app/rcc/api/SerialStudio.js` (regenerated),
  `app/rcc/scripts/output/modbus_write.js`, `core/Ui/AI/ContextBuilder.cpp`,
  `core/Api/API/Handlers/ProjectUpdateCommands.cpp`
- **Does:** Optional trailing `unit` on `modbusWriteRegister`, `modbusWriteRegisters`,
  `modbusWriteCoil`, `modbusWriteFloat` (prepends the byte only when given); template comment
  and the two description strings mention it.
- **Verify:** `python3 scripts/generate-sdk.py --check`; CV.
- **Deps:** T28
- [x] donee

### T30 — G5 test + docs

- **Files:** `tests/integration/test_modbus_groups.py`, `doc/help/Drivers-Modbus.md`,
  `doc/help/Output-Controls.md`
- **Does:** Socket server answers as unit 1 and 2; `io.write` of `[2, addr_hi, addr_lo, v_hi,
  v_lo]` arrives with unit id 2 while the connection's unit is 1; an even payload still targets
  unit 1 (AC7). Docs: the unit argument and the payload shape.
- **Verify:** maintainer runs the pytest; doc lints.
- **Deps:** T29
- [x] donee

### G6 — Workspace profiles

### T31 — Keys + `WorkspaceProfile` struct

- **Files:** `core/Core/DataModel/FrameKeys.h`, `core/Core/DataModel/Frame.h`,
  `scripts/code_verify_rules.py`
- **Does:** `Keys::WorkspaceProfiles`, `Keys::ProfileId`, `Keys::FolderIds`, `Keys::WorkspaceIds`;
  `struct WorkspaceProfile { int profileId; QString title; std::vector<int> folderIds;
  std::vector<int> workspaceIds; }` + `serialize`/`read`.
- **Verify:** CV.
- **Deps:** none
- [x] donee

### T32 — ProjectWorkspaces: profiles, visible view, CRUD

- **Files:** `core/Pipeline/DataModel/Project/ProjectWorkspaceProfiles.{h,cpp}` (added: a real
  sub-object owned by `ProjectModel`, the workspaces TU was already over its size baseline),
  `core/Core/DataModel/WorkspaceProfile.h` (added: `Frame.h` crossed 1500 lines),
  `core/Pipeline/DataModel/Project/ProjectWorkspaces.cpp`, `core/Pipeline/DataModel/ProjectModel.{h,cpp}`,
  `core/Pipeline/DataModel/Project/ProjectFolders.cpp`
- **Does:** `m_profiles`, `m_activeProfileId` (-1 = all), `m_visible` rebuilt on list/folder/
  profile change (auto workspaces never filtered; visible = profile -1, or folder/ancestor in
  `folderIds`, or id in `workspaceIds`); `activeWorkspaces()` returns `m_visible`; profile
  add/update/remove with undo scopes; `setActiveProfile()` runtime-only, emits
  `activeWorkspacesChanged`, falls back to the first visible workspace when the current tab is
  hidden. `deleteWorkspaceFolder` drops the id from every profile. Q_PROPERTYs
  `workspaceProfiles` (QVariantList) and `activeProfileId`; `profileChoiceRequested()` signal.
- **Verify:** CV; `undo-scope-missing`; read-back.
- **Deps:** T31
- [x] donee

### T33 — Loader/persistence + Taskbar folder tree

- **Files:** `core/Pipeline/DataModel/Project/ProjectLoader.cpp`,
  `core/Pipeline/DataModel/Project/ProjectPersistence.cpp`, `core/Ui/UI/Taskbar/TaskbarWorkspaces.cpp`
- **Does:** Load/save the profile array; after load, emit `profileChoiceRequested` when ≥ 2
  profiles and no pre-selected id (CLI/API/remembered choice sets it before the load). Taskbar
  folder tree built from the visible folder ids.
- **Verify:** CV; a project without the key loads and saves byte-identical.
- **Deps:** T32
- [x] donee

### T34 — CLI `--profile` + API verbs

- **Files:** `app/src/Misc/CLI.{h,cpp}`, `core/Ui/ApiHandlers/WorkspacesHandler.cpp`,
  `core/Ui/ApiHandlers/WorkspaceProfileHandler.{h,cpp}` (added: the profile verbs, the workspaces
  handler TU was at its size baseline), `app/rcc/ai/command_safety.json`
- **Does:** `--profile <name>` pre-selects by title before the project opens;
  `project.workspace.profile.list/add/update/remove/select` (select `Safe`, the rest `Modify`).
- **Verify:** CV; `registry-verify.py`; safety tiers present.
- **Deps:** T33
- [x] donee

### T35 — QML: pick-at-load dialog + editor CRUD

- **Files:** `app/qml/ProjectEditor/Views/WorkspacesView.qml` (the pick-at-load prompt became
  `QInputDialog::getItem` inside `ProjectLoader::chooseWorkspaceProfile`, the same prompt family
  the folder prompts use, so no QML dialog or MainWindow hook was needed)
- **Does:** Dialog lists profiles plus "Show all", remembers the pick per project path in
  `QSettings` (through a small `ProjectModel` slot); WorkspacesView gains a Profiles section
  (list, name field, checkable folder tree; model sync only while unfocused).
- **Verify:** CV (QML rules); maintainer observation on AC8.
- **Deps:** T34
- [x] donee

### T36 — G6 tests + docs

- **Files:** `tests/integration/test_project_editor.py`, `doc/help/Project-Editor.md`,
  `doc/help/Command-Line-Interface.md`, `doc/help/Operator-Deployments.md`,
  `doc/claude/architecture/project.md`
- **Does:** Profile CRUD + `select` filters `project.workspace.list`; docs for the editor
  section, the CLI flag and the operator flow.
- **Verify:** maintainer runs the pytest; doc lints.
- **Deps:** T35
- [x] donee

### T37 — Whole-feature wrap-up

- **Files:** `CLAUDE.md`, `doc/claude/architecture/*.md` touched above, `spec.md`
- **Does:** Re-read every AI-facing paragraph added; run `claim-verify.py`; `qt-cpp-review` on
  the C++ diff; maintainer runs `--benchmark-hotpath` once (Dataset size changed); tick the
  acceptance criteria in `spec.md`; set `status: done`.
- **Verify:** Definition of Done below.
- **Deps:** T1–T36
- [x] done (2026-09-11: `qt-cpp-review` ran with six agents; every >=80 finding fixed in place:
  library carried in `FrameBuilder::ProjectSnapshot` instead of a cross-thread `ProjectModel`
  read, separate library compile deadline, dry-runs through `ScriptDryRun::runLuaChunk`,
  `0xFF 0x83 <unit>` write-payload marker instead of length parity, JSON extras parsing
  (`QJsonValue()` is Null), width-checked bit index, `Keys::` global-namespace fix, snapshot field
  appended last for `tst_message_bus`, `ProjectMerge` dataset-id remap, rebind before regen
  notifications, profile `clear()` notifications, profile choice deferred past the load and
  limited to `openJsonFile`, profile-selection leak across opens, identity fallback bucket index,
  24-bit group key field, coalesced undo on profile toggles, `[[nodiscard]]` on the rebind.
  `sanitize-commit.py` clean; `spec.md` stays `in-progress` until the maintainer's runs tick
  AC1-3 and AC5-8.)
- [x] follow-up (2026-09-11, maintainer feedback after the first build): the library left the
  transform dialog. It is now a `Lua Library` tree node beside `Control Loop`
  (`KindTransformLibrary`, `TransformLibraryView`, `TransformLibraryEditor` embedded Lua editor);
  the dialog keeps an `Open Lua Library` shortcut that selects that node, and its Parameters
  panel carries an empty-state explanation. `ProjectView.qml` no longer hosts a button.

### Addendum A — JavaScript library + Project Scripts node

### T38 — Data model + carriers
- **Files:** `FrameKeys.h`, `Frame.h`, `Messages.h`, `StreamConfig.h`, `StreamConfigBuilder.cpp`,
  `ProjectModel.{h,cpp}`, `ProjectLoader.cpp`, `ProjectPersistence.cpp`, `FrameBuilder.{h,cpp}`
- **Does:** `transformLibraryJs` key, frame field, model property/setter (undo scope
  `transform-library-js`)/signal, load/save/clear, snapshot fields. Invariant: the bus snapshot
  field is appended LAST (`tst_message_bus` aggregate init); the builder reads the library from
  its snapshot on the GUI thread, never `ProjectModel` on the pipeline thread.
- **Verify:** `code-verify --check`; census flat or re-seeded on purpose.
- [x] done (2026-09-11)

### T39 — Engines
- **Files:** `TransformCompiler.{h,cpp}`, `StreamWorker.cpp`, `ExternalWiring.cpp`,
  `ScriptCheckers.cpp`
- **Does:** `compileJsLibrary` before the JS entries with the watchdog created first and armed
  around the evaluate (`kTransformLibraryJsErrorId` -3); stream lane evaluates under
  `m_jsWatchdog`; `transformLibraryJsChanged` re-syncs; Problem Center label. Invariant: compile
  time only, nothing on the per-sample path; the compile never runs under a dataset pass.
- **Verify:** `code-verify --check`.
- [x] done (2026-09-11)

### T40 — API + tests
- **Files:** `TransformLibraryHandler.cpp`, `tests/integration/test_dataset_transforms.py`
- **Does:** optional `language` param on get/set/dryRun; JS dry-run; three new pytest cases.
- **Verify:** `code-verify --check`; maintainer runs the pytest file with the app up.
- [x] done (2026-09-11)

### T41 — Editors
- **Files:** `TransformLibraryEditor.{h,cpp}`, `TransformLibraryView.qml`,
  `DatasetTransformEditor.{h,cpp}`, `EditorForms.cpp`
- **Does:** `lua` property on the embedded editor; one view for both languages; dialog carries
  both libraries and its JS validate/test evaluate the JS library first; button + signal per
  language.
- **Verify:** `code-verify --check`.
- [x] done (2026-09-11)

### T42 — Project Scripts tree node
- **Files:** `EntityKinds.h`, `EditorTree.{h,cpp}`, `EditorSelection.{h,cpp}`,
  `EditorSummaries.{h,cpp}`, `ProjectEditor.{h,cpp}`, `ProjectEditor.qml`,
  `ProjectScriptsView.qml` (new), `app/CMakeLists.txt`
- **Does:** `KindScriptsRoot` + `KindJsLibrary` appended; root node with three children,
  persisted expansion, search filter; views + selection + nav; overview pane.
- **Verify:** `code-verify --check`; `registry-verify`.
- [x] done (2026-09-11)

### T43 — Docs + wrap-up
- **Files:** `Dataset-Transforms.md`, `Project-Editor.md`, `API-Reference.md`, `transforms.md`,
  `scripting.md`, `CLAUDE.md`, `spec.md`
- **Does:** document both libraries and the node; `claim-verify`, `documentation-verify`, search
  index; tick AC10 when the pytest is green on the maintainer's side.
- [x] done (2026-09-11; AC9-AC11 stay with the maintainer)

## Definition of Done

- [x] Every acceptance criterion in `spec.md` is met and checked off there.
- [x] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [x] `python3 scripts/generate-property-registry.py --check`, `generate-sdk.py --check`,
      `registry-verify.py`, `layer-verify.py`, `claim-verify.py` all clean.
- [x] `qt-cpp-review` run on the C++ diff; findings addressed or noted.
- [x] `--benchmark-hotpath` not regressed (Dataset struct grew; one maintainer run).
- [x] Relevant `pytest` / `ctest` targets identified for the maintainer to run (listed in
      `plan.md`); `tests/scripts/test_modbus_lua_parser.py` green locally.
- [x] `python scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [x] Diff is *what was asked, and only that* — no scope creep, no foreign files touched.
- [x] `spec.md` status set to `done`.
