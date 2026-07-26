---
spec: 0031-project-undo-redo
phase: plan
status: approved     # draft -> approved (gate before /ss-tasks)
updated: 2026-07-24
---

# Plan 0031 — Transactional undo/redo for project editing

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

Scoped whole-document mementos. A new `DataModel::ProjectHistory` (plain value member of
`ProjectModel`, no singleton dependencies) holds a bounded stack of compact-JSON document
snapshots. Every mutating `ProjectModel` public slot opens a depth-counted RAII undo scope;
only the outermost scope captures `serializeToJson()` as the step's pre-state, so composite
operations (group delete with cascades, `project.batch`, multi-select fan-outs) are atomic by
construction. (Build amendment: capture is two-phase — the scope *stages* the snapshot and
the slot's `setModified(true)` *commits* it, so guard-returning no-op calls never record
steps.) `API::CommandRegistry::execute()` opens a *label frame* (name only, no capture)
so an API step is named after its command and a 1024-op batch is one step. Keystroke bursts
coalesce via editor-provided hint keys — a coalesced burst skips capture entirely, so typing
costs no serialization. Undo lazily serializes the current state as the step's redo state,
then applies the pre-state through a refactored `loadFromJsonDocument` core that keeps the
file path/watcher, skips `jsonFileChanged` (no selection reset, no AppState reload path), and
calls `syncRuntime()` directly. Two rejected alternatives: per-operation inverse commands
(119 hand-written inverses over lossy positional-ID remaps) and signal-driven capture
(signals fire post-mutation, forcing a full serialize per mutating event turn).

## Affected subsystems & files

| File | Change |
|------|--------|
| `app/src/DataModel/Project/ProjectHistory.h` (new) | `ProjectHistory` class: step stack {label, coalesceKey, preState bytes, postState bytes, timestamp}, frame stack for scopes, limits (100 steps / 64 MiB), save-position tracking |
| `app/src/DataModel/Project/ProjectHistory.cpp` (new) | Capture/coalesce/undo/redo/clear logic |
| `app/src/DataModel/ProjectModel.h` | `Q_PROPERTY` `canUndo`/`canRedo`/`undoText`/`redoText` (NOTIFY `projectHistoryChanged`), `public slots:` `undo()`/`redo()`, `setNextUndoHint()`, `m_history` member, scope helper declaration |
| `app/src/DataModel/ProjectModel.cpp` | ctor init, `undo()`/`redo()` impl, history clear in `newJsonFile()`, `lockProject`/`unlockProject` history boundary, scope lines on its ~24 mutating slots |
| `app/src/DataModel/Project/ProjectModelCrud.cpp` | Scope line per mutating slot (~32 sites) |
| `app/src/DataModel/Project/ProjectModelSources.cpp` | Scope lines (~16 sites) |
| `app/src/DataModel/Project/ProjectModelTables.cpp` | Scope lines (~8 sites) |
| `app/src/DataModel/Project/ProjectModelWorkspaces.cpp` | (Build amendment: NO scopes — workspace CRUD is outside undo history per the spec non-goal; also avoids re-entrant capture from the auto-workspace regen listeners) |
| `app/src/DataModel/Project/ProjectModelFolders.cpp` | Scope lines on group/table folder families only (workspace-folder family excluded, 12 sites) |
| `app/src/DataModel/Project/ProjectModelLoading.cpp` | Refactor `loadFromJsonDocument` into shared `applyJsonDocumentCore()` + history-apply entry; history clear on document load |
| `app/src/DataModel/Project/ProjectModelPersistence.cpp` | `finalizeProjectSave()` marks history save position |
| `app/src/DataModel/Project/ProjectEditorCommit.cpp` | `setNextUndoHint()` before per-keystroke commits (~8 handlers); scope frames on bulk commits |
| `app/src/DataModel/Project/ProjectEditorMultiSelect.cpp` | Label frames around fan-out loops (~4 sites, next to existing `setAutoSaveSuspended` pairs) |
| `app/src/API/CommandRegistry.cpp` | `execute()` opens a label frame named by the command (next to `ExecuteDepthGuard`) |
| `app/src/API/Handlers/ProjectHandler.cpp` | Register `project.undo` / `project.redo` |
| `app/src/API/Handlers/ProjectHandlerFile.cpp` | Handler impls: `{performed, undone/redone label}` or `{performed:false, reason}`; add both verbs to `destructiveCommandSet()` |
| `app/src/UI/CommandRegistry.cpp` | Extend `kStandardKeys` with `StandardKey.Undo` / `StandardKey.Redo` |
| `scripts/registry-verify.py` | Mirror `KNOWN_STANDARD_KEYS`; icon tier check |
| `app/rcc/commands/projecteditor.json` | `editor.undo` / `editor.redo` manifest entries (`shortcutWindows: ["editor"]`, `contexts: ["editor"]`, icons `code/undo`, `code/redo`) |
| `app/rcc/commands/layouts/project-toolbar.json` | Toolbar nodes for undo/redo in the file-ops section |
| `app/qml/Commands/ProjectEditorCommandBindings.qml` | `cmdEditorUndo`/`cmdEditorRedo`: `run()`, `enabled: Cpp_JSON_ProjectModel.canUndo/…`, dynamic tooltip `qsTr("Undo: %1").arg(...)` |
| `scripts/code-verify.py` | New rule: mutating slot (`setModified(true)` in body) in ProjectModel TUs must open an undo scope — drift gate for R1 |
| `tests/integration/test_project_undo.py` (new) | AC1/AC2/AC5/AC6 pytest coverage |
| `doc/claude/architecture/project.md` | New "Undo history" section (implement phase) |

