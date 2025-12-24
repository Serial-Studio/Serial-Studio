/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once

#include <cstdint>
#include <optional>

#include "mdf/mdcomment.h"
#include "mdf/mdstring.h"
#include "mdf/mdnumber.h"
#include "mdf/hounit.h"

namespace mdf {

enum class MdMonotony : int {
  MonDecrease = 0,
  MonIncrease,
  StrictDecrease,
  StrictIncrease,
  Monotonous,
  StrictMon,
  NotMono,
};

std::string_view MdMonotonyToString(MdMonotony monotony);
MdMonotony StringToMdMonotony(const std::string& mon_text);

class CnComment : public MdComment {
 public:
  CnComment();
  explicit CnComment(std::string comment);

  void LinkerName(MdString linker_name);
  [[nodiscard]] const MdString& LinkerName() const;

  void LinkerAddress(MdNumber linker_address);
  [[nodiscard]] const MdNumber& LinkerAddress() const;

  void AxisMonotony(MdMonotony axis_monotony);
  [[nodiscard]] MdMonotony AxisMonotony() const;

  void Raster(MdNumber raster);
  [[nodiscard]] const MdNumber& Raster() const;

  void Address(MdNumber address);
  [[nodiscard]] const MdNumber& Address() const;

  [[nodiscard]] std::string ToXml() const override;
  void FromXml(const std::string& xml_snippet) override;

 private:
  MdString linker_name_;
  MdNumber linker_address_;
  std::optional<MdMonotony> axis_monotony_;
  MdNumber raster_;
  MdNumber address_;

};

}  // namespace mdf


