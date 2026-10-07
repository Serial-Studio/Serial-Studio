---
spec: 0093-alarm-band-applicability
phase: tasks
status: approved     # T1-T16 and T17-T26 (amendment 1) approved 2026-10-06
updated: 2026-10-06
---

# Tasks 0093 — Alarm-band applicability

> **Phase 3 of 4 — the ordered checklist.** Decompose [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable*. `/ss-implement` works this list top to
> bottom and keeps the status boxes current. Gate: do not start `/ss-implement` until a human
> marks this `approved`.

## Conventions

- `CV` abbreviates `python scripts/code-verify.py --check <the task's files>`; `SC` abbreviates
  `python scripts/syntax-check.py <the task's C++ files>`. A `SC` SKIP (stale compile database)
  is reported as "unverified", never as a pass.
- The maintainer builds and launches the app. ctest and integration lines run only against an
  existing build dir or a running app (Phase F).
- T1 to T9 and T11 stay in the main session. T10, T12 and T13 are pattern work and may run in a
  Sonnet subagent on a half-page brief; the main session reviews what comes back.
- "The rule" always means `SerialStudio::datasetRendersAlarmBands`. No task may restate its
  terms in a second place.
- **SC status (2026-10-06):** this checkout has no compile database, so every `SC` line below
  is unverified. A `build/` directory has since appeared, and the refreshed `api-schema.json`
  carries T9's new wording, which only a dump from a successful build writes: evidence that T1
  to T9 compiled. T14 remains the formal check.
- T26 is pattern work and may run in a Sonnet subagent, like T12 and T13.

## Phase A — The rule (R1)

### T1 — `datasetRendersAlarmBands`

- **Files:** `core/Pipeline/DataModel/WidgetResolution.h`,
  `core/Pipeline/DataModel/WidgetResolution.cpp`
- **Does:** Adds `[[nodiscard]] bool datasetRendersAlarmBands(const DataModel::Dataset&, const
  DataModel::Group&)` in namespace `SerialStudio`: true when `getDashboardWidget(group)` is
  `DashboardBarPanel`; otherwise false when `hideOnDashboard`; otherwise true when
  `getDashboardWidgets(dataset)` contains Bar, Gauge, Meter or LED. **Invariant: it is built on
  the two existing resolvers and names no widget string of its own, so it cannot drift from
  what `WidgetMapBuilder` builds.** One-line `@brief` states it is rebuild/command rate only.
- **Verify:** CV; SC; `python scripts/layer-verify.py` (no new include expected).
- **Deps:** none
- [x] done

### T2 — Unit table for the rule (AC1)

- **Files:** `app/tests/tst_alarm_band_applicability.cpp`, `app/tests/CMakeLists.txt`
- **Does:** New QtTest unit registered like `tst_workspace_rebind` (sources: the test plus
  `WidgetResolution.cpp`; libs `Qt6::Core`, `Qt6::Gui`, `SerialStudio::Core`). Data-driven
  rows: Bar, Gauge, Meter, LED true; empty widget, Compass, plot-only false; hidden Gauge
  false; hidden LED false; Bar Panel member true with and without `hideOnDashboard`; Canvas
  (`painter`) group member with no widget false.
- **Verify:** CV; SC on the test file; the ctest run itself is T14.
- **Deps:** T1
- [x] done

## Phase B — Alarm system (R2, R3, R4, R5, R10)

### T3 — Track only applicable datasets

- **Files:** `core/Ui/UI/AlarmMonitor.h`, `core/Ui/UI/AlarmMonitor.cpp`
- **Does:** `rebuildTrackers` walks the (group, dataset) pairs of `Dashboard::rawFrame()`, the
  same pairs the dashboard builds widgets from, collects the `uniqueId`s of banded datasets
  that pass the rule, and skips every dataset outside that set. Adds `[[nodiscard]] bool tracks(int uniqueId) const noexcept`. Rewords the class
  `@brief`: independent of the widget being on screen, dependent on it being configured.
  **Invariants: `evaluateAlarms` and `processValue` are not edited and never call the rule;
  the loop still iterates `Dashboard::datasets()` and trackers still resolve by `uniqueId`,
  never a cached `Dataset*`; `trackersRebuilt` keeps its parameterless signature and its
  emit point.**
- **Verify:** CV; SC; read-back that the only new call sites of the rule and of `rawFrame()`
  are inside `rebuildTrackers`.
- **Deps:** T1
- [x] done

### T4 — Reap through the monitor

- **Files:** `core/Ui/UI/Alarms/AlarmAnnunciator.h`, `core/Ui/UI/Alarms/AlarmAnnunciator.cpp`
- **Does:** Stores the monitor pointer `setupExternalConnections` already receives
  (`m_monitor`, `nullptr` in the ctor init list). `onTrackersRebuilt` marks a band point stale
  when `m_monitor` is null or `!m_monitor->tracks(id)`, replacing the local `datasets()` /
  `alarmBands.empty()` test. **Invariants: order stays refresh-flag, then the `m_dropHold`
  return, then the reap (spec 0088 R3: a held table is never reaped); no new connection and no
  `instance()` call (singleton census flat); the `.cpp` is at 1484 of 1500 lines, so the edit
  is net-neutral there.**
- **Verify:** CV; SC; `python scripts/code-verify.py --tu-census --check` and
  `--singleton-census --check`.
- **Deps:** T3
- [x] done

### T5 — "Project defines alarms" uses the rule

- **Files:** `core/Ui/UI/Alarms/AnnunciatorChecker.cpp`
- **Does:** `definesAlarms` requires the rule in addition to `group.enabled`, `dataset.enabled`
  and non-empty bands. The sound-map early return is unchanged.
- **Verify:** CV; SC.
- **Deps:** T1
- [x] done

## Phase C — Project Editor (R6, R7)

### T6 — `alarmBandsApplicable` property

- **Files:** `core/Ui/ProjectEditor/ProjectEditor.h`, `core/Ui/ProjectEditor/ProjectEditor.cpp`
- **Does:** Adds public `datasetAlarmBandsApplicable(const DataModel::Dataset&) const` (owning
  group read from the live project model, bounds-checked like `datasetWidgetEditable`) and
  `Q_PROPERTY(bool alarmBandsApplicable READ alarmBandsApplicable NOTIFY
  editableOptionsChanged)`: the helper on `m_selectedDataset` in `DatasetView`, "any batch
  member passes" in a dataset multi-selection, false otherwise. Also adds
  `alarmBandSelection()`, the applicable members of the multi-selection, which T7 calls from
  both paths (`EditorForms.cpp` was at 1499 of 1500 lines, so the filter could not live there). **Invariant: reuses the
  existing `editableOptionsChanged` signal; no new signal and no new `connect`.**
- **Verify:** CV; SC; read-back of every `Q_EMIT` path that reaches `editableOptionsChanged`
  against the four inputs (selection, dataset widget or LED, hide flag, group widget).
- **Deps:** T1
- [x] done

### T7 — Multi-selection open and commit filter

- **Files:** `core/Ui/ProjectEditor/EditorForms.cpp`, `core/Ui/ProjectEditor/EditorCommit.cpp`
- **Does:** `openAlarmBandsEditorForMultiSelection` and `commitAlarmBandsForSelection` take
  their members from `ProjectEditor::alarmBandSelection()`; both return early when it is empty.
  T6's read-back found that `commitDatasetFormEdit` already emits `editableOptionsChanged`
  after every dataset form edit, so the Hide on Dashboard toggle needs no extra emit. **Invariant: the early return in the
  commit sits before `ProjectUndoFrame` is constructed, so an empty selection records no undo
  entry; the existing frame still wraps the whole write loop as one step.**
- **Verify:** CV; SC.
- **Deps:** T6
- [x] done

### T8 — Button bindings and tooltips

- **Files:** `app/qml/ProjectEditor/Views/DatasetView.qml`,
  `app/qml/ProjectEditor/Views/MultiSelectionView.qml`
- **Does:** Both Alarm Bands buttons bind `enabled: Cpp_JSON_ProjectEditor.alarmBandsApplicable`.
  The four-flag bitmask expression and the `currentDatasetIsEditable` term are deleted from
  `DatasetView.qml`. Tooltips name the five widgets (single view) and say bands apply to the
  selected datasets that show them (multi view). New source strings only; no `.ts` / `.qm`
  edits.
- **Verify:** CV; grep that no `DatasetBar`/`DatasetGauge`/`DatasetMeter`/`DatasetLED` test
  remains on an Alarm Bands button.
- **Deps:** T6
- [x] done

## Phase D — Stated contract (R9)

### T9 — API descriptions

- **Files:** `core/Api/API/Handlers/ProjectDatasetFieldCommands.cpp`,
  `doc/help/API-Reference.md`
- **Does:** Rewrites the `project.dataset.getAlarmBands` and `setAlarmBands` description
  strings: removes "even when no widget is visible" and the "other widget types succeed with
  an empty array" claim; states the rule and that bands elsewhere are stored and ignored.
  Mirrors the wording in the manual's API reference. No handler logic changes.
- **Verify:** CV; SC; grep that both old phrases are gone from `core/` and `doc/help/`; check
  whether CI compares description text with `app/rcc/api/api-schema.json` and note the answer
  for T14. **Answer:** it does not. CI's "Verify generated API surfaces" step checks the
  dataset-property projection and the registries, not command descriptions, so the snapshot
  (and `SerialStudio.js` / `.lua`, `sdk-symbols.json`) keeps the old wording until T14.
- **Deps:** none
- [x] done

### T10 — Manual

- **Files:** `doc/help/Aural-Alerts.md`, `doc/help/Widget-Reference.md`,
  `doc/help/Project-Editor.md`
- **Does:** Via `ss-docs`: Aural Alerts and the Alarm bands section state which datasets raise
  band points and notifications and that bands on any other dataset are kept but ignored;
  Project Editor adds Bar Panel members to the Alarm Bands entry and describes the
  multi-selection behaviour.
- **Verify:** `python scripts/documentation-verify.py` on the three files; every claim checked
  against T1 to T8 as landed.
- **Deps:** T8
- [x] done

### T11 — AI-facing docs

- **Files:** `doc/claude/architecture/dashboard.md`, `app/rcc/ai/skills/dashboard_layout.md`,
  `CLAUDE.md`
- **Does:** dashboard.md "Alarm Bands" section replaces the "fires even when hidden, popped
  out, or `hideOnDashboard`" consequence with the rule, names the function as its single home
  and records that the reap asks the monitor. The assistant skill gains one sentence on which
  widgets make bands alarm. CLAUDE.md gains one Subsystem Contracts row pointing at the
  dashboard.md section.
- **Verify:** `python scripts/claim-verify.py --no-report` on the three files.
- **Deps:** T4, T5
- [x] done

## Phase E — Integration tests (AC2 to AC6, AC8, AC10)

### T12 — Aural-alert suite

- **Files:** `tests/integration/test_aural_alerts.py`
- **Does:** `_load_band_project` gives EGT a Gauge and accepts the dataset shape as a
  parameter. New tests: widget-less dataset raises no point and no band notification (AC2);
  Bar Panel member raises a point (AC3); hidden Gauge raises none (AC4); removing then
  restoring the widget through the API removes then restores the point (AC5); bands on a
  plot-only dataset survive project export (AC8). Same markers as the existing band tests.
  Written by a Sonnet subagent and reviewed in the main session: `widget=""` clears the widget
  (`DatasetApiFields.cpp`), Bar Panel is `widgetType` 10, the export key is `alarmBands`. The
  AC2 notification check is skipped when `notifications.list` is not registered.
- **Verify:** `python -m py_compile`; `pytest --collect-only` on the file; the live run is T15.
- **Deps:** T3, T4
- [x] done

### T13 — Hardening suite

- **Files:** `tests/integration/test_silent_failure_hardening.py`
- **Does:** `_load_band_project` gives its datasets a Gauge. The alarms-disabled test gains the
  widget-less case: no `alarms.disabled` finding until the dataset gets a Gauge (AC6). No API
  switches the master enable, so the new test skips when it is on (it is off by default).
- **Verify:** `python -m py_compile`; `pytest --collect-only` on the file; the live run is T15.
- **Deps:** T5
- [x] done

## Phase G — Per-dataset suppression (amendment 1: R11 to R14)

### T17 — Field and key

- **Files:** `core/Core/DataModel/Frame.h`, `core/Core/DataModel/FrameKeys.h`
- **Does:** `bool suppressAlarms = false;` directly after `enabled` in `Dataset`, filling the
  padding byte before `fftMin` so no other field moves; `Keys::SuppressAlarms("suppressAlarms")`.
  **Invariant: `registry-verify.py` requires every `Dataset` field to be declared exactly once in
  the manifest, so T17 and T18 land as one unit before that check runs.**
- **Verify:** CV; SC; read-back that the member sits in the bool run.
- **Deps:** none
- [x] done

### T18 — Manifest entry and visibility hook

- **Files:** `app/rcc/properties/dataset.json`,
  `core/Pipeline/DataModel/Project/PropertyHooks.h`,
  `core/Pipeline/DataModel/Project/PropertyHooks.cpp`
- **Does:** the `SuppressAlarms` property exactly as the plan's table states (shape of
  `HideOnDashboard`, last row of the Widget Settings builder, `visibleWhen: alarmBandsDrawn`,
  `rebuildTree` and `api.rebuildTree` true), appended to `formIdOrder`, hook declared;
  `PropertyHooks::alarmBandsDrawn(d, pm)` returns true when the dataset has bands and
  `datasetRendersAlarmBands(d, owning group)`. **Invariants: never hand-edit a generated file;
  `formIdOrder` is append-only, so every existing form id keeps its value; the key is never
  `alarmEnabled`.**
