/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "md4block.h"
#include "mdf/mdflogstream.h"
#include "ixmlfile.h"

namespace {

void MakeConstantList(const mdf::IXmlNode &root,
                      mdf::detail::BlockPropertyList &dest) {
  mdf::IXmlNode::ChildList constant_list;
  root.GetChildList(constant_list);
  if (constant_list.empty()) {
    return;
  }
  dest.emplace_back("", "", "", mdf::detail::BlockItemType::BlankItem);
  dest.emplace_back("Constants", "", "",
                    mdf::detail::BlockItemType::HeaderItem);
  for (const auto &c : constant_list) {
    const auto name = c->Attribute<std::string>("name");
    dest.emplace_back(name, c->Value<std::string>());
  }
}

void MakePhysicalDimension(const mdf::IXmlNode &pd,
                           mdf::detail::BlockPropertyList &dest) {
  mdf::IXmlNode::ChildList list;
  pd.GetChildList(list);

  const auto name = pd.Attribute<std::string>("ID");

  std::string label;
  std::ostringstream desc;
  for (const auto &c : list) {
    if (c->IsTagName("SHORT_NAME")) {
      label = c->Value<std::string>();
    } else if (c->IsTagName("DESC")) {
      const auto lang = c->Attribute<std::string>("xml:lang");
      const auto d = c->Value<std::string>();
      if (lang.empty()) {
        if (desc.str().empty()) {
          desc << d;
        } else {
          desc << ", " << d;
        }
      } else {
        if (desc.str().empty()) {
          desc << lang << ":" << d;
        } else {
          desc << ", " << lang << ":" << d;
        }
      }
    }
  }
  dest.emplace_back(label, name, desc.str());
}

void MakePhysicalDimensionList(const mdf::IXmlNode &root,
                               mdf::detail::BlockPropertyList &dest) {
  mdf::IXmlNode::ChildList list;
  root.GetChildList(list);
  if (list.empty()) {
    return;
  }
  dest.emplace_back("", "", "", mdf::detail::BlockItemType::BlankItem);
  dest.emplace_back("Physical Dimensions", "", "",
                    mdf::detail::BlockItemType::HeaderItem);
  for (const auto &c : list) {
    MakePhysicalDimension(*c, dest);
  }
}

void MakeUnitGroup(const mdf::IXmlNode &pd,
                   mdf::detail::BlockPropertyList &dest) {
  mdf::IXmlNode::ChildList list;
  pd.GetChildList(list);

  std::string name;
  std::ostringstream desc;

  for (const auto &c : list) {
    if (c->IsTagName("SHORT_NAME")) {
      name = c->Value<std::string>();
    } else if (c->IsTagName("UNIT-REFS")) {
      mdf::IXmlNode::ChildList ref_list;
      c->GetChildList(ref_list);
      for (const auto &ref : ref_list) {
        const auto id = ref->Attribute<std::string>("ID-REF");
        if (desc.str().empty()) {
          desc << id;
        } else {
          desc << ", " << id;
        }
      }
    }
  }
  dest.emplace_back("Unit Group", name, desc.str());
}

void MakeUnitGroupList(const mdf::IXmlNode &root,
                       mdf::detail::BlockPropertyList &dest) {
  mdf::IXmlNode::ChildList list;
  root.GetChildList(list);
  if (list.empty()) {
    return;
  }
  dest.emplace_back("", "", "", mdf::detail::BlockItemType::BlankItem);
  dest.emplace_back("Unit Groups", "", "",
                    mdf::detail::BlockItemType::HeaderItem);
  for (const auto &c : list) {
    MakeUnitGroup(*c, dest);
  }
}

void MakeUnit(const mdf::IXmlNode &unit, mdf::detail::BlockPropertyList &dest) {
  mdf::IXmlNode::ChildList list;
  unit.GetChildList(list);

  std::ostringstream desc;
  std::string name;
  std::string display_name;
  std::string ref;
  for (const auto &c : list) {
    if (c->IsTagName("SHORT_NAME")) {
      name = c->Value<std::string>();
    } else if (c->IsTagName("DISPLAY-NAME")) {
      display_name = c->Value<std::string>();
    } else if (c->IsTagName("PHYSICAL-DIMENSION-REF")) {
      ref = c->Attribute<std::string>("ID");
    } else if (c->IsTagName("DESC")) {
      const auto lang = c->Attribute<std::string>("xml:lang");
      const auto d = c->Value<std::string>();
      if (lang.empty()) {
        if (desc.str().empty()) {
          desc << d;
        } else {
          desc << ", " << d;
        }
      } else {
        if (desc.str().empty()) {
          desc << lang << ":" << d;
        } else {
          desc << ", " << lang << ":" << d;
        }
      }
    }
  }
  std::ostringstream d;
  d << "Ref: " << ref;

  if (!desc.str().empty()) {
    d << ", " << desc.str();
  }

  dest.emplace_back(name, display_name, d.str());
}

void MakeUnitList(const mdf::IXmlNode &root,
                  mdf::detail::BlockPropertyList &dest) {
  mdf::IXmlNode::ChildList list;
  root.GetChildList(list);
  if (list.empty()) {
    return;
  }
  dest.emplace_back("", "", "", mdf::detail::BlockItemType::BlankItem);
  dest.emplace_back("Units", "", "", mdf::detail::BlockItemType::HeaderItem);
  for (const auto &c : list) {
    MakeUnit(*c, dest);
  }
}

void MakeUnitSpecList(const mdf::IXmlNode &root,
                      mdf::detail::BlockPropertyList &dest) {
  mdf::IXmlNode::ChildList list;
  root.GetChildList(list);
  if (list.empty()) {
    return;
  }
  for (const auto &c : list) {
    if (c->IsTagName("PHYSICAL-DIMENSIONS")) {
      MakePhysicalDimensionList(*c, dest);
    } else if (c->IsTagName("UNITGROUPS")) {
      MakeUnitGroupList(*c, dest);
    } else if (c->IsTagName("UNITS")) {
      MakeUnitList(*c, dest);
    }
  }
}

void MakeCommonProperty(const mdf::IXmlNode &e,
                        mdf::detail::BlockPropertyList &dest) {
  const auto name = e.Attribute<std::string>("name");
  const auto desc = e.Attribute<std::string>("desc");
  auto unit = e.Attribute<std::string>("unit");
  if (unit.empty()) {
    unit = e.Attribute<std::string>("unit_ref");
  }

  std::ostringstream label;
  label << name;
  if (!unit.empty()) {
    label << " [" << unit << "]";
  }

  const auto value = e.Value<std::string>();
  dest.emplace_back(label.str(), value, desc);
}

void MakeTreePropertyList(const mdf::IXmlNode &root,
                          mdf::detail::BlockPropertyList &dest) {
  mdf::IXmlNode::ChildList list;
  root.GetChildList(list);
  if (list.empty()) {
    return;
  }
  dest.emplace_back("", "", "", mdf::detail::BlockItemType::BlankItem);
  dest.emplace_back(root.Attribute<std::string>("name"), "", "",
                    mdf::detail::BlockItemType::HeaderItem);

  // Handle e property first and tree property later
  for (const auto &e : list) {
    if (e->IsTagName("e")) {
      MakeCommonProperty(*e, dest);
    }
  }
  for (const auto &tree : list) {
    if (tree->IsTagName("tree")) {
      MakeTreePropertyList(*tree, dest);
    }
  }
}

void MakeCommonPropertyList(const mdf::IXmlNode &root,
                            mdf::detail::BlockPropertyList &dest) {
  mdf::IXmlNode::ChildList list;
  root.GetChildList(list);
  if (list.empty()) {
    return;
  }
  dest.emplace_back("", "", "", mdf::detail::BlockItemType::BlankItem);
  dest.emplace_back("Properties", "", "",
                    mdf::detail::BlockItemType::HeaderItem);

  // Handle e property first and tree property later
  for (const auto &e : list) {
    if (e->IsTagName("e")) {
      MakeCommonProperty(*e, dest);
    }
  }
  for (const auto &tree : list) {
    if (tree->IsTagName("tree")) {
      MakeTreePropertyList(*tree, dest);
    }
  }
}

}  // namespace

