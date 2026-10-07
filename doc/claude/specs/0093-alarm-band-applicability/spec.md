---
spec: 0093-alarm-band-applicability
title: Alarm-band applicability
status: in-progress  # approved 2026-10-06
created: 2026-10-06
author: Alex Spataru
---

# Spec 0093 — Alarm-band applicability

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.
>
> **Amendment 1 (approved 2026-10-06):** a per-dataset opt-out, R11 to R14 and AC11
> to AC15. R1 to R10 were approved and implemented before it; the maintainer chose its
> semantics, key name and editor placement on 2026-10-06.

## Problem / Motivation

Alarm bands are drawn by five widgets: Bar, Gauge, Meter, LED Panel and Bar Panel. The Project
Editor offers the band editor on most of them. The alarm system does not look at widgets at
all: band-entry notifications, annunciator points and their sounds, and the "this project
defines alarms" indicator all key on one fact, that the dataset holds at least one band.

Bands reach datasets that no band-drawing widget shows through four routes: the widget was
changed after the bands were defined (only a switch to Compass clears them), the
multi-selection band editor writes to every selected dataset, an API write accepts any
dataset, and the v3.3 migration turns the old alarm fields into bands regardless of widget.
The operator then hears an alarm for a band that is drawn nowhere, and the single-dataset
editor refuses to open the band editor for that dataset, so the cause can be neither seen nor
removed there. The reverse gap exists too: a Bar Panel member with no widget of its own draws
bands and alarms, yet the single-dataset editor will not open its band editor.

Measured on 2026-10-06 over the shipped examples, bundled resources and integration
baselines: 60 projects, 56 datasets with bands, 3 of them without a band-drawing widget. All
three are in the System Monitor example (CPU, RAM and Disk usage: plot-only datasets with a
Warning band from 90 up).

A band's severity decides both its color and whether it alarms, and nothing else lets a
project keep a band on screen without annunciating it. A tachometer redline that should color
and flash the gauge but never sound has to be demoted to Info or OK with a color override,
which also stops the widget flashing in it. The legacy `alarmEnabled` API alias is no answer:
setting it to false deletes the dataset's bands. The maintainer asked for a per-dataset
opt-out, off by default (amendment 1).

## Goals

- One rule decides whether a dataset's bands are live, and the alarm system and the Project
  Editor always give the same answer.
- A dataset with bands but no band-drawing widget never notifies, never raises an alarm point
  and never sounds.
- The band editor is reachable for exactly the datasets whose bands are live, Bar Panel
  members included.
- The multi-selection band editor cannot create bands on a dataset that will ignore them.
- Nothing is lost: ignored bands stay in the project file, and configuring a band-drawing
  widget again brings them back to life.
- A project can keep a dataset's bands on screen, colors and flashing included, while the
  alarm system ignores that dataset (amendment 1).

## Non-Goals

- No Problem Center finding, warning or other notice for ignored bands (maintainer decision,
  2026-10-06).
- Canvas groups and widget extensions do not make bands applicable, even though both can read
  band data.
- No stripping or migration of bands in existing files, and no new clearing of bands on a
  widget change. The existing clear on a switch to Compass stays as it is.
- No change to the band read and write commands' behaviour: band writes are still accepted
  for any dataset and band reads return what is stored. Their description text is corrected,
  and the only API addition is the opt-out's own dataset field (R14).
- Suppression is a per-dataset configuration only: no per-band suppression, no time-limited
  shelving in the ISA-18.2 sense, and no change to how widgets color or flash bands.
- No change to how the five widgets draw bands, to band evaluation (range clamp, first-match
  order, cooldown), to notification-kind alarm points, or to the ISA-18.1 sequences.
- Whether an applicable dataset's widget is on screen (active workspace, minimised, popped
  out) stays irrelevant.
- FFT frequency-marker warning and alarm levels are a separate mechanism and are untouched.
- The System Monitor example is not edited here (see Open Questions).

## Requirements

1. **R1 — Applicability rule.** A dataset's alarm bands are *applicable* when, and only when,
   at least one of these holds:
   (a) the dataset's group is shown as a Bar Panel;
   (b) the dataset is not hidden from the dashboard and its widget is Bar, Gauge or Meter;
   (c) the dataset is not hidden from the dashboard and it is an LED Panel member.
2. **R2 — Notifications.** A band-entry notification is posted only for a dataset whose bands
   are applicable.
3. **R3 — Alarm points and sound.** A band alarm point is raised, listed and sounded only for a
   dataset whose bands are applicable.
4. **R4 — Changes while running.** When a dataset with an active band point stops being
   applicable, the point is removed at the next dashboard rebuild and the audible is
   re-arbitrated, exactly as when the dataset is deleted today. When a dataset becomes
   applicable it behaves like a newly added banded dataset.
5. **R5 — "Project defines alarms".** The taskbar's alarms-disabled indicator and the
   Problem Center warning for a disabled master enable count only enabled datasets with
   applicable bands. A project sound map still counts on its own, as today.
6. **R6 — Single-dataset editor.** The Alarm Bands action is enabled exactly when the selected
   dataset's bands would be applicable. A Bar Panel member with no widget of its own can open
   it; a dataset hidden from the dashboard outside a Bar Panel cannot.
7. **R7 — Multi-selection editor.** The Alarm Bands action is enabled when at least one
   selected dataset is applicable. The dialog's scale and prefill come from the applicable
   members only, and applying writes to the applicable members only; the others keep their
   bands untouched.
8. **R8 — Ignored bands are preserved.** Bands on a non-applicable dataset survive load and
   save unchanged, and the readers that are not part of the alarm system (LED threshold
   fallback, Canvas scripts, widget extensions, the band read API) see them as before.
