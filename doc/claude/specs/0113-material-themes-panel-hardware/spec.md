---
spec: 0113-material-themes-panel-hardware
title: Material Themes, the Millennium Theme and Panel-Hardware Controls
status: in-progress  # approved 2026-11-06 (maintainer, relayed through the session)
created: 2026-11-06
author: Alex Spataru
---

# Spec 0113 — Material Themes, the Millennium Theme and Panel-Hardware Controls

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

A theme today is a colour table. The three bundled themes and every theme extension carry the
same shape: a title, its translations, two parameters (the code editor's syntax theme and the
start icon) and about 120 named colours. Nothing in a theme can say that a button has square
corners, that a tab is lettered in capitals, or that a panel is brushed aluminium instead of a
flat grey. So every theme is the same interface in different paint, and a look that depends on
material (an instrument rack, a control panel) cannot be expressed at all.

The same limit shows on the dashboard's two hardware-shaped elements. An output control is a
stock form control tinted with the group colour: a button, a switch, a dial. An operator who
drives real equipment reads a panel by the shape of its parts (a push button, a selector, a
mushroom-head stop) and none of those shapes exist. An LED is a flat disc with a bloom. Both
work, and both look like a form.

Two behaviours are also missing, not only looks. A toggle has two states, and equipment is full
of three-position selectors (close, hold, open; left, off, right), some of which spring back to
the centre. And there is no stop control that behaves like one: a button either pulses or
latches on a second click, but nothing stays down after one press until it is deliberately
released.

## Goals

- A theme can change the material of the interface and the shape of its controls, not only its
  colours, using data alone.
- Every theme file that exists today, bundled or installed, loads and renders exactly as it
  does now.
- One bundled theme, **Millennium**, shows the whole mechanism: an anodized-aluminium
  instrument panel with a dark powder-coated strip, black glass wells and engraved lettering.
- Output controls can be drawn as panel hardware, and a control's style is chosen per control
  in the project, under every theme.
- A three-position selector and a latching emergency stop exist as output controls.
- The LED panel can show panel indicator lamps that keep the glow the current LEDs have.
- A dashboard with many lamps and controls costs no more per frame than it does today.

## Non-Goals

- No theme editor. Themes stay hand-written files.
- No code in a theme. A theme never carries QML, JavaScript, shaders or a stylesheet language
  that is evaluated; it names recipes the application already knows and gives them parameters.
- No restyle of every dialog. Materials cover the main window and the dashboard; dialogs and
  the Project Editor's forms take the theme's flat colours and control style rules.
- No change to which theme is selected, for new or existing users. Millennium is offered, not
  applied.
- No change to the three existing themes' looks, and no migration of their files.
- No selector with more than three positions; a knob with a step already covers a stepped
  numeric range.
- No change to the transmit function contract: it still receives one value and returns the
  bytes to send.
- No safety function. An on-screen emergency stop is a command, see R27.
- No new third-party dependency and no new bundled typeface.

## Requirements

### A. The theme format

1. **R1** — A theme file may carry four new optional sections beside the ones it has today:
   **fonts** (named lettering roles), **materials** (named surfaces built from a recipe and its
   parameters), **surfaces** (which material each part of the window is made of) and **styles**
   (per-control rules). A fifth, **widgets**, selects the look of the LED panel and of the
   output controls. The shape:

   ```json
   {
     "title": "Millennium",
     "parameters": { "code-editor-theme": "millennium", "start-icon": "..." },
     "translations": { "es_MX": "Milenio" },
     "colors": { "window": "#C3C8C3", "text": "#1A1C1E", "...": "every colour key, as today" },

     "fonts": {
       "engraving": { "family": "DIN Alternate", "capitals": true, "spacing": 0.8 }
     },
     "materials": {
       "panel": { "recipe": "brushed-metal", "color": "#B7BDB7", "grain": 0.6 },
       "frame": { "recipe": "gradient", "top": "#7A8189", "bottom": "#5C636B" },
       "strip": { "recipe": "gradient", "top": "#30353B", "bottom": "#1F2226" },
       "well":  { "recipe": "glass", "top": "#0C0E10", "bottom": "#1C2025" }
     },
     "surfaces": {
       "dashboard": "frame",
       "widget": "panel",
       "toolbar": "strip",
       "taskbar": "strip",
       "display": "well"
     },
     "styles": {
       "button":       { "radius": 3, "font": "engraving" },
       "tab":          { "font": "engraving", "indicator": "#35AE55" },
       "group-title":  { "font": "engraving" },
       "table-header": { "material": "strip", "font": "engraving" },
       "input":        { "radius": 3, "border": 1, "focus": "#1F7A33" }
     },
     "widgets": { "led": "lamp", "output": "hardware" }
   }
   ```

