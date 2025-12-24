/*
* Copyright 2024 Ingemar Hedvall
* SPDX-License-Identifier: MIT
*/

#pragma once

#include <gtest/gtest.h>

namespace mdf::test {

class TestChannelArray : public testing::Test {
public:
 static void SetUpTestSuite();
 static void TearDownTestSuite();

};

}  // namespace mdf::test