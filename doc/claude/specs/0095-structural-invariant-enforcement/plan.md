---
spec: 0095-structural-invariant-enforcement
phase: plan
status: approved     # draft -> approved (gate before /ss-tasks)
updated: 2026-10-07
---

# Plan 0095 — Structural invariant enforcement

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

Each milestone replaces a prose rule with the cheapest mechanism that makes a violation loud.
M1 wraps every cached hotpath flag in a zero-cost `Cached<T>` value whose only constructor
registers a derive function with a fixed-size per-owner checker. Existing 1 Hz hooks run the
checker on each owner's own thread, and a mismatch that persists across two checks fires
`SS_ASSERT` and repairs. M2 turns the `bool feedExports` lane identity into two lane types
that carry a `constexpr` trait. The gate's lane-specific methods become reachable only
through those types. M3 introduces a `DSP::RingCapacity` that can only be built from rate ×
window, and folds the plot clocks and display time into one `PlotClockState` with one
`reset()`. M4 (as amended) makes `instance()` failures name their cause and folds "instantiate
+ bind" into one composition-root entry point, so a root cannot skip binding. It also deletes
the dead `m_initialized` flag. M5 adds an `Enforced:` marker that claim-verify resolves, tags
the binding rules, and collapses the enforced ones to pointers.

## Affected subsystems & files

| File | Change |
|------|--------|
| `core/Pipeline/DataModel/CachedFlag.h` (new) | `Cached<T>` value wrapper + `CachedFlagChecker` (fixed `std::array` of entries, no allocation) |
| `core/Pipeline/DataModel/FrameBuilder.h/.cpp` | 5 flags become `Cached<T>`; extract the const derive helpers (`deriveCaptureLatest()`, `deriveChangeDriven()`, `deriveCaptureDatasetValues()`); run the checker from the existing 1 Hz lambda (~:569); lane-typed republish (`republishFrames`, `republishOneFrame`, `emitRepublishedFrame`) |
| `core/Pipeline/DataModel/FrameBuilder/BlockPublisher.h/.cpp` | `m_anyAsyncSink` becomes `Cached<bool>`; `refreshSinkFlag` writes through the derive helper |
| `core/Ui/UI/Dashboard.h/.cpp` | `m_streamAvailable` becomes `Cached<bool>`, checked in the existing 1 Hz slot (~:321), plus a mirror check against `PipelineHost` dashboard-accepting; `PlotClockState` member; ring sizing through `RingCapacity`; delete the dead `cap` locals (~:1698, ~:1796) |
| `core/Pipeline/DataModel/RepublishGate.h` | Lane-specific methods made private; `DashboardLane` / `ExportLane` handle types with `static constexpr bool kFeedsExports` |
| `core/Pipeline/DSP.h` | `RingCapacity` type; `TimeRing` / `EnvelopeRing` ctor + `resizeCapacity` take it instead of `int` |
| `core/Ui/UI/Dashboard/TimeRingSizing.h/.cpp` (new) | The three sizing policies (frame-lane, stream-lane, growth decision) moved out of `Dashboard.cpp` as free functions returning `RingCapacity`, so they are unit-testable |
| `core/Ui/UI/Dashboard/DashboardIngest.h/.cpp` | `PlotClock` moves into `PlotClockState { sources, displayTimeSec, reset() }`; `IngestBindings` binds one reference instead of two |
| `core/Ui/UI/Dashboard/ReplaySeekEngine.h` | Comment update only (names `PlotClockState`) |
| Nine module `instance()` accessors (AppState, Dashboard, Console Handler, FrameParser, FrameBuilder, ProjectModel, PipelineHost, ConnectionManager, NotificationCenter) | Assert message routes through one `SessionContext`-free helper reporting "under construction" vs "not yet constructed" |
| `core/Core/ModuleConstruction.h` (new, tiny) | Module-static "currently constructing" marker set by `SessionContext::create<T>`; readable from any library without including `SessionContext.h` |
| `app/src/SessionContext.h/.cpp` | `create<T>` sets/clears the construction marker |
| `app/src/Misc/ModuleManager.h/.cpp` | `composeSession(BindMode)` = instantiate + (connections) + bind; `instantiateCoreModules` / `bindInterfaces` become private |
| `app/src/Misc/CLI.cpp` | Benchmark, headless and schema-dump roots call `composeSession`; schema dump passes `BindMode::SchemaOnly` |
| `core/Pipeline/DataModel/ProjectModel.h/.cpp` | Delete dead `m_initialized` (decl + 2 writes) |
| `core/Pipeline/DataModel/Project/ProjectPersistence.h` | Drop the stale `m_initialized` comment |
| `app/tests/tst_cached_flags.cpp` (new) | AC1 |
| `app/tests/tst_republish_lanes.cpp` | Port 7 tests to lane handles; add the masked-refresh-through-FrameBuilder-lane-trait case (AC4) |
| `app/tests/tst_time_ring_sizing.cpp` (new) | 44.1/48 kHz × 10 s, saturated-growth, ceiling cases (AC6 automated half) |
| `app/tests/tst_envelope_ring.cpp`, `tst_replay_seek_engine.cpp`, `tst_dashboard_ingest.cpp` | Literal capacities become `RingCapacity::fromRate(window, rate)`; ingest fixture binds `PlotClockState` |
| `app/tests/tst_module_construction.cpp` (new) | AC7 |
| `app/tests/CMakeLists.txt` | Register the four new tests |
| `scripts/claim-verify.py` | `check_enforcers()` + `enforcer-missing` error kind |
| `scripts/doc-anchors.json` | `cached-hotpath-flags` ordered anchor (code `Cached<…> m_x` declarations ↔ dataflow.md list) |
| `CLAUDE.md`, `doc/claude/architecture/{dataflow,dashboard,startup}.md`, `doc/claude/common-mistakes.md` | M5 triage; R5 reconcile (common-mistakes :24 scoped to the Dashboard side); R15 stale claims (startup.md :103, :129, :230) |

