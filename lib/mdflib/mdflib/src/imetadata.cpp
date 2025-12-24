/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "mdf/imetadata.h"
#include "mdf/mdflogstream.h"

#include "ixmlfile.h"

namespace {

void InsertETag(const mdf::ETag& e_tag, mdf::IXmlNode& root) {  // NOLINT
  // First check if this is a tree. If so we create a new node and
  const auto& tree_list = e_tag.TreeList();
  if (e_tag.Name().empty()) {
    return;
  }

  if (!tree_list.empty()) {
    auto& tree = root.AddUniqueNode("tree", "name", e_tag.Name());
    if (!e_tag.Description().empty()) {
      tree.SetAttribute("desc", e_tag.Description());
    }
    if (e_tag.CreatorIndex() >= 0) {
      tree.SetAttribute("ci", e_tag.CreatorIndex());
    }
    for (const auto& tag : tree_list) {
      InsertETag(tag, tree);
    }
  } else {
    auto& tag = root.AddUniqueNode("e", "name", e_tag.Name());
    if (!e_tag.Description().empty()) {
      tag.SetAttribute("desc", e_tag.Description());
    }
    if (e_tag.CreatorIndex() >= 0) {
      tag.SetAttribute("ci", e_tag.CreatorIndex());
    }
    if (!e_tag.Type().empty()) {
      tag.SetAttribute("type", e_tag.Type());
    }
    if (!e_tag.Language().empty()) {
      tag.SetAttribute("xml:lang", e_tag.Language());
    }
    if (e_tag.ReadOnly()) {
      tag.SetAttribute("ro", e_tag.ReadOnly());
    }
    if (!e_tag.Unit().empty()) {
      tag.SetAttribute("unit", e_tag.Unit());
    }
    if (!e_tag.UnitRef().empty()) {
      tag.SetAttribute("unit_ref", e_tag.UnitRef());
    }
    tag.Value(e_tag.Value<std::string>());
  }
}

void FetchETag(const mdf::IXmlNode& root, mdf::ETag& e_tag) {  // NOLINT
  e_tag.Name(root.Attribute<std::string>("name"));
  e_tag.Description(root.Attribute<std::string>("desc"));
  e_tag.CreatorIndex(root.Attribute("ci", -1));
  e_tag.Type(root.Attribute<std::string>("type"));
  e_tag.Language(root.Attribute<std::string>("xml:lang"));
  e_tag.ReadOnly(root.Attribute("ro", false));
  e_tag.Unit(root.Attribute<std::string>("unit"));
  e_tag.UnitRef(root.Attribute<std::string>("unit_ref"));
  e_tag.Value(root.Value<std::string>());

  mdf::IXmlNode::ChildList list;
  root.GetChildList(list);
  for (const auto* child : list) {
    if (child == nullptr) {
      continue;
    }
    if (child->IsTagName("e") || child->IsTagName("tree")) {
      mdf::ETag e_child;
      FetchETag(*child, e_child);
      e_tag.AddTag(e_child);
    }
  }
}

}  // namespace
namespace mdf {

void IMetaData::InitMd(const std::string& root_name) {
  auto xml = CreateXmlFile();
  const auto snippet = XmlSnippet();
  if (!snippet.empty()) {
    xml->ParseString(XmlSnippet());
  }
  auto& root_node = xml->RootName(root_name);
  root_node.AddUniqueNode("TX");
  XmlSnippet(xml->WriteString(true));
}

void IMetaData::StringProperty(const std::string& tag,
                               const std::string& value) {
  auto xml = CreateXmlFile();
  xml->ParseString(XmlSnippet());
  xml->SetProperty(tag, value);
  XmlSnippet(xml->WriteString(true));
}

std::string IMetaData::StringProperty(const std::string& tag) const {
  auto xml = CreateXmlFile();
  if (!xml) {
    return {};
  }

  if (const bool parse = xml->ParseString(XmlSnippet()); !parse) {
    MDF_ERROR() << "Failed to parse XML string. XML: " << XmlSnippet()
      << ", Block Type: " << BlockType();
    return {};
  }

  auto value = xml->Property<std::string>(tag);
  return value;
}

void IMetaData::FloatProperty(const std::string& tag, double value) {
  auto xml = CreateXmlFile();
  xml->ParseString(XmlSnippet());
  xml->SetProperty(tag, value);
  XmlSnippet(xml->WriteString(true));
}

double IMetaData::FloatProperty(const std::string& tag) const {
  auto xml = CreateXmlFile();
  xml->ParseString(XmlSnippet());
  auto value = xml->Property<double>(tag);
  return value;
}

void IMetaData::CommonProperty(const ETag& e_tag) {
  auto xml = CreateXmlFile();

  xml->ParseString(XmlSnippet());
  auto& root_node = xml->RootName(xml->RootName());
  auto& common = root_node.AddUniqueNode("common_properties");
  InsertETag(e_tag, common);
  XmlSnippet(xml->WriteString(true));
}

ETag IMetaData::CommonProperty(const std::string& name) const {
  auto xml = CreateXmlFile();
  xml->ParseString(XmlSnippet());
  const auto* common = xml->GetNode("common_properties");
  ETag tag;
  if (common != nullptr) {
    const auto* tree_tag = common->GetNode("tree", "name", name);
    const auto* e_tag = common->GetNode("e", "name", name);
    if (tree_tag != nullptr) {
      FetchETag(*tree_tag, tag);
    } else if (e_tag != nullptr) {
      FetchETag(*e_tag, tag);
    }
  }
  return tag;
}


void IMetaData::CommonProperties(const std::vector<ETag>& tag_list) {
  auto xml = CreateXmlFile();
  xml->ParseString(XmlSnippet());
  auto& root_node = xml->RootName(xml->RootName());
  auto& common = root_node.AddUniqueNode("common_properties");
  for (const auto& tag : tag_list) {
    InsertETag(tag, common);
  }
  XmlSnippet(xml->WriteString(true));
}

std::vector<ETag> IMetaData::Properties() const {
  std::vector<ETag> tag_list;
  auto xml = CreateXmlFile();
  xml->ParseString(XmlSnippet());
  IXmlNode::ChildList list;
  xml->GetChildList(list);
  for (const auto* child : list) {
    if (child == nullptr) {
      continue;
    }
    if (child->HasChildren()) {
      continue;
    }
    ETag temp;
    temp.Name(child->TagName());
    temp.Value(child->Value<std::string>());
    tag_list.emplace_back(temp);
  }
  return tag_list;
}

std::vector<ETag> IMetaData::CommonProperties() const {
  std::vector<ETag> tag_list;
  auto xml = CreateXmlFile();
  xml->ParseString(XmlSnippet());
  const auto* common = xml->GetNode("common_properties");
  if (common != nullptr) {
    IXmlNode::ChildList list;
    common->GetChildList(list);
    for (const auto* child : list) {
      if (child == nullptr) {
        continue;
      }
      if (child->IsTagName("tree")) {
        ETag tag;
        FetchETag(*child, tag);
        tag_list.emplace_back(tag);
      } else if (child->IsTagName("e")) {
        ETag tag;
        FetchETag(*child, tag);
        tag_list.emplace_back(tag);
      }
    }
  }
  return tag_list;
}

}  // namespace mdf