## Architecture & data flow

**Capture.** `ProjectModel` gains a small RAII type (declared in `ProjectHistory.h`,
instantiated via one macro-or-inline line at the top of each mutating slot). Semantics:

- A depth counter tracks nesting. The **outermost** scope that is a *mutating slot scope*
  captures `serializeToJson()` → compact `QByteArray` before the slot body runs, and pushes
  a step `{label, coalesceKey, preState}`.
- `API::CommandRegistry::execute()` opens a **label frame** — name only (the command name),
  no capture. The first nested mutating slot performs the capture and the step takes the
  frame's name. A `project.batch` re-enters `execute()` per op at depth > 0; those inner
  frames don't rename the step, so the whole batch is one step labeled `project.batch`.
  Commands that mutate nothing (reads, dry runs) never trigger a capture — frames are free.
- `ProjectEditor` bulk operations (multi-select fan-outs, alarm-band commits for selection)
  open a label frame around their loops — same places that already pair
  `setAutoSaveSuspended(true/false)`.
- **Coalescing:** `ProjectEditor` per-keystroke commit handlers call
  `setNextUndoHint(label, key)` (e.g. key `"group-title:3"`) immediately before the model
  call. If the incoming step's key equals the top step's key, the top step is younger than
  1 s, and no undo/redo happened in between, the new capture is **skipped** — the burst
  extends the existing step. Steps without hints never coalesce (each API op is its own step).
- Capture is gated on `m_initialized` (the ctor runs `newJsonFile()` inside the protected
  ctor closure) and on an `applying` flag (see below).

**Undo/redo.** `undo()`: serialize current document → store as the step's `postState`,
apply the step's `preState`, decrement position. `redo()`: apply `postState`, increment.
Apply path = `applyJsonDocumentCore()`, extracted from `loadFromJsonDocument`, with a
history mode that:

- keeps `m_filePath`, does not touch the file watcher (disk unchanged; the next autosave
  re-arms it through `writeProjectFile` as today);
- runs the same sanitizers (`enforceGplSingleSource`, transform/virtual resolution,
  uniqueId seeding) — this is what keeps the GPL invariant and ID fidelity;
- emits the loaded-signal set **minus `jsonFileChanged`** (skipping the BackupManager
  per-undo snapshot and the editor path handlers), then `scheduleAutoSave()`; (build
  amendment: no direct `syncRuntime()` call — `frameDetectionChanged` is already wired to
  `AppState::onProjectLoaded`, which performs the FrameBuilder sync **and** the frameConfig
  re-derivation the plan originally missed);
- sets the `applying` flag so re-entrant mutations from signal listeners (the
  `groupsChanged` auto-workspace regeneration lambda) don't open scopes or capture;
- afterwards `ProjectHistory` sets the modified flag: `setModified(position != savePosition)`.

**Save point.** `finalizeProjectSave()` records the current history position. If the bound
drops steps past the save point, the saved position becomes unreachable (sentinel) and the
project stays modified until the next save — R6 exactly.

**History boundaries.** `loadFromJsonDocument` (open/loadJson/template/import/backup-restore)
and `newJsonFile` clear history. `lockProject`/`unlockProject` also clear it — an undoable
unlock would bypass the lock UX, and lock saves to disk immediately.

**Tree/UI refresh.** Apply emits `groupsChanged`/`actionsChanged`/… → existing queued
`scheduleTreeRebuild` coalesces to one `buildTreeModel()`; dashboard rebuild via the direct
`syncRuntime()`. On the API path, `execute()`'s epoch hook additionally schedules a
coalesced `scheduleProjectApply()` — redundant with the direct sync but idempotent and
queued; left as-is.

## Hotpath & threading impact

