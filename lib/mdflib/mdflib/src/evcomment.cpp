/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */
#include <utility>

#include "mdf/evcomment.h"
#include "mdf/mdflogstream.h"

#include "writexml.h"

namespace mdf {

EvComment::EvComment()
: MdComment("EV") {

}

EvComment::EvComment(std::string comment)
    : EvComment() {
  Comment(std::move(comment));
}

void EvComment::PreTriggerInterval(MdNumber interval) {
  pre_trigger_interval_ = std::move(interval);
}

const MdNumber& EvComment::PreTriggerInterval() const {
  return pre_trigger_interval_;
}

void EvComment::PostTriggerInterval(MdNumber interval) {
  post_trigger_interval_ = std::move(interval);
}

const MdNumber& EvComment::PostTriggerInterval() const {
  return post_trigger_interval_;
}

void EvComment::Timeout(MdNumber timeout) {
  timeout_ = std::move(timeout);
}

const MdNumber& EvComment::Timeout() const {
  return timeout_;
}

std::string EvComment::ToXml() const {
  try {
    auto xml_file = CreateXmlFile("FileWriter");
    constexpr bool ho_active = false;
    auto& root_node = CreateRootNode(*xml_file, ho_active);
    ToTx(root_node);
    if (pre_trigger_interval_.IsActive()) {
      pre_trigger_interval_.ToXml(root_node, "pre_trigger_interval");
    }
    if (post_trigger_interval_.IsActive()) {
      post_trigger_interval_.ToXml(root_node, "post_trigger_interval");
    }
    ToFormula(root_node);
    if (timeout_.IsActive()) {
      timeout_.ToXml(root_node, "timeout");
    }
    ToCommonProp(root_node);
    return xml_file->WriteString(true);
  } catch (const std::exception& err) {
    MDF_ERROR() << "Failed to create the EV comment. Error: " << err.what();
  }
  return {};
}

void EvComment::FromXml(const std::string& xml_snippet) {
  if (xml_snippet.empty()) {
    return;
  }
  try {
    auto xml_file = CreateXmlFile("Expat");
    if (!xml_file) {
      throw std::runtime_error("Failed to create EXPAT parser object");
    }
    const bool parse = xml_file->ParseString(xml_snippet);
    if (!parse) {
      throw std::runtime_error("Failed to parse the XML string.");
    }
    const auto* root_node = xml_file->RootNode();
    if (root_node == nullptr) {
      throw std::runtime_error("There is no root node in the XML string");
    }
    MdComment::FromXml(*root_node);

    IXmlNode::ChildList node_list;
    xml_file->GetChildList(node_list);
    for (const IXmlNode* node : node_list) {
      if (node == nullptr) {
        continue;
      }
      if (node->IsTagName("pre_trigger_interval")) {
        pre_trigger_interval_.FromXml(*node);
      } else if (node->IsTagName("post_trigger_interval")) {
        post_trigger_interval_.FromXml(*node);
      } else if (node->IsTagName("timeout")) {
        timeout_.FromXml(*node);
      }
    }
  } catch (const std::exception& err) {
    MDF_ERROR() << "Failed to parse the EV comment. Error: " << err.what();
  }
}


}  // namespace mdf