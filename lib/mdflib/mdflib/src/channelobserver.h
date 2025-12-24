/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <algorithm>
#include <vector>

#include "mdf/ichannelgroup.h"
#include "mdf/idatagroup.h"
#include "mdf/ichannelobserver.h"


namespace mdf::detail {

template <class T>
class ChannelObserver : public IChannelObserver {
 private:
  uint64_t record_id_ = 0;
  std::vector<T> value_list_;

  const IChannelGroup& group_;       ///< Reference to the channel group (CG) block.


 protected:
  bool GetSampleUnsigned(uint64_t sample, uint64_t& value , uint64_t array_index) const override;

  bool GetSampleSigned(uint64_t sample, int64_t& value, uint64_t array_index) const override;

  bool GetSampleFloat(uint64_t sample, double& value, uint64_t array_index) const override;

  bool GetSampleText(uint64_t sample, std::string& value, uint64_t array_index) const override;

  bool GetSampleByteArray(uint64_t sample,
                          std::vector<uint8_t>& value) const override;

 public:
  ChannelObserver(const IDataGroup& data_group, const IChannelGroup& group,
                  const IChannel& channel)
      : IChannelObserver(data_group,channel),
        group_(group),
        record_id_(group.RecordId())
 {

    const auto array_size = channel_.ArraySize();

    valid_list_.resize(static_cast<size_t>(group_.NofSamples() * array_size), false);
    value_list_.resize(static_cast<size_t>(group.NofSamples() * array_size), T{});

    if (channel_.Type() == ChannelType::VariableLength) {
      offset_list_.resize(static_cast<size_t>(group.NofSamples() * array_size), 0);
    }
    ChannelObserver::AttachObserver();
  }

  ~ChannelObserver() override {
    ChannelObserver::DetachObserver();
  }

  ChannelObserver() = delete;
  ChannelObserver(const ChannelObserver&) = delete;
  ChannelObserver(ChannelObserver&&) = delete;
  ChannelObserver& operator=(const ChannelObserver&) = delete;
  ChannelObserver& operator=(ChannelObserver&&) = delete;

  [[nodiscard]] uint64_t NofSamples() const override {
    // Note that value_list may be an array.
    return group_.NofSamples();
  }

