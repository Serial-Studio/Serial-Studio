---
spec: 0105-visible-only-alarms
phase: tasks
status: approved     # T1-T18 approved 2026-10-28
updated: 2026-10-28
---

# Tasks 0105 — Alarms only while visible

> **Phase 3 of 4 — the ordered checklist.** Decompose [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable* — each one a coherent diff a reviewer
> could read in isolation. `/ss-implement` works this list top to bottom and keeps the status
> boxes current. Gate: do not start `/ss-implement` until a human marks this `approved`.

## Conventions

- `CV` abbreviates `python scripts/code-verify.py --check <the task's files>`; `SC` abbreviates
  `python scripts/syntax-check.py <the task's C++ files>`. A `SC` SKIP (stale or missing compile
  database) is reported as "unverified", never as a pass.
- The maintainer builds and launches the app. ctest and integration lines run only against an
  existing build dir or a running app (Phase G).
- T1 to T3 and T5 to T10 stay in the main session (data model, signal wiring, the rule). T4,
  T11 to T15 are pattern work and may run in a Sonnet subagent on a half-page brief; the main
  session reviews what comes back.
- "The rule" means `SerialStudio::datasetAlarmsNow` (textual home) applied by
  `UI::AlarmMonitor::tracks` (runtime home). No task may restate its terms in a third place.
- Nothing in this list edits `Dashboard.cpp`, `AlarmAnnunciator.cpp`, `WindowManager.cpp` or
  any generated TU by hand.

## Phase A — Data model and manifest (R1, R8)

### T1 — Struct field, key and manifest property

- **Files:** `core/Core/DataModel/Frame.h`, `core/Core/DataModel/FrameKeys.h`,
  `app/rcc/properties/dataset.json`
- **Does:** `bool alarmsWhileVisible = false;` after `suppressAlarms` in `Dataset` with a
  one-line Doxygen tail; `Keys::AlarmsWhileVisible("alarmsWhileVisible")`; a new
  `AlarmsWhileVisible` property copied from `SuppressAlarms` (id, field, `jsonKey`
  `AlarmsWhileVisible`, form id `kDatasetView_AlarmsWhileVisible`, bool, default false, scope
  document, `persist: whenTrue`, section `widget`, `CheckBox`, label "Alarms only while
  visible", description "Notify, raise an alarm point and sound for the bands only while a
  widget showing this dataset is on screen", `visibleWhen: alarmBandsDrawn`, undo "Edit
  Dataset" uncoalesced, `rebuildTree: true`, `api: {expose: true, name: alarmsWhileVisible,
  rebuildTree: true}`). **Invariants: `formIdOrder` is append-only, so the new id goes last;
  the form row is inserted directly after `SuppressAlarms` in the `buildWidgetRangeRows` row
  list (R9); `sizeof(Dataset)` grows by exactly 8 and the alignment `static_assert` still
  holds; `registry-verify.py` proves the struct field, the key and the manifest agree, so the
  three files land in one task.**
- **Verify:** CV; `python scripts/registry-verify.py`; `python scripts/layer-verify.py`.
- **Deps:** none
- **Landed (2026-10-28):** CV 0 errors, layer-verify 0, manifest parses; registry-verify's only
  finding is the regeneration T2 performs. SC unverified: this checkout has no compile
  database.
- [x] done

### T2 — Regenerate the registry artifacts

- **Files:** `core/Pipeline/DataModel/Generated/DatasetRegistry.h`,
  `core/Core/DataModel/Generated/DatasetSerialization.cpp`,
  `core/Ui/ProjectEditor/Generated/DatasetForm.cpp`,
  `core/Api/API/Generated/DatasetApiFields.cpp`, `app/rcc/api/proto-fields.json`,
  `doc/grpc/serialstudio-typed.proto` (all written by the generator)
- **Does:** `python scripts/generate-property-registry.py`, then `--check`. **Invariants:
  no generated file is edited by hand; the gRPC ledger gains one appended number and nothing
  is renumbered (`proto-field-renumbered`); the typed proto and the ledger follow the API
  snapshot and gain the field in full only after T16's dump.**
- **Verify:** `generate-property-registry.py --check`; CV on the four TUs; SC on the four TUs;
  `git diff --stat` shows only the files above.
- **Deps:** T1
- **Landed (2026-10-28):** generator wrote the four TUs (`--check` up to date, CV 0 errors,
  registry-verify CLEAN); the ledger and the typed proto are unchanged until T16's dump, as
  planned.
- [x] done

## Phase B — The rule (AC1)

### T3 — `datasetAlarmsNow`

- **Files:** `core/Pipeline/DataModel/WidgetResolution.h`,
  `core/Pipeline/DataModel/WidgetResolution.cpp`
- **Does:** Adds `[[nodiscard]] bool datasetAlarmsNow(const DataModel::Dataset&, const
  DataModel::Group&, bool shown)`: `datasetRaisesBandAlarms(dataset, group) &&
  (!dataset.alarmsWhileVisible || shown)`. One-line `@brief`: the runtime answer the monitor
  applies; rebuild and visibility-change rate only. **Invariant: built on
  `datasetRaisesBandAlarms`, names no widget term of its own, so the three functions cannot
  drift apart; `datasetRendersAlarmBands` and `datasetRaisesBandAlarms` are not edited.**
- **Verify:** CV; SC; `layer-verify.py` (no new include expected).
- **Deps:** T1
- **Landed (2026-10-28):** CV 0, layer 0, format clean; SC unverified (no compile database).
- [x] done

### T4 — Unit table (AC1)

- **Files:** `app/tests/tst_alarm_band_applicability.cpp`
- **Does:** Third data-driven test `alarmsNow_data` / `alarmsNow` over the rule: {option on,
  shown, Gauge with bands, not suppressed} true; {option on, hidden} false; {option off,
  Gauge with bands} true shown and hidden; {suppressed} false shown and hidden, option on or
  off; {plot-only with bands, option on, shown} false; {Bar Panel member, option on, shown}
  true. The existing `rule` and `raises` tables are untouched. No CMake change: the test
  already compiles `WidgetResolution.cpp`.
- **Verify:** CV; SC on the test file; the ctest run itself is T16.
- **Deps:** T3
- **Landed (2026-10-28):** nine rows, CV 0, format clean; SC unverified.
- [x] done

## Phase C — Visibility inputs (R2, R5)

### T5 — Registry enumerates taskbars

- **Files:** `core/Ui/UI/UISessionRegistry.h`, `core/Ui/UI/UISessionRegistry.cpp`
- **Does:** `[[nodiscard]] const QList<Taskbar*>& taskbars() const` and `signals: void
  taskbarsChanged()`, emitted at the end of `registerTaskbar` and `unregisterTaskbar` after the
  list changes. **Invariants: `primaryTaskbar()`, `taskbarAvailable` and `taskbarUnavailable`
  keep their behaviour and emit order; no new ownership (the registry still does not own the
  taskbars).**
- **Verify:** CV; SC; read-back that both emit points sit after the list mutation.
- **Deps:** none
- **Landed (2026-10-28):** CV 0; both emits follow the mutation and the availability emits.
- [x] done

### T6 — Taskbar reports its view and its pop-outs

- **Files:** `core/Ui/UI/Taskbar.h`, `core/Ui/UI/Taskbar.cpp`
- **Does:** `Q_PROPERTY(bool viewShown READ viewShown WRITE setViewShown NOTIFY
  viewShownChanged)`, default true, in the header's property block; `[[nodiscard]] QList<int>
  shownWindowIds() const` returning the ids in `m_windowIDs` whose model row carries
  `TaskbarModel::WindowNormal`; `public slots: void setPopOutOpen(int windowId, bool open)`
  keeping a `QSet<int> m_popOuts` with `signals: void popOutsChanged()` and `[[nodiscard]]
  const QSet<int>& popOuts() const`. **Invariants: both setters guard-return on no change
  (common-mistakes "Setter without guard return"); `shownWindowIds` reads the taskbar's own
  active-workspace population, never `m_fullModel`'s default states for inactive workspaces;
  no `Q_INVOKABLE void`; header block order kept (`Q_PROPERTY`, `signals:`, `public:`,
  `public slots:`, `private:`); about 50 lines in a 1287-line TU.**
- **Verify:** CV; SC; `python scripts/code-verify.py --tu-census --check`.
- **Deps:** none
- **Landed (2026-10-28):** CV 0, tu-census flat (Taskbar.cpp 1287 to 1355); SC unverified.
- [x] done

### T7 — QML pushes the two facts only it knows

- **Files:** `app/qml/MainWindow/MainWindow.qml`, `app/qml/MainWindow/Panes/Dashboard.qml`,
  `app/qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml`
- **Does:** `MainWindow.qml` passes `dashboardShown: root.dashboardVisible && root.visibility
  !== Window.Minimized` into the Dashboard pane; `Dashboard.qml` declares that property and
  binds `taskBar.viewShown: dashboardShown`, and on `_extWindow` binds `extTaskBar.viewShown:
  _extWindow.visibility !== Window.Minimized && _extWindow.visible`; `DashboardCanvas.qml`
  calls `taskBar.setPopOutOpen(windowId, true)` when `openExternalWidgetWindow` creates a
  window and `setPopOutOpen(windowId, false)` inside the existing `closing` handler before the
  map entry is deleted. **Invariants: bindings, never one-shot assignments, so later changes
  track; the Project Editor is a separate window and does not enter `dashboardShown`; the
  `visibility` enum needs `QtQuick.Window` in scope in each file; no new `Shortcut`, no
  motion.**
- **Verify:** `python scripts/code-verify.py --check` on the three QML files; `qt-qml-review`
  on the diff at T18; the maintainer's AC5 walk at T17.
- **Deps:** T6
- **Landed (2026-10-28):** CV 0 on the three files; the pop-out call sits before the map delete.
- [x] done

## Phase D — Aggregator and monitor (R2 to R5, R13)

### T8 — `UI::WidgetVisibility`

- **Files:** `core/Ui/UI/WidgetVisibility.h`, `core/Ui/UI/WidgetVisibility.cpp`,
  `core/Ui/CMakeLists.txt`
- **Does:** New `QObject` constructed with the `Dashboard*` and the `UISessionRegistry*` it is
  handed (no `instance()` reach of its own). Connects `taskbarsChanged` (re-wiring per-taskbar
  connections, capturing each `QMetaObject::Connection`), every taskbar's `viewShownChanged`,
  `activeGroupIdChanged`, `windowStatesChanged` and `popOutsChanged`, and the dashboard's
  `widgetCountChanged` and `dataReset`; each sets a pending flag and posts one `recompute()`
  to the next event-loop turn. `recompute()` builds `QSet<int>`: for every taskbar with
  `viewShown()`, its `shownWindowIds()`, plus every taskbar's `popOuts()`; each window id
  resolves through `Dashboard::widgetType` / `relativeIndex` / `widgetSlot`: a dataset slot
  adds `getDatasetWidget(type, index).uniqueId`, a group slot adds every `uniqueId` of
  `getGroupWidget(type, index).datasets`. Emits `shownChanged()` only when the set differs.
  `[[nodiscard]] bool isShown(int uniqueId) const` and `[[nodiscard]] const QSet<int>& shown()
  const`. **Invariants: GUI thread only, nothing per frame; a window id absent from the widget
  map (a stale taskbar row during a rebuild) is skipped with `SS_ASSERT_LOG`, never
  dereferenced; dashboard tools never enter the set because no taskbar lists them; an offline
  taskbar (`activeGroupId == -1`) contributes nothing; two `SS_ASSERT`s per function;
  functions under 80 lines; SPDX header; the pair is listed in exactly one CMake target.**
- **Verify:** CV; SC; `layer-verify.py` (`core-unowned`, `pair-split`); `code-verify.py
  --singleton-census --check` flat.
- **Deps:** T5, T6
- **Landed (2026-10-28):** 178-line TU, CV 0, layer 0, no `instance()` reach of its own (the
  registry is handed in); SC unverified.
- [x] done

### T9 — Monitor learns the flag and the shown bit

- **Files:** `core/Ui/UI/AlarmMonitor.h`, `core/Ui/UI/AlarmMonitor.cpp`
- **Does:** Tracker gains `bool whileVisible`, `bool shown`, `bool revealPending`.
  `rebuildTrackers` fills `whileVisible` from `dataset.alarmsWhileVisible` and `shown` from
  `m_visibility->isShown(id)` (true when the aggregator is absent). `tracks(id)` returns
  `datasetAlarmsNow`'s answer from the tracker bits: `!whileVisible || shown`.
  `evaluateAlarms` skips a tracker with `whileVisible && !shown`. `setupExternalConnections`
  constructs the aggregator (`std::unique_ptr<WidgetVisibility> m_visibility`, null in the ctor
  init list) with the dashboard and the session registry and connects `shownChanged` to T10's
  slot. Class `@brief` reworded: notifies whether or not the widget is on screen unless the
  dataset opts into Alarms only while visible. **Invariants: `evaluateAlarms` gains one bool
  test and no lookup; trackers still resolve datasets by `uniqueId` on every pass, never a
  cached `Dataset*`; `trackersRebuilt` keeps its parameterless signature and its emit point in
  `rebuildTrackers`; the headless root, which never calls `setupExternalConnections`, keeps a
  null aggregator and a null-safe `isShown` path.**
- **Verify:** CV; SC; read-back that `datasetRaisesBandAlarms` is still the only term that
  decides tracker creation and that no call to `WidgetVisibility` sits in `evaluateAlarms`.
- **Deps:** T3, T8
- **Landed (2026-10-28):** the registry reaches the monitor by injection,
  `setupExternalConnections(UISessionRegistry&)`, passed from `ModuleManager::wireAnnunciator`
  (maintainer decision; the root's one added reach re-baselined with `--singleton-census
  --accept`, 803 flat). CV 0; SC unverified.
- [x] done

### T10 — Hide reaps, reveal alarms

- **Files:** `core/Ui/UI/AlarmMonitor.h`, `core/Ui/UI/AlarmMonitor.cpp`
- **Does:** New private slot `applyVisibility()`: for each tracker with `whileVisible` whose
  `shown` differs from `m_visibility->isShown(id)`, a hide clears `shown`, `initialized`,
  `hint` and `lastFiredBand` and keeps `lastFireMs`; a reveal sets `shown` and
  `revealPending`. If anything flipped it emits `trackersRebuilt()` once (the annunciator reaps
  the hidden points through its existing `tracks(id)` test, drop hold included), then runs
  `processValue` for every revealed tracker whose dataset in `m_dashboard->datasets()` is
  numeric and finite. In `processValue`, the first-sample branch returns only when
  `revealPending` is false; when it is set the branch clears it and falls through to the
  notification block, so a reveal into a Warning or Critical band notifies under the existing
  per-dataset, per-severity cooldown and a reveal outside those bands seeds silently.
  **Invariants: the reap runs through `trackersRebuilt`, never a new annunciator call, so spec
  0088 R3's held table is honoured; `lastFireMs` survives a hide so two tab flips inside 3 s
  post one notification (R4); the band memory is cleared on hide so a reveal is never judged
  as a transition from the band before the hide; `AlarmAnnunciator.cpp` is not edited.**
- **Verify:** CV; SC; `--tu-census --check`; a read-back of `processValue` confirming the
  `revealPending` path is the only new exit.
- **Deps:** T9
- **Landed (2026-10-28):** seed and transition paths merged behind one `seeding` flag; the only
  new exit is `idx < 0 || (seeding && !reveal)`. CV 0, tu-census flat; SC unverified.
- [x] done

## Phase E — Wording (R11)

### T11 — API descriptions and API reference

- **Files:** `core/Api/API/Handlers/ProjectDatasetFieldCommands.cpp`,
  `doc/help/API-Reference.md`
- **Does:** Both band-command descriptions add that a dataset with `alarmsWhileVisible` on
  alarms only while a widget showing it is on screen; the `project.dataset.update` field list
  gains `alarmsWhileVisible` (bool, optional) after `suppressAlarms`; the two band-command
  paragraphs mirror the strings; the `alarms.state` text says a hidden marked dataset lists no
  point. **Invariant: description literals only, no handler logic.**
- **Verify:** CV; SC; `python scripts/documentation-verify.py --no-report
  doc/help/API-Reference.md`; `python scripts/term-verify.py`.
- **Deps:** T2
- **Landed (2026-10-28):** both descriptions, the field list and the two paragraphs; doc and
  term gates clean.
- [x] done

### T12 — Manual

- **Files:** `doc/help/Widget-Reference.md`, `doc/help/Aural-Alerts.md`,
  `doc/help/Project-Editor.md`
- **Does:** via `ss-docs`: the option in the Alarm bands section (what visible means in the
  spec's words: active tab of the main window's dashboard with the Dashboard pane shown or of
  an external dashboard window, window not minimized, widget normal, pop-outs count, frozen
  counts; a reveal alarms at once; Suppress Alarms wins) and an `alarmsWhileVisible` row in the
  dataset field table; Aural Alerts names it beside the applicability paragraph; Project Editor
  adds the Widget Settings row beside Suppress Alarms and keeps the multi-selection note.
- **Verify:** `documentation-verify.py --no-report` on the three files; `term-verify.py`;
  every claim checked against T1 to T10 as landed.
- **Deps:** T10
- **Landed (2026-10-28):** doc gate 0 findings on the three files; term gate clean.
- [x] done

### T13 — AI-facing docs

- **Files:** `doc/claude/architecture/dashboard.md`, `CLAUDE.md`,
  `app/rcc/ai/skills/dashboard_layout.md`
- **Does:** The "Alarm Bands" section's consequence "Notifications fire whether or not an
  applicable dataset's widget is on screen" gains its exception and names `UI::WidgetVisibility`
  as the source of the shown set and `AlarmMonitor::tracks` as the runtime home, with
  `datasetAlarmsNow` beside the two spec 0093 functions; the CLAUDE.md Subsystem Contracts row
  for alarm bands grows one clause; the skill tells the assistant when to set
  `alarmsWhileVisible` (multi-profile projects) and that it never replaces `suppressAlarms`.
  **Invariants: the corpus lint accepts the field name only once the manifest declares it
  (after T2); every path and symbol the docs name must resolve for `claim-verify.py`.**
- **Verify:** `python scripts/claim-verify.py`; `python scripts/registry-verify.py` (corpus
  field lint); `documentation-verify.py --no-report` on `dashboard.md`.
- **Deps:** T2, T10
- **Landed (2026-10-28):** claim-verify 0 errors, registry-verify CLEAN; dashboard.md's two
  `deliberate` findings and its em-dash count predate this change.
- [x] done

## Phase F — Tests (AC2 to AC8)

### T14 — Integration tests, aural alerts

- **Files:** `tests/integration/test_aural_alerts.py`
- **Does:** `_load_band_project` gains `while_visible=False` (sets `alarmsWhileVisible`); new
  `_load_two_workspace_project(api_client, **band_kwargs)` builds workspace A with a plain
  dataset tile and workspace B with the EGT gauge through `project.workspace.add` and
  `project.workspace.addWidget`, activates A with `ui.window.setActiveGroup`, and returns both
  ids. Tests: AC2 `test_visible_only_gauge_on_hidden_workspace_raises_no_point` (and the
  option-off control), AC3 `test_switching_to_the_workspace_raises_and_leaving_removes`
  (no frames between the switches), AC4 `test_minimizing_the_widget_removes_the_point` (ids
  from `ui.window.listWindowStates`, `setWindowState` 1 then 0, each within a second), AC5b
  `test_reveal_after_acknowledge_is_a_fresh_point` (`alarms.acknowledge`, switch away and back
  after 3.5 s: new unacknowledged point and one notification; two switches inside 3 s: one
  notification), AC5c `test_toggling_the_option_live_follows_the_next_rebuild` (unmarked gauge
  on hidden B with its point raised; `update_dataset(..., alarmsWhileVisible=True)` removes
  it; `False` restores it), AC6 `test_suppress_wins_over_visible_only` (both on: no point shown
  or hidden; `dashboard.getData` still carries the band state), AC8
  `test_alarms_while_visible_round_trips_through_export` (export carries `true`, a reload keeps
  it, a never-set project exports without the key). Pattern work; a Sonnet subagent may write
  it against `test_suppressed_gauge_raises_no_point` and
  `test_suppress_alarms_round_trips_through_export`.
- **Verify:** `python -m py_compile`; `pytest --collect-only` on the file; black and ruff
  clean.
- **Deps:** T2, T10
- **Landed (2026-10-28):** written in the main session; eight tests plus the two-workspace
  helper (workspace A shows a second group's datagrid so the EGT gauge on B is hidden);
  black and ruff clean, 47 tests collect across both suites. First live run (2026-10-28):
  the AC5c test asserted the reap one second after the API toggle with no new data, but a
  dataset edit reaches the dashboard with the next frame (the same reason the 0093 widget
  toggle test sends frames); a probe against the running app showed the point dropping on
  the next frame, so the test now sends frames after the toggle, like its sibling.
- [x] done

### T15 — Integration test, alarms-disabled finding (AC7)

- **Files:** `tests/integration/test_silent_failure_hardening.py`
- **Does:** `test_disabled_finding_counts_visible_only_bands`: Enable Sounds off, a project
  whose only bands sit on a marked gauge on a hidden workspace still reports `alarms.disabled`
  in `problems.run`; skips when Enable Sounds is on, like its siblings.
- **Verify:** `python -m py_compile`; `pytest --collect-only` on the file.
- **Deps:** T2, T10
- **Landed (2026-10-28):** the helper gains `while_visible`; the test hides the gauge behind
  an empty user workspace and still expects the finding.
- [x] done

## Phase G — Maintainer-gated verification

### T16 — Build, unit run, schema refresh

- **Files:** `app/rcc/api/api-schema.json` and the artifacts `sanitize-commit.py` regenerates
  from it (SDK, search index, typed proto, ledger)
- **Does:** Maintainer builds a commercial Release with `SS_BUILD_TESTS=ON`. Against that build
  dir: `ctest -R "tst_alarm_band_applicability|tst_frame_serialization|tst_frame_json_legacy"`
  under `QT_QPA_PLATFORM=offscreen`; `--dump-api-schema` from the commercial binary into the
  scratchpad, validated, then installed; then `generate-sdk.py`,
  `generate-property-registry.py` with `--check` and `--check-snapshot --strict`,
  `registry-verify.py`, `app/rcc/ai/build_search_index.py` (or the whole `sanitize-commit.py`
  when no other spec is in flight in the tree). **Invariants: the dump must come from a commercial
  build (a GPL dump omits the Pro namespaces); exactly the two band descriptions and the new
  field change in the snapshot; the ledger appends, nothing renumbers.**
- **Verify:** ctest output quoted in chat; `git diff --stat` on the generated artifacts shows
  only the expected lines; `--check-snapshot --strict` passes.
- **Deps:** T1 to T13
- **Progress (2026-10-28):** schema half done. The maintainer dumped `api-schema.json` from
  his build (three changes: the two band descriptions and `alarmsWhileVisible` on the dataset
  verbs); `sanitize-commit.py` then regenerated the SDK, command strings, registry, ledger,
  typed proto and search index: `alarmsWhileVisible = 44` appended after 43, nothing
  renumbered; `--check-snapshot --strict` passes (41 dataset fields). ctest pending.
- [ ] done

### T17 — Live run and walks (AC5, AC9, AC11, AC12)

- **Files:** none
- **Does:** Maintainer launches the app with the API server on; then `pytest -v
  tests/integration/test_aural_alerts.py` and `pytest -v
  tests/integration/test_silent_failure_hardening.py` (AC2 to AC8, AC11). Maintainer walks:
  two dashboard windows, a pop-out, the Console pane in front, a frozen dashboard (AC5); the
  editor row beside Suppress Alarms, absent without bands, Mixed in a disagreeing
  multi-selection, one undo step, Alarm Bands still enabled (AC9); the APU test cell project
  with both engine tiers marked and the APS500 simulator profile running: no GTCP85 point on
  the APS500 tab, the GTCP85 Oil Temperature point within a second of switching tabs (AC12);
  one `--benchmark-hotpath` run as the formality for the struct growth.
- **Verify:** pytest summary quoted in chat; each walk recorded against its criterion in
  `spec.md`.
- **Deps:** T14, T15, T16
- **Progress (2026-10-28):** against the maintainer's commercial Release build with the API
  server on: `test_aural_alerts.py` 32 of 32 pass (one first-run failure was the script-change
  consent prompt, answered, then green; the other was the AC5c timing, fixed in T14);
  `test_silent_failure_hardening.py` 12 pass, 3 skip because Enable Sounds is on (AC7 runs
  only with it off, like its two 0093 siblings). AC2, AC3, AC4, AC5b, AC5c, AC6 and AC10 are
  ticked in `spec.md`; AC8's integration half passed and its unit half waits on ctest.
  Remaining: the AC5, AC9 and AC12 walks, AC7 with Enable Sounds off, and the hotpath
  benchmark formality.
- [ ] done

### T18 — Review and handoff

- **Files:** none
- **Does:** `qt-cpp-review` on the C++ diff and `qt-qml-review` on the three QML files;
  counterfactual check named in chat (the rule most at risk is "one rule, one home": evidence
  is a grep showing `alarmsWhileVisible` is read only in `WidgetResolution.cpp` and
  `AlarmMonitor.cpp`, and visibility only through `WidgetVisibility`); `spec.md` acceptance
  boxes ticked and its status set to `done`.
- **Verify:** review findings addressed or listed; `git status` shows only files from the
  plan's table.
- **Deps:** T17
- **Progress (2026-10-28):** counterfactual check run ahead of the build: `alarmsWhileVisible`
  is read in exactly two code sites (`datasetAlarmsNow`, `rebuildTrackers`), visibility is
  asked only through `WidgetVisibility::isShown`, `Dashboard.cpp`, `FrameBuilder.cpp` and
  `AlarmAnnunciator.cpp` are untouched, `evaluateAlarms` gained one bool test. Qt review lint:
  every hit on a pre-existing line. `sanitize-commit.py` ran clean over the tree (33 files,
  all this spec's). `qt-cpp-review` deep pass (six lenses): one confirmed finding, the unused
  `WidgetVisibility::shown()` accessor, removed; one investigation target, a `--headless` run
  wires the aggregator with no taskbars so an opted-in dataset is hidden there, which is the
  rule's own answer (nothing is on screen) and is now stated in the
  `setupExternalConnections` brief; the maintainer may prefer to skip the gate when headless.
  `qt-qml-review` not run
  on the five-line QML diff, pending the maintainer's call as in 0093. SC unverified: no
  compile database in this checkout, so every C++ edit awaits T16's build.
- [ ] done

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` is met and checked off there.
- [ ] `python scripts/code-verify.py --check` is clean on all changed files (no new errors);
  `--tu-census --check` and `--singleton-census --check` show no growth.
- [ ] `scripts/syntax-check.py` passed on every edited C++ file, or each SKIP is named.
- [ ] `qt-cpp-review` and `qt-qml-review` run on the diff; findings addressed or noted.
- [ ] Hotpath not touched: no edit to `Dashboard.cpp`, `FrameBuilder`, `FrameReader` or
  `CircularBuffer`; `evaluateAlarms` gained one bool test and no lookup; one
  `--benchmark-hotpath` run recorded.
- [ ] `tst_alarm_band_applicability` and both integration suites pass (T16, T17).
- [ ] `registry-verify.py`, `layer-verify.py` and `generate-property-registry.py --check` are
  clean, and `api-schema.json` is refreshed from a commercial build so CI's strict snapshot
  check passes.
- [ ] Generated artifacts shared with any other in-flight spec are committed consistently with
  their inputs.
- [ ] `python scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [ ] Diff is *what was asked, and only that*: no example project, no `.ts` / `.qm`, no file
  outside the plan's table.
- [ ] `spec.md` status set to `done`.
