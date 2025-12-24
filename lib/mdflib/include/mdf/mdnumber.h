/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once

#include <cstdint>
#include <string>
#include <sstream>
#include <optional>

#include "mdf/mdstandardattribute.h"
#include "mdf/mdproperty.h"

namespace mdf {

class IXmlNode;

enum class MdByteOrder {
  LittleEndian,
  BigEndian
};

class MdNumber : public MdStandardAttribute {
 public:
  MdNumber() = default;
  explicit MdNumber(double number);
  explicit MdNumber(uint64_t number, MdDataType data_type);

  template<typename T>
  operator T() const {// NOLINT(*-explicit-constructor)
    return Number<T>();
  }

  template<typename T>
  bool operator == (const T& value) const {
    return value == Number<T>();
  }

  [[nodiscard]] bool IsActive() const override;

  template <typename T>
  void Number(T value);

  template <typename T>
  [[nodiscard]] T Number() const;

  void DataType(MdDataType type);
  [[nodiscard]] MdDataType DataType() const;

  void Triggered(bool triggered);
  [[nodiscard]] bool Triggered() const;

  void ByteCount(uint64_t byte_count);
  [[nodiscard]] uint64_t ByteCount() const;

  void BitMask(uint64_t bit_mask);
  [[nodiscard]] uint64_t BitMask() const;

  void ByteOrder(MdByteOrder byte_order);
  [[nodiscard]] MdByteOrder ByteOrder() const;

  void Min(double min);
  [[nodiscard]] const std::optional<double>& Min() const;

  void Max(double max);
  [[nodiscard]] const std::optional<double>& Max() const;

  void Average(double average);
  [[nodiscard]] const std::optional<double>& Average() const;

  void Unit(std::string unit);
  [[nodiscard]] const std::string& Unit() const;

  void UnitRef(std::string unit_ref);
  [[nodiscard]] const std::string& UnitRef() const;

  virtual void ToXml(IXmlNode& root_node, const std::string& tag_name) const;
  void FromXml(const IXmlNode& number_node) override;
 private:
  std::string number_;
  bool triggered_ = false;
  MdDataType data_type_ = MdDataType::MdFloat;
  std::optional<uint64_t> byte_count_;
  std::optional<uint64_t> bit_mask_;
  MdByteOrder byte_order_ = MdByteOrder::LittleEndian;
  std::optional<double> min_;
  std::optional<double> max_;
  std::optional<double> average_;
  std::string unit_;
  std::string unit_ref_;
};

template <typename T>
void MdNumber::Number(T value) {
  try {
    number_ = std::to_string(value);
  } catch (const std::exception&) {
    number_.clear();
    number_.shrink_to_fit();
  }
}

template <>
void MdNumber::Number(uint64_t value);

template <typename T>
T MdNumber::Number() const {
  T value = {};
  std::istringstream temp(number_);
  temp >> value;
  return value;
}

template <>
uint64_t MdNumber::Number() const;
}  // namespace mdf

