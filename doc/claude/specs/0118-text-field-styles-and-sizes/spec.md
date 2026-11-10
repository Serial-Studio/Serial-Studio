---
spec: 0118-text-field-styles-and-sizes
title: Text Field Output Control Styles and Sizes
status: approved     # draft -> approved -> in-progress -> done | shelved; approved by the maintainer 2026-11-10
created: 2026-11-10
author: Alex Spataru
---

# Spec 0118 — Text Field Output Control Styles and Sizes

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

Since spec 0113, a button can be an illuminated push button or an emergency stop, a knob can
be a rotary selector, and a button comes in four sizes. The Text Field output control has none
of this. It always draws as one line of small monospaced text with a send button, at one size.
On a control panel built from lamps, push buttons and selectors, the setpoint or command entry
is the smallest and plainest thing there. Nothing can make it the readable display a setpoint
deserves, a touch-friendly entry for a panel PC, or a terminal-like command line.

The maintainer asked for this on 2026-11-10: the text field should have a style setting and
should use the size classes buttons already have.

## Goals

- A text field can be one of five styles, chosen per control in the Project Editor: Standard
  (today's field), Readout Display, Keypad Entry, Command Line and Compact.
- A text field can be small, normal, large or extra large, like a button.
- Every existing project looks and behaves exactly as before, and saves byte-identical.

## Non-Goals

- No change to what a text field sends: the transmit function, the Text/Number kind, Min, Max,
  Step, Quantize, decimals, units and the range refusal stay as specs 0109 and 0110 define them.
- No theme-driven look for a Standard text field. A hardware theme leaves Standard as it is;
  only a chosen style changes the field (maintainer's decision, 2026-11-10).
- No new control type, and no sizes for the slider, toggle or knob in this spec.
- No on-screen keyboard for Text fields; the keypad is numeric only.

## Requirements

1. **R1** — A text field has a **style**. Standard is what every text field is today, and a
   field with no stored style is Standard.
2. **R2** — **Readout Display** (Number fields). The field shows its value as large digits in a
   recessed display panel, with the units beside them. With a state source it shows the reported
   value. Without one it shows the last value admitted and sent. Clicking or focusing it opens
   it for typing. Enter sends, and Escape abandons the edit and shows the value again.
3. **R3** — **Keypad Entry** (Number fields). Clicking the field opens an on-screen numeric
   keypad: digits, decimal point (absent when decimals are 0), sign (absent when Min is 0 or
   more), backspace, clear, cancel and enter. Enter sends through the same admission as a typed
   value, so an out-of-range value is refused with the same message. The keypad is usable by
   touch alone, and the physical keyboard keeps working.
4. **R4** — **Command Line** (Text fields). The field looks like a terminal prompt: a dark
   well, a prompt mark and monospaced text. Enter sends and clears the line. The Up and Down
   arrows recall the last 20 sent lines of that control in the current session.
5. **R5** — **Compact** (either kind). The field without its send button: Enter sends. A
   Number field keeps its step behaviour on the arrow keys and the wheel.
6. **R6** — A style the field's kind cannot take is treated as Standard. Readout Display and
   Keypad Entry need a Number field, Command Line a Text field. The project keeps the stored
   style, so switching the kind back restores it.
7. **R7** — A text field has a **size**, using the four classes buttons have: Small, Normal,
   Large, Extra Large. The size scales the field's text, its height, its send button and its
   caption, and the panel gives the control a cell large enough for it. A field with no stored
   size is Normal.
8. **R8** — The Project Editor shows **Text Field Style** and **Text Field Size** rows for a
   text field. A style that does not fit the current kind is greyed in the list, with a note
   saying why. Multi-select editing works for both rows, as it does for buttons.
9. **R9** — Style and size are part of the project, and of the API's read and patch commands for
   an output widget. The automation surface addresses a styled field by the same identifier as
   today, and a keypad's keys are addressable for a remote client driving it.
10. **R10** — Every style keeps what a text field shows today: its title, placeholder, the
    outstanding-request and reply states (spec 0110), the dimmed unknown state, and the range
    message.

## Acceptance Criteria

- [ ] **AC1** (R1, R7, R9) — Every example project and test fixture loads and saves
      byte-identical before and after (the existing round-trip tests and property-registry
      baselines).
- [ ] **AC2** (R1, R6, R7, R9) — A unit test round-trips each style and each size through
      save and load. It checks that a Standard, Normal field writes neither key, and that each
      style on the wrong kind reads as Standard while the stored value survives.
- [ ] **AC3** (R3) — A unit test drives the keypad decisions: which keys are offered for a
      given Min, decimals and kind, and what the entered sequence evaluates to, including sign,
      decimal and clear. A value outside Min to Max is refused, never clamped.
- [ ] **AC4** (R4) — A unit test pins the command history: newest first, 20 entries, no
      duplicates of the immediately previous line, and an empty line is never stored.
- [ ] **AC5** (R2, R3, R4, R5, R7) — Maintainer observation on a running build: a panel with
      one field of each style at each size. Each looks as described, sends what the console
      shows, and refuses an out-of-range number with the existing message.
- [ ] **AC6** (R9) — `project.outputWidget.get` reports `style` and `size` for a text field,
      and an API patch of either one changes the dashboard without a reload (integration test
      against the running app).
- [ ] **AC7** (R8) — In the Project Editor, a Number field offers all five styles and greys
      Command Line. A Text field greys Readout Display and Keypad Entry. Multi-selecting two
      fields applies a size to both. (Maintainer observation.)

## Constraints & Invariants

- **Deciding constraint:** purely additive. A project that uses no style and no size saves
  byte-identical, an older build opens a styled field as a Standard field, and no existing key
  changes meaning.
- Output controls are a Pro feature; the new styles are too, with no change to the gating.
- Nothing here touches the acquisition pipeline or the display tick.
- A keypad entry or a command-line recall never transmits by itself. Only Enter or the send
  button sends, and every send goes through the existing admission and pacing.
- Style and theme stay separate axes, as in spec 0113 R18: every style works and sends the same
  bytes under every theme.

## Premise Check

- **Question:** Does any shipped or test project already carry a non-default size or style on a
  text field? Both values are stored today and ignored, so turning them on would silently change
  how such a project looks.
- **Measured:** 70 project files under `examples/`, `tests/` and `app/rcc/` hold 2 text fields
  (both Text kind). Neither carries a non-default size or style.
- **Verdict:** Proceed. Enabling the two fields changes the appearance of no existing project.

## Open Questions

- None blocking. Assumed, and open to review: the command history is per control, kept for the
  session only and not saved in the project; the keypad opens as a popup anchored to the field
  rather than as a modal dialog.
