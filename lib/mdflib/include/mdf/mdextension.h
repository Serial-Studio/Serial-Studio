/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once

#include <memory>
#include <vector>

#include "mdf/mdstandardattribute.h"

namespace mdf {

class MdExtension;
class IXmlNode;

using MdExtensionPtr = std::unique_ptr<MdExtension>;

using MdExtensionList = std::vector<MdExtensionPtr>;

class MdExtension : public MdStandardAttribute {
 public:
  [[nodiscard]] virtual MdExtensionPtr Create() const;
  void ToXml(IXmlNode& ext_node) const override;
  void FromXml(const IXmlNode& ext_node) override;
 protected:
  MdExtension() = default;
};

}  // namespace mdf


