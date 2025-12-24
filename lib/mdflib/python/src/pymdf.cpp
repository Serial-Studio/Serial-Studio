/*
* Copyright 2024 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include "pymdf.h"

#include <pybind11/pybind11.h>

using namespace pymdf;
using namespace pybind11;

PYBIND11_MODULE(pymdf, m) {
  InitSampleRecord(m);
}
