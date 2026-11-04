---
spec: 0109-output-write-limits
phase: plan
status: approved     # approved 2026-11-03 (maintainer): all eight recommended answers
updated: 2026-11-03
---

# Plan 0109 — Output Control Write Limits

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Read the relevant `doc/claude/` sub-docs and the *actual code*
> before writing this — a plan grounded in a stale mental model is worse than no plan.
> Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

Every control's value already funnels through one function: QML calls a concrete control's slot
(`Button::click`, `Toggle::setChecked`, `Slider::setCurrentValue`, `TextField::sendText`), and
each of those calls `Widgets::Output::Base::sendValue` (`Base.cpp:341`), which paces and then
runs the transmit script in `deliverValue` (`Base.cpp:381`). Nothing outside QML reaches a
control model (grep: the only users of `Widgets::Output` outside its directory are
`DashboardWidget.cpp`, `OutputCodeEditor.cpp` and the QML type registration), so pointer input,
keys and the spec-0101 driving surface all arrive there. The range check goes into `sendValue`,
after the two license gates and before pacing, as one pure rule `DataModel::admitOutputValue`
beside `normalize(OutputWidget&)` in `Frame.cpp`. A text field gains an input kind (Text or
Number); slider, knob and Number field gain `decimals` and `units`; the Number field gains
Quantize to Step. The refusal message lives inside the shared `DashboardTextField.qml`, which the
Project Editor preview already reuses, so R5 costs nothing extra. No new class, no new API
command, no hotpath code.

## Affected subsystems & files

