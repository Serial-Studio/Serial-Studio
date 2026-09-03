<!--
SPDX-FileCopyrightText: 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
-->

# Fuzz targets

Each target is one `.cpp` next to this file defining

```cpp
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size);
```

and is registered from `app/tests/CMakeLists.txt`:

```cmake
ss_add_fuzz_target(fuzz_<subject>
  SOURCES fuzz/fuzz_<subject>.cpp
          ${SS_APP_SRC}/<the production TUs it parses with>
  LIBS    Qt6::Core
)
```

Seeds live in `corpus/fuzz_<subject>/`, one file per input. The directory must exist and hold at
least one seed, and every crash a target has ever found belongs in it: the corpus is what turns a
fuzz target into a regression test.

`ENABLE_FUZZERS` is OFF in every default configure. With it off the helper builds the same entry
point behind `CorpusReplayMain.cpp`, a `QTest` that replays every seed, so the target compiles and
the corpus runs under `ctest` on toolchains that have no libFuzzer. With it on (Clang only, and
composable with `DEBUG_SANITIZER`) the helper builds a real libFuzzer binary, and `ctest` replays
the corpus through it with `-runs=0`.

```bash
# corpus tier, any toolchain
ctest --test-dir build/unit-ci -R '^fuzz_' --output-on-failure

# real fuzzing
cmake -B build/fuzz -G Ninja -DSS_BUILD_TESTS=ON -DENABLE_FUZZERS=ON -DDEBUG_SANITIZER=ON \
  -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Debug
cmake --build build/fuzz --target ss_unit_tests
./build/fuzz/app/tests/fuzz_<subject> app/tests/fuzz/corpus/fuzz_<subject>
```
