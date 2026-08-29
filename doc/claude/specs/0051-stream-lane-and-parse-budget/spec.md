---
spec: 0051-stream-lane-and-parse-budget
title: Per-source parse budget + typed stream lane for high-rate sample sources
status: done          # closed 2026-08-20
created: 2026-08-11
author: Alex Spataru
---

# Spec 0051 — Per-source parse budget + typed stream lane for high-rate sample sources

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

A real industrial project (BADAQ: one CAN bus at ~10 Hz plus two stereo audio sources
running per-sample Lua transforms at 48 kHz) degraded the entire dashboard to a visible
1 Hz update rate (`bug-report.md`, 2026-08-11). Two independent failures compound:

**The parse-load circuit breaker is global and phase-locked.** Parse time is accumulated
into a single accumulator over a fixed 1 s window; once the 800 ms limit trips, every
frame from every source is dropped until the window rolls over. The CAN source contributed
microseconds of parse time per second yet its frames were dropped alongside the audio's.
The burst-then-starve cycle reads as "the app runs at 1 FPS", and nothing in the dashboard
says frames are being dropped — the only signals are a console warning and a one-shot
message box.

**Dense sample streams are forced through the per-frame telemetry pipeline.** The frame
pipeline was designed for sparse structured telemetry (CAN, serial: one meaningful frame
at a time, values as strings). Audio is a different data class: a dense, homogeneous,
typed signal. Today each audio sample is individually formatted to CSV text by the driver,
delimiter-scanned, re-parsed back to a number, stored as a string, run through one script
call per transform-bearing dataset (~1–2 µs per call boundary — BADAQ's 8 datasets at
48 kHz is ~384k boundaries/s, i.e. 400–800 ms of work per second on its own), then
published as one dashboard frame per sample with a full deep copy per sample when any
exporter is on. This costs microseconds per sample regardless of which thread runs it, so
one core saturates at a few hundred kilosamples/s and every added channel makes it worse.
The target the application must actually serve is *n* audio inputs at up to 96 kHz plus
heavy per-dataset DSP — BADAQ had to degrade to 16 kHz capture and merged transforms just
to stay under the breaker.

The GUI thread also hosts all of this work today, so parse overload and UI responsiveness
are coupled — that coupling is *why* the breaker exists. Moving all data processing (the
frame pipeline wholesale, plus per-sample stream work) off the GUI thread and charging
budgets per source removes the GUI-protection rationale for a global breaker entirely;
what remains of the budget is per-source overload control.

## Goals

- A cheap source is never starved by an expensive one: with BADAQ-class load (one
  overloaded scripted source + one light source), the light source's dashboard widgets
  keep updating at their natural rate.
- Overload degrades smoothly instead of beating at 1 Hz: an over-budget source visibly
  thins (processes a fraction of its frames) rather than freezing for the rest of a
  fixed window.
- Overload is self-diagnosing: the user can see, in the app, which source is over
  budget and by how much, while it is happening.
- Sample-stream sources (audio today) flow through a typed block path with no
  text round-trip and no per-sample script call: *n* stream sources at 96 kHz with
  per-sample DSP run concurrently without degrading the dashboard, the frame pipeline,
  or the GUI.
- Per-sample DSP stays practical in user scripts: a dataset may define a block-form
  transform that is invoked once per captured block instead of once per sample, with
  per-sample fallback preserved for existing projects.
- Stream-source data still reaches everything it reaches today: plots/FFT/widgets,
  CSV/MDF4/session exports, the data-table store (for BADAQ-style metric engines), and
  API consumers — at rates each consumer can actually use.
- The GUI thread spends no per-sample time on stream sources; its work per stream source
  is bounded per display tick, independent of sample rate.
- The frame (telemetry) pipeline itself runs off the GUI thread: a pathological parser or
  transform script can peg its processing core at 100% while the UI stays fully
  responsive, and the interactive duty-cycle throttle (the 80% breaker's original
  purpose) is no longer needed to protect rendering.
- The application scales across cores by construction: GUI thread + one frame-processing
  thread + one worker per stream source, each independently schedulable by the OS.
