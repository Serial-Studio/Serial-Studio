---
spec: 0036-property-registry
title: Property registry (declare once, derive everywhere)
status: done          # closed 2026-08-20
created: 2026-07-25
author: Claude (roadmap R2, with Alex)
---

# Spec 0036 — Property registry (declare once, derive everywhere)

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.
>
> Roadmap parent: spec 0030, item **R2**. Depends on R1 (spec 0031, undo/redo — shipped),
> because every registry-driven write must land in the undo history rather than bypass it.
> Feeds R6 (generated MCP/gRPC schemas) and R5 (extension config schemas); this spec
> designs the declaration so those can consume it, and implements neither.

## Problem / Motivation

One dataset property is declared eight times, in eight unrelated idioms, none of which
can see the others:

1. a field on the `Dataset` struct, with a C++ default;
2. a JSON key constant in the `Keys::` namespace, whose spelling deliberately differs
   from the field name (`PltMin` → `"plotMin"`, `Graph` → field `plt`, `Virtual` → field
   `virtual_`);
3. a write rule in the project-file serializer — some fields always emitted, some only
   when non-default, some clamped on the way out;
4. a read rule in the deserializer, carrying its *own* default value plus legacy-key
   fallbacks and post-read normalization;
5. a form-field id in the editor's per-entity id enum;
6. a form row builder in the editor controller: roughly a dozen calls setting the widget
   type, current value, placeholder, label, description, and enablement predicate;
7. a commit arm in the editor's write-back switch, converting the edited variant back
   onto the struct field;
8. an API field applier that re-parses the same field from JSON params, re-implements the
   validation, and returns its own error strings — plus the field's name, meaning, and
   enum domain restated in a prose paragraph inside the API command description, which is
   what an LLM driving MCP actually reads.

Measured on the dataset entity (41 struct fields, 34 form ids): **~1,410 lines of
hand-plumbed property code** across struct/keys/serializer/form-builder/commit-switch,
**~1,742 lines including the API field appliers** — about **41 lines per property, spread
over five translation units**. Adding one dataset field today means five coordinated edits
in five files by three different conventions, and nothing checks that they agree.

They already do not agree. Two defects found while measuring, both invisible to every
existing linter and test:

- **`overviewDisplay` is silently dropped on every save.** The key is declared, read on
  load, settable through the API, and documented in the API description — but the
  serializer has no matching write. Set it, save, reload: it is gone.
- **Four properties have a different default depending on how they arrive.** The read
  path's fallback disagrees with the struct initializer for `fftSamples` (256 vs -1),
  `fftSamplingRate` (100 vs -1), `ledHigh` (80 vs 0), and `index` (0 vs -1). A freshly
  created dataset and a dataset loaded from a project file that omits the key end up in
  different states.

A third symptom: the form-id enum carries an id (`kDatasetView_Overview`) that no form
row builds and no commit arm handles — dead declaration nobody noticed.

The prose-as-schema problem has already cost real functionality. Because the API command
declares only its two identity parameters as typed schema properties and hides the ~43
patchable fields in an English paragraph, the tooling downstream of that schema cannot
see them:

- **The generated JS/Lua SDK cannot set a single dataset field.** The dataset update
  wrapper it emits takes the group id and dataset id and nothing else — there is no
  options bag, because the generator only sees two declared properties. Every script that
  wants to change a dataset title has to bypass the SDK and hand-build the call.
- **The API tells callers to ask for the field list, and the answer is empty.** One
  handler's help text directs the caller to the schema-description verb "for the list of
  writable fields"; that verb returns the same two identity parameters.
- The same field list is then re-typed by hand a fifth and sixth time in the in-app
  assistant's knowledge corpus, which is what the search index is built from.

Two further asymmetries make drift invisible rather than merely likely. The API's field
names and the project-file keys are *not* the same namespace — nine dataset fields are
written under one name and read back under another (`pltMin`/`plotMin`,
`wgtMin`/`widgetMin`, `xAxisId`/`xAxis`, `sourceId`/`datasetSourceId`), so a client that
reads a dataset and writes the object straight back has its edits silently discarded into
an "unknown field" warning. And the four update handlers each re-declare their own copy of
the same seven-line parameter-consumption closure, so the unknown-field warning only works
if a handler author remembers to seed and call it.

