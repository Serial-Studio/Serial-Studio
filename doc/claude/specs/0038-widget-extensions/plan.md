---
spec: 0038-widget-extensions
phase: plan
status: draft        # draft -> approved (gate before /ss-tasks)
updated: 2026-07-25
---

# Plan 0038 — Widget-as-extension

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

A new **`UI::WidgetExtensions`** catalog reads `info.json` manifests — first from a bundled
`qrc` tree, then from `<workspace>/Extensions/widget/<id>/` — validates them against a
checked-in JSON Schema, and publishes immutable descriptors. Nothing else happens until the
dashboard needs a widget: `DashboardWidget::buildWidgetForType()` gains one case that resolves
the descriptor's QML file URL and pairs it with a single generic C++ model,
`Widgets::ExtensionData`, that republishes the widget's group or dataset payload on the
existing `Dashboard::updated` tick. The QML delegate already does
`Qt.createComponent(widgetQmlPath)` on a string the C++ side chooses, so third-party widgets
reuse the delegate, the taskbar, the window manager, freeze, and pop-out with no QML change
beyond an error branch. Third-party widgets get one new, explicitly-valued enum entry
(`DashboardExtension = 100`) so no existing widget type is renumbered and extension widgets
always sort last in the widget map; the two converted builtins instead declare
`"replaces": "<builtin id>"`, keeping their existing enum value, ordering, and persisted keys,
so parity costs no migration. Load failures route to the already-shipped `Misc::ProblemCenter`.
Three alternatives were rejected: a C++ plugin ABI (`QPluginLoader` — an ABI contract per Qt
patch release, and a far worse licensing story than QML), a separate `QQmlEngine` per extension
(items cannot be parented across engines, so it does not work), and reusing the plugin
out-of-process model with a shared surface (no path to compositing a remote process into the
canvas at 60 Hz).

## Affected subsystems & files

