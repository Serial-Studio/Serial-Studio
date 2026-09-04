# Architecture — Project Model, Files & Importers

> Part of the architecture corpus ([index](../architecture.md)). Read this file in full
> before touching ProjectModel/ProjectEditor, project JSON serialization, backups, or the
> importers. The ProjectModel ctor closure is a protected surface — see
> [startup.md](startup.md) and CLAUDE.md "Startup & Composition Root".

## ProjectModel / ProjectEditor Split

- `ProjectModel` (`Cpp_JSON_ProjectModel`): pure data — groups, actions, config, file I/O.
- `ProjectEditor` (`Cpp_JSON_ProjectEditor`): editor controller — tree model, form models,
  selection, comboboxes.
- QML enum access: `ProjectModel.SomeEnum` / `ProjectEditor.SomeEnum` — **not** `Cpp_JSON_*`.
- `groupsChanged` → `buildTreeModel()` is `Qt::QueuedConnection`; selection runs via hint
  signals afterwards.
- Title edits update the tree item in-place via `m_*Items` — never call a mutating
  `ProjectModel` function on every keystroke.

## Undo History — `ProjectHistory` (spec 0031)

- Whole-document snapshot undo: every mutating `ProjectModel` slot opens a
  `ProjectUndoScope` (RAII, depth-counted); the outermost scope **stages** a compact-JSON
  `serializeToJson()` pre-state, and the first `setModified(true)` **commits** it as a step —
  a slot that guard-returns without mutating never records a step. Composite operations
  (cascade deletes, `project.batch` via the `ProjectUndoFrame` opened in
  `API::CommandRegistry::execute()`, multi-select fan-outs) are one atomic step by
  construction.
- **Invariant: a new document-mutating slot must open a `ProjectUndoScope`.**
  `code-verify.py:undo-scope-missing` (error) enforces it, and it scans **every spec-0070
  sub-object TU**, not just `ProjectModel*.cpp`: `ProjectModel`, `ProjectEntities`,
  `ProjectOutputWidgets`, `ProjectBulkOps`, `ProjectSources`, `ProjectTables`, `ProjectFolders`,
  `ProjectWorkspaces`, `ProjectLoader`, `ProjectPersistence` and `ProjectPresentation` (Editor
  TUs excluded). The whitelist in that rule holds
  the intentional exceptions — workspace CRUD, presentation-blob setters (`widgetSettings`,
  `widgetDisplay`, tree expansion, layouts), and the history machinery itself. Those stay
  outside undo history by spec; their edits fold into neighboring whole-document snapshots
  (undoing a step can revert workspace edits made after it — accepted quirk).
- Keystroke coalescing: `ProjectEditor` commit handlers call
  `setNextUndoHint(label, key)` right before per-keystroke model calls; a same-key commit
  within 1 s extends the previous step and **skips serialization entirely**.
- Restore path: `applyHistorySnapshot()` → `applyJsonDocumentCore()` (shared with
  `loadFromJsonDocument`) → `emitProjectLoadedSignals(false)`. It must **never emit
  `jsonFileChanged`** (BackupManager would snapshot per undo; editor path handlers would
  fire) — the `frameDetectionChanged` → `AppState::onProjectLoaded` hop already rebuilds
  FrameBuilder + frameConfig. Re-entrant capture during apply is suppressed by the history
  `applying` flag.
- Boundaries: `loadFromJsonDocument`, `newJsonFile`, and `lockProject`/`unlockProject` clear
  history. The standard save paths (`finalizeProjectSave`, `autoSave`) call
  `m_history.markSaved()` — but not every `writeProjectFile()` caller does:
  `persistLegacyMigration()` bypasses it, so treat `markSaved()` as a property of the two save
  slots, not of disk writes. The Dashboard `pointsChanged` handler is **no longer** one of them:
  it calls `setPointCount()` (scope + `setModified`) instead of writing the whole document to
  disk, so changing a display setting no longer rewrites the `.ssproj` (H1). `markSaved()` also breaks the top step's coalesce chain; truncating the
  redo tail invalidates a save point inside it. The modified flag after undo/redo is
  `position != savePosition`, and the position moves only after a snapshot applied cleanly
  (`peekUndoState`/`confirmUndo` pairs).
