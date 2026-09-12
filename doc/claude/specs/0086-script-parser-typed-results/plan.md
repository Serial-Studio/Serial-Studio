---
spec: 0086-script-parser-typed-results
phase: plan
status: approved     # draft -> approved (gate before /ss-tasks)
updated: 2026-09-11
---

# Plan 0086 — Script parser lanes: typed cells, capture only when referenced

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Read the relevant `doc/claude/` sub-docs and the *actual code*
> before writing this — a plan grounded in a stale mental model is worse than no plan.
> Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

A **cell lane** between the span lane and the list fallback. `IScriptEngine` gains
`parseUtf8Cells(frame, ScriptCellRows&)`: the engine runs the script as today, then walks the
result once and fills an engine-owned, reused `ScriptCellRows` (a flat `ScriptCell` array with
row offsets and a byte scratch), where each cell is a byte view plus a `double` and a kind
(number / text). Numbers are formatted once into the scratch by the engine's own rule (Lua:
`%.15g` / integer; JS: an ECMAScript `Number::toString` formatter), so the cell carries exactly
the text today's `QStringList` would have carried, and the frame builder's writer takes the
number from the cell instead of parsing the text back. `FrameBuilder::parseProjectFrameFor`
tries `trySpanLane`, then the new `tryCellLane` (PlainText decoder only), then the list path;
the cell writer is the span writer with a numeric hint, so `dataset.value` keeps its in-place
UTF-8 assign and the rest of `applyDatasetValueSpan` (transforms, capture, table store) is
shared. **Capture scoping** replaces "a Lua parser engine exists" with "some engine's source
references a table-API name" (a compile-time scan at `loadScript` / transform compile, folded
into `FrameParser::refreshEngineCaches` and its epoch) plus a counted external-user arm/disarm
in place of today's sticky bool; and the table store's dataset writers copy strings in place
and take a cached slot instead of a hash lookup.

## Affected subsystems & files

