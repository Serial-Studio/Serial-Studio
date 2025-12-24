/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include <utility>

#include "mdf/mdcomment.h"
#include "mdf/mdflogstream.h"
#include "ixmlfile.h"
#include "ixmlnode.h"


namespace mdf {

MdComment::MdComment(std::string block_name)
    : block_name_(std::move(block_name)) {
}

void MdComment::Comment(std::string comment) {
  Comment(MdString(std::move(comment)));
}

void MdComment::Comment(MdString comment) {
  comment_ = std::move(comment);
}

const MdString& MdComment::Comment() const {
  return comment_;
}

void MdComment::AddProperty(MdProperty property) {
  common_property_.AddProperty(std::move(property));
}

void MdComment::AddProperty(std::string key, std::string value) {
  MdProperty prop(std::move(key), std::move(value));
  AddProperty(prop);
}

void MdComment::AddTree(MdList tree) {
  common_property_.AddTree( std::move(tree));
}

void MdComment::AddList(MdList list) {
  common_property_.AddList( std::move(list) );
}

void MdComment::AddEnumerate(MdEnumerate enumerate) {
  common_property_.AddEnumerate(std::move(enumerate));
}

const MdPropertyList& MdComment::Properties() const {
    return common_property_.Properties();
}

MdPropertyList& MdComment::Properties() {
  return common_property_.Properties();
}

const MdListList& MdComment::Trees() const {
  return common_property_.Trees();
}

MdListList& MdComment::Trees() {
  return common_property_.Trees();
}

const MdListList& MdComment::Lists() const {
  return common_property_.Lists();
}

MdListList& MdComment::Lists() {
  return common_property_.Lists();
}

const MdEnumerateList& MdComment::Enumerates() const {
    return common_property_.Enumerates();
}

MdEnumerateList& MdComment::Enumerates() {
  return common_property_.Enumerates();
}

const MdAlternativeName& MdComment::Names() const {
  return alternative_name_;
}

MdAlternativeName& MdComment::Names() {
  return alternative_name_;
}

const MdFormula& MdComment::Formula() const {
  return formula_;
}

MdFormula& MdComment::Formula() {
  return formula_;
}

void MdComment::AddExtension(MdExtensionPtr extension) {
  extension_list_.emplace_back(std::move(extension));
}

const MdExtensionList& MdComment::Extensions() const {
  return extension_list_;
}

MdExtensionList& MdComment::Extensions() {
  return extension_list_;
}

const MdProperty* MdComment::GetProperty(const std::string& name) const {
  return common_property_.GetProperty(name);
}

const MdEnumerate* MdComment::GetEnumerate(const std::string& name) const {
  return common_property_.GetEnumerate(name);
}

IXmlNode& MdComment::CreateRootNode(IXmlFile& xml_file,
                                    bool init_ho_namespace) const {
  std::ostringstream root_name;
  root_name << block_name_ << "comment";
  IXmlNode& root_node = xml_file.RootName(root_name.str());

  if (init_ho_namespace) {
    root_node.SetAttribute("xmlns:ho","http://www.asam.net/xml" );
  }
  return root_node;
}

void MdComment::SetExtensionCreator(MdExtensionCreator creator) {
  extension_creator_ = std::move(creator);
}

void MdComment::ToXml(IXmlNode& root_node) const {
  ToTx(root_node);
  ToNames(root_node);
  ToFormula(root_node);
  ToCommonProp(root_node);
}

void MdComment::ToTx(IXmlNode& root_node) const {
  comment_.ToXml(root_node, "TX");
}

void MdComment::ToFormula(IXmlNode& root_node) const {
  if (formula_.IsActive()) {
    auto& formula_node = root_node.AddNode("formula");
    formula_.ToXml(formula_node);
  }
}

void MdComment::ToNames(IXmlNode& root_node) const {
  if (alternative_name_.IsActive()) {
    auto& names_node = root_node.AddNode("names");
    alternative_name_.ToXml(names_node);
  }
}
void MdComment::ToCommonProp(IXmlNode& root_node) const {
  if (common_property_.IsActive()) {
    auto& prop_node = root_node.AddNode("common_properties");
    common_property_.ToXml(prop_node);
  }

  if (!extension_list_.empty()) {
    auto& extensions_node = root_node.AddNode("extensions");
    for (const MdExtensionPtr& extension : extension_list_) {
      if (!extension) {
        continue;
      }
      auto& extension_node = extensions_node.AddNode("extension");
      extension->ToXml(extension_node);
    }
  }

}
void MdComment::FromXml(const IXmlNode& root_node) {
  IXmlNode::ChildList node_list;
  root_node.GetChildList(node_list);
  for (const IXmlNode* node : node_list) {
    if (node == nullptr) {
      continue;
    }
    if (node->IsTagName("TX")) {
      comment_.FromXml(*node);
    } else if (node->IsTagName("common_properties")) {
      common_property_.FromXml(*node);
    } else if (node->IsTagName("extensions")) {
      IXmlNode::ChildList ext_list;
      node->GetChildList(ext_list);
      for (const IXmlNode* ext_node : ext_list) {
        if (ext_node != nullptr && ext_node->IsTagName("extension") &&
            extension_creator_) {
          if (MdExtensionPtr extension = extension_creator_();
              extension) {
            extension->FromXml(*ext_node);
            AddExtension(std::move(extension));
          }
        }
      }
    } else if (node->IsTagName("formula")) {
      formula_.FromXml(*node);
    } else if (node->IsTagName("names")) {
      alternative_name_.FromXml(*node);
    }
  }
}

std::string MdComment::ToXml() const {
  try {
    auto xml_file = CreateXmlFile("FileWriter");
    constexpr bool ho_active = false;
    auto& root_node = CreateRootNode(*xml_file, ho_active);
    MdComment::ToXml(root_node);
    return xml_file->WriteString(true);
  } catch (const std::exception& err) {
    MDF_ERROR() << "Failed to create the " << block_name_
                << " comment. Error: " << err.what();
  }
  return {};
}

void MdComment::FromXml(const std::string& xml_snippet) {
  if (xml_snippet.empty()) {
    return;
  }
  try {
    auto xml_file = CreateXmlFile("Expat");
    if (!xml_file) {
      throw std::runtime_error("Failed to create EXPAT parser object");
    }
    const bool parse = xml_file->ParseString(xml_snippet);
    if (!parse) {
      throw std::runtime_error("Failed to parse the XML string.");
    }
    const auto* root_node = xml_file->RootNode();
    if (root_node == nullptr) {
      throw std::runtime_error("There is no root node in the XML string");
    }
    MdComment::FromXml(*root_node);
  } catch (const std::exception& err) {
    MDF_ERROR() << "Failed to parse the " << block_name_
                << " comment. Error: " << err.what();
  }
}


}  // namespace mdf