---
spec: 0036-property-registry
phase: plan
status: draft        # draft -> approved (gate before /ss-tasks)
updated: 2026-07-25
---

# Plan 0036 — Property registry (declare once, derive everywhere)

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Gate: do not start `/ss-tasks` until a human marks this
> `approved`.

## Approach (one paragraph)

A **JSON manifest** (`app/rcc/properties/dataset.json`) declares every persisted/editable
dataset property once, and a new **`scripts/generate-property-registry.py`** emits four
checked-in, compiled C++ translation units from it: the descriptor table, the project-JSON
serializer/deserializer pair, the editor form-row builders + commit dispatcher, and the
API field appliers + typed schema properties. This is spec 0028's shape — declare in JSON
under `app/rcc/`, generate C++, gate drift with `--check` in `sanitize-commit.py` and a
new rule function in `registry-verify.py` — with one simplification: because the generated
artifacts are real C++ TUs rather than JSON read at runtime, `tr()` works natively and no
lupdate stub is needed. Generation is *replacement in place*: the generated functions keep
their current names and signatures (`DataModel::serialize(const Dataset&)`,
`DataModel::ProjectEditor::addGeneralSection(...)`, `API::Handlers::applyDatasetUpdateParams(...)`),
so every existing caller, the ADL-based serializer composition, and the undo choke point
are untouched. Anything that resists declaration — cross-entity validation, commit-time
side effects, live option providers, structural rows — is referenced by *name* from the
manifest and implemented once in a small hand-written hooks TU. Two rejected alternatives:
a C++ `constexpr` descriptor table (the Python generator would have to parse C++ to emit
the API schema and the R6 surfaces, and translations/validation expressions get awkward),
and a runtime reflective property bag (destroys the packing-optimized hotpath structs for
zero benefit to a build-time problem).

## Affected subsystems & files

