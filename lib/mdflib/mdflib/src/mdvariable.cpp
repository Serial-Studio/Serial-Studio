/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include <utility>

#include "mdf/mdvariable.h"

#include "ixmlnode.h"

namespace mdf {

void MdVariable::Variable(std::string variable) {
  Text(std::move(variable));
}

const std::string& MdVariable::Variable() const {
  return Text();
}

void MdVariable::CnBlockName(std::string name) {
  cn_ = std::move(name);
}

const std::string& MdVariable::CnBlockName() const {
  return cn_;
}

void MdVariable::CnSourceName(std::string name) {
  cs_ = std::move(name);
}

const std::string& MdVariable::CnSourceName() const {
  return cs_;
}

void MdVariable::CnSourcePath(std::string path) {
  cp_ = std::move(path);
}

const std::string& MdVariable::CnSourcePath() const {
  return cp_;
}


void MdVariable::CgBlockName(std::string name) {
  gn_ = std::move(name);
}

const std::string& MdVariable::CgBlockName() const {
  return gn_;
}

void MdVariable::CgSourceName(std::string name) {
  gs_ = std::move(name);
}

const std::string& MdVariable::CgSourceName() const {
  return gs_;
}

void MdVariable::CgSourcePath(std::string path) {
  gp_ = std::move(path);
}

const std::string& MdVariable::CgSourcePath() const {
  return gp_;
}

void MdVariable::AddArrayIndex(uint64_t index) {
  index_list_.push_back(index);
}

const MdArrayIndexList& MdVariable::ArrayIndexes() const {
  return index_list_;
}

MdArrayIndexList& MdVariable::ArrayIndexes() {
  return index_list_;
}

void MdVariable::Raw(bool raw) {
  raw_ = raw;
}

bool MdVariable::Raw() const {
  return raw_;
}

void MdVariable::ToXml(IXmlNode& root_node, const std::string& tag_name) const {
  IXmlNode& node = root_node.AddNode(tag_name);

  if (!cn_.empty()) {
    node.SetAttribute("cn", cn_);
  }
  if (!cs_.empty()) {
    node.SetAttribute("cs", cs_);
  }
  if (!cp_.empty()) {
    node.SetAttribute("cp", cp_);
  }

  if (!gn_.empty()) {
    node.SetAttribute("gn", gn_);
  }
  if (!gs_.empty()) {
    node.SetAttribute("gs", gs_);
  }
  if (!gp_.empty()) {
    node.SetAttribute("gp", gp_);
  }

  if (!index_list_.empty()) {
    std::ostringstream idx;
    for (const uint64_t index : index_list_) {
      if (!idx.str().empty()) {
        idx << " ";
      }
      idx << index;
    }
    node.SetAttribute("idx_list_pattern", idx.str());
  }

  if (raw_) {
    node.SetAttribute("raw", raw_);
  }

  MdStandardAttribute::ToXml(node);

  node.Value(Variable());
}

void MdVariable::FromXml(const IXmlNode& var_node) {
  MdString::FromXml(var_node);
  cn_ = var_node.Attribute<std::string>("cn", "");
  cs_ = var_node.Attribute<std::string>("cs", "");
  cp_ = var_node.Attribute<std::string>("cp", "");
  gn_ = var_node.Attribute<std::string>("gn", "");
  gs_ = var_node.Attribute<std::string>("gs", "");
  gp_ = var_node.Attribute<std::string>("gp", "");
  const std::string idx_list = var_node.Attribute<std::string>("idx", "");
  try {
    std::ostringstream temp;
    for (const char input : idx_list) {
      if (iswspace(input)) {
        const uint64_t idx = std::stoull(temp.str());
        AddArrayIndex(idx);
        temp.clear();
        temp.str("");
      }
    }
    if (!temp.str().empty()) {
      const uint64_t idx = std::stoull(temp.str());
      AddArrayIndex(idx);
    }
  } catch (const std::exception& ) {}
  raw_ = var_node.Attribute<bool>("raw", false);
}

}  // namespace mdf