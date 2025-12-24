/*
* Copyright 2023 Ingemar Hedvall
* SPDX-License-Identifier: MIT
*/
#include <gtest/gtest.h>
#include "mdf/mdfhelper.h"

namespace mdf::test {
TEST(TestMdfHelper, TextToByteArray)  // NOLINT
{
  constexpr std::string_view input_text = "ABCDEFG";
  const auto byte_array = MdfHelper::TextToByteArray(input_text.data());
  EXPECT_EQ(byte_array.size(),input_text.size());
  for (size_t index = 0; index < byte_array.size(); ++index) {
    EXPECT_TRUE(input_text[index] ==
                static_cast<const char>(byte_array[index]))
        << input_text[index] << " " << static_cast<const char>(byte_array[index]);
  }
}

}