| File | Change |
|------|--------|
| `app/src/UI/WidgetExtensions.h` / `.cpp` (new) | `UI::WidgetExtensions` singleton: `Descriptor` struct, `Accepts` struct, `ConfigProperty` struct, `rescan()`, `descriptor(id)`, `contains(id)`, `idsForScope(scope)`, `qmlUrl(id)`, `consentGranted(id)` / `grantConsent(id)`, `builtinReplacement(builtinId)`, signal `catalogChanged()`. Constructor is a leaf — member init only, no `instance()` reach (spec-0001 ctor-edge rule). |
| `app/src/UI/WidgetExtensionManifest.cpp` (new) | Manifest parse + validation split out of the catalog TU: schema check, semver range check against `kWidgetApiVersion`, reserved-id check, dependency resolution. Returns a `Descriptor` or a populated `QList<Misc::ProblemCenter::Finding>`. |
| `app/src/UI/Widgets/ExtensionData.h` / `.cpp` (new) | The one C++ model every extension widget gets. `QQuickItem`, ctor `(int index, QQuickItem* parent)`, `Q_PROPERTY` set: `title`, `groupId`, `sourceId`, `uniqueId`, `isGroupWidget`, `datasets` (`QVariantList` of per-dataset maps), `config` (`QJsonObject` from `widgetSettings`), one `updated()` signal, `Q_INVOKABLE setConfigValue(key, value)`. Wired to `Dashboard::updated`, exactly like `Widgets::Bar` / `Widgets::DataGrid`. |
| `app/src/SerialStudio.h` | `DashboardWidget` gains `DashboardExtension = 100` **after** the `#ifdef BUILD_COMMERCIAL` block with an explicit value, so no existing ordinal moves in either build. `constexpr int kWidgetApiVersionMajor/Minor`. |
| `app/src/SerialStudio.cpp` | `getDashboardWidget(group)` / `getDashboardWidgets(dataset)`: after every builtin string comparison misses, consult `WidgetExtensions` and return `DashboardExtension`. `isGroupWidget` / `isDatasetWidget` / `dashboardWidgetTitle` / `dashboardWidgetIconName` / `groupWidgetEligibleForWorkspace` / `datasetWidgetEligibleForWorkspace` handle the new value. `commercialCfg()` **unchanged** — extension metadata is never consulted for a license decision. |
| `app/src/UI/Dashboard.h` / `.cpp` | `buildWidgetGroups` / `processDatasetIntoWidgetMaps` bucket extension entries under `DashboardExtension` and record the owning extension id in a parallel `QVector<QString> m_extensionGroupIds` / `m_extensionDatasetIds` (index-aligned with the bucket). New `[[nodiscard]] QString extensionIdAt(bool group, int relativeIndex) const`. `applyDisplayTitles` / `refreshDisplayTitles` emit `"ext:<id>:<uid>"` as the type token for `DashboardExtension`. `buildValuePushes` adds a widget's `ExtensionData` to `string_targets` when its descriptor declares `readsStringValues`. |
| `app/src/UI/DashboardWidget.cpp` | One new `case SerialStudio::DashboardExtension:` — `m_qmlPath = extensions.qmlUrl(id)`, model `new Widgets::ExtensionData(relativeIndex(), this)`. The two converted builtins' cases lose their `new Widgets::Compass/DataGrid(...)` and take the same path via `builtinReplacement()`. |
| `app/src/UI/Widgets/Compass.h` / `.cpp`, `DataGrid.h` / `.cpp` (deleted) | Replaced by the bundled packages. Removal is the parity proof; keeping them would leave two unreachable classes. |
| `app/src/UI/Taskbar.cpp` | Extension widgets need a title and icon in `TaskbarModel` rows: read them from the descriptor rather than `SerialStudio::dashboardWidgetTitle/Icon` when the type is `DashboardExtension`. |
| `app/src/DataModel/ProjectEditor.cpp` | `m_groupWidgets` / `m_datasetWidgets` combo maps gain one entry per installed extension whose `scope` and `accepts` match, rebuilt on `WidgetExtensions::catalogChanged`. |
| `app/src/DataModel/Project/ProjectEditorSummaries.cpp`, `ProjectModelWorkspaces.cpp`, `app/src/API/Handlers/WorkspacesHandler.cpp` | The four places that mirror the dashboard's widget-type bucketing for workspace/relative-index math must know that `DashboardExtension` buckets by extension id. No change to the `Plot3D`/`Painter` license remaps that live alongside. |
| `app/src/Misc/ExtensionManager.cpp` | `extensionTypes()` gains `"widget"`; `friendlyTypeName()` gains `tr("Widget")`. Nothing else — install/uninstall/update/platform resolution already work for an arbitrary type string. |
| `app/src/Misc/Problems/ExtensionCheckers.h` / `.cpp` (new) | Registers checker `extension.widget` (trigger `ProjectChanged | OnDemand`) reporting manifest, compatibility, dependency, and QML-compile failures collected by the catalog, plus project references to widget ids that are not installed. |
| `app/src/Misc/ProblemCenter.cpp` | One line in `setupExternalConnections()` registering the new checker group. |
| `app/src/Misc/ModuleManager.cpp` | Instantiate `UI::WidgetExtensions` in `instantiateCoreModules()` (before `Dashboard`, after `ProjectModel` — see Architecture); `qmlRegisterType<Widgets::ExtensionData>("SerialStudio", 1, 0, "ExtensionDataModel")`; context property `Cpp_UI_WidgetExtensions`; connect `ExtensionManager::extensionInstalled/extensionUninstalled` → `WidgetExtensions::rescan()`. Remove the two deleted widget registrations. |
| `app/qml/MainWindow/Panes/Dashboard/WidgetDelegate.qml` | Add the `Component.Error` branch: instantiate `ExtensionPlaceholder.qml` and report through `Cpp_Misc_ProblemCenter`. |
| `app/qml/MainWindow/Panes/Dashboard/ExternalWidgetWindow.qml` | Same error branch; its hardcoded `widgetQmlPath` string-compare size table gains a descriptor-driven default for extension widgets. |
| `app/qml/Widgets/Dashboard/ExtensionPlaceholder.qml` (new) | The visible failure tile: widget name, cause, "Open Problem Center" button. |
| `app/qml/Widgets/Dashboard/ExtensionWidgetSettings.qml` (new) | Generic config form rendered from the descriptor's `config` declarations, writing through `ProjectModel::saveWidgetSetting`. Launched from `WidgetToolbar`. |
| `app/qml/Dialogs/ExtensionConsent.qml` (new) | First-load consent: title, author, source URL, install path, the privileges statement, Allow / Don't allow. Decision stored per `<id>@<version>` in `QSettings`. |
| `app/rcc/extensions/schema/widget-manifest.json` (new) | JSON Schema for the `widget` block, used by the C++ validator and by `registry-verify.py`. |
| `app/rcc/extensions/widget/compass/` (new) | Bundled package: `info.json` (`replaces: "compass"`) + `Compass.qml`. |
| `app/rcc/extensions/widget/datagrid/` (new) | Bundled package: `info.json` (`replaces: "datagrid"`) + `DataGrid.qml`. |
| `app/rcc/rcc.qrc`, `app/CMakeLists.txt` | Register the schema + the two bundled packages and the new sources; drop the deleted ones from `SOURCES` / `HEADERS` / `QML_SOURCES`. |
| `scripts/registry-verify.py` | New `check_widget_manifests(errors)`: every bundled manifest validates against the schema, ids are unique and not reserved, `replaces` appears only in bundled packages, every declared `qml` and `files` entry exists, every icon id resolves through the icon registry. |
| `examples/widget-extension/` (new) | The authoring example a third party copies: manifest, QML, README. |
| `doc/help/Extensions.md`, `doc/help/Widget-Extension-Development.md` (new), `doc/help/help.json` | User-facing type documentation + the authoring guide, registered as a manual page. |
| `tests/scripts/test_widget_manifests.py` (new) | Static, runnable without the app: schema validation of the bundled manifests, reserved-id enforcement, `qrc` sync. |
| `tests/integration/test_widget_extensions.py` (new) | AC2/AC5/AC7 coverage over the live API. |
| `tests/README.md` | Catalog rows for the two new test files. |
| `doc/claude/architecture/dashboard.md`, `CLAUDE.md` | Record the catalog, the identity rules, and the trust model. |

