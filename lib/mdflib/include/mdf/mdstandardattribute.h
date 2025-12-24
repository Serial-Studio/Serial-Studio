/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once

#include <cstdint>
#include <string>
#include <map>

namespace mdf {

class IXmlNode;

using AttributeList = std::map<std::string, std::string>;

class MdStandardAttribute {
 public:
  [[nodiscard]] virtual bool IsActive() const;

  MdStandardAttribute() = default;
  explicit MdStandardAttribute(uint64_t index,
                      std::string language = std::string() );

  void HistoryIndex(uint64_t index);
  [[nodiscard]] uint64_t HistoryIndex() const;

  void Language(std::string language);
  [[nodiscard]] const std::string& Language() const;

  void AddCustomAttribute(std::string key, std::string value);
  [[nodiscard]] const AttributeList& CustomAttributes() const;
  [[nodiscard]] AttributeList& CustomAttributes();

  virtual void ToXml(IXmlNode& node) const;
  virtual void FromXml(const IXmlNode& node);
 protected:

  uint64_t ci_ = 0;
  std::string lang_;
  AttributeList custom_attribute_list_;
};

}  // namespace mdf

