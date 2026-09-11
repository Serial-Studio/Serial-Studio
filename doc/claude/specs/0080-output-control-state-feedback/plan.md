---
spec: 0080-output-control-state-feedback
phase: plan
status: approved     # draft -> approved (gate before /ss-tasks)
updated: 2026-09-10
---

# Plan 0080 — Output Control State Feedback

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Read the relevant `doc/claude/` sub-docs and the *actual code*
> before writing this — a plan grounded in a stale mental model is worse than no plan.
> Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

A bound control reads its source on the display tick and nothing else changes. The dashboard
already delivers every dataset's current value to the GUI side: `WidgetMapBuilder` registers each
entry of the `m_datasets` map as a value-push target, so `Dashboard::datasets()` carries live
numbers, and a bound control needs one lookup by unique id per tick. A table-variable source uses
the sanctioned `readTableView` routing, which on the GUI thread reads the mirror snapshot rather
than marshalling. The resolution, the truth rule and the confirm-within clock live in one small
sub-object that each `Output::Base` owns, so the four control classes gain state rather than
logic, and the *only* way a value can reach a control is a setter that does not transmit —
which is how R3 is enforced structurally rather than by discipline.

## Affected subsystems & files

| File | Change |
|------|--------|
| `core/Core/DataModel/FrameKeys.h` | Five new key names for the binding fields, beside the existing `OutputInitialValue` / `OutputColor` group. |
| `core/Core/DataModel/Frame.h` | `OutputWidget` gains `stateSource` (kind), `stateDatasetId`, `stateTable`, `stateVariable`, `stateOnValue`, `stateConfirmMs`. Absent means unbound (R4), so no migration. |
| `core/Core/DataModel/Frame.cpp` | Read/write the new fields with clamping, **and clamp the existing unvalidated ones** — `txEncoding` to the enum's declared range, `sourceId` to non-negative, and an inverted `minValue`/`maxValue` pair — the way `read(AlarmBand&)` already does. |
| `core/Ui/UI/Widgets/Output/StateBinding.h/.cpp` | **New.** Resolves a source to a current value, applies the on-value truth rule (R11), owns the confirm-within clock (R12) and the unknown/stale verdict (R7). One class, one concern, in the directory named after its facade (spec 0070). |
| `core/Ui/UI/Widgets/Output/Base.h/.cpp` | Owns a `StateBinding`; publishes `stateKnown` / `statePending` / `stateOn` / `stateValue` / `stateText`; gains a feedback-apply entry point that is **not** `sendValue` and cannot reach the target. |
| `core/Ui/UI/Widgets/Output/Button.h/.cpp` | A latching button reflects the bound state instead of its own latch. |
| `core/Ui/UI/Widgets/Output/Toggle.h/.cpp` | Same, plus suppression while the operator is acting. |
| `core/Ui/UI/Widgets/Output/Slider.h/.cpp` | Reflects the bound value, suppressed for the duration of a drag (R14). |
| `core/Ui/UI/Widgets/Output/TextField.h/.cpp` | Reflects the bound text when bound. |
| `core/Ui/UI/Widgets/Output/Panel.cpp` | Connects the panel to `UI::Dashboard::updated` and ticks each model's binding once per tick. |
| `core/Ui/UI/Widgets/Output/Preview.cpp` | The preview builds an unbound control: a preview must not read live equipment state, and spec 0079's judging surface already says why. |
| `core/Ui/ProjectEditor/ProjectEditorItemIds.h` | Parameter ids for the new form rows. |
| `core/Ui/ProjectEditor/EditorForms.cpp` | Build the state-source rows: kind, source picker populated from the project, on-value, confirm-within (R9). |
| `core/Ui/ProjectEditor/EditorCommit.cpp` | Commit the new rows through the existing output-widget item-changed path. |
| `core/Ui/Misc/Problems/ProjectCheckers.cpp` | Flag a control bound to a dataset or variable the project no longer defines (R10), next to the existing "transmits to a source that is not defined" check. |
| `app/qml/Widgets/Dashboard/Output/DashboardOutputPanel.qml` | Render unknown and outstanding states; stop driving latch state locally when bound. |
| `app/qml/Widgets/Dashboard/Output/Dashboard{Button,Slider,Toggle,TextField}.qml` | Same, for the components the transmit preview and any future embedder use. |
| `app/tests/tst_output_state_binding.cpp` | **New.** The truth rule, the confirm-within clock and the unknown/stale verdict, unit-tested without a dashboard. |
| `app/tests/CMakeLists.txt` | Register the unit target. |
| `core/Ui/CMakeLists.txt` | Add `StateBinding.cpp`. |
| `tests/integration/test_output_state_feedback.py` | **New.** AC1, AC2, AC4, AC7. |
| `doc/help/Output-Controls.md` | Document binding, the truth rule, the outstanding state and the unknown state. |

