/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */
#include <utility>

#include "mdf/hounit.h"

#include "ixmlnode.h"

namespace mdf {

void HoUnit::Identity(std::string identity) {
  identity_ = std::move(identity);
  if (short_name_.empty()) {
    short_name_ = identity_;
  }
}

const std::string& HoUnit::Identity() const {
  return identity_;
}

void HoUnit::DisplayName(std::string display_name) {
  display_name_ = std::move(display_name);
}

const std::string& HoUnit::DisplayName() const {
  return display_name_;
}

void HoUnit::Factor(double factor) {
  factor_ = factor;
}

double HoUnit::Factor() const {
  return factor_;
}

void HoUnit::Offset(double offset) {
  offset_ = offset;
}

double HoUnit::Offset() const {
  return offset_;
}

void HoUnit::PhysicalDimensionRef(std::string phys_dim_ref) {
  phys_dim_ref_ = std::move(phys_dim_ref);
}

const std::string& HoUnit::PhysicalDimensionRef() const {
  return phys_dim_ref_;
}
void HoUnit::ToXml(IXmlNode& units_node) const {
  auto& node = units_node.AddNode("ho:UNIT");
  node.SetAttribute("ID", identity_);

  if (HoNameDetails::IsActive()) {
    HoNameDetails::ToXml(node);
  }

  if (!display_name_.empty()) {
    node.SetProperty("ho:DISPLAY-NAME", display_name_);
  }
  if (factor_ != 1.0 || offset_ != 0.0) {
    node.SetProperty("ho:FACTOR-SI-TO-UNIT", factor_);
    node.SetProperty("ho:OFFSET-SI-TO-UNIT", offset_);
  }
  if (!phys_dim_ref_.empty()) {
    node.SetProperty("ho:PHYSICAL-DIMENSION-REF", phys_dim_ref_);
  }

}
void HoUnit::FromXml(const IXmlNode& unit_node) {
  identity_ = unit_node.Attribute<std::string>("ID", "");

  IXmlNode::ChildList node_list;
  unit_node.GetChildList(node_list);
  for (const IXmlNode* node : node_list) {
    if (node == nullptr) {
      continue;
    }
    if (node->IsTagName("NAME-DETAILS")) {
      HoNameDetails::FromXml(*node);
    } else if (node->IsTagName("DISPLAY-NAME")) {
      display_name_ = node->Value<std::string>();
    } else if (node->IsTagName("FACTOR-SI-TO-UNIT")) {
      factor_ = node->Value<double>();
    } else if (node->IsTagName("OFFSET-SI-TO-UNIT")) {
      offset_ = node->Value<double>();
    } else if (node->IsTagName("PHYSICAL-DIMENSION-REF")) {
       if (phys_dim_ref_ = node->Attribute<std::string>("ID-REF");
           phys_dim_ref_.empty()) {
         phys_dim_ref_ = node->Value<std::string>();
       }
    }
  }

}



}  // namespace mdf