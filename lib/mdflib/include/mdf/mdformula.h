/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once

#include "mdf/mdsyntax.h"
#include "mdf/mdvariable.h"
#include "mdf/mdstandardattribute.h"

namespace mdf {

class IXmlNode;

class MdFormula : public MdStandardAttribute {
 public:
  [[nodiscard]] bool IsActive() const override;

  void Syntax(MdSyntax syntax);
  [[nodiscard]] const MdSyntax& Syntax() const;

  void AddCustomSyntax(MdSyntax syntax);
  [[nodiscard]] const MdSyntaxList& CustomSyntaxes() const;
  [[nodiscard]] MdSyntaxList& CustomSyntaxes();

  void AddVariable(MdVariable variable);
  [[nodiscard]] const MdVariableList& Variables() const;
  [[nodiscard]] MdVariableList& Variables();

  void ToXml(IXmlNode& formula_node) const override;
  void FromXml(const IXmlNode& formula_node) override;

 private:
  MdSyntax syntax_;
  MdSyntaxList custom_syntax_list_;
  MdVariableList variable_list_;
};

}  // namespace mdf

