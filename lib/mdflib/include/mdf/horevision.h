/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once
#include <cstdint>
#include <vector>
#include <string>

namespace mdf {

class HoRevision;
class IXmlNode;

using HoRevisionList = std::vector<HoRevision>;

struct HoCompanyRevision {
  std::string DataReference;
  std::string Label;
  std::string State;
};
using HoCompanyRevisionList = std::vector<HoCompanyRevision>;

struct HoModification {
  std::string Change;
  std::string Reason;
};
using HoModificationList = std::vector<HoModification>;

class HoRevision {
 public:
  void TeamMemberReference(std::string team_member);
  [[nodiscard]] const std::string& TeamMemberReference() const;

  void RevisionLabel(std::string label);
  [[nodiscard]] const std::string& RevisionLabel() const;

 void State(std::string state);
 [[nodiscard]] const std::string& State() const;

 void Date(std::string date);
 [[nodiscard]] const std::string& Date() const;

 void DateNs(uint64_t ns1970);
 [[nodiscard]] uint64_t DateNs() const;

 void AddCompanyRevision(HoCompanyRevision revision);
 [[nodiscard]] const HoCompanyRevisionList& CompanyRevisions() const;
 [[nodiscard]] HoCompanyRevisionList& CompanyRevisions();


 void AddModification(HoModification modification);
 [[nodiscard]] const HoModificationList& Modifications() const;
 [[nodiscard]] HoModificationList& Modifications();

 void ToXml(IXmlNode& root_node) const;
 void FromXml(const IXmlNode& revision_node);

 private:
  std::string team_member_ref_;
  std::string revision_label_;
  std::string state_;
  std::string date_; ///< ISO date time string

  HoCompanyRevisionList company_revision_list_;
  HoModificationList modification_list_;

};

}  // namespace mdf


