/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */
#include <utility>

#include "mdf/horevision.h"
#include "mdf/itimestamp.h"

#include "ixmlnode.h"

namespace mdf {

void HoRevision::TeamMemberReference(std::string team_member) {
  team_member_ref_ = std::move(team_member);
}

const std::string& HoRevision::TeamMemberReference() const {
  return team_member_ref_;
}

void HoRevision::RevisionLabel(std::string label) {
  revision_label_ = std::move(label);
}

const std::string& HoRevision::RevisionLabel() const {
  return revision_label_;
}

void HoRevision::State(std::string state) {
  state_ = std::move(state);
}

const std::string& HoRevision::State() const {
  return state_;
}

void HoRevision::Date(std::string date) {
  date_ = std::move(date);
}

const std::string& HoRevision::Date() const {
  return date_;
}

void HoRevision::DateNs(uint64_t ns1970) {
  UtcTimestamp date_time(ns1970);
  date_ = date_time.ToIsoDateTime(false);
}

uint64_t HoRevision::DateNs() const {
  UtcTimestamp date_time(date_);
  return date_time.GetUtcTimeNs();
}

void HoRevision::AddCompanyRevision(HoCompanyRevision revision) {
  company_revision_list_.emplace_back(std::move(revision));
}

const HoCompanyRevisionList& HoRevision::CompanyRevisions() const {
  return company_revision_list_;
}

HoCompanyRevisionList& HoRevision::CompanyRevisions() {
  return company_revision_list_;
}

void HoRevision::AddModification(HoModification modification) {
  modification_list_.emplace_back(std::move(modification));
}

const HoModificationList& HoRevision::Modifications() const {
  return modification_list_;
}

HoModificationList& HoRevision::Modifications() {
  return modification_list_;
}

void HoRevision::ToXml(IXmlNode& root_node) const {
  auto& node = root_node.AddNode("ho:DOC-REVISION");
  if (!team_member_ref_.empty()) {
    auto& member_ref_node = node.AddNode("ho:TEAM-MEMBER-REF");
    member_ref_node.SetAttribute("ID-REF", team_member_ref_);
  }
  if (!revision_label_.empty()) {
    node.SetProperty("ho:REVISION-LABEL", revision_label_);
  }
  if (!state_.empty()) {
    node.SetProperty("ho:STATE", state_);
  }
  if (!date_.empty()) {
    node.SetProperty("ho:DATE", date_);
  }

  if (!company_revision_list_.empty()) {
    auto& revisions_node = node.AddNode("ho:COMPANY-REVISION-INFOS");

    for (const auto& revision : company_revision_list_) {
      auto& revision_node = revisions_node.AddNode("ho:COMPANY-REVISION-INFO");
      if (!revision.DataReference.empty()) {
        revision_node.SetProperty("ho:COMPANY-DATA-REF", revision.DataReference);
      }

      if (!revision.Label.empty()) {
        revision_node.SetProperty("ho:REVISION-LABEL", revision.Label);
      }
      if (!revision.State.empty()) {
        revision_node.SetProperty("ho:STATE", revision.State);
      }
    }
  }

  if (!modification_list_.empty()) {
    auto& modifications_node = node.AddNode("ho:MODIFICATIONS");
    for (const auto& modification : modification_list_) {
      auto& modification_node = modifications_node.AddNode("ho:MODIFICATION");
      modification_node.SetProperty("ho:CHANGE", modification.Change);
      if (!modification.Reason.empty()) {
        modification_node.SetProperty("ho:REASON", modification.Reason);
      }
    }
  }
}

void HoRevision::FromXml(const IXmlNode& revision_node) {
  IXmlNode::ChildList node_list;
  revision_node.GetChildList(node_list);
  for (const IXmlNode* node : node_list) {
    if (node == nullptr) {
      continue;
    }
    if (node->IsTagName("TEAM-MEMBER-REF")) {
      if (team_member_ref_ = node->Attribute<std::string>("ID-REF", "");
          team_member_ref_.empty()) {
        team_member_ref_ = node->Value<std::string>();
      }
    } else if (node->IsTagName("REVISION-LABEL")) {
      revision_label_ = node->Value<std::string>();
    } else if (node->IsTagName("STATE")) {
      state_ = node->Value<std::string>();
    } else if (node->IsTagName("DATE")) {
      date_ = node->Value<std::string>();
    } else if (node->IsTagName("COMPANY-REVISION-INFOS")) {
      IXmlNode::ChildList company_list;
      node->GetChildList(company_list);
      for (const IXmlNode* company_node : company_list) {
        if (company_node != nullptr
            && company_node->IsTagName("COMPANY-REVISION-INFO")) {
          HoCompanyRevision revision;
          if (const auto* data_ref_node = company_node->GetNode("COMPANY-DATA-REF");
              data_ref_node != nullptr ) {
            if (revision.DataReference = data_ref_node->Attribute<std::string>("ID-REF", "");
                revision.DataReference.empty() ) {
              revision.DataReference = data_ref_node->Value<std::string>();
            }
          }

          revision.Label =
              company_node->Property<std::string>("REVISION-LABEL", "");
          revision.State =
              company_node->Property<std::string>("STATE", "");
          AddCompanyRevision(revision);
        }
      }
    } else if (node->IsTagName("MODIFICATIONS")) {
      IXmlNode::ChildList modification_list;
      node->GetChildList(modification_list);
      for (const IXmlNode* modification_node : modification_list) {
        if (modification_node != nullptr
            && modification_node->IsTagName("MODIFICATION")) {
          HoModification modification;
          modification.Change =
              modification_node->Property<std::string>("CHANGE", "");
          modification.Reason =
              modification_node->Property<std::string>("REASON", "");
          AddModification(modification);
        }
      }
    }
  }
}

}  // namespace mdf