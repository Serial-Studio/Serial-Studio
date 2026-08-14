---
spec: 0052-bar-panel-alarm-visuals
phase: plan
status: approved     # draft -> approved (gate before /ss-tasks)
updated: 2026-08-12
---

# Plan 0052 — Bar Panel group widget + severity-first alarm visual language

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Grounded in three code surveys (registration touch-points via
> DataGrid/LEDPanel, current Bar/Gauge/Meter rendering + color routing, property/persistence
> pipelines); key claims spot-checked against the code on 2026-08-12.

## Approach (one paragraph)

Add `Widgets::BarPanel`, a free group widget following the LEDPanel/DataGrid pattern
(QQuickItem model validated against the Dashboard bucket, fed by `Dashboard::updated`,
rendered by a declarative QML `Repeater` of rows), persisting one new group document field
`barPanelStyle` (auto/horizontal/vertical) via the `webViewUrl` precedent. Rework band
rendering in the three existing instrument QML files (Bar/Gauge/Meter) to the painter
doctrine: bands drawn full-extent as muted zones, fill/needle tinted by the active band's
severity (nearest-band clamp when the value sits outside every band — computed in
`Widgets::Bar`, which Gauge/Meter already subclass). Default color for non-plot widgets
collapses to the theme's first widget color through the single `DashboardWidget::widgetColor()`
choke point (plus the two direct non-plot callers); explicit overrides and plot-family cycling
untouched. Extreme-hold (R10) is a new declarative dataset property (`extremeHold`) tracked
exactly at ingest via an opt-in push-table in `Dashboard` (frame lane) and block envelopes
(stream lane), exposed per `uniqueId`, cleared on `dataReset`.

## Affected subsystems & files

New files:

| File | Role |
|------|------|
| `app/src/UI/Widgets/BarPanel.h/.cpp` | Group widget model: per-row title/units/value/frac/severity/band-list/extremes, style enum |
| `app/qml/Widgets/Dashboard/BarPanel.qml` | Repeater-of-rows rendering, horizontal + vertical layouts |
| `app/rcc/icons/widgets/{16,24,32,48}/barpanel.svg` | Widget icon tiers |
| `app/rcc/icons/editor/{16,24,32,48}/add-barpanel.svg` | Editor add-action icon |

Edited files (grep-confirmed touch-points):

