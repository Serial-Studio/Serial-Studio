---
spec: 0110-acknowledged-commands
phase: plan
status: approved     # approved 2026-11-03 (maintainer): A1, A2 and all recommended answers; 0080 R10/R11 folded in
updated: 2026-11-03
---

# Plan 0110 — Acknowledged Commands

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Read the relevant `doc/claude/` sub-docs and the *actual code*
> before writing this — a plan grounded in a stale mental model is worse than no plan.
> Gate: do not start `/ss-tasks` until a human marks this `approved`.
>
> Built on [plan 0109](../0109-output-write-limits/plan.md): it assumes 0109's `serialize` move
> out of `Frame.h`, `Base::presentation()`, the `dashboard.output.<groupId>.<widgetId>` cell id and
> the `idPrefix` plumbing are merged first.

## Approach (one paragraph)

A reply is three ordinary datasets the control names: token, result code and an optional message.
Spec 0080 already gives every bound control a display-tick read of `Dashboard::datasets()`
(`Panel::refreshStates`, `Panel.cpp:184`) and a confirm-within clock (`StateBinding`), so this spec
extends both instead of adding a class: `Panel` reads the reply datasets for reply-bound controls
on the same tick, and `StateBinding` gains the reply rules (match only the newest token, an
acknowledgement or refusal ends the window, a control with both sources holds until its state
confirms). `Base` takes a token from a counter kept per reply-token dataset and hands it to
`transmit(value, token)`, arms a single-shot timer on every outstanding window so the window ends
on time even from a silent device, and reports a refusal through the existing `transmitError`
signal. The same two pieces repair the two spec-0080 defects folded in as R10 and R11: the timer
ends a state-only control's "waiting…" without needing a frame, and the request is recorded so a
matching state report ends the wait early. The outcome becomes machine-readable through a small
extension of the spec-0101 element catalog: `ui.automation.listElements` returns a `state` object
for an item that publishes one. No new API command, no pipeline code, no allocation on the
pipeline.

## Affected subsystems & files