2. **R2** — **Backward compatibility rule.** Every new section is optional, and so is every key
   inside one. A theme without a section gets today's rendering for everything that section
   governs. A theme file written before this feature therefore renders pixel-identical to the
   current build, and the three bundled themes are not edited.
3. **R3** — **Forward tolerance.** An unknown section, key, recipe name or surface name is
   ignored, with one warning in the log naming it; the rest of the theme applies. A malformed
   value falls back to the default for that one value. A theme never fails to load because of
   a section it did not have before.
4. **R4** — A theme that uses the new sections still carries the complete colour table. Flat
   colours remain what every surface not covered by a material shows, what a material falls
   back to when it cannot be drawn, and what a build without this feature shows for the same
   file.
5. **R5** — Themes installed as extensions can use every new section. A theme stays data: it
   can select and parameterize recipes, and it cannot make the application load a file outside
   its own package or run anything.
6. **R6** — Switching theme at run time applies materials, style rules, fonts and widget looks
   together, without a restart, the way colours switch today. Following the operating system
   ("System") keeps resolving to the same two themes it resolves to now.
7. **R7** — The recipes available to a theme are at least: a flat colour, a vertical gradient,
   brushed metal (a lit gradient with fine horizontal grain and a bright upper-left and dark
   lower-right edge), and glass (a dark well, shadowed under its top edge with a light line
   along the bottom).
8. **R8** — Style rules can set, per control class, at least: corner radius, border width,
   lettering role, capitals, letter spacing, a gradient or material for the control's face, and
   the indicator colour used for the open tab and the focused field. The control classes are at
   least: button, filled button, tab, input field, group title, table header.
9. **R9** — A lettering role never overrides content the user chose a font for. Dashboard
   values, the terminal, code editors and the user's widget font preference keep their fonts;
   lettering roles apply to chrome captions only (tabs, buttons, headers, group titles, lamp
   and control legends).
10. **R10** — Text in a script the role's typeface does not cover is drawn in the interface
    font, with no missing-glyph boxes, in every language the application ships.

### B. The Millennium theme

11. **R11** — A theme titled **Millennium** is bundled and appears in the theme list of a fresh
    install with no download. It does not become the selected theme for anyone.
12. **R12** — Its look, described in its own terms:
    - **Panel face:** anodized aluminium, pale grey-green (`#B7BDB7`), brushed horizontally,
      lit from above. Dashboard widgets are plates of it standing off a darker frame
      (`#7A8189` to `#5C636B`).
    - **Strip:** the toolbar and taskbar are dark powder coat (`#30353B` to `#1F2226`) with
      pale lettering (`#A9B0B8`, lit `#F2F4F5`); the open tab is marked by a green indicator
      bar with a soft glow.
    - **Wells:** plots, terminals and other displays sit in black glass let into the plate.
    - **Lettering:** captions are dark ink (`#1A1C1E`) on the plates and pale on the strips,
      in the interface fonts. Millennium names no lettering role and bundles no typeface
      (amended 2026-11-06, see Resolved Decisions).
    - **Forms:** window is the panel face a shade lighter, panes are a paler plate, input
      fields are pale wells with a dark edge, filled buttons and table headers are the dark
      strip with lit lettering, the open tab's bar is the lamp green (`#35AE55`) and the
      focused field's edge the deeper green `#1F7A33`.
    - **State colours:** ok `#35AE55`, caution `#F3BD2E`, warning `#DE3E2A`.
13. **R13** — Millennium meets the same legibility bar as the bundled themes: body text and
    captions on the panel face, on wells and on the strip each reach a contrast ratio of at
    least 4.5 to 1.

### C. LED panel lamps

14. **R14** — A theme can select the **lamp** look for the LED panel. A lamp is a faceted lens
    of the LED's own colour seated in a knurled metal bezel: tinted and dark when off, bright
    with a specular highlight and a soft halo when on.
15. **R15** — A lit lamp keeps the bloom and the HDR emissive behaviour the current LED has,
    layered over the lens. It is a hybrid of the two looks, not a replacement of the glow by a
    painted halo. With effects disabled it still reads as lit from the lens and halo alone.
16. **R16** — Colour, label, alarm band and blink behaviour are unchanged; the lamp is a look,
    not a new data model. A theme that does not select it shows today's LEDs unchanged.

