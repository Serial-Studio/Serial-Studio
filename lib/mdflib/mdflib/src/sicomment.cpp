/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include <utility>

#include "mdf/sicomment.h"
#include "mdf/mdflogstream.h"

#include "writexml.h"


namespace mdf {

SiComment::SiComment()
: MdComment("SI") {

}

SiComment::SiComment(std::string comment)
    : SiComment() {
  Comment(std::move(comment));
}

const MdAlternativeName& SiComment::Path() const {
  return path_;
}

MdAlternativeName& SiComment::Path() {
  return path_;
}

const MdAlternativeName& SiComment::Bus() const {
  return bus_;
}

MdAlternativeName& SiComment::Bus() {
  return bus_;
}

void SiComment::Protocol(MdString protocol) {
  protocol_ = std::move(protocol);
}

const MdString& SiComment::Protocol() const {
  return protocol_;
}

std::string SiComment::ToXml() const {
  try {
    auto xml_file = CreateXmlFile("FileWriter");
    constexpr bool ho_active = false;
    auto& root_node = CreateRootNode(*xml_file, ho_active);
    ToTx(root_node);
    ToNames(root_node);
    if (path_.IsActive()) {
      auto& path_node = root_node.AddNode("path");
      path_.ToXml(path_node);
    }
    if (bus_.IsActive()) {
      auto& bus_node = root_node.AddNode("bus");
      bus_.ToXml(bus_node);
    }
    if (protocol_.IsActive()) {
      protocol_.ToXml(root_node, "protocol");
    }
    ToCommonProp(root_node);
    return xml_file->WriteString(true);
  } catch (const std::exception& err) {
    MDF_ERROR() << "Failed to create the SI comment. Error: " << err.what();
  }
  return {};
}

void SiComment::FromXml(const std::string& xml_snippet) {
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
      if (node->IsTagName("path")) {
        path_.FromXml(*node);
      } else if (node->IsTagName("bus")) {
        bus_.FromXml(*node);
      } else if (node->IsTagName("protocol")) {
        protocol_.FromXml(*node);
      }
    }
    MdComment::FromXml(*root_node);
  } catch (const std::exception& err) {
    MDF_ERROR() << "Failed to parse the SI comment. Error: " << err.what();
  }
}


}  // namespace mdf