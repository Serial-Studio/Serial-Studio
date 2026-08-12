---
spec: 0051-stream-lane-and-parse-budget
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-08-11
---

# Tasks 0051 — Per-source parse budget + typed stream lane + off-GUI pipeline + LuaJIT

> **Phase 3 of 4 — the ordered checklist.** Decomposes [`plan.md`](./plan.md); milestone
> boundaries (M1–M7) are clean benchmark/review points. `/ss-implement` works top to bottom.

## Conventions

- One task = one focused, reviewable change. **Verify** confirms the unit before moving on.
- Hotpath-class tasks name their binding invariant in **Does**.
- `--benchmark-hotpath` and `qt-cpp-review` run at every milestone boundary (marked ◆).

## Tasks

## M1 — Per-source parse budget (ACs 1–4)

### T1 — Per-source budget state + fair-share accounting

- **Files:** `app/src/DataModel/FrameBuilder.h`, `app/src/DataModel/FrameBuilder.cpp`
- **Does:** Replace the global `m_parseBudget*` members with per-source
  `BudgetEntry { ewmaNs, decimateN, frameCounter, skipped }` (QHash by sourceId) plus a
  thread-total EWMA (tau ≈ 250 ms). `parseBudgetAccount(sourceId, elapsed)` updates both;
  `parseBudgetSkipFrame(sourceId)` passes everything while total EWMA < 90% of one core,
  else decimates only sources over fair share (`N = ceil(load / fairShare)`). Invariants:
  no allocation/signal/lock on the frame path (QHash entry created on first sight per
  source, steady-state lookup only); counters plain quint64, pulled not pushed (spec 0033);
  `setParseBudgetEnabled(false)` still bypasses everything (benchmark).
- **Verify:** `python scripts/code-verify.py --check app/src/DataModel/FrameBuilder.{h,cpp}`;
  read-back of both call sites (`parseProjectFrame` ×2) passing sourceId.
- **Deps:** none
- [x] done — 2026-08-11, code-verify 0/0; message-box removal pulled forward from T5
  (orphaned includes dropped for zero-warnings)

### T2 — Budget ctest unit

- **Files:** `app/tests/` (new budget suite + CMake registration)
- **Does:** Synthetic load patterns → assert: light source never decimated under heavy
  neighbor; N proportional to overrun; sub-second recovery after load drop (EWMA decay);
  no decimation below 90% total.
- **Verify:** `ctest` against existing build dir (maintainer builds first).
- **Deps:** T1
- [x] done — 2026-08-11; scope amended with maintainer approval: budget engine extracted to
  header-only `app/src/DataModel/ParseBudget.h` (FrameBuilder.cpp has no unit-tier link set),
  FrameBuilder delegates; suite `tst_parse_budget` (5 cases) registered; ctest run pending
  maintainer rebuild with `-DSS_BUILD_TESTS=ON`

### T3 — Per-source parse-load diagnostics + ProblemCenter checker

- **Files:** `app/src/Misc/Problems/LinkCheckers.cpp` (corrected lane, named in chat),
  `app/src/DataModel/FrameBuilder.h` (read accessors)
- **Does:** 1 Hz-pulled checker reads per-source EWMA/decimation counters
  (`[[nodiscard]]` accessors, no caching, no frame-path call sites) and raises/clears a
  ProblemCenter entry naming source + measured load + current N. Invariant: diagnostics
  pulled, never pushed; finding text bucketed so the panel model doesn't churn per tick.
- **Verify:** code-verify on touched files; integration check deferred to T6.
- **Deps:** T1
- [x] done — 2026-08-11; `reportParseThinning` under the existing `link.statistics` checker,
  duty/N banded for text stability, accessors landed in T1

### T4 — Dashboard thinning indicator

- **Files:** `app/src/UI/Dashboard.h/.cpp`, `app/qml/` (indicator badge in the dashboard
  chrome; exact QML file confirmed at edit time and named in chat)
- **Does:** Polled `thinningActive`/`thinningSources` properties refreshed on the 1 Hz
  tick (never per frame); minimal badge bound to them. Clears within 2 s of recovery.
- **Verify:** code-verify; maintainer visual check with T6's repro.
- **Deps:** T3
- [x] done — 2026-08-11; `thinningActive` polled property (1 Hz, transition-emitting) +
  passive alarm-dot badge in `Taskbar.qml` beside the freeze button (no new icon asset,
  IconRegistry untouched)

