---
spec: 0030-improvement-roadmap
title: Architecture improvement roadmap (undo, registry, diagnostics, async, remote)
status: in-progress  # draft -> approved -> in-progress -> done | shelved
approved: 2026-07-24 # roadmap + ordering approved; R1 picked first
created: 2026-07-24
author: Claude (comparative review with Alex)
---

# Spec 0030 — Architecture Improvement Roadmap

> **Umbrella spec.** This is a roadmap, not a single feature: ten improvement items
> distilled from a comparative review of three mature Qt codebases (a 25-year CAD
> application, a process-introspection tool, and a large plugin-based IDE), checked
> against this repo's current state on 2026-07-24. Each item that gets picked up
> graduates to its own numbered spec via `/ss-spec` before any plan or code exists —
> this document only records the WHAT, the WHY, and the agreed priority. Approving
> this spec approves the roadmap and its ordering, not any implementation.

## Problem / Motivation

The comparative review found this repo ahead of all three reference codebases on
engineering hygiene (CI breadth, linters, docs freshness, zero TODO debt) but behind
on five structural patterns those codebases proved out over decades: transactional
undo, declare-once property plumbing, centralized diagnostics, declarative async
orchestration, and UI/core process separation. Each gap is currently paid for in
user pain (unrecoverable project edits), maintenance cost (the same field plumbed
through model, API handlers, and QML), or support load (connection failures that
surface as bare timeouts).

Repo reality this roadmap is grounded in:

- Project editing has no undo. A deleted group with its datasets is gone; the only
  undo anywhere is the text editors' built-in one.
- A dataset property is hand-plumbed through the project model TUs, the API entity
  handlers, and the QML editor forms (~22.7k lines across those concerns). Adding
  one field means three coordinated edits; drift between the surfaces is a
  recurring bug class. Spec 0028 (command registry) already proved the declare-once
  pattern works here.
- Test tiers exist at the top (pytest over the live API) and bottom
  (`tests/scripts/` JS parser units, the `--benchmark-hotpath` CI gate) but the
  middle is missing: no in-process C++ unit tests. `tests/unit/` holds a single
  Python manifest test. A parse-logic regression is caught by CI only after a full
  packaged build.
- Singleton construction order is pinned (spec 0001) but state is still global:
  ~239 files reach through `instance()`. Two projects, parallel in-process tests,
  or a second dashboard session are structurally impossible.
- Headless mode exists (`--headless`, API server on 7777) but the GUI cannot attach
  to a remote or already-running headless instance; the dashboard renders only
  in-process data.
- Connection failures report raw errors. Nothing checks port permissions, driver
  presence, or broker reachability and tells the user what to fix. (The licensing
  subsystem has its own self-test; nothing equivalent exists for I/O.)
- Async flows (connect, retry, handshake, BLE pairing, MQTT reconnect) are
  hand-rolled per driver as state machines and callback chains; reconnect bugs
  recur per-driver instead of being fixed once.
- No problem/diagnostics center: project-file inconsistencies (duplicate frame
  indices, dangling action references) fail silently or surface as blank widgets.
- 87 files use `Q_ASSERT` (no-op in release, crash in debug) in an app that parses
  untrusted bytes from serial ports at 256 kHz.
- No `CMakePresets.json` (no one-command asan/tsan configure), no `REUSE.toml`
  (SPDX compliance manifest) despite the `LICENSES/` directory and dual-license
  model.

## Goals

- Every project-editor mutation is undoable, from UI and API alike.
- A property is declared once and every surface (model, editors, API schema,
  docs) derives from that declaration.
- A C++ logic regression is caught by a test that runs in seconds, before any
  packaged build.
- A failed connection tells the user what is wrong and how to fix it.
- Reconnect/retry/cancel logic exists once, not once per driver.
- Project-file inconsistencies are listed, explained, and clickable — never silent.
- A dashboard can attach to a headless capture running elsewhere, and detach
  without stopping it.
- Sanitizer builds and license compliance are one command each.

## Non-Goals

- No rewrite of working subsystems; every item is incremental and gated.
- No new UI framework, no dependency heavier than a single header-friendly lib
  (the async-orchestration item names one candidate; the plan phase decides).
- The roadmap does not commit to dates; it commits to ordering and dependencies.
- Items are independent specs later; this document does not fix their designs.

## Roadmap Items

Each item below is a future `NNNN` spec. Requirements here are roadmap-level: they
define what the item must achieve to be worth doing, not its full requirement set.