- Lua scripting is fast by default and maximally fast by explicit choice: the runtime
  swap alone speeds every safeguarded Lua script (~6× measured on the block-DSP
  benchmark), and users who vouch for their scripts can trade the runaway watchdog for
  JIT execution (~40×) per project — sized so heavy DSP holds the 96 kHz target on
  Atom/Celeron-class industrial hardware.

## Non-Goals

- **No per-source parallelism *within* the frame pipeline.** The frame pipeline moves off
  the GUI thread as one unit (R16) but stays single-threaded internally: frame-lane
  sources share ordering guarantees (transforms see earlier datasets' finals) and the
  data-table bus, and BADAQ-style metric engines read across sources through it —
  sharding it means locks or lost semantics for sources that, once streams leave the
  lane, are sparse (the CI gate already proves >1 MHz single-core native parse).
  Revisit only if a measured frame-lane workload saturates the processing thread.
- **No source-level multi-channel DSP script.** The block transform is per-dataset
  (chosen 2026-08-11); a cross-channel source-level script is a possible later spec.
- **No Coco integration, no bytecode caching, no new scripting language.** Coco
  addresses yielding across C call boundaries in coroutine schedulers — nothing in this
  design yields mid-block. Scripts are compiled once per project load already; bytecode
  caching saves load time only. (The Lua *runtime* does change — see Part D.)
- **No change to replay/player ingestion semantics.** Players keep their existing lanes.
- **No removal of the per-sample transform contract.** Existing projects with
  `transform(value)` keep working unchanged on both lanes.
- **No crash reporting / telemetry of any kind.**

## Requirements

### A — Parse-load budget redesign (frame lane)

1. **R1 — Per-source accounting.** Parse/transform time is charged to the source that
   incurred it. A budget violation affects only that source's frames; all other sources
   continue processing normally.
2. **R2 — Fair-share decimation instead of gating.** While the processing thread has
   spare capacity (total load under ~90% of its core), no source is thinned regardless
   of individual cost. Under saturation, only sources above their fair share are
   thinned — every Nth frame processed, N proportional to that source's overrun —
   and light sources are never touched. Frames are never dropped in a solid block that
   spans the rest of a fixed window. (Exact thresholds tuned in the plan via benchmark.)
3. **R3 — Continuous recovery.** Budget accounting is smoothed (EWMA-class, settling in
   the low hundreds of ms; no fixed-window phase lock): after load drops, a source
   returns to full rate within a bounded, sub-second settling time rather than at the
   next window boundary.
4. **R4 — Visible overload state.** While any source is being decimated, the app surfaces
   (a) a problem-center entry naming the source and its measured parse load, live-updated,
   and (b) a visible dashboard indication that data is being thinned. Both clear when the
   source recovers.
5. **R5 — Unchanged full-throughput behavior.** With all sources under budget, behavior
   and throughput are identical to today (the benchmark gate still disables the budget
   guard; no per-frame cost is added to the under-budget path beyond what exists today).

### B — Typed stream lane (sample-stream sources)

6. **R6 — Typed block capture.** Sample-stream drivers (audio) publish typed sample
   blocks (native numeric samples + channel count + start timestamp + per-sample step).
   No text serialization of samples anywhere between capture and consumers that need
   numbers. Stream designation is driver-intrinsic by default (a driver capturing
   native typed samples is a stream source) with a per-source project flag that can
   override the default, so a high-rate structured feed can opt into the stream lane
   later without a schema change.
7. **R7 — Off-GUI per-sample work.** All per-sample processing for a stream source
   (decode, DSP, block transforms, display reduction, export marshalling) runs off the
   GUI thread. Work crossing into the GUI thread is per-block/per-tick, bounded and
   independent of sample rate.
8. **R8 — Per-source concurrency.** Each stream source's processing is independent:
   two stream sources at 96 kHz utilize two cores; adding a stream source does not slow
   the others (until physical cores are exhausted).
9. **R9 — Block transform contract.** A dataset on a stream source may define
   `transform_block(samples, info) -> samples` (same language options as today). When
   present it is called once per captured block with the dataset's samples; when absent,
   the existing per-sample `transform(value)` contract applies (called per sample, on the
   stream worker, at full rate). Both forms coexist per project; per-sample fallback
   requires no project migration. `info` carries exactly: `sourceId`, `uniqueId`,
   `blockNumber`, `timestampMs`, `sampleRate`, `count`, and `firstSampleIndex` (running
   per-dataset sample counter, enabling phase-coherent DSP without user-managed state).
   This surface is frozen at ship.
10. **R10 — Script safety parity.** Block transforms run under the same watchdog/timeout,
    error-reporting, and sandbox rules as existing transforms: a hung or throwing block
    transform is aborted and reported without stalling capture, other sources, or the GUI.
11. **R11 — Dashboard fidelity.** Stream-fed plots and FFTs render with no loss of visual
    information relative to today: plot rings receive a min/max-faithful representation of
    the full-rate signal (an envelope is acceptable; naive every-Nth decimation that can
    hide peaks is not), FFTs receive full-rate windows, value widgets show the latest
    sample at display rate. Envelope resolution targets the plot's actual display pixel
    width (per-display-pixel min/max; maintainer decision 2026-08-11 — the plan must
    handle viewport-resize communication to workers without per-frame cross-thread
    traffic).
12. **R12 — Export fidelity.** CSV/MDF4/session exports of stream sources record the
    full-rate transformed signal (every sample, post-transform), not the display
    reduction. Export backpressure must not stall capture or the GUI; sustained
    over-rate export falls behind visibly (existing pool-exhaustion warning semantics)
    rather than silently dropping samples.
13. **R13 — Data-table integration at block rate.** Stream datasets publish their values
    into the data-table store at block rate (latest value per block), so metric-engine
    transforms and control scripts keep reading them; per-sample store writes are not
    performed. Store interaction from stream workers must not introduce data races with
    frame-lane transforms.
14. **R14 — Lane coexistence.** A project mixing frame-lane sources and stream sources
    behaves as the union: frame sources keep today's semantics and ordering; stream
    sources follow R6–R13; the combined dashboard updates all widgets at their natural
    rates. BADAQ's original configuration (CAN + 2× stereo audio at 48 kHz with 8
    transform-bearing datasets) runs without tripping any budget and with every widget
    live.
