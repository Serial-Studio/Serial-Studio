---
spec: 0039-session-context
phase: tasks
status: draft        # draft -> approved (gate before /ss-implement)
updated: 2026-07-25
---

# Tasks 0039 — Session context over global singletons

> **Phase 3 of 4 — the ordered checklist.** Decompose [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable* — each one a coherent diff a reviewer
> could read in isolation. `/ss-implement` works this list top to bottom and keeps the status
> boxes current. Gate: do not start `/ss-implement` until a human marks this `approved`.

## Conventions

- One task = one focused, reviewable change. If a task touches >3 files or needs a paragraph
  to describe, split it.
- **Verify** is how *this* unit is confirmed before moving on — usually
  `python scripts/code-verify.py --check <files>`, plus a test or a read-back where one fits.
- **Deps** lists task IDs that must land first.
- Order so the tree compiles (conceptually) after each task where practical.
- **Standing constraint for every task:** no file on the Driver → FrameReader → FrameBuilder
  → Dashboard path may appear in any diff. If one does, stop and escalate.

## Tasks

### T1 — Add the `SessionContext` type

- **Files:** `app/src/SessionContext.h`, `app/src/SessionContext.cpp`
- **Does:** Global-namespace `class SessionContext`: empty constructor, virtual destructor,
  deleted copy/move, no data member other than the session id. Eight `[[nodiscard]] virtual
  X& f() const` accessors (`appState`, `projectModel`, `frameBuilder`, `frameParser`,
  `connectionManager`, `dashboard`, `console`, `notifications`), each a one-line forward to
  the existing `instance()` in the `.cpp`. `static SessionContext& current()` returns the
  function-local process-default instance. Header uses forward declarations only; the `.cpp`
  carries the includes.
- **Verify:** `python scripts/code-verify.py --check app/src/SessionContext.h
  app/src/SessionContext.cpp` clean; header order matches the CLAUDE.md block order;
  read-back confirms the constructor body is empty and no module-typed member exists.
- **Deps:** none
- [x] done

### T2 — Register `SessionContext` in the build

- **Files:** `app/CMakeLists.txt`
- **Does:** Add `src/SessionContext.cpp` to the sources list next to `src/AppState.cpp`
  (~line 352) and `src/SessionContext.h` to the headers list (~line 503). Not inside any
  `BUILD_COMMERCIAL` block.
- **Verify:** Both entries present, alphabetically consistent with neighbours, outside every
  commercial guard; `grep -n "SessionContext" app/CMakeLists.txt` shows exactly two lines.
- **Deps:** T1
- [x] done
- **Status:** landed by the session coordinator (2026-07-25): `src/SessionContext.cpp` and
  `src/SessionContext.h` registered next to `AppState` in `app/CMakeLists.txt`, outside every
  commercial guard. Original hand-off note kept below for the record:
  `src/SessionContext.cpp` next to `src/AppState.cpp` in the sources list, and
  `src/SessionContext.h` next to `src/AppState.h` in the headers list, both outside every
  `BUILD_COMMERCIAL` guard. Nothing else in this spec compiles until they land.

### T3 — Pin construction in the composition root and re-run the ctor-edge proof

- **Files:** `app/src/Misc/ModuleManager.cpp`, `doc/claude/specs/0039-session-context/ctor-proof.md`
- **Does:** Insert `(void)SessionContext::current();` in `setupCrossModuleConnections()`
  immediately after the `instantiateCoreModules()` call (line ~661) and before the first
  `setupExternalConnections()`. `instantiateCoreModules()` itself is **not** edited. Record
  the spec-0001 verification recipe results in `ctor-proof.md`: grep symmetry against the
  spec-0001 pinned-order table, INV-1 (`restoreLastProject()` still last), INV-2 (context
  properties after wiring, before `m_engine.load`), INV-3 (message handler after
  `Console::Handler` + `NotificationCenter`), and the new out-edge check (empty ctor, no
  module-typed member).
- **Verify:** `git diff app/src/Misc/ModuleManager.cpp` shows exactly one added line and no
  change inside lines 619-654; `ctor-proof.md` records all five checks with the greps used.
- **Deps:** T1, T2
- [x] done
- **Status:** `(void)SessionContext::current();` at ModuleManager.cpp:669, plus the include and
  two lines of `@brief` prose. Spec 0039 adds nothing to `instantiateCoreModules()`; the two
  new entries visible there (`ProblemCenter`, `ConnectionDiagnostics`) belong to specs 0033 and
  0035. All five checks recorded in [`ctor-proof.md`](./ctor-proof.md).

### T4 — Sanction the forwarding file in the linter

- **Files:** `scripts/code_verify_rules.py`
- **Does:** Add `/app/src/SessionContext.cpp` to `_SINGLETON_ROOT_FILES` so the forwarding
  accessor bodies are not reported by `arch-singleton-instance`.
- **Verify:** `python scripts/code-verify.py --check --no-report app/src/SessionContext.cpp`
  reports zero advisories; the repo-wide blocking-error count is unchanged.
- **Deps:** T1
- [x] done

### T5 — Pilot 1: inject `Misc::BackupManager`

- **Files:** `app/src/Misc/BackupManager.h`, `app/src/Misc/BackupManager.cpp`
- **Does:** Constructor becomes `explicit BackupManager(SessionContext& ctx)` (public; copy
  and move stay deleted); add a `SessionContext& m_ctx` reference member; `instance()` becomes
  `static BackupManager singleton(SessionContext::current());`. Replace the `m_projectModel`
  deferred pointer and the direct `ProjectModel::instance()` in `setupExternalConnections()`
  (BackupManager.cpp:99-101) with `m_ctx.projectModel()`; keep the three existing connections
  and their types unchanged.
- **Verify:** `grep -n "::instance()" app/src/Misc/BackupManager.cpp` returns only the
  accessor definition; `python scripts/code-verify.py --check` clean on both files; the
  `Q_ASSERT` density in touched functions does not drop.
- **Deps:** T1, T2, T3
- [x] done
- **Status:** the five `Q_ASSERT(m_projectModel)` guards were converted to
  `SS_ASSERT(m_projectModel != nullptr, <recovery>)` in the same pass (the spec-0033 assert
  rule landed in parallel), so density rose rather than dropped.

### T6 — Pilot 2a: extract `ProtoImporter`'s pure generator

- **Files:** `app/src/DataModel/Importers/ProtoImporter.h`, `app/src/DataModel/Importers/ProtoImporter.cpp`
- **Does:** Promote the `.proto` → project-JSON generation path to a public
  `[[nodiscard]] QJsonObject projectFromProtoFile(const QString& path)` that touches no
  session state; `confirmImport()` calls it. **Verbatim body move only** — no logic edit.
- **Verify:** Read-back diff shows the moved body is character-identical apart from the
  signature and indentation; `python scripts/code-verify.py --check` clean on both files;
  generated project JSON for a fixture is unchanged (maintainer spot-check on import).
- **Deps:** T1
- [x] done
- **Status:** the generator itself moved verbatim — `generateProject()` was renamed
  `projectFromMessages()` with a character-identical body. The public entry the test calls,
  `projectFromProtoFile(path)`, is *new* code (11 lines): it repeats the open / size-cap /
  parse sequence of `showPreview()` minus the four `showMessageBox` calls, because the pure
  path must report failure by returning an empty object rather than raising a dialog. That
  duplication is the one place this task deviates from "verbatim move only"; it is the reason
  the pure path reaches no session state.

### T7 — Pilot 2b: inject `DataModel::ProtoImporter`

- **Files:** `app/src/DataModel/Importers/ProtoImporter.h`, `app/src/DataModel/Importers/ProtoImporter.cpp`
- **Does:** `explicit ProtoImporter(SessionContext& ctx)` (public; copy/move stay deleted),
  `SessionContext& m_ctx` member, `instance()` passes `SessionContext::current()`, and the
  `ProjectModel::instance()` reach at `ProtoImporter.cpp:1004` becomes `m_ctx.projectModel()`.
- **Verify:** `grep -n "::instance()" app/src/DataModel/Importers/ProtoImporter.cpp` returns
  only the accessor definition; `code-verify --check` clean; `Cpp_JSON_ProtoImporter`
  registration in `ModuleManager.cpp:763` is unchanged.
- **Deps:** T6
- [x] done
- **Status:** the reach was a `static auto& pm = ProjectModel::instance();` — the cached-static
  form the old advisory sanctioned — and is now `auto& pm = m_ctx.projectModel();`. The context
  property moved to line 773 with the file's other shifts; the binding is unchanged.

### T8 — Pilot 3: inject `DataModel::DBCImporter` (commercial)

- **Files:** `app/src/DataModel/Importers/DBCImporter.h`, `app/src/DataModel/Importers/DBCImporter.cpp`
- **Does:** Same shape as T6+T7 in one task (the class is commercial-only and its generator
  split is smaller): public `explicit DBCImporter(SessionContext&)`, `m_ctx` member,
  `instance()` passes `current()`, the `ProjectModel::instance()` reach at
  `DBCImporter.cpp:209` becomes `m_ctx.projectModel()`, and `generateProject(...)` is exposed
  as a public `[[nodiscard]] QJsonObject projectFromMessages(...)` by verbatim promotion.
- **Verify:** `grep -n "::instance()" app/src/DataModel/Importers/DBCImporter.cpp` returns
  only the accessor definition; `code-verify --check` clean; `SessionContext.h` names no
  commercial type (`grep -n "Sessions::\|MQTT::\|Licensing::\|Modbus\|CanBus"
  app/src/SessionContext.h` empty).
- **Deps:** T1, T2, T3
- [x] done

### T9 — Linter rule `arch-session-context-bypass`

- **Files:** `scripts/code_verify_rules.py`, `scripts/code-verify.py`
- **Does:** New advisory rule: in any file whose text contains `SessionContext&`, flag every
  `::instance()` occurrence *including* the `static auto&` cache idiom and constructor
  init-list captures, except inside the class's own `instance()` accessor body. Register the
  kind in `_ADVISORY_KINDS` and document it in the report prose next to
  `arch-singleton-instance`.
- **Verify:** Scratchpad snippets — a `Foo::instance()` and a `static auto& f =
  Foo::instance();` in a file mentioning `SessionContext&` both report; the same two in a file
  that does not mention it do not; `SessionContext::current()` inside an `instance()` body
  does not. Repo-wide blocking-error count unchanged; the three pilot files report zero.
- **Deps:** T5, T7, T8
- [x] done
- **Status:** verified end to end. A synthetic pair copied under `app/src/` reported
  `arch-session-context-bypass` twice in the converted file (loose reach and cached static) and
  not at all in the unconverted twin, while `arch-singleton-instance` reported the loose reach
  in both; `SessionContext::current()` inside the `instance()` body reported nothing. Repo-wide:
  548 files, **0 errors**, 231 advisories, of which 0 are `arch-session-context-bypass`. The
  scratch files were removed after the run.

### T10 — Census mode `--singleton-census` plus baseline

- **Files:** `scripts/code-verify.py`, `scripts/code_verify_rules.py`, `scripts/singleton-census.json`
- **Does:** New mode classifying every `::instance()` occurrence under `app/src` into the six
  buckets in `plan.md` (root / accessor / ctor-capture / deferred / static-cache / loose),
  reporting per class and per bucket. `--check` fails when the total or the static-cache
  bucket increases versus `singleton-census.json`; `--accept` re-baselines. Seed the baseline
  from the post-T8 tree.
- **Verify:** Classifier output matches hand counts on three files (`ModuleManager.cpp` 130,
  `BackupManager.cpp` 3, `DBCImporter.cpp` 2) and the tree-wide total matches
  `grep -rc "::instance()" app/src --include="*.cpp" --include="*.h"`; adding a synthetic
  `Foo::instance()` under `app/src` makes `--check` fail and names the file; removing it
  passes again.
- **Deps:** T5, T7, T8
- [x] done
- **Status:** baseline is **1579 occurrences across 191 files**, static-cache **1103** — higher
  than the 1,524 / 969 quoted in `spec.md` because seven other specs landed code in this tree
  between the census in the spec and the one taken here. Classifier cross-checked against
  `grep -c "::instance()"` on five files with exact agreement (`ModuleManager.cpp` 136 all
  root, `SessionContext.cpp` 8 all root, and 1 accessor apiece in the three pilot `.cpp`s).
  Gate proven: the synthetic pair drove `--check` to fail with
  `total 1579 -> 1585, static-cache 1103 -> 1105` and named both files; removing them passed.

### T11 — Wire the census into the sanitize pipeline

- **Files:** `scripts/sanitize-commit.py`
- **Does:** Run `code-verify.py --singleton-census --check` after the existing
  `code-verify --check` stage; a failure stops the pipeline with the re-baseline instruction.
- **Verify:** `python scripts/sanitize-commit.py --help` (or a dry run) shows the new stage in
  the driver sequence; a synthetic census increase stops the pipeline with the expected
  message.
- **Deps:** T10
- [x] done
- **Status:** the stage runs as a blocking gate ("Checking the singleton census") between
  `.code-report` regeneration and `black`, and is listed in the driver header comment.

### T12 — Documentation: session/application split and the injection convention

- **Files:** `doc/claude/architecture/startup.md`, `CLAUDE.md`, `doc/claude/directory-map.md`
- **Does:** New "Session Context" section in `startup.md` — what is session-scoped, what stays
  application-wide, the single publication point, the injection convention for new classes,
  the ctor-edge preservation argument, and the M1/M2/M3 path. Three lines in CLAUDE.md's
  "Startup & Composition Root" block pointing at it and stating the convention. One
  `directory-map.md` entry for `SessionContext.{h,cpp}`.
- **Verify:** `python scripts/documentation-verify.py` clean; the startup.md section states
  the negative rule ("do not call `current()` from a method body") verbatim; CLAUDE.md gains
  no more than three lines.
- **Deps:** T5, T7, T8
- [x] done
- **Status:** closed 2026-07-25, jointly with M2-T15 (the section was written once, covering
  M2 ownership rather than M1 forwarding): `startup.md` gained "Session Context (spec 0039)"
  (split, adopt→live→shutdown, INV-4/5/6, empty-ctor rule, shutdown point + crash class,
  the verbatim negative rule, injection pilots, census gate); CLAUDE.md gained one bullet in
  "Startup & Composition Root"; `directory-map.md` names `SessionContext.h`. Deviation from
  the T12 sketch: the CLAUDE.md bullet exceeds three lines because it also carries the
  shutdown-lockstep rule; the historical crash instances are named by class, not enumerated.

### T13 — Unit test carrier for the injected pilot (gated on spec 0032)

- **Files:** `app/tests/tst_proto_importer.cpp` (+ its `app/tests/CMakeLists.txt` target, per
  spec 0032's Qt Test layout)
- **Does:** Construct a `SessionContext` and a `ProtoImporter` on the stack, run
  `projectFromProtoFile()` against a fixture, assert group count, dataset count, and
  field→dataset mapping. No composition root, no QML engine, no project model constructed.
  If spec 0032's `ctest` target does not exist yet, land the source with its registration
  commented and leave AC3 unchecked.
- **Verify:** `ctest` green locally when 0032's target exists; otherwise the source compiles
  in isolation by inspection and the gating note is recorded in `spec.md` under AC3.
- **Deps:** T7 (and spec 0032's unit target for the runnable half)
- [ ] done
- **Status:** source half done, runnable half gated. `app/tests/tst_proto_importer.cpp` exists
  — seven cases over a stack `SessionContext` + stack `ProtoImporter`, asserting group and
  dataset counts, dense 1..N dataset indices, bool→LED mapping, the generated Lua source, and
  the empty-object failure path. The `ss_add_unit_test` call stays commented, and **not**
  because spec 0032's target is missing (it landed): the minimal link set that resolves is the
  application. `ProtoImporter.cpp` needs `Misc::Utilities::showMessageBox`,
  `ProjectModel::importCompleted` / `importProjectFromJson` plus ProjectModel's metaobject (18
  TUs), and `SerialStudio::groupWidgetId`; `SessionContext.cpp` needs all eight session
  accessors. Registering it would configure and then fail at link, taking `ss_unit_tests` and
  CI down. The four symbol groups are recorded verbatim in `app/tests/CMakeLists.txt` next to
  the commented registration. NOT unblocked by M2 as hoped (2026-07-25): even with the
  generation TU split out, the suite's stack `SessionContext` drags the context's dtor
  closure — six module vtables (moc TUs) + `~ConnectionManager`/`~NotificationCenter` — see
  the M2-T16 refutation in `m2-plan.md`; needs a test-double seam, not the originally hoped
  "context returns interfaces" route. **AC3 stays unchecked.**

### T14 — Handoff verification sweep

- **Files:** none (verification only)
- **Does:** Confirm the whole-feature gate below; record the census delta and the ctor-proof
  outcome in the handoff summary; state explicitly that the roadmap's "two contexts, no state
  bleed" criterion is **not** met and that M2/M3 carry it.
- **Verify:** every box in "Definition of Done" checked.
- **Deps:** T1-T13
- [ ] done
- **Status:** swept, but the gate is not closed: T13 is gated on the M2-T16 TU split (T2 and
  T12 have since closed — build registration landed, docs written 2026-07-25). Recorded for the handoff:
  - Census delta versus the pre-change tree is **zero** — the three conversions each replaced
    one reach with a context call, so the total stayed at 1579 and static-cache at 1103. The
    baseline is a floor to trend down from, not a reduction this spec achieved.
  - Ctor-proof verdict: **preserved, not re-derived** ([`ctor-proof.md`](./ctor-proof.md)).
  - The roadmap's R4 criterion — *two independent contexts in one test process with no state
    bleed* — is **not met** and is not claimed. The eight subsystems the context names are
    still Meyers singletons; a second `SessionContext` would hand out the same objects. M2
    (ownership) and M3 (plurality) carry that criterion.

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` is met and checked off there — except AC3 if
      spec 0032 has not landed, which is recorded as gated rather than silently skipped.
      *(AC1, AC2, AC4, AC5 met. AC3 gated on the link set, AC6 open with T12, AC7 needs the
      maintainer.)*
- [x] `python scripts/code-verify.py --check` is clean on all changed files, and the repo-wide
      **blocking-error count is identical** to the pre-change count (the two new rules are
      advisory). *(10 changed files: 0 errors, 0 advisories. `app/src`: 548 files, 0 errors.)*
- [x] `python scripts/code-verify.py --singleton-census --check` passes; the baseline is
      checked in and the delta versus the pre-change tree is stated in the handoff.
      *(1579 total / 1103 static-cache, matching baseline; delta zero.)*
- [x] `doc/claude/specs/0039-session-context/ctor-proof.md` exists and records all five
      spec-0001 checks (grep symmetry, INV-1, INV-2, INV-3, the SessionContext out-edge
      check) with the commands used — the ctor-edge proof re-run, not assumed.
- [x] `git diff --name-only` contains **no** hotpath file (`FrameReader`, `CircularBuffer`,
      `FrameBuilder`, `ConnectionManager`, `Dashboard`, `DSPSimd.h`, `HotpathOptimization.h`);
      `--benchmark-hotpath` is therefore not required, and the diff is the evidence.
      *(Spec-0039 files: `SessionContext.{h,cpp}`, `ModuleManager.cpp`, the six pilot files,
      three `scripts/`, `singleton-census.json`, and the two under `app/tests/`. The hotpath
      files that are modified in this working tree belong to other specs.)*
- [x] `instantiateCoreModules()` (ModuleManager.cpp:619-654) is byte-identical to its
      pre-change content. *(With respect to this spec: 0039 adds no line there. Specs 0033 and
      0035 added `ProblemCenter` and `ConnectionDiagnostics`; see `ctor-proof.md`.)*
- [ ] `qt-cpp-review` run on the C++ diff; findings addressed or noted.
- [ ] Maintainer launch smoke in ProjectFile, QuickPlot, and ConsoleOnly: startup, project
      restore, backup snapshot on edit, `.proto` import, and (commercial build) `.dbc` import
      behave as before.
- [ ] GPL build (no `BUILD_COMMERCIAL`) compiles with pilots 1 and 2 only; `SessionContext`
      names no commercial type. *(Second half verified by grep; the build is the maintainer's.
      Blocked until T2 registers the two sources.)*
- [ ] `python scripts/documentation-verify.py` clean; `python scripts/sanitize-commit.py` run;
      working tree clean of lint debt. *(Blocked on T12.)*
- [x] Diff is *what was asked, and only that* — three pilots, no fourth conversion, no foreign
      files touched.
- [ ] `spec.md` status set to `done`, with the M1 milestone named and M2/M3 recorded as open.
      *(Stays `in-progress`: T2, T12, and AC3 are open.)*
