---
spec: 0093-alarm-band-applicability
phase: plan
status: approved     # R1-R10 and amendment 1 (R11-R14) approved 2026-10-06
updated: 2026-10-06
---

# Plan 0093 — Alarm-band applicability

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Read the relevant `doc/claude/` sub-docs and the *actual code*
> before writing this — a plan grounded in a stale mental model is worse than no plan.
> Gate: do not start `/ss-tasks` until a human marks this `approved`.
>
> Written in the same pass as the spec at the maintainer's request (2026-10-06); both are
> reviewed together.
>
> **Amendment 1 (approved 2026-10-06, optional T21 row kept):** the per-dataset opt-out (spec R11 to R14) is
> designed in its own section at the end, so the delta can be reviewed as a unit.

## Approach (one paragraph)

One pure function, `SerialStudio::datasetRendersAlarmBands(dataset, group)`, is added beside
`getDashboardWidget` / `getDashboardWidgets` and is built on them, so the rule is the
dashboard's own widget resolution and cannot drift from what the dashboard builds. Five sites
answer from it: `AlarmMonitor::rebuildTrackers` (no tracker means no notification and no
point), the annunciator's reap (which now asks the monitor what it tracks instead of
re-deriving), `AnnunciatorChecker::definesAlarms`, and the two Project Editor Alarm Bands
buttons through one new `ProjectEditor` property. The multi-selection open and commit paths
filter their member list with the same helper. Bands are never cleared or rewritten; a dataset
that fails the rule simply gets no tracker.

## Affected subsystems & files

