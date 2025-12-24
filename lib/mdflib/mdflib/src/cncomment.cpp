/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */
#include <utility>
#include <string_view>
#include <array>

#include "mdf/cncomment.h"
#include "mdf/mdflogstream.h"

#include "writexml.h"

namespace {
  constexpr std::array<std::string_view,7> kMonotonyList = {
    "MON_DECREASE", "MON_INCREASE", "STRICT_DECREASE", "STRICT_INCREASE",
    "MONOTONOUS", "STRICT_MON", "NOT_MON"
  };

}

namespace mdf {
std::string_view MdMonotonyToString(MdMonotony monotony) {
  const auto index = static_cast<size_t>(monotony);
  return index < kMonotonyList.size() ? kMonotonyList[index] : "";
}

MdMonotony StringToMdMonotony(const std::string& mon_text) {
  for (size_t index = 0; index < kMonotonyList.size(); ++index ) {
    if (mon_text == kMonotonyList[index]) {
      return static_cast<MdMonotony>(index);
    }
  }
  return MdMonotony::NotMono;
}

CnComment::CnComment()
: MdComment("CN") {
  linker_address_.DataType(MdDataType::MdHex);
  raster_.DataType(MdDataType::MdDecimal);
  address_.DataType(MdDataType::MdHex);
}

CnComment::CnComment(std::string comment)
    : CnComment() {
  Comment(std::move(comment));
}

void CnComment::LinkerName(MdString linker_name) {
  linker_name_ = std::move(linker_name);
}

const MdString& CnComment::LinkerName() const {
  return linker_name_;
}

void CnComment::LinkerAddress(MdNumber linker_address) {
  linker_address_ = std::move(linker_address);
  linker_address_.DataType(MdDataType::MdHex);
}

const MdNumber& CnComment::LinkerAddress() const {
  return linker_address_;
}

void CnComment::AxisMonotony(MdMonotony axis_monotony) {
  axis_monotony_ = axis_monotony;
}

MdMonotony CnComment::AxisMonotony() const {
  return axis_monotony_.has_value() ? axis_monotony_.value()
                                    : MdMonotony::NotMono;
}

void CnComment::Raster(MdNumber raster) {
  raster_ = std::move(raster);
}

const MdNumber& CnComment::Raster() const {
  return raster_;
}

void CnComment::Address(MdNumber address) {
  address.DataType(MdDataType::MdHex);
  address_ = std::move(address);
}

const MdNumber& CnComment::Address() const {
  return address_;
}

std::string CnComment::ToXml() const {
  try {
    auto xml_file = CreateXmlFile("FileWriter");
    constexpr bool ho_active = false;
    auto& root_node = CreateRootNode(*xml_file, ho_active);
    ToTx(root_node);
    ToNames(root_node);
    if (linker_name_.IsActive()) {
      linker_name_.ToXml(root_node, "linker_name");
    }
    if (linker_address_.IsActive()) {
      linker_address_.ToXml(root_node, "linker_address");
    }
    if (axis_monotony_.has_value()) {
      root_node.SetProperty("axis_monotony",
                            MdMonotonyToString(axis_monotony_.value()));
    }
    if (raster_.IsActive()) {
      raster_.ToXml(root_node, "raster");
    }
    ToFormula(root_node);

    if (address_.IsActive()) {
      address_.ToXml(root_node, "address");
    }
    ToCommonProp(root_node);
    return xml_file->WriteString(true);
  } catch (const std::exception& err) {
    MDF_ERROR() << "Failed to create the CN comment. Error: " << err.what();
  }
  return {};
}

void CnComment::FromXml(const std::string& xml_snippet) {
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
      if (node->IsTagName("linker_name")) {
        linker_name_.FromXml(*node);
      } else if (node->IsTagName("linker_address")) {
        linker_address_.FromXml(*node);
      } else if (node->IsTagName("axis_monotony")) {
        const std::string mon_text = node->Value<std::string>();
        if (!mon_text.empty()) {
          axis_monotony_ = StringToMdMonotony(mon_text);
        }
      } else if (node->IsTagName("raster")) {
        raster_.FromXml(*node);
      } else if (node->IsTagName("address")) {
        address_.FromXml(*node);
      }
    }
  } catch (const std::exception& err) {
    MDF_ERROR() << "Failed to parse the CN comment. Error: " << err.what();
  }
}


}  // namespace mdf