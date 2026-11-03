---
spec: 0108-dataset-formats-and-labels
title: Dataset Value Formats and Labels
status: approved     # approved 2026-11-02: scale and offset free, integer label keys, number when unlisted
approved: 2026-11-02
created: 2026-11-02
author: Alex Spataru
---

# Spec 0108 — Dataset Value Formats and Labels

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

A dataset's value is a number the firmware printed, and the only presentation choices the
Project Editor offers are how many decimals to show (Auto, 0 to 3, Scientific). Anything a real
firmware emits that is not a plain decimal needs a transform script: a state code that means
"Reverse", a status word whose bit 3 is "overtemp", a raw Q15 sample from an IMU, a register
to read in hexadecimal. Three things go wrong at that point, all of them verified in the current
tree rather than hypothesised:

- **A label costs the number.** A transform that returns text makes the dataset non-numeric.
  The plot silently drops the curve, the gauge, bar and alarm bands have nothing to work with,
  and the Historian records text. The CAN Bus example's `CurrentGear` dataset is exactly this
  shape: it shows "Reverse" in the grid and cannot be plotted or alarmed.
- **A label costs a license.** A transform is Pro content (spec 0094): a UART project with one
  lookup table refuses to connect unlicensed. A hobbyist naming four gearbox states pays for a
  scripting engine they never wanted.
- **Importers write code the user cannot read.** The DBC importer turns a value table into a
  Lua chunk; the Modbus map importer turns scale and offset into another. The user sees a
  script they did not write, in a language they may not know, holding what is a table
  of four rows or two numbers.

The audience is the one Serial Studio is known for: a sketch printing comma-separated values.
The sketch already describes its own meaning in the firmware source; today the dashboard can
only show it by moving that meaning into a second program.

## Goals

- A dataset's raw value can be decoded declaratively, without a script: fixed-point with a
  chosen number of fraction bits, a bit field cut out of an integer, and a linear scale and
  offset.
- A dataset's value can be shown in hexadecimal or binary notation without changing the value.
- A dataset can carry a table of value-to-text labels, and every place that shows the value as
  text shows the label, while every numeric consumer keeps the number.
- The DBC and Modbus map importers produce these declarative forms instead of script wherever
  the source file expresses one.
- A project that uses only declarative formats connects without a license.
- Quick Plot and every existing project behave exactly as today.

## Non-Goals

- Not a replacement for transforms. Arbitrary math, cross-dataset reads and stateful filters stay
  in JavaScript or Lua. Transforms remain available on top of a declarative decoder.
- No new frame parsing. The decoder works on the value the parser already produced for the
  dataset; how fields are split out of a frame is unchanged.
- No per-widget formats. A format belongs to the dataset and applies everywhere it is shown.
- No label editing on the dashboard. Labels are project content edited in the Project Editor.
- No change to what the Built-In parser's descriptor accepts.

## Requirements

1. **R1 — Notation.** A dataset's Label Format offers Hexadecimal and Binary in addition to the
   decimal choices. The notation changes only how the number is rendered, with a digit width
   that follows the declared integer width when one is set and the value otherwise. The number
   itself, as seen by plots, alarms, export, the Historian and the API, is unchanged.
2. **R2 — Fixed point.** A dataset may declare that its raw value is a signed or unsigned
   integer of a given width with N fraction bits. The dataset's published value is the raw value
   divided by 2^N; every downstream consumer sees the decoded value, exactly as it would see a
   transform's result.
3. **R3 — Bit field.** A dataset may declare a bit offset and width within its raw integer value.
   The dataset's published value is the extracted field. Several datasets may read different
   fields of the same frame index, so one status word feeds many LEDs.
4. **R4 — Labels.** A dataset may hold a table mapping integer values to text. Wherever the
   value is presented as text (the Data Grid, a gauge or bar caption, an LED title, the overview,
   tooltips, alarm and notification messages that quote the value, the assistant's view of the
   dashboard), the label replaces the number. Wherever the value is consumed as a number (plot
   curves, gauge needle, bar fill, alarm bands, FFT, export, the Historian, the API's numeric
   field), the number is kept. Label keys are integers; a non-integer value is rounded before
   lookup. A value with no label shows the number, and a table may hold one optional catch-all
   row whose text is shown instead.
5. **R5 — Order of application.** The decoder (fixed point, bit field, scale) applies to the
   value that feeds the dataset: for a channel dataset the parsed value, with any transform
   running on the decoded value; for a computed dataset, whose transform is its only source,
   the transform's result. Notation and labels apply to the final value at presentation. A
   dataset may combine a decoder, a transform and labels. (Amended 2026-11-03 during planning:
   a computed dataset has no parsed value to decode first.)
