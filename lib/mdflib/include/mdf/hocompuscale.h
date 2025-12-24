/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once

#include <optional>
#include <vector>

#include "mdf/hoscaleconstraint.h"
#include "mdf/mdstring.h"

namespace mdf {

class HoCompuScale;
class IXmlNode;

using CoeffList = std::vector<double>;
using HoCompuScaleList = std::vector<HoCompuScale>;

class HoCompuScale : public HoScaleConstraint {
 public:
  void AddDescription(MdString description);
  [[nodiscard]] const MdStringList& Descriptions() const;
  [[nodiscard]] MdStringList& Descriptions();

  void ConstFloatValue(double value);
  [[nodiscard]] double ConstFloatValue() const;

  void ConstTextValue(std::string value);
  [[nodiscard]] std::string ConstTextValue() const;

  void AddNumerator(double value);
  [[nodiscard]] const CoeffList& Numerators() const;
  [[nodiscard]] CoeffList& Numerators();

  void AddDenominator(double value);
  [[nodiscard]] const CoeffList& Denominators() const;
  [[nodiscard]] CoeffList& Denominators();

  void GenericMath(std::string math);
  [[nodiscard]] const std::string& GenericMath() const;

  void ToXml(IXmlNode& scales_node) const override;
  void FromXml(const IXmlNode& scale_node) override;
 private:
  MdStringList description_list_;

  std::optional<double> const_float_value_;
  std::optional<std::string> const_text_value_;

  CoeffList numerator_list_;
  CoeffList denominator_list_;
  std::string generic_math_;

};

}  // namespace mdf


