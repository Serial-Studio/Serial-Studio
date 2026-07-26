---
spec: 0032-cpp-unit-tests
phase: plan
status: draft        # draft -> approved (gate before /ss-tasks)
updated: 2026-07-25
---

# Plan 0032 — C++ unit-test tier plus build presets

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Gate: do not start `/ss-tasks` until a human marks this
> `approved`.

## Approach (one paragraph)

Add a Qt Test based unit tier that lives in `app/tests/`, compiles only when
`SS_BUILD_TESTS=ON`, and links the two or three production translation units each suite
actually needs rather than the whole application — so the `qt_add_executable` call, the
QML module, and the two-stage PGO/LTO flow are untouched. Five suites ship in v1, ordered
by link cost: DSP kernels and circular buffer (header-only, zero app TUs), checksums and
frame serialization (one TU each), and the frame reader (a bounded carve-out). A root
`CMakePresets.json` names the configurations these are built under — `dev`, `asan`,
`tsan`, `analysis`, plus a lean CI preset — without any CMake code reading presets, so
default build behavior is unchanged. CI grows one independent job, in the same fast tier
as `lint`, on both an x86-64 and an aarch64 runner because the two SIMD lanes can only be
checked on their own architecture. A build-flag-gated in-app test mode, modeled on
`--benchmark-hotpath`, is specified but deferred to phase 2 with a deliberately trivial
v1.

## Affected subsystems & files

| File | Change |
|------|--------|
| `CMakePresets.json` | **New.** Root presets: hidden `base`, `dev`, `asan`, `tsan`, `analysis`, `unit-ci`, plus matching build and test presets. |
| `CMakeLists.txt` (root) | `option(SS_BUILD_TESTS ... OFF)` next to the existing options; `enable_testing()` guarded by it, after `add_subdirectory(app)`. |
| `app/CMakeLists.txt` | Two lines at the end: `if(SS_BUILD_TESTS) add_subdirectory(tests) endif()`. Nothing else in this file changes. |
| `app/tests/CMakeLists.txt` | **New.** `find_package(Qt6 COMPONENTS Test)`, an `ss_add_unit_test()` helper, the five suite targets, the `ss_unit_tests` aggregate target, `add_test()` registrations. |
| `app/tests/tst_dsp_kernels.cpp` | **New.** SIMD-vs-scalar bit-exactness for every `DSP::` kernel. |
| `app/tests/dsp_scalar_ref.h` | **New.** Declarations of the scalar-lane wrappers (`DspRef::`). |
| `app/tests/dsp_scalar_ref.cpp` | **New.** Compiled with `SS_SIMD_DISABLE` + a distinct namespace macro; forwards to the scalar build of the kernels. |
| `app/tests/tst_circular_buffer.cpp` | **New.** Ring semantics + pattern scans across the wrap boundary. |
| `app/tests/tst_checksums.cpp` | **New.** All ten registered algorithms, known-answer vectors, output byte order. |
| `app/tests/tst_frame_serialization.cpp` | **New.** `serialize`/`read` round-trips over the entity structs via `Keys::`. |
| `app/tests/tst_frame_delimiters.cpp` | **New.** `IO::FrameReader` driven through `processData()`; frames read off `queue()`. |
| `app/src/DSPSimd.h` | **Edit (hotpath header, compile-time only).** Make the SIMD selection opt-out-able and the namespace name overridable, so a second TU can hold a scalar build without an ODR collision. Default build must preprocess identically. |
| `.github/workflows/ci.yml` | **Edit.** New `unit` job, matrix `{ubuntu-24.04, ubuntu-24.04-arm}`, no `needs:`. |
| `tests/README.md` | **Edit.** Document the new tier alongside the existing pytest tiers and `tests/scripts/`. |
| `CLAUDE.md` | **Edit.** The Tests section currently says the C++ hotpath has no test path other than the benchmark; that stops being true. |

Phase 2 only (not in v1 scope, listed so the seam is visible):
`app/src/Misc/CLI.h`, `app/src/Misc/CLI.cpp`, `app/src/SelfTest/SelfTest.{h,cpp}`,
`app/CMakeLists.txt` (source list + `SS_INAPP_TESTS` definition).

## Architecture & data flow

