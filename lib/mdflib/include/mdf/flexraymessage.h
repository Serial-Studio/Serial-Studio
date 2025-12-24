/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once

#include <cstdint>
#include <vector>

#include "mdf/samplerecord.h"

namespace mdf {

class IDataGroup;
class IChannelGroup;

enum class FlexRayMessageType : uint8_t {
  Frame,
  Pdu,
  FrameHeader,
  NullFrame,
  ErrorFrame,
  Symbol
};

enum class FlexRayChannel : uint8_t {
  A = 0,
  B = 1
};

enum class FlexRayChannelMask : uint8_t {
  A = 1,
  B = 2,
  A_B = 3
};

enum class FlexRayDirection : uint8_t {
  Rx = 0,
  Tx = 1
};

enum class FlexRayNullFlag : uint8_t {
  NullFrame = 0,
  NormalFrame = 1
};

enum class FlexRaySymbolType : uint8_t {
  Unknown = 0, ///< Unknown symbol (default)
  CAS = 1,     ///< Collision Avoidance Symbol
  MTS = 2,     ///< Media Test Symbol
  WUS = 3      ///< Wakeup Symbol
};

class IFlexRayEvent {
 public:
  IFlexRayEvent() = delete;
  virtual ~IFlexRayEvent() = default;

  void BusChannel(uint8_t bus_channel) {bus_channel_ = bus_channel;}
  [[nodiscard]] uint8_t BusChannel() const {return bus_channel_;}

  [[nodiscard]] FlexRayMessageType MessageType() const { return message_type_;}

  void FrameId(uint16_t frame_id) { frame_id_ = frame_id; }
  [[nodiscard]] uint16_t FrameId() const { return frame_id_; }

  void CycleCount(uint8_t cycle) { cycle_count_ = cycle; }
  [[nodiscard]] uint8_t CycleCount() const { return cycle_count_; }

  void Channel(FlexRayChannel channel) { channel_ = channel; }
  [[nodiscard]] FlexRayChannel Channel() const { return channel_; }

  void Direction(FlexRayDirection direction) { direction_ = direction; }
  [[nodiscard]] FlexRayDirection Direction() const { return direction_; }

  virtual void ToRaw(SampleRecord& sample) const = 0;

  void DataGroup(const IDataGroup* data_group) const { data_group_ = data_group; }
  void ChannelGroup(const IChannelGroup* channel_group) const { channel_group_ = channel_group; }

 protected:
  explicit IFlexRayEvent(FlexRayMessageType type);

  virtual void DataBytes(std::vector<uint8_t> bytes);
  [[nodiscard]] virtual const std::vector<uint8_t>& DataBytes() const;

  mutable const IDataGroup* data_group_ = nullptr;
  mutable const IChannelGroup* channel_group_ = nullptr;

 private:
  FlexRayMessageType message_type_; ///< Type of FlexRay message.
  uint8_t bus_channel_ = 0;  ///< 8-bit bus channel.
  uint16_t frame_id_ = 0; ///< 11-bit ID.
  uint8_t cycle_count_ = 0; ///< 6-bit cycle counter.
  FlexRayChannel channel_ = FlexRayChannel::A; ///< FlexRay channel.
  FlexRayDirection direction_ = FlexRayDirection::Rx; ///< 1-bit direction.
  std::vector<uint8_t> data_bytes_; ///< Payload 0-254 bytes.
};

class FlexRayFrame : public IFlexRayEvent {
 public:
  FlexRayFrame();

  void PayloadLength(uint8_t length) { payload_length_ = length; }
  [[nodiscard]] uint8_t PayloadLength() const { return payload_length_; }

  void DataBytes(std::vector<uint8_t> bytes) override;
  [[nodiscard]] const std::vector<uint8_t>& DataBytes() const override;

  void Crc(uint32_t crc) { crc_ = crc; }
  [[nodiscard]] uint32_t Crc() const { return crc_; }

  void HeaderCrc(uint16_t crc) { header_crc_ = crc; }
  [[nodiscard]] uint16_t HeaderCrc() const { return header_crc_; }

  void Reserved(bool reserved) { reserved_ = reserved; }
  [[nodiscard]] bool Reserved() const { return reserved_; }

  void PayloadPreamble(bool preamble) { payload_preamble_ = preamble; }
  [[nodiscard]] bool PayloadPreamble() const { return payload_preamble_; }

  void NullFrame(FlexRayNullFlag flag) { null_frame_ = flag; }
  [[nodiscard]] FlexRayNullFlag NullFrame() const { return null_frame_; }

  void SyncFrame(bool sync) { sync_frame_ = sync; }
  [[nodiscard]] bool SyncFrame() const { return sync_frame_; }

  void StartupFrame(bool startup) { startup_frame_ = startup; }
  [[nodiscard]] bool StartupFrame() const { return startup_frame_; }

  void FrameLength(uint32_t length) { frame_length_ = length; }
  [[nodiscard]] uint32_t FrameLength() const { return frame_length_; }

 protected:
  explicit FlexRayFrame(FlexRayMessageType type);
  void ToRaw(SampleRecord& sample) const override;