- **Touches the hotpath?** No. `FrameReader`/`CircularBuffer`/`FrameBuilder` parse paths
  and the Dashboard draw path are untouched. Undo apply calls the existing
  `syncFromProjectModel()` (already invoked on every project edit today), which
  invalidates the frame pool through the existing `invalidateFramePool()` route —
  `structureGeneration` semantics unchanged. `--benchmark-hotpath` run as a regression
  gate by the maintainer, expected delta zero.
- **New cross-thread signal/slot?** No. Everything (ProjectModel, ProjectEditor, API
  command execution, history) is main-thread. Capture and apply are synchronous calls.
- **New input to a cached hotpath flag?** No new flags, no new inputs. `m_changeDriven`,
  `m_streamAvailable`, etc. see undo as an ordinary project sync.
- **Timestamp ownership** — untouched; no frame data involved.

Cost note: one `serializeToJson()` per distinct user-level operation (not per keystroke —
coalesced bursts skip capture; not per batch op — one capture per batch). This is the same
serialization BackupManager already runs on its 5 s debounce; worst-case large projects pay
low single-digit ms per discrete edit, on the GUI thread, off the frame path.

## Data model & persistence

- **No new `Keys::` entries, no schema bump.** Snapshots reuse the existing
  `serializeToJson()` document format verbatim; history is RAM-only and never persisted.
- Snapshot fidelity: `serializeToJson` covers the whole document including `nextUniqueId`,
  so undo restores entity `uniqueId`s exactly (AC2). Project-level frame scalars round-trip
  through source 0, as in normal save/load.
- Known quirk (accepted, documented): snapshots are whole-document, so undoing a document
  step also reverts workspace/widget-settings changes made *after* that step's capture.
  Workspace edits themselves never create steps (spec non-goal), and the alternative —
  splicing live workspace state into restored snapshots — breaks referential integrity
  after structural undos (workspace refs pointing at re-added groups). Recorded in the
  tradeoffs table; revisit only if the spec's workspace non-goal is amended.

## API / SDK surface

- `project.undo`, `project.redo` — registered in `ProjectHandler::registerFileCommands`,
  implemented in `ProjectHandlerFile.cpp`, `emptySchema()`. Success responses:
  `{performed: true, undone: "<label>"}` / `{performed: false, reason: "nothing to undo"}`
  (never an error for empty history — spec R5/AC5), plus the standard `projectEpoch`
  attachment. Both verbs join `destructiveCommandSet()` so the existing pre-mutation
  BackupManager snapshot applies.
- Epoch: apply emits epoch-bumping signals, so `execute()`'s post-hook schedules
  autosave + runtime apply exactly as for any mutation.
- SDK regeneration via `sanitize-commit.py` (`generate-sdk` stage) picks the verbs up.

## QML / UI

