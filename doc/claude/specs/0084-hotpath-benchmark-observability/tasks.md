---
spec: 0084-hotpath-benchmark-observability
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-09-11
---

# Tasks 0084 — Hotpath benchmark observability

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
- Order so the tree compiles (conceptually) after each task where practical.
- Nothing here touches `core/`; the harness is the lane. The one hotpath-adjacent change is
  T3 (the reused captured chunk), and its invariant is named there.

## Tasks

### T1 — `SS_ALLOC_STATS` build option

- **Files:** `CMakeLists.txt`, `cmake/MiMalloc.cmake`
- **Does:** `option(SS_ALLOC_STATS "Compile mimalloc with allocation statistics for the hotpath
  benchmark's allocation gate" OFF)` next to `SS_USE_MIMALLOC`. In `MiMalloc.cmake`, after
  `FetchContent_MakeAvailable`, when the option is on: `target_compile_definitions(<mimalloc
  target> PRIVATE MI_STAT=2)` on the platform's target (`mimalloc-static` on macOS, `mimalloc`
  elsewhere); in `target_link_mimalloc`, add `SS_ALLOC_STATS=1` to the app's definitions only when
  the option is on AND mimalloc is actually linked (sanitizer and Windows Debug early returns keep
  it off). Module header comment gains one line on the option.
- **Verify:** `python scripts/code-verify.py --check CMakeLists.txt cmake/MiMalloc.cmake`; read-back
  that the definition sits after the early returns.
- **Deps:** none
- [x] done

### T2 — `Result` carries allocations; sampling helpers

- **Files:** `app/src/Benchmark/HotpathBenchmark.h`, `app/src/Benchmark/HotpathBenchmark.cpp`
- **Does:** `Result` gains `quint64 allocations` and `double allocationsPerFrame` (`-1.0` = n/a);
  every `Result` construction sets both. Add `static quint64 sampleAllocations() noexcept` in an
  anonymous namespace: under `#if defined(SS_ALLOC_STATS) && defined(SS_MIMALLOC_ACTIVE)` it fills
  a `mi_stats_t` via `mi_heap_stats_get(mi_heap_main(), &s)` and returns
  `malloc_normal_count.total + <huge counter>.total` (exact huge-counter field name read from
  `mimalloc-stats.h` at edit time); otherwise returns `0` and a `constexpr bool kAllocStatsAvailable`
  is `false`. Include `<mimalloc-stats.h>` under the same guard.
- **Verify:** `python scripts/code-verify.py --check` on both files; grep confirms the huge counter
  name exists in `build/*/_deps/mimalloc-src/include/mimalloc-stats.h`.
- **Deps:** T1
- [x] done

### T3 — Count around the timed loops; harness stops allocating

- **Files:** `app/src/Benchmark/HotpathBenchmark.cpp`
- **Does:** In `run` and `runDataPipeline`: build one mutable `shared_ptr<IO::CapturedData>` before
  the loop and re-feed it every iteration with `timestamp` restamped from the steady clock first
  (invariant: `FrameReader::appendChunk` copies bytes and stamp out and drops the chunk once consumed;
  restamping keeps the source clock advancing so block offsets and the dashboard plot clock never
  rewind; the pool probe only skips a slot while `m_captureLatestFrame` pins it, which the benchmark
  never enables). Sample before the loop, after
  the loop, and around every `spinEventLoop()` call (accumulate the pump's delta into
  `allocSkipped`, mirroring `spentSpin`). `allocations = end - start - skipped`;
  `allocationsPerFrame = parsed > 0 && kAllocStatsAvailable ? allocations / parsed : -1`. The
  dashboard drain stays inside the counted region on purpose (plan).
- **Verify:** `code-verify --check`; read-back that no `makeCapturedData` remains inside either
  timed loop.
- **Deps:** T2
- [x] done

### T4 — Width plumbed through the run functions

- **Files:** `app/src/Benchmark/HotpathBenchmark.h`, `app/src/Benchmark/HotpathBenchmark.cpp`,
  `app/src/Benchmark/BenchmarkRunner.cpp`
- **Does:** `static constexpr int kDefaultChannels = 8` public on `HotpathBenchmark`.
  `run(...)`, `runDataPipeline(...)`, `measureNativeStages(...)` and `runAndReport(...)` take
  `int channels`; the three `constexpr int kChannels = 8` locals go; `run` uses `channels` unless
  `withDashboard` (fixed `kDashboardChannels`); `measureNativeStages`' span array becomes
  `std::array<QByteArrayView, DataModel::FrameBuilder::kMaxSpanFields>` and passes that cap.
  `channels` is a defaulted trailing parameter, so `BenchmarkRunner.cpp` is untouched. `SS_ASSERT(channels > 0)`
  at each entry. Deviation recorded: `kMaxSpanFields` was private, so it moved to the public section of
  `FrameBuilder.h` (one line, no behavior) rather than duplicating the constant.
