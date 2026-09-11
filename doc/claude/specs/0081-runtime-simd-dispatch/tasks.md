---
spec: 0081-runtime-simd-dispatch
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-09-11
---

# Tasks 0081 — Runtime SIMD Dispatch

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
- The assistant never compiles. Every "build" or "ctest" verify step is the maintainer's; the
  assistant's verify is the linter plus a read-back of the diff against the plan.

## Tasks

### T1 — `SS_TARGET_AVX2` macro

- **Files:** `core/Core/HotpathOptimization.h`
- **Does:** Adds `SS_TARGET_AVX2` to the toolchain cascade: `__attribute__((target("avx2")))`
  when `__GNUC__` or `__clang__` (covers GCC, Clang, AppleClang, clang-cl, MinGW, IntelLLVM),
  empty under cl.exe (`_MSC_VER && !__clang__`, which exposes `_mm256_*` without flags). The
  `@brief` states the two binding rules: the target string is `avx2` alone (never `fma`, which
  would let the compiler contract multiply-add and break the per-lane bit-exact contract), and
  the macro is always paired with `SS_NEVER_INLINE`, never `SS_FORCE_INLINE`, so the wide body is
  a call boundary the compiler cannot hoist across.
- **Verify:** `python scripts/code-verify.py --check core/Core/HotpathOptimization.h`; read-back
  that no fast-math / no-unwind / `optimize("...")` spelling was introduced.
- **Deps:** none
- [x] done

### T2 — `DSP::SimdLevel` storage, probe and ids

- **Files:** `core/Core/SimdLevel.h` (new), `core/Core/SimdLevel.cpp` (new),
  `core/Core/CMakeLists.txt`
- **Does:** Declares `enum class SimdLevel : quint8 { Scalar, Sse4, Avx2, Neon }` and the free
  functions `activeSimdLevel()` (one relaxed load of a namespace-static `std::atomic<quint8>`
  initialised to Scalar; `noexcept`, no branch, no Qt), `setActiveSimdLevel(level) -> bool`
  (refuses a level not in the supported list, returns false, leaves the active level untouched),
  `supportedSimdLevels()` (fixed-size list, ascending; filled once by the probe in a function-local
  static, never on a kernel path), `bestSupportedSimdLevel()`, `simdLevelId(level)` and
  `parseSimdLevelId(QLatin1StringView) -> std::optional<SimdLevel>` over the stable ids
  `scalar`/`sse4`/`avx2`/`neon`. Probe: under `_MSC_VER` (cl.exe and clang-cl) `__cpuidex` +
  `_xgetbv` from `<intrin.h>` (CPUID.1 ECX OSXSAVE bit 27 and AVX bit 28, XCR0 bits 1 and 2 both
  set, CPUID.7.0 EBX AVX2 bit 5); on every other x86-64 toolchain `__builtin_cpu_init()` then
  `__builtin_cpu_supports("avx2")`; aarch64 lists Scalar and Neon; `SS_SIMD_DISABLE` lists Scalar
  only. No singleton, no `SessionContext`, nothing that could run before `main()` besides the
  zero-initialised atomic. Registers both files in the `SerialStudioCore` source list.
- **Verify:** `python scripts/code-verify.py --check core/Core/SimdLevel.h core/Core/SimdLevel.cpp`;
  `python scripts/layer-verify.py` (new sources owned by exactly one target, no upward include).
- **Deps:** none
- [x] done

### T3 — AVX2 bodies: byte kernels

