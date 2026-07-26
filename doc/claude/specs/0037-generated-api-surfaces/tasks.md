---
spec: 0037-generated-api-surfaces
phase: tasks
status: draft        # draft -> approved (gate before /ss-implement)
updated: 2026-07-25
---

# Tasks 0037 — Generated API surfaces from one source of truth

> **Phase 3 of 4 — the ordered checklist.** Decompose [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable*. `/ss-implement` works this list top to
> bottom and keeps the status boxes current. Gate: do not start `/ss-implement` until a human
> marks this `approved`.

## Conventions

- One task = one focused, reviewable change, sized for a single agent.
- **Verify** is how *this* unit is confirmed before moving on.
- **Deps** lists task IDs that must land first.
- **Spec 0036 must be implemented first.** Every task below reads
  `app/rcc/properties/dataset.json` and/or `scripts/generate-property-registry.py`, which do
  not exist until 0036 lands.
- v1 is the **dataset entity only**, matching 0036 v1. The other three prose-schema verbs
  (`project.group.update`, `project.action.update`, `project.outputWidget.update`) follow when
  their manifests exist.
- **T4 must land before the dataset schema grows on any released build.** The ledger is what
  makes the numbering append-only; growing the schema first is the wire break the spec exists
  to prevent.

## Tasks

### T1 — Extract the shared schema projection

- **Files:** `scripts/generate-property-registry.py`
- **Does:** Factor the property-to-`SchemaProp` mapping that 0036's C++ emitter uses into one
  function, `schema_props_for(entry)`, returning the ordered list of property descriptors
  (name, type, description, enum domain, minimum/maximum, default, binary flag, required-ness)
  for a manifest entry, plus the alias entries required by 0036's R12. The C++ emitter is
  rewritten to render this function's output instead of reading the manifest fields directly.
  No behavior change: the emitted `DatasetApiFields.cpp` must be byte-identical before and
  after. **Binding invariant: this function becomes the single definition of "what schema does
  this property produce"; nothing else in the repo may re-derive it, or the drift class this
  spec removes comes straight back through the projector.**
- **Verify:** run the 0036 generator before and after and diff the emitted
  `app/src/API/Generated/DatasetApiFields.cpp` — zero change; `python scripts/code-verify.py
  --check` on the regenerated file; `python -m py_compile` on the generator.
- **Deps:** none (0036 landed)
- [x] done

### T2 — Snapshot projector and its drift check

- **Files:** `scripts/generate-property-registry.py`
- **Does:** A `--check-snapshot` arm that lowers `schema_props_for()` output through the same
  rules `API::schemaPropToJson` applies (`app/src/API/SchemaBuilder.h:140-182` — union-type
  expansion on `'|'`, the `binary` flag, `enum`/`minimum`/`maximum`/`default` emission, `items`
  for arrays), flattens it the way `CLI::dumpApiSchema` does (`app/src/Misc/CLI.cpp:322-353`
  drops the `type: object` wrapper and lifts `properties`/`required` to the entry top level),
  and compares it against the `project.dataset.*` entries in the committed
  `app/rcc/api/api-schema.json`. On mismatch: exit 1, naming the command, each differing field,
  and the ordered fix (build -> `SerialStudio --dump-api-schema app/rcc/api/api-schema.json` ->
  `python scripts/sanitize-commit.py`), plus an explicit line saying only registry-derived
  verbs are covered. A missing snapshot or manifest reports why it could not run rather than
  passing. **Binding invariant: this check must run with no compiler, no Qt, and no running
  app — that is the entire point.**
- **Verify:** clean against the current tree; seed a manifest property without refreshing the
  snapshot and confirm the message and exit 1; delete the snapshot and confirm the
  cannot-run message; revert both (AC2, AC13).
- **Deps:** T1
- [x] done

### T3 — `generate-sdk.py --check`

- **Files:** `scripts/generate-sdk.py`
- **Does:** Add `argparse` (the script has none today; `main()` at `:423-450` always writes)
  and a `--check` mode that renders `SerialStudio.js`, `SerialStudio.lua`, and
  `sdk-symbols.json` in memory and byte-compares them against the committed files, exiting 1
  on drift with a message naming which artifact is stale. Also emit each enum-valued
  property's domain as a doc comment on the generated options bag, so an SDK reader sees the
  legal values without opening the schema. **Binding invariants: the default no-argument
  invocation must keep writing exactly as before (the commit pipeline calls it with no args,
  `sanitize-commit.py:180-183`); LF-only output via the existing `write_bytes` path;
  determinism via the existing `commands.sort(key=...)`.**
- **Verify:** `--check` clean on the current tree; hand-edit `SerialStudio.js` and confirm
  exit 1, then regenerate; run with no args twice and confirm no diff; confirm
  `scripts/code-verify.py --check` still reports no `sdk-out-of-date`.
- **Deps:** none
- [x] done

### T4 — Proto field-number ledger

- **Files:** `scripts/generate-property-registry.py`, `app/rcc/api/proto-fields.json` (new,
  generated), `app/rcc/rcc.qrc`
- **Does:** An emitter that reads the committed `app/rcc/api/api-schema.json` and produces the
  ledger: per command, `{fields: {param: number}, reserved: [numbers], next: n}`. Seeded once
  from today's numbering so no currently-exported number changes. Assignment rules: `1` is
  reserved in every message for the existing `string id`; a parameter already present keeps
  its number forever; a new parameter takes `next` and increments, processed in sorted-name
  order; a parameter absent from the snapshot moves to `reserved`; `next` never decreases; a
  command absent from the snapshot is retained, never pruned (a GPL-dumped snapshot must not
  delete commercial entries). Deterministic key ordering, LF, do-not-edit marker, `--check`
  byte-compare. Add the qrc entry next to the existing API resources
  (`app/rcc/rcc.qrc:152-157`) — the runtime reads this file. **Binding invariant: numbering is
  append-only released state; an emitter run that would move an existing number is a bug, not
  a diff.**
- **Verify:** generate twice, no diff; `--check` clean, then edit the manifest/snapshot and
  confirm exit 1; hand-verify a sample of commands' numbers against what `ProtoGenerator`
  produces today (alphabetical from 2) so the seed is a no-op for existing clients.
- **Deps:** none
- [x] done

### T5 — Ledger static test

- **Files:** `tests/scripts/test_proto_ledger_static.py` (new), `tests/README.md`
- **Does:** Pure-Python assertions over the committed ledger, no Qt and no running app: numbers
  are unique within a command; `fields` and `reserved` never intersect; `1` is never assigned;
  `next` exceeds every assigned and reserved number; every command in the committed snapshot
  has a ledger entry; a simulated insertion of an alphabetically-early parameter changes no
  existing number; a simulated removal moves the number to `reserved` and never reassigns it.
  Follow the conventions in `tests/README.md` — read it before writing — and add the catalog
  row. (AC6, AC7)
- **Verify:** `pytest tests/scripts/test_proto_ledger_static.py -v` green locally.
- **Deps:** T4
- [x] done

### T6 — Typed proto reference copy

- **Files:** `scripts/generate-property-registry.py`, `doc/grpc/serialstudio-typed.proto`
  (new, generated)
- **Does:** Emit the client-facing typed proto from the committed snapshot plus the ledger:
  the shared messages `ProtoGenerator::writeSharedMessages` defines, one `<Command>Request`
  message per command with `string id = 1` and ledger-numbered parameters, proto `reserved`
  statements for retired numbers, the same JSON-to-proto type mapping
  (`ProtoGenerator.cpp:194-212`), enum domains as trailing comments, and the typed service
  block. Do-not-edit marker, LF, deterministic ordering, `--check` byte-compare. **Binding
  invariants: `doc/grpc/serialstudio.proto` — the dynamic service protoc compiles at
  `app/CMakeLists.txt:943` — is not touched; the new file is not added to `rcc.qrc` and not
  added to any build rule; package name and message shapes match what the runtime generator
  emits so the two can be compared byte for byte.**
- **Verify:** generate twice, no diff; `--check` drift detection; visually diff against the
  runtime generator's emission logic function by function; maintainer runs `protoc` over the
  file (AC8).
- **Deps:** T4
- [x] done

### T7 — `ProtoGenerator` reads the ledger

- **Files:** `app/src/API/GRPC/ProtoGenerator.cpp`, `app/src/API/GRPC/ProtoGenerator.h`
- **Does:** Replace the positional numbering in `buildCommandMessages` (`:123-134`, `int
  field_num = 2` incremented over `QJsonObject` iteration) with a lookup into
  `:/api/proto-fields.json`, loaded once and cached. A command or parameter missing from the
  ledger falls back to "append after this message's current maximum, in sorted-name order" so
  new fields get fresh numbers and existing ones never move. Emit `reserved` statements for
  retired numbers and enum domains as trailing comments, matching T6's output exactly. **Binding
  invariants: `[[nodiscard]]` on every non-void return, header member order and Christmas-tree
  per style, no in-header member init, `Q_EMIT` not `emit`, no allocation concern (this runs
  on export, not per frame) — and the emitted text must be byte-identical to
  `doc/grpc/serialstudio-typed.proto` on a full commercial build.**
- **Verify:** `python scripts/code-verify.py --check` on both files; `qt-cpp-review` on the
  diff (this is the only C++ in the spec); read-back that the fallback path can never move an
  existing number; maintainer compares the runtime export against the checked-in file (AC8).
- **Deps:** T4, T6
- [x] done

### T8 — `code-verify.py`: generated-artifact rules

- **Files:** `scripts/code-verify.py`
- **Does:** Extend the SDK guard block (`_SDK_GENERATED` / `_sdk_consistency_violations`,
  `:2735-2800`) to cover `app/rcc/api/proto-fields.json` and
  `doc/grpc/serialstudio-typed.proto`: violation `api-generated-edited` when an artifact loses
  its do-not-edit marker, and `proto-field-renumbered` when the committed ledger assigns a
  number that a regeneration would not — the guard that catches a hand-reset or hand-edited
  ledger, which is the one way a released number could move. Follow the existing
  best-effort/never-crash convention: if the ledger or generator cannot be loaded, print why
  the check was skipped rather than failing the commit.
- **Verify:** `python scripts/code-verify.py --check` clean over the repo; seed each violation
  (delete a marker, hand-edit a number) and confirm the message; revert.
- **Deps:** T4, T6
- [x] done

### T9 — `registry-verify.py`: corpus field-reference lint

- **Files:** `scripts/registry-verify.py`
- **Does:** Two rule functions in the existing `check_*(errors: list[str])` shape plus two call
  lines in `main()` (`:381-386`): `check_api_snapshot(errors)`, which shells the T2 projection
  check and folds its findings into the shared `errors` list; and
  `check_corpus_field_refs(errors)`, which scans `app/rcc/ai/skills/*.md` for dataset field
  references (fenced tables and inline-code spans) and for stated enum values, and fails when a
  name resolves to nothing in the manifest's `apiName`/`apiAliases`/`jsonKey` namespace or when
  a stated enum value disagrees with the manifest's option source. Intentional exceptions get a
  whitelist entry with a one-line reason each. **Binding invariants: rules take `errors` first,
  append via `fail()`, never raise, never return early on a violation, and never rewrite a
  file — `registry-verify.py` is read-only. Nothing here regenerates
  `app/rcc/ai/search_index.json`.**
- **Verify:** `python scripts/registry-verify.py` — expect real findings from the existing
  corpus drift (that is T10's input); seed a bogus field name and a wrong enum value, confirm
  the messages, revert.
- **Deps:** T2
- [x] done

### T10 — Reconcile the assistant corpus

- **Files:** `app/rcc/ai/skills/api_semantics.md`,
  `app/rcc/ai/skills/project_basics.md`, `app/rcc/ai/skills/dashboard_layout.md`
- **Does:** Fix the disagreements T9 surfaces, against the manifest as ground truth: the three
  independent widget-option bitflag tables (`api_semantics.md:230-236` stops at
  `64 = Waterfall`; `project_basics.md:207` adds `128 = Meter`; `dashboard_layout.md:49, 108,
  175` restates it a third time) and the duplicated short/long range-field mapping
  (`api_semantics.md:255-257`, `project_basics.md:288-289`). Where a fact is stated three
  times, prefer one authoritative statement plus cross-references over three copies — but keep
  the teaching prose, the worked examples, and each file's voice. **Binding invariant: this is
  a correctness pass on hand-written docs, not a rewrite; do not restructure the skills and do
  not touch any file outside the three named.**
- **Verify:** `python scripts/registry-verify.py` clean; read-back that every corrected value
  traces to a manifest entry; `python scripts/documentation-verify.py` unaffected (these files
  are outside its target set, so confirm no new findings rather than assuming).
- **Deps:** T9
- [x] done

### T11 — Commit-pipeline wiring

- **Files:** `scripts/sanitize-commit.py`
- **Does:** New `run_python_step` blocks after the SDK step (`:180-183`): regenerate the ledger
  and the typed proto, then run the snapshot projection check. Update the header comment block
  (`:5-16`), which currently documents the pipeline accurately and must stay so. **Binding
  invariants: the pipeline stays sanitize-only and non-fatal — a failing step prints and
  continues, per `run_python_step` at `:132-147`; new steps sit after both `clang-format`
  passes and after `black`, so they must emit already-formatted output; a second consecutive
  run must leave a clean tree.**
- **Verify:** run `python scripts/sanitize-commit.py` twice; the second run reports no changes
  (AC14, AC3); confirm the header comment matches the new order.
- **Deps:** T2, T4, T6
- [x] done

### T12 — Integration tests

- **Files:** `tests/integration/test_api_surfaces.py` (new), `tests/README.md`
- **Does:** Maintainer-run coverage against a live app on 7777: **AC9** — the MCP `tools/list`
  reply lists every declared dataset field on the dataset update tool as a typed property with
  a description, with enum domains where declared; **AC10** — record the `tools/list` payload
  size so the before/after delta can be reported; **AC11** — the SDK's dataset update wrapper
  sets every declared field and reads each back; **AC8** — the runtime `.proto` export is
  byte-identical to `doc/grpc/serialstudio-typed.proto`. Follow `tests/utils/api_client.py`
  conventions and the markers in `tests/README.md` — read it before writing — and add the
  catalog row. Note in the module docstring that AC8 needs a commercial build with
  `ENABLE_GRPC=ON`.
- **Verify:** `python -m py_compile`; `pytest --collect-only` locally; execution is
  maintainer-run.
- **Deps:** T6, T7
- [x] done

### T13 — CI lint job

- **Files:** `.github/workflows/ci.yml`
- **Does:** Add one "Verify generated surfaces" step to the `lint` job (`:1848-1874`, which
  today runs only `code-verify.py --check`) invoking `registry-verify.py`,
  `generate-command-strings.py --check`, `generate-property-registry.py --check`, and
  `generate-sdk.py --check`. **Binding invariants: `ci.yml` is a contended file this campaign —
  workflow edits are serialized to closeout, so this task lands last and only with the
  coordinator's go-ahead. Do not restructure the job, do not touch any build job, do not change
  `continue-on-error` anywhere. `generate-command-strings.py --check` has zero callers in the
  repo today; wiring it is part of the task, and it may surface pre-existing drift that must be
  reported rather than silently regenerated.**
- **Verify:** read-back of the workflow diff; confirm each invoked check exits 0 on a clean
  tree locally; confirm a deliberately drifted local branch would fail each of them (AC5).
- **Deps:** T2, T3, T4, T6, T9
- [x] done

### T14 — Docs, measurement, self-review

- **Files:** `doc/help/gRPC-Server.md`, `doc/claude/architecture/project.md`, `CLAUDE.md`,
  `doc/claude/specs/0037-generated-api-surfaces/tasks.md`
- **Does:** A paragraph in the gRPC help page naming `doc/grpc/serialstudio-typed.proto` and
  how to codegen from it (run `ss-docs` for that file — it is user-facing and
  `documentation-verify.py` covers `doc/help/**`). A "Generated API surfaces" section in the
  architecture doc: what derives from the 0036 manifest, what is *checked* versus *generated*,
  the append-only numbering rule, and which gate fires when. A short `CLAUDE.md` addition
  extending 0036's registry block with the downstream surfaces and the gate list. Record the
  AC10 payload measurement and the AC1 demonstration result in this file's build notes.
  Counterfactual check at handoff: name the rule this diff most risks violating and the
  concrete evidence it does not; re-read the full diff for scope.
- **Verify:** `python scripts/documentation-verify.py` clean on the help page
  (`doc/claude` is exempt); confirm every factual claim traces to a file read this session.
- **Deps:** all
- [x] done

## Build notes

Implemented 2026-07-25. Deviations and findings worth a reviewer's attention:

- **`sanitize-commit.py` runs the generator, then `--check`, then `--check-snapshot`.** The
  plan said "regenerate the ledger + typed proto"; the generator emits all six artifacts in
  one pass, so the pipeline regenerates and then re-checks rather than gaining a
  ledger-only mode. The append-only invariant plus `proto-field-renumbered` means a
  regeneration can never move a published number, so the silent-rewrite risk is covered.
- **Local-vs-CI split is carried by the `CI` environment variable.** `--check-snapshot`
  warns and exits 0 locally, and fails hard when `--strict` is passed or `CI` is set — the
  coordinator ruling, encoded in one place and restated in the failure message. The CI step
  passes `--strict` explicitly as well.
- **The typed proto was not protoc-valid before this change.** Two pre-existing defects had
  to be fixed for AC8: six commands declare a parameter literally named `id`, which
  collided with the fixed `string id = 1` (now emitted as `id_param` with a `// JSON name:
  id` comment, ledger still keyed by the JSON name); and ten command descriptions contain
  newlines, which produced comment continuation lines with no `//` prefix. `protoc
  --proto_path=doc/grpc -o /dev/null doc/grpc/serialstudio-typed.proto` now passes locally.
- **The ledger was seeded from the committed (pre-0036) snapshot**, which is what preserves
  released numbering: `project.dataset.update` keeps `datasetId = 2`, `groupId = 3`. When
  the maintainer re-dumps the snapshot, the 37 dataset fields append from 4 upward — none of
  them displaces a number a client has seen. 347 commands, 354 messages in the proto.
- **Corpus reconciliation went past the bitflag table.** `api_semantics.md` was missing
  `128 = Meter`, and all three files stated that `project.dataset.update` rejects
  `plotMin`/`widgetMin`. The manifest declares those as accepted aliases, so the corpus was
  teaching the assistant to avoid a working spelling and to expect warnings that never
  arrive. Nine paragraphs corrected across the three named files; no other file touched, and
  `search_index.json` was not regenerated.

- **Review finding fixed.** `ProtoGenerator::jsonScalar` rendered a string enum value as
  `"%1"` with no escaping while the Python side used `json.dumps`, so a vocabulary containing
  a quote or backslash would have produced both invalid proto and a byte divergence between
  the two emitters. Both now share one escaper (`proto_scalar()` / `jsonScalar`): backslash
  then quote. No current enum value contains either, so the regenerated proto is unchanged --
  the fix is for the next vocabulary, not this one.

### Maintainer-run items (not verifiable here)

- **AC1** — add a manifest property, refresh the snapshot, confirm it reaches all four
  surfaces with no hand edit.
- **AC2 (CI half)** — confirmed locally by seeding; the CI failure path was exercised with
  `CI=true`, which exits 1 with the ordered fix.
- **AC8** — export the typed proto from a commercial `ENABLE_GRPC=ON` build and run
  `SS_EXPORTED_PROTO=<path> pytest tests/integration/test_api_surfaces.py -k proto`. Note
  the export is a GUI action (Settings → gRPC), not an API command.
- **AC9/AC10/AC11** — `pytest tests/integration/test_api_surfaces.py` against a running app.
  AC10's payload number is printed by the test; it is **not yet recorded**, because the
  committed snapshot still predates the schema growth, so a before/after delta measured
  today would compare a build against itself.
- **AC13** — GPL-build check that no Pro-only dataset property reaches any surface and that
  the ledger is identical between a GPL and a commercial dump.
- **`--benchmark-hotpath`** adjacency gate.

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` is met or handed to the maintainer as a named
      runtime check (AC1/AC8/AC13 maintainer; AC9/AC10/AC11 pytest; the rest local).
- [x] `python scripts/code-verify.py --check` clean on all changed files, including the new
      `api-generated-edited` and `proto-field-renumbered` rules (no new errors).
- [x] `python scripts/registry-verify.py` clean, including both new rule functions.
- [x] `python scripts/generate-property-registry.py --check`,
      `python scripts/generate-sdk.py --check`, and
      `python scripts/generate-command-strings.py --check` all clean; every generator run twice
      produces no diff.
- [x] `pytest tests/scripts/test_proto_ledger_static.py` green (runs without the app).
- [x] `qt-cpp-review` run on the `ProtoGenerator` diff — the only C++ in this spec; findings
      addressed or noted.
- [ ] `--benchmark-hotpath` run by the maintainer as an adjacency gate (no frame-path edit
      expected; the gRPC server is a benchmarked export sink).
- [x] `pytest tests/integration/test_api_surfaces.py` listed for the maintainer, with the
      commercial + `ENABLE_GRPC=ON` build requirement called out for the proto parity case.
- [x] No gRPC field number that exists on the pre-change build has a different meaning after.
- [ ] `python scripts/sanitize-commit.py` run twice; the second run leaves a clean tree.
- [x] Diff is *what was asked, and only that* — dataset entity only, no QML change, no change
      to the compiled `doc/grpc/serialstudio.proto`, no foreign working-tree files touched.
- [ ] `spec.md` status set to `done`.