| File | Change |
|------|--------|
| `app/rcc/properties/dataset.json` (new) | The declaration: ~41 property entries + option-source definitions + section order. Not added to `rcc.qrc` in v1 (build-time only; see tradeoffs). |
| `app/rcc/properties/schema.json` (new) | JSON Schema for the manifest itself, so `registry-verify.py` validates shape rather than duck-typing. |
| `scripts/generate-property-registry.py` (new) | Reads the manifest, emits the four generated TUs. `--check` = byte-compare, no write, exit 1 on drift. Deterministic ordering, `newline=""` LF policy, same header/marker convention as `generate-command-strings.py`. |
| `app/src/DataModel/Generated/DatasetRegistry.h` (new, generated) | `DataModel::Registry::DatasetProperty` descriptor struct + `constexpr` table (id, field, jsonKey, apiName, type, default, section, widget kind, flags) + the `DatasetItem` form-id enum, replacing the hand-written block in `ProjectEditorItemIds.h`. |
| `app/src/DataModel/Generated/DatasetSerialization.cpp` (new, generated) | `DataModel::serialize(const Dataset&)` and `DataModel::read(Dataset&, const QJsonObject&)` — moved out of `Frame.h`/`Frame.cpp`, same names, same ADL behavior. |
| `app/src/DataModel/Generated/DatasetForm.cpp` (new, generated) | The `ProjectEditor` dataset row builders and the commit dispatcher/sub-appliers, as member functions of `DataModel::ProjectEditor` so `tr()` keeps its existing context. |
| `app/src/API/Generated/DatasetApiFields.cpp` (new, generated) | `applyDatasetUpdateParams` + per-group appliers + the `consumed`-set bookkeeping + the typed schema property list for the dataset verbs. |
| `app/src/DataModel/Project/PropertyHooks.h` / `.cpp` (new, hand-written) | Named escape hatches: validators (`datasetAlias`, `color`, `transformLanguage`), visibility predicates (`insidePainterGroup`, `waterfallEnabled`, `widgetRangeApplicable`), option providers (`xAxisSources`, `waterfallYSources`, `datasetWidgets`, `plotOptions`, `fftWindows`, `fftSampleCounts`, `displayFormats`), and commit side effects (`onWidgetChanged`, `onVirtualChanged`, `onXAxisChanged`, `onAliasRejected`). One implementation each; every surface calls the same one. |
| `app/src/DataModel/Frame.h` | Remove the hand-written `serialize(const Dataset&)` body and the dataset half of `normalizeDatasetRanges`; declare them instead. `Keys::` block untouched. |
| `app/src/DataModel/Frame.cpp` | Remove the hand-written `read(Dataset&, ...)` body; `readDatasetAlarmBands` / `readDatasetFrequencyMarkers` stay hand-written (nested entities, out of v1 scope) and are called from the generated reader via declared `subEntity` hooks. |
| `app/src/DataModel/Project/ProjectEditorItemIds.h` | Dataset id enum deleted; the header includes the generated one. Other entities' enums untouched. |
| `app/src/DataModel/Project/ProjectEditorForms.cpp` | Dataset row builders removed (lines ~904-1494, ~1627-1663); `buildDatasetModel` keeps the structural scaffolding (section headers, sub-editor launch rows) and calls the generated emitter. |
| `app/src/DataModel/Project/ProjectEditorCommit.cpp` | The five dataset sub-appliers and the type-conversion half of `onDatasetItemChanged` removed; the slot keeps hint/tree/emit orchestration and calls the generated dispatcher. |
| `app/src/API/Handlers/ProjectHandlerEntities.cpp` | The four `applyDataset*Fields` functions and the dataset half of `takeParam` removed; `datasetUpdate` keeps resolve/respond and calls the generated applier. |
| `app/src/API/Handlers/ProjectHandler.cpp` | Dataset verb schema switches from `makeSchema({groupId, datasetId})` + prose blob to the generated typed property list; the prose keeps only the guidance paragraphs that are not field enumeration. |
| `scripts/registry-verify.py` | New `check_property_manifests(errors)` rule function + one call line in `main()`: manifest validates against `schema.json`, ids unique, `jsonKey` resolves to a real `Keys::` constant, every declared hook name exists in `PropertyHooks.h`, no field declared twice, every `Dataset` struct field is either declared or explicitly listed as runtime-only. |
| `scripts/code-verify.py` | New rule: a hand-written `Keys::` dataset key referenced outside the generated TUs and `PropertyHooks.cpp` is an error (prevents re-growing a parallel field map). |
| `scripts/sanitize-commit.py` | One `run_python_step` block for the generator, inserted between the SDK step and the command-strings step (the SDK reads `api-schema.json`, which the maintainer re-dumps after a registry change — ordering noted in the header comment). |
| `app/CMakeLists.txt` | Register the four generated TUs + `PropertyHooks.cpp` in `set(SOURCES ...)`. |
| `tests/integration/test_property_registry.py` (new) | AC2/AC3/AC6/AC11/AC12/AC13 coverage. |
| `tests/integration/baselines/` (new) | Checked-in `project.exportJson` baselines for the `examples/` corpus, captured before the change. |
| `tests/README.md` | Catalog row for the new test file. |
| `doc/claude/architecture/project.md` | New "Property registry" section (implement phase). |
| `CLAUDE.md` | New short section pointing at the registry, mirroring the icon/command registry block. |

## Architecture & data flow

**Declaration.** One manifest entry per property, e.g.:

```
{
  "id": "PltMin",                 -> kDatasetView_PltMin, DatasetProperty::PltMin
  "field": "pltMin",              -> Dataset::pltMin
  "jsonKey": "PltMin",            -> Keys::PltMin  (spelled "plotMin" on disk)
  "apiName": "pltMin",            -> API param name; "same" means use jsonKey's string
  "apiAliases": ["plotMin"],      -> R12: accept the disk spelling too
  "type": "double",
  "default": 0,
  "scope": "document",            -> document | runtime (runtime = live-frame only)
  "persist": "always",            -> always | whenNonDefault | never
  "section": "range",
  "widget": "FloatField",
  "label": "Minimum Value",
  "description": "...",
  "placeholder": "0",
  "enabledWhen": "widgetRangeApplicable",
  "coalesce": true,
  "rebuildTree": false,
  "validate": { "pairMinMax": "PltMax" }
}
```