## Architecture & data flow

**Delivery.** `Panel` already builds every control and already lives on the GUI thread. It
connects once to `UI::Dashboard::updated` — the display tick every other widget uses
(`LEDPanel.cpp:47` is the pattern) — and calls `tick()` on each model's binding. A dataset source
resolves through `Dashboard::datasets()`, a `QMap<int, Dataset>` keyed by unique id whose entries
are registered value-push targets in `WidgetMapBuilder::buildDatasetReferences`, so the number is
current. A table-variable source resolves through `readTableView` on the context
`FrameBuilder::guiTableApiContext()` hands out, which on the GUI thread reads the mirror snapshot.

**Applying.** `StateBinding::tick()` produces a verdict — unknown, or a value plus whether it is
on under the truth rule — and `Base` applies it through a private setter that updates displayed
state and emits. That setter is deliberately *not* `sendValue`, and `StateBinding` holds no
`TransmitTarget`, so there is no path from feedback to the wire. That is R3 made structural, the
same shape spec 0079 arrived at after its review found a promise that discipline alone had not
kept.

**Outstanding.** An operator action stamps the binding with what was requested and starts the
confirm-within clock. While it runs, `statePending` is true and feedback does not overwrite the
displayed state. It ends when the source reports the requested state, or when the clock expires,
whichever comes first (R5, R6) — after which the source wins unconditionally.

**Interaction suppression.** The QML control tells the binding when an interaction begins and
ends; between those, feedback is held (R14). Release starts the outstanding window, so a released
slider does not snap back before the equipment has had a chance to move.

## Hotpath & threading impact

- **Touches the hotpath?** **No — with one honest qualifier, stated because the equivalent claim
  in plan 0079 was wrong and had to be corrected after review.** No pipeline code is edited, and
  no per-frame work is added: the value a bound control reads is one the dashboard already
  propagates to the GUI side for its own widgets. The qualifier: a **table-variable** binding
  reaches `FrameBuilder::guiTableApiContext()`, which calls `noteGuiUser()` and therefore arms the
  GUI snapshot mirror for the session. That is a real, permanent per-tick cost, it is exactly the
  latch spec 0079's judging surface was changed to avoid paying for a throwaway engine — and here
  it is **correct**, because a table-bound control is a genuine, long-lived GUI reader and the
  mirror is what it is for. A dataset binding arms nothing. The plan does not pretend this is free;
  it asserts the cost is earned, and only when a project actually binds a table variable.
- **New cross-thread signal/slot?** **No.** Everything added lives on the GUI thread. The table
  read uses `readTableView`, which takes the mirror branch on the GUI thread and never marshals
  once a snapshot exists.
- **New input to a cached hotpath flag?** **Yes, for table bindings only** — `noteGuiUser()`, as
  above. No new flag is introduced and no existing flag gains an unwired input.
- **Timestamp ownership** — unchanged; feedback consumes values the source already stamped and
  stamps nothing.

## Data model & persistence

Six additive fields on `OutputWidget`, with `stateSource == None` as the default, so a project
written before this feature reads back identical and an older build ignores what it does not
know (R4). New `Keys::` entries are the single source of truth for the names.

