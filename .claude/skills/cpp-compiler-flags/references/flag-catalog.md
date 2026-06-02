# Cross-compiler flag catalog

A working reference for what individual flags do across GCC, Clang, and MSVC, plus the PGO
concept. Distilled from the canonical upstream docs (see Sources). Use it to look up a flag or
find a cross-compiler equivalent -- then cross-check against
`serial-studio-build-flags.md` before recommending a change here. **A flag being valid is not a
reason to add it**: this repo bans fast-math, pins x86-64-v2, etc. (see the invariants in
SKILL.md).

Spellings are exact; the leading `/` vs `-` is part of the flag. Where a source documents a
flag without inline prose, or omits a family you'd expect, that caveat is noted -- do not
attribute a claim to a source that doesn't make it.

## Optimization levels

| Intent | GCC / Clang | MSVC cl.exe |
|--------|-------------|-------------|
| None (debug) | `-O0` | `/Od` |
| Light | `-O1` | `/O1` (favor size) |
| Standard release | `-O2` | `/O2` (favor speed) |
| Aggressive | `-O3` | (no direct equal; `/O2` + `/Ob3` is the closest) |
| Size | `-Os` (GCC/Clang), `-Oz` (Clang only, more aggressive) | `/Os`, `/O1` |
| Debug-friendly opt | `-Og` | -- |
| **Avoid** | `-Ofast` (= `-O3 -ffast-math`; Clang deprecates the spelling) | `/fp:fast` territory |

MSVC inline control is `/Ob0` (none) / `/Ob1` (marked-only) / `/Ob2` (compiler choice) / `/Ob3`
(more aggressive than `/Ob2`); other speed knobs: `/Oi` (intrinsics), `/Ot` (favor speed),
`/Oy` (omit frame pointer, x86), `/favor:<blend|AMD64|INTEL64|ATOM>`. On GCC/Clang the frame
pointer is `-fomit-frame-pointer` / `-fno-omit-frame-pointer` (note: `-O1`+ omits it by default
on many targets; sanitizers/profilers want `-fno-omit-frame-pointer`).

`-Ofast` / `/fp:fast` / `-ffast-math` are **banned in this repo** (IEEE-stable telemetry).

## Architecture & tuning

| Intent | GCC / Clang | MSVC |
|--------|-------------|------|
| Target ISA (restricts) | `-march=<arch>` | `/arch:<SSE2\|AVX\|AVX2\|AVX512>` |
| Tune scheduling (no ISA restriction) | `-mtune=<cpu>` | -- |
| ISA on non-x86 (SPARC/ARM/...) | `-mcpu=<cpu>` | -- |
| x86-64 micro-levels | `-march=x86-64-v2` (SSE4.2) / `-v3` (AVX2) / `-v4` (AVX-512) | -- |
| SIMD opt-in | `-msse4.1 -msse4.2 -mavx -mavx2 -mavx512f` | folded into `/arch:` |

This repo pins `-march=x86-64-v2` and adds `-msse4.1` separately for DSP/FFT (non-MSVC).
**Never use `-march=native`** for a shipped binary. `-mtune` changes scheduling only and is
safe across the ISA; `-march` changes the instruction set and can SIGILL on older CPUs.

## Floating point / math

| Behavior | GCC / Clang | MSVC |
|----------|-------------|------|
| Deterministic, IEEE (repo default) | `-fno-fast-math -fno-unsafe-math-optimizations` | `/fp:precise` |
| Fused-multiply-add control | `-ffp-contract=<fast\|on\|off>` | `/fp:contract` |
| FP model | `-ffp-model=<...>` (Clang) | `/fp:<precise\|fast\|strict>` |
| **Fast/lossy -- banned here** | `-ffast-math`, `-funsafe-math-optimizations` | `/fp:fast` |
| errno from math fns | `-fmath-errno` / `-fno-math-errno` | -- |

`-ffp-contract`: `fast` fuses across statements (can change results), `on` only within a
statement, `off` never. The repo keeps `/fp:precise` and does not set a fast contract.

## Codegen, sections, interposition

| Intent | GCC / Clang | MSVC |
|--------|-------------|------|
| Per-function sections (enable dead-strip) | `-ffunction-sections` | `/Gy` |
| Per-data sections | `-fdata-sections` | `/Gw` |
| Dead-strip at link | `-Wl,--gc-sections` (GNU ld/lld), `-Wl,-dead_strip` (macOS) | `/OPT:REF` |
| Fold identical COMDATs | (LTO does this) | `/OPT:ICF` |
| Loop unrolling | `-funroll-loops` | (governed by `/O2`) |
| Vectorize | `-fvectorize` (loop), `-fslp-vectorize` (SLP); `-ftree-vectorize` alias | auto under `/O2`; `/Qpar` for auto-parallelize |
| Drop symbol interposition (faster PIC) | `-fno-semantic-interposition` | n/a |
| Avoid PLT indirection | `-fno-plt` | n/a |