- **Verify:** `python scripts/registry-verify.py`; CV and SC on `PropertyHooks.*`.
- **Deps:** T17
- **Landed (2026-10-06):** the row sits last in `buildWidgetRangeRows` with `visibleWhen`
  only; the generator's `editable_expression` returns `true` without an `enabledWhen`, so the
  builder's `rangeEnabled` flag never greys it out for LED-only or Bar Panel datasets.
- [x] done

### T19 — Regenerate the registry surfaces

- **Files:** the four generated TUs, `app/rcc/api/proto-fields.json`,
  `doc/grpc/serialstudio-typed.proto`
- **Does:** `python scripts/generate-property-registry.py`, then `--check`. Ledger read-back:
  one number per dataset message, appended after that message's maximum, nothing renumbered.
  **Invariant: gRPC numbers are released state; a moved one fails `proto-field-renumbered`.**
- **Verify:** `generate-property-registry.py --check`; CV on the generated files; `git diff
  --stat` shows only the six generated files beyond T17 and T18.
- **Deps:** T18
- **Landed (2026-10-06):** the four C++ units regenerated (enum appended last,
  `kDatasetPropertyCount` 43); `proto-fields.json` and the typed proto were unchanged by this
  run, because the ledger follows the API snapshot. They gain `suppressAlarms` at T14.
