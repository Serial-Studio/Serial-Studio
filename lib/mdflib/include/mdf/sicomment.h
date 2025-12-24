/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once

#include "mdf/mdcomment.h"
#include "mdf/mdalternativename.h"
#include "mdf/mdstring.h"

namespace mdf {

class SiComment : public MdComment {
 public:
  SiComment();
  explicit SiComment(std::string comment);

  [[nodiscard]] const MdAlternativeName& Path() const;
  [[nodiscard]] MdAlternativeName& Path();

  [[nodiscard]] const MdAlternativeName& Bus() const;
  [[nodiscard]] MdAlternativeName& Bus();

  void Protocol(MdString protocol);
  [[nodiscard]] const MdString& Protocol() const;

  [[nodiscard]] std::string ToXml() const override;
  void FromXml(const std::string& xml_snippet) override;
 private:
  // names is defined in MdComment class
  MdAlternativeName path_;
  MdAlternativeName bus_;
  MdString protocol_;
};

}  // namespace mdf