| File | Change |
|------|--------|
| `app/src/SerialStudio.h` | `DashboardWidget::DashboardBarPanel` (append before `DashboardExtension=100`), `GroupWidget::BarPanel` (append at end — ordinals are API state) |
| `app/src/SerialStudio.cpp` | `isGroupWidget` (:99), `dashboardWidgetIconName` (:151), `groupWidgetEligibleForWorkspace` (:255), `dashboardWidgetTitle` (:292), `getDashboardWidget` string map (:396, `"barpanel"`), `groupWidgetId`/`groupWidgetFromId` (:566/:610); new `getDatasetAccentColor(const Dataset&)` |
| `app/src/UI/Dashboard.h/.cpp` | `buildWidgetGroups` generic append covers the new enum (:2427); `buildValuePushes` string_targets block for BarPanel groups (mirror DataGrid :2728-2733); extremes store + opt-in push list (built in `buildValuePushes`/`buildDatasetReferences` pass, updated in `updateDashboardData`, stream lane in `applyStreamUpdate`), getter `datasetExtremes(uniqueId)`, cleared where `dataReset` emits (:1457) |
| `app/src/UI/DashboardWidget.cpp` | `buildWidgetForType` case → BarPanel model + qml path (:427); `builtinWidgetId` untouched (not a replaceable package); `widgetColor()` non-plot branch → `getDatasetAccentColor` (:173-184) |
| `app/src/UI/Widgets/Bar.h/.cpp` | Nearest-band clamp in `recomputeActiveBand`/`bandIndexFor` fallback; extreme-hold properties (`extremesEnabled`, `maxSeen`, `minSeen` fractions) read from Dashboard store |
| `app/src/UI/Widgets/Gauge.cpp`, `Meter.cpp` | Inherit clamp; no structural change (both subclass Bar) |
| `app/qml/Widgets/Dashboard/Bar.qml` | Bands full-length muted on well (replace edge strips :322-347), fill gradient severity-tinted (:296-301), extreme tick markers |
| `app/qml/Widgets/Dashboard/Gauge.qml` | Band arcs widened to full ring zones (:370-430), needle severity tint (:721-770), extreme ticks |
| `app/qml/Widgets/Dashboard/Meter.qml` | Same treatment (:349-414 bands, :740-800 needle) |
| `app/src/UI/Widgets/LEDPanel.cpp` | Legacy `severity == -1` / no-band off-color → accent resolver (:242-279) |
| `app/qml/Widgets/Dashboard/Output/DashboardOutputPanel.qml` | `index+1` palette cycling → accent (:84) |
| `app/src/Misc/ModuleManager.cpp` | `qmlRegisterType<Widgets::BarPanel>` (:579-595 block) |
| `app/CMakeLists.txt` | SOURCES (.cpp :249), headers (:436), QML_SOURCES (:671) |
| `app/src/DataModel/Frame.h` | `Keys::BarPanelStyle`, `Keys::ExtremeHold`; `Group::barPanelStyle` member; widget-gated serialize/read (mirror webViewUrl :1197-1201 / :1392-1396); `Dataset::extremeHold` bool member (serialize/read are generated) |
| `app/rcc/properties/dataset.json` | `ExtremeHold` property entry + `formIdOrder` + form builder row; `visibleWhen` predicate |
| `app/src/DataModel/Project/PropertyHooks.h/.cpp` | `extremeHoldApplicable` predicate (bar/gauge/meter dataset widgets or barpanel group; pattern :534-540) |
| generated: `app/src/DataModel/Generated/{DatasetRegistry.h,DatasetSerialization.cpp,DatasetForm.cpp}`, `app/src/API/Generated/DatasetApiFields.cpp` | regenerated via `scripts/generate-property-registry.py`, never hand-edited |
| `app/src/DataModel/ProjectEditor.h/.cpp` | `m_groupWidgets` entry `"barpanel" → tr("Bar Panel")` (:706-716); editability gates `currentGroupIsEditable` (:218) + `datasetWidgetEditable` (:247) |
| `app/src/DataModel/Project/PropertyHooks.cpp` | `widgetSelectable` gate (:524) |
| `app/src/DataModel/Project/ProjectEditorForms.cpp` | `buildGroupModel` style-combo row gated on `widget == "barpanel"` (pattern :283-348); combo row |
| `app/src/DataModel/Project/ProjectEditorItemIds.h` | `kGroupView_BarPanelStyle` (:64-77) |
| `app/src/DataModel/Project/ProjectEditorCommit.cpp` | `kWidgetEnumMap` pair (:433-444); `onGroupItemChanged` arm for the style combo (webViewUrl template :337-344) |
| `app/src/DataModel/Project/ProjectModelCrud.cpp` | `applyGroupWidget` (:1703), `confirmGroupWidgetChange` compatible source+target lists (:1677-1681), `ensureValidGroup` (:1323-1327) |
| `app/src/DataModel/Project/ProjectEditorTree.cpp` | Group tree icon (:481) |
| `app/src/Misc/Problems/ProjectCheckers.cpp` | Nothing (BarPanel groups need datasets — not self-drawing) |
| `app/src/UI/WidgetExtensions.cpp` | `reservedIds()` += `"barpanel"` (:114-139) |
| `app/rcc/extensions/schema/widget-manifest.json` | `reservedId` enum += `"barpanel"` (:34-57) |
| `app/rcc/rcc.qrc` | Icon entries (widgets + editor tiers) |
| `app/rcc/commands/projecteditor.json`, `app/rcc/commands/layouts/project-toolbar.json`, `app/qml/Commands/ProjectEditorCommandBindings.qml` | `editor.addBarPanel` command + binding + toolbar row |
| `app/qml/ProjectEditor/Views/GroupTemplateMenu.qml`, `FlowDiagram.qml` | Add-group menu entries |
| `app/qml/MainWindow/Panes/Dashboard/ExternalWidgetWindow.qml` | Default window size row for BarPanel.qml (:245-280) |
| `app/src/API/EnumLabels.cpp` | `groupWidgetSlug/Label` (:247-303), `dashboardWidgetSlug/FromSlug` (:394-520) |
| `app/src/API/Handlers/ProjectHandlerEntities.cpp` | widgetType range doc/validation (:168-198) |
| `app/src/API/Handlers/WorkspacesHandler.cpp`, `ProjectHandlerBatch.cpp` | `compatibleWidgetTypes` (:214-231 / :135-152) |
| `app/rcc/api/prelude.js`, `app/rcc/api/SerialStudio.js` | Group-widget enum tables (:199-200 / :258-259) |
| `app/tests/tst_enum_labels.cpp` | GroupWidget sweep + slug round-trip (:412-470, :570-650) |
| `app/rcc/ai/skills/*.md` + `search_index.json` | Widget mention; index rebuilt by sanitize-commit |
| `doc/help/Widget-Reference.md` | User doc entry (via `ss-docs` conventions) |

