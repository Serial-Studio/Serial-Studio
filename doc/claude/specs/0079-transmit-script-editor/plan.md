---
spec: 0079-transmit-script-editor
phase: plan
status: approved     # draft -> approved (gate before /ss-tasks)
updated: 2026-09-10
---

# Plan 0079 — Transmit Script Editor

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Read the relevant `doc/claude/` sub-docs and the *actual code*
> before writing this — a plan grounded in a stale mental model is worse than no plan.
> Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

The transmit editor becomes a modal QML window loaded on demand, following the canvas code
editor exactly: a `Loader` in the Output Widget view whose `active` flag is the window's
lifetime, hosting the existing embedded editor item. Two pieces of shared logic get lifted out
of the places that own them today. The transmit script's *verdict* moves from inside the
assistant's dry-run handler into a Pipeline-layer checker that the editor and the handler both
call, so one script can never get two answers. The transmit *delivery* moves out of
`Output::Base` into a small policy struct injected at construction: the dashboard panel builds
one that writes to the connection manager, the preview builds one that captures bytes and paces
nothing. That injection is what makes the preview structurally unable to transmit — the preview's
object graph never contains a connection manager, rather than containing one it promises not to
call. The preview then instantiates the real `Button`/`Slider`/`Toggle`/`TextField` model objects
and reuses the shipped dashboard control QML verbatim, which is the only way R8's "the exact
payload" can be true.

## Affected subsystems & files

| File | Change |
|------|--------|
| `core/Pipeline/DataModel/Scripting/TransmitScriptCheck.h/.cpp` | **New.** One verdict for a transmit script: `Ok`, `CompileError` (message + line), `NoEntryPoint`. Wraps the code in the same IIFE the runtime uses and runs it in a `ScriptDryRun` budgeted engine. Source-agnostic: validity does not depend on the target. |
| `core/Pipeline/DataModel/Scripting/TransmitScriptEnvironment.h/.cpp` | **New (added during T3).** The one definition of the host surface a transmit script runs against, so the live control, the editor and the dry run cannot drift apart. Its own TU so the checker's unit test does not have to link the script API. |
| `core/Ui/UI/Widgets/Output/TransmitTarget.h` | **New.** Header-only policy struct: a delivery callable plus a minimum send interval. Live and preview build different ones; `Base` holds one and knows nothing about either destination. |
| `core/Ui/UI/Widgets/Output/Preview.h/.cpp` | **New.** `Output::Preview`, a `QQuickItem` that exposes one widget's `widget` map and `model` object the way `Panel` exposes lists of them, rebuilds them when the config or script changes, and reports each produced payload. Holds no connection manager. |
| `core/Ui/UI/Widgets/Output/Base.h/.cpp` | Take a `TransmitTarget` at construction; `sendValue()` applies the target's interval, keeps the license gate, evaluates, and hands the payload to the target. Drops the `API::handlerContext()` reach from the constructor. |
| `core/Ui/UI/Widgets/Output/Button.h/.cpp` | Forward the target through the constructor. |
| `core/Ui/UI/Widgets/Output/Slider.h/.cpp` | Forward the target through the constructor. |
| `core/Ui/UI/Widgets/Output/Toggle.h/.cpp` | Forward the target through the constructor. |
| `core/Ui/UI/Widgets/Output/TextField.h/.cpp` | Forward the target through the constructor. |
| `core/Ui/UI/Widgets/Output/Panel.cpp` | Build the live `TransmitTarget` (connection-manager write, current pacing) and pass it to each model. |
| `core/Ui/ProjectEditor/Editors/OutputCodeEditor.h/.cpp` | Expose template names and an apply-by-index slot for the dropdown; expose the live validity verdict; add `commit()`; make the live write conditional on validity. |
| `core/Api/API/Handlers/ProjectDryRunCommands.cpp` | `outputWidgetDryRun` calls the shared checker for the compile/entry-point verdict instead of its own inline copy; its response shape is unchanged. |
| `app/qml/ProjectEditor/Dialogs/TransmitCodeDialog.qml` | **New.** The modal window: header with template dropdown, Import, Validate, Test; the editor; the preview pane; the validity strip. |
| `app/qml/ProjectEditor/Views/OutputWidgetView.qml` | Remove the inline code header and editor; wrap the parameter table in a `ScrollView` that fills the pane; add the `Loader` and the Edit Code button. |
| `app/src/Misc/ModuleManager.cpp` | Register `Output::Preview` as a QML type alongside the existing `OutputCodeEditor` registration. |
| `core/Ui/CMakeLists.txt` | Add `Preview.cpp`. |
| `core/Pipeline/CMakeLists.txt` | Add `TransmitScriptCheck.cpp`. |
| `app/CMakeLists.txt` | Register `TransmitCodeDialog.qml`. |
| `app/tests/tst_transmit_script_check.cpp` | **New.** ctest unit over the shared verdict: valid script, syntax error with line, compiles-but-no-entry-point, empty script. |
| `app/tests/CMakeLists.txt` | Register the new unit target. |
| `tests/integration/test_output_widget_editor.py` | **New.** AC3, AC4, AC7 over the API server. |
| `doc/help/Output-Controls.md` | The manual describes configuring the transmit function in the tree view and already promises a "template dropdown" that does not exist yet; both become accurate. |
| `doc/claude/architecture/scripting.md` | "Embedded Code Editors" describes all four Project-Editor hosts as living in retained views. The transmit host no longer does; the render-gate discussion needs that exception. |

