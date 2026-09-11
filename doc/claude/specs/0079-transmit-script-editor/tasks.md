---
spec: 0079-transmit-script-editor
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-09-10
---

# Tasks 0079 — Transmit Script Editor

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

### T1 — Shared transmit-script checker

- **Files:** `core/Pipeline/DataModel/Scripting/TransmitScriptCheck.h/.cpp`,
  `core/Pipeline/DataModel/Scripting/TransmitScriptEnvironment.h/.cpp`, `core/Pipeline/CMakeLists.txt`
- **Does:** One verdict type for a transmit script — `Ok`, `CompileError` (message + line),
  `NoEntryPoint`, `Timeout` — produced by wrapping the code in a `typeof`-probing IIFE and running
  it in a budgeted `ScriptDryRun` engine. Source-agnostic: validity never depends on the target, so
  the checker takes no source id. *Amended 2026-09-10: `Timeout` was added during T1 — without it
  T3 cannot preserve the handler's existing `ScriptTimeout` response, which its invariant freezes.
  The verdict carries only the engine's own error text; human wording stays with each caller, so
  the assistant keeps its long corrective paragraph and the editor gets a short translated line.*
- **Verify:** `python scripts/code-verify.py --check core/Pipeline/DataModel/Scripting/TransmitScriptCheck.h core/Pipeline/DataModel/Scripting/TransmitScriptCheck.cpp`; `python scripts/layer-verify.py` (new Pipeline TU must not reach upward).
- **Deps:** none
- [x] done — code-verify 0/0, layer-verify clean

### T2 — Unit test for the checker

- **Files:** `app/tests/tst_transmit_script_check.cpp`, `app/tests/CMakeLists.txt`
- **Does:** ctest cases pinning each verdict: a valid script returns `Ok`; a syntax error returns
  `CompileError` with a non-zero line; a script defining `send()` instead of `transmit()` returns
  `NoEntryPoint`; empty input returns `NoEntryPoint`; a top-level infinite loop returns `Timeout`.
  This is the test AC4 and the T3 delegation both rest on.
- **Verify:** `ctest -R transmit_script_check` against an existing build dir.
- **Deps:** T1
- [x] done — code-verify 0/0. **Not executed:** the available build dir has tests unconfigured, so
  the maintainer must run `ctest -R transmit_script_check` once built.

### T3 — Dry-run handler delegates to the checker

- **Files:** `core/Api/API/Handlers/ProjectDryRunCommands.cpp`
- **Does:** `outputWidgetDryRun` obtains its compile / entry-point verdict from T1 instead of its
  own inline copy. **Invariant:** the response shape is frozen — `ok`, `compileError`, `line`, the
  output fields and `byteCount` keep their exact names and semantics, because AC7 and the
  assistant's existing workflow both read them.
- **Verify:** `python scripts/code-verify.py --check core/Api/API/Handlers/ProjectDryRunCommands.cpp`; read back the handler's response keys against the pre-change list.
- **Deps:** T1
- [x] done — code-verify 0/0; response keys, messages and the ScriptTimeout branch unchanged.
  **Resolved:** the environments were never as divergent as they looked — `ScriptApiCall::installAll`
  already injects the table API (`ScriptApiCall.cpp:620`), so the handler's explicit
  `injectTableApiJS` was redundant in commercial builds. Both callers now prepare through
  `prepareTransmitScriptEngine`, and a `compileTransmitScript` overload lets the handler judge and
  execute the same compiled function through one wrapper definition. The GPL build keeps its
  standalone `injectTableApiJS` so its behavior is unchanged.

### T4 — Transmit target policy

- **Files:** `core/Ui/UI/Widgets/Output/TransmitTarget.h`
- **Does:** Header-only struct carrying a delivery callable and a minimum send interval. It is the
  seam that lets one widget class serve both a device and a capture buffer without knowing which.