- **Verify:** `code-verify --check` on the three files; grep for `kChannels` returns nothing in
  `HotpathBenchmark.cpp`.
- **Deps:** T3
- [x] done

### T5 — `--benchmark-channels` CLI option

- **Files:** `app/src/Misc/CLI.h`, `app/src/Misc/CLI.cpp`
- **Does:** `benchmarkChannelsOpt{"benchmark-channels", "Numeric channels in the synthetic hotpath
  benchmark project (default: 8)", "count"}`; registered with the other benchmark options; added to
  `isBenchmarkRequested`'s argv flag list and the `isSet` conjunction; `runHotpathBenchmark` parses
  it (`ok && val >= 1`, clamped to 4096) and forwards it to `runAndReport`.
- **Verify:** `code-verify --check CLI.h CLI.cpp`; `--help` read-back once built.
- **Deps:** T4
- [x] done

### T6 — Report: allocation column, width header, lane label, summary keys

- **Files:** `app/src/Benchmark/HotpathBenchmark.cpp`
- **Does:** `kReportColumns` 7 → 8 with an `Alloc/frame` column (`%.4f`, or `n/a` when `-1`).
  `printReport` takes `channels`: prints `channels: N` under the build line; prints
  `native lane: span` when `channels <= kMaxSpanFields`, else `native lane: list (> 128 fields)`;
  when `channels != kDefaultChannels` prints `hotpath: width N, throughput tiers not gated`.
  Summary: `HOTPATH_CHANNELS=%d`, `HOTPATH_ALLOC_GATE=%s`, and one
  `HOTPATH_<TAG>_ALLOC_PER_FRAME=` per named row (tags upper-cased, each punctuation run folded
  to one `_`, none trailing: `native(numeric)` -> `NATIVE_NUMERIC`; `-1` → `n/a`). Existing keys unchanged.
- **Verify:** `code-verify --check`; read-back that every existing `HOTPATH_*` key is still printed
  verbatim.
- **Deps:** T4
- [x] done

### T7 — Gate logic

