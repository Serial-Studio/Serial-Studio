---
spec: 0110-acknowledged-commands
phase: tasks
status: approved     # 2026-11-03: reviewed and approved by the main session under the maintainer's delegation (plans: maintainer, tasks: main session)
updated: 2026-11-03
---

# Tasks 0110 — Acknowledged Commands

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
- **Starts from 0109.** This list assumes 0109 is merged: `serialize(OutputWidget)` lives in
  `Frame.cpp`, `Base::presentation()`, `Base::rangeMessage`, the `dashboard.output.G.W` cell id,
  the `idPrefix` plumbing, 0109's four editor ids and the simulator's capture mode all exist.
  Line numbers below are pre-0109 and shift; re-read the slice before editing.
- **Executor** marks a task delegable to a cheaper model against the donor it names:
  `mechanical` for Haiku, `yes` for Sonnet. `main` stays in the main session: the `StateBinding`
  rules (T3, T4), `Base` (T7) and the `Panel` wiring (T8).
- **Syntax check.** Every edited C++ file goes through `python scripts/syntax-check.py`; a SKIP
  is reported as unverified.
- **Hotpath.** No task edits pipeline code. T8 reads dashboard copies on the display tick and is
  the one task near it; its invariants are named there. AC6's benchmark runs in T23.
- **Commit shape.** Amended spec and approved plan and tasks first, in their own commit. Then T1
  to T6 (model, rules, unit tests); T7 to T12 (control, panel, catalog, QML); T13 to T16 (editor,
  Problems, API); T17 to T22 (example, docs, integration tests).
- **Generated files are never edited.** `api-schema.json` and the typed proto change only in T23.

## Tasks

### T1 — Reply keys and fields

- **Files:** `core/Core/DataModel/FrameKeys.h`, `core/Core/DataModel/Frame.h`
- **Does:** `Keys::OutputReplyTokenId("replyTokenDatasetId")`, `OutputReplyCodeId(
  "replyCodeDatasetId")`, `OutputReplyMessageId("replyMessageDatasetId")` after 0109's keys.
  `OutputWidget` gains the three ids, default -1; declaration of `outputExpectsReply(const
  OutputWidget&)`. **Invariant:** -1 means unbound, so a pre-spec project is unbound (R9).
- **Verify:** `python scripts/syntax-check.py core/Core/DataModel/Frame.cpp`; `python
  scripts/code-verify.py --check` on both; `--tu-census --check`.
- **Executor:** yes, donor 0109 T2.
- **Deps:** 0109 merged
- [x] done

### T2 — Persist the reply binding

- **Files:** `core/Core/DataModel/Frame.cpp`
- **Does:** `outputExpectsReply`: token and code both at least 0. `serialize` writes the three
  ids only when a reply is bound, and writes `stateConfirmMs` when a state source **or** a reply
  is bound; `read()` reads them; `normalize()` floors each id at -1. **Invariant:** an unbound
  widget's JSON is byte-identical to today's (AC4); `stateConfirmMs` is already read
  unconditionally (`Frame.cpp:274`), so the reader needs no change.
- **Verify:** `python scripts/syntax-check.py core/Core/DataModel/Frame.cpp`; `python
  scripts/code-verify.py --check core/Core/DataModel/Frame.cpp`.
- **Executor:** yes, donor 0109 T3.
- **Deps:** T1
- [x] done

### T3 — Confirmation ends the wait (R11, the 0080 R12 half)

- **Files:** `core/Ui/UI/Widgets/Output/StateBinding.h`,
  `core/Ui/UI/Widgets/Output/StateBinding.cpp`
- **Does:** `configure` records whether the control is two-state (Toggle, or checkable Button).
  `noteRequested(now, request)` stores the requested value (number and text); `endInteraction`
  keeps the stored request. A confirmation check compares the current verdict with the request:
  `truth()` against requested 1 or 0 for two-state, fuzzy numeric for a value, text for a Text
  field. A non-const `settle(now)` latches a confirmed window closed (`m_requestedMs = -1`);
  `pending()` (`:184-190`) stays a const read. **Invariants:** `StateBinding` includes no
  `TransmitTarget` and nothing that owns one (0080 R3); apart from early confirmation, a
  state-only control's window behaves as before.
