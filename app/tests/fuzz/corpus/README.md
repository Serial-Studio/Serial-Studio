<!--
SPDX-FileCopyrightText: 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
-->

# Fuzz corpora

One directory per fuzz target, named exactly after it: `corpus/fuzz_<subject>/`. Every file in a
directory is one input, replayed through that target's `LLVMFuzzerTestOneInput` by `ctest`. See
[../README.md](../README.md).