| File | Change |
|------|--------|
| `core/Pipeline/DataModel/Scripting/ScriptCells.h` (new) | `struct ScriptCell { QByteArrayView text; double number; quint8 kind; }`, `struct ScriptCellRows { std::vector<ScriptCell> cells; std::vector<qsizetype> rowStarts; QByteArray scratch; void clear() noexcept; }`, `kMaxCellsPerResult` = the existing element cap, `appendNumber(rows, double, fmt)` / `appendText(rows, view)` helpers, `formatJsNumber(double, char*)`. |
| `core/Pipeline/DataModel/Scripting/ScriptCells.cpp` (new) | `formatJsNumber`: shortest round-trip digits via `std::to_chars`, then ECMAScript 6.1.6.1.20 layout rules (exponent form below 1e-6 and at or above 1e21, `NaN`, `Infinity`, `-0` → `0`). |
| `core/Pipeline/DataModel/Scripting/IScriptEngine.h` | `virtual bool parseUtf8Cells(const QByteArray&, ScriptCellRows&)` defaulting to `false` (fallback to the list path), plus `virtual bool referencesTableApi() const noexcept`. |
| `core/Pipeline/DataModel/Scripting/LuaScriptEngine.h/.cpp` | `parseUtf8Cells`: run `parseLuaText`'s pcall, then `collectCells` over the result (scalar, flat table, table of tables); mixed shapes return `false`. `luaValueToString` logic reused as the formatting rule into scratch. `m_referencesTableApi` set in `loadScript`; the API install passes arm/names-only accordingly. |
| `core/Pipeline/DataModel/Scripting/JsScriptEngine.h/.cpp` | `parseUtf8Cells`: iterate the result array, `isNumber() ? toNumber()` (formatted by `formatJsNumber`) : `toString()` bytes; 2-D arrays produce rows; mixed shapes return `false`. Scan in `loadScript`; `installAll(..., ArmCapture|NamesOnly)` by the scan. |
| `core/Pipeline/DataModel/Scripting/ScriptApiCall.h/.cpp` | `referencesTableApi(const QString& source)` (word-boundary scan for `tableGet`, `tableSet`, `tableHandle`, `tableHandleMany`, `tableGetH`, `tableSetH`, `datasetGetRaw`, `datasetGetFinal`, `__ss`); a Lua `installAll(L, sourceId, TableApi)` overload mirroring the JS one. |
| `core/Pipeline/DataModel/Scripting/FrameParser.h/.cpp` | `parseCellsUtf8(frame, sourceId, rows)` mirroring `parseMultiFrameUtf8`'s engine-0 cache path; `hasTableApiEngines()` becomes `anyEngineReferencesTableApi()` computed in `refreshEngineCaches` (epoch bump unchanged). |
| `core/Pipeline/DataModel/FrameBuilder.h/.cpp` | `tryCellLane`; `applyDatasetValuesCells` + `applyDatasetValueCell` sharing the span writer body through a value-source policy; `m_cellRows` member; `refreshDatasetCaptureFlag` reads the new predicate and the external-user count; `m_externalTableApiUsers` bool → `int m_externalTableUsers` with `armExternalTableUser()` / `disarmExternalTableUser()`; per-dataset table slot cache `m_datasetTableSlots` refreshed in `initializeTableStore`. |
| `core/Pipeline/DataModel/FrameBuilder/TransformCompiler.cpp` | Arms capture only when the transform code or the shared library references the table API. |
| `core/Pipeline/IO/StreamWorker.cpp` | Same scan for stream transforms; disarms on teardown. |
| `core/Pipeline/DataModel/DataTable.h/.cpp` | `setDatasetRaw`/`setDatasetFinal` use `assign_string_in_place`; new `setDatasetRawAt(slot, …)` / `setDatasetFinalAt(slot, …)` taking the cached index; `datasetSlots(uniqueId)` accessor. |
| `core/Ui/...` (table views), `core/Api/API/Handlers/...` (datatable verbs), control script host | Call sites of `injectTableApiLua/JS` and `noteGuiUser` become arm/disarm pairs; exact list from the tasks-phase grep of `injectTableApi` and `noteGuiUser`. |
| `app/tests/tst_script_cells.cpp` (new) | AC3, AC4 against real `LuaScriptEngine` / `JsScriptEngine` instances (the unit tier already links LuaJIT and `QJSEngine`, see `tst_lua_compat`, `tst_js_watchdog`). |
| `app/tests/tst_js_number_format.cpp` (new) | `formatJsNumber` against a checked-in corpus. |
| `tests/scripts/gen_js_number_corpus.py` + `tests/fixtures/js-number-format.json` (new) | Corpus generator (Node `String(x)` over edge cases and random doubles) and the committed fixture the ctest reads. |
| `tests/integration/test_table_capture_scoping.py` (new) | AC5, AC6 over the API server. |
| `doc/claude/architecture/scripting.md`, `dataflow.md` | Parser result contract (cell lane), capture rule and its inputs in "Cached Hotpath Flags". |
| `app/rcc/ai/docs/transform_lua.md` / `transform_js.md` | One sentence: capture is armed by referencing the table API. |

## Architecture & data flow

**Cell lane, pipeline thread, plain calls:**

```
parseProjectFrameFor
  trySpanLane ........... Native/PlainText, unchanged
  tryCellLane ........... PlainText decoder && parser.parseCellsUtf8(bytes, sourceId, m_cellRows)
      for each row (rowStarts):
        captureLatestChannelSpans (views, as the span lane)      [m_captureLatestFrame]
        applyDatasetValuesCells(frame, cells, count, info)
        m_stager.stage(sourceId, frame, ts + step * row)
  list path ............. decodeProjectChannels → applyDatasetValues (unchanged fallback)
```

