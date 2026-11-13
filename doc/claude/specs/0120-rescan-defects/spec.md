---
spec: 0120-rescan-defects
title: Defects from the 2026-11-12 quality rescan
status: in-progress  # draft -> approved -> in-progress -> done | shelved
created: 2026-11-12
author: Alex Spataru
---

# Spec 0120 — Defects from the 2026-11-12 quality rescan

> **Step 1 of 2 — what, why, and the design.** Everything above **Design** is readable without
> the codebase. Gate: do not start `/ss-implement` until a human marks this `approved`. It is
> the only gate.

## Problem / Motivation

The 2026-11-12 rescan (five Fable reviewers, each claim checked against the code) found four
groups of defects.

- **Message boxes from scripts and API clients**, new this week. A headless session answers
  "ok" to a yes/no box nobody saw. Boxes stack without limit, so a script calling one from its
  loop floods the screen and plays the error sound each time. A box looks exactly like the
  application's own consent prompt and needs no consent from a remote client. A stopped script
  still receives the user's answer and keeps running on it. Quitting while one is open stalls
  shutdown for five seconds. Automation can neither see nor close one. Macros can wait, against
  the documentation, and miss the watchdog handling the control script has.
- **A project's own scripts reach every API command.** That includes installing extensions from
  a repository they add, changing the licence, and saving over another project, which then
  inherits that project's launch consent.
- **Tests and CI.** The recording-tail tests fail about one run in five on Linux (a test race,
  not data loss). Two rerun entries now hide a regression whose root cause was fixed. A broker or
  simulator that fails to start turns its tests into green skips.
- **Small defects.** The Tiny5 Mono fonts cost every user 19 MB of memory and 30 ms at startup.
  The translation tool says it removes obsolete strings but keeps about 2,050 per language. The
  editor caption goes stale in one multi-selection case.

The deciding constraint is the maintainer's trust rule: **we trust what the user loads, we ask
at most once, and never once per API call.** No fix here adds a prompt.

## Goals

- No script or API client can flood, impersonate, outlive or hang the application with a message
  box.
- A project's scripts keep their full trust except for a short list of commands no project
  script needs, which they are refused without any prompt.
- CI's red means a defect: the recording-tail race and the stale reruns are gone, and a missing
  test service fails loudly where it was expected.
- The small defects are fixed.

## Non-Goals

- **Any new prompt or per-call question**, for anything (the deciding constraint).
- **New API commands or parameters.** The dismiss path reuses automation, and no command
  description changes, so no generated surface needs a rebuild to regenerate.
- **Structural items from the rescan:** accessibility metadata in QML, splitting the oversized
  translation units (spec 0104 C3), QML unit tests, C++ suites for the BLE, HID and Audio drivers,
  turning compiler hardening on by default, and signing extensions. Each is its own project.
- **Deliberate CI choices:** ctest on Linux only (dropped from the other platforms on
  2026-09-03), publishing while the test legs are red, in-app self-tests in shipped binaries
  (`d231c32ef`).
- **The CLI licensing test in CI.** Its deactivate and reset cases would act on CI's licence seat.
- **The sparkplug test's own broker restart skip.** The cause of "mosquitto exited immediately" is
  not yet known, and failing it now would redden CI on a known glitch.
- **The pause window in the pipeline.** A chunk read just before a pause can be dropped within
  microseconds; closing that is a hotpath change that needs its own evidence.
- **Pinning libplctag by commit hash.** A wrong check breaks every configure, and it cannot be
  configured here. It goes on the TODO list.
