/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

/** \file isampleobserver.h
 * \brief Interface class to a sample observer. This class is used internally.
 */
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <set>
#include <functional>

#include "mdf/ichannel.h"
namespace mdf {

class IDataGroup;
class IChannelGroup;

/** \brief Interface to a sample observer that handle incoming samples events.
 */
class ISampleObserver {
 public:
  ISampleObserver() = delete;

  /** \brief Sample observer constructor. */
  explicit ISampleObserver(const IDataGroup& data_group);

  virtual ~ISampleObserver(); ///< Destructor

  void DataGroup(const IDataGroup* data_group) const {
    data_group_ = data_group;
  };

  /** \brief Attach the observer to an observer list (publisher).
   *
   * This function is normally called in the constructor of an observer,
   * so there is normally no need to call it.
   */
  virtual void AttachObserver();

  /** \brief Detach the observer from an observer list
   *
   * This function detach the observer from the observer list. This function
   * shall be called when OnSample() function is not needed anymore. This
   * function is needed to handle dangling pointers. For example it is
   * called after the ReadData() function in a reader.
   */
  virtual void DetachObserver();

  /** \brief Observer function that receives the sample record and parse out
   * a channel value.
   *
   * The function may be override by an inheritance for more complex
   * implementations but in simpler use cases the DoOnSample function
   * should be used instead.
   *
   * The function normally returns true indicating that more reading is
   * expected. If any observer want to stop further parsing, the function
   * should return false.
   *
   * @param sample Sample number.
   * @param record_id Record ID (channel group identity).
   * @param record Sample record (excluding the record ID.
   * @return If this function returns false it indicate that reading should be
   * aborted.
   */
  virtual bool OnSample(uint64_t sample, uint64_t record_id,
                        const std::vector<uint8_t>& record);

  /**
   * \brief Function that test if this observer needs to read a specific
   * record.
   *
   * @param record_id The channel groups record ID
   * @return True if this channel subscribe on this record (channel group).
   */
  [[nodiscard]] bool IsRecordIdNeeded(uint64_t record_id) const {
    return record_id_list_.find(record_id) != record_id_list_.cend();
  }

  /** \brief Function object that is called if assigned.
   *
   * The function object is typically assigned to a lambda function.
   * See also OnSample() function.
   */
  std::function<bool(uint64_t sample, uint64_t record_id,
      const std::vector<uint8_t>& record)> DoOnSample;

  /** \brief The function returns a channel value.
   *
   * Use this function to get a channel value from a sample record
   * buffer. Note that the array index should only be used if
   * the channel is a so-called channel array (CA block).
   * @tparam V Type of value.
   * @param channel Reference to a channel.
   * @param sample Sample number (0..N).
   * @param record Record buffer.
   * @param value Reference to the returning value.
   * @param array_index Array index in case of an array channel object.
   * @return True if the value is valid.
   */
  template <typename V>
  bool GetChannelValue(const IChannel& channel,
      uint64_t sample,
      const std::vector<uint8_t>& record,
      V& value,
      uint64_t array_index = 0) const {

    bool valid = false;
    value = {};
    if (data_group_ == nullptr) {
      // If the data group is null, it means that the channel reference is
      // invalid.
      return valid;
    }

    switch (channel.Type()) {
      case ChannelType::VirtualMaster:
      case ChannelType::VirtualData:
        valid = channel.GetVirtualSample(sample, value);
        break;

        // This channel may reference a SD/CG blocks
      case ChannelType::VariableLength:
        if (channel.VlsdRecordId() == 0) {
          // If variable length, the value is an index into an SD block.
          // Value should be in the channels data list (SD or MLSD). The channels
          // GetChannelValue handle this situation
          valid = channel.GetChannelValue(record, value, array_index);
        } else {
          // If variable length, the value is an index into an SD block.
          // Value should be in the channels data list (VLSD CG). The channels
          // GetChannelValue handle this situation
          valid = channel.GetChannelValue(record, value, array_index);
        }
        break;

      case ChannelType::MaxLength:
      case ChannelType::Sync:
      case ChannelType::Master:
      case ChannelType::FixedLength:
      default:
        valid = channel.GetChannelValue(record, value, array_index);
        break;
    }
    return valid;
  }

  /** \brief Returns the scaled sample value for the channel with the record
   * bytes as input. This function is mainly for internal use but is used
   * together with sample observers.
   *
   * The engineering value is the scaled value of the channel value.
   * The attached conversion block defines the type of scaling to apply.
   * Note that the channel value and the scaled value may have different type
   * and unit.
   *
   * @tparam V Type of value to return.
   * @param channel Reference to the channel.
   * @param sample Sample index.
   * @param record Record data bytes excluding the record ID bytes
   * @param value Returns the scaled value.
   * @param array_index Optional array index in case of channel array.
   * @return Return true if the value is valid.
   */
  template <typename V>
  bool GetEngValue(const IChannel& channel,
      uint64_t sample,
      const std::vector<uint8_t>& record,
      V& value,
      uint64_t array_index = 0) const {

    const auto* conversion = channel.ChannelConversion();
    if (conversion == nullptr) {
      return GetChannelValue(channel,sample, record,
          value, array_index);
    }

    bool valid;
    value = {};
    switch (channel.DataType()) {
      case ChannelDataType::UnsignedIntegerLe:
      case ChannelDataType::UnsignedIntegerBe: {
        uint64_t v = 0;
        valid = GetChannelValue(channel,sample,  record, v,
          array_index) && conversion->Convert(v, value);
        break;
      }

      case ChannelDataType::SignedIntegerLe:
      case ChannelDataType::SignedIntegerBe: {
        int64_t v = 0;
        valid = GetChannelValue(channel,sample,  record, v,
          array_index) && conversion->Convert(v, value);
        break;
      }

      case ChannelDataType::FloatLe:
      case ChannelDataType::FloatBe: {
        double v = 0.0;
        valid = GetChannelValue(channel,sample,  record, v,
          array_index) && conversion->Convert(v, value);
        break;
      }

      case ChannelDataType::StringAscii:
      case ChannelDataType::StringUTF16Be:
      case ChannelDataType::StringUTF16Le:
      case ChannelDataType::StringUTF8: {
        std::string v;
        valid = GetChannelValue(channel,sample,  record, v,
          array_index) && conversion->Convert(v, value);
        break;
      }

      default:
        valid = GetChannelValue(channel,sample,  record, value,
          array_index);
        break;
    }
    return valid;
  }

 protected:
  std::set<uint64_t> record_id_list_; ///< List of subscribed channel groups.

  const IDataGroup* DataGroup() const { return data_group_; } ///< Returns the data group.
 private:
  mutable const IDataGroup* data_group_;  ///< Reference to the data group (DG) block.
};



}  // namespace mdf