### T5 — Remove legacy breaker plumbing + doc sync

- **Files:** `app/src/DataModel/FrameBuilder.h/.cpp`,
  `doc/claude/architecture/dataflow.md`
- **Does:** Delete `kParseBudgetWindowMs`/`kParseBudgetWarnLimitMs` fixed-window fields,
  one-shot message box on trip (superseded by ProblemCenter entry), episode latch;
  dataflow.md budget paragraph rewritten (per-source fair-share, pulled diagnostics).
- **Verify:** grep confirms no `parseBudgetWindow` remnants; code-verify clean.
- **Deps:** T1, T3, T4
- [x] done — 2026-08-11; legacy fields/box removed in T1, dataflow.md gained the
  "Parse-Load Governor" section, benchmark wording fixed in dataflow.md + ss-hotpath
  SKILL.md (one out-of-lane docs line, named in chat)

### T6 ◆ — M1 integration tests + benchmark gate

- **Files:** `tests/integration/test_parse_budget.py` (new)
- **Does:** Automates AC1 (two-source overload → light source keeps rate, heavy thins
  smoothly via `dashboard.getData` timestamps), AC2 (recovery < 1 s via frame counters),
  AC3 (problem-center presence + clearance). Maintainer runs `--benchmark-hotpath`
  (AC4: gated tiers unchanged — budget disabled path must be byte-identical).
- **Verify:** pytest against running app (API 7777); benchmark report.
- **Deps:** T1–T5
- [x] done (code) — 2026-08-11; `test_parse_budget.py` (3 tests) written; NEGATIVE CONTROL
  run against the live pre-fix binary: AC1 test fails exactly as the bug report describes
  (heavy source frozen, 1 distinct value / 2 s) — proves detection. Green run + AC4
  `--benchmark-hotpath` await maintainer rebuild.

### M1 review gate (◆) — closed 2026-08-11

