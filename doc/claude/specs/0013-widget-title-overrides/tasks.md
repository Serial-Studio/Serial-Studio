---
spec: 0013-widget-title-overrides
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-07-16
---

# Tasks 0013 — Per-widget title overrides and freeze-titlebar visibility

> **Phase 3 of 4 — the ordered checklist.** Decompose [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable* — each one a coherent diff a reviewer
> could read in isolation. `/ss-implement` works this list top to bottom and keeps the status
> boxes current. Gate: do not start `/ss-implement` until a human marks this `approved`.

## Conventions

- One task = one focused, reviewable change. If a task touches >3 files or needs a paragraph
  to describe, split it.
- **Verify** is how *this* unit is confirmed before moving on — usually
  `python scripts/code-verify.py --check <files>`, plus a test or a read-back where one fits.
- **Deps** lists task IDs that must land first.
- Order so the tree compiles (conceptually) after each task where practical.

## Tasks

### T1 — Keys constants

- **Files:** `app/src/DataModel/Frame.h`
- **Does:** Adds `Keys::WidgetDisplay("widgetDisplay")`, `Keys::Titles("titles")`,
  `Keys::FreezeTitle("freezeTitle")` to `namespace Keys`. No Frame struct or
  `Frame::serialize` change — the map is ProjectModel state, never frame state (R3
  boundary).
- **Verify:** `python scripts/code-verify.py --check app/src/DataModel/Frame.h`; grep
  confirms no other `"widgetDisplay"` literal exists.
- **Deps:** none
- [x] done

### T2 — ProjectModel state, setters, signal

- **Files:** `app/src/DataModel/ProjectModel.h`, `app/src/DataModel/ProjectModel.cpp`
- **Does:** Adds `m_widgetDisplay` (QJsonObject), getters `displayTitle(uid)`,
  `displayTitles()`, `freezeTitleMode(type, uid)` (returns the resolved mode; per-type
  default via `SerialStudio::dashboardWidgetPaintsTitle`), slots `setDisplayTitle(uid,
  title)` (empty title removes the entry) and `setFreezeTitleMode(type, uid, mode)`
  (mode ∈ bar/painted/hidden; writing the per-type default removes the entry; no "auto",
  revised 2026-07-16), signal `widgetDisplayChanged()`. Setters follow the `saveWidgetSetting` shape:
  gate on `operationMode == ProjectFile`, guard-return on no-op, `setModified(true)`, emit.
  Wires `widgetDisplayChanged` → `scheduleAutoSave` beside the `widgetSettingsChanged`
  hookup (ProjectModel.cpp:155) and resets the member in `newJsonFile()`. **Ctor-closure
  invariant:** `newJsonFile` runs before AppState/Dashboard exist — the reset is a plain
  member assignment, no singleton calls; the `AppState::instance()` use stays inside the
  setters only (runtime-called), mirroring `saveWidgetSetting` exactly.
- **Verify:** `code-verify.py --check` both files; read-back against the
  `saveWidgetSetting` pattern; confirm no new calls inside the ctor-reachable closure.
- **Deps:** T1
- [x] done

### T3 — Persistence round-trip

- **Files:** `app/src/DataModel/Project/ProjectModelPersistence.cpp`,
  `app/src/DataModel/Project/ProjectModelLoading.cpp`
- **Does:** Serializes `widgetDisplay` beside `widgetSettings` (Persistence :322), omitted
  when both inner maps are empty; loads it in `loadWidgetSettingsAndWorkspaces` (Loading
  :807), absent key → empty object. No schema-version bump (older apps ignore unknown root
  keys).
- **Verify:** `code-verify.py --check` both files; read-back: save path and load path use
  `Keys::` constants only (`keys-hardcoded-literal` rule).
- **Deps:** T2
- [x] done

### T4 — Dashboard copy-patch + live re-patch

- **Files:** `app/src/UI/Dashboard.h`, `app/src/UI/Dashboard.cpp`
- **Does:** Applies title overrides to the `copy.title` of every group/dataset entering
  `m_widgetGroups`/`m_widgetDatasets` (in `buildWidgetGroups` /
  `processDatasetIntoWidgetMaps`, incl. the LED-panel synthesized title deriving from the
  *displayed* group title), via one shared apply helper. Adds `refreshDisplayTitles()`
  slot (connected to `ProjectModel::widgetDisplayChanged`): re-patches the copies in
  place, calls `WidgetRegistry::updateWidget(id, title)` for changed widgets, emits new
  `displayTitlesChanged()` signal. **Hotpath invariants:** no frame-path work — patching
  happens at reconfigure or user-edit cadence only; `m_lastFrame` titles are NEVER touched
  (exports/`dashboard.getData` stay canonical, R3); no `widgetCountChanged` emission from
  the refresh slot (that would trigger the full-rebuild R8 bans); no new cached hotpath
  flag.
- **Verify:** `code-verify.py --check` both files; read-back diff against the three named
  invariants; grep confirms `refreshDisplayTitles` never emits `widgetCountChanged`.
- **Deps:** T2
- [x] done

### T5 — DashboardWidget live title notify

- **Files:** `app/src/UI/DashboardWidget.h`, `app/src/UI/DashboardWidget.cpp`
- **Does:** Gives the `widgetTitle` Q_PROPERTY a real `widgetTitleChanged` NOTIFY signal,
  emitted when `Dashboard::displayTitlesChanged` fires (connection made in the ctor
  alongside the existing dashboard wiring; read existing signal wiring in the file first
  per CLAUDE.md).
- **Verify:** `code-verify.py --check` both files; header ordering per style contract
  (`signals:` placement, notify listed in Q_PROPERTY).
- **Deps:** T4
- [x] done

### T6 — Taskbar consumes widgetUpdated

- **Files:** `app/src/UI/Taskbar.h`, `app/src/UI/Taskbar.cpp`
- **Does:** Connects `WidgetRegistry::widgetUpdated` (exists, currently unconsumed) to a
  new slot that resolves the item via `findItemByWidgetId` and updates
  `TaskbarModel::WidgetNameRole`, so live renames reach taskbar entries and widget search
  without a model rebuild. **Signal-wiring invariant:** read the existing registry
  connections in `Taskbar.cpp` before adding; same-thread default connection.
- **Verify:** `code-verify.py --check` both files; read-back of the connect site.
- **Deps:** T4
- [x] done

### T7 — WidgetDelegate explicit freeze title + live caption

- **Files:** `app/qml/MainWindow/Panes/Dashboard/WidgetDelegate.qml`
- **Does:** Replaces the `frozenPanel` bool with an effective freeze-title mode
  (`bar`/`painted`/`hidden`, no "auto" — revised 2026-07-16) resolved from ProjectModel
  (invokable read + `Connections` on `widgetDisplayChanged`; ProjectModel returns the
  per-type default for unset widgets, `painted` for Bar/Gauge/Meter, `bar` otherwise). `frozenHeaderVisible` = frozen && mode `bar` && title non-empty; the
  effective mode is exposed via `windowRoot` for instruments. `windowRoot.title` becomes
  a live binding to `dashboardWidget.widgetTitle` instead of the one-shot assignment.
- **Verify:** `code-verify.py --check` on the file; qml lint via code-verify; read-back:
  no binding loop between delegate and instrument gate (single direction: delegate →
  windowRoot → instrument).
- **Deps:** T5
- [x] done

### T8 — Instrument QML gates (Bar, Gauge, Meter)

- **Files:** `app/qml/Widgets/Dashboard/Bar.qml`, `app/qml/Widgets/Dashboard/Gauge.qml`,
  `app/qml/Widgets/Dashboard/Meter.qml`
- **Does:** Removes the hardcoded `windowRoot.frozenPanel = false` opt-outs; painted
  `WidgetTitleBar` strips and in-face title labels gain the freeze gate (visible unless
  frozen && effective mode ≠ `painted`); title text prefers `windowRoot.title` (live
  override) over `model.title` (ctor snapshot). Normal-mode appearance unchanged (R5:
  name at most once, never header + painted simultaneously).
- **Verify:** `code-verify.py --check` all three; read-back: every `model.title` display
  binding in the three files is either switched or intentionally left (in-face value
  labels that are not the widget name stay).
- **Deps:** T7
- [x] done

### T9 — External window live title

- **Files:** `app/qml/MainWindow/Panes/Dashboard/ExternalWidgetWindow.qml`
- **Does:** The one-shot `windowRoot.title = widgetTitle` (:119) becomes a live binding /
  `widgetTitleChanged` follow, so popped-out windows rename live (R2, AC6).
- **Verify:** `code-verify.py --check`; read-back.
- **Deps:** T5
- [x] done

### T10 — Workspace editor row data

- **Files:** `app/src/DataModel/ProjectEditor.h`,
  `app/src/DataModel/Project/ProjectEditorSummaries.cpp`
- **Does:** `ResolvedWidget` gains `uniqueId` (dataset uid for dataset widgets, group uid
  otherwise) and `isGroupWidget`; `buildResolvedWidgetLookup` fills them;
  `widgetsForWorkspace` rows gain `uniqueId`, `isGroupWidget`, `displayTitle` (current
  override or empty), `freezeTitleMode`, and an `isLedPanel` marker (LED rows: title
  field disabled downstream).
- **Verify:** `code-verify.py --check` both files; read-back: LED-panel branch and
  group-widget branch fill group uid, dataset branch fills dataset uid.
- **Deps:** T2
- [x] done

### T11 — Workspace editor UI

- **Files:** `app/qml/ProjectEditor/Views/WorkspaceView.qml`
- **Does:** Adds per-row editable "Display Title" TextField (placeholder = canonical
  title; commits on editingFinished → `setDisplayTitle`; disabled for LED-panel rows) and
  a freeze-title ComboBox (Title bar / Painted title [instruments only] / Hidden →
  `setFreezeTitleMode`; revised 2026-07-16, no "Auto" entry), both gated
  on `customizeWorkspaces` like existing row edits; refresh rows on
  `widgetDisplayChanged`. **ComboBox restore-race:** suppress the write-back while
  programmatically syncing (the `restoringPage` pattern). Translated strings use `.arg()`
  numbered placeholders only, never `%n` with `.arg()`.
- **Verify:** `code-verify.py --check`; read-back against the two named invariants.
- **Deps:** T10
- [x] done

### T12 — On-widget caption menu button

- **Files:** `app/qml/MainWindow/Panes/Dashboard/WidgetDelegate.qml`,
  `app/qml/Widgets/MiniWindow.qml`, `app/src/UI/WindowManager.cpp`
- **Does:** *(Revised 2026-07-16 from the original hidden right-click menu.)* MiniWindow's
  left caption button becomes a menu button (`menu.svg`, emits `menuClicked`; the
  external-window button is removed and `externControlWidth` → `menuControlWidth`, updated
  at the three WindowManager.cpp drag-region reads). WidgetDelegate opens a caption menu:
  "Rename Widget…" (`rename.svg` → `promptRenameWidget`), "Freeze Title" submenu
  (`freeze.svg`) with icon-free checkable options Title Bar / Painted Title (instruments
  only) / Hidden, and "Open in External Window" (`expand.svg`, emits the delegate-level
  `externalWindowClicked` DashboardCanvas already handles). Menu icons are 16px mono
  icons from `app/rcc/icons/buttons`.
- **Verify:** `code-verify.py --check`; maintainer confirms the menu opens (AC6 setup).
- **Deps:** T7
- [x] done

### T13 — API commands

- **Files:** `app/src/API/Handlers/DashboardHandler.cpp`
- **Does:** Registers `project.dashboard.setWidgetTitle {uniqueId, title}`,
  `project.dashboard.getWidgetTitles {}` (returns overrides + shadowed canonical titles),
  `project.dashboard.setWidgetFreezeTitle {widgetType, uniqueId, mode}` beside
  `setTimeRange`. Mutators validate inputs, return rich old/new responses, and rely on
  the CommandRegistry epoch + debounced-autosave apply path (the `widgetDisplayChanged`
  signal from T2 is the epoch trigger — confirm it reaches `syncFromProjectModel`'s
  wiring per the API-mutation conventions).
- **Verify:** `code-verify.py --check`; read-back: command names, param validation,
  writing the per-type default clears the entry, `painted` rejected for non-instruments.
- **Deps:** T2
- [x] done

### T14 — Safety tiers + API docs

- **Files:** `app/rcc/ai/command_safety.json`, `doc/help/API-Reference.md`
- **Does:** Adds safety-tier entries for the two mutating commands; documents all three
  in API-Reference (params, responses, display-only semantics — canonical titles
  unaffected, R3).
- **Verify:** `pytest tests/scripts/test_ai_assistant_static.py -v` (existing safety
  checks); `documentation-verify.py` via sanitize later.
- **Deps:** T13
- [x] done

### T15 — Static test coverage

- **Files:** `tests/scripts/test_ai_assistant_static.py`
- **Does:** Extends the static suite: the three commands are registered in the safety
  catalog / docs as the existing conventions require (AC7 static half).
- **Verify:** `pytest tests/scripts/test_ai_assistant_static.py -v` passes locally.
- **Deps:** T14
- [x] done

### T16 — Integration tests (maintainer-run)

- **Files:** `tests/integration/test_widget_display.py` (new)
- **Does:** AC1: override round-trip through save/reload; reorder survival; deleted-target
  inertness. AC3: CSV export headers + `project.dataset.get` + `dashboard.getData` stay
  canonical with overrides set. AC7: command semantics (set/get/clear, invalid-mode
  rejection). Marked/structured per `tests/README.md` conventions (read it first).
- **Verify:** File passes `black` + collection (`pytest --collect-only`); execution is
  maintainer-run against a live app (API server on).
- **Deps:** T13
- [x] done

### T17 — Widget-scoped title overrides (mid-build amendment, 2026-07-16)

- **Files:** `spec.md`/`plan.md` (R1 amendment), `app/src/DataModel/ProjectModel.h/.cpp`,
  `app/src/UI/Dashboard.cpp`, `app/src/DataModel/Project/ProjectEditorSummaries.cpp`,
  `app/qml/ProjectEditor/Views/WorkspaceView.qml`,
  `app/qml/MainWindow/Panes/Dashboard/WidgetDelegate.qml`,
  `app/src/API/Handlers/DashboardHandler.cpp`, `doc/help/API-Reference.md`,
  `tests/integration/test_widget_display.py`
- **Does:** Maintainer found the dataset-uid-keyed titles map applied one title to every
  widget of a dataset (Plot + FFT + Waterfall). Titles now hold two scopes: widget-level
  `"type:uid"` (wins) and entity-level `"uid"` (fallback), resolved widget → entity →
  canonical. UI (workspace editor field, caption rename) edits the widget level; the API
  sets either via optional `widgetType`. LED-panel rows became editable (widget-level
  override replaces the composed label verbatim).
- **Verify:** `code-verify.py --check` on all touched files; static + collection tests;
  maintainer AC2/AC6 observation now includes per-widget renames.
- **Deps:** T1-T16
- [x] done

## Definition of Done

- [x] Every acceptance criterion in `spec.md` is met and checked off there (AC2/AC4/AC5/
      AC6/AC8 are maintainer observations/runs — listed for the handoff note).
- [x] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [x] `qt-cpp-review` run on the C++ diff; findings addressed or noted.
- [x] `ss-hotpath` checks pass / `--benchmark-hotpath` not regressed (maintainer/CI; no
      per-frame work was added — counterfactual named at handoff).
- [x] Relevant `pytest` targets listed for the maintainer (`test_widget_display.py`, static
      suite green locally).
- [x] `python scripts/sanitize-commit.py` run; working tree clean of lint debt (search
      index + SDK regen are expected diff noise, named in the commit).
- [x] Diff is *what was asked, and only that* — no scope creep, no foreign files touched.
- [x] `spec.md` status set to `done`.