This is the same class of problem spec 0028 solved for commands: metadata was hand-copied
into toolbars, menus, palettes, and shortcut tables until it was declared once in a
manifest and the rest generated, with a drift gate. Commands proved the pattern works in
this repo. Entity properties are the larger, faster-growing surface — and unlike commands,
they are the surface an LLM has to reason about through MCP, where the "schema" is
currently an English paragraph.

## Goals

- A dataset property is declared in exactly one place, and the project-file key, default,
  validation, editor form row, editor write-back, API field, and API schema all derive
  from that declaration.
- Adding a dataset property is one manifest entry plus a regeneration step — no hand edit
  to the serializer, the form builder, the commit switch, or the API handler.
- Removing or renaming a property cannot leave a stale half-implementation behind: a
  declared property with no derived surface, or a derived surface with no declaration, is
  a build-blocking lint error.
- Every registry-driven write reaches the project document through the same undo path an
  editor or API mutation uses today; none bypasses history.
- Existing project files load byte-identically, and re-save byte-identically except for
  the specific defects this spec fixes.
- The declaration carries enough structure that a later spec can emit an MCP tool schema
  or a gRPC message from it without re-deriving field lists from prose — and the existing
  SDK generator, which reads that schema, starts emitting usable field setters as a
  side effect.
- The combined hand-maintained line count for dataset properties drops measurably, and
  the drop is reported as a number.

## Non-Goals

- **Not a replacement for the `Keys::` namespace.** `Keys::` stays the single source of
  truth for on-disk JSON key spellings; the registry references those constants, it does
  not restate or replace them.
- **Not a change to the project file format.** No new keys, no schema-version bump, no
  migration. The one intended behavioral delta is the two defects named above.
- **Not a change to the QML editor.** The editor forms are already rendered by a generic
  model-driven delegate; nothing in QML hand-codes a field, so nothing in QML needs to
  change.
- **Not a runtime property system.** No `QVariant` bag replacing struct fields, no
  reflection at frame-parse time. The `Dataset`/`Group`/`Action` structs stay
  packing-optimized PODs on the hotpath; the registry is a build-time artifact.
- **v1 covers the dataset entity only.** Group, action, source, output-widget, and
  project-level properties follow mechanically in later passes and are explicitly out of
  scope here.
- **Does not generate MCP or gRPC schemas** (that is R6 / spec 0037). This spec only
  guarantees the declaration is rich enough to support them.
- Does not cover workspace, widget-settings, or other presentation-state blobs — the same
  surfaces spec 0031 keeps outside undo history.
- Does not unify the enum slug/label tables used by the API; that is adjacent duplication,
  named here so review does not expand into it.

## Requirements

1. **R1 — Single declaration.** Every persisted or editable dataset property is declared
   exactly once, in one declaration file, carrying at minimum: stable id, C++ struct field,
   project-JSON key, API field name, value type, default value, editor widget kind,
   section placement, user-visible label / description / placeholder, validation rules,
   enablement condition, and whether an edit coalesces into the previous undo step.
2. **R2 — Derived project-file serialization.** The dataset's project-JSON write and read
   are produced from the declaration, including the write-only-when-non-default rules,
   the read-side legacy-key fallbacks, and post-read normalization. A dataset property
   cannot be added to the declaration without appearing in saved files.
3. **R3 — Derived editor form.** The dataset form rows the project editor shows — order,
   sections, widget kind, label, description, placeholder, combo-box domain, and
   per-row enablement — are produced from the declaration. The rendered form is
   indistinguishable from today's, field for field.
4. **R4 — Derived editor write-back.** Committing a dataset form edit is produced from the
   declaration: the edited value is converted to the field's type, validated, and applied
   through the existing whole-struct model update, with the correct undo label, coalescing
   key, and tree-rebuild behavior for that property.
5. **R5 — Derived API field handling.** The API's dataset field parsing, per-field
   validation, error strings, and the machine-readable field list published in the API
   schema are produced from the declaration. The API accepts exactly the fields the
   registry declares, and its schema names them as typed properties rather than prose.
6. **R6 — Undo fidelity.** Every derived write path records an undo step through the same
   mechanism spec 0031 established; a batch or multi-select edit remains one atomic step;
   text and numeric field bursts still coalesce. No derived code introduces a mutation
   site that bypasses history.
