---
spec: 0100-new-user-clarity
title: New-user clarity — first run, mode model, parser contract, naming, docs
status: in-progress   # draft -> approved -> in-progress -> done | shelved
created: 2026-10-15
author: Alex Spataru
---

# Spec 0100 — New-User Clarity

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

To go from raw bytes to a custom dashboard, a new user must learn about fourteen chained
concepts: operation mode, device, frame delimiters, decoder, frame parser, parsed array,
frame index, dataset, group, dataset widget vs group widget, workspace, and then the optional
layer of transforms, computed datasets, Variables, Control Loop, actions and output widgets.
A 2026-10-15 audit of the UI, the manual and the project model found that the app hands these
over with almost no scaffolding, and often under two names:

- **First run teaches nothing.** The console's launch text is copyright, licence and a Pro
  feature list. Nothing says "pick a port, set the baud rate, press Connect", nothing points
  at the Examples browser, and Quick Plot's input contract (one line per sample, values
  separated by commas, newline-terminated) appears nowhere in the UI. Controls that are
  disabled (Connect with no port, mode radios while connected, export toggles in Console
  Only) never say why.
- **The mode selector is a trap.** It is labelled "Frame Parsing" but governs the whole app;
  the Project Editor is roadblocked by an overlay until the user switches to Project mode,
  and a working Quick Plot session has no path to becoming a project.
- **The parser contract is invisible.** The frame index is 1-based, its own description says
  it is for "aligning datasets in time" (false), and when the parser returns fewer values than
  a dataset's index needs, that widget silently keeps its last value and looks frozen. The
  Problem Center's `short-frames` finding catches this, but it has no Go To, reports only the
  first offending device, and is missing from the Problem Center manual page. Its
  duplicate-index warning contradicts the manual, which presents shared indices as a feature.
- **Composite widgets have unstated rules.** Accelerometer, Gyroscope and 3D Plot need
  exactly three datasets in X, Y, Z order; GPS needs two or three. The editor accepts anything
  and the rule surfaces only as a troubleshooting entry after the dashboard comes up empty.
- **One concept, several names.** Device / Source / Data Source / I/O Interface; group widget
  / Composite Widget; Variables / Data Tables / registers; Control Loop / Control Script;
  Canvas / Painter; Historian / Session Database; Built-In / Native leaking into user text;
  "COM Port" on macOS and Linux; a pane titled "Device Setup" containing a section also titled
  "DEVICE SETUP".
- **Five ways to derive a value, no guide.** Value transform, computed dataset, Variables
  expression, Control Loop, `dashboardTick()`. A dataset also silently becomes "computed" when
  its transform never mentions `value`.
- **The manual has no concepts page.** The Glossary sits under "Support" next to Affiliate and
  Comparison; Identity Model, called "the most common beginner trap", sits under
  "Architecture". Getting Started names frame detection, frame index, datasets and groups
  without defining them, gives no worked Project File example, repeats its operation-mode
  section twice, and presents Pro-only transforms as part of Project mode. Five page filenames
  no longer match their titles. The top first-run failure ("parser returned the wrong number
  of values") has no troubleshooting entry. 31 of 35 examples state no learning goal, and the
  examples index is alphabetical.
- **Pro limits surface late.** "Pro features detected… using fallback widgets" appears only
  after the user has built the thing, and the message doesn't say what to do.

Who feels it: every first-time user, and the support inbox. The 0.16% download-to-paid rate
leaves no slack for an onboarding that loses people before their first plot.

## Goals

- A first-time user with a device printing CSV lines reaches a live plot from the launch
  screen without opening the manual.
- A Quick Plot user can turn their working session into an editable project in one action.
- The Project Editor is never blocked by a mode overlay.
- Every rule the parser and composite widgets impose is visible at the point the user can
  break it, and reported when broken.
- Each concept carries exactly one user-facing name across UI, manual, examples and the
  in-app assistant corpus.
- The manual has an explicit beginner path: concepts, then a worked tutorial, then reference.

## Non-Goals

- **No change to project-file format, API command names or JSON keys, or internal
  identifiers.** `sources`, `sourceId`, `Native`, `painter`, `sessions.*`, `virtual_` and
  `registers` stay. The overloaded dataset `widget` key is left as it is. Renames are
  user-facing only, per the repo's existing rename convention.
- **No inline Pro badges or lock icons** on editor buttons, driver lists or export toggles
  (maintainer decision 2026-10-15). Pro gating behaviour is unchanged; only its messages
  improve.
- **No removal or merging of operation modes.** The three modes, their persistence and their
  CLI flags stay.
- No first-run modal wizard or guided tour overlay.
- No change to which features are Pro, to licensing or trial logic, or to any
  security-sensitive surface.
- No translation work. New and changed strings must be translatable; refreshing the
  translation catalogues stays the maintainer's step.
- No new widget types, drivers or parser engines.
- API-Reference length/restructuring is out of scope (reference material, not onboarding).

## Requirements

### A. First run

1. **R1** — *(Amended 2026-10-15 by the maintainer: the whole welcome guide must fit an 80x24
   console, logo first and copyright directly below, so the guidance is a single start-hint line
   that replaces the old palette tip, and the edition texts were trimmed and reflowed to fit.)*
   Original wording: on launch with no device connected and no project loaded, the console opens
   with a getting-started block of at most 12 lines: choose a device and its settings, press Connect, the format
   Quick Plot expects with a one-line example, and where to find Examples and Help. Licence
   and edition text follows below it, unchanged in substance. This holds for every edition
   (GPL, unlicensed, trial, Pro).
2. **R2** — Each operation-mode choice has a tooltip describing what it does and what it
   expects from the device. Quick Plot's states the line/comma/newline contract. *(Amended
   2026-10-15: tooltips, not inline text, which read too small.)*
