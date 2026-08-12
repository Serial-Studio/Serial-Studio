---
spec: 0051-stream-lane-and-parse-budget
phase: plan
status: approved     # draft -> approved (gate before /ss-tasks)
updated: 2026-08-11
---

# Plan 0051 — Per-source parse budget + typed stream lane + off-GUI pipeline + LuaJIT

> **Phase 2 of 4 — the HOW.** Satisfies every requirement in [`spec.md`](./spec.md).
> Grounded in: dataflow.md, io.md, dashboard.md, export.md, startup.md, scripting.md,
> common-mistakes.md read in full; FrameReader.cpp, CircularBuffer.h, FrameBuilder.h/.cpp,
> Audio input path, Dashboard ingestion read in full; seven targeted code surveys
> (export workers, API server, project properties, Lua inventory, driver contract,
> dashboard rings, startup/threading seams) with file:line receipts.

## Approach (one paragraph)

Five workstreams, shipped as ordered milestones. **M1** replaces the global fixed-window
parse breaker with per-source EWMA fair-share accounting and proportional decimation
(R1–R5). **M2** swaps the vendored Lua 5.4 for vendored LuaJIT with a per-project
Safe/Fast execution mode, migrating the 13 shipped 5.3-syntax scripts and the DBC
generator, and re-establishing the never-aborts-host contract on LuaJIT's error model
(R19–R23). **M3** relocates the frame pipeline off the GUI thread as one unit: a new
`IO::PipelineHost` owns a processing `QThread`; FrameReaders, FrameParser engines, and
FrameBuilder move there wholesale; the Dashboard consumes finished pooled frames from an
SPSC ring drained on the UI tick (R16–R18). **M4–M5** add the typed stream lane: audio
publishes `SampleBlock`s (no CSV text), one `IO::StreamWorker` thread per stream source
runs block/per-sample transforms + per-pixel envelope reduction + full-rate typed
exports, and the Dashboard ingests bounded per-tick display updates (R6–R15). **M6**
adds the block-rate API subscription with drop-oldest-plus-missed-count backpressure
(R24). Chosen shape: *relocate-wholesale* for the frame pipeline (preserves every
single-thread invariant by construction, smallest diff of the three candidates) and
*worker-per-source* for streams (matches per-source engine/state isolation; scales by
construction).

## Architecture & data flow

### M3 — frame pipeline on the processing thread

```
Driver (any thread)
  │ dataReceived(CapturedDataPtr)          Auto → resolves QUEUED (chunk rate, not frame rate)
  ▼
FrameReader::processData        [processing thread]  ← moveToThread at creation
  │ SPSC queue (unchanged), readyRead
  ▼
PipelineHost::onReadyRead       [processing thread]  DirectConnection (same thread)
  │ drains queue; routes by cached operation mode (atomic mirror)
  ▼
FrameBuilder::hotpathRx*        [processing thread]  plain call (today's ConnectionManager
  │ parse → transforms → pool slot                    routing logic relocated into PipelineHost)
  ├─► Dashboard SPSC frame ring ──► GUI uiTimeout tick drains → Dashboard::hotpathRxFrame
  └─► hotpathTxFrame fan-out: detached copy → CSV/MDF4/API/Sessions/MQTT/gRPC
      (their SPSC queues keep exactly ONE producer — now the processing thread)
```

- `PipelineHost` is a new SessionContext-adopted module (census re-baseline). It owns
  the `QThread`, the Dashboard-bound SPSC ring (`ReaderWriterQueue<TimestampedFramePtr>`,
  capacity 8192 = pool size), and the relocated mode-routing from
  `ConnectionManager::onFrameReady` (ConnectionManager.cpp:1749-1752).
- `DeviceManager`/`ConnectionManager` stay on the GUI thread for lifecycle/config; only
  each `FrameReader` (parentless, DeviceManager.cpp:45/129/157) is `moveToThread`'d to
  the processing thread on creation. `resetFrameReader`/`reconfigure` destroy-and-
  recreate via queued invocation on the processing thread (recreation is already the
  contract; no mutexes added).