### R1 — Transactional undo/redo for project editing

Every mutation of the project document (add/delete/modify group, dataset, action,
frame settings; from QML editor or API) becomes an undoable operation. Composite
edits (delete group + children) undo atomically. Keystroke-level edits coalesce.

*Acceptance:* Ctrl+Z / Ctrl+Shift+Z work in the project editor; an automated test
applies N random mutations, undoes all, and the document equals its start state.

*Why first:* retrofit cost grows with every commit that adds a mutation site; the
CRUD/handler TUs are the largest and fastest-growing mutation surface in the repo.

### R2 — Property registry (declare once, derive everywhere)

Each entity property (dataset, group, action, project) is declared exactly once —
type, default, validation, serialization key, editor hint. Model accessors, editor
forms, and API schemas derive from the declaration. Spec 0028 is the in-repo
precedent (commands declared once in manifests; strings generated; drift gated by
`registry-verify.py`); this extends the same philosophy to entity properties.

*Acceptance:* adding a new dataset property is a single declaration plus generated
artifacts; it appears in editor UI, project JSON, and API schema with no hand edits
to any of the three. Combined line count of the hand-plumbed property code drops
measurably.

*Depends on:* R1 (property writes should emit undo operations, not raw setters).

### R3 — C++ unit-test tier plus build presets

An in-process C++ test target (excluded from release builds) covering the pure
logic that today has no fast test: DSP kernels (scalar vs SIMD equivalence),
frame-delimiter parsing edge cases, checksums, frame serialization round-trip.
A `CMakePresets.json` with dev, asan, tsan, and analysis presets; CI runs the unit
tier before any packaging job. A middle "in-app test" mode (tests compiled in
behind a build flag, run via a CLI flag inside the real composition root) follows
the precedent of the in-binary hotpath benchmark.

*Acceptance:* a seeded parser regression fails CI in the lint-tier job in under
five minutes, without any platform packaging having run. `ctest` green locally in
seconds.

### R4 — Session context over global singletons

Introduce a per-session context object that owns what is today global (project
model, dashboard state, I/O manager, frame pipeline), built inside the existing
composition root (spec 0001). Migration is incremental: the context first wraps
the existing singletons, then high-churn classes take it by injection as they are
touched. True app-globals (theme, translator, settings) stay global.

*Acceptance:* two independent contexts can be constructed in one test process with
no state bleed; the `instance()` call-site count is tracked (in `code-verify.py`
advisory) and trends down.

*Depends on:* R3 (a safety net must exist before threading a context through the
hotpath composition).

### R5 — Widget-as-extension

Dashboard widgets become installable packages: a manifest (id, name, accepted
dataset types, config schema, version + host-compat range, required/optional
dependencies, experimental flag), a QML visual, and an optional script hook.
The host loads manifests eagerly but instantiates lazily, only when a matching
dataset type appears. Load failures surface through the problem center (R8),
not silently. Two builtin widgets ship rewritten as extensions to prove parity.

*Acceptance:* a third-party widget installs via the existing extension manager,
renders live data, and survives an app update with no recompile.

*Depends on:* R2 (config schemas reuse the property registry).

### R6 — Generated API surfaces from one source of truth

MCP tool schemas, gRPC message definitions, and handler field maps generate from
the R2 property registry, following the `generate-command-strings.py` pattern:
a generator script plus a `--check` drift gate in `sanitize-commit.py` and CI.

*Acceptance:* adding a registry property and regenerating updates every API
surface; CI fails if a generated file is hand-edited.

*Depends on:* R2.

### R7 — Remote dashboard attach

The GUI can connect to a headless instance (local or remote) and mirror its live
dashboard: dashboard-facing models stream over the existing API transport; the QML
layer binds to mirrored models and cannot tell local from remote. In-process
remains the default and behaves byte-identically to today. Detach leaves the
capture running.

*Acceptance:* `serial-studio --headless project.json` on machine A; GUI on
machine B attaches, sees live widgets, detaches, reattaches. In-process startup
time and hotpath benchmarks unchanged.

*Depends on:* R4 (global state cannot be mirrored). Capstone item; plan late,
start last.

### R8 — Problem center (project + link diagnostics)

A per-session collector that registered checkers report into, with one model
feeding a UI panel and the existing notification center. Initial checkers:
project-schema issues (duplicate frame indices, empty groups, dangling action
references, invalid ranges), link issues (frames seen but none parsed — delimiter
mismatch; checksum failure rate; buffer overruns), script issues (JS/Lua errors
with counts). Each finding: severity, explanation, jump-to-source. Exposed as an
MCP tool so the assistant can read diagnostics.