- **Files:** `core/Core/DSPSimdAvx2.h` (new), `core/Core/CMakeLists.txt`
- **Does:** Creates the header (`#pragma once`, `<immintrin.h>`, `Core/SSAssert.h`,
  `Core/HotpathOptimization.h`; `namespace SS_DSP_NAMESPACE::SimdAvx2`) and writes the 256-bit
  bodies for `forEachByteMatch` (template on the callback, 32-byte blocks, `_mm256_movemask_epi8`
  + `countr_zero` walk in ascending order, same early-abort semantics), `findAnyByte` (up to 8
  needle patterns, first hit index or the consumed count), `widenAscii` (32 bytes in, 32 UTF-16
  units out via `_mm256_cvtepu8_epi16` on each 128-bit half, OR-accumulated high bits) and
  `asciiDots16` (16 bytes in, one 256-bit store out). Binding invariants named here so the edit
  states them: op classes limited to byte compares, masks, integer ops; every body is
  `SS_NEVER_INLINE SS_TARGET_AVX2`; `_mm256_zeroupper()` before return and before any 128-bit tail;
  each body returns the element index it stopped at so the caller's scalar tail resumes exactly
  where the SSE loop's tail would; `SS_ASSERT` density at least two per function; fixed loop
  bounds; no allocation. Registers the header in the `SerialStudioCore` source list.
- **Verify:** `python scripts/code-verify.py --check core/Core/DSPSimdAvx2.h`; read-back against
  the four SSE bodies in `DSPSimd.h` line by line for semantic parity (bit walk order, abort
  path, high-bit accumulation).
- **Deps:** T1
- [x] done

### T4 — AVX2 bodies: float kernels and reductions

- **Files:** `core/Core/DSPSimdAvx2.h`
- **Does:** Adds the 4-wide f64 / 8-wide f32 bodies: `windowedRealSpan` (finite mask via
  `andnot` sign + `cmp lt +inf`, add then mul in f64, `_mm256_cvtpd_ps`, mul in f32; same op order
  as the scalar), `interleaveSpan` (`unpacklo/hi_pd` + `permute2f128`), `widenF32Span`
  (`_mm256_cvtps_pd` per 128-bit half), `powerSpectrum` (the power stage only, `re*re + im*im`
  as two separate ops, `max` operands ordered so NaN propagates like `std::max`), `minF64`,
  `maxF64`, `minMaxF64` (accumulators seeded from `p[0]`, `_mm256_min_pd`/`max_pd` with the
  same operand order as the SSE lane, then `_mm256_zeroupper()` and a 128-bit horizontal tail
  that reproduces the existing `min_sd`/`max_sd` reduction), and `finiteMinMaxPointF<kLane>`
  (`blendv_pd` non-finite to the opposite infinity, then min/max into the caller-seeded
  accumulators). Binding invariants: no horizontal float sums, no reassociation, no FMA, no
  approximate transcendentals; the `-0.0`/`+0.0` sign exception is the only tolerated divergence;
  each body returns the consumed count or updates `lo`/`hi` by reference; header stays under the
  1500-line TU census cap.
- **Verify:** `python scripts/code-verify.py --check core/Core/DSPSimdAvx2.h`;
  `python scripts/code-verify.py --tu-census --check`; read-back of operand order against the SSE
  lane for every `min`/`max`/`add`/`mul`.
- **Deps:** T3
- [x] done

### T5 — Dispatch wiring: byte kernels and header contract

- **Files:** `core/Core/DSPSimd.h`
- **Does:** Includes `Core/SimdLevel.h`, and `Core/DSPSimdAvx2.h` under `SS_SIMD_X86`; rewrites
  the file `@file` block to the four-lane contract (Scalar, SSE4, AVX2, NEON; selected at runtime
  through `DSP::activeSimdLevel()`; wide bodies out-of-line; scalar tail is the Scalar lane).
  Wires `simdForEachByteMatch`, `simdFindAnyByte`, `simdWidenAscii` and `simdAsciiDots16`: one
  `const SimdLevel level = activeSimdLevel();` at the top, then under `SS_SIMD_X86`
  `if (level == SimdLevel::Avx2) { i = SimdAvx2::...; } else if (level == SimdLevel::Sse4) {
  existing SSE loop, byte-for-byte }`, under `SS_SIMD_NEON` `if (level == SimdLevel::Neon) {
  existing NEON loop }`, scalar tail untouched. `simdAsciiDots16` needs a real scalar fallback
  branch at runtime (today its scalar body is `#else`-only). The SWAR 8-byte block in
  `simdWidenAscii` stays unconditional. Binding invariants: the level is read once per call
  into a local (never per element, never re-read), no allocation, no lock, the SSE/NEON loops
  move under the guard without any other change, and every kernel still returns the identical
  value at every level.