- [x] done

### T20 — The alarm side honours the option

- **Files:** `core/Pipeline/DataModel/WidgetResolution.h`,
  `core/Pipeline/DataModel/WidgetResolution.cpp`, `core/Ui/UI/AlarmMonitor.cpp`,
  `core/Ui/UI/Alarms/AnnunciatorChecker.cpp`
- **Does:** `datasetRaisesBandAlarms(dataset, group)` beside the rule; `rebuildTrackers` and
  `definesAlarms` call it; the editor's band gates keep `datasetRendersAlarmBands`.
  **Invariants: the rule keeps one home file; neither function is reachable from
  `evaluateAlarms`; `trackersRebuilt` keeps its signature.**
- **Verify:** CV; SC; grep that hand-written C++ reads `suppressAlarms` only in
  `WidgetResolution.cpp`.
- **Deps:** T19
- [x] done

### T21 — Bell follows offline edits (optional plan row, review target I-001)

- **Files:** `core/Ui/UI/Alarms/AlarmAnnunciator.cpp`
- **Does:** `onHealthTick` calls `refreshConfiguredAlarms`, so the cached flag follows every
  edit path within a second. **Invariants: GUI-thread timer slot; `refreshConfiguredAlarms`
  keeps its guard return before notifying; the file stays under 1500 lines.**
