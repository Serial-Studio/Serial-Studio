/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */
#include <utility>
#include <limits>

#include "mdf/mdnumber.h"

#include "ixmlnode.h"

namespace mdf {

MdNumber::MdNumber(double number)
: MdStandardAttribute(),
  number_(std::to_string(number)),
  data_type_(MdDataType::MdFloat) {
}

MdNumber::MdNumber(uint64_t number, MdDataType data_type)
: MdStandardAttribute(),
  number_(std::to_string(number)),
  data_type_(data_type) {
  if (data_type_ == MdDataType::MdHex) {
    char temp[30] = {};
    std::snprintf(temp, sizeof(temp), "0x%llX", static_cast<unsigned long long>(number));
    number_ = temp;
  }
}

bool MdNumber::IsActive() const {
  return !number_.empty();
}

void MdNumber::DataType(MdDataType type) {
  data_type_ = type;
}

MdDataType MdNumber::DataType() const {
  return data_type_;
}

void MdNumber::Triggered(bool triggered) {
  triggered_ = triggered;
}

bool MdNumber::Triggered() const {
  return triggered_;
}

void MdNumber::ByteCount(uint64_t byte_count) {
  byte_count_ = byte_count;
}

uint64_t MdNumber::ByteCount() const {
  return byte_count_.has_value() ? byte_count_.value() : 8;
}

void MdNumber::BitMask(uint64_t bit_mask) {
  bit_mask_ = bit_mask;
}

uint64_t MdNumber::BitMask() const {
  return bit_mask_.has_value() ? bit_mask_.value() : std::numeric_limits<uint64_t>::max();
}

void MdNumber::ByteOrder(MdByteOrder byte_order) {
  byte_order_ = byte_order;
}

MdByteOrder MdNumber::ByteOrder() const {
  return byte_order_;
}

void MdNumber::Min(double min) {
  min_ = min;
}

const std::optional<double>& MdNumber::Min() const {
  return min_;
}

void MdNumber::Max(double max) {
  max_ = max;
}

const std::optional<double>& MdNumber::Max() const {
  return max_;
}

void MdNumber::Average(double average) {
  average_ = average;
}

const std::optional<double>& MdNumber::Average() const {
  return average_;
}

void MdNumber::Unit(std::string unit) {
  unit_ = std::move(unit);
}

const std::string& MdNumber::Unit() const {
  return unit_;
}

void MdNumber::UnitRef(std::string unit_ref) {
  unit_ref_ = std::move(unit_ref);
}

const std::string& MdNumber::UnitRef() const {
  return unit_ref_;
}

template <>
void MdNumber::Number(uint64_t value) {
  try {
    switch (data_type_) {
      case MdDataType::MdHex: {
        char temp[30] = {};
        std::snprintf(temp, sizeof(temp), "0x%llX", static_cast<unsigned long long>(value));
        number_ = temp;
        break;
      }

      default:
        number_ = std::to_string(value);
        break;
    }
  } catch (const std::exception&) {
    number_.clear();
  }
}

template <>
uint64_t MdNumber::Number() const {
  try {
    if (number_.size() > 2 && number_.substr(0, 2) == "0x") {
      // HEX decode
      const auto hex_value = number_.substr(2);
      return std::stoull(hex_value, nullptr, 16);
    }
    if (!number_.empty()) {
      return std::stoull(number_);
    }
  } catch (const std::exception&) {}

  return 0;
}

void MdNumber::ToXml(IXmlNode& root_node, const std::string& tag_name) const {
  if (tag_name.empty() || !IsActive()) {
    return;
  }
  auto& node = root_node.AddNode(tag_name);

  MdStandardAttribute::ToXml(node);

  if (byte_count_.has_value() && byte_count_.value() != 8) {
    node.SetAttribute("byte_count", byte_count_.value());
  }
  if (bit_mask_.has_value()) {
    char temp[30] = {};
    std::snprintf(temp, sizeof(temp), "0x%llX", static_cast<unsigned long long>(bit_mask_.value()));
    node.SetAttribute("bit_mask", temp);
  }
  if (byte_order_ != MdByteOrder::LittleEndian) {
    node.SetAttribute("byte_order", std::string("BE"));
  }
  if (min_.has_value()) {
    node.SetAttribute("min", min_.value());
  }
  if (max_.has_value()) {
    node.SetAttribute("max", max_.value());
  }
  if (average_.has_value()) {
    node.SetAttribute("average", average_.value());
  }
  if (!unit_.empty()) {
    node.SetAttribute("unit", unit_);
  }
  if (!unit_ref_.empty()) {
    node.SetAttribute("unit_ref", unit_ref_);
  }
  if (triggered_) {
    node.SetAttribute("triggered", triggered_);
  }
  node.Value(number_);

}

void MdNumber::FromXml(const IXmlNode& number_node) {
  const auto attr_list = number_node.GetAttributeList();
  for (const auto& [key, value] : attr_list) {
    if (key == "ci" && !value.empty()) {
      try {
        ci_ = std::stoll(value);
      } catch (const std::exception_ptr&) {
        ci_ = 0;
      }
    } else if (key == "xml:lang") {
      lang_ = value;
    } else if (key == "byte_count") {
      try {
        byte_count_ = std::stoull(value);
      } catch (const std::exception&) {
        byte_count_.reset();
      }
    } else if (key == "bit_mask") {
      try {
        if (value.size() > 2 && value.substr(0, 2) == "0x") {
          // HEX decode
          const auto hex_value = value.substr(2);
          bit_mask_ = std::stoull(hex_value, nullptr, 16);
        } else if (!value.empty()) {
          bit_mask_ = std::stoull(value);
        }
      } catch (const std::exception&) {
        bit_mask_.reset();
      }
    } else if (key == "byte_order") {
      if (value == "BE") {
        byte_order_ = MdByteOrder::BigEndian;
      }
    } else if (key == "min") {
      try {
        min_ = std::stod(value);
      } catch (const std::exception&) {
        min_.reset();
      }
    } else if (key == "max") {
      try {
        max_ = std::stod(value);
      } catch (const std::exception&) {
        max_.reset();
      }
    } else if (key == "average") {
      try {
        average_ = std::stod(value);
      } catch (const std::exception&) {
        average_.reset();
      }
    } else if (key == "unit") {
      unit_ = value;
    } else if (key == "unit_ref") {
      unit_ref_ = value;
    } else if (key == "triggered") {
      triggered_ = number_node.Attribute(key, false);
    } else {
        AddCustomAttribute(key, value);
    }

  }

  number_ = number_node.Value<std::string>();

}

}  // namespace mdf