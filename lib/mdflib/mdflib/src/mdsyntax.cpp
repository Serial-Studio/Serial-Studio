/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */
#include <utility>

#include "mdf/mdsyntax.h"

#include "ixmlnode.h"

namespace mdf {
void MdSyntax::Syntax(std::string syntax) {
  Text(std::move(syntax));
}

const std::string& MdSyntax::Syntax() const {
  return Text();
}

void MdSyntax::Version(std::string version) {
  if (!version.empty() ) {
    version_ = std::move(version);
  }
}

const std::string& MdSyntax::Version() const {
  return version_;
}

void MdSyntax::Source(std::string source) {
  source_ = std::move(source);
}

const std::string& MdSyntax::Source() const {
  return source_;
}

void MdSyntax::ToXml(IXmlNode& root_node, const std::string& tag_name) const {
  IXmlNode& node = root_node.AddNode(tag_name);

  if (!version_.empty()) {
    node.SetAttribute("version", version_);
  }

  if (!source_.empty() && tag_name == "custom_syntax") {
    node.SetAttribute("source", source_);
  }

  MdStandardAttribute::ToXml(node);

  node.Value(Syntax());
}

void MdSyntax::FromXml(const IXmlNode& syntax_node) {
  MdString::FromXml(syntax_node);
  version_ = syntax_node.Attribute<std::string>("version", "");
  source_ = syntax_node.Attribute<std::string>("source", "");

}

}  // namespace mdf