## Architecture & data flow

```
Misc::ExtensionManager  ──installed/uninstalled──►  UI::WidgetExtensions  (catalog, main thread)
   (download / install / update, unchanged)               │ rescan(): qrc packages, then
                                                          │ <workspace>/Extensions/widget/*/info.json
                                                          │ validate -> Descriptor | Findings
                                                          ├──► Misc::ProblemCenter (findings)
                                                          ├──► DataModel::ProjectEditor (picker lists)
                                                          └──► SerialStudio::getDashboardWidget*()
                                                                       │
UI::Dashboard::reconfigureDashboard                                    │ id -> DashboardExtension
   buildWidgetGroups  ── bucket under DashboardExtension ──────────────┘
   registerWidgets    ── WidgetRegistry rows, unchanged
        │
QML WidgetDelegate ─► UI::DashboardWidget::setWidgetIndex
                        buildWidgetForType():
                          m_qmlPath   = extensions.qmlUrl(id)      (file:// or qrc:)
                          m_dbWidget  = new Widgets::ExtensionData(relativeIndex, this)
        │
QML Qt.createComponent(widgetQmlPath)
        ├── Ready  -> createObject({model, windowRoot, color, widgetId})   <- unchanged contract
        └── Error  -> ExtensionPlaceholder.qml + ProblemCenter finding
```

**Manifest.** The existing `info.json` gains `"type": "widget"` and one nested object; every
other key (`id`, `title`, `description`, `author`, `version`, `license`, `category`,
`screenshot`, `files`, `platforms`) keeps its current meaning and its current handling in
`ExtensionManager`.