3. **R3** — The operation-mode group is titled **Operation Mode**, not "Frame Parsing". Its
   three choices keep their descriptive labels: **Console Only (No Parsing)**, **Quick Plot (Comma
   Separated Values)** and **Parse via Project File**. *(Amended 2026-10-15 by the maintainer: the
   short labels were less clear than the originals.)*
4. **R4** — Every control that is disabled because of app state explains why on hover/focus,
   and where applicable what to do. At minimum: Connect (no port or device selected,
   invalid configuration), the mode choices (connected; file playback open), and the export
   toggles (Console Only mode).
5. **R5** — When connected in Quick Plot or Project mode and no frame has arrived after 10
   seconds, the console shows a non-blocking hint naming the likely causes (baud rate, line
   ending/delimiter, mode) and pointing to the Problem Center.
6. **R6** — The Problem Center can be reached from a visible UI control, not only the command
   palette.

### B. Mode model

7. **R7** — While disconnected, opening the Project Editor never shows a mode roadblock. It
   switches the app to Project File mode and posts a notice saying how to go back. While
   connected in another mode, the editor shows a reworded overlay with a "Disconnect and Edit"
   action, because about 30 workspace operations only work in Project File mode. The
   password-lock overlay stays. Creating, opening, saving or generating a project while
   disconnected switches to Project File mode. While connected, the switch takes effect on
   disconnect, with a visible notice. This applies to every *implicit* switch, from the UI or
   the API. Explicit mode commands keep their current behaviour. A project waiting on a
   deferred switch is never silently discarded on quit. *(Amended 2026-10-15 after
   implementation found the mode gates.)*
8. **R8** — In Quick Plot mode, after at least one frame has been received, the user can
   "Save as Project" from the dashboard start menu (the Quick Plot structure exists only once the
   dashboard is populated; amended 2026-10-15). The result is a project with one device carrying the current connection
   settings and the Quick Plot framing, and one dataset per detected channel at matching
   frame indices, each with a plot. After saving, the app is in Project mode with that project
   loaded. Reconnecting to the same stream produces the same plotted values as Quick Plot did.
9. **R9** — The project produced by R8 contains no Pro-only content, so it opens and connects
   in an unlicensed or GPL session.

### C. Parser contract

10. **R10** — The frame index field is described in user-facing text as the 1-based position of
    the value in the parser's output array (first value = 1). The false "aligning datasets in
    time" description is gone from every surface that shows it (editor, API schema, generated
    docs).
11. **R11** — Testing a frame parser with sample data shows, for each returned value, its frame
    index and the dataset(s) that will read it. Datasets whose index exceeds the returned array
    length are flagged in that result.
12. **R12** — During a live session, when a dataset's frame index exceeds the number of values
    the parser returned, the Problem Center reports it for every affected device (not only the
    first), naming the datasets, the highest index and the actual array length, with Go To. It
    reports this from data actually received, sustained, not from a single frame. The finding
    is documented on the Problem Center manual page. *(Amended 2026-10-15: the finding exists
    as `short-frames`. This requirement extends it.)*
13. **R13** — Two datasets of one device reading the same frame index is a supported feature (the
    same value shown two ways), and the manual says so. The Problem Center warns only when
    duplicates also share the same widget type, which is almost certainly a mistake.