- **Verify:** `python scripts/syntax-check.py core/Ui/UI/Widgets/Output/StateBinding.cpp`;
  `python scripts/code-verify.py --check` on both; `grep TransmitTarget` finds nothing in either.
- **Executor:** main.
- **Deps:** none
- [x] done

### T4 — Reply rules and the per-line token counter

- **Files:** `core/Ui/UI/Widgets/Output/StateBinding.h`,
  `core/Ui/UI/Widgets/Output/StateBinding.cpp`
- **Does:** `expectsReply()`; a file-scope GUI-thread `QHash<int, int> s_replyTokens` keyed by
  the reply-token dataset id (-1 for controls without a reply) and a static `resetTokens()` (A1);
  `issueToken()`: next in 1..32767, wrap to 1, never 0, never the token this line showed last
  tick. `noteSent(now, token)`, `observeReply(token, code, message)` (stores the seen token;
  settles only a waiting outcome whose token matches: 0 acknowledged, else refused), and
  `outcome(now)` (waiting past the window is unanswered). `pending` and `holdsDisplay` per the
  plan's table; the reply window is floored at 100 ms. **Invariant:** a control without a reply
  takes none of these branches beyond drawing a token (R9).
- **Verify:** `python scripts/syntax-check.py core/Ui/UI/Widgets/Output/StateBinding.cpp`;
  `python scripts/code-verify.py --check` on both.
- **Executor:** main.
- **Deps:** T2, T3
- [x] done
  (Main session 2026-11-03: `observeReply` takes `nowMs` first, so a reply arriving after the window
  reads unanswered and the rules stay testable with synthetic times; `noteRequested`'s request
  argument defaults to none, which keeps the 0080 callers and tests compiling.)

### T5 — Unit tests for the rules (AC2, AC8)

- **Files:** `app/tests/tst_output_state_binding.cpp`
- **Does:** `init()` calls `resetTokens()`. AC8: the requested state ends the window early for
  a toggle, a value and a text control, any other state leaves it open; the existing
  `anOutstandingRequestClearsOnConfirmationAndOnExpiry` gains its confirmation half. AC2: a reply
  with the previous token after a second send leaves the control waiting. Also: two controls on
  one token dataset never draw the same token; the skip rule; code 0 acknowledges and ends
  `pending`; a refusal ends `holdsDisplay`; an acknowledgement with a state source holds until the
  state confirms and until the window ends when it never does; silence becomes unanswered; the
  100 ms floor. The other nine existing cases pass unchanged.
- **Verify:** `python scripts/syntax-check.py app/tests/tst_output_state_binding.cpp`; `python
  scripts/code-verify.py --check` on it; `ctest -R tst_output_state_binding` after a build.
- **Executor:** yes, donor the existing cases in the same file.
- **Deps:** T4
- [x] done
  - Status: syntax-checked through the test wrapper only; ctest not run (needs a build).

### T6 — Round-trip test for the reply fields (AC4 unit half)

- **Files:** `app/tests/tst_frame_serialization.cpp`
- **Does:** `outputWidgetRoundTripsEveryField` covers the three ids; a reply-only widget writes
  `stateConfirmMs`; an unbound widget writes none of the three keys.
- **Verify:** `python scripts/syntax-check.py app/tests/tst_frame_serialization.cpp`; `ctest -R
  tst_frame_serialization` after a build.
- **Executor:** mechanical, donor 0109 T5's case in the same file.
- **Deps:** T2
- [x] done

### T7 — `Base`: token, window timer, outcome

