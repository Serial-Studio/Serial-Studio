/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */
#include <string_view>
#include <array>
#include <ctime>
#include <sstream>

#include "mdf/mdproperty.h"
#include "mdf/itimestamp.h"
#include "mdf/mdflogstream.h"

#include "ixmlnode.h"
namespace {

constexpr std::array<std::string_view, 9> kDataTypeList = {
  "string", "decimal", "integer", "float",
  "boolean", "date", "time", "dateTime",
  "integer"
};

}
namespace mdf {

MdProperty::MdProperty(std::string property, std::string value)
: name_(std::move(property)),
  value_(std::move(value)) {

}

void MdProperty::Name(std::string name) {
    name_ = std::move(name);
  }

  const std::string& MdProperty::Name() const {
    return name_;
  }

  void MdProperty::Description(std::string description) {
    desc_ = std::move(description);
  }

  const std::string& MdProperty::Description() const {
    return desc_;
  }

  void MdProperty::Unit(std::string unit) {
    unit_ = std::move(unit);
  }

  const std::string& MdProperty::Unit() const {
    return unit_;
  }

  void MdProperty::UnitReference(std::string unit_reference) {
    unit_ref_ = std::move(unit_reference);
  }

  const std::string& MdProperty::UnitReference() const {
    return unit_ref_;
  }

  void MdProperty::DataType(MdDataType data_type) {
    data_type_ = data_type;
  }

  MdDataType MdProperty::DataType() const {
    return data_type_;
  }

  void MdProperty::ReadOnly(bool read_only) {
    read_only_ = read_only;
  }

  bool MdProperty::ReadOnly() const {
    return read_only_;
  }

  void MdProperty::ToXml(IXmlNode& prop_node) const {

    if (!name_.empty()) {
      prop_node.SetAttribute("name", name_);
    } else {
      MDF_ERROR() << "The XML tag requires an name attribute. Tag: "
        << prop_node.TagName();
    }

    if (!desc_.empty()) {
      prop_node.SetAttribute("desc", desc_ );
    }

    if (data_type_ != MdDataType::MdString) {
      prop_node.SetAttribute("type", DataTypeToString());
    }

    if (!unit_.empty()) {
      prop_node.SetAttribute("unit", unit_);
    }

    if (!unit_ref_.empty()) {
      prop_node.SetAttribute("unit_ref", unit_ref_);
    }

    if (read_only_) {
      prop_node.SetAttribute("ro", read_only_);
    }
    MdStandardAttribute::ToXml(prop_node);

    // The property value is saved to the XML
    // but not for an elist property as the
    // value is a list of values.
    if (!prop_node.IsTagName("elist")) {
      prop_node.Value(value_);
    }

  }

  std::string_view MdProperty::DataTypeToString() const {
    if (const auto type = static_cast<uint16_t>(data_type_);
        type < kDataTypeList.size() ) {
      return kDataTypeList[type];
    }
    return kDataTypeList[0];
  }

  void MdProperty::StringToDataType(const std::string& text) {
    if (text.empty()) {
      data_type_ = MdDataType::MdString;
      return;
    }

    for (size_t type = 0; type < kDataTypeList.size(); ++type) {
      if (text == kDataTypeList[type]) {
        data_type_ = static_cast<MdDataType>(type);
        return;
      }
    }
    data_type_ = MdDataType::MdString;
  }

  void MdProperty::FromXml(const IXmlNode& prop_node) {
    MdStandardAttribute::FromXml(prop_node);
    name_ = prop_node.Attribute<std::string>("name", "");
    desc_ = prop_node.Attribute<std::string>("desc", "");
    const std::string type = prop_node.Attribute<std::string>("type", "");
    StringToDataType(type);
    read_only_ = prop_node.Attribute<bool>("ro", false);
    unit_ = prop_node.Attribute<std::string>("unit", "");
    unit_ref_ = prop_node.Attribute<std::string>("unit_ref", "");
    if (!prop_node.IsTagName("elist")) {
      value_ = prop_node.Value<std::string>();
    }
 }

  template <>
  void MdProperty::Value(std::string value) {
    value_ = std::move(value);
  }

  template <>
  void MdProperty::Value(bool value) {
    value_ = value ? "true" : "false";
  }

  template <>
  void MdProperty::Value(uint64_t value) {
    // Handle if value is a Date/Time or DateTime
    switch (DataType()) {
      case MdDataType::MdDate: {
        UtcTimestamp temp(value);
        value_ = temp.ToIsoDate();  // YYYY-MM-DDZ
        break;
      }

      case MdDataType::MdTime: {
        UtcTimestamp temp(value);
        value_ = temp.ToIsoTime(true);  // HH:MM:SS.xxxxx
        break;
      }

      case MdDataType::MdDateTime: {
        UtcTimestamp temp(value);
        value_ = temp.ToIsoDateTime(true);  // YYYY-MM-DDTHH:MM:SS.xxxxxZ
        break;
      }

      case MdDataType::MdBoolean:
        value_ = value > 0 ? "true" : "false";
        break;

      case MdDataType::MdHex: {
        char temp[30] = {};
        const auto tempX = static_cast<long long unsigned int>(value);
        std::snprintf(temp, sizeof(temp), "0x%llX", tempX);
        value_ = temp;
        break;
      }

      default:
        value_ = std::to_string(value);
        break;
    }
  }

  template <>
  std::string MdProperty::Value() const {
    return value_;
  }

  template <>
  uint64_t MdProperty::Value() const {
    try {
      switch (DataType()) {
        case MdDataType::MdInteger:
          if (value_.size() > 2 && value_.substr(0,2) == "0x" ) {
            // HEX decode
            const auto hex_value = value_.substr(2);
            return std::stoull(hex_value, nullptr, 16);
          }
          break;

        case MdDataType::MdDateTime: {
          UtcTimestamp timestamp;
          timestamp.FromIsoDateTime(value_);
          return timestamp.GetUtcTimeNs();
        }

        default:
          break;
      }

      if (!value_.empty()) {
        return std::stoull(value_);
      }

    } catch (const std::exception& ) {}
    return 0;
  }

  template <>
  bool MdProperty::Value() const {
    if (value_.empty()) {
      return false;
    }
    switch (value_[0]) {
      case '1':
      case 't':
      case 'T':
        return true;

      default:
        break;
    }
    return false;
  }


  }  // namespace mdf