qt-cpp-review ran (6 agents + lint). Findings fixed in-place: stale-entry latch
(5× confirmed → `ParseBudget::maintain()` swept from the 1 Hz tick + quiet-offender
ctest case), estimator oscillation (offered-load scaling: charge × decimateN +
0.90/0.70 hysteresis), per-source notification dedup (`entityUniqueId = sourceId`),
inverted `[[likely]]` in skipFrame, badge `Layout.preferredWidth`, proportional test
made symmetric + deterministic. Named decision: the three `SS_ASSERT` sites in
ParseBudget stay release-evaluated (int compares, dwarfed by account's map+FP work) —
deliberate, not a default. Singleton census re-baselined (+3 sites, checker free
functions + Dashboard poll, precedent-conform). sanitize-commit clean.
**Validated on rebuilt binary 2026-08-11: ctest 6/6, pytest 3/3 (AC1/AC2/AC3 green).**
Debug note: first integration run was flaky because the 800k-iteration overload measured
~5 ms/frame in-app = duty exactly 1.0 (threshold coin flip); raised to 4M iterations
(5x oversubscription) + fixture reconnect settle. Live probes confirmed adaptive stride
histogram + per-source finding + fresh light source. REMAINING for AC4: maintainer runs
`--benchmark-hotpath` on the production-optimized configure (the test re-configure printed
"Disabling production optimization flags" — benchmark needs the normal flag flow), plus
the AC3 badge visual check.

## M2 — LuaJIT runtime + Safe/Fast mode (ACs 16–19 interim)

### T7 — Vendor LuaJIT + CMake integration

- **Files:** `lib/luajit/**` (new, pinned release), `lib/CMakeLists.txt`,
  `app/CMakeLists.txt`
- **Does:** Vendored LuaJIT with CMake wrapper (host minilua/buildvm two-stage build,
  per-toolchain branches incl. MSVC), `LUAJIT_ENABLE_LUA52COMPAT` on, `lua54` target
  replaced by `luajit` target; old `lib/lua/` NOT deleted yet (T13). Maintainer builds;
  assistant verifies structure + flags by reading. cpp-compiler-flags skill consulted for
  per-toolchain branches.
- **Verify:** cmake files read-back against LuaJIT's canonical build; maintainer build on
  macOS first (unwind history).
- **Deps:** none (parallel to M1)
- [ ] done

### T8 — API-drift compat header + protected bootstrap

- **Files:** `app/src/DataModel/Scripting/LuaCompatJIT.h` (new),
  `app/src/DataModel/Scripting/LuaScriptEngine.cpp`, `app/src/DataModel/ScriptApiCall.cpp`
- **Does:** Shims for `lua_isinteger` (integral-double probe preserving int-vs-float
  display formatting), `luaL_len`/`lua_geti`/`lua_seti`/`lua_rawlen` mappings; one
  protected-bootstrap C function per engine so every state-setup path (library injection,
  compile) runs under `lua_pcall` — invariant: `lua_atpanic` unreachable, Lua never
  aborts host.
- **Verify:** code-verify; grep confirms zero unprotected `lua_*` state-setup outside
  bootstrap in touched files.
- **Deps:** T7
- [ ] done

### T9 — Engine call-sites on LuaJIT

- **Files:** `app/src/DataModel/FrameBuilder.cpp` (transform engines),
  `app/src/DataModel/Scripting/MacroRunner.cpp`, `app/src/MQTT/PublisherScript.cpp`
- **Does:** Same drift/bootstrap treatment for the three remaining VM owners; catch-walls
  reduced to status handling with a one-line comment each (why vestigial). Watchdog hook
  sites unchanged (Safe mode).
- **Verify:** code-verify; error-path ctest lands in T12.
- **Deps:** T8
- [ ] done

### T10 — LuaCompat inversion + editor/validation surfaces

- **Files:** `app/src/DataModel/Scripting/LuaCompat.cpp` (+.h),
  `app/src/DataModel/Editors/DatasetTransformEditor.cpp`,
  `app/src/MQTT/PublisherScriptEditor.cpp`, `app/src/DataModel/NotificationCenter.cpp`,
  `app/src/DataModel/Scripting/DeviceWriteApi.cpp`,
  `app/src/DataModel/Scripting/DashboardApi.cpp`
- **Does:** `bit32` + math shims rewritten over LuaJIT's native `bit`; sandbox builders
  never open `ffi`/`jit` (sandbox-escape invariant); editor validation states get the
  same bootstrap.
- **Verify:** code-verify; sandbox grep (`ffi`, `luaopen_jit`) returns nothing exposed.
- **Deps:** T8
- [ ] done

### T11 — Project property `luaExecutionMode` + consent UI

- **Files:** `app/src/DataModel/Frame.h` (Keys), `app/src/DataModel/ProjectModel.h/.cpp`,
  `app/src/DataModel/Project/ProjectModelPersistence.cpp`,
  `app/src/DataModel/Project/ProjectModelLoading.cpp`,
  `app/qml/ProjectEditor/Views/ProjectView.qml`
- **Does:** 10-site property pattern (changeDrivenTransforms precedent, undo scope
  included — undo-scope-missing lint applies); "safe" default, absent-key → safe; engines
  read at compile: Safe = `luaJIT_setmode OFF` + LUA_MASKCOUNT hook, Fast = ON + no hook
  (one mode, never two flags); Safe→Fast flip always shows consent dialog naming the
  tradeoff.
- **Verify:** code-verify; load/save round-trip read-back; AC18 flow deferred to T14.
- **Deps:** T9
- [ ] done

### T12 — Shipped-script migration + load-error UX + error-path ctest

- **Files:** `app/rcc/scripts/parser/lua/` (13 files),
  `app/src/DataModel/Importers/DBCImporter.cpp`,
  `app/src/DataModel/Scripting/LuaScriptEngine.cpp` (error enrichment),
  `app/tests/` (LuaJIT error-path suite)
- **Does:** Rewrite 5.3 syntax to `bit.*`/`math.floor`; DBC generator emits
  LuaJIT-compatible code; on compile error, scan source for 5.3 operators and enrich the
  message with construct + replacement (R22); ctest covers error paths (parser,
  transform, macro, MQTT script) per platform + never-aborts-host.
- **Verify:** golden-vector runner (T13) green on all 28 shipped scripts; ctest suite;
  enriched-error unit case.
- **Deps:** T9
- [ ] done

### T13 ◆ — Golden vectors, 5.4 removal, benchmark re-baseline

- **Files:** `tests/scripts/` (golden-vector cases), `lib/lua/**` (delete),
  `lib/CMakeLists.txt`, `cmake/Optimization.cmake`, `cmake/Hardening.cmake`,
  `doc/claude/architecture/scripting.md`
- **Does:** Record 5.4 golden outputs BEFORE deletion (script corpus + importer outputs);
  assert LuaJIT numeric equality (AC16); delete vendored 5.4 + retire its unwind
  carve-outs (cpp-compiler-flags skill for each cmake edit); scripting.md documents the
  5.1+shims surface. Maintainer runs `--benchmark-hotpath` (Lua tiers ≥ today, AC4) and
  the AC19 derated benchmark; AC17 Safe-abort + Fast-containment integration test;
  AC18 consent/persistence observation.
- **Verify:** `pytest tests/scripts/` green; benchmark report archived as new baseline.
- **Deps:** T7–T12
- [ ] done

## M3 — Frame pipeline off the GUI thread (ACs 14, 15, 4)

### T14 — PipelineHost module + composition-root adoption

- **Files:** `app/src/IO/PipelineHost.h/.cpp` (new), `app/src/SessionContext.h/.cpp`,
  `app/src/Misc/ModuleManager.cpp`
- **Does:** Thread-owning module: QThread, Dashboard-bound SPSC ring
  (`ReaderWriterQueue<TimestampedFramePtr>`, 8192), atomic operationMode/isConnected
  mirrors, relocated mode routing. Invariants named at edit: SessionContext ctor/dtor
  stay empty; adoption slot in pinned order (before ConnectionManager); reverse-release
  in `shutdown()`; thread joined in `stopFrameConsumerWorkers()` BEFORE shutdown;
  `SessionContext::current()` only from composition root/instance() (census
  re-baseline expected).
- **Verify:** code-verify incl. `--singleton-census --check` (baseline updated
  deliberately, named in chat); ctor-edge read-back (no instance() calls in ctor).
- **Deps:** M1 complete (budget code stable before it moves threads)
- [ ] done

### T15 — FrameReader + FrameBuilder + FrameParser affinity move

- **Files:** `app/src/IO/DeviceManager.cpp`, `app/src/IO/ConnectionManager.cpp`,
  `app/src/DataModel/FrameBuilder.cpp`, `app/src/Misc/ModuleManager.cpp`
- **Does:** FrameReaders `moveToThread(pipeline)` at create; recreation via queued
  invoke; drain relocates from `DeviceManager::onReadyRead` to
  `PipelineHost::onReadyRead` (readyRead → host, DirectConnection — same thread);
  FrameBuilder + FrameParser moved after `setupExternalConnections()`; remove the
  cross-thread `Q_ASSERT(m_operationMode == AppState...)` (FrameBuilder.cpp:759).
  Invariants: SPSC single-producer/single-consumer identity per queue preserved
  (enumerate in commit message); no mutexes; in-pipeline hops direct; hotpath signal
  chain never queued at frame rate.
- **Verify:** code-verify; connection-type audit read-back (every connect on the frame
  path listed with resolved type); app smoke by maintainer.
- **Deps:** T14
- [ ] done

### T16 — GUI→pipeline marshaling of synchronous entry points

- **Files:** `app/src/API/Handlers/DataTablesHandler.cpp`,
  `app/src/DataModel/Scripting/ScriptApiCall.cpp`, `app/src/DataModel/ControlScript.cpp`,
  `app/src/DataModel/FrameParserModel.cpp` (dryRun live-engine path), player replay call
  sites (`CSV/Player.cpp`, `MDF4/Player.cpp`, `Sessions/Player.cpp`)
- **Does:** `reprocessFrames`/`dashboardTick`/table verbs/dryRun/replay marshaled to the
  pipeline thread (`BlockingQueuedConnection` where the caller needs the result —
  ControlScriptWorker precedent; queued otherwise). Invariant: no per-frame marshaling —
  all these are command/tick/block rate.
- **Verify:** code-verify; grep for remaining direct GUI-thread calls into
  FrameBuilder/FrameParser mutable state (checklist in commit).
- **Deps:** T15
- [ ] done

### T17 — Dashboard tick drain + exporter producer notes

- **Files:** `app/src/UI/Dashboard.cpp`, `app/src/MDF4/Export.cpp`,
  `app/src/CSV/Export.cpp`
- **Does:** uiTimeout lambda drains the frame ring (all pending) before `updated()`;
  pool backpressure semantics unchanged (stalled GUI → pool exhaustion warning). MDF4
  worker's `ConnectionManager::isConnected()` read (MDF4/Export.cpp:137) replaced by
  the PipelineHost atomic; producer-identity comments at each sink enqueue site.
  Invariant: dashboard publish sites still stamp `structureGeneration`; Dashboard stays
  GUI-thread-only.
- **Verify:** code-verify; maintainer smoke (live device, all widget types).
- **Deps:** T15
- [ ] done

### T18 — Benchmark on pipeline thread + flag-refresh audit

- **Files:** `app/src/Benchmark/HotpathBenchmark.cpp`,
  `doc/claude/architecture/dataflow.md`
- **Does:** Drive loop dispatched onto the pipeline thread (blocking); per-flag refresh
  audit: every cached-flag connection listed with its now-resolved type + FIFO argument;
  dataflow.md threading table + cached-flags section rewritten for the two-thread world.
- **Verify:** maintainer runs full gated `--benchmark-hotpath` (AC4 must hold).
- **Deps:** T15–T17
- [ ] done

### T19 ◆ — M3 validation: golden session + UI-immunity

- **Files:** `tests/integration/test_pipeline_thread.py` (new)
- **Does:** AC15 via spec-0047 dual-replay (same input → identical sequences/exports/
  values pre/post-move); AC14 scripted load + maintainer observation (saturating parser,
  UI fluid, streams live); teardown ctest for PipelineHost join ordering.
- **Verify:** pytest + golden harness + maintainer observation logged in spec ACs.
- **Deps:** T14–T18
- [ ] done

## M4 — Stream lane core (ACs 5–9, 13)

### T20 — SampleBlock contract + per-source lane flag

- **Files:** `app/src/IO/HAL_Driver.h`, `app/src/DataModel/Frame.h` (Keys),
  `app/src/DataModel/Project/ProjectModelShared.h`,
  `app/src/DataModel/Project/ProjectModelSources.cpp`, `app/src/DataModel/ProjectModel.h/.cpp`
- **Does:** `SampleBlock` + `publishSampleBlock()` + `isStreamCapable()` (default false);
  per-source `streamLane` ("auto"/"on"/"off", absent → auto) via the frameParserTemplate
  persistence pattern + undo-scoped mutator. Invariant: source-owns-time — `t0` stamped
  at capture, `dt` from device rate.
- **Verify:** code-verify; project save/load round-trip read-back.
- **Deps:** M3 complete
- [ ] done

### T21 — StreamWorker skeleton + lifecycle

- **Files:** `app/src/IO/StreamWorker.h/.cpp` (new), `app/src/IO/ConnectionManager.cpp`,
  `app/src/Misc/ModuleManager.cpp`
- **Does:** Per-stream-source worker (QThread + worker object): raw-block SPSC in,
  worker-affine cadence timer, planar float64 scratch (reused — zero steady-state
  allocation), display-update SPSC out, diagnostics counters (pulled). Lifecycle:
  created/destroyed by ConnectionManager beside DeviceManager; blocking-close → quit →
  wait join in `stopFrameConsumerWorkers`; bounded-wait + warn-and-abandon path for a
  hung engine (R21). Teardown ctest in same task.
- **Verify:** code-verify; teardown ctest (join, abandon, no leak of abandoned thread's
  queues).
- **Deps:** T20
- [ ] done

### T22 — Audio driver typed publish

- **Files:** `app/src/IO/Drivers/Audio.h/.cpp`
- **Does:** RT callback → SPSC (unchanged) → StreamWorker consumes raw PCM; PCM decode
  moves into the worker; CSV text encode + `m_csvStream`/`m_csvBuffer`/`m_csvData` +
  main-thread `processInputBuffer` deleted; sample-clock resync logic preserved verbatim
  (source-owns-time); input-timer double-deleteLater fixed by the rewrite. Frame-lane
  publish path removed for stream-lane-active sources only (lane flag honored).
- **Verify:** code-verify; maintainer smoke: QuickPlot audio silent until T25 wires
  ingest — task ordering noted in chat at edit time.
- **Deps:** T21
- [ ] done

### T23 — Stream transform engines + block contract

- **Files:** `app/src/IO/StreamWorker.cpp`, `app/src/DataModel/FrameBuilder.cpp`
  (compile-transforms helper extraction into a shared unit)
- **Does:** Worker-owned Lua/JS engine built by the extracted helper; `transform_block`
  detected at compile (arity/name probe), called once per block with reused samples
  table + frozen info payload (`sourceId, uniqueId, blockNumber, timestampMs,
  sampleRate, count, firstSampleIndex`); per-sample `transform(value)` fallback loop;
  Safe/Fast watchdog parity (hook+deadline / none); transform errors → pulled counters,
  block falls back to raw (R10).
- **Verify:** code-verify; `tests/scripts/` block-contract units (AC7) green.
- **Deps:** T21, M2 complete
- [ ] done

### T24 — Envelope + FFT + latest-value reduction

- **Files:** `app/src/IO/StreamWorker.cpp`, `app/src/UI/Dashboard.h/.cpp` (pixel-width
  atomic publish on plot resize)
- **Does:** Per-dataset min/max envelope on the TimeRing grid, bucket count = plot pixel
  width (atomic int, GUI-written, worker-read — eventually consistent by design);
  worker-side FFT window ring (capacity = normalizedFftSize from project config); latest
  values. DSPSimd kernels for min/max scan (never inline intrinsics). Invariant:
  worker→GUI payload is O(pixels + fftSize + datasets), never O(samples).
- **Verify:** code-verify; envelope ctest incl. single-sample impulse survival (AC9
  logic tier).
- **Deps:** T21
- [ ] done

### T25 — Dashboard stream ingest + QuickPlot routing

- **Files:** `app/src/UI/Dashboard.h/.cpp`, `app/src/IO/PipelineHost.cpp` (QuickPlot
  sourceId threading), `app/src/DataModel/FrameBuilder.cpp` (template-frame publish for
  stream sources)
- **Does:** `applyStreamUpdate` on the uiTimeout drain: appendDecimated envelope pairs,
  memcpy FFT ring, write latest dataset values, `m_updateRequired = true`; per-source
  PlotClock advanced from block t0 — never cleared (bulkLoad clear semantics explicitly
  NOT reused); stream sources publish template frame at connect for widget structure;
  QuickPlot audio routes through the stream lane with sourceId 0 threaded (agent-found
  gap). Invariants: Dashboard GUI-thread-only; structureGeneration checks intact;
  bounded per-tick work.
- **Verify:** code-verify; maintainer visual: QuickPlot audio waveform+FFT live (AC13),
  project-mode plots/FFT live.
- **Deps:** T22–T24
- [ ] done

### T26 ◆ — M4 validation + stream benchmark phase

- **Files:** `app/src/Benchmark/HotpathBenchmark.cpp` (stream phase),
  `tests/integration/test_stream_lane.py` (new)
- **Does:** Benchmark phase driving synthetic 96 kHz blocks through a StreamWorker
  (throughput counter source for AC6/AC19); integration: AC8/AC17 runaway block
  transform (Safe aborts, Fast contains, disconnect clean); AC5 maintainer GUI
  profiling + render-FPS observation; AC9 impulse + sine-bin scripted check.
- **Verify:** pytest; benchmark report; maintainer profiling session.
- **Deps:** T20–T25
- [ ] done

## M5 — Stream exports + table store (ACs 10, 11)

### T27 — Typed export consumers

- **Files:** `app/src/MDF4/Export.h/.cpp`, `app/src/CSV/Export.h/.cpp`,
  `app/src/IO/StreamWorker.cpp`
- **Does:** `FrameConsumer<StreamBlockItem>` consumers (AudioExport precedent,
  FrameConsumer<T> generic): MDF4 native numeric per-sample channels, CSV numeric
  writeRow variant; per-sample timestamps derived `t0 + i*dt` (never monotonic bump);
  full-rate post-transform values (R12); backpressure = existing pool-exhaustion
  semantics, capture never stalls.
- **Verify:** code-verify; AC10 integration test (row/sample counts + checksum vs known
  vector).
- **Deps:** M4 complete
- [ ] done

### T28 — Block-rate table-store publish

- **Files:** `app/src/IO/StreamWorker.cpp`, `app/src/DataModel/FrameBuilder.cpp`
  (queued ingest slot on pipeline thread)
- **Does:** Latest per-block dataset values queued (block rate ~100 Hz) to the pipeline
  thread, which writes `setDatasetRaw/Final` there — store single-writer invariant
  preserved (all writes on pipeline thread), change-driven versioning works unchanged.
  Invariant: never per-sample store writes (R13).
- **Verify:** code-verify; AC11 integration test (frame-lane virtual dataset reads
  stream slot at block rate); maintainer TSan run over mixed-lane session.
- **Deps:** T27
- [ ] done

## M6 — Block-rate API (AC 20)

### T29 — stream.subscribe verbs + worker fan-out

- **Files:** `app/src/API/Server.h/.cpp`, `app/src/API/Handlers/StreamHandler.cpp` (new),
  `app/src/API/CommandHandler.cpp`, `app/src/IO/StreamWorker.cpp`
- **Does:** Connection-scoped `stream.subscribe`/`unsubscribe` (mirror-style pre-registry
  dispatch + ConnectionState fields + disconnect cleanup); per-subscriber drop-oldest
  ring with `missed` counter on the worker; `streamBlock` NDJSON line (base64 float32le)
  through ServerWorker queued write with sessionId revalidation. Rate/size limits reuse
  existing constants.
- **Verify:** code-verify; AC20 integration test (slow client sees missed counts, fast
  client + capture unaffected).
- **Deps:** M5 complete
- [ ] done

### T30 ◆ — SDK/schema regeneration

- **Files:** `app/rcc/api/api-schema.json` (re-dump), generated SDK files via
  `scripts/generate-sdk.py`, command strings, registry checks
- **Does:** `--dump-api-schema` (maintainer runs binary) → regenerate chain; never
  hand-edit generated files; proto ledger untouched (NDJSON-only surface).
- **Verify:** `python scripts/sanitize-commit.py` (runs the full check chain).
- **Deps:** T29
- [ ] done

## M7 — Whole-feature validation (ACs 6, 12, 19)

### T31 — Scaling + derated floor runs

- **Files:** `tests/integration/test_stream_scaling.py` (new), benchmark scripts in
  scratchpad promoted only if maintainer wants them kept
- **Does:** 1→2→4 concurrent 96 kHz synthetic stream sources: per-source throughput
  within 10% (AC6, dev machine + 5× derate margin, marked provisional); AC19 Safe ≥ 3×
  hooked-5.4 baseline, Fast holds 8×96 kHz with ≥2× headroom at ×5 margin.
- **Verify:** pytest + archived benchmark reports referenced from spec AC checkboxes.
- **Deps:** M4–M6
- [ ] done

### T32 — BADAQ definition-of-done + docs + CLAUDE.md

- **Files:** spec ACs, `doc/claude/architecture/{io,dashboard,export,startup}.md`,
  `CLAUDE.md` (threading section), memory dir
- **Does:** Maintainer restores the original un-degraded BADAQ project (48 kHz,
  un-merged metrics): no budget warning, all widgets live (AC12). Architecture docs +
  CLAUDE.md updated for: PipelineHost thread, stream lane, LuaJIT runtime, new budget.
  `bug-report.md` disposition decided by maintainer (their file — not touched without
  instruction).
- **Verify:** maintainer sign-off recorded in spec; `ss-ai-audit`-style read of updated
  docs against code.
- **Deps:** everything
- [ ] done

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` is met and checked off there (AC6/AC19
      marked provisional pending physical floor box, per spec).
- [ ] `python scripts/code-verify.py --check` clean on all changed files (singleton-census
      re-baseline named and justified).
- [ ] `qt-cpp-review` run at every ◆ milestone; findings addressed or noted.
- [ ] `--benchmark-hotpath` full gated suite green at M1/M2/M3/M4 boundaries; new
      baseline archived after M2.
- [ ] `pytest` targets listed in plan.md identified for the maintainer; runnable tiers
      (tests/scripts/) green.
- [ ] `python scripts/sanitize-commit.py` run before every commit; commits only with
      explicit per-turn permission.
- [ ] Diff is what was asked, and only that — file list = plan's lane; discoveries named
      in chat first.
- [ ] `spec.md` status set to `done`.