## Architecture & data flow

### M1 — `Cached<T>` and the checker

- `Cached<T>` holds only `T value` (same size/alignment as today's member). Reads are an
  `SS_FORCE_INLINE` conversion operator. Writes go through `set(T)`. Its single constructor is
  `Cached(CachedFlagChecker&, const char* name, Derive fn, Repair fn)`, so an unregistered
  cached flag cannot be declared (the compile-time half of R4). `Derive` and `Repair` are plain
  function pointers plus a `void*` owner context. There is no `std::function` and no allocation.
- `CachedFlagChecker` is a `std::array<Entry, 16>` with a count and a per-entry "suspect"
  counter. `check()` derives each entry's fresh value, increments `suspect` on a mismatch, and
  on the **second consecutive** mismatch runs `SS_ASSERT(cached == fresh, repair(owner))`. One
  check of slack absorbs a queued refresh that is legitimately in flight. Refreshes from GUI
  emitters auto-queue to the pipeline thread by design (spec 0051 M3), and the check rides the
  same FIFO, so one tick of lag is legal and two is a missed wire.
- **Skip predicate per entry:** the dataset-capture pair (`m_changeDriven`,
  `m_captureDatasetValues`) is legitimately stale while `m_captureFlagsDirty` is set or the
  engine epoch moved, so the entry exposes `isSettled()` and the checker skips unsettled
  entries.
- **Repair** calls the existing refresh path (e.g. `refreshLatestFrameCapture()`), not a raw
  write, so edge side effects (`clearLatestFrames()`, `setDashboardAccepting`) stay correct.
- **Owners and threads:**
  - *FrameBuilder checker* (5 flags + BlockPublisher's `m_anyAsyncSink`): pipeline thread, run
    from the existing `timeout1Hz` lambda that already auto-queues there (FrameBuilder.cpp ~:569).
  - *Dashboard checker* (`m_streamAvailable`, derived via the existing const `streamAvailable()`,
    plus a second entry comparing it to the PipelineHost accepting mirror): GUI thread, from the
    existing 1 Hz slot.
- **What each entry verifies** (the hop it can see from its owning thread):

  | Flag | Fresh value from |
  |---|---|
  | `m_operationMode` | PipelineHost atomic mirror |
  | `m_playerOpen` | `anyPlayerOpen()` mask |
  | `m_captureLatestFrame` | `deriveCaptureLatest()` |
  | `m_changeDriven`, `m_captureDatasetValues` | their derive helpers |
  | `m_anyAsyncSink` | sink loop |
  | `m_streamAvailable` | `streamAvailable()` |

  Upstream hops (AppState → mirror, players → mask) are GUI-side mirrors. A third, GUI-side
  checker entry on PipelineHost covers the operation-mode mirror against AppState. The player
  mask stays unverified: it is bus-fed, and the bus is not a polling surface. That residual is
  recorded in M5's debt list.
- **R1 (single derivation):** each refresh slot is rewritten to `flag.set(deriveX())`, so the
  refresh and the checker share one expression.
- **R4 (registration gate):** a `cached-hotpath-flags` ordered anchor captures every
  `Cached<…> m_name` declaration and requires dataflow.md "Cached Hotpath Flags" to list them
  in order. That means a new `Cached<T>` cannot ship undocumented, and a documented flag cannot
  stay a plain member. Residual: a brand-new plain `bool` used as a cache is invisible to any
  tool. ss-hotpath and dataflow.md define a cached hotpath flag as "a `Cached<T>`", which turns
  that case into a definitional review catch. See Tradeoffs.

### M2 — typed lanes

`RepublishGate` keeps its two sets, `clear()`, `noteChanged()`, `notePublishedTemplate()` and
`sinkDirty()` public. `needed(key, changed, bool)` and `notePublished(key, bool)` become
private templates on the trait. `DashboardLane` and `ExportLane` are thin handles holding
`RepublishGate&` and exposing `needed(key, changed)` / `notePublished(key)`. The lanes are
friends of the gate. Each handle carries `static constexpr bool kFeedsExports`, and that is the
only way a lane's identity exists.

In `FrameBuilder`, `republishFrames`, `republishOneFrame` and `emitRepublishedFrame` become
`template <class Lane>`. The callers are `dashboardTick()` → `ExportLane` and
`reprocessFrames()` / the stream-dirty refresh → `DashboardLane`. The sink-mask line at ~:1220
uses `!Lane::kFeedsExports || !frameIsTableFed(frame)`, so the `frameIsTableFed` term stays
independent of the lane. Calling the wrong lane's method no longer compiles (AC3). The
templates are defined and instantiated only in `FrameBuilder.cpp`.

### M3 — `RingCapacity` and `PlotClockState`

- `DSP::RingCapacity`: a private `int` constructor. It has one public factory,
  `fromRate(double windowSec, double rateHz)`, which computes ceil(rate × window) clamped to
  `[1, kMaxRateSizedRingSamples]` (the constant moves from Dashboard.cpp into DSP.h beside the
  type). Two policy transforms take a bound, not a source: `atLeast(int)` and `atMost(int)`. The
  `int value()` accessor exists for the ring internals. There is no implicit conversion from an
  integer, so `EnvelopeRing(64, 1.0)` stops compiling (AC5).
- `TimeRingSizing` holds the existing policies verbatim:
  - Frame lane: `fromRate(window × headroom, kAssumedMaxRateHz).atMost(kMaxTimeRingSamples)`.
  - Stream lane: `fromRate(window × headroom, rate).atLeast(1024)`.
  - Growth: `fromRate(window, 1 / samplePeriod)` clamped the same way, plus the 1.5× hysteresis
    decision returned as an `std::optional<RingCapacity>`.

  `Dashboard::growTimeRing` and `makeHistoryRing` call these.
- `PlotClockState { QMap<int, PlotClock> sources; double displayTimeSec; void reset(); }` lives
  in `DashboardIngest.h` next to `PlotClock`.
  - Dashboard owns one instance.
  - `IngestBindings` carries `PlotClockState&`.
  - `resetPlotClocks()` becomes `m_plotClockState.reset()`.
  - `reconfigureDashboard`'s save/restore copies the whole struct. That makes R10's "no half
    save" structural: there is no longer a separate double to save.
  - The tight-reference warning at DashboardIngest.cpp ~:643 (move-assign invalidates
    references) still applies, now to `sources`.

### M4 — construction guard and single composition entry

- `Core::ModuleConstruction` (`core/Core/ModuleConstruction.h`, Core layer, so every library
  can include it) holds a module-static `const char* s_constructing` plus
  `[[noreturn]] void reportUnavailable(const char* module)`. That function prints
  `"<module>::instance() reached while <module> is under construction"` when `s_constructing`
  names the module. Otherwise it prints `"... before <module> was constructed (pinned order;
  currently constructing <X>)"`. `SessionContext::create<T>` sets and clears the marker with a
  scope guard. Each of the nine `instance()` accessors swaps its `qFatal` literal for
  `reportUnavailable("X")`. This is a plain module-static, the same pattern as
  `MirrorSession::mirroring()`. It adds no construction and no singleton reach.
- `ModuleManager::composeSession(BindMode)`: `Full` runs instantiate → the root's existing
  connection step → `bindInterfaces()`. `SchemaOnly` runs instantiate only. The relative order
  of connections and bind is preserved per root (verified at task time against
  ModuleManager.cpp ~:1177 and ~:1053). `instantiateCoreModules` and `bindInterfaces` go
  private, so a new root cannot instantiate without choosing a bind mode (R13, compile-enforced).
  The schema-dump exemption is now explicit in the call.
- R14 needs no new mechanism. The existing singleton census already buckets function-static
  reaches (`static-cache`, baseline 421) and fails on growth. The startup.md proof section is
  rewritten to say that, and narrows the ctor-edge re-run trigger to "a new function-static
  singleton reach inside a module ctor" (census-visible) instead of "any edit to the closure".
- R15: delete `m_initialized`, and fix startup.md :103 (line citation), :129 (stale gate
  claim) and :230 (the "every root" claim now points at `composeSession`).

### M5 — enforcer markers and triage

- Marker syntax inside docs: `Enforced: <kind>:<name>`, where kind is one of:
  - `code-verify`: a rule id literal in `code-verify.py` / `code_verify_rules.py`.
  - `ctest`: an `ss_add_unit_test(<name>` entry in `app/tests/CMakeLists.txt`.
  - `anchor`: a name in `doc-anchors.json`.
  - `script`: a path that exists, optionally with a flag string present in the script.
  - `hook`: a file in `.claude/hooks/`.
  - `compile`: a symbol, resolved by the existing symbol resolver.
  `Debt:` marks an enforceable-but-not-yet rule. An unmarked rule is a judgment call.
- `check_enforcers()` runs beside `check_symbols` in claim-verify's per-doc loop, honors
  `_visible_lines` fences, and emits `enforcer-missing` (error, baselinable like the others).
  The five existing `Codified:` ledger tags are rewritten to carry a marker, so R18 covers the
  ledger too.
- Triage, applied after M1–M4 land so that the markers name the new mechanisms:
  - Each binding bullet in CLAUDE.md (Threading & Hotpath, Startup, Subsystem Contracts) gets a
    marker.
  - An `Enforced:` bullet collapses to one sentence plus its marker. Its rationale and incident
    history move to the architecture doc it already links (moved, not deleted).
  - `Debt:` entries collect in a new "Unenforced invariants" section at the end of
    common-mistakes.md, ordered by blast radius (silent data loss first) (R19).
  - Before/after line counts are reported in the handoff (AC9).

### M6 — race-free flag inputs (added at the tasks gate)

The pipeline thread reads these today with no synchronization. All become relaxed atomics,
because each is a standalone verdict that gates nothing published through it:

| Field | Reader on pipeline | Writers | Change |
|---|---|---|---|
| `ControlScript::m_running` (`core/Pipeline/DataModel/Scripting/ControlScript.h:93`) | `refreshLatestFrameCapture` | ControlScript.cpp :148, :323, :359, :389 (GUI) | `std::atomic<bool>`, explicit `load`/`store` |
| `API::Server::m_enabled`, `m_clientCount` (`Server.h:186-187`) | `sinkActive()` via `refreshSinkFlag` | Server.cpp :237, :327, :402, :1116 | `std::atomic<bool>` / `std::atomic<int>` |
| `MQTT::Publisher::m_enabled` | `sinkActive()`, and the **per-frame** frame tap at Publisher.cpp :1334 | :825 | `std::atomic<bool>`; the frame-tap read stays one relaxed load (same codegen as a plain load on x86/ARM64) |
| `Widgets::AudioExport::m_activeSessions` (a `QSet`) | `sinkActive()` → `hasActiveSessions()` | AudioExport.cpp :590, :609, :627, :648 | Add `std::atomic<bool> m_anyActiveSession` stored after each mutation; `hasActiveSessions()` reads it, and the set is never read cross-thread |
| `ProjectModel::m_changeDrivenTransforms` | `refreshDatasetCaptureFlag` | ProjectModel.cpp :786, :1075ff; ProjectLoader.cpp :1363 | `std::atomic<bool>`; ProjectPersistence.cpp :136 reads via `load()`. This edits the ProjectModel ctor init list (:67). That is a constant initializer and reaches nothing, so the ctor closure stays reach-free. |

**As built:** MQTT's `sinkActive()` reads the existing `m_hotEnabled` atomic mirror instead
of converting `m_enabled`. `API::GRPC::GRPCServer::m_enabled`
(`app/src/API/GRPC/GRPCServer.h/.cpp`) was a missed eighth sink and became atomic too.

CSV, MDF4, InfluxDB and Sessions already read atomics (`consumerEnabled()` /
`m_exportEnabled`), so no change. `IBlockSink::sinkActive()` gains a `@brief` stating the
contract (pipeline-thread caller, lock-free, race-free) (R22). The `notify`/`Q_EMIT` side of
each setter is unchanged, so the change-signal → cache-refresh wiring is untouched.

## Hotpath & threading impact

- **Touches the hotpath? Yes.** FrameBuilder, BlockPublisher and Dashboard flag reads, the
  republish path, and dashboard ingest are all touched. The rules preserved:
  - `Cached<T>` is a single `T` with an inline read, so per-frame codegen is a plain load,
    identical to today. The checker runs at 1 Hz and never per frame.
  - No allocation anywhere new: the checker uses a fixed array, function pointers and no
    `std::function`. `RingCapacity` is an `int` in a struct. `PlotClockState` holds the same
    members.
  - No new connection on the frame path. The checker rides existing 1 Hz connections. The
    pipeline-side check is auto-queued (tick rate) and the GUI-side check is GUI-local. There is
    no new `BlockingQueuedConnection`, mutex or bus traffic.
  - The lane templates are tick-rate synthetic refresh, not the parse path.
- **New cross-thread signal/slot?** No. The checker reuses the existing `timeout1Hz` →
  FrameBuilder auto-queued hop.
- **New input to a cached flag?** No. The plan wraps the existing flags and changes no inputs.
  The checker reads the same inputs the refresh slots already read from the same thread. The
  pre-existing unsynchronized reads from the pipeline thread are fixed in M6.
- **Timestamp ownership:** unchanged. M3 alters sizing types only, not time sources.
- **Benchmark plan:** the maintainer runs `--benchmark-hotpath` after M1 and after M3, and all
  nine gates must pass (AC10). `lua+dashboard` exercises the ingest/clock path and `lua+exporters`
  exercises the republish lanes indirectly.

## Data model & persistence

None. No project JSON, schema, recording format or API change.

## API / SDK surface

None.

## QML / UI

None.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| M1 check site | (a) new pipeline-thread QTimer; (b) ride the existing 1 Hz auto-queued lambda; (c) check per frame in debug only | **(b)**: no new timer or thread affinity work, FIFO with the refreshes it audits. (c) misses release, where the field failures happened. |
| M1 mismatch trigger | first mismatch vs. two consecutive | **Two**: a legitimately in-flight queued refresh would otherwise abort debug builds spuriously. Recovery is still ≤ 2 s. |
| R4 enforcement | (a) wrapper type + doc anchor; (b) code-verify heuristic flagging any bool member read in a hotpath TU; (c) census of hotpath-TU members | **(a)**: compile-time registration plus a doc-sync anchor. (b)/(c) are noisy and still can't tell a cache from a config field. The residual (a new plain-bool cache) is stated, not hidden. |
| M2 lane identity | (a) two handle types + templates; (b) `enum class Lane` parameter; (c) keep the bool, add an assert | **(a)**: (b) is still a runtime value a caller can get wrong. Only distinct types make a mix-up fail to compile. |
| M3 test literals | (a) test-only int escape hatch; (b) `fromRate(window, rate)` in tests | **(b)**: an escape hatch would be the exact bypass R9 forbids. `fromRate(1.0, 64.0)` reads fine. |
| M3 frame-lane sizing | keep `timeRingCapacity` as a raw sample formula vs. express it as `fromRate(window, kAssumedMaxRateHz)` | **fromRate**: it already is rate × window (1.024 MHz assumed max). Saying so makes R9 uniform. |
| M4 shape | (a) original two-phase `initialize()` after all modules; (b) `initialize()` right after ProjectModel adoption; (c) diagnostic guard + single compose entry | **(c)**: (a) changes AppState's initial frame config, which reads the default Source. (b) moves the hazard without removing it. The runtime adoption assert already covers the failure class, so only diagnostics, bind coverage and doc truth remained. The spec was amended accordingly. |
| R13 enforcement | (a) claim-verify anchor; (b) make instantiate/bind private behind `composeSession` | **(b)**: compile-enforced, and the exemption is explicit in the call. |
| M5 marker form | (a) free-prose `Codified:`; (b) machine-parseable `Enforced: kind:name` | **(b)**: R18 needs a resolvable reference. The prose tags are rewritten. |

## Risks & mitigations

- **Spurious debug aborts from the checker** while a legitimate refresh is mid-flight or during
  session teardown. *Mitigation:* the two-tick rule plus `isSettled()`. The checker is also
  skipped while the pipeline is stopping (it is driven by the same lambda that already guards
  teardown).
- **Repair masking a real bug in release.** *Mitigation:* reported once per site via
  `lcSoftAssert`. The diagnostic names the flag, so field logs show it.
- **`Cached<T>` changing FrameBuilder layout / cache-line packing.** *Mitigation:* same size and
  alignment as `T`, declared in the same position. Benchmark gate.
- **Template republish lanes bloating FrameBuilder.cpp past the TU census.** *Mitigation:* the
  three functions are short, and two instantiations add no source lines. Check `--tu-census`.
- **`composeSession` reordering the GUI root's connection/bind sequence** (common-mistakes:
  "leave a sink out and its recording is a valid-looking empty file"). *Mitigation:* each
  root's current statement order is copied verbatim into its mode, then confirmed with
  `--selftest`, the integration suite and an export smoke check.
- **The ordered `composition-root-order` anchor regex matching new lines.** *Mitigation:*
  `composeSession` contains no `SessionContext::create<` or `(void)X::instance()` text. Run
  claim-verify.
- **Silent-breakage classes exposed:** cached-flag wiring (M1 *is* the mitigation), republish
  lanes (spec 0064, M2 tests), time-ring sizing (2026-08 incidents, `tst_time_ring_sizing`), and
  plot-clock reset (structural after M3).

## Test & verification plan

- **AC1** — `tst_cached_flags`: a fake owner with 2 flags. Corrupt one, run `check()` once (no
  repair, suspect = 1), run it again → repaired. With `SS_ASSERT_NONFATAL` set, the debug path
  is observed via the repaired value. A settled/unsettled skip case. A capacity-16 bound case.
- **AC2** — demonstrated once during implementation: a `Cached<bool>` added without a doc entry
  → claim-verify `anchor-drift`. Reverted, not checked in.
- **AC3 / AC5** — compile-fail probes (wrong lane method; `EnvelopeRing(64, 1.0)`) fed to
  `scripts/syntax-check.py`, demonstrated once and not checked in.
- **AC4** — `tst_republish_lanes`: the 7 existing tests ported, plus "N masked refreshes then an
  export tick publishes" through the lane handles.
- **AC6** — `tst_time_ring_sizing` (44.1 kHz and 48 kHz × 10 s fill the window; saturated
  growth ≥ 1.5×; ceiling clamp), plus the existing `tst_envelope_ring`, `tst_dashboard_ingest`
  (`plotClockContinuesFromBlockSpan`) and `tst_replay_seek_engine`. Maintainer observation: a
  48 kHz audio QuickPlot fills 10 s and survives a rebuild without blanking.
- **AC7** — `tst_module_construction`: a fake module whose ctor reaches its own `instance()`,
  and an accessor reached before create. Each message is asserted via the debug-abort hook.
- **AC8** — demonstrated once: a root calling the now-private `instantiateCoreModules` fails to
  compile. Census: a seeded function-static reach grows `--singleton-census --check`.
- **AC9** — claim-verify passes on the triaged docs and fails on a seeded `Enforced:
  ctest:tst_nope`. Live-guidance line count reported before and after.
- **AC10** — maintainer runs `--benchmark-hotpath` (all nine gates) and the integration suite
  (`pytest tests/integration/ -v`, app up with API server; covers export/recording paths).
- **Static** — `scripts/syntax-check.py` on every edited C++ file; `code-verify.py --check`;
  `--tu-census --check`; `--singleton-census --check`; `layer-verify.py`; `qt-cpp-review`
  before handoff; `sanitize-commit.py` before commit.
- **AC11** — `tst_sink_activity_threading`: a writer thread toggles the `AudioExport`
  session-summary flag and a fake sink's activity while a reader polls `sinkActive()`. The
  maintainer's TSan build is the authoritative check. Read-back of all seven `sinkActive()`
  implementations is recorded in tasks.md.
- **Milestone order:** M6 → M2 → M3 → M1 → M4 → M5 (M6 first, so M1's checker audits
  race-free inputs). M2 and M3 are self-contained and compile-checked.
  M1 is the largest hotpath touch and benefits from the M3 cleanup in Dashboard first. M5 runs
  last so its markers name real mechanisms. Each milestone leaves the tree green.
