/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */
#include <utility>

#include "mdf/ccunit.h"
#include "mdf/mdflogstream.h"

#include "writexml.h"

namespace mdf {
CcUnit::CcUnit()
: MdComment("CC") {
}

CcUnit::CcUnit(std::string unit)
: MdComment("CC") {
  Comment(MdString(std::move(unit)));
}

void CcUnit::Unit(MdString unit) {
  Comment(std::move(unit));
}

const MdString& CcUnit::Unit() const {
  return Comment();
}

void CcUnit::UnitRef(MdString unit_ref) {
  unit_ref_ = std::move(unit_ref);
}

const MdString& CcUnit::UnitRef() const {
  return unit_ref_;
}

IXmlNode& CcUnit::CreateRootNode(IXmlFile& xml_file,
                                    bool init_ho_namespace) const {
  std::ostringstream root_name;
  root_name << block_name_ << "unit";
  IXmlNode& root_node = xml_file.RootName(root_name.str());
  if (init_ho_namespace) {
    root_node.SetAttribute("xmlns:ho","http://www.asam.net/xml" );
  }
  return root_node;
}

std::string CcUnit::ToXml() const {
  try {
    auto xml_file = CreateXmlFile("FileWriter");
    constexpr bool ho_active = false;
    auto& root_node = CreateRootNode(*xml_file, ho_active);
    MdComment::ToXml(root_node);
    if (unit_ref_.IsActive()) {
      auto& unit_ref_node = root_node.AddNode("ho_unit");
      unit_ref_.MdStandardAttribute::ToXml(unit_ref_node);
      unit_ref_node.SetAttribute("unit_ref", unit_ref_.Text());
    }
    return xml_file->WriteString(true);
  } catch (const std::exception& err) {
    MDF_ERROR() << "Failed to create the CC unit. Error: " << err.what();
  }
  return {};
}

void CcUnit::FromXml(const std::string& xml_snippet) {
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
      if (node->IsTagName("ho_unit")) {
        unit_ref_.MdStandardAttribute::FromXml(*node);
        if (unit_ref_.Text(node->Attribute<std::string>("unit_ref", ""));
            unit_ref_.Text().empty() ) {
          unit_ref_.Text(node->Value<std::string>());
        }
      }
    }
    MdComment::FromXml(*root_node);
  } catch (const std::exception& err) {
    MDF_ERROR() << "Failed to parse the SI comment. Error: " << err.what();
  }
}


}