Four generated consumers read the same entry:

1. **Serializer** — `persist` drives whether the write is unconditional or guarded on
   `!= default`; `jsonKey` names the `Keys::` constant; `scope: runtime` is skipped for
   document writes. Reader emits `ss_jsr(obj, Keys::X, <default>)` with the *manifest*
   default, which is by construction the same value as the struct initializer — this is
   what closes the four default-drift defects. `legacyKeys` on an entry emits the
   read-side fallback (`Keys::Min`/`Keys::Max`); `validate.pairMinMax` emits the existing
   swap-if-inverted normalization.
2. **Form** — one uniform ~9-call row emitter per entry, in `section` then declaration
   order. `visibleWhen`/`enabledWhen` resolve to `PropertyHooks` predicates taking
   `(const Dataset&, const ProjectModel&)`. `options` resolves to an `OptionSource`
   (below).
3. **Commit** — a generated `switch` over the id enum converting the row's variant to the
   field type; entries with `onCommit` call the named hook *after* the field write, and
   the hook owns the side effect (form rebuild, cross-field mutation, deferred rebuild).
   The generated dispatcher writes into a `Dataset&` and returns whether a rebuild is
   needed; it never calls `ProjectModel` itself.
4. **API** — a generated applier per entry: `takeParam` on `apiName` and each `apiAlias`,
   type coercion (`SerialStudio::toDouble` for numerics, per the existing convention),
   declared clamps, named validators returning error strings, and `rebuildTree`
   accumulation. The `consumed` set is seeded and the unknown-field warning emitted by the
   generated wrapper, so it can no longer be forgotten. The same table emits the typed
   `properties`/`required` block for the dataset verbs' schema.

**Option sources.** The editor stores a *positional index* in the row's editable value, and
today each combo hand-rolls a forward search and a reverse lookup, under four incompatible
conventions. The registry introduces one `OptionSource` abstraction in `PropertyHooks.h`
with four adapters, and every combo declares which it uses:

- `staticMap` — key→label map, stored value is the key (`datasetWidgets`, `displayFormats`).
- `parallelValues` — labels list + values list, stored value is the value (`fftWindows`,
  `fftSampleCounts`).
- `liveProvider` — labels + values computed from current project state (`xAxisSources`,
  `waterfallYSources`).
- `tuple` — one row drives multiple fields (`plotOptions` → `plt` + `log`); declared as a
  single entry with a `fields` list and a tuple option source.

Each adapter exposes `indexForValue()` / `valueForIndex()`, so the generated form and
commit code contain no search loops, and the API emits the *value* domain (not indices) as
its schema enum.

**Undo routing — the choke point is unchanged.** Nothing generated touches `m_groups`
directly and no new `ProjectModel` slot is created. All three write paths converge on the
existing whole-struct update:

- Editor: generated commit mutates the `ProjectEditor`'s `m_selectedDataset` copy, then
  the hand-written `onDatasetItemChanged` calls
  `ProjectModel::setNextUndoHint(label, coalesceKey)` — label and key both produced from
  the manifest entry (`coalesce: true` yields a per-field key
  `"dataset:<groupId>:<datasetId>:<id>"`; `coalesce: false` yields an empty key so the step
  never merges) — followed by
  `ProjectModel::updateDataset(groupId, datasetId, dataset, rebuildTree)`.
- API: generated applier patches a local `Dataset` copy; `datasetUpdate` calls the same
  `ProjectModel::updateDataset(...)`. Batches remain one step through the existing
  `DataModel::ProjectUndoFrame` opened in `API::CommandRegistry::execute()`.
- Multi-select: unchanged; the existing `ProjectUndoFrame` + `setNextUndoHint` pair in
  `ProjectEditorMultiSelect.cpp` still wraps the fan-out.