| File | Change |
|------|--------|
| `core/Core/DataModel/Frame.h` | `OutputInputKind` enum (Text=0, Number=1); `OutputWidget` gains `inputKind`, `decimals` (-1 = automatic), `units`, `quantize`; declarations of `outputIsNumeric(const OutputWidget&)` and `admitOutputValue(...)`. **Moves the body of `serialize(const OutputWidget&)` (`Frame.h:212-263`) to `Frame.cpp`**: the header is at 1493 lines and `--tu-census` counts headers, so the new fields only fit if the inline body leaves (`read()` and `serialize(AlarmBand)` already live in the .cpp). |
| `core/Core/DataModel/Frame.cpp` | `serialize(OutputWidget)` body (moved) writing the four new keys only when non-default (AC3); `read()` reads them; `normalize()` clamps `inputKind` and `decimals` (-1..9); `outputIsNumeric` (Slider, Knob, or TextField with Number) and `admitOutputValue` (parse, reject non-finite, quantize from Min when on, range check). |
| `core/Core/DataModel/FrameKeys.h` | `OutputInputKind("inputKind")`, `OutputDecimals("outputDecimals")`, `OutputUnits("outputUnits")`, `OutputQuantize("quantizeToStep")` beside the 0080 group (`FrameKeys.h:138-160`). |
| `core/Ui/UI/Widgets/Output/Base.h/.cpp` | Admission in `sendValue`; `rangeMessage` property (+ `rangeMessageChanged`) set on refusal and cleared on the next accepted value; `numeric` and `decimals` properties; `m_units` finally initialised from config (today it is declared, exposed as `units`, and never assigned: ctor `Base.cpp:38-51`); `stepped(from, steps)` and `formatted(value)` slots so the step/clamp and the automatic-decimals rule live once in C++; static `presentation(const OutputWidget&)` building the QML widget map. |
| `core/Ui/UI/Widgets/Output/TextField.h/.cpp` | `sendText` returns whether the value was admitted (reads `rangeMessage()` after `sendValue`), so QML keeps the typed text on refusal (R2). |
| `core/Ui/UI/Widgets/Output/Panel.cpp` | Widget map from `Base::presentation()` instead of the inline copy (`Panel.cpp:88-103`). |
| `core/Ui/UI/Widgets/Output/Preview.cpp` | Same, replacing `Preview.cpp:217-231`. Both maps must gain the same new keys; one builder is fewer lines than two edits and closes the drift the QML comment at `DashboardOutputPanel.qml:139-141` already warns about. |
| `app/qml/Widgets/Dashboard/Output/DashboardTextField.qml` | Number mode: numeric input hints and no `Validator` (a ranged validator swallows Enter, so the refusal would never show), range label `<prefix>.outOfRange` under the field, keep the text on refusal and the formatted admitted value on success, Up/Down nudge, wheel when focused, units suffix. Text mode unchanged. `objectName`s from an `idPrefix` property: `.input`, `.send`, `.outOfRange`. |
| `app/qml/Widgets/Dashboard/Output/DashboardSlider.qml`, `DashboardKnob.qml` | `Keys.onPressed` (arrows, Shift x10) and a focus-gated `WheelHandler`, both writing `model.currentValue = model.stepped(...)` and accepting the event, so the control's `value` binding to `desiredValue` is never broken; value label through `model.formatted()` plus units; `objectName` `.slider` / `.knob`. |
| `app/qml/Widgets/Dashboard/Output/DashboardOutputPanel.qml` | Cell `objectName: "dashboard.output." + owData.groupId + "." + owData.widgetId` and `idPrefix` passed to the text field, slider and knob components. |
| `core/Ui/ProjectEditor/EditorForms/OutputValueRows.h/.cpp` | **New**, donor `OutputStateRows.{h,cpp}`. `appendOutputValueRows(model, widget)`: the Value Range header, an Input row (Text / Number) for a text field, then Min, Max, Step, Decimals, Units for every numeric control and Quantize to Step for a Number field. Takes the body of `EditorForms::buildOutputWidgetValueRows` (`EditorForms.cpp:1302-1344`), which shrinks to one call: `EditorForms.cpp` is at 1486 lines. |
| `core/Ui/ProjectEditor/EditorForms.cpp` | `buildOutputWidgetValueRows` delegates (net about -40 lines). Both callers (`EditorForms.cpp:1362`, `EditorMultiSelect.cpp:305`) keep calling the member. |
| `core/Ui/ProjectEditor/ProjectEditorItemIds.h` | Append `kOutputWidget_InputKind`, `_Decimals`, `_Units`, `_Quantize` after `kOutputWidget_StateConfirmMs` (`:118`). |
| `core/Ui/ProjectEditor/EditorCommit.cpp` | Four cases in `applyOutputWidgetField` (`:1168`); rebuild the form when `InputKind` changes (beside the `Checkable` / `StateSource` rebuild at `:1257`). |
| `core/Ui/ProjectEditor/EditorMultiSelect.cpp` | The four new fields in `outputWidgetEditValues` (`:243-261`) so a mixed selection shows "Mixed". |
| `core/Ui/CMakeLists.txt` | Add `OutputValueRows.{h,cpp}` beside `OutputStateRows` (`:316`, `:341`). |
| `core/Api/API/Handlers/ProjectOutputWidgetCommands.cpp` | `outputWidgetGet` (`:216-277`) returns `inputKind`, `decimals`, `units`, `quantize`; description names them (R6, AC2). |
| `core/Api/API/Handlers/ProjectUpdateCommands.cpp` | `applyOutputWidgetFields` (`:183-242`) takes the four keys before its `normalize()`; description (`:351-373`) names them. |
| `app/rcc/scripts/output/pwm_control.js`, `pid_setpoint.js` | Drop the `Math.max/Math.min` clamps; a comment points at the control's Min and Max (Decision 7). |
| `doc/help/Output-Controls.md` | Text Field gains its Number kind and the refusal; Slider/Knob tables gain Decimals and Units; a short Limits section; PWM and PID examples lose their clamps (R8, AC5). |
| `doc/help/API-Reference.md` | Output-widget get/update entries list the four fields. |
| `app/rcc/ai/skills/output_widgets.md`, `app/rcc/ai/docs/output_widget_js.md` | The assistant learns the Number kind, that the control enforces the range before `transmit()`, and to read `project.outputWidget.get` before driving. |
| `doc/claude/architecture/dashboard.md` | "Output Widgets (Pro)" (`:578`) gains two sentences: where admission happens and that a second per-control clamp is a defect. |
| `app/tests/tst_output_write_limits.cpp` + `app/tests/CMakeLists.txt` | **New** ctest, donor `tst_output_state_binding`, links `SerialStudio::Core` only. |
| `app/tests/tst_frame_serialization.cpp` | `outputWidgetRoundTripsEveryField` covers the four fields; a new case pins that defaults are omitted. |
| `tests/utils/device_simulator.py` | Opt-in capture of bytes the app writes (the watcher only peeks today, `device_simulator.py:152-182`); off by default so no existing test changes. |
| `tests/integration/test_output_write_limits.py` | **New**: AC1 (gui marker), AC2, AC3, AC5 and the R3 structural scan. |
| `app/rcc/api/api-schema.json`, `doc/grpc/serialstudio-typed.proto` | Not hand-edited: the two descriptions changed, so the maintainer re-dumps the schema after a build and reruns `generate-sdk.py` (project.md "Generated API Surfaces"). |