| File | Change |
|------|--------|
| `core/Core/DataModel/Frame.h` | `OutputWidget` gains `replyTokenDatasetId`, `replyCodeDatasetId`, `replyMessageDatasetId` (all -1); declaration of `outputExpectsReply(const OutputWidget&)` (token and code both bound). |
| `core/Core/DataModel/Frame.cpp` | `serialize` writes the three ids only when a reply is bound, and writes `stateConfirmMs` when a state source **or** a reply is bound (today only the former, `Frame.h:247-249`); `read()` reads them; `normalize()` floors ids at -1; `outputExpectsReply` body. |
| `core/Core/DataModel/FrameKeys.h` | `OutputReplyTokenId("replyTokenDatasetId")`, `OutputReplyCodeId("replyCodeDatasetId")`, `OutputReplyMessageId("replyMessageDatasetId")`. |
| `core/Ui/UI/Widgets/Output/StateBinding.h/.cpp` | Reply state and rules: `expectsReply()`, `issueToken()`, `noteSent(now, token)`, `observeReply(token, code, message)`, `outcome(now)`. The token counters live in one GUI-thread `QHash<int, int>` at file scope in `StateBinding.cpp`, keyed by the reply-token dataset id (-1 for controls without a reply), with a static `resetTokens()` (A1). `noteRequested(now, request)` records what was asked and a confirmation check ends the window when the state verdict matches it (R11). Still holds no `TransmitTarget`. |
| `core/Ui/UI/Widgets/Output/Base.h/.cpp` | Token into `evaluateTransmitFunction(value, token)` (`Base.cpp:402`, `QJSValueList{jsValue, token}` at `:408`); the admitted value is passed to `noteOperatorRequest`, which also fires for reply-only controls (today it returns when unbound, `:316`); `refreshState` evaluates the outcome before its state-only early return (`:274`); a single-shot `m_windowTimer` armed whenever a window opens, state or reply (R10); `transmitError` once per refusal; properties `expectsReply`, `outcome`, `outcomeText`, `automationState`; slot `observeReply`. |
| `core/Ui/UI/Widgets/Output/Panel.h/.cpp` | `refreshStates` also visits reply-bound controls (`:191` skips anything unbound today); private `observeReply(Base*)` beside `observeTableState`; `StateBinding::resetTokens()` when `IO::ConnectionManager::connectedChanged` reports connected, through the `API::handlerContext().connectionManager` reference `liveTransmitTarget` already uses (`:40`). |
| `core/Ui/UI/Widgets/Output/Preview.cpp` | `rebuildModel` clears the reply ids beside the state source (`:193`): the preview reads no live equipment. |
| `core/Ui/UI/Automation/ElementCatalog.h/.cpp` | `Element` gains `QJsonObject state`; `describe` (`:220`) reads an `automationState` property (a `QVariantMap`, or a `QJSValue` from a QML `var`) unless the item is masked; `toJson` (`:334`) emits `state` when non-empty. |
| `app/qml/Widgets/Dashboard/Output/DashboardOutputPanel.qml` | The cell publishes `automationState` from its model; the readout (`:125-137`) shows for reply-bound controls too, with "acknowledged", "refused: <message or code>" and "unanswered" in three colours distinct from "live", "waiting…" and "no data"; full message in the tooltip. `idPrefix` passed to button and toggle. |
| `app/qml/Widgets/Dashboard/Output/DashboardButton.qml`, `DashboardToggle.qml` | `idPrefix` and `objectName` `.button` / `.toggle` (0109 named the numeric three). |
| `core/Ui/ProjectEditor/EditorForms/OutputStateRows.h/.cpp` | Reply rows: Reply Token, then Result Code and Reply Message once a token is chosen, each a picker over `datasetCandidates()` (R1); Confirm Within shows when a state source or a reply is bound (today it is unreachable without a state source, `:136-137`), its description naming both (R5). `applyOutputReplyTarget(widget, project, role, index)` shares the picker walk with `applyDatasetTarget` (`:214`). |
| `core/Ui/ProjectEditor/ProjectEditorItemIds.h` | Append `kOutputWidget_ReplyToken`, `_ReplyCode`, `_ReplyMessage` after 0109's ids. |
| `core/Ui/ProjectEditor/EditorCommit.cpp` | Three cases in `applyOutputWidgetField`; rebuild the form when `ReplyToken` changes. |
| `core/Ui/Misc/Problems/ProjectCheckers.cpp` | `inspectStateBindings` (`:390`) also flags a reply dataset the project no longer defines (Decision 6). |
| `core/Api/API/Handlers/ProjectOutputWidgetCommands.cpp` | `outputWidgetGet` returns the reply ids when bound, and `stateConfirmMs` when either source is bound (`:262-266`). |
| `core/Api/API/Handlers/ProjectUpdateCommands.cpp` | `applyOutputStateFields` (`:145-175`) takes the three keys; the description names them and `transmit(value, token)`. |
| `core/Api/API/Handlers/ProjectDryRunCommands.cpp` | `runOutputWidgetSample` passes token `1` as the second argument (`:733`), so a script that uses the token dry-runs as it runs live; description says so. |
| `doc/help/Output-Controls.md` | "Acknowledged Commands" section: the token argument, the reply convention, the three outcomes, how they combine with State Feedback, the example. |
| `doc/help/API-Reference.md` | Reply fields on get/update; the `state` object on `ui.automation.listElements`. |
| `app/rcc/ai/skills/output_widgets.md`, `app/rcc/ai/docs/output_widget_js.md` | `transmit(value, token)`, reply binding, reading the outcome after driving a control. |
| `doc/claude/architecture/automation.md`, `doc/claude/architecture/dashboard.md` | The `state` key and who may publish it; the reply read and its tick-rate contract. |
| `examples/Acknowledged Commands/` (`.ino`, `.ssproj`, `README.md`), `examples/examples.json`, `examples/README.md` | **New** UART example (R8). |
| `app/tests/tst_output_state_binding.cpp` | AC2, AC8 and the reply rules; `anOutstandingRequestClearsOnConfirmationAndOnExpiry` finally tests the confirmation half its name promises. |
| `app/tests/tst_frame_serialization.cpp` | Reply fields round-trip; absent when unbound (AC4 unit half). |
| `app/tests/tst_ui_automation.cpp` | An item with `automationState` reports `state`; a masked field never does. |
| `tests/integration/test_acknowledged_commands.py` | **New**: AC1, AC3 and AC7 (`gui` marker), AC4. Uses 0109's capture mode in `tests/utils/device_simulator.py`. |
| `tests/integration/test_output_state_feedback.py` | The R3 source scan also covers `observeReply`. |
| `app/rcc/api/api-schema.json`, `doc/grpc/serialstudio-typed.proto` | Not hand-edited: descriptions changed, so the maintainer re-dumps after a build. |

