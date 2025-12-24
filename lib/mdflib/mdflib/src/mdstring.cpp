/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */


#include <utility>

#include "mdf/mdstring.h"
#include "mdf/mdfhelper.h"

#include "ixmlnode.h"

namespace mdf {
MdString::MdString(const char* text, uint64_t history_index,
                   std::string language)
    : MdStandardAttribute(history_index, std::move(language) ),
      text_(text != nullptr ? text : "") {

}

MdString::MdString(const std::string_view& text, uint64_t history_index,
                   std::string language)
: MdStandardAttribute(history_index, std::move(language) ),
text_(text) {

}

MdString::MdString(std::string text, uint64_t history_index,
                   std::string language)
 : MdStandardAttribute(history_index, std::move(language)),
   text_(std::move(text)) {
}

MdString::operator const std::string& () const {
  return text_;
}

bool MdString::operator==(const MdString& text) const {
  return text_ == text.Text()
         && HistoryIndex() == text.HistoryIndex()
         && Language() == text.Language();
}

bool MdString::operator==(const std::string& text) const {
  return text_ == text;
}

bool MdString::operator==(const std::string_view& text) const {
  return text_ == text;
}

bool MdString::IsActive() const {
  return !text_.empty();
}

void MdString::Text(std::string text) {
  MdfHelper::Trim(text);
  text_ = std::move(text);
}

const std::string& MdString::Text() const {
  return text_;
}

bool MdString::operator<(const MdString& text) const {
  return text_ < text.text_;
}

void MdString::Offset(uint64_t offset) {
  offset_ = std::make_optional<uint64_t>(offset);
}

uint64_t MdString::Offset() const {
  return offset_.has_value() ? offset_.value() : 0;
}

void MdString::ToXml(IXmlNode& root_node,
                     const std::string& tag_name) const {
  // The TX tag should always be added even if no text.
  if (!text_.empty() || tag_name == "TX") {
    IXmlNode& node = root_node.AddNode(tag_name);
    node.Value(text_);
    MdStandardAttribute::ToXml(node);
    if (tag_name == "linker_name" &&
        offset_.has_value() && offset_.value() > 0) {
      node.SetAttribute("offset", offset_.value());
    }
  }
}



void MdString::FromXml(const IXmlNode& node) {
  const auto attr_list = node.GetAttributeList();
  for (const auto& [key, value] : attr_list) {
    if (key == "ci" && !value.empty()) {
      try {
        ci_ = std::stoll(value);
      } catch (const std::exception&) {
        ci_ = 0;
      }
    } else if (key == "xml:lang") {
      lang_ = value;
    } else if (key == "offset") {
      try {
        offset_ = stoull(value);
      } catch (const std::exception& ) {
        offset_.reset();
      }
    } else {
      AddCustomAttribute(key, value);
    }
  }
  text_ = node.Value<std::string>();
}
}  // namespace mdf