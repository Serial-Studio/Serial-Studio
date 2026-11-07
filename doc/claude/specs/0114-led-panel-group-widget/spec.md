---
spec: 0114-led-panel-group-widget
title: LED Panel as a Group Widget
status: approved     # draft -> approved -> in-progress -> done | shelved
created: 2026-11-07
author: Alex Spataru
---

# Spec 0114 — LED Panel as a Group Widget

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

Every panel on the dashboard is a group: the user picks a group widget (Data Grid, Multi-Plot,
Bar Panel, GPS Map, 3D Plot) and the group's datasets become its rows, curves or bars. The LED
Panel is the one exception. There is no "LED Panel" group kind; instead each dataset carries a
**Show in LED Panel** checkbox, and the dashboard assembles a panel per group out of whichever
datasets have it ticked. The panel borrows the host group's title and identity, so one group
produces two dashboard widgets that share a name and an id. The manual's widget table lists the
LED Panel's key as "auto" because there is nothing a user can type.

The flag was meant to let one dataset be both a Data Grid cell and an LED without duplicating
it. In practice nobody uses that: all 21 LED datasets across the seven bundled examples sit in
Data Grid groups with no plot, no FFT and no dataset widget. Meanwhile the design costs real
things. A user who wants an LED-only panel has to create a group with *no* widget and tick every
dataset, which is a group widget spelled awkwardly. **Hide on Dashboard** removes a dataset from
the LED panel too, so "LED but not in the grid" is impossible. The Project Editor offers an
"LED Indicator" dataset button beside Plot and Gauge, while the panel it feeds behaves like Bar
Panel. The device-declaration library (spec 0111) mirrors the split into firmware: a sketch
declares a Bar Panel on the group but an LED on each channel. And the alarm-band rule carries
an LED special case that would collapse into the Bar Panel branch if the panel were a group.

The firmware library is at version 1.0.0 and not yet registered with the Arduino Library
Manager. Once it is, removing a channel call becomes a breaking change for every sketch; today
it is free. That fixes the order of work: this spec lands before the registry PR.

## Goals

- An LED Panel is a group widget the user picks like Bar Panel; every dataset in the group is
  one LED.
- The per-dataset LED checkbox is gone from the Project Editor, the project file, the API and
  the firmware declaration vocabulary.
- A project saved before this change opens with the same set of dashboard widgets, in the same
  order, showing the same values and colors, without the user doing anything.
- Alarm bands on an LED Panel member behave exactly as they do today on an LED (color, label,
  blink, notifications, aural alerts, visible-only alarms).
- The manual, the examples, the assistant corpus and the firmware library describe one way to
  make an LED Panel.

## Non-Goals

- No change to how an individual LED is drawn or to the panel's column layout.
- No change to alarm-band semantics, severities, cooldowns or the deadband.
- No new per-LED options (shapes, labels, latching). A single new group-level option is in
  scope only if the plan finds it necessary for parity; none is expected.
- No change to Bar Panel, Data Grid or any other group widget.
- No change to the dataset `ledHigh` threshold's meaning; it stays a per-dataset property.
- No attempt to merge a migrated panel back into its source group on save.

## Requirements

1. **R1 — Group widget.** The group widget list offers an LED Panel with its own key, chosen
   wherever Bar Panel can be chosen: the Project Editor's group widget selector, the add-widget
   dialog, the API's group commands and the firmware declaration vocabulary.
2. **R2 — Membership is the group.** Every dataset in an LED Panel group is shown as an LED.
   **Hide on Dashboard** on a member hides that LED only, the same as it hides a Bar Panel row.
   No other property decides membership.
3. **R3 — LED state.** A member with alarm bands lights in the active band's color, shows the
   band's label and blinks when the band says so, and is off outside every band. A member
   without bands lights in the dataset color when its value meets or exceeds its `ledHigh`
   threshold. This is today's behavior, unchanged.