```jsonc
{
  "id": "com.acme.thermal-map",
  "type": "widget",
  "title": "Thermal Map", "author": "Acme", "version": "1.2.0", "license": "MIT",
  "files": ["info.json", "ThermalMap.qml", "icon.svg"],
  "widget": {
    "apiVersion": "1.0",              // manifest grammar the package was written against
    "hostCompat": ">=1.0 <2.0",       // host widget-API range this package supports
    "scope": "dataset",               // "dataset" | "group" — which picker it appears in
    "qml": "ThermalMap.qml",          // entry, must resolve inside the package directory
    "icon": "icon.svg",               // package-relative, or an icon-registry id
    "accepts": {
      "datasets": { "min": 1, "max": 1 },
      "value": "numeric"              // "numeric" | "string" | "any"
    },
    "readsStringValues": false,       // drives buildValuePushes string-target registration
    "defaultSize": { "width": 400, "height": 300 },
    "config": [                       // spec-0036 declaration vocabulary, scalar subset
      { "id": "palette", "type": "choice", "default": "inferno",
        "label": "Palette", "options": ["inferno", "viridis", "grey"] },
      { "id": "smoothing", "type": "double", "default": 0.25,
        "label": "Smoothing", "min": 0, "max": 1 }
    ],
    "dependencies": {
      "required": [{ "id": "com.acme.colormaps", "version": ">=1.0" }],
      "optional": []
    },
    "experimental": false
  }
}
```

`"replaces": "<builtin widget string>"` is an additional key accepted **only** in bundled
packages; the validator rejects it on any package loaded from disk, and `registry-verify.py`
rejects it in any manifest outside `app/rcc/extensions/`.

**Eager metadata, lazy instantiation.** `rescan()` reads and validates every manifest —
JSON parse, schema check, semver range, reserved-id check, dependency resolution, and a
`QFile::exists` on the declared QML entry. It never touches `QQmlComponent`. Compilation
happens inside `Qt.createComponent()` in the delegate, the first time a project actually
places that widget, and Qt caches the compiled component for subsequent instances.

**Identity, and why it is stable.** Two separate mechanisms, for two separate needs:

- *Third-party widgets* use `DashboardExtension = 100`. Because `m_widgetGroups` /
  `m_widgetDatasets` are `QMap`s keyed by the enum, extension widgets always iterate **after**
  every builtin, so installing or removing a package can never shift a builtin widget's
  `widgetIndex`. Within the bucket, entries sort by `(extensionId, existing group/dataset
  order)`, which is independent of install order. Persisted keys (workspaces, freeze title
  mode, display titles, per-widget settings) substitute `"ext:<extensionId>"` for the numeric
  type token, so they survive uninstall/reinstall of unrelated packages.
- *The two converted builtins* keep their existing enum values via `replaces`. `getDashboardWidget`
  still maps `"compass"` and `"datagrid"` to `DashboardCompass` / `DashboardDataGrid`; only
  `buildWidgetForType` changes, resolving the QML and the model from the bundled package.
  Nothing about ordering, persisted keys, or project files moves — which is the whole reason
  for choosing this over converting them to `DashboardExtension`.

**Composition-root placement.** `UI::WidgetExtensions` is constructed in
`ModuleManager::instantiateCoreModules()` **after `ProjectModel`, before `Dashboard`**. Its
constructor initializes members only (no scan, no `instance()` call, no `QSettings` read); the
first `rescan()` and the `ExtensionManager` connections are made from
`setupCrossModuleConnections()`, matching the `ProblemCenter` precedent. It therefore adds no
outgoing constructor edge and the spec-0001 ordering proof re-runs trivially.

**Config storage.** Declared config values live in the existing per-project widget-settings
store: `ProjectModel::widgetSettings(widgetId)` / `saveWidgetSetting(widgetId, key, value)`,
where `widgetId` is the `"type:groupId:datasetIndex"` string `DashboardWidget::widgetId()`
already produces. That gives per-project scoping, the existing 1.5 s debounced autosave, and —
per spec 0031 — correct exclusion from undo history, at the cost of no new persistence code.