namespace mdf::detail {

Md4Block::Md4Block() {
    block_type_ = "##MD";
}

Md4Block::Md4Block(const std::string &text) {
  if (const bool xml = !text.empty() && text[0] == '<'; xml) {
    block_type_ = "##MD";
  } else {
    block_type_ = "##TX";
  }
  text_ = text;
}

void Md4Block::TxComment(const std::string &tx_comment) {
  block_type_ = "##MD";
  auto xml = CreateXmlFile();
  xml->ParseString(XmlSnippet());
  xml->SetProperty("TX", tx_comment);
  XmlSnippet(xml->WriteString(true));
}

std::string Md4Block::TxComment() const {
  if (IsTxtBlock()) {
    return Tx4Block::TxComment();
  }
  auto xml = CreateXmlFile();
  if (!xml) {
    return {};
  }
  if (bool parse = xml->ParseString(text_); !parse) {
    MDF_ERROR() << "Parse of XML snippet failed. XML: " << text_
      << ", Block Type: " << BlockType();
    return text_;
  };
  return xml->Property<std::string>("TX");

}

void Md4Block::GetBlockProperty(BlockPropertyList &dest) const {
  // If it is a plain text box
  if (IsTxtBlock()) {
    return Tx4Block::GetBlockProperty(dest);
  }
  // Parse out the XML data
  auto xml = CreateXmlFile();
  const bool parse = xml->ParseString(Text());
  if (!parse) {
    return Tx4Block::GetBlockProperty(dest);
  }

  // Check if the XML only include a TX tag. Then its treated as a normal text
  // block.
  IXmlNode::ChildList root_list;
  xml->GetChildList(root_list);
  if (root_list.size() <= 1) {
    if (!TxComment().empty()) {
      dest.emplace_back("Comment", TxComment());
    }
    return;
  }

  // Time to present the metadata items. Note that the HD block is the main
  // (only) holder of metadata
  dest.emplace_back("", "", "", BlockItemType::BlankItem);

  // Present as META data
  dest.emplace_back("Meta Data", "", "", BlockItemType::HeaderItem);

  const auto comment = xml->Property<std::string>("TX");
  if (!comment.empty()) {
    dest.emplace_back("Comment", comment);
  }

  const auto time_source = xml->Property<std::string>("time_source");
  if (!time_source.empty()) {
    dest.emplace_back("Time Source", time_source);
  }

  const auto tool_id = xml->Property<std::string>("tool_id");
  if (!tool_id.empty()) {
    dest.emplace_back("Tool ID", tool_id);
  }

  const auto tool_vendor = xml->Property<std::string>("tool_vendor");
  if (!tool_vendor.empty()) {
    dest.emplace_back("Tool Vendor", tool_vendor);
  }

  const auto tool_version = xml->Property<std::string>("tool_version");
  if (!tool_version.empty()) {
    dest.emplace_back("Tool Version", tool_version);
  }

  const auto user_name = xml->Property<std::string>("user_name");
  if (!user_name.empty()) {
    dest.emplace_back("User Name", user_name);
  }

  const auto pre_trigger = xml->Property<std::string>("pre_trigger_interval");
  if (!pre_trigger.empty()) {
    dest.emplace_back("Pre-Trigger Interval", pre_trigger);
  }

  const auto post_trigger = xml->Property<std::string>("post_trigger_interval");
  if (!post_trigger.empty()) {
    dest.emplace_back("Post-Trigger Interval", post_trigger);
  }

  const auto formula = xml->Property<std::string>("formula");
  if (!formula.empty()) {
    dest.emplace_back("Formula", formula);
  }

  const auto timeout = xml->Property<std::string>("timeout");
  if (!timeout.empty()) {
    dest.emplace_back("Timeout", timeout);
  }

  const auto path = xml->Property<std::string>("path");
  if (!path.empty()) {
    dest.emplace_back("Path", path);
  }

  const auto bus = xml->Property<std::string>("bus");
  if (!bus.empty()) {
    dest.emplace_back("Bus", bus);
  }

  const auto protocol = xml->Property<std::string>("protocol");
  if (!protocol.empty()) {
    dest.emplace_back("Protocol", protocol);
  }

  // List of properties
  const auto *properties = xml->GetNode("common_properties");
  if (properties != nullptr) {
    MakeCommonPropertyList((*properties), dest);
  }
  // List of constants
  const auto *constants = xml->GetNode("constants");
  if (constants != nullptr) {
    MakeConstantList(*constants, dest);
  }

  // List of Units
  const auto *units = xml->GetNode("UNIT-SPEC");
  if (units != nullptr) {
    MakeUnitSpecList(*units, dest);
  }

  const auto *names = xml->GetNode("names");
  if (names != nullptr) {
    IXmlNode::ChildList name_list;
    names->GetChildList(name_list);
    if (!name_list.empty()) {
      dest.emplace_back("", "", "", BlockItemType::BlankItem);
      dest.emplace_back("Alternative Names", "", "", BlockItemType::HeaderItem);
    }
    std::string name;
    std::string display;
    std::string desc;
    std::string vendor;
    for (const auto &n : name_list) {
      if (n->TagName() == "name") {
        if (!name.empty()) {
          std::ostringstream label;
          label << name;

          std::stringstream value;
          value << display;

          std::stringstream description;
          if (!vendor.empty()) {
            description << vendor << ":";
          }
          description << desc;

          dest.emplace_back(label.str(), value.str(), description.str());
        }
        name.clear();
        display.clear();
        desc.clear();
        vendor.clear();
        name = n->Value<std::string>();

      } else if (n->TagName() == "display") {
        display = n->Value<std::string>();
      } else if (n->TagName() == "description") {
        desc = n->Value<std::string>();
      } else if (n->TagName() == "vendor") {
        vendor = n->Value<std::string>();
      }
    }
    if (!name.empty()) {
      std::ostringstream label;
      label << name;

      std::stringstream value;
      value << display;

      std::stringstream description;
      if (!vendor.empty()) {
        description << vendor << ":";
      }
      description << desc;

      dest.emplace_back(label.str(), value.str(), description.str());
    }
  }
}

void Md4Block::XmlSnippet(const std::string &text) { text_ = text; }

const std::string &Md4Block::XmlSnippet() const { return text_; }

}  // namespace mdf::detail