## Architecture & data flow

**Send.** `Base::deliverValue` (`Base.cpp:381`) takes a token from `StateBinding::issueToken()`:
the next integer in 1..32767 on the counter for this control's reply-token dataset, wrapping to
1, never 0, and never equal to the token that dataset showed on the last tick. One counter per
reply line (A1) means two controls answering on one line can never wait on the same number;
controls without a reply share the counter keyed -1. The skip is what makes "resets on connect"
safe: a device that kept running across a reconnect still reports its last token, and a reset
counter would otherwise hand out that same number and read the stale reply as an instant
acknowledgement. The token goes to `transmit(value, token)`; a script that ignores the argument
behaves as today. `noteOperatorRequest(value)` then records the admitted value as the request
(R11) and opens the window; when the control expects a reply and the script produced bytes,
`noteSent` records the token and the outcome becomes waiting. `m_windowTimer` starts for the
window either way (R10).

**Reply.** On each display tick `Panel::refreshStates` calls `observeReply(model)` for a
reply-bound control: three lookups in `Dashboard::datasets()`, token and code from
`numericValue`, the message from `value` (or the number when the sample is numeric, because the
`m_datasets` copies are not string targets and their `value` is only current for text samples).
`StateBinding::observeReply` stores the token it saw and, when the outcome is waiting and the token
equals the expected one, settles it: code 0 is acknowledged, anything else refused (R2, R3). An
older token changes nothing.

**Outcome rules** (R3 to R5), all in `StateBinding`, one window (`stateConfirmMs`, R5):

| Outcome | `pending` | `holdsDisplay` (state-bound only) |
|---------|-----------|-----------------------------------|
| state-only control, inside the window (R11) | true until the state confirms | same |
| waiting on a reply, inside the window | true | true |
| acknowledged, no state source | false | n/a |
| acknowledged, with state source | false | true until the state confirms, or the window ends |
| refused | false | false: the reported state wins at once |
| unanswered (window ended while waiting) | false | false |

"The state confirms" means the current verdict matches the recorded request: the 0080 truth rule
for a two-state control (requested 1 or 0 against `truth()`), a fuzzy numeric compare for a value
control, a text compare for a Text field. It latches: once confirmed, the window is closed and the
control follows its source again. This is spec 0080 R12's missing half (R11): today `pending()`
(`StateBinding.cpp:184-190`) is time-only and no request is stored, although the unit test
`anOutstandingRequestClearsOnConfirmationAndOnExpiry` is named for both.

**Expiry (R10).** `m_windowTimer` calls `refreshState` when the window's time is up; for a reply
that latches unanswered, for a state-only control it ends "waiting…" and applies the verdict. The
timer exists because `Panel::refreshStates` runs on `Dashboard::updated`, which is emitted only
when a block arrived (`Dashboard.cpp:396-399`): a device that went silent left every control
waiting forever, the 0080 defect R10 names, and would leave a reply unanswered forever.