## Hotpath & threading impact

- **Touches the hotpath?** **No.** `FrameReader`, `CircularBuffer`, `FrameBuilder`, the span
  fast lane, and `Dashboard::hotpathRxFrame` are untouched. The catalog is consulted in
  `SerialStudio::getDashboardWidget*()`, which runs at **reconfigure** time only, and the
  lookup is a `QHash` probe reached only after every builtin string comparison has missed —
  so projects with no extension widget pay one already-failing comparison chain, unchanged.
  `Widgets::ExtensionData` refreshes on `Dashboard::updated` (UI cadence, 1-240 Hz), never on
  the frame path, exactly like `Widgets::Bar` and the class it replaces. The one hotpath-
  adjacent edit is `buildValuePushes`: an extension group widget that declares
  `readsStringValues` is added to the `string_targets` set. That set is built at reconfigure;
  the per-frame walk is unchanged, and a widget that does not declare the flag adds no target,
  so the existing "string values are written only where observable" invariant holds. This is
  precisely the gotcha `dashboard.md` warns about, made declarable instead of discoverable.
- **New cross-thread signal/slot?** No. `ExtensionManager`, `WidgetExtensions`, `Dashboard`,
  `ProjectEditor`, and `ProblemCenter` are all main-thread; every new connection is a default
  (auto → direct) same-thread connection. Extension QML runs on the GUI thread like all QML.
- **New input to a cached hotpath flag?** No. No new flags, and none of `m_operationMode`,
  `m_anyAsyncSink`, `m_captureLatestFrame`, `m_changeDriven`, or `m_streamAvailable` gains an
  input. A catalog change triggers a normal dashboard reconfigure through the existing
  project-changed path.
- **Timestamp ownership** — untouched. Extension widgets read already-stamped values from the
  dashboard's model accessors; nothing re-stamps.

## Data model & persistence

- **No new `Keys::` entries and no project-schema bump.** A group's `widget` string and a
  dataset's `widget` string already carry arbitrary text; an extension id is just a value the
  builtin maps do not claim. A project referencing an uninstalled id currently resolves to
  `DashboardNoWidget` and the widget silently vanishes; after this change it resolves to a
  placeholder plus a problem-center finding.
- **Reserved ids.** The validator refuses any package id equal to a builtin group or dataset
  widget string (`datagrid`, `map`, `gps`, `gyro`, `gyroscope`, `multiplot`, `accelerometer`,
  `plot3d`, `image`, `painter`, `webview`, `terminal`, `clock`, `stopwatch`,
  `notification-log`, `led-panel`, `bar`, `gauge`, `compass`, `meter`) unless the package is
  bundled and declares `replaces` for that exact string. This is the concrete mechanism behind
  R10: no extension id can ever resolve to a Pro builtin.
- **Persisted keys** gain the `"ext:<id>"` type token described above. Existing keys are
  untouched, including for the two converted widgets.
- **Consent decisions** persist in `QSettings` under `WidgetExtensionConsent/<id>` storing the
  approved version string, so a package update re-asks.
- **`installed.json`** and the per-platform `files` resolution are reused verbatim.

## API / SDK surface

- No new commands. `extensions.list` / `getInfo` / `install` / `uninstall` already cover
  widget packages once `"widget"` is a known type, and `problems.list` already surfaces the
  findings.
- `project.group.update` / `project.dataset.update` accept an extension id in the `widget`
  field with no change, because the field is a free string today. The dataset and group verb
  descriptions gain one sentence naming installed extension ids as legal values.
- **No new licensing surface.** No handler consults `WidgetExtensions` for a license decision,
  and `SerialStudio::activated()` gains no call site.

## QML / UI

