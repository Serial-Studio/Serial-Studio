---
spec: 0077-independent-modules
title: Independent core modules — downward-only layering, bus-carried upward traffic, per-library compile
status: done          # closed 2026-09-12: maintainer ran the build/run gates and closed
created: 2026-09-08
author: Alex Spataru
---

# Spec 0077 — Independent core modules

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

Spec 0076 split the application into seven static libraries under `core/` and added a typed
in-process message bus. It stopped short of independence by design: the five partition
libraries (Pipeline, Devices, Storage, Api, Ui) were moved verbatim, so each one still declares
every other library's include root plus the application's, and their real dependency graph is
a full cycle held together by singleton accessors. Measured on the tree today:

| Symptom | Measured |
|---|---|
| Upward `#include`s across the five partition libraries and the app | 594 over 16 directed edges (the ratchet baseline) |
| Cross-library singleton reaches (`X::instance()` into another library) | ~1091 sites over 142 (caller-library, callee-class) pairs |
| Unit-test registrations that recompile production sources instead of linking an archive | 283, because linking any partition archive drags the whole cycle |
| Message-bus publishers or subscribers outside its own unit test | 0; the bus is built, tested and unwired |

The consequence the maintainer feels: no library can be compiled, tested or reused without
the whole tree. A change to a GUI utility recompiles the pipeline; a pipeline header change
recompiles the drivers; a test of a codec-adjacent class links the dashboard. The
"virtual CAN bus" the libraries were meant to talk over carries nothing, and cross-library
behaviour is still discovered by reading singleton reaches and observe-only `connect` calls
on foreign objects.

The census says where the debt is, and it is not primarily a messaging problem:

- **Roughly half of every library's upward includes into the application are one header**:
  the application-level holder of the shared enums (bus type, operation mode, widget kinds,
  frame detection, and so on). Every layer needs that vocabulary; it lives at the top instead
  of the bottom.
- **Most of Pipeline's and Devices' includes into Ui are GUI-free utilities** that happen to
  live in the UI layer (translation, the shared timer clock, workspace path resolution, icon
  lookup), plus one helper that is not GUI-free: the message-box function, which every lower
  library calls for prompts and questions and which therefore needs a prompt seam owned
  below Ui rather than a move. Font and theme lookups are UI material reached from
  project-editor form code that lives in Pipeline but is UI code.
- **The genuinely runtime-state reaches are few and concentrated**: operation mode, licence
  activation, connection state, language, project loaded/modified, replay commands from
  Storage to the dashboard, console settings and window requests from Api to Ui.
- **The largest singleton counts are on edges the target graph allows** (Api reading device
  managers and the project model). Those are not layering debt; they are a style debt to be
  addressed separately.
- **The Devices↔Pipeline seam is the hotpath ingest.** Both directions count as debt under
  the target graph, and the bus is forbidden there by design.

So "make the modules talk over the bus" is the right instrument for the second group only.
The first group is a relocation, the third stays a direct call, and the fourth needs an
interface owned below both libraries. Treating all four as bus migrations would put
request/response semantics on a fire-and-forget bus, hide the remaining coupling behind
type-erased handlers, and touch ~1091 sites for a result the include gate cannot even
measure.

## Goals

- **A strict downward-only dependency graph among the seven libraries**, the one spec 0076
  declared as the target: foundation and protocols at the bottom; Pipeline and Devices as
  siblings above them; Storage above Pipeline; Api above Pipeline, Devices and Storage; Ui
  above all of those; the executable above everything. Every ratcheted edge reaches zero and
  flips to strict, so an upward include is a CI error, not a counted debt.
- **Each library compiles against only the layers below it.** A library's include roots
  list itself and its allowed lower layers, nothing else; the application's sources are not
  visible from any library. A CI leg builds each library target on its own from a clean
  configure and proves it.
- **Upward and sideways runtime communication goes over the message bus**, as retained
  state topics for "what is the current X" reads and as request topics for "please do Y"
  commands, with a documented vocabulary that grows only through a spec. A lower library
  never names a higher library's class.
- **No cross-library singleton reach outside the composition root.** The root wires the
  modules together and may reach down into any library; nothing inside a library reaches
  another library's accessor. Direct calls on allowed downward edges survive but are made
  through references the root hands in, so the callee is visible as a dependency rather than
  a global.
- **The unit-test tier links the archives.** Registrations that today recompile a production
  source because the archive would drag the cycle link the library target instead, so a test
  exercises the same compiled object as the application and the suite stops rebuilding the
  tree.
- **The hotpath is untouched in behaviour and throughput**, and the Devices↔Pipeline ingest
  seam resolves without the bus and without a new per-frame indirection.
- **The AI-facing documentation and gates describe the finished state**: the layering doc,
  the message vocabulary rules, the composition-root contract, and the censuses that measure
  the shift (upward includes to zero, cross-library reaches to zero, bus reaches counted as
  the successor).

