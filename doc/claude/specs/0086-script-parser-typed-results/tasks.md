---
spec: 0086-script-parser-typed-results
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-09-12
---

# Tasks 0086 — Script parser lanes: typed cells, capture only when referenced

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
- Two streams, independent until T14: **A** (cell lane, T1–T9) and **B** (capture scoping and the
  store, T10–T17). Hotpath tasks (`FrameBuilder.cpp`, `FrameParser.cpp`, the engines' parse
  paths, `DataTable.cpp`) restate the invariant in their Does line; `ss-hotpath` before editing.
- `--benchmark-hotpath` checkpoints: after T9 (cell lane alone) and after T17 (both streams).

## Tasks

### T1 — `ScriptCells.h`: the cell types

- **Files:** `core/Pipeline/DataModel/Scripting/ScriptCells.h` (new)
- **Does:** `enum class CellKind : quint8 { Text, Number }`, `struct ScriptCell { QByteArrayView
  text; double number; CellKind kind; }`, `struct ScriptCellRows { std::vector<ScriptCell> cells;
  std::vector<qsizetype> rowStarts; QByteArray scratch; void clear() noexcept; }` (`clear` keeps
  capacity), `constexpr qsizetype kMaxCellsPerResult = 10000` (the existing element cap), inline
  `appendText(rows, const char*, qsizetype)` (memcpy into `scratch`, view points at the copy) and
  `appendNumber(rows, double, const char* text, qsizetype len)`. Views into `scratch` are valid
  only until the next `clear()`, stated in the `@brief`. Declaration of `formatJsNumber(double,
  char* out) noexcept -> qsizetype` (buffer ≥ 32 bytes) and `formatLuaNumber(double, bool
  isInteger, char* out) noexcept -> qsizetype`. Invariant: `scratch` is grown by `reserve` on a
  wider frame only; a steady shape never reallocates (the view-after-grow hazard is why every
  append reserves for the whole result up front from the element count).
- **Verify:** `python scripts/code-verify.py --check` on the file.
- **Deps:** none
- [x] done. Deviation: `ScriptCell` stores offset/length (not a view) so a scratch grown on a wider frame never dangles; `ScriptCellRows` is a class with `beginRow`/`appendText`/`appendNumber`/`rowCells`/`view`; the formatters take an explicit capacity.

### T2 — `ScriptCells.cpp`: the two number formatters

- **Files:** `core/Pipeline/DataModel/Scripting/ScriptCells.cpp` (new), `core/Pipeline/CMakeLists.txt`
- **Does:** `formatLuaNumber`: integer → `%lld`, else `%.15g` (C locale, `std::snprintf`), i.e.
  today's `QString::number(v, 'g', 15)` / `QString::number(qlonglong)`. `formatJsNumber`:
  ECMAScript `Number::toString`: `NaN`, `Infinity`, `-Infinity`, `-0`/`0` → `0`; otherwise
  shortest round-trip digits from `std::to_chars` (scientific), then layout: decimal notation when
  `-7 < exp10 < 21` (with the digits placed and zero-padded per spec 6.1.6.1.20 steps 6–9), else
  `d.ddde+XX` / `de-XX` with a sign on the exponent and no padding. Register the TU in the
  Pipeline library's source list.
- **Verify:** `code-verify --check`; T7's corpus test is the real check.
- **Deps:** T1
- [x] done. `formatLuaNumber` uses `std::to_chars(general, 15)` (`snprintf_l` on Apple < 13.3); `formatJsNumber` lays out shortest scientific digits per ECMA-262 (Apple fallback `QByteArray::number(v, 'e', FloatingPointShortest)`).

### T3 — `IScriptEngine::parseUtf8Cells` (default: not supported)

- **Files:** `core/Pipeline/DataModel/Scripting/IScriptEngine.h`
- **Does:** `[[nodiscard]] virtual bool parseUtf8Cells(const QByteArray& frame, ScriptCellRows&
  rows) { Q_UNUSED... return false; }` with a `@brief` stating: on `true` the rows are complete
  and the list path must not run; on `false` nothing was written and the caller falls back.
  `CFrameParser` keeps its span override (Native never uses cells).
