---
spec: 0109-output-write-limits
phase: tasks
status: approved     # 2026-11-03: reviewed and approved by the main session under the maintainer's delegation (plans: maintainer, tasks: main session)
updated: 2026-11-03
---

# Tasks 0109 — Output Control Write Limits

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
- **Executor** marks a task delegable to a cheaper model against the donor it names (CLAUDE.md
  "Delegation & Cost"): `mechanical` for Haiku, `yes` for Sonnet. Tasks marked `main` stay in
  the main session: the admission rule (T4), the `sendValue` edit beside the license gates (T6).
- **Syntax check.** Every edited C++ file goes through `python scripts/syntax-check.py`; a new TU
  borrows a same-library donor's compile entry. A SKIP is reported as unverified.
- **No hotpath.** No task touches the pipeline, `FrameReader`, `CircularBuffer`, `FrameBuilder`
  or dashboard ingest; `--benchmark-hotpath` is not required.
- **Commit shape.** The amended spec and the approved plan and tasks land first in their own
  commit. Then: T1 alone with a `Mechanical:` trailer; T2 to T5 (model and rule); T6 to T11
  (controls); T12 to T14 (editor and API); T15 to T19 (docs and tests). 0110 rebases on this.
- **Generated files are never edited.** `api-schema.json` and the typed proto change only in T20.

## Tasks

### T1 — Move the `OutputWidget` serializer body out of `Frame.h`

- **Files:** `core/Core/DataModel/Frame.h`, `core/Core/DataModel/Frame.cpp`
- **Does:** The inline body of `serialize(const OutputWidget&)` (`Frame.h:212-263`) becomes a
  declaration; the body moves verbatim to `Frame.cpp` beside `read(OutputWidget&)`. Donor:
  `serialize(const AlarmBand&)`, declared in the header and defined in `Frame.cpp`. No
  behaviour change: it pays for the new fields, since `Frame.h` is at 1493 lines and
  `--tu-census` counts headers.
- **Verify:** `python scripts/syntax-check.py core/Core/DataModel/Frame.cpp`; `python
  scripts/code-verify.py --check` on both; `wc -l` shows `Frame.h` about 40 lines shorter;
  `git diff --stat` shows a pure move.
- **Executor:** mechanical.
- **Deps:** none
- [x] done

### T2 — Keys, the input-kind enum and the four fields

- **Files:** `core/Core/DataModel/FrameKeys.h`, `core/Core/DataModel/Frame.h`
- **Does:** `Keys::OutputInputKind("inputKind")`, `OutputDecimals("outputDecimals")`,
  `OutputUnits("outputUnits")`, `OutputQuantize("quantizeToStep")` after the 0080 group
  (`FrameKeys.h:154-160`). `enum class OutputInputKind : quint8 { Text = 0, Number = 1 }` beside
  `OutputStateSource` (`Frame.h:173`). `OutputWidget` gains `inputKind = Text`, `decimals = -1`
  (automatic), `units`, `quantize = false`. Declarations of `outputIsNumeric(const
  OutputWidget&)`, `enum class OutputAdmission { Accepted, NotANumber, OutOfRange }` and
  `admitOutputValue(const OutputWidget&, const QVariant&, double&)`. **Invariant:** every
  default reproduces today's behaviour (R7); `Keys::` is the only spelling of the names.
- **Verify:** `python scripts/syntax-check.py core/Core/DataModel/Frame.cpp`; `python
  scripts/code-verify.py --check` on both; `python scripts/code-verify.py --tu-census --check`.
- **Executor:** yes, donor the 0080 block (`OutputStateSource`, `stateSource` fields).
- **Deps:** T1
- [x] done

### T3 — Persist and normalise the four fields

- **Files:** `core/Core/DataModel/Frame.cpp`
- **Does:** `serialize` writes each new key only when it differs from its default; `read()`
  reads them (`Frame.cpp:247-281`); `normalize()` (`:211-244`) clamps `inputKind` to the enum and
  `decimals` to -1..9 and simplifies `units`. **Invariant:** a pre-spec project serialises
  byte-identical (AC3), and `normalize` stays the one validation point both doors call.
- **Verify:** `python scripts/syntax-check.py core/Core/DataModel/Frame.cpp`; `python
  scripts/code-verify.py --check core/Core/DataModel/Frame.cpp`.
- **Executor:** yes, donor the `stateSource` block in the same `serialize` and the `stateSource`
  clamp in `normalize`.
- **Deps:** T2
- [x] done

### T4 — The admission rule