`ScriptCellRows` is owned by `FrameBuilder` (one per builder, reused; `clear()` keeps
capacity). The engine writes into it: `scratch` receives formatted numbers and copied string
bytes (Lua strings live on the Lua stack only until popped, so they are copied; the copy is a
`memcpy` into reserved bytes), `cells[i].text` views into `scratch`, `cells[i].number` is set
for number cells. Capacity grows on first sight of a wider frame and never shrinks, so a
steady frame shape allocates nothing (R2). The element cap is the existing `kMaxElements`.

**Writer.** `applyDatasetValueCell(dataset, cells, count, info)` is `applyDatasetValueSpan`
with one substitution: `numericValue = cell.kind == Number ? cell.number :
SerialStudio::toDouble(cell.text, &isNumeric)`; `isNumeric` is `true` for number cells.
Everything after the value (raw copy per spec 0085's rule, capture, transform, expression
publish, final capture) is the shared body, expressed as one `SS_FORCE_INLINE` template over
a value-source policy so the span lane's codegen is unchanged.

**Formatting rules (R5).**
- Lua: integer-valued → `QString::number(lua_tointeger)` today, i.e. `%lld`; else
  `QString::number(v, 'g', 15)`, i.e. `%.15g` in the C locale. Both reproduced with
  `std::snprintf` into the scratch; identical output is a corpus assertion.
- JS: `QJSValue::toString()` on a number is ECMAScript `Number::toString`: shortest
  round-trip digits, decimal form for 1e-6 ≤ |x| < 1e21, exponent form (`1e+21`,
  `1e-7`) outside, `NaN`, `Infinity`, `-Infinity`, and `-0` → `"0"`. `std::to_chars` gives
  the shortest digits; `formatJsNumber` applies the layout. Pinned by the Node-generated
  corpus.

**Multi-frame results (R4).** Table of tables / 2-D array → one row per inner table in order;
`rowStarts` delimits rows; empty inner rows are skipped as today. Mixed scalar-plus-vector
shapes (`unzipMixedFrames`) return `false` from `parseUtf8Cells` and take the list path,
unchanged.

**Capture scoping (R6, R7).** Inputs to `m_captureDatasetValues` become:

| Input | Today | After |
|-------|-------|-------|
| parser engines | `m_hasLuaEngine` (any Lua parser) | any engine whose source references a table-API name (`refreshEngineCaches`, epoch bump) |
| transform engines | `hasScriptEngines()` | any transform (or shared library) whose source references a table-API name |
| stream transforms | arm on inject, never disarm | arm when referenced, disarm on worker teardown |
| external users | sticky `bool` set by any `injectTableApi*`, cleared on project load | `int` count: control script start/stop, API server enable/disable, table views open/close, transmit environments |

The scan is a word-boundary match over the script source for the nine names (eight helpers
plus the JS bridge object). It is conservative on purpose: a name inside a comment or string
arms capture (a few percent per frame), while the alternative, a runtime first-touch flag,
loses the first frame's values. Both the epoch and the counter feed the existing
`m_captureFlagsDirty` → `refreshDatasetCaptureFlag()` re-derive on the builder thread, so the
flag is never stale (spec-0051 two-thread refresh rule: refresh slots are pipeline-affine and
queued from GUI emitters).

**Table store writes (R8).** `setDatasetRawAt(slot, numeric, str, isNum)`:
`assign_string_in_place(rv.stringValue, str)` instead of share-assign. This matters twice:
it removes the allocation seen in the profile, and it stops the store from pinning
`dataset.value`'s buffer, which is the `common-mistakes.md` "share-assign re-links buffers"
case (the next in-place write detached and re-allocated). The slot pair per dataset is cached
in `FrameBuilder::m_datasetTableSlots` (indexed by dataset ordinal, rebuilt in
`initializeTableStore`), replacing two `QHash::constFind` per dataset per frame.

## Hotpath & threading impact