**Reporting.** On the transition to refused, `Base` emits `transmitError` once, so the cell's
existing red border and tooltip carry it (`DashboardOutputPanel.qml:94-115`), and the readout keeps
"refused: <message>" until the next send, so an operator who looked away still finds it (R6).
`automationState` holds `outcome` (`none`, `waiting`, `acknowledged`, `refused`, `unanswered`),
`token`, `code`, `message`, `waiting`, `known`, `holding` and 0109's `outOfRange`; the cell binds it
and the catalog returns it under `state` for `dashboard.output.<groupId>.<widgetId>` (R6, R7). A
client reads `token` before and after it drives the control to know the outcome is its own.

**Never transmits.** The reply path is `Panel` → `Base::observeReply` → `StateBinding` →
`refreshState` → `applyStateVerdict`; none of it touches `m_target` (spec 0080 R3).

## Hotpath & threading impact

- **Touches the hotpath?** **No.** Reply datasets are ordinary columns: parsed, staged and
  published exactly like every other dataset. Where they are read, by line:
  - `Dashboard::onDisplayTick` (`Dashboard.cpp:384-400`, the `TimerEvents::uiTimeout` slot wired at
    `:334`) drains the block ring and emits `updated()` at most once per tick.
  - `DashboardIngest::applyBlockValues` (`DashboardIngest.cpp:289-302`) has by then written each
    dataset's `numericValue`, `isNumeric` and `displaySampleMs` into the GUI-side copies,
    `m_datasets` among them, on the GUI thread. Hidden datasets are included, because
    `WidgetMapBuilder::processDatasetIntoWidgetMaps` inserts into `m_datasets` before its
    `hideOnDashboard` return (`WidgetMapBuilder.cpp:118-131`).
  - `Panel` is connected to `Dashboard::updated` (`Panel.cpp:78`); `refreshStates` (`:184`) reads
    `Dashboard::datasets()` (`Dashboard.h:311`).

  So: no new signal from the pipeline, no per-frame emission, no new push target, no allocation on
  the pipeline. GUI cost per tick is three `QMap::constFind` and one implicitly shared `QString`
  copy per visible reply-bound control.
- **Tick sampling is enough, by convention.** Only the newest sample per tick is seen, and the
  drain keeps only the newest block past its budget (`Dashboard.cpp:422-429`). The convention
  therefore has the device repeat its latest reply in every frame; a reply sent once can be missed
  under load and would read as unanswered, which the manual says.
- **New cross-thread signal/slot?** **No.** `m_windowTimer` is a GUI-thread timer at command rate;
  `ConnectionManager::connectedChanged` to `Panel` joins two GUI-thread objects.
- **New input to a cached hotpath flag?** **No.**
- **Timestamp ownership.** Unchanged. Matching is by token; no time is stamped or re-stamped.
- **AC6.** One `--benchmark-hotpath` before and after, expected flat since no pipeline TU changes.

## Data model & persistence

Three additive integer fields, -1 meaning unbound, written only when token and code are both bound,
so a pre-spec project reads back unbound and saves byte-identical (R9, AC4). `stateConfirmMs` is
read unconditionally today (`Frame.cpp:274`), so writing it for a reply-only control needs no
reader change. No version bump; older builds ignore the keys. A reply binding does not depend on a
state source, and the two share only the window.

## API / SDK surface

No new command, so no capability, sealed-policy or command-safety entry is due. The new surface is
the `state` object inside `ui.automation.listElements`, which already carries its classification:
plain capability, the assistant's safe tier, denied in a sealed session (automation.md "Gating").
`project.outputWidget.get` / `.update` carry the three ids; `.dryRun` documents token 1. The
`transmit(value, token)` signature is documented, not enforced.

## QML / UI

Three new readout states with words and colours: acknowledged in the muted placeholder colour with
the accent, refused in the alarm colour with the message, unanswered in the warning colour. The
existing "waiting…" covers the outstanding reply. The Project Editor's State Feedback section gains
the reply pickers after the state rows; the pickers list the project's datasets, never a free
field (R1).

## Tradeoffs & alternatives considered

Named candidates before converging:
- **Extend-0080** (chosen): reply rules in `StateBinding`, reply read in `Panel::refreshStates`.
- **ReplyBinding class**: a sibling sub-object. Rejected: R5 makes the window shared state, and two
  classes writing one clock is how the rules drift.