- **Verify:** CV; SC; `code-verify.py --tu-census --check` and `--singleton-census --check`.
- **Deps:** T20
- **Landed (2026-10-06):** first as a `groupsChanged` connect; the amendment review showed the
  Alarm Bands dialog and multi-selection commits emit no `groupsChanged`, so on the maintainer's
  decision the connect became the 1 Hz refresh. 1486 of 1500 lines; censuses flat.
- [x] done

### T22 — Unit rows (AC12)

- **Files:** `app/tests/tst_alarm_band_applicability.cpp`
- **Does:** `raises_data()` / `raises()` with the plan's rows; the `rule` table is unchanged.
- **Verify:** CV; SC; clang-format clean; the ctest run is T14.
- **Deps:** T20
- [x] done

### T23 — API descriptions and API reference

- **Files:** `core/Api/API/Handlers/ProjectDatasetFieldCommands.cpp`, `doc/help/API-Reference.md`
- **Does:** both band-command descriptions say that bands on a dataset with `suppressAlarms`
  on are drawn but never notify or alarm; the `project.dataset.update` field list gains
  `suppressAlarms` (bool, optional); the two band-command paragraphs mirror the strings.
  **Invariant: the `.cpp` also carries spec 0094's hunks; only the two description literals
  change.**
- **Verify:** CV; SC; `documentation-verify.py` on `API-Reference.md`.
- **Deps:** T19
- [x] done

