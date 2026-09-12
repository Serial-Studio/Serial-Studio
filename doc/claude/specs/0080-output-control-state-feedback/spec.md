---
spec: 0080-output-control-state-feedback
title: Output Control State Feedback
status: done          # closed 2026-09-12: maintainer ran the build/run gates and closed
created: 2026-09-10
author: Alex Spataru
---

# Spec 0080 — Output Control State Feedback

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

An output control shows the state the **user last set**, never the state the **equipment is
actually in**. A toggle latches when it is clicked, a slider stays where it was dragged, a
latching button keeps its pressed look — all of it local UI state that has never been checked
against a single byte of telemetry.

The maintainer's case is a load bank. The bank and its cooling fan each have a control. Stopping
the bank also stops the fan, because the fan is interlocked in the equipment, not in Serial
Studio. The fan's control never hears about it: it still reads "Fan: started" while the fan is
stopped. An operator looking at that dashboard is being told something false about a machine
that is running, and the more faithful the rest of the dashboard is, the more they will trust it.

The general shape is any equipment where state changes for a reason other than the operator's
last click: an interlock, a local HMI, a safety trip, a watchdog reset, a second Serial Studio
instance, or simply a command that was sent and refused. Today every one of those desynchronises
the control silently and permanently, and the only way back is for the operator to notice, guess,
and click twice. Output controls are the one dashboard element that does not read telemetry, and
that gap is exactly where the wrong information lives.

## Goals

- A control can be bound to a telemetry value, and thereafter shows what the equipment reports
  rather than what the operator last requested.
- A control whose equipment changes state behind the operator's back updates itself.
- The operator can tell the difference between "the equipment reports this" and "I asked for this
  and it has not happened yet".
- Binding is optional; an unbound control behaves exactly as it does today.
- Binding never creates a command loop: reflecting telemetry is not an operator action.

## Non-Goals

- No new transport, no new telemetry. The bound value is one the project already receives.
- No control logic. This reflects state; it does not implement interlocks, retries, or
  confirmation protocols.
- No change to the transmit function contract. Feedback is about what a control *shows*, not
  about what it sends.
- No history or trending on the control. The current value is the whole feature.
- Not a replacement for a readback indicator widget. A project that wants a separate lamp showing
  fan state can still have one; this is about the control itself not lying.

## Requirements

1. **R1** — An output control may optionally name a **state source**: a dataset the project already
   defines, or a variable in a data table.
2. **R2** — While bound, the control's displayed state follows its state source, regardless of what
   the operator last did.
3. **R3** — Applying a state-source value to a control **never transmits**. Reflecting equipment
   state is not an operator action, and a control that echoed its own feedback would command the
   equipment in a loop.
4. **R4** — A control with no state source behaves exactly as it does today, including projects
   written before this feature exists.
5. **R5** — Between the operator acting and the state source confirming, the control shows that the
   request is outstanding, distinguishably from both settled states.
6. **R6** — If the state source does not confirm within a bounded time, the control stops showing
   the request as outstanding and returns to displaying what the source reports.
7. **R7** — A control whose state source has produced no value yet, or whose source has gone stale
   (the link dropped, the dataset stopped arriving), shows that its state is unknown rather than
   silently showing a default.
8. **R8** — The mapping from a source value to a control's displayed state is explicit per control
   type, and configurable where a single interpretation would be wrong: a toggle and a latching
   button need a truth rule, a slider and a knob need a value, a text field needs text.
9. **R9** — The Project Editor lets the user pick a state source from what the project actually
   defines, rather than typing an identifier, and shows plainly when a control is unbound.
10. **R10** — Binding survives a project save/load round trip, and a project whose bound dataset was
    later deleted loads without error and reports the broken binding as a project problem.
11. **R11** — A control that shows a two-state condition carries an optional **on-value**. Left
    empty, any non-zero source value means on. Filled, the control is on exactly when the source
    equals it, compared as text or as a number so both `RUN` and `1` work. Anything needing a
    threshold, a bitmask or hysteresis is expressed in the dataset's existing transform rather
    than here, so this feature adds no new place for a user to write code.