| File | Change |
|------|--------|
| `core/Pipeline/DataModel/WidgetResolution.h` / `.cpp` | New `[[nodiscard]] bool datasetRendersAlarmBands(const DataModel::Dataset&, const DataModel::Group&)`: true when `getDashboardWidget(group) == DashboardBarPanel`, else false when `hideOnDashboard`, else true when `getDashboardWidgets(dataset)` contains Bar, Gauge, Meter or LED. |
| `core/Ui/UI/AlarmMonitor.h` / `.cpp` | `rebuildTrackers` keeps iterating `Dashboard::datasets()` and skips a dataset that fails the rule; applicability is collected by walking the (group, dataset) pairs of `Dashboard::rawFrame()`, the same pairs the dashboard builds widgets from. New `[[nodiscard]] bool tracks(int uniqueId) const noexcept`. Class `@brief` reworded: independence is from the widget being on screen, not from it being configured. |
| `core/Ui/UI/Alarms/AlarmAnnunciator.h` / `.cpp` | Keep the monitor pointer already passed to `setupExternalConnections` (`m_monitor`, null in the ctor list and in the headless root). `onTrackersRebuilt` reaps a band point when the monitor no longer tracks its id, replacing the local `datasets()` / `alarmBands.empty()` test. Order unchanged: refresh the flag, return on `m_dropHold`, then reap. |
| `core/Ui/UI/Alarms/AnnunciatorChecker.cpp` | `definesAlarms` adds the rule to its existing `group.enabled && dataset.enabled && !alarmBands.empty()` test. |
| `core/Ui/ProjectEditor/ProjectEditor.h` / `.cpp` | New `Q_PROPERTY(bool alarmBandsApplicable READ alarmBandsApplicable NOTIFY editableOptionsChanged)` and a public helper `datasetAlarmBandsApplicable(const Dataset&) const` (owning group read from the live project model, same shape as `datasetWidgetEditable`). The property is the helper on the selected dataset in `DatasetView`, and "any batch member passes" in a dataset multi-selection. A third method, `alarmBandSelection()`, returns the applicable members of the multi-selection for the open and commit paths (added during implement: `EditorForms.cpp` sat at 1499 of 1500 lines, so the filter could not be written inline there). |
| `core/Ui/ProjectEditor/EditorForms.cpp` | `openAlarmBandsEditorForMultiSelection`: range union and the shared-bands prefill are computed over `alarmBandSelection()`. |
| `core/Ui/ProjectEditor/EditorCommit.cpp` | `commitAlarmBandsForSelection`: writes to `alarmBandSelection()` only, returning before the undo frame when it is empty. No emit was needed: `commitDatasetFormEdit` already ends every dataset form edit with `editableOptionsChanged`. |
| `app/qml/ProjectEditor/Views/DatasetView.qml` | Alarm Bands `enabled:` becomes `Cpp_JSON_ProjectEditor.alarmBandsApplicable`; the four-flag bitmask expression and the `currentDatasetIsEditable` term are removed; tooltip names the five widgets. |
| `app/qml/ProjectEditor/Views/MultiSelectionView.qml` | Alarm Bands gets the same `enabled:` binding; tooltip says bands apply to the selected datasets that show them. |
| `core/Api/API/Handlers/ProjectDatasetFieldCommands.cpp` | Description strings only for `project.dataset.getAlarmBands` and `setAlarmBands`: drop "even when no widget is visible" and the "other widget types succeed with an empty array" claim; state the rule and that bands elsewhere are stored and ignored. No handler logic changes. |
| `app/rcc/api/api-schema.json` and the artifacts generated from it | Refreshed by the maintainer after a build (`--dump-api-schema`), then `sanitize-commit.py` regenerates the SDK and typed proto. Not hand-edited. |
| `app/tests/tst_alarm_band_applicability.cpp`, `app/tests/CMakeLists.txt` | New unit (AC1), registered like `tst_workspace_rebind` with `WidgetResolution.cpp` as a source. |
| `tests/integration/test_aural_alerts.py` | `_load_band_project` gives EGT a Gauge (AC10). New tests for AC2 to AC5 and AC8. |
| `tests/integration/test_silent_failure_hardening.py` | `_load_band_project` gives its datasets a Gauge; the alarms-disabled test gains the widget-less case (AC6). |
| `doc/help/Aural-Alerts.md`, `Widget-Reference.md`, `Project-Editor.md`, `API-Reference.md` | State the rule; add Bar Panel to the editor's list; describe the multi-selection behaviour; mirror the reworded API descriptions. |
| `doc/claude/architecture/dashboard.md` | "Alarm Bands" section: replace the "fires even when hidden, popped out, or `hideOnDashboard`" consequence with the rule and name the function as its single home. |
| `app/rcc/ai/skills/dashboard_layout.md` | One sentence so the assistant does not put bands on a plot-only dataset expecting an alarm. |
| `CLAUDE.md` | One Subsystem Contracts row pointing at the dashboard.md section. |

Not touched: `Dashboard`, `WidgetMapBuilder`, the five band-drawing widgets, `PainterDataBridge`,
`ExtensionData`, the legacy band migration in `Frame.cpp`, `PropertyHooks::onWidgetChanged`,
the project schema, and every example project. No file under `app/src/Licensing/`,
`core/Core/Licensing/`, `MachineID` or the API Server auth/consent set is involved.

## Architecture & data flow

Runtime. `Dashboard::reconfigureDashboard` assigns `m_lastFrame`, fills `m_datasets` through
`WidgetMapBuilder`, then emits `widgetCountChanged`; `resetData` clears both before it emits.
`AlarmMonitor::rebuildTrackers` runs on those two signals, so the frame and the dataset map are
always in the same state when it reads them. It walks the frame's (group, dataset) pairs once
per rebuild, collects the ids of banded datasets that pass the rule, and creates a tracker
only for those. Walking the pairs avoids any dependence on `groupId` conventions and treats a
dataset id that appears in two groups as applicable only where a banded copy is drawn. Everything downstream is unchanged: `evaluateAlarms` walks the trackers,
`processValue` emits `bandTransition` and posts the notification, the annunciator raises or
clears the point. A dataset without a tracker therefore produces neither.

