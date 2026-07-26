---
spec: 0037-generated-api-surfaces
title: Generated API surfaces from one source of truth
status: draft        # draft -> approved -> in-progress -> done | shelved
created: 2026-07-25
author: Claude (roadmap R6, with Alex)
---

# Spec 0037 — Generated API surfaces from one source of truth

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.
>
> Roadmap parent: spec 0030, item **R6**. Depends on **R2 (spec 0036, property registry)**,
> which declares each dataset property once and makes the API's typed schema derive from
> that declaration. 0036 stops at the schema; this spec follows the schema outward to every
> surface that consumes it, and makes each of those surfaces verifiable without a build.
> v1 is scoped to the **dataset entity**, matching 0036 v1.

## Problem / Motivation

Serial Studio already has a single source of truth for API metadata at runtime: the input
schema each command registers with the command registry. Five separate consumers read that
one object and reshape it:

1. the MCP `tools/list` reply, which copies the schema verbatim as a tool's `inputSchema`
   (347 commands, no filtering, no exposure flag);
2. the `--dump-api-schema` CLI flag, which flattens the registry into the checked-in
   `api-schema.json` snapshot;
3. the JS/Lua SDK generator, which turns that snapshot's required properties into positional
   arguments and its optional properties into an options bag;
4. the gRPC typed-proto generator, which emits one request message per command with one
   field per schema property;
5. the in-app assistant's schema-description verb, which returns the schema to the model.

The architecture is right. What is missing is that **nothing verifies the chain**, and two
links in it are broken in ways that only show up at a user's site.

**The checked-in snapshot can only be refreshed by running a built binary.** Everything
downstream — the SDK, the symbol list the script completer reads, and any future proto
artifact — is generated from a file that a human must remember to re-dump after changing
C++. The repo's own linter documents the hole in a comment: its SDK staleness rule "cannot
see commands added in C++ until `--dump-api-schema` refreshes api-schema.json, so it only
catches a generator or prelude edit that was never regenerated." A field added to a handler
and never dumped is invisible to every check the project has.

**The drift gates that do exist are not wired to anything that fails.** The command-strings
generator ships a correct `--check` mode that re-renders and byte-compares; no caller
invokes it — not the commit pipeline (which regenerates instead of checking) and not CI. The
CI lint job runs exactly one command, `code-verify.py --check`. The registry verifier, the
SDK generator, and the command-strings check are all local-discipline-only. A contributor who
edits a manifest and forgets to run the commit script gets a green CI.

**The typed gRPC proto assigns field numbers by iteration order.** The generator walks the
schema's `properties` object — which iterates alphabetically — and assigns 2, 3, 4, ...
in that order. Today `project.dataset.update` declares two properties, so nothing has ever
moved. After 0036 it declares roughly forty-five, and every future property inserted
alphabetically before an existing one **renumbers every field after it**. Protobuf field
numbers are the wire format: a renumbered field is not a compile error, it is a client that
silently reads `title` out of the bytes that used to hold `units`. The generated proto is not
checked in either, so there is no diff in which a reviewer could ever notice.

**The same field list is retyped by hand in the assistant corpus, and has already drifted.**
Three bundled skill documents independently restate the dataset widget-option bitflags. Two
of them stop at `64 = Waterfall`; a third adds `128 = Meter`. Two of them independently
restate the same short-name/long-name range-field mapping (`pltMin`/`plotMin`,
`wgtMin`/`widgetMin`). These files are the corpus the in-app assistant searches, so the
disagreement is served to the model as fact. Nothing checks any of it against the code.

Spec 0036 fixes the *upstream* of this chain: one manifest entry per dataset property, and a
typed schema derived from it instead of a prose paragraph. That change is what makes the
downstream surfaces suddenly matter — a schema that goes from two properties to forty-five is
exactly the change that renumbers a proto, grows an MCP payload, and reveals whether the
snapshot on disk still matches the code. Landing 0036 without this spec means shipping the
break.

