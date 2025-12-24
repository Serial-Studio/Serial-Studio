/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
/** \file xmlnode.h
 * \brief Implement an XML node.
 */
#pragma once
#include <expat.h>

#include <memory>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

#include "ixmlfile.h"
#include "ixmlnode.h"

namespace mdf {

/** \class XmlNode xmlnode.h "xmlnode.h"
 * \brief Implement an XML node
 */
class XmlNode : public IXmlNode {
 public:
  explicit XmlNode(const std::string &tag);  ///< Constructor
  ~XmlNode() override = default;             ///< Default destructor
  XmlNode() = delete;  ///< Block usage of default constructor

  void AppendData(const char *buffer,
                  int len);  ///< Append data to the node. Expat parser data.
  void SetAttribute(const XML_Char **attribute_list);  ///< Sets the attributes.
                                                       ///< Expat parser data.
};
}  // namespace mdf