Nothing in the running application changes. The structure is a build-graph change:

```
CMakePresets.json  (client-side only; no CMake code reads it)
        │
        ▼
cmake --preset dev  ->  -DSS_BUILD_TESTS=ON
        │
root CMakeLists  ── option(SS_BUILD_TESTS) ── enable_testing()
        │
app/CMakeLists ── (unchanged app target) ── add_subdirectory(tests)
                                                    │
                                    app/tests/CMakeLists.txt
                                    │
        ┌───────────────┬───────────┼──────────────┬────────────────┐
        ▼               ▼           ▼              ▼                ▼
 tst_dsp_kernels  tst_circular_  tst_checksums  tst_frame_    tst_frame_
 (+dsp_scalar_ref) buffer        (+Checksum.cpp) serialization  delimiters
        │               │           │           (+Frame.cpp)   (+FrameReader.cpp,
        │               │           │                            Checksum.cpp,
        │               │           │                            NotificationCenter.cpp,
        │               │           │                            AppPlatform.cpp)
        └───────────────┴───────────┴──────────────┴────────────────┘
                                    │
                        add_test() -> ctest -> exit code
```

`app/tests` is a subdirectory of `app`, so it inherits that directory's `AUTOMOC ON`,
`include_directories(src)`, and the directory-level compile/link options the sanitizer and
optimization modules install. That inheritance is the reason the tests live under `app/`
rather than under the repo-root `tests/` tree: an `asan` preset instruments the test
binaries for free, and `#include "IO/Checksum.h"` resolves the same way it does in
production code.

Each suite is one `QObject` in one `.cpp` with `private slots:` test functions, ending in
`#include "tst_<name>.moc"` — the standard single-file Qt Test idiom, required because
AUTOMOC generates the meta-object into a `.moc` the TU must include itself. Entry point is
`QTEST_APPLESS_MAIN` for the four pure-logic suites (no `QCoreApplication`, so no platform
plugin and no `QT_QPA_PLATFORM` needed in CI) and `QTEST_GUILESS_MAIN` for the frame-reader
suite, which emits signals and whose overflow path posts a queued invocation.

**Test-suite inventory and what each one buys:**

| Suite | Links | Covers | A failure means |
|-------|-------|--------|-----------------|
| `tst_dsp_kernels` | `dsp_scalar_ref.cpp` only | Every kernel in `DSP::` — the byte scans (`simdForEachByteMatch`, `simdFindAnyByte`), the f64 reductions (`simdMinF64`, `simdMaxF64`, `simdMinMaxF64`), `simdFiniteMinMaxPointF<0/1>`, `simdWindowedComplexFill`, `simdRingsToPoints`, `simdAsciiDots16`. Data-driven over lengths that straddle every vector-block boundary (0,1,3,4,7,8,15,16,17,31,63,64,255,1024), over source pointer offsets 0..15, and over payloads carrying NaN, ±0.0, ±inf, and denormals. Vector result compared to scalar result by bit pattern (`std::bit_cast<quint64>`), not by `==` or `qFuzzyCompare`. | The bit-exactness contract in CLAUDE.md and spec 0021 is broken; plots, FFTs, and delimiter scans differ between architectures or between build configurations. |
| `tst_circular_buffer` | nothing (header-only) | `roundUpToPowerOfTwo`; append/read/peek/discard accounting; `freeSpace`/`size`/`capacity` invariants; `setCapacity` reconfigure; overflow counter; `peekRangeInto` writing in place; `buildKMPTable`; `findPatternKMP` and `findFirstOfPatterns` with the pattern (a) entirely in the linear region, (b) straddling the wrap, (c) at the last byte, (d) absent, (e) longer than the content, (f) at the 8-byte short-pattern boundary that selects the memcmp lane vs the KMP lane, (g) 8 simultaneous patterns (the documented maximum). | Frames are silently mis-split at ring wrap — the single hardest bug class in the reader to reproduce live. |
| `tst_checksums` | `Checksum.cpp` | All ten registry entries (`XOR-8`, `MOD-256`, `CRC-8`, `CRC-16`, `CRC-16-MODBUS`, `CRC-16-CCITT`, `Fletcher-16`, `CRC-32`, `Adler-32`, and the empty name) against published check vectors for `"123456789"`; zero-length input; single-byte input; the **output byte order** of each (`CRC-16-MODBUS` packs little-endian, the rest of the 16-bit family big-endian) since that is what actually breaks user projects; `availableChecksums()` and `checksumFunctionMap()` agreeing key-for-key; and the unknown-name case, which today returns an empty result indistinguishable from the empty algorithm. | A user's device-side checksum stops matching, and every frame is rejected with no diagnostic. |
| `tst_frame_serialization` | `Frame.cpp` | `toJson`/`fromJson` round-trips for `Dataset`, `Group`, `Action`, `Source`, `Frame`, `AlarmBand`, `FrequencyMarker`, `OutputWidget`, `RegisterDef`, `TableDef`, and the workspace/folder structs; defaults survive a round-trip; unknown keys are tolerated; `read()` returns false on malformed input rather than half-populating; `Keys::` constants are used for every field the test builds by hand, so a renamed key fails the test instead of silently changing the on-disk format. Fixtures are inline raw string literals — no data files, so ctest has no working-directory dependence. | Saved projects change shape or lose fields between versions. |
| `tst_frame_delimiters` | `FrameReader.cpp`, `Checksum.cpp`, `NotificationCenter.cpp`, `Platform/AppPlatform.cpp` | End-delimited, start-delimited, and start+end-delimited extraction; a delimiter split across two `processData` chunks; empty frames between back-to-back delimiters; a delimiter as the final byte; multi-byte delimiters at the 8-byte lane boundary; multiple start sequences; checksum validation returning `FrameOk` / `ChecksumError` / `ChecksumIncomplete`; `droppedFrameCount`/`overflowCount` accounting. Frames are read off `queue()` after `readyRead()`. | Frames stop being found on a device that worked yesterday — the top support symptom. |