- `FrameBuilder` and `FrameParser` are constructed at the composition root as today,
  then `moveToThread` after `setupExternalConnections()`. All their queued slot wiring
  (player lambdas, sync signals, compile defers) automatically executes on the
  processing thread — Qt routes by affinity; in-pipeline hops stay direct calls.
- GUI→pipeline entry points become queued or blocking-queued invocations:
  `reprocessFrames`/`dashboardTick` (control script; `BlockingQueuedConnection`,
  matching the ControlScriptWorker apiCall precedent), table-store API verbs
  (`DataTablesHandler`), `FrameParserModel::dryRun` live-engine path, player
  `replayChannels*` calls (block-shaped already).
- Dashboard drain: the existing `uiTimeout` lambda (Dashboard.cpp:248-253) gains a
  drain loop ahead of the `updated()` emission: pop all pending `TimestampedFramePtr`,
  call `hotpathRxFrame` for each, then coalesce. Pool backpressure semantics unchanged:
  a stalled GUI pins slots → pool exhausts → existing heap-fallback warning.
- Cached flags: `m_operationMode`, `m_playerOpen`, `m_anyAsyncSink`,
  `m_captureLatestFrame`, `m_changeDriven` stay plain members of FrameBuilder — their
  refresh slots now execute on the processing thread via the same connections
  (auto-queued cross-thread). Ordering vs. in-flight frames is FIFO within the
  pipeline's event queue, so a refresh can lag emission by queued hops but can never be
  torn or lost; the dataflow.md "DirectConnection refresh" rule is superseded *for the
  cross-thread edges only* and the doc gets updated. `PipelineHost` keeps atomic
  mirrors of `operationMode` and `isConnected` for its routing + the MDF4 worker fix.
- The debug-only `Q_ASSERT(m_operationMode == AppState::instance().operationMode())`
  (FrameBuilder.cpp:759) is removed (cross-thread singleton poll).
- Benchmark: `HotpathBenchmark` runs its drive loop *on the processing thread* via a
  blocking dispatch, so gated numbers measure the same single-threaded pipeline;
  dashboard-floor phases exercise the new ring+drain path.

### M4 — stream lane

```
miniaudio RT callback ── SPSC (raw PCM, exists) ──► StreamWorker thread [per stream source]
  10 ms timer NOW WORKER-AFFINE (today's timer ticks a MAIN-thread processInputBuffer —
  Audio.cpp:674-691 receiver is `this`; the CSV text encode currently runs on the GUI thread)
  │ decode PCM → planar float64 scratch (reused)
  │ per dataset: transform_block (one pcall, samples table reused) | per-sample transform loop
  │ ├─► envelope reducer: min/max pairs on the TimeRing::appendDecimated grid,
  │ │     bucket count = plot pixel width (atomic int written by GUI on resize)
  │ ├─► FFT window ring (worker-side AxisData, capacity = normalizedFftSize)
  │ ├─► latest values (per dataset)
  │ ├─► typed export block → FrameConsumer<StreamBlockItem> (MDF4 native, CSV numeric rows)
  │ ├─► API block ring (per subscribed client, drop-oldest + missed counter)
  │ └─► table-store publish (block rate, queued to processing thread)
  ▼
StreamDisplayUpdate SPSC ──► GUI uiTimeout drain (bounded: pixels + fft window + values)
```

- `IO::SampleBlock` (new, HAL_Driver.h): `std::vector<float> samples` (interleaved),
  `int channels`, `qsizetype frames`, `SteadyTimePoint t0`, `nanoseconds dt`. Published
  via new `HAL_Driver::publishSampleBlock()`; `HAL_Driver::isStreamCapable()` default
  false, Audio overrides true. Effective lane = driver default overridden by the
  per-source project flag (R6): `"auto"` (driver decides) / `"on"` / `"off"`.
- Stream sources bypass FrameReader/FrameBuilder entirely. Dashboard structure still
  comes from the template-frame publish at connect (`publishSourceTemplateFrame`,
  FrameBuilder.cpp:837) so widget models build; live values then arrive only via the
  display-update path. FFT/plot ring *capacities* keep deriving from project config
  exactly as today (Dashboard.cpp:2803-2817, 74-91).
