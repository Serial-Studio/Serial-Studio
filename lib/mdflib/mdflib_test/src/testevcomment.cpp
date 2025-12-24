/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */
#include <string_view>
#include <string>

#include <gtest/gtest.h>

#include "mdf/evcomment.h"

namespace mdf::test {

TEST(TestEvComment, Properties) {
  EvComment comment;

  constexpr std::string_view kTx = "EV Test Properties";
  comment.Comment(MdString(kTx));

  constexpr double kPreTrigInterval = 11.23;
  comment.PreTriggerInterval(MdNumber(kPreTrigInterval));
  EXPECT_DOUBLE_EQ(comment.PreTriggerInterval(), kPreTrigInterval);

  constexpr double kPostTrigInterval = 12.23;
  comment.PostTriggerInterval(MdNumber(kPostTrigInterval));
  EXPECT_DOUBLE_EQ(comment.PostTriggerInterval(), kPostTrigInterval);

  constexpr double kTimeout = 100.456;
  MdNumber timeout;
  timeout.Triggered(true);
  timeout.Number(kTimeout);
  comment.Timeout(timeout);

  EXPECT_DOUBLE_EQ(comment.Timeout(), kTimeout);


  const std::string xml_snippet = comment.ToXml();
  std::cout << xml_snippet << std::endl;

  EvComment comment1;
  comment1.FromXml(xml_snippet);
  EXPECT_EQ(comment1.Comment(), kTx);

  EXPECT_DOUBLE_EQ(comment1.PreTriggerInterval(), kPreTrigInterval);
  EXPECT_DOUBLE_EQ(comment1.PostTriggerInterval(), kPostTrigInterval);
  EXPECT_DOUBLE_EQ(comment1.Timeout(), kTimeout);
  EXPECT_DOUBLE_EQ(comment1.Timeout().Triggered(), true);
}

} // end namespace