## Goals

- Adding a dataset property to the 0036 manifest and regenerating updates every API surface
  that names dataset fields, with no hand edit to any of them.
- A generated API artifact that has been hand-edited, or a manifest change that was never
  regenerated, fails a check that runs in CI — not in code review, and not only on the
  maintainer's machine.
- The parts of the checked-in API snapshot that derive from the registry can be verified
  against the declaration **without building or running the application**, closing the hole
  the SDK staleness rule documents.
- A gRPC client can generate stubs from an artifact in the repository, and a property added
  later never changes the meaning of a field number that already shipped.
- The dataset field names and enum domains stated in the bundled assistant corpus are checked
  against the declaration, so the corpus cannot disagree with the code.
- Every drift gate the repository owns has a caller: the commit pipeline, CI, or both.

## Non-Goals

- **Not a second declaration.** This spec adds no new manifest of its own; the 0036 dataset
  manifest is the source of truth and this spec only projects it outward. A new property is
  still declared exactly once, in 0036's file.
- **Not a rewrite of the schema builder or the command registry.** Commands keep registering
  their schema the way they do today; the registered command record grows no new field.
- **v1 covers the dataset entity only**, matching 0036 v1. Group, action, source, and
  output-widget verbs keep their prose descriptions until their manifests exist.
- **Not a change to the dynamic gRPC service the build compiles.** The proto that protoc
  compiles into the binary keeps its current shape; only the *typed, client-facing* proto is
  in scope.
- **Not an MCP protocol change.** No new MCP methods, no protocol-version bump, no
  per-command exposure flag, no change to which commands become tools.
- **Does not generate assistant prose.** The corpus stays hand-written; this spec only checks
  the field names and enum values it states against the declaration.
- **Does not make the application serve schemas at runtime.** Any runtime schema endpoint is
  a later spec's problem.
- **Does not attempt to generate the checked-in API snapshot from the manifest.** The
  snapshot covers all 347 commands and only a build can produce it; this spec verifies the
  part that is derivable and says so explicitly for the rest.
- Does not renumber, rename, or remove any existing API command, parameter, or proto field.

## Requirements

1. **R1 — One declaration, every surface.** Adding a dataset property to the 0036 manifest
   and running the regeneration step updates every derived API surface in scope. No surface
   requires a second, hand-written edit naming the same field.
2. **R2 — Buildless verification of the derivable part.** The dataset verbs' typed properties
   and required list in the checked-in API snapshot are verified against the manifest by a
   check that runs with no compiler, no Qt, and no running application. A mismatch fails with
   a message that names the offending command, the offending field, and the exact steps to
   fix it.
3. **R3 — Generated artifacts are byte-reproducible and marked.** Every artifact this spec
   generates is deterministic across platforms and runs, carries a visible
   "generated, do not edit" marker, and is checked in and reviewable as a diff.
4. **R4 — Hand-editing a generated artifact fails a check.** Editing a generated file, or
   changing the declaration without regenerating, is caught by the standard verification
   pipeline and by CI.
5. **R5 — Every drift gate has a caller.** The verification steps this spec adds, and the
   existing generated-artifact check that currently has no caller, run in the commit pipeline
   and in the CI lint job.
6. **R6 — Stable gRPC field numbers.** A gRPC field number, once published for a command's
   parameter, never changes meaning. Adding a property assigns the next unused number;
   removing one retires its number permanently rather than freeing it for reuse. The
   numbering is recorded in a reviewable, checked-in artifact.
7. **R7 — A gRPC client can codegen from the repository.** The typed, per-command proto is
   checked in, so a client generates stubs from the repo instead of running the application
   to export one. The artifact the running application exports and the artifact in the
   repository agree.
8. **R8 — MCP tool schemas carry the declared fields.** Every dataset field the manifest
   declares appears in the corresponding MCP tool's `inputSchema` as a typed property with a
   description, and with its enum domain where the declaration defines one. No dataset field
   is discoverable only through prose.
