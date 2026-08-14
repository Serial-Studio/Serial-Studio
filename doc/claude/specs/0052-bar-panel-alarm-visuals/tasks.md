---
spec: 0052-bar-panel-alarm-visuals
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-08-12
---

# Tasks 0052 — Bar Panel group widget + severity-first alarm visual language

> **Phase 3 of 4 — the ordered checklist.** Decompose [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable*. `/ss-implement` works this list top to
> bottom and keeps the status boxes current. Gate: do not start `/ss-implement` until a human
> marks this `approved`.

## Conventions

- One task = one focused, reviewable change. Verify per task; `code-verify.py --check` on
  every touched file is implied even where not restated.
- Icons (T18) need maintainer-supplied or derived SVG artwork; flagged as the one task with
  an external input.

## Tasks

### T1 — Enums + string maps in SerialStudio

- **Files:** `app/src/SerialStudio.h`, `app/src/SerialStudio.cpp`
- **Does:** Add `DashboardWidget::DashboardBarPanel` (append immediately before
  `DashboardExtension = 100` — ordinals are persisted state, never reorder) and
  `GroupWidget::BarPanel` (append at end — ordinals leak into the JSON API). Wire all six
  maps: `isGroupWidget`, `dashboardWidgetIconName` (`"widgets/barpanel"`),
  `groupWidgetEligibleForWorkspace`, `dashboardWidgetTitle` ("Bar Panels"),
  `getDashboardWidget` (`"barpanel"`), `groupWidgetId`/`groupWidgetFromId`.
- **Verify:** `python scripts/code-verify.py --check app/src/SerialStudio.h app/src/SerialStudio.cpp`;
  grep shows every switch listed in plan touched.
- **Deps:** none
- [x] done — `groupWidgetEligibleForWorkspace` needed no edit (default-true exclusion list)

### T2 — Frame.h data model (group + dataset fields)

- **Files:** `app/src/DataModel/Frame.h`
- **Does:** Add `Keys::BarPanelStyle("barPanelStyle")` + `Keys::ExtremeHold("extremeHold")`;
  `Group::barPanelStyle` (QString, empty = auto) with widget-gated serialize/read mirroring
  the `webViewUrl` lines; `Dataset::extremeHold = false` in the bool run (serialize/read for
  datasets are GENERATED — do not hand-write them). Group path has no drift lint: field,
  key, serialize, read all land in this one task.
- **Verify:** code-verify clean; `read(serialize(g))` round-trip reasoning read-back;
  T16 adds the ctest row.
- **Deps:** none
- [x] done

### T3 — extremeHold property manifest + regeneration

- **Files:** `app/rcc/properties/dataset.json`, `app/src/DataModel/Project/PropertyHooks.h`,
  `app/src/DataModel/Project/PropertyHooks.cpp`, generated:
  `app/src/DataModel/Generated/{DatasetRegistry.h,DatasetSerialization.cpp,DatasetForm.cpp}`,
  `app/src/API/Generated/DatasetApiFields.cpp`
- **Does:** Manifest entry `ExtremeHold` (bool, default false, `persist: whenTrue`,
  CheckBox, `visibleWhen: extremeHoldApplicable`, api expose `extremeHold`, undo "Edit
  Dataset") + `formIdOrder` append + form-builder row; new `PropertyHooks::
  extremeHoldApplicable` (dataset widget ∈ {bar, gauge, meter} OR owning group widget ==
  "barpanel", pattern `widgetRangeApplicable`). Run `scripts/generate-property-registry.py`;
  never hand-edit generated TUs. Expected: `--check-snapshot` warns locally until the
  maintainer's build regenerates `api-schema.json`/proto ledger (append-only, lands at next
  free field number).
- **Verify:** `python scripts/registry-verify.py`; generator byte-stable on second run;
  code-verify clean.
- **Deps:** T2 (Keys constant must exist), T1 (barpanel string for the predicate)
- [x] done — used `enabledWhen` (widget-section convention) instead of `visibleWhen`;
  reserved-id pair (WidgetExtensions.cpp + widget-manifest.json) pulled forward from T14
  because registry-verify gates on it; snapshot warning expected until maintainer build

### T4 — Severity clamp + extremes surface in Widgets::Bar

- **Files:** `app/src/UI/Widgets/Bar.h`, `app/src/UI/Widgets/Bar.cpp`
- **Does:** `bandIndexFor`/`recomputeActiveBand` fall back to nearest band (painter
  `bandSev`: containment first, else min distance) so `activeBandSeverity`,
  `alarmTriggered`, and QML tint inherit the clamp; severity stays -1 only when no bands
  exist (QML gates severity tint on `>= 0` — `alarmColorForSeverity(-1)` returns WARNING,
  never call it unguarded). Add `extremesEnabled`/`maxSeen`/`minSeen` Q_PROPERTYs (raw +
  normalized) fed from the Dashboard extremes store in `updateData` (getter added in T6;
  this task can stub against `extremeHold` false). Gauge/Meter inherit — no edits there.
- **Verify:** code-verify clean; logic covered by T16 ctest (gap→nearest,
  overrange→outermost, no-bands→-1).
- **Deps:** T2
- [x] done — extremes read lives in a protected `refreshExtremes(dataset)` all three
  `updateData` overrides call (Gauge.cpp/Meter.cpp got that call; enabled flag read live
  from the dataset copy so no subclass ctor edits); Dashboard::datasetExtremes lands in T6

### T5 — BarPanel widget model (C++)

- **Files:** `app/src/UI/Widgets/BarPanel.h`, `app/src/UI/Widgets/BarPanel.cpp`,
  `app/CMakeLists.txt`, `app/src/Misc/ModuleManager.cpp`
- **Does:** New `Widgets::BarPanel` (QQuickItem, LEDPanel pattern):
  `VALIDATE_WIDGET(DashboardBarPanel, index)` + `GET_GROUP` snapshot (per-row title/units/
  min-max/decimals/band lists — reuse the `BarBand` normalization), `connect(Dashboard::
  updated → updateData)` re-reading values + per-row frac/severity (nearest-band clamp,
  same rules as T4) + extremes; exposes index-addressed QVariantList/typed-list properties +
  `styleMode` (from `Group::barPanelStyle`); one `updated()` per tick, no per-row signals.
  Register in CMake SOURCES/headers and `qmlRegisterType` (name `BarPanelModel`; `BarModel`
  is taken by the dataset Bar).
- **Verify:** code-verify clean on all four files.
- **Deps:** T1, T2, T4
- [x] done

### T6 — Dashboard: bucket coverage, string_targets, extremes store

- **Files:** `app/src/UI/Dashboard.h`, `app/src/UI/Dashboard.cpp`
- **Does:** HOTPATH-ADJACENT — read ingest/reconfigure sections in full first; binding
  invariants: pre-resolved push tables only, no per-frame lookup/alloc/signal, rebuild in
  the matching `configure*`/`buildValuePushes`, drop in `clearPushTables`. (1) confirm
  generic `buildWidgetGroups` append covers the new enum (no code if generic); (2) BarPanel
  groups join `string_targets` in `buildValuePushes` (mirror DataGrid block) so non-numeric
  rows don't read stale strings; (3) extremes store `QHash<int uniqueId, Extremes>` +
  opt-in push list built with the push tables, updated in `updateDashboardData` (two
  compares/stores per flagged dataset per frame; empty-check when unused), stream-lane fold
  in `applyStreamUpdate` from block envelope min/max; `[[nodiscard]] datasetExtremes(uid)`
  getter; clear at the `dataReset` emit site only — the store survives reconfigure (keyed
  by uniqueId).
- **Verify:** code-verify clean; maintainer `--benchmark-hotpath` at DoD (ingest-cost line
  vs baseline); T4 stub replaced with real getter.
- **Deps:** T1, T3 (extremeHold field), T5 (enum bucket exercised)
- [x] done — store is `QMap` (Qt6 QHash values are not address-stable), extremes survive
  reconfigure via the `resetData(notify)` distinction, stream lane folds latest + envelope
  in `applyStreamChannel`

### T7 — DashboardWidget wiring: QML path + accent choke point

- **Files:** `app/src/UI/DashboardWidget.cpp`, `app/src/SerialStudio.h`,
  `app/src/SerialStudio.cpp`
- **Does:** `buildWidgetForType` case for BarPanel (model + `qrc` QML path). Add
  `SerialStudio::getDatasetAccentColor(const Dataset&)` (valid override → override, else
  `getDatasetColor(1)`); `widgetColor()` routes dataset-scope widgets through it EXCEPT
  plot-class {Plot, FFTPlot, Waterfall} which keep `getDatasetColor(dataset)`. Group
  widgets keep transparent.
- **Verify:** code-verify clean; read-back confirms plot-class exception list.
- **Deps:** T1, T5
- [x] done — also added Q_INVOKABLE no-arg `getDatasetAccentColor()` for QML consumers;
  split commercial cases out of `buildWidgetForType` (function crossed the 100-line cap)

### T8 — BarPanel.qml

- **Files:** `app/qml/Widgets/Dashboard/BarPanel.qml`, `app/CMakeLists.txt` (QML_SOURCES),
  `app/qml/MainWindow/Panes/Dashboard/ExternalWidgetWindow.qml` (default size row)
- **Does:** Repeater-of-rows rendering: horizontal (label | banded track | value) and
  vertical rake (value / column / label); `styleMode` auto resolves from aspect ratio +
  row count; muted full-extent band zones (`Qt.alpha(bandColor, ~0.30)` over well), severity
  fill gated on `severity >= 0`, extreme tick pair, elide/drop labels at small sizes,
  CommonFonts scaling, spring fill animation, non-numeric rows show text value.
- **Verify:** code-verify clean (QML rules); visual pass deferred to maintainer ACs.
- **Deps:** T5, T7
- [x] done — fonts via `Cpp_Misc_CommonFonts.widgetFont()` (the `font.*` idiom in Bar.qml
  is baseline debt new files must not copy)

### T9 — Bar.qml band doctrine

- **Files:** `app/qml/Widgets/Dashboard/Bar.qml`
- **Does:** Replace edge strips (:322-347) with full-length muted zones inside the well;
  fill gradient anchors on severity color when `model.activeBandSeverity >= 0` else
  `root.color`; extreme tick markers when `model.extremesEnabled`; keep the opaque
  well-ring redraw above zones. Value-box/text discipline unchanged (neutral until
  warning+).
- **Verify:** code-verify clean.
- **Deps:** T4
- [x] done

### T10 — Gauge.qml + Meter.qml band doctrine

- **Files:** `app/qml/Widgets/Dashboard/Gauge.qml`, `app/qml/Widgets/Dashboard/Meter.qml`
- **Does:** Widen rim-arc bands into full ring zones (painter dial recipe), muted alpha;
  needle stroke/gradient re-anchors on severity color when `severity >= 0` else
  `root.color`; extreme tick marks on the arc when enabled; keep each file's opaque
  face-ring redraws and Meter's ±90° end-clamp behavior.
- **Verify:** code-verify clean.
- **Deps:** T4
- [x] done — Gauge zone ring 0.025→0.055 of face width, Meter 0.035→0.075 of faceR;
  extreme ticks reuse each dial's tick geometry

### T11 — LEDPanel + Output panel accent

- **Files:** `app/src/UI/Widgets/LEDPanel.cpp`,
  `app/qml/Widgets/Dashboard/Output/DashboardOutputPanel.qml`
- **Does:** LEDPanel legacy `severity == -1` / no-band off-color resolves through
  `getDatasetAccentColor`; Output panel per-control `getDatasetColor(index + 1)` cycling →
  accent resolver. Band-driven LED colors unchanged (spec non-goal).
- **Verify:** code-verify clean; both files stay in the R6 lane only.
- **Deps:** T7
- [x] done

### T12 — Project editor: combo, gates, style row, commit

- **Files:** `app/src/DataModel/ProjectEditor.h`, `app/src/DataModel/ProjectEditor.cpp`,
  `app/src/DataModel/Project/ProjectEditorForms.cpp`,
  `app/src/DataModel/Project/ProjectEditorItemIds.h`,
  `app/src/DataModel/Project/ProjectEditorCommit.cpp`,
  `app/src/DataModel/Project/PropertyHooks.cpp`
- **Does:** `m_groupWidgets` entry `"barpanel" → tr("Bar Panel")`; editability gates treat
  barpanel like datagrid (`currentGroupIsEditable`, `datasetWidgetEditable`,
  `widgetSelectable` — three duplicated checks, all must move together); `kGroupView_
  BarPanelStyle` enumerator; style combo row (Auto/Horizontal/Vertical) in
  `buildGroupModel` gated on `widget == "barpanel"`; `kWidgetEnumMap` pair; commit arm per
  webViewUrl template (`setNextUndoHint` + `updateGroup` — undo scope comes from the
  ProjectModel mutator, first `setModified(true)` commits the staged memento).
- **Verify:** code-verify clean; read-back of all three gates.
- **Deps:** T1, T2
- [x] done — commit arm delegated to `applyGroupBarPanelStyleEdit` (and LogX/LogY folded
  into `applyGroupLogAxisEdit`) because `onGroupItemChanged` crossed the 100-line cap

### T13 — ProjectModel CRUD compatibility

- **Files:** `app/src/DataModel/Project/ProjectModelCrud.cpp`,
  `app/src/DataModel/Project/ProjectEditorTree.cpp`
- **Does:** `applyGroupWidget` arm; `confirmGroupWidgetChange` compatible SOURCE and TARGET
  lists both gain barpanel (missing either silently wipes datasets on widget switch);
  `ensureValidGroup` accepts new datasets in barpanel groups; group tree icon.
- **Verify:** code-verify clean; read-back of both compatibility lists.
- **Deps:** T12
- [x] done — tree icon needed no edit (generic via `getDashboardWidget` + T1 icon map)

### T14 — Reserved-id contract + commands + menus

- **Files:** `app/src/UI/WidgetExtensions.cpp`,
  `app/rcc/extensions/schema/widget-manifest.json`, `app/rcc/commands/projecteditor.json`,
  `app/rcc/commands/layouts/project-toolbar.json`,
  `app/qml/Commands/ProjectEditorCommandBindings.qml`,
  `app/qml/ProjectEditor/Views/GroupTemplateMenu.qml`,
  `app/qml/ProjectEditor/Views/FlowDiagram.qml`
- **Does:** `reservedIds()` += "barpanel" and schema `reservedId` enum in lockstep
  (registry-verify fails on drift); `editor.addBarPanel` command manifest + toolbar layout
  row + QML binding (id → handler + handler impl); add-group menu entries. Run
  `scripts/generate-command-strings.py` if command strings are generated.
- **Verify:** `python scripts/registry-verify.py` clean.
- **Deps:** T1
- [x] done — FlowDiagram uses `Action` declarations consumed by three menus; all three got
  the new entry

### T15 — API surfaces

- **Files:** `app/src/API/EnumLabels.cpp`,
  `app/src/API/Handlers/ProjectHandlerEntities.cpp`,
  `app/src/API/Handlers/WorkspacesHandler.cpp`,
  `app/src/API/Handlers/ProjectHandlerBatch.cpp`, `app/rcc/api/prelude.js`,
  `app/rcc/api/SerialStudio.js`
- **Does:** `groupWidgetSlug/Label` + `dashboardWidgetSlug/FromSlug` entries;
  `project.group.add` widgetType range + enum-hint prose; both `compatibleWidgetTypes`
  lists; JS enum tables. (`api-schema.json`/proto/SDK regenerate on maintainer build —
  noted, not hand-edited.)
- **Verify:** code-verify clean; T16 enum-label sweep passes.
- **Deps:** T1
- [x] done — `compatibleWidgetTypes` needed no edit (generic over `getDashboardWidget`);
  adjacent staleness fixed and named in chat: JS `__ssGroupWidgets` tables were missing
  `webview: 9` — added with `barpanel: 10`

### T16 — C++ unit tests

- **Files:** `app/tests/tst_enum_labels.cpp`, `app/tests/tst_frame_serialization.cpp`,
  `app/tests/tst_bar_bands.cpp` (new), `app/tests/CMakeLists.txt`
- **Does:** Extend GroupWidget/DashboardWidget sweeps for the new enumerators + slug
  round-trip; frame round-trip rows for `barPanelStyle` (widget-gated) and `extremeHold`
  (whenTrue persist); new `tst_bar_bands` covering containment, gap→nearest-band,
  overrange→outermost, no-bands→-1, custom band color passthrough.
- **Verify:** `ctest` against existing build dir once maintainer rebuilds; until then
  code-verify clean + read-back.
- **Deps:** T1, T2, T4
- [x] done — PLAN DEVIATION (named in chat): band lookup extracted to header-only
  `app/src/UI/WidgetBands.h` (Bar + BarPanel now delegate) because linking Bar.cpp into a
  ctest suite would drag Dashboard/SessionContext, which the tier forbids; `tst_bar_bands`
  tests the shared header with zero link deps

### T17 — pytest integration coverage

- **Files:** `tests/integration/` (extend the project-editor/dashboard suites per their
  existing layout)
- **Does:** Cases for AC1/AC2/AC9: create group → set widget "barpanel" via API → assert
  `dashboard.getData` shows the widget; set `barPanelStyle`, save/load round-trip; enable
  `extremeHold`, drive excursion via API, assert persistence key round-trip (visual extremes
  stay maintainer ACs). Marked to skip when app not up (`nc -z 127.0.0.1 7777` probe
  convention).
- **Verify:** `pytest tests/integration/<files> -v` against a running app (maintainer or
  sanctioned run when app is up).
- **Deps:** T6, T12, T13, T15
- [x] done — new `tests/integration/test_bar_panel.py` (6 cases, collection clean); needs
  the REBUILT app (running binary predates barpanel/extremeHold), so execution is a
  maintainer step

### T18 — Icons + qrc (external artwork input)

- **Files:** `app/rcc/icons/widgets/{16,24,32,48}/barpanel.svg`,
  `app/rcc/icons/editor/{16,24,32,48}/add-barpanel.svg`, `app/rcc/rcc.qrc`
- **Does:** Widget + editor add icons in all four tiers (derive from bar icon per
  icon-tooling recipe: normalize-viewbox + qrc-sync, request-px = render-size), qrc
  registration in each tier block.
- **Verify:** `python scripts/registry-verify.py` (qrc sync check);
  `scripts/icon-report.py` clean.
- **Deps:** T1 (icon name referenced by `dashboardWidgetIconName`)
- [x] done — pulled forward (command manifest gate needed the icons); placeholder three-bar
  glyph in family palette, `overlay-add-icon.py` badge for the editor icon; maintainer may
  replace artwork

### T19 — Docs + AI corpus

- **Files:** `doc/help/Widget-Reference.md`, `app/rcc/ai/skills/dashboard_layout.md` (and
  siblings that enumerate group widgets), `app/rcc/ai/search_index.json` (rebuilt by
  sanitize-commit, not hand-edited)
- **Does:** Bar Panel entry (what it shows, style option, extreme-hold checkbox, band
  doctrine note for Bar/Gauge/Meter) following ss-docs conventions; AI-corpus group-widget
  lists updated.
- **Verify:** `scripts/documentation-verify.py`; sanitize-commit rebuilds the index.
- **Deps:** T8 (behavior final before documenting)
- [x] done — doc review surfaced a real defect fixed in SerialStudio.h: DashboardBarPanel
  after the commercial ifdef had a build-dependent ordinal (18 GPL / 23 commercial); now
  pinned `= 90` so persisted workspace refs are identical in both builds

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` — ALL NINE are still unrun; each needs the
      rebuilt binary (ctest + pytest suites written and collecting; visual and benchmark
      checks are maintainer-run).
- [x] `python scripts/code-verify.py --check` clean on all changed files (no new errors).
- [x] `qt-cpp-review` run on the C++ diff; findings addressed or noted. Phase 1 lint clean on
      all 28 files. Phase 2's six agents died on an API credit error mid-run, so the review
      was completed inline against the same six missions; findings below.
- [ ] `--benchmark-hotpath` not regressed (MAINTAINER RUN; compare the
      `HOTPATH_DASHBOARD_INGEST_COST` line against the pre-change baseline).
- [x] `pytest` targets listed for the maintainer: `tests/integration/test_bar_panel.py`
      (6 cases), plus the existing `test_project_editor.py` / `test_project_import_export.py`
      suites; needs a REBUILT app with the API server on.
- [x] `python scripts/sanitize-commit.py` run; generated artifacts byte-stable; singleton
      census re-baselined (1679 -> 1680, the BarPanel `Dashboard::instance()` in its ctor,
      matching every sibling widget); `--check-snapshot` warning expected until the
      maintainer's build refreshes `api-schema.json` / the proto ledger.
- [x] Diff is *what was asked, and only that* — the two R6 files outside the widget set
      (LEDPanel.cpp, DashboardOutputPanel.qml) were named in the plan; the only other
      adjacent edit is the `webview: 9` fix in the two JS enum tables, named in T15.
- [ ] `spec.md` status set to `done` — held at `in-progress` until the acceptance runs pass.

## Post-implementation fixes (maintainer-reported, 2026-08-12)

Four defects found while exercising the build. Three are pre-existing bugs outside spec
0052's original lane, kept here because they were fixed in the same pass.

1. **Audio dead in ProjectFile mode** (`IO/ConnectionManager.cpp`, spec-0051 regression).
   `buildStreamConfig` bound each stream channel to the POSITIONAL
   `dataset_unique_id(group, dataset)`, but the dashboard registers stream targets under the
   dataset's PERSISTED `uniqueId` (`allocateUniqueId()`; the loader backfills positional ids
   only for legacy files). Every ProjectFile channel therefore addressed an id nothing was
   registered under. QuickPlot synthesizes both sides with the same function, which is why it
   alone worked. Fix: use `dataset.uniqueId`, positional only as the legacy fallback. This
   also repaired two silent consequences — the data-table store was keyed on the wrong ids,
   and `m_streamDatasetIds` never matched, so stream transforms were double-applied on the
   frame lane.
2. **Waterfall never fed by the stream lane** (`UI/Dashboard.{h,cpp}`,
   `IO/ConnectionManager.cpp`). `StreamTargets` carried plot/multiplot/FFT targets but no
   waterfall, and a waterfall-only dataset never requested an FFT window from the worker —
   the same class of gap spec 0051 M4 hit with sweep/trigger. Fix: `channel.fft =
   dataset.fft || dataset.waterfall` (Pro-gated), a `waterfallIndexes` target list, and a
   feed loop pushing `channel.fftWindow` into `m_waterfallValues` (both FFT and waterfall
   rings hold time-domain samples and transform themselves).
3. **Auto-layout resize refresh — RETRACTED, no fix landed.** I first added a
   `geometryChange()` override to `WindowManager`, believing the resize hook was missing.
   That was wrong: the constructor already connects `widthChanged`/`heightChanged` to
   `triggerLayoutUpdate()` (present in HEAD), so canvas resize was already wired and the
   override was pure duplicate work. The bad diagnosis came from a grep for
   `triggerLayoutUpdate` callers that filtered out any line containing `::triggerLayoutUpdate`
   — which is exactly how those two `connect()` lines are spelled. The override has been
   removed. Leading remaining suspect if the symptom persists: `autoLayout()` early-returns
   while `anyWindowMaximized()`, so a canvas resize taken while a widget window is maximized
   does not re-tile; `WidgetDelegate`'s 250 ms `onStateChanged` timer is what heals it on
   restore. Needs a reproduction against a rebuilt binary before anything is changed.
4. **Bar Panel bars uneven and wiggling on value change** (`BarPanel.qml`, spec 0052's own
   defect). `Layout.preferredWidth` is only a hint — a Text's `implicitWidth` still acts as
   its effective minimum, so a longer value string widened that cell and stole width from the
   `fillWidth` track. Fix: pin the label/value cells with matching
   `Layout.minimumWidth`/`maximumWidth`, give every rake column `Layout.preferredWidth: 1`
   so widths divide equally regardless of content, and render values in the mono font so
   digit changes cannot shift metrics.

5. **Manual-layout seams drifted apart on canvas resize** (`UI/WindowManager.{h,cpp}`,
   `UI/SnapGuides.{h,cpp}`). `applyManualAnchors` scales and rounds every window
   independently (`qRound` on width, height and all four margins), so edges that were flush
   opened 1-4 px gaps as the canvas changed size. Fixed in two composed stages:
   - **Primary — snap the rescale to the ladder.** New `Snap::snapToFraction()` pulls each
     scaled edge onto the nearest wrench stop of the canvas (bounds count as stops) when one
     is within `kSeamWeldTolerance` (6 px). Two windows sharing a seam therefore arrive at
     the *same* coordinate by construction rather than by tolerance guessing, and the layout
     re-canonicalises onto the ladder every resize.
   - **Backstop — `weldManualSeams()`.** Still needed for two reasons: seams sitting at
     coordinates the ladder does not name (windows dragged flush against each other at an
     arbitrary x, which sibling-edge snapping permits), and applying `kSeamSpacing = -1` so
     abutting borders overlap into one shared line instead of drawing two. It clusters edges
     per axis, welds each group onto one coordinate (a cluster touching a canvas bound adopts
     that bound), then re-applies with the shared-border offset.

   Both run only on a real canvas-size change. Freeze mode is unaffected — it gates input,
   not layout. Interactive gestures need no equivalent: a sibling-edge candidate already
   carries `siblingSpacing` and outranks a fraction stop at equal distance
   (`kRankEdge = 0 < kRankFraction = 3`), so when a neighbour is present the flush-with-shared-
   border snap wins, and when none is present there is no border to double.
6. **Wrench-fraction resize stops + preview** (`UI/SnapGuides.{h,cpp}`,
   `UI/WindowManager.{h,cpp}`, `DashboardCanvas.qml`). Fraction snapping already existed for
   both move and resize but ran an n/2..n/8 ladder including thirds, fifths and sevenths.
   Replaced with the imperial wrench ladder `{2, 4, 8, 16}` (every coarse stop is also a fine
   stop, so labels reduce cleanly: 8/16 -> 1/2), and each rung is skipped on an axis whose
   stops would sit closer than `kMinFractionSpacing` (40 px) — sixteenths on a wide canvas,
   quarters on a narrow one. Ties already favour sibling edges (`kRankEdge = 0` beats
   `kRankFraction = 3` in `pickCandidate`), which is the requested precedence, so that needed
   no change. `appendResizeCandidates` snaps the widget's **size** to `extent * num / den`, so
   the geometry genuinely lands on the fraction. Added `Snap::fractionLabel()` for reading a
   size back as a reduced fraction: it answers only when the size is ON a stop (±1 px) and
   returns empty otherwise. That exactness is the point — an earlier revision reported the
   *nearest* stop instead, which on a narrow canvas (where the finest live rung can be
   quarters) labelled a widget 1.3x too large as "1/4". Snap and label now share one formula,
   one rung set and one spacing gate, so they agree by construction. A
   `fractionPreviewRect`/`fractionPreviewLabel` pair on WindowManager renders one translucent
   footprint with a `Width: 1/2    Height: 1/4` chip; only axes actually on a stop are named
   (a side-handle drag shows just that axis), and the label always describes the widget's own
   size, not its placement. The chip may overflow the footprint (a 1/16 tile is narrower than
   the readout) and is clamped inside the canvas. Gated on guides being enabled, and cleared
   through `clearSnapGuides()`, so it inherits the existing release / freeze / clear aborts.

7. **Configurable manual-layout border spacing** (`UI/Dashboard.{h,cpp}`,
   `UI/WindowManager.cpp`, `Dialogs/Settings.qml`, `WidgetDelegate.qml`). Manual gestures were
   borrowing `autoLayoutSpacing` for their flush-snap gap, so the two layout modes could not
   be tuned independently. Added `Dashboard::manualLayoutSpacing` mirroring the auto-layout
   property exactly: `int`, clamped `>= -1`, default -1 (shared border), persisted as
   `Dashboard/ManualLayoutSpacing`, with a Settings spin box beside the auto-layout controls.
   It now feeds the move snap, the resize snap and `weldManualSeams()`, and a
   `manualLayoutSpacingChanged` hook re-welds the live layout so the value applies without a
   gesture. `WidgetDelegate.shadowEnabled` was reading `autoLayoutSpacing` unconditionally;
   it now reads whichever spacing governs the active mode, so flush manual tiles suppress
   their drop shadows exactly as flush auto tiles already did.

8. **Manual layout broke when the project loaded at a different window size**
   (`UI/WindowManager.cpp`). A saved layout persists `canvasWidth`/`canvasHeight`, and both
   restore paths (`applySavedGeometries`, `preloadPendingGeometries`) set the manual
   reference from them — but then applied each saved rect at its SAVED PIXEL SIZE via
   `anchoredGeometry()`, anchoring without ever rescaling. Nothing corrected it afterwards
   because `triggerLayoutUpdate()` gates the manual rescale on `sizeChanged`, which compares
   the current canvas against the LAST canvas, not against the canvas the geometry came
   from. Resize the window before loading and the canvas does not change *during* the load,
   so the rescale never fired; resizing afterwards did fire it, which is why the bug looked
   like "works on resize, broken on load". Fix: one shared `scaledManualGeometry()` helper
   (scale from reference canvas, clamp, anchor) now serves all three sites — both restore
   paths and `applyManualAnchors()`, which previously open-coded the same math. `restoreLayout`
   additionally welds seams so a loaded layout matches a resized one exactly.

## Review findings (inline qt-cpp-review, Phase 2 missions)

- **Fixed — dead code (Ownership/Quality).** `Widgets::Bar::bandIndexFor` and
  `nearestBandIndex` were orphaned by the T16 delegation to `Widgets::Bands` (zero call
  sites: not Gauge, not Meter, not QML — both `protected`, non-invokable). Removed from
  `Bar.h`/`Bar.cpp`.
- **Verified — extremes pointer lifetime (Thread Safety/Ownership).** `ExtremePush` holds
  raw pointers into `m_datasetExtremes` and `m_datasets` (both `QMap`, node-address-stable —
  the same guarantee `m_datasetReferences` already relies on). `clearPushTables()`
  (Dashboard.cpp:1431) runs BEFORE `m_datasets.clear()` (:1461) and
  `m_datasetExtremes.clear()` (:1484), so no push can outlive its targets. All
  `m_datasets` inserts happen in `buildWidgetGroups`, which precedes `buildValuePushes`.
- **Verified — pinned enum ordinal (API correctness).** Every int→`DashboardWidget` /
  `GroupWidget` cast in the repo is a value-preserving switch or map lookup; no loop,
  range-iteration, or threshold comparison against `DashboardExtension` exists, so
  `DashboardBarPanel = 90` breaks nothing. `widgetType` bound is now `0..BarPanel` (10) and
  the range stays contiguous; the existing `range(7)` editor test remains valid.
- **Verified — QML model contract.** `styleMode` / `titles` / `bands` are marked `CONSTANT`
  and are written only inside `buildRows()`, which runs once from the constructor. Ctor
  parenting matches `DataGrid` / `LEDPanel` exactly.
- **Verified — persistence.** `extremeHold` reads with default `false` and writes only when
  true (persist-when-true honored); `barPanelStyle` is widget-gated on both sides.
- **Noted, not fixed — NaN widget ranges (pre-existing).** A project supplying a
  non-numeric `widgetMin`/`widgetMax` (fast_float parses `"nan"`) makes `row.ranged` true
  and `rowFraction` produce NaN, which QML would reject as a geometry value. `Widgets::Bar`
  has had the identical exposure via `computeFractional` since long before this spec;
  BarPanel deliberately matches that behavior rather than diverging. Repo-wide hardening is
  out of this spec's lane — flagged for a future pass.
