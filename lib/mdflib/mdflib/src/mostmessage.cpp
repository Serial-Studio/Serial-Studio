/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */
#include <utility>

#include "mdf/mostmessage.h"
#include "mdf/idatagroup.h"
#include "mdf/mdfhelper.h"

namespace mdf {
IMostEvent::IMostEvent(MostMessageType type)
: message_type_(type) {

}

MostEthernetPacket::MostEthernetPacket()
: IMostEvent(MostMessageType::EthernetPacket) {
}

void MostEthernetPacket::DataBytes(std::vector<uint8_t> bytes) {
  data_bytes_ = std::move(bytes);
  const auto nof_bytes = static_cast<uint32_t>(data_bytes_.size());
  if (nof_bytes > specified_nof_bytes_) {
    specified_nof_bytes_ = nof_bytes;
  }
  if (nof_bytes > received_nof_bytes_) {
    received_nof_bytes_ = nof_bytes;
  }
}

const std::vector<uint8_t>& MostEthernetPacket::DataBytes() const {
  return data_bytes_;
}

MostEthernetPacket::MostEthernetPacket(MostMessageType type)
    : IMostEvent(type) {

}

void MostEthernetPacket::ToRaw(SampleRecord& sample) const {
  const bool mandatory_members_only = data_group_ != nullptr ?
                                                             data_group_->MandatoryMembersOnly() : false;
  const size_t record_size = mandatory_members_only ? 36 : 42;

  // Get a reference to the fixed size record buffer and size it
  auto& record = sample.record_buffer;
  record.resize(record_size);
  // The time allocate the first 8 bytes (double)
  record[8] = BusChannel();
  record[9] = static_cast<uint8_t>(Speed()) & 0x03;
  record[9] |= (static_cast<uint8_t>(TransferType()) & 0x01) << 2;
  record[9] |= (static_cast<uint8_t>(Direction()) & 0x01) << 3;
  MdfHelper::UnsignedToRaw(true, 0, 48, Source(), record.data() + 10);
  MdfHelper::UnsignedToRaw(true, 0, 48, Destination(), record.data() + 16);
  MdfHelper::UnsignedToRaw(true, 0, 16, SpecifiedNofBytes(), record.data() + 22);
  MdfHelper::UnsignedToRaw(true, 0, 16, ReceivedNofBytes(), record.data() + 24);
  MdfHelper::UnsignedToRaw(true, 0, 16, DataBytes().size(), record.data() + 26);

  sample.vlsd_data = true;
  sample.vlsd_buffer = DataBytes();

  // The data index have in reality not been updated at this point, but
  // it will be updated when the sample buffer is written to the disc.
  // We need to save the data bytes to a temp buffer (VLSD data).
  constexpr uint64_t data_index = 0;
  if (mandatory_members_only) {
    MdfHelper::UnsignedToRaw(true, 0, 64, data_index, record.data() + 28);
    return;
  }
  MdfHelper::UnsignedToRaw(true, 0, 32, Crc(), record.data() + 28);
  record[32] = static_cast<uint8_t>(CompleteAck()) & 0x07;
  record[32] |= (static_cast<uint8_t>(PreemptiveAck()) & 0x07) << 3;
  record[33] = static_cast<uint8_t>(TxAck());
  MdfHelper::UnsignedToRaw(true, 0, 64, data_index, record.data() + 34);
}

MostPacket::MostPacket()
: MostEthernetPacket(MostMessageType::Packet) {

}

MostPacket::MostPacket(MostMessageType type) : MostEthernetPacket(type) {}

void MostPacket::ToRaw(SampleRecord& sample) const {
  const bool mandatory_members_only = data_group_ != nullptr ?
                                                             data_group_->MandatoryMembersOnly() : false;
  const size_t record_size = mandatory_members_only ? 28 : 35;

  // Get a reference to the fixed size record buffer and size it
  auto& record = sample.record_buffer;
  record.resize(record_size);
  // The time allocate the first 8 bytes (double)
  record[8] = BusChannel();
  record[9] = static_cast<uint8_t>(Speed()) & 0x03;
  record[9] |= (static_cast<uint8_t>(TransferType()) & 0x01) << 2;
  record[9] |= (static_cast<uint8_t>(Direction()) & 0x01) << 3;
  MdfHelper::UnsignedToRaw(true, 0, 16, Source(), record.data() + 10);
  MdfHelper::UnsignedToRaw(true, 0, 16, Destination(), record.data() + 12);
  MdfHelper::UnsignedToRaw(true, 0, 16, SpecifiedNofBytes(), record.data() + 14);
  MdfHelper::UnsignedToRaw(true, 0, 16, ReceivedNofBytes(), record.data() + 16);
  MdfHelper::UnsignedToRaw(true, 0, 16, DataBytes().size(), record.data() + 18);

  sample.vlsd_data = true;
  sample.vlsd_buffer = DataBytes();

  // The data index have in reality not been updated at this point, but
  // it will be updated when the sample buffer is written to the disc.
  // We need to save the data bytes to a temp buffer (VLSD data).
  constexpr uint64_t data_index = 0;
  if (mandatory_members_only) {
    MdfHelper::UnsignedToRaw(true, 0, 64, data_index, record.data() + 20);
    return;
  }
  MdfHelper::UnsignedToRaw(true, 0, 32, Crc(), record.data() + 20);
  record[24] = static_cast<uint8_t>(CompleteAck()) & 0x07;
  record[24] |= (static_cast<uint8_t>(PreemptiveAck()) & 0x07) << 3;
  record[25] = static_cast<uint8_t>(TxAck());
  record[26] = PacketIndex();
  MdfHelper::UnsignedToRaw(true, 0, 64, data_index, record.data() + 27);
}

MostMessage::MostMessage()
: MostPacket(MostMessageType::Message) {

}

void MostMessage::ToRaw(SampleRecord& sample) const {
  const bool mandatory_members_only = data_group_ != nullptr ?
      data_group_->MandatoryMembersOnly() : false;
  const size_t record_size = mandatory_members_only ? 28 : 35;

  // Get a reference to the fixed size record buffer and size it
  auto& record = sample.record_buffer;
  record.resize(record_size);
  // The time allocate the first 8 bytes (double)
  record[8] = BusChannel();
  record[9] = static_cast<uint8_t>(Speed()) & 0x03;
  record[9] |= (static_cast<uint8_t>(TransferType()) & 0x01) << 2;
  record[9] |= (static_cast<uint8_t>(Direction()) & 0x01) << 3;
  MdfHelper::UnsignedToRaw(true, 0, 16, Source(), record.data() + 10);
  MdfHelper::UnsignedToRaw(true, 0, 16, Destination(), record.data() + 12);
  MdfHelper::UnsignedToRaw(true, 0, 16, SpecifiedNofBytes(), record.data() + 14);
  MdfHelper::UnsignedToRaw(true, 0, 16, ReceivedNofBytes(), record.data() + 16);
  MdfHelper::UnsignedToRaw(true, 0, 16, DataBytes().size(), record.data() + 18);

  sample.vlsd_data = true;
  sample.vlsd_buffer = DataBytes();

  // The data index have in reality not been updated at this point, but
  // it will be updated when the sample buffer is written to the disc.
  // We need to save the data bytes to a temp buffer (VLSD data).
  constexpr uint64_t data_index = 0;

  if (mandatory_members_only) {
    MdfHelper::UnsignedToRaw(true, 0, 64, data_index, record.data() + 20);
    return;
  }
  MdfHelper::UnsignedToRaw(true, 0, 32, Crc(), record.data() + 20);
  record[24] = static_cast<uint8_t>(CompleteAck()) & 0x07;
  record[24] |= (static_cast<uint8_t>(PreemptiveAck()) & 0x07) << 3;
  record[24] |= (static_cast<uint8_t>(TxFailed()) & 0x01) << 6;
  record[25] = static_cast<uint8_t>(TxAck()) & 0x0F;
  record[25] |= (static_cast<uint8_t>(ControlMessageType()) & 0x0F) << 4;
  record[26] = PacketIndex();
  MdfHelper::UnsignedToRaw(true, 0, 64, data_index, record.data() + 27);
}

MostSignalState::MostSignalState()
: IMostEvent(MostMessageType::SignalState) {

}

void MostSignalState::ToRaw(SampleRecord& sample) const {
  constexpr size_t record_size = 10;
  auto& record = sample.record_buffer;
  record.resize(record_size);
  // The time allocate the first 8 bytes (double)
  record[8] = BusChannel();
  record[9] = static_cast<uint8_t>(SignalState());

  sample.vlsd_data = false;
  sample.vlsd_buffer.clear();
  sample.vlsd_buffer.shrink_to_fit();
}

MostMaxPosInfo::MostMaxPosInfo()
    : IMostEvent(MostMessageType::MaxPosInfo) {

}
void MostMaxPosInfo::ToRaw(SampleRecord& sample) const {
  constexpr size_t record_size = 10;
  auto& record = sample.record_buffer;
  record.resize(record_size);
  // The time allocate the first 8 bytes (double)
  record[8] = BusChannel();
  record[9] = DeviceCount();

  sample.vlsd_data = false;
  sample.vlsd_buffer.clear();
  sample.vlsd_buffer.shrink_to_fit();
}

MostBoundDesc::MostBoundDesc()
    : IMostEvent(MostMessageType::BoundDesc) {

}

void MostBoundDesc::ToRaw(SampleRecord& sample) const {
  constexpr size_t record_size = 11;
  auto& record = sample.record_buffer;
  record.resize(record_size);
  // The time allocate the first 8 bytes (double)
  record[8] = BusChannel();
  MdfHelper::UnsignedToRaw(true, 0, 16, BoundaryCount() , record.data() + 9);

  sample.vlsd_data = false;
  sample.vlsd_buffer.clear();
  sample.vlsd_buffer.shrink_to_fit();
}

MostAllocTable::MostAllocTable()
    : IMostEvent(MostMessageType::AllocTable) {

}

void MostAllocTable::TableData(std::vector<uint8_t> bytes) {
  table_data_ = std::move(bytes);
}

const std::vector<uint8_t>& MostAllocTable::TableData() const {
  return table_data_;
}

void MostAllocTable::ToRaw(SampleRecord& sample) const {
  constexpr size_t record_size = 22;

  // Get a reference to the fixed size record buffer and size it
  auto& record = sample.record_buffer;
  record.resize(record_size);
  // The time allocate the first 8 bytes (double)
  record[8] = BusChannel();
  MdfHelper::UnsignedToRaw(true, 0, 16, FreeByteCount(), record.data() + 9);
  record[11] = static_cast<uint8_t>(TableLayout());
  MdfHelper::UnsignedToRaw(true, 0, 16, static_cast<uint16_t>(TableData().size()),
                           record.data() + 12);


  sample.vlsd_data = true;
  sample.vlsd_buffer = TableData();

  // The data index have in reality not been updated at this point, but
  // it will be updated when the sample buffer is written to the disc.
  // We need to save the data bytes to a temp buffer (VLSD data).
  constexpr uint64_t data_index = 0;
  MdfHelper::UnsignedToRaw(true, 0, 64, data_index, record.data() + 14);
}


MostSysLockState::MostSysLockState()
: IMostEvent(MostMessageType::SysLockState) {

}

void MostSysLockState::ToRaw(SampleRecord& sample) const {
  constexpr size_t record_size = 10;
  auto& record = sample.record_buffer;
  record.resize(record_size);
  // The time allocate the first 8 bytes (double)
  record[8] = BusChannel();
  record[9] = RingIsClosed() ? 1 : 0;;

  sample.vlsd_data = false;
  sample.vlsd_buffer.clear();
  sample.vlsd_buffer.shrink_to_fit();
}

MostShutdownFlag::MostShutdownFlag()
    : IMostEvent(MostMessageType::ShutdownFlag) {

}

void MostShutdownFlag::ToRaw(SampleRecord& sample) const {
  constexpr size_t record_size = 10;
  auto& record = sample.record_buffer;
  record.resize(record_size);
  // The time allocate the first 8 bytes (double)
  record[8] = BusChannel();
  record[9] = ShutdownFlag() ? 1 : 0;

  sample.vlsd_data = false;
  sample.vlsd_buffer.clear();
  sample.vlsd_buffer.shrink_to_fit();
}

}  // namespace mdf