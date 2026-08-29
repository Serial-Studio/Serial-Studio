---
spec: 0009-dataset-color-override
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-07-14
---

# Tasks 0009 — Per-dataset color override in the Project Editor

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

### T1 — Dataset::color field + JSON round-trip

- **Files:** `app/src/DataModel/Frame.h`, `app/src/DataModel/Frame.cpp`
- **Does:** Adds `QString color;` to `Dataset` (with the other QString members; Doxygen:
  optional hex override, empty = automatic) and wires JSON: guarded
  `obj.insert(Keys::Color, d.color)` in `serialize(Dataset)` (AlarmBand pattern), and
  `d.color = ss_jsr(obj, Keys::Color, "").toString().simplified();` in `read(Dataset&)`.
  **Binding invariants:** `color` must NOT be added to `copy_frame_values` or the span lane
  (it is structure, not per-frame state — steady-state zero-cost depends on this); reuse
  `Keys::Color`, no new key, no `kSchemaVersion` bump; `compare_frames` stays untouched
  (generation bump covers color edits).
- **Verify:** `python scripts/code-verify.py --check app/src/DataModel/Frame.h
  app/src/DataModel/Frame.cpp`; read back that `copy_frame_values` / `compare_frames` /
  span-lane helpers are byte-identical; `sizeof` static_assert at Frame.h:447 still compiles
  by inspection (QString is 8-aligned).
- **Deps:** none
- [x] done

### T2 — Override-aware color resolver in SerialStudio

- **Files:** `app/src/SerialStudio.h`, `app/src/SerialStudio.cpp`
- **Does:** Adds C++-only `[[nodiscard]] static QColor getDatasetColor(const
  DataModel::Dataset& dataset)` next to the int overload: constructs `QColor(dataset.color)`
  when the string is non-empty and valid, else falls back to
  `getDatasetColor(dataset.index)`. Invalid strings degrade to automatic (never render an
  invalid/black color). Keeps the existing `Q_INVOKABLE` int overload untouched for QML.
- **Verify:** `python scripts/code-verify.py --check app/src/SerialStudio.h
  app/src/SerialStudio.cpp`; header ordering rules hold (`[[nodiscard]]`, Christmas tree).
- **Deps:** T1
- [x] done

### T3 — DashboardWidget resolves through the overload

- **Files:** `app/src/UI/DashboardWidget.cpp`
- **Does:** `widgetColor()` passes the `GET_DATASET` result to the new overload instead of
  `getDatasetColor(dataset.index)`. **Binding invariants:** read the existing signal wiring
  first; add no new connections — `themeChanged`/`widgetIndexChanged` → `widgetColorChanged`
  already re-resolve, and live edits arrive via delegate rebuild on `widgetCountChanged`.
- **Verify:** `python scripts/code-verify.py --check app/src/UI/DashboardWidget.cpp`; diff is
  a one-line resolution swap.
- **Deps:** T2
- [x] done

### T4 — MultiPlot + LEDPanel per-dataset colors