- **Files:** `core/Core/DataModel/Frame.cpp`
- **Does:** `outputIsNumeric`: Slider, Knob, or TextField with `Number`. `admitOutputValue`:
  a number type is taken as is; a string through `SerialStudio::toDouble` with its ok flag;
  non-finite is `NotANumber` (reuse `finiteOr`'s reasoning, `:201`); with `quantize` and a
  positive step, `min + round((v - min) / step) * step`; then `OutOfRange` outside Min to Max,
  else `Accepted` with the double. Pure, no Qt GUI, no translation (Decisions 3 and 4).
- **Verify:** `python scripts/syntax-check.py core/Core/DataModel/Frame.cpp`; `python
  scripts/code-verify.py --check core/Core/DataModel/Frame.cpp`.
- **Executor:** main.
- **Deps:** T2
- [x] done
  (Main session 2026-11-03: a quantized value that overshoots Max by rounding alone, under Step*1e-9, snaps
  to Max instead of being refused.)

### T5 — Unit tests: the rule and the round trip

- **Files:** `app/tests/tst_output_write_limits.cpp` (new), `app/tests/CMakeLists.txt`,
  `app/tests/tst_frame_serialization.cpp`
- **Does:** New target registered like `tst_output_state_binding` (`CMakeLists.txt:2461-2467`),
  linking `SerialStudio::Core` only. Cases: in range, below, above, `"abc"`, `"inf"`, `"nan"`;
  Min 0 Step 5 quantizes 43 to 45 and 42 to 40; quantize off leaves 43; Max 100 quantizes 103 to
  105 and refuses it; `outputIsNumeric` for every type and both kinds. In
  `tst_frame_serialization`, `outputWidgetRoundTripsEveryField` (`:377`) covers the four fields
  and a new case asserts a default widget's JSON has none of the four keys (AC3 unit half).
  Every expected value is a literal.
- **Verify:** `python scripts/syntax-check.py` on both `.cpp`; `python scripts/code-verify.py
  --check` on all three; after the maintainer builds, `ctest -R
  "tst_output_write_limits|tst_frame_serialization"`.
- **Executor:** yes, donor `app/tests/tst_output_state_binding.cpp` and its registration.
- **Deps:** T3, T4
- [x] done
  - Status: Written; both .cpp pass syntax-check. Rule cases (tst_output_write_limits) cannot link or run until T4 defines outputIsNumeric/admitOutputValue; not built.

### T6 — Admission in `Base::sendValue`

- **Files:** `core/Ui/UI/Widgets/Output/Base.h`, `core/Ui/UI/Widgets/Output/Base.cpp`
- **Does:** After the two license checks (`Base.cpp:343-349`) and before pacing, a private
  `admit()` runs `admitOutputValue` for a numeric control and replaces the value with the double;
  on refusal it sets `rangeMessage` ("Allowed range: %1 to %2%3" or "Enter a number from %1 to
  %2%3", through `formatted()`) and returns. A non-numeric control passes through untouched.
  Adds `numeric` and `decimals` (CONSTANT), `rangeMessage` (+ `rangeMessageChanged`), initialises
  `m_units` from config (never assigned today, ctor `:38-51`), and public slots `stepped(from,
  steps)` (`qBound(min, from + steps * step, max)`) and `formatted(value)` (decimals, or the
  automatic `step < 1 ? 2 : 0`). **Invariants:** the license gate order is unchanged and nothing
  is added ahead of it; a refusal runs no script, replaces no held value, calls no
  `noteOperatorRequest` and writes nothing; `deliverValue` stays reachable only from `sendValue`
  and `flushPendingValue` (R3).
- **Verify:** `python scripts/syntax-check.py core/Ui/UI/Widgets/Output/Base.cpp`; `python
  scripts/code-verify.py --check` on both; `python scripts/code-verify.py --singleton-census
  --check`.
- **Executor:** main.
- **Deps:** T4
- [x] done
  (Main session 2026-11-03: also adds `admittedValue` (NOTIFY) so a quantized send shows what was sent; a
  license refusal returns before `admit()`, so `rangeMessage` is untouched by it.)

### T7 — One widget map for the panel and the preview

- **Files:** `core/Ui/UI/Widgets/Output/Base.h/.cpp`, `core/Ui/UI/Widgets/Output/Panel.cpp`,
  `core/Ui/UI/Widgets/Output/Preview.cpp`
- **Does:** Static `Base::presentation(const OutputWidget&)` builds the `QVariantMap` both copies
  build today (`Panel.cpp:88-103`, `Preview.cpp:217-231`), plus `groupId`, `widgetId`,
  `inputKind`, `decimals`, `units`. Both call sites use it.
- **Verify:** `python scripts/syntax-check.py` on the three `.cpp`; `python
  scripts/code-verify.py --check` on all; the two old map blocks are gone.
- **Executor:** yes, donor `Panel.cpp:88-103`.
- **Deps:** T6
- [x] done
  - Status: syntax-check ok on Base/Panel/Preview; the two IconEngine includes left Panel.cpp and Preview.cpp with the map code.

### T8 — `TextField::sendText` reports admission

- **Files:** `core/Ui/UI/Widgets/Output/TextField.h`, `core/Ui/UI/Widgets/Output/TextField.cpp`
- **Does:** `sendText` returns whether the value was admitted (`rangeMessage().isEmpty()` after
  `sendValue`), `[[nodiscard]]`, still a public slot. `sendValue` keeps its `void` signature.
- **Verify:** `python scripts/syntax-check.py core/Ui/UI/Widgets/Output/TextField.cpp`; `python
  scripts/code-verify.py --check` on both.
- **Executor:** mechanical.
- **Deps:** T6
- [x] done
  - Status: Written; syntax-check FAILS until T6 adds Base::rangeMessage() (only error: undeclared identifier).

### T9 — Slider and knob: Step keys, focused wheel, decimals, units, ids

- **Files:** `app/qml/Widgets/Dashboard/Output/DashboardSlider.qml`,
  `app/qml/Widgets/Dashboard/Output/DashboardKnob.qml`
- **Does:** `property string idPrefix`; `objectName` `idPrefix + ".slider"` / `".knob"` when the
  prefix is set. `Keys.onPressed`: Up/Right +1, Down/Left -1, Shift x10, writes
  `model.currentValue = model.stepped(model.currentValue, n)` and accepts the event. A
  `WheelHandler` enabled only while the control has `activeFocus` does the same per notch
  (Decision 6). Value label through `model.formatted()` plus the units. **Invariant:** never
  assign `control.value` from a key or wheel handler: the built-in key path breaks the binding to
  `desiredValue`, which is why a keyboard-nudged slider stops following 0080 feedback today.
- **Verify:** `python scripts/code-verify.py --check` on both; `qt-qml-review` on the pair.
- **Executor:** yes, donor `DashboardSlider.qml`'s `onMoved` / `onPressedChanged` and the
  `objectName` style of `WidgetDelegate.qml:35`.
- **Deps:** T6, T7
- [x] done
  - Status: code-verify clean; qt-qml-review not run. Needs T6 for stepped()/formatted()/units.

### T10 — The Number text field

- **Files:** `app/qml/Widgets/Dashboard/Output/DashboardTextField.qml`
- **Does:** `idPrefix`; `objectName`s `.input`, `.send`, `.outOfRange`. Text kind unchanged
  (clears after sending). Number kind: numeric input hints, **no `Validator`** (a bounded one
  swallows Enter and the range message would never show); placeholder names the range; send
  keeps the text when `sendText` returns false and shows `model.formatted(admitted)` when it
  returns true; Up/Down nudge the text through `model.stepped()` from the parsed text (Min when
  empty), Shift x10, without sending (Decision 5); focused-only wheel; units suffix; the
  `rangeMessage` label under the row in the alarm colour, visible while non-empty. The label
  lives here, not in the panel cell, so the preview shows it (R5).
- **Verify:** `python scripts/code-verify.py --check` on the file; `qt-qml-review`.
- **Executor:** yes, donor T9's key handler and `DashboardSlider.qml`'s label styling.
- **Deps:** T8, T9
- [x] done
  - Status: code-verify clean; needs T6 (rangeMessage, stepped, formatted) and T8. After a send it shows model.admittedValue when the model has one, else the parsed number (T6 should expose it so a quantized 43 reads 45).

### T11 — Cell identity in the output panel

- **Files:** `app/qml/Widgets/Dashboard/Output/DashboardOutputPanel.qml`
- **Does:** The cell `Rectangle` gets `objectName: "dashboard.output." + cell.owData.groupId +
  "." + cell.owData.widgetId` and passes that as `idPrefix` to the slider, knob and text field
  components. The transmit preview passes no prefix and stays unnamed.
- **Verify:** `python scripts/code-verify.py --check` on the file (`qml-objectname-literal`
  clean).
- **Executor:** mechanical, donor `WidgetDelegate.qml:35`.
- **Deps:** T7, T9, T10
- [x] done
  - Status: code-verify clean.

### T12 — Value Range rows in their own file

- **Files:** `core/Ui/ProjectEditor/EditorForms/OutputValueRows.h/.cpp` (new),
  `core/Ui/ProjectEditor/EditorForms.cpp`, `core/Ui/CMakeLists.txt`
- **Does:** `appendOutputValueRows(model, widget)` takes the body of
  `EditorForms::buildOutputWidgetValueRows` (`EditorForms.cpp:1302-1344`), whose body becomes one
  call, so both callers (`:1362`, `EditorMultiSelect.cpp:305`) are untouched. The section shows
  for a text field too: Input (Text, Number) first; Min, Max, Step, Decimals, Units for every
  numeric control (labels per Decision 2); Quantize to Step for a Number field. Visibility goes
  through `DataModel::outputIsNumeric`. CMake beside `OutputStateRows` (`:316`, `:341`).
- **Verify:** `python scripts/syntax-check.py` on both `.cpp` (new TU via a same-library donor
  entry); `python scripts/code-verify.py --check` on all; `--tu-census --check` shows
  `EditorForms.cpp` shrinking.
- **Executor:** yes, donor `core/Ui/ProjectEditor/EditorForms/OutputStateRows.{h,cpp}`.
- **Deps:** T2, T4
- [x] done
  - Status: syntax-check ok; EditorForms.cpp 1486 -> 1448 lines.

### T13 — Editor commit and multi-select

- **Files:** `core/Ui/ProjectEditor/ProjectEditorItemIds.h`,
  `core/Ui/ProjectEditor/EditorCommit.cpp`, `core/Ui/ProjectEditor/EditorMultiSelect.cpp`
- **Does:** Append `kOutputWidget_InputKind`, `_Decimals`, `_Units`, `_Quantize` after
  `kOutputWidget_StateConfirmMs` (`:118`). Four cases in `applyOutputWidgetField` (`:1168`),
  clamped as `normalize` does; `InputKind` joins the rebuild condition at `:1257`. The four
  fields in `outputWidgetEditValues` (`EditorMultiSelect.cpp:243-261`).
- **Verify:** `python scripts/syntax-check.py` on both `.cpp`; `python scripts/code-verify.py
  --check` on all three.
- **Executor:** yes, donor the `kOutputWidget_StateConfirmMs` case and its row.
- **Deps:** T12
- [x] done
  - Status: syntax-check ok.

### T14 — API describe and patch

- **Files:** `core/Api/API/Handlers/ProjectOutputWidgetCommands.cpp`,
  `core/Api/API/Handlers/ProjectUpdateCommands.cpp`
- **Does:** `outputWidgetGet` (`:216-277`) returns `inputKind`, `decimals`, `units`, `quantize`;
  its description names them. `applyOutputWidgetFields` (`:183-242`) takes the four keys before
  its `normalize()`; the `project.outputWidget.update` description (`:351-373`) names them and
  says the control enforces Min to Max before `transmit()`. No capability or sealed change.
- **Verify:** `python scripts/syntax-check.py` on both; `python scripts/code-verify.py --check`
  on both; `python scripts/registry-verify.py`.
- **Executor:** yes, donor `applyOutputStateFields` (`ProjectUpdateCommands.cpp:145-175`) and
  the state block of `outputWidgetGet`.
- **Deps:** T3
- [x] done
  - Status: syntax-check ok; registry-verify clean.

### T15 — Templates and the manual

- **Files:** `app/rcc/scripts/output/pwm_control.js`, `app/rcc/scripts/output/pid_setpoint.js`,
  `doc/help/Output-Controls.md`
- **Does:** Both templates lose their `Math.max(..., Math.min(...))` clamp and say in a comment
  that the control's Min and Max hold the range; `Math.round` stays (Decision 7). Manual: Text
  Field gains the Number kind, the out-of-range message and Step keys; Slider and Knob tables gain
  Decimals and Units with Min, Max, Step naming (Decision 2); a short limits paragraph; the PWM and
  PID examples match the templates (R8, AC5).
- **Verify:** `python scripts/documentation-verify.py`; `python scripts/term-verify.py`; grep
  shows no clamp in either template or manual example.
- **Executor:** yes, with the `ss-docs` skill; donor the manual's State Feedback section.
- **Deps:** T10, T12
- [x] done
  - Status: documentation-verify and term-verify clean.

### T16 — API reference and assistant corpus

- **Files:** `doc/help/API-Reference.md`, `app/rcc/ai/skills/output_widgets.md`,
  `app/rcc/ai/docs/output_widget_js.md`
- **Does:** The get and update entries list the four fields. The assistant learns the Number
  kind, that the control enforces the range before `transmit()`, and to read
  `project.outputWidget.get` before driving a control.
- **Verify:** `python scripts/registry-verify.py` (corpus field lint); `python
  scripts/term-verify.py`.
- **Executor:** yes, donor the existing `stateConfirmMs` entries in each file.
- **Deps:** T14
- [x] done
  - Status: registry-verify and term-verify clean.

### T17 — Architecture note

- **Files:** `doc/claude/architecture/dashboard.md`
- **Does:** "Output Widgets (Pro)" (`:578`) gains two sentences: admission happens once, in
  `Base::sendValue` after the license gates, through `DataModel::admitOutputValue`; a per-control
  clamp is a defect, the slider's geometry clamp excepted.
- **Verify:** `python scripts/claim-verify.py --no-report`.
- **Executor:** yes.
- **Deps:** T6
- [x] done
  - Status: claim-verify clean.

### T18 — Capture mode in the device simulator

- **Files:** `tests/utils/device_simulator.py`
- **Does:** Opt-in `capture=True`: the client watcher (`_monitor_client`, `:152-182`) consumes
  and stores what the app writes, behind a lock, with `received()` and `clear_received()`. Off by
  default, so the peek-only behaviour every existing test relies on is unchanged.
- **Verify:** `python -m py_compile tests/utils/device_simulator.py`; a local smoke run: start a
  capturing simulator, connect a plain socket, send bytes, read them back from `received()`.
- **Executor:** yes, donor `_monitor_client`.
- **Deps:** none
- [x] done
  - Status: Smoke run passed: capture=True stores bytes, capture=False unchanged.

### T19 — Integration and structural tests

- **Files:** `tests/integration/test_output_write_limits.py` (new)
- **Does:** AC1 (`gui` marker): project with a Number field 0 to 100, Step 5, Quantize on,
  transmit `"SP " + v + "\n"`; click `dashboard.output.G.W.input`, key `250`, Enter: the
  `.outOfRange` element is listed and nothing is captured in 500 ms; key `43`, Enter: one payload,
  `SP 45`. AC2: `project.outputWidget.get` returns `type` 3, `inputKind` 1, `minValue`,
  `maxValue`, `stepSize`, `units`. AC3: Text and Number round-trip; a pre-spec fixture's output
  widgets are byte-identical after save. AC5 (static): no clamp in the manual's examples or the
  two templates. R3 (static): `deliverValue(` called only from `sendValue` and
  `flushPendingValue`, admission before pacing in `sendValue`.
- **Verify:** the static tests run here with `pytest
  tests/integration/test_output_write_limits.py -k "static" -v`; the rest needs the app.
- **Executor:** yes, donors `tests/integration/test_ui_automation.py` (`ui`, `link`, `_element`,
  `_drive`), `test_output_state_feedback.py` (source scans), `test_output_widgets.py` (project
  builders).
- **Deps:** T11, T14, T15, T18
- [x] done
  - Status: Static tests pass except the admission scan, which needs T6; the app-driven tests were only py_compiled.

### T20 — Maintainer: build, schema, tests, observation

- **Files:** `app/rcc/api/api-schema.json` and the SDK files `generate-sdk.py` rewrites
  (regenerated); translation files (regenerated)
- **Does:** Build; `ctest -R "tst_output_write_limits|tst_frame_serialization|
  tst_output_state_binding"`; `--dump-api-schema` and `python scripts/generate-sdk.py`;
  regenerate translations for the renamed rows; launch with the API server on and run T19
  (including `-m gui`); AC4 by hand.
- **Verify:** AC1 to AC5 ticked in `spec.md` with the date.
- **Executor:** maintainer.
- **Deps:** T1 to T19
- [ ] done

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` is met and checked off there (AC1 by T19 and T20;
  AC2 by T19; AC3 by T5 and T19; AC4 by T20; AC5 by T15 and T19).
- [ ] `python scripts/code-verify.py --check` is clean on all changed files (no new errors);
  `--tu-census --check` and `--singleton-census --check` do not grow.
- [ ] `python scripts/syntax-check.py` ran on every edited C++ file; any SKIP named in the handoff.
- [ ] `qt-cpp-review` on T1 to T8 and T12 to T14; `qt-qml-review` on T9 to T11.
- [ ] Hotpath not touched; no benchmark run required.
- [ ] Relevant `pytest` targets identified for the maintainer: `tests/integration/
  test_output_write_limits.py`, `test_output_widgets.py`, `test_output_state_feedback.py`.
- [ ] `python scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [ ] Diff is *what was asked, and only that*: no foreign files, no `.ts` edited by hand.
- [ ] `spec.md` status set to `done`.
