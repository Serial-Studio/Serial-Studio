# Serial Studio build flags (the real layout)

The authoritative map of what this repo actually passes to each toolchain, and *why*. The four
cmake modules are the source of truth -- if this doc and a module disagree, the module wins
(and this doc needs updating). All flag spellings below are copied from the modules as they
stand; verify against the file before acting, because the build moves fast (recent commits
churned the clang-cl ThinLTO/PGO branch).

Files: `cmake/Optimization.cmake`, `cmake/Hardening.cmake`, `cmake/Sanitizers.cmake`,
`cmake/MiMalloc.cmake`. Options + includes: `CMakeLists.txt`. PGO driver: `.github/workflows/ci.yml`.

## Toolchains in play

GCC (Linux), Clang (Linux), AppleClang (macOS), MSVC cl.exe (Windows), clang-cl (Windows,
MSVC ABI), Clang/MinGW and GCC/MinGW (Windows), IntelLLVM. The production branch is a single
`if(PRODUCTION_OPTIMIZATION)` with one `elseif` per toolchain -- **exactly one runs**. Branch
order matters: clang-cl reports `MSVC=ON` so it is matched *before* cl.exe; Clang/MinGW before
GCC/MinGW.

## Optimization.cmake

### Always-on (every non-MSVC TU, when PRODUCTION_OPTIMIZATION)
```
-fexceptions -funwind-tables -fasynchronous-unwind-tables      # Lua throws across the VM; see below
# macOS link: -Wl,-keep_dwarf_unwind
add_compile_definitions(NDEBUG)
```

### Per-toolchain production flags

**GCC/Clang (Linux), AppleClang (macOS), Clang/GCC MinGW, IntelLLVM** (the "-O3 family"):
```
-O3 -funroll-loops -fomit-frame-pointer
-fno-fast-math -fno-unsafe-math-optimizations            # IEEE-stable, non-negotiable
-ffunction-sections -fdata-sections                       # paired with --gc-sections / -dead_strip
-flto=auto                                                # unless DISABLE_LTO
-fno-semantic-interposition                               # GCC and IntelLLVM only
# link: -Wl,--gc-sections   (macOS: -Wl,-dead_strip)   + -flto=auto unless DISABLE_LTO
```
Arch baseline appended by platform: x86-64 -> `-march=x86-64-v2`; aarch64 -> `-march=armv8-a`
+ link `-latomic`; armv7l -> `-march=armv7-a -mfpu=neon -mfloat-abi=hard` + `-latomic`.
IntelLLVM also adds `-static`.

**MSVC cl.exe:**
```
/permissive- /Zc:__cplusplus /Zc:preprocessor /MP
/O2 /Ot /Oi /Ob3 /fp:precise /Gw /Gy /DNDEBUG
/GL                                                       # compile, unless DISABLE_LTO
# link: /OPT:REF /OPT:ICF   + /LTCG unless DISABLE_LTO
```

**clang-cl (Windows, MSVC ABI):**
```
/O2 /Oi /Ot /Gy /Gw /clang:-march=x86-64-v2 /fp:precise /DNDEBUG
/clang:-flto=thin                                         # compile only, unless DISABLE_LTO
# link (lld-link, native flags only): /OPT:REF /OPT:ICF
#                                     /lldltocache:${CMAKE_BINARY_DIR}/lto.cache  unless DISABLE_LTO
```

Both Windows branches first strip CMake's injected `/Ob2` (from `*_RELEASE` flags) and `/W3`
(base flags) via `string(REGEX REPLACE ...)` so the repo's `/Ob3` and per-target `/W4` win
without a flood of D9025 "overriding" warnings. No `/GL`/`/LTCG` on clang-cl (those are
cl.exe-only).

### SIMD (separate from -march)
```
-msse4.1     # non-MSVC x86-64 only, for vectorized DSP/FFT. MSVC/clang-cl skipped (x64 ABI = SSE2 baseline).
```

### PGO (ENABLE_PGO, staged by PGO_STAGE)
Profile data dir: `PGO_PROFILE_DIR` (default `${CMAKE_BINARY_DIR}/pgo-profiles`). Needs
`PRODUCTION_OPTIMIZATION` for meaningful results (warns otherwise). Defines
`SS_PGO_INSTRUMENT` in the GENERATE stage.