 private:
  uint8_t payload_length_ = 0; /// 7-bit payload length (words).
  uint32_t crc_ = 0; ///< 24-bit trailer checksum.
  uint16_t header_crc_ = 0; ///< 11-bit checksum.

  bool reserved_ = false; ///< Reserved bit flag.
  bool payload_preamble_ = false; ///< Payload preamble flag.
  FlexRayNullFlag null_frame_ = FlexRayNullFlag::NormalFrame;
  bool sync_frame_ = false; ///< Sync frame flag.
  bool startup_frame_ = false; ///< Startup frame flag.
  uint32_t frame_length_ = 0; ///< Frame length in nano-seconds.
};

class FlexRayPdu : public IFlexRayEvent {
 public:
  FlexRayPdu();

  void Length(uint8_t length) { pdu_length_ = length; }
  [[nodiscard]] uint8_t Length() const { return pdu_length_; }

  void DataBytes(std::vector<uint8_t> bytes) override;
  [[nodiscard]] const std::vector<uint8_t>& DataBytes() const override;

  void Multiplexed(bool multiplexed) { multiplexed_ = multiplexed; }
  [[nodiscard]] bool Multiplexed() const { return multiplexed_; }

  void Switch(uint16_t value) { switch_ = value; }
  [[nodiscard]] uint16_t Switch() const { return switch_; }

  void Valid(bool valid) { valid_ = valid; }
  [[nodiscard]] bool Valid() const { return valid_; }

  void UpdateBitPos(uint16_t pos) { update_bit_pos_ = pos; }
  [[nodiscard]] uint16_t UpdateBitPos() const { return update_bit_pos_; }

  void ByteOffset(uint8_t offset) { pdu_byte_offset_ = offset; }
  [[nodiscard]] uint8_t ByteOffset() const { return pdu_byte_offset_; }

  void BitOffset(uint8_t offset) { pdu_bit_offset_ = offset; }
  [[nodiscard]] uint8_t BitOffset() const { return pdu_bit_offset_; }
 protected:
  void ToRaw(SampleRecord& sample) const override;
 private:
  using IFlexRayEvent::Direction; // Hide the usage of this function

  uint8_t pdu_length_ = 0; /// PDU length in bytes.
  bool multiplexed_ = false; ///< Multiplexed flag.
  uint16_t switch_ = 0; ///< Switch value for multiplexed PDUs.
  bool valid_ = true; ///< Valid flag for PDU.
  uint16_t update_bit_pos_ = 0xFFF; ///< 12-bit bit pos
  uint8_t pdu_byte_offset_ = 0; ///< PDU byte offset (8-bit)
  uint8_t pdu_bit_offset_ = 0; ///< PDU bit offset (3-bit)
};

class FlexRayFrameHeader : public FlexRayFrame {
 public:
  FlexRayFrameHeader();
  void FillDataBytes(std::vector<uint8_t> bytes);
  [[nodiscard]] const std::vector<uint8_t>& FillDataBytes() const;
 protected:
  void ToRaw(SampleRecord& sample) const override;
 private:
  using FlexRayFrame::DataBytes;
};

class FlexRayNullFrame : public FlexRayFrame {
 public:
  FlexRayNullFrame();
 protected:
  void ToRaw(SampleRecord& sample) const override;
 private:

};

class FlexRayErrorFrame : public FlexRayFrame {
 public:
  FlexRayErrorFrame();

  void Syntax(bool syntax) { syntax_error_ = syntax; }
  [[nodiscard]] bool Syntax() const { return syntax_error_; }

  void Content(bool content) { content_error_ = content; }
  [[nodiscard]] bool Content() const { return content_error_; }

  void Boundary(bool boundary) { boundary_error_ = boundary; }
  [[nodiscard]] bool Boundary() const { return boundary_error_; }

  void TxConflict(bool conflict) { tx_conflict_ = conflict; }
  [[nodiscard]] bool TxConflict() const { return tx_conflict_; }

  void Valid(bool valid) { valid_ = valid; }
  [[nodiscard]] bool Valid() const { return valid_; }
 protected:
  void ToRaw(SampleRecord& sample) const override;
 private:
  bool syntax_error_ = false;
  bool content_error_ = false;
  bool boundary_error_ = false;
  bool tx_conflict_ = false;
  bool valid_ = false;

};

class FlexRaySymbol : public IFlexRayEvent {
 public:
  FlexRaySymbol();
  void ChannelMask(FlexRayChannelMask mask) { mask_ = mask; }
  [[nodiscard]] FlexRayChannelMask ChannelMask() const { return mask_; }

  void Length(uint8_t length) { length_ = length; }
  [[nodiscard]] uint8_t Length() const { return length_; }

  void Type(FlexRaySymbolType type) { type_ = type; }
  [[nodiscard]] FlexRaySymbolType Type() const { return type_; }
 protected:
  void ToRaw(SampleRecord& sample) const override;
 private:
  using IFlexRayEvent::FrameId;
  using IFlexRayEvent::Channel;
  using IFlexRayEvent::Direction;

  FlexRayChannelMask mask_ = FlexRayChannelMask::A_B;
  uint8_t length_ = 0; ///< Unit macro-ticks (MT)
  FlexRaySymbolType type_ = FlexRaySymbolType::Unknown;
};
}  // namespace mdf


