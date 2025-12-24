/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */
#include <utility>

#include "mdf/hounitgroup.h"

#include "ixmlnode.h"

namespace mdf {

void HoUnitGroup::Category(HoUnitGroupCategory category) {
  category_ = category;
}

HoUnitGroupCategory HoUnitGroup::Category() const {
  return category_;
}

void HoUnitGroup::AddUnitReference(std::string unit_ref) {
  unit_ref_list_.emplace(std::move(unit_ref));
}

const HoUnitRefList& HoUnitGroup::UnitRefs() const {
  return unit_ref_list_;
}

HoUnitRefList& HoUnitGroup::UnitRefs() {
  return unit_ref_list_;
}

void HoUnitGroup::ToXml(IXmlNode& unit_groups_node) const {
  auto& node = unit_groups_node.AddNode("ho:UNITGROUP");

  if (HoNameDetails::IsActive()) {
    HoNameDetails::ToXml(node);
  }

  node.SetProperty("ho:CATEGORY", HoUnitGroupCategoryToString(category_));

  if (!unit_ref_list_.empty()) {
    auto& unit_refs_node = node.AddNode("ho:UNIT-REFS");
    for (const std::string& unit_ref : unit_ref_list_) {
      auto& unit_ref_node = unit_refs_node.AddNode("ho:UNIT-REF");
      unit_ref_node.SetAttribute("ID-REF", unit_ref);
    }
  }
}

void HoUnitGroup::FromXml(const IXmlNode& group_node) {
  HoNameDetails::FromXml(group_node);
  IXmlNode::ChildList node_list;
  group_node.GetChildList(node_list);
  for (const IXmlNode* node : node_list) {
    if (node == nullptr) {
      continue;
    }

    if (node->IsTagName("CATEGORY")) {
      category_ = StringToHoUnitGroupCategory(node->Value<std::string>());
    } else if (node->IsTagName("UNIT-REFS")) {
      IXmlNode::ChildList unit_list;
      node->GetChildList(unit_list);
      for (const IXmlNode* ref_node : unit_list) {
        if (ref_node != nullptr && ref_node->IsTagName("UNIT-REF") ) {
          std::string unit_ref = ref_node->Attribute<std::string>("ID-REF", "");
          if (unit_ref.empty()) {
            unit_ref = ref_node->Value<std::string>();
          }
          AddUnitReference(unit_ref);
        }
      }
    }
  }

}

}  // namespace mdf