## Non-Goals

- Removing singletons *inside* a library, or changing how the QML layer reaches C++ objects
  (the context-property names the QML tree depends on stay). An accessor reached only from
  its own library is not layering debt; it is a separate style spec if wanted at all.
- Rewriting intra-library signal/slot wiring onto the bus. Signals between two objects in one
  library are that library's business; the bus is the cross-library seam only.
- Routing frames, blocks, structure snapshots or any per-frame traffic over the bus. The
  pooled SPSC block lane of spec 0055 stays the only publication path.
- Shared or dynamic libraries, exported/installed targets, a public SDK, or plugin loading of
  first-party C++ across an ABI boundary.
- Splitting the pipeline, devices or storage libraries into finer targets, or moving the
  god-object facades of spec 0070 between libraries beyond what the target graph forces.
- Multi-process or out-of-process transport for the bus.
- Any user-visible behaviour change: dashboards, exports, recordings, API replies, licensing
  outcomes and project files must be indistinguishable before and after.
- Landing everything in one uncompiled overnight run. Each phase is compiled and run by the
  maintainer before the next begins, even though the program is committed once.

## Requirements

1. **R1** — The target dependency graph is the one spec 0076 declared. Every partition edge in
   the ratchet baseline reaches zero upward includes, and the gate then treats that edge as
   strict: a single upward include fails CI with a layering error rather than a growth error.
2. **R2** — Each library declares as include roots only itself and its allowed lower layers.
   The application's own source tree is not an include root of any library. A CI job builds
   every library target individually from a fresh configure (no application target, no test
   target) on at least one platform, and the job is a hard gate.
3. **R3** — The shared enum vocabulary and the GUI-free utilities every layer needs (the
   translation service, the shared timer clock, workspace path resolution, the third-party
   header-only helpers, notifications as a cross-cutting concern) live in the foundation layer
   or lower, and are reached by include, not by bus.
4. **R4** — Project-editor form and presentation code that only exists to drive the UI lives
   in the UI layer, so Pipeline holds no icon, font or theme reach, and the editor lands there
   as one class per translation-unit pair (the spec 0070 shape; maintainer direction
   2026-09-08), with its QML-facing surface unchanged.
5. **R5** — Every upward *runtime* dependency (a lower library needing a fact owned by a
   higher one) is a retained bus topic published by the owner: operation mode, licence
   activation, connection state per source, language, project loaded/modified, dashboard
   structure generation. A lower library reads the latest value or subscribes; it never names
   the owner.
6. **R6** — Every upward or sideways *command* (a library asking another to act: replay window
   loads and plot resets from Storage to the dashboard, console setting and window requests
   from Api to Ui, replay column registration from Storage to the pipeline, external command
   execution requests from Ui to Api) is a request topic the owner subscribes to, with the
   outcome reported as a follow-up state topic where a caller needs it. No request topic
   blocks its publisher.
7. **R7** — The bus vocabulary is a single reviewed set of plain value types built from the
   foundation layer's types only; a topic is added, renamed or changed only through a spec,
   and the topic file states the checklist a reviewer applies (value types only, no enum owned
   above the foundation, a field change is a wire break for every reader).
8. **R8** — The bus is owned by the composition root, constructed before any module, and
   handed to each core module at construction. A module holds its subscriptions as members
   and never reaches a global to find the bus. The transitional global accessor remains only
   for code the plan explicitly lists, and its use count is a ratcheted census that must not
   grow after the last phase.
9. **R9** — Direct calls that remain on allowed downward edges (Api into Devices, Pipeline and
   Storage; Ui into Pipeline, Devices, Storage and Api; Storage into Pipeline) are made through
   a reference the composition root hands in, so a library's dependencies are visible in its
   constructors, not discovered from its accessors. The cross-library singleton-reach census
   reaches zero outside the composition root.
10. **R10** — The Devices↔Pipeline ingest seam is expressed as an interface owned by a layer
    below both, keeps the bound-pointer, direct-connection, zero-allocation shape of specs
    0055/0075, and adds no per-frame virtual dispatch beyond what exists today.
11. **R11** — Unit-test registrations link the partition archives wherever the library now
    builds standalone; no suite is dropped or has its assertions changed; total suite and fuzz
    counts are unchanged or grow.
12. **R12** — The work proceeds in ordered phases, each leaving the working tree green on
    every gate (`code-verify`, `layer-verify`, `claim-verify`, `registry-verify`, unit tier,
    integration tier against a running app, the hotpath benchmark) and each compiled and run
    by the maintainer before the next phase starts. The whole program lands as ONE commit
    (maintainer decision, 2026-09-08); the phases are working-tree checkpoints, not history.
    A phase never widens a ratchet baseline to make room for itself.