*Acceptance:* loading a project with duplicate frame indices produces a listed,
clickable problem instead of a silently wrong dashboard; "why is my widget empty"
is answerable from the panel.

*No dependencies — highest support-load reduction per line of code; start any time.*

### R9 — Connection diagnostics self-test

An ordered check runner for the I/O stack: serial driver enumeration, port
permissions (dialout group on Linux, drivers on Windows/macOS), BLE stack state,
broker reachability when MQTT is configured, audio subsystem. Each check yields
pass/fail/warn plus a concrete remedy string. Runs from the welcome page and
automatically when a connection attempt fails. Findings flow through R8. Named
"connection diagnostics" to avoid colliding with the licensing self-test.

*Acceptance:* fresh Linux machine, user not in dialout: the failed connect shows
the exact remedy command, not a timeout.

*Depends on:* R8.

### R10 — Declarative async orchestration for I/O flows

Connection lifecycles (open, handshake, retry with backoff, teardown; BLE
discovery + pairing; MQTT reconnect; process launch) are expressed as composable
task trees — sequential/parallel groups with built-in timeout, cancellation, and
error propagation — instead of per-driver state machines. One vetted library
candidate exists (BSD-licensed, standalone, Qt-native); the plan phase confirms
it against the no-heavy-dependency constraint or specifies a minimal in-repo
equivalent.

*Acceptance:* reconnect logic exists in exactly one place; a loop test that
severs the link 100 times mid-stream (R3 tier) always recovers and leaks nothing.

*Pairs with R9; start any time.*

## Adopt-directly items (no separate spec needed)

- **Soft assert macro** — `SS_ASSERT(cond, action)`: logs location, executes a
  recovery action, never crashes release. Migrate the 87 files using `Q_ASSERT`;
  ban `Q_ASSERT` in `code-verify.py`. Supports the existing assertion-density rule
  with release-safe semantics.
- **`REUSE.toml` + CI `reuse lint`** — completes the existing `LICENSES/` directory
  into verifiable SPDX compliance for the dual-license model.
- **UI token lint** — extend `code-verify.py` to flag hard-coded colors, font
  sizes, and pixel spacing outside the theme layer (no such rule exists today).
- **`CMakePresets.json`** — folded into R3.

## Sequencing

```
Start now (independent):   R3 (tests + presets)      R1 (undo)
                           R8 (problem center)       R10 (task trees)
                           adopt-directly items
Then:                      R9 (after R8)             R2 (after R1)
                           R4 (after R3)
Then:                      R6 (after R2)             R5 (after R2)
Capstone:                  R7 (after R4)
```

Highest regret if deferred: R1 — its retrofit cost grows with every mutation site
added. Best support-return per effort: R8 + R9.

## Constraints & Invariants

- Nothing here may regress the `--benchmark-hotpath` gates or touch the cached
  hotpath flags without the dataflow-doc review the hotpath rules require.
- The spec-0001 composition-root ordering proof governs any R4 work.
- Facade headers / QML contracts preserved by the spec-0002 TU split stay intact.
- No item introduces a build-time dependency on a running network service.
- Each item runs the full spec workflow (`/ss-spec` → `/ss-plan` → `/ss-tasks` →
  `/ss-implement`) when picked up; this roadmap is not a license to skip gates.

## Open Questions

- R2: does the registry cover QML-side form generation in v1, or only model + API
  (forms follow later)?
- R5: sandbox boundary for extension script hooks — reuse the existing JS engine
  watchdog, or a stricter capability model?
- R7: is the existing gRPC transport the mirror channel, or a dedicated stream?
- R10: vendor the external task-tree library or write the ~small subset needed?

## Research notes (input to future plan phases)

Patterns observed in the reference codebases, recorded so plan phases do not
re-derive them: operation objects with apply/undo/merge and a per-document stack;
property adaptors aggregated into one generic property model, with typed "aspect"
declarations carrying value + persistence + editor widget; plugin specs with
version/compat ranges and required/optional dependency types plus lazy
instantiation keyed on declared supported types; a remote-model layer that
serializes item models over a message protocol with an object broker giving
location transparency (same code path in-process and remote); registered checkers
feeding one problem model; launcher self-tests with per-check remedy text; task
trees with barriers, conditionals, and transport adapters; soft asserts with
recovery actions; design-token enforcement as a lint rule.