## Hotpath & threading impact

- **Touches the hotpath?** One file, compile-time only: `app/src/DSPSimd.h`. No behavioral
  change. The two edits are (1) wrapping the `SS_SIMD_X86` / `SS_SIMD_NEON` defines so a
  translation unit can opt out with `-DSS_SIMD_DISABLE`, and (2) making the namespace name
  a macro that defaults to `DSP`. Both are inert unless the macro is defined, which only
  `app/tests/dsp_scalar_ref.cpp` does. Everything else — `FrameReader`, `CircularBuffer`,
  `FrameBuilder`, `Dashboard`, the span fast lane — is **read and exercised, never
  modified**. The SPSC/no-mutex, `Qt::DirectConnection`, no-alloc, slot-pool, and
  cached-flag rules are preserved trivially because no production control flow changes.
  Verification: `--benchmark-hotpath --min-fps 256000` before and after the `DSPSimd.h`
  edit, plus a preprocessed-output diff (`-E`) of one consumer TU (`app/src/DSP.h`'s user,
  e.g. `UI/Widgets/Plot.cpp`) proving the default build sees identical tokens.
- **New cross-thread signal/slot?** None. `tst_frame_delimiters` drives `FrameReader`
  entirely on the test's own thread, matching the production main-thread contract; it must
  not spawn a producer thread in v1 (a threaded SPSC stress test belongs behind the `tsan`
  preset later, and would be the first thing to write once presets exist).
- **New input to a cached hotpath flag?** None. No test constructs `FrameBuilder`,
  `Dashboard`, or anything that reads `m_operationMode` / `m_anyAsyncSink` /
  `m_captureLatestFrame` / `m_streamAvailable`.
- **Timestamp ownership** — unchanged. `tst_frame_delimiters` supplies its own
  `CapturedData` timestamps at the point it would be the driver, which is exactly the
  production contract, and asserts that `FrameReader` advances per-frame timestamps by
  `frameStep` rather than re-stamping.

## Data model & persistence

No schema change, no `Frame.h` `Keys::` additions, no writer-version bump, no
`widgetSettings` or Sessions-DB change. `tst_frame_serialization` **consumes** `Keys::` as
the single source of truth and constructs its fixtures from those constants; a key rename
that is not mirrored in the fixture makes the test fail to compile, which is the desired
coupling. No test writes to disk, no test reads a fixture file, and no test touches
`QSettings` — so a test run cannot corrupt the maintainer's application state.