4. **R4 — Bands apply.** A dataset in an LED Panel group is one of the places where alarm bands
   are drawn and alarm, under the one rule that already covers Bar, Gauge, Meter and Bar Panel
   members. A dataset that leaves an LED Panel group keeps its bands but they stop drawing and
   alarming, exactly as when a Bar widget is removed.
5. **R5 — Flag removed.** The **Show in LED Panel** checkbox and its project key are removed.
   The LED threshold field stays, shown for members of an LED Panel group that have no bands,
   as it is shown today for flagged datasets without bands.
6. **R6 — Faithful migration.** On opening a project written before this change, every group
   that contains flagged datasets gains a new LED Panel group immediately after it, holding a
   copy of each flagged dataset (same frame index, title, units, threshold, bands and format).
   The source group keeps its datasets, minus the flag. The new group's title is the source
   group's title followed by a short, translated suffix. The migrated project is marked
   modified so the user is prompted to save it in the new form.
7. **R7 — Migration is one-shot and idempotent.** A project saved after migration never
   migrates again. A project with the old key and no flagged dataset changes only by dropping
   the key.
8. **R8 — Quick Plot and declarations.** A device declaration naming the LED Panel group
   widget produces an LED Panel group. The per-channel LED call and key are removed from the
   Arduino, C and Python declaration vocabularies; a stream that still sends the old key is
   accepted and the key ignored, so pre-release firmware does not fail to parse.
9. **R9 — API.** The project API exposes the LED Panel as a group widget value and no longer
   exposes a per-dataset LED option. A client that sets the removed option on a dataset gets
   the same refusal as any unknown property. Generated API surfaces (gRPC, the SDK, the
   schema dump) follow from the registry; the removed field's gRPC number is reserved, never
   reused.
10. **R10 — Editor affordances.** The dataset toolbar's **LED Indicator** button is replaced by
    an LED Panel entry among the group widgets. Adding a dataset to an LED Panel group
    pre-configures it the way adding one to a Bar Panel does.
11. **R11 — Documentation.** The manual's widget reference lists the LED Panel with its key
    and "1+" datasets like Bar Panel, the "auto" row disappears, the Project Editor page drops
    the LED dataset section, and every other page that mentions the flag describes the group
    widget instead. The assistant corpus and the firmware README follow.
12. **R12 — Examples.** The seven bundled examples that use the flag are re-saved in the new
    form so a fresh install never shows a migration prompt for an example.

## Acceptance Criteria

- [ ] **AC1 (R1, R9)** — With the app up, the API group-widget enumeration contains the LED
      Panel key and the dataset property listing contains no LED option; the property-registry
      integration test passes with regenerated baselines.
- [ ] **AC2 (R6, R7)** — For each of the seven examples, a scratchpad script that loads the
      pre-change file and the migrated save reports the same dashboard widget multiset
      (one Data Grid plus one LED Panel per affected group, same member titles and thresholds).
      Opening the migrated save again produces no further change. This runs as a pytest
      integration test against the running app, loading the pre-change files from a fixture
      directory.
- [ ] **AC3 (R3, R4)** — The alarm-band applicability unit test gains cases for an LED Panel
      member (bands draw and alarm), an LED Panel member with Hide on Dashboard (no widget, no
      alarm) and a dataset moved out of an LED Panel group (bands kept, inert). It passes under
      `ctest`.
- [ ] **AC4 (R3)** — In the running app, the CAN Bus example's door and light LEDs light,
      label and blink as before the change (maintainer observation against a screenshot taken
      before the change).
- [ ] **AC5 (R8)** — The firmware goldens (C, Arduino, Python) no longer contain the LED key,
      the LED Panel group widget round-trips through a declaration into a project, and a
      golden stream that still carries the old key parses with the key ignored.
- [ ] **AC6 (R5, R10)** — In the Project Editor, a dataset shows no LED section; a group set to
      LED Panel shows the threshold field on a member without bands and hides it once bands
      exist; the add-widget dialog lists LED Panel among group widgets.
