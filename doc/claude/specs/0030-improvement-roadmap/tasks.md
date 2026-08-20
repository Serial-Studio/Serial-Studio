# Spec 0030 — Campaign Tracker

> **2026-08-20 update:** `CMakePresets.json` was later removed (maintainer prefers manual
> configures); preset mentions below are historical. Unit tier + CI `unit` job stand.

> Umbrella execution tracker. Maintainer instructed full-roadmap implementation
> (2026-07-25) with per-item artifacts still produced; Claude self-gates phases and
> the maintainer reviews the landed diffs. R1 shipped as spec 0031 (undo/redo).

Status as of 2026-07-25. "Code-complete" means every task in that spec's `tasks.md` is ticked and
the diff is in the working tree; it does **not** mean built, run, or committed — no agent in this
campaign builds, so every `ctest` / `pytest` / benchmark acceptance criterion is the maintainer's.

| Item | Spec | Wave | Status | Depends on | Notes |
|------|------|------|--------|------------|-------|
| R1 undo/redo | 0031 | — | done | — | `ProjectHistory`, shipped, uncommitted in tree |
| R3 C++ unit tier + presets | 0032 | 1 | code-complete | — | T1-T12 + T14 ticked; six ctest suites, `CMakePresets.json`, CI `unit` job. **Open: T13** (seeded-regression proof) and every build/`ctest` acceptance criterion — maintainer |
| R8 problem center | 0033 | 1 | code-complete | — | Core + checkers + pulled counters + `problems.*` API + QML + tests + docs; `Misc::ProblemCenter` pinned after `NotificationCenter` |
| R10 async task trees | 0034 | 1 | code-complete | — | Engine, `HAL_Driver` hook, `ConnectionFlows`, Network/MQTT migrations, `io.getStatus` fields, `test_link_recovery.py`, docs. Build registration closed |
| Adopt: SS_ASSERT, REUSE.toml, UI token lint, presets | — | 1 | landed | — | SS_ASSERT sweep complete (~918 call sites across the four target areas); `REUSE.toml` green; QML/UI token lint rules in `code-verify.py`; `CMakePresets.json` in place |
| R9 connection diagnostics | 0035 | 2 | code-complete | R8 ✓ | `Misc::ConnectionDiagnostics` + `diagnostics.*` API + QML + `test_connection_diagnostics.py` + `test_diagnostics_static.py` |
| R2 property registry | 0036 | 2 | phases 1+2 landed | R1 ✓ | Manifest `app/rcc/properties/dataset.json`, generator + `--check` gate, four generated TUs, `PropertyHooks`, call sites reduced. **Open:** T3 (registry-verify rule), T11 (code-verify parallel-map rule), T12/T13 partials, T14 docs; T1 baseline needs the live app |
| R6 generated API surfaces | 0037 | 3 | code-complete | R2 ✓ | Proto field ledger + typed proto shipped, drift gates in sanitize + CI lint, corpus drift fixed. **Open:** CI `--check-snapshot --strict` fails by design until `api-schema.json` is re-dumped from a commercial build (maintainer) |
| R5 widget-as-extension | 0038 | 3 | code-complete | R2 ✓ | Legal gates answered (2026-07-25): consent-model v1, GPL-3.0-or-later. Catalog + manifest + consent gate + bucketing + config forms + Compass/DataGrid conversions + drift gates + tests + docs. Open: maintainer ACs, deferred old-widget removals |
| R4 session context | 0039 | 3 | M1+M2 landed | R3 ✓ | M1 seam + M2 full ownership: all 8 subsystems SessionContext-owned unique_ptr slots, reverse teardown, 8-wave ctor proof. Open: maintainer 3-mode launches + benchmark tiers vs Wave A baseline |
| R7 remote dashboard attach | 0040 | 4 | code-complete | R4 ✓ | Server (mirror publisher/handler/protocol) + viewer (client/session/attach dialog/T17 stream input) + `--api-external`/`--api-token`. v1 = ProjectFile-only, ungated. Open: maintainer live attach AC + T24-T27 (integration test/help/arch doc) |

## Wave log

- 2026-07-25: campaign started. Wave 1 spec/plan authoring launched (opus agents).
- 2026-07-25: specs 0032/0033/0034/0036/0039 authored (draft). Adopt-directly design done —
  found confirmed UB (ReportData OOB write, BinaryTemplates div0/shift, ProjectHistory deque
  underflow); UB fixes launched ahead of the Q_ASSERT sweep.