### T24 — Manual

- **Files:** `doc/help/Widget-Reference.md`, `doc/help/Aural-Alerts.md`,
  `doc/help/Project-Editor.md`
- **Does:** via `ss-docs`: the option in the Alarm bands section and a `suppressAlarms` row in
  the dataset field table; Aural Alerts names it beside the applicability paragraph; Project
  Editor adds the Widget Settings row and the multi-selection note (the first selected dataset
  decides whether the row shows; an edit applies to every member).
- **Verify:** `documentation-verify.py`; every claim checked against T17 to T20 as landed.
- **Deps:** T20
- [x] done

### T25 — AI-facing docs

- **Files:** `doc/claude/architecture/dashboard.md`, `CLAUDE.md`,
  `app/rcc/ai/skills/dashboard_layout.md`
- **Does:** name `datasetRaisesBandAlarms` beside `datasetRendersAlarmBands` as the rule's home
  and say the editor gates use the latter; the skill tells the assistant to set
  `suppressAlarms` for display-only bands. **Invariants: the corpus lint accepts a field name
  only once the manifest declares it; `CLAUDE.md` also carries spec 0094's row, so only the 0093
  row changes.**
- **Verify:** `claim-verify.py`; `registry-verify.py` (corpus field lint).
- **Deps:** T19, T20
- [x] done

### T26 — Integration tests (AC11, AC13, AC15)

- **Files:** `tests/integration/test_aural_alerts.py`,
  `tests/integration/test_silent_failure_hardening.py`
- **Does:** `_load_band_project` gains a `suppressed` parameter (default off). AC11: a suppressed
  Gauge in its Critical band raises no point and no band notification; `update_dataset(...,
  suppressAlarms=False)` plus in-band frames raises it. AC13: the export carries
  `suppressAlarms: true`, survives loading the exported JSON back, and a never-set project
  exports with no key. AC15: no `alarms.disabled` finding for a suppressed-only project with
  Enable Sounds off, and the finding once the option is cleared; skips when Enable Sounds is on.
- **Verify:** `python -m py_compile`; `pytest --collect-only` on both files.
- **Deps:** T19
- **Landed (2026-10-06):** written by a Sonnet subagent, reviewed in the main session:
  `test_suppressed_gauge_raises_no_point` (AC11), `test_suppress_alarms_round_trips_through_export`
  (AC13, reloads into a fresh project and asserts the plain export still has bands), and
  `test_disabled_finding_ignores_suppressed_bands` (AC15). 38 tests collect; black and ruff clean.