14. **R14** — When a dataset becomes computed because its transform does not reference the
    input value, the editor says so next to the greyed frame index, in words.

### D. Composite widget rules

15. **R15** — When a group uses a composite widget, the group editor states that widget's
    dataset requirement (count, and order/role where it matters, e.g. "3 datasets: X, Y, Z in
    this order") before the user adds datasets, and shows a visible warning while the
    requirement is not met.
16. **R16** — The Problem Center reports a composite widget whose dataset requirement is not met,
    with Go To.

### E. One name per concept

17. **R17** — A canonical user-facing glossary of names is fixed (plan to finalize the list),
    including at least: **Device** for every data source, hardware or not (Process I/O, MQTT
    and file playback included), replacing Source, Data Source and I/O Interface in user text;
    **Group Widget** for the group-level widget (paired with **Dataset Widget**; not Composite
    Widget); **Variables** for user
    tables (not Data Tables or registers); **Control Loop**; **Canvas Widget**; **Historian**;
    **Built-In** parser (never "Native" in user text).
18. **R18** — Every user-facing string in the UI, the manual, examples READMEs and the in-app
    assistant corpus uses the canonical names from R17. Exceptions are API identifiers,
    JSON keys and file paths shown verbatim as code. Where an identifier differs from the
    user name, it is explained once (Glossary and the relevant reference page). The API
    Reference and API command descriptions keep the API's own vocabulary (`source`, `table`,
    `painter`), because they describe identifiers like `project.source.*`. The Glossary maps
    each API term to its user name. *(Amended 2026-10-15.)*
19. **R19** — Platform-specific labels are correct per platform: the serial port field is not
    labelled "COM Port" on macOS or Linux.
20. **R20** — No panel title is repeated as a section title inside the same panel, and
    user-facing jargon on beginner-visible surfaces is either replaced with plain wording or
    carries an explanatory tooltip (e.g. the alarm-sequence setting, "Send DTR Signal",
    "Data Conversion Method").
21. **R21** — The overlapping dataset widget names ("Bar/Level", "Level Indicator",
    "Bar Panel") are disambiguated so each name maps to one thing.

### F. Manual and examples

22. **R22** — A "Concepts" page sits immediately after Getting Started in the Basics section. It
    explains the chain device → frame → decoder → parser → array → frame index → dataset →
    group → widget → workspace in one diagram plus short definitions, and links each term to
    its reference page.
23. **R23** — The Glossary moves into the Basics section. Identity Model moves next to the
    Project Editor material. Non-support pages (About, Comparison, Use Cases, Affiliate) are
    separated from FAQ, Troubleshooting and Problem Center.
24. **R24** — Getting Started links the Concepts page and Glossary before using any term,
    states the operation modes once, drops the install instructions it duplicates from the
    installation pages (linking them instead), and marks Pro-only features as Pro.
25. **R25** — A worked Project File tutorial takes a concrete sample frame from a sketch the
    reader can flash, through device, delimiters, parser, datasets, a group with a composite
    widget, and a workspace, ending with the exact dashboard the reader should see.
26. **R26** — A decision guide tells the user which of the five derived-value mechanisms to use
    for which job, with one example each and Pro status stated.
27. **R27** — Troubleshooting gains an entry for "parser returns the wrong number of values /
    widget frozen", matching the R12 finding. Linux serial-port permission guidance has one
    canonical home, which the other pages link to rather than restate.
28. **R28** — Pages whose filenames do not match their titles are renamed to the title. Each
    old path remains as a stub page that points to the new one, so external links and in-app
    help references keep working. At minimum: Variables, Canvas Widget, Frame Parser (covering
    JavaScript and Lua), Control Loop, Historian.
29. **R29** — Pro marking in page titles and section listings follows one convention across the
    manual, including the Drivers section.
30. **R30** — Every example README opens with what it teaches and its prerequisites. The examples
    index offers a beginner → advanced learning order alongside the existing table.

### G. Pro messaging

31. **R31** — The GPL-build fallback-widget notice names which items in the project need Pro,
    what the user sees instead, and the action that resolves it (the official build). In
    commercial builds the notice never shows. There, the connect-time Pro refusal already names
    the project's Pro content and must keep naming it, with the right action for the licence
    state. *(Amended 2026-10-15: the fallback notice is GPL-build only.)*

## Acceptance Criteria

- [ ] **AC1** (R1) — In the running app, launching each edition state (GPL build, unlicensed,
  trial, Pro) with no project shows the getting-started block first in the console.
- [ ] **AC2** (R2–R4) — Maintainer walkthrough: each mode choice shows its description. Hovering
  each listed disabled control in each blocking state shows a reason.