- **Files:** `core/Ui/UI/Widgets/Output/Base.h`, `core/Ui/UI/Widgets/Output/Base.cpp`
- **Does:** `deliverValue` draws `issueToken()` and calls `evaluateTransmitFunction(value,
  token)`, which passes `QJSValueList{jsValue, token}` (`:408`). `noteOperatorRequest(value,
  bytesSent)` records the request for every state-bound control (R11), calls `noteSent` for a
  reply-bound one when bytes were sent, and starts `m_windowTimer` (single-shot, the window plus
  1 ms) for either; `endInteraction` restarts it (R10). The timer's slot is `refreshState`.
  `refreshState` evaluates the outcome before its state-only early return (`:274`), calls
  `settle`, emits `transmitError` once on entering refused, and announces through
  `stateChanged`. Adds `expectsReply` (CONSTANT), `outcome`, `outcomeText` (message, or the code),
  `automationState` (`outcome` slug, `token`, `code`, `message`, `waiting`, `known`, `holding`,
  `outOfRange`) and the `observeReply` slot. **Invariants:** the license gates in `sendValue` and
  `deliverValue` keep their order and nothing goes ahead of them; nothing on the reply or timer
  path touches `m_target` (0080 R3); the timer is GUI-thread at command rate.
- **Verify:** `python scripts/syntax-check.py core/Ui/UI/Widgets/Output/Base.cpp`; `python
  scripts/code-verify.py --check` on both; `--singleton-census --check`.
- **Executor:** main.
- **Deps:** T4
- [x] done
  (Main session 2026-11-03: a refusal emits `transmitError` as "Refused: <message or code>";
  `refreshState` settles first, so a confirmation latch applies to every bound control.)

### T8 — `Panel` reads replies; `Preview` reads none

- **Files:** `core/Ui/UI/Widgets/Output/Panel.h`, `core/Ui/UI/Widgets/Output/Panel.cpp`,
  `core/Ui/UI/Widgets/Output/Preview.cpp`
- **Does:** `refreshStates` (`Panel.cpp:184`) visits a control that is state-bound or
  reply-bound (`:191`); a private `observeReply(Base*)` beside `observeTableState` does three
  `constFind`s on `m_dashboard.datasets()`, takes token and code from `numericValue` and the
  message from `value`, or from the number when `isNumeric`. The constructor connects
  `connectionManager.connectedChanged` (the `handlerContext()` reference at `:40`) to
  `StateBinding::resetTokens()` when connected. `Preview::rebuildModel` sets the three ids to -1
  beside `stateSource` (`Preview.cpp:193`). **Invariants:** reads happen on `Dashboard::updated`
  only, at display-tick rate, with no new push target, per-frame signal or pipeline allocation; a
  dataset read never takes the table path (that arms the mirror); the new connection joins two
  GUI-thread objects.
- **Verify:** `python scripts/syntax-check.py` on both `.cpp`; `python scripts/code-verify.py
  --check` on all three (`pipeline-queued-hop` clean).
- **Executor:** main.
- **Deps:** T7
- [x] done

### T9 — The catalog's `state` object

- **Files:** `core/Ui/UI/Automation/ElementCatalog.h`,
  `core/Ui/UI/Automation/ElementCatalog.cpp`
- **Does:** `Element` gains `QJsonObject state`. `describe` (`:220`) reads the item's
  `automationState` property, a `QVariantMap` or a `QJSValue` (a QML `var`), unless the item is
  masked; `toJson` (`:334`) inserts `state` when non-empty. **Invariant:** a masked field never
  reports one; nothing else in the reply shape changes.
- **Verify:** `python scripts/syntax-check.py core/Ui/UI/Automation/ElementCatalog.cpp`; `python
  scripts/code-verify.py --check` on both.
- **Executor:** yes, donor the `checked` read in `describe` and its `toJson` line.
- **Deps:** none
- [x] done

### T10 — Catalog unit test

- **Files:** `app/tests/tst_ui_automation.cpp`
- **Does:** An item named `a.b` with `property var automationState: ({outcome: "refused"})`
  reports `state.outcome`; a masked text field with the property reports none.
- **Verify:** `python scripts/syntax-check.py app/tests/tst_ui_automation.cpp`; `ctest -R
  tst_ui_automation` after a build.