15. **R15 — Mode coverage.** The stream lane serves both ProjectFile and QuickPlot modes
    for audio sources (QuickPlot audio currently rides the same text path).

### C — Frame pipeline off the GUI thread

16. **R16 — Off-GUI frame processing.** Frame extraction, parsing, per-dataset transforms
    and frame assembly for frame-lane sources run off the GUI thread, as a single
    processing unit that preserves today's ordering and data-table semantics. The GUI
    thread receives finished frames in batches bounded per display tick.
17. **R17 — UI immune to script load.** With a frame parser or transform script consuming
    100% of the processing unit's core, the UI remains fully interactive (menus, dialogs,
    window drag, widget rendering at display rate) and every other source — frame-lane
    or stream — keeps updating.
18. **R18 — Ordering and consumer contracts preserved.** Relative to today, consumers
    observe no reordering: per-source frame order is preserved, exports record the same
    sequences, transform ordering (raw-of-all, final-of-earlier) is unchanged, and
    timestamps still originate at the capture boundary.

### D — Lua runtime: LuaJIT with a user-controlled safety mode

Grounding (measured 2026-08-11, scratchpad `block_transform_bench.lua` /
`bench_interrupt.lua`, M2 Pro, heavy biquad + RMS per sample): Lua 5.4 with the
count-hook watchdog armed ≈ 6.1 Msamples/s; LuaJIT interpreter mode (hooks honored)
≈ 36 Msamples/s; LuaJIT with JIT active ≈ 255 Msamples/s **but debug hooks never fire
inside compiled traces — a runaway loop ran to completion un-interrupted**, so a JIT-on
engine cannot offer watchdog aborts. Decision (maintainer, 2026-08-11): ship LuaJIT and
make the tradeoff a per-project user choice; floor hardware is Atom/Celeron-class Intel,
where the derated numbers still clear the R8 target in either mode.