## Architecture & data flow

- **BarPanel model** (GUI thread): ctor validates `VALIDATE_WIDGET(DashboardBarPanel, index)`,
  snapshots per-row static data from `GET_GROUP` (title, units, min/max, decimals, band list
  via the same normalization `Widgets::Bar::buildBands` uses — factored into a shared helper
  or duplicated per-row structs), connects `Dashboard::updated → updateData`. `updateData`
  re-reads `numericValue`/`value` per dataset, recomputes per-row frac + active severity
  (nearest-band clamp), pulls extremes from the Dashboard store for opted-in rows, emits one
  `updated()`. Row data exposed as index-addressed Q_PROPERTY lists (LEDPanel idiom).
- **Orientation**: `Group::barPanelStyle` ∈ {`""`/`"auto"`, `"horizontal"`, `"vertical"`}
  reaches the model at ctor; `"auto"` resolved in QML from aspect ratio + row count
  (Bar.qml `isHorizontal` heuristic generalized: rows when wide-ish or many channels,
  columns when tall or few).
- **Severity clamp** (R5): `Widgets::Bar::recomputeActiveBand` falls back to the
  nearest band (painter `bandSev` algorithm) when no band contains the value; all
  downstream consumers (`activeBandSeverity`, `alarmTriggered`, QML tint) inherit it.
  `alarmColorForSeverity(-1)` is never reached when bands exist; the no-bands case keeps
  severity -1 and QML renders plain accent (R7).