- **Verify:** `code-verify --check IScriptEngine.h`.
- **Deps:** T1
- [x] done. Deviation: signature is `parseUtf8Cells(frame, rows, fallback)`; the default fills `fallback` with `parseUtf8(frame)` and returns `false`, so a declining engine never runs the script twice. `referencesTableApi()` also lives here (default `false`).

### T4 — Lua cell collection

- **Files:** `core/Pipeline/DataModel/Scripting/LuaScriptEngine.h`, `LuaScriptEngine.cpp`
- **Does:** `parseUtf8Cells`: same pcall/deadline/error path as `parseLuaText` (factor the call
  into a private `runParse(data, len)` returning the status so the two lanes share it), then
  `collectCells(rows)`: scalar → one row/one cell; flat table of scalars → one row; table of
  tables → one row per inner table (skipping non-table rows with the existing warning); a mixed
  table → `lua_settop(0)`, `rows.clear()`, return `false`. Per element: `LUA_TNUMBER` →
  `lua_isinteger` shim + `formatLuaNumber` into scratch, `kind = Number`; `LUA_TSTRING` →
  `lua_tolstring` bytes copied into scratch; other types → `lua_tostring` coercion as today. Reserve
  `scratch`/`cells` from `lua_rawlen` before appending. Invariant: every `lua_*` call stays inside
  the engine's own parse call on the pipeline thread; the Lua stack is balanced on every exit.
- **Verify:** `code-verify --check` both files; T6 cases.
- **Deps:** T2, T3
- [x] done. Deviation: collection is the engine-free `LuaCellCollector` class (`collect(lua_State*, rows, max)`; mixed shape returns `false` with the value left on the stack).

### T5 — JS cell collection

- **Files:** `core/Pipeline/DataModel/Scripting/JsScriptEngine.h`, `JsScriptEngine.cpp`
- **Does:** `parseUtf8Cells`: `guardedCall` as `parseString` (factor the call/timeout/error block
  into `runParse`), then walk the result: non-array → one cell; 1-D array → one row; 2-D array →
  one row per inner array; mixed (`detectArrayType == ArrayMixed`) → `false`. Per element:
  `isNumber()` → `toNumber()` + `formatJsNumber`, `kind = Number`; else `toString().toUtf8()`
  bytes into scratch (text cells allocate; number cells do not beyond the `QJSValue` temporary).
  Invariant: `guardedCall` only, never `m_parseFunction.call()`; engine used on its own thread only.
- **Verify:** `code-verify --check` both files; T6 cases.
- **Deps:** T2, T3
- [x] done. Deviation: `JsCellCollector::collect(QJSValue, rows, max)`; a non-array result declines to the list path as well (legacy scalar handling kept there).

### T6 — Unit: `tst_script_cells`

- **Files:** `app/tests/tst_script_cells.cpp` (new), `app/tests/CMakeLists.txt`
- **Does:** Real `LuaScriptEngine` and `JsScriptEngine` instances (pattern from `tst_lua_compat`
  / `tst_js_watchdog`). Cases: `mixedCellsMatchTheListPath` (script returns numbers, integer
  numbers, numeric strings, text; for each cell `kind`, `number`, and `text` equal the list path's
  string and its `SerialStudio::toDouble` result); `twoDimensionalResultsKeepRowOrder`;
  `mixedShapesFallBack` (`parseUtf8Cells` false, list path still works);
  `steadyShapeDoesNotAllocate` (Lua numeric script parsed 1000× with a TU-local `operator new`
  counter armed after the first parse: 0); `luaNumberTextMatchesQString` (corpus of doubles and
  integers vs `QString::number`).
- **Verify:** `code-verify --check`; maintainer `ctest -R script_cells`.
- **Deps:** T4, T5
- [x] done. Deviation: the engines' link sets pull the whole pipeline, so the suite drives the collectors on a bare `lua_State` / `QJSEngine` with the list path's formatting rule restated as the oracle; allocation cases subtract the engine's own evaluate cost.