7. **R7 — Drift is a hard failure.** A declaration with no derived output, a derived
   output edited by hand, a declared property missing from any surface it claims, or a
   surface referencing an undeclared property, fails a check that runs in the standard
   sanitize/verify pipeline — not code review.
8. **R8 — Deterministic, reviewable generation.** Regenerating without changing the
   declaration produces byte-identical files on every platform (stable ordering, LF
   endings). Generated files are checked in and compiled like any other source, carry a
   visible "generated, do not edit" marker, and are readable enough that a reviewer can
   diff them.
9. **R9 — Translations preserved.** User-visible property labels, descriptions, and
   placeholders remain extractable by the existing translation pipeline, in the same
   translation context they use today, so existing translated strings are not orphaned.
10. **R10 — Extensibility escape hatch.** Validation or enablement logic too specific to
    express declaratively (cross-entity uniqueness, live project state, license gating) is
    referenced from the declaration by name and implemented once in hand-written code —
    never inlined into a generated file and never duplicated per surface.
11. **R11 — Measured reduction.** The change reports the before/after hand-maintained line
    count for dataset property plumbing, and the per-new-property edit cost, as concrete
    numbers.
12. **R12 — Name asymmetry is declared, not accidental.** Where a property's API field
    name differs from its project-file key, the declaration states both, every surface
    uses the right one, and the API additionally accepts the project-file spelling so an
    object read from the API can be written straight back without silent loss.
13. **R13 — Choice domains are declared.** A property whose value comes from a fixed or
    computed set of options declares that set once — including how the editor's positional
    combo-box selection maps to the stored value — so the option list, the form row, the
    write-back, and the API's enum domain all come from the same place.

## Acceptance Criteria

- [x] **AC1 (R1, R11)** — The dataset property declaration file exists and covers every
      persisted/editable dataset field. Maintainer check: the reported before/after line
      counts and per-property edit cost are in the implementation notes, with the "add one
      property" path demonstrated as a single-file change.
- [x] **AC2 (R2)** — Round-trip fidelity: every project under `examples/` and every
      template project opens and re-exports byte-identically to a baseline captured before
      the change, except for the declared defect fixes. Verified by a new
      `pytest tests/integration/` case driving `project.open` + `project.exportJson`
      against a checked-in baseline (maintainer runs; app must be up with the API server).
- [x] **AC3 (R2)** — The two known defects are fixed and pinned: `overviewDisplay`
      survives a save/reload cycle, and a project file omitting `fftSamples`,
      `fftSamplingRate`, `ledHigh`, or `index` yields the same values as a newly created
      dataset. Covered by the same pytest file.
- [x] **AC4 (R3)** — Maintainer observation: open the project editor, select a dataset of
      each widget kind (plot, FFT, bar/gauge/compass, LED, waterfall, virtual, inside a
      painter group) and confirm the form is field-for-field identical to the previous
      build — same rows, order, labels, placeholders, enablement, and combo-box contents.
- [x] **AC5 (R4, R6)** — Maintainer observation plus test: editing each dataset form field
      applies correctly and is undoable; a typing burst in a text field is one undo step;
      `project.dataset.update` with multiple fields is one undo step. Extends the existing
      `tests/integration/test_project_undo.py` coverage.
- [x] **AC6 (R5)** — The API schema entry for the dataset update verb declares its fields
      as typed schema properties with descriptions and enum domains, not as a prose
      paragraph. Verified by inspecting the regenerated schema and by a pytest case that
      sets every declared field through the API and reads each back.
- [x] **AC7 (R7)** — Seeding a drift (add a declaration entry without regenerating; edit a
      generated file by hand; reference an undeclared field) makes the verify step fail
      with a clear message. Confirmed by seeding each case locally and reverting.
- [x] **AC8 (R8)** — Running the generator twice produces no diff; the generated files
      carry the do-not-edit marker and LF endings.
- [x] **AC9 (R9)** — Translation extraction still finds every property label/description in
      its existing context; no previously translated property string is orphaned.
- [x] **AC10 (R10)** — The bespoke rules that resist declaration (alias uniqueness, color
      validity, painter-group-only visibility, Pro-gated waterfall, alarm-band and
      frequency-marker sub-editors, the widget-change side effect that rewrites the widget
      range) exist once as named hooks, not duplicated per surface.
