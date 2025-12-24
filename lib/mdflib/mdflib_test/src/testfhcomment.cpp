/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */
#include <string_view>
#include <string>

#include <gtest/gtest.h>

#include "mdf/fhcomment.h"

namespace mdf::test {

TEST(TestFhComment, Properties) {
  FhComment comment;

  constexpr std::string_view kTx = "FH Test Properties";
  comment.Comment(MdString(kTx));

  constexpr std::string_view kToolId = "ACME MDF";
  comment.ToolId(MdString(kToolId));
  EXPECT_EQ(comment.ToolId(), kToolId);

  constexpr std::string_view kToolVendor = "ACME Corporation";
  comment.ToolVendor(MdString(kToolVendor));
  EXPECT_EQ(comment.ToolVendor(), kToolVendor);

  constexpr std::string_view kToolVersion = "V12.45";
  comment.ToolVersion(MdString(kToolVersion));
  EXPECT_EQ(comment.ToolVersion(), kToolVersion);

  constexpr std::string_view kUserName = "Coyote";
  comment.UserName(MdString(kUserName));
  EXPECT_EQ(comment.UserName(), kUserName);

  const std::string xml_snippet = comment.ToXml();
  std::cout << xml_snippet << std::endl;

  FhComment comment1;
  comment1.FromXml(xml_snippet);
  EXPECT_EQ(comment1.Comment(), kTx);

  EXPECT_EQ(comment1.ToolId(), kToolId);
  EXPECT_EQ(comment1.ToolVendor(), kToolVendor);
  EXPECT_EQ(comment1.ToolVersion(), kToolVersion);
  EXPECT_EQ(comment1.UserName(), kUserName);
}
} // end namespace