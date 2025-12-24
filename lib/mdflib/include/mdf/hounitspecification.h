/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once
#include <optional>

#include "mdf/hoadmindata.h"
#include "mdf/hophysicaldimension.h"
#include "mdf/hounitgroup.h"
#include "mdf/hounit.h"
#include "mdf/hointerval.h"

namespace mdf {

class IXmlNode;

class HoUnitSpecification  {
 public:
  [[nodiscard]] bool IsActive() const;
  [[nodiscard]] const HoAdminData& AdminData() const;
  [[nodiscard]] HoAdminData& AdminData();

  void AddPhysicalDimension(HoPhysicalDimension dimension);
  [[nodiscard]] const HoPhysicalDimensionList& PhysicalDimensions() const;
  [[nodiscard]] HoPhysicalDimensionList& PhysicalDimensions();

  void AddUnitGroup(HoUnitGroup group);
  [[nodiscard]] const HoUnitGroupList& UnitGroups() const;
  [[nodiscard]] HoUnitGroupList& UnitGroups();

  void AddUnit(HoUnit unit);
  [[nodiscard]] const HoUnitList& Units() const;
  [[nodiscard]] HoUnitList& Units();

  void ToXml(IXmlNode& root_node) const;
  void FromXml(const IXmlNode& unit_spec_node);

 private:
  HoAdminData admin_data_;
  HoPhysicalDimensionList phys_dim_list_;
  HoUnitGroupList unit_group_list_;
  HoUnitList unit_list_;

};

}  // namespace mdf