- Manifest: `editor.undo` / `editor.redo` in `app/rcc/commands/projecteditor.json` —
  `kind: action`, `category: project`, `contexts: ["editor"]`,
  `shortcut: "StandardKey.Undo"` / `"StandardKey.Redo"`, `shortcutWindows: ["editor"]`,
  icons `code/undo` / `code/redo` (exist at tier 24; add 16-tier copies if
  `registry-verify.py`'s render-size lint fires).
- `kStandardKeys` (C++) + `KNOWN_STANDARD_KEYS` (Python) gain `Undo`/`Redo` —
  `QKeySequence::keyBindings` then yields platform-correct sets (Cmd+Z / Cmd+Shift+Z on
  macOS, Ctrl+Z / Ctrl+Y + Ctrl+Shift+Z on Windows) for free.
- Bindings in `ProjectEditorCommandBindings.qml`: `run()` → `Cpp_JSON_ProjectModel.undo()`;
  `enabled: Cpp_JSON_ProjectModel.canUndo` (shortcut Instantiator already gates on
  `editorInteractive`, so locked/non-ProjectFile states disable them); dynamic tooltip
  `qsTr("Undo: %1").arg(Cpp_JSON_ProjectModel.undoText)` — the registry supports
  per-binding tooltips but not dynamic titles, so R3's "show what will be undone" lands in
  the tooltip; static titles stay "Undo"/"Redo".
- Toolbar: two `command` nodes in the file-ops section of `project-toolbar.json`.
- Focus interplay: `TextInput`/`TextEdit` accept `ShortcutOverride` for editing keys, so a
  focused text field keeps its native text undo and the window-level shortcut fires only
  otherwise; the embedded code editors already reroute `ShortcutOverride` to their own
  stacks. Ctrl+Z is bound nowhere today (verified), so no ambiguous-shortcut collision.
  Verified live during implement (AC3/AC4 observations).

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Undo mechanism | per-op inverse commands; signal-driven rolling snapshot; scoped memento | **Scoped memento** — capture-before-mutation by construction, atomic composites free, zero bespoke inverse logic over lossy positional-ID remaps |
| Snapshot form | C++ deep-copy struct; compact JSON bytes | **JSON bytes** — reuses the proven serialize/load pair, AC1 is literally its round-trip, immune to drift when a new document member is added (serializer is the contract) |
| Step naming | extend CommandModel/toolbar for dynamic titles; tooltip-only | **Tooltip-only v1** — registry supports per-binding tooltips today; dynamic titles = registry surgery for cosmetic gain |
| Shortcut declaration | literal `"Ctrl+Z"`; extend StandardKey tables | **Extend tables** — platform-correct multi-bindings (incl. Ctrl+Y on Windows) from `QKeySequence::keyBindings`; two small table edits |
| Workspace edits vs snapshots | splice live workspace state into restores; accept whole-doc revert | **Accept + document** — splicing dangles refs after structural undos; spec already excludes workspace steps |
| Lock/unlock | undoable; history boundary | **Boundary (clear history)** — undoable unlock defeats the lock UX; lock saves to disk immediately anyway |
| Bound | steps-only; bytes-only; both | **100 steps + 64 MiB** — steps bound the common case, bytes bound pathological giant projects |
| R1 drift gate | code review only; lint rule | **`code-verify.py` rule** — slot with `setModified(true)` in ProjectModel TUs must open a scope; new mutation sites can't silently bypass history |

## Risks & mitigations

- **Ctor closure (spec 0001 protected surface).** `ProjectModel` ctor runs `newJsonFile()`
  before AppState/Dashboard exist. `ProjectHistory` is a plain member (no singleton calls in
  its ctor); capture is gated on `m_initialized`. Any edit inside the closure re-triggers
  the ctor-edge check — named at implement time.
- **Re-entrant capture during apply.** `groupsChanged` listeners mutate (auto-workspace
  regeneration). The `applying` flag suppresses scopes/captures for the duration; without
  it, undo would push garbage steps or recurse.
- **Selection reset / spurious reload.** Apply must not emit `jsonFileChanged` — that path
  triggers `AppState::onProjectLoaded` and the path-change selection reset. Direct
  `syncRuntime()` + the loaded-signal subset instead.
- **Autosave/backup interplay.** Apply calls `scheduleAutoSave()` (autosaved file converges
  to restored state; watcher re-armed by the write) and reaches BackupManager through the
  normal `modifiedChanged`/`contentTouched` debounce. No new persistence paths.
- **Missed mutation sites (R1).** The lint rule plus the pytest random-mutation round-trip
  are the two nets; a site that slips both shows up as a non-restoring field in AC1's
  byte-identical comparison.
- **Per-keystroke `updateGroup`/`updateDataset` commits** (verified: `onTextEdited` →
  commit per character). Without hints these would each capture; the hint/coalesce path is
  therefore mandatory in the same change, not a follow-up.
- **Translation strings**: tooltips use `%1` + `.arg()` (never `%n`), per the
  common-mistakes translation row.
- Pre-existing, out of lane, flagged for a separate pass: `mutationEpoch` is not bumped by
  widget-title verbs / `updateSourceFrameParserLanguage`, so `execute()`'s autosave hook
  misses them today. Undo does not depend on the epoch, but the gap remains.

## Test & verification plan

- **Unit (I can run):** none — no `tests/scripts/` surface involved (no JS parser change).
- **Integration (maintainer runs, app up with API server on 7777):**
  - `tests/integration/test_project_undo.py` (new):
    - AC1 — seed project, apply N=50 randomized mutations (entity CRUD, moves, batch,
      setters) via API, capture `project.exportJson` before/after, undo N → byte-identical
      to start, redo N → byte-identical to end.
    - AC2 — group with 5 datasets, `project.group.delete`, `project.undo` → structure,
      order, `uniqueId`s, all fields identical.
    - AC5 — `project.batch` (mixed ops) + single `project.undo` reverts all; `project.undo`
      on empty history → `{performed: false}`, `success: true`.
    - AC6 — save (API), mutate, undo past save point → `modified` true via
      `project.getStatus`; redo to save point → false.
- **Maintainer observations:** AC3 (shortcuts + palette/toolbar enablement/tooltips),
  AC4 (keystroke burst = one undo), AC7 (undo dataset delete with live widget on
  dashboard → widget returns with live data; tree/forms consistent).
- **Hotpath:** `--benchmark-hotpath` once before commit (no regression expected; gate
  anyway since `syncFromProjectModel`/pool invalidation sit adjacent).
- **Static:** `python scripts/code-verify.py --check` (incl. the new rule),
  `python scripts/registry-verify.py`, `qt-cpp-review` before handoff,
  `python scripts/sanitize-commit.py` before commit (regenerates SDK + command strings).