- **Muted zones**: QML draws each band full-extent at low alpha over the track/face
  (`Qt.alpha(bandColor, ~0.30)` composited on the well color — the QML equivalent of the
  painter's `mix(sev, track, 0.72)`), custom band color honored; the existing opaque
  ring/border redraws keep edges crisp.
- **Extreme-hold** (R10): `Dashboard` keeps `QHash<int, Extremes>` keyed by `uniqueId`,
  entries created lazily for datasets with `extremeHold`. Frame lane: an opt-in push list
  (pointer + slot pairs, built alongside the other push tables) updated in
  `updateDashboardData` — two compares/stores per flagged dataset per frame, zero-iteration
  when unused. Stream lane: `applyStreamUpdate` folds each block's envelope min/max into the
  same slots (per block, bounded). Cleared at the `dataReset` emit site; preserved across
  reconfigure (keyed by uniqueId, not index). Widgets read `datasetExtremes(uniqueId)` at
  tick rate; no new signals.
- **Single accent** (R6): `SerialStudio::getDatasetAccentColor(dataset)` = valid explicit
  override → override, else `getDatasetColor(1)` (first theme widget color).
  `DashboardWidget::widgetColor()` routes dataset-scope widgets through it except the
  plot-class set {Plot, FFTPlot, Waterfall}; LEDPanel's no-band/legacy off-color and the
  Output panel's per-control accent switch to the same resolver. MultiPlot/Plot3D/GPS
  untouched.

## Hotpath & threading impact

- **Touches the hotpath?** Yes — one bounded addition to `Dashboard::updateDashboardData`
  (GUI-thread ingest, part of the benchmarked dashboard-ingest cost): the extremes push
  list. Opt-in per dataset; with the feature unused the cost is one empty-container check
  per frame. No allocation, no locking, no signal per frame; slots are plain doubles
  resolved at reconfigure into a push table (the sanctioned pattern — common-mistakes
  "per-frame lookups"). I will read `Dashboard.cpp` ingest/reconfigure sections in full and
  let `ss-hotpath` fire before editing; gate with `--benchmark-hotpath`
  (`HOTPATH_DASHBOARD_INGEST_COST` line) before/after.
- **New cross-thread signal/slot?** No. Everything runs on the GUI thread
  (`Dashboard::updated`, `dataReset`, theme signals). Stream-lane extremes ride the existing
  `applyStreamUpdate` call on the display tick.
- **New input to a cached hotpath flag?** No. No changes to `streamAvailable`,
  `m_anyAsyncSink`, operation-mode caches. The extremes push list rebuilds with the other
  push tables in `reconfigureDashboard`/`clearPushTables`, sharing their staleness contract.
- **Timestamp ownership** — untouched; no stamping anywhere in this change.
- Frame lane untouched upstream of Dashboard: no FrameBuilder/FrameReader/CircularBuffer
  edits, no new Frame copies, nothing on the pipeline thread.

## Data model & persistence

- `Keys::BarPanelStyle("barPanelStyle")` — group key, written only when
  `widget == "barpanel"` and value non-default (mirror `webViewUrl`); read tolerant of
  absence (default auto). Old apps ignore the unknown key; old projects load with default.
- `Keys::ExtremeHold("extremeHold")` — dataset key via the property registry manifest:
  `type: bool`, `default: false`, `persist: whenTrue`, `scope: document`,
  `widget: CheckBox`, `visibleWhen: extremeHoldApplicable`, `api: {expose: true,
  name: "extremeHold"}`, undo label "Edit Dataset". Generator rewrites the four dataset
  artifacts; proto ledger appends the field number (append-only, CI-pinned; the number
  materializes on the maintainer's next build — expected `--check-snapshot` local warning).
- `Dataset::extremeHold` + `Group::barPanelStyle` struct members hand-added in `Frame.h`
  (dataset serialize/read generated; group serialize/read hand-written per precedent).
- No writer-version bump: both keys additive and optional.
- No widgetSettings usage for orientation (document state; widgetSettings would no-op
  outside ProjectFile mode and key on positional groupId).

## API / SDK surface

- `GroupWidget::BarPanel` appended (ordinal stable); `EnumLabels` slug `"barpanel"` /
  label "Bar Panel"; `dashboardWidgetSlug` for the DashboardWidget enum.
- `project.group.add` widgetType validation range + doc strings extended;
  `compatibleWidgetTypes` lists in Workspaces/Batch handlers.
- `project.dataset.update` gains `extremeHold` via the generated `DatasetApiFields.cpp`.
- JS enum tables (`prelude.js`, `SerialStudio.js`) extended; `api-schema.json` +
  `proto-fields.json` + SDK surfaces regenerate on the maintainer's build (spec 0047
  pattern — noted as a maintainer AC).
- No commercial gating anywhere (R9).

## QML / UI

- `BarPanel.qml`: WidgetTitleBar + Repeater rows. Horizontal row = title (elided) | track |
  value text; vertical = columns with value above, label below (painter rake). Track: well
  rect, muted band zones, severity fill, extreme tick pair, border redraw. Fonts via
  `Cpp_Misc_CommonFonts` scaling; labels drop/elide below size thresholds (AC8). Spring
  animation on fill like Bar.qml. Theme-reactive through color bindings.
- Bar/Gauge/Meter QML edits keep each file's existing structure (edge-strip Repeater
  replaced by full zones; needle gradient re-anchored on severity color when
  `activeBandSeverity >= 0`).
- Project editor: style combo row (Auto/Horizontal/Vertical) in the group form, gated on
  the barpanel widget; combo restore-race N/A (form model, not live QML combo).
- Editor checkbox "Hold min/max markers" appears for datasets whose widget is
  bar/gauge/meter or whose group is a barpanel.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Peak tracking | tick-sample in widget / ingest push-table / central monitor poll | **Ingest push-table** — tick sampling misses inter-tick transients (the vibration use case); push-table is the codebase's sanctioned per-frame shape, opt-in and near-zero when off |
| Bar Panel rendering | declarative Repeater / QQuickPaintedItem canvas / compose Bar.qml | **Repeater** — matches widget QML idiom, theme-reactive, UI-rate cost fine for realistic channel counts; canvas revisit only if profiling demands |
| Orientation storage | group document field / widgetSettings | **Group field** — editor-native, undoable, survives group reorder; widgetSettings no-ops outside ProjectFile and keys positionally |
| Single-accent mechanism | choke-point branch + accent resolver / flatten theme `widget_colors` | **Choke point** — flattening the palette would kill plot multicolor (spec violation) |
| Accent color source | `widgetColors()[0]` (= `getDatasetColor(1)`) / theme `accent` key | **widgetColors()[0]** — tuned for data display per theme; `accent` is chrome-oriented |
| Severity clamp location | C++ `Widgets::Bar` / per-QML-file JS | **C++ base class** — one implementation feeds Bar, Gauge, Meter, and (via shared struct) BarPanel; QML copies would drift |
| Extremes survival | clear on reconfigure / persist across reconfigure by uniqueId | **Persist by uniqueId** — a layout tweak should not erase a session peak; `dataReset` is the intentional clear |