`ProjectModel::updateDataset` already opens
`const ProjectUndoScope undo_scope{*this, tr("Edit Dataset")}`; the two-phase capture then
runs `ProjectHistory::stageCapture()` on scope entry and `ProjectHistory::commitPending()`
from `ProjectModel::setModified(true)`. Because no generated code introduces a
`setModified(true)` outside that funnel, `code-verify.py:undo-scope-missing` keeps passing
unchanged, and the R1 guarantee is inherited rather than re-implemented.

**Multi-selection simplification (free win).** `ProjectEditorMultiSelect.cpp` currently
builds a throwaway form model just to harvest a `ParameterType → value` map. With the
descriptor table it reads the table directly; the harvest hack is deleted.

## Hotpath & threading impact

- **Touches the hotpath?** **No.** `FrameReader`, `CircularBuffer`, `FrameBuilder`, the
  span fast lane, and the Dashboard draw path are untouched. The `Dataset` struct is not
  modified — no member added, no reorder, no indirection; the `alignas(8)` +
  `static_assert(sizeof % alignof == 0)` invariants are unchanged, and the generator is
  forbidden from emitting anything that changes the struct. The descriptor table is
  `constexpr` and is consulted only by editor/API/serialization code, never per frame.
  One adjacency to flag: `serialize(const Dataset&)` is shared between project-file writes
  and live API frame broadcasts, so it moves TUs — hence the `scope` field, and hence
  `--benchmark-hotpath` is run as a regression gate even though no parse code changes.
- **New cross-thread signal/slot?** No. Everything (ProjectModel, ProjectEditor, API
  command execution) is main-thread, as today.
- **New input to a cached hotpath flag?** No. No new flags; `m_changeDriven`,
  `m_streamAvailable`, `m_operationMode` see registry-driven edits as ordinary project
  edits, through the same `updateDataset` → `groupsChanged` → sync route.
- **Timestamp ownership** — untouched; no frame data or driver boundary involved.

## Data model & persistence

- **No new `Keys::` entries, no schema-version bump, no migration.** The manifest
  references existing `Keys::` constants by their C++ identifier; `registry-verify.py`
  fails if a referenced constant does not exist.
- **Intended output deltas (the only ones).** Two defect fixes change bytes on save:
  `overviewDisplay` is now written (it is read and API-settable today but never
  serialized), and the read-side defaults for `fftSamples`, `fftSamplingRate`, `ledHigh`,
  and `index` become the struct defaults. Older Serial Studio versions ignore unknown keys,
  so the `overviewDisplay` addition is forward-safe; the default alignment only affects
  files that *omit* the key, which today load into a state a freshly created dataset never
  has. The round-trip baseline test asserts exactly this delta set and nothing else.
- **Legacy aliases preserved.** `Keys::Min`/`Keys::Max` remain read-only fallbacks for the
  three min/max pairs, declared as `legacyKeys` on those entries; `alarmLow`/`alarmHigh`/
  `alarmEnabled` remain read-only v3.3 migration inputs and stay in the hand-written
  `readDatasetAlarmBands` (nested entity, out of v1 scope).
- Nested entities (`alarmBands`, `fftMarkers`) stay hand-written and are invoked from the
  generated reader/writer through declared `subEntity` hooks — they are collections of a
  different entity, and folding them in is a later pass.

## API / SDK surface

- No new commands and no renames. `project.dataset.update` / `.add` / `.getBy*` keep their
  ids and behavior.
- The dataset verbs' schema gains typed `properties` for every declared field (type,
  description, enum domain where applicable). The prose description keeps its *guidance*
  paragraphs (when to use `virtual`, what `index` means, the options bitflags) and drops
  the field enumeration, which is now machine-readable.
- **R12:** every property whose `apiName` differs from its `jsonKey` also accepts the
  `jsonKey` spelling via `apiAliases`, closing the read-then-write-back loss. Writes still
  emit the canonical name.
- **SDK follow-through:** `api-schema.json` is a runtime dump
  (`SerialStudio --dump-api-schema`), and `generate-sdk.py` builds `SerialStudio.js/.lua`
  from it. Once the schema declares the fields, the SDK generator emits usable setters with
  no change to `generate-sdk.py`. The maintainer must re-dump the schema after building, in
  the order: build → `--dump-api-schema` → `sanitize-commit.py`.
