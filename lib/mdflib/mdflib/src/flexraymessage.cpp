/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include "mdf/flexraymessage.h"

#include "mdf/idatagroup.h"
#include "mdf/mdfhelper.h"

namespace mdf {
IFlexRayEvent::IFlexRayEvent(FlexRayMessageType type)
    : message_type_(type) {

}

void IFlexRayEvent::DataBytes(std::vector<uint8_t> bytes) {
  data_bytes_ = std::move(bytes);
}

const std::vector<uint8_t>& IFlexRayEvent::DataBytes() const {
  return data_bytes_;
}

FlexRayFrame::FlexRayFrame()
: IFlexRayEvent(FlexRayMessageType::Frame) {

}

FlexRayFrame::FlexRayFrame(FlexRayMessageType type)
: IFlexRayEvent(type) {

}

void FlexRayFrame::DataBytes(std::vector<uint8_t> bytes) {
  IFlexRayEvent::DataBytes(std::move(bytes));
}

const std::vector<uint8_t>& FlexRayFrame::DataBytes() const {
  return IFlexRayEvent::DataBytes();
}

void FlexRayFrame::ToRaw(SampleRecord& sample) const {
  const bool mandatory_members_only = data_group_ != nullptr ?
                     data_group_->MandatoryMembersOnly() : false;
  const size_t record_size = mandatory_members_only ? 24 : 34;

  // Get a reference to the fixed size record buffer and size it
  auto& record = sample.record_buffer;
  record.resize(record_size);
  // The time allocate the first 8 bytes (double)
  record[8] = BusChannel();
  MdfHelper::UnsignedToRaw(true, 0, 11, FrameId(), record.data() + 9);
  record[11] = static_cast<uint8_t>(CycleCount()) & 0x3F;
  record[11] |= (static_cast<uint8_t>(FlexRayChannel()) & 0x01) << 6;
  record[11] |= (static_cast<uint8_t>(Direction()) & 0x01) << 7;
  record[12] = PayloadLength();
  record[13] = static_cast<uint8_t>(DataBytes().size());

  sample.vlsd_data = true;
  sample.vlsd_buffer = DataBytes();

  // The data index have in reality not been updated at this point, but
  // it will be updated when the sample buffer is written to the disc.
  // We need to save the data bytes to a temp buffer (VLSD data).
  constexpr uint64_t data_index = 0;
  if (mandatory_members_only) {
    MdfHelper::UnsignedToRaw(true, 0, 64, data_index, record.data() + 14);
    return;
  }
  MdfHelper::UnsignedToRaw(true, 0, 24, Crc(), record.data() + 14);
  MdfHelper::UnsignedToRaw(true, 0, 16, HeaderCrc(), record.data() + 17);
  record[19] = Reserved() ? 0x01 : 0x00;
  record[19] |= (PayloadPreamble() ? 0x01 : 0x00) << 1;
  record[19] |= (static_cast<uint8_t>(NullFrame()) & 0x01) << 2;
  record[19] |= (SyncFrame() ? 0x01 : 0x00) << 3;
  record[19] |= (StartupFrame() ? 0x01 : 0x00) << 4;
  MdfHelper::UnsignedToRaw(true, 0, 32,FrameLength(), record.data() + 20);
  MdfHelper::UnsignedToRaw(true, 0, 64, data_index, record.data() + 24);
}

FlexRayPdu::FlexRayPdu()
: IFlexRayEvent(FlexRayMessageType::Pdu) {

}

void FlexRayPdu::DataBytes(std::vector<uint8_t> bytes) {
  IFlexRayEvent::DataBytes(std::move(bytes));
}

const std::vector<uint8_t>& FlexRayPdu::DataBytes() const {
  return IFlexRayEvent::DataBytes();
}

void FlexRayPdu::ToRaw(SampleRecord& sample) const {
  const bool mandatory_members_only = data_group_ != nullptr ?
                   data_group_->MandatoryMembersOnly() : false;
  const size_t record_size = mandatory_members_only ? 24 : 28;

  // Get a reference to the fixed size record buffer and size it
  auto& record = sample.record_buffer;
  record.resize(record_size);
  // The time allocate the first 8 bytes (double)
  record[8] = BusChannel();
  MdfHelper::UnsignedToRaw(true, 0, 11, FrameId(), record.data() + 9);
  record[11] = static_cast<uint8_t>(CycleCount() & 0x3F);
  record[11] |= (static_cast<uint8_t>(FlexRayChannel()) & 0x01) << 6;
  record[12] = Length();
  record[13] = static_cast<uint8_t>(DataBytes().size());

  sample.vlsd_data = true;
  sample.vlsd_buffer = DataBytes();

  // The data index have in reality not been updated at this point, but
  // it will be updated when the sample buffer is written to the disc.
  // We need to save the data bytes to a temp buffer (VLSD data).
  constexpr uint64_t data_index = 0;
  if (mandatory_members_only) {
    MdfHelper::UnsignedToRaw(true, 0, 64, data_index, record.data() + 14);
    return;
  }
  record[14]= Multiplexed() ? 0x01 : 0x00;
  record[14] |= (Valid() ? 0x01 : 0x00) << 1;
  record[14] |= (BitOffset() & 0x07) << 2;
  MdfHelper::UnsignedToRaw(true, 0, 16, Switch(), record.data() + 15);;
  MdfHelper::UnsignedToRaw(true, 0, 16, UpdateBitPos(), record.data() + 17);
  record[19] = ByteOffset();
  MdfHelper::UnsignedToRaw(true, 0, 64, data_index, record.data() + 20);
}

FlexRayFrameHeader::FlexRayFrameHeader()
: FlexRayFrame(FlexRayMessageType::FrameHeader) {

}

void FlexRayFrameHeader::FillDataBytes(std::vector<uint8_t> bytes) {
  FlexRayFrame::DataBytes(std::move(bytes));
}

const std::vector<uint8_t>& FlexRayFrameHeader::FillDataBytes() const {
  return FlexRayFrame::DataBytes();
}

void FlexRayFrameHeader::ToRaw(SampleRecord& sample) const {
  const bool mandatory_members_only = data_group_ != nullptr ?
                      data_group_->MandatoryMembersOnly() : false;
  const size_t record_size = mandatory_members_only ? 13 : 34;

  // Get a reference to the fixed size record buffer and size it
  auto& record = sample.record_buffer;
  record.resize(record_size);
  // The time allocate the first 8 bytes (double)
  record[8] = BusChannel();
  MdfHelper::UnsignedToRaw(true, 0, 11, FrameId(), record.data() + 9);
  record[11] = static_cast<uint8_t>(CycleCount()) & 0x3F;
  record[11] |= (static_cast<uint8_t>(FlexRayChannel()) & 0x01) << 6;
  record[11] |= (static_cast<uint8_t>(Direction()) & 0x01) << 7;
  record[12] = PayloadLength();

  if (mandatory_members_only) {
    sample.vlsd_data = false;
    sample.vlsd_buffer.clear();
    sample.vlsd_buffer.shrink_to_fit();
    return;
  }
  record[13] = static_cast<uint8_t>(FillDataBytes().size());
  sample.vlsd_data = true;
  sample.vlsd_buffer = FillDataBytes();

  // The data index have in reality not been updated at this point, but
  // it will be updated when the sample buffer is written to the disc.
  // We need to save the data bytes to a temp buffer (VLSD data).
  constexpr uint64_t data_index = 0;

  MdfHelper::UnsignedToRaw(true, 0, 24, Crc(), record.data() + 14);
  MdfHelper::UnsignedToRaw(true, 0, 16, HeaderCrc(), record.data() + 17);
  record[19] = Reserved() ? 0x01 : 0x00;
  record[19] |= (PayloadPreamble() ? 0x01 : 0x00) << 1;
  record[19] |= (static_cast<uint8_t>(NullFrame()) & 0x01) << 2;
  record[19] |= (SyncFrame() ? 0x01 : 0x00) << 3;
  record[19] |= (StartupFrame() ? 0x01 : 0x00) << 4;
  MdfHelper::UnsignedToRaw(true, 0, 32,FrameLength(), record.data() + 20);
  MdfHelper::UnsignedToRaw(true, 0, 64, data_index, record.data() + 24);
}

FlexRayNullFrame::FlexRayNullFrame()
: FlexRayFrame(FlexRayMessageType::NullFrame) {
  NullFrame(FlexRayNullFlag::NullFrame);
}

void FlexRayNullFrame::ToRaw(SampleRecord& sample) const {
  const bool mandatory_members_only = data_group_ != nullptr ?
                                                             data_group_->MandatoryMembersOnly() : false;
  const size_t record_size = mandatory_members_only ? 12 : 34;

  // Get a reference to the fixed size record buffer and size it
  auto& record = sample.record_buffer;
  record.resize(record_size);
  // The time allocate the first 8 bytes (double)
  record[8] = BusChannel();
  MdfHelper::UnsignedToRaw(true, 0, 11, FrameId(), record.data() + 9);
  record[11] = static_cast<uint8_t>(CycleCount()) & 0x3F;
  record[11] |= (static_cast<uint8_t>(FlexRayChannel()) & 0x01) << 6;
  record[11] |= (static_cast<uint8_t>(Direction()) & 0x01) << 7;
  if (mandatory_members_only) {
    sample.vlsd_data = false;
    sample.vlsd_buffer.clear();
    sample.vlsd_buffer.shrink_to_fit();
    return;
  }
  record[12] = PayloadLength();
  record[13] = static_cast<uint8_t>(DataBytes().size());

  sample.vlsd_data = true;
  sample.vlsd_buffer = DataBytes();

  // The data index have in reality not been updated at this point, but
  // it will be updated when the sample buffer is written to the disc.
  // We need to save the data bytes to a temp buffer (VLSD data).
  constexpr uint64_t data_index = 0;

  MdfHelper::UnsignedToRaw(true, 0, 24, Crc(), record.data() + 14);
  MdfHelper::UnsignedToRaw(true, 0, 16, HeaderCrc(), record.data() + 17);
  record[19] = Reserved() ? 0x01 : 0x00;
  record[19] |= (PayloadPreamble() ? 0x01 : 0x00) << 1;
  record[19] |= (static_cast<uint8_t>(NullFrame()) & 0x01) << 2;
  record[19] |= (SyncFrame() ? 0x01 : 0x00) << 3;
  record[19] |= (StartupFrame() ? 0x01 : 0x00) << 4;
  MdfHelper::UnsignedToRaw(true, 0, 32,FrameLength(), record.data() + 20);
  MdfHelper::UnsignedToRaw(true, 0, 64, data_index, record.data() + 24);
}

FlexRayErrorFrame::FlexRayErrorFrame()
: FlexRayFrame(FlexRayMessageType::ErrorFrame) {

}

void FlexRayErrorFrame::ToRaw(SampleRecord& sample) const {
  const bool mandatory_members_only = data_group_ != nullptr ?
                                                             data_group_->MandatoryMembersOnly() : false;
  const size_t record_size = mandatory_members_only ? 12 : 35;

  // Get a reference to the fixed size record buffer and size it
  auto& record = sample.record_buffer;
  record.resize(record_size);
  // The time allocate the first 8 bytes (double)
  record[8] = BusChannel();
  MdfHelper::UnsignedToRaw(true, 0, 11, FrameId(), record.data() + 9);
  record[11] = static_cast<uint8_t>(CycleCount()) & 0x3F;
  record[11] |= (static_cast<uint8_t>(FlexRayChannel()) & 0x01) << 6;
  record[11] |= (static_cast<uint8_t>(Direction()) & 0x01) << 7;
  if (mandatory_members_only) {
    sample.vlsd_data = false;
    sample.vlsd_buffer.clear();
    sample.vlsd_buffer.shrink_to_fit();
    return;
  }
  record[12] = PayloadLength();
  record[13] = static_cast<uint8_t>(DataBytes().size());

  sample.vlsd_data = true;
  sample.vlsd_buffer = DataBytes();

  // The data index have in reality not been updated at this point, but
  // it will be updated when the sample buffer is written to the disc.
  // We need to save the data bytes to a temp buffer (VLSD data).
  constexpr uint64_t data_index = 0;

  MdfHelper::UnsignedToRaw(true, 0, 24, Crc(), record.data() + 14);
  MdfHelper::UnsignedToRaw(true, 0, 16, HeaderCrc(), record.data() + 17);

  record[19] = Reserved() ? 0x01 : 0x00;
  record[19] |= (PayloadPreamble() ? 0x01 : 0x00) << 1;
  record[19] |= (static_cast<uint8_t>(NullFrame()) & 0x01) << 2;
  record[19] |= (SyncFrame() ? 0x01 : 0x00) << 3;
  record[19] |= (StartupFrame() ? 0x01 : 0x00) << 4;
  record[19] |= (Syntax() ? 0x01 : 0x00) << 5;
  record[19] |= (Content() ? 0x01 : 0x00) << 6;
  record[19] |= (Boundary() ? 0x01 : 0x00) << 7;
  record[20] = TxConflict() ? 0x01 : 0x00;
  record[20] |= (Valid() ? 0x01 : 0x00) << 1;

  MdfHelper::UnsignedToRaw(true, 0, 32,FrameLength(), record.data() + 21);
  MdfHelper::UnsignedToRaw(true, 0, 64, data_index, record.data() + 25);
}

FlexRaySymbol::FlexRaySymbol()
: IFlexRayEvent(FlexRayMessageType::Symbol) {

}

void FlexRaySymbol::ToRaw(SampleRecord& sample) const {
  const bool mandatory_members_only = data_group_ != nullptr ?
                              data_group_->MandatoryMembersOnly() : false;
  const size_t record_size = mandatory_members_only ? 10 : 12;
  sample.vlsd_data = false;
  sample.vlsd_buffer.clear();
  sample.vlsd_buffer.shrink_to_fit();
  // Get a reference to the fixed size record buffer and size it
  auto& record = sample.record_buffer;
  record.resize(record_size);
  // The time allocate the first 8 bytes (double)
  record[8] = BusChannel();
  record[9] = static_cast<uint8_t>(CycleCount()) & 0x3F;
  record[9] |= (static_cast<uint8_t>(FlexRayChannelMask()) & 0x03) << 6;
  if (mandatory_members_only) {
    return;
  }

  record[10] = Length();
  record[11] = static_cast<uint8_t>(Type());
}

}  // namespace mdf