- Dashboard gains a stream ingest surface: `applyStreamUpdate(...)` — appendDecimated
  the envelope pairs into the source's TimeRings (clock continuity: worker supplies
  monotonic block times; per-source PlotClock advanced from block t0, never cleared —
  the `bulkLoadPlotWindow` clear/re-anchor semantics are explicitly NOT reused),
  memcpy the FFT ring content, write latest dataset values, set `m_updateRequired`.
  All bounded per tick, independent of sample rate (R7).
- Engines: StreamWorker owns its Lua state / QJSEngine (built by the existing
  compile-transforms machinery refactored into a shared helper), armed with the same
  watchdog discipline (Safe: hook+deadline; Fast: none). `transform_block` detected at
  compile like `acceptsInfo` today (arity probe); `info` table per R9, reused per call.
- Teardown: StreamWorkers follow the `FrameConsumer` stop pattern (blocking close →
  `quit()` → `wait()`), joined from `ModuleManager::stopFrameConsumerWorkers()`
  alongside the eight existing workers; Fast-mode hang → `wait(timeout)` then
  warn-and-abandon (spec R21, 0046 precedent). Fixes the existing double-`deleteLater`
  on the audio input timer while that code is rewritten.

### M1 — budget (implemented pre-M3 on the GUI thread, unchanged by the move)

Per-source `BudgetEntry { ewmaNs, decimateN, counter }` hashed by sourceId in
FrameBuilder. `parseBudgetAccount(sourceId, elapsed)` updates the source EWMA and a
thread-total EWMA (tau ≈ 250 ms). `parseBudgetSkipFrame(sourceId)`: if total < 90% of
one core → never skip; else sources whose share exceeds `total_budget / active_sources`
get `decimateN = ceil(sourceLoad / fairShare)` and pass every Nth frame. Counters are
plain quint64 polled at 1 Hz (spec 0033 pattern): a new `ConnectionDiagnostics` checker
raises the ProblemCenter entry with per-source load; Dashboard exposes
`streamThinning` (polled property) for the QML indicator. Nothing on the frame path
signals or allocates.

### M2 — LuaJIT

- `lib/luajit/` vendored (pinned release), built via a CMake wrapper replicating
  LuaJIT's two-stage build (host minilua/buildvm → per-arch VM); `lib/lua/` (5.4)
  deleted; `configure_third_party_lib` + link sites updated. The 5.4-as-C++ unwind
  machinery (lib/lua/CMakeLists.txt:47-114) and its cmake ecosystem constraints
  (Optimization.cmake:53-61, Hardening.cmake:141 pac-ret note) are retired; docs
  updated.
- Error model: lua_atpanic must be unreachable — audit every VM entry: execution paths
  already run under `lua_pcall`/status checks; state-setup paths (library injection,
  compile) get wrapped in one protected bootstrap C function per engine. The 16
  now-vestigial catch-walls shrink to plain status handling.
