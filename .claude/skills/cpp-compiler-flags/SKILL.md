---
name: cpp-compiler-flags
description: >-
  C++ compiler/linker flag guidance for Serial Studio's build (GCC, Clang, AppleClang, MSVC
  cl.exe, clang-cl, MinGW, IntelLLVM). Use when reading, changing, or reasoning about the
  cmake flag modules (Optimization/Hardening/Sanitizers/MiMalloc), tuning -O/-march/LTO/PGO,
  adding a per-toolchain branch, debugging a flag that one compiler rejects, or explaining
  what a flag does. Encodes this repo's actual flag layout and its non-negotiable invariants
  (IEEE-stable math, Lua unwind tables, x86-64-v2 baseline, the two-stage PGO flow). It does
  NOT build, configure, or run cmake -- the developer does that.
---

# Serial Studio -- C++ compiler flags

Serial Studio's release builds are aggressively optimized and hardened across seven
toolchains. The flag surface lives in **four cmake modules**, not scattered through
`CMakeLists.txt` or per-target calls, and those modules are the **single source of truth** --
read them before answering anything flag-related:

| Module | Owns |
|--------|------|
| `cmake/Optimization.cmake` | `-O3`/`/O2`, `-march`, SIMD, LTO, and the full PGO (GENERATE/USE) flow, per toolchain. |
| `cmake/Hardening.cmake` | Stack protectors, FORTIFY, CFI/CET, RELRO/DEP, ASLR, `serial_studio_harden()` per-target deltas. |
| `cmake/Sanitizers.cmake` | ASan+UBSan (`DEBUG_SANITIZER`) and the separate TSan build (`ENABLE_TSAN`). |
| `cmake/MiMalloc.cmake` | The process-wide mimalloc allocator override (Windows/MSVC + Linux; macOS opts out). |

Wiring: options are declared in `CMakeLists.txt` (around lines 79-87); the modules are
`include()`d there (MiMalloc -> Optimization -> Sanitizers, then Hardening). The PGO two-stage
flow is driven by `.github/workflows/ci.yml`.

## This skill does NOT build

Per CLAUDE.md, **the developer runs cmake, the compiler, and the benchmark themselves.** Never
invoke `cmake`/`jom`/`clang`/`cl`/the compiler to "test" a flag. Reason from the modules and
the references; propose the edit; hand off. Flag changes that touch the hotpath are gated by
`--benchmark-hotpath` (256 kHz) -- flag it for the developer to re-run; see [[ss-hotpath]].

## Build-option matrix (all default OFF except where noted)

| Option | Effect | Set by |
|--------|--------|--------|
| `PRODUCTION_OPTIMIZATION` | The aggressive per-toolchain release branch (`-O3`/`/O2`, `-march=x86-64-v2`, LTO). | Release CI; manual. |
| `ENABLE_PGO` + `PGO_STAGE` | Profile-guided opt; `GENERATE` then `USE`. Needs `PRODUCTION_OPTIMIZATION`. | CI two-pass; manual. |
| `ENABLE_HARDENING` | Defense-in-depth (canaries, FORTIFY, CFI, RELRO, CFG). | CI; **auto-on** for Release/RelWithDebInfo and sandboxed builds. |
| `DEBUG_SANITIZER` | ASan + UBSan (Debug only, GCC/Clang). | Manual. |
| `ENABLE_TSAN` | ThreadSanitizer (proves the lock-free hotpath); mutually exclusive with ASan and MSVC. | Manual / CI thread-safety job. |

`DISABLE_LTO` is internal: forced ON for Flatpak/sandboxed builds (which also auto-enable
hardening). LTO is otherwise ON whenever `PRODUCTION_OPTIMIZATION` is.

## Non-negotiable invariants (do not regress these)

These are deliberate choices baked into the modules. A flag change that breaks one is a bug,
not an optimization:

- **IEEE-stable math, always.** Every non-MSVC branch pins `-fno-fast-math
  -fno-unsafe-math-optimizations`; MSVC/clang-cl use `/fp:precise`. **Never add `-ffast-math`,
  `/fp:fast`, `-funsafe-math-optimizations`, `-Ofast`, or `-ffp-contract=fast`** -- telemetry
  output must be bit-stable across builds and platforms. This is the one rule most likely to be
  "helpfully" violated.
