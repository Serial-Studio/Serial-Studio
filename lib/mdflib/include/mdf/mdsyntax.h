/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once

#include <string>
#include <map>
#include "mdf/mdstring.h"

namespace mdf {
class MdSyntax;

using MdSyntaxList = std::map<std::string, MdSyntax>;

class MdSyntax : public MdString {
 public:
  void Syntax(std::string syntax);
  [[nodiscard]] const std::string& Syntax() const;

  void Version(std::string version);
  [[nodiscard]] const std::string& Version() const;

  void Source(std::string source);
  [[nodiscard]] const std::string& Source() const;

  void ToXml(IXmlNode& root_node, const std::string& tag_name) const override;
  void FromXml(const IXmlNode& syntax_node) override;
 private:
  std::string version_ = "1.0";
  std::string source_;
};

}  // namespace mdf

