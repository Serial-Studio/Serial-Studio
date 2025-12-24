/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

/** \file ichannelobserver.h
 * \brief A channel observer is holds a list of channel samples for a
 * channel.
 */
#pragma once
#include <string>
#include <vector>

#include "mdf/ichannel.h"
#include "mdf/isampleobserver.h"


namespace mdf {

/** \brief The channel observer object shall hold all samples for a channel.
 *
 * The main purpose for a channel observer is to store all channel samples for
 * a channel. This object is used when reading data from a MDF file.
 */
class IChannelObserver : public ISampleObserver {
 protected:
   const IChannel& channel_; ///< Reference to the channel (CN) block.
   bool read_vlsd_data_ = true; ///< Defines if the VLSD bytes should be read.

  std::vector<uint64_t> offset_list_; ///< Only used for VLSD channels.
  std::vector<bool> valid_list_; ///< List of valid samples.

  virtual bool GetSampleUnsigned(uint64_t sample, uint64_t& value, uint64_t array_index)
      const = 0; ///< Returns a unsigned  sample value.
  virtual bool GetSampleSigned(uint64_t sample, int64_t& value, uint64_t array_index)
      const = 0; ///< Returns a signed sample value.
  virtual bool GetSampleFloat(uint64_t sample, double& value, uint64_t array_index)
      const = 0; ///< Returns a float sample value.
  virtual bool GetSampleText(uint64_t sample, std::string& value, uint64_t array_index)
      const = 0; ///< Returns a string sample value.
  virtual bool GetSampleByteArray(uint64_t sample, std::vector<uint8_t>& value)
      const = 0; ///< Returns a byte array sample value.

 public:
  explicit IChannelObserver(const IDataGroup& dataGroup, const IChannel& channel); ///< Constructor.

  ~IChannelObserver() override = default; ///< Default destructor.

  IChannelObserver() = delete;
  IChannelObserver(const IChannelObserver&) = delete;
  IChannelObserver(IChannelObserver&&) = delete;
  IChannelObserver& operator=(const IChannelObserver&) = delete;
  IChannelObserver& operator=(IChannelObserver&&) = delete;

  [[nodiscard]] virtual uint64_t NofSamples()
      const = 0; ///< Returns number of samples.

  [[nodiscard]] std::string Name() const; ///< Channel name

  [[nodiscard]] std::string Unit() const; ///< Channel unit.

  /** \brief Returns the channel object.
   *
   * Returns the channel object associated with this observer. The channel
   * is mostly needed to check if the channel is an array channel. Array
   * channels are seldom used and are complex to handle.
   * @return Returns the channel object.
   */
  [[nodiscard]] const IChannel& Channel() const;

  [[nodiscard]] bool IsMaster() const; ///< True if this is the master channel.

  [[nodiscard]] bool IsArray() const; ///< True if this channel is an array channel.

  /** \brief Returns a shape vector that describes an array dimension.
   *
   * This function is used to size an array in an external library. The Eigen
   * and XTensor C++ libraries are typical
   *
   * @return Shape vector that describes an array dimension.
   */
  [[nodiscard]] std::vector<uint64_t> Shape() const;

  /** \brief Property interface that defines if the VLSD raw data should be read
   * or not.
   *
   * Reading VLSD raw data may fail if there is no room for the bytes in the
   * primary memory. Normally is this property normally set to true but if set
   * to false, only the offset into the VLSD block is read. This call should
   * be followed buy one or more call to the MdfWriter::ReadVlsdData() function.
   * By this, only partial samples can be read in and thus saving primary
   * memory.
   * @param read_vlsd_data Set to false if VLSD bytes shouldn't be read.
   */
  void ReadVlsdData(bool read_vlsd_data);

  /** \brief Returns the read VLSD bytes property. */
  [[nodiscard]] bool ReadVlsdData() const { return read_vlsd_data_; }

  /** \brief If this is an array channel, this function returns the array size.
   *
   * Returns the array size if the channel is an array channel. The function
   * returns 1 if not is an array channel.
   * @return
   */
  [[nodiscard]] uint64_t ArraySize() const;

  /** \brief Returns the channel value for a sample.
   *
   * Returns the (unscaled) so-called channel value for a specific sample.
   * @tparam V Type of value
   * @param sample Sample number (0..).
   * @param value The channel value.
   * @param array_index Optional array index if the channel is an array.
   * @return True if value is valid.
   */
  template <typename V>
  bool GetChannelValue(uint64_t sample, V& value, uint64_t array_index = 0 ) const;

