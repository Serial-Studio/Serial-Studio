/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */
#include <utility>

#include "mdf/honamedetails.h"

#include "ixmlnode.h"

namespace mdf {

bool HoNameDetails::IsActive() const {
  return !short_name_.empty()
      || !long_name_list_.empty() || !description_list_.empty();
}

void HoNameDetails::ShortName(std::string short_name) {
  short_name_ = std::move(short_name);
}

const std::string& HoNameDetails::ShortName() const {
  return short_name_;
}

void HoNameDetails::AddLongName(MdString long_name) {
  long_name_list_.emplace_back(std::move(long_name));
}

const MdStringList& HoNameDetails::LongNames() const {
  return long_name_list_;
}

MdStringList& HoNameDetails::LongNames() {
  return long_name_list_;
}

void HoNameDetails::AddDescription(MdString description) {
  description_list_.emplace_back(std::move(description));
}

const MdStringList& HoNameDetails::Descriptions() const {
  return description_list_;
}

MdStringList& HoNameDetails::Descriptions() {
  return description_list_;
}

void HoNameDetails::ToXml(IXmlNode& root_node) const {
  if (!IsActive()) {
    return;
  }
  if (!short_name_.empty()) {
    root_node.SetProperty("ho:SHORT-NAME", short_name_);
  }
  for (const MdString& name : long_name_list_) {
    name.ToXml(root_node, "ho:LONG-NAME");
  }
  for (const MdString& desc : description_list_) {
    desc.ToXml(root_node, "ho:DESC");
  }
}

void HoNameDetails::FromXml(const IXmlNode& root_node) {
  IXmlNode::ChildList node_list;
  root_node.GetChildList(node_list);
  for (const IXmlNode* node : node_list) {
    if (node == nullptr) {
      continue;
    }
    if (node->IsTagName("SHORT-NAME")) {
      short_name_ = node->Value<std::string>();
    } else if (node->IsTagName("LONG-NAME")) {
      MdString long_name;
      long_name.FromXml(*node);
      AddLongName(long_name);
    } else if (node->IsTagName("DESC")) {
      MdString description;
      description.FromXml(*node);
      AddDescription(description);
    }

  }
}

}  // namespace mdf