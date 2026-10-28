---
spec: 0105-visible-only-alarms
title: Alarms only while visible
status: in-progress  # approved 2026-10-28; implementation started 2026-10-28
approved: 2026-10-28 # visibility rule, reveal rules, key and label, live toggle, API state decided
created: 2026-10-28
author: Alex Spataru
---

# Spec 0105 — Alarms only while visible

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Approved 2026-10-28; `/ss-plan` may
> start when the maintainer asks for it.

> **Decisions folded in (maintainer, 2026-10-28):** visible means shown on an active tab
> with the Dashboard pane the shown view and the window not minimized, a frozen dashboard
> counts; a reveal is a fresh, unacknowledged band entry under the usual cooldown; nothing is
> recorded for hidden band time; the option is per dataset like Suppress Alarms, key
> `alarmsWhileVisible`, label *Alarms only while visible*; a live toggle applies at the next
> dashboard rebuild; a hidden marked dataset is absent from the alarm state; no notice for a
> marked dataset that no workspace shows. In the test cell project both engine tiers opt in.

## Problem / Motivation

A project that serves more than one device profile from one dashboard has every profile's
datasets live at once. The APU test cell project is the case in hand: two engine tiers, APS500
and GTCP85, read the same acquisition channels through their own groups, each tier on its own
workspace, and the engine under test decides which tier means anything. Thermocouple channel 5
is EGT probe 5 to the APS500 and the oil temperature to the GTCP85. During an APS500 run that
channel sits near 1000 F, so the GTCP85 Oil Temperature dataset is in its Critical band (255 F
switch) from light-off to shutdown, and the GTCP85 Inlet Air Temperature dataset, fed by EGT
probe 6, is in its Critical band (150 F) the same way. Two annunciator points, two sounds and
two notifications, for a tab nobody is looking at, on every run.

