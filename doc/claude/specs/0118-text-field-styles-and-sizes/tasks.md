---
spec: 0118-text-field-styles-and-sizes
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement); approved by the maintainer 2026-11-10
updated: 2026-11-10
---

# Tasks 0118 — Text Field Output Control Styles and Sizes

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
- No task touches the hotpath. The compile database is still missing under `build/`, so C++ is
  syntax-checked by the maintainer's build and CI, not locally.

## Tasks

### Stage A — model and pure decisions

### T1 — Text-field styles in the style enum

- **Files:** `core/Core/DataModel/OutputWidget.h`, `core/Core/DataModel/OutputWidget.cpp`
- **Does:** Appends `ReadoutDisplay = 4`, `KeypadEntry = 5`, `CommandLine = 6`, `Compact = 7`.
  `normalize()` clamps to `Compact`, allows 4 to 7 only on `TextField`, and keeps a kind
  mismatch (R6). `applyStyleChoice()` maps a text field's combo index 0 to 4. Adds
  `effectiveTextFieldStyle()`. Invariant: Standard and Normal still write no key (R28 rule), and
  `outputSendsUnpaced` and `outputStartsLatched` stay unchanged.
- **Verify:** `code-verify.py --check`; read-back of `normalize()` for each type.
- **Deps:** none
- [x] done

### T2 — Keypad and history decisions

- **Files:** `core/Core/DataModel/OutputTextEntry.h/.cpp` (new), `core/Core/CMakeLists.txt`
- **Does:** Adds `keypadOffersDecimal`, `keypadOffersSign`, `keypadApply` (digit, point, sign,
  backspace, clear; one point, no leading zeros) and `pushCommandHistory` (newest first, cap
  20, no consecutive duplicate, no empty line). Pure, with no Qt object.
- **Verify:** `code-verify.py --check`; `layer-verify.py` (the new file is owned).
- **Deps:** none
- [x] done

### T3 — Units for AC2, AC3, AC4

- **Files:** `app/tests/tst_output_text_entry.cpp` (new), `app/tests/tst_frame_serialization.cpp`,
  `app/tests/CMakeLists.txt`
- **Does:** AC3 and AC4 against T2, plus the out-of-range refusal through `admitOutputValue`.
  AC2 round-trips each style and size; a Standard, Normal field writes neither key; a
  wrong-kind style resolves to Standard and is still stored; a text-field style on a button
  resets. Executor: Sonnet; donors are `tst_output_hardware_decisions.cpp` and the 0113 cases in
  `tst_frame_serialization.cpp`.
- **Verify:** `code-verify.py --check`; CI ctest.
- **Deps:** T1, T2
- [x] done

### Stage B — controls and layout

### T4 — The text-field control: history and keypad slots, effective style

- **Files:** `core/Ui/UI/Widgets/Output/TextField.h/.cpp`, `core/Ui/UI/Widgets/Output/Base.cpp`
- **Does:** A Text-kind send that is admitted is pushed into a per-control history.
  `historyAt(int)`, plus keypad slots that wrap the T2 functions. `presentation()` adds
  `effectiveStyle`. Invariant: a recall or a keypad key never transmits; only `sendText` does.
- **Verify:** `code-verify.py --check`.
- **Deps:** T1, T2
- [x] done

### T5 — Panel cells sized for text-field size and style

- **Files:** `core/Ui/UI/Widgets/Output/PanelLayout.cpp`
- **Does:** The size scale applies to `TextField`. The readout gets a taller minimum and Compact
  a narrower one.
- **Verify:** `code-verify.py --check`; read-back against `minSize` for each style.
- **Deps:** T1
- [x] done

### Stage C — Project Editor

### T6 — Greyable combo entries

- **Files:** `core/Ui/ProjectEditor/ProjectEditor.h`, `app/qml/ProjectEditor/Views/TableDelegate.qml`
- **Does:** The role `ComboBoxDisabled = Qt::UserRole + 26` (`comboBoxDisabled`). The ComboBox
  delegate draws the listed items disabled and refuses to activate them. An absent role keeps
  today's delegate for every other form.
- **Verify:** `code-verify.py --check`; qmllint.
- **Deps:** none
- [x] done

### T7 — Style and Size rows for a text field

- **Files:** `core/Ui/ProjectEditor/EditorForms/OutputValueRows.cpp`, `core/Ui/ProjectEditor/EditorCommit.cpp`,
  `core/Ui/ProjectEditor/EditorMultiSelect.cpp`
- **Does:** `appendTextFieldRows` adds Text Field Style (five entries; disabled per kind, with
  a description saying why) and Text Field Size. The commit routes the text field's style index
  through `applyStyleChoice`. Multi-select offers both rows for a text-field-only selection.
- **Verify:** `code-verify.py --check`; read-back that an input-kind change rebuilds the form.
- **Deps:** T1, T6
- [x] done (multi-select now also shows the presentation rows, Show Title and the selector
  rows included, when every selected control has the same type)

