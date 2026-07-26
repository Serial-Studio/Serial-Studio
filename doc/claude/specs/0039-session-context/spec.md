---
spec: 0039-session-context
title: Session context over global singletons
status: in-progress  # draft -> approved -> in-progress -> done | shelved
created: 2026-07-25
author: Claude (roadmap item R4, spec 0030)
---

# Spec 0039 — Session context over global singletons

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.
>
> Roadmap parent: spec 0030, item R4. Depends on R3 (spec 0032, C++ unit tier) for the
> safety net that makes injected classes worth injecting. Capstone dependant: R7 (spec
> 0040, remote dashboard attach) cannot mirror state that only exists as a process global.

## Problem / Motivation

Every piece of session state in this application is a process global. The project
document, the dashboard, the I/O manager, the frame pipeline, and the parser are Meyers
singletons reached through `instance()`. Spec 0001 fixed the *dangerous* half of that —
construction order is now pinned in code and the AppState/ProjectModel re-entrancy hazard
is dead — but it deliberately did not remove the globals, and the current census shows
what the 2026-07 sweep actually accomplished:

- 1,524 `::instance()` occurrences across 176 of the 509 C++ files under `app/src`.
- **969 of them are the function-local `static auto& x = X::instance();` cache idiom** —
  the dominant form by a wide margin. That idiom binds a function to one specific
  singleton for the life of the process, on first call.
- 90 are constructor init-list captures, 20 are deferred pointers assigned during wiring,
  137 live in the composition root itself, 63 are the accessors' own definitions.
- The `arch-singleton-instance` linter advisory currently reports **zero** findings,
  because every one of those forms is sanctioned. The count that the roadmap proposes to
  "track and trend down" is already at its floor while the global-state surface is
  undiminished. The sweep converted *how* dependencies are acquired; it did not reduce
  *what* is global.

The cost is structural, not cosmetic. There is no way to hold two project documents at
once, no way to run an in-process test against a real subsystem without booting the whole
composition root (which is also why the C++ unit tier of R3 has so little it can reach),
and no way for a GUI to mirror a second, remote capture session — R7's entire premise.
Every one of those is blocked by the same missing thing: an object that says "this is the
state of *a* session", so that the answer to "which project model?" can ever be anything
other than "the one".

The migration cannot be a flag day. 1,524 acquisition sites and roughly sixty QML context
properties that need stable long-lived `QObject` addresses make a big-bang rewrite exactly
the option spec 0001 already rejected, twice, with reasons that have not changed. What is
missing is the seam: a context object that today merely *names* the session-scoped
subsystems, so classes can start taking it instead of reaching for globals, one class at a
time, as they are touched for other reasons.

## Goals

- A single named object stands for "the state of one session", listing exactly which
  subsystems are session-scoped and which are application-wide.
- A class can declare its session dependencies in its constructor instead of reaching for
  globals in its method bodies, and three real classes do so as proof.
- A converted class's pure logic can be exercised from a C++ unit test without starting
  the composition root, the QML engine, or a project model.
- The global-state surface is measured honestly — a census that counts the cached-static
  and captured forms, not only the loose calls the current advisory misses — and cannot
  grow silently.
- A maintainer reading the startup docs can tell, without reading code, whether a given
  subsystem belongs to the session or to the application.
- Nothing about startup order, hotpath throughput, or QML bindings changes observably.

## Non-Goals

- **Not removing any singleton.** Every `instance()` accessor stays, keeps its address,
  and stays registered as its QML context property. This spec adds a seam; it does not
  close one.
- **Not achieving two independent sessions.** That is the end state, not this milestone —
  see "Acceptance Criteria" for the honest intermediate. Anything that claims otherwise in
  v1 would be false.
- **Not touching the hotpath.** No frame-path class takes the context in v1. The
  `--benchmark-hotpath` gates must be unaffected *by construction*, not by measurement.
- **Not changing the pinned construction order.** Spec 0001's order is not reordered, not
  extended, and not re-derived by this work.
- **Not container DI or a service locator.** Both were rejected in spec 0001 for reasons
  that still hold; this is plain constructor injection with one publication point.
- **Not migrating the 1,524 sites.** Three pilot classes, chosen for low risk, are the
  whole conversion scope. Everything else converts when it is touched for other reasons.
- **Not exposing the context to QML.** QML keeps binding to the same context properties.
- **Not moving true application-globals** — translator, theme, fonts, icon registry,
  workspace paths, timers, licensing — into the session. They are app-wide and stay so.

## Requirements

1. **R1** — A session context type exists and names every session-scoped subsystem: the
   project document, application state, the frame pipeline (builder and parser), the I/O
   manager, the dashboard, the console, and the notification center.
