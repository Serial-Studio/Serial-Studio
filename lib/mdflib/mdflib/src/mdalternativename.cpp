/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include <utility>

#include "mdf/mdalternativename.h"
#include "mdf/mdflogstream.h"

#include "ixmlnode.h"

namespace mdf {

bool MdAlternativeName::IsActive() const {
  return !name_list_.empty() || !display_list_.empty() ||
      !vendor_list_.empty() || !description_list_.empty();
}

void MdAlternativeName::DefaultName(std::string name) {
  default_name_ = std::move(name);
}

const std::string& MdAlternativeName::DefaultName() const {
  return default_name_;
}

void MdAlternativeName::AddAlternativeName(std::string alternative_name) {
  AddAlternativeName(MdString(std::move(alternative_name)));
}

void MdAlternativeName::AddAlternativeName(MdString alternative_name) {
  if (const std::string& name = alternative_name.Text();
     name.empty()) {
    MDF_ERROR() << "Empty alternative name is not allowed.";
    return;
  }
  name_list_.emplace_back(std::move(alternative_name));
}

void MdAlternativeName::AddDisplayName(MdString display_name) {
  if (const std::string& name = display_name.Text();
      name.empty()) {
    MDF_ERROR() << "Empty display name is not allowed.";
    return;
  }
  display_list_.emplace_back(std::move(display_name));
}

void MdAlternativeName::AddVendor(MdString vendor) {
  if (const std::string& name = vendor.Text();
      name.empty()) {
    MDF_ERROR() << "Empty vendor name is not allowed.";
    return;
  }
  vendor_list_.emplace_back(std::move(vendor));
}

void MdAlternativeName::AddDescription(MdString description) {
  description_list_.emplace_back(std::move(description));
}

const MdStringList& MdAlternativeName::AlternativeNames() const {
  return name_list_;
}

MdStringList& MdAlternativeName::AlternativeNames() {
  return name_list_;
}

const MdStringList& MdAlternativeName::DisplayNames() const {
  return display_list_;
}

MdStringList& MdAlternativeName::DisplayNames() {
  return display_list_;
}

const MdStringList& MdAlternativeName::Vendors() const {
  return vendor_list_;
}

MdStringList& MdAlternativeName::Vendors() {
  return vendor_list_;
}


const MdStringList& MdAlternativeName::Descriptions() const {
  return description_list_;
}

MdStringList& MdAlternativeName::Descriptions() {
  return description_list_;
}

void MdAlternativeName::ToXml(IXmlNode& names_node) const {

  MdStandardAttribute::ToXml(names_node);
  if (!default_name_.empty()) {
    names_node.SetAttribute("name", default_name_);
  }

  for (const MdString& name : name_list_) {
    name.ToXml(names_node, "name");
  }

  for (const MdString& display : display_list_) {
    display.ToXml(names_node, "display");
  }

  for (const MdString& vendor : vendor_list_) {
    vendor.ToXml(names_node, "vendor");
  }

  for (const MdString& description : description_list_) {
    description.ToXml(names_node, "description");
  }
}

void MdAlternativeName::FromXml(const IXmlNode& names_node) {
  MdStandardAttribute::FromXml(names_node);
  default_name_ = names_node.Attribute<std::string>("name", "");

  IXmlNode::ChildList node_list;
  names_node.GetChildList(node_list);
  for (const IXmlNode* node : node_list) {
    if (node == nullptr) {
      continue;
    }
    if (node->IsTagName("name")) {
      MdString name;
      name.FromXml(*node);
      AddAlternativeName(name);
    } else if (node->IsTagName("display")) {
      MdString display;
      display.FromXml(*node);
      AddDisplayName(display);
    } else if (node->IsTagName("vendor")) {
      MdString vendor;
      vendor.FromXml(*node);
      AddVendor(vendor);
    } else if (node->IsTagName("description")) {
      MdString desc;
      desc.FromXml(*node);
      AddDescription(desc);
    }
  }
}


}  // namespace mdf