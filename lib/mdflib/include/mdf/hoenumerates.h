/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once

#include <cstddef>
#include <string_view>
#include <string>

namespace mdf {

enum class HoDataType : size_t {
  Uint8 = 0,
  Int8,
  Uint16,
  Int16,
  Uint32,
  Int32,
  Uint64,
  Int64,
  Float32,
  Float64,
  AsciiString,
  Unicode2String,
  ByteField,
  BitField,
  Other
};

std::string_view HoDataTypeToString(HoDataType data_type);
HoDataType StringToHoDataType(const std::string& data_type);

enum class HoIntervalType : size_t {
  Open = 0,
  Closed,
  Infinite
};

std::string_view HoIntervalTypeToString(HoIntervalType type);
HoIntervalType StringToHoIntervalType(const std::string& interval_type);

enum class HoValidity : size_t {
  Valid = 0,
  NotValid,
  NotAvailable,
  NotDefined,
  Error,
  Other,
};

std::string_view HoValidityToString(HoValidity valid);
HoValidity StringToHoValidity(const std::string& valid);

enum class HoDataCategory : size_t {
  LendingLengthInfoType = 0,
  EndOfPdu,
  MinMaxLengthType,
  StandardLengthType
};

std::string_view HoDataCategoryToString(HoDataCategory category);
HoDataCategory StringToHoDataCategory(const std::string& category);

enum class HoDataEncoding : size_t {
  Signed = 0,
  Unsigned,
  Bit,
  IeeeFloatingType,
  Bcd,
  DspFractional,
  Sm,
  BcdP,
  BcdUp,
  OneC,
  TwoC,
  Utf8,
  Ucs2,
  Iso8859_1,
  Iso8859_2,
  Windows1252,
};

std::string_view HoDataEncodingToString(HoDataEncoding encoding);
HoDataEncoding StringToHoDataEncoding(const std::string& encoding);

enum class HoDataTermination : size_t {
  None = 0,
  Zero,
  HexFF,
  Length
};

std::string_view HoDataTerminationToString(HoDataTermination termination);
HoDataTermination StringToHoDataTermination(const std::string& termination);

enum class HoRoleType : size_t {
  Manufacturer = 0,
  Supplier
};

std::string_view HoRoleTypeToString(HoRoleType role);
HoRoleType StringToHoRoleTYpe(const std::string& role);

enum class HoComputationMethodCategory : size_t {
  Identical = 0,
  Linear,
  ScaleLinear,
  TextTable,
  TabNoInterpolation,
  Formula
};

std::string_view HoComputationMethodCategoryToString(
    HoComputationMethodCategory category);
HoComputationMethodCategory StringToHoComputationMethodCategory(
    const std::string& category);

enum class HoUnitGroupCategory : size_t {
  Country = 0,
  EquivUnits,
};

std::string_view HoUnitGroupCategoryToString(
    HoUnitGroupCategory category);
HoUnitGroupCategory StringToHoUnitGroupCategory(
    const std::string& category);

} // end namespace mdf
