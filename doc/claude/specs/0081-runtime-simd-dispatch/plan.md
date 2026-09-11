---
spec: 0081-runtime-simd-dispatch
phase: plan
status: approved     # draft -> approved (gate before /ss-tasks)
updated: 2026-09-11
---

# Plan 0081 — Runtime SIMD Dispatch

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Read the relevant `doc/claude/` sub-docs and the *actual code*
> before writing this — a plan grounded in a stale mental model is worse than no plan.
> Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

A process-wide kernel level (`DSP::SimdLevel`: Scalar, Sse4, Avx2, Neon) lives in a new
`core/Core/SimdLevel.h/.cpp` as a relaxed atomic plus a one-time CPU probe; every public kernel in
`core/Core/DSPSimd.h` reads it once per call and either runs today's SSE4/NEON loop, calls an
out-of-line AVX2 body from a new sibling header `core/Core/DSPSimdAvx2.h`, or falls straight
through to the scalar tail, which thereby becomes the selectable Scalar lane without a second body.
The AVX2 bodies are `SS_NEVER_INLINE` and carry a per-function `target("avx2")` attribute
(empty on cl.exe, which exposes the intrinsics without flags), so no translation unit is ever
compiled wide and the dispatch is a call boundary. A root-owned `Misc::SimdSettings` QObject in
`core/Ui/Misc/` persists the choice under `App/SimdLevel`, feeds the Startup-tab combobox, and
resolves the startup level from the persisted id and an optional `--simd <level>` pin in
`main.cpp` before `runApplication()`, so the benchmark, selftest and headless roots all see it.
The existing `tst_dsp_kernels` bit-exact suite gains a level loop so every kernel is checked at
every supported level against the scalar oracle.

## Affected subsystems & files

