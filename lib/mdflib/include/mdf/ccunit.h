/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once

#include "mdf/mdcomment.h"
#include "mdf/mdstring.h"

namespace mdf {

class CcUnit : public MdComment {
 public:
  CcUnit();
  explicit CcUnit(std::string unit);

  void Unit(MdString unit);
  [[nodiscard]] const MdString& Unit() const;

  void UnitRef(MdString unit_ref);
  [[nodiscard]] const MdString& UnitRef() const;

  [[nodiscard]] std::string ToXml() const override;
  void FromXml(const std::string& xml_snippet) override;
 protected:
  [[nodiscard]] IXmlNode& CreateRootNode(IXmlFile& xml_file,
                                   bool init_ho_namespace) const override;
 private:
  MdString unit_ref_;
};

} // end namespace
