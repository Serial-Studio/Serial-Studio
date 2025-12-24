/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */
#include <utility>

#include "mdf/cccomment.h"
#include "mdf/mdflogstream.h"

#include "writexml.h"

namespace mdf {

CcComment::CcComment()
: MdComment("CC") {
}

CcComment::CcComment(std::string comment)
    : MdComment("CC") {
  Comment(MdString(std::move(comment)));
}

const HoCompuMethod& CcComment::CompuMethod() const {
  return method_;
}

HoCompuMethod& CcComment::CompuMethod() {
  return method_;
}

std::string CcComment::ToXml() const {
  try {
    auto xml_file = CreateXmlFile("FileWriter");
    const bool ho_active = method_.IsActive();
    auto& root_node = CreateRootNode(*xml_file, ho_active);
    ToTx(root_node);
    ToNames(root_node);
    ToFormula(root_node);

    if (ho_active) {
      method_.ToXml(root_node);
    }

    ToCommonProp(root_node);
    return xml_file->WriteString(true);
  } catch (const std::exception& err) {
    MDF_ERROR() << "Failed to create the HD comment. Error: " << err.what();
  }
  return {};
}

void CcComment::FromXml(const std::string& xml_snippet) {
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
    MdComment::FromXml(*root_node);

    IXmlNode::ChildList node_list;
    xml_file->GetChildList(node_list);
    for (const IXmlNode* node : node_list) {
      if (node == nullptr) {
        continue;
      }
      if (node->IsTagName("COMPU-METHOD")) {
        method_.FromXml(*node);
      }
    }
  } catch (const std::exception& err) {
    MDF_ERROR() << "Failed to parse the CC comment. Error: " << err.what();
  }
}


}  // namespace mdf