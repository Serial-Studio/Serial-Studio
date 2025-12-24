/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
*/

#pragma once
#include <gtest/gtest.h>

#include "mdf/itimestamp.h"
#include "mdf/mdffactory.h"

namespace mdf::test {

class TestTimestamp : public testing::Test {
 public:
  static void SetUpTestSuite();
  static void TearDownTestSuite();
};

}  // namespace mdf::test


