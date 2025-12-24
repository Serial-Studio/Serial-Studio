/*
* Copyright 2024 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */
#pragma once
#include <pybind11/pybind11.h>

namespace pymdf {
void InitSampleRecord(pybind11::module& m);
} // pymdf