## Architecture & data flow

**Admission.** `sendValue(value)` becomes: license gate, license token check (both unchanged,
`Base.cpp:343-349`), then `admit(value)`, then pacing and `deliverValue` as today. `admit` asks
`outputIsNumeric(config)`; a non-numeric control (button, toggle, Text field) passes through
untouched. A numeric control calls `DataModel::admitOutputValue(config, value, out)`, which returns
`Accepted`, `NotANumber` or `OutOfRange`. On `Accepted` the double (quantized to Min + k x Step
when Quantize is on) replaces the value and `rangeMessage` clears. Otherwise `rangeMessage` is set
("Allowed range: 0 to 100 %" or "Enter a number from 0 to 100 %") and `sendValue` returns before
pacing: no held value is replaced, no script runs, `noteOperatorRequest` is not called, nothing
is written. The refusal touches no project or checkpoint path.

**Why there, and only there.** `deliverValue` is reached from `sendValue` and `flushPendingValue`
only, and `flushPendingValue` only flushes what `sendValue` admitted, so a control kind added later
inherits the check by calling `sendValue`. A pytest source scan pins that. The slider's existing
`qBound` (`Slider.cpp:53`) stays, because the spec's non-goal keeps slider and knob clamping; it
is geometry (the control cannot show an out-of-range value), and the admission behind it never
fires for them.

**Value types.** A Number field now hands `transmit()` a JS number; a Text field still hands the
typed string (R1, R7). Slider and knob are unchanged.

**Step.** `Base::stepped(from, steps)` returns `qBound(min, from + steps x step, max)`. Each of the
three numeric QML controls maps Up/Right to +1 and Down/Left to -1, Shift multiplies by ten, and a
`WheelHandler` active only while the control has focus maps a notch to one step. Slider and knob
write the model setter and accept the event: the built-in key handling would assign the
control's `value` and break its binding to `desiredValue`, which is today's quiet reason a
keyboard-nudged slider stops following spec-0080 feedback. The Number field nudges its text and
sends on Enter (Decision 5).

**Driving surface.** The cell carries `dashboard.output.<groupId>.<widgetId>` (the same pair
`project.outputWidget.get` takes); the field, send button and range label append `.input`,
`.send`, `.outOfRange`. The preview leaves `idPrefix` empty and therefore names nothing. Typing
through `ui.automation.key` lands in the focused field and Enter fires `onAccepted`, which is the
path above.

## Hotpath & threading impact

- **Touches the hotpath?** **No.** No pipeline, `FrameReader`, `CircularBuffer`, `FrameBuilder` or
  dashboard-ingest code changes. Admission runs on the GUI thread inside an operator action, at
  interaction rate.
- **New cross-thread signal/slot?** **No.** Everything added lives on the GUI thread.
- **New input to a cached hotpath flag?** **No.**
- **Timestamp ownership** — unchanged; nothing here stamps or reads sample time.

## Data model & persistence

Four additive `OutputWidget` fields, each written only when it differs from its default:
`inputKind` (Text), `outputDecimals` (-1, automatic), `outputUnits` (empty), `quantizeToStep`
(false). A pre-spec project therefore reads back with every text field as Text and serialises
byte-identical (R7, AC3). `normalize()` (`Frame.cpp:211-244`), which both doors into the model
already call, clamps the kind and the decimals. Automatic decimals reproduce today's slider rule
(`step < 1 ? 2 : 0`, `DashboardSlider.qml:100`), so an existing slider renders exactly as before.
No schema or writer version bump: older builds ignore the keys.

## API / SDK surface

