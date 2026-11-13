---
spec: 0120-rescan-defects
phase: tasks
updated: 2026-11-12
---

# Tasks 0120 — Defects from the 2026-11-12 quality rescan

> **The ordered checklist.** `/ss-implement` decomposes the Design in [`spec.md`](./spec.md)
> into units that are small, ordered, and *individually verifiable*, works the list top to
> bottom and keeps the boxes current. No approval gate: the spec was the gate.

## Conventions

- One task = one focused, reviewable change.
- **Verify** is `code-verify.py --check` plus `syntax-check.py` on every C++ file (new files
  borrow a donor compile entry; a SKIP means "it compiles" is unverified), plus the stated check.
- Nothing is committed unless the maintainer asks.

## Tasks

### T1 — Wait-box registry in `UserWaitScope`

- **Files:** `core/Core/Api/UserWaitScope.{h,cpp}`
- **Does:** Adds a GUI-thread registry for the one open script wait box: `registerCloser`,
  `clearCloser`, `boxOpen()`, `cancelOpenBox()`. Binding invariant: GUI thread only, so no lock;
  the existing thread-local wait grant is untouched.
- **Verify:** syntax-check via a Core donor; code-verify.
- **Deps:** none
- [x] done

### T2 — `MessageBoxGate` decisions and ctest

- **Files:** `core/Ui/ApiHandlers/MessageBoxGate.h` (new), `app/tests/tst_message_box_gate.cpp`
  (new), `app/tests/CMakeLists.txt`
- **Does:** Header-only decisions: refuse while busy, refuse a headless wait, refuse a wait
  without the grant, map a cancelled box to the cancel answer (AC1).
- **Verify:** syntax-check (donor + QtTest); code-verify.
- **Deps:** none
- [x] done

### T3 — `Utilities::showMessageBox` open hook

- **Files:** `core/Ui/Misc/Utilities.{h,cpp}`
- **Does:** Optional `onOpen(QMessageBox*)` callback invoked just before `exec()`. Binding
  invariant: every existing caller compiles and behaves unchanged (default argument).
- **Verify:** syntax-check `Utilities.cpp`; grep callers.
- **Deps:** none
- [x] done

### T4 — `WindowHandler::showMessageBox` rules

- **Files:** `core/Ui/ApiHandlers/WindowHandler.cpp`
- **Does:** One-at-a-time counter for posted and waiting boxes, fixed provenance line, no app
  icon, headless refusal for wait and a log line for post, registers the closer around the wait
  and answers cancel when closed by it. No command description or parameter change.
- **Verify:** syntax-check; code-verify; `generate-command-strings.py --check`.
- **Deps:** T1, T2, T3
- [x] done

### T5 — Interface-driving classification (protected)

- **Files:** `core/Api/API/CommandCapabilities.cpp`
- **Does:** Moves `ui.window.showMessageBox` from the plain list to `DrivesUi`. The sealed list
  and the safety tier are unchanged.
- **Verify:** syntax-check; read-back of the remote gate path.
- **Deps:** none
- [x] done

### T6 — `ScriptCommandPolicy` and ctest

- **Files:** `core/Api/API/ScriptCommandPolicy.h` (new), `app/tests/tst_script_command_policy.cpp`
  (new), `app/tests/CMakeLists.txt`
- **Does:** Pure refusal: command name, params and open project path in, refusal or nothing out
  (AC2).
- **Verify:** syntax-check (donor + QtTest); code-verify.
- **Deps:** none
- [x] done

### T7 — Script origin in `CommandHandler`

- **Files:** `core/Api/API/CommandHandler.{h,cpp}`
- **Does:** `CommandOrigin::Script`; `execute()` uses it; the script `DispatchPolicyScope` opens
  AFTER the trusted `RemoteDispatchScope`, whose policy shadow would otherwise blank it. Remote and
  trusted paths are unchanged.
- **Verify:** syntax-check; code-verify.
- **Deps:** T6
- [x] done

### T8 — Control script: stop at the waiting call, close on stop, quit

- **Files:** `core/Pipeline/DataModel/Scripting/ControlScriptWorker.{h,cpp}`,
  `core/Pipeline/DataModel/Scripting/ControlScript.{h,cpp}`, `app/src/Misc/ModuleManager.cpp` (only
  if `onQuit` latches)
