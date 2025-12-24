/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once
#include <string>

#include "mdf/honamedetails.h"

namespace mdf {

class HoPhysicalDimension;
class IXmlNode;

/** \brief The list is sorted on the Identity attribute . */
using HoPhysicalDimensionList = std::map<std::string, HoPhysicalDimension>;

class HoPhysicalDimension : public HoNameDetails {
 public:
  void Identity(std::string identity);
  [[nodiscard]] const std::string& Identity() const;

  void LengthExponent(double length_exp);
  [[nodiscard]] double LengthExponent() const;

  void MassExponent(double mass_exp);
  [[nodiscard]] double MassExponent() const;

  void TimeExponent(double time_exp);
  [[nodiscard]] double TimeExponent() const;

  void CurrentExponent(double current_exp);
  [[nodiscard]] double CurrentExponent() const;

  void TemperatureExponent(double temp_exp);
  [[nodiscard]] double TemperatureExponent() const;

  void MolarExponent(double molar_exp);
  [[nodiscard]] double MolarExponent() const;

  void LuminousExponent(double luminous_exp);
  [[nodiscard]] double LuminousExponent() const;

  void ToXml(IXmlNode& root_node) const override;
  void FromXml(const IXmlNode& dim_node) override;

 private:
  std::string identity_;
  double length_exp_ = 0.0;
  double mass_exp_ = 0.0;
  double time_exp_ = 0.0;
  double current_exp_ = 0.0;
  double temp_exp_ = 0.0;
  double molar_exp_ = 0.0;
  double luminous_exp_ = 0.0;
};

}  // namespace mdf


