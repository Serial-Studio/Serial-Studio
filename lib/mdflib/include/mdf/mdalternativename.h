/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once
#include <map>
#include <string>
#include <vector>

#include "mdf/mdstring.h"
#include "mdf/mdstandardattribute.h"

namespace mdf {

class IXmlNode;

using MdStringList = std::vector<MdString>;

class MdAlternativeName : public MdStandardAttribute {
 public:
  [[nodiscard]] bool IsActive() const override;

  void DefaultName(std::string name);
  [[nodiscard]] const std::string& DefaultName() const;

  void AddAlternativeName(MdString alternative_name);
  void AddAlternativeName(std::string alternative_name);
  void AddDisplayName(MdString display_name);
  void AddVendor(MdString vendor);
  void AddDescription(MdString description);

  [[nodiscard]] const MdStringList& AlternativeNames() const;
  [[nodiscard]] MdStringList& AlternativeNames();

  [[nodiscard]] const MdStringList& DisplayNames() const;
  [[nodiscard]] MdStringList& DisplayNames();

  [[nodiscard]] const MdStringList& Vendors() const;
  [[nodiscard]] MdStringList& Vendors();

  [[nodiscard]] const MdStringList& Descriptions() const;
  [[nodiscard]] MdStringList& Descriptions();

  void ToXml(IXmlNode& names_node) const override;
  void FromXml(const IXmlNode& names_node) override;

 private:
  std::string default_name_;
  MdStringList name_list_;
  MdStringList display_list_;
  MdStringList vendor_list_;
  MdStringList description_list_;

};

}  // namespace mdf