  /** \brief Simple function that returns all non-scaled samples.
   *
   * Returns all non-scaled sample values. Do not use this function for array
   * channels. Use the GetChannelValues() function for array values.
   *
   * @tparam V Type of values
   * @param values The destination sample list.
   * @return Returns a list of valid booleans.
   */
  template<typename V>
  std::vector<bool> GetChannelSamples(std::vector<V>& values) const;

  /** \brief Returns a vector of array channel values for a specific sample.
   *
   * The function is intended to be used on channel arrays. It returns all
   * scaled array values for a sample. Note that the valid flags are return
   * by the GetValidList() function.
   *
   * The values vector doesn't need to be sized.
   *
   * If used on non-array channels, it returns a vector of one values.
   * @tparam V Type of value
   * @param sample Sample index.
   * @param values The destination vector of values.
   * @return Returns a vector of valid booleans.
   */
  template<typename V>
  std::vector<bool> GetChannelValues(uint64_t sample,
                                     std::vector<V>& values) const;

  /** \brief Returns the engineering value for a specific value.
   *
   * Returns the engineering (scaled) value for a specific value.
   * @tparam V Type of return value
   * @param sample Sample number (0..).
   * @param value The return value.
   * @param array_index Optional array index if the channel is an array.
   * @return True if the value is valid.
   */
  template <typename V>
  bool GetEngValue(uint64_t sample, V& value, uint64_t array_index = 0) const;

  /** \brief Simple function that returns all scaled samples.
   *
   * Returns all scaled sample values. Do not use this function for array
   * channels. Use the GetChannelValues() function for array values.
   *
   * @tparam V Type of values
   * @param values The destination sample list.
   * @return Returns a list of valid booleans.
   */
  template<typename V>
  std::vector<bool> GetEngSamples(std::vector<V>& values) const;

  /** \brief Returns a vector of array values for a specific sample.
   *
   * The function is intended to be used on channel arrays. It returns all
   * scaled array values for a sample. Note that the valid flags are return
   * by the GetValidList
   *
   * If used on non-array channels, the value vector is of size 1.
   * @tparam V Type of value
   * @param sample Sample index.
   * @param values List of returning array scaled values.
   * @return Returns a list of valid/invalid flags (boolean).
   */
  template<typename V>
  std::vector<bool> GetEngValues(uint64_t sample, std::vector<V>& values) const;

  /** \brief Support function that convert a sample value to a user friendly string
   *
   * Function that convert a sample to a user friendly string. For non-array samples,
   * the function is the same as GetChannelValue but for array sample a JSON array
   * string is returned.
   * @param sample Sample index
   * @return JSON formatted string
   */
  [[nodiscard]] std::string EngValueToString(uint64_t sample) const;

  /** \brief Returns the sample to VLSD offset list.
   *
   * This VLSD offset list is only needed when setting the ReadVLSDData()
   * property to false. By only reading the offset list, the VLSD bytes can be
   * read later as sample by sample or as a range of VSLD bytes. THis read
   * saves primary memory and is much faster if only some samples are needed.
   * @return
   */
  [[nodiscard]] const std::vector<uint64_t>& GetOffsetList() const {
     return offset_list_;
  }

