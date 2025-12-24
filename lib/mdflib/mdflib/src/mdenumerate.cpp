/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include "mdf/mdenumerate.h"
#include "ixmlnode.h"

namespace mdf {

void MdEnumerate::AddValue(MdString value) {
  value_list_.emplace_back(std::move(value));
}

void MdEnumerate::ToXml(IXmlNode& elist_node) const {
  // Enumerate is a normal property but the value is an enumerate
  // list instead of a single value.
  MdProperty::ToXml(elist_node);

  for (const auto& enum_value : value_list_) {
    enum_value.ToXml(elist_node, "eli");
  }
}

void MdEnumerate::FromXml(const IXmlNode& elist_node) {
  MdProperty::FromXml(elist_node);
  IXmlNode::ChildList node_list;
  elist_node.GetChildList(node_list);
  for (const IXmlNode* node : node_list) {
    if (node != nullptr && node->IsTagName("eli")) {
      MdString value;
      value.FromXml(*node);
      AddValue(value);
    }
  }
}
const MdEnumerateValueList& MdEnumerate::ValueList() const {
  return value_list_;
}

MdEnumerateValueList& MdEnumerate::ValueList() {
  return value_list_;
}


}  // namespace mdf