- Commercial surfaces unchanged: Pro-gated properties (`waterfall` and friends) carry a
  `pro: true` flag and the generator emits the same `#ifdef BUILD_COMMERCIAL` guards used
  today.

## QML / UI

**No QML change.** Every project-editor form is already rendered by one generic delegate
(`app/qml/ProjectEditor/Views/TableDelegate.qml`) that switches on the row's widget-kind
role; the per-field code is entirely C++. The generated rows carry the same roles, the same
widget-kind vocabulary, and the same positional-index convention for choice fields, so the
delegate is untouched. The hand-written parts of `DatasetView.qml` (the visualization
toggle ribbon, the alarm-band and frequency-marker dialog buttons) are structural and stay
as they are.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Declaration format | JSON manifest; C++ `constexpr` table; X-macro header | **JSON manifest** — matches the 0028 precedent, is directly consumable by the Python generator *and* by R6's schema emitters without a C++ parser, and keeps validation/editor metadata out of a header the hotpath includes. A C++ table would force the generator to parse C++ (or force R6 to duplicate the data), and X-macros are unreadable at 41 properties x 15 attributes. |
| Translations | lupdate stub like 0028; native `tr()` in generated code | **Native `tr()`** — the generated form builders are member functions of `ProjectEditor`, so `tr()` resolves to the existing translation context and every already-translated property string stays valid. 0028 needed a stub only because its manifests are read at runtime. |
| Generated code shape | replace function bodies in place; new API alongside old | **Replace in place** — generated functions keep today's names and signatures, so callers, ADL serializer composition, and the undo choke point are untouched, and the diff is reviewable as "this block moved and became derived". |
| v1 entity scope | dataset only; dataset + group; all entities | **Dataset only** — biggest surface (~1,410 core lines, 41 fields), exercises every hard case (choice fields, live option providers, side effects, Pro gating, legacy keys, nested entities). Group/action/source/output-widget follow mechanically once the emitter is proven. |
| Serialization in v1 | derive it; leave hand-written | **Derive it** — the spec's acceptance explicitly names project JSON as a derived surface, and the default-drift defects live precisely in the gap between the struct and the reader. Risk is bounded by the baseline round-trip corpus. |
| Manifest in `rcc.qrc` | ship it; build-time only | **Build-time only in v1** — nothing reads it at runtime yet. R6/R8 can add the qrc line when they need runtime schema serving; shipping it now is dead weight and an extra binary-size cost for no v1 consumer. |
| Hard cases | inline in the generator; named hooks | **Named hooks** — the generator stays a dumb emitter, the ~10 bespoke rules live once in a hand-written TU with real types and a debugger, and `registry-verify.py` checks every referenced hook exists. |
| Drift verifier | new script; extend `registry-verify.py` | **Extend `registry-verify.py`** — already wired into `sanitize-commit.py`, already has the collect-errors/exit-code shape; a new rule function plus one call line inherits reporting for free. |
| Defect fixes | in this spec; separate pre-commit | **In this spec, asserted explicitly** (open question for the maintainer) — they are the motivating evidence and the baseline test can pin the exact delta; splitting them costs a second baseline capture. |
| Nested entities (`alarmBands`, `fftMarkers`) | derive; keep hand-written | **Keep hand-written** — they are collections of a different entity with their own v3.3 migration; deriving them needs entity nesting the manifest does not have in v1. |

## Risks & mitigations

- **Silent project-file regression.** The highest-consequence risk: a generated serializer
  that differs subtly from the hand-written one corrupts user projects. Mitigation: capture
  `project.exportJson` baselines for the whole `examples/` corpus **before** any code
  change (first task), then assert byte equality modulo the two declared deltas. This test
  is the gate on the serialization task, not an afterthought.
- **Form regression that only shows at runtime.** Rows that vanish, become editable when
  they should not, or lose their combo domain. Mitigation: the manifest is authored by
  *transcribing* the existing builders row by row (not by re-deriving intent), the
  generated file is diffed against the removed code as a task step, and AC4 walks a dataset
  of every widget kind.
