---
spec: 0031-project-undo-redo
phase: tasks
status: approved     # pre-approved by maintainer 2026-07-24 ("tasks and then implement")
updated: 2026-07-24
---

# Tasks 0031 — Transactional undo/redo for project editing

> **Phase 3 of 4 — the ordered checklist.** Decompose [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable*. `/ss-implement` works this list top to
> bottom and keeps the status boxes current.

## Conventions

- One task = one focused, reviewable change.
- **Verify** is how *this* unit is confirmed before moving on.
- **Deps** lists task IDs that must land first.

## Tasks

### T1 — ProjectHistory core class

- **Files:** `app/src/DataModel/Project/ProjectHistory.h` (new),
  `app/src/DataModel/Project/ProjectHistory.cpp` (new)
- **Does:** Step stack {label, coalesceKey, preState, postState, timestamp}, frame stack
  (label frames vs capture frames, depth counter), coalescing rule (same key + < 1 s + no
  intervening undo → skip capture), bounds (100 steps / 64 MiB, oldest dropped, save-position
  sentinel), save-position tracking, clear(). No QObject, no singleton calls — plain class,
  callback into owner for serialize/apply (ctor-closure safety: constructible before any
  other singleton exists). Register both TUs in the app CMake source list.
- **Verify:** `python scripts/code-verify.py --check` on both files; read-back against plan
  "Capture"/"Undo/redo" sections.
- **Deps:** none
- [x] done

### T2 — ProjectModel facade: properties, slots, scope plumbing

- **Files:** `app/src/DataModel/ProjectModel.h`, `app/src/DataModel/ProjectModel.cpp`
- **Does:** `Q_PROPERTY` `canUndo`/`canRedo`/`undoText`/`redoText` NOTIFY
  `projectHistoryChanged`; `public slots:` `undo()`, `redo()`, `setNextUndoHint()`;
  `m_history` member (ctor init list, no in-header init); scope RAII helper visible to the
  Project TUs (in `ProjectModelShared.h` or ProjectModel.h). Capture gated on
  `m_initialized` **and** not-applying. **Binding invariants: ProjectModel ctor closure is
  protected (spec 0001) — history ctor must not touch other singletons; `newJsonFile()` runs
  inside the closure, so gate on `m_initialized`; header order per style (Q_PROPERTY →
  signals → ctor → public → slots → private).** History clear in `newJsonFile()` and
  `lockProject`/`unlockProject`.
- **Verify:** `code-verify.py --check`; confirm no `instance()` call reachable from the
  ctor closure via read-back.
- **Deps:** T1
- [x] done

### T3 — Apply path: refactor loadFromJsonDocument, implement undo/redo apply

- **Files:** `app/src/DataModel/Project/ProjectModelLoading.cpp`,
  `app/src/DataModel/Project/ProjectModelPersistence.cpp`
- **Does:** Extract `applyJsonDocumentCore()` shared by `loadFromJsonDocument` (unchanged
  external behavior) and the new history-apply entry: keeps `m_filePath`/watcher, runs
  sanitizers (`enforceGplSingleSource`, transform/virtual resolution, uniqueId seeding),
  emits loaded-signal set **minus `jsonFileChanged`**, then `syncRuntime()` +
  `scheduleAutoSave()`; `applying` flag suppresses re-entrant capture (auto-workspace
  regen lambda on `groupsChanged` mutates). History cleared in `loadFromJsonDocument`;
  `finalizeProjectSave()` records save position. **Binding invariants: never emit
  `jsonFileChanged` from history apply (selection reset + AppState reload); watcher re-arm
  stays owned by `writeProjectFile`/load paths; `setModified(false)` semantics on empty
  docs unchanged.**
- **Verify:** `code-verify.py --check`; read-back diff of `loadFromJsonDocument` — external
  callers see identical behavior (same signals, same order).
- **Deps:** T2
- [x] done

### T4 — Undo scopes: ProjectModel.cpp + Crud TU

- **Files:** `app/src/DataModel/ProjectModel.cpp`,
  `app/src/DataModel/Project/ProjectModelCrud.cpp`
- **Does:** One scope line at the top of every mutating slot (~24 + ~32 sites), label =
  human-readable operation ("Delete Group", "Add Dataset", ...). Nested mutators join the
  outer scope via depth counter. Prompt wrappers and transient-selection setters get no
  scope (they don't mutate the document).
- **Verify:** `grep -c` scope lines vs the plan's site counts; `code-verify.py --check`.
- **Deps:** T2
- [x] done

### T5 — Undo scopes: Sources, Tables, Workspaces, Folders TUs

- **Files:** `app/src/DataModel/Project/ProjectModelSources.cpp`,
  `app/src/DataModel/Project/ProjectModelTables.cpp`,
  `app/src/DataModel/Project/ProjectModelWorkspaces.cpp`,
  `app/src/DataModel/Project/ProjectModelFolders.cpp`
- **Does:** Same mechanical scope line per mutating slot (~16/8/17/21 sites). Load-time
  silent mutators (`seedNextUniqueIdFromGroups`, migrations, `sanitize*Folders`) get no
  scope — they run under load, which clears history anyway.
- **Verify:** `grep -c` scope lines vs plan counts; `code-verify.py --check`.
- **Deps:** T2
- [x] done

### T6 — Editor hints and bulk frames

- **Files:** `app/src/DataModel/Project/ProjectEditorCommit.cpp`,
  `app/src/DataModel/Project/ProjectEditorMultiSelect.cpp`
- **Does:** `setNextUndoHint(label, key)` immediately before each per-keystroke commit
  (~8 handlers; key = `"<entity>-<field>:<id>"`); label frames around multi-select fan-out
  loops (~4 sites, alongside the existing `setAutoSaveSuspended` pairs) so N-item bulk
  edits are one step. **Binding invariant: per-keystroke commits call
  `updateGroup`/`updateDataset` with `rebuildTree=false` — hints must not change that
  path.**
- **Verify:** `code-verify.py --check`; read-back: every `onTextEdited`-driven commit
  handler sets a hint.
- **Deps:** T4
- [x] done

### T7 — API label frames + project.undo/redo verbs

- **Files:** `app/src/API/CommandRegistry.cpp`,
  `app/src/API/Handlers/ProjectHandler.cpp`,
  `app/src/API/Handlers/ProjectHandlerFile.cpp`
- **Does:** `execute()` opens a label frame named by the command (next to
  `ExecuteDepthGuard`, so batch inner ops nest); register `project.undo`/`project.redo`
  (`emptySchema()`), impls return `{performed, undone|redone}` or
  `{performed:false, reason}` (success, never error, on empty history) +
  `attachProjectEpoch`; both verbs added to `destructiveCommandSet()`.
- **Verify:** `code-verify.py --check`; read-back: frame opens before the pre-mutation
  backup snapshot logic misfires? — frame must not capture (label only), snapshot order
  unchanged.
- **Deps:** T3, T4
- [x] done

### T8 — StandardKey tables + command manifests

- **Files:** `app/src/UI/CommandRegistry.cpp`, `scripts/registry-verify.py`,
  `app/rcc/commands/projecteditor.json`,
  `app/rcc/commands/layouts/project-toolbar.json`
- **Does:** Add `Undo`/`Redo` to `kStandardKeys` and `KNOWN_STANDARD_KEYS`; manifest
  entries `editor.undo`/`editor.redo` (kind action, category project, contexts
  `["editor"]`, `shortcut: StandardKey.Undo/Redo`, `shortcutWindows: ["editor"]`, icons
  `code/undo`/`code/redo`); two toolbar nodes in the file-ops section.
- **Verify:** `python scripts/registry-verify.py` clean (add 16-tier icon copies only if
  its render-size lint fires); `python scripts/generate-command-strings.py --check`.
- **Deps:** none (parallel-safe; QML binding lands in T9)
- [x] done

### T9 — QML bindings

- **Files:** `app/qml/Commands/ProjectEditorCommandBindings.qml`
- **Does:** `cmdEditorUndo`/`cmdEditorRedo` QtObjects: `run()` →
  `Cpp_JSON_ProjectModel.undo()`/`redo()`; `enabled: Cpp_JSON_ProjectModel.canUndo`/
  `canRedo`; dynamic tooltip `qsTr("Undo: %1").arg(Cpp_JSON_ProjectModel.undoText)`
  (numbered placeholders only — never `%n` with `.arg()`); map entries for both ids.
  **Binding invariant: shortcut Instantiator already gates `editorInteractive` — do not
  duplicate mode/lock checks in `enabled`.**
- **Verify:** `code-verify.py --check`; `registry-verify.py` (binding/manifest join).
- **Deps:** T2, T8
- [x] done

### T10 — code-verify drift rule

- **Files:** `scripts/code-verify.py`
- **Does:** New rule: in the ProjectModel TUs, a function body containing
  `setModified(true)` must open an undo scope (or carry an explicit suppression) — error
  level, scoped to `app/src/DataModel/Project/ProjectModel*.cpp` + `ProjectModel.cpp`.
  Whitelist the known non-step mutators (load-time migrations, `resolveDiskFileChange`
  direct-flag writes).
- **Verify:** `python scripts/code-verify.py --check` over the Project TUs → zero errors
  after T4/T5; seed a temp violation locally to confirm the rule fires, then revert it.
- **Deps:** T4, T5
- [x] done

### T11 — pytest coverage

- **Files:** `tests/integration/test_project_undo.py` (new)
- **Does:** AC1 randomized 50-mutation round-trip (byte-compare `project.exportJson`);
  AC2 group-with-datasets delete/undo fidelity (order, uniqueIds, fields); AC5 batch +
  single undo, empty-history `{performed:false}`; AC6 save-point modified-flag semantics
  via `project.getStatus`. Follow `tests/utils/api_client.py` conventions + markers from
  `tests/README.md` (read before writing).
- **Verify:** `python -m py_compile`; pytest collection (`pytest --collect-only`) locally;
  execution is maintainer-run (needs live app).
- **Deps:** T7
- [x] done

### T12 — Docs + self-review

- **Files:** `doc/claude/architecture/project.md`, `doc/claude/specs/0031-*/tasks.md`
- **Does:** "Undo history" section in project.md (capture model, boundaries, the
  workspace-revert quirk, the drift rule); counterfactual check at handoff: name the rule
  this diff most risks violating + evidence it doesn't; re-read full diff for scope.
- **Verify:** `python scripts/documentation-verify.py` not applicable (doc/claude exempt);
  read-back.
- **Deps:** all
- [x] done

## Build deviations (recorded during /ss-implement, 2026-07-25)

- **Two-phase capture.** Scopes *stage* the snapshot; `setModified(true)` *commits* it.
  Closes the junk-step hole (guard-returning setters, canceled prompts) the plan's
  capture-at-entry design had. `ProjectHistory::stageCapture()/commitPending()`.
- **T5 scope counts.** Workspace CRUD, workspace-folder family, `hideGroup`/`showGroup`,
  and presentation-blob setters got NO scopes — the spec keeps them outside undo history,
  and unscoped presentation writes also protect the redo tail from the queued
  `buildTreeModel` → `setTreeExpansion` write-back. Enforced via the
  `undo-scope-missing` whitelist. Actual scope lines: 93 (13 ProjectModel, 42 Crud,
  19 Sources, 7 Tables, 12 Folders).
- **Apply path resync.** No direct `syncRuntime()`: `frameDetectionChanged` →
  `AppState::onProjectLoaded` already rebuilds FrameBuilder and re-derives frameConfig
  (which a plain `syncRuntime()` would have missed).
- **Save points.** `autoSave()` also calls `markSaved()` (disk truth, not just explicit
  saves), so the modified flag always reflects document-vs-disk.
- **Extra files touched beyond plan list:** `app/CMakeLists.txt` (TU registration),
  `app/src/UI/CommandStrings.cpp` (regenerated), `tests/README.md` (test catalog row),
  `app/src/DataModel/ProjectEditor.cpp` (two coalesce hints: painter code, transmit
  function — per-keystroke commit paths the plan missed).

## qt-cpp-review round (6 agents, 2026-07-25) — fixes applied

- **Save-point staleness (confirmed 88):** redo-tail truncation in `commitPending()` now
  invalidates a save position inside the discarded tail; `markSaved()` breaks the top
  step's coalesce chain so a save inside a keystroke burst stays byte-accurate.
- **Coalesce-after-redo data loss (confirmed 85):** the coalesce branch clears a stale
  `postState` (and its byte accounting) so redo re-materializes the true end state.
- **Position-before-apply desync:** `stateForUndo/Redo` replaced with
  `peekUndoState/peekRedoState` + `confirmUndo/confirmRedo`; `undo()`/`redo()` return
  bool, move position only after a clean apply, and the API handlers report
  `performed:false` on apply failure.
- **Code-editor coalescing gap (confirmed 92):** `ProjectUndoScope` gained an optional
  slot-level coalesce key; `setControlScriptCode`/`setFrameParserCode`/
  `storeFrameParserCode`/`updateSourceFrameParser` self-coalesce, painter/transmit
  editors hint via `ProjectEditor.cpp` — typing no longer serializes per character or
  floods history.
- **No-op capture cost:** equality/bounds guards hoisted above the scope in the 13
  scalar setters and the parser-code slots.
- **Dialog reentrancy:** undo scopes moved below the confirmation dialogs in
  `deleteCurrent{Group,Action,Dataset,OutputWidget}`, `deleteSource`,
  `importTableFromCsv`; the `deleteGroup/deleteDataset/deleteAction/deleteOutputWidget`
  wrappers lost their redundant outer scopes (inner slot scopes carry the step).
  `setGroupWidget` keeps its top scope on purpose: `confirmGroupWidgetChange` mutates the
  live group around its dialog, so pre-capture must precede it.
- **API double-rebuild:** `execute()`'s epoch hook skips `project.undo`/`project.redo`
  (apply already syncs + schedules autosave) — no second pool invalidation.
- **Misc:** dead `m_stackRecorded` flag removed; `[[nodiscard]]` on `leave()`;
  `widgetSettingsChanged` emitted on apply when restored settings are empty (guard in
  `emitProjectLoadedSignals` would have silenced consumers); byte cap documented as
  deliberately soft (~2x transient with a full redo tail).
- **Accepted / deferred:** whole-doc snapshots reverting presentation state = documented
  spec quirk; no-op `update*` calls can still record empty steps (needs entity
  `operator==`, deferred); script `apiCall("project.undo")` mid-parse shares the
  pre-existing synchronous-teardown hazard class with `project.open` (fix belongs in
  ScriptApiCall deferral, out of this spec's lane — flagged to maintainer).

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` is met or handed to the maintainer as a named
      runtime check (AC3/AC4/AC7 observations; AC1/AC2/AC5/AC6 pytest).
- [ ] `python scripts/code-verify.py --check` clean on all changed files (no new errors).
- [ ] `qt-cpp-review` run on the C++ diff; findings addressed or noted.
- [ ] `--benchmark-hotpath` identified for maintainer (no hotpath edits; adjacency gate).
- [ ] `pytest tests/integration/test_project_undo.py` listed for the maintainer.
- [ ] `python scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [ ] Diff is *what was asked, and only that* — no foreign files touched.
- [x] `spec.md` status set to `done`.
