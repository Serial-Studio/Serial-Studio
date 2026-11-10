---
spec: 0117-dataset-dialog-commands
phase: tasks
status: approved     # maintainer asked on 2026-11-09 to run plan, tasks and implement in one pass
updated: 2026-11-09
---

# Tasks 0117 — Palette and context-menu commands for the dataset dialogs

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

### T1 — Value-label rows from any dataset

- **Files:** `core/Ui/ProjectEditor/EditorCommit.h`, `core/Ui/ProjectEditor/EditorCommit.cpp`,
  `core/Ui/ProjectEditor/ProjectEditor.h`
- **Does:** `valueLabelRows()` takes the dataset whose labels it turns into rows; the inline
  selection opener passes the selected dataset. `EditorCommit.cpp` must not grow (1491 of 1500).
- **Verify:** `syntax-check.py` on `EditorCommit.cpp` and `ProjectEditor.cpp`; `wc -l` shows no
  growth in `EditorCommit.cpp`.
- **Deps:** none
- [x] done

### T2 — Per-dataset openers in EditorForms

- **Files:** `core/Ui/ProjectEditor/EditorForms.h`, `core/Ui/ProjectEditor/EditorForms.cpp`
- **Does:** the single-dataset bodies of the Alarm Bands and Frequency Markers selection
  openers move into `openAlarmBandsEditorFor(const Dataset&)` and
  `openFrequencyMarkersEditorFor(const Dataset&)`; the selection openers call them with the
  selected dataset (the multi-selection branch stays where it is); a bounds-checked resolver
  returns the selected dataset's in-editor copy when the ids match it, otherwise the model's.
  Invariant: the dialog's captured uniqueId is the dataset the payload came from, because the
  commit resolves by it.
- **Verify:** `syntax-check.py`; `code-verify.py --check`; read-back that the selection path
  emits the same payload as before.
- **Deps:** none
- [x] done

### T3 — Public openers and the one availability rule per dialog

- **Files:** `core/Ui/ProjectEditor/ProjectEditor.h`, `core/Ui/ProjectEditor/ProjectEditor.cpp`
- **Does:** `DatasetDialog` enum; public slots `openAlarmBandsEditorFor`,
  `openFrequencyMarkersEditorFor`, `openValueLabelsEditorFor` (groupId, datasetId);
  `datasetFrequencyMarkersApplicable(const Dataset&)`; `frequencyMarkersApplicable` property
  (NOTIFY `editableOptionsChanged`); `datasetDialogApplicable(dialog, groupId, datasetId)`
  invokable (dataset and group enabled, then the dialog's rule). Invariant: one rule per
  dialog; the Alarm Bands rule stays `datasetAlarmBandsApplicable`.
- **Verify:** `syntax-check.py` on `ProjectEditor.cpp` and every TU including `ProjectEditor.h`
  that the database knows; `code-verify.py --check`; header ordering per `code-style.md`.
- **Deps:** T1, T2
- [x] done

### T4 — Catalog entries and the dataset menu

- **Files:** `app/rcc/commands/projecteditor.json`, `app/rcc/commands/layouts/editor-menus.json`
- **Does:** the three commands (spec OQ2 titles, tooltips, the toolbar icons,
  `contexts: ["editor"]`), listed in the `dataset` menu after Remove Transform.
- **Verify:** JSON parses; `registry-verify.py` after T5.
- **Deps:** none
- [x] done

### T5 — Bindings: menu, palette, toolbar

- **Files:** `app/qml/Commands/ProjectEditorMenuBindings.qml`,
  `app/qml/Commands/ProjectEditorCommandBindings.qml`,
  `app/qml/ProjectEditor/Views/DatasetView.qml`
- **Does:** menu bindings visible from `datasetDialogApplicable` on a single dataset target and
  running the per-target opener; palette bindings visible while the editor shows a single
  enabled dataset that the rule accepts and running the selection opener; the Freq. Markers
  button reads `frequencyMarkersApplicable`. Invariant: no binding re-derives a rule.
- **Verify:** `registry-verify.py` CLEAN; `code-verify.py --check`; qmllint shows no new kind of
  warning.
- **Deps:** T3, T4
- [x] done

### T5b — One host per dialog (amendment 1)

- **Files:** `app/qml/ProjectEditor/ProjectEditor.qml`,
  `app/qml/ProjectEditor/Views/DatasetView.qml`,
  `app/qml/ProjectEditor/Views/MultiSelectionView.qml`
- **Does:** moves the Alarm Bands, Frequency Markers and Value Labels hosts out of the two views
  into the editor window, unchanged. Invariant: exactly one host per open signal, whatever view
  is loaded.
- **Verify:** grep shows one `onOpen…Editor` handler per dialog, all in `ProjectEditor.qml`; no
  dialog file names an id of the views it left; `code-verify.py --check`.
- **Deps:** none
- [x] done

### T6 — Translations, manual, TODO

- **Files:** `core/Ui/UI/CommandStrings.cpp` (generated), `doc/help/Project-Editor.md`,
  `doc/claude/TODO.md`
- **Does:** regenerate the command strings; name the menu and palette entries beside the three
  toolbar descriptions; remove the "Commands to add" item.
- **Verify:** `generate-command-strings.py --check`; `documentation-verify.py`,
  `term-verify.py`, `claim-verify.py --check`.
- **Deps:** T4
- [x] done

### T7 — Review and handoff

- **Files:** none (fixes land in the task they belong to)
- **Does:** `qt-cpp-review` and `qt-qml-review` on the diff; counterfactual check named in chat
  (the rule most at risk is "one rule per dialog": evidence is a grep showing no binding
  re-derives a rule); spec criteria ticked where a check ran, the rest left for the
  maintainer's observations.
- **Verify:** review findings addressed or listed; `git status` shows only files from the plan.
- **Deps:** T1 to T6
- [x] done
  - Status (2026-11-09): `qt-cpp-review` (six lenses) on the C++ diff: lint 0 errors, no confirmed
    findings; targets recorded: a full Dataset copy per menu evaluation (microseconds at menu
    rate), a silent no-op when the dialog's dataset was deleted (existing commit behavior).
    `qt-qml-review` (six lenses) on the QML diff: no confirmed findings; one target acted on, a
    hoisted dialog surviving a project switch could commit into the new project's dataset at the
    same position, so the dialogs now close on a path change and when the editor hides
    (amendment 1). Counterfactual: no QML binding derives an availability rule; the Frequency
    Markers rule is one C++ function behind the toolbar, palette and menu, and Alarm Bands stays
    spec 0093's `datasetRendersAlarmBands`.

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` is met and checked off there.
- [ ] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [ ] `qt-cpp-review` run on the C++ diff; findings addressed or noted.
- [ ] `ss-hotpath` checks pass / `--benchmark-hotpath` not regressed (if hotpath touched).
- [ ] Relevant `pytest` tests identified for the maintainer to run (listed in `plan.md`).
- [ ] `python scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [ ] Diff is *what was asked, and only that* — no scope creep, no foreign files touched.
- [ ] `spec.md` status set to `done`.
