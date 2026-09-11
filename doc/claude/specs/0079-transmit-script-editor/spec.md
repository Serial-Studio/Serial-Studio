---
spec: 0079-transmit-script-editor
title: Transmit Script Editor
status: in-progress  # NOT done: R9 unmet, see plan.md correction # draft -> approved -> in-progress -> done | shelved
created: 2026-09-10
author: Alex Spataru
---

# Spec 0079 — Transmit Script Editor

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

An output widget's transmit function is the one script in Serial Studio that is still authored
in a cramped inline pane. Every sibling script surface already opens a real editor window: the
per-dataset value transform, the canvas widget's draw code, and the MQTT publisher script each
get a window sized for code, with room for templates, a language or template selector, and a
test affordance. The transmit function instead shares the Output Widget view with the widget's
parameter table, so the code area is whatever height is left over, and the actions that belong
to it are crammed into a 30-pixel strip.

The consequence is not only cosmetic. The transmit function is the only script surface with **no
validation at all**. A dataset transform is checked before it is applied and refuses to save with
a syntax error or a missing entry point; the frame parser has an explicit Validate action. A
transmit script is accepted as typed, and the first signal that it is broken is a red flash on the
dashboard control at runtime, after the user has already clicked the thing that was supposed to
command their hardware. Worse, the failure modes that matter most are not syntax errors at all
but scaling and mapping mistakes: a slider that sends the raw position instead of the scaled
setpoint, a toggle whose on/off labels map to the wrong bytes. Those are invisible to the current
test affordance, which only accepts a typed sample value and cannot exercise the control's own
range, step, or labels.

The project also carries a documented cost for embedded code editors living inside long-lived
views. Two incidents (2026-08-17 and 2026-08-18) measured an embedded editor consuming 13% and
later 56% of the GUI thread while the user was not even looking at it, because the item stayed
instantiated and kept repainting. The current gating keeps that specific bug closed, but the
transmit editor remains the only script editor still embedded in a view that the Project Editor
retains. Moving it into a window that exists only while open removes the whole class of exposure
rather than continuing to gate it.

## Goals

- Authoring a transmit function feels like authoring any other script in the product: a window
  sized for code, with its actions in one place.
- A user learns that a transmit script is broken while writing it, not when they click the
  control on a live dashboard.
- A user can see what their script actually sends when driven by the real control, with the
  widget's own range, step, and labels, without any of it reaching connected hardware.
- The built-in transmit templates become discoverable rather than hidden behind a picker button.
- The Output Widget view becomes a view about the widget, with its parameter table given the
  full pane.

## Non-Goals

- The control script, the other script surface still edited inline, is untouched here. It also
  has no template catalog at all, which is a separate gap worth its own change.
- No change to how transmit functions execute on a live dashboard: the same engine, the same
  watchdog, the same rate limit, the same license gate.
- No change to the transmit script language. It stays JavaScript; no Lua option is added.
- No new transmit template content. This exposes the existing catalog differently; adding
  protocols to it is a separate request.
- No change to how the API or the assistant writes a transmit function. Validation added here is
  an editor guard, not a new API contract.
- The preview is not a device simulator. It shows the bytes a script produces; it does not model
  what a device would answer.

## Requirements

1. **R1** — The transmit function is edited in a dedicated window opened from the Output Widget
   view. The view no longer hosts a code editor.
2. **R2** — With the editor gone, the Output Widget view gives its parameter table the full pane,
   scrolling when the table is taller than the space available.
3. **R3** — The editor window offers the existing built-in transmit templates as a visible
   selection, not behind a separate picker step. Choosing one replaces the script under edit.
4. **R4** — The editor window offers importing a script from a file, and an explicit Validate
   action that reports the current script's verdict on demand.
5. **R5** — The window continuously shows the script's validity while the user types, in three
   distinguishable states: valid, a syntax error with its message, and a script that compiles but
   defines no transmit entry point.
6. **R6** — A script that is not valid is not persisted to the project from the editor window,
   and the user is told which of the two failures blocked it. Writes that do not originate in the
   editor window are unaffected and keep today's behavior.
7. **R7** — The window shows a live instance of the widget's own control type, honoring the
   widget's configured range, step size, labels, and units, and it is interactive.
8. **R8** — Interacting with that control shows the exact payload the current script produces for
   that interaction, in the same raw and hexadecimal form the existing sample-value test uses,
   including the payload's byte count.
9. **R9** — Interacting with the preview never transmits to a device, in any application state,
   including while a source is connected and transmitting.
10. **R10** — While the script fails validation, the preview control stays interactive and shows
    the last payload that compiled, marked as not current, with the error carried in the validity
    display.
11. **R11** — The existing sample-value test remains reachable from the editor window, so a value
    the control's own range cannot produce, or a hexadecimal input, can still be exercised.
12. **R12** — The editor window works with no device connected and with no source configured;
    neither validation nor preview requires a live link.
13. **R13** — While the editor window is closed, it costs nothing: no repainting, no retained
    editor instance.
14. **R14** — A project that already contains an invalid transmit script still loads, and that
    script still opens in the editor for repair. The validity gate applies to saving, never to
    loading.
15. **R15** — The editor window is application-modal. The widget it was opened for is the widget
    it edits, and the project tree's selection cannot move while it is open.