- [ ] **AC7 (R11)** — `documentation-verify.py` and `term-verify.py` pass; a search of
      `doc/help`, the assistant corpus and the firmware README for the removed checkbox label
      and key returns nothing except the migration note.
- [ ] **AC8 (R9)** — The gRPC descriptor diff shows the dataset field number reserved, no field
      renumbered; the generated-surface check in CI passes.
- [ ] **AC9** — `--benchmark-hotpath` is unchanged: the change touches project structure and
      widget assembly, never the per-frame path.

## Constraints & Invariants

- **Faithful first.** The deciding constraint: a migrated dashboard must match the old one
  widget for widget. 6 of the 9 example groups that hold LED datasets are mixed (LED and
  non-LED datasets together), so retargeting the source group's widget would delete Data Grids
  users rely on. Copying into a sibling group is the only faithful rule.
- **One home for band applicability.** Whether a dataset's bands draw, and whether they alarm,
  keep their single existing rule; the LED Panel joins it as "member of an LED Panel group",
  not as a second rule.
- **Generated surfaces are never hand-edited.** The dataset property registry drives the
  project keys, the editor form, the API, the gRPC descriptor, the SDK and the firmware key
  vocabulary; the change is made once in the registry and regenerated.
- **gRPC field numbers are append-only.** The removed field's number is reserved.
- **Wire compatibility for the dashboard mirror.** Dropping a dataset key from the serialized
  project and adding migrated groups changes what a viewer receives; the plan decides whether
  this is a wire-version bump with regenerated fixtures or stays inside the current version.
- **Saved layouts.** A migrated LED Panel is a new group with a new unique id, so a layout the
  user saved for the old auto-panel does not carry over; auto-layout places it once. No
  attempt is made to rebind it.
- **No change on the per-frame path, no new allocation in widget assembly beyond what Bar
  Panel already does.**
- **Order of work.** This lands, and the examples and firmware goldens are regenerated, before
  the Arduino Library Manager registration PR (spec 0111 T46).
- **Undo.** Setting a group's widget to LED Panel is an ordinary undoable edit. The load-time
  migration is not an undo step; it marks the project modified.
- **Sealed and unlicensed sessions** are unaffected: LED Panel is a free-tier widget today and
  stays one, and the migration adds no script or transform content.

## Premise Check

- **Question:** Does the flag's one advantage, a dataset that is an LED *and* something else
  without duplication, bind in practice, and can a faithful migration be done by retargeting
  the group's widget alone?
- **Measured:** Across the seven bundled examples, 21 datasets carry the flag. 21 of 21 have
  no dataset widget, no plot and no FFT; all sit in Data Grid groups. Of the 9 groups that hold
  them, 3 are all-LED and 6 are mixed (for example Modbus PLC Simulator: 9 datasets, 2 LEDs;
  Sparkplug: 7 datasets, 3 LEDs).
- **Verdict:** The additive premise does not bind, so removing the flag loses nothing the
  examples exercise. Retargeting alone would delete six Data Grids, so the copy-into-sibling
  rule in R6 is required, not optional. Proceed.

## Open Questions

Resolved with the maintainer on 2026-11-07:

- **Q1 — `ledHigh` stays** as the no-bands fallback (R3, R5). Retiring it in favor of an
  Indicator band would change meets-or-exceeds into a band range and rewrite thresholds during
  migration.
- **Q2 — Migrated group title** is "<Group> LEDs", translated. Two groups get distinct names in
  the editor tree; the dashboard title differs slightly from the old auto-panel.
- **Q3 — Firmware** removes the per-channel LED call and key from the Arduino, C and Python
  vocabularies; the parser still accepts and ignores the old key (R8).
- **Q4 — Examples** are migrated by the implementation with the same rule the loader applies,
  in one commit that touches only the seven `.ssproj` files, diff reviewed before it lands.

Still open, decided in the plan:

- **Q5 — Mirror wire version.** Whether dropping the dataset key and adding migrated groups is
  a wire-version bump with regenerated fixtures or stays inside the current version.