Reap. `trackersRebuilt` stays parameterless. `AlarmAnnunciator::onTrackersRebuilt` drops each
band point whose id fails `m_monitor->tracks(id)`. That covers a deleted dataset, emptied bands
and lost applicability with one test, and the monitor's rebuild stays the single runtime
decision site. A dataset that regains applicability gets a fresh tracker; its baseline capture
emits a `bandTransition`, which raises the point without a notification, the same path a
reconnect takes today (R4).

Project-defines-alarms. `m_hasConfiguredAlarms` is refreshed on `trackersRebuilt`,
`soundsChanged` and `jsonFileChanged`, and the Problem Center checker calls `definesAlarms`
fresh on every run. A widget, LED or hide edit changes the answer only through a dashboard
rebuild, which emits `widgetCountChanged`, so the existing triggers cover it; the pill that
reads the cached flag only exists while the dashboard is up.

Editor. `alarmBandsApplicable` notifies on `editableOptionsChanged`, which already fires on
`datasetModelChanged`, `groupModelChanged` and `ProjectModel::groupsChanged`, so a change of
selection, of the dataset's widget or LED flag, or of the owning group's widget re-evaluates
the binding. The multi-selection aggregate model rebuild emits `datasetModelChanged`, which
chains to the same signal.

## Hotpath & threading impact

- **Touches the hotpath?** No. `Dashboard` is read through two existing const accessors and not
  edited. `AlarmMonitor::evaluateAlarms` (display-tick rate, GUI thread) is not modified; the
  rule runs only in `rebuildTrackers` (rebuild rate), `definesAlarms` (command rate) and editor
  property reads. `getDashboardWidgets` allocates a small list per call, which is acceptable at
  those rates and must never be called from `evaluateAlarms`.
- **New cross-thread signal/slot?** No. No signal signature changes and no new connection; every
  object involved lives on the GUI thread.
- **New input to a cached hotpath flag?** No. `m_hasConfiguredAlarms` is a UI flag, not a
  hotpath one; its refresh triggers are listed above.
- **Timestamp ownership** — not applicable.

## Data model & persistence

None for R1 to R10. No `Keys::` addition, no schema or writer version change, no migration.
Bands on a non-applicable dataset are read, kept in memory and written back exactly as today.
Amendment 1 adds one optional key; see its section.

## API / SDK surface

Behaviour unchanged. Two description strings change, which flows into `api-schema.json`, the
generated SDK text and `doc/help/API-Reference.md`. `alarms.state` keeps its shape; it lists
fewer points for projects with orphan bands. `ProjectApiSupport`'s `"alarm"` summary tag still
means "bands are stored" and is left alone.

## QML / UI

Two `enabled:` bindings and two tooltips. No new component, model or motion. The changed
tooltips are new source strings; the translation files are regenerated by the maintainer's
usual flow and are not edited here.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Home of the rule | (a) pure function beside widget resolution, built on it; (b) a `PropertyHooks` predicate taking `ProjectModel`; (c) the dashboard publishes the ids that landed in band-drawing buckets | (a). The monitor has frame groups and the editor has project groups; only a `(dataset, group)` function serves both. (c) is the literal truth but the editor and the checker have no dashboard to ask, so a second rule would exist anyway. |
| Where the alarm system is gated | (a) at the monitor, so notifications and points stop together; (b) at the annunciator only, so sound stops and notifications continue | (a). A "value entered the warning band" notification for a band drawn nowhere is the same incoherence as the sound. (b) would also leave the reap and the flag each needing their own filter. |
| Reap source of truth | (a) annunciator asks `AlarmMonitor::tracks(id)`; (b) annunciator re-derives the rule from the dashboard frame; (c) `trackersRebuilt` carries the id list | (a). One runtime decision site and no signal signature change. It is also line-neutral in `AlarmAnnunciator.cpp`, which sits at 1484 of the 1500-line cap. |
| Single-dataset editor gate | (a) the rule alone; (b) `currentDatasetIsEditable &&` the rule, as today | (a). The spec's goal is that the button and the alarm agree. (b) leaves a dataset in a fixed-layout group with a stale `bar` widget or LED flag alarming with the button disabled. Reviewer-visible widening: such a dataset can now open the band editor. |
| Multi-selection gate | (a) enabled when any member is applicable, apply to applicable members only; (b) enabled only when every member is applicable | (a), per the maintainer's scope answer. A mixed selection stays useful and cannot create orphans. |
| Hidden datasets | (a) hidden means not applicable, except in a Bar Panel; (b) hiding is layout only | (a), maintainer decision. The Bar Panel exception follows the dashboard: the panel builds a row for every group dataset. |
| Orphan bands | (a) ignore silently; (b) Problem Center warning; (c) clear on widget change | (a), maintainer decision. |