`-fno-semantic-interposition` lets the optimizer assume a symbol won't be replaced at runtime,
enabling cross-call inlining in PIC -- this repo uses it on GCC and IntelLLVM. `/Gw` and `/Gy`
are the MSVC prerequisites for `/OPT:REF` to strip unused data/functions.

## Link-Time Optimization (LTO)

| Form | GCC / Clang | MSVC |
|------|-------------|------|
| Enable | `-flto` (Clang `-flto=full`) | `/GL` (compile) + `/LTCG` (link) |
| Parallel/scalable | `-flto=thin` (Clang), `-flto=auto` (GCC: pick job count) | `/LTCG` (use `/CGTHREADS`) |
| ThinLTO backend parallelism | `-flto-jobs=<n>` (Clang) | -- |
| Whole-program devirt | `-fwhole-program-vtables` (needs `-flto`) | (in `/GL`) |
| Fat objects (LTO + non-LTO) | `-ffat-lto-objects` | -- |
| ThinLTO incremental cache | `-Wl,--thinlto-cache-dir=<dir>` (lld) / `/lldltocache:<dir>` (lld-link) | -- |

Repo: `-flto=auto` on the GCC/Clang family, `/GL`+`/LTCG` on cl.exe, `/clang:-flto=thin`
+ `/lldltocache:` on clang-cl. **ThinLTO cache is an LLD linker flag, not a clang driver flag**
-- under clang-cl the native `/lldltocache:` is used because CMake invokes lld-link directly.

## Profile-Guided Optimization (PGO)

### Concept (compiler-neutral)
PGO (a.k.a. feedback-directed optimization / FDO) feeds the compiler a profile of the program's
actual runtime behavior so it optimizes from measured data, not just static heuristics. Phases:

1. **Instrument** -- build with counters inserted.
2. **Train** -- run the instrumented binary on a **representative** workload to emit profile data.
3. **Recompile** -- rebuild consuming the profile.

A good profile improves branch layout, inlining, hot/cold splitting, function/basic-block
ordering for I-cache locality, register allocation, and devirtualization. **The training
workload must be representative of real usage, or PGO can pessimize the binary.** In this repo
the training run is the hotpath benchmark (`--benchmark-hotpath`), which is exactly the path
that must be fast. Sampling-based PGO / AutoFDO (Google) is an alternative that reads hardware
perf counters off an already-optimized binary (lower overhead, no special build) instead of
inserting counters -- this repo uses the instrumentation flavor.

### Flags by compiler

| Compiler | Instrument (GENERATE) | Optimize (USE) | Data format |
|----------|----------------------|----------------|-------------|
| GCC | `-fprofile-generate` (+`-fprofile-dir=`) | `-fprofile-use -fprofile-correction` | `*.gcda` |
| Clang / AppleClang | `-fprofile-generate[=dir]` (compile **and** link) | `-fprofile-use[=path]` | `*.profraw` -> `llvm-profdata merge` -> `*.profdata` |
| clang-cl | `/clang:-fprofile-generate=<dir>` (compile only) | `/clang:-fprofile-use=<profdata>` (compile only) | same as Clang |
| MSVC cl.exe | `/GL` + link `/LTCG /GENPROFILE` | `/GL` + link `/LTCG /USEPROFILE` | `*.pgd` |

Clang also has `-fprofile-instr-generate` / `-fprofile-instr-use` (source-level instrumentation,
distinct from the IR-level `-fprofile-generate`), `-fcs-profile-generate` (context-sensitive,
a second pass), and sampling: `-fprofile-sample-use=` / `-fauto-profile=`. MSVC PGO chain:
`/GL` -> `/LTCG /GENPROFILE` -> train -> `/LTCG /USEPROFILE`; `/LTCG:PGINSTRUMENT` and
`/LTCG:PGOPTIMIZE` are the equivalent sub-option spellings.

## Security / hardening

| Protection | GCC / Clang | MSVC |
|------------|-------------|------|
| Stack canaries | `-fstack-protector-strong` (or `-all`) | `/GS` |
| Stack-clash | `-fstack-clash-protection` | -- |
| FORTIFY runtime checks | `-D_FORTIFY_SOURCE=2` or `=3` (needs `-O1`+) | (CRT-provided) |
| Control-flow integrity | `-fcf-protection=full` (x86 CET), `-fsanitize=cfi` (needs LTO) | `/guard:cf` + link `/GUARD:CF` |
| ARM pointer auth / BTI | `-mbranch-protection=standard` | -- |
| ASLR | `-fPIC` (+ PIE link) | link `/DYNAMICBASE` (+ `/HIGHENTROPYVA` for 64-bit) |
| DEP / non-exec stack | `-Wl,-z,noexecstack` | link `/NXCOMPAT` |
| Full RELRO | `-Wl,-z,relro -Wl,-z,now` | -- |
| Shadow stack compat | (`-fcf-protection`) | link `/CETCOMPAT` |
| Spectre v1 mitigation | -- | `/Qspectre` (cl.exe only; clang-cl rejects) |
| Extra security warnings | `-Wformat-security` | `/sdl` |
| Zero-init automatics | `-ftrivial-auto-var-init=zero` | (no direct equal) |
| Hidden default visibility | `-fvisibility=hidden` | (dllexport model) |