- **Verify:** `python scripts/code-verify.py --check core/Ui/UI/Widgets/Output/TransmitTarget.h`
- **Deps:** none
- [x] done — code-verify 0/0; carries `kLiveSendIntervalMs` so Panel can build the live pacing.

### T5 — Base accepts an injected target

- **Files:** `core/Ui/UI/Widgets/Output/Base.h/.cpp`
- **Does:** Construct from a `TransmitTarget`; `sendValue()` takes its pacing from the target and
  hands the payload to it. **Invariants:** the commercial-token gate stays *before* evaluation, or
  the preview becomes a way to run transmit code an unlicensed build cannot otherwise reach; the
  constructor must stop reaching `API::handlerContext()`, so `code-verify --singleton-census` must
  shrink or hold, never grow.
- **Verify:** `python scripts/code-verify.py --check core/Ui/UI/Widgets/Output/Base.h core/Ui/UI/Widgets/Output/Base.cpp`; `python scripts/code-verify.py --singleton-census --check`.
- **Deps:** T4
- [x] done — code-verify 0/0; census holds at 795. Base now prepares its engine through
  `prepareTransmitScriptEngine`, so the live control and the checker share one environment
  definition; `installProtocolHelpers` routes through it too.

### T6 — Concrete widgets forward the target

- **Files:** `core/Ui/UI/Widgets/Output/Button.h/.cpp`, `Slider.h/.cpp`, `Toggle.h/.cpp`,
  `TextField.h/.cpp`
- **Does:** Each constructor takes the target and passes it to `Base`. Eight files but one
  mechanical edit repeated four times, so it stays one reviewable diff rather than four
  near-identical ones. `Button.h/.cpp` carry uncommitted work — re-read them before editing.
- **Verify:** `python scripts/code-verify.py --check` on the eight files.
- **Deps:** T5
- [x] done — code-verify 0/0 on all eight.

### T7 — Panel builds the live target

- **Files:** `core/Ui/UI/Widgets/Output/Panel.cpp`
- **Does:** Build the device target once (connection-manager write, the interval `Base` used to
  hardcode) and pass it to every model it constructs. **Invariant:** this becomes the single place
  that knows where a payload goes; nothing below it may reacquire the connection manager. The file
  carries uncommitted work — re-read it before editing.
- **Verify:** `python scripts/code-verify.py --check core/Ui/UI/Widgets/Output/Panel.cpp`. **Checkpoint:** after this task the dashboard must behave exactly as before — same bytes, same pacing, same license behavior. Confirm on a real project before continuing.
- **Deps:** T5, T6
- [x] done — code-verify 0/0. Structural proof of the spec's deciding constraint: `ConnectionManager`
  and `handlerContext` now appear in **no** widget header and in none of Base/Button/Slider/Toggle/
  TextField — only `Panel.cpp` resolves a destination. **Checkpoint not confirmed:** requires a
  build and a real project, which is the maintainer's to run.

### T8 — Output widget preview host

- **Files:** `core/Ui/UI/Widgets/Output/Preview.h/.cpp`, `core/Ui/CMakeLists.txt`
- **Does:** A `QQuickItem` exposing one widget's config map and model object the way `Panel`
  exposes lists of them, rebuilt when the script becomes valid, keeping the previous model and a
  stale marker while it is not (R10). **Invariants:** the preview's object graph contains no
  connection manager — that is the spec's deciding constraint, not a style preference; its target
  interval is zero so every interaction reports (R16); its engine installs helpers with the
  widget's real source id, matching the live path rather than the dry-run path's zero (R17).
- **Verify:** `python scripts/code-verify.py --check core/Ui/UI/Widgets/Output/Preview.h core/Ui/UI/Widgets/Output/Preview.cpp`; grep the new TU for any connection-manager reference and confirm there is none.
- **Deps:** T5, T6
- [x] done — code-verify 0/0. `ConnectionManager`/`handlerContext` appear in no widget TU but Panel.cpp; capture target has minIntervalMs 0; binds from the editor selection so QML never rebuilds a config.