16. **R16** — The preview reports a payload for every interaction the control produces, including
    the intermediate values of a drag. It does not reproduce the live path's send pacing.
17. **R17** — The preview evaluates the script with the same source identity the live control
    uses, so a script that calls a source-dependent helper previews exactly as it will run.

## Acceptance Criteria

- [ ] **AC1** — Opening an output widget in the Project Editor shows a parameter table filling the
      pane and a button that opens the transmit editor window; no code editor appears in the view.
      (In-app observation.)
- [ ] **AC2** — The editor window lists every template present in the built-in transmit catalog,
      and selecting each one loads a script that passes validation. (In-app observation, one pass
      over the catalog.)
- [x] **AC3** — A preview cannot transmit. *Amended 2026-09-10: the API exposes no transmitted-byte
      counter and cannot drive a modal editor window, so the runtime form of this check is not
      automatable.* Replaced by a stronger, durable guarantee plus an observation: a source-level
      guard asserts that no output widget class outside `Panel.cpp` references a connection at all
      (`tests/integration/test_output_widget_editor.py`, runs without the app), and the maintainer
      confirms once with hardware attached that driving the preview sends nothing.
- [ ] **AC4** — A script with a syntax error, and a script that compiles but defines no transmit
      entry point, are each reported in the validity display and each refused on save with a
      message naming the failure; the project file is unchanged afterward. *Amended 2026-09-10: the
      save gate is a GUI path the API cannot drive, so this is a maintainer observation.* The
      verdict behind it is unit-tested (`tst_transmit_script_check`) and its API-side twin is
      asserted by `test_dry_run_reports_the_shared_verdict`.
- [ ] **AC5** — A script written against a slider's configured range produces, in the preview byte
      view, the same payload the dashboard control produces for the same position. (In-app
      observation against one scaling template.)
- [ ] **AC6** — Introducing a syntax error mid-edit leaves the preview control usable and its byte
      view showing the previous payload marked stale; fixing the error restores live output
      without reopening the window. (In-app observation.)
- [x] **AC7** — Writing a transmit function through the API with a deliberate syntax error still
      succeeds, confirming the gate did not leak into the non-editor path.
      (`tests/integration/test_output_widget_editor.py::test_api_still_accepts_an_invalid_transmit_script`.)
- [ ] **AC8** — Sampling the running app with the editor window closed, after having opened and
      closed it, shows no editor render activity. (Sampling recipe from the mistakes ledger.)
- [ ] **AC9** — Loading a project whose stored transmit script has a syntax error succeeds, shows
      the widget, and opens that script in the editor with the error reported. (In-app
      observation.)

## Constraints & Invariants

- **The preview must not be able to reach the device write path.** This is the deciding
  constraint of the whole feature: previewing a relay-toggle or PWM script against connected
  hardware would physically actuate it. A preview that merely avoids calling the send path today
  is not enough; the impossibility should be structural.
- Output widgets are a Pro feature and their transmit execution is license-gated. The preview must
  not become a way to exercise transmit behavior that an unlicensed build cannot otherwise reach.
- The verdict the editor shows must be the same verdict the assistant's script dry-run reports.
  Two definitions of "valid" for one script surface is a defect, not a detail.
- No unconditional per-tick repaint for the embedded editor widget, in the window or out of it.
  This is the 2026-08-17 and 2026-08-18 incident class; any new render path is checked by
  sampling, not by reasoning.
- Must not regress the acquisition hotpath gate. Nothing here belongs on a per-frame path.
- No new third-party dependency.
- Output widgets exist only in project-file mode; the feature inherits that and adds no
  QuickPlot behavior.
- The existing sample-value test surface keeps its current behavior; this feature adds a second
  way to exercise a script, it does not redefine the first.

## Resolved Decisions

All three questions that blocked planning were settled with the maintainer on 2026-09-10 and are
now carried as R15 to R17. The reasoning is kept here because it constrains the plan.

- **Modal, not modeless (R15).** A modeless window would let the tree selection move out from
  under an editor whose script is persisted as it is typed, and would put the product in a state
  it has nowhere else: a window editing one widget while the tree highlights another. Modality
  removes the ambiguity instead of managing it, and hands the preview a widget configuration that
  cannot change under it.
- **Every interaction, not the live pacing (R16).** The preview answers what a script produces for
  a value. Send pacing is a property of the transport, not of the script, and dropping the
  intermediate values of a drag would hide the scaling behavior the preview exists to show. This
  puts a cost constraint on the plan: the evaluation runs per interaction step, so it must stay
  cheap enough for a drag.
- **The widget's own source, not zero (R17).** The live transmit engine installs script helpers
  against the widget's configured source; the sample-value test, the assistant dry-run, and the
  transform editor all install against zero. The preview follows the live path, because a preview
  that can disagree with runtime is worse than no preview.

## Open Questions

- None blocking. One pre-existing divergence was found while resolving R17 and is deliberately
  left alone here: the non-live script surfaces (sample-value test, assistant dry-run, transform
  editor) install helpers against a different source identity than the live path does. Whether
  those should be reconciled is a real question, but it is not this feature's to answer, and
  changing them would alter behavior no requirement here depends on.