19. **R19 — Wholesale runtime swap.** The vendored Lua 5.4 runtime is replaced by
    vendored LuaJIT across every Lua surface (frame parsers, per-dataset transforms,
    block transforms), with a compatibility layer restoring the newer-Lua library
    functions shipped scripts rely on (`string.pack`/`unpack`, `table.unpack`, etc.).
    The supported script language surface (LuaJIT 5.1 + shims) is documented.
20. **R20 — Per-project Lua execution mode.** A project-level setting (same pattern as
    change-driven transforms) selects: **Safe** — interpreter mode, watchdog armed,
    runaway scripts aborted and reported (default for new and existing projects); or
    **Fast** — JIT enabled, watchdog structurally off, chosen through an explicit
    consent step that names the tradeoff (a runaway script stalls its source until
    disconnect). The two are one mode switch because JIT-on with a "watchdog" would be
    a silently dead safety control; the app never advertises watchdog protection in
    Fast mode.
21. **R21 — Fast-mode containment.** A runaway script in Fast mode stalls only its own
    source's processing; every other source, the GUI, and app lifecycle stay live.
    Disconnect, project change, and quit never block on a hung script worker
    (warn-and-abandon teardown), and the user is told which dataset/script hung.
22. **R22 — Script compatibility.** All shipped Lua parser/transform templates and
    importer-generated scripts (DBC, Modbus map) run in both modes with results equal
    to today's Lua 5.4 output; the 13 shipped scripts and the DBC generator that use
    5.3 bitwise/floor-division syntax are migrated to LuaJIT-compatible form
    (`bit.*`, `math.floor`) inside this spec. A user script using 5.3-only syntax
    fails at load with an actionable error (construct named, replacement suggested).
    The "Lua never aborts host" contract is re-established on LuaJIT's error model
    (every VM entry through protected calls; per-toolchain verification), replacing
    the 5.4-as-C++ exception machinery.
23. **R23 — Mode performance.** On floor hardware (Atom/Celeron-class), Safe mode
    sustains at least 3× today's hooked-5.4 block-DSP throughput, and Fast mode
    sustains the full R8 target (n × 96 kHz with heavy per-sample transforms) with
    headroom.

### E — Block-rate stream API

24. **R24 — Block-rate API subscription.** API clients can subscribe to a stream
    source's typed blocks (post-transform) and receive them at block rate with their
    timestamps and sample metadata, enabling external real-time DSP consumers. A slow
    subscriber must not stall capture, workers, or other consumers: backpressure policy
    is drop-oldest-with-count (the subscriber learns how many blocks it missed), never
    unbounded buffering. Subscription lifecycle follows the existing API server session
    rules (per-client, cleared on disconnect).

## Acceptance Criteria

- [x] **AC1 (R1/R2/R3)** — Reproduction from `bug-report.md` (10 Hz source + one audio
  source with duplicated trivial Lua per-sample transforms sized to exceed the budget):
  the slow source's dashboard values update at 10 Hz throughout; the audio source thins
  smoothly (no 1 s lockstep steps). Verified via API-driven integration test polling
  `dashboard.getData` timestamps for both sources, plus maintainer observation in-app.
- [x] **AC2 (R3)** — After the overload script is removed (project edit or disconnect/
  reconnect), the previously-thinned source returns to full rate in < 1 s. Integration
  test on frame counters via the API.
- [x] **AC3 (R4)** — While thinning is active, the problem center lists the offending
  source with a live parse-load figure, and the dashboard shows a thinning indicator;
  both clear within 2 s of recovery. Maintainer observation + API problem-center query
  where exposed.
- [x] **AC4 (R5)** — `--benchmark-hotpath` gated tiers (Native 1.024 MHz … JS mixed
  64 kHz) pass unchanged on the PGO binary. CI gate.
- [x] **AC5 (R6/R7)** — With one 96 kHz stereo stream source connected and per-sample Lua
  transforms on every channel, GUI-thread profiling shows no per-sample work attributable
  to the stream source (maintainer profiling run), and the dashboard stays at full UI
  frame rate (existing render-FPS observation).