## Architecture & data flow

**Opening.** The Output Widget view holds a `Loader` with `active: false`, exactly as
`GroupView.qml` does for the canvas editor. The Edit Code button sets `active: true` and calls
`showDialog()`; the window's `closing` handler sets `active: false`, which destroys the window
and with it the embedded editor item — this is what makes R13 true by construction rather than
by a render gate.

**Editing and validity.** `OutputCodeEditor` keeps its existing document wiring. On every
`textChanged` it now asks `TransmitScriptCheck` for a verdict and republishes it as a property
the window's status strip binds to. The verdict, not the text, gates the write: a valid script
is written to the project model exactly as today; an invalid one leaves the stored script at its
last valid value. That keeps R6 and R14 without introducing a staging buffer — the project always
holds the last script that compiled.

**Closing with an invalid script.** The window refuses to close silently. The user is told which
of the two failures blocked the save and offered Fix (stay) or Discard (close, project keeps the
last valid script). This is the only path that can lose typed text, and it is explicit.

**Preview.** `Output::Preview` owns a `DataModel::OutputWidget` config assembled from the widget
under edit plus the script currently in the editor. It builds the same concrete model class
`Panel` would build for that type, constructed with a capture `TransmitTarget`: delivery appends
to the preview's byte buffer and emits `payloadProduced`, interval is zero so every interaction
reports (R16). The engine is installed with the widget's real source id (R17), matching
`Base`'s constructor rather than the dry-run path's zero. When the script becomes valid again the
preview rebuilds its model; while it is invalid the preview keeps the last model and marks its
byte view stale (R10). The preview QML switches on widget type and instantiates the shipped
`DashboardButton` / `DashboardSlider` / `DashboardToggle` / `DashboardTextField` components,
binding `model` to the preview's model object — the same binding `DashboardOutputPanel.qml` makes.

**Why the license gate stays in the preview.** `Base::sendValue()` checks the commercial token
before evaluating. The preview goes through the same method, so an unlicensed build cannot use
the preview to exercise transmit behavior it otherwise cannot reach — the spec constraint.

## Hotpath & threading impact

> **CORRECTED 2026-09-10 after review — the answer below was wrong.** `prepareTransmitScriptEngine`
> reaches `ScriptApiCall::installAll` -> `FrameBuilder::injectTableApiJS`, which (a) blocking-marshals
> to the pipeline thread through a nested `QEventLoop` (`runOnObjectThread`, which CLAUDE.md reserves
> for command-rate waits and forbids for a repeating GUI-thread script path) and (b) latches
> `m_externalTableApiUsers` + `TableSnapshotChannel::noteGuiUser()`, turning on per-dataset capture
> and per-tick snapshot publishing for the rest of the session. Three agents found this
> independently. The editor validates on a 300 ms debounce and the preview rebuild pays it a second
> time, so this is a repeating path, not a one-shot.