No new command. `project.outputWidget.get` reports `inputKind`, `decimals`, `units`, `quantize`
next to the `type`, `minValue`, `maxValue`, `stepSize` it already returns (AC2);
`project.outputWidget.update` accepts them. Capability classes are unchanged:
`argumentsAddCapabilities` grants `InstallsScript` to `outputWidget.update` only for
`transmitFunction` (`CommandCapabilities.cpp:639-640`), and the sealed policy already denies the
whole `project.outputWidget.*` family (`kDenied`, `SealedPolicy.cpp:325-330`). Integers, not slugs, for the
kind, matching `type` and `stateSource` (no `EnumLabels` entry exists for either).

## QML / UI

The Number field reads like the slider beside it: the same accent, units suffix, the range label
in the alarm colour under the field, no dialog. The Project Editor's Value Range section now
appears for text fields too, with the Input row first so switching to Number reveals the range
rows on the rebuild that `EditorCommit` already performs for `Checkable`. The range rows read
Min, Max, Step, Decimals, Units (Decision 2), the same words the manual tables use.

## Tradeoffs & alternatives considered

Named candidates before converging:
- **Gate-in-Base** (chosen): one check in `sendValue`, one pure rule in `Frame.cpp`.
- **Clamp-per-control**: each control bounds its own value, as the slider does. Rejected: it is
  exactly the "second clamp" the spec calls a defect, and a new control kind would forget it.
- **Script-side limits**: pass Min/Max into `transmit()` and let the script refuse. Violates the
  assumed constraint that the limit is visible to editor, preview and API; checking whether it
  binds: it does, because R5 and R6 need the limit outside the script.
- **NumberField type**: a sixth `OutputWidgetType` instead of an input kind. Rejected: it widens
  the type enum every switch, schema text and test fixture know, for no behaviour the kind lacks.

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Where the rule lives | (a) `admitOutputValue` in `Frame.cpp`; (b) a new `ValueGate` class beside `StateBinding`; (c) private in `Base` | **(a)**: beside `normalize`, testable with `SerialStudio::Core` alone, reusable by spec 0111; no new class. (c) is untestable without a QJSEngine and the license guards. |
| Refuse or clamp a typed value | refuse / clamp | **Refuse**, decided by the maintainer (R2). |
| Number field after a send | clear like Text / keep the admitted value | **Keep**, formatted: it shows what was sent after quantizing and is the base the arrows nudge from. Text keeps clearing (R7). |
| Key handling | built-in `Slider` keys / own `Keys` handler | **Own handler**: built-in keys break the `value` binding, and they cannot do x10. |
| Widget map | two inline copies / `Base::presentation()` | **One builder**: both copies must change anyway. |
| Validator | `DoubleValidator` bounded to Min/Max / none | **None**: a bounded validator swallows Enter, so the operator would get silence instead of the range. |

## Risks & mitigations

- **Silent refusal.** A refusal the operator cannot see is the failure this spec removes. The label
  is inside the shared component, so the preview shows it too, and AC1 asserts it on screen.
- **Wheel actuation while scrolling.** A slider in a scrolling panel that takes the wheel moves a
  setpoint when the operator meant to scroll. The wheel acts only on a focused control.
- **Frame.h over the TU line.** Moving the `serialize` body out pays for the new fields; the
  census ratchet catches a miscount.
- **Language-dependent ids.** `objectName`s are literals joined with `groupId`/`widgetId`, which
  `qml-objectname-literal` accepts.
- **Limits are project data.** Any API client that can edit the project can widen Min/Max without
  a consent prompt, as it can every other field today. The limit protects the operator from a
  typo, not the device from a hostile client; the raw write stays behind its own consent.
- **Preview drift.** One `presentation()` builder for both maps.

## Test & verification plan

- **Unit (ctest, runnable here against an existing build):** `tst_output_write_limits`:
  in-range accepted, below and above refused, `"abc"`, `"inf"`, `"nan"` refused, quantize from
  Min (Min 0, Step 5: 43 to 45, 42 to 40), quantize off leaves 43, quantize then range (Max 100:
  103 quantizes to 105 and is refused), `outputIsNumeric` per type and kind.
  `tst_frame_serialization`: new fields round-trip; defaults omitted (AC3 unit half).
