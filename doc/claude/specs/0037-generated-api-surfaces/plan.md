---
spec: 0037-generated-api-surfaces
phase: plan
status: draft        # draft -> approved (gate before /ss-tasks)
updated: 2026-07-25
---

# Plan 0037 — Generated API surfaces from one source of truth

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Gate: do not start `/ss-tasks` until a human marks this `approved`.
>
> Reads spec 0036's [`plan.md`](../0036-property-registry/plan.md) as its foundation: the
> dataset manifest at `app/rcc/properties/dataset.json`, the generator at
> `scripts/generate-property-registry.py`, and the generated typed schema block emitted into
> `app/src/API/Generated/DatasetApiFields.cpp`.

## Approach (one paragraph)

`API::CommandDefinition::inputSchema` is already the single runtime hub: MCP `tools/list`
copies it verbatim, `--dump-api-schema` flattens it into `app/rcc/api/api-schema.json`,
`generate-sdk.py` turns that snapshot into JS/Lua wrappers, and
`API::GRPC::ProtoGenerator` walks it to emit per-command proto messages. Spec 0036 makes the
dataset half of that hub derive from a manifest. This spec therefore adds **no new schema
emitter** — it adds **projection, numbering, and gates**. Three new emitters land inside the
existing `scripts/generate-property-registry.py`: a *snapshot projector* that computes, in
Python, the exact `properties`/`required` block 0036's generated C++ registers for the
dataset verbs and byte-compares it against the committed snapshot (closing the
"cannot see C++ changes without a build" hole the SDK staleness rule documents in
`scripts/code-verify.py:2802-2812`); a *proto field-number ledger* at
`app/rcc/api/proto-fields.json` that makes gRPC numbering append-only, replacing
`ProtoGenerator.cpp:123-134`'s alphabetical-iteration numbering that silently renumbers every
field after an inserted property; and a *typed proto reference copy* at
`doc/grpc/serialstudio-typed.proto` so clients codegen from the repository. Around them:
`generate-sdk.py` gains the `--check` mode it never had, `registry-verify.py` gains an
assistant-corpus reference lint (three bundled skill files restate the dataset bitflag table
and already disagree on the highest bit), `code-verify.py`'s generated-artifact banner rule
extends to the new files, and the CI lint job — which today runs exactly one command — starts
invoking every drift gate the repo owns, including `generate-command-strings.py --check`,
which currently has no caller anywhere. Two rejected shapes: a separate
`generate-api-surfaces.py` (would re-implement the manifest loader and could disagree with
0036's own reading of it), and having the runtime `ProtoGenerator` serve a bundled proto
instead of generating one (would make a GPL build export a proto listing commercial RPCs it
cannot serve).

## Affected subsystems & files

| File | Change |
|------|--------|
| `scripts/generate-property-registry.py` (from 0036) | Three new emitters + their `--check` arms, sharing 0036's manifest loader, ordering, marker, and byte-compare machinery. Key refactor: the property-to-`SchemaProp` mapping 0036 uses to emit C++ is factored into one function that the snapshot projector also calls, so the two can never disagree. New subcommands/flags for the proto ledger and the typed proto. |
| `app/rcc/api/proto-fields.json` (new, generated, checked in, added to `rcc.qrc`) | The gRPC numbering ledger: per command, `{fields: {param: number}, reserved: [numbers], next: n}`. Numbers are append-only; a removed parameter's number moves to `reserved` and is never reused. Number `1` is reserved everywhere for the existing `string id` field. |
| `doc/grpc/serialstudio-typed.proto` (new, generated, checked in) | The client-facing typed proto — one `<Command>Request` message per registered command, numbered from the ledger. Reference copy for `protoc`; **not** bundled and **not** compiled by the build. `doc/grpc/serialstudio.proto` (the dynamic service protoc actually compiles, `app/CMakeLists.txt:943`) is untouched. |
| `app/src/API/GRPC/ProtoGenerator.cpp` | `buildCommandMessages` stops deriving field numbers from `QJsonObject` iteration order and reads them from the bundled ledger; an unknown command or parameter (an extension-registered command, a build the ledger predates) falls back to deterministic append-after-max rather than renumbering. Emits `enum` domains as trailing comments and preserves today's type mapping. |
| `app/src/API/GRPC/ProtoGenerator.h` | One private helper declaration for the ledger lookup; no public signature change. |
| `scripts/generate-sdk.py` | Add `argparse` and a `--check` mode that renders in memory and byte-compares `SerialStudio.js` / `SerialStudio.lua` / `sdk-symbols.json`, exiting 1 on drift (the script has no argument parsing today and always writes, `generate-sdk.py:423-450`). Emit each enum-valued property's domain as a doc comment on the options bag. |
| `scripts/registry-verify.py` | Two new rule functions in the existing `check_*(errors)` shape plus two call lines in `main()` (`:381-386`): `check_api_snapshot(errors)` (dataset verbs' projected schema equals the committed snapshot) and `check_corpus_field_refs(errors)` (every dataset field name and declared enum value stated in `app/rcc/ai/skills/*.md` exists in the manifest with that meaning). |
| `scripts/code-verify.py` | Extend `_SDK_GENERATED` / `_sdk_consistency_violations` (`:2735-2800`) to cover `proto-fields.json` and `serialstudio-typed.proto`: banner present, artifact not hand-edited. New violation ids `api-generated-edited`, `proto-field-renumbered`. |
| `scripts/sanitize-commit.py` | New pipeline steps after the SDK step (`:180-183`): regenerate the ledger + typed proto, then the snapshot projection check. Header comment block (`:5-16`) updated to match — it is currently accurate and must stay so. |
| `.github/workflows/ci.yml` | The `lint` job (`:1848-1874`) gains one "Verify generated surfaces" step running `registry-verify.py`, `generate-command-strings.py --check`, `generate-property-registry.py --check`, and `generate-sdk.py --check`. **Contended file** — sequenced to campaign closeout. |
| `app/rcc/rcc.qrc` | One `<file>api/proto-fields.json</file>` entry alongside the existing API resources (`:152-157`). |
| `app/rcc/ai/skills/api_semantics.md`, `project_basics.md`, `dashboard_layout.md` | Reconcile the three independent widget-option bitflag tables (`api_semantics.md:230-236` stops at `64 = Waterfall`; `project_basics.md:207` adds `128 = Meter`) and the duplicated short/long range-field mapping (`api_semantics.md:255-257`, `project_basics.md:288-289`) against the manifest. Prose stays hand-written; only the facts are corrected. |
| `tests/integration/test_api_surfaces.py` (new) | AC9 (MCP typed properties + enum domains), AC8 (runtime proto export equals the checked-in copy), AC11 (SDK setters), AC10 (payload measurement). |
| `tests/scripts/test_proto_ledger_static.py` (new) | AC6/AC7 — pure-Python, no app: numbers append-only, reserved never reused, ledger consistent with the committed proto. Runs in the pytest tier that needs no binary. |
| `tests/README.md` | Catalog rows for both new test files. |
| `doc/help/gRPC-Server.md` | One paragraph naming the checked-in typed proto and how to codegen from it. |
| `doc/claude/architecture/commands-icons.md` or `doc/claude/architecture/project.md` | Short "Generated API surfaces" section: what derives from the manifest, what is checked versus generated, which gate fires when. |
| `CLAUDE.md` | Extend 0036's registry block with the downstream surfaces and the gate list. |

## Architecture & data flow

**The hub and its five consumers (today).** Every handler registers
`{name, description, inputSchema, handler}` (`app/src/API/CommandRegistry.h:41-46` — four
fields, no category, no exposure flag). From that one `inputSchema`:

```
CommandDefinition::inputSchema
  |-- MCPHandler::generateToolsFromRegistry()      -> tools/list, verbatim, no filtering
  |-- CLI::dumpApiSchema()                         -> app/rcc/api/api-schema.json (flattened)
  |     `-- scripts/generate-sdk.py                -> SerialStudio.js/.lua, sdk-symbols.json
  |-- GRPC::ProtoGenerator::buildCommandMessages() -> typed .proto (runtime export only)
  `-- AI::ToolDispatcher::describeCommand()        -> meta.describeCommand
```

Spec 0036 makes the dataset verbs' `inputSchema` derive from `app/rcc/properties/dataset.json`
via generated C++. This spec adds three Python paths that read the same manifest and the same
snapshot, so the chain becomes checkable:

```
app/rcc/properties/dataset.json  (0036, single declaration)
  |
  |-- [0036] generated C++ SchemaProp table -> inputSchema at runtime
  |
  `-- [0037] schema_props_for(entry)  <-- ONE shared function, called by both
        |-- 0036's C++ emitter
        `-- snapshot projector -> compare against api-schema.json's project.dataset.* entries

app/rcc/api/api-schema.json (maintainer-dumped, all 347 commands)
  |-- generate-sdk.py            -> SDK  (+ new --check)
  `-- [0037] proto emitters
        |-- proto-fields.json    (ledger; append-only numbering)
        `-- serialstudio-typed.proto (reference copy for clients)
              ^
              `-- runtime ProtoGenerator reads the SAME ledger from qrc
```

**Why the snapshot is checked, not generated.** `api-schema.json` is a dump of all 347
commands, ~300 of which register hand-written schemas in C++. Generating it in Python would
mean re-implementing every handler's schema — the exact duplication this roadmap item removes.
Instead the projector covers only the registry-derived verbs and states that scope in its
failure message. This is the piece that closes the documented hole: today the SDK staleness
rule "cannot see commands added in C++ until `--dump-api-schema` refreshes api-schema.json"
(`scripts/code-verify.py:2808-2812`); after this change, a manifest property that never
reached the snapshot fails in CI with no build involved.

**The shared-projection invariant.** The projector and 0036's C++ emitter must agree
*exactly* — same property names, same alias handling, same type strings, same required list,
same enum domains, same description text. Two implementations of one mapping is the drift
class this spec exists to kill, so both call one `schema_props_for(entry)` function in
`generate-property-registry.py`. The C++ emitter renders its return value as
`API::SchemaProp` initializers; the projector renders it through the same lowering
`API::schemaPropToJson` performs (`app/src/API/SchemaBuilder.h:140-182`), including the
`'|'`-separated union-type expansion and the `binary` flag.

**Proto numbering.** `ProtoGenerator::buildCommandMessages` currently does:

```cpp
const auto props = def.inputSchema.value("properties").toObject();
int field_num    = 2;
for (auto pit = props.begin(); pit != props.end(); ++pit)
  msg_out << "  " << field_type << " " << field_name << " = " << field_num++ << ";\n";
```

`QJsonObject` iterates by key, so numbering is alphabetical and positional. With two
properties nothing has ever moved; with ~45 it renumbers on every insertion. The ledger
inverts the relationship: numbers are data, not a side effect of iteration order. Assignment
rule, applied only by the Python emitter:

- number `1` is reserved in every message for the existing `string id`;
- a parameter already in the ledger keeps its number, forever;
- a new parameter takes `next` and increments it, processed in sorted-name order for
  determinism;
- a parameter that disappears from the snapshot moves its number into `reserved` and the
  emitter writes a proto `reserved` statement, so protoc itself enforces non-reuse;
- `next` never decreases.

The runtime generator reads `:/api/proto-fields.json` and looks numbers up. Unknown command
or unknown parameter (an extension-registered command, or a binary newer than its bundled
ledger) falls back to "append after the message's current maximum, sorted by name" — new
fields get fresh numbers, existing ones never move.

**Corpus reference lint.** `check_corpus_field_refs` extracts identifier-shaped tokens from
the fenced tables and inline-code spans of `app/rcc/ai/skills/*.md`, keeps those that look
like dataset field references (present in the manifest's `apiName`/`apiAliases`/`jsonKey`
namespace, or matching a known-but-undeclared pattern), and fails on a name that resolves to
nothing. Declared enum domains stated in the corpus (`widget` values, `displayFormat` values,
the widget-option bit values) are compared against the manifest's option sources. This is a
*reference* check: it never rewrites prose and it never touches
`app/rcc/ai/search_index.json`, whose 1.3 MB BM25 diffs are already noisy.

## Hotpath & threading impact

- **Touches the hotpath?** **No.** `FrameReader`, `CircularBuffer`, `FrameBuilder`, the span
  fast lane, and the Dashboard draw path are untouched. The one C++ file that changes,
  `ProtoGenerator.cpp`, runs only when a user exports a `.proto` from Preferences — it is not
  on any frame path. One adjacency to flag: the gRPC server is on the export fan-out
  (`app/src/IO/ConnectionManager.cpp:576, 605`) and is benchmarked as a live sink
  (`app/src/Benchmark/HotpathBenchmark.cpp:447-470`), so `--benchmark-hotpath` is run as a
  regression gate even though nothing on that path is edited. The ledger is parsed once, on
  first export, from a Qt resource — never per frame, never per RPC.
- **New cross-thread signal/slot?** No. No new signals, no new connections, no new threads.
  The ledger read happens on whichever thread invokes the export, exactly as the current
  generation does.
- **New input to a cached hotpath flag?** No. No flags are added, read, or invalidated;
  `m_anyAsyncSink` and friends see no new inputs.
- **Timestamp ownership** — untouched. No frame data, no driver boundary, no export worker is
  involved.

## Data model & persistence

- **No project-file change.** No new `Keys::` entries, no schema-version bump, no migration,
  no change to what a `.ssproj` contains. This spec touches API-surface artifacts only.
- **Two new checked-in artifacts.** `app/rcc/api/proto-fields.json` (bundled; the runtime
  reads it) and `doc/grpc/serialstudio-typed.proto` (not bundled; a reference for `protoc`).
  Both are generated, both carry a do-not-edit marker, both are byte-reproducible.
- **The ledger is append-only and must be treated as released state.** Once a number ships in
  a release, changing it changes the wire meaning of a field. `code-verify.py` gains
  `proto-field-renumbered` specifically so a regenerated ledger that *moves* an existing
  number — which can only happen if someone edited it by hand or reset it — is an error, not a
  quiet diff.
- **Snapshot provenance.** `api-schema.json` reflects the build it was dumped from. A GPL dump
  omits the 12 commercial handler namespaces (`app/src/API/CommandHandler.cpp:256-269`).
  Because the ledger is append-only and keyed by command name, a GPL-dumped snapshot must
  never *drop* ledger entries — the emitter only adds and reserves, never prunes an absent
  command. The projector's failure message states which build a refresh must come from.

## API / SDK surface

- **No new commands, no renames, no parameter changes.** Every existing command keeps its
  name, its parameters, and its behavior.
- **MCP** — no protocol change, no new methods, no exposure flag. Dataset tools' `inputSchema`
  gains typed properties as a consequence of 0036; this spec adds the assertion that they are
  present with descriptions and enum domains, and the measurement of the `tools/list` payload
  delta across 347 tools (`MCPHandler.cpp:631-654` has no filtering, so every command is a
  tool and payload growth is global, not local).
- **SDK** — `generate-sdk.py` already emits an options bag for optional properties
  (`ordered_params`, `:61-66`), so the dataset update wrapper gains its setters with no
  generator change once the schema declares them; today it emits
  `project.dataset.update = function(groupId, datasetId)` with no way to pass a field. What
  this spec adds is `--check` (the script has no `argparse` at all today) and enum-domain doc
  comments. `sdk-symbols.json` follows automatically.
- **gRPC** — the compiled dynamic service (`doc/grpc/serialstudio.proto`, port 8888) is
  unchanged. The typed proto gains stable numbering and a checked-in reference copy.
  `ENABLE_GRPC` defaults `OFF` (`CMakeLists.txt:85`) but every official CI binary is built
  with it `ON` (nine configure steps), so this is shipped surface, not optional surface.
- **Commercial gating** — unchanged. Pro-only properties keep the `#ifdef BUILD_COMMERCIAL`
  guards 0036 emits; the ledger never renumbers because a commercial command is absent.

## QML / UI

**No QML change.** No new components, no new models, no dialog changes. The Preferences
"Export .proto" action keeps its current wiring and behavior; only what the generator writes
changes.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Where the emitters live | new `generate-api-surfaces.py`; extend `generate-property-registry.py` | **Extend 0036's generator** — one manifest loader, one determinism/marker/`--check` machinery, and crucially one `schema_props_for()` shared by the C++ emitter and the snapshot projector. A second script would re-implement the manifest reading and could disagree with 0036 about what the manifest says, recreating the drift class. |
| The API snapshot | generate it in Python; check it against the manifest; leave it ungated | **Check it** — it covers 347 commands, ~300 with hand-written C++ schemas, and only a build can produce it. Generating it means re-implementing every handler's schema. Checking the registry-derived slice is the whole win (buildless verification) at a fraction of the cost. |
| Proto numbering | keep alphabetical iteration; declare numbers in the manifest; a generated ledger | **Generated ledger** — manifest-declared numbers would put a wire-format concern into the property declaration and make every new property a numbering decision for the author; alphabetical iteration is the bug. A ledger is derived, append-only, reviewable as a diff, and enforceable by `protoc` through `reserved`. |
| Runtime proto generation | serve a bundled generated proto; keep generating, read numbers from the ledger; delete the runtime export | **Keep generating, read the ledger** — serving a bundled artifact would make a GPL build export a proto listing the 12 commercial namespaces it cannot serve, and would make an extension-registered command invisible. Reading the ledger fixes the actual defect (numbering) while keeping the export build-accurate. Cost: two emitters of *shape*, mitigated by an equality test against the checked-in copy on a full build. |
| Typed proto location | bundle it in `rcc.qrc`; check it into `doc/grpc/`; both | **`doc/grpc/` only** — it is a client-facing codegen input, not something the app reads; bundling it would add ~100 KB to every binary for no runtime consumer. `rcc.qrc` has no `../` precedent, so bundling would force a second copy, which is the drift class we are removing. The ledger, which the runtime *does* read, is the one that gets bundled. |
| Drift-gate mechanism | embedded checksum banner; re-render and byte-compare; git-diff heuristics | **Re-render + byte-compare**, exactly `generate-command-strings.py:105-114`'s `current != content -> return 1`. A checksum banner is self-referential, churns the diff on every regeneration, and adds nothing over comparing against a checked-in output. The marker banner stays, but as a *readability and lint* signal (`code-verify.py`'s `sdk-generated-edited` pattern), not as the integrity mechanism. |
| Corpus treatment | generate the corpus tables; lint the references; leave it alone | **Lint the references** — the corpus is teaching prose whose value is its voice and worked examples; generating it would flatten that. A reference lint catches the failure that actually happened (three tables disagreeing about the highest widget-option bit) without touching the writing, and without forcing a 1.3 MB search-index regeneration. |
| CI scope | add only the new checks; wire every existing gate | **Wire every existing gate** — `generate-command-strings.py --check` is a correct drift gate with zero callers anywhere in the repo, and `registry-verify.py` runs only in the local commit script. Adding new gates to a CI job that does not run the old ones would be theatre. |
| Entity scope | dataset only; dataset + the other three prose-schema verbs | **Dataset only** — the other three `project.*.update` verbs share the prose-as-schema shape, but their manifests do not exist until 0036's follow-up. Covering them here would mean hand-writing the field lists this spec exists to eliminate. |

## Risks & mitigations

- **Silent gRPC wire break — the headline risk.** A renumbered field is not a compile error;
  it is a client reading `title` from the bytes that held `units`. Mitigations: the ledger is
  append-only by construction; removals emit proto `reserved` so protoc enforces non-reuse;
  `code-verify.py:proto-field-renumbered` fails if a regenerated ledger moves an existing
  number; a static test (`tests/scripts/`, no app needed) asserts append-only and
  reserved-never-reused; the ledger lands and is reviewed *before* the dataset schema grows.
- **A gate that fires when the contributor cannot clear it.** The snapshot projection check
  can fail between a C++/manifest change and the maintainer's `--dump-api-schema` refresh. A
  gate people learn to ignore is worse than no gate. Mitigations: the failure message names
  the command, the field, the exact ordered fix (build -> `--dump-api-schema` ->
  `sanitize-commit.py`), and the fact that only registry-derived verbs are covered; the check
  is added to CI in the same task as the message, not before.
- **The projector and 0036's C++ emitter disagreeing.** Two renderings of one mapping is the
  precise failure this roadmap item exists to remove; building it would be self-defeating.
  Mitigation: a single `schema_props_for()` function feeds both, and the refactor that
  introduces it is its own task with a read-back verification against 0036's emitted C++.
- **MCP payload growth across 347 tools.** `tools/list` has no filtering and no paging; every
  command is a tool. Typed properties may cost more bytes than the prose paragraph they
  replace. Mitigation: measured, not assumed (AC10) — before/after sizes recorded as numbers;
  if the delta is material, the finding is surfaced as a decision rather than absorbed.
- **Deleting the runtime generator's numbering logic breaks GPL builds.** Explicitly avoided
  by the chosen design: the runtime keeps generating, so a GPL build's proto still lists only
  the RPCs it serves. The fallback path for unknown commands is the safety net.
- **`ci.yml` is contended.** The campaign serializes workflow edits to closeout; landing a
  `lint` job change mid-campaign risks a conflict with the other roadmap specs.
  Mitigation: the CI task is last in the ordering and is explicitly flagged as
  coordinator-sequenced.
- **`sanitize-commit.py` step ordering.** Steps run after both `clang-format` passes and after
  `black`, and the pipeline is non-fatal by design (`run_python_step` prints and continues,
  `:132-147`). A generator inserted in the wrong place either misses formatting or flaps
  against it. Mitigation: the new steps emit already-formatted output, sit next to the SDK
  step, and the "second consecutive run is clean" property is an explicit acceptance
  criterion (AC14).
- **Corpus lint false positives.** Prose contains identifier-shaped tokens that are not
  dataset fields. Mitigation: the lint keys on the manifest's own name namespace and on
  fenced/inline-code contexts, and every intentional exception is whitelisted with a one-line
  reason, following `registry-verify.py`'s existing style.
- **Search-index churn.** `app/rcc/ai/search_index.json` is 1.3 MB and its BM25 tie-ordering
  reshuffles when unrelated content shifts token counts. Mitigation: the corpus fixes are one
  task, the index regeneration rides the normal commit pipeline, and no task in this spec
  regenerates the index for its own purposes.

## Test & verification plan

- **Unit (I can run):** `tests/scripts/test_proto_ledger_static.py` — pure Python, no Qt, no
  app: every ledger number is unique per command, `reserved` and `fields` never intersect,
  `next` exceeds every assigned number, the committed typed proto's numbers match the ledger,
  and a simulated property insertion changes no existing number (**AC6, AC7**). Also
  `python -m py_compile` and `pytest --collect-only` on the new integration module.
- **Integration (maintainer runs; app up with the API server on 7777):**
  - `tests/integration/test_api_surfaces.py` (new):
    - **AC9** — an MCP `tools/list` reply lists every declared dataset field on the dataset
      update tool as a typed property with a description, and enum-valued fields carry their
      domains.
    - **AC10** — records `tools/list` payload size; the before number is captured on the
      pre-change build and reported alongside.
    - **AC11** — the SDK's dataset update wrapper sets every declared field and reads each
      back.
    - **AC8** — the runtime `.proto` export is byte-identical to
      `doc/grpc/serialstudio-typed.proto` (requires a full commercial build with
      `ENABLE_GRPC=ON`, which is what CI produces).
- **Generator checks (I can run):** run each generator twice and diff (**AC3**); seed each
  drift class — manifest changed without regeneration, generated artifact hand-edited, marker
  deleted, bogus corpus field name, wrong corpus enum value — confirm the message, revert
  (**AC2, AC4, AC12**); `python scripts/generate-property-registry.py --check`,
  `python scripts/generate-sdk.py --check`, `python scripts/registry-verify.py`,
  `python scripts/generate-command-strings.py --check` all clean.
- **CI:** inspect the `lint` job and confirm it fails on a deliberately drifted branch
  (**AC5**).
- **Maintainer checks:** `protoc` accepts `doc/grpc/serialstudio-typed.proto` (**AC8**); a GPL
  build exposes no Pro-only dataset property in the MCP schema, SDK, or typed proto, and its
  ledger is unchanged versus a commercial one (**AC13**); the demonstration property from
  **AC1** reaches all four surfaces with no hand edit; a second consecutive
  `sanitize-commit.py` run leaves a clean tree (**AC14**).
- **Hotpath:** `--benchmark-hotpath` once before commit, as an adjacency gate — `ProtoGenerator`
  is not on a frame path, but the gRPC server it belongs to is a benchmarked export sink.
- **Static:** `python scripts/code-verify.py --check` (including the new
  `api-generated-edited` / `proto-field-renumbered` rules); `qt-cpp-review` on the
  `ProtoGenerator` diff — the only C++ in this spec; `python scripts/sanitize-commit.py`
  before commit.
