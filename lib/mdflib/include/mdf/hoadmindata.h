/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once

#include "mdf/horevision.h"

namespace mdf {
class IXmlNode;

class HoAdminData {
 public:
  [[nodiscard]] bool IsActive() const;
  void AddRevision(HoRevision revision);
  [[nodiscard]] const HoRevisionList& Revisions() const;
  [[nodiscard]] HoRevisionList& Revisions();

  void ToXml(IXmlNode& root_node) const;
  void FromXml(const IXmlNode& admin_node);

 private:
  HoRevisionList revision_list_;
};

}  // namespace mdf