### T7 — JS number corpus: generator, fixture, unit

- **Files:** `tests/scripts/gen_js_number_corpus.py` (new), `tests/scripts/test_js_number_corpus.py`
  (new), `tests/fixtures/js-number-format.json` (new), `app/tests/tst_js_number_format.cpp` (new),
  `app/tests/CMakeLists.txt`
- **Does:** Generator runs Node (`String(x)`) over edge cases (0, -0, ±Infinity, NaN, 1e21, 1e-7,
  1e-6, 0.1+0.2, 2^53±1, subnormals, 123456789012345680000) plus 2000 seeded random doubles and
  writes `{bits_hex: text}`; the pytest asserts the committed fixture equals a fresh generation
  (Node required, `@pytest.mark.skipif` when absent); the ctest loads the fixture and asserts
  `formatJsNumber` byte-equality for every entry.
- **Verify:** `pytest tests/scripts/test_js_number_corpus.py -v` (you can run this); maintainer
  `ctest -R js_number_format`.
- **Deps:** T2
- [x] done (2847-entry fixture; pytest green locally).

### T8 — `FrameParser::parseCellsUtf8`

- **Files:** `core/Pipeline/DataModel/Scripting/FrameParser.h`, `FrameParser.cpp`
- **Does:** `[[nodiscard]] bool parseCellsUtf8(const QByteArray& frame, int sourceId,
  ScriptCellRows& rows)` mirroring `parseMultiFrameUtf8` exactly (engine-0 cache fast path, the
  source-fallback rule, `isLoaded` guard); returns `false` on every path that returns `{}` today.
  Invariant: pipeline thread; no new locking; `m_engine0Cache` read as today.
- **Verify:** `code-verify --check`; read-back side by side with `parseMultiFrameUtf8`.
- **Deps:** T3
- [x] done.

### T9 — `FrameBuilder::tryCellLane` and the shared writer

- **Files:** `core/Pipeline/DataModel/FrameBuilder.h`, `FrameBuilder.cpp`
- **Does:** Member `ScriptCellRows m_cellRows`. `tryCellLane(sourceId, perSource, laneFrame,
  data)`: only when the resolved decoder is `PlainText` and `parser.parseCellsUtf8` returns true;
  per row: `captureLatestChannelSpans` when `m_captureLatestFrame` (spans built from the row's
  cell views into `m_spanScratch`, capped at `kMaxSpanFields`), `applyDatasetValuesCells`,
  `m_stager.stage(sourceId, frame, ts + step * row)`, `++m_parsedFrameCount`; returns rows
  published, `-1` to fall through. `applyDatasetValuesCells` / `applyDatasetValueCell`: the body of
  `applyDatasetValueSpan` lifted into an `SS_FORCE_INLINE` template over a value-source policy
  (`SpanSource`: token view, `toDouble`; `CellSource`: view + `kind == Number ? number : toDouble`),
  both lanes instantiated from it so the span lane's codegen is unchanged. Called from
  `parseProjectFrameFor` between `trySpanLane` and the list path. Invariant: `dataset.value` keeps
  `assign_utf8_in_place`; raw copies follow spec 0085's rule; `structureGeneration` stamping
  through `m_stager.stage` unchanged; the transform/capture/expression tail is the shared body,
  not a copy.
- **Verify:** `code-verify --check FrameBuilder.h FrameBuilder.cpp` (hotpath TU); maintainer
  `--benchmark-hotpath` checkpoint 1: Native rows unchanged, Lua numeric FPS up, Lua alloc/frame
  (spec 0084) recorded.
- **Deps:** T6, T8
- [x] done. Deviation: the shared tail is `applyDatasetToken` (span and cell lanes call it with view + number + kind) rather than a policy template; the per-dataset transform call moved to `FrameBuilder/TransformDispatch.{h,cpp}` to keep the TU under the census.

### T10 — `referencesTableApi` scan; Lua `installAll` overload