## Risks & mitigations

- **Dashboard ingest regression** — the one hotpath edit. Mitigation: push-table shape,
  opt-in list, benchmark before/after (`HOTPATH_DASHBOARD_INGEST_COST`), `SS_ASSERT_HOTPATH`
  only where a guard provably ran.
- **`alarmColorForSeverity(-1)` = warning** — severity-tint bindings must gate on
  `activeBandSeverity >= 0` explicitly (agent-confirmed footgun).
- **Group-path drift has no lint** (no registry gate on hand-written Group serialize) —
  the plan adds field + Keys + serialize + read in one task with a round-trip test
  (`tst_frame_serialization` pattern) so a missed serializer is caught by ctest, not users.
- **`alarmTriggered` semantics shift** with nearest-band clamp: a value in a gap between
  bands now adopts the nearest band's severity, so the value-box flash can fire where it
  previously didn't. Painter-consistent and spec-intended (R5); AlarmMonitor notifications
  are untouched (separate tracker). Named here so review sees it.
- **Compatibility lists** (`confirmGroupWidgetChange`, `ensureValidGroup`, three
  editability gates): missing any one silently wipes datasets or locks editing on widget
  switch — each is an explicit task with an in-app check.
- **Enum append-only**: `DashboardWidget` before `DashboardExtension = 100`,
  `GroupWidget` at end; `tst_enum_labels` sweep enforces.
- **Stale strings**: BarPanel shows `Dataset::value` for non-numeric rows → must join
  `string_targets` in `buildValuePushes` (DataGrid block mirrored) or text rows go stale.
- **Translations**: new QML/C++ enter lupdate via CMake lists automatically; `.ts`/`.qm`
  files are the user's — never regenerated by me (trust contract).
- **Scope discipline**: DashboardOutputPanel + LEDPanel accent edits are the only files
  touched outside the widget/registration set, both required by R6; named here as the lane.

## Test & verification plan

- **AC1 (Bar Panel renders)** — maintainer: multi-dataset group in the running app;
  `pytest tests/integration/` project-editor path: `project.group.update` to `"barpanel"`,
  `dashboard.getData` shows the group; new integration case added to the project suite.
- **AC2 (style option persists)** — pytest round-trip: set `barPanelStyle` via API/editor,
  save, reload, assert JSON key + combo state; maintainer visual flip check.
- **AC3 (band zones + severity fill on all four widgets)** — maintainer: drive values
  through bands via `tests/utils/api_client.py` / simulator; visual check light + dark.
- **AC4 (overrange clamp)** — ctest unit on `Widgets::Bar` band logic (new
  `tst_bar_bands.cpp`: containment, gap→nearest, overrange→outermost); maintainer visual.
- **AC5 (single accent vs plot multicolor)** — maintainer: fresh project, mixed widgets;
  assert MultiPlot cycles while instruments share accent; override still honored.
- **AC6 (legacy projects unchanged)** — existing `tst_frame_serialization` /
  `tst_frame_json_legacy` stay green; new round-trip rows for `barPanelStyle` +
  `extremeHold`; pytest project load suite green.
- **AC7 (hotpath)** — maintainer runs `--benchmark-hotpath` (all nine gates + ingest-cost
  line) on the PGO build; expectation: no measurable delta with extremeHold unused.
- **AC8 (small-size legibility)** — maintainer visual, both orientations, both themes.
- **AC9 (extreme markers)** — pytest integration: feed excursion up/down via API, read
  widget state (`dashboard.getData` extremes if exposed, else maintainer visual); data
  reset clears; option-off shows nothing.
- **Static**: `python scripts/code-verify.py --check` on every touched file;
  `scripts/registry-verify.py` (reserved ids, manifests, commands);
  `scripts/generate-property-registry.py` + byte-compare via `sanitize-commit.py`;
  `qt-cpp-review` before handoff.
- **Unit (runnable by me)**: `ctest` against existing build dir once maintainer rebuilds
  (`tst_enum_labels`, `tst_frame_serialization`, new `tst_bar_bands`).
