/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */
#include <array>
#include <string_view>
#include "mdf/hoenumerates.h"



namespace {

constexpr std::array<std::string_view, 15> kDataTypeList = {
    "A_UNIT8", "A_INT8", "A_UINT16", "A_UNIT16",
    "A_UNIT32", "A_INT32", "A_UINT64", "A_UNIT64",
    "A_FLOAT32", "A_FLOAT64", "A_ASCIISTRING", "A_UNICODE2STRING",
    "A_BYTEFIELD", "A_BITFIELD", "OTHER"
};

constexpr std::array<std::string_view, 3> kIntervalTypeList = {
    "OPEN", "CLOSED", "INFINITE"
};

constexpr std::array<std::string_view, 6> kValidityList = {
    "VALID", "NOT-VALID", "NOT-AVAILABLE", "NOT-DEFINED", "ERROR", "OTHER"
};

constexpr std::array<std::string_view, 4> kDataCategoryList = {
    "LEADING-LENGTH-INFO-TYPE", "END-OF-PDU",
    "MIN-MAX-LENGTH-TYPE", "STANDARD-LENGTH-TYPE"
};

constexpr std::array<std::string_view, 16> kDataEncodingList = {
    "SIGNED", "UNSIGNED", "BIT", "IEEE-FLOATING-TYPE",
    "BCD", "DSP-FRACTIONAL", "SM", "BCD-P",
    "BCD-UP", "1C", "2C", "UTF-8",
    "UCS-2", "ISO-8859-1", "ISO-8859-2", "WINDOWS-1252"
};

constexpr std::array<std::string_view, 4> kDataTerminationList = {
    "NONE", "ZERO", "HEX-FF", "LENGTH"
};

constexpr std::array<std::string_view, 2> kRoleTypeList = {
    "MANUFACTURER", "SUPPLIER"
};

constexpr std::array<std::string_view, 6> kComputationMethodCategoryList = {
    "IDENTICAL", "LINEAR", "SCALE-LINEAR", "TEXTTABLE",
    "TAB-NOINTP", "FORMULA"
};

constexpr std::array<std::string_view, 2> kUnitGroupCategoryList = {
    "COUNTRY", "EQUIV-UNITS"
};

template<typename T, std::size_t S>
T StringToEnumerate(const std::string& enum_text,
                    const std::array<std::string_view, S>& enum_list,
                    const T default_enum) {
  for (size_t index = 0; index < enum_list.size(); ++index) {
    if (enum_text == enum_list[index]) {
      return static_cast<T>(index);
    }
  }
  return default_enum;
}

}

namespace mdf {

std::string_view HoDataTypeToString(HoDataType data_type) {
  const auto index = static_cast<size_t>(data_type);
  return index < kDataTypeList.size() ? kDataTypeList[index] : "";
}

HoDataType StringToHoDataType(const std::string& data_type) {
  return StringToEnumerate<HoDataType, 15>(data_type, kDataTypeList,
                                       HoDataType::Other);
}


std::string_view HoIntervalTypeToString(HoIntervalType type) {
  const auto index = static_cast<size_t>(type);
  return index < kIntervalTypeList.size() ?  kIntervalTypeList[index] : "";
}

HoIntervalType StringToHoIntervalType(const std::string& interval_type) {
  return StringToEnumerate<HoIntervalType, 3>(interval_type, kIntervalTypeList,
                                           HoIntervalType::Closed);
}

std::string_view HoValidityToString(HoValidity valid) {
  const auto index = static_cast<size_t>(valid);
  return index < kValidityList.size() ?  kValidityList[index] : "";
}

HoValidity StringToHoValidity(const std::string& valid) {
  return StringToEnumerate<HoValidity, 6>(valid, kValidityList,
                                              HoValidity::Valid);
}

std::string_view HoDataCategoryToString(HoDataCategory category) {
  const auto index = static_cast<size_t>(category);
  return index < kDataCategoryList.size() ?
                            kDataCategoryList[index] : "";
}

HoDataCategory StringToHoDataCategory(const std::string& category) {
  return StringToEnumerate<HoDataCategory, 4>(category, kDataCategoryList,
                                          HoDataCategory::StandardLengthType);
}

std::string_view HoDataEncodingToString(HoDataEncoding encoding) {
  const auto index = static_cast<size_t>(encoding);
  return index < kDataEncodingList.size() ? kDataEncodingList[index] : "";
}

HoDataEncoding StringToHoDataEncoding(const std::string& encoding) {
  return StringToEnumerate<HoDataEncoding, 16>(encoding, kDataEncodingList,
                                              HoDataEncoding::Utf8);
}

std::string_view HoDataTerminationToString(HoDataTermination termination) {
  const auto index = static_cast<size_t>(termination);
  return index < kDataTerminationList.size() ? kDataTerminationList[index] : "";
}

HoDataTermination StringToHoDataTermination(const std::string& termination) {
  return StringToEnumerate<HoDataTermination, 4>(termination, kDataTerminationList,
                                               HoDataTermination::None);
}

std::string_view HoRoleTypeToString(HoRoleType role) {
  const auto index = static_cast<size_t>(role);
  return index < kRoleTypeList.size() ? kRoleTypeList[index] : "";
}

HoRoleType StringToHoRoleType(const std::string& role) {
  return StringToEnumerate<HoRoleType, 2>(role, kRoleTypeList,
                                                 HoRoleType::Supplier);
}

std::string_view HoComputationMethodCategoryToString(
    HoComputationMethodCategory category) {
  const auto index = static_cast<size_t>(category);
  return index < kComputationMethodCategoryList.size() ? kComputationMethodCategoryList[index] : "";
}

HoComputationMethodCategory StringToHoComputationMethodCategory(const std::string& category) {
  return StringToEnumerate<HoComputationMethodCategory, 6>(category, kComputationMethodCategoryList,
                                                           HoComputationMethodCategory::Identical);
}

std::string_view HoUnitGroupCategoryToString(
    HoUnitGroupCategory category) {
  const auto index = static_cast<size_t>(category);
  return index < kUnitGroupCategoryList.size() ? kUnitGroupCategoryList[index] : "";
}

HoUnitGroupCategory StringToHoUnitGroupCategory(const std::string& category) {
  return StringToEnumerate<HoUnitGroupCategory, 2>(category, kUnitGroupCategoryList,
                                                   HoUnitGroupCategory::Country);
}
}