- **Choice-field index/value mismatch.** The single most error-prone conversion, with four
  incompatible existing conventions. Mitigation: `OptionSource` adapters land and are
  reviewed *before* any combo property is declared; AC13 sets every enum-valued field by
  value through the API and reads it back.
- **Commit-time reentrancy.** Some edits rebuild the form synchronously, others must defer
  through a zero-timer with a uniqueId re-check; mixing them reenters or drops the edit.
  Mitigation: the generated dispatcher never rebuilds — it returns, and the hand-written
  slot or a named `onCommit` hook owns rebuild timing, preserving today's sync/deferred
  split verbatim.
- **`serialize` is shared with live frame broadcasts.** Declaring a runtime value as a
  document property (or vice versa) changes what the API streams. Mitigation: the `scope`
  field is mandatory; `registry-verify.py` fails if any `Dataset` field is neither declared
  nor listed as runtime-only, so the five runtime fields must be named explicitly.
- **Ctor-closure / composition-root exposure.** `ProjectModel`'s ctor closure is a
  protected surface. Mitigation: the descriptor table is `constexpr` with no dynamic
  initialization and no singleton reach; `PropertyHooks` option providers take the model by
  reference from their caller rather than calling `instance()`. Any hook that would need a
  singleton is a review stop.
- **Generated-code review fatigue.** ~1,400 lines of generated C++ in one diff. Mitigation:
  land it as one task per surface (table → serialization → form → commit → API), each
  independently verifiable, with the removed hand-written block quoted in the task.
- **`code-verify.py` style rules on generated output.** The generator must emit code that
  passes the linter (100 cols, no in-body comments, `[[nodiscard]]`, brace-free single
  statements, SPDX header). Mitigation: the generator is developed against
  `code-verify.py --check` from its first emission; `clang-format` runs over generated
  files like any other source in `sanitize-commit.py`, so the generator must be
  format-stable (emit already-formatted output, or the `--check` gate will flap against
  clang-format's rewrite — verified as an explicit task step).

## Test & verification plan

- **Unit (I can run):** none — no `tests/scripts/` surface (no JS parser change). The
  generator itself is verified by running it twice and diffing (`AC8`), and by
  `--check` after a seeded manifest edit (`AC7`).
- **Integration (maintainer runs; app up with the API server on 7777):**
  - `tests/integration/test_property_registry.py` (new):
    - **AC2** — for every project under `examples/` and every bundled template:
      `project.open` → `project.exportJson`, byte-compare against the checked-in baseline,
      allowing only the two declared deltas.
    - **AC3** — `overviewDisplay` survives save/reload; a project omitting `fftSamples`,
      `fftSamplingRate`, `ledHigh`, `index` loads to the same values as a freshly added
      dataset.
    - **AC6/AC12** — the dataset verbs' schema lists typed field properties; the
      schema-description verb returns the writable field list; every described field is
      settable and readable.
    - **AC11** — read a dataset, write the returned object back unchanged: no
      `unknown_field` warnings, no value loss.
    - **AC13** — set each enum-valued field by value, read it back unchanged.
  - `tests/integration/test_project_undo.py` (extended, **AC5**): a multi-field
    `project.dataset.update` is one undo step; undo restores every field.
- **Maintainer observations:** **AC4** — dataset forms for plot / FFT / bar-gauge-meter /
  compass / LED / waterfall / virtual / painter-child datasets are field-for-field
  identical to the previous build. **AC9** — translation extraction finds every property
  string in the `ProjectEditor` context, no orphans.
- **Hotpath:** `--benchmark-hotpath` once before commit. No parse-path edit is expected;
  the gate covers the `Dataset` struct / serializer adjacency.
- **Static:** `python scripts/code-verify.py --check` (including the new hard-coded-key
  rule); `python scripts/registry-verify.py` (including the new manifest rule);
  `python scripts/generate-property-registry.py --check`; `qt-cpp-review` on the
  hand-written C++ diff (hooks + the reduced call sites — not the generated bulk);
  `python scripts/sanitize-commit.py` before commit.