### T9 — Register the preview as a QML type

- **Files:** `app/src/Misc/ModuleManager.cpp`
- **Does:** `qmlRegisterType` for the preview beside the existing output-editor registration.
  **Invariant:** registration only — nothing is added to `instantiateCoreModules()`, whose
  construction order is pinned and whose edits re-trigger the ctor-edge proof.
- **Verify:** `python scripts/code-verify.py --check app/src/Misc/ModuleManager.cpp`; confirm `instantiateCoreModules()` is untouched in the diff.
- **Deps:** T8
- [x] done — code-verify 0/0; 2 insertions, `instantiateCoreModules()` untouched.

### T10 — Editor exposes its template catalog

- **Files:** `core/Ui/ProjectEditor/Editors/OutputCodeEditor.h/.cpp`
- **Does:** Publish the catalog's display names as a property and add an apply-by-index slot, so a
  QML dropdown can replace the modal picker. The existing picker slot stays until T12 removes its
  last caller.
- **Verify:** `python scripts/code-verify.py --check` on both files; read back that the property lists all fifteen catalog entries.
- **Deps:** none
- [x] done — code-verify 0/0; `templateNames` + `applyTemplate(index)` exposed.

### T11 — Editor validity, commit, and the gated write

- **Files:** `core/Ui/ProjectEditor/Editors/OutputCodeEditor.h/.cpp`
- **Does:** Republish T1's verdict as a bindable property recomputed on text change; add
  `commit()` to flush a pending IME composition; make the project write conditional on the verdict
  so an invalid script leaves the stored one at its last valid value (R6, R14). **Invariant:** the
  existing write guard — current view is the output-widget view and a widget is selected — stays
  exactly as it is; modality (R15) is what keeps it sufficient, so do not relax it.
- **Verify:** `python scripts/code-verify.py --check` on both files; type a syntax error and read back that the stored transmit function is unchanged.
- **Deps:** T1, T10
- [x] done — code-verify 0/0. Validation is debounced 300 ms — an engine per keystroke was far too costly — and `commit()` flushes the IME and validates immediately so nothing is left in the debounce at close.

### T12 — The transmit editor window

- **Files:** `app/qml/ProjectEditor/Dialogs/TransmitCodeDialog.qml`, `app/CMakeLists.txt`
- **Does:** Modal window with the template dropdown, Import, Validate and Test in its header, the
  editor and the preview pane side by side, the validity strip, and a close path offering Fix or
  Discard when the script is invalid. **Invariant:** populate the template combo with its signals
  blocked and set the current index explicitly afterward — an unguarded restore fires a selection
  change on open and overwrites the user's script with a template.
- **Verify:** `python scripts/code-verify.py --check` on the QML; open the window on a widget with a custom script and confirm the script is intact after the combo populates.
- **Deps:** T9, T10, T11
- [x] done — code-verify 0/0 after `--fix`. Two defects caught before handoff: `MessageDialog` had no import and no precedent here (now a Controls `Dialog`), and `selectedOutputWidget` is a C++ struct QML cannot read (preview now binds itself).

### T13 — Output Widget view rework

- **Files:** `app/qml/ProjectEditor/Views/OutputWidgetView.qml`
- **Does:** Remove the inline code header and editor; wrap the parameter table in the same
  `ScrollView` shape the dataset and group views use; add the Loader and the Edit Code button using
  the `editor/edit-code` icon. **Invariant:** the Loader goes inactive on close so the window and
  its editor item are destroyed — that is what makes R13 true, and it is the 2026-08-17 /
  2026-08-18 render-cost lesson applied by construction rather than by a gate.
- **Verify:** `python scripts/code-verify.py --check` on the QML; AC1 by observation.
- **Deps:** T12
- [x] done — code-verify 0/0 after `--fix`; no `OutputCodeEditor` remains in the view.

### T14 — Integration tests