- Scope placement rule: a slot that shows a confirmation dialog opens its scope **after**
  the dialog (nested event loops can commit a staged capture under the wrong label);
  `setGroupWidget` is the one exception because `confirmGroupWidgetChange` mutates the live
  group around its dialog. Per-keystroke code slots (`storeFrameParserCode`,
  `setControlScriptCode`, ...) pass a slot-level coalesce key to `ProjectUndoScope` instead
  of relying on editor hints.
- Bounds: 100 steps / 64 MiB, oldest dropped; a dropped save point pins the project modified
  until the next save. API verbs: `project.undo` / `project.redo` (never error on empty
  history — `{performed:false, reason}`).

## Mutation Rules Earned in Spec 0075

- **Deletion order is an explicit rank, not the `ItemKind` enum's numeric order.**
  `ProjectBulkOps.h`'s `batchDeleteRank()` / `batchDeleteOrderBefore()` are inline in the header
  (so the rule is unit-testable without the application, and the `.cpp` `static_assert`s the nine
  kind constants against `ProjectEditor::ItemKind`): datasets, tables, actions, output widgets,
  groups, workspaces, then folders, descending id within a rank. A folder deleted before the
  tables inside it left orphans (H3).
- **A deferred document mutation must be flushed before serialization.** The auto-workspace
  regeneration is queued (`scheduleWorkspaceRegen()` / `flushWorkspaceRegen()`) so a bulk delete
  regenerates once instead of once per group, and `writeProjectFile()` — the single choke point
  for every disk write, auto-save and save-as included — flushes it first, or a save could
  serialize a stale workspace list. Any future "coalesce this ProjectModel work" change owes the
  same flush. It also costs one event-loop turn: code reading `activeWorkspaces()` immediately
  after a group mutation sees the previous list for that turn.
- **Do not defer `frameDetectionChanged`.** `API::CommandRegistry::execute()` gates auto-save and
  the project apply on `mutationEpoch()` changing *within* the handler call, and the epoch is
  bumped by signal emissions — so deferring the one signal a delimiter-only command emits would
  make `project.frameParser.update` look like a no-op: no auto-save, no pipeline apply. A safe
  version of that optimisation has to debounce the *consumer*
  (`AppState::onProjectLoaded` -> `FrameBuilder::syncFromProjectModel`) and flush before
  `AppState::frameConfig()` is read at driver-open time, or a delimiter typed just before Connect
  opens the link with the previous frame config.
- **A 0 ms coalescing timer cannot collapse keystrokes.** Each keystroke is its own event-loop
  turn, so "one sync per burst" only ever means one per turn. The workspace-regen queue helps
  because a bulk delete emits N `groupsChanged` inside ONE call; a per-keystroke path needs a real
  idle debounce with the flush obligation above.
- **Compound edits get a compound mutator.** `setSourceFrameParserTemplateAndParams()` writes the
  template id and its params in one scope, so picking a native template is one undo step rather
  than two (H5).
- **`ProjectHistory::enterScope()` consumes the pending editor hint unconditionally**, so moving a
  `ProjectUndoScope` below a guard return also stops the hint being consumed on that path, and it
  can leak into an unrelated later step. Check every caller before relocating a scope.