- **Touches the hotpath?** Yes: `FrameBuilder::parseProjectFrameFor` (new lane between the
  existing two), the dataset writer (shared body), `FrameParser` (new entry mirroring an
  existing one), both script engines' result conversion, `DataTableStore` dataset writers.
  Rules preserved:
  - everything runs on the pipeline thread; engines are used only from it; the cell rows are
    a builder member, never crossing a thread;
  - no allocation in steady state on the cell lane for number-only results (Lua); JS keeps
    per-cell `QJSValue` temporaries (see Tradeoffs); the list path is unchanged as fallback;
  - `dataset.value` keeps `assign_utf8_in_place`; the store's string copy becomes in-place,
    removing a share-assign that was degrading the span lane;
  - no `lua_*` or `QJSValue` call inside a routed lambda: the scan runs at `loadScript` on the
    engine's own thread, and cell collection runs inside the existing parse call;
  - `structureGeneration` stamping is unchanged (the cell lane stages through
    `m_stager.stage` like the list path);
  - `m_captureDatasetValues` is a cached flag and gains two inputs; both are wired to the
    existing dirty/re-derive path (table above), which is the rule `common-mistakes.md` and
    dataflow.md "Cached Hotpath Flags" pin.
- **New cross-thread signal/slot?** No. Arm/disarm from GUI-side users goes through the
  existing `invokeOnBuilderThreadBlocking` marshal that `injectTableApi*` already uses.
- **New input to a cached hotpath flag?** Yes, `m_captureDatasetValues`: (1) per-engine
  table-reference flags, surfaced through `FrameParser::refreshEngineCaches` and its epoch
  (already re-derived per frame on epoch change); (2) the external-user counter, which sets
  `m_captureFlagsDirty` on the builder thread. Both documented in dataflow.md.
- **Timestamp ownership** — unchanged: rows stamp `data->timestamp + step * row` exactly as
  the list path does.
- **Benchmark plan.** Before/after on the same machine: Lua numeric, JS numeric, Lua mixed,
  JS mixed FPS; spec-0084 allocations per frame for the Lua and JS numeric rows (Lua target 0;
  JS recorded); Native rows unchanged.

## Data model & persistence

None.

## API / SDK surface

None. The `datatables` verbs are an external user and now arm/disarm capture with the API
server's enabled state instead of leaving it armed for the session.

## QML / UI

None visible. Table views arm capture while open (they already call `noteGuiUser`); the
tasks phase pairs each with a disarm.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Result representation | (a) cells with byte views into an engine scratch; (b) `QVariantList`; (c) `std::vector<std::variant<double, QString>>` | **(a)**. Reuses the span writer and its in-place UTF-8 assign; zero allocation for steady shapes; text cells are exactly what the Native lane already consumes. (b)/(c) allocate per cell or per string. |
| Eager vs lazy display text for number cells | eager format into scratch vs format on demand | **Eager** (spec open question, recommended). Keeps spec 0055 D6 untouched and makes R5 a pure formatting-equivalence test; lazy belongs to the future display-text spec. |
| JS number text | (a) `QJSValue::toString()` (allocates); (b) Qt shortest (`FloatingPointShortest`); (c) ECMAScript-compatible formatter over `std::to_chars` | **(c)**. (a) is the allocation being removed; (b) differs from JS at the exponent boundaries (`0.00001` vs `1e-05`), breaking R5. |
| JS per-cell `QJSValue` temporaries | (a) accept, record the JS floor separately; (b) prelude that packs numbers into a typed array read back in one call | **(a)** for this spec, measured. `QJSEngine` exposes no zero-copy typed-array read, so (b) still goes through `QJSValue`. R2 is met for Lua; the JS win is removing the format/parse round trip (spec open question 3 already allows this split). |
| Table-API reference detection | compile-time word scan vs runtime first-touch flag | **Scan** (spec open question, recommended). Conservative, no first-frame gap, and it matches how transmit environments already distinguish `ArmCapture` from `NamesOnly`. |
| External users | sticky bool (today) vs counter with disarm | **Counter**. R7 requires closing a user to stop capture; a bool cannot express two users. |
| Store string write | share-assign (today) vs `assign_string_in_place` | **In place**. Removes the allocation and the buffer pinning; the equality no-op check is unchanged. |
| Decoder coverage of the cell lane | PlainText only vs all decoders | **PlainText** first. The gated tiers are PlainText; Binary/Hex/Base64 scripts keep the list path and are a named follow-up. |