## API / SDK surface

None in v1. No new API handler, no `EnumLabels` entry, no SDK regeneration, no `apiCall`
reach.

Phase 2 adds a **CLI flag, not an API command**: `--selftest` (optionally
`--selftest=<suite>`), registered in `Misc::CliOptions`, handled in `CLI::process()` before
the GUI comes up, and added to `CLI::isCliEarlyExit()` so the application does not build a
window. This is the exact shape of the four flags already there —
`--benchmark-hotpath`, `--dump-api-schema`, `--selftest-offline-license`,
`--validate-guards`. The name `--selftest` is chosen to sit beside the existing Pro-only
`--selftest-offline-license` without colliding, and must work in a GPL build. Gated at
compile time by `SS_INAPP_TESTS`; absent from the parser when the option is off.

## QML / UI

None. No QML file, no new component, no theme surface.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Test framework | **Qt Test**, doctest, Catch2, GoogleTest | **Qt Test.** It is already in the Qt 6.11.1 installation that CI caches and the maintainer has — enabling it is `find_package(Qt6 COMPONENTS Test)` and nothing else, which is the only option that satisfies the "no new dependency" constraint at zero cost. It brings the data-driven `QTest::addColumn`/`addRow` table idiom (a perfect fit for the length/offset/algorithm matrices that make up most of these suites), `QSignalSpy` for the reader suite and for whatever R4 needs later, and `add_test` integration with no glue. doctest is the strongest rival (single header, fastest compiles) but is a new vendored dependency in `lib/`, has no Qt signal support, and would make this the only non-Qt test idiom in a Qt codebase — a real cost for a marginal compile-time win on five small binaries. Catch2 costs more build time than doctest for the same objections. GoogleTest is the heaviest and would need `FetchContent` at configure time. |
| Sharing production sources | Whole-app **OBJECT library**, static library split, **direct per-suite source listing**, link against the app executable | **Direct per-suite source listing.** An OBJECT library over `${SOURCES}` would mean rewriting the existing `qt_add_executable(${SOURCES} ...)` call — precisely the thing the constraints forbid disturbing — and it interacts badly with both the two-stage PGO flow (profile bookkeeping across an intermediate target) and `qt_add_qml_module` finalization. It would also make every sanitizer build compile the entire application twice. The per-suite listing costs a few recompiled TUs (three suites need one TU each, one needs four, one needs none) and buys total isolation: the app target, PGO, LTO, hardening, and signing are untouched, and a test binary links in seconds. The escape hatch when a suite's link set grows past what is reasonable is the phase-2 in-app mode, which is exactly why it exists. |
| Test location | `tests/cpp/` (with the pytest tiers), **`app/tests/`** | **`app/tests/`.** Two concrete reasons, both verified in the tooling. (1) `scripts/sanitize-commit.py` clang-formats `SOURCE_DIRS = ("app", "doc", "examples")` — C++ under the repo-root `tests/` tree would silently never be formatted, so the style contract would not apply to the new code at all. (2) As a subdirectory of `app`, the test targets inherit `include_directories(src)`, `AUTOMOC`, and the directory-level sanitizer/optimization flags, so `-DDEBUG_SANITIZER=ON` instruments them without extra wiring. The pytest tree stays where it is and gains a pointer in `tests/README.md`. |
| Linter coverage of test code | Extend `_is_first_party` to `app/tests`, **accept the default relaxation**, move tests under `app/src` | **Accept the relaxation, and document it.** `code-verify.py`'s `_is_first_party()` matches only consecutive `app/src` or `app/qml` path parts, so `app/tests/*.cpp` is walked (it is under a default target) and gets line-ending normalization, but not the structural rule set: no `doc-missing-brief-cpp`, no `cxx-inbody-comment`, no `cxx-function-too-long`, no `arch-singleton-instance`. For test code that is the right default — a data-driven test function is legitimately long, an explanatory in-body comment about a check vector is legitimately useful, and a test that constructs a singleton should not raise an architecture advisory. clang-format still applies, so the 100-column / 2-space / pointer-binding contract is enforced. This is a decision to record, not an oversight; opting in later is a one-line change to `_is_first_party`. |
| DSP scalar reference | Oracle reimplementation in the test, short-input coverage of the tail loop, **compile-time opt-out macro + namespace macro** | **Opt-out macro plus namespace macro.** The header has no scalar entry point today, and `simdAsciiDots16`'s scalar branch is dead code on every shipping target — so without a source hook, R5 simply cannot be met, and the two alternatives test a reimplementation of the contract rather than the contract itself. The naive form of the hook (compile the same header twice with different macros) is an **ODR violation**: `DSP::` kernels are `inline` with external linkage, so two definitions in one program let the linker fold them and the "scalar" wrapper silently calls the vector code — a test that passes for the wrong reason. Making the namespace name a macro gives the second TU distinct symbols (`DspRef::`) and removes the hazard. Cost: two default-inert lines in a hotpath header. Flagged as an open question in `spec.md` because it is a source change in a protected file. |
| CI placement | Steps inside the existing `lint` job, **an independent sibling job**, a job gated on the build jobs | **Independent sibling job, no `needs:`.** The roadmap says "the lint-tier job", meaning the fast pre-packaging tier, and this satisfies that: it starts at workflow trigger, in parallel with `lint` and the builds, and gates nothing downstream. Folding it into `lint` would make a 1-3 minute pure-Python job depend on a Qt cache restore, a CMake install, and apt packages, so a Qt cache miss would slow down style feedback for reasons that have nothing to do with style. Keeping them separate also lets the two fail independently, which is what a reviewer wants to read. |
| CI architectures | Linux x86_64 only, **x86_64 + aarch64**, all five platforms | **x86_64 + aarch64.** `SS_SIMD_X86` and `SS_SIMD_NEON` are selected from the target architecture, so an x86-only job would leave the NEON lane of every kernel unverified — and NEON is what ships on Apple Silicon and the arm64 Linux packages. Both runners already exist in the workflow. Windows and macOS are deliberately excluded from v1 to hold the job count and the time budget; the maintainer builds on Windows locally, and the presets make running the tier there a one-liner. |
| Presets scope | Presets drive CI and local builds, **presets are additive and optional** | **Additive and optional.** No CMake code reads a preset; CI's existing hand-written configure lines stay exactly as they are except in the new job. `CMakeUserPresets.json` is already in `.gitignore`, so a developer can extend the set locally without touching the tracked file. |
| `analysis` preset content | Wire `CMAKE_CXX_CLANG_TIDY`, **export `compile_commands.json` only** | **Export only.** The repo's `.clang-tidy` is `Checks: '*'` with `WarningsAsErrors: '*'`; wiring that into the build would fail on effectively every translation unit, including vendored ones, and the preset would be useless on day one. Exporting the compilation database makes `clang-tidy`, `clangd`, and `include-what-you-use` work over selected files, which is what the preset is actually for. |