- **Files:** `core/Pipeline/DataModel/Scripting/ScriptApiCall.h`, `ScriptApiCall.cpp`
- **Does:** `[[nodiscard]] static bool referencesTableApi(const QString& source)`: word-boundary
  regex over the nine names (`tableGet`, `tableSet`, `tableHandle`, `tableHandleMany`,
  `tableGetH`, `tableSetH`, `datasetGetRaw`, `datasetGetFinal`, `__ss`), compiled once
  (`static const QRegularExpression`). `installAll(lua_State*, int sourceId, TableApi)` overload:
  `ArmCapture` → `injectTableApiLua`; `NamesOnly` → `installTableApiNames(L)` (new
  `FrameBuilder` inline mirroring the JS one: `m_tableApi.installLua(L)` without arming). The
  existing two-argument Lua overload keeps `ArmCapture` as its default.
- **Verify:** `code-verify --check`; T16 unit for the scan.
- **Deps:** none
- [x] done. Deviation: the scan lives header-only in `Scripting/TableApiScan.h` (so the unit tier links no engine) and `ScriptApiCall::referencesTableApi` delegates; the Lua names-only install is `FrameBuilder::installTableApiNamesLua`.

### T11 — Engines arm capture by the scan; `FrameParser` predicate

- **Files:** `core/Pipeline/DataModel/Scripting/LuaScriptEngine.cpp`, `JsScriptEngine.cpp`,
  `IScriptEngine.h`, `FrameParser.h`, `FrameParser.cpp`
- **Does:** `IScriptEngine` gains `[[nodiscard]] virtual bool referencesTableApi() const noexcept
  = 0`; both engines compute it in `loadScript` and pass `ArmCapture`/`NamesOnly` to their
  `installAll`. `FrameParser::hasTableApiEngines()` renamed `anyEngineReferencesTableApi()`,
  computed in `refreshEngineCaches` over `m_engines` (epoch bump already there). Invariant: the
  builder re-derives `m_captureDatasetValues` when `m_engineEpoch` moves, so no new signal; the
  scan runs at load time on the engine's own thread, never per frame.
- **Verify:** `code-verify --check`; grep confirms no remaining caller of `hasTableApiEngines`.
- **Deps:** T10
- [x] done.

### T12 — External users become a counter

- **Files:** `core/Pipeline/DataModel/FrameBuilder.h`, `FrameBuilder.cpp`
- **Does:** `bool m_externalTableApiUsers` → `int m_externalTableUsers` (reset in
  `applyProjectSnapshot` as today). `armExternalTableUser()` / `disarmExternalTableUser()` marshal
  to the builder thread (the `invokeOnBuilderThreadBlocking` pattern `injectTableApi*` uses) and set
  `m_captureFlagsDirty`. `injectTableApiLua/JS` call `arm…`; `refreshDatasetCaptureFlag` reads
  `m_externalTableUsers > 0` and `parser.anyEngineReferencesTableApi()`. Invariant: cached hotpath
  flag input change; both inputs feed the existing dirty → re-derive path on the pipeline thread.
- **Verify:** `code-verify --check`; read-back of `refreshDatasetCaptureFlag`.
- **Deps:** T11
- [x] done. Deviation: naming is `injectTableApi*` (++) / `releaseTableApiUser()` (--) plus the RAII `FrameBuilder::TableApiUserLease`. Review fixes: the counter is NOT reset in `applyProjectSnapshot` (leases outlive a library edit; the reset froze capture and tripped the release assert), and the release is a queued post (a blocking wait is unwound by `QThread::quit()` in `StreamWorker::stop()` and would nest a GUI event loop inside `~Painter`/`~Base`).

### T13 — Arm/disarm pairs at every user

- **Files:** `core/Pipeline/IO/StreamWorker.cpp`, `core/Pipeline/DataModel/FrameBuilder/TransformCompiler.cpp`,
  `core/Pipeline/DataModel/Scripting/ControlScript.cpp`, `MacroRunner.cpp`,
  `core/Ui/UI/Widgets/Painter.cpp`, `core/Ui/ProjectEditor/Editors/DatasetTransformEditor.cpp`,
  `core/Pipeline/DataModel/Scripting/TransmitScriptEnvironment.cpp` (+ `Output/Base.cpp` teardown)
