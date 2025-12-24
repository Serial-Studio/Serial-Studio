/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once

#include <string>

#include "mdf/mdstring.h"

namespace mdf {

class IXmlNode;

class HoNameDetails {
 public:
  [[nodiscard]] virtual bool IsActive() const;

  void ShortName(std::string short_name);
  [[nodiscard]] const std::string& ShortName() const;

  void AddLongName(MdString long_name);
  [[nodiscard]] const MdStringList& LongNames() const;
  [[nodiscard]] MdStringList& LongNames();

 void AddDescription(MdString description);
 [[nodiscard]] const MdStringList& Descriptions() const;
 [[nodiscard]] MdStringList& Descriptions();

 virtual void ToXml(IXmlNode& root_node) const;
 virtual void FromXml(const IXmlNode& root_node);
 protected:
  std::string short_name_;
  MdStringList long_name_list_;
  MdStringList description_list_;
};

}  // namespace mdf