## Risks & mitigations

- **Formatting drift.** Lua and JS number text must match today's byte-for-byte or exports
  and dashboards change silently. Corpus tests for both rules (Lua: `%.15g` and integer
  cases against `QString::number` in the same test; JS: Node-generated fixture).
- **`QJSValue::isNumber` vs today's `toString`.** A JS `true`/`null` cell today stringifies
  to `"true"`/`"null"`; the cell lane keeps that by treating non-number, non-string values as
  text via `toString()`, same as today.
- **Lua integer detection.** `lua_isinteger` is a compat shim over LuaJIT's doubles; the cell
  lane calls the same shim so integer-valued doubles keep printing without a fraction.
- **Scan false negatives.** A script that reaches the table API through an alias
  (`local g = tableGet`) still contains the name; one built by string concatenation from
  `_G` does not. Documented in `transform_lua.md`/`transform_js.md`; the runtime bridge also
  arms capture on first call as a safety net (one-frame gap only in that pathological case).
- **Disarm site missed.** Capture stays armed (today's behavior), never off while needed.
  Tasks phase enumerates every `injectTableApi*` / `noteGuiUser` caller and pairs it.
- **Silent-breakage classes exposed** (`common-mistakes.md`): cached-flag input without a
  refresh wire (both inputs wired, tested by AC5/AC6); share-assign on the span lane (being
  removed, not added); `QList<QStringList>` from a new template (the cell lane is the
  replacement, the list path stays as fallback only); `guardedCall` for JS (unchanged).
- **Change-driven transforms.** `changedSince` semantics rely on the store's identical-value
  no-op; the in-place write keeps the equality check before the write.

## Test & verification plan

- **Unit (maintainer builds, `ctest`):**
  - AC3: `tst_script_cells` — Lua and JS parsers returning `{1, 2.5, "x", "7"}`-style mixed
    rows; per cell: `kind`, `number`, and `text` equal to what the list path produced for the
    same script (run both paths in the test), and equal to the Native span lane fed the
    equivalent delimited text for numeric detection.
  - AC4: `tst_script_cells` — 2-D results: row count, order, values equal to the list path;
    mixed scalar/vector results fall back (`parseUtf8Cells` returns `false`).
  - R2: `tst_script_cells` — Lua numeric result parsed 1000 times with a TU-local
    `operator new` counter armed after the first parse: zero.
  - R5: `tst_js_number_format` over `tests/fixtures/js-number-format.json`; Lua formatting
    cases in `tst_script_cells`.
  - R8: `tst_data_table` (existing table-store test, extended): repeated `setDatasetFinalAt`
    with changing strings of equal length allocates nothing after the first.
- **`tests/scripts/` (you can run):** existing parser units unchanged (AC2);
  `gen_js_number_corpus.py` regenerates the fixture (Node required) and a pytest asserts the
  committed fixture matches the generator's output.
- **Integration (maintainer runs, app up with API server):**
  - AC5: `test_table_capture_scoping.py` — parser-only project: `datatables` write clock
    stays at its initial value over 10 s of frames; adding a control script that calls
    `tableGet` makes the clock advance from the first frame.
  - AC6: same file — add `tableGet` to a transform while connected: the transform's output
    reflects the live value within one frame; remove it: the write clock stops.
  - AC7: `test_export_replay_fidelity.py` and the replay tests unchanged.
- **Hotpath:** `--benchmark-hotpath` before/after (AC1); the spec-0084 allocation column for
  Lua numeric reads 0; JS numeric count and FPS recorded in tasks as the JS floor.
- **Static:** `python scripts/code-verify.py --check` on every touched file (hotpath TUs
  block on violations); `qt-cpp-review` on the engines, `FrameBuilder.cpp` hunks and
  `DataTable.cpp`; `python scripts/sanitize-commit.py` before commit; `claim-verify.py`
  after the doc edits.

## Implementation deviations (2026-09-12)

- `ScriptCellRows` is a class (offset/length cells, `beginRow`/`append*`/`rowCells`/`view`);
  `IScriptEngine::parseUtf8Cells(frame, rows, fallback)` hands the list result back on `false`
  so a declining engine never runs the script twice.
- Collection is two engine-free classes, `LuaCellCollector` and `JsCellCollector`, so the unit
  tier drives them on a bare `lua_State` / `QJSEngine`; a JS non-array result also declines.
- The per-dataset transform call left `FrameBuilder.cpp` as `FrameBuilder/TransformDispatch`
  (TU census); the shared writer tail is `applyDatasetToken`, not a policy template.
- The table-API scan is header-only (`Scripting/TableApiScan.h`); `ScriptApiCall::referencesTableApi`
  delegates. Parser and transform engines install names only and never arm.
- External users: `injectTableApi*` (++) / `releaseTableApiUser()` (--) / `TableApiUserLease`.
- Builder does one `datasetSlots()` lookup per dataset per frame instead of an ordinal cache.
- Integration observable: `project.dataTable.getValue` on `__datasets__/raw:<id>` (a non-arming
  reader) instead of a write-clock verb, which does not exist.

## Review fixes (qt-cpp-review, 2026-09-12)

- `applyProjectSnapshot` no longer zeroes `m_externalTableUsers` (leases outlive a library edit);
  `releaseTableApiUser()` AND both `injectTableApi*` arms are queued posts. A blocking arm is
  skipped by `runOnObjectThread` when the worker's loop is unwound by `QThread::quit()` (pool
  rebuilt mid-inject on the field-project load, 2026-09-12): the worker's armed flag stayed set, the
  later release hit zero and the debug assert aborted. A blocking release also nested a GUI loop
  inside `~Painter` / `~Output::Base`.
