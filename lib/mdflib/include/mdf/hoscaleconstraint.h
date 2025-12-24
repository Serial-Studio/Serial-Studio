/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once

#include <vector>

#include "mdf/hoenumerates.h"
#include "mdf/hointerval.h"

namespace mdf {

class HoScaleConstraint;
class IXmlNode;

using HoScaleConstraintList = std::vector<HoScaleConstraint>;

class HoScaleConstraint {
 public:
  [[nodiscard]] bool IsActive() const;

  void LowerLimit(HoInterval limit);
  [[nodiscard]] const HoInterval& LowerLimit() const;

  void UpperLimit(HoInterval limit);
  [[nodiscard]] const HoInterval& UpperLimit() const;

  void Validity(HoValidity validity);
  [[nodiscard]] HoValidity Validity() const;

  virtual void ToXml(IXmlNode& root_node) const;
  virtual void FromXml(const IXmlNode& scale_node);

 protected:
  HoInterval lower_limit_;
  HoInterval upper_limit_;
  HoValidity validity_ = HoValidity::Valid;
};

}  // namespace mdf

