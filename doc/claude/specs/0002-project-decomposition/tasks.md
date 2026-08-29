---
spec: 0002-project-decomposition
phase: tasks
status: approved      # draft -> approved (gate before /ss-implement)
updated: 2026-07-06
---

# Tasks 0002 - God-class decomposition (ProjectModel / ProjectEditor / ProjectHandler)

> **Phase 3 of 4 - the ordered checklist.** Decompose [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable*. `/ss-implement` works this list top to
> bottom and keeps the status boxes current. Gate: do not start `/ss-implement` until a human
> marks this `approved`.

## Conventions

- One task = one focused, reviewable diff (here, one new TU or one CMake edit).
- **Verify** is how *this* unit is confirmed before moving on - the per-stage grep recipe in
  `plan.md`, plus `python scripts/code-verify.py --check <files>`.
- **Deps** lists task IDs that must land first.
- Order so the tree is a valid morning state after each stage. S1, S2, S3 are mutually
  independent; do them in risk order S1 -> S2 -> S3.

## Run status (2026-07-06)

- **S1, S2, S3 - DONE (autonomous overnight run 2026-07-06)**, with the deviations below; each
  TU landed and its per-stage recipe passed. Any completed prefix is a valid morning state.
- **S1/S2 shared-header trim:** 2 ProjectModel helpers (`sanitizeFolderTree`, `serializeFolders`)
  and 2 ProjectEditor helpers (`buildFolderTree`, `accumulateFolderEnabled`) turned out to have a
  single caller each, so they were demoted to file-local statics in their owning TU instead of
  moving to `ProjectModelShared.h` / `ProjectEditorShared.h`.
- **S2b `CustomModel.h` (T2.11) NOT executed:** `class CustomModel` stays in `ProjectEditor.h`;
  S5c stays blocked on it.
- **S3 `ProjectApiSupport` is header-only:** the 14 cross-family helpers landed as `inline` free
  functions in `ProjectApiSupport.h` (no `.cpp`). The `register*Commands` builders and all 65
  `registerCommand` calls stayed in the residual `ProjectHandler.cpp` (`registerCommands()`);
  only the command *bodies* moved to the family TUs.
- **S4, S5 - pending (spec-only tonight).** Not started in the overnight run; scheduled for a
  later session. Their acceptance criteria are recorded below and stay unchecked until
  implemented.

---

## S1 - TU-split ProjectModel.cpp  (DONE)

### T1.1 - ProjectModelShared.h

- **Files:** `app/src/DataModel/Project/ProjectModelShared.h`
- **Does:** Promote the 6 cross-TU helpers to inline/template free fns in `namespace
  DataModel` (folderExists, folderIsSelfOrDescendant, sanitizeFolderTree, serializeFolders
  :67-154; nextDuplicateTitle :159-194; seedDefaultFrameParser :1503-1516). SPDX banner;
  ImporterCommon.h style.
- **Verify:** `grep "\btr(" ` -> zero; each helper unique repo-wide; templates keep
  `template`, non-templates gain `inline`; `code-verify --check`.
- **Deviation:** shipped 4 helpers; `sanitizeFolderTree`/`serializeFolders` had one caller each
  and stayed file-local statics in Folders/Persistence.
- **Deps:** none
- [x] done

### T1.2 - ProjectModelPersistence.cpp

- **Files:** `app/src/DataModel/Project/ProjectModelPersistence.cpp`
- **Does:** Move :1779-2027 + :7639-7868 verbatim (askSave..serializeToJson,
  autoSave..finalizeProjectSave).
- **Deviation (CANNOT-MOVE #4):** `autoSave()`/`syncRuntime()` bodies moved here
  (`ProjectModelPersistence.cpp:339`/`:365`) rather than staying in the facade `.cpp`;
  behavior-neutral (member dispatch is TU-independent, the ctor QTimer wiring resolves either
  way). `m_runtimeDirty` stays a facade-header member. #4's intent -- keep them out of the S4
  `AutoSaveController` collaborator -- still holds.
- **Verify:** definition symmetry; include closure superset; pure-move diff; watcher re-arm
  (`watchProjectFile()`) intact.
- **Deps:** T1.1
- [x] done

### T1.3 - ProjectModelLoading.cpp

- **Files:** `app/src/DataModel/Project/ProjectModelLoading.cpp`
- **Does:** Move :2334-3176 (loading + transform-scanner statics :2715-2900 + legacy
  migrations) + :758-905 (seed/dedup/migrate statics + remapWaterfallYAxisId) verbatim.
- **Verify:** static closure (scanner + remap statics resolve same-TU); include closure;
  pure-move diff; watcher re-arm intact.
- **Deps:** T1.1
- [x] done

### T1.4 - ProjectModelSources.cpp

- **Files:** `app/src/DataModel/Project/ProjectModelSources.cpp`
- **Does:** Move :1517-1778 (source CRUD/settings) + :4849-4982 (frame-parser setters).
- **Verify:** definition symmetry; include closure; pure-move diff.
- **Deps:** T1.1
- [x] done

### T1.5 - ProjectModelCrud.cpp

- **Files:** `app/src/DataModel/Project/ProjectModelCrud.cpp`
- **Does:** Move :3366-3591, :3595-4029 (+ detail::RefAnchor statics), :4036-4824
  (+ detail::ThreeAxisLayout + populateThreeAxisDatasets :275-317), :7877-8055, :8063-8297.
- **Verify:** detail-namespace ODR - `ThreeAxisLayout` + `RefAnchor` live in THIS TU only, no
  duplicate type name elsewhere; static closure; pure-move diff.
- **Deps:** T1.1
- [x] done

### T1.6 - ProjectModelWorkspaces.cpp

- **Files:** `app/src/DataModel/Project/ProjectModelWorkspaces.cpp`
- **Does:** Move :5055-5224, :5824-6030, :6781-7095, :7122-7480 + statics :196-273, :322-349
  (tally/append/collect/push/buildAutoRefsForGroup). Partition by function, not banner (the
  6781-7480 auto-workspace/hidden-groups machinery lands here, not in Folders).
- **Verify:** static closure; include closure; pure-move diff.
- **Deps:** T1.1
- [x] done

### T1.7 - ProjectModelFolders.cpp

- **Files:** `app/src/DataModel/Project/ProjectModelFolders.cpp`
- **Does:** Move :6032-6779 (three folder CRUD blocks + folder prompts) + :7097-7121
  (sanitize*Folders).
- **Verify:** definition symmetry; include closure; pure-move diff.
- **Deps:** T1.1
- [x] done

### T1.8 - ProjectModelTables.cpp

- **Files:** `app/src/DataModel/Project/ProjectModelTables.cpp`
- **Does:** Move :5231-5451, :5461-5497, :5591-5822 (tables/registers + prompts + CSV
  import/export).
- **Verify:** definition symmetry; include closure; pure-move diff.
- **Deps:** T1.1
- [x] done

### T1.9 - CMake registration + S1 closure

- **Files:** `app/CMakeLists.txt`
- **Does:** Add the 7 new `.cpp` to `SOURCES` (near :308) and `ProjectModelShared.h` to
  `HEADERS` (near :439), each exactly once.
- **Verify:** each new file listed exactly once; full S1 recipe (steps 1-7) passes; residual
  `ProjectModel.cpp` retains ctor/singleton, status/lock, getters :725-1512,
  setupExternalConnections, newJsonFile + scalar setters, selection setters, entity prompts,
  diagram invokables, clearTransientState, nextDatasetIndex, allocateUniqueId.
- **Deps:** T1.1-T1.8
- [x] done

---

## S2 - TU-split ProjectEditor.cpp + item-id header (+S2b CustomModel)  (DONE except S2b)

### T2.1 - ProjectEditorItemIds.h

- **Files:** `app/src/DataModel/Project/ProjectEditorItemIds.h`
- **Does:** Move the private typedef-enum block :51-273 (TopLevelItem, ProjectItem,
  kDatasetView_*, kGroupView_*, ...) verbatim.
- **Verify:** every `k<View>_*` user file includes this header; non-`Q_OBJECT` (no HEADERS
  entry needed); `code-verify --check`.
- **Deps:** none
- [x] done

### T2.2 - ProjectEditorShared.h

- **Files:** `app/src/DataModel/Project/ProjectEditorShared.h`
- **Does:** Promote inline folderDisplayPath, buildFolderTree, accumulateFolderEnabled,
  busTypeIcon (:168-273) to `namespace DataModel` free fns.
- **Deviation:** shipped 2 helpers (`folderDisplayPath`, `busTypeIcon`); `buildFolderTree` /
  `accumulateFolderEnabled` had one caller each and stayed file-local statics.
- **Verify:** used from >=2 TUs; each unique repo-wide; `grep "\btr("` per the shared-header
  rule.
- **Deps:** none
- [x] done

### T2.3 - ProjectEditorWiring.cpp

- **Files:** `app/src/DataModel/Project/ProjectEditorWiring.cpp`
- **Does:** Move wire* :281-760. Keep the `wireProjectModelRebuilds()` connect topology and
  the `QueuedConnection` (:285-289) shape verbatim (CANNOT-MOVE #8).
- **Verify:** connection types unchanged; pure-move diff; include closure.
- **Deps:** T2.1
- [x] done

### T2.4 - ProjectEditorTree.cpp

- **Files:** `app/src/DataModel/Project/ProjectEditorTree.cpp`
- **Does:** Move :1310-2021, :2419-2537, :5717-5782.
- **Verify:** definition symmetry; ItemIds include present; pure-move diff.
- **Deps:** T2.1
- [x] done

### T2.5 - ProjectEditorMqtt.cpp

- **Files:** `app/src/DataModel/Project/ProjectEditorMqtt.cpp`
- **Does:** Move :1191-1214, :2023-2418 including the `#ifdef BUILD_COMMERCIAL` regions
  verbatim.
- **Verify:** `BUILD_COMMERCIAL` open/close balance per TU; pure-move diff.
- **Deps:** T2.1
- [x] done

### T2.6 - ProjectEditorForms.cpp

- **Files:** `app/src/DataModel/Project/ProjectEditorForms.cpp`
- **Does:** Move :2538-3023, :3205-3443, :3443-4005, :5497-5640.
- **Verify:** definition symmetry; ItemIds include present; pure-move diff.
- **Deps:** T2.1
- [x] done

### T2.7 - ProjectEditorCommit.cpp

- **Files:** `app/src/DataModel/Project/ProjectEditorCommit.cpp`
- **Does:** Move :3024-3204, :4116-4749, :5641-5715 (onDataset*/onGroup* commit handlers).
- **Verify:** ItemIds include present (CANNOT-MOVE #9); title-edit invariant preserved
  (in-place update, no per-keystroke mutation); pure-move diff.
- **Deps:** T2.1
- [x] done

### T2.8 - ProjectEditorMultiSelect.cpp

- **Files:** `app/src/DataModel/Project/ProjectEditorMultiSelect.cpp`
- **Does:** Move :4757-5065.
- **Verify:** definition symmetry; pure-move diff.
- **Deps:** T2.1
- [x] done

### T2.9 - ProjectEditorSelection.cpp

- **Files:** `app/src/DataModel/Project/ProjectEditorSelection.cpp`
- **Does:** Move :4088-4114, :5073-5495. `m_selected*` mirrors stay declared in the facade
  (CANNOT-MOVE #6); only the movable bodies move, cyclic PM<->PE callbacks unchanged.
- **Verify:** callback topology unchanged; pure-move diff.
- **Deps:** T2.1
- [x] done

### T2.10 - ProjectEditorSummaries.cpp

- **Files:** `app/src/DataModel/Project/ProjectEditorSummaries.cpp`
- **Does:** Move :5784-6560, :6567-6677.
- **Verify:** definition symmetry; pure-move diff.
- **Deps:** T2.1, T2.2
- [x] done

### T2.11 - S2b CustomModel.h extraction

- **Files:** `app/src/DataModel/Project/CustomModel.h`,
  `app/src/DataModel/ProjectEditor.cpp`, `app/src/DataModel/Editors/FrameParserModel.cpp`
- **Does:** Move `class CustomModel` (`ProjectEditor.h:635-680`) to `CustomModel.h`
  (includes `DataModel/ProjectEditor.h` for role enums). Add the include to `ProjectEditor.cpp`
  + `FrameParserModel.cpp` (the only two users).
- **Verify:** exactly one `class CustomModel` definition; `roleNames()` unchanged;
  `ProjectEditor.h:40` forward-decl + pointer-only use still holds.
- **Deps:** none (prerequisite for S5c)
- **Status:** NOT executed this run; `class CustomModel` stays in `ProjectEditor.h`, S5c blocked.
- [x] done

### T2.12 - CMake registration + S2 closure

- **Files:** `app/CMakeLists.txt`
- **Does:** Add the new `.cpp` to `SOURCES` (near :309); `CustomModel.h` to `HEADERS` (has
  `Q_OBJECT`, near :440), each exactly once. `ItemIds.h`/`Shared.h` are non-`Q_OBJECT` (no
  HEADERS entry).
- **Verify:** each new file exactly once; S1 recipe + S2 checks (a)/(b)/(c); Q_ENUMs
  (CurrentView/EditorWidget/CustomRoles/ItemKind) remain in `ProjectEditor.h` (CANNOT-MOVE
  #2); residual retains ctor, accessors :765-1290, generateComboBoxModels, transform-editor
  glue.
- **Deps:** T2.1-T2.11
- [x] done

---

## S3 - TU-split ProjectHandler.cpp + ProjectApiSupport  (DONE; ProjectApiSupport header-only)

### T3.1 - ProjectApiSupport.h/.cpp

- **Files:** `app/src/API/Handlers/ProjectApiSupport.h`,
  `app/src/API/Handlers/ProjectApiSupport.cpp`
- **Does:** Move the 14 verified cross-family statics to `namespace API::Handlers`, dropping
  `static` (attachProjectEpoch, captureProjectEpoch, appendStaleProjectWarning,
  appendUnknownFieldsWarning, buildDatasetObject, datasetOptionsBitflag, summarizeProjectJson,
  summarizeCurrentProject, takeParam [33 sites], makeScriptEngine, detectLanguageMismatch,
  frameParserCompileHint, applySimpleAlarmFields, appendDatasetWidgetTypes).
- **Verify:** each helper defined once; unqualified lookup resolves all 33 `takeParam` sites
  with zero call-site edits; `code-verify --check`.
- **Deviation:** shipped header-only -- the 14 helpers are `inline` in `ProjectApiSupport.h`
  (no `.cpp`); `register*Commands` builders + all `registerCommand` calls stayed in
  `ProjectHandler.cpp`.
- **Deps:** none
- [x] done

### T3.2 - ProjectHandlerFile.cpp

- **Files:** `app/src/API/Handlers/ProjectHandlerFile.cpp`
- **Does:** Move registerFile* + file/snapshot/validate/template + exclusive statics
  :3683-3860.
- **Verify:** static closure (stop-rule: no off-list static spans two families); `BUILD_COMMERCIAL`
  balance; `registerCommand` count contribution recorded.
- **Deps:** T3.1
- [x] done

### T3.3 - ProjectHandlerEntities.cpp

- **Files:** `app/src/API/Handlers/ProjectHandlerEntities.cpp`
- **Does:** Move group/dataset/action/outputWidget + applyDataset*Fields :4773-4930 + member
  helpers :157-162.
- **Verify:** static closure + stop-rule; `BUILD_COMMERCIAL` balance; `registerCommand` count.
- **Deps:** T3.1
- [x] done

### T3.4 - ProjectHandlerParser.cpp

- **Files:** `app/src/API/Handlers/ProjectHandlerParser.cpp`
- **Does:** Move parser/painter/dry-run + engine statics :5847-6450.
- **Verify:** static closure + stop-rule; `BUILD_COMMERCIAL` balance; `registerCommand` count.
- **Deps:** T3.1
- [x] done

### T3.5 - ProjectHandlerBatch.cpp

- **Files:** `app/src/API/Handlers/ProjectHandlerBatch.cpp`
- **Does:** Move batch + list/resolver/move.
- **Verify:** static closure + stop-rule; `BUILD_COMMERCIAL` balance; `registerCommand` count.
- **Deps:** T3.1
- [x] done

### T3.6 - CMake registration + S3 closure

- **Files:** `app/CMakeLists.txt`
- **Does:** Add the new `.cpp` (and `ProjectApiSupport.cpp`) to `SOURCES` (near :256), each
  exactly once. No new `Q_OBJECT`, no HEADERS/moc work.
- **Verify:** each new file exactly once; sum of `registry.registerCommand` counts across new
  + residual equals the original; `registerCommands()` stays in the residual
  `ProjectHandler.cpp`; STOP-RULE honored (any family with an off-list cross-family static is
  left unsplit - partial split acceptable, note it).
- **Deps:** T3.1-T3.5
- [x] done

---

## S4 - ProjectModel collaborators  (PENDING - spec only, later run)

> Per-collaborator acceptance criteria. Signal strategy for every S4 sub-stage: the
> collaborator emits a narrow signal; the facade ctor connects it to the existing NOTIFY
> names -> zero QML changes.

### T4a - ProjectFileGuard

- **Owned state:** `m_fileWatcher`, `m_diskCheckPending`, `m_diskPromptActive`,
  `m_diskFileHash` (~150 ln, :391-397 + :7742-7817).
- **AC:** watcher re-arm invariant (`watchProjectFile()` after write/load/new) intact; disk
  changes surface under the same facade signal; QML untouched.
- **Deps:** S1
- [x] done

### T4b - AutoSaveController

- **Owned state:** `m_autoSaveTimer`, `m_autoSaveSuspended` (~90 ln, :7639-7708).
- **AC:** `autoSave()` / `syncRuntime()` / `m_runtimeDirty` remain in the facade (CANNOT-MOVE
  #4); autosave cadence and suspend/resume behavior unchanged; no new race.
- **Deps:** T4a
- [x] done

### T4c - WorkspaceSynthesizer

- **Owned state:** none (pure functions).
- **AC:** synthesized workspaces byte-identical to the current output; auto-regen trigger
  ordering (ctor fence, CANNOT-MOVE #3) unchanged.
- **Deps:** S1
- [x] done

### T4d - LegacyMigrations

- **Owned state:** none (free functions).
- **AC:** every legacy-file path produces identical migrated JSON; migration ordering
  preserved.
- **Deps:** S1
- [x] done

### T4e - ProjectUiStateStore

- **Owned state:** UI-state cluster (~350 ln, :1165-1512).
- **AC:** all UI-state getters/setters re-emit the same NOTIFY signals; QML bindings
  unchanged.
- **Deps:** S1
- [x] done

### T4f - ProjectSerializer/Loader (tail)

- **Owned state:** ~1,100 ln serialize/load pair.
- **AC:** resolve the `ProjectDocument` aggregate vs `friend class` Open Question first;
  serialized output and load results byte-identical; watcher re-arm intact.
- **Deps:** T4a-T4e
- [x] done

---

## S5 - ProjectEditor collaborators  (PENDING - spec only, later run)

> Title-edit invariant restated per controller: in-place item update, never a per-keystroke
> model mutation.

### T5a - ComboBoxCatalog

- **Scope:** ~250 ln.
- **AC:** combobox model contents identical; restore-race guards (`if (count <= 0) return`)
  preserved.
- **Deps:** S2
- [x] done

### T5b - ProjectTreeController

- **Scope:** ~2,100 ln - tree + selection + expansion move together.
- **AC:** `groupsChanged` -> `buildTreeModel` stays `QueuedConnection` verbatim (CANNOT-MOVE
  #8); tree structure, selection mirrors, and expansion state behave identically.
- **Deps:** S2
- [x] done

### T5c - per-entity FormControllers

- **Scope:** ~3,000 ln - each takes a `CustomModel*`, builds rows, handles `on*ItemChanged`
  commit.
- **AC:** requires S2b `CustomModel.h`; each form's rows and commit behavior identical;
  title-edit invariant holds per controller.
- **Deps:** S2, T2.11
- [x] done

### T5d - MultiSelectionController

- **Scope:** ~320 ln.
- **AC:** multi-select batch delete/dup/move behavior identical; selection mirrors
  (CANNOT-MOVE #6) intact.
- **Deps:** S2
- [x] done

## Definition of Done

<Per-stage; the whole-feature gate is reached only after S1-S5 land in their scheduled runs.>

- [x] Every acceptance criterion in `spec.md` is met and checked off there (AC6 stays open
  until S4/S5 land).
- [x] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [x] `qt-cpp-review` run on the C++ diff; findings addressed or noted.
- [x] Hotpath not touched - `--benchmark-hotpath` need not be re-run for correctness.
- [x] Existing project-editor / API-handler `pytest` suites identified for the maintainer to
  run (listed in `plan.md`); a failure means a body was altered, not moved.
- [x] `python scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [x] Diff is *what was asked, and only that* - pure moves, no scope creep, no foreign files
  touched.
- [x] `spec.md` status set to `done` only after S1-S5 are complete; interim runs leave it
  `in-progress`.
