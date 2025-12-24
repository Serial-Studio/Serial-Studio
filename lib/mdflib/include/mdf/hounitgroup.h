/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once

#include <string>
#include <set>

#include "mdf/honamedetails.h"
#include "mdf/hoenumerates.h"

namespace mdf {

class HoUnitGroup;
class IXmlNode;

using HoUnitGroupList = std::map<std::string, HoUnitGroup>;
using HoUnitRefList = std::set<std::string>;

class HoUnitGroup : public HoNameDetails {
 public:
  void Category(HoUnitGroupCategory category);
  [[nodiscard]] HoUnitGroupCategory Category() const;

  void AddUnitReference(std::string unit_ref);
  [[nodiscard]] const HoUnitRefList& UnitRefs() const;
  [[nodiscard]] HoUnitRefList& UnitRefs();

  void ToXml(IXmlNode& unit_groups_node) const override;
  void FromXml(const IXmlNode& group_node) override;

 private:
  HoUnitGroupCategory category_ = HoUnitGroupCategory::Country;
  HoUnitRefList unit_ref_list_;

};

}  // namespace mdf


