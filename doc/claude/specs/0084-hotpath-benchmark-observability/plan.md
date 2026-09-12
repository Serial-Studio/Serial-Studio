---
spec: 0084-hotpath-benchmark-observability
phase: plan
status: approved     # draft -> approved (gate before /ss-tasks)
updated: 2026-09-11
---

# Plan 0084 — Hotpath benchmark observability

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Read the relevant `doc/claude/` sub-docs and the *actual code*
> before writing this — a plan grounded in a stale mental model is worse than no plan.
> Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

Three independent additions to the benchmark harness and the build, no pipeline code touched.
**Allocation count**: mimalloc 3.4.5 keeps per-thread heap statistics when compiled with
`MI_STAT=2`; a new CMake option `SS_ALLOC_STATS` (default OFF) turns that on for the fetched
allocator and defines `SS_ALLOC_STATS=1` for the app, and `HotpathBenchmark` samples
`mi_heap_stats_get(mi_heap_main())` around each timed loop, subtracting the event-loop
pump the same way it already subtracts its wall time. CI sets the option only on the
PGO-instrumented configure that every release job already runs, so the training run doubles as
the allocation gate (the count is a property of the code, not of the optimization level) and
the shipped PGO-use binary prints `n/a`. **Width knob**: `--benchmark-channels N` threads a
`channels` argument from `CLI` through `runAndReport` into `run`, `runDataPipeline` and
`measureNativeStages`; the dashboard rows keep their fixed all-widget project. **Symbols**:
non-MSVC production branches gain `-gline-tables-only` (`-g1` on GCC), the commercial
hardening block emits a dSYM / `.debug` file before it strips, the clang-cl branch stops
discarding its PDB, and each release job uploads a `symbols-<platform>` artifact.

## Affected subsystems & files

| File | Change |
|------|--------|
| `app/src/Benchmark/HotpathBenchmark.h` | `Result` gains `allocations` (quint64) and `allocationsPerFrame` (double, `-1` = n/a); `run`/`runDataPipeline`/`runAndReport`/`measureNativeStages` take `int channels`; `kDefaultChannels = 8` public. |
| `app/src/Benchmark/HotpathBenchmark.cpp` | Allocation sampling helpers (`#if SS_ALLOC_STATS && SS_MIMALLOC_ACTIVE`); pre-built captured chunk reused across the loop; pump and dashboard drain excluded from the count; width plumbed; report column, header line, `HOTPATH_*_ALLOC_PER_FRAME`, `HOTPATH_CHANNELS`, `HOTPATH_ALLOC_GATE`; Native alloc gate folded into `allPassed`; throughput gates skipped at non-default width; span/list lane label. |
| `app/src/Benchmark/HotpathReport.h` / `.cpp` (new) | Report rendering extracted from `HotpathBenchmark.cpp` as the static-only `Benchmark::HotpathReport` class (row constants, table, ratio lines, gate verdicts, summary keys): the benchmark TU crossed the 1500-line census cap once the allocation column and gates landed (implementation note, 2026-09-11). |
| `app/src/Benchmark/BenchmarkRunner.cpp` | Untouched: `channels` is a defaulted trailing parameter on `run` / `runDataPipeline`, so the dialog's call sites compile unchanged (implementation note, 2026-09-11). |
| `core/Pipeline/DataModel/FrameBuilder.h` | One line: `kMaxSpanFields` moves from the private to the public section so the benchmark sizes its span array and labels the lane against the real cap instead of a duplicated constant (implementation note, 2026-09-11; the only touch under `core/`). |
| `app/src/Misc/CLI.h` | `benchmarkChannelsOpt` (`--benchmark-channels <count>`). |
| `app/src/Misc/CLI.cpp` | Registers the option, adds it to `isBenchmarkRequested`, parses and clamps it, forwards to `runAndReport`. |
| `CMakeLists.txt` | `option(SS_ALLOC_STATS ... OFF)` next to `SS_USE_MIMALLOC`. |
| `cmake/MiMalloc.cmake` | When `SS_ALLOC_STATS`: `target_compile_definitions(<mimalloc target> PRIVATE MI_STAT=2)` and `SS_ALLOC_STATS=1` on the app in `target_link_mimalloc`. |
| `cmake/Optimization.cmake` | `-gline-tables-only` (Clang/AppleClang/MinGW-Clang) or `-g1` (GCC) in each non-MSVC production branch. |
| `app/CMakeLists.txt` | Hardening block: `dsymutil` (macOS) / `objcopy --only-keep-debug` + `--add-gnu-debuglink` (Linux) POST_BUILD *before* the existing strip; drop `/DEBUG:NONE` so clang-cl keeps its PDB. |
| `.github/workflows/ci.yml` | `-DSS_ALLOC_STATS=ON` on the five PGO-GENERATE configures; training steps write `--benchmark-output pgo-train.txt`, Windows checks `HOTPATH_PASS=1`; upload `pgo-train.txt`; upload `symbols-<platform>` after the build (dSYM / `.debug` / PDB). |
| `.github/actions/pgo-train-linux/action.yml` | Same training-step change for the two Linux jobs. |
| `doc/help/Command-Line-Interface.md` | Document `--benchmark-channels`. |
| `doc/claude/architecture/dataflow.md` | "Hotpath Benchmark" section: allocation column, gate location (training build), width knob, 128-field span cap note. |
| `.claude/skills/ss-hotpath/SKILL.md` | Tier table gains the allocation gate row. |
| `doc/claude/scripts.md` | None (no new script). |
| `tests/scripts/test_cpp_regressions.py` | Docstring of the `claimPoolSlot` scan says "retire when the benchmark grows a per-frame allocation counter"; keep the test (it runs without a build), update the docstring to point at the gate. |