- [x] done

### T27 — Multi-selection edit crash (found in the AC14 walk)

- **Files:** `core/Ui/ProjectEditor/EditorMultiSelect.cpp`
- **Does:** `onMultiSelectionItemChanged` posts its autosave flush instead of running it inline.
  The maintainer hit `[FATL] Object ... destroyed while one of its QML signal handlers is in
  progress` at `TableDelegate.qml:1028` when toggling Suppress Alarms on several datasets:
  the fan-out rebuilt the form model, then the flush re-synced the pipeline through
  `runOnObjectThread`, whose nested event loop let the view destroy the delegate whose
  `onActivated` was still running. **Invariants: no nested event loop under a QML handler;
  the model swap stays synchronous like the single-dataset path; one undo step and one
  autosave per fan-out.** By the code, every yes/no or dropdown row of a dataset multi-selection
  in a saved project took this path. `fanOutputWidgetSelectionEdit` had the same shape and
  got the same fix on the maintainer's request (2026-10-07); the toolbar-driven
  `changeDatasetOptionForSelection` keeps its inline flush, since no form delegate calls it.
- **Verify:** CV; clang-format clean; censuses flat; the maintainer repeats the toggle.
- **Deps:** T26
- [x] done

## Phase F — Maintainer-gated verification

### T14 — Build, unit run, schema refresh

- **Files:** `app/rcc/api/api-schema.json` and the artifacts `sanitize-commit.py` regenerates
  from it
- **Does:** Maintainer builds. Then against that build dir: `ctest -R
  "tst_alarm_band_applicability|tst_frame_serialization|tst_frame_json_legacy|tst_annunciator_sequence"`,
  and the schema snapshot is refreshed with `--dump-api-schema` followed by
  `python scripts/sanitize-commit.py`. Generated files are never hand-edited. Since amendment 1
  the refresh is required, not only cosmetic: CI's strict snapshot check fails on a dataset
  field the committed snapshot lacks.
- **Verify:** ctest output quoted in chat; `git diff --stat` on the generated artifacts shows
  the description text plus the new `suppressAlarms` field and nothing else from 0093;
  `generate-property-registry.py --check-snapshot --strict` passes. The dump must come from a
  commercial build: a GPL dump omits the Pro namespaces (the generator's own ordered fix).
- **Deps:** T1 to T9, T17 to T23
- **Progress (2026-10-07):** schema half done. `Serial-Studio-Pro.exe --dump-api-schema` from the
  maintainer's 10:31 commercial Release build (dumped to scratch, validated, then installed):
  446 commands, exactly three changed (the two band descriptions and `suppressAlarms` on
  `project.dataset.update`). Then, individually rather than the whole-tree sanitize (spec 0094
  in flight): `generate-sdk.py`, `generate-command-strings.py --check`,
  `generate-property-registry.py` + `--check` + `--check-snapshot --strict` (passes),
  `registry-verify.py` (CLEAN), `build_search_index.py`. Ledger: `suppressAlarms = 42` appended
  after 41, nothing renumbered or dropped versus HEAD. **ctest blocked:** that build has
  `SS_BUILD_TESTS` off (CI's unit tier is `-DSS_BUILD_TESTS=ON`, `ctest` under
  `QT_QPA_PLATFORM=offscreen`), and it has no compile database for `syntax-check.py`.
- [ ] done

### T15 — Live run and editor walk (AC7, AC14)

- **Files:** none
- **Does:** Maintainer launches the app with the API server on. Then `pytest
  tests/integration/test_aural_alerts.py tests/integration/test_silent_failure_hardening.py
  -v`. Maintainer walks the Alarm Bands button over the seven dataset shapes in the single
  view and applies bands to one mixed multi-selection (AC7), then walks the Suppress Alarms
  row: shown for a Gauge with bands, absent without bands and for plot-only, one undo step,
  Mixed in a disagreeing multi-selection, Alarm Bands still enabled when suppressed (AC14),
  and, if T21 landed, the bell's dimmed state following an offline toggle.
- **Verify:** pytest summary quoted in chat; the walk's result recorded against AC7 and AC14
  in `spec.md`.
- **Deps:** T12, T13, T14, T26
- **Progress (2026-10-07):** both suites run against the maintainer's 11:42 rebuild (it
  includes both multi-selection crash fixes): 38 of 38 pass. Two tests first failed with
  `CONSENT_REQUIRED` because the script-change consent prompt was still open; they passed on a
  rerun once it was answered. Enable Sounds was off, so AC6 and AC15 ran rather than
  skipped. AC2, AC3, AC4, AC5, AC6, AC10, AC11, AC13 and AC15 are ticked in `spec.md`. AC8's
  integration half passed; its unit half waits on ctest. Remaining: the AC7 and AC14 walk.
