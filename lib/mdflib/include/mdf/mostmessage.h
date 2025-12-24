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


enum class MostMessageType : uint8_t {
  Message,
  Packet,
  EthernetPacket,
  SignalState,
  MaxPosInfo,
  BoundDesc,
  AllocTable,
  SysLockState,
  ShutdownFlag
};

enum class MostSpeed : uint8_t {
  Most25 = 0,
  Most50 = 1,
  Most150 = 2
};

enum class MostTransferType : uint8_t {
  Spy = 0,
  Node = 1
};

enum class MostDirection : uint8_t {
  Rx = 0,
  Tx = 1
};

enum class MostCompleteAck : uint8_t {
  NoResponse = 0,
  CrcError = 1,
  Ok = 4,
};

enum class MostPreemptiveAck : uint8_t {
  NoResponse = 0,
  BufferFull = 1,
  Ok = 4,
};

enum class MostTxAck : uint8_t {
  Ignored = 0x00,
  Ok = 0x01,
  FailedFormatError = 0x02,
  FailedNetworkOff = 0x04,
  FailedTimeout = 0x05,
  FailedWrongTarget = 0x08,
  OkOneSuccess = 0x09,
  FailedBadCrc = 0x0C,
  FailedReceiverBufferFull = 0x0E
};

enum class MostControlMessageType : uint8_t {
  NormalMessage = 0x00,
  RemoteRead = 0x01,
  RemoteWrite = 0x02,
  ResourceAllocate = 0x03,
  ResourceDeallocate = 0x04,
  RemoteGetSource = 0x05,

};

enum class MostStateOfSignal : uint8_t {
  Unknown = 0,
  SignalOnLock = 1,
  SignalOff = 2,
  SignalOnNoLock = 3,
  StableLock = 16,
  CriticalUnlock = 32
};

enum class MostTableLayout : uint8_t {
  ChannelResourceAllocationTable = 0,
  LabelList = 1
};

class IMostEvent {
 public:
  IMostEvent() = delete;
  virtual ~IMostEvent() = default;

  void BusChannel(uint8_t bus_channel) {bus_channel_ = bus_channel;}
  [[nodiscard]] uint8_t BusChannel() const {return bus_channel_;}

  [[nodiscard]] MostMessageType MessageType() const { return message_type_;}

  virtual void ToRaw(SampleRecord& sample) const = 0;

  void DataGroup(const IDataGroup* data_group) const { data_group_ = data_group; }
  void ChannelGroup(const IChannelGroup* channel_group) const { channel_group_ = channel_group; }

 protected:
  explicit IMostEvent(MostMessageType type);
  mutable const IDataGroup* data_group_ = nullptr;
  mutable const IChannelGroup* channel_group_ = nullptr;

 private:
  MostMessageType message_type_;
  uint8_t bus_channel_ = 0;
};

class MostEthernetPacket : public IMostEvent {
 public:
  MostEthernetPacket();

  void Speed(MostSpeed speed) { speed_ = speed; }
  [[nodiscard]] MostSpeed Speed() const { return speed_; }

  void TransferType(MostTransferType type) { transfer_type_ = type; }
  [[nodiscard]] MostTransferType TransferType() const { return transfer_type_; }

  void Direction(MostDirection direction) { direction_ = direction; }
  [[nodiscard]] MostDirection Direction() const { return direction_; }

  void Source(uint64_t source) { source_ = source; }
  [[nodiscard]] uint64_t Source() const { return source_; }

  void Destination(uint64_t destination) { destination_ = destination; }
  [[nodiscard]] uint64_t Destination() const { return destination_; }

  void SpecifiedNofBytes(uint16_t nof_bytes) { specified_nof_bytes_ = nof_bytes; }
  [[nodiscard]] uint16_t SpecifiedNofBytes() const { return specified_nof_bytes_; }

  void ReceivedNofBytes(uint16_t nof_bytes) { received_nof_bytes_ = nof_bytes; }
  [[nodiscard]] uint16_t ReceivedNofBytes() const { return received_nof_bytes_; }

  void DataBytes(std::vector<uint8_t> bytes);
  [[nodiscard]] const std::vector<uint8_t>& DataBytes() const;

  void Crc(uint32_t crc) { crc_ = crc; }
  [[nodiscard]] uint32_t Crc() const { return crc_; }

  void CompleteAck(MostCompleteAck ack) { complete_ack_ = ack; }
  [[nodiscard]] MostCompleteAck CompleteAck() const { return complete_ack_; }

  void PreemptiveAck(MostPreemptiveAck ack) { preemptive_ack_ = ack; }
  [[nodiscard]] MostPreemptiveAck PreemptiveAck() const { return preemptive_ack_; }