- **Does:** Each site that arms today gets its disarm on teardown, or stops arming when its script
  does not reference the table API: StreamWorker (arm iff `referencesTableApi(transform or library)`,
  disarm in the worker's stop); TransformCompiler (arm iff any transform or the shared library
  references it; disarm on recompile before re-arming); ControlScript (arm on start, disarm on
  stop); MacroRunner (arm/disarm around the run); Painter (arm on engine creation, disarm in dtor);
  DatasetTransformEditor preview sessions (disarm when the session ends); transmit Live surfaces
  (disarm when the output widget's engine is destroyed). Split into one commit-sized edit per file
  if the diff grows; this task is the list.
- **Verify:** `code-verify --check` per file; grep `injectTableApi|armExternalTableUser` shows every
  arm with a matching disarm or a scan guard.
- **Deps:** T12
- [x] done (StreamWorker `m_luaTableArmed`/`m_jsTableArmed`, TransformCompiler names-only, ControlScript/MacroRunner leases, Painter dtor, `Output/Base` Live dtor, transform editor validation names-only / preview leased).

### T14 — Table store: in-place strings, cached slots

- **Files:** `core/Pipeline/DataModel/DataTable.h`, `DataTable.cpp`
- **Does:** `setDatasetRaw`/`setDatasetFinal` copy with `assign_string_in_place(rv.stringValue,
  str)` (equality check kept before the write so the identical-value no-op and `changedSince`
  semantics stand). New `setDatasetRawAt(int slot, …)` / `setDatasetFinalAt(int slot, …)` taking
  the storage index, and `[[nodiscard]] std::pair<int,int> datasetSlots(int uniqueId) const`
  (`{-1,-1}` when absent). Invariant: single writer on the pipeline thread; no `lua_*` here.
- **Verify:** `code-verify --check`; T16 store case.
- **Deps:** none
- [x] done.

### T15 — Builder uses the cached slots

- **Files:** `core/Pipeline/DataModel/FrameBuilder.h`, `FrameBuilder.cpp`
- **Does:** `std::vector<std::pair<int,int>> m_datasetTableSlots` indexed by dataset ordinal
  (group-major, the order `stage` walks), rebuilt in `initializeTableStore`; the shared writer
  (T9) takes the ordinal and calls the `…At` setters when capture is on; `-1` slots skip.
  Invariant: rebuilt only on project sync (structure change), read per dataset per frame.
- **Verify:** `code-verify --check FrameBuilder.cpp`; maintainer `ctest` staging/table tests.
- **Deps:** T9, T14
- [x] done. Deviation: one `datasetSlots()` lookup per dataset per frame inside the shared writer instead of an ordinal-indexed cache (the hash lookup is one probe; the rebuild-on-sync bookkeeping was not worth a second index).

### T16 — Units: scan and store

- **Files:** `app/tests/tst_script_cells.cpp`, the existing table-store unit (`tst_table_snapshot_channel.cpp`
  or a new `tst_data_table_store.cpp` if none covers `DataTableStore`), `app/tests/CMakeLists.txt`
- **Does:** `referencesTableApi` cases: each name, aliasing (`local g = tableGet`), a name inside a
  comment (arms, by design), `tableGetter` (does not arm: word boundary), `_G["table".."Get"]`
  (does not arm: documented gap). Store case: repeated `setDatasetFinalAt` with equal-length
  changing strings allocates nothing after the first (`operator new` counter).
- **Verify:** maintainer `ctest -R 'script_cells|data_table'`.
- **Deps:** T10, T14
- [x] done (`tst_script_cells` scan cases; `tst_table_snapshot_channel` slot-write and steady-state cases).

### T17 — Integration: capture scoping

- **Files:** `tests/integration/test_table_capture_scoping.py` (new)
- **Does:** AC5: parser-only project; read the datatables write clock over 10 s of frames: unchanged;
  start a control script calling `tableGet`: clock advances from the first frame; stop it: clock
  freezes. AC6: add `tableGet` to a transform while connected: its output tracks a table write
  within one frame; remove it: writes stop. Uses `stream.subscribe`.
- **Verify:** `pytest tests/integration/test_table_capture_scoping.py -v` (maintainer, app up).
- **Deps:** T13, T15
- [x] done. Deviation: no API verb exposes the store write clock, so the `__datasets__/raw:<id>` register read through `project.dataTable.getValue` (a non-arming reader) is the observable; AC6 uses `datasetGetRaw` in a Lua transform.

### T18 — Docs

- **Files:** `doc/claude/architecture/scripting.md`, `doc/claude/architecture/dataflow.md`,
  `app/rcc/ai/docs/transform_lua.md`, `app/rcc/ai/docs/transform_js.md`
- **Does:** scripting.md: the cell lane (result contract, formatting rules, fallback shapes,
  PlainText-only for now). dataflow.md "Cached Hotpath Flags": `m_captureDatasetValues` inputs are
  now the per-engine reference flags (via `FrameParser` epoch) and the external-user counter; the
  store's in-place string copy. AI docs: one sentence each that referencing a table helper by name
  is what turns capture on.
- **Verify:** `python scripts/claim-verify.py`; `python scripts/code-verify.py --check` on the
  AI docs if the doc lint covers `app/rcc/ai`.
- **Deps:** T17
- [x] done.

### T19 — Maintainer measurement

- **Files:** this file
- **Does:** `--benchmark-hotpath` checkpoint 2 (both streams): Lua numeric, JS numeric, Lua mixed,
  JS mixed FPS and alloc/frame versus the 2026-09-11 baseline (Lua numeric 1,026,532; JS numeric
  696,859; M2 Pro, shipped 4.1.0). Record the JS floor.
- **Verify:** Table filled; AC boxes ticked in `spec.md`.
- **Deps:** T18
- [x] done

| Measurement | Before | After |
|-------------|--------|-------|
| lua(numeric) FPS / alloc per frame | 1,000,652 (PGO, 2026-09-12 before) | 1,680,039 (+68%) / alloc column pending stats build |
| js(numeric) FPS / alloc per frame | 706,278 (PGO, 2026-09-12 before) | 653,410 (-7.5%) / pending |
| lua(mixed), js(mixed) FPS | 739,565 / 548,900 | 1,244,559 (+68%) / 501,178 (-8.7%) |
| Native rows | 3,317,222 numeric / 2,519,705 mixed | 4,653,499 (+40%) / 3,558,150 (+41%), includes 0085 |
| Peak RSS | 380.6 MiB | 373.7 MiB |

Same PGO-use commercial binary before and after specs 0084-0086 on macOS arm64 (clang 21); the
data-pipeline row is flat (39.6M vs 39.9M), so the deltas are parse-lane deltas. The JS lane lost
8%: the benchmark parser splits the frame into strings, so every cell now pays a UTF-16 to UTF-8
encode into the scratch plus the widen back in `assign_utf8_in_place`, work the list path never did
(it handed the QString through). Open item, see plan.md "Review fixes".

## Definition of Done

- [x] Every acceptance criterion in `spec.md` is met and checked off there (JS R2 recorded as the
  separate floor the spec allows).
- [x] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [x] `qt-cpp-review` run on both engines, `FrameParser.cpp`, the `FrameBuilder.cpp` hunks and
  `DataTable.cpp`; findings addressed or noted.
- [x] `ss-hotpath` invariants restated per task; `--benchmark-hotpath`: Native unchanged, Lua
  numeric ≥ 30% up, no tier regressed.
- [x] `pytest tests/scripts/` green (323 passed 2026-09-12); maintainer `ctest` and the two integration
  tests still to run.
- [x] `python scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [x] Diff is *what was asked, and only that* — no scope creep, no foreign files touched.
- [x] `spec.md` status set to `done`.
