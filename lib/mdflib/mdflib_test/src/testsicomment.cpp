/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */
#include <string_view>
#include <string>

#include <gtest/gtest.h>

#include "mdf/sicomment.h"

namespace mdf::test {

TEST(TestSiComment, Properties) {
  SiComment comment;

  constexpr std::string_view kTx = "SI Test Properties";
  comment.Comment(MdString(kTx));


  MdAlternativeName& path = comment.Path();
  constexpr std::string_view kPath = "ACME Street";
  path.AddAlternativeName(MdString(kPath));
  EXPECT_EQ(path.AlternativeNames().size(), 1);

  MdAlternativeName& bus = comment.Bus();
  constexpr std::string_view kBus = "ACME Bus";
  bus.AddAlternativeName(MdString(kBus));
  EXPECT_EQ(bus.AlternativeNames().size(), 1);

  constexpr std::string_view kProtocol = "ACME Protocol";
  comment.Protocol(MdString(kProtocol));
  EXPECT_EQ(comment.Protocol(), kProtocol);

  const std::string xml_snippet = comment.ToXml();
  std::cout << xml_snippet << std::endl;

  SiComment comment1;
  comment1.FromXml(xml_snippet);
  EXPECT_EQ(comment1.Comment(), kTx);

  EXPECT_EQ(comment1.Path().AlternativeNames().size(), 1);
  EXPECT_EQ(comment1.Bus().AlternativeNames().size(), 1);
  EXPECT_EQ(comment1.Protocol(), kProtocol);
}

} // end namespace mdf::test