- **Touches the hotpath?** ~~**No.**~~ **Yes, indirectly — see the correction above.** Nothing here runs on `FrameReader`, `CircularBuffer`,
  `FrameBuilder`, the span fast lane, or the dashboard draw path. Output widgets are GUI-thread
  objects driven by user interaction; the transmit script runs synchronously on the GUI thread
  under the existing 500 ms watchdog, unchanged.
- **New cross-thread signal/slot?** **No.** Every object added here lives on the GUI thread. The
  preview's payload signal is a direct GUI-thread emission.
- **New input to a cached hotpath flag?** **No.** No change to `m_operationMode`,
  `m_anyAsyncSink`, `m_captureLatestFrame`, `m_changeDriven`, or `m_streamAvailable`.
- **Timestamp ownership** — not applicable; the preview produces no frames and stamps nothing.

One performance note that is *not* hotpath but is real: R16 evaluates the script per interaction
step, so a slider drag runs `transmit()` per step on the GUI thread. The existing watchdog bounds
a single call at 500 ms, which is far too coarse for a drag. The preview therefore collapses
pending evaluations to the latest value per event-loop turn rather than queueing one per step —
the user still sees every value the control settles on, without a backlog of stale evaluations.

## Data model & persistence

No schema change. The transmit function is an existing field on the output-widget structure, and
this feature only changes *when* it is written, never its shape or its key. No `Frame.h` `Keys::`
addition, no writer-version bump, no migration: a project written before this change and one
written after are byte-identical for the same script. R14 is satisfied because loading never
consults the checker.

## API / SDK surface

No new handler, no new command, no `EnumLabels` slug, no generated-SDK change. The only API-side
edit is internal: `outputWidgetDryRun` delegates its compile/entry-point verdict to the shared
checker. Its request parameters and response fields (`ok`, `compileError`, `line`, `output*`,
`byteCount`) stay exactly as they are, which AC7 asserts. Per spec R6 the API keeps accepting an
invalid transmit script; the gate is the editor's, not the protocol's.

## QML / UI

`TransmitCodeDialog.qml` is a `Window` with `flags: Qt.Dialog`, `modality: Qt.ApplicationModal`
(R15), sized like the canvas editor. Layout top to bottom: a header row carrying the template
`ComboBox`, Import, Validate and Test; a horizontal split with the code editor on the left and the
preview pane on the right; a validity strip; a close row. The template combo needs the usual
restore-race guard — it must not fire its `onCurrentIndexChanged` while being populated, or
opening the window would overwrite the user's script with a template.

`OutputWidgetView.qml` loses its code header and editor and gains the `ScrollView` wrapper that
`DatasetView.qml` and `GroupView.qml` already use for their parameter tables, plus the Loader and
the Edit Code button in the existing toolbar's right-hand group.

The preview pane reuses the four dashboard control components unmodified. They are not copied or
re-implemented; the preview binds the same `model` property they already expect.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| How the preview is prevented from transmitting | (a) injected delivery policy; (b) a `preview` flag on `Base`; (c) a separate preview model class duplicating the QML surface | **(a)** — the spec asks for structural impossibility, and only (a) removes the connection manager from the preview's object graph. (b) is a flag someone can flip; (c) drifts from the real control and breaks R8. |
| Where the validity verdict lives | (a) shared Pipeline-layer checker; (b) duplicate the logic in the editor | **(a)** — the spec makes "editor and dry-run agree" a constraint, and two copies of a three-way verdict is precisely how they stop agreeing. |
| What happens to invalid text on close | (a) prompt Fix / Discard; (b) close and silently keep the last valid script; (c) persist the invalid script and raise a Problems entry | **(a)** — (b) loses typed work with no signal, and (c) contradicts R6. The prompt is the only option where the user chooses. |
| Save cadence | (a) keep live-save, gated on validity; (b) stage edits and write on apply | **(a)** — live-save already works and modality (R15) removes the ambiguity that made staging attractive. (b) would add a buffer whose only job is to be discarded. |
| Preview pacing implementation | (a) collapse pending evaluations per event-loop turn; (b) evaluate synchronously per step | **(a)** — R16 wants every settled value, not a backlog; (b) makes a drag as slow as the script. |
| Preview rebuild granularity | (a) rebuild the model when the script becomes valid; (b) recompile in place | **(a)** — the model compiles its script in its constructor today, and rebuilding is cheap at human edit rates. (b) would add a mutable path to a class whose properties are all `CONSTANT`. |