| Toolchain | GENERATE | USE | Data |
|-----------|----------|-----|------|
| MSVC cl.exe | `/GL` + link `/LTCG /GENPROFILE /PGD:.../profile.pgd` | `/GL` + link `/LTCG /USEPROFILE:PGD=...` | `profile.pgd` |
| clang-cl | `/clang:-fprofile-generate=<dir>` (compile only) | `/clang:-fprofile-use=<merged.profdata>` (compile only) | `*.profraw` -> `merged.profdata` |
| Clang/AppleClang | `-fprofile-generate=<dir>` on **compile AND link** | `-fprofile-use=<merged.profdata>` on **compile AND link** | `*.profraw` -> `merged.profdata` (via `llvm-profdata merge`) |
| GCC | `-fprofile-generate -fprofile-dir=<dir>` (compile+link) | `-fprofile-use -fprofile-dir=<dir> -fprofile-correction` (compile+link) | `*.gcda` |

Why the link-side differs: Clang/AppleClang link through the clang **driver**, so `-fprofile-*`
must be on both compile and link (the driver links `clang_rt.profile` and feeds the profile into
LTO). clang-cl links via **lld-link directly**, so a `-fprofile-*` link flag would be rejected
as an unknown argument -- the instrumented objects embed an `/INCLUDE:` that pulls in
`clang_rt.profile`, and `-fprofile-use` shapes codegen entirely at compile time, so the link
step needs no PGO flag. The USE stage validates that the expected profile artifact exists
(`profile.pgd` / `*.profraw` / `*.gcda`) and `FATAL_ERROR`s if not; Clang/AppleClang also runs
`llvm-profdata merge` (requires `llvm-profdata` on PATH -- CI passes
`-DLLVM_PROFDATA=$(xcrun -f llvm-profdata)` on macOS).

### The CI two-pass (`ci.yml`)
For each release platform: configure `PGO_STAGE=GENERATE` -> build instrumented -> run the
training workload (`--headless --benchmark-hotpath --min-fps 1`, a representative hotpath run)
-> reconfigure `PGO_STAGE=USE` -> rebuild optimized -> **gate** (`--benchmark-hotpath --min-fps
256000`). The training run *is* the representative workload that makes PGO help instead of hurt;
the 256 kHz gate is a hard release gate on the shipped PGO binary. See [[ss-hotpath]].

### DISABLE_LTO / sandboxed builds
`PRODUCTION_OPTIMIZATION` + Flatpak (`FLATPAK_ID` env or `FLATPAK_BUILD`) -> `DISABLE_LTO=ON`
and `ENABLE_HARDENING` forced ON. LTO is otherwise ON.

## Hardening.cmake

`ENABLE_HARDENING` defaults OFF (fast, debuggable dev builds). It **auto-forces ON** for
`Release`/`RelWithDebInfo` (`HARDEN_AUTO_ON_RELEASE`, default ON) and for sandboxed builds, so
a manual optimized build can't accidentally ship unhardened.

### Directory-level block (when ENABLE_HARDENING)

**MSVC / clang-cl:**
```
compile: /GS /guard:cf
link:    /GUARD:CF /DYNAMICBASE /CETCOMPAT
```

**GCC/Clang/AppleClang:**
```
-fstack-protector-strong -Wformat -Wformat-security -Werror=format-security
-fstack-clash-protection                      # GNU/Clang, non-Apple
-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2         # needs -O1+; warns + skips on unoptimized Debug
-mbranch-protection=standard                  # ARM-only (aarch64/arm64), not when x86 also present, NOT Apple
-fcf-protection=full                          # x86-64 Linux (Intel CET)
-fsanitize=cfi -fvisibility=hidden (+link -fsanitize=cfi)   # aarch64 Linux, Clang, requires LTO
-fPIC -fno-plt + link -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack -Wl,-z,separate-code  # Linux
-fPIC                                          # macOS (no RELRO/CFI/CET there)
```
AppleClang supports neither `-fsanitize=cfi` nor `-fcf-protection` -- both are gated off Apple.

