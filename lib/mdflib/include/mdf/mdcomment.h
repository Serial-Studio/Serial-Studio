/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
*/

#pragma once
#include <string>
#include <memory>
#include <functional>

#include "mdf/mdalternativename.h"
#include "mdf/mdstandardattribute.h"
#include "mdf/mdstring.h"
#include "mdf/mdlist.h"
#include "mdf/mdformula.h"
#include "mdf/mdextension.h"

namespace mdf {

class IXmlFile;
class IXmlNode;

using MdExtensionCreator = std::function<MdExtensionPtr()>;

class MdComment {
 public:

  explicit MdComment(std::string block_name);
  virtual ~MdComment() = default;

  void Comment(std::string comment);
  void Comment(MdString comment);
  [[nodiscard]] const MdString& Comment() const;

  void AddProperty(MdProperty property);
  void AddProperty(std::string key, std::string value);
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

  [[nodiscard]] const MdAlternativeName& Names() const;
  [[nodiscard]] MdAlternativeName& Names();

  [[nodiscard]] const MdFormula& Formula() const;
  [[nodiscard]] MdFormula& Formula();

  void SetExtensionCreator(MdExtensionCreator creator);
  void AddExtension(MdExtensionPtr extension);
  [[nodiscard]] const MdExtensionList& Extensions() const;
  [[nodiscard]] MdExtensionList& Extensions();

  [[nodiscard]] virtual std::string ToXml() const;
  virtual void FromXml(const std::string& xml_snippet);
 protected:
  std::string block_name_; ///< Block name as "HD".
  MdString comment_; ///< TX tag in XML
  MdList common_property_;
  MdAlternativeName alternative_name_;
  MdFormula formula_;
  MdExtensionList extension_list_;
  MdExtensionCreator extension_creator_;

  [[nodiscard]] virtual IXmlNode& CreateRootNode(IXmlFile& xml_file,
                                         bool init_ho_namespace) const;
  void ToXml(IXmlNode& root_node) const;
  void ToTx(IXmlNode& root_node) const;
  void ToFormula(IXmlNode& root_node) const;
  void ToNames(IXmlNode& root_node) const;
  void ToCommonProp(IXmlNode& root_node) const;
  void FromXml(const IXmlNode& root_node);

 };

 template< typename T>
 T MdComment::GetPropertyValue(const std::string& name) const {
   return common_property_.GetPropertyValue<T>(name);
 }
}  // namespace mdf