- [ ] done

### T16 — Review and handoff

- **Files:** none
- **Does:** `qt-cpp-review` on the C++ diff and `qt-qml-review` on the QML diff; counterfactual
  check named in chat (the rule most at risk is "one rule, one implementation": evidence is a
  grep for band-widget tests outside `WidgetResolution.cpp`); `spec.md` acceptance boxes ticked.
- **Verify:** review findings addressed or listed; `git status` shows only files from the
  plan's table.
- **Progress (2026-10-06):** `qt-cpp-review` ran ahead of the build: lint 0 errors / 0
  advisories, six lenses, 0 confirmed findings, 2 investigation targets, neither changed in
  code. (1) The cached `m_hasConfiguredAlarms` flag is not refreshed by a widget, LED or hide
  edit made while disconnected; band edits had the same gap before. (2) The single-dataset
  branch of `alarmBandsApplicable` reads the editor's cached `m_selectedDataset`, the same
  exposure the old `datasetOptions` gate had. `qt-qml-review` not run: two-binding diff, linter
  clean, pending the maintainer's call. `sanitize-commit.py` passed once; rerun after T14.
  Since spec 0094 appeared in the same working tree, the whole-tree `sanitize-commit.py` is
  deliberately not run from this session: it reformats and regenerates every changed file,
  0094's in-flight ones included. Amendment 1 was checked file by file instead (clang-format
  dry-run, `code-verify`, both censuses, `layer-verify`, `registry-verify`, the generator's
  `--check`, `claim-verify`, `documentation-verify`, black and ruff on the tests).
  Amendment review (2026-10-06): lint 0 errors / 0 advisories over 15 files, six lenses,
  one confirmed finding (three `@brief`s the amendment made wrong, in `ProjectEditor.cpp`,
  `AlarmMonitor.h` and `AlarmAnnunciator.cpp`), fixed. One investigation target (the bell
  flag missed edits that emit no `groupsChanged`) closed by moving T21 to the 1 Hz tick on
  the maintainer's decision. The snapshot item is T14. Final sweep: 23 files lint-clean,
  censuses flat, generator and registry clean, no format drift, 38 tests collect.
- **Deps:** T15
- [ ] done

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` is met and checked off there.
- [ ] `python scripts/code-verify.py --check` is clean on all changed files (no new errors);
  `--tu-census --check` and `--singleton-census --check` show no growth.
- [ ] `scripts/syntax-check.py` passed on every edited C++ file, or each SKIP is named.
- [ ] `qt-cpp-review` and `qt-qml-review` run on the diff; findings addressed or noted.
- [ ] Hotpath not touched: no edit to `Dashboard`, `FrameBuilder`, `FrameReader` or
  `CircularBuffer`, and the rule is not called from `evaluateAlarms`.
- [ ] `tst_alarm_band_applicability` and both integration suites pass (T14, T15).
- [ ] `registry-verify.py` and `generate-property-registry.py --check` are clean, and
  `api-schema.json` is refreshed from a build so CI's strict snapshot check passes.
- [ ] The generated API files shared with spec 0094 are committed consistently with their
  inputs: both specs together, or the second one regenerated on top of the first.
- [ ] `python scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [ ] Diff is *what was asked, and only that*: no example project, no `.ts` / `.qm`, no file
  outside the plan's table.
- [ ] `spec.md` status set to `done`.
