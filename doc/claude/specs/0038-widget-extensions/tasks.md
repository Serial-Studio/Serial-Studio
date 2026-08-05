---
spec: 0038-widget-extensions
phase: tasks
status: draft        # draft -> approved (gate before /ss-implement)
updated: 2026-07-25
---

# Tasks 0038 — Widget-as-extension

> **Phase 3 of 4 — the ordered checklist.** Decompose [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable*. `/ss-implement` works this list top to
> bottom and keeps the status boxes current. Gate: do not start `/ss-implement` until a human
> marks this `approved`.

## Conventions

- One task = one focused, reviewable change. **Verify** is how *this* unit is confirmed before
  moving on; **Deps** lists task IDs that must land first.
- `python scripts/code-verify.py --check <files>` is implied on every task that touches C++ or
  QML and is not repeated in the Verify line unless it is the only check.
- Tasks T1 and T26 are maintainer-run (the agent cannot build or run the app).
- The tree stays conceptually compilable after each task: the enum, the catalog, and the model
  land before anything consumes them, and the two builtin conversions land last.

## Pending build registrations (T2-T9 pass, maintainer)

The phase-1 agent did not edit `app/CMakeLists.txt`. Before the next build, add to the app
target's sources/headers:

- `app/src/UI/WidgetExtensions.h`
- `app/src/UI/WidgetExtensions.cpp`
- `app/src/UI/WidgetExtensionManifest.cpp`
- `app/src/Misc/Problems/ExtensionCheckers.h`
- `app/src/Misc/Problems/ExtensionCheckers.cpp`

`app/rcc/extensions/schema/widget-manifest.json` is already listed in `app/rcc/rcc.qrc`, which
needs no CMake change.

## Pending build registrations (T10-T17 pass, maintainer)

Phase 2 did not edit `app/CMakeLists.txt` either. Add to the app target:

- `app/src/UI/Widgets/ExtensionData.h`
- `app/src/UI/Widgets/ExtensionData.cpp`
- `app/qml/Widgets/Dashboard/ExtensionPlaceholder.qml` (to `QML_SOURCES`; the delegates load it
  by its `qrc:/serial-studio.com/gui/qml/...` URL, so it must ship in the QML module)

## Pending build registrations (T18-T23 pass, maintainer)

Phase 3 did not edit `app/CMakeLists.txt` either. Add to `QML_SOURCES`:

- `app/qml/Dialogs/ExtensionConsent.qml`
- `app/qml/Widgets/Dashboard/ExtensionWidgetSettings.qml`

The two bundled packages ship through `app/rcc/rcc.qrc` (four new `<file>` entries, already
added) and need no CMake change.

**Deferred removals (T22/T23).** The conversions land additively: `Widgets::Compass`,
`Widgets::DataGrid`, their `ModuleManager` QML registrations, and
`app/qml/Widgets/Dashboard/{Compass,DataGrid}.qml` are still compiled in but are now
unreachable, because `builtinReplacement()` wins in `DashboardWidget::setWidgetIndex`.
Deleting them requires `app/CMakeLists.txt` and `app/src/Misc/ModuleManager.cpp`, both of
which this pass was not allowed to touch. Remove together:
`app/src/UI/Widgets/Compass.{h,cpp}`, `app/src/UI/Widgets/DataGrid.{h,cpp}`, the two
`qmlRegisterType` lines, the two `#include`s and two `case` bodies in
`app/src/UI/DashboardWidget.cpp`, the two `QML_SOURCES` entries, and the two old QML files.

## Recorded gap closed in the T18-T23 pass

**Widget-display keys did not round-trip the `ext:<id>:<uid>` token.** T11 made
`Dashboard::applyDisplayTitles` *read* display titles for extension widgets under
`"ext:<id>:<uid>"`, but `ProjectModel` still *wrote* and read the numeric `"<type>:<uid>"` key.
Since every extension widget shares `DashboardExtension = 100`, that key cannot address one
package, so renaming an extension widget silently did nothing and freeze-title modes collided
across packages.

Closed with one file-local helper, `extension_scope_key(widgetType, uniqueId, groups)` in
`ProjectModel.cpp`: for any type other than `DashboardExtension` it returns today's key
unchanged; otherwise it resolves the package id from the project itself (the group's or
dataset's own `widget` string, matched by `uniqueId`) and returns
`WidgetExtensions::persistedTypeToken(id) + ":" + uid` — the same token the dashboard reads,
from the single shared formatter. An entity that names no package falls back to the numeric key.
It is used by exactly the three functions named in the hand-off: `freezeTitleMode()`,
`setFreezeTitleMode()`, and `promptRenameWidget()` (which now stages through
`stageDisplayTitle()` with the resolved key instead of the numeric-key wrapper).

**Residue, closed in the T24-T28 pass:** `widgetDisplayTitle()` and `setWidgetDisplayTitle()` —
the API-facing read/write pair — now build their key with the same
`extension_scope_key(widgetType, uniqueId, m_groups)` helper, so a rename issued through the API
addresses the same `"ext:<id>:<uid>"` entry the caption menu and the dashboard read. Two one-line
edits, no other line in `ProjectModel.cpp` touched; `widget_scope_key()` stays as the helper's
non-extension branch. `code-verify --check`: clean.

## Recorded gaps closed in the T24-T28 pass

**The dataset widget picker could not offer a package (T20's deferred half).** Since spec 0037 the
dataset form's widget row is generated from `app/rcc/properties/dataset.json`, where the domain was
a fixed `StaticMapOptions` table, so `m_datasetWidgets` no longer fed it and a dataset-scope package
was reachable only through the API.

Closed with a fifth option-source kind rather than a special case: `extensibleMap` is a `staticMap`
whose declared rows a named `optionProvider` hook appends to at runtime.

- `PropertyHooks::ExtensibleMapOptions` (new, `PropertyHooks.h/.cpp`): fixed table first, provider
  rows after. `indexForValue` searches the table, then the appended rows, then answers 0;
  `valueForIndex` maps `index - count` into the appended rows. **Built-in indices never move**, so a
  machine with no package installed sees exactly today's five rows and no stored value shifts.
- `PropertyHooks::widgetExtensionOptions()` (new): one `(widget string, title)` pair per registered
  dataset-scope package, skipping any package that `replaces` a built-in — the same rule
  `ProjectEditor::appendExtensionWidgets()` applies, so `m_datasetWidgets.size()` (the bound
  `datasetFormEditAccepted` and the multi-select harvest check) and the generated domain agree.
- `dataset.json`: `datasetWidgets.kind` -> `extensibleMap` + `"hook": "widgetExtensionOptions"`, and
  a `hooks` entry of kind `optionProvider` so `registry-verify.py` proves the hook exists in
  `PropertyHooks.h`. `properties/schema.json` accepts the new kind.
- `scripts/generate-property-registry.py`: the kind is mapped in `option_type`, `emit_option_table`
  (same `StaticOptionEntry` table), and a new `emit_option_source` branch; `schema_props_for` keeps
  it in the enum-domain kinds, so the `api-schema.json` projection is byte-identical and
  `--check-snapshot` sees no change. Regenerated; `--check` reports up to date.

The declared API enum for `widget` therefore still lists the built-in strings only, which is
accurate: an extension id is a legal value of a free-string field, not a member of the built-in
domain.

**Build registration is now load-bearing.** `PropertyHooks.cpp` includes `UI/WidgetExtensions.h` and
calls into the catalog; the `app/CMakeLists.txt` entries for `WidgetExtensions.cpp` /
`WidgetExtensionManifest.cpp` have landed (source and header lists), so the link is satisfied.

## Tasks

### T1 — Capture the pre-change baselines

- **Files:** `tests/integration/baselines/` (new), notes in this file.
- **Does:** Maintainer captures, before any code change: `project.exportJson` for every
  `examples/` project that uses a Compass or DataGrid widget, and a `--benchmark-hotpath` run
  on a representative project. These are the only defensible references for AC8 and AC9.
- **Verify:** Baseline files checked in; benchmark numbers recorded in the task note.
- **Deps:** none
- [ ] done

### T2 — Add the `widget` extension type to the extension manager

- **Files:** `app/src/Misc/ExtensionManager.cpp`
- **Does:** Adds `"widget"` to `extensionTypes()` and `tr("Widget")` to `friendlyTypeName()`.
  Nothing else — install, uninstall, update, path-safety, and per-platform file resolution
  already handle an arbitrary type string.
- **Verify:** `code-verify --check`; read-back that no other branch in the TU switches on the
  type list.
- **Deps:** none
- **Note:** Done. `extensionTypes()` gains `"widget"`, `friendlyTypeName()` gains `tr("Widget")`.
  Read-back: the only other type-keyed site in the TU is the cosmetic `typeOrder` sort table in
  `applyFilter()` (unlisted types fall to 99), so widget packages sort last in the browser list;
  left alone per this task's "nothing else". Launch/install/uninstall paths key off `"plugin"`
  only.
- [x] done

### T3 — Manifest schema and reserved-id list

- **Files:** `app/rcc/extensions/schema/widget-manifest.json` (new), `app/rcc/rcc.qrc`
- **Does:** Writes the JSON Schema for the `widget` block exactly as specified in `plan.md`
  (scope, qml, icon, accepts, readsStringValues, defaultSize, config, dependencies,
  hostCompat, apiVersion, experimental, bundled-only `replaces`), including the reserved
  builtin-id enumeration. Registers it in `rcc.qrc`.
- **Verify:** Schema validates the example manifest from `plan.md` and rejects a manifest
  using a reserved id, checked with a throwaway `jsonschema` call.
- **Deps:** none
- **Note:** Done. `app/rcc/extensions/schema/widget-manifest.json` (draft-07) + one `rcc.qrc`
  entry (`:/extensions/schema/widget-manifest.json`). Checked with `jsonschema`: the plan's
  example validates; `id: "plot3d"` is rejected by the reserved-id enum; `scope: "tool"` is
  rejected; the `qml` pattern rejects `../escape.qml`, `/abs.qml`, and backslash paths while
  allowing package subdirectories. The schema is the machine-readable contract for
  `registry-verify.py` / `tests/scripts` (T24); the C++ validator implements the equivalent
  checks by hand -- no JSON Schema engine is linked into the app.
- [x] done

### T4 — `UI::WidgetExtensions` header and descriptor types

- **Files:** `app/src/UI/WidgetExtensions.h` (new)
- **Does:** Declares the singleton and its value types (`Descriptor`, `Accepts`,
  `ConfigProperty`, `Dependency`) plus the public API named in `plan.md`. Constructor is a
  leaf: member init only, no `instance()` reach, no scan. `kWidgetApiVersionMajor/Minor`
  constants land in `app/src/SerialStudio.h`.
- **Verify:** `code-verify --check` (header ordering, `[[nodiscard]]`, no in-header init).
- **Deps:** none
- **Note:** Done. `app/src/UI/WidgetExtensions.h` declares `Scope`, `ValueKind`, `Accepts`,
  `ConfigProperty`, `Dependency`, `Descriptor`, the catalog API, and the free
  `parseWidgetManifest()` / `widgetVersionInRange()` used by the manifest TU.
  `kWidgetApiVersionMajor/Minor` (1.0) landed in `SerialStudio.h`. `code-verify --check`: clean.
- [x] done

### T5 — Manifest parsing and validation

- **Files:** `app/src/UI/WidgetExtensionManifest.cpp` (new), `app/CMakeLists.txt`
- **Does:** Implements parse + validate: schema check, `hostCompat` semver-range evaluation
  against the host widget-API version, reserved-id rejection, `replaces` accepted only for
  bundled packages, QML-entry existence, and config-property normalization. Returns either a
  `Descriptor` or a list of `Misc::ProblemCenter::Finding` values — it never logs and never
  reports directly.
- **Verify:** `code-verify --check`; each rejection path returns a distinct finding `code`.
- **Deps:** T3, T4
- **Note:** Done in `app/src/UI/WidgetExtensionManifest.cpp` (NOT yet listed in
  `app/CMakeLists.txt` -- see the registration list below). Distinct codes:
  `widget-manifest-invalid`, `widget-replaces-forbidden`, `widget-id-reserved`,
  `widget-api-version`, `widget-host-incompatible`, `widget-qml-missing`,
  `widget-config-invalid`. `hostCompat` is evaluated by `widgetVersionInRange()` (space-separated
  `>= <= > < = ==` clauses over `QVersionNumber`; empty/`*` = any; an unparsable clause fails
  closed). Entry-point validation requires the file to exist *and* resolve inside the package
  directory. The TU never logs, never reports, and never touches QML.
- [x] done

### T6 — Catalog scan, precedence, and consent storage

- **Files:** `app/src/UI/WidgetExtensions.cpp` (new), `app/CMakeLists.txt`
- **Does:** Implements `rescan()` (bundled `qrc` packages first, then
  `<workspace>/Extensions/widget/*/info.json`; a disk package may not shadow a bundled id),
  the lookup accessors, `builtinReplacement()`, `catalogChanged()`, and consent read/write
  against `QSettings` keyed `WidgetExtensionConsent/<id>` storing the approved version.
- **Verify:** `code-verify --check`; read-back that no `QQmlComponent` or QML type is touched
  anywhere in the TU (the lazy-instantiation guarantee).
- **Deps:** T5
- **Note:** Done in `app/src/UI/WidgetExtensions.cpp`. `rescan()` = bundled `:/extensions/widget`
  first, then `<workspace>/Extensions/widget/*/info.json` (256-package cap, 1 MB manifest cap via
  `Misc::JsonValidator`); a disk package that repeats a registered id is ignored with
  `widget-package-shadowed`. `resolveDependencies()` drops packages whose *required* dependency is
  missing or out of range (`widget-dependency-missing`) and only reports optional gaps
  (`widget-dependency-optional`). Consent lives in `QSettings` under
  `WidgetExtensionConsent/<id>` storing the approved version string, so a package update re-asks.
  **Consent gate:** `canInstantiate(id)` = registered AND (bundled OR consent recorded for the
  installed version); `qmlUrl(id)` returns an empty string when it is false, which makes it the
  single choke point every later instantiation path must go through (default-deny). Read-back:
  the TU includes no QML header and constructs no `QQmlComponent`.
- [x] done

### T7 — Composition-root wiring and the ctor-edge proof

- **Files:** `app/src/Misc/ModuleManager.cpp`
- **Does:** Instantiates `UI::WidgetExtensions` in `instantiateCoreModules()` after
  `ProjectModel` and before `Dashboard`; performs the first `rescan()` and connects
  `ExtensionManager::extensionInstalled` / `extensionUninstalled` → `rescan()` from
  `setupCrossModuleConnections()`; exposes `Cpp_UI_WidgetExtensions`.
- **Verify:** Re-run the ctor-edge proof in `doc/claude/specs/0001-composition-root/` and
  record the result in the task note; `code-verify --check`.
- **Deps:** T6
- **Note:** Done. `(void)UI::WidgetExtensions::instance();` sits immediately before
  `(void)UI::Dashboard::instance();` (so: after ProjectModel, Dashboard still last).
  `setupCrossModuleConnections()` connects `ExtensionManager::extensionInstalled` /
  `extensionUninstalled` and `WorkspaceManager::pathChanged` to `rescan()`, then calls
  `rescan()` once -- placed **before** `appState->restoreLastProject()`, so the first project
  load already sees the catalog. Context property `Cpp_UI_WidgetExtensions` registered in
  `registerCoreContextProperties()`. The `pathChanged -> rescan()` edge is an addition beyond
  the task text, mirroring the ThemeManager/ExtensionManager precedent (the package root moves
  with the workspace).
  **Ctor-edge proof, re-run:** `WidgetExtensions::WidgetExtensions()` is `: m_settings() {}` --
  no `::instance()` call, no scan, no `QSettings` read, no signal connection, so it adds zero
  outgoing ctor edges; the pinned order's INV-1..INV-3 are unaffected and no existing entry
  moved. Nothing was added to any ctor-reachable closure (in particular, ProjectModel's ctor
  closure is untouched).
- [x] done

### T8 — Problem-center checker for widget extensions

- **Files:** `app/src/Misc/Problems/ExtensionCheckers.h` / `.cpp` (new),
  `app/src/Misc/ProblemCenter.cpp`, `app/CMakeLists.txt`
- **Does:** Registers checker id `extension.widget` on `ProjectChanged | OnDemand`, reporting
  the catalog's stored findings plus project references to widget ids that are not installed.
  Each finding carries severity, explanation, remedy, and a `jump` of `"group"` / `"dataset"`
  where an entity is named.
- **Verify:** `code-verify --check`; `problems.listCheckers` will report the new id (asserted
  later in T24).
- **Deps:** T6
- **Note:** Done. `app/src/Misc/Problems/ExtensionCheckers.{h,cpp}` follow the ScriptCheckers
  idiom (free statics + `registerAll()`), registered from `ProblemCenter::setupExternalConnections()`
  as `extension.widget` on `ProjectChanged | OnDemand`. It republishes the catalog's findings and
  walks the project for group/dataset `widget` strings that are neither empty nor reserved:
  `widget-not-installed` (Error, jump `group`/`dataset`) and `widget-consent-required` (Warning)
  for a package that is installed but not yet allowed to run. Reserved ids are skipped
  build-independently, so a GPL build does not report `image`/`painter` as missing extensions.
- [x] done

### T9 — `DashboardExtension` enum value and classification helpers

- **Files:** `app/src/SerialStudio.h`, `app/src/SerialStudio.cpp`
- **Does:** Adds `DashboardExtension = 100` **after** the `#ifdef BUILD_COMMERCIAL` block with
  an explicit value, and handles it in `isGroupWidget`, `isDatasetWidget`,
  `dashboardWidgetTitle`, `dashboardWidgetIconName`, `dashboardWidgetIcon`,
  `groupWidgetEligibleForWorkspace`, `datasetWidgetEligibleForWorkspace`. No resolution logic
  yet. `commercialCfg()` is deliberately untouched.
- **Verify:** `code-verify --check`; read-back that every existing enumerator's ordinal is
  unchanged in both build configurations, and that `commercialCfg()` has no new input.
- **Deps:** none
- **Note:** Partially done -- read the deviation before continuing.
  Landed: `DashboardExtension = 100` as the last enumerator, *after* the
  `#ifdef BUILD_COMMERCIAL` block, so every existing enumerator keeps its implicit ordinal in
  both configurations (GPL last = `DashboardNoWidget` 17; commercial last = `DashboardPainter`
  22); the enum's doxygen now records that ordinals are persisted state and that an extension
  widget's scope/title/icon come from the descriptor. `dashboardWidgetTitle()` gains
  `tr("Extension Widgets")`. `commercialCfg()` and `activated()` are untouched and gained no
  input. `API::EnumLabels::dashboardWidgetSlug()` needed a case (its switch has no `default`, so
  the new enumerator would be a `-Wswitch` warning, and the repo ships zero warnings); the
  `fromSlug` inverse gained the matching `"extension"` line.
  **Deviation (decide in T11):** `isGroupWidget()` / `isDatasetWidget()` were left enum-pure, so
  both answer *false* for `DashboardExtension`. One enum value serves both scopes, so these two
  predicates cannot answer from the enum alone, and they are used as a *discriminator*
  (`Dashboard::widgetCount` picks `m_widgetGroups` vs `m_widgetDatasets`;
  `DashboardWidget::widgetTitle`/`widgetUniqueId`/`widgetSourceId` pick group vs dataset;
  `Taskbar` and `WorkspacesHandler` do the same) -- making both return true would silently route
  every group-scope extension widget down the dataset branch. Returning false keeps today's
  behaviour exactly (nothing buckets under the new value until T11) and leaves the choice
  explicit: T11 must add a scope-aware path (e.g. `Dashboard::extensionIdAt()` plus
  descriptor-driven branches at the five call sites) rather than relying on these predicates.
  `groupWidgetEligibleForWorkspace` / `datasetWidgetEligibleForWorkspace` already return true for
  unlisted values, which is the wanted answer for extension widgets, so they needed no edit.
  `dashboardWidgetIconName()` falls through to the generic `widgets/group` artwork; the real icon
  comes from the descriptor in T21.
- [x] done

### T10 — Resolve extension ids in the widget-string mappers

- **Files:** `app/src/SerialStudio.cpp`
- **Does:** `getDashboardWidget(group)` and `getDashboardWidgets(dataset)` consult
  `WidgetExtensions` **only after** every builtin string comparison misses, returning
  `DashboardExtension`. The Pro `#ifdef` branches and the GPL `painter → datagrid` fallback
  are untouched and still evaluated first.
- **Verify:** `code-verify --check`; read-back that no code path can reach a Pro enum value
  from catalog data (the R10 evidence).
- **Deps:** T6, T9
- **Note:** Done. One file-local helper `isWidgetExtension(id, scope)` consults the catalog and is
  called as the *last* statement of `getDashboardWidget()` (after the GPL `painter -> datagrid`
  fallback and both `#ifdef` blocks) and as the `else if` of the dataset widget-string map lookup
  in `getDashboardWidgets()`, so the builtin comparison chain is unchanged and still evaluated
  first. **R10 evidence:** the helper's only possible return is `DashboardExtension`; it never
  reads a manifest field other than `scope`, and a package id equal to a builtin string cannot be
  registered (`parseWidgetManifest` rejects reserved ids), so no catalog data can select a Pro
  enumerator on any build. The helper also gates on scope, so a dataset-scope package cannot be
  hoisted into a group widget slot or vice versa.
- [x] done

### T11 — Dashboard bucketing and extension identity

- **Files:** `app/src/UI/Dashboard.h`, `app/src/UI/Dashboard.cpp`
- **Does:** `buildWidgetGroups` and `processDatasetIntoWidgetMaps` bucket extension entries
  under `DashboardExtension`, recording owning ids in the index-aligned
  `m_extensionGroupIds` / `m_extensionDatasetIds`, sorted by `(extensionId, existing order)`.
  Adds `extensionIdAt(bool group, int relativeIndex)`. `applyDisplayTitles` /
  `refreshDisplayTitles` emit `"ext:<id>:<uid>"` as the type token for extension widgets.
- **Verify:** `code-verify --check`; read-back that no builtin widget's `widgetIndex` or
  `relativeIndex` can shift as a result of catalog contents.
- **Deps:** T10
- **Note:** Done, with the coordinator's descriptor-driven-scope ruling implemented as
  `Dashboard::widgetSlot(type, relativeIndex)` -> `ExtensionSlot {valid, group, bucketIndex,
  extensionId}`: the single place that discriminates group from dataset. `isGroupWidget()` /
  `isDatasetWidget()` stay enum-pure. The five discriminator sites now read the slot:
  `Dashboard::widgetCount` (returns `m_extensionGroupIds + m_extensionDatasetIds` for the
  extension key), `DashboardWidget::widgetTitle` / `widgetUniqueId` / `widgetSourceId` (plus
  `widgetColor`, which had the same enum test), `Taskbar::collectGroupWidgetIds` +
  `appendGroupChildItem`, and `WorkspacesHandler::dashboardRelativeIndexFor` (T12).
  **Index layout:** one enum value serves both scopes, so the relative index is *combined* --
  group-scope slots occupy `[0, groupCount)` and dataset-scope slots follow;
  `datasetBucketBase(key)` supplies that offset in `registerWidgets` and `refreshDisplayTitles`
  so `WidgetRegistry`'s per-type creation order matches. `registerWidgets` also stopped using
  `widgetCount(key)` as the loop bound (identical for every builtin, wrong for the shared key)
  and uses the bucket size. `applyDisplayTitles` emits `"ext:<id>:<uid>"` via
  `WidgetExtensions::persistedTypeToken`.
  **Deviation (ordering):** entries are recorded in project walk order within each scope block,
  *not* stable-sorted by `(extensionId, order)`. Walk order is equally independent of install
  order, and it is what all four bucketing mirrors (T12) already compute naturally -- replicating
  an id-sort in each of them was the larger risk, since a mirror that disagrees with
  `buildWidgetGroups` silently orphans workspace refs. Nothing consumes package contiguity.
  **No builtin can shift:** `m_widgetGroups` / `m_widgetDatasets` are `QMap`s keyed by the enum
  and `DashboardExtension = 100` is the highest key, so extension entries always iterate last;
  every other bucket's contents and order are byte-identical to before.
- [x] done

### T12 — Mirror the bucketing in the workspace and summary paths

- **Files:** `app/src/DataModel/Project/ProjectEditorSummaries.cpp`,
  `app/src/DataModel/Project/ProjectModelWorkspaces.cpp`,
  `app/src/API/Handlers/WorkspacesHandler.cpp`
- **Does:** Teaches the three places that re-derive the dashboard's widget bucketing that
  `DashboardExtension` sub-buckets by extension id, so workspace refs and relative indices
  agree with the dashboard. The `Plot3D` / `Painter` license remaps alongside are untouched.
- **Verify:** `code-verify --check`; read-back diffing each site against
  `Dashboard::buildWidgetGroups` for agreement.
- **Deps:** T11
- **Note:** Done. Because the bucket keeps walk order (T11 deviation), every mirror's existing
  per-type running counter is already correct for *group-scope* extension widgets; the only
  divergence was the dataset-scope block's offset. New shared helper
  `SerialStudio::extensionGroupWidgetCount(groups)` supplies it and is used to seed the
  dataset counter in `ProjectEditor::buildResolvedWidgetLookup`, `ProjectEditor::allWidgetsSummary`
  and `ProjectModel::buildAutoWorkspaces`. `WorkspacesHandler::dashboardRelativeIndexFor` resolves
  the scope from the target entity (`groupHasExtensionWidget`) instead of the enum predicates,
  then seeds the same offset; `groupContribution` / `datasetOffsetInGroup` needed no change --
  they already count by widget key. The `Plot3D` / `Painter` license remaps are untouched.
  **Known gap (for T18+):** `ProjectModel::widgetTypeCountsForGroup` and the
  `shiftWorkspaceRefsAfter*Delete` maintenance helpers keep one merged counter per type. That is
  correct for a project whose extension widgets are all one scope and can misalign an extension
  ref in a project that mixes both scopes; splitting those counters into the two blocks is a
  separate change and was left out rather than half-done (the editor's unresolved-ref cleanup
  already surfaces the result).
- [x] done

### T13 — `Widgets::ExtensionData`, the generic widget model

- **Files:** `app/src/UI/Widgets/ExtensionData.h` / `.cpp` (new), `app/CMakeLists.txt`,
  `app/src/Misc/ModuleManager.cpp`
- **Does:** Implements the one model every extension widget receives, built explicitly against
  what `Widgets::Compass` and `Widgets::DataGrid` expose today so the later conversions cannot
  discover a gap: title, ids, per-dataset maps (value, numeric value, units, min/max, alarm
  state, uniqueId, index), config object, `updated()` on `Dashboard::updated`. Registers it as
  `ExtensionDataModel` in the `SerialStudio` QML module.
- **Verify:** `code-verify --check`; a written field-by-field comparison against
  `Compass.h` / `DataGrid.h` in the task note — any value that cannot be exposed generically
  is a **stop**, not a workaround.
- **Deps:** T4
- **Note:** Done in `app/src/UI/Widgets/ExtensionData.{h,cpp}`, registered as `ExtensionDataModel`
  in `ModuleManager::registerQmlTypes()`. Shape: scalar "lead dataset" properties plus an
  internal `ExtensionRowsModel : QAbstractListModel` (`datasets`), copying DataGrid's proven
  reset/`updateRow`+`dataChanged` discipline so a 50-dataset group does not rebuild delegates on
  every tick. `updated()` fires only when a volatile field actually moved (`DSP::notEqual` for
  the numeric compare, so a NaN stream cannot storm).
  **Ctor deviation:** `(extensionId, widgetType, relativeIndex, parent)` rather than the plan's
  `(index, parent)` -- the `replaces` path (T15) instantiates this model for a *builtin* type
  whose data lives in that builtin's bucket, so the type cannot be assumed.
  **Field-by-field vs `Compass.h`:** `value` -> `value` (raw heading; the fmod/wrap and the
  16-wind `cardinal` string are package QML logic, they read nothing the host has);
  `title`/`units`/`displayFormat`/`decimalPoints` -> same names; `alarmsDefined`/`alarmTriggered`
  -> same names (Compass hardcoded false, the generic version answers from `alarmBands`, a
  superset). No gap.
  **Field-by-field vs `DataGrid.h`:** `rowsModel` (title, value, widgets) -> `datasets` model
  with roles `title` / `text` / `widgets` plus `value`, `numericValue`, `isNumeric`, `units`,
  `minValue`, `maxValue`, `decimalPoints`, `displayFormat`, `uniqueId`, `index`, `alarmsDefined`,
  `alarmSeverity`; `formatValue()` -> the `text` role, byte-identical formatting logic (FMT_VAL /
  fixed decimals / units suffix), which the package needs since context hardening hides
  `Cpp_UI_Dashboard.formatValue`; `datasetWidgets()` -> the `widgets` role, same
  `{windowId, icon, title}` build and same plots-last ordering; `paused` -> `paused` (writable,
  same resume-pulls-a-snapshot behaviour); `titleHeader`/`valueHeader` -> **not** exposed: they
  are `tr("Title")` / `tr("Value")` literals with no data behind them, so the package supplies
  its own `qsTr` strings (noted for T23: those two strings leave the app's translation catalog).
  Config: `config` (descriptor defaults overlaid with `ProjectModel::widgetSettings`) +
  `setConfigValue(key, value)` writing through `saveWidgetSetting` under the same
  `"type:groupId:datasetIndex"` key `DashboardWidget::widgetId()` produces, refreshed on
  `widgetSettingsChanged`.
- [x] done

### T14 — String-value target registration

- **Files:** `app/src/UI/Dashboard.cpp`
- **Does:** `buildValuePushes` adds an extension widget's `ExtensionData` to the
  `string_targets` set when its descriptor declares `readsStringValues`. Reconfigure-time only;
  the per-frame walk is not modified.
- **Does not:** change how numeric datasets propagate, or add any per-frame branch.
- **Verify:** `code-verify --check`; invoke `ss-hotpath` and state in the task note how the
  no-alloc / cached-flag / string-target invariants are preserved.
- **Deps:** T11, T13
- **Note:** Done as `Dashboard::addExtensionStringTargets(targets)`, called once from
  `buildValuePushes` right after the DataGrid block: it walks the two extension buckets and
  inserts a widget's dataset copies into `string_targets` only when
  `descriptor(id).readsStringValues` is true.
  **Hotpath evidence (binding invariants named before the edit: reconfigure-time resolution
  only; no allocation or Frame copy on the dashboard path; no new input to a cached flag;
  source owns time):** `hotpathRxFrame` and `updateDashboardData` are not touched -- the diff
  contains no edit inside either. The per-frame walk still reads `ValuePush::targets` /
  `stringTargets` positionally; for a project with no extension widget both vectors are
  bit-identical to before, because the new helper returns after two `QMap::constFind` misses at
  reconfigure and inserts nothing. For a project *with* an extension widget that does not declare
  the flag, still nothing is inserted -- the declaration is the whole gate. No cached flag
  (`m_operationMode`, `m_anyAsyncSink`, `m_captureLatestFrame`, `m_changeDriven`,
  `m_streamAvailable`) gains an input, no new signal crosses a thread, and `ExtensionData`
  refreshes on `Dashboard::updated` (UI cadence), never on the frame path. The catalog lookups
  added to `getDashboardWidget*` (T10) also run only at reconfigure, and for the common
  empty/known widget string they early-out before touching the catalog hash.
- [x] done

### T15 — `buildWidgetForType` extension case and `replaces` resolution

- **Files:** `app/src/UI/DashboardWidget.cpp`, `app/src/UI/DashboardWidget.h`
- **Does:** Adds `case SerialStudio::DashboardExtension:` resolving `m_qmlPath` from the
  descriptor and constructing `Widgets::ExtensionData`; and routes any builtin type for which
  `builtinReplacement()` returns a bundled package through the same path. Builtins with no
  replacement are unchanged.
- **Verify:** `code-verify --check`; read-back that `m_qmlPath` for a non-extension widget is
  still a compile-time `qrc:` literal.
- **Deps:** T6, T13
- **Note:** Done. `setWidgetIndex` clears `m_extensionId` / `m_extensionError` and calls the new
  `buildExtensionModel()`; only when that returns false does `buildWidgetForType()` run (kept a
  separate function so the switch stays under the 100-line cap). `buildExtensionModel()` resolves
  the id from `Dashboard::extensionSlot` for `DashboardExtension`, otherwise from
  `catalog.builtinReplacement(builtinWidgetId(type))`, then sets `m_qmlPath = catalog.qmlUrl(id)`
  and constructs `Widgets::ExtensionData`. **Read-back:** every `case` in `buildWidgetForType()`
  still assigns a string literal, and that switch is now only reached when no package serves the
  slot -- which is every builtin until a bundled package declares `replaces` (T22/T23).
  `builtinWidgetId()` maps only the free, non-tool widget strings (datagrid, multiplot,
  accelerometer, gyro, gps, webview, led-panel, bar, gauge, compass, meter); Pro types
  (plot3d/image/painter/waterfall/output-panel) and the tools are deliberately absent, so not even
  a bundled package can become the implementation of a Pro widget.
  New QML surface on `DashboardWidget`: `widgetIsExtension`, `widgetExtensionId`,
  `widgetExtensionError`, and `createExtensionItem(parent, properties)` (T17).
- [x] done

### T16 — Delegate error branch and the failure placeholder

- **Files:** `app/qml/Widgets/Dashboard/ExtensionPlaceholder.qml` (new),
  `app/qml/MainWindow/Panes/Dashboard/WidgetDelegate.qml`,
  `app/qml/MainWindow/Panes/Dashboard/ExternalWidgetWindow.qml`, `app/CMakeLists.txt`
- **Does:** Both delegates gain a `Component.Error` branch that instantiates the placeholder
  and reports through the problem center; `ExternalWidgetWindow`'s hardcoded size table gains
  a descriptor-driven default for extension widgets.
- **Verify:** `code-verify --check`; `python scripts/registry-verify.py` (QML icon
  render-size lint on the new file).
- **Deps:** T8, T15
- **Note:** Done. `ExtensionPlaceholder.qml` (warning icon at its 32 px tier, widget title,
  cause, "Open Problem Center") plus a `showPlaceholder(reason)` and a factored `bindToolbar()`
  in both delegates. Both now branch on `widgetIsExtension` into `createExtensionItem(...)` and
  fall back to the placeholder when it returns null; the builtin path's `Component.Error` branch
  replaces its `console.error` with the same placeholder. `ExternalWidgetWindow` sizes an
  extension pop-out from `Cpp_UI_WidgetExtensions.packageInfo(id).width/height` (two new keys on
  `packageInfo`) instead of its hardcoded path table, which is untouched for builtins.
  Reporting channel: `WidgetExtensions::reportLoadFailure(id, error)` (new public slot) appends a
  deduplicated `widget-load-failed` finding to the catalog's list and calls
  `ProblemCenter::runNow()`, so the `extension.widget` checker from T8 republishes it; the
  finding is dropped on the next `rescan()`.
- [x] done

### T17 — Extension QML context hardening

- **Files:** `app/src/UI/DashboardWidget.cpp` (or the component-creation site chosen in T15)
- **Does:** Creates each extension component in its own `QQmlContext` with the `Cpp_*` names
  shadowed to `undefined`. The code comment states, in one sentence, that this is a speed bump
  and not a boundary — the consent flow is what makes the trust model honest.
- **Verify:** `code-verify --check`; read-back that builtin widgets still use the root context
  unchanged.
- **Deps:** T15
- **Note:** Done inside `DashboardWidget::createExtensionItem()`: the component is created with
  `createWithInitialProperties(properties, context)` where `context` is a fresh `QQmlContext`
  parented to this widget, with every name from `WidgetExtensions::hostContextNames()` set to an
  invalid `QVariant`. The function's doxygen states in one sentence that the shadowing is a speed
  bump and not a boundary, and that consent is what makes the trust model honest; the name list's
  own doxygen repeats it and records that a missing name only leaks a name.
  **Read-back:** builtins never enter this function -- `createExtensionItem` returns immediately
  when `m_extensionId` is empty, and both delegates only call it under `widgetIsExtension`; the
  builtin branch still uses `Qt.createComponent` + `createObject` in the delegate's own context.
  **Known drift risk (T24 candidate):** `hostContextNames()` is a hand-kept mirror of
  `ModuleManager`'s `setContextProperty` calls -- Qt exposes no way to enumerate a context's
  properties -- so a new `Cpp_*` registration will not be shadowed until it is added here. A lint
  rule comparing the two lists would close that.
- [x] done

### T18 — First-load consent

- **Files:** `app/qml/Dialogs/ExtensionConsent.qml` (new), `app/qml/main.qml`,
  `app/src/UI/WidgetExtensions.cpp`, `app/CMakeLists.txt`
- **Does:** Before a disk-installed package is instantiated for the first time at a given
  version, shows title, author, source repository, install path, and the privileges statement,
  with Allow / Don't allow. Declining leaves the package installed and inert (placeholder +
  finding). Bundled packages are exempt.
- **Verify:** `code-verify --check`; read-back that no code path instantiates a
  non-bundled package before `consentGranted()` returns true.
- **Deps:** T15, T16
- **Note:** Done. Flow: `DashboardWidget::buildExtensionModel()` resolves `qmlUrl(id)`; when that
  comes back empty it calls the new `WidgetExtensions::requestConsent(id)`, which emits
  `consentRequested(id)` **once per package per scan** and only while the answer is still open
  (bundled, already-granted, and already-declined packages never prompt). `main.qml` holds a
  `DialogLoader` for the new `app/qml/Dialogs/ExtensionConsent.qml` plus
  `app.showExtensionConsent(id)`, which queues ids while the loader is still loading. The dialog
  states id, title, author, version, license, description, install path, and the privilege
  statement, with **Allow** / **Don't Allow**.
  **Decisions persist both ways:** `grantConsent` writes `WidgetExtensionConsent/<id> = <version>`
  (and clears any refusal); the new `declineConsent` writes `WidgetExtensionDecline/<id> =
  <version>` so a refused package stays installed and inert across restarts, and a version bump
  re-asks (AC6). `revokeConsent` clears both.
  A declined widget's `ExtensionPlaceholder` now says so and offers **Review and Allow…**, which is
  also the way back after a refusal.
  Consent takes effect immediately: `grantConsent` emits `catalogChanged`, and both delegates gained
  a `Connections` that re-runs the new `DashboardWidget::reloadWidget()` slot and rebuilds the
  widget in place, so nothing waits for a project reload. No loop is possible -- the second pass
  either succeeds or hits the already-decided early-out in `requestConsent`.
  **Read-back (no pre-consent execution):** the only path that compiles package QML is
  `DashboardWidget::createExtensionItem()`, which returns `nullptr` when `m_qmlPath` is empty;
  `m_qmlPath` comes only from `qmlUrl()`, which returns `{}` unless `canInstantiate()` (registered
  AND bundled-or-consented). Both delegates call `createExtensionItem` and fall back to the
  placeholder. The host-side `Widgets::ExtensionData` is still constructed without consent -- it is
  the host's own model, runs no package code, and is what gives the placeholder a title.
  **Source repository is not shown:** neither `info.json` nor `ExtensionManager`'s installed record
  carries the repository URL a package came from, so the dialog states the install path instead.
  Adding a `source` field is a manifest change and was left for the signing question in `spec.md`.
- **Deviation:** the task listed `app/src/UI/WidgetExtensions.cpp`; the signal/slot pair also needed
  `WidgetExtensions.h`, and the prompt trigger needed three lines in `DashboardWidget.cpp`.
- [x] done

### T19 — Generic config form

- **Files:** `app/qml/Widgets/Dashboard/ExtensionWidgetSettings.qml` (new),
  `app/qml/Widgets/Dashboard/WidgetToolbar.qml`, `app/CMakeLists.txt`
- **Does:** Renders the descriptor's `config` declarations (scalars and fixed choices) as an
  editable form, reading and writing through `ProjectModel::widgetSettings` /
  `saveWidgetSetting` under the widget's existing `widgetId`. No widget-specific UI code.
- **Verify:** `code-verify --check`; `registry-verify.py`.
- **Deps:** T13, T15
- **Note:** Done. `app/qml/Widgets/Dashboard/ExtensionWidgetSettings.qml` is a `SmartDialog` that
  renders `Cpp_UI_WidgetExtensions.configProperties(id)` (new `Q_INVOKABLE` returning the
  declarations as plain maps) through one `Repeater`: `bool` -> `Switch`, `choice` -> `ComboBox`,
  `string` -> `TextField`, `int`/`double` -> `SpinBox` (doubles are scaled by 100 internally, since
  `SpinBox` is integer-valued). Values are package defaults overlaid with
  `Cpp_JSON_ProjectModel.widgetSettings(widgetId)`, and every edit writes
  `saveWidgetSetting(widgetId, key, value)` under the widget's existing
  `"type:groupId:datasetIndex"` key. There is no widget-specific UI anywhere in the file, and
  "Restore Defaults" re-writes the declared defaults.
- **Deviation (launch point):** the task named `WidgetToolbar.qml`. `WidgetToolbar` is a container
  the *package* declares, so a package that ships no toolbar would have no way to reach its own
  settings, and the host cannot inject a button into package QML. The launcher is instead the
  widget caption menu in `WidgetDelegate.qml` ("Widget Settings…", next to "Rename Widget…"), which
  every widget has and which the host owns; it is hidden when the package declares no config. The
  delegate exposes `extensionId` / `extensionWidgetId` (set from the embedded `DashboardWidget`)
  and loads the dialog lazily. External pop-out windows have no caption menu, so the form is
  reachable from the canvas only.
- [x] done

### T20 — Extension entries in the project-editor pickers

- **Files:** `app/src/DataModel/ProjectEditor.cpp`
- **Does:** `m_groupWidgets` / `m_datasetWidgets` gain one entry per installed package whose
  `scope` matches the picker and whose `accepts` matches the entity, rebuilt on
  `WidgetExtensions::catalogChanged`.
- **Verify:** `code-verify --check`; read-back that the existing builtin entries and their
  ids are unchanged.
- **Deps:** T6
- **Note:** Done for group scope, partially deferred for dataset scope -- read the gap.
  New `ProjectEditor::appendExtensionWidgets()` (called at the end of `generateComboBoxModels()`)
  inserts one entry per registered package into `m_groupWidgets` or `m_datasetWidgets` by declared
  scope, using the descriptor title. A bundled package that declares `replaces` is skipped, so the
  built-in entries and their ids are untouched (read-back: every existing `insert()` line is
  unchanged and no key is overwritten -- a package cannot hold a reserved id unless it replaces a
  built-in, which is exactly the skipped case).
  `ProjectEditorWiring.cpp` connects `WidgetExtensions::catalogChanged` to a rebuild of the combo
  models plus the group form, so installing or removing a package updates the picker live.
  **Commit path:** `applyGroupWidgetEdit` gained an extension branch *before* the built-in
  string -> `SerialStudio::GroupWidget` enum map, because that map would have folded any unknown id
  into `NoGroupWidget`. The branch writes the id into `group.widget` and calls `updateGroup(...)`,
  keeping the group's datasets (an extension group widget renders whatever datasets exist, so the
  destructive-change confirmation that fixed-layout widgets need does not apply).
  **Gap (dataset scope):** since spec 0037 the dataset form's widget row is generated from
  `app/rcc/properties/dataset.json` into `DataModel::Generated/DatasetForm.cpp`, where the domain is
  a fixed `StaticMapOptions` table -- `m_datasetWidgets` no longer feeds it. Surfacing packages
  there needs a catalog-backed `OptionSource` kind in `PropertyHooks` plus a generator change, which
  is its own task; hand-editing generated code would be overwritten by
  `scripts/generate-property-registry.py`. Until then a dataset-scope package is selectable through
  the API (`project.dataset.update` takes the id as a free string) and renders correctly, but does
  not appear in the editor's dataset combo. The converted Compass is unaffected: it keeps the
  built-in `compass` entry via `replaces`. `m_datasetWidgets` still receives the entries so the
  generator fix has one place to read; the only present-day effect is that
  `datasetFormEditAccepted`'s upper bound for `kDatasetView_Widget` grows past the generated table's
  five rows, which `StaticMapOptions::valueForIndex` already clamps to an empty widget string.
  **Ctor-edge:** the new connection sits in `wireExternalSignals()`, which is ctor-reachable from
  `ProjectEditor`. `WidgetExtensions`' constructor is a leaf (member init only, no `instance()`
  reach, no scan), so constructing it from there adds no cycle and no ordering constraint; the
  first `rescan()` still happens in `setupCrossModuleConnections()`.
- [x] done

### T21 — Taskbar titles and icons for extension widgets

- **Files:** `app/src/UI/Taskbar.cpp`
- **Does:** `TaskbarModel` rows for `DashboardExtension` take their title and icon from the
  descriptor instead of `SerialStudio::dashboardWidgetTitle` / `dashboardWidgetIcon`.
- **Verify:** `code-verify --check`.
- **Deps:** T11
- **Note:** Done. Two file-local helpers in `Taskbar.cpp` -- `taskbarIcon(type, extensionId, large)`
  and `taskbarIconId(type, extensionId)` -- return the package's declared artwork for
  `DashboardExtension` and fall through to `SerialStudio::dashboardWidgetIcon/IconId` for every
  built-in. They are used by `buildOverviewGroupItem` (which gained an `extensionId` parameter fed
  from the frame group's own `widget` string) and `appendGroupChildItem` (which already resolves
  `Dashboard::widgetSlot`, so it reads `slot.extensionId`; the slot lookup moved a few lines up).
  Icons resolve through the new `WidgetExtensions::iconUrl(id, px)`: an icon-registry id
  ("category/name") goes through `Misc::IconRegistry::iconById`, a package-relative file resolves to
  a `qrc:`/`file:` URL. `IconIdRole` keeps the built-in id for a package that ships a file, since an
  icon id cannot name one -- the row still gets the right artwork through `WidgetIconRole`.
  **Titles:** taskbar rows already carry the *entity* title (group name / widget title from the
  dashboard copies), not the widget-type label, so no title source changed; the caption icon of the
  widget window and its pop-out was the other generic-artwork site and now uses the same descriptor
  lookup (`widgetIcon()` in `WidgetDelegate.qml` / `ExternalWidgetWindow.qml`).
  `Taskbar::createItemFromWidgetInfo` was left alone: it has no relative index to resolve a package
  from, and it has no caller (dead helper).
- [x] done

### T22 — Convert Compass to a bundled package

- **Files:** `app/rcc/extensions/widget/compass/info.json` + `Compass.qml` (new),
  `app/qml/Widgets/Dashboard/Compass.qml` (deleted),
  `app/src/UI/Widgets/Compass.h` / `.cpp` (deleted), `app/src/Misc/ModuleManager.cpp`,
  `app/rcc/rcc.qrc`, `app/CMakeLists.txt`
- **Does:** The bundled dataset-scope package with `replaces: "compass"`. The QML starts as a
  verbatim copy of the existing file with only its `model` property retyped to
  `ExtensionDataModel` and the corresponding property reads adapted. Drops the C++ model and
  its QML registration.
- **Verify:** `code-verify --check`; `registry-verify.py`;
  `pytest tests/scripts/test_widget_manifests.py`; a line-by-line diff of the new QML against
  the deleted one recorded in the task note.
- **Deps:** T15, T19
- **Note:** Done, additively -- the C++ model and the old QML are still compiled in but unreachable
  (see "Pending build registrations (T18-T23)" for why and for the removal list).
  `app/rcc/extensions/widget/compass/{info.json,Compass.qml}`, four `rcc.qrc` entries.
  **Parity method:** `Compass.qml` was `cp`-ed from `app/qml/Widgets/Dashboard/Compass.qml` and then
  edited in place; `code-verify --check` and `qmllint` are clean and the diff is exactly five hunks:
  1. `required property CompassModel model` -> `required property ExtensionDataModel model`.
  2. New `heading` / `cardinal` readonly properties + a `cardinalDirection()` function that is a
     line-by-line port of `Widgets::Compass::cardinalDirection` (same eight 22.5-degree bands, same
     labels). The C++ model wrapped the value into 0..360 and published the label; `ExtensionData`
     publishes the raw dataset value, so the package wraps it -- `((v % 360) + 360) % 360` is the
     JS spelling of the old `std::fmod` + negative fixup.
  3. `root.model.value` -> `root.heading` in the spring accumulator.
  4. `root.model.cardinal` -> `root.cardinal` in the digital readout.
  5. Page persistence moved from `Cpp_JSON_ProjectModel.widgetSettings/saveWidgetSetting` to
     `model.config["page"]` / `model.setConfigValue("page", ...)`. Byte-identical on disk:
     `ExtensionData::widgetId()` produces the same `"type:groupId:datasetIndex"` key the QML used
     to pass. `page` is deliberately **not** declared in `config[]`, so it round-trips (stored keys
     always merge) without appearing in the generic settings form.
  Everything else -- every gradient stop, tick, shape, shadow and font expression -- is byte-identical,
  including the `Cpp_UI_Dashboard.formatValue` padding helper and the theme/font/graphics-backend
  reads (see the context-hardening deviation below).
  **Behavioural differences (both supersets):** `model.alarmTriggered` answered `false` unconditionally
  in `Widgets::Compass`; `ExtensionData` answers from the dataset's alarm bands, so a compass dataset
  that declares bands now tints the needle. The eight cardinal labels move from the C++ "Compass"
  translation context to the package's own `qsTr` context (same strings, new context -- like the
  DataGrid headers in T23).
  **Deviation (context hardening, T17):** `createExtensionItem()` now shadows the host `Cpp_*` names
  only for packages the user installed; a bundled package is exempt, exactly as it is exempt from
  consent. Without this a verbatim copy could not read `Cpp_ThemeManager` / `Cpp_Misc_CommonFonts` /
  `Cpp_Misc_GraphicsBackend` and the two conversions would have had to be rewritten rather than
  copied, which is the opposite of the plan's parity mitigation. Third-party packages keep the full
  speed bump; the function's doxygen states both halves.
