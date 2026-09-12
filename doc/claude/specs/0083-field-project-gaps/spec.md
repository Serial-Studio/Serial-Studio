---
spec: 0083-field-project-gaps
title: Close the gaps a real industrial deployment works around with scripts
status: done          # closed 2026-09-12: maintainer ran the build/run gates and closed
created: 2026-09-11
author: Alex Spataru
---

# Spec 0083 — Close the gaps a real industrial deployment works around with scripts

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.
>
> This is an umbrella spec. It records six gaps found by auditing one production project,
> ranked. When a gap is picked up it gets its own numbered spec; this one is then updated to
> point at it. The evidence, with file and line references on both sides, lives in
> [findings.md](findings.md) in this directory.

## Problem / Motivation

The field project is the Serial Studio project behind an engine test cell: CAN acquisition
boards, audio vibration channels and an RS-485 load bank carrying two Modbus units, all in one
`.ssproj` with dozens of data tables, more than a hundred groups, several hundred datasets and a
couple dozen workspaces. It is the largest and longest-lived real project the maintainer has, and
it is maintained almost entirely by Python generators in a private repository rather than in the
app.

Those generators are not a workaround smell in themselves: the calibration data comes from
certificates in a workbook and must be regenerated under a commit gate. But roughly half of
their code exists only to compensate for things Serial Studio cannot express, and every one
of those things is a general industrial need, not a quirk of one project:

- Hundreds of dataset transforms collapse to about a hundred real formulas; each sensor
  channel carries its own copy of the conversion and the piecewise calibration code, differing
  only in a few constants. Changing a formula means regenerating hundreds of datasets.
  Transforms are most of the project file.
- Workspace tiles reference widgets by a per-type ordinal, so inserting a group anywhere
  shifts every later tile. The generators re-stamp every reference on every run and must
  keep certain groups last; a per-engine project can never drop a group.
- The register-map importer (and the DBC and Protobuf importers) can only create a new
  project, so adding a Modbus meter to an existing dashboard means a script.
- The importer cannot say which unit a register block belongs to, cannot describe a status
  bit inside a holding register, cannot mark a register writable, and its generated parser
  guesses which reply it is looking at by position. The driver itself already supports
  per-block units; the importer lags it.
- Writes always go to the connection's unit, so a two-unit bus can only be commanded on one.
- A project that serves several engine variants has no way to show a subset of its
  workspaces, so a generator filters the file per engine.

## Goals

- A project can define shared transform code once and parameterise it per dataset, so a
  formula lives in one place.
- A workspace tile keeps pointing at its widget when groups are added, removed or reordered.
- Every importer can add its result to the open project as well as create a new one.
- The Modbus importer can express a multi-unit bus, status bits inside registers, writable
  registers and word order, and its generated parser identifies replies by their contents.
- A Modbus control can target a unit other than the connection's default.
- A project can declare named workspace profiles and the operator picks one at load.

## Non-Goals

- Rewriting the field project's generators inside Serial Studio, or shipping the field project as an example.
- A general Lua package system (`require` from disk, third-party modules). One project-level
  library chunk is the whole ask.
- Changing how CAN or DBC projects decode frames.
- Any hotpath change: every item here is configuration, project-model or importer work.

## Requirements

1. **R1** — A project can carry one shared Lua library; every Lua dataset transform of that
   project can call its functions without copying them.
2. **R2** — A dataset can carry named parameters that its transform reads, so identical
   transform code can serve many datasets.
3. **R3** — A workspace tile is resolved by the group it was placed for, not by its position
   among widgets of the same type; a re-ordered project opens with every tile in place.
4. **R4** — The Modbus, DBC and Protobuf importers offer "add to current project" beside
   "create new project".
5. **R5** — The Modbus register map format accepts a unit id per register; blocks are polled
   from that unit and the generated parser routes each reply by the unit and function code
   it carries, never by poll position.
6. **R6** — The Modbus register map format can describe a single bit inside a holding or
   input register, and such rows produce LED-style datasets.
7. **R7** — The Modbus register map format can mark a register writable; writable rows
   produce an output control that writes that register.
8. **R8** — The Modbus register map format can state word order per entry.
9. **R9** — A Modbus write can name the unit it targets.
10. **R10** — A project can declare workspace profiles (named subsets of its workspace
    folders); the operator selects one when opening the project.

## Acceptance Criteria

