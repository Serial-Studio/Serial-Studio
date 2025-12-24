/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include "mdf/mdextension.h"

#include "ixmlnode.h"

namespace mdf {

MdExtensionPtr MdExtension::Create() const {
  return MdExtensionPtr(new MdExtension);
}

void MdExtension::ToXml(IXmlNode &ext_node) const {
  MdStandardAttribute::ToXml(ext_node);
  // The edd-user needs to override this function
}

void MdExtension::FromXml(const IXmlNode &ext_node) {
  MdStandardAttribute::FromXml(ext_node);
  // The end-user needs to override this function
}

}  // namespace mdf