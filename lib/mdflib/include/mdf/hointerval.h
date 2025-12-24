/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once

#include <optional>
#include "mdf/hoenumerates.h"

namespace mdf {

class IXmlNode;

class HoInterval {
 public:
  [[nodiscard]] bool IsActive() const;

  void Limit(double limit);
  [[nodiscard]] double Limit() const;

  void Type(HoIntervalType type);
  [[nodiscard]] HoIntervalType Type() const;

  void ToXml(IXmlNode& root_node, const std::string_view& tag_name) const;
  void FromXml(const IXmlNode& limit_node);
 private:
  std::optional<double> limit_;
  HoIntervalType type_ = HoIntervalType::Closed;
};

}  // namespace mdf