- **Verify:** `python scripts/code-verify.py --check core/Core/DSPSimd.h`; `git diff` read-back
  confirming the SSE/NEON loop bodies are unchanged apart from indentation.
- **Deps:** T2, T3
- [x] done (loops moved verbatim into `SimdSse4` / `SimdNeon` helper namespaces: an inline guard would have pushed the byte scanners to nesting depth 4, a hard lint error; intrinsic multiset identical to HEAD)

### T6 — Dispatch wiring: float kernels and reductions

- **Files:** `core/Core/DSPSimd.h`
- **Does:** Same wiring for `SimdDetail::windowedRealSpan`, `interleaveSpan`, `widenF32Span`,
  `simdPowerSpectrumDb`, `simdMinF64`, `simdMaxF64`, `simdMinMaxF64` and
  `simdFiniteMinMaxPointF<kLane>`. The reductions keep their `n >= 4` entry guard on the SSE and
  NEON lanes and use `n >= 8` for the AVX2 lane so a short span never enters a wide body; the
  wide body's tail hand-off writes `i`, `lo`, `hi` exactly as the SSE lane does before the scalar
  loop resumes. `simdDeinterleaveToF64`'s multi-channel branch stays scalar at every level
  (documented in its `@brief`); `simdWindowedRealFill` and `simdRingsToPoints` need no change
  since they delegate to the span kernels. Binding invariants as T5, plus: the per-lane float op
  order is the scalar's (add, mul, convert, mul), and the file stays under 1500 lines.
- **Verify:** `python scripts/code-verify.py --check core/Core/DSPSimd.h`;
  `python scripts/code-verify.py --tu-census --check`; maintainer builds the tree once here (the
  first compile of all four lanes on the local toolchain).
- **Deps:** T4, T5
- [x] done (same rewrite as T5; header 1168 lines, census unchanged; maintainer compile pending)

### T7 — Bit-exact suite: level loop and selection tests

- **Files:** `app/tests/tst_dsp_kernels.cpp`
- **Does:** Adds a `forEachSupportedLevel(callable)` helper that iterates
  `DSP::supportedSimdLevels()`, calls `DSP::setActiveSimdLevel()` before each pass, and restores
  `bestSupportedSimdLevel()` afterwards; every existing kernel slot runs its comparison inside it,
  with the level id in the `QVERIFY2` message. Extends the shared length/offset table to cover
  lengths 1 through 33 and 63/64/65 so 32-byte, 16-byte and tail boundaries are all crossed at
  every level. New slots: `levelSelectionRefusesUnsupported` (every enumerator absent from the
  supported list is refused and the active level is unchanged), `idRoundTrip` (`simdLevelId` and
  `parseSimdLevelId` invert each other; an unknown id parses to nothing),
  `scalarLevelMatchesOracleOnEveryInput` (Scalar forced, every kernel, every table row), and
  `scalarOnlyBuildListsScalar` guarded to run when `supportedSimdLevels().size() == 1`.
- **Verify:** `python scripts/code-verify.py --check app/tests/tst_dsp_kernels.cpp`; maintainer:
  `ctest --test-dir <build> -R tst_dsp_kernels --output-on-failure` on the local machine, and once
  on a Windows (MSVC) build for the cl.exe codegen of the AVX2 bodies (AC1, AC2, AC8).
- **Deps:** T6
- [x] done (a `level` data column on every table subsumes the planned `scalarLevelMatchesOracleOnEveryInput` slot: Scalar rows run every kernel on every input; maintainer ctest pending)

### T8 — `Misc::SimdSettings` object

- **Files:** `core/Ui/Misc/SimdSettings.h` (new), `core/Ui/Misc/SimdSettings.cpp` (new),
  `core/Ui/CMakeLists.txt`