12. **R12** — Each bound control carries its own **confirm-within** time, with a default. The
    outstanding state clears when the source confirms or when that time elapses, whichever comes
    first.
13. **R13** — A state source is either a dataset the project defines or a variable in a data table,
    chosen per control.
14. **R14** — While the operator is interacting with a continuous control, feedback does not move
    it. On release the outstanding window begins, after which the control re-latches to the
    source.

## Acceptance Criteria

- [x] **AC1** — With a toggle bound to a dataset, injecting a value that means "off" turns the
      toggle off without the operator touching it, and injecting "on" turns it on. *Amended
      2026-09-10: no API command reads a live widget's displayed state, so this is a maintainer
      observation.* The rule behind it is unit-tested (`tst_output_state_binding`).
- [x] **AC2** — Applying a state-source value transmits nothing. *Amended 2026-09-10: the API
      exposes no transmitted-byte counter, so the runtime form is a maintainer observation.*
      Replaced by three source-level guards that hold for control types not yet written:
      `StateBinding` contains no transmit target, `Base::refreshState` contains no `m_target`
      reference, and no control's `applyStateVerdict` calls its own transmitting setter
      (`tests/integration/test_output_state_feedback.py`, runs without the app).
- [x] **AC3** — The load-bank case end to end: two controls, the second bound to a dataset the
      first switches off, and clicking the first visibly changes the second. (Maintainer
      observation on a real project.)
- [x] **AC4** — A control with no state source behaves identically to the current build, and a
      project file written before this feature loads unchanged. (`pytest tests/integration/` on a
      pre-existing project fixture.)
- [x] **AC5** — Acting on a bound control shows the outstanding state, and it clears both when the
      source confirms and, separately, when the source never confirms. (Maintainer observation.)
- [x] **AC6** — A project whose bound dataset was deleted loads, shows the control, and raises the
      broken binding in the Problems list. (Maintainer observation.)
- [x] **AC7** — Binding round-trips with the same source selected
      (`test_output_state_feedback.py::test_binding_round_trips`). Requires the running app.

## Constraints & Invariants

- **Feedback must not command.** R3 is the deciding constraint: any path where displaying a value
  can reach the transmit path is a defect, not a tuning issue. A control that echoes its own
  feedback drives the equipment.
- Must not regress the acquisition hotpath gate. Whatever delivers values to controls runs at
  display rate, not per frame, and adds no per-frame work to the pipeline.
- Output widgets are a Pro feature; this inherits that gating.
- Must not add a new input to a cached hotpath flag without wiring its refresh — the class of
  silent breakage the dataflow doc names.
- Project schema changes are additive, with the absent case meaning "unbound", so no migration is
  required and older builds ignore the field.
- The dashboard already knows dataset values at display rate; this feature consumes what exists
  rather than introducing a second delivery path for the same numbers.

## Resolved Decisions

Settled with the maintainer on 2026-09-10 and carried as R11 to R14. The reasoning is kept here
because it constrains the plan.

- **Match value, defaulting to a non-zero test (R11).** A plain non-zero rule reads `"STOP"` as
  on, because a non-empty string is truthy. An expression per control would add a fourth user
  scripting surface to a product that has just spent this cycle proving how much each one costs to
  make safe. The match value covers the equipment cases directly, and everything harder already
  has a home in the per-dataset transform.
- **Per-control confirm-within, with a default (R12).** A contactor and a fan differ by two orders
  of magnitude, so any single global number is wrong for one of them. Fidelity to specific
  equipment is what the feature exists for.
- **Datasets and table variables together (R13).** The stored field has the same shape either way,
  so splitting the work means a second schema change and a second pass over the picker. The real
  cost is one additional delivery path: datasets already arrive at display rate, table variables
  come from the table store.
- **Feedback yields to the operator (R14).** A setpoint control that moves under a dragging finger
  is unusable, and suppressing feedback for the duration of an interaction is what HMI setpoint
  controls do.

## Open Questions

- None blocking.