- **Regenerating the `.ts` files** after the tool is fixed. That stays the maintainer's step.
- **Tiny5 Mono's size in the binary** (2.7 MB compressed): a product choice.
- **Process items** (the commit gate's isolation rules, spec weight). The maintainer is editing
  those himself.

## Requirements

**Message boxes from scripts and API clients**

1. **R1 — One at a time.** While a box sent by a script or API client is showing or queued, a
   new request is refused with an error that says a box is already showing. Boxes never stack,
   so the error sound plays at most once per box.
2. **R2 — Provenance.** Every such box carries a line its sender cannot change, saying it comes
   from the project's scripts or an API client and not from Serial Studio. It never shows the
   application's own icon.
3. **R3 — Remote consent.** A remote API client needs the existing interface-actions consent
   before it can show a box. That consent is asked once and remembered, as for the other
   interface-driving commands. A sealed deployment's own scripts can still show boxes.
4. **R4 — Headless.** In a session with no display, a waiting request is refused with an error
   naming the reason. A non-waiting one is written to the log instead of vanishing.
5. **R5 — Stop.** When the control script or a macro stops (disconnect, project change, user
   stop) while its box is open, the box closes. The waiting call does not return an answer to
   running code; the script stops at that call.
6. **R6 — Quit.** Quitting while a script's box is open closes the box and shuts down promptly,
   without the shutdown wait running out or warning that the script did not stop.
7. **R7 — Macros.** Macros may wait for an answer, as the control script can, and this is
   documented. Time spent waiting on the user is not billed to a macro's time budget, and no
   deadline that passed during the wait can interrupt it afterwards.
8. **R8 — Automation.** While a script or API box is open, the automation window list shows it as
   a modal window with its title and text, the main window reports itself modal, and an Escape
   key sent through automation closes it. The waiting call then receives the cancel answer.
9. **R9 — Documentation.** The docs say which scripts may wait (the control script and macros).
   They also say what "control-script only" means for program launch: refused to remote API
   clients, and consent-gated for every script.

**A project's scripts**

10. **R10 — Refused commands.** A project's own scripts (parser, transforms, control script,
    macros) are refused, with an error and no prompt, for:
    - adding or removing an extension repository, and installing or uninstalling an extension;
    - activating, deactivating or changing the licence, and starting a trial;
    - saving the project to any file other than the one it was opened from.

    Saving the project to its own file still works, and everything else stays as trusted as
    today.
11. **R11 — Other callers unchanged.** Remote API clients and the AI assistant keep exactly the
    gates they have today; R10 applies only to a project's scripts.

**Tests and CI**

12. **R12 — Recording tail.** The recording-tail tests wait until the application has received
    their last frame before pausing or disconnecting, and the test device sends each frame at
    once, without coalescing delay.
13. **R13 — Reruns.** The two parse-budget tests leave the automatic rerun list, now that their
    root cause is fixed.
14. **R14 — Missing services fail.** On a CI leg that starts an MQTT broker or the OPC UA
    simulator, a test that cannot reach that service fails instead of skipping. Local runs keep
    skipping.

**Small fixes**

15. **R15 — Fonts on demand.** The Tiny5 Mono families still appear in the console font list,
    but their font files load only when one of them is chosen, or restored from settings at
    startup.
16. **R16 — Translations.** Updating translations removes obsolete strings, as the tool already
    says it does.
17. **R17 — Editor caption.** The Project Editor caption's text and icon follow a
    multi-selection that changes item kind without leaving the multi-selection view.

## Acceptance Criteria

- [ ] **AC1** — A ctest suite pins the message-box decisions (R1, R4, R5, R7): refused while
  busy, a headless wait refused, a wait allowed only where granted, and a cancelled wait
  answering cancel.
- [ ] **AC2** — A ctest suite pins the script command policy (R10, R11). Each refused command is
  refused for script origin and allowed otherwise. Saving to the project's own path is allowed;
  saving to another path is refused, as is saving to a path when the project has no file yet.
- [ ] **AC3** — A pytest integration test loads a project whose control script tries a refused
  command and a save to another path, records both results in a data table, and the test reads
  them back. Both are refused and no file is created (R10). The same suite confirms that a remote
  client's call to those commands behaves as before (R11).
- [x] **AC4** — Static pytest checks pin the wiring:
  - the wait permission is granted only by the control script's and macros' dispatch (R7);
  - the message box is classified as interface-driving and stays allowed in sealed sessions
    (R3);
  - the refusal list lives in one place (R10);
  - the translation tool passes the obsolete-removal flag (R16).
- [ ] **AC5** — A `gui`-marked test, run by the maintainer with the app on a desk, covers R2, R5,
  R6 and R8:
  - a box shows the provenance line and not the app icon;
  - automation lists it as modal, and Escape closes it with the cancel answer;
  - stopping the script while it is open closes it, and the script stops;
  - quitting while it is open exits promptly with no "did not stop" warning.
