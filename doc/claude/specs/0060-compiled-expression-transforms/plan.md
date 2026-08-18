---
spec: 0060-compiled-expression-transforms
phase: plan
status: approved     # gate auto-approved, overnight run 2026-08-17 (unattended; maintainer re-reviews)
updated: 2026-08-17
---

# Plan 0060 — Compiled expression transforms

> **Phase 2 of 4 — the HOW.** Gate auto-approved (maintainer asked for B to be planned and
> implemented; open questions resolved conservatively, recorded below).

## Approach (one paragraph)

A purpose-built evaluator (`DataModel::Expression`, `app/src/DataModel/Scripting/
ExpressionTransform.{h,cpp}`): tokenizer + precedence-climbing parser to a flat postfix
`Program` (bounded `kMaxNodes = 512`, stack depth 64), evaluated by a fixed-array stack machine
with no allocation and no engine. Sibling datasets and `sample(name, k)` history go through one
per-source `SlotTable` (slot = index handed out at compile time; latest value + a
`kMaxHistory = 256` ring per referenced dataset). Both lanes evaluate the same `Runtime::run(v,
t, table)`: the frame lane in `FrameBuilder` (a fourth engine kind in `TransformEngine`, keyed
`{sourceId, SerialStudio::Expression}`, `m_exprEngineForSource` cached in `beginDatasetPass`;
every dataset's final value publishes into the table behind one `[[unlikely]]` branch), the
stream lane in `StreamProcessor` (expression channels run in a sample-major second pass after
the ordinary channels, publishing per sample so "sibling = latest published value" holds
identically). `transformLanguage` gains `SerialStudio::Expression = 3` (validator, manifest text,
API handler, enum labels, editor combo with a live test that compiles against the project's
dataset titles). Benchmark: an ungated `HOTPATH_STREAM_EXPR_FPS` readout (R7).

## Affected subsystems & files

| File | Change |
|------|--------|
| `app/src/DataModel/Scripting/ExpressionTransform.{h,cpp}` | new evaluator, SlotTable, Runtime |
| `app/src/SerialStudio.h` | `ScriptLanguage::Expression = 3` |
| `app/src/DataModel/Project/PropertyValidators.cpp` | accept 3 |
| `app/rcc/properties/dataset.json` + `app/src/API/Generated/DatasetApiFields.cpp` (regenerated) | API text |
| `app/src/API/EnumLabels.cpp`, `app/tests/tst_enum_labels.cpp` | slug/label + rows |
| `app/src/API/Handlers/ProjectHandlerEntities.cpp` | accept language 3, skip JS/Lua mismatch heuristic |
| `app/src/DataModel/FrameBuilder.{h,cpp}` | engine kind, compile, apply, publish, capture-flag refinement |
| `app/src/IO/StreamWorker.{h,cpp}`, `app/src/IO/ConnectionManager.cpp` | `title` in channel config, expression pass |
| `app/src/DataModel/Editors/DatasetTransformEditor.{h,cpp}` | third language, test/validate |
| `app/src/Benchmark/HotpathBenchmark.cpp` | expression stream readout |
| `app/CMakeLists.txt`, `app/tests/CMakeLists.txt`, `app/tests/tst_expression_transform.cpp` | build + unit test |

## Hotpath & threading impact

- **Touches the hotpath?** Yes, minimally: `applyDatasetValue*` gain one predicted-not-taken
  branch (`if (m_exprEngineForSource)`) that publishes into the SlotTable; `applyTransform`
  gains a language check ahead of the Lua/JS dispatch. No allocation, no locks, no engine.
  The capture flag no longer turns on for expression-only projects (they never read the table
  store), so the numeric gates should not move; report the benchmark.
- **New cross-thread signal/slot?** No. SlotTable is owned by whichever lane's thread runs it.
- **New input to a cached hotpath flag?** No.
- **Timestamp ownership** — unchanged (`t` = `info.timestampMs`, filled when engines exist).

## Decisions taken for the open questions

| Question | Decision |
|----------|----------|
| User-facing name | "Expression" (combo row 3); dedicated one-line field deferred |
| History rings | per referenced dataset only (compile-time discovery through `slotFor`) |
| Pro gate | none (free) — a maintainer flip in the editor combo if wanted |
| Benchmark in CI | ungated readout only, printed with the stream phase |
| Sibling semantics | latest *published* value on both lanes; a sibling later in order yields the previous sample; `v` is always the dataset's raw sample |

## Risks & mitigations

- `--check-snapshot` fails in CI until the API snapshot is refreshed from a build (manifest text
  changed).
- `QCodeEditor::setCompleter(nullptr)` for the Expression row: verify it tolerates null.
- Names shadowed by built-ins (`v t n dt pi e nan inf`) need braces to reach a dataset of that
  title only if it also matches a builtin (documented in the header).

## Test & verification plan

- `tst_expression_transform` (ctest, unbuilt tonight); `tst_enum_labels` updated.
- `--benchmark-hotpath`: gated tiers unchanged; `HOTPATH_STREAM_EXPR_FPS` printed.
- Spec AC1-AC5 in the running app.