  /** \brief Returns the sample to valid list.  */
  [[nodiscard]] const std::vector<bool>& GetValidList() const {
    return valid_list_;
  }

};

template <typename V>
bool IChannelObserver::GetChannelValue(uint64_t sample, V& value, uint64_t array_index) const {
  bool valid = false;
  value = {};
  switch (channel_.DataType()) {
    case ChannelDataType::CanOpenDate:
    case ChannelDataType::CanOpenTime: {
      // All times are stored as ns since 1970 (uint64_t)
      uint64_t v = 0;  // ns since 1970
      valid = GetSampleUnsigned(sample, v, array_index);
      value = static_cast<V>(v);
      break;
    }

    case ChannelDataType::UnsignedIntegerLe:
    case ChannelDataType::UnsignedIntegerBe: {
      uint64_t v = 0;
      valid = GetSampleUnsigned(sample, v, array_index);
      value = static_cast<V>(v);
      break;
    }

    case ChannelDataType::SignedIntegerLe:
    case ChannelDataType::SignedIntegerBe: {
      int64_t v = 0;
      valid = GetSampleSigned(sample, v, array_index);
      value = static_cast<V>(v);
      break;
    }

    case ChannelDataType::FloatLe:
    case ChannelDataType::FloatBe: {
      double v = 0.0;
      valid = GetSampleFloat(sample, v, array_index);
      value = static_cast<V>(v);
      break;
    }

    case ChannelDataType::StringAscii:
    case ChannelDataType::StringUTF8:
    case ChannelDataType::StringUTF16Le:
    case ChannelDataType::StringUTF16Be: {
      std::string v;
      valid = GetSampleText(sample, v, array_index);
      std::istringstream s(v);
      s >> value;
      break;
    }

    case ChannelDataType::MimeStream:
    case ChannelDataType::MimeSample:
    case ChannelDataType::ByteArray: {
      std::vector<uint8_t> v;
      valid = GetSampleByteArray(sample, v);
      value = static_cast<V>(v.empty() ? V{} : v[0]);
      break;
    }

    default:
      break;
  }
  return valid;
}

/** \brief Returns the sample channel value as a string. */
template <>
bool IChannelObserver::GetChannelValue(uint64_t sample,
                                       std::string& value, uint64_t array_index) const;

/** \brief Returns the sample channel value as a byte array. */
template <>
bool IChannelObserver::GetChannelValue(uint64_t sample,
                                       std::vector<uint8_t>& value, uint64_t array_index) const;

template <typename V>
std::vector<bool> IChannelObserver::GetChannelSamples(
    std::vector<V>& values) const {
  const uint64_t nof_samples = NofSamples();
  std::vector<bool> valid_array(nof_samples, false);
  values.resize(nof_samples, {});
  uint64_t sample = 0;
  for (auto& value : values) {
    valid_array[sample] = GetChannelValue(sample, value, 0);
    ++sample;
  }
  return valid_array;
}

template <typename V>
std::vector<bool> IChannelObserver::GetChannelValues(uint64_t sample,
                                             std::vector<V>& values) const {
  const auto array_size = ArraySize();
  std::vector<bool> valid_array(array_size, false);
  values.resize(array_size, {});
  uint64_t index = 0;
  for (auto& value : values) {
    valid_array[index] = GetChannelValue(sample, value, index);
    ++index;
  }
  return valid_array;
}

template <typename V>
bool IChannelObserver::GetEngValue(uint64_t sample, V& value, uint64_t array_index) const {
  const auto* conversion = channel_.ChannelConversion();
  if (conversion == nullptr) {
    return GetChannelValue(sample, value, array_index);
  }

  bool valid = false;
  value = {};
  switch (channel_.DataType()) {
    case ChannelDataType::UnsignedIntegerLe:
    case ChannelDataType::UnsignedIntegerBe: {
      uint64_t v = 0;
      valid = GetSampleUnsigned(sample, v, array_index);
      if (valid) {
        valid = conversion->Convert(v, value);
      }
      break;
    }

    case ChannelDataType::SignedIntegerLe:
    case ChannelDataType::SignedIntegerBe: {
      int64_t v = 0;
      valid = GetSampleSigned(sample, v, array_index)
        && conversion->Convert(v, value);
      break;
    }

    case ChannelDataType::FloatLe:
    case ChannelDataType::FloatBe: {
      double v = 0.0;
      valid = GetSampleFloat(sample, v, array_index)
        && conversion->Convert(v, value);
      break;
    }

    case ChannelDataType::StringAscii:
    case ChannelDataType::StringUTF16Be:
    case ChannelDataType::StringUTF16Le:
    case ChannelDataType::StringUTF8: {
      std::string v;
      valid = GetSampleText(sample,  v, array_index);
      std::string temp;
      conversion->Convert(v, temp);
      std::istringstream temp1(temp);
      temp1 >> value;
      break;
    }

    default:
      valid = GetChannelValue(sample, value);  // No conversion is allowed;
      break;
  }
  return valid;
}

/** \brief Returns the sample engineering (channel) value as a byte array. */
template <>
bool IChannelObserver::GetEngValue(uint64_t sample,
                                       std::vector<uint8_t>& value,
                                       uint64_t array_index) const;
template <typename V>
std::vector<bool> IChannelObserver::GetEngSamples(
    std::vector<V>& values) const {
  const uint64_t nof_samples = NofSamples();
  std::vector<bool> valid_array(nof_samples, false);
  values.resize(nof_samples, {});
  uint64_t sample = 0;
  for (auto& value : values) {
    valid_array[sample] = GetEngValue(sample, value, 0);
    ++sample;
  }
  return valid_array;
}

template <typename V>
std::vector<bool> IChannelObserver::GetEngValues(uint64_t sample,
                                                 std::vector<V>& values) const {
  const auto array_size = ArraySize();
  std::vector<bool> valid_array(array_size, false);
  values.resize(array_size, {});
  uint64_t index = 0;
  for (auto& value : values) {
    valid_array[index] = GetEngValue(sample, value, index);
    ++index;
  }
  return valid_array;
}

}  // namespace mdf
