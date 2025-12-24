/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once
#include <string>
#include <string_view>
#include "mdf/mdcomment.h"
#include "mdf/mdstring.h"

namespace mdf {

class FhComment : public MdComment {
 public:
  FhComment();
  explicit FhComment(std::string comment);

  void ToolId(const std::string_view& tool_id) { ToolId(MdString(tool_id)); }
  void ToolId(MdString tool_id);
  [[nodiscard]] const MdString& ToolId() const;

  void ToolVendor(const std::string_view& tool_vendor) { ToolVendor(MdString(tool_vendor)); }
  void ToolVendor(MdString tool_vendor);
  [[nodiscard]] const MdString& ToolVendor() const;

  void ToolVersion(const std::string_view& tool_version) { ToolVersion(MdString(tool_version)); }
  void ToolVersion(MdString tool_version);
  [[nodiscard]] const MdString& ToolVersion() const;

  void UserName(const std::string_view& user_name) { UserName(MdString(user_name)); }
  void UserName(MdString user_name);
  [[nodiscard]] const MdString& UserName() const;

  [[nodiscard]] std::string ToXml() const override;
  void FromXml(const std::string& xml_snippet) override;
 private:
  MdString tool_id_;
  MdString tool_vendor_;
  MdString tool_version_;
  MdString user_name_;
};

}  // namespace mdf


