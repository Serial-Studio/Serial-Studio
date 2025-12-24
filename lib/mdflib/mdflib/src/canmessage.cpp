/*
* Copyright 2023 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */
#include <array>

#include "mdf/canmessage.h"
#include "mdf/mdfhelper.h"
#include "mdf/ichannel.h"
#include "mdf/ichannelgroup.h"
#include "mdf/idatagroup.h"



namespace {

constexpr uint32_t kExtendedBit    = 0x80000000;
constexpr uint32_t k11BitMask      = 0x7FF;
constexpr size_t kDirBit         = 0;
constexpr size_t kSrrBit         = 1;
constexpr size_t kEdlBit         = 2;
constexpr size_t kBrsBit         = 3;
constexpr size_t kEsiBit         = 4;
constexpr size_t kWakeUpBit      = 5;
constexpr size_t kSingleWireBit  = 6;
constexpr size_t kRtrBit         = 7;
constexpr size_t kR0Bit          = 8;
constexpr size_t kR1Bit          = 9;

constexpr std::array<size_t,16> kDataLengthCode =
    {0,1,2,3,4,5,6,7,8,12,16,20,24,32,48,64};

} // end namespace

namespace mdf {

void CanMessage::MessageId(uint32_t msg_id) {
  message_id_ = msg_id;
  if (msg_id > k11BitMask) {
    // more than 11 bit means extended ID
    message_id_ |= kExtendedBit;
  }
}

uint32_t CanMessage::MessageId() const { return message_id_; }

uint32_t CanMessage::CanId() const { return message_id_ & ~kExtendedBit; }

void CanMessage::ExtendedId(bool extended) {
  if (extended) {
    message_id_ |= kExtendedBit;
  } else {
    message_id_ &= ~kExtendedBit;
  }
}

bool CanMessage::ExtendedId() const {
  return (message_id_ & kExtendedBit) != 0;
}

void CanMessage::Dlc(uint8_t dlc) {
  dlc_ =  dlc;
}

uint8_t CanMessage::Dlc() const { return dlc_ & 0x0F; }

void CanMessage::DataLength(uint8_t data_length) {
  if (data_length != data_bytes_.size()) {
    data_bytes_.resize(data_length);
  }

  uint8_t dlc = 0;
  for (const auto data_size : kDataLengthCode) {
    if (data_length <= data_size) {
      break;
    }
    ++dlc;
  }
  Dlc(dlc);
}

uint8_t CanMessage::DataLength() const {
  return static_cast<uint8_t>(data_bytes_.size());
}

void CanMessage::DataBytes(std::vector<uint8_t> data) {
  DataLength(static_cast<uint8_t>(data.size()));
  data_bytes_ = std::move(data);
}

const std::vector<uint8_t>& CanMessage::DataBytes() const {
  return data_bytes_;
}

void CanMessage::Dir(bool transmit) {
  flags_.set(kDirBit, transmit);
}

bool CanMessage::Dir() const {
  return flags_.test(kDirBit);
}

void CanMessage::Srr(bool srr) {
  flags_.set(kSrrBit, srr);
}

bool CanMessage::Srr() const {
  return flags_.test(kSrrBit);
}

void CanMessage::Edl(bool edl) {
  flags_.set(kEdlBit, edl);
}

bool CanMessage::Edl() const {
  return flags_.test(kEdlBit);
}

void CanMessage::Brs(bool brs) {
  flags_.set(kBrsBit, brs);
}

bool CanMessage::Brs() const {
  return flags_.test(kBrsBit);
}

void CanMessage::Esi(bool esi) {
  flags_.set(kEsiBit, esi);
}

bool CanMessage::Esi() const {
  return flags_.test(kEsiBit);
}

void CanMessage::Rtr(bool rtr) {
  flags_.set(kRtrBit, rtr);
}

bool CanMessage::Rtr() const {
  return flags_.test(kRtrBit);
}

void CanMessage::WakeUp(bool wake_up) {
  flags_.set(kWakeUpBit, wake_up);
}

bool CanMessage::WakeUp() const {
  return flags_.test( kWakeUpBit);
}

void CanMessage::SingleWire(bool single_wire) {
  flags_.set(kSingleWireBit, single_wire);
}

bool CanMessage::SingleWire() const {
  return flags_.test(kSingleWireBit);
}

void CanMessage::R0(bool flag) {
  flags_.set(kR0Bit, flag);
}

bool CanMessage::R0() const {
  return flags_.test(kR0Bit);
}

void CanMessage::R1(bool flag) {
  flags_.set(kR1Bit, flag);
}

bool CanMessage::R1() const {
  return flags_.test(kR1Bit);
}

void CanMessage::BusChannel(uint8_t channel) {
  bus_channel_ = channel;
}

uint8_t CanMessage::BusChannel() const {
  return bus_channel_;
}

void CanMessage::BitPosition(uint16_t position) {
  bit_position_ = position;
}

uint16_t CanMessage::BitPosition() const {
  return bit_position_;
}

void CanMessage::ErrorType(mdf::CanErrorType error_type) {
  error_type_ =error_type;
}

CanErrorType CanMessage::ErrorType() const {
  return error_type_;
}

void CanMessage::FrameDuration(uint32_t duration) {
  frame_duration_ = duration;
}

uint32_t CanMessage::FrameDuration() const {
  return frame_duration_;
}

void CanMessage::Crc(uint32_t crc) {
  crc_ = crc;
}

uint32_t CanMessage::Crc() const {
  return crc_;
}

void CanMessage::Timestamp(double timestamp) {
  timestamp_ = timestamp;
}

double CanMessage::Timestamp() const {
  return timestamp_;
}

size_t CanMessage::DlcToLength(uint8_t dlc) {
  return dlc < kDataLengthCode.size() ? kDataLengthCode[dlc] : 0;
}

void CanMessage::ToRaw( MessageType msg_type, SampleRecord& sample,
                       size_t max_data_length, bool save_index ) const {
  constexpr uint64_t data_index = 0;
  const bool mandatory_only = data_group_ != nullptr ? data_group_->MandatoryMembersOnly() : false;
  size_t record_size = 0;
  // Ensure that 8 bytes is added
  if (max_data_length < 8) {
    max_data_length = 8;
  }

  auto& record = sample.record_buffer;
  switch (msg_type) {
    case MessageType::CAN_DataFrame:
      record_size = mandatory_only ? 23 - 8 : 32 - 8;
      record_size += save_index ? 8 : max_data_length;
      if (record.size() != record_size) {
        record.resize(record_size);
      }
      record[8] = BusChannel();
      MdfHelper::UnsignedToRaw(true, 0, 32, MessageId(), record.data() + 9);
      record[13] =  Dlc();
      record[14] = static_cast<uint8_t>(DataLength());
      if (mandatory_only) {
        if (save_index) {
          // The data index have in reality not been updated at this point, but
          // it will be updated when the sample buffer is written to the disc.
          // We need to save the data bytes to a temp buffer (VLSD data).
          sample.vlsd_data = true;
          sample.vlsd_buffer = data_bytes_;
          MdfHelper::UnsignedToRaw(true, 0, 64, data_index, record.data() + 15);
        } else {
          sample.vlsd_data = false;
          sample.vlsd_buffer.clear();
          sample.vlsd_buffer.shrink_to_fit();
          for (size_t index = 0; index < max_data_length; ++index) {
            record[15 + index] =
                index < data_bytes_.size() ? data_bytes_[index] : 0xFF;
          }
        }
        break;
      }
      record[13] |= (Dir() ? 0x01 : 0x00) << 4;
      record[13] |= (Srr() ? 0x01 : 0x00) << 5;
      record[13] |= (Edl() ? 0x01 : 0x00) << 6;
      record[13] |= (Brs() ? 0x01 : 0x00) << 7;
      record[15] = Esi() ? 0x01 : 0x00;
      record[15] |= (WakeUp() ? 0x01 : 0x00) << 1;
      record[15] |= (SingleWire() ? 0x01 : 0x00) << 2;
      record[15] |= (R0() ? 0x01 : 0x00) << 3;
      record[15] |= (R1() ? 0x01 : 0x00) << 4;
      MdfHelper::UnsignedToRaw(true, 0, 32, FrameDuration(), record.data() + 16);
      MdfHelper::UnsignedToRaw(true, 0, 32, Crc(), record.data() + 20);
      if (save_index) {
        // The data index have in reality not been updated at this point, but
        // it will be updated when the sample buffer is written to the disc.
        // We need to save the data bytes to a temp buffer (VLSD data).
        sample.vlsd_data = true;
        sample.vlsd_buffer = data_bytes_;
        MdfHelper::UnsignedToRaw(true, 0, 64, data_index, record.data() + 24);
      } else {
        sample.vlsd_data = false;
        sample.vlsd_buffer.clear();
        sample.vlsd_buffer.shrink_to_fit();
        for (size_t index = 0; index < max_data_length; ++index) {
          record[24 + index] =
              index < data_bytes_.size() ? data_bytes_[index] : 0xFF;
        }
      }
      break;

    case MessageType::CAN_RemoteFrame:
      record_size = mandatory_only ? 15 : 24 ;
      if (record.size() != record_size) {
        record.resize(record_size);
      }
      record[8] = BusChannel();
      MdfHelper::UnsignedToRaw(true, 0, 32, MessageId(), record.data() + 9);
      record[13] = Dlc() & 0x0F;
      record[14] = static_cast<uint8_t>(DataLength());
      if (mandatory_only) {
        break;
      }
      record[13] |= (Dir() ? 0x01 : 0x00) << 4;
      record[13] |= (Srr() ? 0x01 : 0x00) << 5;
      record[13] |= (WakeUp() ? 0x01 : 0x00) << 6;
      record[13] |= (SingleWire() ? 0x01 : 0x00) << 7;
      record[15] = R0() ? 0x01 : 0x00;
      record[15] |= (R1() ? 0x01 : 0x00) << 1;
      MdfHelper::UnsignedToRaw(true, 0, 32, FrameDuration(), record.data() + 16);
      MdfHelper::UnsignedToRaw(true, 0, 32, Crc(), record.data() + 20);
      break;

    case MessageType::CAN_ErrorFrame:
      record_size = 34 - 8;
      record_size += save_index ? 8 : max_data_length;
      if (record.size() < record_size) {
        record.resize(record_size);
      }
      record[8] = BusChannel();
      MdfHelper::UnsignedToRaw(true, 0, 32, MessageId(), record.data() + 9);
      record[13] = Dlc() & 0x0F;
      record[14] = DataLength();
      record[13] |= (Dir() ? 0x01 : 0x00) << 4;
      record[13] |= (Srr() ? 0x01 : 0x00) << 5;
      record[13] |= (Edl() ? 0x01 : 0x00) << 6;
      record[13] |= (Brs() ? 0x01 : 0x00) << 7;
      record[15] = Esi() ? 0x01 : 0x00;
      record[15] |= (WakeUp() ? 0x01 : 0x00) << 1;
      record[15] |= (SingleWire() ? 0x01 : 0x00) << 2;
      record[15] |= (R0() ? 0x01 : 0x00) << 3;
      record[15] |= (R1() ? 0x01 : 0x00) << 4;
      record[15] |= (static_cast<uint8_t>(ErrorType()) & 0x07) << 5;
      MdfHelper::UnsignedToRaw(true, 0, 32, FrameDuration(), record.data() + 16);
      MdfHelper::UnsignedToRaw(true, 0, 16, BitPosition(), record.data() + 20);
      MdfHelper::UnsignedToRaw(true, 0, 32, Crc(), record.data() + 22);
      if (save_index) {
        // The data index have in reality not been updated at this point, but
        // it will be updated when the sample buffer is written to the disc.
        // We need to save the data bytes to a temp buffer (VLSD data).
        sample.vlsd_data = true;
        sample.vlsd_buffer = data_bytes_;
        MdfHelper::UnsignedToRaw(true, 0, 64, data_index, record.data() + 26);
      } else {
        sample.vlsd_data = false;
        sample.vlsd_buffer.clear();
        sample.vlsd_buffer.shrink_to_fit();
        for (size_t index = 0; index < max_data_length; ++index) {
          record[26 + index] =
              index < data_bytes_.size() ? data_bytes_[index] : 0xFF;
        }
      }
      break;


    case MessageType::CAN_OverloadFrame:
      record_size = 10;
      if (record.size() != record_size) {
        record.resize(record_size);
      }
      record[8] = BusChannel();
      record[9] = Dir() ? 0x01 : 0x00;
      break;

    default:
      break;
  }
}

void CanMessage::Reset() {
  bus_channel_ = 0;
  message_id_ = 0;
  dlc_ = 0;
  flags_.reset();
  data_bytes_.clear();
  data_bytes_.shrink_to_fit();
  bit_position_ = 0;
  error_type_ = CanErrorType::UNKNOWN_ERROR;
  frame_duration_ = 0;
  data_group_ = nullptr;
  channel_group_ = nullptr;
}

}  // namespace mdf