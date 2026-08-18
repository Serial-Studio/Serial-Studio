# Architecture — Frame Parsers, Transforms, Data Tables & Control Scripts

> Part of the architecture corpus ([index](../architecture.md)). Read this file in full
> before touching the JS/Lua/Native parser engines, per-dataset transforms, the data-table
> store, the control script, or the code-editor QML.

## Lua Runtime — LuaJIT (spec 0051)

The vendored runtime is **LuaJIT 2.1** (`lib/luajit/`, pinned via `.relver`; replaced Lua 5.4
2026-08). The user-facing script surface is **Lua 5.1 + shims**: `LUAJIT_ENABLE_LUA52COMPAT`
is on, `LuaCompat.cpp` restores 5.2-era names plus a `bit32` table over the native `bit`
library, and `LuaCompatJIT.h` restores the newer C-API subset (`lua_rawlen`, `lua_isinteger`,
`lua_geti/seti`, `luaL_len/requiref/tolstring`, `lua_pushglobaltable`) under canonical names —
include it after the Lua headers in every VM TU, and wrap those Lua includes in `extern "C"`
(LuaJIT is C; the old 5.4 tree was compiled as C++). **5.3 bitwise/floor-division syntax
(`<< >> & | ~ //`) does not parse**: shipped scripts and both importer generators were
migrated to `bit.*`/`math.floor` (values wider than 32 bits accumulate in plain arithmetic —
exact to 2^53, the pipeline's double ceiling), and a user script that fails compilation with
that syntax gets an enriched error naming the replacement. `string.pack/unpack` do not exist.
Per-project **Safe/Fast** mode (`ProjectModel::luaFastMode`, consent-gated in the UI):
Safe = interpreter + count-hook watchdog (default); Fast = JIT on, watchdog structurally off
(hooks never fire inside compiled traces — measured, which is why it is one mode switch, not
two flags). Macros and the MQTT publisher are pinned Safe regardless (macro Lua runs on the
GUI thread). `ffi` and `jit` modules are never opened in any sandbox, and every state-setup
path runs under a protected bootstrap so `lua_atpanic` is unreachable.

## Frame Parser — Three Languages (JS + Lua + Native)

- `IScriptEngine` is the abstraction. Three impls:
  - `JsScriptEngine` — `QJSEngine` with `ConsoleExtension + GarbageCollectionExtension`
    only (**not** `AllExtensions`). Watchdog: **always route through
    `JsScriptEngine::guardedCall()`** (concrete engine — the `IScriptEngine` interface does
    not carry it); never call `parseFunction.call()` directly.
  - `LuaScriptEngine` — LuaJIT (`lib/luajit`), one `lua_State*` per source.
  - `CFrameParser` — native C++ parametrized templates (`SerialStudio::Native = 2`). The
    "script" is a canonical JSON descriptor `{"params": {...}, "template": "<id>"}` built by
    `CFrameParser::buildDescriptor()` (compact + key-sorted, so the BackupManager SHA stays
    byte-stable). Template registry: `Scripting/NativeTemplates/` — stateless
    `INativeTemplate` descriptors (id, tr()'d metadata, param schema, `makeParser()`) +
    per-source stateful `INativeParser` instances (latch buffers live in the instance, never
    in the registry singleton). Catalog/schema for QML + AI:
    `CFrameParser::templateCatalog()` / `templateSchema(id)`.
- **Engine mismatch detection uses `IScriptEngine::language()`** (the old `dynamic_cast`
  bool check silently broke with 3 languages). `FrameParser::parseMultiFrame*` never falls
  back to source 0 across language boundaries.
- Native config persists in `Source::frameParserTemplate` (string id) +
  `Source::frameParserParams` (JSON object). `FrameParser::scriptForSource()` builds the
  descriptor for native sources (empty template id falls back to the default `delimited`
  comma config).
- **Language switches convert the template, both directions.**
  `FrameParser::nativeEquivalentForFile()` / `fileForNativeTemplate()` map script template
  file basenames ⇄ native ids (+ `delimited` separator params for the CSV/TSV/pipe/semicolon
  variants); `JsCodeEditor::switchNativeLanguage` uses them, so JS → Native → Lua lands on
  the equivalent Lua template, never stale wrong-language code. Custom code that matches no
  template falls back to the default template (same semantics as the JS ↔ Lua switch).
- UI: `SourceFrameParserView.qml` is **the only parser editor view** — every parser tree
  item (source 0 included) routes through `selectSourceParserItem` →
  `SourceFrameParserView` (the old project-level `FrameParserView.qml` was dead code and
  was deleted). In native mode it swaps the code editor for `NativeParserPane.qml` (param
  form + Markdown documentation page) and hides the code-editor toolbar row; the native
  template combo lives in the secondary toolbar next to a Help button and "Test With
  Sample Data". Per-template docs ship at
  `app/rcc/scripts/native/<id>.md` (exposed via
  `FrameParserModel::templateDocumentation`). The bridge is
  `DataModel::FrameParserModel` (registered as QML type `FrameParserModel` in the
  `SerialStudio` module, see ModuleManager.cpp:545).
- **The frame parser test dialog is QML** (`app/qml/Dialogs/FrameParserTest.qml`, one dialog
  for all three languages), backed by the same `FrameParserModel` bridge: pipeline
  get/setters write through `ProjectModel::updateSource`, and `dryRun()` branches by source
  language (JS/Lua → live engines via `runFrameParserPipeline`, Native →
  `runNativeTemplatePipeline`). The old QWidget `FrameParserTestDialog` was deleted;
  `JsCodeEditor::prepareParserTest()` only loads/validates the script before QML opens
  the dialog. **The dialog is hosted by a `DialogLoader` in `ProjectEditor.qml`
  (`parserTestLoader.openTester(sourceId)`)** — never instantiate it inside the
  Loader-managed views, or any `updateSource` triggered from the dialog rebuilds the view
  and destroys the window mid-interaction.
- **JS interruption is cross-thread.** A `QTimer` on the thread running `QJSValue::call()`
  can never fire — the event loop is blocked while the script runs (this was a real,
  shipped no-op against `while(true){}`). `JsWatchdogThread` (a dedicated `QThread` polling
  armed `JsWatchdog`s every 20 ms) flips `setInterrupted(true)` from off-thread, which Qt
  documents as thread-safe. Lua uses an in-engine `LUA_MASKCOUNT` hook + `QDeadlineTimer`
  instead. Every JS engine (parser, transform, Painter, Output, MQTT) holds a `JsWatchdog`
  that registers with the thread; `arm()`/`disarm()` are lock-free atomic-deadline stores
  safe on the hotpath. **`setInterrupted(true)` may appear only in `JsWatchdogThread.cpp`**
  — `code-verify.py:js-interrupt-off-thread` blocks it anywhere else.
- Per-source `frameParserLanguage` (0 = JS, 1 = Lua, 2 = Native) picks the engine in
  `FrameParser::engineForSource()`. JS/Lua templates in `app/rcc/scripts/parser/{js,lua}/`
  + `templates.json`; native templates are compiled in (`nativeTemplates()` registry,
  delimited/comma is the default).
- **New projects/sources default to Native CSV** (`seedDefaultFrameParser` in
  ProjectModel.cpp: language = Native, template = delimited, params = schema defaults).
  `frameParserCode` is not seeded; the switch mapping generates the equivalent script
  template on demand.

**Binary-decoder scripts get byte tables, not strings.** Writing string-based byte access
(`string.byte`, `string.unpack`, `charCodeAt`, `split`) in a Lua/JS parser that runs under
the **Binary decoder** — including the Lua templates the importers generate (`DBCImporter`,
`ModbusMapImporter`) — breaks every frame: with `SerialStudio::Binary`,
`LuaScriptEngine::parseBinary` pushes the frame as a **1-indexed table of byte integers** and
`JsScriptEngine::parseBinary` passes an `Array` of bytes — not a string. Every frame dies with
`bad argument #1 to 'byte' (string expected, got table)` (Lua) or `frame.split is not a
function` (JS). Index the table/Array directly (`frame[1]`, sawtooth/LSB walks like the DBC
template); when `string.unpack` is genuinely needed (multi-byte/float fields, Modbus), convert
once at the top of `parse()`:
`if type(frame) == "table" then frame = string.char(table.unpack(frame)) end`. The shipped
parser templates (`app/rcc/scripts/parser/`) and `ProtoImporter`'s `bytesToString` are the
reference patterns; both importers shipped this bug in 2026-06.

## Embedded Code Editors — Hidden-Widget Plumbing

The QML-embedded code editors (`JsCodeEditor` & siblings) are an offscreen `QCodeEditor`
grabbed into a `QQuickPaintedItem`, so three invariants hold them together:

- `renderWidget()` first calls `syncWidgetPosition()` (moves the hidden top-level to the
  item's screen position — completer popups and drag auto-scroll resolve global coordinates
  through it).
- `event()` forwards `ShortcutOverride` to the widget (editing keys handled natively —
  without it QML `Shortcut` soup decides, and duplicated sequences across the same window go
  *ambiguous* and silently dead).
- `keyPressEvent` reroutes to `completer()->popup()` while it's visible (the popup never has
  real focus in the embedded setup).

`JsCodeEditor` & the other project-editor siblings call `renderWidget()` **directly** from their
input handlers and grab unconditionally on `uiTimeout` — dropping that re-introduces one-timer-tick
lag on every keystroke/drag. They live in modal editors, so the cost is bounded by the dialog.

**`MacroEditor` is the exception and must stay one (2026-08-17).** It is embedded in the main
window (Macros dialog, and the console annotations decoder tab), where an unconditional
`m_widget.grab()` per tick cost **13% of the main thread plus a per-frame texture upload for the
whole session** — measured with `sample`, not guessed. Handlers there call `scheduleRender()`
(sets `m_dirty`), the `uiTimeout` slot grabs only when `isVisible() && (m_dirty ||
hasActiveFocus())`, and the widget's `textChanged` / `selectionChanged` / `cursorPositionChanged`
feed the same flag so undo, paste, `clear()` and `selectAll()` still repaint. Focus keeps the caret
blinking; unfocused steady state costs zero grabs. Never re-wire an embedded editor's tick to grab
unconditionally — check with a sample first if you think you need to.

## Per-Dataset Value Transforms

- Each dataset may carry a `transformCode` string (language matches its source). The user
  defines `function transform(value) -> number`.
- **Compile-once, call-many.** `FrameBuilder::compileTransforms()` runs on project load /
  connection open. One Lua state or `QJSEngine` per source; per-dataset function refs cached
  in `m_transformEngines`.
- **Lua isolation**: `luaL_dostring` once; top-level `local`s become upvalues in the
  `transform` closure, so two datasets sharing the same Lua state don't clobber each other.
- **JS isolation**: user code is wrapped in an IIFE at compile time so top-level `var`s are
  closure-scoped per dataset.
- **Hotpath**: `applyTransform(language, uniqueId, rawValue, info)` → cached per-source
  engine pointer (`m_luaEngineForSource` / `m_jsEngineForSource`, refreshed on `sourceId`
  change in `applyDatasetValues`; **no `std::map::find` per dataset per frame**) →
  `lua_pcall` / `QJSValue::call`. Single-arg transforms skip the info table / object
  allocation: `acceptsInfo` is detected at compile time via `lua_getinfo(">u")` (Lua) and
  `function.length` (JS) and stored on the per-dataset ref.
- **JS watchdog is frame-level**, not per-call. `applyDatasetValues` arms the active source's
  per-engine watchdog (`m_jsEngineForSource->jsWatchdog->arm()`) once, runs the dataset loop,
  and disarms it. The 100 ms budget covers the whole frame's transforms collectively, and the
  interrupt is delivered off-thread by `JsWatchdogThread` (see Frame Parser). On timeout
  `applyTransformJs` sets `m_jsTransformTimedOut`; the user-facing notification is posted once
  from the main thread after the loop, never from the watchdog thread.
- Non-finite numeric results are rejected (`[[unlikely]]` guarded) and `rawValue` is returned.
- **Stream-lane datasets run their transform in the `IO::StreamWorker`, not here (spec 0051).**
  That engine is worker-owned (same safe-lib sandbox, same Safe/Fast mode) and prefers
  `transform_block(samples, info) -> samples`, called ONCE per captured block with the
  dataset's samples; `info` carries exactly `sourceId`, `uniqueId`, `blockNumber`,
  `timestampMs`, `sampleRate`, `count`, `firstSampleIndex` (frozen at ship). A dataset that
  only defines `transform(value)` still works — it is called per sample on the worker, at full
  rate. A failing or watchdog-aborted block falls back to raw samples and counts an error.
- **Editor**: `DatasetTransformEditor` prefills a multiline-comment placeholder when the
  dataset has no transform; `onApply` runs `validateTransform(code, language, error)` which
  returns a `TransformStatus` (`Ok` / `SyntaxError` / `NoFunction`). `SyntaxError` and
  `NoFunction` (the placeholder or any code that doesn't define `transform()`) block
  persistence and keep the dialog open with a warning, so the placeholder never persists.
- **Expression transforms (spec 0060, `transformLanguage == SerialStudio::Expression`).** A
  third mode with no script engine: `DataModel::Expression` (`Scripting/ExpressionTransform.h`)
  compiles one arithmetic expression (`v t n dt`, `+ - * / % ^`, comparisons, `? :`, a fixed
  function set, siblings by Script Alias or `{uniqueId}` / `{aliased name}`, `sample(name, k)` up to
  256) into a flat postfix `Program`; `evaluate()` is a fixed-array stack machine, allocation-
  free, bounded by the program length (`kMaxNodes` 512). One `SlotTable` per source holds the
  latest published value + a history ring for every dataset any expression of that source
  refers to; every dataset publishes its final value into it (`FrameBuilder::applyDatasetValue*`
  behind `if (m_exprEngineForSource) [[unlikely]]`), so a sibling reads "the latest published
  value" — a sibling processed later in dataset order yields the previous frame's value. The
  stream lane runs the same `Runtime::run` in `StreamProcessor::processExpressionChannels`, a
  sample-major second pass after the ordinary channels, publishing per sample, so both lanes
  are bit-identical by construction. Compile errors are `noteTransformError`'d once (dataset
  publishes raw); expression-only projects do not turn on the table-store capture flag.
  `table(name, register)` reads a DataTableStore register **read-only**, resolved to a store
  handle at compile time and read per sample through a thunk on the owning thread; the resolver
  is installed by the frame lane and the editor only, so `table()` on a stream-lane source is a
  compile error ("not available for this source") rather than a cross-thread read.
  `#` runs to end of line as a comment (`#` starts no token, so unlike `//` it cannot collide
  with division). Editor: third combo row, `DataModel::ExpressionHighlighter` (comments, numbers,
  braced titles, built-ins, call names — the JS highlighter coloured keywords this language does
  not have), the template combo is **disabled** for Expression because templates only carry
  `luaCode`/`jsCode`, the placeholder documents the whole language and ends on `v` so the default
  still compiles, and the live test compiles against the project's titles. Note `sample()` takes a
  bare or `{braced}` name, never a quoted string. Benchmark: ungated `HOTPATH_STREAM_EXPR_FPS` in
  the stream phase.

## Data Tables — Central Data Bus

- `DataModel::DataTableStore`: flat pre-allocated register store, **no hotpath allocation**.
- **System table `__datasets__`**: auto-generated. Two registers per dataset:
  `raw:<uniqueId>`, `final:<uniqueId>`. Populated by FrameBuilder during parsing.
- **User tables**: defined in project JSON under `"tables"`. Registers are `Constant`
  (read-only at runtime) or `Computed` (writable by transforms, **persists across frames** —
  no automatic per-frame reset). The `defaultValue` is the value at project load only.
  Computed registers are the natural place for filter state, integrators, edge counters,
  and latched flags; for a "clear me each frame" effect, the transform writes the reset
  value explicitly at the top of an early dataset.
- **Transform API** (injected at compile time): `tableGet`, `tableSet`, `datasetGetRaw`,
  `datasetGetFinal`. Lua = C closures; JS = `TableApiBridge` QObject.
- **Thread routing (spec 0051 M5) — one rule, both languages.** The store's single writer is the
  pipeline thread. `DataModel::readTableView` / `writeTableStore` (templates in `DataTable.h`)
  are the *only* definition of how a script reaches it, and both the JS `TableApiBridge` and
  the Lua C closures go through them, carrying the same `DataModel::TableApiContext`
  (`store` + `owner` + `mirror`). Reads: live store when the caller owns it (parser and dataset
  transforms), the GUI-side `DataTableSnapshot` mirror on the GUI thread (painters, editors,
  dialogs), the live store behind `PipelineHost::runOnObjectThread` for any other worker
  (control script, macros, stream-lane transforms). Writes: direct / queued fire-and-forget from the GUI / blocking
  elsewhere. **Never make a GUI-thread read block on the pipeline thread** — the wait spins a
  nested `QEventLoop` that fires the display tick and re-enters the very script that is
  mid-call, and at 26 reads per painter frame it blew the 250 ms watchdog outright.
- **The mirror.** `FrameBuilder::publishTableSnapshot()` (pipeline) copies the store into a
  4-slot SPSC ring only when `generation`/`writeClock` moved; `drainTableSnapshot()` (GUI)
  adopts the newest and requests the next, once per display tick, before `Dashboard::updated()`.
  It is one tick stale by construction, and armed only by `noteGuiTableApiUser()` — i.e. when a
  script engine is wired up *on the GUI thread*, so pipeline-side transform engines never make
  the builder pay for a snapshot nobody reads.
- **Lua closures: resolve, route, then push.** A `lua_State` belongs to one thread, so no
  `lua_*` call may appear inside a routed lambda — read the arguments, route the pure store
  access, push the result after. This is also why `luaDatasetSelector` runs `luaL_checkinteger`
  up front: its error `longjmp` must unwind the Lua state's own thread, not the pipeline's.
  The interned-pointer fast paths (`getByInternedKey`, `getDataset*ByAliasInterned`) are valid
  **only** on the store thread — they key on raw `lua_State` string pointers, which mean nothing
  across states — so off-thread callers take the QString-keyed lookups instead.
- **Processing order**: group-array then dataset-array. A transform sees raw of ALL datasets,
  final of EARLIER datasets only.
- `applyTransform` returns `QVariant` (double or QString). `Dataset` has
  `rawNumericValue`/`rawValue` snapshots and `virtual_` (no frame index — transform-only).

## Control Script — Worker Thread, apiCall Only

The control script runs on a worker thread with apiCall only: `ControlScriptWorker` evaluates
setup()/loop() in its own QJSEngine and installs ONLY `__ss_bridge` (apiCall marshalled to the
GUI thread via `BlockingQueuedConnection`) — never the direct helper bridges (`__ss`,
`__ss_db`, ... wrap main-thread singletons and must not execute off-thread). The SDK prelude
(`app/rcc/api/prelude.js`, embedded into `SerialStudio.js` by `scripts/generate-sdk.py`)
defines control-mode fallbacks that re-route the friendly globals through apiCall:
`notify*` → `notifications.*`, `tableGet`/`tableSet` → `project.dataTable.getValue`/`setValue`
(live store values; `DataTablesHandler`, backed by `FrameBuilder::tableStore()`).
`datasetGetRaw`/`datasetGetFinal` remain parser/transform-only. `io.getLatestFrame` returns
`ageMs` (steady-clock ms since capture) for staleness watchdogs — never compare its monotonic
`timestampMs` against `Date.now()`. A new control-script global must follow the apiCall
fallback pattern; installing a direct bridge on the worker engine is a threading bug. Dataset
transforms re-run only on frame arrival, so table writes made while the device is silent
don't render until the next frame. Two SDK calls close that gap, both running the same
transform-only pass (`reprocessDatasetValues`) over the live frames (per-source frames when
populated, else `m_frame`) and sharing the private `republishFrames(bool feedExports)` helper:
`refreshDashboard()` (`dashboard.reprocess` → `FrameBuilder::reprocessFrames`, `feedExports`
false) runs **synchronously** and publishes to `Dashboard::hotpathRxFrame` directly, skipping
the sink fan-out so a synthetic refresh never re-records samples already
exported on arrival; `dashboardTick()` (`dashboard.tick` → `FrameBuilder::dashboardTick`,
`feedExports` true) runs **synchronously**: the call seeds the source frames (from the project
template when none has arrived yet, so it works from the very first `loop()`) and runs one
`republishFrames(true)` immediately, publishing *through* `publishBlock` so a table-driven
control-script simulation both renders and feeds the CSV/MDF4/session/MQTT/API exports (still
gated on `m_anyAsyncSink`). Every tick renders its own frame, so a per-frame control-script
curve (a Lorenz attractor, for example) is not decimated. The `dashboard.tick` response
reports `published`. (Spec 0023 briefly deferred and coalesced this into the UI-timer window;
that was reverted because it jagged per-frame curves. CSV interval-snapshot mode remains the
export-rate bound.)

## Control Script — Per-Connection Lifecycle

The control-script lifecycle is per-connection and force-restarted: `ControlScript`
edge-tracks `shouldRun()` (`m_shouldRun`) and on every rising edge queues stop-then-start, so
each connection gets a fresh engine (top-level state resets, setup() re-runs). `stopWorker()`
always queues the idempotent worker stop even when `m_running` is false — a worker/GUI
desync must never keep an old engine alive across a cycle — and a setup() exception stops
the worker (loop never arms with the GUI showing stopped). `FrameBuilder::onConnectedChanged`
clears `m_latestFrames` on BOTH edges: with the API server on, `m_captureLatestFrame` stays
true across a disconnect, and a stale retained frame would otherwise leak into the next
connection's `io.getLatestFrame`/`newFrame()`. `controlscript.dryRun` compile-checks source
in a throwaway GUI-thread engine (stub `__ss_bridge` + SDK prelude + `JsWatchdog`) without
installing it; `controlscript.getCode`/`setCode` are registry aliases of `get`/`set`. The
agent-facing globals reference is `:/ai/docs/control_script_js.md`
(`meta.fetchScriptingDocs{kind:'control_script_js'}`; allow-lists in
`ContextBuilder::scriptingDocFor` AND `ToolDispatcher::getScriptingDocs`, plus `rcc.qrc`).
