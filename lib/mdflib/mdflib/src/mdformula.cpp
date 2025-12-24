/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include <utility>

#include "mdf/mdformula.h"
#include "mdf/mdflogstream.h"

#include "ixmlnode.h"

namespace mdf {
bool MdFormula::IsActive() const {
  return syntax_.IsActive();
}

void MdFormula::Syntax(MdSyntax syntax) {
  syntax_ = std::move(syntax);
}

const MdSyntax& MdFormula::Syntax() const {
  return syntax_;
}

void MdFormula::AddCustomSyntax(MdSyntax syntax) {
  const std::string& key = syntax;
  if (key.empty()) {
    MDF_ERROR() << "A custom syntax cannot be empty.";
    return;
  }

  if ( const auto itr = custom_syntax_list_.find(key);
       itr != custom_syntax_list_.cend()) {
    MDF_ERROR() << "A duplicate custom attribute syntax found. Syntax: "
        << itr->second.Syntax();
    return;
  }

  custom_syntax_list_.emplace(key, std::move(syntax));
}

const MdSyntaxList& MdFormula::CustomSyntaxes() const {
  return custom_syntax_list_;
}

MdSyntaxList& MdFormula::CustomSyntaxes() {
  return custom_syntax_list_;
}

void MdFormula::AddVariable(MdVariable variable) {
  const std::string& key = variable;
  if (key.empty()) {
    MDF_ERROR() << "A variable cannot be empty.";
    return;
  }

  if ( const auto itr = variable_list_.find(key);
      itr != variable_list_.cend()) {
    MDF_ERROR() << "A duplicate variable found. Variable: "
                << itr->second.Variable();
    return;
  }

  variable_list_.emplace(key, std::move(variable));
}

const MdVariableList& MdFormula::Variables() const {
  return variable_list_;
}

MdVariableList& MdFormula::Variables() {
  return variable_list_;
}

void MdFormula::ToXml(IXmlNode& formula_node) const {
  MdStandardAttribute::ToXml(formula_node);

  syntax_.ToXml(formula_node, "syntax");

  for (const auto& [custom_name, custom_syntax] : custom_syntax_list_) {
    custom_syntax.ToXml(formula_node, "custom_syntax");
  }

  if (!variable_list_.empty()) {
    auto& variables_node = formula_node.AddNode("variables");
    for (const auto& [variable_name, variable] : variable_list_) {
      variable.ToXml(variables_node, "var");
    }
  }
}

void MdFormula::FromXml(const IXmlNode& formula_node) {
  MdStandardAttribute::FromXml(formula_node);
  IXmlNode::ChildList node_list;
  formula_node.GetChildList(node_list);
  for (const IXmlNode* node : node_list) {
    if (node == nullptr) {
      continue;
    }
    if (node->IsTagName("syntax")) {
      syntax_.FromXml(*node);
    } else if (node->IsTagName("custom_syntax")) {
      MdSyntax syntax;
      syntax.FromXml(*node);
      AddCustomSyntax(syntax);
    } else if (node->IsTagName("variables")) {
      IXmlNode::ChildList var_list;
      node->GetChildList(var_list);
      for (const IXmlNode* var_node : var_list) {
        if (var_node != nullptr && var_node->IsTagName("var")) {
          MdVariable var;
          var.FromXml(*var_node);
          AddVariable(var);
        }
      }
    }
  }
}

}  // namespace mdf