6. **R6 — Importers.** The DBC importer expresses a signal's value descriptions as labels, not as
   script, and a labelled signal is plotted like any other. A regenerated CAN Bus example carries
   no label code for `CurrentGear`: its transform is the same data-table read every imported
   dataset carries, and it plots. (Amended 2026-11-03 during planning: the Modbus map importer
   already holds a register's scale, offset and bit position as one declarative line per
   register in the generated parser, not in a dataset transform, so the two Modbus clauses are
   withdrawn; the parser stays the declarative home of those numbers.)
7. **R7 — No license.** Declarative formats are not Pro content: the connect gate of spec 0094
   keeps counting transforms and user Variables only. The manual never describes a format as
   requiring a license.
8. **R8 — Project Editor.** Each format is set on the dataset page from a choice list and a few
   numeric fields; labels are edited as a two-column table. Nothing about the editor changes for
   a dataset that uses none of them.
9. **R9 — Surfaces.** The API, the gRPC surface and widget extensions expose the new properties
   and, for a labelled dataset, both the number and the label text, so a remote client or an
   extension renders the same thing the dashboard does.
10. **R10 — Scale.** A dataset may declare a linear scale and offset; the published value is
    `raw * scale + offset`, and like the other decoders it is configuration, not Pro content.
11. **R11 — Compatibility.** A project saved without these properties loads and behaves exactly as
    today. A project saved with them opens in an older release showing the undecoded number, with
    no error.

## Acceptance Criteria

- [ ] **AC1** — A unit test decodes fixed-point and bit-field cases bit-exactly, including
  signed widths, the full-width field and the top bit, and refuses an out-of-range declaration.
- [ ] **AC2** — An integration test over the API loads a project with a labelled dataset and
  reads back the numeric value and the label text for a known frame; the plot's curve list
  still includes the dataset.
- [ ] **AC3** — An integration test saves and reloads a project using every format and compares
  the round trip; a project without the keys round-trips byte-identical to today.
- [ ] **AC4** — Importing the CAN Bus example's DBC yields a label table for the gear signal, a
  transform holding only the shared data-table read, and the signal enabled for plotting
  (amended 2026-11-03 with R6).
- [ ] **AC5** — A UART project with labels, a bit field and a fixed-point dataset, and no
  transform, connects with no license and no trial.
- [ ] **AC6** — The hotpath gate (`--benchmark-hotpath`) reports no regression with decoders
  configured on every dataset of the benchmark project.
- [ ] **AC7** — Maintainer observation: the regenerated CAN Bus example shows "Reverse" in the
  Data Grid and a numeric curve on the plot for the same dataset.

## Constraints & Invariants

- Decoding runs per sample on the acquisition pipeline: no allocation, no per-sample lookup
  that scales with the label count, no regression of the 256 kHz gate.
- A label never alters the number. The two travel together; a consumer picks one.
- The dataset property registry generates every surface; no generated artifact is hand-edited,
  and the gRPC field numbers are append-only.
- One name per concept across the editor, the manual and the API (spec 0100): "Format" for the
  decoder and notation, "Labels" for the table.
- Trial parity: the manual never says a format needs a license, and the feature is identical in
  the free build.
- The vocabulary is the wire's vocabulary. Spec 0111 lets a device declare its datasets with
  exactly these properties, so every format and the label table must be expressible as a short
  text field a sketch can print, and nothing here may exist that the registry does not hold.

## Premise Check

- **Question:** Does the current path to a label cost the user the number and the
  license, and is the need common enough to justify a declarative form?
- **Measured:** (1) The plot widget hides any dataset whose value is non-numeric, so a text
  transform removes the curve; verified in the current tree. (2) The connect gate counts every
  dataset with non-blank transform code as Pro content, so one label transform refuses an
  unlicensed connect (spec 0094, confirmed in code and in the manual). (3) In the examples tree,
  67 datasets carry a transform: 63 are importer-generated reads of a Variables entry, and of the
  4 hand-written ones, 1 is a label table, 2 are linear unit scales and 1 is a rolling RMS. The
  label and scale cases, 3 of 4, would become declarative. (4) Hexadecimal and binary notation
  appear in zero examples because they do not exist.
- **Verdict:** The cost is real and structural. The frequency evidence is thin because the
  examples are curated rather than user projects; proceed on reasoning, and let support traffic
  about "show text for a code" settle the priority against spec 0109 and 0110.

## Open Questions

None at the gate. The draft's three questions (linear scale and offset as a free format, label
keys on non-integer values, the text for an unlisted value) were decided by the maintainer on
2026-11-02 and are folded into R4, R6 and R10 above.