`_FORTIFY_SOURCE` needs optimization to work (no-op at `-O0`); this repo uses `=2` at the
directory level and upgrades to `=3` per-target on Linux. `-fsanitize=cfi` requires LTO and is
used only on aarch64-Linux/Clang here. AppleClang supports neither CFI nor CET.

## Sanitizers (diagnostic builds, not shipped)

| Tool | Flag | Notes |
|------|------|-------|
| AddressSanitizer | `-fsanitize=address` | use-after-free, overflows, leaks; ~2x slower, 2-3x memory. |
| UndefinedBehavior | `-fsanitize=undefined` | null deref, integer overflow; ~10-20% slower. |
| ThreadSanitizer | `-fsanitize=thread` | data races; **cannot coexist with ASan**; not MSVC. |
| MemorySanitizer | `-fsanitize=memory` | uninitialized reads (Clang; not used here). |
| Recover / trap control | `-fno-sanitize-recover=<...>`, `-fsanitize-trap=<...>` | |

Pair sanitizers with `-g -fno-omit-frame-pointer` for usable traces. MSVC has `/fsanitize=address`
(ASan) but no TSan. This repo: `DEBUG_SANITIZER` = ASan+UBSan, `ENABLE_TSAN` = a separate TSan
build (see `serial-studio-build-flags.md`).

## MSVC language-conformance flags worth knowing

`/permissive-` (standards-conformance mode), `/Zc:__cplusplus` (report the real `__cplusplus`
value -- off by default without it), `/Zc:preprocessor` (conforming preprocessor), `/std:c++20`,
`/MP` (parallel compile). This repo sets `/permissive- /Zc:__cplusplus /Zc:preprocessor /MP` on
cl.exe.

## clang-cl: the MSVC-ABI Clang driver

clang-cl accepts MSVC-style `/`-options (`/O2`, `/Od`, `/Ox`, `/arch:`) **and** forwards
Clang/GCC-style flags through the **`/clang:<arg>`** passthrough (e.g. `/clang:-flto=thin`,
`/clang:-march=x86-64-v2`, `/clang:-fprofile-generate=`). Critical for this repo: CMake links
the clang-cl target via **lld-link directly, not the clang-cl driver**, so driver-style LTO/PGO
flags must NOT appear on the link line -- only native lld-link flags (`/lldltocache:`,
`/OPT:REF`, `/OPT:ICF`). See the clang-cl gotcha in SKILL.md and the per-toolchain table in
`serial-studio-build-flags.md`.

## Sources & accuracy caveats

- **GCC**: SPEC CPU2017 GCC flags description, `gcc.2018-02-16.html`. This is one 2018
  submission's flag *subset*, not the full GCC manual -- it does **not** list `-O0/-Os/-Og`,
  the LTO family, `-ffunction-sections`/`--gc-sections`, `-fno-semantic-interposition`,
  `-fprofile-correction`/`-fprofile-dir=`, or `-fomit-frame-pointer`. Those families above come
  from standard GCC usage (and are present in this repo's modules), not from that page. For
  authoritative GCC flag text use the upstream GCC `Optimize-Options` manual.
- **Clang**: official Clang Command Line Reference. It documents driver flags but does **not**
  enumerate `-march=` values (so x86-64-v2/v3/v4 are valid but not listed there), does not
  describe the `/clang:` passthrough or MSVC `/O` spellings (those live in the Clang Users
  Manual "MSVC compatibility"), and the ThinLTO cache flag is an **LLD** option, not a clang
  driver flag.
- **MSVC**: "Compiler options listed by category" (cl.exe). The linker flags (`/LTCG`,
  `/GENPROFILE`, `/USEPROFILE`, `/OPT:REF`/`/OPT:ICF`, `/GUARD:CF`, `/DYNAMICBASE`, `/NXCOMPAT`,
  `/HIGHENTROPYVA`, `/CETCOMPAT`, `/INTEGRITYCHECK`) are on the **companion MSVC Linker options**
  page, not the compiler page. `/arch:` and `/Ob<n>` sub-values live on each flag's own topic
  page, not the category index.
- **PGO concept**: Wikipedia "Profile-guided optimization". It names the technique, the
  representativeness caveat, and sampling/AutoFDO, and lists GCC/Clang/MSVC as implementors, but
  does **not** list the specific `-fprofile-*` flags -- those come from the compiler docs above.