- [x] **AC6 (R8)** — Scaling run: 1 → 2 → 4 concurrent 96 kHz stream sources with block
  transforms; per-source throughput (samples processed/s, exposed via diagnostics
  counters) stays within 10% of the single-source figure while cores remain available.
  Interim on the dev machine with the 5× derate margin; provisional until re-run on a
  physical floor box.
- [x] **AC7 (R9)** — `tests/scripts/`-style unit coverage for the block contract:
  `transform_block` present → called once per block with the full sample array, output
  array replaces samples; absent → `transform(value)` called per sample with identical
  results to today's path on the same input vector (bit-exact for numeric passthrough).
- [x] **AC8 (R10)** — A block transform containing `while true do end` (Lua) and a JS
  equivalent: capture continues, the dataset falls back to raw samples for that block,
  the transform-error diagnostics report it, GUI never blocks. Integration test +
  maintainer observation.
- [x] **AC9 (R11)** — Synthetic single-cycle impulse injected into a 96 kHz stream: the
  plot ring's rendered envelope contains the impulse peak (no peak swallowed by
  decimation); FFT of a known sine shows the expected bin. Scripted check against
  `dashboard.getData` / ring snapshots where exposed, else maintainer observation with a
  signal generator.
- [x] **AC10 (R12)** — CSV and MDF4 exports of a 10 s 96 kHz capture contain
  (sample-rate × 10 s) ± one block of rows/samples with post-transform values; checksum
  comparison against the known input vector. Integration test with a synthetic stream
  source or loopback audio.
- [x] **AC11 (R13)** — A frame-lane virtual dataset whose transform reads a stream
  dataset's table slot updates at block rate with the stream's latest value; ThreadSanitizer
  (or equivalent maintainer-run race check) is clean over a mixed-lane session.
- [x] **AC12 (R14)** — Restored BADAQ project (48 kHz, original 8 transform datasets,
  un-merged metrics engine) on the maintainer's machine: no budget warning, CAN and audio
  widgets all live, APS500 painter values advance at data rate. Maintainer observation —
  this is the incident's definition of done.
- [x] **AC13 (R15)** — QuickPlot with an audio device: waveform + FFT live at the
  configured sample rate, no text path in the trace. Maintainer observation.
- [x] **AC14 (R16/R17)** — With a deliberately expensive (bounded, non-hanging) Lua
  frame parser saturating the frame-processing unit, the UI stays interactive (window
  drag, dialog open, menu navigation judged fluid by the maintainer; render FPS at
  display rate) and a concurrent stream source's widgets keep updating. Maintainer
  observation + existing render-FPS readout.
- [x] **AC15 (R18)** — A recorded session captured before and after the change on the
  same input replays identically: same frame sequences per source, same export rows,
  same final dataset values (golden-session regression harness, spec 0047 machinery).
- [x] **AC16 (R19/R22)** — The full shipped script corpus (parser templates, transform
  templates, DBC/Modbus importer outputs) passes `tests/scripts/`-tier execution on
  LuaJIT in both modes, with numeric outputs equal to the recorded Lua 5.4 golden
  vectors (numbers cross the C boundary as doubles in both runtimes, so equality is
  exact, not approximate).
- [x] **AC17 (R20)** — Safe mode: the AC8 runaway-transform scenario still aborts and
  reports within the watchdog budget on LuaJIT. Fast mode: the same scenario stalls
  only its source; other sources and the GUI stay live; disconnect completes without
  blocking and names the hung script (integration test + maintainer observation).
- [x] **AC18 (R20)** — Switching a project Safe → Fast requires the consent step once;
  the setting persists in the project file; existing projects load in Safe with no
  prompt. Maintainer observation.
- [x] **AC19 (R23)** — Safe-mode block-DSP throughput ≥ 3× the hooked-5.4 baseline
  recorded before the swap, and Fast mode sustains 8 ch × 96 kHz with the
  heavy-transform benchmark with ≥ 2× headroom. Interim: run on the dev machine with
  a 5× single-thread derate margin (targets must clear at ×5); marked provisional
  until re-run on a physical floor box (see Constraints).
