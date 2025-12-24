/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <map>

#include <mdf/mdstring.h>

namespace mdf {

class MdVariable;

using MdVariableList = std::map<std::string, MdVariable>;
using MdArrayIndexList = std::vector<uint64_t>;

class MdVariable : public MdString {
 public:
  void Variable(std::string variable);
  [[nodiscard]] const std::string& Variable() const;

  void CnBlockName(std::string name);
  [[nodiscard]] const std::string& CnBlockName() const;

  void CnSourceName(std::string name);
  [[nodiscard]] const std::string& CnSourceName() const;

  void CnSourcePath(std::string path);
  [[nodiscard]] const std::string& CnSourcePath() const;

  void CgBlockName(std::string name);
  [[nodiscard]] const std::string& CgBlockName() const;

  void CgSourceName(std::string name);
  [[nodiscard]] const std::string& CgSourceName() const;

  void CgSourcePath(std::string path);
  [[nodiscard]] const std::string& CgSourcePath() const;

  void AddArrayIndex(uint64_t index);
  [[nodiscard]] const MdArrayIndexList& ArrayIndexes() const;
  [[nodiscard]] MdArrayIndexList& ArrayIndexes();

  void Raw(bool raw);
  [[nodiscard]] bool Raw() const;

  void ToXml(IXmlNode& root_node, const std::string& tag_name) const override;
  void FromXml(const IXmlNode& var_node) override;
 private:
  std::string cn_; ///< CN block name.
  std::string cs_; ///< CN source (SI) name.
  std::string cp_; ///< CN source (SI) path.
  std::string gn_; ///< CG block name.
  std::string gs_; ///< CG source (SI) name.
  std::string gp_; ///< CG source (SI) path.

  std::vector<uint64_t> index_list_; // CA array index

  bool raw_ = false;
};

}  // namespace mdf