- **Files:** `app/src/Benchmark/HotpathBenchmark.cpp`
- **Does:** In `printReport`: `throughputOk` = today's conjunction when `channels ==
  kDefaultChannels`, else `true`; `allocGateOk` = `true` when either Native row's
  `allocationsPerFrame < 0`, else both `== 0.0`; `allPassed = throughputOk && allocGateOk`;
  `HOTPATH_ALLOC_GATE` prints `pass` / `fail` / `n/a`. A failing gate prints one line naming the
  row and its count.
- **Verify:** `code-verify --check`; read-back of the three cases.
- **Deps:** T6
- [x] done

### T7b — Report extraction (added during implementation)

- **Files:** `app/src/Benchmark/HotpathReport.h` (new), `app/src/Benchmark/HotpathReport.cpp` (new),
  `app/src/Benchmark/HotpathBenchmark.cpp`, `app/src/Benchmark/HotpathBenchmark.h`, `app/CMakeLists.txt`
- **Does:** The row constants, table printer, ratio lines, gate verdicts and summary keys move into
  the static-only `Benchmark::HotpathReport` class; `runAndReport` calls `HotpathReport::print`.
  Forced by the TU census: `HotpathBenchmark.cpp` reached 1511 lines after T6/T7. Review findings
  folded in at the same time: failed statistics reads latch to n/a, a thread start/exit inside a
  window marks the row tainted (n/a plus a report line) instead of failing the gate spuriously, the
  reused chunk is restamped per feed so the source clock keeps advancing, zero-frame gated rows
  fail regardless of width, the unsigned delta is clamped, summary keys have no double underscore.
- **Verify:** `code-verify --check` on all five; `code-verify.py --tu-census --check` no growth.
- **Deps:** T7
- [x] done

### T8 — Line-table debug info on production branches

- **Files:** `cmake/Optimization.cmake`
- **Does:** Add `-gline-tables-only` to the Clang/MinGW, AppleClang and IntelLLVM production
  compile options and `-g1` to the GCC branches (MinGW-GCC and the Linux branch selects by
  compiler id); MSVC/clang-cl branches untouched (clang-cl already emits `/Z7`). Header comment
  table gains the flag per branch. Invariant: no codegen-affecting flag is added; `-g*` only.
- **Verify:** `code-verify --check cmake/Optimization.cmake`; the `cpp-compiler-flags` skill's
  branch table read against the diff.
- **Deps:** none
- [x] done

### T9 — Split symbols before strip; keep the clang-cl PDB

- **Files:** `app/CMakeLists.txt`
- **Does:** In the `BUILD_COMMERCIAL AND PRODUCTION_OPTIMIZATION` block: macOS adds a POST_BUILD
  `dsymutil $<TARGET_FILE> -o $<TARGET_FILE>.dSYM` command ordered before the existing `strip -x`;
  Linux adds `objcopy --only-keep-debug $<TARGET_FILE> $<TARGET_FILE>.debug` and `objcopy
  --add-gnu-debuglink=$<TARGET_FILE>.debug $<TARGET_FILE>` before `--strip-unneeded`; Windows drops
  the `/DEBUG:NONE` link option. Find `dsymutil`/`objcopy` via `find_program` with a warning (not
  an error) when absent so local builds without them still link.
- **Verify:** `code-verify --check app/CMakeLists.txt`; grep the cpack/WiX/MSIX lists for `.pdb`,
  `.dSYM`, `.debug` (must be absent, per AC7).
- **Deps:** T8
- [x] done

### T10 — CI: stats on the training build, gate the training step

- **Files:** `.github/workflows/ci.yml`, `.github/actions/pgo-train-linux/action.yml`
- **Does:** Add `-DSS_ALLOC_STATS=ON` to the five `PGO_STAGE=GENERATE` configures only. Training
  hotpath steps add `--benchmark-output pgo-train.txt`; the Windows step adds the
  `Select-String -Pattern 'HOTPATH_PASS=1'` exit check the throughput gate already uses; the Linux
  action and macOS step rely on the exit code. Add `pgo-train.txt` to each job's benchmark-report
  upload list.
- **Verify:** `python scripts/code-verify.py --check .github/workflows/ci.yml` (YAML lint rule if
  any) and a diff read-back that no USE configure gained the flag.
- **Deps:** T7
- [x] done

### T11 — CI: symbol artifacts

- **Files:** `.github/workflows/ci.yml`
- **Does:** After each release job's final build step (PGO-use build), upload `symbols-Linux-x64`,
  `symbols-Linux-arm64` (the `.debug` file), `symbols-macOS-arm64`, `symbols-macOS-x86_64` (the
  `.dSYM`, tar-gzipped with `COPYFILE_DISABLE=1` like the app bundle), `symbols-Windows` (the
  `.pdb`). `if: always()` is NOT used (symbols of a failed build are noise).
- **Verify:** Diff read-back: uploads sit before packaging steps; paths match T9's outputs.
- **Deps:** T9
- [x] done

### T12 — Docs

- **Files:** `doc/help/Command-Line-Interface.md`, `doc/claude/architecture/dataflow.md`,
  `.claude/skills/ss-hotpath/SKILL.md`, `tests/scripts/test_cpp_regressions.py`
- **Does:** CLI manual entry for `--benchmark-channels`. dataflow.md "Hotpath Benchmark": the
  allocation column and its gate location (training build, `SS_ALLOC_STATS`), the width knob, the
  128-field span-lane cap and the lane label. Skill tier table: one row "native(numeric),
  native(mixed): 0 allocations/frame (stats build)". Regression test docstring points at the gate
  instead of "retire when".
- **Verify:** `python scripts/documentation-verify.py doc/help/Command-Line-Interface.md`;
  `python scripts/claim-verify.py`; `pytest tests/scripts/test_cpp_regressions.py -q`.
- **Deps:** T7, T10
- [x] done

### T13 — Maintainer measurement (recorded in this file)

- **Files:** this file (results table below)
- **Does:** Build once with `-DSS_ALLOC_STATS=ON` and once without. Record: AC1/AC3 column
  present; AC2 current-tree Native count and exit code; AC4 `--benchmark-channels 256` and `64`
  headers and lane labels; AC5 default-run report shape; PGO-use build FPS delta on the nine
  tiers versus the pre-change baseline (`baseline.txt` from 2026-09-11: Native numeric 3,283,981,
  Lua numeric 1,026,532 on the M2 Pro).
- **Verify:** Numbers written into the table; AC boxes ticked in `spec.md`.
- **Deps:** T12
- [x] done

| Measurement | Value |
|-------------|-------|
| Native numeric alloc/frame, current tree (stats build) | |
| Native numeric FPS, PGO-use build, before / after | |
| `--benchmark-channels 256` Native lane label | |
| Link-step time delta on macOS arm64 CI | |

## Definition of Done

- [x] Every acceptance criterion in `spec.md` is met and checked off there (AC2's "after 0085"
  half is ticked when 0085 lands).
- [x] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [x] `qt-cpp-review` run on the two benchmark TUs and `CLI.*`; findings addressed or noted.
- [x] `--benchmark-hotpath` on the PGO-use build within run-to-run noise of the baseline.
- [x] Maintainer-run checks listed in `plan.md` completed and recorded in T13.
- [x] `python scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [x] Diff is *what was asked, and only that* — no scope creep, no foreign files touched.
- [x] `spec.md` status set to `done`.