- **Files:** `tests/integration/test_output_widget_editor.py`
- **Does:** AC3 (drive the preview with a source connected; the link's transmitted-byte counter
  must not move, while the dashboard widget moves it), AC4 (an invalid script leaves the stored
  function unchanged), AC7 (the same invalid script still writes successfully through the API,
  proving the gate did not leak out of the editor).
- **Verify:** `pytest tests/integration/test_output_widget_editor.py -v` with the app running and the API server enabled.
- **Deps:** T13
- [x] done — Source-level guards pass (`2 passed`, no app needed). API cases need the running app.

### T15 — Documentation

- **Files:** `doc/help/Output-Controls.md`, `doc/claude/architecture/scripting.md`
- **Does:** The manual currently tells the user to configure the transmit function from the tree
  view and already promises a template dropdown that did not exist; both become accurate, and the
  import button's new home is named. The scripting sub-doc describes all four Project-Editor
  editor hosts as living in retained views — the transmit host no longer does, and the render-gate
  discussion needs that exception recorded.
- **Verify:** `python scripts/claim-verify.py` (0 new); `python scripts/documentation-verify.py` on the help page.
- **Deps:** T13
- [x] done — documentation-verify 0 findings; claim-verify 0 new.

### T16 — Review fixes (added 2026-09-10 after `qt-cpp-review`)

- **Files:** `core/Pipeline/DataModel/Scripting/TransmitScriptEnvironment.{h,cpp}`,
  `TransmitScriptCheck.{h,cpp}`, `core/Pipeline/DataModel/Scripting/ScriptApiCall.{h,cpp}`,
  `core/Pipeline/DataModel/FrameBuilder.h`, `core/Ui/UI/Widgets/Output/{TransmitTarget.h,Base.cpp,
  Preview.h,Preview.cpp}`, `core/Ui/ProjectEditor/Editors/OutputCodeEditor.{h,cpp}`,
  `core/Api/API/Handlers/ProjectDryRunCommands.cpp`, both QML files
- **Does:** Six agents found the change did not meet R9 and that the plan's hotpath answer was
  wrong. Fixes: a `Judging` host surface that keeps every name but makes `deviceWrite`,
  `actionFire` and `apiCall` inert and installs the table API by name only (no pipeline marshal,
  no session-long capture flags); the surface carried on `TransmitTarget` so preview, validation,
  sample test and dry run all judge while only the dashboard acts; one shared wrapper so a script
  ending in a comment cannot pass validation then fail in the widget; `readCode()` no longer
  persists or validates inline; a re-entrancy latch on validation and on preview rebuild; the
  close path fixed so Cancel can cancel; payload reporting coalesced per event-loop turn; the
  constructor's compile watchdog-armed; `widgetId` bound so the preview renders at all; dead
  `selectTemplate()` removed.
- **Verify:** `code-verify --check` clean on all touched files; `layer-verify` clean; singleton
  census at baseline; worst-TU unchanged.
- **Deps:** T1-T15
- [x] done

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` is met and checked off there.
- [ ] `python scripts/code-verify.py --check` is clean on all changed files (no new errors, no new
      advisories in new code).
- [ ] `python scripts/code-verify.py --singleton-census --check` has not grown. It holds at the
      795 baseline rather than shrinking: `handlerContext()` is a root-bound module accessor,
      not an `::instance()` reach, so removing it was never going to move this counter.
- [ ] `python scripts/layer-verify.py` clean for the new Pipeline TU.
- [ ] `qt-cpp-review` run on the C++ diff; findings addressed or noted.
- [ ] Hotpath untouched, as `plan.md` states; `--benchmark-hotpath` still passes in CI.
- [ ] `pytest tests/integration/test_output_widget_editor.py` identified for the maintainer to run.
- [ ] `python scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [ ] Diff is *what was asked, and only that* — the file list above is the lane; anything else is
      raised in chat, not slipped in.
- [ ] `spec.md` status set to `done`.