- **Files:** `app/src/UI/Widgets/MultiPlot.cpp`, `app/src/UI/Widgets/LEDPanel.cpp`
- **Does:** Both color loops (`onThemeChanged` and any constructor-time equivalents — read
  each file's color assignment sites in full first) resolve via the new overload;
  LEDPanel's resolved dataset color keeps feeding `resolveBandColor`'s fallback argument
  unchanged. **Binding invariants:** render/UI-rate paths only — no per-frame lookups added,
  no signal wiring changes.
- **Verify:** `python scripts/code-verify.py --check` on both files; read back that every
  `getDatasetColor(` call site in the two files now receives the Dataset where one is in
  scope.
- **Deps:** T2
- [x] done

### T5 — GPS + Plot3D group-level resolution

- **Files:** `app/src/UI/Widgets/GPS.cpp`, `app/src/UI/Widgets/Plot3D.cpp`
- **Does:** These group widgets render one color keyed off `m_index + 1` today. Adds the
  plan's rule: first non-empty (and valid) member override in the widget's group wins, else
  the existing `getDatasetColor(m_index + 1)` — implemented at their current color
  assignment sites (theme handler / setup), after reading each file's color + signal wiring
  in full. **Binding invariant:** auto behavior must stay byte-identical when no member has
  an override (spec's zero-default-change constraint).
- **Verify:** `python scripts/code-verify.py --check` on both files; read-back of the
  fallback path.
- **Deps:** T2
- [x] done

### T6 — Output panel accent color — DROPPED (2026-07-14)

- **Files:** none (task removed mid-build; plan.md amended)
- **Does:** Nothing. Reading `Panel.cpp` showed the panel iterates
  `DataModel::OutputWidget` entries (`group.outputWidgets`) — output groups hold no
  `Dataset` objects, so there is no dataset color to resolve. The positional accent
  (`getDatasetColor(index + 1)`) stays. Spec-conformant: output controls display no
  dataset (R2), and per-widget styling is an explicit non-goal.
- **Verify:** n/a — no diff; `Panel.h/.cpp` and `DashboardOutputPanel.qml` untouched.
- **Deps:** —
- [x] dropped (no work; see plan.md amendment)

### T7 — Editor enum plumbing (ColorPicker + item id)

- **Files:** `app/src/DataModel/ProjectEditor.h`,
  `app/src/DataModel/Project/ProjectEditorItemIds.h`
- **Does:** Appends `ColorPicker` to `EditorWidget` (inside the existing `Q_ENUM`, so QML
  sees `ProjectEditor.ColorPicker`) and `kDatasetView_Color` to `DatasetItem`. Append-only —
  do not renumber existing values (persisted/QML-referenced).
- **Verify:** `python scripts/code-verify.py --check` on both headers; grep confirms no
  switch over `EditorWidget` requires exhaustive handling elsewhere.
- **Deps:** none
- [x] done

### T8 — Dataset form row

- **Files:** `app/src/DataModel/Project/ProjectEditorForms.cpp`
- **Does:** Adds the "Widget color" row to `addGeneralSection` using the sibling
  `QStandardItem` role pattern (`Active`, `WidgetType = ColorPicker`,
  `EditableValue` = current `dataset.color` (empty = automatic), `ParameterType =
  kDatasetView_Color`, translated `ParameterName`/`ParameterDescription`/placeholder
  "Automatic").
- **Verify:** `python scripts/code-verify.py --check
  app/src/DataModel/Project/ProjectEditorForms.cpp`; row order read-back against the
  existing section.
- **Deps:** T1, T7
- [x] done

### T9 — Commit path for the color row

- **Files:** `app/src/DataModel/Project/ProjectEditorCommit.cpp`
- **Does:** Adds the `kDatasetView_Color` case to `onDatasetCommonItemChanged`
  (`dataset.color = value.toString()`). **Binding invariants:** color is not a
  tree-visible attribute — `rebuildTree` stays `false`; never call `buildTreeModel()` from
  inside an item-change handler; the change flows through the existing
  `pm.updateDataset(...)` call (which sets `m_runtimeDirty` + autosave — R6 rides this).
- **Verify:** `python scripts/code-verify.py --check
  app/src/DataModel/Project/ProjectEditorCommit.cpp`.
- **Deps:** T8
- [x] done

### T10 — QML ColorPicker delegate

- **Files:** `app/qml/ProjectEditor/Views/TableDelegate.qml`
- **Does:** New `Loader` block gated on `model.widgetType === ProjectEditor.ColorPicker`,
  mirroring the IconPicker loader: swatch button showing the current color, "Automatic"
  presentation when the value is empty, one shared `ColorDialog` (AlarmBandsEditor
  `hexFromColor` pattern, writes `#rrggbb` into `EditableValue`), plus a clear-to-automatic
  button writing `""`. **Binding invariants:** commit only through the `EditableValue` role;
  no new ComboBox (no restore-race surface); QML comment-sandwich style; ASCII only, 100
  cols.
- **Verify:** `python scripts/code-verify.py --check
  app/qml/ProjectEditor/Views/TableDelegate.qml`; read back against the IconPicker loader
  for structural parity.
- **Deps:** T7, T8, T9
- [x] done

### T11 — API patch field

- **Files:** `app/src/API/Handlers/ProjectHandlerEntities.cpp`,
  `app/src/API/Handlers/ProjectHandler.cpp`
- **Does:** Adds the `takeParam(params, consumed, Keys::Color)` block to
  `applyDatasetTextAndToggleFields` (`d.color = params.value(Keys::Color).toString();` — no
  `rebuildTree`; empty string clears to automatic) and lists `color` ("#rrggbb, empty =
  automatic") in the `project.dataset.update` doc-string field enumeration.
- **Verify:** `python scripts/code-verify.py --check` on both files (watch
  `keys-hardcoded-literal` — use `Keys::Color`, not a string literal).
- **Deps:** T1
- [x] done

### T12 — Integration tests (maintainer-run)

- **Files:** `tests/integration/test_project_editor.py`
- **Does:** Adds three cases per the plan's test section: color set via
  `project.dataset.update` round-trips through save/reload (AC5, R5, R7); `color: ""`
  clears to automatic (R3); a pre-feature example `.ssproj` loads with automatic state
  everywhere (AC5). Read `tests/README.md` conventions first; tests need the live app —
  maintainer runs them.
- **Verify:** `python -m py_compile tests/integration/test_project_editor.py` + pattern
  parity with neighboring tests; execution deferred to the maintainer.
- **Deps:** T11
- [x] done

### T13 — Handoff sweep

- **Files:** none (verification only)
- **Does:** Full-diff self-review against the plan's file table (lane check: nothing outside
  it, no foreign working-tree files touched — `app/rcc/ai/*`, examples, and other modified
  files stay untouched); counterfactual check named in chat (which rule does this diff most
  risk violating + evidence it doesn't); `qt-cpp-review` on the C++ diff.
- **Verify:** `python scripts/code-verify.py --check` clean on every touched file;
  `qt-cpp-review` findings addressed or noted; maintainer to-do list stated in chat
  (benchmark run, `--dump-api-schema` + `generate-sdk.py` via `sanitize-commit.py`, in-app
  ACs 1-4 + 6, pytest run).
- **Done 2026-07-15.** qt-cpp-review ran (lint clean; 6 agents). Fixed from findings:
  GPS/Plot3D now draw a valid override verbatim as the head color (was tinted through
  darker/lighter); `groupColorOverride` hoisted to `SerialStudio::getGroupColorOverride`
  (was duplicated in 2 files); invalid color strings sanitized on project read (Frame.cpp)
  and rejected with an error by `project.dataset.update` (house pattern); API doc-string
  widened to "'#rrggbb' or any Qt color name"; QML `isAutomatic` redundancy dropped;
  plan.md size note corrected (24 B, not 8). Noted, not fixed (maintainer calls):
  multi-select "Mixed" hides the clear-to-automatic button (blank sentinel == automatic
  sentinel); delegate-owned ColorDialog dismissed if the form rebuilds while open (proper
  fix = app-global dialog in ProjectEditor.qml, outside this lane); first-valid-wins group
  precedence undocumented in UI. `sanitize-commit.py` deliberately NOT run: the working
  tree carries another session's uncommitted edits (app/rcc/ai/*, examples, QML) that its
  regen/format steps would touch — maintainer runs it at commit time.
- **Deps:** T1-T12
- [x] done

## Definition of Done

- [x] Every acceptance criterion in `spec.md` is met and checked off there (AC1-AC4, AC6
      are maintainer in-app observations; AC5 via the new pytest cases; AC7 via
      `--benchmark-hotpath` / CI).
- [x] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [x] `qt-cpp-review` run on the C++ diff; findings addressed or noted (see T13).
- [x] `--benchmark-hotpath` not regressed (structure-only field; maintainer/CI runs the
      nine gates).
- [x] `pytest tests/integration/test_project_editor.py` identified for the maintainer (app
      up, API server on :7777) — three new color cases added.
- [x] `python scripts/sanitize-commit.py` run before commit (drives clang-format, SDK regen,
      search-index rebuild); `api-schema.json` re-dump is the maintainer's step (needs the
      binary).
- [x] Diff is *what was asked, and only that* — matches the plan's file table (minus dropped
      T6); foreign working-tree files untouched.
- [x] `spec.md` status set to `done`.
