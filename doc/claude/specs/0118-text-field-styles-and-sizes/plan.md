---
spec: 0118-text-field-styles-and-sizes
phase: plan
status: approved     # draft -> approved (gate before /ss-tasks); approved by the maintainer 2026-11-10
updated: 2026-11-10
---

# Plan 0118 — Text Field Output Control Styles and Sizes

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Read the relevant `doc/claude/` sub-docs and the *actual code*
> before writing this — a plan grounded in a stale mental model is worse than no plan.
> Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

The four new styles are appended to the existing `DataModel::OutputWidgetStyle` enum (values 4
to 7), so a text field keeps the single `style` key, API field and editor row that spec 0113
gave buttons and knobs. `normalize()` allows these values only on a text field. A style that
does not fit the field's Text/Number kind is stored as is (R6). One pure function picks the
style actually drawn, and every consumer calls it. Size needs no model change: every output
widget already stores `size`, and the presentation map already carries `sizeScale`. Only the
panel packer and the text field's QML start reading it. The keypad's key set, its entry
editing and the command history are pure Core functions, so their acceptance criteria are unit
tests: a text field control cannot be built in a unit test (spec 0113 T23). The QML gains one
part per style, loaded only for that style.

## Affected subsystems & files

| File | Change |
|------|--------|
| `core/Core/DataModel/OutputWidget.h/.cpp` | Append `ReadoutDisplay = 4`, `KeypadEntry = 5`, `CommandLine = 6`, `Compact = 7` to `OutputWidgetStyle`. `normalize()` clamps to `Compact` and allows 4 to 7 only on `TextField`. `applyStyleChoice()` maps the text field's combo index 0 to 4 onto Standard and 4 to 7. Adds `effectiveTextFieldStyle(const OutputWidget&)`, the R6 kind rule. |
| `core/Core/DataModel/OutputTextEntry.h/.cpp` (new), `core/Core/CMakeLists.txt` | Pure keypad and history decisions: `keypadOffersDecimal`, `keypadOffersSign`, `keypadApply(entry, key)` (digit, point, sign toggle, backspace, clear; one point, no leading zeros), and `pushCommandHistory(list, line)` (newest first, cap 20, no consecutive duplicate, no empty line). |
| `core/Ui/UI/Widgets/Output/TextField.h/.cpp` | A per-control session history (`pushCommandHistory` on every Text-kind send) and `historyAt(int)` for the Up/Down recall. Keypad helpers exposed as slots that call the Core functions. |
| `core/Ui/UI/Widgets/Output/Base.cpp` | The presentation map adds `effectiveStyle`, so QML never re-derives R6. |
| `core/Ui/UI/Widgets/Output/PanelLayout.cpp` | The size scale also applies to `TextField`. Per-style minimum sizes: the readout is taller (large digits), Compact is narrower (no send button). |
| `core/Ui/ProjectEditor/ProjectEditor.h` | New role `ComboBoxDisabled = Qt::UserRole + 26`, a list of disabled indices, added to `roleNames()`. |
| `core/Ui/ProjectEditor/EditorForms/OutputValueRows.cpp` | `appendTextFieldRows` adds the Text Field Style row (five entries; the ones the kind cannot take are disabled, and the description says why) and the Text Field Size row. |
| `core/Ui/ProjectEditor/EditorCommit.cpp` | A style change on a text field goes through `applyStyleChoice`, and `kOutputWidget_InputKind` already rebuilds the form, so the greying follows the kind. `kOutputWidget_Size` already exists. |
| `core/Ui/ProjectEditor/EditorMultiSelect.cpp` | Already copies size and style. Verify that multi-select offers both rows when every selected control is a text field. |
| `app/qml/ProjectEditor/Views/TableDelegate.qml` | The ComboBox delegate honours `comboBoxDisabled`: those items are drawn disabled and cannot be activated. |
| `app/qml/Widgets/Dashboard/Output/DashboardTextField.qml` | Reads `effectiveStyle` and `sizeScale`. Standard and Compact share today's field (Compact hides the send button). Readout, Keypad and Command Line load their own parts. Fonts, heights and the send button scale with the size. |
| `app/qml/Widgets/Dashboard/Output/TextFieldReadout.qml`, `TextFieldKeypad.qml`, `TextFieldCommandLine.qml` (new), `app/CMakeLists.txt` | One Pro-only part per style, with the commercial header. The keypad is a `Popup` anchored to the field, with object names `<prefix>.keypad.<key>`. |
| `core/Api/API/Handlers/ProjectUpdateCommands.cpp`, `ProjectOutputWidgetCommands.cpp` | `size` and `style` are already parsed and returned. The descriptions gain the text field's style values and say that size applies to buttons and text fields. |
| `app/tests/tst_frame_serialization.cpp`, `app/tests/tst_output_text_entry.cpp` (new), `app/tests/CMakeLists.txt` | AC2, AC3, AC4. |
| `tests/integration/test_output_text_field_styles.py` (new) | AC6. |
| `doc/help/Output-Controls.md`, `app/rcc/ai/skills/output_widgets.md` | The styles, the sizes, the kind rule. `Output-Controls.md` has open maintainer edits, so it lands after them. |

## Architecture & data flow

Project JSON `outputStyle` (already exists) and `outputSize` (already exists) feed
`OutputWidget::style` and `size`. `normalize()` applies the type rule only.
`Base::presentation()` calls `effectiveTextFieldStyle()` (the kind rule) to fill
`effectiveStyle` and `sizeScale`. `DashboardTextField.qml` picks a part from that.