9. **R9 — MCP payload cost is measured, not assumed.** The change in `tools/list` response
   size is measured and reported. If the typed properties cost more than the prose they
   replace, the number is stated and accepted explicitly rather than discovered later.
10. **R10 — The SDK exposes the declared fields.** The generated JS and Lua SDK emit a
    dataset update wrapper that can set every declared field, and the SDK generator gains a
    verification mode so its output is gated like the other generated artifacts.
11. **R11 — The assistant corpus cannot contradict the declaration.** A dataset field name or
    declared enum value stated in the bundled assistant corpus must exist in the manifest with
    that meaning; a stale name or a disagreeing enum value fails a check. The corpus
    disagreements that exist today are fixed as part of this work.
12. **R12 — Pro gating is preserved end to end.** A generated surface never publishes a
    Pro-only property on a GPL build, and the field numbering never shifts because a
    commercial command is absent from the build the snapshot was dumped from.
13. **R13 — The gates fail loudly and actionably.** Every check this spec adds prints what
    drifted and the ordered commands that fix it. A check that cannot run (missing snapshot,
    missing manifest) says so rather than passing silently.

## Acceptance Criteria

- [ ] **AC1 (R1)** — Maintainer demonstration: add one property to the 0036 dataset manifest,
      run the regeneration step and the maintainer's snapshot refresh, and confirm the field
      appears in the checked-in API snapshot, the MCP tool schema, the JS and Lua SDK
      wrappers, and the typed proto — with no hand edit to any of those four.
- [ ] **AC2 (R2, R13)** — Seeding a divergence between the manifest and the committed API
      snapshot (add a manifest property without refreshing the snapshot) fails the check with
      a message naming the command, the field, and the ordered fix. Confirmed by seeding
      locally and reverting. The check runs with no build.
- [ ] **AC3 (R3)** — Running every generator twice produces no diff; each generated artifact
      carries its do-not-edit marker and LF endings. Confirmed locally.
- [ ] **AC4 (R4)** — Hand-editing each generated artifact (including deleting its marker)
      fails verification with a clear message. Confirmed by seeding each case and reverting.
- [ ] **AC5 (R5)** — The CI lint job runs the registry verifier, the property-registry
      generator check, the command-strings check, and the SDK check, and fails the build when
      any of them drifts. Confirmed by inspecting the workflow and by a deliberately drifted
      branch.
- [ ] **AC6 (R6)** — Adding a property whose name sorts alphabetically before existing ones
      leaves every previously assigned field number unchanged and assigns the new field the
      next unused number. Verified by a static test over the numbering artifact plus a
      before/after diff in the seeded demonstration from AC1.
- [ ] **AC7 (R6)** — Removing a property retires its number: the number is recorded as
      reserved and is never reassigned. Verified by the same static test.
- [ ] **AC8 (R7)** — Maintainer check: the typed proto exported by the running application is
      byte-identical to the one checked into the repository, and `protoc` accepts the
      checked-in file.
- [ ] **AC9 (R8)** — An MCP `tools/list` reply for the dataset update tool lists every
      declared field as a typed property with a description, and the enum-valued fields carry
      their domains. Verified by a new `pytest tests/integration/` case (maintainer runs; app
      must be up with the API server).
- [ ] **AC10 (R9)** — The before and after `tools/list` payload sizes are recorded as numbers
      in the implementation notes, with the delta stated.
- [ ] **AC11 (R10)** — The generated SDK's dataset update wrapper accepts an options bag
      covering every declared field, and the SDK generator's verification mode fails on a
      stale committed SDK. Verified by inspecting the regenerated SDK plus a pytest case that
      sets fields through the SDK path.
- [ ] **AC12 (R11)** — Seeding a bogus dataset field name and a wrong enum value into the
      bundled corpus fails the check; the corpus disagreements that exist today (the
      widget-option bitflag tables that disagree on the highest bit, and the duplicated
      range-field mapping) are reconciled against the manifest.