## Risks & mitigations

- **Two integration helpers encode the old behaviour.** Both `_load_band_project` helpers create
  a widget-less banded dataset and the suites expect it to alarm. They are updated in the same
  change; leaving either out fails the suite on the first run, which is the intended signal.
- **System Monitor example goes quiet** for its three usage bands (spec Open Question). Not
  edited here; the maintainer decides separately because the mirror fixtures and one
  integration baseline are generated from that file.
- **A second copy of the rule.** The QML bitmask expression is deleted rather than extended, the
  reap asks the monitor, and the dashboard.md section plus the CLAUDE.md row name the function
  as the only home. AC1 pins the table.
- **Drop hold (spec 0088 R3).** The reap still runs after the `m_dropHold` return, so a held
  table is never reaped even though the monitor has no trackers during the outage.
- **Headless root.** It passes no monitor and nothing evaluates bands there; the reap guards the
  null pointer and finds no band points.
- **Hide toggle not notifying the editor property.** Checked at task time in the dataset form
  commit path; the fix, if needed, is one emit of an existing signal.
- **`api-schema.json` goes stale until the next build.** Sequenced in tasks: maintainer builds,
  dumps the schema, then `sanitize-commit.py`. Whether CI compares description text against
  the snapshot is checked at task time; the refresh is a named task either way so it cannot be
  skipped.
- **TU size.** `AlarmAnnunciator.cpp` is 16 lines under the cap; the edit there must be
  net-neutral. `code-verify.py --tu-census --check` runs before handoff.
- **Behaviour reversal for existing projects.** Accepted by the maintainer without a warning.
  The manual and the API descriptions state the rule so the change is at least documented.

## Test & verification plan

- **Unit (maintainer builds, then `ctest`):** `tst_alarm_band_applicability` covers AC1: Bar,
  Gauge, Meter, LED true; empty widget, Compass, plot-only false; hidden Gauge false; Bar Panel
  member true, hidden or not; Canvas group member with no widget false. Existing
  `tst_frame_serialization` and `tst_frame_json_legacy` stay green (AC8).
- **Integration (maintainer launches the app with the API server on; I may then run them):**
  `pytest tests/integration/test_aural_alerts.py tests/integration/test_silent_failure_hardening.py -v`
  - AC2: widget-less banded dataset in its Critical band gives `alarms.state` no points and no
    band notification; with a Gauge it gives a priority-2 point.
  - AC3: Bar Panel group, member without a widget, point raised.
  - AC4: Gauge plus `hideOnDashboard`, no point.
  - AC5: point active, widget removed through `project.dataset.update`, point gone; widget
    restored, point back.
  - AC6: master enable off, `problems.run` has no `alarms.disabled` for the widget-less project
    and has it once the dataset gets a Gauge.
  - AC8: bands written to a plot-only dataset are present in the exported project JSON.
  - AC10: the existing band-entry test passes with the helper's Gauge.
- **Maintainer observation (AC7):** button state for the seven dataset shapes in the single
  view, and a mixed multi-selection apply.
