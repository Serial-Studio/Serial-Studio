/*
 * Copyright 2023 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

/** \file
 * Defines an interface against a sample reduction (SR) block.
 * The sample reduction block (SR) stores the minimal, maximum and average
 * value for each channel in a CG block.
 */

#pragma once

#include "mdf/iblock.h"
#include <string>
#include "mdf/ichannel.h"
namespace mdf {
class IChannelGroup;

/** \brief Type of master for a sample reduction (SR) block.
 *
 */
enum class SrSyncType : uint8_t {
  Undefined = 0,
  Time = 1,
  Angle = 2,
  Distance = 3,
  Index = 4,
};

namespace SrFlag {

constexpr uint8_t InvalidationByte = 0x01; ///< The block contains an invalidation byte.
constexpr uint8_t DominantBit = 0x02; ///< Dominant invalidation flag.

} // End namespace SrFlag

/** \brief Template class that is used to handle reduction sample.
 *
 * @tparam T Type of value.
 */
template <typename T>
struct SrValue {
  T MeanValue = {}; ///< Mean value.
  T MinValue = {};  ///< Min value.
  T MaxValue = {};  ///< Max value.
  bool MeanValid = false; ///< Mean value valid.
  bool MinValid = false;  ///< Min value valid.
  bool MaxValid = false;  ///< Max value valid.
};

/** \class ISampleReduction mdf/isamplereduction.h isamplereduction.h
 * \brief Defines an interface to a sample reduction (SR) block.
 *
 * Sample reduction (SR) blocks stores min, max and average values for the all
 * samples or for an interval of samples values.
 */
class ISampleReduction : public IBlock {
public:
 /** \brief Sets number of samples in the block.
  *
  * @param nof_samples Number of sample values.
  */
  virtual void NofSamples(uint64_t nof_samples) = 0;

  /** \brief Returns number of samples.
   *
   * @return Number of samples.
   */
  [[nodiscard]] virtual uint64_t NofSamples() const = 0;

  /** \brief Sets the interval value.
   *
   * Sets the interval value. Note that the value is dependent on
   * the synchronization type.
   * @param interval Interval value.
   */
  virtual void Interval(double interval) = 0;

  /** \brief Returns the interval value.
   *
   * @return The interval value.
   */
  [[nodiscard]] virtual double Interval() const = 0;

  /** \brief Synchronization type. Example Time or number of samples.
   *
   * @param type Type of synchronization
   */
  virtual void SyncType(SrSyncType type);

  /** \brief return type of synchronization. */
  [[nodiscard]] virtual SrSyncType SyncType() const;

  virtual void Flags(uint8_t flags); ///< Sets SR flags.
  [[nodiscard]] virtual uint8_t Flags() const; ///< Returns the SR flags.

  /** \brief Returns its channel group. */
  [[nodiscard]] virtual const IChannelGroup* ChannelGroup() const = 0;

  /** \brief Returns the channel value for a specific sample.
   *
   * @tparam T Type of vaöue
   * @param channel Reference to the channel.
   * @param sample Sample index.
   * @param array_index Array index. Only used for an array channel.
   * @param value Returns the SR value.
   */
  template <typename T>
  void GetChannelValue( const IChannel& channel, uint64_t sample,
  uint64_t array_index, SrValue<T>& value ) const;

  /** \brief Returns the scaled SR value.
   *
   * @tparam T Type of value.
   * @param channel Reference to the channel
   * @param sample Sample index.
   * @param array_index Array offset in case of array type.
   * @param value Returns the SR sample value.
   */
  template <typename T>
  void GetEngValue( const IChannel& channel, uint64_t sample,
                    uint64_t array_index, SrValue<T>& value ) const;

  virtual void ClearData() = 0; ///< Resets the internal SR data bytes.

 protected:
  /** brief Returns the value as unsigned integer. */
  virtual void GetChannelValueUint( const IChannel& channel, uint64_t sample,
                                uint64_t array_index, SrValue<uint64_t>& value ) const = 0;

  /** brief Returns the value as an integer. */
  virtual void GetChannelValueInt( const IChannel& channel, uint64_t sample,
                                uint64_t array_index, SrValue<int64_t>& value ) const = 0;
  /** Returns the value as a double value. */
  virtual void GetChannelValueDouble( const IChannel& channel, uint64_t sample,
                                uint64_t array_index, SrValue<double>& value ) const = 0;
};

template<typename T>
void ISampleReduction::GetChannelValue( const IChannel& channel, uint64_t sample,
                                        uint64_t array_index, SrValue<T>& value ) const {
  value = {};
  switch (channel.DataType()) {
    case ChannelDataType::UnsignedIntegerLe:
    case ChannelDataType::UnsignedIntegerBe: {
      SrValue<uint64_t> temp;
      GetChannelValueUint(channel, sample, array_index, temp);
      value.MeanValue = static_cast<T>(temp.MeanValue);
      value.MinValue = static_cast<T>(temp.MinValue);
      value.MaxValue = static_cast<T>(temp.MaxValue);
      value.MeanValid = temp.MeanValid;
      value.MinValid = temp.MinValid;
      value.MaxValid = temp.MaxValid;
      break;
    }

    case ChannelDataType::SignedIntegerLe:
    case ChannelDataType::SignedIntegerBe: {
      SrValue<int64_t> temp;
      GetChannelValueInt(channel, sample, array_index, temp);
      value.MeanValue = static_cast<T>(temp.MeanValue);
      value.MinValue = static_cast<T>(temp.MinValue);
      value.MaxValue = static_cast<T>(temp.MaxValue);
      value.MeanValid = temp.MeanValid;
      value.MinValid = temp.MinValid;
      value.MaxValid = temp.MaxValid;
      break;
    }

    case ChannelDataType::FloatLe:
    case ChannelDataType::FloatBe: {
      SrValue<double> temp;
      GetChannelValueDouble(channel, sample, array_index, temp);
      value.MeanValue = static_cast<T>(temp.MeanValue);
      value.MinValue = static_cast<T>(temp.MinValue);
      value.MaxValue = static_cast<T>(temp.MaxValue);
      value.MeanValid = temp.MeanValid;
      value.MinValid = temp.MinValid;
      value.MaxValid = temp.MaxValid;
      break;
    }

    default:
      break;
  }
}

/** \brief Specialized function that return an unsigned value. */
template<>
void ISampleReduction::GetChannelValue( const IChannel& channel, uint64_t sample,
                 uint64_t array_index, SrValue<std::string>& value) const;



template<typename T>
void ISampleReduction::GetEngValue( const IChannel& channel, uint64_t sample,
                                        uint64_t array_index, SrValue<T>& value ) const {
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
        value.MeanValue = static_cast<T>(temp.MeanValue);
        value.MinValue = static_cast<T>(temp.MinValue);
        value.MaxValue = static_cast<T>(temp.MaxValue);
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
        value.MeanValue = static_cast<T>(temp.MeanValue);
        value.MinValue = static_cast<T>(temp.MinValue);
        value.MaxValue = static_cast<T>(temp.MaxValue);
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
        value.MeanValue = static_cast<T>(temp.MeanValue);
        value.MinValue = static_cast<T>(temp.MinValue);
        value.MaxValue = static_cast<T>(temp.MaxValue);
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

/** \brief Specialized function that returns SR values as strings. */
template<>
void ISampleReduction::GetEngValue( const IChannel& channel, uint64_t sample,
                                    uint64_t array_index, SrValue<std::string>& value ) const;
} // mdf