- [x] **AC1** — The field project's sensor channels share one conversion function from the project library
      with three parameters each, and the dashboard readings are bit-identical to today's.
- [x] **AC2** — Inserting a new group before existing ones in the editor leaves every
      workspace tile attached to its original widget (integration test on a fixture project).
- [x] **AC3** — Importing the field project's load-bank register map into an open project adds one
      source and its groups, and leaves every existing group, table and workspace untouched.
- [x] **AC4** — A register map with two units yields two polled blocks on different unit ids
      and a parser that decodes a captured two-unit reply sequence correctly with one reply
      dropped (JS or Lua unit test under `tests/scripts/`). Verified 2026-09-11:
      `tests/scripts/test_modbus_lua_parser.py` (4 tests, `luajit`).
- [x] **AC5** — A `bit` row on a holding register shows as an LED that follows the bit.
- [x] **AC6** — A writable row shows as a control whose press changes the register on the
      Modbus PLC Simulator example.
- [x] **AC7** — A control can write to unit 2 on the simulator while the connection's unit
      is 1.
- [x] **AC8** — Opening the field project with one engine profile shows only that engine's workspaces
      and the generated per-engine project file is no longer needed.

> Implementation landed 2026-09-11 (plan + tasks in this directory). AC1, AC2, AC3, AC5, AC6,
> AC7 and AC8 need a built app: the maintainer runs `tests/integration/test_dataset_transforms.py`,
> `test_workspace_identity.py`, `test_modbus_groups.py`, `test_project_editor.py`, the ctests
> `tst_workspace_rebind` and `tst_project_merge`, and the field-project / Modbus PLC Simulator
> observations listed in `plan.md`; the boxes above are ticked as each one passes.

## Addendum A (2026-09-11) — JavaScript library and a "Project Scripts" tree node

Maintainer feedback after the first build. Two additions, same spec.

**A1 — JavaScript transform library.** Gap 1 shipped a Lua-only library. The JS lane has the same
shape (one `QJSEngine` per source, transforms are IIFE closures that resolve globals through the
engine's global object), so a second project-level chunk, `transformLibraryJs`, is evaluated once
into that global object before the JS entries compile, on both lanes. Symmetric with the Lua one:
compile-time only, its own watchdog window, a failure leaves the entries compiling and is reported
in the Problem Center under "the shared JavaScript library". The API keeps one command family,
`project.transformLibrary.get/set/dryRun`, with an optional `language` (`"lua"` default, `"js"`).

**A2 — Project Scripts node.** The project-level scripts leave the tree root and sit under one
top-level item, **Project Scripts**, holding **Control Loop**, **Lua Library** and
**JavaScript Library** (the dialog-hosted library editor of the first build is gone; each library
is a full-page embedded editor like the control loop). Selecting the node shows a short overview
of the three with open buttons.

- [x] **AC9** — A JS transform `function transform(v) { return scale(v, params) }` resolves
      `scale` from `transformLibraryJs` on the frame lane and on the stream lane; a library with a
      syntax error leaves the transform reporting its raw value and the Problem Center naming the
      JavaScript library.
- [x] **AC10** — `project.transformLibrary.set {code, language: "js"}` persists as
      `transformLibraryJs` in the project file, round-trips through `get`, and `dryRun` reports a
      JS syntax error with its line.
- [x] **AC11** — The Project Editor tree shows **Project Scripts** with the three children;
      selecting a library child opens its editor, edits land in the model with undo, and the
      expansion state of the node persists like the other roots.

## Constraints & Invariants

- No hotpath regression: library evaluation and parameter injection happen at transform
  compile time, never per sample.
- Project files written by earlier versions must open unchanged; new keys are optional and
  old ordinal references keep resolving.
- The importer's existing CSV/XML/JSON files must import exactly as before when the new
  columns are absent.
- The word `unit` is already accepted as a synonym for the units column; the unit-id column
  needs a name that cannot collide with it.
- Pro gating stays where it is (Modbus, importer, output widgets).

## Open Questions

- Library scope: one chunk per project, or one per source? Per project is simpler and what
  the field project needs; per source would mirror how the engines are already keyed.
- Should dataset parameters be free-form key/value, or typed (number/string/table name)?
- Profiles: a project-level list, or a flag on each workspace folder? The former allows
  overlapping profiles.
- Which of the six items ship as one spec each, and in which order? The findings file
  recommends library + parameters first, tile identity second, importer work third.