- **Hotpath:** not touched; no `--benchmark-hotpath` run required.
- **Static:** `scripts/syntax-check.py` on every edited C++ file; `code-verify.py --check` on
  the touched files plus `--tu-census --check` and `--singleton-census --check`;
  `layer-verify.py`; `claim-verify.py` and `documentation-verify.py` for AC9; `qt-cpp-review`
  and `qt-qml-review` on the diff; `sanitize-commit.py` before commit.

## Amendment 1 — per-dataset suppression (R11 to R14)

### Approach

A new dataset property, `suppressAlarms`, is declared once in the property manifest, shaped
like `hideOnDashboard`: a bool, off by default, written only when true. The generator gives it
serialization, the editor row, undo, the multi-selection row, the API field and its gRPC field
number. The alarm side reads it through one new function beside the rule,
`SerialStudio::datasetRaisesBandAlarms(dataset, group)`: not suppressed, has bands, and
`datasetRendersAlarmBands`. The monitor and the checker switch to it; the editor's band gates
keep `datasetRendersAlarmBands`, because a suppressed dataset's bands are still drawn and
editable. A suppressed dataset gets no tracker, so its notification, point and sound stop
together, and the existing reap removes a live point.

### Affected files

| File | Change |
|------|--------|
| `core/Core/DataModel/Frame.h` | `bool suppressAlarms = false;` directly after `enabled` in `Dataset`. Twelve ints and fifteen bools end at byte 63, so the new bool fills the padding byte before `fftMin` and `sizeof(Dataset)` stays the same (a layout argument; the build confirms it). |
| `core/Core/DataModel/FrameKeys.h` | `Keys::SuppressAlarms("suppressAlarms")`. |
| `app/rcc/properties/dataset.json` | New `SuppressAlarms` property: bool, default false, `persist: whenTrue`, `CheckBox`, label "Suppress Alarms", section `widget` as the Widget Settings builder's last row, `visibleWhen: alarmBandsDrawn`, undo "Edit Dataset" uncoalesced, `rebuildTree: true`, `api: {expose: true, name: suppressAlarms, rebuildTree: true}`. Appended to `formIdOrder`, so every existing form id keeps its value. Hook `alarmBandsDrawn` declared as a predicate. |
| `core/Pipeline/DataModel/Project/PropertyHooks.h` / `.cpp` | `alarmBandsDrawn(d, pm)`: the dataset has bands and `datasetRendersAlarmBands(d, owning group)`, with the owning group bounds-checked from `pm.groups()` like `widgetSelectable`. |
| Generated by `scripts/generate-property-registry.py` | `DatasetRegistry.h`, `DatasetSerialization.cpp`, `DatasetForm.cpp`, `DatasetApiFields.cpp`, `app/rcc/api/proto-fields.json` (one number appended per dataset message, nothing renumbered) and `doc/grpc/serialstudio-typed.proto`. Never hand-edited. The ledger and the typed proto follow the API snapshot, so they gain the field only after T14's dump (observed at T19). |
| `core/Pipeline/DataModel/WidgetResolution.h` / `.cpp` | `datasetRaisesBandAlarms(dataset, group)`. The rule keeps one home file. |
| `core/Ui/UI/AlarmMonitor.cpp` | `rebuildTrackers` collects ids with `datasetRaisesBandAlarms`. |
| `core/Ui/UI/Alarms/AnnunciatorChecker.cpp` | `definesAlarms` uses `datasetRaisesBandAlarms`. |
| `core/Ui/UI/Alarms/AlarmAnnunciator.cpp` (optional) | Closes review target I-001: `onHealthTick` (the existing 1 Hz tick) calls `refreshConfiguredAlarms`, so the taskbar bell's disabled hint follows every edit made while disconnected within a second. Changed from a `groupsChanged` connect after the amendment review (maintainer decision, 2026-10-06): the Alarm Bands dialog and multi-selection commits emit no `groupsChanged`. The file goes from 1484 to 1486 of 1500. |
| `app/tests/tst_alarm_band_applicability.cpp` | A second data-driven test for `datasetRaisesBandAlarms` (AC12). |
| `core/Ui/ProjectEditor/EditorMultiSelect.cpp` (crash fix, added 2026-10-06) | Found in the AC14 walk: toggling a yes/no row in a dataset multi-selection crashed. The fan-out rebuilt the form model, then flushed the autosave inline; the flush's pipeline re-sync spins a nested event loop, which destroyed the committing QML delegate mid-handler. The flush is now posted to the next event-loop turn, in the dataset fan-out and, on the maintainer's request, the output-widget fan-out. The path predates 0093; the new row exposed it. |
| `core/Api/API/Handlers/ProjectDatasetFieldCommands.cpp` | Both band-command descriptions name `suppressAlarms`. No logic change. |
| `doc/help/Widget-Reference.md`, `Aural-Alerts.md`, `Project-Editor.md`, `API-Reference.md` | The option in the Alarm bands section and the dataset field table, the Widget Settings row, the multi-selection note, and the `project.dataset.update` field list. |
| `doc/claude/architecture/dashboard.md`, `CLAUDE.md`, `app/rcc/ai/skills/dashboard_layout.md` | Name `datasetRaisesBandAlarms` beside `datasetRendersAlarmBands` as the rule's home; the assistant skill names the field. |
| `tests/integration/test_aural_alerts.py`, `test_silent_failure_hardening.py` | AC11, AC13, AC15. |
| `app/rcc/api/api-schema.json` and the SDK | Maintainer build, `--dump-api-schema`, then `sanitize-commit.py`. Required this time: CI runs `--check-snapshot --strict`, which compares the dataset verbs' typed schema, so a new field without a refreshed snapshot fails CI. |

