---
spec: 0109-output-write-limits
title: Output Control Write Limits
status: approved     # approved 2026-11-02: refuse with the range shown, Step quantisation off by default
approved: 2026-11-02
created: 2026-11-02
author: Alex Spataru
---

# Spec 0109 — Output Control Write Limits

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

An output control's Min, Max and Step describe the control's geometry, not a contract with the
device. The slider and the knob cannot produce a value outside their range, so for them the two
coincide. The text field has no numeric notion at all: it hands whatever was typed to the
transmit script, so an operator who types `250` into a field bound to a 0 to 100 setpoint sends
`250`. The manual's own transmit examples clamp inside the script, which means the real limit
lives in code that the Project Editor, the preview, the API and the assistant cannot see, and
that each project author writes again.

The hobbyist case is a text field for a PWM duty or a servo angle beside a sketch that trusts its
input. The operator case is a setpoint field on a machine that does not. In both, the dashboard
is the last place a wrong number can be stopped, and today it does not try.

## Goals

- Every numeric output control enforces its range before the transmit script runs, on every way
  a value can enter it.
- A text field can be declared numeric, with the same Min, Max, Step and decimals a slider has.
- A refused value tells the operator what the allowed range is, without a modal and without a
  transmit.
- Step is honoured by keyboard and wheel increments, so an operator can nudge a setpoint without
  dragging.
- The range is visible wherever the control is described: the Project Editor, the preview, the
  API and the assistant.
- Existing projects behave exactly as today.

## Non-Goals

- No limits on raw sends. The console and the API's raw write stay unrestricted; they are the
  unrestricted path, not a control.
- No device-side limits, interlocks or read-back checks. Whether the device accepts the value is
  spec 0110's concern; this spec stops a value the project itself declared out of range.
- No change to what the slider and knob already do; they keep clamping.
- No per-dataset limits for incoming telemetry; alarm bands already cover that.
- No firmware-declared ranges. A device that declares its own controls and their limits is
  spec 0111, which reuses the Min, Max and Step fields this spec defines.

## Requirements

1. **R1 — Numeric text field.** A text field has an input kind: Text, which behaves exactly as
   today, or Number, which parses the typed value as a number and carries Min, Max, Step,
   decimals and units. A Number field whose input is not a number refuses it.
2. **R2 — Refusal.** A Number field value outside Min to Max is refused: nothing is transmitted,
   the field keeps the typed text for correction, and a message beside the control names the
   allowed range. The message clears on the next accepted value.
3. **R3 — Enforcement point.** The range is enforced before the transmit script, so a script
   never receives an out-of-range number from any control. This holds for pointer input,
   keyboard input, the driving surface of spec 0101 and any future way a value enters a control.
4. **R4 — Step.** Arrow keys and the mouse wheel move a slider, knob or Number field by one Step;
   with a modifier, by ten. A Number field has a Quantise to Step option, off by default; when
   it is on and Step is greater than zero, a typed value is quantised to Step before the range
   check.
5. **R5 — Preview.** The Project Editor preview exercises the limits: an out-of-range value
   shows the refusal the dashboard would show, and never produces bytes.
6. **R6 — Description.** The API surface that describes an output control reports its kind, Min,
   Max, Step and units, so a remote client or the assistant knows the range before driving it.
7. **R7 — Compatibility.** A project saved before this spec loads with every text field as Text
   and every other control unchanged.
8. **R8 — Manual.** The output controls page documents the numeric text field and the refusal,
   and its transmit examples stop clamping in script where the control now does it.

## Acceptance Criteria

- [ ] **AC1** — An integration test drives a Number text field through the driving surface with
  a value above Max and observes, on a loopback network Device, that no bytes arrive and the
  control reports a refusal; an in-range value arrives once, quantised to Step.
- [ ] **AC2** — An integration test reads a project with a Number field back over the API and
  finds kind, Min, Max, Step and units in the control's description.
- [ ] **AC3** — A project round trip keeps a Text field as Text and a Number field as Number;
  a pre-spec project loads with no change to its serialised form on save.
- [ ] **AC4** — Maintainer observation: typing `250` into a 0 to 100 Number field shows the
  range beside the control and the device receives nothing; the arrow keys nudge the slider by
  one Step.
- [ ] **AC5** — The manual's transmit examples contain no range clamp that the control performs.

## Constraints & Invariants

- The enforcement sits on the one path every control's value takes to its transmit script, so a
  new control kind inherits it. A second clamp in a specific control is a defect.
- Refusal never transmits, never takes a checkpoint and never blocks the dashboard.
- The rate limit between sends, the license gate on transmit scripts and the state feedback of
  spec 0080 are unchanged.
- One name per concept (spec 0100): the editor, manual and API say Min, Max, Step for every
  numeric control.

## Premise Check

- **Question:** Can a value outside a control's declared range reach the device today, and
  through which controls?
- **Measured:** Of the five control kinds, the slider and knob bound the value to Min to Max
  before it is held; the button and toggle carry no numeric value; the text field passes the
  typed string to the transmit script untouched, with no Min or Max in the editor. The manual
  shows two transmit scripts that clamp by hand, confirming authors are working around the gap.
- **Verdict:** The gap is real but narrow: one control kind plus the absence of a shared
  enforcement point. Worth doing as the small spec it is, and not worth more.

## Open Questions

None at the gate. The draft's two questions (clamp or refuse, quantise to Step by default) were
decided by the maintainer on 2026-11-02 and are folded into R2 and R4 above.
