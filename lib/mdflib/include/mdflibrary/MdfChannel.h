/*
 * Copyright 2023 Simplxs
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include "MdfChannelConversion.h"
#include "MdfSourceInformation.h"

using namespace MdfLibrary::ExportFunctions;

namespace MdfLibrary {
class MdfChannel {
 private:
  mdf::IChannel* channel;

 public:
  MdfChannel(mdf::IChannel* channel) : channel(channel) {
    if (channel == nullptr) throw std::runtime_error("MdfChannel Init failed");
  }
  MdfChannel(const mdf::IChannel* channel)
      : MdfChannel(const_cast<mdf::IChannel*>(channel)) {}
  ~MdfChannel() { channel = nullptr; }
  mdf::IChannel* GetChannel() const { return channel; }
  int64_t GetIndex() const { return MdfChannelGetIndex(channel); }
  std::string GetName() const {
    std::string str(MdfChannelGetName(channel, nullptr) + 1, '\0');
    str.resize(MdfChannelGetName(channel, str.data()));
    return str;
  }
  void SetName(const char* name) { MdfChannelSetName(channel, name); }
  std::string GetDisplayName() const {
    std::string str(MdfChannelGetDisplayName(channel, nullptr) + 1, '\0');
    str.resize(MdfChannelGetDisplayName(channel, str.data()));
    return str;
  }
  void SetDisplayName(const char* name) {
    MdfChannelSetDisplayName(channel, name);
  }
  std::string GetDescription() const {
    std::string str(MdfChannelGetDescription(channel, nullptr) + 1, '\0');
    str.resize(MdfChannelGetDescription(channel, str.data()));
    return str;
  }
  void SetDescription(const char* desc) {
    MdfChannelSetDescription(channel, desc);
  }
  bool IsUnitUsed() { return MdfChannelIsUnitUsed(channel); }
  std::string GetUnit() const {
    std::string str(MdfChannelGetUnit(channel, nullptr) + 1, '\0');
    str.resize(MdfChannelGetUnit(channel, str.data()));
    return str;
  }
  void SetUnit(const char* unit) { MdfChannelSetUnit(channel, unit); }
  ChannelType GetType() const { return MdfChannelGetType(channel); }
  void SetType(ChannelType type) { MdfChannelSetType(channel, type); }
  ChannelSyncType GetSync() const { return MdfChannelGetSync(channel); }
  void SetSync(ChannelSyncType type) { MdfChannelSetSync(channel, type); }
  ChannelDataType GetDataType() const { return MdfChannelGetDataType(channel); }
  void SetDataType(ChannelDataType type) {
    MdfChannelSetDataType(channel, type);
  }
  uint32_t GetFlags() const { return MdfChannelGetFlags(channel); }
  void SetFlags(uint32_t flags) { MdfChannelSetFlags(channel, flags); }
  uint64_t GetDataBytes() const { return MdfChannelGetDataBytes(channel); }
  void SetDataBytes(uint64_t bytes) { MdfChannelSetDataBytes(channel, bytes); }
  bool IsPrecisionUsed() { return MdfChannelIsPrecisionUsed(channel); }
  uint8_t GetPrecision() const { return MdfChannelGetPrecision(channel); }
  bool IsRangeUsed() { return MdfChannelIsRangeUsed(channel); }
  double GetRangeMin() const { return MdfChannelGetRangeMin(channel); }
  double GetRangeMax() const { return MdfChannelGetRangeMax(channel); }
  void SetRange(double min, double max) {
    MdfChannelSetRange(channel, min, max);
  }
  bool IsLimitUsed() { return MdfChannelIsLimitUsed(channel); }
  double GetLimitMin() const { return MdfChannelGetLimitMin(channel); }
  double GetLimitMax() const { return MdfChannelGetLimitMax(channel); }
  void SetLimit(double min, double max) {
    MdfChannelSetLimit(channel, min, max);
  }
  bool IsExtLimitUsed() { return MdfChannelIsExtLimitUsed(channel); }
  double GetExtLimitMin() const { return MdfChannelGetExtLimitMin(channel); }
  double GetExtLimitMax() const { return MdfChannelGetExtLimitMax(channel); }
  void SetExtLimit(double min, double max) {
    MdfChannelSetExtLimit(channel, min, max);
  }
  double GetSamplingRate() const { return MdfChannelGetSamplingRate(channel); }
  uint64_t GetVlsdRecordId() const {
    return MdfChannelGetVlsdRecordId(channel);
  }
  void SetVlsdRecordId(uint64_t record_id) {
    MdfChannelSetVlsdRecordId(channel, record_id);
  }
  uint32_t GetBitCount() const { return MdfChannelGetBitCount(channel); }
  void SetBitCount(uint32_t bits) { MdfChannelSetBitCount(channel, bits); }
  uint16_t GetBitOffset() const { return MdfChannelGetBitOffset(channel); }
  void SetBitOffset(uint16_t bits) { MdfChannelSetBitOffset(channel, bits); }
  const MdfMetaData GetMetaData() const {
    return MdfChannelGetMetaData(channel);
  }
  const MdfSourceInformation GetSourceInformation() const {
    return MdfChannelGetSourceInformation(channel);
  }
  const MdfChannelConversion GetChannelConversion() const {
    return MdfChannelGetChannelConversion(channel);
  }
  std::vector<MdfChannel> GetChannelCompositions() {
    size_t count = MdfChannelGetChannelCompositions(channel, nullptr);
    if (count <= 0) return {};
    auto pChannels = new mdf::IChannel*[count];
    MdfChannelGetChannelCompositions(channel, pChannels);
    std::vector<MdfChannel> channelss;
    channelss.reserve(count);
    for (size_t i = 0; i < count; i++) channelss.emplace_back(pChannels[i]);
    delete[] pChannels;
    return channelss;
  }
  MdfMetaData CreateMetaData() { return MdfChannelCreateMetaData(channel); }
  MdfSourceInformation CreateSourceInformation() {
    return MdfChannelCreateSourceInformation(channel);
  }
  MdfChannelConversion CreateChannelConversion() {
    return MdfChannelCreateChannelConversion(channel);
  }
  MdfChannel CreateChannelComposition() {
    return MdfChannelCreateChannelComposition(channel);
  }
  void SetChannelValue(const int64_t value, bool valid = true) {
    MdfChannelSetChannelValueAsSigned(channel, value, valid);
  }
  void SetChannelValue(const uint64_t value, bool valid = true) {
    MdfChannelSetChannelValueAsUnSigned(channel, value, valid);
  }
  void SetChannelValue(const double value, bool valid = true) {
    MdfChannelSetChannelValueAsFloat(channel, value, valid);
  }
  void SetChannelValue(const char* value, bool valid = true) {
    MdfChannelSetChannelValueAsString(channel, value, valid);
  }
  void SetChannelValue(const uint8_t* value, size_t size, bool valid = true) {
    MdfChannelSetChannelValueAsArray(channel, value, size, valid);
  }
};
}  // namespace MdfLibrary