### Stage D — dashboard parts

### T8 — Sizes and Compact in the text field

- **Files:** `app/qml/Widgets/Dashboard/Output/DashboardTextField.qml`
- **Does:** Reads `effectiveStyle` and `sizeScale`. Fonts, field height, the send button and
  the caption scale with the size. Compact hides the send button, and Enter still sends.
- **Verify:** `code-verify.py --check`; qmllint.
- **Deps:** T4
- [x] done (the caption scales through a new `textScale` on `OutputSectionLabel.qml`, default 1)

### T9 — Readout Display part

- **Files:** `app/qml/Widgets/Dashboard/Output/TextFieldReadout.qml` (new), `DashboardTextField.qml`,
  `app/CMakeLists.txt`
- **Does:** A dark-glass display with large digits and units, showing the reported value when
  bound or the last admitted value otherwise. Click or focus edits in place, Enter sends, and
  Escape restores.
- **Verify:** `code-verify.py --check`; qmllint.
- **Deps:** T8
- [x] done

### T10 — Keypad Entry part

- **Files:** `app/qml/Widgets/Dashboard/Output/TextFieldKeypad.qml` (new), `DashboardTextField.qml`,
  `app/CMakeLists.txt`
- **Does:** A popup keypad anchored to the field, with keys from the T4 slots, sign and point
  only when offered, and object names `<prefix>.keypad.<key>`. Opens only when enabled and
  connected; `PopupEnter`/`PopupExit` motion. Enter sends through `sendText`.
- **Verify:** `code-verify.py --check`; qmllint.
- **Deps:** T8
- [x] done (sign and point keep their grid slots and are hidden, not removed)

### T11 — Command Line part

- **Files:** `app/qml/Widgets/Dashboard/Output/TextFieldCommandLine.qml` (new), `DashboardTextField.qml`,
  `app/CMakeLists.txt`
- **Does:** A dark well with a `›` prompt and the mono font. Up and Down recall through
  `historyAt`, and Enter sends and clears.
- **Verify:** `code-verify.py --check`; qmllint.
- **Deps:** T8
- [x] done (no separate `TextFieldCommandLine.qml`: the command line is the same field with a
  dark palette, a prompt mark in its left padding and Up/Down recall, which needs no new part)

### Stage E — API, docs, integration

### T12 — API descriptions

- **Files:** `core/Api/API/Handlers/ProjectUpdateCommands.cpp`, `ProjectOutputWidgetCommands.cpp`
- **Does:** The descriptions list the text field's style values 4 to 7 and the kind rule, and
  say that size applies to buttons and text fields. The schema dump and the SDK are the
  maintainer's steps after the build.
- **Verify:** `code-verify.py --check`; lines under 100 columns.
- **Deps:** T1
- [x] done

### T13 — Integration test for AC6

- **Files:** `tests/integration/test_output_text_field_styles.py` (new)
- **Does:** Loads a project with a text field, checks that `get` reports style and size, and
  that patching each one rebuilds the dashboard field (seen through
  `ui.automation.listElements`). Executor: Sonnet; donor is
  `tests/integration/test_device_declaration.py`'s consent-tolerant drive.
- **Verify:** run against the maintainer's running app.
- **Deps:** T1, T8
- [x] done (4 passed against the maintainer's build, 2026-11-10)

### T14 — Help and AI skill

- **Files:** `doc/help/Output-Controls.md`, `app/rcc/ai/skills/output_widgets.md`
- **Does:** The four styles, the sizes and the kind rule. `Output-Controls.md` waits for the
  maintainer's open edits; ask first.
- **Verify:** `documentation-verify.py`; `sanitize-commit.py` (doc/help is a generator input).
- **Deps:** T7 to T11
- [x] done (the page also gained the toggle lever and pointer knob sentences 0113 T25 left pending)

### T15 — Live review (AC5, AC7)

- **Files:** none committed (rig in the scratchpad)
- **Does:** A rig project with one field per style at two sizes, run on the maintainer's build:
  screenshots, sends checked on the device log, and an out-of-range number refused.
- **Verify:** maintainer observation.
- **Deps:** T8 to T13
- [x] done (every style driven over automation on the maintainer's build: keypad sent
  `TRIM -12.5` and refused 99, command line sent and recalled, readout edited and sent `SET 37`;
  the readout now shows `--` until a value is sent; AC7 editor greying is the maintainer's look)

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` is met and checked off there.
- [ ] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [ ] `qt-cpp-review` run on the C++ diff; findings addressed or noted.
- [ ] Hotpath untouched (no `--benchmark-hotpath` run needed).
- [ ] `pytest tests/integration/test_output_text_field_styles.py` identified for the maintainer.
- [ ] `python scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [ ] Diff is *what was asked, and only that* — no scope creep, no foreign files touched.
- [ ] `spec.md` status set to `done`.
