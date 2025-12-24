/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once

#include <string>
#include <map>

#include "mdf/mdstandardattribute.h"
#include "mdf/mdproperty.h"
#include "mdf/mdenumerate.h"

namespace mdf {
class MdList;
class IXmlNode;

using MdListList = std::map<std::string, MdList>;

class MdList : public MdStandardAttribute {
 public:
  [[nodiscard]] bool IsActive() const override;

  void Name(std::string name);
  [[nodiscard]] const std::string& Name() const;

  void Description(std::string description);
  [[nodiscard]] const std::string& Description() const;

  void AddProperty(MdProperty property);
  void AddTree(MdList tree);
  void AddList(MdList list);
  void AddEnumerate(MdEnumerate enumerate);

  [[nodiscard]] const MdPropertyList& Properties() const;
  [[nodiscard]] MdPropertyList& Properties();

  [[nodiscard]] const MdProperty* GetProperty(const std::string& name) const;

  template< typename T>
  [[nodiscard]] T GetPropertyValue(const std::string& name) const;

  [[nodiscard]] const MdEnumerate* GetEnumerate(const std::string& name) const;

  [[nodiscard]] const MdListList& Trees() const;
  [[nodiscard]] MdListList& Trees();

  [[nodiscard]] const MdListList& Lists() const;
  [[nodiscard]] MdListList& Lists();

  [[nodiscard]] const MdEnumerateList& Enumerates() const;
  [[nodiscard]] MdEnumerateList& Enumerates();

  void ToXml(IXmlNode& list_node) const override;
  void FromXml(const IXmlNode& root_node) override;
 protected:
  [[nodiscard]] bool IsDuplicateName(const std::string& name) const;

 private:
  std::string name_;
  std::string desc_;

  MdPropertyList property_list_;
  MdListList tree_list_;
  MdListList list_list_;
  MdEnumerateList enumerate_list_;

};

template< typename T>
T MdList::GetPropertyValue(const std::string& name) const {
  const MdProperty* prop = GetProperty(name);
  return prop != nullptr ? prop->Value<T>() : T{};
}

}  // namespace mdf

