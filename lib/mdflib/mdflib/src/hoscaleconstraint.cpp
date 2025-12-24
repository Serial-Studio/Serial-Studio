/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include <string_view>

#include "mdf/hoscaleconstraint.h"
#include "mdf/hoenumerates.h"

#include "ixmlnode.h"

namespace {
constexpr std::string_view kValidity = "VALIDITY";

constexpr std::string_view kLowerLimit = "ho:LOWER-LIMIT";
constexpr std::string_view kUpperLimit = "ho:UPPER-LIMIT";

constexpr std::string_view kScaleConstraint = "ho:SCALE-CONSTR";

}

namespace mdf {

bool HoScaleConstraint::IsActive() const {
  return lower_limit_.IsActive() || upper_limit_.IsActive();
}

void HoScaleConstraint::LowerLimit(HoInterval limit) {
  lower_limit_ = limit;
}

const HoInterval& HoScaleConstraint::LowerLimit() const {
  return lower_limit_;
}

void HoScaleConstraint::UpperLimit(HoInterval limit) {
  upper_limit_ = limit;
}

const HoInterval& HoScaleConstraint::UpperLimit() const {
  return upper_limit_;
}

void HoScaleConstraint::Validity(HoValidity validity) {
  validity_ = validity;
}

HoValidity HoScaleConstraint::Validity() const {
  return validity_;
}

void HoScaleConstraint::ToXml(IXmlNode& root_node) const {
  // Assume that the ho namespace is set in the MD comment root
  if (!IsActive()) {
    return;
  }
  auto& scale_node = root_node.AddNode(std::string(kScaleConstraint));
  if (lower_limit_.IsActive()) {
    lower_limit_.ToXml(scale_node, kLowerLimit);
 }
 if (upper_limit_.IsActive()) {
  upper_limit_.ToXml(scale_node, kUpperLimit);
 }
 if (validity_ != HoValidity::Valid) {
    scale_node.SetAttribute(std::string(kValidity),
                         std::string(HoValidityToString(validity_)));
  }
}

void HoScaleConstraint::FromXml(const IXmlNode& scale_node) {
  const auto validity_text =
      scale_node.Attribute<std::string>("VALIDITY", "");
  if (!validity_text.empty()) {
    validity_ = StringToHoValidity(validity_text);
  }

  IXmlNode::ChildList node_list;
  scale_node.GetChildList(node_list);
  for (const auto* node : node_list) {
    if (node == nullptr) {
      continue;
    }
    if (node->IsTagName("LOWER-LIMIT")) {
      lower_limit_.FromXml(*node);
    } else if (node->IsTagName("UPPER-LIMIT")) {
      upper_limit_.FromXml(*node);
    }
  }
}

}  // namespace mdf