- **Integration (maintainer runs, API server on, Pro build):**
  `tests/integration/test_output_write_limits.py`:
  - **AC1** (`gui` marker): loopback network Device with capture, a Number field 0 to 100, Step 5,
    Quantize on, transmit `"SP " + v + "\n"`. Click `.input`, key `250`, Enter: the
    `.outOfRange` element is listed and no byte arrives in 500 ms. Key `43`, Enter: exactly one
    payload, `SP 45`.
  - **AC2**: `project.outputWidget.get` on the loaded project returns `type` 3, `inputKind` 1,
    `minValue`, `maxValue`, `stepSize`, `units`.
  - **AC3**: a Text field and a Number field round-trip through save and load; a pre-spec fixture
    project's output-widget objects are byte-identical after a save.
  - **AC5** (static, no app): no transmit example in `Output-Controls.md` and neither template
    clamps with `Math.max(..., Math.min(...))`.
  - **R3** (static, no app): `deliverValue(` is called only from `sendValue` and
    `flushPendingValue`, and the admission precedes pacing inside `sendValue`.
- **Maintainer observation:** AC4 (typing 250 shows the range, nothing reaches the device; arrows
  nudge the slider by one Step, Shift by ten).
- **Hotpath:** not touched; no `--benchmark-hotpath` run required.
- **Static:** `scripts/syntax-check.py` on every edited C++ file (new TU through a same-library
  donor's compile entry); `code-verify.py --check`, `--tu-census --check`,
  `--singleton-census --check` (no new `instance()` reach is planned); `layer-verify.py`;
  `term-verify.py`; `qt-cpp-review` and `qt-qml-review` before handoff; `sanitize-commit.py`
  before commit.

## Cross-spec touchpoints (0109 merges first, 0110 second)

| Shared file | 0109 owns | 0110 adds on top |
|-------------|-----------|------------------|
| `Frame.h` / `Frame.cpp` / `FrameKeys.h` | The `serialize` move, the four fields and keys, `outputIsNumeric`, `admitOutputValue` | Three reply fields and keys, `outputExpectsReply`, writing `stateConfirmMs` when a reply is bound |
| `Base.h/.cpp` | Admission, `rangeMessage`, `stepped`, `formatted`, `presentation` | Token, `transmit(value, token)`, reply outcome, the expiry timer, `automationState` (which carries 0109's `outOfRange`) |
| `Panel.cpp`, `Preview.cpp` | Switch to `presentation()` | Reply reads per tick and token reset (`Panel`); clear the reply binding (`Preview`) |
| `DashboardOutputPanel.qml` | Cell `objectName`, `idPrefix` plumbing | Outcome readout, `automationState` on the cell, `idPrefix` to button and toggle |
| `ProjectEditorItemIds.h`, `EditorCommit.cpp` | Four ids appended, four cases | Three ids appended after 0109's, three cases |
| `ProjectOutputWidgetCommands.cpp`, `ProjectUpdateCommands.cpp` | Four fields in get and update | Three reply fields |
| `tests/utils/device_simulator.py` | Opt-in capture | Reuses it to play firmware |
| `tst_frame_serialization.cpp`, `Output-Controls.md`, `API-Reference.md`, AI corpus, `dashboard.md` | Their own sections | Their own sections |

Merge conflicts are confined to append points (enum tail, switch cases, map keys, doc sections);
0110 rebases onto 0109 before implementation starts.

## Decisions (approved 2026-11-03, maintainer)

All eight open questions closed on the recommended answer; R1, R2, R4 and R8 of `spec.md` were
amended to match.

1. **Name.** "Out of range" in UI and API (`outOfRange`); "refused" stays spec 0110's device
   outcome.
2. **Editor labels.** The output-control rows read Min, Max, Step (dataset form untouched); the
   maintainer regenerates translations, since `.ts` files are not edited by hand.
3. **Decimals.** Display only, default automatic.
4. **Quantize origin.** Multiples of Step counted from Min.
5. **Number field arrows.** Edit the text; Enter sends.
6. **Wheel.** Focused control only, for all three.
7. **Bundled templates.** `pwm_control.js` and `pid_setpoint.js` lose their clamps with the manual.
8. **Modifier for x10.** Shift.
