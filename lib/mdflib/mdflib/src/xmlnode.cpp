/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "xmlnode.h"

#include <sstream>

namespace mdf {

void XmlNode::AppendData(const char *buffer, int len) {
  if (buffer == nullptr || len <= 0) {
    return;
  }
  std::ostringstream temp;
  for (int ii = 0; ii < len; ++ii) {
    if (buffer[ii] != '\0') {
      temp << buffer[ii];
    }
  }
  value_ += temp.str();
}

XmlNode::XmlNode(const std::string &tag_name) : IXmlNode(tag_name) {}

void XmlNode::SetAttribute(const XML_Char **attribute_list) {
  attribute_list_.clear();
  if (attribute_list == nullptr) {
    return;
  }

  std::string key;
  for (int ii = 0; attribute_list[ii] != nullptr; ++ii) {
    if ((ii % 2) == 0) {
      key = attribute_list[ii];
    } else {
      std::string value = attribute_list[ii];
      attribute_list_.insert({key, value});
    }
  }
}

}  // namespace mdf