- **The widget contract is unchanged**, and that is the point: an extension's root item
  declares the same four properties every builtin declares (`color`, `windowRoot`, `widgetId`,
  `model`), with `model` typed `ExtensionDataModel` instead of a per-widget type. The optional
  `hasToolbar` mirror works as-is.
- Extension QML may `import QtQuick`, `import QtQuick.Controls`, and
  `import SerialStudio 1.0`. It may **not** rely on the app's internal QML components: those
  are reachable only by relative path inside the compiled `gui` module, so a disk-loaded file
  cannot import them. v1 does not publish a component library for extensions; the authoring
  guide says so plainly.
- **Context hardening (a speed bump, not a boundary).** Each extension component is created in
  its own `QQmlContext` whose `Cpp_*` names are shadowed to `undefined`, so casual access to
  the project model, licensing objects, and I/O manager fails. Extension QML can still reach
  equivalent capability by other routes; the consent dialog, not this measure, is what makes
  the trust model honest, and the code comment must say so.
- New QML: the placeholder tile, the generic config form, the consent dialog. `WidgetDelegate`
  and `ExternalWidgetWindow` each gain an error branch.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Extension code format | C++ plugin (`QPluginLoader`); QML file; out-of-process | **QML file** — no ABI contract to maintain across Qt patch releases, no compiler on the author's machine, and it drops straight into the delegate's existing `Qt.createComponent` path. A C++ plugin would also make the GPL question far worse. |
| Enum strategy | new value per extension; one `DashboardExtension`; string-keyed types everywhere | **One explicitly-valued `DashboardExtension = 100`** — dynamic values would change across sessions and break persisted keys; replacing the enum with strings is a repo-wide refactor for no user-visible gain. The high explicit value guarantees no renumbering in either build and puts extension widgets last in map order. |
| Converted builtins' identity | convert to `DashboardExtension`; keep the enum via `replaces` | **`replaces`** — converting would move both widgets to the end of the widget map and change their persisted type token, forcing a workspace/display-title migration for existing users. `replaces` keeps ordering, keys, and project files byte-identical, and still exercises the whole extension path. |
| Data access | per-widget generated model; one generic model; direct QML access to `Cpp_UI_Dashboard` | **One generic `ExtensionData`** — direct dashboard access would make every extension depend on singleton internals and defeat context hardening; per-widget generated models are R2/R6 work with no v1 payoff. |
| Config persistence | new project-JSON section; existing `widgetSettings` | **Existing `widgetSettings`** — per-project scoping, debounced autosave, and undo exclusion all come free, and no schema change is needed. |
| Config declaration | new mini-language; spec-0036 vocabulary subset | **0036 subset** — the roadmap's stated dependency; forking the vocabulary would guarantee two dialects. v1 supports scalars and fixed choices only. |
| Script hook | reuse the JS watchdog engine; none | **None** — the QML file already has strictly more reach than a watchdogged JS hook, so adding one buys no containment while implying some. Revisit only if a real boundary is ever built. |
| Bundled package location | seed to the workspace on first run; `qrc` | **`qrc`** — a bundled package cannot then be deleted, corrupted, or shadowed by a disk package, which matters because one of them is the GPL fallback target for Painter groups. |
| Trust model | curated-only; signed-only; consent-gated | **Consent-gated, signing recommended** (open question) — curated-only kills the feature's point, and signing infrastructure is a larger commitment than v1 needs; consent is honest and cheap. |
| Failure reporting | log + skip; problem center | **Problem center** — it is already implemented, already has an API verb, a panel, a taskbar badge, and jump-to-entity, and the roadmap named it as R5's reporting surface. |

## Risks & mitigations

- **The parity conversion ships a visual or behavioral regression in two widgets that
  currently work.** Highest-consequence risk. Mitigation: `replaces` keeps every identity and
  ordering invariant fixed so only rendering can differ; the converted QML starts as a verbatim
  copy of the existing file with only its `model` property type changed; AC8 compares against
  the previous build on the example projects; the bundled packages live in `qrc` so they cannot
  be missing at runtime.