2. **R2** — The context is available from a single, documented publication point, and that
   point is the only sanctioned way to reach it outside constructor injection.
3. **R3** — The context is created inside the existing composition root, after the pinned
   core-module order has fully run and before any cross-module wiring, and it creates
   nothing itself.
4. **R4** — Three pilot classes — one lifecycle class, one always-built pure-logic class,
   and one commercial class — take the context as a constructor parameter and acquire
   every session dependency through it, with no remaining global reach in their bodies.
5. **R5** — A pilot's pure logic is constructible and exercisable in a C++ unit test
   against a locally constructed context, without the composition root running.
6. **R6** — A converted class cannot silently regress: any global reach reintroduced into a
   class that takes the context is reported by the linter.
7. **R7** — The global-state surface is counted per class and per acquisition form against
   a checked-in baseline, and an increase in the total is a gated failure, not a silent
   drift.
8. **R8** — Startup documentation states the session/application split and the injection
   convention for new classes, so the next class written follows it without being told.
9. **R9** — Startup behavior, operation modes, QML bindings, and hotpath throughput are
   observably unchanged.

## Acceptance Criteria

The roadmap's stated acceptance for R4 — *"two independent contexts can be constructed in
one test process with no state bleed"* — is **not** achievable in this spec and must not be
claimed. The subsystems the context names are still singletons; a second context would
hand out the same objects. This spec's honest milestone is the seam plus proof plus
measurement; the phased path to the roadmap's criterion is recorded below it.

- [x] **AC1 (R1, R2, R3)** — The context type exists, names the session subsystems, and is
  created in the composition root between the pinned module order and the wiring sequence.
  Verified by reading the composition-root sequence and by the spec-0001 verification
  recipe (grep symmetry + INV-1/INV-2/INV-3) re-run and recorded in this spec directory.
  *M1 publication was `(void)SessionContext::current();` between `instantiateCoreModules()`
  and the first `setupExternalConnections()` (now a vestigial no-op at ModuleManager.cpp:688 —
  since M2 the context is first reached as the opening statement of
  `instantiateCoreModules()` and adopts modules through the pinned order); the re-run is
  [`ctor-proof.md`](./ctor-proof.md), superseded for ownership by
  [`ctor-proof-m2.md`](./ctor-proof-m2.md).*
- [x] **AC2 (R4)** — For each of the three pilot classes: the constructor takes the context,
  every session dependency in the class body is reached through it, and a grep for
  `::instance()` in that file returns only the class's own accessor definition.
  *`BackupManager.cpp`, `ProtoImporter.cpp`, `DBCImporter.cpp` each report exactly 1
  occurrence, all classified `accessor` by the census.*
- [ ] **AC3 (R5)** — A C++ unit test constructs a context and a pilot on the stack, runs the
  pilot's pure generation path against a fixture, and asserts the produced project — with
  no composition root, no QML engine, and no project model constructed. (Runs on the R3
  unit target; if spec 0032 has not landed, the test source lands with the target stubbed
  out and the criterion is checked when 0032 lands.)
  ***Gated, not skipped.** `app/tests/tst_proto_importer.cpp` is written and lands with this
  spec. Spec 0032's target exists, but the suite cannot be registered: linking it drags in
  `Misc::Utilities`, `ProjectModel` (18 TUs), `SerialStudio`, and all eight session accessors,
  i.e. the application. The four undefined-symbol groups are recorded next to the commented
  `ss_add_unit_test` call in `app/tests/CMakeLists.txt`. **Correction (M2 planning):** ownership
  alone does not cut that link set — the blocker is TU reach, not acquisition. AC3 closes by
  splitting the pure generator into its own TU (`m2-plan.md`, M2-T16), which M2 carries.*
- [x] **AC4 (R6)** — A synthetic global reach added to a converted class is reported by the
  linter; the same construct in an unconverted class is not. Blocking-error count for the
  repo is unchanged by the new rule. *Both halves reproduced with a synthetic pair under
  `app/src`; repo-wide blocking errors 0 before and after, `arch-session-context-bypass` is
  advisory and reports 0 on the real tree.*
- [x] **AC5 (R7)** — The census reports the per-class and per-form breakdown; the baseline is
  checked in; adding a new global-reach site anywhere under `app/src` makes the census check
  fail, and the failure names the file. *Baseline `scripts/singleton-census.json`: 1579
  occurrences over 191 files, static-cache 1103. The synthetic pair drove `--check` to fail
  and named both files.*
