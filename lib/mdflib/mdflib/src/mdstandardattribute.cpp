/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include <utility>

#include "mdf/mdstandardattribute.h"
#include "mdf/mdfhelper.h"
#include "mdf/mdflogstream.h"

#include "ixmlnode.h"

namespace mdf {

MdStandardAttribute::MdStandardAttribute(uint64_t index,
                                         std::string language)
: ci_(index),
  lang_(std::move(language) ) {
}

bool MdStandardAttribute::IsActive() const {
  if (ci_ > 0) {
    return true;
  }
  if (!lang_.empty()) {
    return true;
  }
  return !custom_attribute_list_.empty();
}


void MdStandardAttribute::HistoryIndex(uint64_t index) {
  ci_ = index;
}

uint64_t MdStandardAttribute::HistoryIndex() const {
  return ci_;
}

void MdStandardAttribute::Language(std::string language) {
  lang_ = std::move(language);
}

const std::string& MdStandardAttribute::Language() const {
  return lang_;
}

void MdStandardAttribute::AddCustomAttribute(std::string key,
                                             std::string value) {
  MdfHelper::Trim(key);

  if (key.empty()) {
    MDF_ERROR() << "Empty key is not allowed. Value: " << value;
    return;
  }

  if ( const auto itr = custom_attribute_list_.find(key);
       itr != custom_attribute_list_.cend()) {
    MDF_ERROR() << "Duplicate key found. Key/Value: "
                << itr->first << "/" << itr->second;
    return;
  }
  custom_attribute_list_.emplace(std::move(key), std::move(value));
}

const AttributeList& MdStandardAttribute::CustomAttributes() const {
  return custom_attribute_list_;
}

AttributeList& MdStandardAttribute::CustomAttributes() {
  return custom_attribute_list_;
}

void MdStandardAttribute::ToXml(IXmlNode& node) const {
  // Add attributes to the node
  if (ci_ > 0) {
    node.SetAttribute("ci", ci_);
  }
  if (!lang_.empty()) {
    node.SetAttribute("xml:lang", lang_);
  }
  for (const auto& [key,value] : custom_attribute_list_) {
    node.SetAttribute(key, value);
  }
}

void MdStandardAttribute::FromXml(const IXmlNode& node) {
  const auto attr_list = node.GetAttributeList();
  for (const auto& [key, value] : attr_list) {
    if (key == "ci" && !value.empty()) {
      try {
        ci_ = std::stoll(value);
      } catch (const std::exception_ptr&) {
        ci_ = 0;
      }
    } else if (key == "xml:lang") {
      lang_ = value;
    } else {
      AddCustomAttribute(key, value);
    }
  }
}

}  // namespace mdf