- [x] done

### T23 — Convert DataGrid to a bundled package

- **Files:** `app/rcc/extensions/widget/datagrid/info.json` + `DataGrid.qml` (new),
  `app/qml/Widgets/Dashboard/DataGrid.qml` (deleted),
  `app/src/UI/Widgets/DataGrid.h` / `.cpp` (deleted), `app/src/Misc/ModuleManager.cpp`,
  `app/rcc/rcc.qrc`, `app/CMakeLists.txt`
- **Does:** The bundled group-scope package with `replaces: "datagrid"` and
  `readsStringValues: true` — the first real consumer of T14. Same verbatim-copy discipline as
  T22. Because this widget is also the GPL fallback target for Painter groups, the package
  must be `qrc`-bundled and non-uninstallable.
- **Verify:** `code-verify --check`; `registry-verify.py`;
  `pytest tests/scripts/test_widget_manifests.py`; read-back confirming the Painter GPL
  fallback still resolves.
- **Deps:** T14, T22
- **Note:** Done, additively, same discipline as T22.
  `app/rcc/extensions/widget/datagrid/{info.json,DataGrid.qml}`, `scope: "group"`,
  `replaces: "datagrid"`, `readsStringValues: true`.
  **Parity method:** `cp` of `app/qml/Widgets/Dashboard/DataGrid.qml` plus four hunks:
  1. `required property DataGridWidget model` -> `ExtensionDataModel`, and `import "../"` dropped
     (it resolved to the application's QML tree, which a package directory cannot reach).
  2. **Recorded gap, now closed in the package:** `model.titleHeader` / `model.valueHeader` were
     `tr("Title")` / `tr("Value")` literals with no data behind them, so `ExtensionData` does not
     expose them; the package declares its own `qsTr("Title")` / `qsTr("Value")`. Those two strings
     leave the application's translation catalog and live in the package's context.
  3. `model.rowsModel` -> `model.datasets`, and the delegate's `value` role -> the `text` role
     (`ExtensionRowsModel.text` is the formatted string the old `value` role carried; `value` is now
     the raw one). Seven `rowItem.value` reads became `rowItem.text`; the `widgets` role and its
     jump-button Repeater are unchanged.
  4. The `WidgetToolbar` + `DashboardToolButton` pair (application components, not importable from a
     package) was replaced by a package-local 48 px strip with the same visibility policy
     (`!frozen && height >= 220`) and a `ToolButton` carrying the same pause/resume icons, tooltip,
     and 24x24 / 18 px icon geometry. The built-in strip's horizontal scrolling and edge fades are
     not reproduced -- they only engage when the buttons overflow, which one button never does, so
     the rendered result is the same.
  Everything else (header gradient, separator math, row metrics, hover tooltips, RTL mirroring) is
  byte-identical.
  **Painter GPL fallback read-back:** unchanged and still correct. `SerialStudio::getDashboardWidget`
  still maps a `painter` group to `DashboardDataGrid` on a GPL build *before* any catalog lookup;
  that widget then lands in the `DashboardDataGrid` bucket, and `builtinReplacement("datagrid")`
  resolves the bundled package -- which is `qrc`-bundled, so it cannot be missing, deleted, or
  shadowed by a disk package (`loadPackage` refuses a disk id that repeats a registered one).
  **String targets:** the bundled DataGrid keeps the existing `buildValuePushes` DataGrid block,
  which keys on `m_widgetGroups[DashboardDataGrid]` and is therefore implementation-independent;
  `addExtensionStringTargets` (T14) covers the `DashboardExtension` buckets, so the manifest's
  `readsStringValues: true` is declarative here rather than load-bearing -- it becomes load-bearing
  the day a third-party group package needs string values.
  **Schema fix:** the T3 schema rejected every reserved id outright, which its own `replaces`
  documentation contradicted and which made both bundled manifests invalid. The top-level `id`
  now carries a paired `anyOf`: a reserved id is accepted only when the `widget` block declares
  `replaces`. Verified with `jsonschema`: both bundled manifests validate, the same manifest without
  `replaces` is rejected, and an ordinary third-party id still validates. The host is unchanged and
  still stricter (bundled AND `replaces == id`).
- [x] done

### T24 — Verifier rule and runnable static test

- **Files:** `scripts/registry-verify.py`, `tests/scripts/test_widget_manifests.py` (new),
  `tests/README.md`
- **Does:** Adds `check_widget_manifests(errors)` plus its call line (bundled manifests
  validate; ids unique and non-reserved; `replaces` only in `app/rcc/extensions/`; every
  `files` / `qml` / `icon` entry resolves; `rcc.qrc` in sync), and the mirrored runnable
  pytest.
- **Verify:** `python scripts/registry-verify.py`;
  `pytest tests/scripts/test_widget_manifests.py -v`; a seeded bad manifest fails both.
- **Deps:** T23
- **Note:** Done. `registry-verify.py` gains `check_widget_manifests()` (schema seeds, reserved-id
  agreement, qrc sync, per-package validation) and `check_host_context_names()`, both called from
  `main()`. Rules: the schema accepts a plain third-party manifest, rejects a reserved id without
  `replaces`, accepts the same id *with* `replaces`, and rejects a `../` qml entry (positive **and**
  negative seeds, so a weakened schema fails the gate); the reserved list must match across
  `widget-manifest.json`, `WidgetExtensions::reservedIds()`, and every widget string
  `SerialStudio::getDashboardWidget*` resolves (parsed out of `SerialStudio.cpp` -- a new builtin
  string that is not reserved is an error, which is R10 on the drift side); each bundled package's
  `files` must match its directory both ways, `qml`/`icon` must resolve, id must equal the directory
  name, `replaces` must equal the id; `rcc.qrc` and `app/rcc/extensions/` must agree both ways.
  `jsonschema` is an **optional** dependency: when it is missing the schema seeds print a skip line
  and every hand-written rule still runs, so the gate never silently weakens in CI.
  **Drift lint (the T17 known risk):** `hostContextNames()` is compared against every
  `setContextProperty("Cpp_*")` in `ModuleManager.cpp`; a registration missing from the list is an
  error naming the fix, and a stale entry is an error too. Thirteen value/build constants
  (`Cpp_AppName`, `Cpp_CommercialBuild`, ...) are exempt by name -- shadowing them narrows nothing.
  `tests/scripts/test_widget_manifests.py`: 23 tests, green (`pytest tests/scripts/test_widget_manifests.py`),
  covering the manifests, the schema rules, the reserved-id agreement, qrc sync, the verifier
  functions themselves (including a seeded manifest without `replaces`, a seeded missing file, a
  schema with its `anyOf` removed, and a seeded unshadowed context property -- each must fail), and
  consent-string hygiene: no spec-0038 file (C++, QML, manifests, help page, example) may use
  "sandbox/isolated/secure/safe" outside a denial.
  `python scripts/registry-verify.py` -> CLEAN.
- [x] done

### T25 — Integration tests

- **Files:** `tests/integration/test_widget_extensions.py` (new), `tests/README.md`
- **Does:** Writes the AC2 / AC5 / AC7 / AC8 cases enumerated in `plan.md`'s test plan,
  including the five seeded failure cases and the reserved-id collision case.
- **Verify:** Test file lints clean under `black`; the maintainer runs it against a live app.
- **Deps:** T24
- **Note:** Done. `tests/integration/test_widget_extensions.py`, 20 collected tests, `black` clean,
  `py_compile` clean, `--collect-only` clean (the agent cannot run them against a live app).
  **Two tiers, and the reason is a real gap:** nothing in the API forces a catalog `rescan()` --
  the catalog only rebuilds on install, uninstall, or a workspace-path change -- so packages a test
  writes to disk are invisible until Serial Studio restarts. The catalog-free tier (checker
  registration, `widget-not-installed` on a group and on a dataset with the right `jump` and
  `entityUniqueId`, the finding clearing when the reference is cleared, builtin strings never
  reported as extensions, the reference surviving a project round-trip, an id that merely looks
  like `plot3d` staying unresolved, and the two bundled conversions still resolving as builtins)
  runs as-is. The seeded tier writes eight packages under `<workspace>/Extensions/widget` and skips
  with an explanatory message until the app has been restarted once; it then asserts exactly one
  finding per seeded failure (`widget-manifest-invalid`, `widget-host-incompatible`,
  `widget-dependency-missing`, `widget-qml-missing`, `widget-replaces-forbidden`,
  `widget-id-reserved`), scope acceptance for a dataset- and a group-scope package, the
  consent-required warning, and that the app stays up. `SS_WORKSPACE` overrides the workspace path;
  `SS_CLEAN_TEST_PACKAGES=1` enables the teardown test.
  **Not covered, deliberately:** the QML-compile failure case (AC5's fourth) needs the widget to
  actually be placed on a live dashboard, which the project API cannot drive -- it stays a
  maintainer observation in T26, as does AC8's byte-compare against the T1 baseline (T1 is open, so
  there is no baseline to compare against; the test asserts export stability instead).
- [x] done

### T26 — Maintainer verification pass

- **Files:** none
- **Does:** Maintainer runs `pytest tests/integration/test_widget_extensions.py`, the
  `--benchmark-hotpath` comparison against T1, and the observation list (AC1, AC3, AC4, AC6,
  AC8 visual parity, AC10, AC11).
- **Verify:** Every acceptance criterion in `spec.md` checked off or explicitly deferred with
  a reason.
- **Deps:** T25
- [ ] done

### T27 — Authoring example and user documentation

- **Files:** `examples/widget-extension/` (new), `doc/help/Widget-Extension-Development.md`
  (new), `doc/help/Extensions.md`, `doc/help/help.json`
- **Does:** The copyable example package, the authoring guide (manifest reference, the import
  surface, the `ExtensionDataModel` contract, hosting, and the trust statement), and the
  `widget` type added to the existing extensions page. The guide must state the trust model
  in the product's own words and must not describe extensions as sandboxed.
- **Verify:** `python scripts/documentation-verify.py`; the new page is reachable from
  `help.json`.
- **Deps:** T26
- **Note:** Done ahead of T26 (docs do not depend on the maintainer pass; T26 stays open).
  `examples/widget-extension/` = `info.json` + `LevelBar.qml` + `README.md`: a dataset-scope package
  with two declared settings, written as the thing a third party copies. It is *not* listed in
  `examples/examples.json`, so the in-app examples browser (which fetches that manifest from
  GitHub) is unaffected.
  `doc/help/Widget-Extension-Development.md` (new, registered in `help.json` under Integration,
  next to Extensions): overview, trust model, package structure, full `info.json` reference,
  the four required QML properties, the `ExtensionDataModel` property/role tables, declared
  settings, install/test loop, the failure table mapped to the real finding codes, and
  distribution. `doc/help/Extensions.md` gains the widget type in the overview list, the
  post-install behaviour, the `widget` value in the `type` field reference, a `widget/` folder in
  the repository-structure sample, and a "Widget `info.json`" pointer.
  **Trust wording:** the guide states in its own section that a package runs with the
  application's privileges, that nothing contains it, and that consent -- not containment -- is the
  model; `tests/scripts/test_widget_manifests.py` lints both new files for softened wording.
  `python scripts/documentation-verify.py` -> 123 files, 0 findings.
- [x] done

### T28 — Architecture notes

- **Files:** `doc/claude/architecture/dashboard.md`, `CLAUDE.md`
- **Does:** Records the catalog, the two identity mechanisms (`DashboardExtension = 100` +
  `ext:<id>` tokens for third-party packages, `replaces` for bundled conversions), the
  `readsStringValues` requirement, and the trust model, in the existing terse voice.
- **Verify:** `python scripts/documentation-verify.py` (advisory for these paths); read-back
  against the shipped code.
- **Deps:** T27
- **Note:** Done. `doc/claude/architecture/dashboard.md` gains a "Widget Extensions (spec 0038)"
  section before Workspaces: the catalog and its composition-root placement, eager-metadata /
  lazy-instantiation, the two identity mechanisms (`DashboardExtension = 100` + `"ext:<id>"` tokens
  + `Dashboard::widgetSlot` as the only scope discriminator + the combined index layout, versus
  `replaces` for the bundled conversions), and the four rules that bite: `readsStringValues` as the
  declarable form of the stale-string gotcha the same file already warns about, the reserved-id
  three-way agreement, the `extensibleMap` route into the generated dataset picker, and the trust
  model (default-deny `canInstantiate`, per-version consent, `Cpp_*` shadowing as a speed bump with
  bundled packages exempt, `hostContextNames()` linted), plus the finding-code list. The
  `architecture.md` index row gained "widget extensions". `CLAUDE.md` gained a five-line pointer
  section before Code Style. Read-back: every claim was checked against the shipped code in this
  pass (enum value and placement, `widgetSlot`, `persistedTypeToken`, `extension_scope_key` call
  sites, `addExtensionStringTargets`, `builtinWidgetId`, `canInstantiate`/`qmlUrl`,
  `createExtensionItem`'s bundled exemption, and the checker's codes).
- [x] done

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` is met and checked off there.
- [ ] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [x] `python scripts/registry-verify.py` passes, including the new widget-manifest rule.
- [ ] `qt-cpp-review` run on the C++ diff; findings addressed or noted.
- [ ] `--benchmark-hotpath` compared against the T1 baseline, on a project with no extension
      widget and on one using the converted DataGrid; no regression.
- [x] The spec-0001 ctor-edge proof re-run and recorded (T7).
- [x] `pytest tests/scripts/test_widget_manifests.py` green (23 tests); the integration file is
      `tests/integration/test_widget_extensions.py` (20 tests, two tiers — see T25).
- [x] No documentation, UI string, or comment describes extension widgets as sandboxed,
      isolated, or safe — now lint-enforced by `test_no_spec_0038_file_claims_containment`.
- [ ] `python scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [ ] Diff is *what was asked, and only that* — no scope creep, no foreign files touched.
- [ ] `spec.md` status set to `done`.
</content>
