/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <gtest/gtest.h>

//#include "mdf/itimestamp.h"
#include "mdf/mdffactory.h"

namespace mdf::test {

class TestWrite : public testing::Test {
 public:
  static void SetUpTestSuite();
  static void TearDownTestSuite();

 protected:
};
/*

void CreateMdfWithTime(const std::string& filepath, MdfWriterType writerType,
                       ITimestamp& timestamp);

void TestMdf3Time(const std::string& filepath, uint64_t time);

void TestMdf4Time(const std::string& filepath, uint64_t time,
                  uint16_t tz_offset_min, uint16_t dst_offset_min);
*/
}  // namespace mdf::test
