/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include <utility>

#include "mdf/fhcomment.h"
#include "mdf/mdflogstream.h"

#include "writexml.h"

namespace mdf {

FhComment::FhComment()
: MdComment("FH") {

}

FhComment::FhComment(std::string comment)
    : FhComment() {
  Comment(std::move(comment));
}

void FhComment::ToolId(MdString tool_id) {
  tool_id_ = std::move(tool_id);
}

const MdString& FhComment::ToolId() const {
  return tool_id_;
}

void FhComment::ToolVendor(MdString tool_vendor) {
  tool_vendor_ = std::move(tool_vendor);
}

const MdString& FhComment::ToolVendor() const {
  return tool_vendor_;
}

void FhComment::ToolVersion(MdString tool_version) {
  tool_version_ = std::move(tool_version);
}

const MdString& FhComment::ToolVersion() const {
  return tool_version_;
}

void FhComment::UserName(MdString user_name) {
  user_name_ = std::move(user_name);
}

const MdString& FhComment::UserName() const {
  return user_name_;
}

std::string FhComment::ToXml() const {
  try {
    auto xml_file = CreateXmlFile("FileWriter");
    constexpr bool ho_active = false;
    auto& root_node = CreateRootNode(*xml_file, ho_active);
    ToTx(root_node);
    if (tool_id_.IsActive()) {
      tool_id_.ToXml(root_node, "tool_id");
    }
    if (tool_vendor_.IsActive()) {
      tool_vendor_.ToXml(root_node, "tool_vendor");
    }
    if (tool_version_.IsActive()) {
      tool_version_.ToXml(root_node, "tool_version");
    }
    if (user_name_.IsActive()) {
      user_name_.ToXml(root_node, "user_name");
    }
    ToCommonProp(root_node);
    return xml_file->WriteString(true);
  } catch (const std::exception& err) {
    MDF_ERROR() << "Failed to create the EV comment. Error: " << err.what();
  }
  return {};
}

void FhComment::FromXml(const std::string& xml_snippet) {
  if (xml_snippet.empty()) {
    return;
  }
  try {
    auto xml_file = CreateXmlFile("Expat");
    if (!xml_file) {
      throw std::runtime_error("Failed to create EXPAT parser object");
    }
    const bool parse = xml_file->ParseString(xml_snippet);
    if (!parse) {
      throw std::runtime_error("Failed to parse the XML string.");
    }
    const auto* root_node = xml_file->RootNode();
    if (root_node == nullptr) {
      throw std::runtime_error("There is no root node in the XML string");
    }
    IXmlNode::ChildList node_list;
    xml_file->GetChildList(node_list);
    for (const IXmlNode* node : node_list) {
      if (node == nullptr) {
        continue;
      }
      if (node->IsTagName("tool_id")) {
        tool_id_.FromXml(*node);
      } else if (node->IsTagName("tool_vendor")) {
        tool_vendor_.FromXml(*node);
      } else if (node->IsTagName("tool_version")) {
        tool_version_.FromXml(*node);
      } else if (node->IsTagName("user_name")) {
        user_name_.FromXml(*node);
      }
    }
    MdComment::FromXml(*root_node);
  } catch (const std::exception& err) {
    MDF_ERROR() << "Failed to parse the EV comment. Error: " << err.what();
  }
}


}  // namespace mdf