### D. Output controls: style, state and payload model

17. **R17** — Every output control carries a **style**, chosen from a dropdown in the Project
    Editor. Each control type offers its own list, and the first entry, **Standard**, is what
    the control is today. A control with no stored style is Standard, so every existing project
    loads and behaves unchanged. Styles by type:
    - **Button:** Standard, Illuminated push button, Emergency stop.
    - **Toggle:** Standard, Lever switch, Rotary selector (two positions).
    - **Selector** (new, R21): Rotary selector, Lever switch.
    - **Slider, Knob, Text field:** Standard only in this spec.
18. **R18** — Style and theme are separate axes. The style says what the control *is* and is
    stored in the project; the theme says how it is *painted* and belongs to the user. Every
    style is available and behaves the same under every theme. A theme that selects the
    hardware look paints each style as a physical part, Standard included (a button becomes a
    flush push button in a metal bezel, a toggle a lever, a knob a pointer knob with detent
    marks); any other theme paints it in that theme's plain manner.
19. **R19** — **Illuminated push button.** A round coloured head in a metal bezel that is lit
    while the control is on (latching button) or while it is held (momentary), in the control's
    colour, and visibly lower and darker while pressed. It sends what a button sends today.
20. **R20** — The hardware looks keep everything a control shows now: its title, on and off
    labels, icon where the style has room for one, size class, the dimmed unknown state, the
    outstanding-request state and the range message.
21. **R21** — **Selector.** A new output control with three positions: left, centre, right.
    Each position has a label and a value; moving to a position passes that position's value to
    the transmit function, which returns the payload, so each position has its own payload.
    Default values are -1, 0 and 1. The starting position is configurable and defaults to
    centre.
22. **R22** — A selector has a **return** option: *stays* (each position is maintained) or
    *springs to centre* (left and right hold only while pressed; on release the selector
    returns to centre and transmits the centre value once).
23. **R23** — A selector bound to a state source shows the position whose value equals the
    reported one, compared as text or as a number, and shows the unknown state when no
    position matches. Reflecting a reported position never transmits.
24. **R24** — **Emergency stop** is a button style: a red domed mushroom head on a yellow
    legend ring carrying the emergency-stop lettering and the twist-release arrows. Red and
    yellow are fixed; no theme and no control colour changes them, and the style is
    recognisable under every theme.
25. **R25** — It latches. One press transmits the *pressed* value (1) and the head stays
    visibly down; letting go of the pointer does not release it. Release is a separate,
    deliberate action on the latched head, shown as a twist, and transmits the *released* value
    (0). The transmit function may return nothing for the released value, in which case nothing
    is sent. A double-click cannot press and release in one gesture.
26. **R26** — The latched state belongs to the control, not to its picture: it holds when the
    pointer leaves, when the panel scrolls or is hidden and shown again, and when the theme
    changes. A press is transmitted at once, never held back by the pacing that spaces out
    ordinary control changes, and never asks for confirmation. If the press could not be
    transmitted, the control says so instead of looking stopped. With a state source the head
    follows what the equipment reports, as a latching button does; without one it starts
    released each time the dashboard is built. The release is paced like any other change.
27. **R27** — **An on-screen emergency stop is a command sent over the data link.** It reaches
    the equipment only if the computer, the application, the link and the device firmware are
    all working, so it is never a substitute for a hardwired emergency-stop circuit. The
    Project Editor says this where the style is selected, and the help page for output controls
    says it.
28. **R28** — A project written with a style, a selector or an emergency stop round-trips
    through save and load. A project that uses none of them is byte-identical on save to what
    the current build writes. The automation and API surfaces address the restyled controls by
    the same names as today, and report an emergency stop's latch and a selector's position the
    way they report a toggle's state.

## Acceptance Criteria

- [ ] **AC1** (R2) — With each of the three bundled themes selected, screenshots of the main
      window, a dashboard with every widget type, the Project Editor and Preferences match the
      same screenshots from the build before this feature. (Maintainer observation, image diff.)
- [ ] **AC2** (R2, R3) — A unit test loads the three bundled theme files, a file with none of
      the new sections, and a file with an unknown section, an unknown recipe and a malformed
      value: all load, the first four resolve to the default rendering for every surface and
      control class, and the last logs one warning per unknown name.
- [ ] **AC3** (R5) — A theme extension naming a material file outside its own folder is loaded
      with that material ignored and a warning. (Unit test on a fixture folder.)
