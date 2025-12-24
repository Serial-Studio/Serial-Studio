/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "MdfChannel.h"
#include "mdflibrary.h"

namespace MdfLibrary {

int64_t MdfChannel::Index::get() {
  return channel_ != nullptr ? channel_->Index() : 0;
}

String^ MdfChannel::Name::get() {
  return channel_ != nullptr ? MdfLibrary::Utf8Conversion(channel_->Name())
                             : gcnew String("");
}

void MdfChannel::Name::set(String^ name) {
  if (channel_ != nullptr) {
    channel_->Name(MdfLibrary::Utf8Conversion(name));
  }
}

String^ MdfChannel::DisplayName::get() {
  return channel_ != nullptr
             ? MdfLibrary::Utf8Conversion(channel_->DisplayName())
             : gcnew String("");
}

void MdfChannel::DisplayName::set(String^ name) {
  if (channel_ != nullptr) {
    channel_->DisplayName(MdfLibrary::Utf8Conversion(name));
  }
}

String^ MdfChannel::Description::get() {
  return channel_ != nullptr
             ? MdfLibrary::Utf8Conversion(channel_->Description())
             : gcnew String("");
}

void MdfChannel::Description::set(String^ desc) {
  if (channel_ != nullptr) {
    channel_->Description(MdfLibrary::Utf8Conversion(desc));
  }
}

bool MdfChannel::UnitUsed::get() {
  return channel_ != nullptr ? channel_->IsUnitValid() : false;
}

String^ MdfChannel::Unit::get() {
  return channel_ != nullptr ? MdfLibrary::Utf8Conversion(channel_->Unit())
                             : gcnew String("");
}

void MdfChannel::Unit::set(String^ unit) {
  if (channel_ != nullptr) {
    channel_->Unit(MdfLibrary::Utf8Conversion(unit));
  }
}

ChannelType MdfChannel::Type::get() {
  const auto temp =
      channel_ != nullptr ? channel_->Type() : mdf::ChannelType::FixedLength;
  return static_cast<ChannelType>(temp);
}

void MdfChannel::Type::set(ChannelType type) {
  if (channel_ != nullptr) {
    channel_->Type(static_cast<mdf::ChannelType>(type));
  }
}

ChannelSyncType MdfChannel::Sync::get() {
  const auto temp =
      channel_ != nullptr ? channel_->Sync() : mdf::ChannelSyncType::Time;
  return static_cast<ChannelSyncType>(temp);
}

void MdfChannel::Sync::set(ChannelSyncType type) {
  if (channel_ != nullptr) {
    channel_->Sync(static_cast<mdf::ChannelSyncType>(type));
  }
}

ChannelDataType MdfChannel::DataType::get() {
  const auto temp = channel_ != nullptr ? channel_->DataType()
                                        : mdf::ChannelDataType::FloatLe;
  return static_cast<ChannelDataType>(temp);
}

void MdfChannel::DataType::set(ChannelDataType type) {
  if (channel_ != nullptr) {
    channel_->DataType(static_cast<mdf::ChannelDataType>(type));
  }
}

CnFlag MdfChannel::Flags::get() {
  auto temp = channel_ != nullptr ? static_cast<CnFlag>(channel_->Flags()) : CnFlag::None;
  return temp;
}

void MdfChannel::Flags::set(CnFlag flags) {
  if (channel_ != nullptr) channel_->Flags(static_cast<uint32_t>(flags));
}

uint64_t MdfChannel::DataBytes::get() {
  return channel_ != nullptr ? channel_->DataBytes() : 0;
}

void MdfChannel::DataBytes::set(uint64_t bytes) {
  if (channel_ != nullptr) {
    channel_->DataBytes(bytes);
  }
}

bool MdfChannel::PrecisionUsed::get() {
  return channel_ != nullptr ? channel_->IsDecimalUsed() : false;
}

unsigned char MdfChannel::Precision::get() {
  return channel_ != nullptr ? channel_->Decimals() : 2;
}

bool MdfChannel::RangeUsed::get() {
  bool used = false;
  if (channel_ != nullptr) {
    const auto optional = channel_->Range();
    used = optional.has_value();
  }
  return used;
}

Tuple<double, double>^ MdfChannel::Range::get() {
  double min = 0, max = 0;
  if (channel_ != nullptr) {
    const auto optional = channel_->Range();
    if (optional.has_value()) {
      min = optional.value().first;
      max = optional.value().second;
    }
  }
  return gcnew Tuple<double, double>(min, max);
}

void MdfChannel::Range::set(Tuple<double, double>^ range) {
  if (channel_ != nullptr) {
    channel_->Range(range->Item1, range->Item2);
  }
}

bool MdfChannel::LimitUsed::get() {
  bool used = false;
  if (channel_ != nullptr) {
    const auto optional = channel_->Limit();
    used = optional.has_value();
  }
  return used;
}

Tuple<double, double>^ MdfChannel::Limit::get() {
  double min = 0, max = 0;
  if (channel_ != nullptr) {
    const auto optional = channel_->Limit();
    if (optional.has_value()) {
      min = optional.value().first;
      max = optional.value().second;
    }
  }
  return gcnew Tuple<double, double>(min, max);
}

void MdfChannel::Limit::set(Tuple<double, double>^ limit) {
  if (channel_ != nullptr) {
    channel_->Limit(limit->Item1, limit->Item2);
  }
}

bool MdfChannel::ExtLimitUsed::get() {
  bool used = false;
  if (channel_ != nullptr) {
    const auto optional = channel_->ExtLimit();
    used = optional.has_value();
  }
  return used;
}

Tuple<double, double>^ MdfChannel::ExtLimit::get() {
  double min = 0, max = 0;
  if (channel_ != nullptr) {
    const auto optional = channel_->ExtLimit();
    if (optional.has_value()) {
      min = optional.value().first;
      max = optional.value().second;
    }
  }
  return gcnew Tuple<double, double>(min, max);
}

void MdfChannel::ExtLimit::set(Tuple<double, double>^ ext_limit) {
  if (channel_ != nullptr) {
    channel_->ExtLimit(ext_limit->Item1, ext_limit->Item2);
  }
}

double MdfChannel::SamplingRate::get() {
  return channel_ != nullptr ? channel_->SamplingRate() : 0;
}

void MdfChannel::SamplingRate::set(double rate) {
  if (channel_ != nullptr) {
    channel_->SamplingRate(rate);
  }
}

uint64_t MdfChannel::VlsdRecordId::get() {
  return channel_ != nullptr ? channel_->VlsdRecordId() : 0;
}

void MdfChannel::VlsdRecordId::set(uint64_t recordId) {
  if (channel_ != nullptr) {
    channel_->VlsdRecordId(recordId);
  }
}

uint32_t MdfChannel::BitCount::get() {
  return channel_ != nullptr ? channel_->BitCount() : 0; 
}

void MdfChannel::BitCount::set(uint32_t bits) {
  if (channel_ != nullptr) {
    channel_->BitCount(bits);
  }
}

uint16_t MdfChannel::BitOffset::get() {
  return channel_ != nullptr ? channel_->BitOffset() : 0; 
}

void MdfChannel::BitOffset::set(uint16_t bits) {
  if (channel_ != nullptr) {
    channel_->BitOffset(bits);
  }
}

MdfSourceInformation^ MdfChannel::SourceInformation::get() {
  auto* temp = channel_ != nullptr ?
    channel_->SourceInformation() : nullptr;
  return temp != nullptr ? gcnew MdfSourceInformation(temp) : nullptr;
}

MdfChannelConversion^ MdfChannel::ChannelConversion::get() {
  auto* conversion =
      channel_ != nullptr ? channel_->ChannelConversion() : nullptr;
  return conversion != nullptr ?
    gcnew MdfChannelConversion(conversion) : nullptr;
}

array<MdfChannel^>^ MdfChannel::ChannelCompositions::get() {
  if (channel_ == nullptr) {
    return gcnew array<MdfChannel^>(0);
  }
  const auto temp = channel_->ChannelCompositions();
  auto list = gcnew array<MdfChannel^>(static_cast<int>(temp.size()));
  for (size_t index = 0; index < temp.size(); ++index) {
    list[static_cast<int>(index)] = gcnew MdfChannel(temp[index]);
  }
  return list;
}

MdfMetaData^ MdfChannel::MetaData::get() {
  auto* temp = channel_ != nullptr ? channel_->MetaData() : nullptr;
  return temp != nullptr ? gcnew MdfMetaData(temp) : nullptr;
}

MdfSourceInformation^ MdfChannel::CreateSourceInformation() {
  auto* temp=channel_ != nullptr ?
    channel_->CreateSourceInformation() : nullptr;
  return  gcnew MdfSourceInformation(temp);
}

MdfChannelConversion^ MdfChannel::CreateChannelConversion() {
  auto* temp =
      channel_ != nullptr ? channel_->CreateChannelConversion() : nullptr;
  return gcnew MdfChannelConversion(temp);
}

MdfChannel^ MdfChannel::CreateChannelComposition() {
  auto* temp =
      channel_ != nullptr ? channel_->CreateChannelComposition() : nullptr;
  return gcnew MdfChannel(temp);  
}

MdfMetaData^ MdfChannel::CreateMetaData() {
  auto* temp = channel_ != nullptr ? channel_->MetaData() : nullptr;
  return gcnew MdfMetaData(temp);   
}

void MdfChannel::SetChannelValue(const int64_t value, bool valid) {
  if (channel_ != nullptr) channel_->SetChannelValue(value, valid);
}

void MdfChannel::SetChannelValue(const uint64_t value, bool valid) {
  if (channel_ != nullptr) channel_->SetChannelValue(value, valid);
}

void MdfChannel::SetChannelValue(const double value, bool valid) {
  if (channel_ != nullptr) channel_->SetChannelValue(value, valid);
}

void MdfChannel::SetChannelValue(String^ value, bool valid) {
  if (channel_ != nullptr)
    channel_->SetChannelValue(MdfLibrary::Utf8Conversion(value), valid);
}

void MdfChannel::SetChannelValue(array<Byte>^ value, bool valid) {
  std::vector<uint8_t> temp(value->Length);
  for (int i = 0; i < value->Length; ++i) temp[i] = value[i];
  if (channel_ != nullptr) channel_->SetChannelValue(temp, valid);
}

MdfChannel::MdfChannel(mdf::IChannel* channel) { channel_ = channel; }

}  // namespace MdfLibrary
