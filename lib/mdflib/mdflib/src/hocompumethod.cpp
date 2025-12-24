/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include <utility>

#include "mdf/hocompumethod.h"
#include "mdf/hoscaleconstraint.h"

#include "ixmlnode.h"

namespace mdf {

bool HoCompuMethod::IsActive() const {
  return HoNameDetails::IsActive()
         || !unit_ref_.empty()
         || !physical_constraint_list_.empty()
         || !internal_constraint_list_.empty()
      || !compu_scale_list_.empty()
      || default_float_value_.has_value()
      || default_text_value_.has_value();
}

void HoCompuMethod::Category(HoComputationMethodCategory category) {
  category_ = category;
}

HoComputationMethodCategory HoCompuMethod::Category() const {
  return category_;
}

void HoCompuMethod::UnitReference(std::string unit_ref) {
  unit_ref_ = std::move(unit_ref);
}

const std::string& HoCompuMethod::UnitReference() const {
  return unit_ref_;
}

void HoCompuMethod::AddPhysicalConstraint(HoScaleConstraint constraint) {
  physical_constraint_list_.emplace_back(std::move(constraint) );
}

const HoScaleConstraintList& HoCompuMethod::PhysicalConstraints() const {
  return physical_constraint_list_;
}

HoScaleConstraintList& HoCompuMethod::PhysicalConstraints() {
  return physical_constraint_list_;
}

void HoCompuMethod::AddInternalConstraint(HoScaleConstraint constraint) {
  internal_constraint_list_.emplace_back(std::move(constraint));
}

const HoScaleConstraintList& HoCompuMethod::InternalConstraints() const {
  return internal_constraint_list_;
}

HoScaleConstraintList& HoCompuMethod::InternalConstraints() {
  return internal_constraint_list_;
}

void HoCompuMethod::AddCompuScale(HoCompuScale scale) {
  compu_scale_list_.emplace_back(std::move(scale));
}

const HoCompuScaleList& HoCompuMethod::CompuScales() const {
  return compu_scale_list_;
}

HoCompuScaleList& HoCompuMethod::CompuScales() {
  return compu_scale_list_;
}

void HoCompuMethod::DefaultFloatValue(double value) {
  default_float_value_ = value;
}

double HoCompuMethod::DefaultFloatValue() const {
  return default_float_value_.has_value() ? default_float_value_.value() : 0.0;
}

void HoCompuMethod::DefaultTextValue(std::string value) {
  default_text_value_ = std::move(value);
}

std::string HoCompuMethod::DefaultTextValue() const {
  return default_text_value_.has_value() ? default_text_value_.value() : std::string();
}

void HoCompuMethod::ToXml(IXmlNode& root_node) const {
  if (!IsActive()) {
    return;
  }
  auto& method_node = root_node.AddNode("ho:COMPU-METHOD");

  HoNameDetails::ToXml(method_node);
  method_node.SetProperty("ho:CATEGORY",
                       HoComputationMethodCategoryToString(category_));
  if (!unit_ref_.empty()) {
    auto& unit_ref_node = method_node.AddNode("ho:UNIT-REF");
    unit_ref_node.SetAttribute("ID-REF", unit_ref_);
  }

  if (!physical_constraint_list_.empty()) {
    auto& phys_node = method_node.AddNode("ho:PHYS-CONSTRS");
    for (const HoScaleConstraint& scale : physical_constraint_list_) {
      scale.ToXml(phys_node);
    }
  }
  if (!internal_constraint_list_.empty()) {
    auto& internals_node = method_node.AddNode("ho:INTERNAL-CONSTRS");
    for (const HoScaleConstraint& scale : internal_constraint_list_) {
      scale.ToXml(internals_node);
    }
  }
  if (!compu_scale_list_.empty() || default_float_value_.has_value()
                                         || default_text_value_.has_value()) {
    IXmlNode& internal_node = method_node.AddNode("ho:COMPU-INTERNAL-TO-PHYS");
    if (!compu_scale_list_.empty()) {
      IXmlNode& scales_node = internal_node.AddNode("ho:COMPU-SCALES");
      for (const HoCompuScale& scale : compu_scale_list_) {
        scale.ToXml(scales_node);
      }
    }
    if (default_float_value_.has_value() || default_text_value_.has_value()) {
      IXmlNode& default_node = internal_node.AddNode("ho:COMPU-DEFAULT-VALUE");
      if (default_float_value_.has_value()) {
        default_node.SetProperty("ho:V", default_float_value_.value());
      }
      if (default_text_value_.has_value()) {
        default_node.SetProperty("ho:VT", default_text_value_.value());
      }
    }
  }


}

void HoCompuMethod::FromXml(const IXmlNode& root_node) {
  IXmlNode::ChildList node_list;
  root_node.GetChildList(node_list);
  for (const IXmlNode* node : node_list) {
    if (node == nullptr) {
      continue;
    }
    if (node->IsTagName("NAME-DETAILS")) {
      HoNameDetails::FromXml(*node);
    } else if (node->IsTagName("CATEGORY")) {
      const auto category_text = node->Value<std::string>();
      if (!category_text.empty()) {
        category_ = StringToHoComputationMethodCategory(category_text);
      }
    } else if (node->IsTagName("UNIT-REF")) {
      if (unit_ref_ = node->Attribute<std::string>("ID-REF", "");
          unit_ref_.empty()) {
        unit_ref_ = node->Value<std::string>();
      }
    } else if (node->IsTagName("PHYS-CONSTRS")) {
      IXmlNode::ChildList const_list;
      node->GetChildList(const_list);
      for (const IXmlNode* const_node : const_list) {
        if (const_node != nullptr && const_node->IsTagName("SCALE-CONSTR")) {
          HoScaleConstraint constraint;
          constraint.FromXml(*const_node);
          AddPhysicalConstraint(constraint);
        }
      }
    } else if (node->IsTagName("INTERNAL-CONSTRS")) {
      IXmlNode::ChildList const_list;
      node->GetChildList(const_list);
      for (const IXmlNode* const_node : const_list) {
        if (const_node != nullptr && const_node->IsTagName("SCALE-CONSTR")) {
          HoScaleConstraint constraint;
          constraint.FromXml(*const_node);
          AddInternalConstraint(constraint);
        }
      }
    } else if (node->IsTagName("COMPU-INTERNAL-TO-PHYS")) {
      IXmlNode::ChildList phys_list;
      node->GetChildList(phys_list);
      for (const IXmlNode* phys_node : phys_list) {
        if (phys_node == nullptr) {
          continue;
        }
        if (phys_node->IsTagName("COMPU-SCALES")) {
          IXmlNode::ChildList scale_list;
          phys_node->GetChildList(scale_list);
          for (const IXmlNode* scale_node : scale_list) {
            if (scale_node != nullptr && scale_node->IsTagName("COMPU-SCALE")) {
              HoCompuScale scale;
              scale.FromXml(*scale_node);
              AddCompuScale(scale);
            }
          }
        } else if (phys_node->IsTagName("COMPU-DEFAULT-VALUE")) {
          IXmlNode::ChildList value_list;
          phys_node->GetChildList(value_list);
          for (const IXmlNode* value_node : value_list) {
            if (value_node == nullptr) {
              continue;
            }
            if (value_node->IsTagName("V")) {
              default_float_value_ = value_node->Value<double>();
            } else if (value_node->IsTagName("VT")) {
              default_text_value_ = value_node->Value<std::string>();
            }
          }
        }
      }
    }
  }
}

}  // namespace mdf