13. **R13** — The composition-root contract of spec 0039 holds throughout: construction order
    is pinned, the constructor-closure surfaces stay free of reaches into modules built later,
    shutdown releases in reverse order after the pipeline thread and stream workers join, and
    the ctor-edge proof of spec 0001 is re-run for every phase that touches ctor-reachable
    code.

## Acceptance Criteria

- [x] **AC1** — `scripts/layer-verify.py` reports every edge at zero and the baseline file
  holds no ratcheted edges; a scratch file with one upward include fails with a strict layering
  error on every partition layer.
- [x] **AC2** — A CI job builds each of the seven library targets alone from a fresh configure
  (no application, no tests) and passes; deleting one library's lower-layer include root makes
  it fail, proving the roots are real.
- [x] **AC3** — `code-verify.py --singleton-census` shows zero cross-library reaches outside
  the composition root, and the transitional bus accessor count matches the plan's allowed
  list exactly.
- [x] **AC4** — A publish/subscribe census (new `code-verify.py` mode) lists every topic with at
  least one publisher and at least one subscriber, and no topic appears in a hotpath TU
  (`bus-on-hotpath` stays clean).
- [x] **AC5** — `--benchmark-hotpath` holds every tier at its current gate on the release
  binary after the final phase and after the Devices↔Pipeline phase in particular.
- [x] **AC6** — `ctest` passes with the same or larger suite and fuzz counts; the number of
  registrations that recompile a partition source drops to the plan's residual list.
- [x] **AC7** — The full `pytest` integration, security and performance suites pass against a
  running build of the final phase; project files, CSV/MDF4/Historian recordings and API
  replies are byte-for-byte or field-for-field identical to the pre-program build on the
  fixture projects.
- [x] **AC8** — In the running app: switching operation mode, activating or deactivating a
  licence, changing language, connecting and pausing a device, loading a project, and replaying
  a recording all behave as before, observed by the maintainer against a checklist in the plan.
- [x] **AC9** — `claim-verify.py`, `registry-verify.py`, `documentation-verify.py` and
  `reuse lint` are clean; `CLAUDE.md`, the directory map and the dataflow/startup docs describe
  the strict graph, the bus vocabulary rule and the injection contract.
- [x] **AC10** — The program lands as one commit on `master`; the plan's per-phase checklist
  records which gates were green at each checkpoint, since `git bisect` cannot isolate a phase.

## Constraints & Invariants

- **No compiler in the agent loop.** Every phase is sized so the maintainer can configure,
  build, run the unit tier and the benchmark, and exercise the app between phases. The plan
  names the build and check commands per phase.
- **Hotpath rules are non-negotiable**: no allocation on the publish path, direct connections
  between pipeline-thread objects, cached hotpath flags re-wired whenever their input moves,
  the bus never on a per-frame TU, source owns time, diagnostics pulled not pushed.
- **The bus is command/state/notification rate only.** A topic that turns out to run at frame
  rate is a design error to be fixed at the producer, never by giving the bus queues.
- **Publishers never wait on subscribers.** No blocking delivery, no request topic whose
  publisher spins an event loop for the answer. Where a synchronous answer is genuinely
  needed on an allowed downward edge, that is a direct call through an injected reference,
  not a bus round trip.
- **Composition-root invariants of specs 0001/0039/0042 hold at every phase**: pinned
  construction order, licensing built first after translation, the ctor-closure surfaces
  untouched or re-proven, `SessionContext::current()` called only from the root and accessor
  forwarders, shutdown in reverse order after thread joins.
- **Behaviour-preserving.** No feature, format, API reply, or licensing outcome changes.
  Every moved definition is verbatim unless the plan calls out the edit.
- **Gates ratchet down, never up.** No baseline (layer, singleton, TU, dup, claim) is widened to
  admit a phase.
- **One class, one `.h`/`.cpp` pair, one target.** Moves between libraries are whole pairs; no
  class is split across translation units or targets; no double moc.
- **GPL and Pro source sets are conserved.** Pro-only code stays out of the GPL binary; moving
  a file between libraries carries its gating with it.
- **Trust Contract.** Nothing outside a phase's file list is touched; foreign working-tree
  files are never reverted; nothing is committed without per-phase permission.

## Open Questions

Resolved by the maintainer on 2026-09-08 (all recommendations accepted except item 5):

1. **Bus ownership** — constructor injection. The bus is built first and handed to each core
   module's constructor; the transitional global accessor survives only on a plan-listed
   allowlist and is ratcheted.
2. **Target graph** — spec 0076's graph stands: Pipeline and Devices are siblings over the
   foundation; the ingest seam is an interface owned below both.
3. **Scope** — cross-library reaches only. Intra-library accessors and the QML
   context-property surface are out of scope.
4. **Allowed-edge reaches** — converted to injected references in this program, as the last
   phase.
5. **Granularity** — one spec, a phased plan with maintainer-compiled checkpoints, but ONE
   commit for the whole program (the maintainer's call, overriding the per-phase-commit
   recommendation; R12 and AC10 amended accordingly).