- [ ] **AC4** (R6, R11) — On a fresh settings store the selected theme is what it is today, and
      Millennium is in the list; selecting it changes materials, lettering and control shapes
      without a restart, and selecting Default restores AC1's rendering. (Maintainer
      observation; list membership by a unit test on the catalog.)
- [ ] **AC5** (R10) — With Millennium selected and the language set in turn to Russian,
      Japanese, Arabic and Hindi, no caption shows a missing-glyph box. (Maintainer
      observation.)
- [ ] **AC6** (R13) — A script computes the contrast ratio of every text and surface colour pair
      R12 names, from the theme file, and each is at least 4.5. (Runs without the application.)
- [ ] **AC7** (R14 to R16) — An LED panel under Millennium shows lamps; off, on and blinking
      lamps are distinguishable with effects on and with effects off; under Default the same
      panel matches AC1. (Maintainer observation.)
- [ ] **AC8** (goal: frame cost) — On a dashboard with 64 lamps and 32 output controls,
      Millennium's GUI-thread frame time is within 10 % of Default's on the same project and
      data, and the acquisition benchmark gate is unchanged. (Maintainer run.)
- [ ] **AC9** (R17, R28) — Every example project and every project fixture in the test suite
      loads and saves byte-identical before and after. (Existing round-trip tests.)
- [ ] **AC10** (R21, R22) — A unit test drives a selector through left, centre and right and
      records the values handed to the transmit function: -1, 0, 1 by default, the configured
      values otherwise, and with spring return a release from either side yields the centre
      value exactly once.
- [ ] **AC11** (R23) — A unit test feeds a bound selector each position's value, a non-matching
      value and no value: the three positions, then unknown twice, and nothing is transmitted.
- [ ] **AC12** (R25, R26) — A unit test presses an emergency-stop button, then presses it again
      inside the double-click interval, then releases it: the transmit function sees 1 once,
      then 0 once; the latch is on between them; a press issued inside the pacing window of a
      previous send is delivered immediately.
- [ ] **AC13** (R24, R27) — Under each bundled theme and Millennium an emergency stop is red on
      yellow, and the Project Editor shows the R27 notice when the style is chosen. (Maintainer
      observation.)
- [ ] **AC14** (R18) — The same project shows every style under Default and under Millennium,
      and each control transmits the same bytes in both. (Maintainer observation with the
      console open.)

## Constraints & Invariants

- **Existing themes are untouched.** R2 is the deciding constraint: the mechanism is opt-in per
  theme, and an absent section means the current code path, not a default material that
  happens to look the same.
- **No per-frame painting.** A material or a hardware part is drawn when its size, colour or
  state changes and is reused until then; nothing in this feature repaints on the display tick.
  The Premise Check has the numbers that make this a rule and not a preference.
- **No surface as large as the window is rendered as one image.** Grain is a repeating pattern;
  lighting and edges are composed around it.
- **A theme is data.** No path in a theme escapes its package, nothing in it is evaluated, and
  the extension trust model is unchanged.
- **Output controls stay a Pro feature**, the selector and every style included. The theme
  engine, the Millennium theme and the LED lamp look are available in every edition.
- **Feedback never commands** (spec 0080 R3) holds for the selector and the emergency stop.
- **Write limits and acknowledged commands** (specs 0109, 0110) apply to the new controls
  unchanged, with the one stated exception that an emergency-stop press is not paced.
- **Project schema changes are additive.** An absent style means Standard; an older build
  ignores the field. A selector opened in an older build is a degraded control, and that is
  stated in the release notes.
- **Automation names are not derived from a theme or a style** (spec 0101).
- **Emissive elements keep the HDR path** (spec 0089): a lit lamp or illuminated button behind
  an effect layer must not clamp to SDR.
- **Motion** (the press, the lever throw, the twist release) honours the reduce-motion setting;
  state changes stay visible without it.
- **One name per concept** (spec 0100) for the new user-facing terms: Style, Selector,
  Emergency stop, Illuminated push button, Lever switch, Rotary selector.
- The acquisition hotpath gate is not regressed; nothing here runs per frame of data.

## Premise Check

- **Question:** Is a painted material affordable on a dashboard with many widgets, and can
  materials reach the visible interface without touching most of the 253 QML files?