## Risks & mitigations

- **Working-tree collision — cleared 2026-09-10.** `Output/Button.h`, `Output/Button.cpp` and
  `Output/Panel.cpp` carry uncommitted work, but the maintainer has confirmed it is finished and
  this plan may edit them. They are still uncommitted, so the target-injection task should re-read
  each file before editing rather than working from this plan's snapshot of their shape.
- **ComboBox restore race.** Populating the template combo can fire a selection change that
  overwrites the user's script on open. This is the failure the QML sub-doc calls out; the combo
  is populated with signals blocked and the current index set explicitly afterward.
- **Embedded editor render cost.** The 2026-08-17 and 2026-08-18 incidents were both an embedded
  editor repainting when nobody was looking. The Loader-destroys-on-close design removes the item
  entirely, but AC8 verifies it by sampling rather than by assuming — that is the whole lesson of
  those two incidents.
- **License gate coverage.** The preview must keep the commercial-token check. A refactor of
  `sendValue()` that moves the gate below the evaluation, or into the live target only, would
  open exactly the hole the spec's constraint names.
- **The `handlerContext()` reach.** `Base`'s constructor currently reaches
  `API::handlerContext().connectionManager`. Removing it is a small win for the singleton census;
  it also means `Panel` becomes the one place that knows the destination, which is where the
  composition-root doctrine wants it.
- **Divergent source identity.** R17 has the preview use the widget's real source while the
  dry-run and sample-value test keep using zero. That divergence is now deliberate and recorded in
  the spec; a later reconciliation should change all three together, not one.

## Test & verification plan

- **Unit (runnable here):** `app/tests/tst_transmit_script_check.cpp` via `ctest` against an
  existing build — valid script returns `Ok`; a syntax error returns `CompileError` with a
  non-zero line; a script defining `send()` instead of `transmit()` returns `NoEntryPoint`; empty
  input returns `NoEntryPoint`. This is the check both AC4 and the dry-run delegation rest on.
- **Integration (maintainer runs the app, API server enabled):**
  `tests/integration/test_output_widget_editor.py` —
  **AC3** connect a loopback source, read the link's transmitted-byte counter, drive the preview,
  assert the counter is unchanged, then drive the dashboard widget and assert it moved;
  **AC4** write a syntax-error script and a no-entry-point script through the editor path and
  assert the stored transmit function is unchanged;
  **AC7** write a syntax-error script through `project.outputWidget.update` and assert it
  succeeds, proving the gate did not leak into the API.
- **Maintainer observation:** AC1 (view layout), AC2 (every template loads and validates), AC5
  (preview payload matches the dashboard payload for one scaling template), AC6 (stale marking and
  recovery), AC9 (a project with a broken stored script still loads and opens).
- **Sampling:** AC8 — `sample <pid>` after opening and closing the window, per the recipe in
  `common-mistakes.md` "Diagnosing a GUI Stall"; expect no editor render frames.
- **Hotpath:** not touched, so `--benchmark-hotpath` is not a gate for this change. It still runs
  in CI and must not regress.
- **Static:** `python scripts/code-verify.py --check` clean on every touched file, including the
  singleton census (which should improve, not grow); `python scripts/layer-verify.py` for the new
  Pipeline file; `qt-cpp-review` before handoff; `python scripts/sanitize-commit.py` before commit.