Every part sends through the control's existing `sendText()`, so admission, pacing, reply
tokens and the range message are unchanged (R10). The keypad edits a local entry string
through `keypadApply`; only its Enter key calls `sendText`. The command line pushes into the
control's history on a successful Text send, and its arrow keys read `historyAt(i)`. A recall
fills the line and never sends.

## Hotpath & threading impact

None. No pipeline, FrameReader, FrameBuilder or Dashboard tick code is touched, no cached
hotpath flag gains an input, and no signal hop is added. Everything runs on the GUI thread at
operator-action or project-edit rate.

## Data model & persistence

- No new key. `outputStyle` and `outputSize` already exist, are written only when not default,
  and are stored for every output widget type today (premise check: no shipped project carries
  either on a text field).
- `normalize()` accepts styles 4 to 7 on a text field only, and resets them on other types,
  the 0113 rule. It does **not** reset a kind-mismatched text-field style (R6); the mismatch is
  resolved at draw time.
- Released builds (4.2.1 and earlier) have no `outputStyle` key and ignore it. A master build
  from before 0118 clamps 4 to 7 to 3 (Selector), finds that a knob style on a text field, and
  resets it to Standard. Both open the field as Standard, as the spec requires.

## API / SDK surface

`project.outputWidget.update` already takes `style` and `size`, and `.get` already returns
them. Only the description text changes, so the schema dump (`--dump-api-schema`) and
`generate-sdk.py` are maintainer steps after the build, as in 0113 T22. The automation surface
keeps `<prefix>.input` for the field. Keypad keys are `<prefix>.keypad.0` to `.9`, `.point`,
`.sign`, `.back`, `.clear`, `.cancel` and `.enter`, and the catalogue reports them as buttons
(stock `Button`s).

## QML / UI

- Readout: a dark-glass display drawn from theme colours (`console_base`, `console_text`), so
  it reads as a display under every theme, with digits at `2.2 * sizeScale` of the UI font.
  Editing swaps in the line field in place, and Escape restores the readout.
- Keypad: a 4x4 grid of stock buttons in a `Popup`. Sign and point are hidden when not
  offered. Its motion follows `PopupEnter`/`PopupExit` and `reduceMotion`.
- Command line: a dark well with a `›` prompt in the mono font. Up/Down recall, and Enter sends
  and clears.
- Compact: today's field without the send button.
- Sizes scale the field font, its height, the send button and the caption by `sizeScale`
  (0.75 to 1.6, the button table).

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| How the styles are stored | A: append to the shared `OutputWidgetStyle`. B: per-type ordinal (1 = Readout on a text field). C: a separate `textFieldStyle` key | **A.** One key, unambiguous API values, and the 0113 type validation reused. B makes `style: 1` depend on type; C adds a second style axis for clients and the editor. |
| Where the R6 kind rule lives | A: `normalize()` resets it (as 0113 does for type). B: keep the stored style, resolve at draw time | **B**, as the spec requires. A field's kind is toggled often in the editor, and losing the style each time is a trap. One pure function serves both C++ and QML. |
| Keypad form | A: a popup anchored to the field. B: a modal dialog. C: an always-visible inline keypad | **A**, the spec's assumption. B takes focus from the dashboard and C takes a large cell even when unused. |
| Where keypad and history logic lives | A: Core pure functions. B: QML JavaScript | **A**, so AC3 and AC4 are unit tests. QML JS is untestable here without a GUI test. |
| Greying combo entries | A: new `ComboBoxDisabled` role plus a delegate. B: no greying, only the description | **A**, since R8 asks for it explicitly. It is one role and one delegate change, and other rows can reuse it. |

## Risks & mitigations

- **Older builds drop the style on save.** An older build's `normalize()` resets styles 4 to 7
  to Standard; if that build then saves the project, the style is lost. This is acceptable: it
  is what the spec allows (opens as Standard), and 0113 behaves the same way.
- **Keypad popup and the dashboard's focus model.** A popup inside a frozen or locked dashboard
  could open on a disabled control. The keypad opens only when the control is enabled and the
  link is up, the same gate as the send button.
- **`TableDelegate.qml` is shared by every editor form.** The disabled-item delegate applies
  only when the role is set, and an absent role keeps today's delegate.
- **Help page contention.** `Output-Controls.md` has the maintainer's uncommitted edits. The
  help task runs last, after asking.

## Test & verification plan

- **Unit (ctest, CI):** `tst_frame_serialization` (AC2: every style and size round-trip;
  Standard/Normal writes neither key; a wrong-kind style reads as Standard through
  `effectiveTextFieldStyle` and the stored value survives; a text-field style on a button
  resets). `tst_output_text_entry` (AC3: offered keys by Min, decimals and kind, entry editing
  with sign, point and clear, and an out-of-range result refused by the existing
  `admitOutputValue`, never clamped; AC4: history order, cap, duplicate and empty rules).
- **Integration (pytest, app up):** `tests/integration/test_output_text_field_styles.py`
  (AC6: `get` reports style and size; patching either rebuilds the dashboard field, checked
  through `ui.automation.listElements`).
- **Round-trip (AC1):** the existing frame-serialization and property-registry baseline suites
  stay green with no recapture.
- **Maintainer observation (AC5, AC7):** a rig project with one field per style and size,
  driven over the API as in the 0113 visual pass.
- **Static:** `code-verify.py --check`, qmllint on the new parts, `layer-verify.py` for the new
  Core file, `sanitize-commit.py` before the push.