The alarm system today has exactly one lever below the master enable: Suppress Alarms (spec
0093, amendment 1), a static per-dataset opt-out. It cannot express "this dataset matters when
its profile is on screen", so the operator either hears the idle profile's alarms or loses the
GTCP85 alarms for good. Spec 0093 R10 made alarms independent of what is on screen, and that
stays right for the common project: a tachometer redline on another tab must still sound. What
is missing is a per-dataset opt-in to the opposite rule, so that one project can carry several
profiles and only the profile the operator has in front of them alarms. The maintainer chose
this design on 2026-10-28 over a project-side gate in the transforms (which would have turned
the idle tier's values into text and emptied its recordings).

The dashboard already knows what is on screen: which workspace each dashboard window shows,
whether a window is minimized, whether a widget on a canvas is normal, minimized or closed,
and which widgets are popped out. The console annotation layer already pauses on the same kind
of fact (no view showing it), so "gate on a view being on screen" has a precedent in the code
base.

## Goals

- A dataset can be marked so that its band alarms exist only while one of its widgets is on
  screen; every unmarked dataset keeps the spec 0093 behaviour unchanged.
- One project can hold several device profiles on their own workspaces, with the idle
  profiles silent and the shown profile alarming exactly as a single-profile project would.
- Revealing a marked dataset whose value is already in an alarm band alarms at once; hiding it
  again removes its alarm points and the audible follows.
- The option is set, stored, edited and read through the same surfaces as Suppress Alarms, so
  nothing new has to be learned to use it.
- Nothing runs per frame, and band evaluation keeps its display-tick cadence.

## Non-Goals

- No automatic profile selection: the application does not know which profile is "the one
  under test"; the operator's choice of tab is the signal, and that is the whole feature.
- No per-band, per-group or per-workspace variant of the option (maintainer decision,
  2026-10-28). A group or workspace is marked by marking its datasets, which the multi-selection
  editor already does in one step.
- No change to what is recorded, exported or published: a hidden marked dataset still logs its
  values, still drives the Historian and every sink, and its widgets still colour and flash
  their bands when shown. Only notifications, annunciator points and sounds are gated.
- No change to the applicability rule of spec 0093 R1, to Suppress Alarms, to band evaluation
  (range clamp, first-match order, deadband, cooldown), to the ISA-18.1 sequences or to the
  event sounds.
- No occlusion tracking: a window covered by another application, on a locked screen or on a
  disconnected monitor still counts as visible if the dashboard considers it shown. The
  dashboard's own knowledge of windows, tabs and widget states is the only source of truth.
- No notification, log entry or point for band transitions that happen while a marked dataset
  is hidden; what happened off screen leaves no trace, and a reveal does not say how long the
  value has been in its band (maintainer decision, 2026-10-28).
- No Problem Center finding or editor notice for a marked dataset that no workspace shows and
  that can therefore never alarm, consistent with spec 0093's silence on ignored bands
  (maintainer decision, 2026-10-28).
- No new alarm point state: a hidden marked dataset is absent from the annunciator and the
  alarm state the API exposes, not listed as "hidden" (maintainer decision, 2026-10-28).
- The external dashboard window keeps a fresh id across sessions today, which orphans its
  per-window layouts (found 2026-10-28 in the same project). That is a separate defect and is
  not addressed here.

## Requirements

1. **R1 — The option.** Every dataset has an *Alarms only while visible* option, off by default.
   Spec 0093 R10 (alarms whether or not the widget is on screen) continues to hold for every
   dataset whose option is off.
2. **R2 — Visible, defined.** A marked dataset is *visible* when at least one widget showing it
   is on screen: the widget sits on the active workspace of the main window's dashboard or of an
   external dashboard window, that window is not minimized, in the main window the Dashboard
   pane is the shown view (a Console or Project Editor pane in front hides the dashboard), and
   the widget's own state is normal, neither minimized nor closed. A popped-out widget window
   counts while it is open. A frozen dashboard is still on screen and counts. A Bar Panel member
   is visible through its panel, an LED Panel member through its panel. Any other situation is
   *hidden*; windows covered by other applications, locked screens and disconnected monitors
   are not tracked (maintainer decisions, 2026-10-28).
3. **R3 — The gate.** For a marked dataset whose bands are applicable (spec 0093 R1) and whose
   Suppress Alarms option is off, a band-entry notification is posted, a band alarm point is
   raised, listed and sounded, only while the dataset is visible. While it is hidden it behaves
   as a dataset with no applicable bands: no notification, no point, no sound.
4. **R4 — Reveal.** When a marked dataset becomes visible while its value sits in a Warning or
   Critical band, that counts as entering the band: a fresh, unacknowledged point is raised, the
   notification is posted and the audible is arbitrated, even if the operator had acknowledged
   the same band before hiding it; the usual 3 s per-dataset, per-severity cooldown applies, so
   flipping between tabs cannot repeat the notification faster than a band entry could. When a
   marked dataset becomes hidden, its band point is removed and the audible is re-arbitrated,
   exactly as spec 0093 R4 does for a dataset that stops being applicable. Nothing is remembered
   across a hide (maintainer decisions, 2026-10-28).
5. **R5 — Promptness.** A change of visibility takes effect within one second, whichever way it
   happened: a tab switch in any dashboard window, a window opened, closed, minimized or
   restored, a widget minimized, closed, restored, popped out or popped back in, or the same
   actions driven through the API or a script.
6. **R6 — Precedence.** Suppress Alarms wins: a dataset with both options on never alarms, visible
   or not, and its bands stay drawn (spec 0093 R11). The option on a dataset whose bands are not
   applicable, or that has no bands, is stored and ignored, like the bands themselves.
7. **R7 — Project defines alarms.** A marked dataset with applicable bands counts toward the
   taskbar's alarms-disabled indicator and the Problem Center warning for a disabled master
   enable, whether it is visible or not, because it can alarm.
8. **R8 — Default and persistence.** The option is off by default. A project file stores it only
   while it is on, under the dataset key `alarmsWhileVisible`; a file without the key loads with
   the option off, and resaving a project that never used it leaves the file unchanged. No
   schema version bump (maintainer decision on the key, 2026-10-28).
9. **R9 — Editor.** The dataset form shows an *Alarms only while visible* checkbox directly
   beside Suppress Alarms, under the same conditions (the dataset has bands that a widget draws),
   editable in a dataset multi-selection, shown as Mixed when members disagree, one undo step per
   change. The Alarm Bands action is unaffected by the option. The label is the one name for the
   option across the editor, the manual, the API descriptions and the assistant corpus.
10. **R10 — API.** The option is readable and writable through the dataset update and
    per-dataset option commands, in the same way as Suppress Alarms, and a dataset read returns
    it. The alarm state the API exposes lists no point for a hidden marked dataset; a client
    that wants to know why reads the dataset's option.
11. **R11 — Stated contract.** The band read and write command descriptions and the manual state
    the rule: which datasets alarm, that a marked dataset alarms only while visible, what visible
    means (R2), and that Suppress Alarms wins.
12. **R12 — Build parity.** Identical in GPL and commercial builds, and in operator runtime
    deployments, where the external dashboard window is usually the only window the operator
    sees.
13. **R13 — Toggling while connected.** Turning the option on or off during a session applies at
    the next dashboard rebuild, as Suppress Alarms does (spec 0093 R14): on, with the widget
    hidden, removes the point and re-arbitrates the audible; off restores spec 0093 behaviour and
    raises the point again if the value is still in band. No reconnect is needed (maintainer
    decision, 2026-10-28).

## Acceptance Criteria

- [ ] **AC1 (R2, R3, R6)** — A unit test tabulates the rule over option on/off, visible/hidden,
  Suppress Alarms on/off and applicable/not: only {option on, visible, not suppressed,
  applicable} and {option off, not suppressed, applicable} alarm; the drawn-bands table of spec
  0093 AC1 is unchanged by the option.
- [x] **AC2 (R3)** — Integration, live data: a marked Gauge dataset on workspace B is driven into
  its Critical band while workspace A is active; the alarm state shows no point and no band
  notification is posted. The same dataset with the option off raises the point.
- [x] **AC3 (R4)** — Integration: continuing AC2, switching the active workspace to B through the
  API raises the point and posts the notification without a new sample; switching back to A
  removes the point.
- [x] **AC4 (R2, R5)** — Integration: with workspace B active and the point raised, minimizing the
  widget through the window-state command removes the point within a second; restoring it
  raises the point again. The same holds for minimizing and restoring the whole window where the
  API can drive it.
- [ ] **AC5 (R2)** — Maintainer observation with two dashboard windows: the point exists while
  the marked dataset's workspace is the active tab of either window, and disappears when neither
  shows it; a popped-out widget keeps it while the pop-out is open; switching the main window to
  the Console pane while it is the only window showing the workspace removes the point, and
  freezing the dashboard does not.
- [x] **AC5b (R4)** — Integration: with the point raised and acknowledged, switching the active
  workspace away and back raises a new unacknowledged point and posts a notification once the
  cooldown has elapsed; two switches inside the cooldown post one notification.
- [x] **AC5c (R13)** — Integration: with a point raised on a hidden workspace for an unmarked
  dataset, setting the option through the API removes the point at the next rebuild; clearing it
  brings the point back while the value is still in the band.
- [x] **AC6 (R6)** — Integration: the AC2 dataset with both options on raises no point visible or
  hidden; its gauge still shows the band colour.
- [ ] **AC7 (R7)** — Integration: with the master enable off, a project whose only bands sit on a
  marked, hidden dataset still reports the alarms-disabled finding.
- [ ] **AC8 (R8, R10)** — Integration: the option set through the API is present in the exported
  project and survives a reload; a project that never set it exports without the key; the
  existing serialization round-trip units stay green.
- [ ] **AC9 (R9)** — Maintainer observation in the running editor: the checkbox appears beside
  Suppress Alarms for a Gauge with bands and is absent otherwise; a multi-selection whose members
  disagree shows Mixed; toggling is one undo step; the Alarm Bands button stays enabled.
- [x] **AC10 (R11)** — The documentation, terminology and claim checks pass with the reworded
  contract.
- [ ] **AC11 (R5, constraint)** — Nothing runs per frame on the acquisition pipeline and the
  hotpath benchmark gate is unchanged; the existing band-entry integration tests stay green with
  no change to the workspace.
- [ ] **AC12 (motivating case)** — The APU test cell project, with its GTCP85 tier marked and the
  APS500 simulator profile running, raises no GTCP85 point while the APS500 workspace is shown
  and raises the GTCP85 Oil Temperature point within a second of switching to the GTCP85 tab.

## Constraints & Invariants

- **Deciding constraint: one rule, one home.** Whether a dataset's bands alarm has a single
  answer today (drawn, non-empty, not suppressed). The visibility term joins that one answer; no
  consumer evaluates visibility on its own, or the orphan-alarm class of spec 0093 returns with a
  new face (a point listed for a dataset the editor says cannot alarm, or the reverse).
- Visibility is derived from state the dashboard already keeps about its windows, tabs and
  widget states, re-derived on the events that change it, never by walking the widget tree on
  every evaluation; band evaluation stays at display-tick rate and nothing touches the frame
  pipeline.
- The held point table of spec 0088 R3 (link drop) is never reaped by a visibility change.
- The reveal reuses the existing per-dataset cooldown and the existing point model; no new
  cooldown, no new point state and no memory of acknowledgements across a hide.
- Identical behaviour in GPL and commercial builds and in sealed runtime deployments.
- No project-file schema change and no version bump: one optional dataset key, written only when
  true, which older versions ignore (they keep alarming, which is the pre-0105 behaviour).
- The key must not reuse the legacy `alarmEnabled` name, whose `false` deletes bands.
- This amends spec 0093 in two places, both on purpose: R10 becomes "for a dataset whose Alarms
  only while visible option is off", and the non-goal "whether the widget is on screen stays
  irrelevant" is narrowed the same way. Everything else in 0093 stands.
- Existing integration tests that assert alarms on a hidden workspace keep passing, since no
  dataset in them has the option on.

## Premise Check

- **Question:** Does one project serving two profiles put the idle profile's datasets in alarm
  bands, and would hiding the profile's tab be enough to silence them? If the idle tier
  alarmed only transiently, or the idle tier had to stay on screen, a static opt-out would do and
  this spec would be waste.
- **Measured:** From the APU test cell project (two engine tiers, 2026-10-28), by cross-reading
  every banded dataset of one tier against the role the same channel has on the other tier.
  Idle GTCP85 during an APS500 run: 2 datasets, Oil Temperature and Inlet Air Temperature, read
  EGT probes 5 and 6 (985 F steady in the simulator's APS500 profile, up to the 1303 F run limit
  on the engine) against Critical bands that start at 255 F and 150 F: 2 Critical points for
  the entire run, every run. Idle APS500 during a GTCP85 run: 0 datasets in a band (probes 5 and
  6 then read oil and inlet temperatures, inside the EGT Normal band). During a test the idle
  tier's workspace is not on screen, so the tab is a faithful signal for this project.
- **Verdict:** The premise holds: the false alarms are permanent, not transient, and tab
  visibility separates the two profiles cleanly. The cost side (a visibility fact per tracker at
  display-tick rate) cannot be measured without a build; it is bounded by the constraint above
  and belongs to the plan's hotpath answer.

## Open Questions

None at the gate. The draft's three questions (key and label, the main window's shown pane,
a record of hidden band time) and five more raised while closing it (reveal as a fresh entry
under the cooldown, the live toggle, the API state of a hidden dataset, a never-shown notice,
which test cell datasets opt in) were decided by the maintainer on 2026-10-28 and are folded
into the requirements and non-goals above. One note for the plan rather than a question: the
operator deployment of the test cell shows the dashboard in the external window, so R2's main
window pane rule is exercised there only when the main window is also on screen.

Follow-up outside this spec: once the feature ships, every banded dataset under the test cell
project's `Engines/APS500` and `Engines/GTCP85` groups opts in; the Generator panel, the DAQ
tiers and the load bank keep alarming from any tab (maintainer decision, 2026-10-28).
