/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include <utility>

#include "mdf/hocompuscale.h"

#include "ixmlnode.h"

namespace mdf {

void HoCompuScale::AddDescription(MdString description) {
  description_list_.emplace_back(std::move(description));
}

const MdStringList& HoCompuScale::Descriptions() const {
  return description_list_;
}

MdStringList& HoCompuScale::Descriptions() {
  return description_list_;
}

void HoCompuScale::ConstFloatValue(double value) {
  const_float_value_ = value;
}

double HoCompuScale::ConstFloatValue() const {
  return const_float_value_.has_value() ? const_float_value_.value() : 0.0;
}

void HoCompuScale::ConstTextValue(std::string value) {
  const_text_value_ = std::move(value);
}

std::string HoCompuScale::ConstTextValue() const {
  return const_text_value_.has_value() ? const_text_value_.value() : std::string();
}

void HoCompuScale::AddNumerator(double value) {
  numerator_list_.emplace_back(value);
}

const CoeffList& HoCompuScale::Numerators() const {
  return numerator_list_;
}

CoeffList& HoCompuScale::Numerators() {
  return numerator_list_;
}

void HoCompuScale::AddDenominator(double value) {
  denominator_list_.emplace_back(value);
}

const CoeffList& HoCompuScale::Denominators() const {
  return denominator_list_;
}

CoeffList& HoCompuScale::Denominators() {
  return denominator_list_;
}

void HoCompuScale::GenericMath(std::string math) {
  generic_math_ = std::move(math);
}

const std::string& HoCompuScale::GenericMath() const {
  return generic_math_;
}

void HoCompuScale::ToXml(IXmlNode& scales_node) const {
  auto& scale_node = scales_node.AddNode("ho:COMPU-SCALE");
  for (const MdString& desc : description_list_) {
    desc.ToXml(scale_node, "ho:DESC");
  }
  if (lower_limit_.IsActive()) {
    lower_limit_.ToXml(scale_node, "ho:LOWER-LIMIT");
  }
  if (upper_limit_.IsActive()) {
    upper_limit_.ToXml(scale_node, "ho:UPPER-LIMIT");
  }
  if (const_float_value_.has_value() || const_text_value_.has_value()) {
    auto& const_node = scale_node.AddNode("ho:COMPU-CONST");
    if (const_float_value_.has_value()) {
      const_node.SetProperty("ho:V", const_float_value_.value());
    }
    if (const_text_value_.has_value()) {
      const_node.SetProperty("ho:VT", const_text_value_.value());
    }
  }
  if (!numerator_list_.empty() || !denominator_list_.empty()) {
    IXmlNode& coeffs_node = scale_node.AddNode("ho:COMPU-RATIONAL-COEFFS");

    IXmlNode& num_node = coeffs_node.AddNode("ho:COMPU-NUMERATOR");
    for (const double& numerator : numerator_list_) {
      IXmlNode& v_node = num_node.AddNode("ho:V");
      v_node.Value(numerator);
    }

    IXmlNode& denom_node = coeffs_node.AddNode("ho:COMPU-DENOMINATOR");
    for (const double& denominator : denominator_list_) {
      IXmlNode& v_node = denom_node.AddNode("ho:V");
      v_node.Value(denominator);
    }
  }
  if (!generic_math_.empty()) {
    scale_node.SetProperty("ho:COMPU-GENERIC-MATH", generic_math_);
  }
}

void HoCompuScale::FromXml(const IXmlNode& scale_node) {
  IXmlNode::ChildList node_list;
  scale_node.GetChildList(node_list);

  for (const IXmlNode* node : node_list) {
    if (node == nullptr) {
      continue;
    }
    if (node->IsTagName("DESC")) {
      MdString desc;
      desc.FromXml(*node);
      AddDescription(desc);
    } else if (node->IsTagName("LOWER-LIMIT")) {
      lower_limit_.FromXml(*node);
    } else if (node->IsTagName("UPPER-LIMIT")) {
      upper_limit_.FromXml(*node);
    } else if (node->IsTagName("COMPU-CONST")) {
      IXmlNode::ChildList const_list;
      node->GetChildList(const_list);
      for (const IXmlNode* v_node : const_list) {
        if (v_node == nullptr) {
          continue;
        }
        if (v_node->IsTagName("V")) {
          const_float_value_ = v_node->Value<double>();
        } else if (v_node->IsTagName("VT")) {
          const_text_value_ = v_node->Value<std::string>();
        }
      }
    } else if (node->IsTagName("COMPU-RATIONAL-COEFFS")) {
      IXmlNode::ChildList coeffs_list;
      node->GetChildList(coeffs_list);
      for (const IXmlNode* coeff_node : coeffs_list) {
        if (coeff_node == nullptr) {
          continue;
        }
        if (coeff_node->IsTagName("COMPU-NUMERATOR")) {
          IXmlNode::ChildList v_list;
          coeff_node->GetChildList(v_list);
          for (const IXmlNode* v_node : v_list ) {
            if (v_node != nullptr && v_node->IsTagName("V")) {
              AddNumerator(v_node->Value<double>());
            }
          }
        } else if (coeff_node->IsTagName("COMPU-DENOMINATOR")) {
          IXmlNode::ChildList v_list;
          coeff_node->GetChildList(v_list);
          for (const IXmlNode* v_node : v_list ) {
            if (v_node != nullptr && v_node->IsTagName("V")) {
              AddDenominator(v_node->Value<double>());
            }
          }
        }
      }
    } else if (node->IsTagName("COMPU-GENERIC-MATH")) {
      generic_math_ = node->Value<std::string>();
    }
  }
}

}  // namespace mdf