- **Unwind tables on every TU (non-MSVC).** `-fexceptions -funwind-tables
  -fasynchronous-unwind-tables`; on macOS the lua54 target additionally opts out of LTO with
  `-fno-lto -femit-dwarf-unwind=no-compact-unwind` (Xcode 26's ld drops fallback DWARF unwind
  under `-flto`, llvm#135888, and made `-Wl,-keep_dwarf_unwind` a no-op). Lua is compiled as
  C++ and throws across the VM stack; without unwind metadata, `--gc-sections`/`-dead_strip`
  + LTO can drop a personality routine and turn a routine Lua error into `std::terminate`. See
  [[ss-cpp-modern]] and the Lua exception-safety setup. Do not remove these to "shrink" the
  binary.
- **x86-64-v2 is the conservative baseline (SSE4.2, 2012+ CPUs).** Every x86-64 branch uses
  `-march=x86-64-v2` / `/clang:-march=x86-64-v2`. Do not bump to `v3`/`v4` or `-march=native`
  for shipped binaries -- it would SIGILL on older CPUs. ARM baselines: aarch64 -> `armv8-a`
  (+`-latomic`); armv7l -> `armv7-a -mfpu=neon -mfloat-abi=hard` (hardfloat pinned on purpose).
- **`-msse4.1` is added for DSP/FFT on non-MSVC x86-64**, separate from the `-march` baseline.
  MSVC/clang-cl are skipped (the x64 ABI guarantees SSE2; no explicit SSE4.1 there).
- **Hardening auto-enables for optimized configs.** A manual `Release` that forgets
  `-DENABLE_HARDENING=ON` still gets hardened (belt-and-suspenders in `Hardening.cmake`). Don't
  "simplify" that away.
- **Per-toolchain branches are exclusive and order-sensitive.** clang-cl reports `MSVC=ON`, so
  its branch must precede cl.exe's; MinGW-Clang precedes MinGW-GCC. Adding a branch in the
  wrong order silently routes a compiler to the wrong flags.

## The clang-cl / lld-link gotcha (Windows, MSVC ABI)

This trips up almost everyone, so it is stated once here and detailed in the reference:
CMake links this target with **`lld-link` directly, not through the `clang-cl` driver.** So:

- LTO/PGO **compile** flags use the `/clang:` passthrough (`/clang:-flto=thin`,
  `/clang:-fprofile-generate=`, `/clang:-fprofile-use=`) and go on the **compile step only**.
- The **link** step gets *native* lld-link flags only (`/lldltocache:`, `/OPT:REF`, `/OPT:ICF`)
  -- never driver-style `-flto=thin` / `-fprofile-*`, which lld-link rejects as bogus input.
- ThinLTO needs no link flag: bitcode `.obj`s are auto-detected; PGO instrumentation embeds an
  `/INCLUDE:` that pulls in `clang_rt.profile` automatically.
- clang-cl rejects `/Qspectre /sdl /ZH:SHA_256` (those are cl.exe-only); it takes `/GS
  /guard:cf` and the link-side `/GUARD:CF /DYNAMICBASE /CETCOMPAT`.

## When to use which reference

- **Changing this repo's build, or "why is this flag here / who sets it"** ->
  `references/serial-studio-build-flags.md`. The authoritative per-module, per-toolchain map of
  what Serial Studio actually passes, the PGO CI flow, mimalloc, and the rationale for each
  invariant.
- **"What does flag X do" / "what's the Clang equivalent of `/GL`" / picking a new flag** ->
  `references/flag-catalog.md`. A cross-compiler catalog (GCC / Clang / MSVC families + the PGO
  concept) distilled from the canonical upstream docs, with per-source accuracy caveats.

Always cross-check the catalog against the repo module before recommending a change: a flag
being valid is not a reason to add it here (see the IEEE-math invariant).

## Output expectations

Follow CLAUDE.md's handoff rules: one-line statement of intent before a non-trivial change,
a one/two-sentence summary when you stop, no new doc files unless asked. Propose targeted
`Edit`s to the cmake modules -- do not rewrite a whole module. If a change could shift hotpath
throughput, say so and point the developer at `--benchmark-hotpath` ([[ss-hotpath]]); you do
not run it. For style/structure on any C++ you touch alongside, defer to [[ss-verify]].

## References

- `references/serial-studio-build-flags.md` -- this repo's real flag layout and invariants.
- `references/flag-catalog.md` -- cross-compiler flag catalog from upstream GCC/Clang/MSVC docs
  + the PGO concept, with citations and caveats.