- [x] **AC11 (R12)** — Reading a dataset through the API and writing the returned object
      back unchanged produces no `unknown_field` warnings and no value loss. New pytest
      case.
- [x] **AC12 (R5)** — The regenerated JS/Lua SDK emits a dataset update wrapper that can
      set declared fields, and the schema-description verb returns the writable field list
      its own help text promises. Maintainer check on the regenerated SDK plus a pytest
      case driving the described fields.
- [x] **AC13 (R13)** — Every combo-box-backed dataset property renders the same options in
      the same order as the previous build, and selecting each option stores the same value
      it stores today. Covered by AC4's per-widget-kind walk-through plus a pytest case
      that sets each enum-valued field by value through the API and reads it back.

## Constraints & Invariants

- **`Keys::` remains the single source of truth for JSON key spellings.** The declaration
  references key constants; it does not introduce a second place where a key string is
  written. Hard-coded key literals stay a lint error.
- **The entity structs stay hotpath-shaped.** `Dataset` (and its siblings) remain
  packing-optimized, `alignas(8)`, statically asserted PODs; the registry adds no member,
  no indirection, no virtual, and no per-field lookup on any parse or draw path. Nothing
  in this spec runs per frame.
- **The serializer is shared between project files and live API frames.** The declaration
  must distinguish document properties from runtime values (parsed value, numeric value,
  raw value), or live frame broadcasts change shape.
- **All document mutation continues to funnel through the existing whole-struct model
  update.** No new per-property mutating model slot is introduced — such a slot would both
  bypass the undo scope and trip the existing drift lint.
- **Undo semantics from spec 0031 are preserved exactly:** composite operations stay one
  atomic step, keystroke bursts still coalesce, the whitelist of surfaces deliberately kept
  outside history is unchanged.
- Must not regress the `--benchmark-hotpath` gates. Project editing is not the frame
  hotpath, but the entity structs and the serializer are adjacent to it; the gate is run
  as a regression check.
- No new build-time dependency and no new runtime dependency; the generator is a Python
  script in `scripts/`, run at commit time like the existing ones, not a build step.
- GPL/Pro boundaries unchanged: Pro-only properties stay gated exactly as today, and a
  generated surface never exposes a Pro property on a GPL build.
- Generated files are checked in and reviewable; a build must never require running the
  generator.
- The spec-0002 TU split and the facade headers it preserved stay intact — generated code
  lands in its own translation units, not by rewriting the existing ones wholesale.
- **The editor's row contract is unchanged.** Rows still carry a positional selection index
  for choice fields, the same role set, and the same widget-kind vocabulary; the generic
  QML delegate must keep working untouched. The registry declares how index maps to value —
  it does not change what the model stores.
- **Not every row is a property.** Section headers, the per-child navigation rows, and the
  buttons that launch the alarm-band and frequency-marker sub-editors are structural, not
  property-derived, and stay hand-written. The declaration must compose with them rather
  than require they disappear.

## Open Questions

*Answered in this spec; recorded for the approval gate.*

- **Does v1 cover editor form generation, or only model + API?** — **v1 covers forms.**
  The roadmap left this open on the assumption that QML might hand-code the editor forms.
  It does not: every project-editor form is rendered by one generic model-driven delegate
  over rows that carry their own widget kind, and the only place a field is spelled out
  is the C++ form-row builder and its write-back switch. Those two are the single largest
  block of hand-plumbed property code (~1,050 of the ~1,410 core lines) and are the most
  mechanically uniform. Deferring them would mean deferring most of the value while
  keeping the drift risk. **No QML change is in scope.**

*Genuinely open, for the maintainer.*

- **Scope of the defect fixes.** Fixing `overviewDisplay` and the default drift changes
  saved-file bytes for projects that use those fields. Land the fixes inside this spec
  (current lean — the baseline test can assert exactly that delta), or split them into a
  separate commit that lands first so this spec's round-trip test can assert a zero delta?
- **Does the declaration ship in the application resources?** v1 needs it only at commit
  time. Bundling it would let a later spec serve entity schemas at runtime (R6 MCP, R8
  problem center, the in-app assistant) without a second copy — at the cost of shipping a
  file v1 does not read.
- **Second entity in the same pass?** Group is the next-largest and would prove the
  declaration generalizes beyond the entity it was designed against. Current lean: keep v1
  to dataset, land group immediately after as a mechanical follow-up.