- API drift shims (one `LuaCompatJIT.h`): `lua_isinteger` → integral-double probe that
  preserves today's int-vs-float display formatting; `luaL_len/lua_geti/lua_seti/
  lua_rawlen/luaL_requiref` → LuaJIT equivalents (5.2 compat functions LuaJIT already
  ships under `LUAJIT_ENABLE_LUA52COMPAT`, which we enable).
- `LuaCompat.cpp` inverts: `bit32` + convenience names re-implemented over LuaJIT's
  native `bit`; `ffi`/`jit` never opened in sandboxes (Fast mode toggles the engine via
  `luaJIT_setmode`, not via the `jit` module).
- Script migration: 13 shipped parsers + DBCImporter generator rewritten to
  `bit.band/bor/bxor/lshift/rshift` + `math.floor` division. Load-time diagnostics: on
  compile error, source is scanned for 5.3 operators and the error is enriched with the
  construct + replacement (R22).
- Mode plumbing: project-level property `luaExecutionMode` ("safe"/"fast") following
  the changeDrivenTransforms 10-site pattern; consent dialog on first flip to Fast;
  engines read it at compile time (Safe: `LUAJIT_MODE_OFF` + LUA_MASKCOUNT hook +
  deadline; Fast: `LUAJIT_MODE_ON`, no hook).

### M6 — block API

Connection-scoped verbs `stream.subscribe` / `stream.unsubscribe` (mirror-style
pre-registry dispatch, Server.cpp:1380 precedent). Per-subscriber ring on the
StreamWorker (drop-oldest; `missed` counter accumulates and is reported on the next
delivered block). Wire: NDJSON line `{"streamBlock": {sourceId, uniqueId, seq, missed,
t0Ms, dtNs, count, data: <base64 float32le>}}`, delivered through the existing
ServerWorker queued-write path with sessionId revalidation. SDK/schema regenerate via
the standard `--dump-api-schema` → `generate-sdk.py` chain.

## Affected subsystems & files

| File | Change |
|------|--------|
| `app/src/DataModel/FrameBuilder.h/.cpp` | M1 per-source budget (replaces global breaker); M2 transform-engine LuaJIT bootstrap + mode; M3 thread affinity, marshaled entry points, Q_ASSERT removal; compile-transforms helper extraction for stream workers |
| `app/src/DataModel/ParseBudget.h` (new) | Header-only fair-share budget engine (no Qt) so the ctest tier links it directly; FrameBuilder delegates (amended 2026-08-11, maintainer approved — FrameBuilder.cpp has no unit-tier link set) |
| `app/src/IO/PipelineHost.h/.cpp` (new) | Processing thread owner: drain/route (from ConnectionManager.cpp:1749), Dashboard frame ring, atomic mode/connected mirrors |
| `app/src/IO/StreamWorker.h/.cpp` (new) | Per-source stream worker: decode, transforms, envelope, FFT ring, latest values, export/API/table fan-out |
| `app/src/IO/HAL_Driver.h` | `SampleBlock`, `publishSampleBlock()`, `isStreamCapable()` |
| `app/src/IO/Drivers/Audio.h/.cpp` | Typed block publish; delete CSV encode + csv stream members; worker-affine cadence; teardown rewrite (fixes double-deleteLater, Audio.cpp:381/681) |
| `app/src/IO/ConnectionManager.h/.cpp` | FrameReader thread placement on create/reset; StreamWorker lifecycle per stream source; routing relocation |
| `app/src/IO/DeviceManager.h/.cpp` | Drain relocation (onReadyRead → PipelineHost); reader recreation via queued invoke |
| `app/src/IO/FrameReader.h/.cpp` | No logic change; NotificationCenter invoke already queued; affinity only |
| `app/src/UI/Dashboard.h/.cpp` | uiTimeout drain loop; `applyStreamUpdate` ingest; thinning indicator property; plot-width atomic publish on resize |
| `app/qml/...` (dashboard indicator, `ProjectEditor/Views/ProjectView.qml`, source view) | Thinning badge; Lua mode switch + consent dialog; per-source stream-lane combo |
| `app/src/DataModel/ProjectModel.h/.cpp`, `Project/ProjectModelPersistence.cpp`, `Project/ProjectModelLoading.cpp`, `Project/ProjectModelSources.cpp`, `Project/ProjectModelShared.h`, `app/src/DataModel/Frame.h` | `luaExecutionMode` project property (10-site pattern, ProjectModel.h:86 precedent); per-source `streamLane` field (frameParserTemplate pattern, Frame.h:539) |
| `lib/luajit/**` (new), `lib/lua/**` (deleted), `lib/CMakeLists.txt`, `app/CMakeLists.txt`, `cmake/Optimization.cmake`, `cmake/Hardening.cmake` | Runtime swap + build integration; retire 5.4 unwind carve-outs |
| `app/src/DataModel/Scripting/LuaScriptEngine.h/.cpp`, `LuaCompat.h/.cpp`, new `LuaCompatJIT.h`, `ScriptApiCall.cpp`, `MacroRunner.cpp`, `NotificationCenter.cpp`, `DeviceWriteApi.cpp`, `DashboardApi.cpp`, `Editors/DatasetTransformEditor.cpp`, `MQTT/PublisherScript.cpp`, `MQTT/PublisherScriptEditor.cpp` | LuaJIT API drift, protected bootstrap, mode wiring, catch-wall reduction |
| `app/rcc/scripts/parser/lua/*.lua` (13 files), `app/src/DataModel/Importers/DBCImporter.cpp` | 5.3-syntax migration to `bit.*` |
| `app/src/CSV/Export.h/.cpp`, `app/src/MDF4/Export.h/.cpp`, `app/src/Sessions/Export.cpp` | Producer-thread notes; MDF4 worker `ConnectionManager` read → atomic (MDF4/Export.cpp:137); typed `StreamBlockItem` consumers (M5) |
| `app/src/API/Server.h/.cpp`, new `app/src/API/Handlers/StreamHandler.cpp`, `app/src/API/CommandHandler.cpp` | stream.subscribe verbs, block fan-out, missed-count |
| `app/src/API/Handlers/DataTablesHandler.cpp`, `app/src/DataModel/Scripting/ScriptApiCall.cpp`, `ControlScript.cpp` | Marshal table/dashboard verbs to processing thread |
| `app/src/Misc/ModuleManager.cpp`, `app/src/SessionContext.h/.cpp` | PipelineHost adoption + pinned-order entry; thread joins in `stopFrameConsumerWorkers` (ModuleManager.cpp:414); census re-baseline |
| `app/src/Misc/Problems/LinkCheckers.cpp` | Per-source parse-load checker (1 Hz pull; corrected from ConnectionDiagnostics — the pulled 1 Hz checkers live here) |
| `app/src/Benchmark/HotpathBenchmark.cpp` | Drive loop dispatched to processing thread; stream-lane benchmark phase (M4) |
| `doc/claude/architecture/{dataflow,io,dashboard,export,scripting,startup}.md`, `CLAUDE.md` | Post-implementation doc updates per milestone |
| `tests/**`, `app/tests/**` | See test plan |

## Hotpath & threading impact

- **Touches the hotpath?** Yes — every protected file. Read in full this session
  (FrameReader.cpp, CircularBuffer.h, FrameBuilder.h/.cpp, Dashboard ingestion,
  Audio path); `ss-hotpath` invoked. Preservation: SPSC structures keep exactly one
  producer and one consumer each (enumerated above per queue); zero new mutexes on any
  per-frame/per-sample path; in-pipeline hops remain direct calls on one thread;
  pool/aliasing-ptr discipline unchanged (`TimestampedFramePtr` refcount is atomic —
  cross-thread hand-off is safe by construction, raw slot pointers never cross);
  `structureGeneration` stamping sites unchanged; span-lane allocation-freedom
  untouched by M1/M3 (only the breaker call sites change).
- **`--benchmark-hotpath` plan:** run after M1 (budget disabled path identical), M2
  (Lua tiers expected ≥ today; new baseline recorded), M3 (gated tiers must hold —
  drive loop on processing thread), M4 (new stream-lane throughput phase added,
  ungated initially). Any regression blocks the milestone.
- **New cross-thread signal/slot?** Yes, all chunk/block/tick rate, never frame rate:
  driver `dataReceived` → FrameReader (queued, chunk rate — today it is already queued
  for worker-thread drivers, HID/USB/Process); GUI config → pipeline (queued);
  control-script + API synchronous verbs → pipeline (`BlockingQueuedConnection`,
  ControlScript precedent); worker→GUI display updates via SPSC rings drained on
  `uiTimeout` (no signals at data rate — the 65536-queue lesson is the design's
  center).
- **New input to a cached hotpath flag?** No new inputs. The existing refresh
  connections change execution thread (see M3 notes); each flag's wiring is
  re-verified per the dataflow.md table during implementation, and dataflow.md's
  "DirectConnection refresh" rule is rewritten for the two-thread world.
- **Timestamp ownership:** unchanged — drivers stamp (`SampleBlock.t0` at capture,
  CapturedData as today); workers derive per-sample times as `t0 + i*dt`; exporters
  keep `monotonicFrameNs` as safety net only. Stream export needs per-sample ns from
  block metadata, never the +1 ns collision bump.

## Data model & persistence

- `Frame.h` Keys: `Keys::LuaExecutionMode` (`"luaExecutionMode"`, string
  `"safe"`/`"fast"`, default `"safe"`, absent → safe); `Keys::SourceStreamLane`
  (`"streamLane"`, string `"auto"`/`"on"`/`"off"`, default auto, absent → auto).
  Both hand-written surfaces (no spec-0036 generated artifacts involved — confirmed:
  generator reads dataset.json only).
- Old files load unchanged (both keys defaulted). No Sessions DB schema change in
  M1–M4; M5 stream full-rate recording uses the existing readings shape at sample
  granularity (rowid PKs unaffected).
- gRPC/proto: no dataset-property changes → no field-ledger movement; new stream API
  is NDJSON-side only in this spec (proto exposure deferred with the rest of typed
  gRPC evolution).

## API / SDK surface

- `stream.subscribe` / `stream.unsubscribe` (connection-scoped, mirror-style
  dispatch); `streamBlock` push line (base64 float32le). Registered docs via
  SchemaBuilder; snapshot re-dumped (`--dump-api-schema`) → `generate-sdk.py`,
  `generate-command-strings`, registry checks — all already chained in
  `sanitize-commit.py`.
- `dashboard.getData` unchanged (serves dashboard-rate state; stream datasets appear
  with latest values).
- Existing silent SPSC drop in `Server::hotpathTxFrame` stays as-is for frames; the
  stream path introduces the counted-drop concept (new).

## QML / UI

- ProjectView Settings: Lua execution mode switch/combo + one-time Fast consent
  dialog (blocking MessageDialog, persisted acknowledgment in project file via the
  mode value itself — Safe→Fast transition always prompts).
- Source editor view: stream-lane combo (Auto/On/Off), visible only for
  stream-capable buses initially.
- Dashboard: small thinning badge (polled property, existing taskbar/indicator
  pattern); ProblemCenter entry carries the numbers.
- No ComboBox restore-race exposure beyond existing guards (new combos follow the
  `count <= 0` guard rule).

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Frame pipeline off-GUI shape | Relocate-wholesale (PipelineHost + moveToThread) / new pipeline object rebuilt per connect / full actor mailbox | **Relocate-wholesale** — preserves every single-thread invariant by construction and keeps the composition root intact; actor rewrite touches every config setter for no additional safety |
| Stream topology | Worker per source / shared worker pool | **Worker per source** — engines, pools, and state are already per-source; isolation is the safety story; pool scheduling buys nothing measured |
| Where transforms run for streams | In FrameBuilder (marshal blocks to processing thread) / in StreamWorker | **StreamWorker** — keeps per-sample work off the shared pipeline thread (R8 scaling), engine ownership matches thread ownership |
| Dashboard handoff | Per-frame queued signals / SPSC ring + tick drain | **SPSC + drain** — the 65536-queue lesson; bounded GUI work per tick |
| Envelope producer | Worker-side per-pixel reduction (atomic width) / GUI-side reduction from raw blocks | **Worker-side** — GUI cost must be O(pixels), not O(samples); width changes are rare and eventually consistent (one tick of stale bucketing on resize is invisible) |
| LuaJIT integration | Wholesale swap + 52COMPAT + shims / dual runtime / defer | **Wholesale swap** (maintainer decision after evidence review) — dual runtime rejected for lua_* symbol collision fragility; migration + load-error UX accepted in amended spec |
| Safe/Fast semantics | Two independent flags (watchdog, JIT) / one mode enum | **One mode** — JIT-on + watchdog-on is a measured silently-dead safety control; never ship a fake toggle |
| Budget accounting | Fair-share EWMA / fixed per-source cap / queue-depth trigger | **Fair-share EWMA** (maintainer, Q1) — light sources untouchable by construction, no idle-machine thinning |
| Stream designation | Driver-intrinsic only / flag only / **intrinsic default + flag override** | Maintainer choice (Q3) — schema exists day one, audio needs zero user action |
| Block wire format | base64 float32le NDJSON / binary side-channel | **base64 NDJSON** — server has no binary framing today; a binary channel is a future spec if profiling demands it |

## Risks & mitigations

- **Worker teardown (the repo's recurring crash class).** All new threads join in
  `ModuleManager::stopFrameConsumerWorkers()` before `SessionContext::shutdown()`;
  StreamWorkers use blocking-close → quit → wait; Fast-mode hang → bounded wait →
  warn-and-abandon (0046 pattern). Explicit ctest teardown suite per worker.
- **LuaJIT error model (highest-risk workstream).** Panic-unreachable audit + protected
  bootstrap per engine; per-toolchain error-path ctest (error in parser, transform,
  macro, MQTT script × {macOS clang, Windows MSVC/MinGW, Linux GCC}); macOS unwind
  history says validate early on real hardware. Golden vectors (AC16) catch numeric
  drift from the lost integer subtype.
- **Cached-flag refresh lag across threads.** Refreshes are FIFO in the pipeline queue —
  cannot be lost, only late by design; per-flag re-verification checklist in tasks;
  dataflow.md rewritten so future edits don't resurrect the old rule blindly.
- **Pool slots crossing threads.** Only refcounted `TimestampedFramePtr` crosses; the
  `use_count()==1` probe stays exact because all aliases are created on the pipeline
  thread and released by drain/export consumers via atomic refcount (release ordering
  suffices; probe may see a stale "busy" for one frame, never a stale "free" — safe
  direction). Raw `Dataset*`/span views never leave the pipeline thread
  (common-mistakes.md:22).
- **Benchmark validity.** Gated tiers re-run per milestone; drive loop executes on the
  pipeline thread so numbers stay apples-to-apples; CI gate unchanged.
- **API/exporter single-producer invariants.** Producer moves from GUI to pipeline
  thread — still exactly one; asserted in code comments at each enqueue site; MDF4
  worker singleton read replaced by pushed atomic.
- **QuickPlot audio routing.** Stream lane must thread a sourceId through the
  QuickPlot path (agent finding: typed path would otherwise enter the non-source
  branch); QuickPlot stream config derives from the UI driver as today.
- **Scope**: file list above is the lane; anything discovered outside it gets named in
  chat before touching (Trust Contract).

## Test & verification plan

- **Unit (runnable by assistant):**
  - `tests/scripts/` block-transform contract cases (AC7): `transform_block` called
    once per block, arity/fallback, info payload fields, Lua + JS.
  - Golden-vector runner for the migrated shipped scripts (AC16): recorded 5.4 outputs
    vs LuaJIT outputs, numeric equality.
- **C++ ctest (existing build dir, maintainer builds):** budget fair-share unit
  (synthetic per-source loads → decimation pattern, AC1/AC2 logic tier); envelope
  reducer min/max fidelity incl. single-sample impulse (AC9 logic tier); SampleBlock
  timestamp derivation; LuaJIT error-path suite (AC17 logic tier + never-aborts-host);
  StreamWorker teardown (join, abandon path); PipelineHost drain ordering (AC15
  support).
- **Integration (running app, API 7777):** AC1/AC2 (two-source overload → per-source
  rates via `dashboard.getData` timestamps), AC3 (problem-center query + indicator),
  AC8/AC17 (runaway transform Safe/Fast), AC10 (export row counts + checksums),
  AC11 (cross-lane table read + maintainer TSan run), AC18 (mode persistence),
  AC20 (subscribe, slow-client missed counts), AC13 (QuickPlot audio).
- **Hotpath:** `--benchmark-hotpath` full gated suite per milestone (AC4); new
  stream-lane phase (AC5/AC6 throughput source); AC19 derated targets scripted.
- **Golden session:** spec-0047 dual-replay before/after M3 (AC15).
- **Maintainer observations:** AC5 (GUI profiling), AC12 (BADAQ un-degraded project —
  the incident's definition of done), AC14 (UI fluidity under saturated parser),
  AC9 visual impulse check.
- **Static:** `code-verify.py --check` on every touched file (hotpath violations
  block); `qt-cpp-review` before each milestone handoff; `sanitize-commit.py` before
  every commit (drives SDK/schema regen checks for M6).

## Milestones (implementation order)

1. **M1 budget** — standalone, fixes the reported bug; ACs 1–4.
2. **M2 LuaJIT** — runtime swap + mode + migration; ACs 16–19 (interim derate).
3. **M3 pipeline thread** — PipelineHost move; ACs 14, 15, 4 re-run.
4. **M4 stream lane core** — SampleBlock, StreamWorker, dashboard ingest, QuickPlot;
   ACs 5–9, 13.
5. **M5 stream exports + table-store** — typed MDF4/CSV, block-rate store; ACs 10, 11.
6. **M6 block API** — subscribe verbs + SDK regen; AC 20.
7. **M7 validation** — BADAQ project, scaling runs, floor derate closure; ACs 6, 12, 19.
