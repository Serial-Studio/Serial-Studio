/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once

#include <cstdint>
#include <string_view>
#include <string>

#include "mdf/mdfenumerates.h"

namespace mdf {

class MdfWriter;
class IChannel;
class IChannelGroup;
class IDataGroup;
class ISourceInformation;

enum class MessageFilter : int {
  NoFilter = 0,
  ChannelFilter,
  MessageIdFilter
};

class IConfigAdapter {
 public:
    virtual ~IConfigAdapter() = default;
    IConfigAdapter() = delete;
  explicit IConfigAdapter(const MdfWriter& writer);

  void BusType(uint16_t bus_type);
  [[nodiscard]] uint16_t BusType() const;

  [[nodiscard]] const std::string& BusName() const;

  void Protocol(std::string protocol);
  [[nodiscard]] const std::string& Protocol() const;

  void FilterType(MessageFilter type);
  [[nodiscard]] MessageFilter FilterType() const;

  void ChannelFilter(uint8_t channel);
  [[nodiscard]] uint8_t ChannelFilter() const;

  void IdFilter(uint64_t message_id);
  [[nodiscard]] uint64_t IdFilter() const;

  void MessageName(std::string message_name);
  [[nodiscard]] const std::string& MessageName() const;

  void MessageLength(uint64_t message_length);
  [[nodiscard]] uint64_t MessageLength() const;

  void NetworkName(std::string network_name);
  [[nodiscard]] const std::string& NetworkName() const;

  void DeviceName(std::string device_name);
  [[nodiscard]] const std::string& DeviceName() const;

  virtual void CreateConfig(IDataGroup& dg_block) = 0;

    /** \brief Defines the type of variable length storage.
     *
     * Defines how the raw variable length data (byte array) is stored. It is
     * mainly used when storing bus messages.
     *
     * By default the fixed length storage is used. This means that the data
     * records have fixed length. This is the traditional way of storage.
     * Variable length channels as strings and byte arrays are stored in
     * separate signal data blocks (SD) and the fixed length record, stores
     * a 64-bit offset into the SD block..
     *
     * The Variable Length Signal Data (VLSD) is similar to fixed length
     * storage but the signal data is stored in a separate channel group.
     * The extra channel group, doesn't have any channels. By this option,
     * all data can be appended directly instead of storing the signal
     * data in temporary in memory.
     *
     * The Maximum Length Signal Data is used when the raw data bytes doesn't
     * exceed 8 bytes. Instead of storing an 64-bit offset, it stores the raw
     * bytes and the number of bytes in a separate channel.
     * @param storage_type Type of storage.
     */
  void StorageType(MdfStorageType storage_type) {
    storage_type_ = storage_type;
  }

  /** \brief Define only mandatory members.
   *
   * This option defines if only mandatory members should be automatic
   * generated.
   * This property is only used for bus configurations.
   * When the property is true, only data frames are saved.
   *
   * It's questionable if this property should be used for on-line
   * loggers but if only correct data frames are of interest, the
   * property can be set to true.
   * @param mandatory_only If set true, only mandatory members are created.
   */
  void MandatoryMembersOnly(bool mandatory_only) {
    mandatory_members_only_ = mandatory_only;
  }
  [[nodiscard]] bool MandatoryMembersOnly() const {
    return mandatory_members_only_;
  }

  /** \brief Sets max number of payload data bytes.
   *
   * This setting is used for MLSD storage type. It defines the max number of
   * data bytes that can be stored. Default set to  8 bytes which in reality
   * is the only valid value. Actual number of bytes are defines in a separate
   * channel.
   *
   * Standard CAN and LIN max 8 byte of data while CAN FD can have up to
   * 64 data bytes.
   * @param max_length Maximum number of payload data bytes.
   */
  void MaxLength(uint32_t max_length) {max_length_ = max_length;};

  /** \brief Returns maximum number of data bytes. */
  [[nodiscard]] uint32_t MaxLength() const { return max_length_; }
 protected:
  const MdfWriter& writer_;

  MdfStorageType storage_type_ = MdfStorageType::FixedLengthStorage;
  uint16_t bus_type_ = 0;
  bool mandatory_members_only_ = false;
  uint32_t max_length_ = 8;


  std::string protocol_;
  MessageFilter filter_type_ = MessageFilter::NoFilter;
  uint8_t bus_channel_ = 0;
  uint64_t message_id_ = 0;
  std::string message_name_;
  uint64_t message_length_ = 0;
  std::string network_name_;
  std::string device_name_;

  void BusName(std::string bus_name);

  [[nodiscard]] MdfStorageType StorageType() const {
    return storage_type_;
  }

  virtual IChannel* CreateTimeChannel(IChannelGroup& group,
                                     const std::string_view& name) const;
  virtual ISourceInformation* CreateSourceInformation(IChannelGroup& group) const;
  virtual ISourceInformation* CreateSourceInformation(IChannel& channel) const;
  virtual IChannel* CreateBitChannel(IChannel& parent,
                                    const std::string_view& name,
                                    uint32_t byte_offset,
                                    uint16_t bit_offset) const;

  virtual IChannel* CreateBitsChannel(IChannel& parent,
                                     const std::string_view& name,
                                     uint32_t byte_offset,
                                     uint16_t bit_offset,
                                     uint32_t nof_bits ) const;

  virtual IChannel* CreateBusChannel(IChannel& parent_channel) const;

  /** \brief Creates a sub-channel (composite) channel (unsigned)
   *
   * This support function can be used when creating unsigned integer
   * composite also known as sub-channel. The function creates a
   * channel name based upon the parent channel name with the sub-name
   * appended ("parent-nqme.sub-name").
   * @param parent_channel Reference to the parent channel.
   * @param sub_name Sub-name without the dot.
   * @param byte_offset Byte offset into the channel group record
   * @param bit_offset Bit offset relative the byte offset.
   * @param nof_bits Number of bits
   * @return Returns a pointer to the created channel or null on failure.
   */
  IChannel* CreateSubChannel(IChannel& parent_channel,
                             const std::string_view& sub_name,
                             uint32_t byte_offset,
                             uint16_t bit_offset,
                             uint32_t nof_bits) const;

  /** brief Creates a channel group name.
   *
   * Creates a CG name depending on the filter type and its so-called
   * base name. The filter_type combined with bus channel and optional
   * message name/ID.
 * @param base_name Defines what basic type of message.
 * @return A standard group name.
   */
  [[nodiscard]] virtual std::string MakeGroupName(
      const std::string_view& base_name) const;

 private:
  std::string bus_name_;
};

}  // namespace mdf


