/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include <mdf/ichannel.h>

#include "MdfChannelObserver.h"
#include "mdflibrary.h"


using namespace System;

namespace MdfLibrary {

MdfChannelObserver::~MdfChannelObserver() {
  this->!MdfChannelObserver();
}


uint64_t MdfChannelObserver::NofSamples::get() {
  return observer_ != nullptr ? observer_->NofSamples() : 0;
}

String^ MdfChannelObserver::Name::get() {
  return observer_ != nullptr ? MdfLibrary::Utf8Conversion(observer_->Name()) : gcnew String("");
}

String^ MdfChannelObserver::Unit::get() {
  return observer_ != nullptr ? MdfLibrary::Utf8Conversion(observer_->Unit()) : gcnew String("");
}

MdfChannel^ MdfChannelObserver::Channel::get() {
  const auto* temp = observer_ != nullptr ?
    &observer_->Channel() : nullptr;
  return temp != nullptr ?
    gcnew MdfChannel(const_cast<mdf::IChannel*>(temp)) : nullptr;
}

bool MdfChannelObserver::IsMaster() {
  return observer_ != nullptr ? observer_->IsMaster() : false;
}

bool MdfChannelObserver::GetChannelValueAsUnsigned(uint64_t sample,
  uint64_t% value) {
  uint64_t temp = 0;
  const auto valid  = observer_ != nullptr ?
    observer_->GetChannelValue(sample, temp) : false;
  value = temp;
  return valid;
}

bool MdfChannelObserver::GetChannelValueAsSigned(uint64_t sample,
  int64_t% value) {
  int64_t temp = 0;
  const auto valid  = observer_ != nullptr ?
    observer_->GetChannelValue(sample, temp) : false;
  value = temp;
  return valid;
}

bool MdfChannelObserver::GetChannelValueAsFloat(uint64_t sample,
  double% value) {
  double temp = 0;
  const auto valid  = observer_ != nullptr ?
    observer_->GetChannelValue(sample, temp) : false;
  value = temp;
  return valid;
}

bool MdfChannelObserver::GetChannelValueAsString(uint64_t sample,
  String^% value) {
  if (observer_ == nullptr ) {
    return false;
  }
  std::string temp;
  const auto valid = observer_->GetChannelValue(sample, temp);
  value = MdfLibrary::Utf8Conversion(temp);
  return valid;
}

bool MdfChannelObserver::GetChannelValueAsArray(uint64_t sample,
  array<Byte>^% value) {
  if (observer_ == nullptr ) {
    return false;
  }
  std::vector<uint8_t> temp;
  const auto valid = observer_->GetChannelValue(sample, temp);
  value = gcnew array<Byte>(static_cast<int>(temp.size()));
  for (size_t index = 0; index < temp.size(); ++index) {
    value[static_cast<int>(index)] = temp[index];
  }
  return valid;
}

bool MdfChannelObserver::GetEngValueAsUnsigned(uint64_t sample, uint64_t% value) {
  uint64_t temp = 0;
  const auto valid  = observer_ != nullptr ?
    observer_->GetEngValue(sample, temp) : false;
  value = temp;
  return valid;
}

bool MdfChannelObserver::GetEngValueAsSigned(uint64_t sample,
  int64_t% value) {
  int64_t temp = 0;
  const auto valid  = observer_ != nullptr ?
    observer_->GetEngValue(sample, temp) : false;
  value = temp;
  return valid;
}

bool MdfChannelObserver::GetEngValueAsFloat(uint64_t sample,
  double% value) {
  double temp = 0;
  const auto valid  = observer_ != nullptr ?
    observer_->GetEngValue(sample, temp) : false;
  value = temp;
  return valid;
}

bool MdfChannelObserver::GetEngValueAsString(uint64_t sample, String^% value) {
  if (observer_ == nullptr ) {
    return false;
  }
  std::string temp;
  const auto valid = observer_->GetEngValue(sample, temp);
  value = MdfLibrary::Utf8Conversion(temp);
  return valid;
}

bool MdfChannelObserver::GetEngValueAsArray(uint64_t sample,
  array<Byte>^% value) {
  if (observer_ == nullptr ) {
    return false;
  }
  // Note that engineering value cannot be byte arrays, so I assume
  // that it was the channel value that was requested.
  std::vector<uint8_t> temp;
  const auto valid = observer_->GetChannelValue(sample, temp);
  value = gcnew array<Byte>(static_cast<int>(temp.size()));
  for (size_t index = 0; index < temp.size(); ++index) {
    value[static_cast<int>(index)] = temp[index];
  }
  return valid;
}

MdfChannelObserver::!MdfChannelObserver() {
  delete observer_;
  observer_ = nullptr;
}

MdfChannelObserver::MdfChannelObserver(mdf::IChannelObserver* observer)
  : observer_(observer) {
  
}
}
