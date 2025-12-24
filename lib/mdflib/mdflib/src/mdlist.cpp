/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include <utility>

#include "mdf/mdlist.h"
#include "mdf/mdflogstream.h"

#include "ixmlnode.h"

namespace mdf {

bool MdList::IsActive() const {
  // If the name is empty. It might be the root element
  return !name_.empty() || !desc_.empty() || !property_list_.empty()
      || !tree_list_.empty() || !list_list_.empty()
      || !enumerate_list_.empty();
}

void MdList::Name(std::string name) {
  name_ = std::move(name);
}

const std::string& MdList::Name() const {
  return name_;
}

void MdList::Description(std::string description) {
  desc_ = std::move(description);
}

const std::string& MdList::Description() const {
  return desc_;
}

void MdList::AddProperty(MdProperty property) {
  if (const auto& name = property.Name();
      !IsDuplicateName(name) ) {
    property_list_.emplace(name, std::move(property));
  }
}

void MdList::AddTree(MdList tree) {
  if (const auto& name = tree.Name();
      !IsDuplicateName(name) ) {
    tree_list_.emplace(name, std::move(tree));
  }
}

void MdList::AddList(MdList list) {
  if (const auto& name = list.Name();
      !IsDuplicateName(name)) {
    list_list_.emplace(name, std::move(list));
  }
}

void MdList::AddEnumerate(MdEnumerate enumerate) {
  if (const auto& name = enumerate.Name();
      !IsDuplicateName(name)) {
    enumerate_list_.emplace(name, std::move(enumerate));
  }
}

bool MdList::IsDuplicateName(const std::string& name) const {
  if (name.empty()) {
    MDF_ERROR() << "List name cannot be empty.";
    return true;
  }

  if (const auto property_itr = property_list_.find(name);
      property_itr != property_list_.cend()) {
    MDF_ERROR() << "A duplicate property name found. Property: "
                << property_itr->first;
    return true;
  }

  if (const auto tree_itr = tree_list_.find(name);
      tree_itr != tree_list_.cend()) {
    MDF_ERROR() << "A duplicate tree name found. Tree: "
                << tree_itr->first;
    return true;
  }

  if (const auto list_itr = list_list_.find(name);
      list_itr != list_list_.cend()) {
    MDF_ERROR() << "The duplicate list name found. List: "
                << list_itr->first;
    return true;
  }

  if (const auto enum_itr = enumerate_list_.find(name);
      enum_itr != enumerate_list_.cend()) {
    MDF_ERROR() << "The duplicate enumerate name found. Enumerate: "
                << enum_itr->first;
    return true;
  }
  return false;
}

const MdPropertyList& MdList::Properties() const {
  return property_list_;
}

MdPropertyList& MdList::Properties() {
  return property_list_;
}

const MdListList& MdList::Trees() const {
  return tree_list_;
}

MdListList& MdList::Trees() {
  return tree_list_;
}

const MdListList& MdList::Lists() const {
  return list_list_;
}

MdListList& MdList::Lists() {
  return list_list_;
}

const MdEnumerateList& MdList::Enumerates() const {
  return enumerate_list_;
}

MdEnumerateList& MdList::Enumerates() {
  return enumerate_list_;
}

const MdProperty* MdList::GetProperty(const std::string& name) const {
  const size_t separator = name.find_first_of('/');
  if (separator == std::string::npos) {
    // Fetch the property
    if (const auto itr_prop = property_list_.find(name);
        itr_prop != property_list_.cend()) {
      return &itr_prop->second;
    }

    for (const auto& [tree_name, tree] : tree_list_) {
      if (const MdProperty* temp = tree.GetProperty(name);
          temp != nullptr) {
        return temp;
      }
    }

    for (const auto& [list_name, list] : list_list_) {
      if (const MdProperty* temp = list.GetProperty(name);
          temp != nullptr) {
        return temp;
      }
    }
  } else {
    const auto prop_name = name.substr(separator + 1);
    for (const auto& [tree_name, tree] : tree_list_) {
      if (const MdProperty* temp = tree.GetProperty(prop_name);
          temp != nullptr) {
        return temp;
      }
    }

    for (const auto& [list_name, list] : list_list_) {
      if (const MdProperty* temp = list.GetProperty(prop_name);
          temp != nullptr) {
        return temp;
      }
    }
  }
  return nullptr;
}


const MdEnumerate* MdList::GetEnumerate(const std::string& name) const {
  const size_t separator = name.find_first_of('/');
  if (separator == std::string::npos) {
    // Fetch the property
    if (const auto itr_enum = enumerate_list_.find(name);
        itr_enum != enumerate_list_.cend()) {
      return &itr_enum->second;
    }

    for (const auto& [tree_name, tree] : tree_list_) {
      if (const MdEnumerate* temp = tree.GetEnumerate(name);
          temp != nullptr) {
        return temp;
      }
    }

    for (const auto& [list_name, list] : list_list_) {
      if (const MdEnumerate* temp = list.GetEnumerate(name);
          temp != nullptr) {
        return temp;
      }
    }
  } else {
    const auto enum_name = name.substr(separator + 1);
    for (const auto& [tree_name, tree] : tree_list_) {
      if (const MdEnumerate* temp = tree.GetEnumerate(enum_name);
          temp != nullptr) {
        return temp;
      }
    }

    for (const auto& [list_name, list] : list_list_) {
      if (const MdEnumerate* temp = list.GetEnumerate(enum_name);
          temp != nullptr) {
        return temp;
      }
    }
  }
  return nullptr;
}


void MdList::ToXml(IXmlNode& list_node) const {

  // The list can hold another lists but the root has no attributes
  const std::string& tag_name = list_node.TagName();
  if (tag_name != "common_properties") {
    if (!name_.empty()) {
      list_node.SetAttribute("name", name_);
    } else {
      MDF_ERROR() << "The XML tag needs a name attribute. Tag: " << tag_name;
    }

    if (!desc_.empty()) {
      list_node.SetAttribute("desc", desc_);
    }
    MdStandardAttribute::ToXml(list_node);
  }

  IXmlNode& sub_node = tag_name == "list" ? list_node.AddNode("li") : list_node;


  for (const auto& [e_tag, e_prop] : property_list_) {
    auto& e_node = sub_node.AddNode("e");
    e_prop.ToXml(e_node);
  }

  for (const auto& [tree_tag, tree] : tree_list_) {
    auto& tree_node = sub_node.AddNode("tree");
    tree.ToXml(tree_node);
  }

  for (const auto& [list_tag, l_list] : list_list_) {
    auto& l_node = sub_node.AddNode("list");
    l_list.ToXml(l_node);
  }

  for (const auto& [enum_tag, enum_list] : enumerate_list_) {
    auto& elist_node = sub_node.AddNode("elist");
    enum_list.ToXml(elist_node);
  }

}

void MdList::FromXml(const IXmlNode& root_node) {
  const std::string tag_name = root_node.TagName();
  if (tag_name != "common_properties" && tag_name != "li") {
    MdStandardAttribute::FromXml(root_node);
    name_ = root_node.Attribute<std::string>("name", "");
    desc_ = root_node.Attribute<std::string>("desc", "");
  }
  IXmlNode::ChildList node_list;
  root_node.GetChildList(node_list);
  for (const IXmlNode* node : node_list) {
    if (node == nullptr) {
      continue;
    }
    if (node->IsTagName("e") ) {
      MdProperty property;
      property.FromXml(*node);
      AddProperty(property);
    } else if (node->IsTagName("tree")) {
      MdList tree;
      tree.FromXml(*node);
      AddTree(tree);
    } else if (node->IsTagName("list")) {
      MdList list;
      list.MdStandardAttribute::FromXml(*node);
      list.Name(node->Attribute<std::string>("name", "") );
      list.Description( node->Attribute<std::string>("desc", ""));

      IXmlNode::ChildList li_list;
      node->GetChildList(li_list);
      for (const IXmlNode* li_node : li_list) {
        if (li_node != nullptr && li_node->IsTagName("li") ) {
          list.FromXml(*li_node);
        }
      }
      AddList(list);
    } else if (node->IsTagName("elist")) {
      MdEnumerate enumerate;
      enumerate.FromXml(*node);
      AddEnumerate(enumerate);
    }
  }

}

}  // namespace mdf