- [ ] **AC3** (R5) — Connect a TCP/UART source that sends nothing: the hint appears after 10 s and disappears when frames arrive.
- [ ] **AC4** (R7) — From Quick Plot and Console Only while disconnected, opening the Project
  Editor switches to Project File with a notice, and shows no overlay. While connected, the
  reworded overlay offers Disconnect and Edit. New/Open/Save while connected shows the notice,
  and the mode flips on disconnect.
- [ ] **AC5** (R8, R9) — A ctest unit pins the generated project: devices, framing, parser,
  indices 1..N, titles and plots for a given Quick Plot structure, with no Pro content. In a
  maintainer walkthrough, a known CSV stream gives the same values in Quick Plot and after
  Save as Project + reconnect, and the saved project connects in an unlicensed session.
  *(Amended 2026-10-15: no new API command is added just for testing.)*
- [ ] **AC6** (R10) — No remaining occurrence of the "aligning datasets in time" wording in the
  repo's user-facing and generated surfaces (grep). The generator checks stay green.
- [ ] **AC7** (R11) — The parser test with a 2-value sample against a project with indices 1–3
  shows index 3's dataset flagged.
- [ ] **AC8** (R12, R16) — Integration test: a stream that yields fewer values than the highest
  index raises the new Problem Center finding. A composite widget with the wrong dataset count
  raises its finding. Both clear when fixed.
- [ ] **AC9** (R12) — `--benchmark-hotpath` passes all gates with no regression outside the
  historical noise band.
- [ ] **AC10** (R15, R14) — Maintainer walkthrough: Accelerometer group with 2 datasets shows
  the requirement and the warning. A transform without `value` shows the computed explanation.
- [ ] **AC11** (R17, R18) — A term audit over QML strings, the help manual, examples READMEs and
  the assistant corpus finds no banned synonyms outside code spans and the explained-once
  locations. The audit should be scripted so it can re-run.
- [ ] **AC12** (R19–R21) — Maintainer check on macOS/Linux and Windows of the port label. Review
  of the listed jargon items.
- [ ] **AC13** (R22–R30) — `documentation-verify.py` and `claim-verify.py` pass. Every page is
  registered in the help index. Each old filename resolves to a stub linking its replacement.
  The ss-docs tell-check passes. Every example README has the learning-goal opener.
- [ ] **AC14** (R25) — The maintainer follows the tutorial end to end on real hardware or the
  simulator and gets the dashboard it describes.
- [ ] **AC15** (R31) — In a GPL build, a project with Pro items shows a notice listing them. In a
  commercial build, connecting it unlicensed and expired gives a refusal that lists them, with
  the right action for each state.

## Constraints & Invariants

- **Deciding constraint: renames are skin-deep.** Project files, API commands and fields,
  gRPC/mirror wire formats, MQTT topics and internal identifiers must not change. Existing
  projects, scripts and API clients keep working byte-for-byte.
- The live array-length check (R12) must not add per-frame allocation, locking, signals or
  queued emissions. Diagnostics stay pulled on the existing slow tick. The 256 kHz
  `--benchmark-hotpath` gate must not regress.
- Mode auto-switching must not change mode while connected, must not reorder startup, and
  must keep the mode's persistence and CLI behaviour.
- Save as Project (R8) works in every edition, including GPL builds.
- User-facing text derived from the single dataset-property manifest changes only through
  that manifest and its generators, never by hand-editing a generated artifact.
- Doc redirects must keep every existing in-app help reference and external URL resolving.
- The in-app assistant corpus must be updated in the same change as the UI renames, or the
  assistant teaches stale names.
- Motion and HDR contracts apply to any new hint or notice UI. There are no new popups
  outside the shared transitions.
- No inline Pro badges (Non-Goals). Licensing logic is untouched.
- Work is large. Plan should split it into independently shippable milestones (suggested:
  A+G copy; B; C+D; E; F), each leaving the tree consistent.

## Open Questions

None open. Resolved with the maintainer 2026-10-15:

- **Q1 — Shared frame index:** supported. The warning is narrowed to duplicates with the same
  widget type (R13).
- **Q2 — Group-level widget name:** Group Widget (R17).
- **Q3 — "Device" scope:** uniform, including non-hardware inputs (R17).
- **Q4 — Thresholds:** no-data hint after 10 s (R5). Getting-started block is at most 12
  lines (R1).
- **Q5 — Mode wording:** settled here. The title is Operation Mode, and the choice labels
  stay Console Only / Quick Plot / Project File (R3).