- [x] **AC20 (R24)** — An API client subscribes to a 96 kHz stream source and receives
  post-transform blocks with correct timestamps/metadata; a deliberately slow client
  observes drop-oldest behavior with an accurate missed-block count while capture,
  dashboard, and a second fast client remain unaffected. Integration test via
  `tests/utils/api_client.py`.

## Constraints & Invariants

- **Per-sample work must never cross a thread boundary or reach the GUI thread; cross-
  thread traffic is per-block.** (The deciding constraint. The repo already learned that
  per-frame queued hops collapse at ≥10 kHz.)
- Must not regress the 256 kHz hotpath CI gate or any of its tiers (AC4).
- Frame-lane invariants stay intact on their new owning thread: the frame path remains
  single-threaded internally (SPSC buffers, no new mutexes on the per-frame path), zero
  steady-state allocation on the dashboard path, direct (non-queued) hops within the
  processing unit, cached hotpath flags (refreshes must remain race-free across the
  GUI/processing boundary), source-owns-time timestamping, transform ordering (a
  transform sees raw of all datasets, final of earlier datasets only).
- GUI ↔ processing traffic is batched: per display tick for dashboard updates, never
  per-frame queued signal emissions at data rate (the 65536-slot queue lesson).
- Stream sources obey source-owns-time: block timestamps originate at the capture
  boundary and are never re-stamped downstream.
- Script sandbox rules are unchanged (safe-libs Lua, watchdog/interrupt discipline in
  Safe mode, `guardedCall` for JS); block transforms add no new sandbox surface. On
  LuaJIT the `ffi` and `jit` modules are never exposed to user scripts in either mode
  (FFI is a full sandbox escape).
- The "Lua never aborts the host" contract survives the runtime swap on every supported
  toolchain (error unwinding across the C++ boundary, panic handling, exception-safety
  parity with today's vendored 5.4 — this has bitten before on macOS).
- Floor hardware is Atom/Celeron-class Intel (maintainer decision 2026-08-11). No such
  box is in the lab yet: floor-sensitive ACs (AC6, AC19) run interim on the dev machine
  against a 5× single-thread derate margin and are marked provisional until validated
  on a physical floor box.
- Existing projects (per-sample transforms, current audio configs) load and run with no
  migration and no behavior change other than improved throughput/overload behavior —
  with ONE amended exception (maintainer decision 2026-08-11, after planning surfaced
  it): user Lua scripts written with Lua 5.3-only syntax (`<<` `>>` `&` `|` `~` `//`)
  do not parse on LuaJIT. The shipped script corpus and importer generators are
  migrated to LuaJIT-compatible form as part of this spec; a user script that fails
  for this reason must produce an actionable load-time error naming the construct and
  its `bit.*` replacement, never a silent dead parser.
- Audio capture is a commercial feature today; the stream lane must respect existing
  license gating and add no new gating of its own.
- No new third-party dependency beyond the vendored LuaJIT runtime (replacing vendored
  Lua 5.4) and its compatibility shim, version-pinned like the runtime it replaces.
- Teardown discipline: stream workers must join deterministically on disconnect/project
  change/quit (this codebase's recurring crash class is worker teardown — treat it as a
  first-class requirement of the design, not an afterthought).
- ProjectFile and QuickPlot modes both covered; ConsoleOnly unaffected.

## Open Questions

None — all resolved with the maintainer, 2026-08-11:

- **Q1 (budget semantics)** → fair-share + proportional decimation, EWMA smoothing;
  folded into R2/R3. Exact thresholds tuned in the plan via benchmark.
- **Q2 (`transform_block` info)** → minimal + `firstSampleIndex`; frozen in R9.
- **Q3 (stream designation)** → driver-intrinsic default + per-source project flag
  override; folded into R6.
- **Q4 (envelope resolution)** → per-display-pixel; folded into R11 (plan must solve
  viewport-resize communication without per-frame cross-thread traffic).
- **Q5 (API exposure)** → block-rate subscription API in scope now; R24/AC20.
- **Q6 (floor box)** → no physical box yet; derated-proxy interim (5× margin on dev
  machine), AC6/AC19 provisional until a floor box validates them; in Constraints.