- [ ] **AC6** — The recording-tail tests pass on every CI leg for ten consecutive master runs
  (R12).
- [x] **AC7** — The rerun list no longer names the two parse-budget tests (R13).
- [x] **AC8** — A unit test in the script tier drives the missing-service helper both ways
  (required, so it fails; not required, so it skips), and the CI workflow sets the requirement
  only on legs that start the service (R14).
- [ ] **AC9** — The maintainer sees the Tiny5 families in the console font list. Choosing one
  renders it, and so does restarting with it selected. Startup memory drops by about 19 MB with
  another font selected, measured with `footprint` (R15).
- [ ] **AC10** — The maintainer, in a multi-selection of datasets, adds a group to the selection
  and the caption updates (R17).
- [x] **AC11** — The generated-surface checks CI runs pass locally: `registry-verify.py`,
  `generate-command-strings.py --check`, `generate-command-safety.py --check`,
  `generate-sdk.py --check`. `code-verify.py --check` and `syntax-check.py` are clean on every
  changed file, and `pytest scripts/tests` and `pytest tests/scripts` pass.

## Constraints & Invariants

- **No new prompts, no per-call questions** (deciding constraint).
- No new API command, parameter or command description. A change to any of those regenerates
  surfaces that need a built binary.
- No hotpath change: nothing on the frame path, the block path or the display tick.
- The application's own dialogs behave exactly as today. Provenance, the one-at-a-time rule and
  closing on stop apply only to boxes sent through the API.
- `setInterrupted(true)` stays only in the watchdog thread (enforced: `code-verify:
  js-interrupt-site`). Stopping a script at its waiting call does not add a new interrupt site.
- Security-sensitive surface: the interface-driving classification (R3) changes an API
  consent table. That change and the script refusals (R10) are named in chat with their failure
  modes before they land. The licensing handlers themselves are not edited.
- Commits, when asked: at least three, because the commit gate isolates the consent tables and
  the CI workflows.

## Premise Check

- **Question:** Would refusing those commands to project scripts break anything that exists, and
  are the other defects real at the size reported?
