/*
 * Copyright 2023 Simplxs
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include "MdfMetaData.h"

using namespace MdfLibrary::ExportFunctions;

namespace MdfLibrary {
class MdfChannelConversion {
 private:
  mdf::IChannelConversion* conversion;

 public:
  MdfChannelConversion(mdf::IChannelConversion* conversion)
      : conversion(conversion) {
    if (conversion == nullptr)
      throw std::runtime_error("MdfChannelConversion Init failed");
  }
  MdfChannelConversion(const mdf::IChannelConversion* conversion)
      : MdfChannelConversion(const_cast<mdf::IChannelConversion*>(conversion)) {
  }
  ~MdfChannelConversion() { conversion = nullptr; }
  int64_t GetIndex() const { return MdfChannelConversionGetIndex(conversion); }
  std::string GetName() const {
    std::string str(MdfChannelConversionGetName(conversion, nullptr) + 1, '\0');
    str.resize(MdfChannelConversionGetName(conversion, str.data()));
    return str;
  }
  void SetName(const char* name) {
    MdfChannelConversionSetName(conversion, name);
  }
  std::string GetDescription() const {
    std::string str(MdfChannelConversionGetDescription(conversion, nullptr) + 1, '\0');
    str.resize(MdfChannelConversionGetDescription(conversion, str.data()));
    return str;
  }
  void SetDescription(const char* desc) {
    MdfChannelConversionSetDescription(conversion, desc);
  }
  std::string GetUnit() const {
    std::string str(MdfChannelConversionGetUnit(conversion, nullptr) + 1, '\0');
    str.resize(MdfChannelConversionGetUnit(conversion, str.data()));
    return str;
  }
  void SetUnit(const char* unit) {
    MdfChannelConversionSetUnit(conversion, unit);
  }
  ConversionType GetType() const {
    return MdfChannelConversionGetType(conversion);
  }
  void SetType(ConversionType type) {
    MdfChannelConversionSetType(conversion, type);
  }
  bool IsPrecisionUsed() {
    return MdfChannelConversionIsPrecisionUsed(conversion);
  }
  uint8_t GetPrecision() const {
    return MdfChannelConversionGetPrecision(conversion);
  }
  bool IsRangeUsed() { return MdfChannelConversionIsRangeUsed(conversion); }
  double GetRangeMin() const {
    return MdfChannelConversionGetRangeMin(conversion);
  }
  double GetRangeMax() const {
    return MdfChannelConversionGetRangeMax(conversion);
  }
  void SetRange(double min, double max) {
    MdfChannelConversionSetRange(conversion, min, max);
  }
  uint16_t GetFlags() const { return MdfChannelConversionGetFlags(conversion); }
  const MdfChannelConversion GetInverse() const {
    return MdfChannelConversionGetInverse(conversion);
  }
  const MdfMetaData GetMetadata() const {
    return MdfChannelConversionGetMetaData(conversion);
  }
  std::string GetReference(uint16_t index) const {
    std::string str(MdfChannelConversionGetReference(conversion, index, nullptr) + 1, '\0');
    str.resize(MdfChannelConversionGetReference(conversion, index, str.data()));
    return str;
  }
  void SetReference(uint16_t index, const char* ref) {
    MdfChannelConversionSetReference(conversion, index, ref);
  }
  double GetParameter(uint16_t index) const {
    return MdfChannelConversionGetParameterAsDouble(conversion, index);
  }
  uint64_t GetParameterUint(uint16_t index) const {
    return MdfChannelConversionGetParameterAsUInt64(conversion, index);
  }
  void SetParameter(uint16_t index, double param) {
    MdfChannelConversionSetParameterAsDouble(conversion, index, param);
  }
  void SetParameter(uint16_t index, uint64_t param) {
    MdfChannelConversionSetParameterAsUInt64(conversion, index, param);
  }
  MdfChannelConversion CreateInverse() {
    return MdfChannelConversionCreateInverse(conversion);
  }
};
}  // namespace MdfLibrary