### Per-target deltas: `serial_studio_harden(<target>)`
PRIVATE (so `lib/` static libs built with `/w`/`-w` are untouched), no-op unless
`ENABLE_HARDENING`. Layered on top of the directory block:
- **MSVC cl.exe (not clang-cl):** `/Qspectre /sdl /ZH:SHA_256` (clang-cl rejects these);
  link `/HIGHENTROPYVA /NXCOMPAT` (+ `/INTEGRITYCHECK` if `ENABLE_INTEGRITY_CHECK` -- requires a
  signed binary). clang-cl gets the link-side `/HIGHENTROPYVA /NXCOMPAT` too, but not the three
  compile flags.
- **macOS:** `-fstack-protector-strong`, `_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_FAST`,
  `-ftrivial-auto-var-init=zero` (via `_ss_add_auto_var_init`).
- **Linux:** `_FORTIFY_SOURCE=3` (upgraded from the directory block's `=2`),
  `_GLIBCXX_ASSERTIONS`, `-ftrivial-auto-var-init=zero`.

`_ss_add_auto_var_init` only fires on GCC 12+ / Clang 16+ / AppleClang 15+ (zero-inits
automatic vars -- kills a class of uninitialized-read bugs).

## Sanitizers.cmake

`DEBUG_SANITIZER` (Debug, GCC/Clang only -- not MSVC):
```
compile: -fsanitize=address -fsanitize=undefined -g -fno-omit-frame-pointer
link:    -fsanitize=address -fsanitize=undefined
```
`ENABLE_TSAN` (its own build -- cannot coexist with ASan, `FATAL_ERROR`s if both set or on MSVC):
```
compile: -fsanitize=thread -g -fno-omit-frame-pointer -O1
link:    -fsanitize=thread
```
TSan exists to prove the lock-free hotpath (driver thread -> main via SPSC `CircularBuffer`,
`Qt::DirectConnection` across threads) holds under load -- it is the only tool that can. See
[[ss-hotpath]] for what it's guarding.

## MiMalloc.cmake

Process-wide allocator override on Windows/MSVC (incl. clang-cl) and Linux; **macOS opts out**
(good system allocator, fragile DYLD interposing). FetchContent-builds mimalloc v2.1.9 with
`MI_OVERRIDE`/`MI_BUILD_SHARED`. Rationale: the frame-parse hotpath allocates many small
QString/QByteArray/Lua buffers per frame; the system heap (MSVC CRT / glibc) is slower, and
glibc thrashes per-thread arenas under the main-allocates / worker-frees pattern. clang-cl
builds mimalloc as C++ (`MI_USE_CXX`) to dodge a C-atomics miscompile in `segment-map.c`.
Included **before** Optimization.cmake so it isn't swept into LTO/PGO. Call
`target_link_mimalloc(<target>)`:
- Windows/MSVC: link `mimalloc` + `/INCLUDE:mi_version`, copy `mimalloc.dll` +
  `mimalloc-redirect.dll` (patches malloc process-wide at load, Qt DLLs included).
- Linux: `-Wl,--no-as-needed mimalloc` (force the interpose).

## Why the invariants exist (one line each)

- **No fast-math** -> bit-stable telemetry output across builds/platforms. A reproducible
  number is a feature; a faster wrong-in-the-last-bit number is a regression.
- **Unwind tables everywhere** -> Lua (compiled as C++) throws across the VM stack on any
  runtime error; without unwind metadata, `--gc-sections`/`-dead_strip` + LTO can strip a
  personality routine and a routine Lua type error becomes `std::terminate`.
- **No pac-ret on Apple ARM** -> `-mbranch-protection` signs LR with PACIASP; macOS compact
  unwind can't encode RA signing (arm64e-only), so every function falls back to DWARF and
  the system unwinder fails to authenticate the signed return addresses during a throw --
  the same Lua error then aborts even though all unwind metadata is present and valid.
- **x86-64-v2 baseline** -> runs on 2012+ CPUs; `v3`/`v4`/`native` would SIGILL on older
  hardware that the shipped binary must support.
- **Hardening auto-on for optimized configs** -> can't accidentally ship an unhardened release.
- **mimalloc before Optimization.cmake** -> keep the third-party allocator out of the app's
  LTO/PGO unit.