- **Does:** Per-run stop flag raised by `stopWorker()`; the bridge throws into the engine after a
  dispatch that returns with it raised. `stopWorker()` and `shutdown()` cancel an open box;
  `shutdown()` defers the join while a box is on the stack. Binding invariants: no new
  `setInterrupted` site; the flag is release/acquire; the worker still never runs on the pipeline
  thread.
- **Verify:** syntax-check all; code-verify (`js-interrupt-site` stays clean).
- **Deps:** T1
- [x] done

### T9 — Macros: watchdog disarm, stop at the waiting call, close on stop

- **Files:** `core/Pipeline/DataModel/Scripting/MacroWorker.cpp`,
  `core/Pipeline/DataModel/Scripting/MacroRunner.cpp`
- **Does:** Disarms the watchdog before the blocking dispatch; throws (JS) or raises a Lua error
  after a dispatch that ends with stop requested; `stop()` cancels an open box.
- **Verify:** syntax-check; code-verify.
- **Deps:** T1
- [x] done

### T10 — Automation sees and closes the box

- **Files:** `core/Ui/UI/Automation/WindowInventory.{h,cpp}`,
  `core/Ui/ApiHandlers/AutomationHandler.cpp`
- **Does:** Lists the open modal box via `QGuiApplication::modalWindow()`, reports the main
  window modal while it is open, and routes an Escape key to it. Read
  `doc/claude/architecture/automation.md` first.
- **Verify:** syntax-check; code-verify.
- **Deps:** T4
- [x] done

### T11 — Tiny5 Mono on demand

- **Files:** `core/Ui/Misc/CommonFonts.{h,cpp}`, `core/Ui/Console/Handler.cpp`
- **Does:** Explicit family-to-file table; `ensureFamilyLoaded(family)`; no startup registration;
  the console merges the bundled families into its list and loads a family before the fixed-pitch
  check and on settings restore. Binding invariant: existing signal wiring on the console font
  properties unchanged.
- **Verify:** syntax-check; code-verify; a PySide6 probe that the five family names match the files.
- **Deps:** none
- [x] done

### T12 — Editor caption follows a multi-selection

- **Files:** `core/Ui/ProjectEditor/EditorMultiSelect.cpp`
- **Does:** Also emits `selectedTextChanged` when the batch changes inside the multi-selection view.
- **Verify:** syntax-check; code-verify.
- **Deps:** none
- [ ] done — reverted in T17: `MultiSelectionView.qml` reads neither `selectedText` nor
  `selectedIcon` and hard-codes the dataset icon, so the emission changed nothing. The real fix is
  a QML binding outside the file list, so the design goes back to the maintainer.

### T13 — Translation tool drops obsolete strings

- **Files:** `app/translations/translation_manager.py`, `scripts/tests/` (one case)
- **Does:** Passes `-no-obsolete` to `lupdate`; a static check pins it.
- **Verify:** `pytest scripts/tests -q`.
- **Deps:** none
- [x] done

### T14 — Recording-tail race, reruns, required services

- **Files:** `tests/integration/test_recording_fidelity.py`, `tests/utils/device_simulator.py`,
  `tests/conftest.py`, `tests/integration/conftest.py`, `.github/workflows/test-suite.yml`,
  `scripts/tests/` (one case)
- **Does:** The tail tests wait for their last frame before the edge; the simulator sets
  `TCP_NODELAY`; the two parse-budget names leave `_FLAKY_TESTS`; a missing broker or simulator
  fails when the leg requires it; the workflow sets the requirement only where it starts the
  service.
- **Verify:** `pytest scripts/tests -q`, `pytest tests/scripts -q`, collect-only on the touched
  integration files.
- **Deps:** none
- [x] done

### T15 — Message-box and script-policy integration tests

- **Files:** `tests/integration/test_message_box.py`, `tests/integration/test_script_command_policy.py`
  (new)
- **Does:** Static wiring checks (AC4), the `gui` case (AC5), and the control-script refusal test
  with the remote-unchanged check (AC3).
- **Verify:** collect-only; run against the live app if it is up.
- **Deps:** T4, T5, T7, T8
- [x] done

### T16 — Docs and TODO