Nothing under `core/` changes behavior; the one edit there is the access-level move above.

## Architecture & data flow

**Allocation sampling.** Benchmark thread only, plain calls:

```
runAndReport
  └─ run(...) / runDataPipeline(...)
       setup (project load, chunk build, engine compile)      <- not counted
       captured = makeCapturedData(chunk)                       <- built once, before the loop
       allocStart = sampleAllocations()                         <- mi_heap_stats_get(default heap)
       loop: reader.processData(captured); drain; [drain rings]
             every 16 ms: pumpStart = sample; spinEventLoop(); allocSkipped += sample - pumpStart
       allocEnd = sampleAllocations()
       result.allocations = allocEnd - allocStart - allocSkipped
       result.allocationsPerFrame = parsed > 0 ? allocations / parsed : -1
```

`sampleAllocations()` returns `malloc_normal_count.total + malloc_huge_count.total` from a
`mi_stats_t` filled by `mi_heap_stats_get(mi_heap_main(), &s)`. In mimalloc 3.4.5 (which has no
`mi_heap_get_default`; heaps are arena-level and threads attach a `theap` to them) the read
merges the calling thread's `theap` counters into the main heap's aggregate and returns that
aggregate. Another thread's counters land there only when it exits or reads statistics itself,
so a window is the benchmark thread's alone as long as no thread ends inside it. The Native
rows are the first two parse runs, before any Lua/JS engine, watchdog thread or exporter worker
exists, which is what makes their gate exact (implementation note, 2026-09-11: this replaced
the plan's original per-thread claim; the spec constraint was amended to match). Without `SS_ALLOC_STATS` (or without mimalloc), `sampleAllocations()` is not
compiled and every `Result` carries `-1`.

The dashboard drain inside `run` (`drainDashboardRings`) runs on the benchmark thread and *is*
counted for the dashboard rows; those rows are never alloc-gated, and the count is informative
there (it is the dashboard ingest's own allocation figure).

**Width.** `channels` replaces the two `constexpr int kChannels = 8` locals in `run` and
`runDataPipeline` and the one in `measureNativeStages`, whose `std::array<QByteArrayView, 64>`
becomes `kMaxSpanFields`-sized. `buildProjectJson` and `buildChunk` already take `channels`.
The dashboard project (`buildDashboardProjectJson`) stays fixed at 13 datasets: those rows
measure widget ingest, not width (spec R5 amended to say so). Above `kMaxSpanFields` (128)
`trySpanLane` returns `-1` and Native parses through the list lane; the report prints
`native lane: span` or `native lane: list (> 128 fields)` so a wide run is never mistaken for a
span-lane number.

**Gating.** `printReport` receives `channels`. `allPassed` becomes:
`throughputOk && allocGateOk`, where `throughputOk` is today's conjunction when
`channels == kDefaultChannels` and `true` otherwise (rows still print PASS/FAIL against their
target for reference, and a new `hotpath: width 256, throughput tiers not gated` line says
why), and `allocGateOk` is `true` when Native rows read `-1`, else
`native.allocationsPerFrame == 0 && nativeMix.allocationsPerFrame == 0`. Summary keys:
`HOTPATH_CHANNELS=%d`, `HOTPATH_ALLOC_GATE=pass|fail|n/a`, and one
`HOTPATH_<TAG>_ALLOC_PER_FRAME=%.4f` per named row (`-1` prints as `n/a`).

**CI placement.** The PGO-GENERATE configure gets `-DSS_ALLOC_STATS=ON`; the USE configure does
not, so the shipped binary is byte-for-byte what it is today apart from debug info. The training
run already executes every row with `--min-fps 1`; with the gate in `allPassed` its exit code
now carries the allocation verdict. Linux/macOS training steps run under `set -e` semantics
(bash step, nonzero exit fails); the Windows step only enforces a timeout today, so it gains the
same `Select-String HOTPATH_PASS=1` check the Windows throughput gate uses. Each training step
writes `--benchmark-output pgo-train.txt`, uploaded with the existing benchmark-report artifact.

**Symbols.** Compile: `-gline-tables-only` keeps function names, line tables and inlined-frame
records (what a profiler and a symbolicated crash stack need) at a fraction of full `-g` DWARF,
and never changes codegen. Link/post-build, inside the existing `BUILD_COMMERCIAL AND
PRODUCTION_OPTIMIZATION` block: macOS `dsymutil <exe> -o <exe>.dSYM` then the existing
`strip -x`; Linux `objcopy --only-keep-debug <exe> <exe>.debug`, `objcopy
--add-gnu-debuglink=<exe>.debug <exe>`, then the existing `--strip-unneeded`; Windows removes
`/DEBUG:NONE` so the clang-cl branch's `/Z7` + `/DEBUG` PDB survives (cpack does not install it,
per the comment already in `Optimization.cmake`). CI uploads `symbols-Linux-x64`,
`symbols-Linux-arm64`, `symbols-macOS-arm64`, `symbols-macOS-x86_64`, `symbols-Windows`
right after the build step of each job, before packaging. Non-commercial (GPL) builds are not
stripped today and stay that way; they simply gain line tables.

## Hotpath & threading impact

- **Touches the hotpath?** No. `FrameReader`, `FrameBuilder`, `BlockStager`, `Dashboard` are
  unchanged. The harness changes its own input pattern in one way: the captured chunk is built
  once (a mutable `shared_ptr<CapturedData>`) and re-fed every iteration with its `timestamp`
  restamped from the steady clock immediately before each `processData`, instead of a fresh
  chunk per iteration. `FrameReader::appendChunk` copies the bytes into its ring and the stamp
  into its `PendingChunk`, and drops its reference once the chunk's bytes are consumed, so the
  reused object is equivalent to today's fresh one: same bytes, a fresh per-chunk stamp, so the
  source clock keeps advancing and neither the block stager's per-sample offsets nor the
  dashboard floor's plot clock see a rewind (review finding, 2026-09-11). `--benchmark-hotpath`
  numbers are expected to move by at most the one avoided allocation per 1000 frames.
- **New cross-thread signal/slot?** No.
- **New input to a cached hotpath flag?** No.
- **Timestamp ownership** — unchanged; the harness stamps its synthetic chunk at construction
  as today.
- **`MI_STAT=2` cost**: only in the `SS_ALLOC_STATS` build, which is the PGO training binary.
  mimalloc is excluded from PGO (included before `Optimization.cmake`), so its profile is not
  affected; the app's profile sees the same control flow with slower mallocs, which does not
  change which branches are hot.

## Data model & persistence

None. No `Keys::`, no project JSON, no schema.

## API / SDK surface

None. `--benchmark-channels` is a CLI option, not an API verb; `Command-Line-Interface.md`
documents it.

## QML / UI

None. The in-app benchmark dialog keeps calling `run`/`runDataPipeline` with the default width
and never sees the allocation column (its `Result` copies carry `-1` unless the developer built
with `SS_ALLOC_STATS`, in which case the dialog could show it later; out of scope).

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Where the allocation counter comes from | (a) mimalloc per-thread heap stats in a stats-enabled build; (b) `MI_STAT=1` always on, gate on bytes in the final run; (c) platform malloc interposition around the loop | **(a)**. Exact count, per thread by construction, zero cost in the shipped allocator. (b) taxes every malloc in the product for a CI number and breaks the spec's "must not perturb" constraint. (c) reaches Qt's mallocs only on macOS. |
| Which CI build carries the gate | (a) the PGO-GENERATE training build; (b) a sixth "stats" build per job; (c) the sanitizer job | **(a)**. It already runs the full benchmark on every platform; allocation count is codegen-independent so gating there is exact; no added build time. (b) doubles build cost. (c) has mimalloc disabled. |
| Count vs bytes | `MI_STAT=2` count vs `MI_STAT=1` bytes | **Count**. The spec asks for allocations per frame, and the stats build is not shipped so the per-bin overhead of level 2 is free. |
| Gate at non-default width | alloc gate on / off when `--benchmark-channels` is set | **On**. Allocation-free is width-independent; only throughput tiers are calibrated at 8. |
| Width above the 128-field span cap | (a) clamp the knob at 128; (b) allow and label the lane | **(b)**. The spec names 256 and 635; the list-lane cost at those widths is exactly the unknown the knob exists to measure. The report labels the lane so nobody reads a list-lane number as a span-lane regression. |
| Debug info level | `-g` full vs `-gline-tables-only` / `-g1` | **Line tables**. Enough for profilers and symbolicated crashes, small enough not to strain the LTO link on the arm64 runners; full `-g` can be a later switch if variable inspection is ever needed. |
| Symbol upload timing | after build vs after packaging | **After build**, before cpack/macdeployqt touch the bundle, so the artifact matches the binary CI benchmarks. |
| The harness's own per-chunk allocation | keep and subtract vs remove | **Remove** (build the captured chunk once). Subtracting a known count is fragile; removing it makes "zero" mean zero. |

## Risks & mitigations

- **mimalloc stats field names.** `malloc_normal_count` is confirmed in `mimalloc-stats.h` of
  the fetched 3.4.5; the huge-object counter's exact name is verified at implement time. Both
  are behind `#if SS_ALLOC_STATS && SS_MIMALLOC_ACTIVE`, so a mismatch is a compile error in
  the stats build only.
- **Training run now fails CI on allocation.** Intended (spec R2, AC2), but it fires before the
  shipped binary exists. Ordering: land 0084 with the gate, expect the Linux/macOS/Windows
  training steps to go red on the current tree, land 0085 immediately after. If the two cannot
  land back to back, ship 0084 with `HOTPATH_ALLOC_GATE` reported but not folded into
  `allPassed`, and flip it in 0085's first task. Decision recorded here; default is the gate on.
- **Stats build ≠ shipped build allocation behavior.** The one known divergence is compiler
  elision of `new`/`delete` pairs, which the stats build (same `-O3`, no PGO) performs the same
  way. Accepted.
- **`-gline-tables-only` with `-flto=auto` on AppleClang.** Debug info through LTO is
  supported; the risk is link time and memory on the macOS arm64 runner. Mitigation: line
  tables only; measure the link-step time delta in the first CI run and record it in tasks.
- **Windows PDB now kept.** `/DEBUG` already runs in the clang-cl branch; removing
  `/DEBUG:NONE` only stops discarding the file. Verify cpack/MSIX packaging does not pick up
  the `.pdb` (the existing comment says it does not; the tasks phase greps the WiX/MSIX lists).
- **Span cap surprise.** A first `--benchmark-channels 256` run will show Native far slower
  than 8 channels because it leaves the span lane. The lane label in the report is the
  mitigation; the finding itself is for spec 0085's successor.
- **Silent-breakage classes exposed** (from `common-mistakes.md`): the Windows `-platform`
  injection and the GUI-subsystem console detach both apply to the training step; the plan
  changes neither (same flags as today plus `--benchmark-output`). MMCSS registration is
  unchanged.

## Test & verification plan

- **Unit (you can run):** none of `tests/scripts/` covers the benchmark; `python
  scripts/code-verify.py --check` on every touched C++/CMake file.
- **Maintainer runs (needs a build):**
  - AC1, AC3: build with `-DSS_ALLOC_STATS=ON`, run `--benchmark-hotpath --min-fps 1
    --benchmark-seconds 2 --benchmark-frames 100000`; the table has an `Alloc/frame` column,
    every named row has a `HOTPATH_<TAG>_ALLOC_PER_FRAME=` value, Lua rows read nonzero and
    `HOTPATH_PASS` reflects only the Native alloc gate and throughput.
  - AC2: same run on the current tree shows `native(numeric)` nonzero (the map churn,
    expected ≈ 0.03 per frame: two allocations per 64-sample block) and exit code 1; re-run
    after 0085 shows 0 and exit code 0.
  - AC4: `--benchmark-channels 256`: header says `channels: 256`, summary has
    `HOTPATH_CHANNELS=256`, Native rows print `native lane: list (> 128 fields)`, exit code
    ignores throughput targets. `--benchmark-channels 64` prints `native lane: span`.
  - AC5: `--min-fps 1` with no width option: report identical in shape to today plus the new
    column and lines; `HOTPATH_CHANNELS=8`.
  - AC6: after a CI run, download `symbols-macOS-arm64`; `xctrace record --template 'Time
    Profiler'` over the shipped binary, `xctrace symbolicate --dsym`; frames resolve to
    `DataModel::BlockStager::stage` and friends.
  - AC7: `ls -la` of the packaged binary before/after and `nm | wc -l` unchanged on macOS
    (`strip -x` still runs last); Linux `readelf -S` shows no `.debug_*` sections and a
    `.gnu_debuglink`.
- **Hotpath:** `--benchmark-hotpath` on the PGO-use build: all nine tiers within run-to-run
  noise of the pre-change numbers (the only harness change is the reused chunk).
- **Static:** `python scripts/code-verify.py --check <files>`; `qt-cpp-review` on the two
  benchmark TUs before handoff; `python scripts/sanitize-commit.py` before commit;
  `python scripts/claim-verify.py` after the doc edits (the `ss-hotpath` skill and
  `dataflow.md` pin benchmark constants).