| File | Change |
|------|--------|
| `core/Core/SimdLevel.h` (new) | `namespace DSP`: `enum class SimdLevel : quint8`, `activeSimdLevel()` (relaxed load), `setActiveSimdLevel()` (refuses unsupported), `supportedSimdLevels()`, `bestSupportedSimdLevel()`, `simdLevelId()` / `parseSimdLevelId()` (stable ids `scalar`, `sse4`, `avx2`, `neon`). No Qt types beyond `QtGlobal`; no singleton. |
| `core/Core/SimdLevel.cpp` (new) | The probe: one CPUID/XGETBV sequence on every x86 toolchain (CPUID.1 OSXSAVE+AVX, XCR0 bits 1-2, CPUID.7.0 AVX2) behind two three-line shims, `__cpuidex` + `_xgetbv` from `<intrin.h>` on cl.exe and `<cpuid.h>` + an `xgetbv` asm statement on GCC/Clang families (amended during T2: clang-cl's `__builtin_cpu_supports` needs compiler-rt's `__cpu_model`, which its default link does not provide); aarch64 lists Scalar+Neon; `SS_SIMD_DISABLE` lists Scalar only. Stores the level in a `std::atomic<quint8>` initialised to Scalar. |
| `core/Core/DSPSimdAvx2.h` (new) | `namespace SS_DSP_NAMESPACE::SimdAvx2`: one `SS_NEVER_INLINE SS_TARGET_AVX2` body per kernel span (`windowedRealSpan`, `interleaveSpan`, `widenF32Span`, `forEachByteMatch`, `findAnyByte`, `widenAscii`, `powerSpectrum`, `minF64`, `maxF64`, `minMaxF64`, `finiteMinMaxPointF<kLane>`, `asciiDots16`). Each processes whole 256-bit vectors, ends with `_mm256_zeroupper()` once every 256-bit value is dead and before any 128-bit reduction tail, and returns the element count consumed (or updates `lo`/`hi` by reference) so the caller's inline SSE4 lane finishes the residue and the scalar tail resumes exactly where the scalar loop would. Included by `DSPSimd.h` only under `SS_SIMD_X86`. Amended after review: `asciiDots16` has no AVX2 body (a fixed 16-byte kernel cannot amortize the call boundary); the AVX2 level runs the 128-bit lane for it. |
| `core/Core/DSPSimd.h` | Header comment: four-lane contract and the dispatch rules. Every public kernel: `const auto level = activeSimdLevel();` then `#if SS_SIMD_X86` `if (level == Avx2) SimdAvx2::...; else if (level == Sse4) SimdSse4::...;` `#elif SS_SIMD_NEON` `if (level == Neon) SimdNeon::...;`, scalar tail unchanged. The existing SSE and NEON loops move verbatim into `SimdSse4` / `SimdNeon` helper namespaces with the AVX2 helpers' signatures (amended during T5: an inline guard around the byte scanners' for/for/if reaches nesting depth 4, which `cxx-nesting-too-deep` rejects). The SWAR 8-byte block in `simdWidenAscii` stays unconditional (plain integer code, part of the scalar reference's production form, bit-identical). `simdDeinterleaveToF64`'s multi-channel branch stays scalar at every level (it has no vector body today; single-channel goes through `widenF32Span`, which gains all lanes). |
| `core/Core/HotpathOptimization.h` | `SS_TARGET_AVX2`: `__attribute__((target("avx2")))` on GCC/Clang/AppleClang/clang-cl/MinGW/IntelLLVM, empty on cl.exe. Doc block names the two rules: never add `fma` to the target string, and never pair it with `SS_FORCE_INLINE`. |
| `core/Core/CMakeLists.txt` | Add the three new sources (layer-verify `core-unowned` fails otherwise). |
| `core/Ui/Misc/SimdSettings.h` / `.cpp` (new) | Plain `QObject`, no `instance()`. `Q_PROPERTY(QString currentLevel READ WRITE NOTIFY)` (ids: `auto`, or a level id), `Q_PROPERTY(QVariantList availableLevels CONSTANT)` of `{id, label}` where the Auto label is `tr("Auto (%1)")` with the resolved level; `static void applyConfiguredLevel(const QString& pinnedId)` reads `QSettings` key `App/SimdLevel` (default `auto`), lets a valid pin win, maps unknown or unsupported ids to `auto`, calls `DSP::setActiveSimdLevel()`, and logs only a rejected preference or pin (the per-start level line was removed at the maintainer's request on 2026-09-11). The setter validates, persists, applies live, emits. |
| `core/Ui/Misc/ContextRegistry.cpp` | Add `Cpp_Misc_SimdSettings` to the object-name table (the one list `registry-verify.py` and `hostContextNames()` share). |
| `app/src/Misc/ModuleManager.h` / `.cpp` | Own `std::unique_ptr<Misc::SimdSettings>`; construct in the ctor, `registry.add("Cpp_Misc_SimdSettings", ...)` in `registerCoreContextProperties()`. |
| `app/src/Misc/CLI.h` / `.cpp` | `simdLevelOpt` (`--simd <auto|scalar|sse4|avx2|neon>`) registered in `registerOptions()` so the parser accepts and documents it; no consumer inside `process()`. |
| `app/src/main.cpp` | Next to `GraphicsBackend::applyConfiguredBackend()` (line 260): `Misc::SimdSettings::applyConfiguredLevel(Misc::CLI::argvValueFor(argc, argv, "--simd"))`. Runs before `runApplication()`, so `CLI::process()`'s benchmark, selftest and headless roots inherit it. |
| `app/qml/Dialogs/Settings/SettingsStartupPage.qml` | New row after "Keep Display Awake": `Label` "Kernel Optimization" + `Widgets.Combo` bound to `Cpp_Misc_SimdSettings.availableLevels` / `currentLevel` (the `currentIndex` lookup + `onActivated` shape of the Rendering Backend row, without the restart prompt), plus a footnote label. |
| `app/tests/tst_dsp_kernels.cpp` | Each kernel test iterates `DSP::supportedSimdLevels()`, sets the level, runs the kernel, compares bitwise against the `DspSimdScalar` oracle. New slots: `levelSelectionRefusesUnsupported`, `scalarLevelMatchesOracleOnEveryInput`, `idRoundTrip`. Length tables extended with widths 1..33 so 256-bit, 128-bit and tail boundaries are all crossed. |
| `.github/workflows/ci.yml` | Linux x86_64 and Windows: a second gated `--benchmark-hotpath --min-fps 256000 --simd sse4` step and an informational `--simd scalar` step (`continue-on-error`). Linux arm64: gated `--simd neon` is Auto already; add informational `--simd scalar`. macOS Intel (Rosetta) stays informational. |
| `scripts/tests/test_ci_workflow.py` | Only if its step-shape rules reject the added steps (check during implementation). |
| `doc/claude/architecture/kernels.md` | Four-lane contract, the dispatch rules (level read once per call, noinline + target attribute, zeroupper, no FMA, no wide TU), the settings key and the CLI pin. |
| `CLAUDE.md` | One clause in the "Reuse the kernels" bullet: lanes are selected at runtime through `DSP::activeSimdLevel()`; wide bodies live in `DSPSimdAvx2.h` under the same bit-exact contract. |

Not touched: `app/tests/CMakeLists.txt` (the existing `tst_dsp_kernels` registration and scalar-oracle
TU stay as they are), `doc/help` (the manual does not document the System-section rows today),
`scripts/code-verify.py` (`DSPSimd.h` uses `SS_ASSERT`, not `SS_ASSERT_HOTPATH`, so the
`hotpath-assert-scope` whitelist needs no entry for the new header), translations (`.ts` are derived).

## Architecture & data flow

- **Level storage.** `SimdLevel.cpp` holds `static std::atomic<quint8> s_level{Scalar}` and a
  function-local static `Supported` struct filled by the probe on first use (not on the hotpath).
  `activeSimdLevel()` is `SS_FORCE_INLINE`-free, `noexcept`, one relaxed load; every kernel copies it
  into a local once, so a switch mid-span cannot happen and no ordering is needed (every lane is
  bit-identical, so a stale read is never observable).
- **Startup.** `main()` → `setupQtApplicationMetadata()` (QSettings usable) → `prepareEnvironment()`
  → **`SimdSettings::applyConfiguredLevel(pin)`** → `GraphicsBackend::applyConfiguredBackend()` →
  `runApplication()`. Resolution order: valid supported pin > valid supported persisted id > Auto
  (best supported). An unknown or unsupported pin logs `[simd] ignoring --simd <x>` and falls
  through. Kernels invoked before this point (none known) would run Scalar, which is safe anywhere.
- **Live change.** QML `onActivated` → `SimdSettings::setCurrentLevel(id)` (GUI thread) →
  `QSettings` write → `DSP::setActiveSimdLevel()` (relaxed store) → `currentLevelChanged`. Pipeline,
  stream-worker and render threads pick the new level up on their next kernel call.
- **Selftest and benchmark roots.** `CLI::process()` runs after `applyConfiguredLevel()`, so
  `runHotpathBenchmark()` and the `--selftest` suites inherit the level with no root-specific code.
- **ctest.** The unit executables never run `main.cpp`; the suite sets the level explicitly per
  iteration and restores Auto at the end of each slot.

## Hotpath & threading impact

- **Touches the hotpath?** Yes. `simdFindAnyByte` sits in `CircularBuffer`'s multi-pattern scan,
  `simdWidenAscii` in `Frame.h`'s `assign_utf8_in_place` on the span fast lane,
  `simdForEachByteMatch` in the Native text-delimited split, `simdDeinterleaveToF64` on the
  stream-worker thread, and the reductions on the Dashboard draw path. Preservation: no allocation
  (the level is a static atomic, the local copy is a byte), no lock, no signal, no queue; the SSE4
  and NEON loops are untouched, only guarded by an equality compare on a value already in a
  register; the dispatch is once per span, never per element; the AVX2 call boundary is taken only
  when the wide lane is active. `--benchmark-hotpath` plan: three local runs each of the current
  build and the new build pinned `--simd sse4` (the dispatch-overhead measurement), then `--simd
  auto`; the SSE4-pinned median must sit inside the run-to-run band of the current build (CI's
  ±45-56% spread cannot prove a small delta, so the local repeated run is the evidence). The
  per-frame kernels handle tens of bytes, so the expected result is a flat gate; the wins are in
  the plot, FFT, waterfall and stream staging costs reported by the `lua+dashboard` row.
- **New cross-thread signal/slot?** No. The GUI writes and the pipeline/stream/render threads read
  one relaxed atomic; `currentLevelChanged` is GUI-to-QML only.
- **New input to a cached hotpath flag?** No existing flag changes. The level is itself a cached
  value with one writer and no derivation, so there is no refresh to wire.
- **Timestamp ownership.** Unaffected; no kernel touches a timestamp.

## Data model & persistence

`QSettings` key `App/SimdLevel`, string id, default `auto`. Persisted as an id rather than an
integer so a settings file carried between an x86 and an ARM machine, or edited by hand, can never
name a level by an ambiguous number (R6). No project JSON, no schema version, no API surface.

## API / SDK surface

None. The spec scopes the control to Preferences and the command line; no `system.*` handler is
added, and no `EnumLabels` slug.

## QML / UI

One row in the System section of `SettingsStartupPage.qml`, between "Keep Display Awake" and the
performance-hints footnote: `Label` "Kernel Optimization" and a `Widgets.Combo` whose model is
`availableLevels.map(e => e.label)`, with the same `currentIndex` lookup-by-id binding and
`onActivated` guard as the Rendering Backend combo. No restart prompt, since the change applies
live. A second footnote label explains: "Auto uses the widest instruction set this CPU supports.
Every level produces identical values; lower levels are for troubleshooting."

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Dispatch shape | Wrapper branch per kernel; function-pointer table; whole-TU duplicate; compiler `target_clones` | **Wrapper branch.** Baseline lane stays today's inlinable code, the scalar tail is the Scalar lane for free, template kernels keep working. Pointer table kills inlining on the span lane; whole-TU breaks the spec's no-wide-TU rule and there is no TU to duplicate; `target_clones` is ELF-only. |
| Where AVX2 bodies live | Inside `DSPSimd.h`; sibling `DSPSimdAvx2.h` | **Sibling header.** `DSPSimd.h` is 763 lines and the TU census counts headers against 1500; a second lane per kernel would land near the cap. |
| Settings object lifetime | Meyers singleton with `instance()` (GraphicsBackend precedent); root-owned QObject | **Root-owned.** The singleton census blocks any growth in `instance()` reaches; a root-owned object is census-neutral and matches spec 0077's direction. |
| Level storage | Plain static bool like `Core::Runtime`; `std::atomic` relaxed | **Relaxed atomic.** The GUI writes while three other threads read; a plain static is a data race the TSan CI leg would report. Cost is identical to a plain load on x86 and ARM. |
| CPU probe | `__builtin_cpu_supports` everywhere; hand-rolled CPUID everywhere; split by ABI | **Split by ABI.** `<intrin.h>` CPUID/XGETBV is guaranteed on cl.exe and clang-cl without a compiler-rt dependency; `__builtin_cpu_supports` on GCC/Clang already includes the OS XSAVE check. `IsProcessorFeaturePresent(PF_AVX2...)` was rejected: the constant is absent on older Windows 10 SDKs. |
| Target attribute on cl.exe | Emulate via `/arch` per TU; rely on intrinsics being always available | **Rely on intrinsics.** cl.exe compiles `_mm256_*` in a baseline TU; only the noinline boundary and explicit `_mm256_zeroupper()` are needed. |
| Apply timing | Next start (matches the neighbouring rows); immediate | **Immediate**, per the spec's open question: bit-identical lanes have no observable seam and live switching is the support-triage use case. |
| Pin semantics | Pin makes the run read-only; pin sets the startup level only | **Startup only.** A user changing the combobox during a pinned run persists that choice like any other edit; the pin is a launch argument, not a mode. Keeps `SimdSettings` free of a second state. |

## Risks & mitigations

- **Wide code leaking into baseline callers.** Header-inline AVX2 bodies are emitted per TU as
  COMDATs, but every copy is AVX2 and `SS_NEVER_INLINE`, so a linker's pick is always an
  out-of-line AVX2 function called only behind the level compare. Mitigation is structural; the
  Rosetta and any AVX-less machine run is the observation (AC9).
- **SSE/AVX transition penalties.** Mixed VEX and legacy encodings inside one body stall pre-Skylake
  parts and create false dependencies later. Every AVX2 body calls `_mm256_zeroupper()` before its
  128-bit reduction tail and before return; GCC/Clang insert one automatically, cl.exe does not.
- **Floating-point contraction.** GCC's default `-ffp-contract=fast` would fuse `(raw+offset)*scale`
  if FMA were available. The target string is `avx2` alone, so no FMA instruction can be emitted;
  the bit-exact test catches any regression of this rule.
- **MinGW 32-byte stack alignment.** GCC realigns frames that hold `__m256` locals; the historical
  failure is passing `__m256` by value across a call, which no body does. No MinGW CI leg exists;
  named as a known gap.
- **Coverage gap on MSVC.** ctest runs only on the two Linux jobs, so the cl.exe codegen of the AVX2
  bodies is compared bitwise nowhere in CI; the Windows benchmark steps exercise them for crashes
  only. Mitigation: the maintainer runs `ctest` against the Windows build once during
  implementation (AC1 on Windows is a manual observation until ctest returns to that job).
- **Dispatch overhead on the span lane.** One load and one predictable compare per kernel call, on
  kernels called per dataset per frame. Gated by the local repeated benchmark; if the SSE4-pinned
  median falls outside the current band, the fallback is to read the level once per frame in
  `assign_utf8_in_place`'s caller and pass it down, not to remove the runtime choice.
- **Early callers before `applyConfiguredLevel()`.** Run Scalar, correct on every machine; the
  window is the first few milliseconds of `main()`.
- **CI workflow shape rules.** `scripts/tests/test_ci_workflow.py` encodes step conventions and the
  ctest matrix; the added benchmark steps must satisfy it or its rule is updated in the same pass.
- **Silent-breakage classes from `common-mistakes.md`.** None of the cached-flag, queued-hop,
  slot-pool or timestamp classes apply: no flag, no signal, no allocation, no stamp is added.

## Test & verification plan

- **AC1 (bit-exact at every level)** — `app/tests/tst_dsp_kernels.cpp`, every slot looped over
  `DSP::supportedSimdLevels()` against the `DspSimdScalar` oracle, including denormals, NaN, both
  infinities, both zeros, empty spans and lengths 1..33. Runs in `ctest` on the Linux x86_64 and
  arm64 jobs; maintainer runs it once on the Windows build (MSVC codegen).
- **AC2 (unsupported level refused)** — new slot `levelSelectionRefusesUnsupported`: every level not
  in the supported list is rejected by `setActiveSimdLevel()` and the active level is unchanged;
  `parseSimdLevelId` rejects unknown ids.
- **AC3 (combobox contents and persistence)** — maintainer observation on the AVX2 machine and on
  Apple Silicon: entries, Auto label, choice survives a restart.
- **AC4 (no visible change across levels)** — maintainer observation with a plot, FFT and waterfall
  live; export the same CSV window at each level and compare byte-for-byte.
- **AC5 (gate passes pinned)** — `ci.yml` gated steps `--simd sse4` and Auto on Linux x86_64 and
  Windows; `--simd scalar` informational. Locally: three runs each, current vs new, pinned SSE4.
- **AC6 (baseline lane exercised in CI)** — the `--simd sse4` gate step above; a regression in
  dispatch overhead fails the job on the lane the runner would otherwise never take.
- **AC7 (startup log)** — struck with R10; `serial-studio --headless --simd bogus` prints only
  the `[simd] ignoring --simd bogus` rejection line.
- **AC8 (scalar-only build)** — the existing oracle TU compiled with `SS_SIMD_DISABLE` already proves
  the header builds scalar-only; the new `supportedSimdLevels()` under that define returns Scalar
  only, asserted in the suite via the oracle namespace's copy.
- **AC9 (Rosetta)** — the existing informational macOS Intel benchmark step; the level line in its
  log shows whether the translation layer advertised AVX2, and the run must still pass.
- **Static:** `python scripts/code-verify.py --check` on every touched file; `layer-verify.py`
  (new core sources owned, no upward include); `registry-verify.py` (context name);
  `--singleton-census --check` must show zero growth; `--tu-census --check` unchanged;
  `claim-verify.py` after the `kernels.md` and `CLAUDE.md` edits; `qt-cpp-review` before handoff;
  `sanitize-commit.py` before commit.