9. **R9 — Stated contract.** The band read and write API descriptions and the manual state the
   rule: which datasets alarm, and that bands elsewhere are stored and ignored.
10. **R10 — Visibility independence.** An applicable dataset notifies and alarms whether or not
    its widget is currently instantiated or on screen.
11. **R11 — Per-dataset suppression (amendment 1).** A dataset whose Suppress Alarms option is
    on never posts a band-entry notification, never raises or lists a band alarm point, and
    never sounds, even when its bands are applicable. Its bands are still drawn, and its
    widgets still color and flash exactly as they do without the option.
12. **R12 — Default and persistence (amendment 1).** The option is off by default. A project
    file stores it only while it is on, under the key `suppressAlarms`; a file without the
    key loads with the option off, and resaving a project that never used it leaves the file
    unchanged.
13. **R13 — Editor (amendment 1).** The dataset form shows a Suppress Alarms checkbox as the
    last row of Widget Settings while the dataset has bands that a widget draws, and hides it
    otherwise. It is editable in a dataset multi-selection (shown as Mixed when members
    disagree), and each change is one undo step. The Alarm Bands action stays available for a
    suppressed dataset, because its bands are still drawn.
14. **R14 — Interplay (amendment 1).** Suppressed datasets do not count toward "project defines
    alarms" (R5). Toggling the option while connected removes or restores the band point at
    the next dashboard rebuild, as R4. The option is readable and writable through the dataset
    API as `suppressAlarms`, and the legacy `alarmEnabled` alias keeps its current behaviour.

## Acceptance Criteria

- [ ] **AC1 (R1)** — A unit test tabulates the rule: Bar, Gauge, Meter and LED are applicable;
  no widget, Compass and plot-only are not; a hidden Gauge is not; a Bar Panel member is,
  hidden or not; a Canvas group member with no widget is not.
- [x] **AC2 (R2, R3)** — Integration, live data: a banded dataset with no widget driven into
  its Critical band raises no alarm point and posts no band notification; the same dataset
  with a Gauge raises a Warning-priority point.
- [x] **AC3 (R1a)** — Integration: a Bar Panel member with no widget of its own raises a point.
- [x] **AC4 (R1b)** — Integration: a Gauge dataset hidden from the dashboard raises no point.
- [x] **AC5 (R4)** — Integration: with a band point active, removing the dataset's widget
  through the API removes the point; restoring the widget brings it back while the value is
  still in the band.
- [x] **AC6 (R5)** — Integration: with the master enable off, a project whose only bands sit on
  a widget-less dataset reports no alarms-disabled finding; giving that dataset a Gauge makes
  the finding appear.
- [ ] **AC7 (R6, R7)** — Maintainer observation in the running editor: the Alarm Bands button is
  enabled for Bar, Gauge, Meter, LED and Bar Panel member datasets and disabled for a
  plot-only dataset and a hidden Gauge; on a mixed multi-selection, applying bands changes the
  applicable datasets and leaves the others as they were.
- [ ] **AC8 (R8)** — Integration: bands written to a plot-only dataset are still present in the
  exported project; the existing serialization round-trip units stay green.
- [x] **AC9 (R9)** — The documentation and claim checks pass with the reworded contract.
- [x] **AC10 (R10)** — The existing band-entry integration test passes with its dataset given a
  Gauge and no change to the workspace.
- [x] **AC11 (R11, R14)** — Integration, live data: a suppressed Gauge dataset driven into its
  Critical band raises no alarm point and posts no band notification; turning the option off
  through the API and sending more in-band frames raises the point.
- [ ] **AC12 (R11)** — Unit: a second rule table shows that a suppressed Gauge, LED and Bar
  Panel member raise no alarms while the first table still reports their bands as drawn.
- [x] **AC13 (R12, R14)** — Integration: `suppressAlarms` set through the API is present in the
  exported project and survives a reload; a project that never set it exports with no
  `suppressAlarms` key at all.
- [ ] **AC14 (R13)** — Maintainer observation in the running editor: the row appears for a
  Gauge with bands and is absent for a Gauge without bands and for a plot-only dataset;
  toggling it is one undo step; a multi-selection whose members disagree shows Mixed; the
  Alarm Bands button stays enabled for a suppressed Gauge.
- [x] **AC15 (R14)** — Integration: with the master enable off, a project whose only bands sit
  on a suppressed Gauge reports no alarms-disabled finding; turning the option off makes the
  finding appear.

## Constraints & Invariants

- **Deciding constraint: one rule, one implementation.** It must be derived from the
  dashboard's own widget resolution so it cannot drift from what the dashboard builds, and
  every consumer (band tracking, point reaping, the project-defines-alarms flag, both editor
  buttons) must answer from it.
- Identical in GPL and commercial builds: all five widgets exist in both.
- Nothing on the frame hotpath changes; band evaluation stays at display-tick rate.
- The link-drop hold from spec 0088 R3 is preserved: a held point table is never reaped.
- This deliberately reverses two documented behaviours: band notifications for a dataset
  hidden from the dashboard, and for a dataset with no band-drawing widget. Both are stated
  today in the dashboard architecture notes and in the band-write API description.
- No project-file schema change and no version bump. Amendment 1 adds one optional dataset key,
  written only when true, which older versions ignore (they keep alarming).
- The opt-out's key must not reuse the legacy `alarmEnabled` name, whose `false` deletes bands.
- Existing integration tests that rely on a widget-less banded dataset alarming are updated in
  the same change.

## Open Questions

- **System Monitor example.** It is the only shipped project affected: its three usage
  datasets carry a Warning band that will stop notifying. Recommendation: leave the example
  unchanged in this spec. Giving those datasets a Bar widget changes the example's dashboard
  and feeds the mirror fixtures and an integration baseline, so it is a separate decision.