- **Executor:** yes, donor the existing catalog cases in the file.
- **Deps:** T9
- [x] done
  - Status: the new case adds `property var automationState` to two fixture items in `app/tests/support/automation/Fixture.qml`. `test-syncheck.py` fails on this file with pre-existing errors (QImage include, line 91) unrelated to the new case, so the test is unverified by compiler.

### T11 — Outcome readout on the cell

- **Files:** `app/qml/Widgets/Dashboard/Output/DashboardOutputPanel.qml`
- **Does:** The cell binds `property var automationState` from its model. The readout
  (`:125-137`) shows for a reply-bound control too: "acknowledged" (muted), "refused: <text>"
  (alarm colour), "unanswered" (warning colour), distinct from "live", "waiting…" and "no data";
  the full text in the tooltip. `idPrefix` passed to the button and toggle components.
- **Verify:** `python scripts/code-verify.py --check` on the file; `python scripts/term-verify.py`;
  `qt-qml-review`.
- **Executor:** yes, donor the existing readout `Label` and its colour choices.
- **Deps:** T7
- [x] done
  - Status: code-verify clean, qmllint reports no syntax error; qt-qml-review not run, not rendered.

### T12 — Button and toggle ids

- **Files:** `app/qml/Widgets/Dashboard/Output/DashboardButton.qml`,
  `app/qml/Widgets/Dashboard/Output/DashboardToggle.qml`
- **Does:** `property string idPrefix`; `objectName` `idPrefix + ".button"` / `".toggle"` when
  set.
- **Verify:** `python scripts/code-verify.py --check` on both (`qml-objectname-literal` clean).
- **Executor:** mechanical, donor 0109's `DashboardSlider.qml`.
- **Deps:** T11
- [x] done
  - Status: code-verify clean, qmllint reports no syntax error; qt-qml-review not run, not rendered.

### T13 — Reply pickers in the editor

- **Files:** `core/Ui/ProjectEditor/EditorForms/OutputStateRows.h`,
  `core/Ui/ProjectEditor/EditorForms/OutputStateRows.cpp`
- **Does:** After the state rows: Reply Token, then Result Code and Reply Message once a token is
  chosen, each a combo over `"None"` plus `datasetCandidates()` (R1). The early return at
  `:136-137` no longer hides the reply rows or Confirm Within, which shows when either source is
  bound and whose description names both (R5). `applyOutputReplyTarget(widget, project, role,
  index)` shares one dataset-at-index walk with `applyDatasetTarget` (`:214`). **Invariant:** the
  picker list and the resolver walk the project in the same order.
- **Verify:** `python scripts/syntax-check.py` on the `.cpp`; `python scripts/code-verify.py
  --check` on both.
- **Executor:** yes, donor the state target picker in the same file.
- **Deps:** T1
- [x] done

### T14 — Editor ids and commit

- **Files:** `core/Ui/ProjectEditor/ProjectEditorItemIds.h`,
  `core/Ui/ProjectEditor/EditorCommit.cpp`
- **Does:** Append `kOutputWidget_ReplyToken`, `_ReplyCode`, `_ReplyMessage` after 0109's ids.
  Three cases in `applyOutputWidgetField` through `applyOutputReplyTarget`; `ReplyToken` joins
  the rebuild condition.
- **Verify:** `python scripts/syntax-check.py core/Ui/ProjectEditor/EditorCommit.cpp`; `python
  scripts/code-verify.py --check` on both.
- **Executor:** yes, donor the `kOutputWidget_StateTarget` case.
- **Deps:** T13
- [x] done

### T15 — Problems: a missing reply dataset

- **Files:** `core/Ui/Misc/Problems/ProjectCheckers.cpp`
- **Does:** `inspectStateBindings` (`:390`) also flags a reply token, code or message id the
  project no longer defines, as a warning with its own code and a jump to the group (Decision 6).
- **Verify:** `python scripts/syntax-check.py core/Ui/Misc/Problems/ProjectCheckers.cpp`; `python
  scripts/code-verify.py --check` on it.