- [ ] **AC13 (R12)** — Maintainer check on a GPL build: no Pro-only dataset property appears
      in the MCP tool schema, the SDK, or the typed proto, and the field numbering artifact is
      unchanged between a GPL and a commercial snapshot.
- [ ] **AC14 (R1, R5)** — The commit pipeline runs the new generation and verification steps
      in an order that produces a clean tree on a second consecutive run.

## Constraints & Invariants

- **The 0036 manifest is the only declaration.** This spec introduces no second place where a
  dataset field name, type, default, or enum domain is written. A field name appearing in
  generated output must be traceable to a manifest entry.
- **No new build-time or runtime dependency.** Generators are Python scripts in `scripts/`,
  run at commit time like the existing ones, never as a build step. Nothing added here may
  make a build require running a generator.
- **The commit pipeline stays sanitize-only.** It never commits, never pushes, and a failing
  verification step reports rather than aborting the developer's tree.
- **Determinism is a hard requirement, not a preference.** Sorted iteration, explicit LF
  writes, no dictionary-order dependence, no timestamps or machine identifiers in generated
  output.
- **Generated artifacts are checked in and reviewable.** A reviewer must be able to read the
  diff of a generated file and understand what changed; a build must never require running
  the generator.
- **Wire compatibility is one-way.** Field numbers and command names are append-only. No task
  in this spec may renumber, rename, or repurpose anything a released client could already be
  using.
- **The `--benchmark-hotpath` gates must not regress.** Nothing here runs per frame, but the
  gRPC server is on the export fan-out and the gate is run as a regression check because a
  gRPC-adjacent file changes.
- **GPL/Pro boundaries unchanged.** Commercial commands and Pro-only properties stay gated
  exactly as today; a GPL build's surfaces must never name a Pro property.
- **The CI workflow is a contended file.** The campaign serializes workflow edits; the CI task
  in this spec must be sequenced accordingly rather than landing opportunistically.
- **The corpus check is a reference lint, not a rewrite.** It must not force a regeneration of
  the large bundled search index in the same pass, whose diffs are already noisy.
- **Every existing API command, parameter name, and behavior is preserved.** This spec changes
  how surfaces are derived and verified, never what they mean.

## Open Questions

*Answered in this spec; recorded for the approval gate.*

- **Which surfaces are generated versus merely checked in v1?** The checked-in API snapshot
  covers all 347 commands and only a build can produce it, so it is **checked** against the
  manifest for the dataset verbs rather than generated. The typed proto and its field-number
  ledger are **generated**. The MCP tool schema and the SDK are **inherited** — they already
  derive from the same schema object and need verification plus, for the SDK, a check mode.
  Generating the snapshot would mean reimplementing 300+ hand-written schemas in Python; that
  is the opposite of one source of truth.
- **New script or extend the 0036 generator?** **Extend it.** A second script would
  re-implement the manifest loader and could disagree with 0036's own reading of the same
  file — reintroducing the drift class this roadmap item exists to remove.

*Genuinely open, for the maintainer.*

- **Does the typed proto ship in the repository, or stay an export-only feature?** Checking it
  in is what lets a client codegen without running the app and what makes a renumbering
  visible in review — but it is a new checked-in artifact that must be kept current. Current
  lean: check it in, and have the application's export serve the checked-in artifact so there
  is exactly one emitter.
- **Should the CI lint job fail on a stale API snapshot, or warn?** Failing is the point of a
  gate, but the snapshot can only be refreshed from a build, so a contributor without a build
  cannot clear the failure. Current lean: fail, with a message that names the ordered fix and
  states that only registry-derived verbs are covered.
- **How far does the corpus lint reach?** Dataset field names and declared enum values are
  clearly in scope. Whether it should also check the widget-option bitflag tables that the
  corpus states in three places (and that already disagree) is a judgment call about how much
  hand-written prose a lint should police.
