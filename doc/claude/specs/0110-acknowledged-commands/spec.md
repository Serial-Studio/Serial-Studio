---
spec: 0110-acknowledged-commands
title: Acknowledged Commands
status: approved     # approved 2026-11-02: reply datasets, per-control integer token, no notification
approved: 2026-11-02
created: 2026-11-02
author: Alex Spataru
---

# Spec 0110 — Acknowledged Commands

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

An output control transmits and forgets. Spec 0080 taught controls to reflect what the
equipment reports, with an outstanding window after a click; when the window expires the control
falls back to the reported state. That makes a control stop lying about state, but it leaves
three outcomes indistinguishable: the device applied the command slowly, the device refused it,
and the device never received it. A refusal with a reason, "setpoint above limit", "interlock
active", "busy", has no channel at all. The firmware can print it to the console, but the control
that sent the command never learns, and the operator sees a button that settles back as if
nothing happened.

For a sketch on a desk this is the difference between a dashboard that sends and one that
converses: a button whose sketch answers "OK" or "no, too hot" is a tool rather than a demo. For
an operator it closes the one gap spec 0080 named and declined: a command that was sent and
refused.

## Goals

- A control may expect an explicit reply from the device, and shows whether the command was
  acknowledged, refused with the device's reason, or unanswered within a timeout.
- The reply travels through the frames the project already parses; no new transport, no second
  parser, no new wire format imposed on anyone.
- The convention is small enough for a sketch to implement in one `print` line, and is delivered with
  an example that does.
- A late or stale reply never acknowledges a newer command.
- A remote client can learn the outcome of a command it caused.
- Controls without a reply configured behave exactly as today, spec 0080 included.

## Non-Goals

- No retries, queuing or guaranteed delivery. One send, one reply or a timeout.
- No device-side library beyond an example sketch.
- No replacement for state feedback. An acknowledgement means the device accepted the request;
  the reported state means the equipment is there. A control may use both.
- No change to the transmit function's contract beyond making a correlation token available.
- No acknowledgement for raw sends from the console or the API's raw write.
- No notification on refusal. The control and the transmit error log carry it; the annunciator
  is for alarms.
- No device-side library of its own. The firmware library of spec 0111 is the reference client
  for the reply convention; this spec defines the convention and the app side.

## Requirements

1. **R1 — Reply source.** A control may name where its reply arrives: datasets in the project
   that the firmware updates with the result of the most recent command, carrying at least a
   correlation token and a result code, and optionally a message. The Project Editor offers the
   project's datasets for each role rather than free-typed identifiers.
2. **R2 — Correlation.** Every send carries a token the transmit function can include in the
   payload: a small integer per control that increments on every send and resets on connect. A reply counts only when its token matches the control's most recent send; a reply
   carrying an older token is ignored.
3. **R3 — Outcomes.** Within the control's timeout, a reply with a zero result code shows the
   command as acknowledged; a non-zero code shows it as refused and displays the message, or
   the code when there is none; no matching reply shows it as unanswered. The three states are
   visually distinct from each other and from the settled states of spec 0080.
4. **R4 — Together with state feedback.** An acknowledgement ends the outstanding window early;
   a refusal ends it and the control returns to the reported state; a control with both a reply
   source and a state source shows acknowledged until the state source confirms.
5. **R5 — Timeout.** The timeout is per control and reuses the outstanding window of spec 0080,
   so one number governs both.
6. **R6 — Where refusals go.** A refusal is also recorded where transmit errors are shown, so an
   operator who looked away still finds it, and it is observable over the API as the control's
   last outcome.
7. **R7 — Remote outcome.** A client that drives a control through the API or the driving
   surface can read that control's outcome for the send it caused, including the message.
8. **R8 — Convention and example.** The manual documents one reply convention for text frames,
   and a UART example sketch is included with it: the sketch echoes the token and answers with a code
   and a short reason for an out-of-range request.
9. **R9 — Compatibility.** A control with no reply source behaves exactly as before this spec; a
   project saved without the properties loads unchanged.

## Acceptance Criteria

- [ ] **AC1** — An integration test with a loopback network Device plays the firmware: it
  receives the control's payload, extracts the token and replies in a frame; the control reads
  acknowledged over the API. The same test replies with a non-zero code and a message and reads
  refused with that message; and does not reply and reads unanswered after the timeout.
- [ ] **AC2** — A unit test feeds a reply carrying the previous token after a second send and
  confirms the control stays outstanding for the newer one.
- [ ] **AC3** — An integration test binds both a reply source and a state source and observes
  acknowledged followed by the settled state, and refused followed by the reported state.
- [ ] **AC4** — A project round trip keeps the reply configuration; a pre-spec project loads
  with every control unbound.
- [ ] **AC5** — Maintainer observation with the example sketch on a board: pressing the button
  shows acknowledged; sending a value the sketch refuses shows the sketch's reason on the
  control.
- [ ] **AC6** — The hotpath gate (`--benchmark-hotpath`) reports no regression; reply datasets
  are ordinary datasets on the acquisition pipeline.

## Constraints & Invariants

- Replies ride the existing frame path and are read at display-tick rate; no per-frame signal
  from the pipeline to a control, no allocation on the pipeline for a reply.
- Reflecting a reply is not an operator action and never transmits (spec 0080 R3 holds).
- The transmit script's license gate and the rate limit are unchanged; the token is available
  to the script but never required by it.
- The remote gate's capability classification and the sealed-session policy cover any new API
  surface this adds (specs 0096, 0098).
- One name per concept (spec 0100): "acknowledged", "refused", "unanswered" in the editor, the
  manual and the API.

## Premise Check

- **Question:** Can a refused command be told apart from a slow or lost one today, by any
  channel the control can see?
- **Measured:** The control's state verdict carries four fields: known, on, value and text, and
  no failure field. After the outstanding window, 3000 ms by default, a refused command and an
  unanswered one both revert to the reported state and look identical. A firmware's only channel
  for a reason is the console, which the control does not read.
- **Verdict:** The gap is structural, not a tuning problem; proceed. The cost side, whether
  sketch authors adopt a reply convention, cannot be measured before the example ships, so the
  example is part of the deliverable rather than a follow-up.

## Open Questions

None at the gate. The draft's three questions (reply datasets versus a dedicated reply line, the
token form, a notification on refusal) were decided by the maintainer on 2026-11-02 and are
folded into R1, R2 and the non-goals above.