- **Push-on-reply**: register a reply-bound control as a value-push target so it reacts per sample.
  Breaks the assumed tick-rate constraint; checking whether that constraint binds: it does, the
  spec states it, and the convention makes tick sampling lossless anyway.
- **Outcome command**: a new `dashboard.output.outcome` command instead of the catalog `state`.

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Outcome over the API | (a) `state` in `listElements`; (b) a new command; (c) translated readout labels with stable ids | **(a)**: no new command to classify in three registries, and a live control exists only while its tile does, so (b) needs the same reach with more surface. (c) is not machine-readable. |
| Token argument | (a) second argument to `transmit`; (b) a global injected before each call | **(a)**: visible in the signature, ignored by existing scripts, identical in dry runs. |
| Confirmation | (a) compare the state verdict with the recorded request, for every state-bound control; (b) hold for the rest of the window | **(a)**: R4 and R11 both say "until the state source confirms"; one rule serves the acknowledged and the state-only case. |
| Expiry | (a) per-control single-shot timer; (b) wait for the next tick; (c) a 1 Hz panel refresh | **(a)**: (b) never fires on a silent device, (c) is up to a second late. |
| Token counter (A1) | (a) per reply-token dataset, file-scope map in `StateBinding.cpp`; (b) per control | **(a)**, approved: (b) let one reply settle two controls; the map sits with the rule so `tst_output_state_binding` covers it. |
| Message text | (a) `value`, or the number when numeric; (b) `value` only | **(a)**: `value` on the `m_datasets` copy is stale for numeric samples. |

## Risks & mitigations

- **Cross-control token collision.** Closed by A1: controls on one reply line share one counter.
  The map is process-wide and GUI-thread only; a unit test resets it in `init()`.
- **Feedback loop.** The reply path cannot reach `m_target`; `StateBinding` still includes no
  `TransmitTarget`; the source scan is extended.
- **Behaviour change for state-only controls.** R10 and R11 change spec-0080 controls on purpose:
  "waiting…" now ends early on confirmation and on time when silent. The existing ten unit cases
  must still pass unchanged apart from the strengthened confirmation case.
- **Silent device.** Covered by the timer.
- **Stale reply after reconnect.** Covered by the skip rule.
- **Confirm Within of 0.** Legal for 0080, but with a reply it would read unanswered at once. The
  reply window is floored at 100 ms.
- **Lifetime.** A control's outcome lives as long as its tile; switching workspace drops it. The
  manual says so.
- **Unconsented read.** `state` is readable without consent like the rest of the catalog; it
  carries device text and a counter, nothing secret, and a masked field never reports one.
- **Reply-only two-state control refused.** It keeps the operator's position with "refused" beside
  it; without a state source there is no reported state to return to (R4 scopes the return to
  state-bound controls).

## Test & verification plan

- **Unit (ctest, runnable here):** `tst_output_state_binding`: AC2 (a reply carrying the previous
  token after a second send leaves the control waiting); AC8 (the requested state ends the window
  early for a toggle, a value and a text control, another state leaves it open); two controls on
  one token dataset never draw the same token; the skip rule; code 0 acknowledges and ends
  `pending`; a refusal ends `holdsDisplay`; an acknowledgement with a state source holds until the
  verdict matches, and until the window ends when it never does; silence becomes unanswered;
  `resetTokens` restarts at 1 but still skips the shown token. `tst_frame_serialization`: AC4 unit
  half. `tst_ui_automation`: `state` reported, masked item silent.