`read(OutputWidget&)` today validates only `type` and `size`. The new fields are clamped on read,
and **the existing unvalidated ones are clamped too** (maintainer's call, 2026-09-10): an
out-of-range `txEncoding` currently reaches `static_cast<SerialStudio::TextEncoding>` as undefined
behaviour, a negative `sourceId` trips `SS_ASSERT_LOG` on the script-environment path, and an
inverted `minValue`/`maxValue` pair reaches Qt's `qBound`, whose `Q_ASSERT(!(max < min))` aborts a
debug build from file content alone. The sibling `read(AlarmBand&)` twenty lines below already
swaps an inverted pair, so this closes a gap rather than inventing a policy. Clamping is silent by
design for the new fields and for `sourceId`; an out-of-range `txEncoding` or a swapped pair is
worth a warning, because a project quietly losing its configured encoding is hard to diagnose from
the dashboard. This is a behaviour change to project loading, so it carries its own task and its
own test rather than riding along inside the feature work.

## API / SDK surface

No new command. `project.outputWidget.get` / `.update` carry the new fields automatically as part
of the widget object; the generated dataset-property artifacts are untouched because this is an
output widget, not a dataset. `EnumLabels` gains slugs for the state-source kind so the API and the
assistant name it the same way the UI does.

## QML / UI

The controls gain three display states where they had one: settled-from-source, outstanding, and
unknown. Unknown must be visibly distinct from off — an operator reading "off" when the truth is
"we have not heard" is the same class of lie this spec exists to remove. The Project Editor grows
a state-source group on the output-widget form: kind, a picker populated from the project's
datasets or table variables, the on-value field (shown only for two-state controls), and
confirm-within. The picker follows the existing combo restore-race guard.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Value delivery | (a) tick-read from `Dashboard::datasets()`; (b) register the control as a per-sample push target; (c) expose the source to the transmit script | **(a)** — the value is already on the GUI side, and (b) wires a control into the per-sample propagation table to deliver updates faster than it can render. (c) re-adds a script surface the 0079 review just finished containing. |
| Where feedback is applied | (a) a private setter on `Base` that cannot reach the target; (b) reuse `sendValue` with a "don't transmit" flag | **(a)** — R3 is the deciding constraint, and a flag is exactly the shape that failed review in 0079. No path, not a guarded path. |
| Truth rule location | (a) in `StateBinding`; (b) in each control | **(a)** — one rule, unit-testable without a dashboard, and the four controls stay dumb. |
| Outstanding clock owner | (a) `StateBinding`; (b) QML | **(a)** — QML would have to own a timer per control and the rule would live in four files. |
| Table binding in pass one | (a) yes; (b) datasets only | **(a)** per spec R13 — but it is the half that arms the mirror, so if the plan is trimmed under time pressure this is the piece to defer, not the dataset path. |
| Preview behaviour | (a) preview builds an unbound control; (b) preview honours the binding | **(a)** — a preview that reads live equipment state while you edit a script is a surprise, and 0079 spent its review learning what the preview should not reach. |

## Risks & mitigations

- **Feedback loop.** The defining failure: a control that transmits on feedback commands its own
  equipment. Mitigated structurally — `StateBinding` holds no target and the apply path is not
  `sendValue` — and asserted by AC2, which watches the wire rather than trusting the code.
- **Unknown rendered as off.** The most likely *quiet* defect, because a default-constructed bool
  is false. The unknown state gets its own rendering and its own acceptance criterion.
- **Mirror arming.** Named in the hotpath section rather than discovered later; a dataset binding
  must not accidentally take the table path and arm it.
- **Stale-source detection.** "The link dropped" and "this value legitimately has not changed" look
  identical from a value alone; the binding needs a received-time, not a value comparison.
- **Form growth.** Four more rows on a widget form that already carries about a dozen. The on-value
  row is shown only for two-state controls to keep the common case unchanged.

## Test & verification plan

- **Unit (runnable here):** `app/tests/tst_output_state_binding.cpp` via `ctest` — empty on-value
  means non-zero; a filled on-value matches as text and as a number so `RUN` and `1` both work;
  `"STOP"` is off under a filled rule and on under a bare non-zero rule, which is the case that
  motivated R11; the confirm-within clock clears on confirmation and on expiry; no value yet and a
  stale value both report unknown.
- **Integration (maintainer runs the app, API server enabled):**
  `tests/integration/test_output_state_feedback.py` — **AC1** feed the bound dataset and read the
  control's state back; **AC2** with a source connected, feeding the bound dataset leaves the
  transmitted-byte count unchanged; **AC4** a pre-existing project fixture loads and behaves
  identically; **AC7** binding round-trips through save/load.
- **Maintainer observation:** AC3 (the load bank, two controls, one switching the other off), AC5
  (outstanding clears both ways), AC6 (deleted source raises a Problems entry).
- **Hotpath:** not touched; `--benchmark-hotpath` remains a CI gate and must not move. Worth one
  before/after comparison anyway on a project that binds a table variable, since that is the path
  that arms the mirror.
- **Static:** `code-verify.py --check` clean on every touched file including the new advisories
  rule; `layer-verify.py`; singleton census must not grow; `qt-cpp-review` before handoff —
  it earned its place last time; `sanitize-commit.py` before commit.
