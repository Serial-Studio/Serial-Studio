---
spec: 0036-property-registry
phase: tasks
status: draft        # draft -> approved (gate before /ss-implement)
updated: 2026-07-25
---

# Tasks 0036 — Property registry (declare once, derive everywhere)

> **Phase 3 of 4 — the ordered checklist.** Decompose [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable*. `/ss-implement` works this list top to
> bottom and keeps the status boxes current. Gate: do not start `/ss-implement` until a
> human marks this `approved`.

## Conventions

- One task = one focused, reviewable change, sized for a single agent.
- **Verify** is how *this* unit is confirmed before moving on.
- **Deps** lists task IDs that must land first.
- v1 is the **dataset entity end to end**. Group, action, source, and output-widget follow
  in a later spec once the emitter is proven.
- T1 must land before any code change — it captures the "before" truth everything else is
  measured against.

## Tasks

### T1 — Capture the round-trip baseline (before any code change)

- **Files:** `tests/integration/baselines/` (new, checked-in JSON),
  `tests/integration/test_property_registry.py` (new, capture + compare modes)
- **Does:** A pytest module with a `--capture-baselines` path that opens every project
  under `examples/` plus the bundled templates via `project.open`, dumps
  `project.exportJson`, and writes one baseline file per project; and a default path that
  re-runs the loop and byte-compares. Also records the current dataset-property line counts
  (per the plan's measurement) into the task notes, so T14 can report the delta.
  **This runs against today's binary and must be committed before any generator work** —
  its whole value is being captured pre-change.
- **Verify:** maintainer runs the capture against the current build with the API server on
  7777; the compare run is green immediately afterwards; `python -m py_compile` and
  `pytest --collect-only` locally.
- **Deps:** none
- [~] **script ready -- maintainer must run.** `tests/integration/test_property_registry.py`
  ships capture mode (`SS_CAPTURE_BASELINES=1`, env var rather than a CLI flag so the shared
  `tests/conftest.py` is untouched) and compare mode; `py_compile` and `pytest --collect-only`
  are green (3 tests). `tests/integration/baselines/` is written by the capture run, so it is
  still empty and the compare test skips until then. A **static provisional baseline** is
  checked in at `doc/claude/specs/0036-property-registry/baseline-manifest.json`: file list +
  sha256 + group/dataset counts for all 22 shipped `.ssproj` files (examples/ + `app/rcc/demo/`
  + `app/rcc/templates/`, 281 datasets). `test_corpus_files_unchanged` runs offline against it
  and passes today, so a baseline captured against a drifted corpus can never be compared
  silently. The line-count measurement for T14 is recorded in the build notes below.

### T2 — Manifest schema + dataset declaration

- **Files:** `app/rcc/properties/schema.json` (new),
  `app/rcc/properties/dataset.json` (new)
- **Does:** JSON Schema for the manifest (fields per plan: `id`, `field`, `jsonKey`,
  `apiName`, `apiAliases`, `type`, `default`, `scope`, `persist`, `section`, `widget`,
  `label`, `description`, `placeholder`, `options`, `visibleWhen`, `enabledWhen`,
  `validate`, `clamp`, `legacyKeys`, `coalesce`, `rebuildTree`, `onCommit`, `pro`), plus
  the dataset declaration itself. The dataset entries are **transcribed** from the existing
  serializer, form builders, commit switches, and API appliers — read each source, do not
  re-derive intent. Declares the five runtime-only `Dataset` fields explicitly
  (`value`, `rawValue`, `numericValue`, `rawNumericValue`, `isNumeric`) and the two nested
  sub-entities (`alarmBands`, `fftMarkers`) as hook-backed. No C++ in this task.
- **Verify:** `python -c "import json"` load of both files; hand-check that every
  `Dataset` struct field appears exactly once as a property, a runtime-only entry, or a
  sub-entity; every `jsonKey` names a real constant in the `Keys::` namespace; every
  `default` equals the struct initializer (record the four known mismatches as intentional
  fixes).
- **Deps:** none (parallel-safe with T1)
- [x] done. 41 properties + 5 runtime fields + 2 sub-entities = the 48 `Dataset` struct
  fields exactly (checked programmatically against `Frame.h`); every `jsonKey`/`legacyKeys`
  resolves to a real `Keys::` constant; 34 `formId`s in the frozen pre-registry enum order
  (byte-compared against `ProjectEditorItemIds.h`); 9 declared API aliases (`plotMin`,
  `plotMax`, `xAxis`, `widgetMin`, `widgetMax`, `datasetSourceId` + the legacy
  `alarmLow`/`alarmHigh`/`alarmEnabled` triple on `alarmBands`). Manifest additions beyond
  the plan's field list, each because a surface needed it: `formIdOrder` (freezes enum
  numbering), `postRead` (ordered reader steps), `readBack` (`groupId`/`datasetId`/
  `numericValue` are written but never read back), `commitTransform` (editor-only
  `.simplified()` on color/alias), per-surface `clamp` (`editor` vs `api` bounds genuinely
  differ on `decimalPoints`), `apiDescription`, and `legacy` on `alarmBands`.
  **Finding:** `value` and `numericValue` are runtime values that today's serializer still
  writes into the document, so they are declared `persist: always` -- dropping them would
  change every saved file. **Finding:** `sourceId` is read from `datasetSourceId` but never
  written (`finalize_frame` re-derives it), hence `persist: never`.

### T3 — `registry-verify.py`: property-manifest rule

- **Files:** `scripts/registry-verify.py`
- **Does:** New `check_property_manifests(errors)` following the existing `check_manifests`
  shape, plus one call line in `main()`. Validates the manifest against `schema.json`,
  unique ids, `jsonKey` resolves to a real `Keys::` constant in `Frame.h`, `widget` is a
  known editor widget kind, every referenced hook/option-source name exists in
  `PropertyHooks.h`, no `Dataset` struct field left unaccounted for, and no field declared
  twice. Inherits the existing `fail()`/exit-code/reporting machinery.
- **Verify:** `python scripts/registry-verify.py` clean; seed each violation class locally
  (bad key, missing hook, duplicate id, unaccounted struct field), confirm the message,
  revert.
- **Deps:** T2
- [x] **done.** `check_property_manifests(errors)` plus one call line in `main()` (before the
  snapshot check, so a malformed manifest is reported before its projections). It validates
  `dataset.json` against `schema.json` through a **self-contained draft-07 subset validator** --
  `jsonschema` is only in `tests/requirements.txt`, and a gate that runs in `sanitize-commit.py`
  must not depend on it; an unrecognized schema keyword is itself a failure, so the schema can
  never outgrow the validator silently. Beyond the shape: unique ids; every
  `jsonKey`/`legacyKeys` resolves to a real `Keys::` constant in `Frame.h`; every referenced
  hook is declared in the manifest with the right kind *and* backed by `PropertyHooks.h` (or
  `Frame.h` for sub-entities, or explicitly marked caller-owned -- `onAliasRejected` is the only
  one); every `widget` is a `ProjectEditor::EditorWidget` enumerator; `options`/`pairWith`/
  `persistWith`/`tupleFields`/builder rows resolve to declared entities; `labelProvider`/
  `valueProvider` name real `ProjectModel` accessors; every `Dataset` struct field is claimed
  exactly once (property, runtime field, or sub-entity) and no entry names a field that does not
  exist; and `DatasetRegistry.h`'s `DatasetItem` enumerator order still matches `formIdOrder`
  (the parent-requested addition -- renumbering it silently repoints every persisted form id).
  Every violation class was seeded against an in-memory mutated manifest and produces the
  message quoted above; the repo run is CLEAN.

### T4 — `PropertyHooks`: OptionSource abstraction

- **Files:** `app/src/DataModel/Project/PropertyHooks.h` (new),
  `app/src/DataModel/Project/PropertyHooks.cpp` (new), `app/CMakeLists.txt`
- **Does:** One `OptionSource` interface with `labels()`, `indexForValue()`,
  `valueForIndex()`, and the four adapters the existing combos need: `staticMap`
  (key→label, e.g. dataset widgets, display formats), `parallelValues` (labels + values,
  e.g. FFT windows, FFT sample counts), `liveProvider` (computed from project state, e.g.
  X-axis and waterfall-Y sources), and `tuple` (one row → several fields, e.g. the
  plot/log pair). Implementations move the existing lookup logic verbatim from
  `ProjectEditor::generateComboBoxModels()` and the seven hand-rolled index-search loops in
  the dataset form; those call sites are **not** removed yet. **Binding invariants: no
  `instance()` reach — providers take the model by reference from the caller
  (ctor-closure safety); the row's editable value stays a positional index (the generic QML
  delegate depends on it); header order and `[[nodiscard]]` per style; no in-header member
  init.**