- **Integration (maintainer runs, API on, Pro build):** `tests/integration/test_acknowledged_commands.py`:
  - **AC1** (`gui`): a project whose CSV frame ends in token, code and message, a button whose
    transmit is `"BLINK " + token + "\n"`, Confirm Within 800 ms. The test clicks
    `dashboard.output.G.W.button`, reads the token from the captured payload and streams frames
    echoing it with code 0: `state.outcome` is `acknowledged` with that token. Again with code 3
    and `too hot`: `refused`, message `too hot`. Again without echoing: `unanswered` after the
    window, both with frames still flowing and with the device silent.
  - **AC3** (`gui`): a toggle with a state source and a reply. Acknowledge and then report the new
    state: `acknowledged` and `holding`, then `holding` false with the toggle `checked` as
    requested. Refuse: `holding` false at once and `checked` back to the reported state.
  - **AC4**: reply ids round-trip through `update` and `get` and a save and load; a pre-spec
    fixture loads with every control unbound.
  - **AC7** (`gui`): a toggle with a state source only, Confirm Within 800 ms. Click it, stop the
    simulator sending, and read `state.waiting` false within 1.8 s.
- **Maintainer observation:** AC5 with the example sketch on a board.
- **Hotpath:** AC6, `--benchmark-hotpath` before and after.
- **Static:** `scripts/syntax-check.py` on every edited C++ file; `code-verify.py --check`,
  `--tu-census --check`, `--singleton-census --check` (no new `instance()`); `layer-verify.py`;
  `term-verify.py`; `qt-cpp-review` and `qt-qml-review`; `sanitize-commit.py`.

## Cross-spec touchpoints (0109 merges first, 0110 second)

| Shared file | 0109 owns | 0110 adds on top |
|-------------|-----------|------------------|
| `Frame.h` / `Frame.cpp` / `FrameKeys.h` | `serialize` moved to the .cpp, input-kind fields, `admitOutputValue` | Reply fields and keys, `outputExpectsReply`, the widened `stateConfirmMs` write |
| `Base.h/.cpp` | Admission, `rangeMessage`, `presentation()` | Token, reply timer, outcome properties, `automationState` (including `outOfRange`) |
| `Panel.cpp`, `Preview.cpp` | `presentation()` | Reply read, token reset; reply cleared in the preview |
| `DashboardOutputPanel.qml` | Cell id, `idPrefix` to the numeric controls | Readout states, `automationState`, `idPrefix` to button and toggle |
| `ProjectEditorItemIds.h`, `EditorCommit.cpp` | Four ids and cases | Three ids appended after them, three cases |
| `ProjectOutputWidgetCommands.cpp`, `ProjectUpdateCommands.cpp` | Range fields | Reply fields |
| `tests/utils/device_simulator.py` | Capture mode | Uses it |
| `tst_frame_serialization.cpp`, `Output-Controls.md`, `API-Reference.md`, AI corpus, `dashboard.md` | Own sections | Own sections |

Owned by 0110 alone: `StateBinding`, `ElementCatalog`, `OutputStateRows`, `ProjectCheckers`,
`ProjectDryRunCommands`, `DashboardButton/Toggle.qml`, `automation.md`, the example.

## Decisions (approved 2026-11-03, maintainer)

All eight open questions closed on the recommended answer; R2, R6, R9 and the non-goals of
`spec.md` were amended, and R10, R11, AC7 and AC8 added.

1. **A1.** One token counter per reply-token dataset, kept in `StateBinding.cpp` rather than the
   `Base.cpp` the draft named, so the unit test reaches it.
2. **A2.** A refusal goes through `transmitError`; the readout keeps it until the next send.
3. **Outcome surface.** The catalog `state` object, no new command.
4. **Slug.** `waiting`.
5. **Acknowledged on a reply-only control.** Stays until the next send, muted.
6. **Problems.** A missing reply dataset is flagged inside `inspectStateBindings`.
7. **Reply window floor.** 100 ms, reply only.
8. **Dry runs.** Token 1.
9. **0080 defects.** "Waiting…" on a silent device (R10) and the confirmation half of 0080 R12
   (R11) are fixed here.

## Follow-ups (out of scope)

- A text field bound to a numeric dataset shows a stale string: the `m_datasets` copies are not
  string targets, so `value` is current only for text samples (`WidgetMapBuilder::buildValuePushes`).
- After R10, a control whose device went silent with no window open still reads "live" until a
  frame arrives, for the same reason R10 names; a periodic re-check would close it.
