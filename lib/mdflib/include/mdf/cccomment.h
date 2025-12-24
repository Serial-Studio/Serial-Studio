/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once
#include <string>

#include "mdf/mdcomment.h"
#include "mdf/hocompumethod.h"

namespace mdf {

class CcComment : public MdComment {
 public:
  CcComment();
  explicit CcComment(std::string comment);

  [[nodiscard]] const HoCompuMethod& CompuMethod() const;
  [[nodiscard]] HoCompuMethod& CompuMethod();

  [[nodiscard]] std::string ToXml() const override;
  void FromXml(const std::string& xml_snippet) override;
 private:
  HoCompuMethod method_;
};

}  // namespace mdf

