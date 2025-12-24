/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */
#include <utility>

#include "mdf/hounitspecification.h"
#include "mdf/mdflogstream.h"

#include "ixmlnode.h"

namespace mdf {

bool HoUnitSpecification::IsActive() const {
  return admin_data_.IsActive() || !phys_dim_list_.empty()
         || !unit_group_list_.empty() || !unit_list_.empty();
}

const HoAdminData& HoUnitSpecification::AdminData() const {
  return admin_data_;
}

HoAdminData& HoUnitSpecification::AdminData() {
  return admin_data_;
}

void HoUnitSpecification::AddPhysicalDimension(HoPhysicalDimension dimension) {
  const std::string& key = dimension.Identity();
  if (key.empty()) {
    MDF_ERROR() << "A physical dimension must have an identity to be referenced";
    return;
  }

  if (const auto itr = phys_dim_list_.find(key);
      itr != phys_dim_list_.cend() ) {
    MDF_ERROR() << "The dimension is already defined. Identity: " << key;
    return;
  }
  phys_dim_list_.emplace(key,std::move(dimension));
}

const HoPhysicalDimensionList& HoUnitSpecification::PhysicalDimensions() const {
  return phys_dim_list_;
}

HoPhysicalDimensionList& HoUnitSpecification::PhysicalDimensions() {
  return phys_dim_list_;
}

void HoUnitSpecification::AddUnitGroup(HoUnitGroup group) {
  const std::string& key = group.ShortName();
  if (key.empty()) {
    MDF_ERROR() << "A unit group must have a short name.";
    return;
  }

  if (const auto itr = unit_group_list_.find(key);
      itr != unit_group_list_.cend() ) {
    MDF_ERROR() << "The unit group is already defined. Short Name: " << key;
    return;
  }
  unit_group_list_.emplace(key,std::move(group));
}

const HoUnitGroupList& HoUnitSpecification::UnitGroups() const {
  return unit_group_list_;
}

HoUnitGroupList& HoUnitSpecification::UnitGroups() {
  return unit_group_list_;
}

void HoUnitSpecification::AddUnit(HoUnit unit) {
  const std::string& key = unit.Identity() ;
  if (key.empty()) {
    MDF_ERROR() << "A unit must have an identity.";
    return;
  }

  if (const auto itr = unit_list_.find(key);
      itr != unit_list_.cend() ) {
    MDF_ERROR() << "The unit is already defined. Short Name: " << key;
    return;
  }
  unit_list_.emplace(key,std::move(unit));
}

const HoUnitList& HoUnitSpecification::Units() const {
  return unit_list_;
}

HoUnitList& HoUnitSpecification::Units() {
  return unit_list_;
}

void HoUnitSpecification::ToXml(IXmlNode& root_node) const {
  if (!IsActive()) {
    return;
  }

  auto& node = root_node.AddNode("ho:UNIT-SPEC");

  if (admin_data_.IsActive()) {
    admin_data_.ToXml(node);
  }

  if (!phys_dim_list_.empty()) {
    auto& phys_dims_node = node.AddNode("ho:PHYSICAL-DIMENSIONS");
    for (const auto& [name, phys_dim] : phys_dim_list_) {
      phys_dim.ToXml(phys_dims_node);
    }
  }

  if (!unit_group_list_.empty()) {
    auto& unit_groups_node = node.AddNode("ho:UNITGROUPS");
    for (const auto& [name, unit_group] : unit_group_list_) {
      unit_group.ToXml(unit_groups_node);
    }
  }

  if (!unit_list_.empty()) {
    auto& units_node = node.AddNode("ho:UNITS");
    for (const auto& [name, unit] : unit_list_) {
      unit.ToXml(units_node);
    }
  }

}

void HoUnitSpecification::FromXml(const IXmlNode& unit_spec_node) {
  IXmlNode::ChildList node_list;
  unit_spec_node.GetChildList(node_list);

  for ( const auto* node : node_list) {
    if (node == nullptr) {
      continue;
    }
    if (node->IsTagName("ADMIN-DATA")) {
      admin_data_.FromXml(*node);
    } else if (node->IsTagName("PHYSICAL-DIMENSIONS")) {
      IXmlNode::ChildList dim_list;
      node->GetChildList(dim_list);
      for (const IXmlNode* dim_node : dim_list ) {
        if (dim_node != nullptr && dim_node->IsTagName("PHYSICAL-DIMENSION")) {
          HoPhysicalDimension dimension;
          dimension.FromXml(*dim_node);
          AddPhysicalDimension(dimension);
        }
      }
    } else if (node->IsTagName("UNITGROUPS")) {
      IXmlNode::ChildList group_list;
      node->GetChildList(group_list);
      for (const IXmlNode* group_node : group_list ) {
        if (group_node != nullptr && group_node->IsTagName("UNITGROUP")) {
          HoUnitGroup group;
          group.FromXml(*group_node);
          AddUnitGroup(group);
        }
      }
    } else if (node->IsTagName("UNITS")) {
      IXmlNode::ChildList unit_list;
      node->GetChildList(unit_list);
      for (const IXmlNode* unit_node : unit_list ) {
        if (unit_node != nullptr && unit_node->IsTagName("UNIT")) {
          HoUnit unit;
          unit.FromXml(*unit_node);
          AddUnit(unit);
        }
      }
    }
  }
}

}  // namespace mdf