  void TxAck(uint8_t ack) { tx_ack_ = ack; }
  [[nodiscard]] uint8_t TxAck() const { return tx_ack_; }

 protected:
  void ToRaw(SampleRecord& sample) const override;

  explicit MostEthernetPacket(MostMessageType type);

 private:
   MostSpeed speed_ = MostSpeed::Most25;
   MostTransferType transfer_type_ = MostTransferType::Spy;
   MostDirection direction_ = MostDirection::Rx;
   uint64_t source_ = 0;
   uint64_t destination_ = 0;
   uint16_t specified_nof_bytes_ = 0;
   uint16_t received_nof_bytes_ = 0;
   std::vector<uint8_t> data_bytes_;

   uint32_t crc_ = 0;
   MostCompleteAck complete_ack_ = MostCompleteAck::Ok;
   MostPreemptiveAck preemptive_ack_ = MostPreemptiveAck::Ok;
   uint8_t tx_ack_ = 0;
};

class MostPacket : public MostEthernetPacket {
 public:
  MostPacket();

  void PacketIndex(uint8_t packet_index) { packet_index_ = packet_index; }
  [[nodiscard]] uint8_t PacketIndex() const { return packet_index_;}

 protected:
  explicit MostPacket(MostMessageType type);
  void ToRaw(SampleRecord& sample) const override;

 private:
  uint8_t packet_index_ = 0;
};

class MostMessage : public MostPacket {
 public:
  MostMessage();

  void TxFailed(bool failed) { tx_failed_ = failed; }
  [[nodiscard]] bool TxFailed() const { return tx_failed_;}

  void ControlMessageType(MostControlMessageType type) { control_message_type_ = type; }
  [[nodiscard]] MostControlMessageType ControlMessageType() const { return control_message_type_;}

 protected:
  void ToRaw(SampleRecord& sample) const override;
 private:
  bool tx_failed_ = false;
  MostControlMessageType control_message_type_ = MostControlMessageType::NormalMessage;
};

class MostSignalState : public IMostEvent {
 public:
  MostSignalState();

  void SignalState(MostStateOfSignal state) { state_ = state; }
  [[nodiscard]] MostStateOfSignal SignalState() const { return state_; }

 protected:
  void ToRaw(SampleRecord& sample) const override;

 private:
  MostStateOfSignal state_ = MostStateOfSignal::Unknown;
};

class MostMaxPosInfo : public IMostEvent {
 public:
  MostMaxPosInfo();

  void DeviceCount(uint8_t count) { device_count_ = count; }
  [[nodiscard]] uint8_t DeviceCount() const { return device_count_; }

 protected:
  void ToRaw(SampleRecord& sample) const override;

 private:
  uint8_t device_count_ = 0;
};

class MostBoundDesc : public IMostEvent {
 public:
  MostBoundDesc();

  void BoundaryCount(uint16_t count) { boundary_count_ = count; }
  [[nodiscard]] uint16_t BoundaryCount() const { return boundary_count_; }

 protected:
  void ToRaw(SampleRecord& sample) const override;

 private:
  uint16_t boundary_count_ = 0;
};

class MostAllocTable : public IMostEvent {
 public:
  MostAllocTable();

  void FreeByteCount(uint16_t count) { free_byte_count_ = count; }
  [[nodiscard]] uint16_t FreeByteCount() const { return free_byte_count_; }

  void TableLayout(MostTableLayout layout) { layout_ = layout; }
  [[nodiscard]] MostTableLayout TableLayout() const { return layout_; }

  void TableData(std::vector<uint8_t> bytes);
  [[nodiscard]] const std::vector<uint8_t>& TableData() const;

 protected:
  void ToRaw(SampleRecord& sample) const override;

 private:
  uint16_t free_byte_count_ = 0;
  MostTableLayout layout_ = MostTableLayout::ChannelResourceAllocationTable;
  std::vector<uint8_t> table_data_;
};

class MostSysLockState : public IMostEvent {
 public:
  MostSysLockState();

  void RingIsClosed(bool closed) { ring_is_closed_ = closed; }
  [[nodiscard]] bool RingIsClosed() const { return ring_is_closed_; }

 protected:
  void ToRaw(SampleRecord& sample) const override;

 private:
  bool ring_is_closed_ = false;
};

class MostShutdownFlag : public IMostEvent {
 public:
  MostShutdownFlag();

  void ShutdownFlag(bool shutdown) { shutdown_flag_ = shutdown; }
  [[nodiscard]] bool ShutdownFlag() const { return shutdown_flag_; }

 protected:
  void ToRaw(SampleRecord& sample) const override;

 private:
  bool shutdown_flag_ = false;
};

}  // namespace mdf