- **Measured (2026-11-12):**
  - **Script refusal list:** zero callers. No example project, help page or firmware sample calls
    an extension, licensing or save-as command from a script. The only hits are the generated
    SDK wrappers and remote-client tests.
  - **Headless answer:** the dialog helper returns "ok" on the offscreen platform whatever
    buttons were offered (read, `Utilities.cpp:239`).
  - **Macros:** they dispatch through the same function that grants the wait (read,
    `ControlScriptWorker.cpp:82` and `MacroRunner.cpp:178`), and their bridge skips the watchdog
    disarm (read, `MacroWorker.cpp:96`).
  - **Consent cadence:** interface-actions consent asks once and is remembered (read, the
    consent gate's granted state).
  - **Tiny5 Mono:** registering the 20 faces costs 30 ms and adds 18.9 MB of resident memory
    (measured with PySide6 6.11).
  - **Recording tail:** a test race (Fable trace). The test toggles pause or disconnect 20 ms
    after its last send without checking the app received it. Nagle plus Linux delayed ACK can
    hold that frame past the toggle, and no product hop loses a frame the pipeline already holds.
- **Verdict:** Proceed. Nothing existing breaks, and every defect is real at the reported size.

## Design

### Approach

Message-box rules sit in the API handler, around the shared dialog helper. The helper gains only
an optional hook that hands the open box to its caller, so the application's own dialogs are
unchanged. The decisions (busy, headless, wait permission, cancel) live in a small header-only
gate with no Qt widgets, so a ctest can pin them, following the spec 0119 pattern.

Script-origin refusal adds a third command origin, `Script`, used only by the one entry point all
three scripting bridges share. Under it a dispatch policy refuses the R10 list, decided by a pure
function a ctest can pin. It opens after the trusted scope, whose policy shadow would otherwise
blank it.

A stopped script is stopped at the call site: the bridge throws into the engine when its stop
flag was raised during the call. That needs no new interrupt site.

### Affected files

| File | Change |
|------|--------|
| `core/Core/Api/UserWaitScope.{h,cpp}` | Add: whether a script wait box is open, a registered closer, and `cancelOpenBox()`, all GUI thread. |
| `core/Ui/ApiHandlers/MessageBoxGate.h` (new) | Header-only decisions: busy, headless, wait allowed, cancel answer. |
| `core/Ui/ApiHandlers/WindowHandler.cpp` | One-at-a-time counter, provenance line, no app icon, headless refusal and log, register the closer around the wait, cancel answer. |
| `core/Ui/Misc/Utilities.{h,cpp}` | Optional `onOpen(QMessageBox*)` hook on `showMessageBox`; defaults keep every caller. |
| `core/Api/API/CommandHandler.{h,cpp}` | `CommandOrigin::Script`; `execute()` uses it; the script `DispatchPolicyScope` opens after the trusted scope. |
| `core/Api/API/ScriptCommandPolicy.h` (new) | Pure refusal decision: command name, params and open project path in; refusal or nothing out. |
| `core/Api/API/CommandCapabilities.cpp` | `ui.window.showMessageBox` becomes `DrivesUi` (consent table, protected). The sealed list is unchanged. |
| `core/Pipeline/DataModel/Scripting/ControlScriptWorker.{h,cpp}` | Per-run stop flag; after a dispatch that returns with the flag raised, throw into the engine. |
| `core/Pipeline/DataModel/Scripting/ControlScript.cpp` | Raise the stop flag and `cancelOpenBox()` in `stopWorker()`. In `shutdown()`, if a box is open, cancel it and defer the join to the `aboutToQuit` pass once the stack has unwound. |
| `core/Pipeline/DataModel/Scripting/MacroWorker.cpp` | Disarm the watchdog before the blocking dispatch; throw (JS) or raise a Lua error after a dispatch that ends with stop requested. |
| `core/Pipeline/DataModel/Scripting/MacroRunner.cpp` | `stop()` also calls `cancelOpenBox()`. |
| `app/src/Misc/ModuleManager.cpp` | `onQuit` defers the control-script join while a box is open, only if its own latch would otherwise skip the second pass (read first). |
| `core/Ui/UI/Automation/WindowInventory.{h,cpp}`, `core/Ui/ApiHandlers/AutomationHandler.cpp` | List the open API box via `QGuiApplication::modalWindow()`, report the main window modal, route Escape to the box. Read `doc/claude/architecture/automation.md` first. |
| `core/Ui/Misc/CommonFonts.{h,cpp}` | Explicit family-to-file table for Tiny5 Mono; `ensureFamilyLoaded(family)`; no startup registration. |
| `core/Ui/Console/Handler.cpp` | `availableFonts()` merges the bundled families; `setFontFamily` and the settings restore load the family first. |
| `core/Ui/ProjectEditor/EditorMultiSelect.cpp` | Also emit `selectedTextChanged` when the batch changes inside the multi-selection view. |
| `app/translations/translation_manager.py` | Pass `-no-obsolete` to `lupdate`. |
| `tests/integration/test_recording_fidelity.py` | Before the edge, wait for the last frame via the latest-frame API. |
| `tests/utils/device_simulator.py` | Set `TCP_NODELAY` on the accepted socket. |
| `tests/conftest.py` | Remove the two parse-budget names from `_FLAKY_TESTS`. |
| `tests/integration/conftest.py` | A missing broker or simulator fails when `SS_REQUIRE_BROKER` / `SS_REQUIRE_OPCUA_SIM` is set. |
| `.github/workflows/test-suite.yml` | Set those variables only on legs that start the service. |
| `tests/integration/test_message_box.py` | Static checks (AC4) and the `gui` test (AC5). |
| `tests/integration/test_script_command_policy.py` (new) | AC3. |
| `scripts/tests/` (one test) | AC8 and the translation flag. |
| `app/tests/tst_message_box_gate.cpp`, `app/tests/tst_script_command_policy.cpp`, `app/tests/CMakeLists.txt` | AC1, AC2. |
| `doc/claude/architecture/scripting.md`, `doc/help/API-Reference.md` (prose only), `doc/claude/TODO.md` | R9, the libplctag pin and the `.ts` regeneration step. |

### Hotpath & threading impact

- **Touches the hotpath?** No. Nothing on the frame path, the block path, FrameBuilder, FrameReader,
  CircularBuffer or the display tick changes.
- **New cross-thread signal/slot?** No. The script workers already block on the GUI thread with
  a `BlockingQueuedConnection` into their marshaller; that is unchanged. The new per-run stop flag
  is an atomic, written on the GUI thread and read on the worker after the call returns
  (release/acquire).
- **New input to a cached hotpath flag?** No.
- **Timestamp ownership** — untouched.
- **Quit ordering:** cancelling the box inside a quit that started in the box's own modal loop
  cannot return `exec()` until the quit handler unwinds. So the join is deferred to the second,
  `aboutToQuit` pass, and the trace is written into the implementation's test notes.

### Data model, API and UI

- No `Keys::`, schema or project-JSON change, and no new or renamed command or parameter.
- One classification change: `ui.window.showMessageBox` moves to `DrivesUi` for remote callers.
  The safety tier (AI assistant) and the sealed list are unchanged.
- New error messages (busy, headless, refused for project scripts) are runtime strings, not
  generated surfaces.
- UI: the box gains a fixed provenance line and loses the app-icon option. The console font list
  is unchanged from the user's point of view.

### Tradeoffs

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Gating project scripts | Prompt per command; prompt once per project; refuse a short list with no prompt | **Refuse the list** (maintainer, 2026-11-12): trust what is loaded, never ask per call. |
| Stopping a stale script | Fire the watchdog now; throw at the waiting call | **Throw at the call**: deterministic, and no new interrupt site. A script that catches and ignores the error is its author's choice. |
| Flood | Refuse while one is showing; newest replaces the text | **Refuse** (maintainer): nothing is lost silently, and the caller can retry. |
| Dismissal | Automation Escape; a new dismiss command | **Escape** (maintainer): no new command, so no consent tables, schema or SDK change. |
| Macros and the wait | Allow and document; control script only | **Allow** (maintainer): a confirm-before-send dialog is a natural macro use. |
| Provenance wording | Name the exact sender; one fixed line | **One fixed line**: naming the sender needs a third origin channel through every caller, and the defect is only that a box can pass as the app's own. |
| Closing the box on stop | Close the app's active modal widget; a handle registered by the API path | **Registered handle**: the active modal widget can be the app's own dialog. |
| Save-as from a script | Refuse another path; key launch consent by file content | **Refuse another path**: one rule, no consent-key migration. |
| Tiny5 loading | Lazy per family; drop variants | **Lazy per family**: keeps the maintainer's font choice and removes the cost for everyone else. |

### Risks

- **A deferred join is skipped** if `onQuit` latches on its first pass. Mitigation: read `onQuit`
  before editing; AC5 covers quitting with a box open.
- **A dangling box pointer** after the box closes. Mitigation: `QPointer`, cleared by the
  registered closer's owner when `exec()` returns.
- **The refusal list breaks a user's project.** The premise found no such script. The error names
  the rule.
- **Headless remote clients lose boxes.** One with no interface-actions consent and no
  `SERIAL_STUDIO_API_AUTO_CONSENT` is now refused; CI sets the override.
- **The policy shadow.** A script policy opened before the trusted scope would be blanked by it.
  AC2/AC3 catch the ordering.
- **Font restore.** A persisted Tiny5 choice must load before the fixed-pitch check, or the
  console falls back. AC9 covers a restart with it selected.
- **Silent-breakage classes** from `common-mistakes.md`: a new modal path that skips the wait
  scope, and a signal-wiring change on the console font properties.

### Tests

- **AC1, AC2:** `tst_message_box_gate`, `tst_script_command_policy` (ctest, CI Linux and both
  sanitizers).
- **AC3:** `tests/integration/test_script_command_policy.py` (all four CI legs).
- **AC4:** static cases in `tests/integration/test_message_box.py`, plus one `scripts/tests` case
  for the translation flag.
- **AC5:** the `gui`-marked case in `test_message_box.py`, run by the maintainer.
- **AC6:** CI history.
- **AC7:** read-back.
- **AC8:** a `scripts/tests` unit for the helper, plus a workflow read-back.
- **AC9, AC10:** maintainer observation.
- **AC11:** the local commands named there.

## Open Questions

- None. The maintainer answered the four design calls on 2026-11-12: refuse all three groups to
  scripts, allow macros to wait, refuse while a box is showing, and dismiss through automation
  Escape.
