/*
 * Copyright 2023 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "mdf/isamplereduction.h"
#include "mdf/ichannel.h"

namespace mdf {

void ISampleReduction::SyncType(SrSyncType type) {
  // MDF 3  only support time sync type.
}

SrSyncType ISampleReduction::SyncType() const {
  // MDF 3 only support time sync type.
  return SrSyncType::Time;
}

void ISampleReduction::Flags(uint8_t flags) {
  // MDF 3 doesn't have this property.
}

uint8_t ISampleReduction::Flags() const {
  // MDF doesn't have this property.
  return 0;
}

template<>
void ISampleReduction::GetChannelValue( const IChannel& channel, uint64_t sample,
                                        uint64_t array_index, SrValue<std::string>& value ) const {
  value = {};
  switch (channel.DataType()) {
    case ChannelDataType::UnsignedIntegerLe:
    case ChannelDataType::UnsignedIntegerBe: {
      SrValue<uint64_t> temp;
      GetChannelValueUint(channel, sample, array_index, temp);
      value.MeanValue = std::to_string(temp.MeanValue);
      value.MinValue = std::to_string(temp.MinValue);
      value.MaxValue = std::to_string(temp.MaxValue);
      value.MeanValid = temp.MeanValid;
      value.MinValid = temp.MinValid;
      value.MaxValid = temp.MaxValid;
      break;
    }

    case ChannelDataType::SignedIntegerLe:
    case ChannelDataType::SignedIntegerBe: {
      SrValue<int64_t> temp;
      GetChannelValueInt(channel, sample, array_index, temp);
      value.MeanValue = std::to_string(temp.MeanValue);
      value.MinValue = std::to_string(temp.MinValue);
      value.MaxValue = std::to_string(temp.MaxValue);
      value.MeanValid = temp.MeanValid;
      value.MinValid = temp.MinValid;
      value.MaxValid = temp.MaxValid;
      break;
    }

    case ChannelDataType::FloatLe:
    case ChannelDataType::FloatBe: {
      SrValue<double> temp;
      GetChannelValueDouble(channel, sample, array_index, temp);
      value.MeanValue = std::to_string(temp.MeanValue);
      value.MinValue = std::to_string(temp.MinValue);
      value.MaxValue = std::to_string(temp.MaxValue);
      value.MeanValid = temp.MeanValid;
      value.MinValid = temp.MinValid;
      value.MaxValid = temp.MaxValid;
      break;
    }

    default:
      break;
  }
}

template<>
void ISampleReduction::GetEngValue( const IChannel& channel, uint64_t sample,
                                    uint64_t array_index, SrValue<std::string>& value ) const {
  value = {};
  const auto* channel_conversion = channel.ChannelConversion();

  switch (channel.DataType()) {
    case ChannelDataType::UnsignedIntegerLe:
    case ChannelDataType::UnsignedIntegerBe: {
      SrValue<uint64_t> temp;
      GetChannelValueUint(channel, sample, array_index, temp);
      if (channel_conversion != 0) {
        const bool mean_valid = channel_conversion->Convert(temp.MeanValue, value.MeanValue);
        const bool min_valid = channel_conversion->Convert(temp.MinValue, value.MinValue);
        const bool max_valid = channel_conversion->Convert(temp.MaxValue, value.MaxValue);
        value.MeanValid = temp.MeanValid && mean_valid;
        value.MinValid = temp.MinValid && min_valid;
        value.MaxValid = temp.MaxValid && max_valid;
      } else {
        value.MeanValue = std::to_string(temp.MeanValue);
        value.MinValue = std::to_string(temp.MinValue);
        value.MaxValue = std::to_string(temp.MaxValue);
        value.MeanValid = temp.MeanValid;
        value.MinValid = temp.MinValid;
        value.MaxValid = temp.MaxValid;
      }
      break;
    }

    case ChannelDataType::SignedIntegerLe:
    case ChannelDataType::SignedIntegerBe: {
      SrValue<int64_t> temp;
      GetChannelValueInt(channel, sample, array_index, temp);
      if (channel_conversion != 0) {
        const bool mean_valid = channel_conversion->Convert(temp.MeanValue, value.MeanValue);
        const bool min_valid = channel_conversion->Convert(temp.MinValue, value.MinValue);
        const bool max_valid = channel_conversion->Convert(temp.MaxValue, value.MaxValue);
        value.MeanValid = temp.MeanValid && mean_valid;
        value.MinValid = temp.MinValid && min_valid;
        value.MaxValid = temp.MaxValid && max_valid;
      } else {
        value.MeanValue = std::to_string(temp.MeanValue);
        value.MinValue = std::to_string(temp.MinValue);
        value.MaxValue = std::to_string(temp.MaxValue);
        value.MeanValid = temp.MeanValid;
        value.MinValid = temp.MinValid;
        value.MaxValid = temp.MaxValid;
      }
      break;
    }

    case ChannelDataType::FloatLe:
    case ChannelDataType::FloatBe: {
      SrValue<double> temp;
      GetChannelValueDouble(channel, sample, array_index, temp);
      if (channel_conversion != 0) {
        const bool mean_valid = channel_conversion->Convert(temp.MeanValue, value.MeanValue);
        const bool min_valid = channel_conversion->Convert(temp.MinValue, value.MinValue);
        const bool max_valid = channel_conversion->Convert(temp.MaxValue, value.MaxValue);
        value.MeanValid = temp.MeanValid && mean_valid;
        value.MinValid = temp.MinValid && min_valid;
        value.MaxValid = temp.MaxValid && max_valid;
      } else {
        value.MeanValue = std::to_string(temp.MeanValue);
        value.MinValue = std::to_string(temp.MinValue);
        value.MaxValue = std::to_string(temp.MaxValue);
        value.MeanValid = temp.MeanValid;
        value.MinValid = temp.MinValid;
        value.MaxValid = temp.MaxValid;
      }
      break;
    }

    default:
      break;
  }
}

} // mdf