- 2026-07-25: implementation in flight — 0032 T1/T2/T10 (build harness, owns app/CMakeLists),
  0033 T1/T2/T4/T5/T7/T8 (core + counters), 0034 engine (app/src/Async), UB fixes, REUSE.toml
  (no ci.yml yet), QML token lint rules. Rule: only coordinator/harness agent touches
  app/CMakeLists.txt; ci.yml edits serialized to closeout.
- Coordinator rulings recorded: 0033 — 1 Hz fixed tick, aggregate notifications, Warning for
  dup indices, problems.* scope, checksum-warning throttle in scope. 0034 — in-repo engine,
  behavior-identical v1. 0036 — defect fixes in-spec as declared deltas, manifest in qrc,
  dataset-only v1. 0039 — plain class, no deprecation markers. SS_ASSERT — 2-arg, debug abort
  + release soft. Pending maintainer: GPL only/or-later, LICENSE_COMMERCIAL.md text,
  0036 T1 baseline capture (needs live app), 0032 T13 seeded-regression proof (needs builds).
- 2026-07-25 (closeout): waves 1 and 2 are code-complete — 0032 (six ctest suites, presets, CI
  `unit` job), 0033 (problem center), 0034 (task trees, both driver migrations, build registration
  closed), 0035 (connection diagnostics) — and the adopt-directly set landed whole (SS_ASSERT
  sweep, REUSE.toml, UI token lints, CMakePresets). Wave 3 is partial: 0036 phases 1+2 landed
  (manifest + generator + four generated TUs + `PropertyHooks`), 0039 M1 landed (SessionContext
  published after the pinned order), 0037 code-complete (ledger + typed proto + revived CI
  gates; snapshot gate red until the maintainer re-dumps api-schema.json), 0038 spec'd and held on the maintainer's
  legal gates. Wave 4 (0040) has its pre-gate artifacts done and waits on 0039 M2. Docs
  reconciled in this pass: `tests/README.md` + `CLAUDE.md` (0032 T12), `architecture/startup.md`
  (pinned order incl. ProblemCenter / ConnectionDiagnostics / licensing block / SessionContext),
  `architecture/project.md` (property registry). **Nothing in the campaign has been built, run,
  or committed** — every `ctest` / `pytest` / `--benchmark-hotpath` criterion is still open, and
  the maintainer legal gates from the previous entry remain unanswered.

### Full-roadmap completion (2026-07-25, session 2)

Maintainer answered the four gate questions (0038 consent-model v1 + signing-later; licensing = GPL-3.0-or-later; commercial license text lives at `LICENSES/LicenseRef-SerialStudio-Commercial.txt`; enable supervised TCP recovery). All of it then landed:

- **License correction sweep:** 610 SPDX `GPL-3.0-only` → `GPL-3.0-or-later` (dual-license form), 504 prose repoints to the real commercial-license path, 14 edge-case files, REUSE.toml annotations. `reuse lint` green 4223/4223.
- **SS_ASSERT.h namespace collision fixed** (shadowed `class SerialStudio` → renamed helpers to `SSAssertDetail`; this was the build-breaker in the maintainer's first compile).
- **0034 v2:** BLE migration (real timeouts, errorOccurred wired, epoch guard), UART supervised auto-reconnect, Modbus event-loop removed, ZMODEM retry-reset fix, TCP supervised recovery (F3 give-up path).
- **0036 closed:** manifest gate, parallel-map lint, descriptor-driven multi-select, extensibleMap dataset picker, tests, docs. R11 final −91%.
- **0038 R5 fully implemented** across four phases (consent, config forms, Compass+DataGrid conversions, drift gates, authoring example).
- **0039 M2 complete:** 8-wave ownership migration to SessionContext, reverse teardown, consolidated ctor proof.
- **0040 R7 complete:** server mirror + viewer attach.
- Serial diagnostics false-failure fixed (checked UI driver portList vs QSerialPortInfo); diagnostics UI moved from Setup pane to Problem Center dialog header; ProblemCenter dialog made draggable.
- Census re-baselined (1595/1123). Full sanitize + all verifiers green (0 code-verify errors, 338 script/unit tests, registry/docs/reuse clean).

Nothing committed. Maintainer still owns: full build + `ctest` + `--benchmark-hotpath` (M2 Wave A baseline + FrameBuilder release-assert cost) + pytest integration + live-hardware ACs + `--dump-api-schema` regen (CI snapshot gate red until then) + spec baseline captures + the first commit.

### Post-review fixes landed (2026-07-25)

Eight confirmed findings from the campaign code review, fixed in the working tree (not built, not
committed):

- post-review fixes landed: `Async::RetryPolicy::initialConnect()` is a flat 300 ms x 5 schedule
  (multiplier 1.0, initial = ceiling), so a fully failed user-initiated connect stays inside the
  blocking budget the wait cursor used to cover; `autoReconnect()` untouched, spec 0034 T5 note and
  `tst_async_engine`'s schedule table updated.
- post-review fixes landed: `ConnectionManager::disconnectDevice()` no longer emits
  `connectedChanged` twice on the disconnect-while-connecting path. The fallback emission runs
  only when `concludeConnectRequest()` did not settle a pending request.
- post-review fixes landed: `disconnectDevice(int)` and `disconnectDevice(HAL_Driver*)` conclude the
  connect request after teardown, so a device closed mid-open can no longer strand `m_connectPending`
  and its wait cursor; both paths still emit exactly one `connectedChanged`.
- post-review fixes landed: MQTT reports `openFinished(false, ...)` when the open flow succeeds but
  the session is already gone (broker dropped between subscribe and completion), so
  `DeviceManager::m_opening` clears; the settings-change teardown flow stays silent.
- post-review fixes landed: `MQTT::beginOpen()` clears `m_failureNotified`, restoring the documented
  one-error-box-per-open-request behavior.
- post-review fixes landed: `FrameReader::processData()` recovers from an out-of-range operation mode
  by dropping the chunk instead of assigning `ConsoleOnly`, which permanently muted the reader.
- post-review fixes landed: the three per-frame `SS_ASSERT_LOG` conditions in `Dashboard` that ran
  `widgetCount()` / `m_widgetGroups.contains()` are now fenced debug-only `Q_ASSERT`s, per the
  SSAssert.h cost contract.
- post-review fixes landed: the `m_points` clamp recoveries in `configureLineSeries()`,
  `configurePlot3DSeries()` and `configureMultiLineSeries()` emit `pointsChanged()` with the
  assignment, matching every other write site.

### SS_ASSERT hotpath regression fixed (2026-07-27)

The first post-campaign CI run with a working optimized gate (582dad5d; the runs in between lost
the gate to the big_db PGO-training crash skipping it) showed the SS_ASSERT sweep cost ~5% of
hotpath throughput: `HOTPATH_STAGE_PUBLISH_NS` 291-317 → 360 on the optimized gate, instrumented
`HOTPATH_NATIVE_FPS` 785-819k → 749-752k, tokenize 221-257 → 266-306 ns, all bracketed exactly at
c249720a. Root cause: `Q_ASSERT` compiled out of release; `SS_ASSERT` evaluates its condition in
every build and the sweep placed it on per-frame, per-dataset, and per-cell kernels — worst site
was `assign_utf8_in_place` (`Frame.h`), the span lane's per-cell QString write. Instrumented
builds amplified it further: `-fprofile-update=atomic` adds an atomic counter to every new branch.

Fix landed (not built, not committed):

- `SS_ASSERT_HOTPATH(cond)` added to `SSAssert.h`: debug = `SS_ASSERT_LOG`; release =
  `static_cast<void>(false && (cond))` (parsed, never evaluated, zero codegen). No recovery
  action by design — it would never run. Admissibility matches SS_ASSUME (condition restates a
  guard that provably already ran) without the optimizer promise.
- ~45 sites migrated across `Frame.h`, `FrameBuilder.cpp`, `FrameReader.cpp`, `Dashboard.cpp`,
  `CircularBuffer.h` — the per-frame chain, per-dataset applies, per-cell formatters, and the
  delimiter-scan kernels. Kept as `SS_ASSERT`: `checksum()` (device bytes), per-chunk boundary
  entries (`processData`, `appendChunk`, `readX*`), `findFirstOfPatterns` pattern-count (user
  config, clamp is load-bearing), and all reconfigure/compile/cold paths.
- Blocking `hotpath-assert-scope` rule in `code-verify.py` pins the macro to the hotpath TU
  whitelist.

Maintainer gate: rerun `--benchmark-hotpath` — expect publish back near ~315 ns and native
above 2.05M on the optimized run.