### Data flow

The flag rides the `Dataset` struct into every frame the FrameBuilder publishes, and into the
mirror's structure message, which carries the serialized project document verbatim, so a
mirror viewer honours it with no wire change and no `kWireVersion` bump. Propagation needs
nothing new. An editor edit marks the runtime dirty, and the debounced auto-save re-syncs the
frame pipeline (`ProjectPersistence.cpp:513`), as it does for every dataset edit. An API edit
bumps the mutation epoch and `CommandRegistry` re-syncs at once (`CommandRegistry.cpp:364`).
Either way the dashboard rebuilds on the new structure snapshot
(`Dashboard::applyStructureSnapshot`), the monitor rebuilds its trackers, and the reap or the
baseline capture follows (R4). `rebuildTree: true` also makes the edit emit `groupsChanged`,
which re-runs the Problem Center checkers, so the alarms-disabled finding follows the toggle.

### Hotpath & threading

None. The new bool fills existing padding. `datasetRaisesBandAlarms` runs only where
`datasetRendersAlarmBands` already runs (rebuild and command rate), and `evaluateAlarms` still
never calls either. No new thread hop. The 1 Hz refresh runs on the GUI thread, walks the
project once per second as the Problem Center checker already does, and
`refreshConfiguredAlarms` guards its notify, so QML sees nothing unless the flag flips.

### Data model & persistence

One key, `suppressAlarms`, written only when true and read with default false. No schema or
writer version bump: a bump would send older files through the migration that drops
customized workspaces. Older versions ignore the key and keep alarming, and one that resaves
the file drops it, since writers emit only the fields they know.

### API / SDK surface

Generated: `project.dataset.update` accepts `suppressAlarms` (bool), and the dataset's
serialized form carries it when true; the ledger appends one gRPC field number per dataset
message. The band commands change only their descriptions. `alarmEnabled` keeps routing
through `applySimpleAlarmFields`.

### QML / UI

The generated `CheckBox` row only; the dataset form delegate already renders checkbox rows, so
no QML changes. Two new source strings (label and description) for the maintainer's lupdate.

### Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| What suppression stops | (a) all alarm output; (b) also widget flashing; (c) points and sound only; (d) sound only | (a), maintainer decision 2026-10-06. One gate (no tracker) stops all three outputs and reuses the reap. |
| Key and label | `suppressAlarms`; `muteAlarms`; `alarmsDisabled`; never `alarmEnabled` | `suppressAlarms` / "Suppress Alarms", maintainer decision. ISA-18.2's term; no collision with the master Mute or the two enable flags. |
| Editor placement | Widget Settings row; new Alarms section; Alarm Bands dialog | Widget Settings row, maintainer decision. No hand-written form code, and the manifest has no section-level visibility. |
| Row visibility | `visibleWhen` bands drawn; `enabledWhen`; always visible | `visibleWhen`, so the form is unchanged for the many datasets without bands. |
| Where the alarm side reads it | a second function beside the rule; a flag check at each call site | Second function: the monitor and the checker cannot disagree, and the rule keeps one home file. |
| `rebuildTree` | true, like `hideOnDashboard`; false | true. The edit emits `groupsChanged`, which re-runs the Problem Center checkers and lets an API toggle re-sync through the epoch at once. Cost: one tree rebuild per toggle. |

### Risks & mitigations

- **A hidden flag outlives its bands.** Removing the bands or the widget hides the row with
  the flag still on; it reappears checked as soon as bands are drawn again. Visible, never
  silent; accepted.
- **Multi-selection follows the first member.** The aggregate form is built from the first
  selected dataset (`EditorMultiSelect.cpp:184`), so the row shows only when that member has
  drawn bands, and an edit applies to every member, banded or not. Harmless, because the flag
  does nothing without drawn bands; the manual says so.
- **Assistant corpus lint.** `dashboard_layout.md` may name `suppressAlarms` only once the
  manifest declares it; the tasks order the skill edit after regeneration.
- **Snapshot gate.** CI fails until `api-schema.json` is refreshed from a build, so T14 now
  gates the commit, not only the wording.
- **Generated files shared with spec 0094.** The working tree also holds spec 0094, and the
  generator and `sanitize-commit.py` rewrite `proto-fields.json`, the typed proto, the SDK and
  the search index as whole files from the combined tree. Those files cannot be split between
  a 0093 and a 0094 commit by hunk: either the two specs land together, or the second to
  commit regenerates on top of the first.
- **Manifest gates.** `registry-verify.py` proves the struct field is declared exactly once,
  the `jsonKey` names a real `Keys::` constant, the hook exists and `formIdOrder` matches the
  enum; `generate-property-registry.py --check` and `proto-field-renumbered` guard the outputs.

### Test & verification plan

- **Unit:** `tst_alarm_band_applicability` gains a `raises` table (AC12): an unsuppressed Gauge
  with bands is true; a suppressed Gauge, a suppressed LED and a suppressed Bar Panel member are
  false; a Gauge without bands and a plot-only dataset with bands are false. The existing
  `rule` table stays as it is.
- **Integration:** AC11 (suppressed Gauge in its Critical band: no point and no band
  notification; turning the option off through `project.dataset.update` and sending in-band
  frames raises the point), AC13 (the export carries `suppressAlarms: true` and a reload keeps
  it; a project that never set it exports with no key), AC15 (no `alarms.disabled` finding for
  a suppressed-only project with Enable Sounds off; it appears once the option is cleared;
  skips when Enable Sounds is on, like AC6).
- **Maintainer observation:** AC14 (row visibility, one undo step, Mixed, the Alarm Bands
  button still enabled) and the bell's dimmed state following an offline band-dialog or
  multi-selection edit within a second.
- **Static:** `registry-verify.py`, `generate-property-registry.py --check`, `code-verify.py`
  (including `proto-field-renumbered`), `claim-verify.py`, `documentation-verify.py`, and
  `qt-cpp-review` on the amendment diff.
