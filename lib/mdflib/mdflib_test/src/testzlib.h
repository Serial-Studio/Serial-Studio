/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <gtest/gtest.h>
namespace mdf::test {

class TestZlib : public ::testing::Test {
 protected:
  static void SetUpTestCase();
  static void TearDownTestCase();
};

}  // namespace mdf::test
