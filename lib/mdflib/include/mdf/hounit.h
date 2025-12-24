/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once

#include <string>
#include <map>

#include "mdf/honamedetails.h"

namespace mdf {

class HoUnit;

using HoUnitList = std::map<std::string, HoUnit>;

class HoUnit : public HoNameDetails {
 public:
  void Identity(std::string identity);
  [[nodiscard]] const std::string& Identity() const;

  void DisplayName(std::string display_name);
  [[nodiscard]] const std::string& DisplayName() const;

  void Factor( double factor);
  [[nodiscard]] double Factor() const;

  void Offset( double offset);
  [[nodiscard]] double Offset() const;

  void PhysicalDimensionRef(std::string phys_dim_ref);
  [[nodiscard]] const std::string& PhysicalDimensionRef() const;

  void ToXml(IXmlNode& units_node) const override;
  void FromXml(const IXmlNode& unit_node) override;

 private:
  std::string identity_;

  std::string display_name_;
  double factor_ = 1.0;
  double offset_ = 0.0;
  std::string phys_dim_ref_;
};

}  // namespace mdf