- **Does:** Plain `QObject` with no `instance()`. `Q_PROPERTY(QString currentLevel READ
  currentLevel WRITE setCurrentLevel NOTIFY currentLevelChanged)` and `Q_PROPERTY(QVariantList
  availableLevels READ availableLevels CONSTANT)` of `{id, label}` maps: `auto` first with label
  `tr("Auto (%1)")` naming the best supported level, then each supported level with its display
  label (`Scalar`, `SSE4`, `AVX2`, `NEON`). Static `applyConfiguredLevel(const QString& pinnedId)`
  reads `QSettings` key `App/SimdLevel` (default `auto`), resolves pin > preference > auto,
  maps an unknown or unsupported id to `auto` (and logs `[simd] ignoring --simd <x>` for a bad
  pin), calls `DSP::setActiveSimdLevel()`, and prints one `qInfo().noquote()` line
  `[simd] level=<id> (auto|preference|pinned)` (line later removed at the maintainer's request,
  2026-09-11). The ctor reads the persisted id and mirrors an
  unsupported one as `auto` without rewriting the file; the setter validates, persists the id,
  applies live through `DSP::setActiveSimdLevel()`, emits. Header follows the repo layout
  (`Q_OBJECT` → `Q_PROPERTY` → `signals:` → ctor/deleted copies → `public:` → `public slots:` →
  `private:`, no in-header member init). Registers the pair in the `SerialStudioUi` source list.
- **Verify:** `python scripts/code-verify.py --check core/Ui/Misc/SimdSettings.h
  core/Ui/Misc/SimdSettings.cpp`; `python scripts/layer-verify.py`;
  `python scripts/code-verify.py --singleton-census --check` shows zero growth.
- **Deps:** T2
- [x] done (census 794/794, layer-verify clean)

### T9 — Context registration and root ownership

- **Files:** `core/Ui/Misc/ContextRegistry.cpp`, `app/src/Misc/ModuleManager.h`,
  `app/src/Misc/ModuleManager.cpp`
- **Does:** Adds `Cpp_Misc_SimdSettings` to the object-name table in `ContextRegistry.cpp` (the
  one list `registry-verify.py` and `WidgetExtensions::hostContextNames()` read; there is no
  second copy to update). `ModuleManager` gains `std::unique_ptr<Misc::SimdSettings>
  m_simdSettings` (forward-declared in the header, constructed in the ctor init list after
  `m_settings`) and `registry.add("Cpp_Misc_SimdSettings", m_simdSettings.get())` in
  `registerCoreContextProperties()` next to the GraphicsBackend line. Binding invariant: nothing
  here reaches an application singleton, and `instantiateCoreModules()` order is untouched.
- **Verify:** `python scripts/code-verify.py --check` on the three files;
  `python scripts/registry-verify.py`; `python scripts/code-verify.py --singleton-census --check`.
- **Deps:** T8
- [x] done (census flat; registry-verify's one failure is the maintainer's in-progress `Cpp_AppCommit`, not this change)

### T10 — CLI pin and startup application

- **Files:** `app/src/Misc/CLI.h`, `app/src/Misc/CLI.cpp`, `app/src/main.cpp`
- **Does:** Adds `simdLevelOpt` (`--simd <level>`, value name `auto|scalar|sse4|avx2|neon`, help
  text says it pins the kernel level for this run and is not saved) to `CliOptions` and
  `registerOptions()` so the parser accepts and documents it; `process()` does not consume it.
  In `main()`, immediately before `GraphicsBackend::applyConfiguredBackend()`, calls
  `Misc::SimdSettings::applyConfiguredLevel(Misc::CLI::argvValueFor(argc, argv, "--simd"))`.
  Binding invariants: this sits after `setupQtApplicationMetadata()` (QSettings needs the
  organisation and application names) and before `runApplication()`, so `CLI::process()`'s
  benchmark, selftest and headless roots inherit the level; no `ModuleManager` member carries
  it because the benchmark root never constructs one.
