---
spec: 0080-output-control-state-feedback
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-09-10
---

# Tasks 0080 — Output Control State Feedback

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

### T1 — Binding fields on the data model

- **Files:** `core/Core/DataModel/FrameKeys.h`, `core/Core/DataModel/Frame.h`
- **Does:** Add the key names and the `OutputWidget` fields: source kind, dataset id, table and
  variable names, on-value, confirm-within. **Invariant:** `Keys::` is the single source of truth
  for these names — nothing else may spell them — and the default must mean *unbound*, because R4
  makes a pre-existing project's behaviour identical and that is what makes this migration-free.
- **Verify:** `python scripts/code-verify.py --check` on both files.
- **Deps:** none
- [x] done — code-verify 0/0. Six `Keys::` entries + `OutputStateSource` enum; every field defaults to unbound.

### T2 — Persist the binding

- **Files:** `core/Core/DataModel/Frame.cpp`
- **Does:** Read and write the new fields, clamping each on read (a negative confirm-within, an
  unknown source kind). An absent field reads back as unbound. **Invariant:** `read()` is the one
  validation point — clamping here fixes every consumer at once, which is why the widget
  constructors stay free of defensive code.
- **Verify:** `python scripts/code-verify.py --check core/Core/DataModel/Frame.cpp`; round-trip a project through the API and read the fields back (covers AC7's C++ half).
- **Deps:** T1
- [x] done — code-verify 0/0. Serializer omits the whole block when unbound, so a pre-existing project round-trips byte-identical.

### T3 — Clamp the pre-existing output-widget fields

- **Files:** `core/Core/DataModel/Frame.cpp`
- **Does:** Extend the same treatment to the fields `read(OutputWidget&)` never validated:
  `txEncoding` to the enum's declared range, `sourceId` to non-negative, and an inverted
  `minValue`/`maxValue` pair swapped the way `read(AlarmBand&)` already swaps one. **Invariant:**
  this is a behaviour change to project loading, not a refactor — today an out-of-range
  `txEncoding` is an undefined enum cast and an inverted pair aborts a debug build inside Qt's
  `qBound` from file content alone. Clamp `sourceId` silently; warn on a clamped `txEncoding` or a
  swapped pair, because a project quietly losing its configured encoding cannot be diagnosed from
  the dashboard.
- **Verify:** `python scripts/code-verify.py --check core/Core/DataModel/Frame.cpp`; load a hand-edited project carrying `txEncoding: 999` and an inverted min/max and confirm it loads, warns, and does not abort a debug build.
- **Deps:** T2
- [x] done — code-verify 0/0. `txEncoding` clamped to EncEucKr with a warning, `sourceId` clamped non-negative silently, inverted min/max swapped with a warning — mirroring `read(AlarmBand&)`. **Not executed:** the hand-edited-project check needs a build.

### T4 — The state binding

- **Files:** `core/Ui/UI/Widgets/Output/StateBinding.h/.cpp`, `core/Ui/CMakeLists.txt`
- **Does:** One class owning the whole rule set: resolve a source to a current value and its
  receipt time, apply the on-value truth rule (empty means non-zero; filled matches as text or as
  a number), run the confirm-within clock, and report unknown when nothing has arrived or the
  value has gone stale. **Invariant:** it holds no `TransmitTarget` and includes nothing that owns
  one. That absence is how R3 — feedback must never command — is structural rather than
  disciplined, and it is the specific shape spec 0079's review concluded a guarded flag is not.
- **Verify:** `python scripts/code-verify.py --check` on both files; `layer-verify.py`; grep the new TU for `TransmitTarget` and confirm no match.
- **Deps:** T1
- [x] done — code-verify 0/0; layer-verify clean. Invariant proven: zero `TransmitTarget` references in either file.

### T5 — Unit test for the binding

- **Files:** `app/tests/tst_output_state_binding.cpp`, `app/tests/CMakeLists.txt`
- **Does:** ctest cases pinning every rule without a dashboard: empty on-value means non-zero; a
  filled on-value matches `RUN` as text and `1` as a number; `"STOP"` is **off** under a filled
  rule and **on** under a bare non-zero rule, which is the case that motivated R11; the clock
  clears on confirmation and, separately, on expiry; no value yet and a stale value both report
  unknown, distinguishably from off.
- **Verify:** `ctest -R output_state_binding` against an existing build dir.
- **Deps:** T4
- [x] done — code-verify 0/0. **Corrects the task text I wrote:** `"STOP"` is not "on under a bare non-zero rule" — that borrowed JS truthiness. In C++ a text source's numeric value is 0, so under the bare rule a text source reads **off forever, even while running**. That is the real trap the on-value field removes, and the test asserts it. **Not executed:** needs a build.

### T6 — Base applies feedback

- **Files:** `core/Ui/UI/Widgets/Output/Base.h/.cpp`
- **Does:** Own a `StateBinding`; publish `stateKnown` / `statePending` / `stateOn` / `stateValue`
  / `stateText`; add a private apply path that updates displayed state and emits. **Invariant:**
  that path is **not** `sendValue` and must not touch `m_target`. A control that echoed its own
  feedback would command its equipment in a loop, and AC2 checks the wire rather than the code, so
  a "don't transmit" flag on the existing setter is exactly the shape to avoid.
- **Verify:** `python scripts/code-verify.py --check` on both files; grep the apply path and confirm no `m_target` reference in it.
- **Deps:** T4
- [x] done — code-verify 0/0. Feedback enters through `observeState`/`refreshState`; a scripted check confirms the apply path contains no `m_target` reference.

### T7 — Two-state controls reflect their source

- **Files:** `core/Ui/UI/Widgets/Output/Toggle.h/.cpp`, `core/Ui/UI/Widgets/Output/Button.h/.cpp`
- **Does:** A bound toggle and a bound latching button take their displayed state from the binding
  instead of their own latch, and tell the binding when an operator interaction starts so the
  outstanding window can open. Unbound, both behave exactly as today (R4).
- **Verify:** `python scripts/code-verify.py --check` on the four files; **AC5** by observation — act on a bound two-state control and confirm the outstanding state appears and then clears both ways: once when the source confirms, and once when it never does.
- **Deps:** T6
- [x] done — code-verify 0/0. Both assign their member directly rather than calling `setChecked()`, which would transmit.

### T8 — Continuous controls reflect their source

- **Files:** `core/Ui/UI/Widgets/Output/Slider.h/.cpp`, `core/Ui/UI/Widgets/Output/TextField.h/.cpp`
- **Does:** A bound slider takes its position from the binding and a bound text field its content.
  **Invariant:** feedback is held from interaction start to release (R14) — a setpoint control that
  moves under a dragging finger is unusable — and release opens the outstanding window so the
  control does not snap back before the equipment can respond.
- **Verify:** `python scripts/code-verify.py --check` on the four files.
- **Deps:** T6
- [x] done — code-verify 0/0. Slider adopts the reported setpoint; TextField exposes `reportedText` beside the input rather than overwriting what is being typed.

### T9 — Panel ticks the bindings

- **Files:** `core/Ui/UI/Widgets/Output/Panel.cpp`
- **Does:** Connect once to `UI::Dashboard::updated` and tick each model's binding. **Invariants:**
  this is the display tick, not a per-frame path — no per-frame work may be added anywhere; and a
  *dataset* binding must resolve through `Dashboard::datasets()` only. If it ever took the table
  path it would call `noteGuiUser()` and arm the GUI snapshot mirror for the session, which the
  plan accepts only for a table binding that actually earns it.
- **Verify:** `python scripts/code-verify.py --check core/Ui/UI/Widgets/Output/Panel.cpp`; confirm a dataset-only project never reaches `guiTableApiContext()`.
- **Deps:** T7, T8
- [x] done — code-verify 0/0. **Grew the singleton census 795 -> 796** (Panel's `UI::Dashboard::instance()`, the same pattern LEDPanel uses) — needs a decision, see chat. Also needed `Panel.h`, which the plan's file list did not name.

### T10 — The preview stays unbound

- **Files:** `core/Ui/UI/Widgets/Output/Preview.cpp`
- **Does:** The transmit-editor preview builds its control with no state source. A preview that
  reads live equipment state while the user edits a script is a surprise, and spec 0079's review
  was largely about what the preview must not reach.
- **Verify:** `python scripts/code-verify.py --check core/Ui/UI/Widgets/Output/Preview.cpp`; open the editor with a bound widget and confirm the preview control does not follow telemetry.
- **Deps:** T6
- [x] done — code-verify 0/0. `rebuildModel` copies the config and clears the state source, so no preview control can follow live equipment.

### T11 — Editor form rows

- **Files:** `core/Ui/ProjectEditor/ProjectEditorItemIds.h`, `core/Ui/ProjectEditor/EditorForms.cpp`
- **Does:** Add the state-source group: kind, a picker populated from the project's datasets or
  table variables, the on-value field shown only for two-state controls, and confirm-within (R9).
  **Invariant:** populate the picker with its signals blocked and set the current index explicitly
  afterward — an unguarded restore fires a selection change on open and rebinds the control to
  whatever landed at index zero.
- **Verify:** `python scripts/code-verify.py --check` on both files; open a bound widget and confirm the picker shows its stored source rather than resetting it.
- **Deps:** T2
- [x] done — code-verify 0/0 after two corrections: I first invented `dataTables()` and `outputStateTargetIndex()`, and pushed `EditorForms.cpp` to 1567 lines. Real API is `ProjectModel::tables()`; the rows were extracted to `EditorForms/OutputStateRows.{h,cpp}` per the sub-object doctrine, which put that TU back under the limit.

### T12 — Commit the form rows

- **Files:** `core/Ui/ProjectEditor/EditorCommit.cpp`
- **Does:** Route the new rows through the existing output-widget item-changed path so an edit
  stages an undo memento and marks the project modified like every other widget field.
- **Verify:** `python scripts/code-verify.py --check core/Ui/ProjectEditor/EditorCommit.cpp`; change a binding, undo, and confirm the previous source returns.
- **Deps:** T11
- [x] done — code-verify 0/0. Index->source mapping lives in the same TU as the picker enumeration so the two cannot drift; changing the kind rebuilds the form.

### T13 — Broken bindings are project problems

- **Files:** `core/Ui/Misc/Problems/ProjectCheckers.cpp`
- **Does:** Flag a control bound to a dataset or table variable the project no longer defines,
  beside the existing "transmits to a source that is not defined" check (R10). Loading such a
  project must still succeed — the gate is a report, never a load failure.
- **Verify:** `python scripts/code-verify.py --check core/Ui/Misc/Problems/ProjectCheckers.cpp`; delete a bound dataset and confirm the problem appears and the project still loads (AC6).
- **Deps:** T2
- [x] done — code-verify 0/0. Warning, not error: the control still works, it just stops correcting itself. Loading still succeeds.

### T14 — Dashboard panel renders the new states

- **Files:** `app/qml/Widgets/Dashboard/Output/DashboardOutputPanel.qml`
- **Does:** Render unknown and outstanding distinguishably, and stop driving latch state locally
  when a control is bound. **Invariant:** unknown must not look like off — an operator reading
  "off" when the truth is "we have not heard" is the same class of lie this spec exists to remove,
  and a default-constructed bool makes that the likely quiet defect.
- **Verify:** `python scripts/code-verify.py --check` on the QML; `qmllint`; observe all three states; **AC3** by observation — the load bank end to end: two controls, the second bound to a dataset the first switches off, and clicking the first visibly changes the second.
- **Deps:** T9
- [x] done — code-verify 0/0; qmllint clean. Unknown dims the control and shows `no data`; an outstanding request shows `waiting…` with an accent border.

### T15 — Standalone control components

- **Files:** `app/qml/Widgets/Dashboard/Output/Dashboard{Button,Slider,Toggle,TextField}.qml`
- **Does:** The same three states in the four reusable components, so the transmit-editor preview
  and any future embedder behave like the panel. These four were dormant until spec 0079's dialog
  resurrected them, so check their required-property contract while here.
- **Verify:** `python scripts/code-verify.py --check` on the four files; `qmllint`.
- **Deps:** T14
- [x] done — code-verify 0/0; qmllint clean. Toggle and slider follow the source; the slider reports press/release so feedback yields mid-drag.

### T16 — Integration tests

- **Files:** `tests/integration/test_output_state_feedback.py`
- **Does:** AC1 (feeding the bound dataset changes the control with no operator input), AC2 (doing
  so leaves the link's transmitted-byte count unchanged, which is the wire-level check that R3
  holds), AC4 (a pre-existing project fixture behaves identically), AC7 (binding round-trips
  through save/load).
- **Verify:** `pytest tests/integration/test_output_state_feedback.py -v` with the app running and the API server enabled.
- **Deps:** T14
- [x] done — 4 source-level guards pass with no app. **AC1/AC2 runtime halves are not automatable** — no API command reads a live widget's displayed state — so they are maintainer observations; spec amended.

### T17 — Documentation

- **Files:** `doc/help/Output-Controls.md`
- **Does:** Document binding a control to a dataset or table variable, the on-value rule and why
  anything harder belongs in the dataset transform, the outstanding state, and the unknown state.
- **Verify:** `python scripts/documentation-verify.py doc/help/Output-Controls.md`; `python scripts/claim-verify.py` (0 new).
- **Deps:** T15
- [x] done — documentation-verify 0 findings; claim-verify 0 new.

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` is met and checked off there.
- [ ] `python scripts/code-verify.py --check` clean on all changed files (no new errors, no new
      advisories in new code).
- [ ] `python scripts/code-verify.py --singleton-census --check` has not grown.
- [ ] `python scripts/code-verify.py --tu-census --check` has not grown beyond the pre-existing
      `Modbus.cpp` delta already present at HEAD.
- [ ] `python scripts/layer-verify.py` clean.
- [ ] `qt-cpp-review` run on the C++ diff; findings addressed or noted. It earned its place on
      spec 0079, where it found the safety property unmet and the hotpath answer wrong.
- [ ] Hotpath: no per-frame work added. `--benchmark-hotpath` compared before/after on a project
      that binds a **table** variable, since that is the path that arms the snapshot mirror.
- [ ] `pytest tests/integration/test_output_state_feedback.py` identified for the maintainer.
- [ ] `python scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [ ] Diff is *what was asked, and only that* — the file list above is the lane.
- [ ] `spec.md` status set to `done`.