  bool OnSample(uint64_t sample, uint64_t record_id,
                const std::vector<uint8_t>& record) override {
    bool parse_record = record_id == record_id_;
    if (channel_.VlsdRecordId() > 0 &&  record_id == channel_.VlsdRecordId()) {
      parse_record = true;
    }
    if (!parse_record) {
      return true;
    }

    // const auto* channel_array = channel_.ChannelArray();
    const auto array_size = channel_.ArraySize();

    T value{};
    bool valid;
    switch (channel_.Type()) {

      case ChannelType::VirtualMaster:
      case ChannelType::VirtualData:
        if (record_id_ == record_id) {
          for ( uint64_t array_index = 0; array_index < array_size; ++array_index) {
            const auto sample_index = static_cast<size_t>((sample * array_size) + array_index);
            valid = channel_.GetVirtualSample(sample, value);
            if (sample_index < value_list_.size()) {
              value_list_[sample_index] = value;
            }
            if (sample_index < valid_list_.size()) {
              valid_list_[sample_index] = valid;
            }
          }
        }
        break;

        // This channel may reference a SD/CG blocks
      case ChannelType::VariableLength:
        if (record_id_ == record_id) {
          // If variable length, the value is an index into an SD block.
          for (uint64_t array_index = 0; array_index < array_size; ++array_index) {
            uint64_t offset = 0; // Offset into SD/CG block
            valid = channel_.GetUnsignedValue(record, offset, array_index);

            const auto sample_index = static_cast<size_t>((sample * array_size) + array_index);
            if (sample_index < offset_list_.size()) {
              offset_list_[sample_index] = offset;
            }
            if (sample_index < valid_list_.size()) {
              valid_list_[sample_index] = valid;
            }
            if (ReadVlsdData() && channel_.VlsdRecordId() == 0) {
              // Value should be in the channels data list (SD). The channels
              // GetChannelValue handle this situation
              valid = channel_.GetChannelValue(record, value, array_index);
              if (sample_index < valid_list_.size()) {
                valid_list_[sample_index] = valid;
              }
              if (sample_index < value_list_.size()) {
                value_list_[sample_index] = value;
              }
            }
          }
        } else if (channel_.VlsdRecordId() > 0 &&
                   record_id == channel_.VlsdRecordId()) {
          // Add the VLSD offset data to this channel
          for (uint64_t array_index = 0; array_index < array_size; ++array_index) {
            const auto sample_index = static_cast<size_t>((sample * array_size) + array_index);
            valid = channel_.GetChannelValue(record, value, array_index);
            if (sample_index < value_list_.size()) {
              value_list_[sample_index] = value;
            }
            if (sample_index < valid_list_.size()) {
              valid_list_[sample_index] = valid;
            }
          }
        }
        break;

      case ChannelType::MaxLength:
      case ChannelType::Sync:
      case ChannelType::Master:
      case ChannelType::FixedLength:
      default:
        if (record_id_ == record_id) {
          for (uint64_t array_index = 0; array_index < array_size; ++array_index) {
            const auto sample_index = static_cast<size_t>((sample * array_size) + array_index);
            valid = channel_.GetChannelValue(record, value, array_index);
            if (sample_index < value_list_.size()) {
              value_list_[sample_index] = value;
            }
            if (sample_index < valid_list_.size()) {
              valid_list_[sample_index] = valid;
            }
          }
        }
        break;
    }
    return true;
  }

};


template <class T>
bool ChannelObserver<T>::GetSampleUnsigned(uint64_t sample,
                                           uint64_t& value,
                                           uint64_t array_index) const {
  const auto array_size = channel_.ArraySize();
  const auto sample_index = static_cast<size_t>((sample * array_size) + array_index);
  value = sample_index < value_list_.size() ?
      static_cast<uint64_t>(value_list_[sample_index]) : static_cast<uint64_t>(T{});
  return sample_index < valid_list_.size() && valid_list_[sample_index];
}

// Specialization of the above template function just to keep the compiler
// happy. Fetching an unsigned value as a byte array is not supported.
template <>
bool ChannelObserver<std::vector<uint8_t>>::GetSampleUnsigned(
    uint64_t sample, uint64_t& value, uint64_t array_index) const;

template <>
bool ChannelObserver<std::string>::GetSampleUnsigned(uint64_t sample,
                                                     uint64_t& value,
                                                     uint64_t array_index) const;

template <class T>
bool ChannelObserver<T>::GetSampleSigned(uint64_t sample,
                                         int64_t& value,
                                         uint64_t array_index) const {
  const auto array_size = channel_.ArraySize();
  const auto sample_index = static_cast<size_t>((sample * array_size) + array_index);
  value = sample_index < value_list_.size() ? static_cast<int64_t>(value_list_[sample_index]) : 0;
  return sample_index < valid_list_.size() && valid_list_[sample_index];
}

template <>
bool ChannelObserver<std::vector<uint8_t>>::GetSampleSigned(
    uint64_t sample, int64_t& value, uint64_t array_index) const;

template <>
bool ChannelObserver<std::string>::GetSampleSigned(uint64_t sample,
                                                   int64_t& value,
                                                   uint64_t array_index) const;

template <class T>
bool ChannelObserver<T>::GetSampleFloat(uint64_t sample, double& value,
                                        uint64_t array_index) const {
  const auto array_size = channel_.ArraySize();
  const auto sample_index = static_cast<size_t>( (sample * array_size) + array_index);
  value = sample_index < value_list_.size() ? static_cast<double>(value_list_[sample_index]) : 0;
  return sample_index < valid_list_.size() && valid_list_[sample_index];
}

template <>
bool ChannelObserver<std::vector<uint8_t>>::GetSampleFloat(uint64_t sample,
                                                           double& value,
                                                           uint64_t array_index) const;

template <>
bool ChannelObserver<std::string>::GetSampleFloat(uint64_t sample,
                                                  double& value,
                                                  uint64_t array_index) const;

template <class T>
bool ChannelObserver<T>::GetSampleText(uint64_t sample,
                                       std::string& value,
                                       uint64_t array_index) const {
  const auto array_size = channel_.ArraySize();
  const auto sample_index = static_cast<size_t>( (sample * array_size) + array_index );
  try {
    std::ostringstream temp;
    if (sample_index < value_list_.size()) {
      temp << value_list_[sample_index];
    }

    value = temp.str();
    return sample_index < valid_list_.size() && valid_list_[sample_index];
  } catch (const std::exception& ) {}
  return false;
}

template <>
bool ChannelObserver<std::vector<uint8_t>>::GetSampleText(
    uint64_t sample, std::string& value, uint64_t  array_index) const;

// A little bit dirty trick, but it's the specialized function below that
// normally used.
template <class T>
bool ChannelObserver<T>::GetSampleByteArray(uint64_t sample,
                                            std::vector<uint8_t>& value) const {
  value = {};
  return sample < valid_list_.size() && valid_list_[static_cast<size_t>(sample)];
}

// This specialized function is actually doing the work with byte arrays
template <>
bool ChannelObserver<std::vector<uint8_t>>::GetSampleByteArray(
    uint64_t sample, std::vector<uint8_t>& value) const;

}  // namespace mdf::detail