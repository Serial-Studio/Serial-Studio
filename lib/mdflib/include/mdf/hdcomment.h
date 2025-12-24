/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once

#include <map>
#include <string>
#include <cstdint>

#include "mdf/mdcomment.h"
#include "mdf/mdstring.h"
#include "mdf/hounitspecification.h"

namespace mdf {

using MdConstantList = std::map<MdString, std::string>;

class HdComment : public MdComment {
 public:
  HdComment();
  explicit HdComment(std::string comment);

  void Author(std::string author);
  [[nodiscard]] std::string Author() const;

  void Department(std::string department);
  [[nodiscard]] std::string Department() const;

  void Project(std::string project);
  [[nodiscard]] std::string Project() const;

  void Subject(std::string subject);
  [[nodiscard]] std::string Subject() const;

  void MeasurementUuid(std::string uuid);
  [[nodiscard]] std::string MeasurementUuid() const;

  void RecorderUuid(std::string uuid);
  [[nodiscard]] std::string RecorderUuid() const;

  void RecorderFileIndex(int64_t index);
  [[nodiscard]] int64_t RecorderFileIndex() const;

  void TimeSource(MdString time_source);
  [[nodiscard]] const MdString& TimeSource() const;

  void AddConstant(MdString constant, std::string expression);
  [[nodiscard]] const MdConstantList& Constants() const;
  [[nodiscard]] MdConstantList& Constants();

  [[nodiscard]] const HoUnitSpecification& UnitSpecification() const;
  [[nodiscard]] HoUnitSpecification& UnitSpecification();

  [[nodiscard]] std::string ToXml() const override;
  void FromXml(const std::string& xml_snippet) override;
 protected:
  MdString time_source_;
  MdConstantList constant_list_;
  HoUnitSpecification unit_spec_;
};

}  // namespace mdf