- `splitScientific` read the exponent with `atoi` past an unterminated buffer: bounded parse.
- `formatLuaNumber` spells non-finite values as Qt did (`nan` unsigned, `inf`, `-inf`); Lua
  strings stop at the first NUL like `QString::fromUtf8(const char*)` did.
- The Apple pre-13.3 `shortestScientific` fallback is allocation-free (`snprintf_l` precision
  search with `strtod_l` round-trip) instead of `QByteArray::number`.
- Both collectors walk the result once (first element decides flat vs nested; a mismatch declines
  on the same walk); JS reads `length` once and text cells encode UTF-16 straight into the scratch
  (`ScriptCellRows::appendUtf16`).
- `FrameParser::parseCellsUtf8` declines Native engines without running them, so a rejected Native
  frame parses once through the list path instead of three times.
- `installTableApiNames` / `installTableApiNamesLua` are out of line and stubbed in
  `tst_stream_worker` with `releaseTableApiUser`; the worker uses the header-only scan.
- `~Output::Base` releases only when the surface was prepared (`m_sourceId >= 0`).
- `ScriptCellRows::clear()` releases capacity past a high-water mark so one pathological result
  does not pin memory for the session.
- Allocation tests probe address stability of the scratch, the cell array and the store's register
  buffer (Qt containers and LuaJIT allocate outside operator new).
- Not addressed, maintainer call: `applyDatasetToken` is the out-of-line shared tail of the span
  and cell lanes; if `--benchmark-hotpath` shows the Native 4x tier moved, mark it
  `SS_FORCE_INLINE` in `.h`/`.cpp` lockstep. `tst_stream_worker` also references the spec-0083
  `TransformCompiler` helpers (`loadLuaLibraryChunk`, `pushTransformParams`,
  `transformParamsJson`), which are outside this spec's diff.