## Risks & mitigations

- **ODR collision between the vector and scalar builds of `DSPSimd.h`** — the highest-value
  failure mode, because it produces a *green* test that checks nothing. Mitigated by the
  namespace macro (distinct symbols), and pinned by a deliberate meta-check: one test case
  asserts that a kernel with an architecture-specific divergence the scalar path does not
  reproduce actually reports the divergence when the vector block is perturbed. The task
  list includes a one-time manual verification (perturb a vector block, confirm red).
- **`FrameReader`'s link set grows past its carve-out.** `NotificationCenter`'s header
  reaches `QSystemTrayIcon`, `QJSEngine`, and Lua forward declarations; `SerialStudio.h`
  reaches `Frame.h`. The saving grace is that `SerialStudio::toDouble` is header-inline, so
  `SerialStudio.cpp` (which cascades into CSV/MDF4/Misc/Licensing) should not be needed.
  Expect link-error-driven iteration. Mitigation and stop rule: if the suite needs more
  than the four named TUs, stop, cover delimiter logic at the `CircularBuffer` level (which
  is where the scan lanes actually live), and move reader-level coverage to the phase-2
  in-app mode. This is written into the task's acceptance so it is a decision, not a
  slippage.
- **Qt Test runs `private slots:` in declaration order**, and the repo's style pressure is
  toward reordering declarations. `code-verify.py`'s christmas-tree auto-fix is QML-only, so
  it will not reorder C++ test slots — but a human or a future rule could. Mitigation: every
  test function is self-contained (no state carried between slots), so declaration order is
  never load-bearing; stated as a convention comment at the top of each suite.