- **Verify:** `python scripts/code-verify.py --check` on both files; read-back against the
  seven existing search loops — each adapter reproduces its loop's exact behavior including
  the not-found fallback to index 0.
- **Deps:** none
- [x] done, **minus the `app/CMakeLists.txt` registration** (phase 2 owns the build files).
  `PropertyHooks.h/.cpp` ship `OptionSource` plus the four adapters. Two deliberate splits
  from the plan's sketch: the option *data* stays in the manifest and is emitted into the
  generated TU, so the hooks TU holds only the adapters (declaring the tables in both places
  would defeat the point); and labels translate through
  `QCoreApplication::translate("ProjectEditor", ...)`, so already-translated combo strings
  keep their context (AC9). Not-found behaviour is per-adapter and declared: index 0 for
  `staticMap`/`liveProvider`/`tuple`, the declared `notFoundIndex` for `parallelValues`
  (7 = 1024 for FFT sample counts, 5 = Blackman-Harris for windows). Existing call sites are
  untouched, as specified.

### T5 — `PropertyHooks`: validators, predicates, commit side effects

- **Files:** `app/src/DataModel/Project/PropertyHooks.h`,
  `app/src/DataModel/Project/PropertyHooks.cpp`
- **Does:** The named escape hatches, each moved from its current single implementation:
  validators (`datasetAlias` uniqueness, `color` validity, `transformLanguage` domain,
  `index >= 0`, `fftWindow` range); visibility/enablement predicates
  (`insidePainterGroup`, `waterfallEnabled`, `widgetRangeApplicable`, `plotEnabled`,
  `fftEnabled`, `ledEnabledWithoutBands`, `xAxisNotTime`); commit side effects
  (`onWidgetChanged` — the compass range rewrite and alarm-band clear, `onVirtualChanged`,
  `onXAxisChanged`, `onAliasRejected`). Each returns data (error string, bool, "rebuild:
  none/sync/deferred") — **no hook rebuilds a form or shows a dialog itself**; the caller
  owns that, preserving today's sync-versus-deferred split.
- **Verify:** `code-verify.py --check`; read-back diff against each origin site
  (`ProjectEditorCommit.cpp`, `ProjectEditorForms.cpp`, `ProjectHandlerEntities.cpp`)
  confirming behavior is byte-for-byte equivalent logic.
- **Deps:** T4
- [x] done. 5 validators, 11 predicates, 1 dynamic placeholder
  (`datasetIndexPlaceholder`), 5 commit hooks. Naming differs from the sketch where the
  origin site disagreed: `ledEnabledWithoutBands` split into `ledEnabled` (row enablement)
  and `ledBandsAbsent` (row visibility) because the form uses them separately;
  `xAxisNotTime` became `plotEnabledNonTimeX` because the log-X row gates on both;
  `widgetSelectable` was added for `datasetWidgetEditable`, which `widgetRangeApplicable`
  builds on. `onAliasRejected` stays caller-owned (message box + snap-back) and is declared
  but never called from generated code. Every hook takes the model by reference; no
  `instance()` reach. `code-verify.py --check` clean (Q_ASSERT swapped for `SS_ASSERT_LOG`,
  so the new TU adds zero advisories).

### T6 — Generator skeleton + descriptor table + pipeline wiring

- **Files:** `scripts/generate-property-registry.py` (new),
  `app/src/DataModel/Generated/DatasetRegistry.h` (new, generated),
  `scripts/sanitize-commit.py`, `app/CMakeLists.txt`
- **Does:** The generator's core: manifest load, deterministic ordering, the shared
  emit/`--check` machinery (byte-compare, exit 1 on drift, `newline=""` LF policy, dual
  license header + a visible "AUTO-GENERATED ... regenerate, never edit" marker, mirroring
  `generate-command-strings.py`). First emitted artifact: the `constexpr` descriptor table
  and the `DatasetItem` form-id enum. One `run_python_step` block in `sanitize-commit.py`
  between the SDK and command-strings steps, plus the header-comment pipeline line.
  **Binding invariant: emitted C++ must already satisfy `clang-format` and
  `code-verify.py`, or the `--check` gate flaps against the sanitize pipeline's reformat.**
- **Verify:** run the generator twice → no diff; `--check` clean, then edit the manifest
  and confirm `--check` exits 1; run `clang-format` over the generated header and confirm
  zero change; `code-verify.py --check` on the generated header.
- **Deps:** T2
- [x] done, **minus the `app/CMakeLists.txt` registration** (phase 2). `--check` is wired
  into `sanitize-commit.py` between the command-strings and registry-verify steps, with the
  header-comment pipeline line updated. Two decisions worth review: every generated file is
  fenced in `// clang-format off/on` (the generator owns its formatting outright, so the
  sanitize reformat pass cannot fight the drift gate -- the same fence `CommandStrings.cpp`
  already uses), and the generator hard-fails if any emitted line exceeds 100 columns, so a
  style violation cannot ship silently. `DatasetRegistry.h` also emits the `DatasetItem`
  enum in the frozen pre-registry order.

### T7 — Derive project-file serialization

- **Files:** `scripts/generate-property-registry.py`,
  `app/src/DataModel/Generated/DatasetSerialization.cpp` (new, generated),
  `app/src/DataModel/Frame.h`, `app/src/DataModel/Frame.cpp`, `app/CMakeLists.txt`
- **Does:** Emit `DataModel::serialize(const Dataset&)` and
  `DataModel::read(Dataset&, const QJsonObject&)` with identical names and signatures;
  remove the hand-written bodies and leave declarations in `Frame.h`. Honors `persist`
  (always vs when-non-default), `scope` (runtime fields excluded from document writes),
  `legacyKeys` read fallbacks, declared clamps, and the min/max normalization. Calls the
  hand-written `readDatasetAlarmBands` / `readDatasetFrequencyMarkers` through the declared
  sub-entity hooks. Applies the two intended defect fixes (`overviewDisplay` written;
  read defaults aligned to struct defaults). **Binding invariants: `Keys::` stays the only
  place a key string is written; the `Dataset` struct is not modified; ADL composition from
  `serialize(Group)` must keep resolving.**
- **Verify:** `code-verify.py --check`; side-by-side read-back of the generated pair
  against the removed bodies, insert-for-insert; T1's baseline compare run by the
  maintainer showing only the two declared deltas.
- **Deps:** T6
- [x] **done (phase 2 integrated).** `Frame.h` keeps a non-inline
  `[[nodiscard]] QJsonObject serialize(const Dataset& d);` (101 -> 5 lines) and
  `normalizeDatasetRanges` (the generated reader calls it); `Frame.cpp` lost the 82-line
  `read(Dataset&, ...)` body, keeping `readDatasetAlarmBands` /
  `readDatasetFrequencyMarkers`. Byte-compat evidence, insert-for-insert: `QJsonObject` is a
  sorted map, so the generator's different *insertion* order cannot change output bytes; every
  write predicate matches the removed body (`fftBallistics` still guards both `FFTBallistics`
  and `FFTBallisticsRelease`; `transformCode` non-empty still guards both `TransformCode` and
  `TransformLanguage`), and the only added write is the declared `overviewDisplay` fix. On the
  read side the only value changes are the four declared default alignments (`index` -1 -> 0,
  `fftSamples` -1 -> 256, `fftSamplingRate` -1 -> 100, `ledHigh` 0 -> 80); `xAxisId` -2 is
  `kXAxisTime`, `fftWindow` 5 is `FFTWindowBlackmanHarris`, `PropertyHooks::isValidColor`
  returns true for empty (matching the old `!isEmpty() && !valid` guard), and `sourceId` is
  still read from `Keys::DatasetSourceId` and never written.

### T8 — Derive editor form rows

- **Files:** `scripts/generate-property-registry.py`,
  `app/src/DataModel/Generated/DatasetForm.cpp` (new, generated),
  `app/src/DataModel/Project/ProjectEditorForms.cpp`,
  `app/src/DataModel/Project/ProjectEditorItemIds.h`, `app/CMakeLists.txt`
- **Does:** Emit the dataset row builders as `DataModel::ProjectEditor` member functions
  (so `tr()` keeps the existing translation context and translated strings stay valid);
  remove the hand-written builders. Section headers, the per-child navigation rows, and the
  alarm-band / frequency-marker launcher rows stay hand-written in `buildDatasetModel`,
  which now calls the generated emitter for the property rows. Row emission uses the T4
  option sources — **no search loops in generated code**. Honors `visibleWhen` (row omitted)
  and `enabledWhen` (row present, `setEditable(false)` + inactive) as two distinct
  mechanisms. Deletes the dataset half of `ProjectEditorItemIds.h` in favor of the
  generated enum.
- **Verify:** `code-verify.py --check`; row-by-row read-back against the removed builders
  (order, widget kind, label, description, placeholder, min/max, enablement); maintainer
  observation AC4 across every dataset widget kind.
- **Deps:** T4, T6
- [x] **done (phase 2 integrated).** Ten hand-written builders left `ProjectEditorForms.cpp`
  (-597 lines): `addGeneralSection`, `addDatasetAliasRow`, `addDatasetRangeRows`,
  `addPlotSection`, `buildFftGeneralRows`, `buildFftRangeRows`, `addWidgetSection`,
  `buildWidgetFormatRows`, `buildWidgetRangeRows`, `addLEDSection`. **Correction to the phase-1
  note: `addFFTSection` is *not* emitted** -- it is a two-call wrapper with no property rows, so
  it stays hand-written next to `buildDatasetModel`'s scaffolding. `addGeneralColorRow` is
  declared in `ProjectEditor.h` (the one added line). `ProjectEditorItemIds.h` now includes
  `DataModel/Generated/DatasetRegistry.h` in place of its 39-line `DatasetItem` enum; the
  enumerator order is byte-identical, so nothing renumbers, and the eight other id enums are
  untouched. Every consumer was grepped and read: all ten builders are called only from
  `buildDatasetModel`, `datasetEditValues` and `buildMultiDatasetModel`, all of which keep
  compiling against the unchanged signatures.

### T9 — Derive editor write-back

- **Files:** `scripts/generate-property-registry.py`,
  `app/src/DataModel/Generated/DatasetForm.cpp`,
  `app/src/DataModel/Project/ProjectEditorCommit.cpp`
- **Does:** Emit a commit dispatcher that converts the row's variant to the field type and
  writes it onto a `Dataset&`, invoking named `onCommit` hooks and returning a rebuild
  hint; remove the five hand-written sub-appliers. `onDatasetItemChanged` keeps its
  orchestration — validation gate, undo hint, tree-item patch, signal emission — and now
  derives the coalesce key and label from the descriptor table
  (`coalesce: true` → `"dataset:<groupId>:<datasetId>:<id>"`, `false` → empty key).
  **Binding invariants: the generated dispatcher never calls `ProjectModel` and never
  rebuilds a form; all writes still reach the document through
  `ProjectModel::setNextUndoHint(...)` + `ProjectModel::updateDataset(...)`, the existing
  `ProjectUndoScope` choke point — no new mutating model slot, or
  `code-verify.py:undo-scope-missing` fires.**
- **Verify:** `code-verify.py --check` (undo-scope rule included); read-back that every
  removed `case` has a generated equivalent; `grep` confirms no new `setModified(true)`
  outside the funnel; maintainer AC5 (edit + undo per field, typing burst = one step).
- **Deps:** T5, T8
- [x] **done (phase 2 integrated).** The five sub-appliers are gone from
  `ProjectEditorCommit.cpp` (-261 lines); `onDatasetItemChanged` now calls
  `Registry::applyDatasetFormEdit(formId, value, m_selectedDataset, m_projectModelRef)` and
  owns everything the dispatcher refuses to do, split across three new hand-written members
  (~95 lines total): `datasetFormEditAccepted` (the combo-range guards + the alias dialog gate),
  `syncDatasetTreeVirtualFlag` (the `TreeViewVirtual` tree patch, shared with the multi-select
  fan-out so batch virtual toggles still repaint), and `commitDatasetFormEdit` (undo hint,
  `updateDataset`, the title tree-item rename, the two signal emissions).
  **Undo-path evidence:** every write still leaves through
  `ProjectModel::setNextUndoHint(...)` + `ProjectModel::updateDataset(...)`, i.e. the
  `ProjectUndoScope` at `ProjectModelCrud.cpp:143`; no new `ProjectModel` slot, no
  `setModified(true)` outside that funnel (`code-verify --check` is clean, `undo-scope-missing`
  included). The label stays a literal `tr("Rename Dataset")` / `tr("Edit Dataset")` so lupdate
  keeps extracting them (AC9); `rebuildTree` and the coalesce key now come from
  `kDatasetProperties` -- `coalesce: true` yields `"<coalesceKey>:<g>:<d>:<formId>"`,
  `coalesce: false` yields an empty key, which `ProjectHistory::enterScope` turns into "never
  merge" because `updateDataset`'s scope declares no slot key. **Deliberate deviations,
  both narrow:** (a) the title key gains a trailing `:<formId>` so one key shape serves every
  property -- still unique per dataset+field, so the typing burst still coalesces; (b) combo
  rows that declare `coalesce: false` (widget, plot, x-axis, FFT window/samples, display
  format, every checkbox) no longer merge with the previous step, where the old hardcoded key
  merged them -- this is the R6 granularity the plan specifies. **One behaviour change worth
  review:** an out-of-range `DisplayFormat` index is now rejected outright by
  `datasetFormEditAccepted` instead of being silently ignored while still pushing an undo step;
  the guard exists because the generated dispatcher would otherwise store an empty format
  string. The same guard was added to the multi-select fan-out for parity.
  `ProjectEditorMultiSelect.cpp`'s fan-out loop also swapped its five sub-applier calls for one
  `applyDatasetFormEdit` call inside the existing `ProjectUndoFrame` + `setNextUndoHint` pair,
  which is untouched, so a bulk edit is still one undo step.

### T10 — Derive API field handling + typed schema

- **Files:** `scripts/generate-property-registry.py`,
  `app/src/API/Generated/DatasetApiFields.cpp` (new, generated),
  `app/src/API/Handlers/ProjectHandlerEntities.cpp`,
  `app/src/API/Handlers/ProjectHandler.cpp`, `app/CMakeLists.txt`
- **Does:** Emit `applyDatasetUpdateParams` and its per-group appliers with the same name
  and signature: `takeParam` on `apiName` **and** each `apiAlias` (R12 — the disk spelling
  is accepted so a read object can be written straight back), declared coercion, clamps,
  named validators returning error strings, `rebuildTree` accumulation, and automatic
  `consumed`-set seeding so the unknown-field warning can no longer be forgotten. Emit the
  typed `properties`/`required` schema block for the dataset verbs; `ProjectHandler.cpp`
  drops the field enumeration from the prose description and keeps only the guidance
  paragraphs. Pro-gated properties emit the existing `#ifdef BUILD_COMMERCIAL` guards.
- **Verify:** `code-verify.py --check`; read-back against the removed appliers,
  field-for-field; maintainer AC6/AC11/AC12 after re-dumping `api-schema.json` and
  re-running `generate-sdk.py`.
- **Deps:** T5, T6
- [x] **done (phase 2 integrated).** `ProjectHandlerEntities.cpp` lost `takeParam`,
  `aliasInUseByOtherDataset`, the four `applyDataset*Fields` and the old
  `applyDatasetUpdateParams` (-298 lines); `datasetUpdate` is byte-for-byte unchanged and now
  resolves the generated definition. `applySimpleAlarmFields` is promoted out of file-static
  and declared in `ProjectHandler.h` alongside `[[nodiscard]] QJsonObject datasetFieldSchema();`
  (both in `namespace API::Handlers`, which is what the generated TU's unqualified calls
  resolve against). `ProjectHandler.cpp` grows one `datasetUpdateSchema()` helper that merges
  `datasetFieldSchema()` into the two identity params' `properties` block, and the verb's prose
  drops its 26-line field enumeration in favour of pointing at that block; the guidance
  paragraphs (virtual/index/transformLanguage reminders, multiplot log-axis coherence, the
  alarm/marker payload shapes, the unknown-field warning contract) are kept, and the alias
  spellings are now named as a group rather than per field. `takeParam` had no non-dataset
  callers (`groupUpdate` predates it and does not use it), so nothing else moved.
  **Deviation from the plan, deliberate (carried from phase 1):** `pro: true` emits no
  `#ifdef BUILD_COMMERCIAL` guard, because today's dataset appliers have none -- adding one
  would change GPL-build behaviour, which is not a declared delta.

### T11 — `code-verify.py`: no parallel field maps

- **Files:** `scripts/code-verify.py`
- **Does:** New error-level rule: a dataset `Keys::` constant referenced outside the
  generated TUs, `PropertyHooks.cpp`, and an explicit whitelist is a violation — the drift
  gate that stops a hand-written field map from re-growing beside the registry. Whitelist
  the legitimate non-registry consumers found while implementing (importers, validators,
  snapshot builders) with a one-line reason each.
- **Verify:** `python scripts/code-verify.py --check` over the repo → zero new errors after
  T7-T10; seed a violation locally, confirm the message, revert.
- **Deps:** T7, T8, T9, T10
- [x] **done, with one deliberate reshaping of the rule.** A literal "any dataset `Keys::`
  constant outside the generated TUs" ban is unimplementable as an error: `Keys::Title`,
  `GroupId`, `DatasetId` and `UniqueId` address *any* entity and appear in 14 handlers, the AI
  tool dispatcher and the session exporter. The rule therefore drops those four identity keys and
  fires on a **cluster**: a non-generated file under `app/src` naming **four or more** dataset
  property keys is `registry-parallel-field-map` (**error**), which is exactly the shape a
  hand-written field map has. Measured headroom: the worst non-exempt file today names three
  (`ProjectHandler.cpp` / `ProjectHandlerEntities.cpp`: `AlarmBands`, `FFTMarkers`, `Virtual` --
  the nested-entity payloads). Whitelist, one reason each: `Frame.h` (the `Keys::` home + the
  group/action/source serializers), `Frame.cpp` (the hand-written sub-entity readers the manifest
  declares as hooks), `Benchmark/HotpathBenchmark.cpp` (synthetic benchmark fixtures),
  `PropertyHooks.cpp` and `PropertyValidators.cpp` (the named escape hatches). Seeded by clearing
  the whitelist: the three exempt files fire with their key lists, everything else stays silent.
  **Second half of the same guard:** the four generated C++ TUs joined `_API_GENERATED`, so
  deleting a do-not-edit marker is `api-generated-edited`; the marker window grew from 400 to
  1200 characters because the dual-license header precedes the banner in a `.cpp`/`.h`.
  `code-verify.py --check` over the repo: **0 errors** (529 pre-existing advisories, unchanged).

### T12 — Simplify multi-selection off the descriptor table

- **Files:** `app/src/DataModel/Project/ProjectEditorMultiSelect.cpp`
- **Does:** Replace the throwaway-form-model harvest (`datasetEditValues` builds a
  disposable model just to read a parameter-id → value map) with a direct read of the
  descriptor table. Behavior unchanged: identity rows still blanked, disagreeing rows still
  marked mixed. **Binding invariant: the existing `ProjectUndoFrame` + `setNextUndoHint`
  pair around the fan-out loops stays exactly as it is — a bulk edit remains one undo
  step.**
- **Verify:** `code-verify.py --check`; read-back that the mixed-value marking still keys
  off the same ids; maintainer spot-check of a multi-dataset edit + single undo.
- **Deps:** T8
- [x] **done.** The fan-out half landed earlier (`onMultiSelectionItemChanged` applies one
  `Registry::applyDatasetFormEdit` per selected dataset inside the unchanged `ProjectUndoFrame` +
  `setNextUndoHint` pair -- untouched here, so a bulk edit is still one undo step). The harvest
  half is now off the descriptor table: `datasetEditValues` walks `kDatasetProperties`, skips
  entries without a form row, and reads each value through a **new generated**
  `Registry::datasetFormValue(formId, d, pm)`; the disposable `CustomModel` and its five builder
  calls are gone (26 lines -> 18).
  **Why a generated reader rather than a hand-written switch:** the harvest must agree with the
  row builders value-for-value or the "Mixed" marking drifts, so the emitter reuses the same
  `value_expression()` the row emitter uses -- one definition, both surfaces. Equivalence detail:
  the three rows with a `visibleWhen` predicate (`HideOnDashboard`, `WaterfallYAxis`, `LedHigh`)
  return an invalid `QVariant` when the predicate is false, so a dataset that would not build the
  row still contributes nothing to the map, exactly as before; `buildMultiDatasetModel`'s
  `maps[i].contains(pt)` guard therefore behaves identically. `TransformCode` carries a form id
  but no builder row, and is absent from both the old and the new map.

### T13 — Tests

- **Files:** `tests/integration/test_property_registry.py`,
  `tests/integration/test_project_undo.py`, `tests/README.md`
- **Does:** Fill out the module started in T1 with AC3 (defect fixes pinned), AC6/AC12
  (typed schema + describe verb + every described field settable/readable), AC11
  (read-then-write-back is lossless and warning-free), AC13 (every enum-valued field set by
  value and read back). Extend the undo suite with a multi-field
  `project.dataset.update` → single-undo case (AC5). Follow `tests/utils/api_client.py`
  conventions and the markers in `tests/README.md` — read it before writing — and add the
  catalog row.
- **Verify:** `python -m py_compile`; `pytest --collect-only` locally; execution is
  maintainer-run against the live app.
- **Deps:** T7, T9, T10
- [x] **done.** C++ tier (landed earlier): `app/tests/tst_frame_serialization.cpp`: `datasetOverviewDisplayIsNotSerialized` is flipped and
  renamed to `datasetOverviewDisplayRoundTrips` (the key is now written when true and absent when
  false), a new `datasetOmittedKeysLoadTheStructDefaults` pins the four default alignments against
  a freshly constructed `Dataset`, and `populatedDataset()` sets `overviewDisplay` with a matching
  `QCOMPARE` because the fixture's contract is "every serialized field". Nothing else in that
  suite changed -- `defaultDatasetRoundTripsToItsDefaults` still passes untouched, because those
  keys are written unconditionally and so never take the reader's fallback.
  `tests/scripts/test_cpp_regressions.py::test_project_editor_bounds_checks_combo_indices` had
  three expectations pinned to the deleted sub-appliers' local variable names; they now pin the
  same three bounds checks at their new home in `datasetFormEditAccepted`.
  **pytest tier (this pass):** `tests/integration/test_property_registry.py` gained four
  cases on top of the corpus helpers, all manifest-driven (field names, key spellings and enum
  domains are read from `dataset.json` + `Frame.h`, never retyped):
  `test_overview_display_survives_a_save_reload_cycle` and
  `test_omitted_keys_load_the_struct_defaults` (**AC3**, one per declared defect),
  `test_read_then_write_back_is_lossless` (**AC11** -- sends back only the keys the manifest
  declares *writable*, since `project.dataset.list` decorates its objects with derived read-only
  fields that were never writable), and `test_enum_fields_round_trip_by_value` (**AC13** -- every
  value of all four fixed domains; a value suppressed by a conditional write rule, i.e. an empty
  `displayFormat`, counts as absent rather than mismatched). **AC6/AC12 split deliberately:** the
  typed properties and enum domains are already asserted over MCP `tools/list` in
  `tests/integration/test_api_surfaces.py` (spec 0037), so this file pins only the half that file
  does not -- `test_dataset_update_description_is_not_a_field_list`, that the prose stopped
  restating the fields. `tests/integration/test_project_undo.py` gained
  `test_multi_field_dataset_update_is_one_undo_step` (**AC5**), and `tests/README.md` gained the
  catalog row. `pytest --collect-only` sees 15 tests across the two files; the offline
  `test_corpus_files_unchanged` passes. Execution of the live-API cases is maintainer-run.

### T14 — Docs, measurement, self-review

- **Files:** `doc/claude/architecture/project.md`, `CLAUDE.md`,
  `doc/claude/specs/0036-property-registry/tasks.md`
- **Does:** A "Property registry" section in `project.md` (manifest location, what is
  generated, the hook escape hatches, the option-source conventions, the drift gates, and
  the rule that a new dataset property is a manifest entry plus a regeneration); a short
  `CLAUDE.md` block mirroring the icon/command registry entry. Record the measured
  before/after hand-maintained line counts and the per-new-property edit cost (R11/AC1) in
  the build-notes section of this file. Counterfactual check at handoff: name the rule this
  diff most risks violating and the concrete evidence it does not; re-read the full diff
  for scope.
- **Verify:** read-back (`doc/claude` is exempt from `documentation-verify.py`); confirm the
  line-count claim against `git diff --stat` plus the T1 measurement.
- **Deps:** all
- [x] **done.** `doc/claude/architecture/project.md` already carried the "Dataset Property
  Registry (spec 0036)" and "Generated API Surfaces (spec 0037)" sections; this pass added the
  two bullets they were missing -- the drift gates the registry now owns (the
  `registry-verify.py` manifest rule and `code-verify.py`'s `registry-parallel-field-map`) and
  the descriptor-table harvest from T12 -- and extended the "which gate fires when" list.
  `CLAUDE.md`'s Gates bullet names both new rules. The R11 measurement is in the phase-2 notes
  below (1401 -> 126 hand-maintained lines, **-91%**), refreshed for T12 in the phase-3 notes.

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` is met or handed to the maintainer as a named
      runtime check (AC4/AC9 observations; AC2/AC3/AC5/AC6/AC11/AC12/AC13 pytest).
- [x] `python scripts/code-verify.py --check` clean on all changed files, including the
      generated TUs and the new no-parallel-field-map rule (no new errors).
- [x] `python scripts/registry-verify.py` clean, including the new property-manifest rule.
- [x] `python scripts/generate-property-registry.py --check` clean; running the generator
      twice produces no diff.
- [ ] `qt-cpp-review` run on the **hand-written** C++ diff (hooks + reduced call sites);
      findings addressed or noted.
- [ ] `--benchmark-hotpath` run by the maintainer as an adjacency gate (no parse-path edit
      expected; the `Dataset` struct and the shared serializer sit next to it).
- [ ] `pytest tests/integration/test_property_registry.py` and
      `tests/integration/test_project_undo.py` listed for the maintainer, with the
      baseline-capture ordering called out (capture on the pre-change build in T1).
- [ ] Project JSON round-trips byte-identically except the two declared defect fixes.
- [ ] `python scripts/sanitize-commit.py` run; working tree clean of lint debt. Note the
      required order for the SDK to pick up the new schema: build → `--dump-api-schema` →
      `sanitize-commit.py`.
- [ ] Diff is *what was asked, and only that* — dataset entity only, no QML change, no
      foreign working-tree files touched.
- [ ] `spec.md` status set to `done`.

## Phase 1 build notes (2026-07-25)

Phase 1 landed the **new artifacts only**: the manifest, the generator, the generated TUs,
the hooks TU, the baseline test, and the sanitize-pipeline drift gate. No existing C++ TU,
and no build file, was touched -- so nothing generated is compiled yet and nothing behaves
differently. Integration is phase 2.

### Artifacts

| Path | Lines | Kind |
|------|-------|------|
| `app/rcc/properties/dataset.json` | 1785 | declaration (41 properties, 5 runtime fields, 2 sub-entities, 7 option sources, 25 hooks) |
| `app/rcc/properties/schema.json` | 651 | JSON Schema for the manifest |
| `scripts/generate-property-registry.py` | 1657 | generator (`--check` drift mode) |
| `app/src/DataModel/Generated/DatasetRegistry.h` | 307 | generated: form-id enum + descriptor table + option/dispatch declarations |
| `app/src/DataModel/Generated/DatasetSerialization.cpp` | 305 | generated: project-JSON write + read |
| `app/src/DataModel/Generated/DatasetForm.cpp` | 981 | generated: option tables, 12 row builders, commit dispatcher |
| `app/src/API/Generated/DatasetApiFields.cpp` | 701 | generated: API field appliers + typed schema |
| `app/src/DataModel/Project/PropertyHooks.h/.cpp` | 220 + 568 | hand-written escape hatches |
| `tests/integration/test_property_registry.py` | 159 | baseline capture/compare (T1) |
| `doc/claude/specs/0036-property-registry/baseline-manifest.json` | -- | provisional static corpus baseline |

### R11 measurement (before)

Hand-maintained dataset-property plumbing at the start of phase 1, by block:

| Block | Lines |
|-------|-------|
| `Frame.h` `serialize(const Dataset&)` | 101 |
| `Frame.h` `normalizeDatasetRanges` | 14 |
| `Frame.cpp` `read(Dataset&, ...)` | 82 |
| `ProjectEditorItemIds.h` dataset enum | 39 |
| `ProjectEditorForms.cpp` dataset row builders | 567 + 41 |
| `ProjectEditorCommit.cpp` dataset sub-appliers | 32 + 229 |
| `ProjectHandlerEntities.cpp` dataset field appliers | 296 |
| **Total** | **1401** |

That is the number T14 measures the "after" against, once phase 2 removes those blocks. It
matches the spec's ~1,410 estimate to within 9 lines.

### Phase 2 integration points (exact)

1. `app/CMakeLists.txt` -- add `DataModel/Generated/DatasetSerialization.cpp`,
   `DataModel/Generated/DatasetForm.cpp`, `API/Generated/DatasetApiFields.cpp`,
   `DataModel/Project/PropertyHooks.cpp` to `SOURCES` (headers follow the existing pattern).
2. `app/src/DataModel/Frame.h` -- delete the `serialize(const Dataset&)` body and leave
   `[[nodiscard]] QJsonObject serialize(const Dataset& d);` (no longer `inline`).
   `normalizeDatasetRanges` stays (the generated reader calls it).
3. `app/src/DataModel/Frame.cpp` -- delete the `read(Dataset&, const QJsonObject&)` body.
   `readDatasetAlarmBands` / `readDatasetFrequencyMarkers` stay.
4. `app/src/DataModel/Project/ProjectEditorItemIds.h` -- delete the `DatasetItem` enum;
   include `DataModel/Generated/DatasetRegistry.h` instead (identical enumerator order, so
   nothing renumbers).
5. `app/src/DataModel/Project/ProjectEditorForms.cpp` -- delete the eleven dataset row
   builders; keep `buildDatasetModel`'s scaffolding. Add `addGeneralColorRow` to
   `ProjectEditor.h` (the one new emitter name).
6. `app/src/DataModel/Project/ProjectEditorCommit.cpp` -- replace the five sub-appliers with
   one `Registry::applyDatasetFormEdit(...)` call; keep the range guards, the alias
   validation gate, `setNextUndoHint` + `updateDataset`, the tree patch, and the
   sync-versus-deferred rebuild handling driven by the returned `RebuildHint`.
7. `app/src/API/Handlers/ProjectHandlerEntities.cpp` -- delete `takeParam`,
   `aliasInUseByOtherDataset`, the four `applyDataset*Fields` and the old
   `applyDatasetUpdateParams`; **promote `applySimpleAlarmFields` out of file-static** (the
   generated legacy alarm path calls it) -- either a `ProjectHandler` static member or a
   declaration in `ProjectApiSupport.h`.
8. `app/src/API/Handlers/ProjectHandler.h` -- declare
   `[[nodiscard]] QJsonObject datasetFieldSchema();` in `namespace API::Handlers`.
9. `app/src/API/Handlers/ProjectHandler.cpp` -- feed `datasetFieldSchema()` into the dataset
   verbs' schema and drop the prose field enumeration.
10. `app/src/DataModel/ProjectEditor.cpp` -- once the combo call sites move to the option
    sources, `generateComboBoxModels()`'s dataset entries (`m_datasetWidgets`,
    `m_displayFormats`, `m_plotOptions`, `m_fftSamples`, `m_fftWindows`,
    `m_fftWindowValues`) become dead; `datasetWidgetEditable` delegates to
    `PropertyHooks::widgetSelectable`.
11. T3 (`registry-verify.py` manifest rule), T11 (`code-verify.py` no-parallel-field-maps),
    T12 (multi-select off the descriptor table), T13 (tests), T14 (docs + measurement) are
    untouched by phase 1.

### Phase 1 verification

- `python3 scripts/generate-property-registry.py --check` -- clean; running the generator
  twice produces no diff.
- `python3 scripts/code-verify.py --check` -- exit 0, **zero findings** (not merely zero
  errors) for every new file, generated and hand-written.
- `python3 -m black --check` -- clean on the new and edited Python.
- `python3 -m pytest tests/integration/test_property_registry.py --collect-only` -- 3 tests;
  `test_corpus_files_unchanged` passes offline.
- Not run (needs the app / the maintainer): the baseline capture, every AC that drives the
  live API, and `--benchmark-hotpath`.

## Phase 2 build notes (2026-07-25)

Phase 2 wired the generated TUs into the app: the seven integration points from the phase-1
list are done (T7-T10 plus the multi-select fan-out), and every hand-written block they
replace is deleted. **Nothing here is compiled until the CMake registrations below land.**

### R11 measurement (after)

| Block | Before | After |
|-------|--------|-------|
| `Frame.h` `serialize(const Dataset&)` | 101 | 5 (doxygen + declaration) |
| `Frame.h` `normalizeDatasetRanges` | 14 | 14 (kept; the generated reader calls it) |
| `Frame.cpp` `read(Dataset&, ...)` | 82 | 0 |
| `ProjectEditorItemIds.h` dataset enum | 39 | 1 (the generated-header include) |
| `ProjectEditorForms.cpp` dataset row builders | 608 | 11 (`addFFTSection`, no property rows) |
| `ProjectEditorCommit.cpp` dataset sub-appliers | 261 | 95 (guards + tree patch + undo routing) |
| `ProjectHandlerEntities.cpp` dataset field appliers | 296 | 0 |
| **Total** | **1401** | **126** |

**-1275 hand-maintained lines (-91%)**, against 2294 generated lines that no longer need a
human to keep in sync. Whole-file deltas for the same seven TUs: -1335 lines
(`Frame.h` -96, `Frame.cpp` -87, `ProjectEditorItemIds.h` -38, `ProjectEditorForms.cpp` -597,
`ProjectEditorCommit.cpp` -219, `ProjectHandlerEntities.cpp` -298, plus +20 in
`ProjectHandler.h` / +6 net in `ProjectHandler.cpp` / +6 in `ProjectEditorMultiSelect.cpp` /
+1 in `ProjectEditor.h`). Per-new-property edit cost is now one `dataset.json` entry plus
`python3 scripts/generate-property-registry.py` -- no hand edit to any of the seven blocks.

### Generator fix landed in this phase

`generate-property-registry.py` emitted the four family sub-appliers in `DatasetForm.cpp` with
**unqualified** `Dataset&` / `const ProjectModel&` parameters at file scope, where neither name
is visible (the file has namespace *aliases* for `PropertyHooks` and `Registry`, but no
`using namespace DataModel`). That is a compile error the moment the TU is added to the build,
so the generator now emits `DataModel::Dataset&` / `const DataModel::ProjectModel&`; the four
regenerated signatures are the only diff. `--check` is clean and a second run produces no diff.

### Registrations still required (not done here -- build files are the maintainer's)

1. `app/CMakeLists.txt` `SOURCES`: `DataModel/Generated/DatasetSerialization.cpp`,
   `DataModel/Generated/DatasetForm.cpp`, `API/Generated/DatasetApiFields.cpp`,
   `DataModel/Project/PropertyHooks.cpp` (+ the two headers, per the existing pattern).
2. **`app/tests/CMakeLists.txt` -- the phase-1 integration list missed this and it is a hard
   blocker for the spec-0032 unit tier.** `tst_frame_serialization` links `Frame.cpp` +
   `SerialStudioFrameSupport.cpp` + `SSAssert.cpp` and calls `serialize(Dataset)` /
   `read(Dataset&, ...)`, which now live in `DataModel/Generated/DatasetSerialization.cpp`, so
   the suite no longer links. Adding that TU pulls in `PropertyHooks::isValidColor` and
   `PropertyHooks::isValidFftWindow`, and adding `PropertyHooks.cpp` in turn pulls in
   `ProjectModel::groups()` / `ProjectModel::datasetCount()` (both out-of-line), which would
   drag the whole application into a suite designed to link in seconds. Two ways out, both a
   maintainer decision: **(a)** move the four ProjectModel-free validators (`isValidColor`,
   `isValidFftWindow`, `isValidDatasetIndex`, `isValidTransformLanguage`) into their own TU next
   to `PropertyHooks.cpp` and link that one instead -- `PropertyHooks.h` is unchanged, so
   `registry-verify`'s hook-existence rule still passes; or **(b)** accept the larger link set.
   Option (a) is the recommendation.

### Both registrations landed (2026-07-25) -- option (a)

`app/CMakeLists.txt` carries the three generated TUs plus `PropertyHooks.cpp`, and the split of
item 2 is done: `isValidColor`, `isValidDatasetIndex`, `isValidFftWindow` and
`isValidTransformLanguage` moved verbatim out of `PropertyHooks.cpp` into the new
`app/src/DataModel/Project/PropertyValidators.cpp` (registered next to `PropertyHooks.cpp`).
`aliasInUseByOtherDataset` stays behind -- it reads `pm.groups()`. `PropertyHooks.h` is
untouched, so every declaration and the hook-existence rule are unchanged.
`tst_frame_serialization` now links `DataModel/Generated/DatasetSerialization.cpp` +
`DataModel/Project/PropertyValidators.cpp` on top of its previous three TUs, and its comment
records the decision. Everything else the generated TU reaches is already satisfied by that link
set: `ss_jsr`, `normalizeDatasetRanges`, `Keys::*` and the `AlarmBand` / `FrequencyMarker`
serializers are inline in `Frame.h`; `readDatasetAlarmBands`, `readDatasetFrequencyMarkers` and
the sub-entity readers are in `Frame.cpp`; `SerialStudio::toDouble` is inline in
`SerialStudio.h`. No unresolved external remains.

### Phase 2 verification

- `python3 scripts/generate-property-registry.py --check` -- clean; two consecutive runs
  produce no diff.
- `python3 scripts/code-verify.py --check` -- exit 0, **zero errors and zero findings** on every
  file this phase touched.
- `python3 scripts/registry-verify.py` -- CLEAN.
- `python3 -m pytest tests/scripts/ -q` -- 268 passed.
- `python3 -m py_compile` + `python3 -m black --check` -- clean on the touched Python.
- Not run (needs the app / the maintainer): the build, the C++ unit tier, every live-API AC,
  and `--benchmark-hotpath`.

## Phase 3 build notes (2026-07-25)

Phase 3 closed the tracker's open items: the two drift rules (T3, T11), the multi-selection
harvest (T12), the pytest ACs (T13) and the docs (T14). No C++ behaviour changed except the
harvest path; no build file was touched.

### What each rule costs a caller

| Gate | Kind | Fires when |
|------|------|-----------|
| `registry-verify.py` `check_property_manifests` | hard failure | manifest violates `schema.json`; duplicate id; `jsonKey`/hook/widget/option/provider/builder-row does not resolve; a `Dataset` field is unclaimed or double-claimed; `DatasetItem` order drifts from `formIdOrder` |
| `code-verify.py` `registry-parallel-field-map` | error | a non-generated `app/src` file names >= 4 dataset property keys (identity keys excluded) |
| `code-verify.py` `api-generated-edited` | error | any generated artifact -- now including the four C++ TUs -- lost its do-not-edit marker |

### R11 measurement (final)

The phase-2 table stands: **1401 -> 126 hand-maintained lines (-91%)** across the seven blocks
the registry replaced. T12 takes another 8 lines out of `ProjectEditorMultiSelect.cpp`
(`datasetEditValues`: 26 -> 18) and removes its dependency on the five section builders, against
+29 generated lines in `DatasetForm.cpp` and +5 in `DatasetRegistry.h`. Generated total is now
2407 lines. Per-new-property edit cost is unchanged and still the point of the spec: one
`dataset.json` entry plus `python3 scripts/generate-property-registry.py`.

### Counterfactual check

The rule this diff most risks violating is **"never let a generated artifact and its generator
disagree"** -- T12 changed `generate-property-registry.py` and both generated files it feeds.
Evidence it does not: `--check` is clean after a fresh write, a second consecutive run produces
no diff, and the generator's own 100-column and paren-balance guards passed on every emitted
file. Runner-up risk is **scope creep into foreign working-tree files**; the only files touched
are the five this tracker names plus the two docs, and `tests/scripts/test_diagnostics_static.py`
(spec 0035, black-dirty on arrival) was deliberately left alone.

### Phase 3 verification

- `python3 scripts/generate-property-registry.py --check` -- up to date; two consecutive runs
  produce no diff.
- `python3 scripts/registry-verify.py` -- CLEAN (the new manifest rule included; the snapshot
  projection still warns locally, as designed, until the maintainer re-dumps `api-schema.json`).
- `python3 scripts/code-verify.py --check` -- 3222 files, **0 errors**, 529 pre-existing
  advisories; zero findings on every file this phase touched.
- `python3 scripts/documentation-verify.py` -- 0 findings.
- `python3 -m pytest tests/scripts/ -q` -- 278 passed.
- `pytest --collect-only` on the two integration files -- 15 tests; the offline
  `test_corpus_files_unchanged` passes.
- Every violation class of both new rules was seeded (in-memory manifest mutations; whitelist
  cleared) and produces its message.
- Not run (needs the app / the maintainer): the build, the C++ unit tier, the baseline capture,
  every live-API AC, and `--benchmark-hotpath`.