- **`Dataset::sourceId` has exactly one derivation rule** (`finalize_frame`: it equals its
  group's). Normalising it inside `ProjectEntities::updateGroup` covers the editor and the API
  `group.update` path in one place (H9).
- **A failed external reload leaves the document attached.** `restoreDetachedDocument()`
  re-attaches the path, re-arms the watcher, restores or raises the modified flag and posts a
  NotificationCenter warning (H6) — a corrupt write on disk used to detach the open project.
- **`savePluginState` is ProjectFile-only**, like its sibling `saveWidgetSetting`: a QuickPlot
  plugin must not mutate or dirty a loaded project's widget-settings blob.

## On-Disk Change Detection — `ProjectModel` File Watcher

- A `QFileSystemWatcher` on `m_filePath` detects external edits: 500 ms debounce →
  SHA-256 content compare against `m_diskFileHash` → prompt to reload (or notification +
  `setModified(true)` in suppressed/API mode). Deletion posts a warning and dirties the
  project so a save can recreate the file. Signal: `projectFileChangedOnDisk()`.
- **Invariant**: every successful disk write or load must re-arm the watcher + hash via
  `watchProjectFile()`. `writeProjectFile()`, `loadFromJsonDocument()`, and `newJsonFile()`
  already do; a new save/load path that bypasses them will make self-writes look like
  external edits (QSaveFile's atomic rename also drops the watch on some platforms).

## Rolling Backups — `BackupManager`

- Auto-snapshots the project on a 5s debounce. The **whole-project SHA-1** over
  `serializeToJson()` is the sole write arbiter: identical content never duplicates a snapshot,
  any byte difference (incl. `frameParserCode`) does. Restore round-trips parser code + engines.
- Trigger is **decoupled from the dirty flag**. `setModified()` suppresses the flag for a
  structurally empty project (no groups/actions/tables/workspaces), but still emits
  `contentTouched` so parser-only edits on an empty project reach the snapshot path. Wire any new
  "edit that should back up but not dirty the project" through `contentTouched`, not a forced
  `modifiedChanged`.

## Multi-Source Architecture

- `DataModel::Source` entries in `Frame.h`. `FrameBuilder::hotpathRxSourceFrame(sourceId, data)`
  routes per-source frames. `FrameParser` keeps one engine per `sourceId`.
- GPL: `openJsonFile()` truncates `m_sources` to 1; `addSource()` is gated by
  `BUILD_COMMERCIAL`.
- Bus type change: open a one-shot `contextsRebuilt` connection that disconnects itself, then
  `buildSourceModel`. Don't force-rebuild on selection.

## Project File JSON Keys — `Keys::` Namespace

Every JSON key used in `.json`/`.ssproj` files is declared in `namespace Keys` in
**`core/Pipeline/DataModel/FrameKeys.h`** (which `Frame.h` includes — the namespace has not been in
`Frame.h` itself since spec 0070) as `inline constexpr QLatin1StringView` (alias `KeyView`).

- **Never hardcode** `"busType"`, `"frameStart"`, etc. in writers/readers or MCP handlers —
  use `Keys::BusType`, `Keys::FrameStart`. (`code-verify.py:keys-hardcoded-literal`.) That lint
  matches a hand-curated `_PROJECT_KEY_LITERALS` set in `scripts/code_verify_rules.py`, **not**
  `FrameKeys.h`: adding a `Keys::` constant does not make its raw literal an error until the set
  gains it too.
- `ss_jsr(obj, Keys::Foo, default)` is the canonical reader.
- **Legacy aliases (read canonical first, write both)**: `checksum` ↔ `checksumAlgorithm`,
  `decoder` ↔ `decoderMethod`. Older Serial Studio versions still load 3.3+ files.
- **Schema versioning** (`kSchemaVersion = 3`): `ProjectModel::serializeToJson()` always
  stamps `schemaVersion`, `writerVersion`, `writerVersionAtCreation`. Live runtime frames
  broadcast over the API keep `schemaVersion = 0` — `Frame::serialize` only emits when the
  Frame already carries a stamp. `current_writer_version()` lives in `Frame.cpp` so
  `Frame.h` doesn't need `AppInfo.h`.
- Use `obj.contains(Keys::Foo)` to detect "field absent", not `std::isnan` on a default-zero
  read.

## Dataset Property Registry (spec 0036)

Every persisted or editable **dataset** property is declared once, in
`app/rcc/properties/dataset.json` (shape pinned by `schema.json` beside it): field name and type,
`Keys::` spelling, default, serializer write rule, reader fallbacks and legacy keys, form section
and row, validator, enablement predicate, and API field name plus typed schema.

- **Four checked-in TUs are generated from it**, never hand-edited:
  `DataModel/Generated/DatasetRegistry.h` (descriptor table + form-id enum),
  `DataModel/Generated/DatasetSerialization.cpp` (project-JSON write/read),
  `DataModel/Generated/DatasetForm.cpp` (editor rows + commit dispatcher), and
  `API/Generated/DatasetApiFields.cpp` (API field appliers + typed schema).
- **Generator**: `scripts/generate-property-registry.py`; `--check` is gated in
  `sanitize-commit.py`, so a manifest edit that was not regenerated fails the pipeline. Output is
  deterministic and fenced with `clang-format off/on` so the reformat pass cannot fight `--check`.
- **Adding or changing a dataset property = edit the manifest, rerun the generator.** Editing a
  generated file loses the change on the next run; editing only the struct leaves the property
  unserialized, unformed, and invisible to the API.
- **Anything the manifest cannot express lives in
  `core/Pipeline/DataModel/Project/PropertyHooks.{h,cpp}`** and is *referenced by name* from the
  manifest: option sources (fixed, parallel-list, and project-state-derived choice domains),
  validators, enablement predicates, dynamic placeholders, and commit side effects. A commit hook
  returns a `RebuildHint` instead of rebuilding a form itself, so the editor keeps owning the
  synchronous-versus-deferred rebuild split.
- **Two drift gates guard the declaration itself.** `registry-verify.py`'s
  `check_property_manifests()` validates the manifest against `schema.json` (with a built-in
  draft-07 subset validator, so no new dependency), and proves every `jsonKey` names a real
  `Keys::` constant, every hook name is backed by `PropertyHooks.h`, every widget is a real
  `ProjectEditor::EditorWidget`, every `Dataset` struct field is declared exactly once (property,
  runtime field, or sub-entity), and that `DatasetItem`'s enumerator order still matches
  `formIdOrder`. `code-verify.py`'s `registry-parallel-field-map` (error) fires when a
  non-generated file spells out four or more dataset property keys — the hand-written field map
  growing back. Both list their exceptions inline, with a reason each.
- **The multi-selection harvest reads the table, not a throwaway model.**
  `ProjectEditor::datasetEditValues()` walks `kDatasetProperties` and asks the generated
  `Registry::datasetFormValue()` for each row's value; a row whose `visibleWhen` predicate is
  false yields an invalid `QVariant` and is skipped, exactly as the row builders omit it.
- **Undo is unchanged by the derivation.** `applyDatasetFormEdit()` only mutates the struct;
  `ProjectEditor::commitDatasetFormEdit()` is the single choke point that calls
  `setNextUndoHint()` + `ProjectModel::updateDataset()`, taking the coalesce key and the
  rebuild-tree flag from the property's descriptor rather than from a per-field `if`.

## Generated API Surfaces (spec 0037)

Everything downstream of the 0036 manifest is either *generated* from it or *checked*
against it — never retyped. `API::CommandDefinition::inputSchema` is the runtime hub; MCP
`tools/list` copies it verbatim, `--dump-api-schema` flattens it into
`app/rcc/api/api-schema.json`, and the SDK and proto emitters read that snapshot.

- **One shared projection.** `schema_props_for(prop, manifest)` in
  `scripts/generate-property-registry.py` is the single definition of "what schema does this
  property produce". The C++ emitter (`DatasetApiFields.cpp`) and the Python snapshot
  projector both call it, so a second reading of the manifest cannot disagree with the first.
- **Generated, checked in:** `app/rcc/api/proto-fields.json` (the gRPC field-number ledger,
  bundled in `rcc.qrc` because the runtime reads it) and
  `doc/grpc/serialstudio-typed.proto` (the client-facing typed proto, not bundled, not built).
  `doc/grpc/serialstudio.proto` — the dynamic service protoc compiles — is untouched.
- **Checked, not generated:** `api-schema.json` covers all 347 commands, ~300 with
  hand-written C++ schemas, so only a build can produce it.
  `generate-property-registry.py --check-snapshot` projects the dataset verbs' typed schema
  and byte-compares that slice. It **warns locally and fails in CI** (`--strict`, or `CI` in
  the environment), because a contributor without a build cannot clear it; the message names
  the command, each differing field, and the ordered fix.
- **gRPC numbers are append-only released state.** `ProtoGenerator::buildCommandMessages`
  reads the ledger instead of numbering by `QJsonObject` iteration order, which was
  alphabetical — inserting `alias` ahead of `color` used to renumber every field after it,
  silently changing what a shipped client reads. A parameter keeps its number forever, a
  removed one moves to `reserved`, `1` is the request id everywhere, and a parameter the
  bundled ledger predates is appended after the message's current maximum.
- **Which gate fires when:** `code-verify.py` → `api-generated-edited` (marker deleted from any
  generated artifact, the four C++ TUs included), `proto-field-renumbered` (a number moved versus
  HEAD), `registry-parallel-field-map` (a hand-written dataset field map); `registry-verify.py` →
  the property-manifest rule, the corpus field/enum reference lint, and the snapshot projection;
  `generate-sdk.py --check` and
  `generate-property-registry.py --check` → byte-compare of every generated artifact. All of
  them run in the CI `lint` job and in `sanitize-commit.py`.
- **The assistant corpus is linted, not generated.** `app/rcc/ai/skills/*.md` may not state a
  dataset field name, a widget-option bit, or an enum value the code contradicts — including
  claiming the API rejects a spelling the manifest declares as an alias.

## Modbus Map Importer (Pro)

`DataModel::ModbusMapImporter` imports CSV/XML/JSON →
auto-generates a Modbus project; preview in `ModbusPreviewDialog.qml`. Pairs with
`IO::Drivers::Modbus::generateProject`.

## Importer Parser Output

The Modbus map and DBC importers generate **commented,
declarative Lua parsers** (`frameParserLanguage = Lua`), not native map templates — the
`modbus_register_map` / `can_signal_map` templates and `MapTemplates.cpp` were removed
(projects that referenced them must be re-imported). The generated parser decodes through
a spec table (one line per signal/register, DBC `CM_` comments inlined) and publishes
**physical values into per-group data tables** via `tableSet`; every dataset is
`virtual: true` with a Lua `tableGet` transform (`ImporterCommon.h::applyTableTransform`),
so nothing depends on positional parser channels (parsers return `{ 0 }` as a dummy row —
an empty return would skip the frame and starve the transforms). The Modbus Lua keeps the
driver's round-robin poll cursor as chunk-local state and resyncs on the response function
code (RegBool decodes the whole word; bit path only for coil/discrete blocks); the CAN Lua
mirrors the DBC bit semantics (Motorola sawtooth walk, Intel LSB-first, Qt endian flag
verbatim) — both pinned by `test_cpp_regressions.py` R14/R15 against the codegen. The CAN
driver publishes standard frames as `[ID_hi, ID_lo, DLC, data...]` (11-bit id, byte 0 top
bit always clear) and extended frames as `[0x80|ID28..24, ID23..16, ID15..8, ID7..0, DLC,
data...]` — bit 7 of byte 0 selects the form, `write()` mirrors it, and the generated Lua's
`frame_id()` decodes both (pinned by R17). The Modbus *driver* quick-connect
(`buildFrameParser`) and the Protobuf importer still generate their own script parsers.

## Importer Dashboards (DBC + Modbus Map)

Summary-first projects. Every group is a
DataGrid (DBC still detects GPS / accelerometer / gyroscope groups), analog datasets carry
plot + bar/gauge/meter toggles disclosed on demand via the data grid's pop-out buttons,
boolean signals are LEDs with an explicit `[0.5, 1]` Ok alarm band (no reliance on the
runtime `ledHigh` synthesis), DBC `VAL_` value tables become Lua transforms returning the
label text (only when factor = 1 / offset = 0), and `displayFormat` decimals derive from
the scaling factor. Generated bar/gauge/meter datasets get the analog display policy
(`ImporterCommon.h::applyAnalogDisplayPolicy`): integer-aligned tick counts (0-10 → 11
ticks, 0-150 → 7) and integer labels once the range spans more than one unit. Both
importers seed **customized workspaces** — a leading Overview
aggregating every group's refs (multi-group projects only), then one workspace per group,
each holding only the group-widget ref (+ LED panel ref), user-range IDs (≥ 5000 so the
load-time auto-range remap never fires) — through
`Importers/ImporterCommon.h::finalizeImportedProject`, which also assigns group uniqueIds,
serializes the data tables, and stamps `schemaVersion` + `nextUniqueId`: omit those stamps
and the loader treats the import as an older-schema project and silently drops the seeded
workspaces.