- **`.moc` include omitted** — a suite that declares `Q_OBJECT` in a `.cpp` without
  `#include "tst_x.moc"` fails to link with an unresolved vtable, which reads as a
  mysterious error. Mitigated by the `ss_add_unit_test()` helper's documentation and by all
  five suites being written the same way.
- **ASan plus the mimalloc override.** mimalloc installs a process-wide allocator override
  (on macOS via a static `force_load` interpose, per spec 0025), which is exactly the shape
  AddressSanitizer's own interposition cannot see through — the combination produces either
  false positives or silence. The `asan` and `tsan` presets therefore set
  `SS_USE_MIMALLOC=OFF`.
- **Preset schema version versus the project's CMake floor.** The project declares
  `cmake_minimum_required(VERSION 3.20)`; `CMakePresets.json` is a client-side feature, and
  the schema version chosen determines the CMake version needed to *consume* it, not to
  build the project. The exact version-to-CMake mapping must be confirmed against the CMake
  documentation when the file is written, and the chosen version noted in `tests/README.md`.
- **Qt cache miss in the new CI job.** The Qt cache entries are restore-only (the
  `INSTALL_QT_*` flags are all `false`), keyed `qt-Linux-6.11.1-x64` and
  `qt-Linux-6.11.1-arm64`. On a miss the job must fall back to `scripts/install-qt.sh` like
  the build jobs do, which blows the five-minute budget for that one run. The AC is measured
  on a cache hit; state this in the job's comment so a slow first run is not read as a
  regression.
- **Scope creep into the app target.** Every task in this spec that touches an existing file
  touches it by two lines or fewer, except `ci.yml`. Self-review at handoff checks exactly
  that: `git diff --stat` on `app/CMakeLists.txt` and `CMakeLists.txt` must show single-digit
  line counts, and `app/src/DSPSimd.h` must show no change in generated code.
- **A green tier that tests nothing.** Five suites that all pass on day one prove very
  little. The seeded-regression exercise (AC5) is the antidote and is a required task, not
  an optional nicety.

## Test & verification plan

- **Unit (the deliverable itself):** `ctest --preset dev --output-on-failure` runs
  `dsp_kernels`, `circular_buffer`, `checksums`, `frame_serialization`, and
  `frame_delimiters`. The maintainer runs this; the assistant does not build.
- **Seeded regression (AC3, AC5):** on a scratch branch, (a) perturb one comparison in a
  `DSPSimd.h` vector block and confirm `dsp_kernels` fails; (b) perturb one byte comparison
  in the delimiter scan and confirm `frame_delimiters` fails and the CI `unit` job goes red
  with no packaging job having produced an artifact. Discard the branch.
- **Cross-architecture (AC4):** the CI `unit` job's two matrix legs both green; the log
  line each suite prints on startup names the active SIMD lane.
- **Hotpath:** `--benchmark-hotpath --min-fps 256000` before and after the `DSPSimd.h`
  edit, on the same machine, plus a preprocessed-output (`-E`) comparison of one consumer
  TU to prove the default build is unchanged.
- **Presets (AC6):** `cmake --preset dev`, `--preset asan`, `--preset tsan`,
  `--preset analysis` each configure from a clean checkout; `dev` builds and runs ctest;
  `analysis` produces `compile_commands.json`.
- **Integration / security / perf (maintainer runs, unchanged):** the existing
  `pytest tests/integration/test_frame_parsing.py` checksum and delimiter cases are the
  cross-check that the new unit vectors agree with observed end-to-end behavior. Any
  disagreement between the two tiers is a finding, not a test bug to paper over.
- **Static:** `python scripts/code-verify.py --check` over the changed files (noting that
  `app/tests` receives line-ending normalization but not the structural rule set — see the
  tradeoff table); `qt-cpp-review` on the C++ diff, focused on the `DSPSimd.h` edit;
  `python scripts/sanitize-commit.py` before commit, which clang-formats `app/tests`
  because it is under `app/`.