- **Verify:** `python scripts/code-verify.py --check` on the three files; maintainer:
  `serial-studio --headless --simd sse4` prints nothing about the level (line removed), `--simd bogus`
  prints the ignore line then `level=<best> (auto)` (AC7, half of AC9's evidence).
- **Deps:** T8
- [x] done (maintainer run pending)

### T11 — Preferences combobox

- **Files:** `app/qml/Dialogs/Settings/SettingsStartupPage.qml`
- **Does:** Inserts, after the "Keep Display Awake" row and before the performance-hints
  footnote, a `Label` "Kernel Optimization" and a `Widgets.Combo` with `model:
  Cpp_Misc_SimdSettings.availableLevels.map(e => e.label)`, the `currentIndex` lookup-by-id
  binding and the `onActivated` early-return guard copied from the Rendering Backend row, writing
  `Cpp_Misc_SimdSettings.currentLevel = id` with no restart prompt. Adds a second footnote label
  in the existing style: "Auto uses the widest instruction set this CPU supports. Every level
  produces identical values; lower levels are for troubleshooting." All strings through `qsTr`.
- **Verify:** `python scripts/code-verify.py --check app/qml/Dialogs/Settings/SettingsStartupPage.qml`;
  maintainer: open Preferences → Startup, confirm the entries and Auto label, pick SSE4, restart,
  reopen (AC3); switch levels with a plot, FFT and waterfall live and diff two CSV exports (AC4).
- **Deps:** T9
- [x] done (maintainer observation pending)

### T12 — Hotpath benchmark checkpoint (maintainer)

- **Files:** none
- **Does:** Clean measurement point with all lanes wired and the pin available. Maintainer
  builds the production configuration and runs `--headless --benchmark-hotpath --min-fps 256000`
  three times each for: the previous commit's binary, the new binary with `--simd sse4`, and the
  new binary with `--simd auto`; records the medians of the parser tiers and the `lua+dashboard`
  row. Pass condition: the SSE4-pinned median sits inside the previous build's three-run band
  (dispatch overhead invisible); Auto is reported, not gated. If SSE4-pinned falls outside the
  band, the fallback named in the plan applies (read the level once per frame in
  `assign_utf8_in_place`'s caller and pass it down) before continuing.
- **Verify:** the recorded numbers pasted into this task's done line.
- **Deps:** T10
- [x] done (2026-09-11, maintainer's Windows MSVC Release build, NOT the PGO build CI gates on; no
  pre-change binary on the machine. native(numeric) frames/s: Auto 1,006,290 and 973,850; SSE4
  pinned 1,002,555; Scalar pinned 1,014,179. Run-to-run spread 4 %, lane choice inside it, so the
  dispatch cost is not measurable; extract stage 80-85 ns on every lane. All four runs miss the
  4x tier (1,024,000) by 1-5 % and pass every other gate, which is this non-PGO local build's
  standing, not a lane effect: Scalar, which takes no dispatch branch and no vector lane, is the
  fastest of the four. The CI PGO binary is the gate of record.)

### T13 — CI benchmark steps

- **Files:** `.github/workflows/ci.yml`, `scripts/tests/test_ci_workflow.py` (only if its
  step-shape rules reject the additions)
- **Does:** Linux x86_64 and Windows jobs: after the existing gated step, a second gated step
  `--benchmark-hotpath --min-fps 256000 --simd sse4` (Windows through the same `Start-Process` +
  `HOTPATH_PASS=1` grep shape), and an informational `--simd scalar` step with
  `continue-on-error: true`. Linux arm64: informational `--simd scalar` only (Auto is already
  NEON). macOS jobs unchanged. Runs `python -m pytest scripts/tests/test_ci_workflow.py` and
  amends its rule only where the new steps are rejected for shape, never to weaken the ctest
  matrix assertions.
- **Verify:** `python -m pytest scripts/tests/test_ci_workflow.py -q`; YAML read-back of each new
  step against its sibling (AC5, AC6).
- **Deps:** T10
- [x] done (40 workflow tests pass unchanged; the scalar readout runs at `--min-fps 1` so the never-soft rule stays intact)

### T14 — Docs: kernels.md and CLAUDE.md

- **Files:** `doc/claude/architecture/kernels.md`, `CLAUDE.md`
- **Does:** `kernels.md` gains a "Runtime lanes (spec 0081)" block: the four lanes, the dispatch
  rules (level read once per call, wide bodies `SS_NEVER_INLINE SS_TARGET_AVX2` in
  `core/Core/DSPSimdAvx2.h`, `_mm256_zeroupper()` before any 128-bit tail, target string `avx2`
  alone, no TU compiled wide), the settings key `App/SimdLevel`, the `--simd` pin, and the
  `tst_dsp_kernels` level loop as the mechanical check. `CLAUDE.md`'s "Reuse the kernels" bullet
  gains one clause naming runtime selection through `DSP::activeSimdLevel()` and the sibling
  header. No other doc changes.
- **Verify:** `python scripts/claim-verify.py` clean (every new path and symbol resolves);
  `python scripts/documentation-verify.py` unaffected (AI-facing tier is exempt, run for safety).
- **Deps:** T6, T10
- [x] done (claim-verify 0 errors, documentation-verify 0 findings)

### T15 — Static gates and self-review

- **Files:** none new
- **Does:** Runs the whole gate set over the finished diff: `python scripts/code-verify.py
  --check`, `--singleton-census --check`, `--tu-census --check`, `--dup-census --check`,
  `python scripts/layer-verify.py`, `python scripts/registry-verify.py`,
  `python scripts/claim-verify.py`, then `qt-cpp-review` on the C++ diff, then the counterfactual
  check: the rule this diff most risks is "wide code visible to a baseline caller", and the
  evidence is that every `_mm256_` token lives inside a `SS_NEVER_INLINE SS_TARGET_AVX2` function
  in `DSPSimdAvx2.h` and nowhere else (`grep -rn _mm256_ core app/src`). Confirms the diff touches
  only the files in the plan's table.
- **Verify:** every command above exits 0; grep output lists only `DSPSimdAvx2.h`.
- **Deps:** T7, T11, T13, T14
- [x] done (all gates clean except registry-verify's pre-existing `Cpp_AppCommit`; the only
  `_mm256_` hit outside the AVX2 header is a doc comment in `HotpathOptimization.h`.
  `qt-cpp-review` ran six agents; fixed from it: SimdSettings built after the Translator,
  `currentLevel` follows a `--simd` pin, out-of-line `~ModuleManager`, `argvValueFor` accepts
  `--flag=value`, ignored-preference log line, one zeroupper per reduction body after both
  accumulators are split, width-gated AVX2 dispatch chained into the inline SSE lane, and the
  16-byte hex-dump kernel keeps its 128-bit lane at every level, a deliberate exception to R1
  because a fixed 16-byte kernel cannot amortize a call boundary. `sanitize-commit.py` NOT run:
  the tree holds the maintainer's unrelated in-progress edits and the script rewrites in place.)

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` is met and checked off there (AC1/AC2/AC8 via
      `tst_dsp_kernels` on Linux CI plus one Windows ctest run; AC3/AC4/AC7 maintainer
      observations; AC5/AC6 the new CI steps; AC9 the Rosetta informational step's log line).
- [ ] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [ ] `qt-cpp-review` run on the C++ diff; findings addressed or noted.
- [ ] `--benchmark-hotpath` not regressed: T12's SSE4-pinned median inside the previous build's
      band; Auto recorded.
- [ ] Relevant `pytest` targets identified for the maintainer: `scripts/tests/test_ci_workflow.py`
      (no `tests/integration` case applies; the feature has no API surface).
- [ ] `python scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [ ] Diff is *what was asked, and only that* — no scope creep, no foreign files touched,
      no `.ts`/`.qm` edits.
- [ ] `spec.md` status set to `done`.
