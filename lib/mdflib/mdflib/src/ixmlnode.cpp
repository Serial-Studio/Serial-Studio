/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "ixmlnode.h"

#include <algorithm>
#include <cstring>

#include "mdf/mdfhelper.h"
#include "xmlnode.h"

namespace {
std::string XmlString(const std::string &text) {
  std::ostringstream xml_string;
  for (char in_char : text) {
    switch (in_char) {
      case '<':
        xml_string << "&lt;";
        break;

      case '>':
        xml_string << "&gt;";
        break;

      case '&':
        xml_string << "&amp;";
        break;

      case '\"':
        xml_string << "&quot;";
        break;

      case '\'':
        xml_string << "&apos;";
        break;

      default:
        xml_string << in_char;
        break;
    }
  }
  return xml_string.str();
}

}  // namespace
namespace mdf {

IXmlNode::IXmlNode(const std::string &tag_name) : tag_name_(tag_name) {
  MdfHelper::Trim(tag_name_);
}

void IXmlNode::AddNode(std::unique_ptr<IXmlNode> p) {
  node_list_.push_back(std::move(p));
}

IXmlNode &IXmlNode::AddNode(const std::string &name) {
  auto node = CreateNode(name);
  auto &ret = *node;
  AddNode(std::move(node));
  return ret;
}

IXmlNode &IXmlNode::AddUniqueNode(const std::string &name) {
  auto itr = std::find_if(node_list_.begin(), node_list_.end(),
                          [&](const auto &ptr)
                          { return ptr && ptr->IsTagName(name); });
  return itr != node_list_.end() ?
    *(itr->get()) :
    AddNode(name);
}

IXmlNode &IXmlNode::AddUniqueNode(const std::string &name,
                                  const std::string &key,
                                  const std::string &attr) {
  auto itr = std::find_if(node_list_.begin(), node_list_.end(),
                          [&](const auto &ptr) {
    return ptr && ptr->IsTagName(name) && ptr->IsAttribute(key, attr);
  });
  if (itr != node_list_.end()) {
    return *(itr->get());
  }
  auto &node = AddNode(name);
  node.SetAttribute(key, attr);
  return node;
}

std::unique_ptr<IXmlNode> IXmlNode::CreateNode(const std::string &name) const {
  return std::make_unique<XmlNode>(name);
}

const IXmlNode *IXmlNode::GetNode(const std::string &tag) const {
  const auto itr = std::find_if(node_list_.cbegin(), node_list_.cend(),
                                [&](const auto &ptr) {
                                  return ptr && ptr->IsTagName(tag); });
  return itr == node_list_.cend() ? nullptr : itr->get();
}

const IXmlNode *IXmlNode::GetNode(const std::string &tag,
                                  const std::string &key,
                                  const std::string &value) const {
  const auto itr = std::find_if(node_list_.cbegin(), node_list_.cend(),
                                [&](const auto &ptr) {
    return ptr && ptr->IsTagName(tag) && ptr->IsAttribute(key, value);
  });
  return itr == node_list_.cend() ? nullptr : itr->get();
}

void IXmlNode::GetChildList(IXmlNode::ChildList &child_list) const {
  for (const auto &p : node_list_) {
    if (!p) {
      continue;
    }
    child_list.push_back(p.get());
  }
}

bool IXmlNode::IsTagName(const std::string &tag) const {
  if (Platform::stricmp(tag.c_str(), tag_name_.c_str()) == 0) {
    return true;
  }
  // try the tag name without namespace
  const auto *ns = strchr(tag_name_.c_str(), ':');
  if (ns != nullptr) {
    ++ns;
    if (Platform::stricmp(tag.c_str(), ns) == 0) {
      return true;
    }
  }
  return false;
}

bool IXmlNode::IsAttribute(const std::string &key,
                           const std::string &value) const {
  return std::any_of(attribute_list_.cbegin(), attribute_list_.cend(),
                     [&](const auto &itr) {
    return Platform::stricmp(itr.first.c_str(), key.c_str()) == 0 &&
           Platform::stricmp(itr.second.c_str(), value.c_str()) == 0;
  });
}

void IXmlNode::Write(std::ostream &dest, size_t level) {  // NOLINT

  for (size_t tab = 0; tab < level; ++tab) {
    dest << "  ";
  }
  dest << "<" << TagName();
  for (const auto &attr : attribute_list_) {
    dest << " " << attr.first << "='" << XmlString(attr.second) << "'";
  }
  if (node_list_.empty() && value_.empty()) {
    dest << "/>" << std::endl;
  } else if (node_list_.empty()) {
    dest << ">" << XmlString(value_) << "</" << TagName() << ">" << std::endl;
  } else {
    dest << ">" << std::endl;
    for (const auto &node : node_list_) {
      if (!node) {
        continue;
      }
      node->Write(dest, level + 1);
    }
    for (size_t tab1 = 0; tab1 < level; ++tab1) {
      dest << "  ";
    }
    dest << "</" << TagName() << ">" << std::endl;
  }
}

void IXmlNode::DeleteNode(const std::string &name) {
  for (auto itr = node_list_.begin(); itr != node_list_.end(); /* No ++itr */) {
    const auto* node = itr->get();
    if (node == nullptr || !node->IsTagName(name) ) {
      ++itr;
      continue;
    }
    itr = node_list_.erase(itr);
  }
}

void IXmlNode::DeleteNode(const IXmlNode *node) {
  const auto itr = std::find_if(node_list_.cbegin(), node_list_.cend(),
                          [&] (auto& item) {
    return item.get() == node;
  });
  if (itr != node_list_.end()) {
    node_list_.erase(itr);
  }
}

IXmlNode::KeyValueList IXmlNode::GetAttributeList() const {
  KeyValueList attr_list;
  for (const auto& [key, value] : attribute_list_) {
    if (!key.empty()) {
      attr_list.emplace(key, value);
    }
  }
  return attr_list;
}

bool IXmlNode::HasChildren() const {
  return !node_list_.empty();
}

template <>
void IXmlNode::Value(const bool &value) {
  value_ = value ? "true" : "false";
}

template <>
void IXmlNode::Value(const std::string &value) {
  value_ = value;
}

template <>
void IXmlNode::SetAttribute(const std::string &key, const bool &value) {
  attribute_list_.insert({key, value ? "true" : "false"});
}

}  // namespace mdf