- **Executor:** yes, donor the `dangling-state-source` finding in the same function.
- **Deps:** T1
- [x] done

### T16 — API fields and the dry-run token

- **Files:** `core/Api/API/Handlers/ProjectOutputWidgetCommands.cpp`,
  `core/Api/API/Handlers/ProjectUpdateCommands.cpp`,
  `core/Api/API/Handlers/ProjectDryRunCommands.cpp`
- **Does:** `outputWidgetGet` returns the three ids when bound and `stateConfirmMs` when either
  source is bound (`:262-266`). `applyOutputStateFields` (`:145-175`) takes the three keys; the
  update description names them and `transmit(value, token)`. `runOutputWidgetSample` passes
  token 1 as the second argument (`:733`) and the dry-run description says so (Decision 8). No
  capability, sealed or safety-tier entry changes.
- **Verify:** `python scripts/syntax-check.py` on the three; `python scripts/code-verify.py
  --check` on them; `python scripts/registry-verify.py`.
- **Executor:** yes, donor 0109 T14.
- **Deps:** T2
- [x] done

### T17 — The example sketch and project

- **Files:** `examples/Acknowledged Commands/AcknowledgedCommands.ino`,
  `examples/Acknowledged Commands/AcknowledgedCommands.ssproj`,
  `examples/Acknowledged Commands/README.md` (all new)
- **Does:** A UART sketch printing one CSV line per frame that ends in the last token, code and
  message, repeated every frame. It accepts `BLINK <token>` (always code 0, `OK`) and `SP <value>
  <token>`, refusing a setpoint above 80 with code 2 and `above limit`. The project: a button and
  a Number field 0 to 100 (0109), both bound to the same three reply datasets, the token passed
  in each transmit script; hidden reply datasets. The README states the convention and the
  setpoint's two limits (control 100, device 80).
- **Verify:** load the `.ssproj` through `project.loadJson` in a running app, or read it back
  with `python -m json.tool`; `python scripts/documentation-verify.py` on the README.
- **Executor:** yes, donor `examples/Project Tutorial/` (sketch, project and README) with the
  `ss-docs` skill for the README.
- **Deps:** T16
- [x] done

### T18 — Register the example

- **Files:** `examples/examples.json`, `examples/README.md`
- **Does:** An entry (`requiresPro: true`, category and difficulty like its neighbours) and a
  line in the examples index.
- **Verify:** `python -m json.tool examples/examples.json`; `python
  scripts/documentation-verify.py`.
- **Executor:** mechanical, donor the "Project Tutorial" entry.
- **Deps:** T17
- [x] done

### T19 — Manual and API reference

- **Files:** `doc/help/Output-Controls.md`, `doc/help/API-Reference.md`
- **Does:** An "Acknowledged Commands" section after State Feedback: `transmit(value, token)`,
  the reply convention for text frames (token, code, message as the last fields, repeated every
  frame, code 0 acknowledged, no separator inside a message), the three outcomes and how they
  combine with state feedback, the shared token per reply line, the outcome lasting as long as the
  control's tile, the example. State Feedback's "Waiting and No Data" says the wait now ends on
  confirmation and on time (R10, R11). API reference: the reply fields and the `state` object on
  `ui.automation.listElements`.
- **Verify:** `python scripts/documentation-verify.py`; `python scripts/term-verify.py`.
- **Executor:** yes, with the `ss-docs` skill; donor the State Feedback section.
- **Deps:** T11, T16, T17
- [x] done
  - Status: written against the plan; the readout wording ("acknowledged", "refused: <message>", "unanswered") must match T11 when it lands.

### T20 — Assistant corpus

- **Files:** `app/rcc/ai/skills/output_widgets.md`, `app/rcc/ai/docs/output_widget_js.md`
- **Does:** `transmit(value, token)`, the reply fields, and reading `state.outcome` and
  `state.token` from `ui.automation.listElements` after driving a control.
