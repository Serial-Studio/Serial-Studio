/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */
#include <utility>

#include "mdf/hoadmindata.h"

#include "ixmlnode.h"

namespace mdf {

bool HoAdminData::IsActive() const {
  return !revision_list_.empty();
}

void HoAdminData::AddRevision(HoRevision revision) {
  revision_list_.emplace_back(std::move(revision));
}
const HoRevisionList& HoAdminData::Revisions() const {
  return revision_list_;
}

HoRevisionList& HoAdminData::Revisions() {
  return revision_list_;
}

void HoAdminData::ToXml(IXmlNode& root_node) const {
  if (!IsActive()) {
    return;
  }
  auto& admin_node = root_node.AddNode("ho:ADMIN-DATA");
  auto& revisions_node = admin_node.AddNode("ho:DOC-REVISIONS");
  for (const auto& revision : revision_list_) {
    revision.ToXml(revisions_node);
  }
}

void HoAdminData::FromXml(const IXmlNode& admin_node) {
  IXmlNode::ChildList node_list;
  admin_node.GetChildList(node_list);
  for (const IXmlNode* node : node_list ) {
    if (node != nullptr && node->IsTagName("DOC-REVISIONS")) {
      IXmlNode::ChildList revision_list;
      node->GetChildList(revision_list);
      for (const IXmlNode* rev_node : revision_list) {
        if (rev_node != nullptr && rev_node->IsTagName("DOC-REVISION")) {
          HoRevision revision;
          revision.FromXml(*rev_node);
          AddRevision(revision);
        }
      }
    }
  }
}

}  // namespace mdf