- [x] **AC6 (R8)** — The startup architecture doc has a session-context section stating the
  session/application split and the convention for new classes; CLAUDE.md points at it.
  *Closed 2026-07-25 via T12 + M2-T15: `startup.md` "Session Context (spec 0039)" section
  (ownership, INV-4/5/6, empty-ctor rule, shutdown point + crash class, the verbatim
  negative rule), CLAUDE.md "Startup & Composition Root" bullet, `directory-map.md` entry.*
- [ ] **AC7 (R9)** — Maintainer launches in ProjectFile, QuickPlot, and ConsoleOnly modes:
  startup, project restore, dashboard, and console behave as before. `--benchmark-hotpath`
  is *not required* — no hotpath file is touched — and the diff is checked to prove it.
  *The no-hotpath-file half is checked; the launches are the maintainer's.
  `app/CMakeLists.txt` registers `src/SessionContext.{h,cpp}` (T2 done), so the launches are
  unblocked — see also the M2 maintainer-gate checklist in `ctor-proof-m2.md`.*

**Phased path to the roadmap criterion** (each phase its own spec, not this one):

- **M1 — this spec.** The seam exists, three classes are injected, the surface is measured.
- **M2 — ownership.** The context stops forwarding and starts owning: session subsystems
  move from Meyers accessors to context-owned `unique_ptr` slots, `instance()` becomes a
  forwarder class by class, and teardown becomes an explicit reverse-order release while
  `qApp` is still alive. This is where the ctor-edge proof genuinely moves and must be
  re-derived. Phase plan: [`m2-plan.md`](./m2-plan.md). Note the correction it carries — the
  pinned order stays in `instantiateCoreModules()` and must **not** become the context's
  constructor body, because a constructing ctor re-enters the `current()` Meyers guard from
  every module ctor that reaches a singleton.
  **Status (2026-07-25): code-complete, maintainer gates open.** All eight waves have landed —
  `NotificationCenter`, `ProjectModel`, `AppState`, `FrameBuilder`, `ConnectionManager`,
  `Console::Handler`, `FrameParser`, and `UI::Dashboard` are owned by the context, `shutdown()`
  releases them in reverse pinned order while `qApp` is alive, and the ctor-edge proof is recorded
  as **re-derived** with the eight-wave C1-C11 table in
  [`ctor-proof-m2.md`](./ctor-proof-m2.md). Nothing has been built or launched by the implementing
  pass: the three-mode launches, the nine-tier `--benchmark-hotpath` comparison against the Wave A
  baseline, and the real-device quit paths (window close, Cmd-Q, Dock-quit, USB/HID attached) are
  all still open and listed as one checklist in that file. M2-T15 (docs), M2-T16 (the AC3 TU
  split), and M2-T18 (handoff) remain.
- **M3 — plurality.** A second context is constructible; `current()` becomes scoped rather
  than process-wide; the roadmap's "two contexts, no state bleed" criterion becomes
  testable. R7 (remote attach) consumes this: a remote dashboard is a second context whose
  project model, dashboard state, I/O manager, and frame pipeline are fed from the wire
  rather than from local hardware, which is only expressible once M3 exists.

## Constraints & Invariants

- **Spec 0001's pinned construction order is not modified.** No entry added, removed, or
  reordered. Any change to it re-triggers the ctor-edge proof; this spec is designed
  specifically to avoid that trigger, and the plan must state how.
- **ProjectModel's constructor closure stays protected.** Nothing reachable from it may
  name the session context, for the same reason it may not name AppState or Dashboard.
- **No hotpath file is touched.** The frame path keeps its captured members and cached
  flags exactly as they are; the context has no per-frame call site in v1.
- **Startup invariants INV-1..INV-3 hold** (wiring before project restore; context
  properties after wiring and before QML load; message handler after console and
  notification center exist).
- **The commercial pilot compiles out cleanly** in a GPL build; the session context itself
  must not depend on any commercial symbol.
- **No new dependency**, no new build-time service, no new QML surface.
- **One publication point.** The seam must not become a service locator: exactly one
  documented accessor, sanctioned in exactly one place per class, gated by the linter.

## Open Questions

- Should the context be a `QObject` (future session lifecycle signals, and eventually a
  QML surface for R7) or a plain class (nothing can bind to it prematurely)? The plan
  recommends plain for v1; confirm before implementation.
- Do the pilot classes keep their `instance()` accessors indefinitely, or is there a
  deprecation marker on them once injected? (Affects whether M2 can be mechanical.)
- Should the census gate block on the total only, or also on a per-form floor (e.g. the
  static-cache bucket may never grow even if the total falls)?
- Is `Console::Handler` session-scoped or application-scoped? It is arguably per-session
  (its buffer belongs to a capture) but is also the sink for application-wide warnings via
  the Qt message handler. The plan assumes session-scoped; confirm.