- **Verify:** `python scripts/registry-verify.py`; `python scripts/term-verify.py`.
- **Executor:** yes, donor 0109 T16's additions.
- **Deps:** T16
- [x] done

### T21 — Architecture docs

- **Files:** `doc/claude/architecture/automation.md`, `doc/claude/architecture/dashboard.md`
- **Does:** `automation.md`: the `state` object, who may publish `automationState`, and that a
  masked item never does. `dashboard.md` "Output Widgets (Pro)": the reply read on the display
  tick, the device-repeats convention that makes tick sampling enough, the per-line token, and the
  window timer that ends a wait on a silent device.
- **Verify:** `python scripts/claim-verify.py --no-report`.
- **Executor:** yes.
- **Deps:** T8, T9
- [x] done
  - Status: written against the plan; re-check the `Base`/`Panel` names once T7 and T8 land.

### T22 — Integration and structural tests

- **Files:** `tests/integration/test_acknowledged_commands.py` (new),
  `tests/integration/test_output_state_feedback.py`
- **Does:** AC1 (`gui`): a CSV project ending in token, code and message, a button with
  `"BLINK " + token + "\n"`, Confirm Within 800 ms; click `dashboard.output.G.W.button`, read
  the token from the capture, stream frames echoing it with code 0 and read `state.outcome`
  `acknowledged` with that token; code 3 and `too hot` reads `refused` with the message; no echo
  reads `unanswered`, with frames flowing and with the device silent. AC3 (`gui`): a toggle with
  both sources, acknowledged then settled; refused then back to the reported `checked`. AC7
  (`gui`): a state-only toggle, click, stop the simulator, `state.waiting` false within 1.8 s.
  AC4: reply ids through `update`, `get`, save and load; a pre-spec fixture unbound. The 0080
  source scan also covers `observeReply`.
- **Verify:** the static scans run here with `pytest tests/integration/
  test_output_state_feedback.py -k "target or setters" -v`; the rest needs the app.
- **Executor:** yes, donors 0109's `test_output_write_limits.py` and
  `tests/integration/test_ui_automation.py`.
- **Deps:** T8, T11, T12, T16
- [x] done
  - Status: files parse and collect; the new source scan passes; the runtime cases need the app (not run).

### T23 — Maintainer: build, benchmark, schema, tests, board

- **Files:** `app/rcc/api/api-schema.json` and the SDK files `generate-sdk.py` rewrites
  (regenerated); translations (regenerated)
- **Does:** Build; `ctest -R "tst_output_state_binding|tst_frame_serialization|
  tst_ui_automation"`; `--benchmark-hotpath` before and after (AC6); `--dump-api-schema` and
  `python scripts/generate-sdk.py`; run T22 with the API server on (including `-m gui`); AC5 with
  the example on a board.
- **Verify:** AC1 to AC8 ticked in `spec.md` with the date.
- **Executor:** maintainer.
- **Deps:** T1 to T22
- [ ] done

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` is met and checked off there (AC1 by T22 and T23;
  AC2 by T5; AC3 by T22; AC4 by T6 and T22; AC5 and AC6 by T23; AC7 by T22; AC8 by T5).
- [ ] `python scripts/code-verify.py --check` is clean on all changed files (no new errors);
  `--tu-census --check` and `--singleton-census --check` do not grow.
- [ ] `python scripts/syntax-check.py` ran on every edited C++ file; any SKIP named in the handoff.
- [ ] `qt-cpp-review` on T1 to T4, T7 to T9 and T13 to T16; `qt-qml-review` on T11 and T12.
- [ ] `--benchmark-hotpath` run by the maintainer with the nine gates unchanged (AC6).
- [ ] Relevant `pytest` targets identified for the maintainer: `tests/integration/
  test_acknowledged_commands.py`, `test_output_state_feedback.py`, `test_output_write_limits.py`.
- [ ] `python scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [ ] Diff is *what was asked, and only that*: the stale text-field string (plan follow-up) is not
  in it.
- [ ] `spec.md` status set to `done`.
