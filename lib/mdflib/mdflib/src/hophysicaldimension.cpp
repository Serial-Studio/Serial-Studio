/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include "mdf/hophysicaldimension.h"

#include "ixmlnode.h"

namespace mdf {

void HoPhysicalDimension::Identity(std::string identity) {
  identity_ = std::move(identity);
}

const std::string& HoPhysicalDimension::Identity() const {
  return identity_;
}

void HoPhysicalDimension::LengthExponent(double length_exp) {
  length_exp_ = length_exp;
}

double HoPhysicalDimension::LengthExponent() const {
  return length_exp_;
}

void HoPhysicalDimension::MassExponent(double mass_exp) {
  mass_exp_ = mass_exp;
}

double HoPhysicalDimension::MassExponent() const {
  return mass_exp_;
}

void HoPhysicalDimension::TimeExponent(double time_exp) {
  time_exp_ = time_exp;
}

double HoPhysicalDimension::TimeExponent() const {
  return time_exp_;
}

void HoPhysicalDimension::CurrentExponent(double current_exp) {
  current_exp_ = current_exp;
}

double HoPhysicalDimension::CurrentExponent() const {
  return current_exp_;
}

void HoPhysicalDimension::TemperatureExponent(double temp_exp) {
  temp_exp_ = temp_exp;
}

double HoPhysicalDimension::TemperatureExponent() const {
  return temp_exp_;
}

void HoPhysicalDimension::MolarExponent(double molar_exp) {
  molar_exp_ = molar_exp;
}

double HoPhysicalDimension::MolarExponent() const {
  return molar_exp_;
}

void HoPhysicalDimension::LuminousExponent(double luminous_exp) {
  luminous_exp_ = luminous_exp;
}

double HoPhysicalDimension::LuminousExponent() const {
  return luminous_exp_;
}

void HoPhysicalDimension::ToXml(IXmlNode& root_node) const {
  auto& node = root_node.AddNode("ho:PHYSICAL-DIMENSION");
  node.SetAttribute("ID", identity_);

  if (HoNameDetails::IsActive()) {
    HoNameDetails::ToXml(node);
  }
  if (length_exp_ != 0.0) {
    node.SetProperty("LENGTH-EXP", length_exp_);
  }
  if (mass_exp_ != 0.0) {
    node.SetProperty("MASS-EXP", mass_exp_);
  }
  if (time_exp_ != 0.0) {
    node.SetProperty("TIME-EXP", time_exp_);
  }
  if (current_exp_ != 0.0) {
    node.SetProperty("CURRENT-EXP", current_exp_);
  }
  if (temp_exp_ != 0.0) {
    node.SetProperty("TEMPERATURE-EXP", temp_exp_);
  }
  if (molar_exp_ != 0.0) {
    node.SetProperty("MOLAR-AMOUNT-EXP", molar_exp_);
  }
  if (luminous_exp_ != 0.0) {
    node.SetProperty("LUMINOUS-INTENSITY-EXP", luminous_exp_);
  }
}

void HoPhysicalDimension::FromXml(const IXmlNode& dim_node) {
  IXmlNode::ChildList node_list;
  dim_node.GetChildList(node_list);
  identity_ = dim_node.Attribute<std::string>("ID", "");
  for (const IXmlNode* node : node_list) {
    if (node == nullptr) {
      continue;
    }
    if (node->IsTagName("NAME-DETAILS")) {
      HoNameDetails::FromXml(*node);
    } else if (node->IsTagName("LENGTH-EXP")) {
      length_exp_ = node->Value<double>();
    } else if (node->IsTagName("MASS-EXP")) {
      mass_exp_ = node->Value<double>();
    } else if (node->IsTagName("TIME-EXP")) {
      time_exp_ = node->Value<double>();
    } else if (node->IsTagName("CURRENT-EXP")) {
      current_exp_ = node->Value<double>();
    } else if (node->IsTagName("TEMPERATURE-EXP")) {
      temp_exp_ = node->Value<double>();
    } else if (node->IsTagName("MOLAR-EXP")) {
      molar_exp_ = node->Value<double>();
    } else if (node->IsTagName("LUMINOUS-EXP")) {
      luminous_exp_ = node->Value<double>();
    }
  }
}

}  // namespace mdf