- **Files:** `doc/claude/architecture/scripting.md`, `doc/help/API-Reference.md` (prose only),
  `doc/claude/TODO.md`
- **Does:** Which scripts may wait, what control-script-only means for launch, the new refusals;
  TODO gains the libplctag pin and the `.ts` regeneration step.
- **Verify:** `claim-verify.py --check`; `documentation-verify.py` on the help page.
- **Deps:** T4, T7, T9
- [x] done

### T17 — Review fixes (qt-cpp-review, 2026-11-12)

- **Files:** `ControlScriptWorker.{h,cpp}`, `ControlScript.cpp`, `MacroRunner.cpp`, `MacroWorker.cpp`,
  `UserWaitScope.{h,cpp}`, `WindowHandler.cpp`, `ScriptCommandPolicy.h`,
  `tst_script_command_policy.cpp`, `app/src/Misc/ModuleManager.cpp`, `EditorMultiSelect.cpp`
  (revert), `tests/integration/test_message_box.py`, `doc/claude/architecture/scripting.md`
- **Does:** A stop belongs to one run (the marshaller's `stopRun`/`nextRun`/`beginRun`), so a stop is
  never erased by the next start, a posted dispatch after it is refused, every later apiCall
  throws, `delay()` and `deviceWriteAndWait` abort, and a stop-induced error (compile, setup or
  loop) ends the run without a script error. The box closer names its waiting marshaller, so a stop
  closes only its own script's box. `onQuit`'s latched pass joins the worker the first pass left
  inside the box. The macro bridge re-arms before a stop throws. The modal loop shadows the
  script policy. The save policy checks the path the saver writes, case exact, and refuses
  `askPath` and a bare save with no project file. The R17 emission, a verified no-op, is
  reverted; the real fix is QML and goes back to the maintainer.
- **Verify:** syntax-check and code-verify; the static cases; the new non-gui regression
  `test_a_stop_mid_api_call_is_no_script_error`, run by the maintainer with a current build.
- **Deps:** T1–T16
- [x] done

## Open after T17 (maintainer)

- **Decide:** the save-policy widening that landed in T17 (refuse `askPath`, a bare save with no
  project file, a path the saver would extend, a case variant); the R17 QML fix and an AC10
  scenario that reaches the multi-selection view (`spec.md` still describes the reverted design);
  the `project.batch` open-change-save chain (TODO.md); and the R3 wording, which says the
  interface-driving consent is "remembered" when it is asked once per session and never saved.
- **Run, in order, with a current build:** `test_message_box.py::test_a_stop_mid_api_call_is_no_script_error`
  (see it fail once with the `endedByStop` guard in `runLoopTick` removed) and
  `test_script_command_policy.py`; then `ctest -R "tst_message_box_gate|tst_script_command_policy"`;
  then the `gui` case. None of the stop or quit path has executed yet; `start` is now invoked by
  name with a second argument, and a mismatch would show only as a script that never starts.
- **Known gaps, not fixed:** a macro's box at quit is closed by `qApp->exit()`, not by a stop
  (`MacroRunner` is QML-owned, out of `onQuit`'s reach); `~MacroRunner` inside its own box's loop
  would burn its join budget (unconfirmed reachable); the doc comment at
  `core/Pipeline/DataModel/Scripting/ScriptApiCall.cpp` still calls scripts "Trusted origin".
- **Outside the file list:** CMake registrations in `core/Ui`, `core/Api` and `app/tests`,
  `tests/utils/required_services.py`, `scripts/tests/test_test_tooling.py`,
  `doc/claude/architecture/automation.md`.
- **Locally:** `doc/help` changed, and `sanitize-commit.py` cannot run while the maintainer's own
  files are dirty, so the AI search index is stale until it does. Without
  `SERIAL_STUDIO_API_AUTO_CONSENT`, the message-box API tests now ask the interface-driving consent
  once per session.

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` is met or handed to the maintainer, and checked off
  there.
- [x] `code-verify.py --check` and `syntax-check.py` are clean on all changed files.
- [x] The generated-surface checks (AC11) pass locally.
- [x] `qt-cpp-review` run on the C++ diff; findings addressed or noted.
- [x] Diff is what was asked, and only that; no foreign files touched.
- [ ] `spec.md` status set to `done` once the maintainer-run criteria pass.