- **Measured:** (1) Cost, Qt raster engine, offscreen, one core of the maintainer's Mac. A
  brushed plate rendered at window size costs 51 ms and 7.9 MiB at 1920x1080 and 358 ms and
  56 MiB at 5120x2880; a 512 px repeating tile costs 7 ms and 1 MiB, once. A lamp at 2x costs
  0.18 ms and 26 KiB per state to render once. Repainting lamps every frame costs 1.7 ms for
  16, 7.0 ms for 64 (42 % of a 60 Hz frame) and 29.8 ms for 256; drawing the same lamps from
  cached images costs 0.04, 0.18 and 0.73 ms. (2) Reach, counted in the QML tree. The main
  window's surfaces are read at about 30 sites in 15 files (dashboard background 10 sites in 5
  files, toolbar 10 in 6, widget window 6 in 4, panes 2, taskbar 1, caption 1). The colours
  dialogs are built from are read at 228 sites in about 140 files (group box background 153 in
  91, window 75 in 52). Theme colours are read 2874 times in 184 files.
- **Verdict:** Holds, with two rules it forces. Cached parts cost about one fortieth of
  per-frame painting, so caching is a requirement, and a window-sized image is ruled out by its
  resize cost and memory, so grain must tile. Materials on the main window and dashboard are a
  bounded change; materials inside every dialog are not, which is why dialogs are a non-goal
  and take flat colours and style rules instead.

## Resolved Decisions

Settled with the maintainer on 2026-11-06, each on the recommended option; the answers reached
the working session through its question dialog. The reasoning is kept because it constrains
the plan.

- **Theme name: Millennium.**
- **One spec, not two.** The hardware looks and the materials are designed together and land in
  the stage order A, B, C, D, with D ordered standard and illuminated push button, then
  emergency stop, then selector.
- **The selector is a new control type (R21).** A toggle is a boolean everywhere (its value, its
  on-value rule, its two labels) and a third position would bend each of those. Existing
  toggles are untouched. Accepted cost: an older build shows a selector as a different control.
- **The default theme does not change.** Millennium is bundled and selected for no one (R11).
- **An emergency-stop press is sent at once (R26)**, a deliberate and documented exception to
  the 50 ms pacing of spec 0109. Its release stays paced like any other control change.
- **After a reconnect or a reopened dashboard an emergency stop starts released**, unless it is
  bound to a dataset that reports the real state, which it then follows. Remembering a latch
  across sessions would show a stop the equipment may no longer be in.
- **Focus colour.** The lamp green (`#35AE55`) measures 1.7 to 1 against the window colour and
  2.4 to 1 against a pale input field, under the 3 to 1 usually asked of a focus indicator. The
  focused field's edge is the deeper green `#1F7A33` (3.2 and 4.5 to 1); the lamp green stays
  for the open tab's bar on the dark strip and for the lamps.
- **Image-backed materials are deferred to a later spec.** Painted recipes only: the four
  recipes of R7 cover Millennium, and files in a theme package add a path-handling surface.

### Amended 2026-11-06, after the maintainer ran the first build

- **No special typeface.** Millennium keeps the interface fonts. Lettering roles stay in the
  theme format (R1, R9, R10) for themes that want one; the bundled theme uses none, and the
  typeface first bundled for it is removed.
- **Gradients run dark at the top and lighter at the bottom**, as in the other themes, so a
  caption joins the toolbar above it. This holds for pane captions, the command palette, the
  breadcrumb bar and every other top and bottom colour pair.
- **Secondary and frozen title bars and table headers are steel, not dark**, and pane
  backgrounds are told apart from secondary toolbars.
- **LED rows take the control box's cell** under the lamp look: rounded, in the group box
  colours, and taller so the bezel has room.
- **The grain is two tiles of unrelated sizes**, because one tile visibly repeated on a tall
  widget.
- **Hardware push buttons are the default** for a standard output button under every theme; a
  theme opts out with the `plain` output look. This is a deliberate exception to R2 for the
  output panel, decided after seeing the controls in the running build.
- **Strips are dark steel, not near-black**, and no surface is pure white: dial faces, bar
  wells, the console and the plot area are satin tones of the plate.
- **The selector is a knob style, not a new control type** (this replaces the earlier
  decision): one labelled detent per Step from Min to Max gives two, three or n positions, an
  older build shows it as an ordinary knob with the same values, and no type is added. Spring
  return (R22) is still to be built.
- The colour table was reworked against the bundled and the published themes; the values in
  R12 are the materials' anchors, and the theme file is the record of the rest.

## Open Questions

- None blocking. Two recommendations stand unless the maintainer says otherwise: the emergency
  stop's ring reads "EMERGENCY STOP" in the interface language, repeated around the ring, with
  the control's own title as the caption beside it; and hardware looks for the slider and the
  text field (a fader, a keypad entry window) are follow-ups outside this spec.
