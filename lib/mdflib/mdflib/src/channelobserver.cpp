/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "channelobserver.h"

namespace mdf::detail {

template <>
bool ChannelObserver<std::vector<uint8_t>>::GetSampleUnsigned(
    uint64_t sample, uint64_t &value, uint64_t array_index) const {
  const auto array_size = channel_.ArraySize();
  const auto sample_index = static_cast<size_t>( (sample * array_size) + array_index );
  value = 0; // value_list is a byte array
  return sample_index < valid_list_.size() && valid_list_[sample_index];
}

template <>
bool ChannelObserver<std::string>::GetSampleUnsigned(uint64_t sample,
                                                     uint64_t &value,
                                                     uint64_t array_index) const {
  const auto array_size = channel_.ArraySize();
  const auto sample_index = static_cast<size_t>( (sample * array_size) + array_index );
  // Convert the string to an unsigned value
  value = sample_index < value_list_.size() ? std::stoull(value_list_[sample_index]) : 0;
  return sample_index < valid_list_.size() && valid_list_[sample_index];
}

template <>
bool ChannelObserver<std::vector<uint8_t>>::GetSampleSigned(
    uint64_t sample, int64_t &value, uint64_t array_index) const {
  const auto array_size = channel_.ArraySize();
  const auto sample_index = static_cast<size_t>( (sample * array_size) + array_index );
  value = 0; // value_list is a byte array
  return sample_index < valid_list_.size() && valid_list_[sample_index];
}

template <>
bool ChannelObserver<std::string>::GetSampleSigned(uint64_t sample,
                                                   int64_t &value,
                                                   uint64_t array_index) const {
  const auto array_size = channel_.ArraySize();
  const auto sample_index = static_cast<size_t>( (sample * array_size) + array_index );
  value = sample_index < value_list_.size() ? std::stoll(value_list_[sample_index]) : 0;
  return sample_index < valid_list_.size() && valid_list_[sample_index];
}

template <>
bool ChannelObserver<std::vector<uint8_t>>::GetSampleFloat(
    uint64_t sample, double &value, uint64_t array_index) const {
  const auto array_size = channel_.ArraySize();
  const auto sample_index = static_cast<size_t>((sample * array_size) + array_index);
  value = 0.0; // value_list is a byte array
  return sample_index < valid_list_.size() && valid_list_[sample_index];
}

template <>
bool ChannelObserver<std::string>::GetSampleFloat(uint64_t sample,
                                                  double &value,
                                                  uint64_t array_index) const {
  const auto array_size = channel_.ArraySize();
  const auto sample_index = static_cast<size_t>( (sample * array_size) + array_index );
  value = sample_index < value_list_.size() ? std::stod(value_list_[sample_index]) : 0;
  return sample_index < valid_list_.size() && valid_list_[sample_index];
}

template <>
bool ChannelObserver<std::vector<uint8_t>>::GetSampleText(
    uint64_t sample, std::string &value, uint64_t array_index) const {
  const auto array_size = channel_.ArraySize();
  const auto sample_index = static_cast<size_t>( (sample * array_size) + array_index );
  // The value_list consist of byte arrays. Convert o
  std::ostringstream s;
  if (sample_index < value_list_.size()) {
    const auto &list = value_list_[sample_index];
    for (const auto byte : list) {
      s << std::setfill('0') << std::setw(2) << std::uppercase << std::hex
        << static_cast<uint16_t>(byte);
    }
  }
  value = s.str();
  return sample_index < valid_list_.size() && valid_list_[sample_index];
}

template <>
bool ChannelObserver<std::vector<uint8_t>>::GetSampleByteArray(
    uint64_t sample, std::vector<uint8_t> &value) const {
  if (sample < value_list_.size()) {
    value = value_list_[static_cast<size_t>(sample)];
  }
  return sample < valid_list_.size() && valid_list_[static_cast<size_t>(sample)];
}

}  // namespace mdf::detail