- **`ExtensionData` does not actually cover what `DataGrid` needed.** The conversion is the
  test of the data API, and discovering a gap late would strand the design. Mitigation: build
  `ExtensionData` against the two widgets' current C++ *first*, as its own task, before either
  QML file is written; if a value cannot be exposed generically, that is a design stop, not a
  workaround.
- **String-value staleness.** An extension group widget that reads `Dataset::value` without
  declaring `readsStringValues` renders stale strings — the exact failure `dashboard.md`
  documents. Mitigation: the flag is declared, `registry-verify.py` warns when a package's QML
  references a value field without the flag, and the converted `DataGrid` (which needs it)
  exercises the path.
- **Consent fatigue turning into rubber-stamping.** A dialog nobody reads is not consent.
  Mitigation: it appears once per package version, not per session or per widget instance;
  bundled packages never trigger it; the wording names the concrete privilege, not a generic
  warning.
- **A hostile package.** This is the accepted residual risk of the whole feature, and there is
  no mitigation inside v1 beyond consent, reserved ids, context hardening, and the maintainer's
  answer on signing. The spec states it; the documentation must state it; nothing may claim
  otherwise.
- **Catalog rescan racing a live dashboard.** An uninstall while the widget is on screen must
  not leave a dangling component. Mitigation: `rescan()` only mutates the catalog and emits
  `catalogChanged`; teardown goes through the existing reconfigure path, which already deletes
  and rebuilds every `DashboardWidget` model via `deleteLater()`.
- **Ctor-edge proof.** Adding a module to `instantiateCoreModules()` re-triggers the spec-0001
  check. Mitigation: the constructor is a leaf by construction (member init only), and the
  proof is re-run as an explicit task step.
- **Scope creep into a component library.** The first third-party author will ask for the app's
  QML widgets. Mitigation: named as a non-goal, and the authoring guide states the import
  surface explicitly.
- **`code-verify.py` on new QML and C++.** New files must clear the linter on first pass
  (100 columns, no in-body comments, `[[nodiscard]]`, header ordering, SPDX). Mitigation:
  `--check` runs per task.

## Test & verification plan

- **Unit (I can run):** `tests/scripts/test_widget_manifests.py` — bundled manifests validate
  against `widget-manifest.json`; ids are unique and non-reserved; `replaces` appears only in
  bundled manifests; every `files`, `qml`, and `icon` entry resolves; `rcc.qrc` lists every
  bundled file. Plus `python scripts/registry-verify.py`.
- **Integration (maintainer runs; app up with the API server on 7777):**
  `tests/integration/test_widget_extensions.py` —
  - **AC2** — a dataset-scope package is rejected as a group `widget` value and accepted as a
    dataset one; a package declaring one dataset is rejected for a three-dataset group.
  - **AC5** — the five seeded failure cases each yield exactly one `problems.list` entry with
    the expected `checkerId` and `code`, and the app stays up.
  - **AC7** — a package whose id collides with `plot3d` / `image` / `painter` fails to
    register; a project using it does not gain a Pro widget.
  - **AC8** — `project.open` an example using Compass and DataGrid, `project.exportJson`, and
    byte-compare against a baseline captured before the change.
- **Maintainer observations:** **AC1** (install from a local repo folder, then relocate the
  app), **AC3** (config form round-trip), **AC4** (startup with ten unused packages),
  **AC6** (consent flow, decline, restart, version bump), **AC8** visual parity,
  **AC10** (move/resize/pop-out/freeze/workspace/retitle), **AC11** (author a widget from the
  example and the guide alone).
- **Hotpath:** `--benchmark-hotpath` before and after, on a project with no extension widget
  and on one using the converted DataGrid, for **AC9**. No parse-path edit is expected; the
  gate covers the `buildValuePushes` and widget-map adjacency.
- **Static:** `python scripts/code-verify.py --check` on every changed file;
  `python scripts/registry-verify.py`; `qt-cpp-review` on the C++ diff;
  `python scripts/sanitize-commit.py` before commit.
</content>
