/*
* Copyright 2024 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include <algorithm>

#include "mdf/linmessage.h"
#include "mdf/mdfwriter.h"
#include "mdf/mdfhelper.h"
#include "mdf/idatagroup.h"

#include "littlebuffer.h"

namespace {
  constexpr uint8_t kLinIdMask = 0x3F; ///< 6 bit mask
  constexpr uint8_t kDirMask = 0x80;   ///< High bit mask
  constexpr uint8_t kReceivedMask = 0x0F; ///< Low 4 bit
  constexpr uint8_t kLengthMask = 0xF0;   ///< High 4 bit
  constexpr uint8_t kExpectedLength = 0x0F;
  constexpr uint8_t kDominantTypeMask = 0x30;

  void FillDataBytes(std::vector<uint8_t>& record, uint16_t offset,
                     const std::vector<uint8_t>& data_bytes) {
    constexpr uint16_t kMaxDataBytes = 8;
    const uint16_t copy_size = std::min(static_cast<uint16_t>(data_bytes.size()),
      kMaxDataBytes);
    std::copy_n(data_bytes.begin(), copy_size, record.begin() + offset);

    if (copy_size < kMaxDataBytes) {
      constexpr uint8_t kPaddingByte = 0xFF;
      std::fill_n(record.begin() + offset + copy_size,
                  kMaxDataBytes - copy_size, kPaddingByte);
    }
  }

}

namespace mdf {

LinMessage::LinMessage(const MdfWriter&r)
: LinMessage() {
}


void LinMessage::BusChannel(uint8_t channel) {
  bus_channel_  = channel;
}

uint8_t LinMessage::BusChannel() const {
  return bus_channel_;
}

void LinMessage::LinId(uint8_t id) {
  lin_id_ &= ~kLinIdMask;
  lin_id_ |= (id & kLinIdMask);
}

uint8_t LinMessage::LinId() const {
  return lin_id_ & kLinIdMask;
}

void LinMessage::Dir(bool transmit) {
  if (transmit) {
    lin_id_ |= kDirMask;
  } else {
    lin_id_ &= ~kDirMask;
  }
}

bool LinMessage::Dir() const {
  return (lin_id_ & kDirMask) != 0;
}

void LinMessage::ReceivedDataByteCount(uint8_t nof_bytes) {
  data_length_ &= ~kReceivedMask;
  data_length_ |= nof_bytes & kReceivedMask;
}

uint8_t LinMessage::ReceivedDataByteCount() const {
  return data_length_ & kReceivedMask;
}

void LinMessage::DataLength(uint8_t nof_bytes) {
  data_length_ &= ~kLengthMask;
  data_length_ |= (nof_bytes << 4) & kLengthMask;
}

uint8_t LinMessage::DataLength() const {
  return (data_length_ & kLengthMask) >> 4;
}

void LinMessage::ChecksumModel(LinChecksumModel model) {
  checksum_model_ = model;
}

LinChecksumModel LinMessage::ChecksumModel() const {
  return checksum_model_;
}

void LinMessage::Reset() {
  bus_channel_ = 0x00;
  lin_id_ = 0;
  data_length_ = 0;
  checksum_model_ = LinChecksumModel::Unknown;
  crc_ = 0;
  sof_ = 0;
  baudrate_ = 0.0;
  response_baudrate_ = 0.0;
  break_length_ = 0;
  delimiter_break_length_ = 0;
  data_bytes_.clear();
  data_bytes_.shrink_to_fit();
  spare_ = 0;
  total_signal_length_ = 0;
}

void LinMessage::ToRaw( LinMessageType msg_type, SampleRecord& sample) const {

  switch (msg_type) {
    case LinMessageType::LIN_Frame:
      MakeDataFrame(sample);
      break;

    case LinMessageType::LIN_WakeUp:
      MakeWakeUp(sample);
      break;

    case LinMessageType::LIN_ChecksumError:
      MakeChecksumError(sample);
      break;

    case LinMessageType::LIN_TransmissionError:
      MakeTransmissionError(sample);
      break;

    case LinMessageType::LIN_SyncError:
      MakeSyncError(sample);
      break;

    case LinMessageType::LIN_ReceiveError:
      MakeReceiveError(sample);
      break;

    case LinMessageType::LIN_Spike:
      MakeSpike(sample);
      break;

    case LinMessageType::LIN_LongDominantSignal:
      MakeLongDominantSignal(sample);
      break;

    default:
      break;
  }
}

void LinMessage::MakeDataFrame(SampleRecord& sample) const {
  auto& record = sample.record_buffer;
  const bool mandatory = data_group_ != nullptr ? data_group_->MandatoryMembersOnly() : false;
  const size_t record_size = mandatory ? 19 : 45;

  if (record.size() != record_size) {
    record.resize(record_size);
  }
  // First 8 bytes are time in seconds (double)
  record[8] = BusChannel();
  record[9] = LinId() & kLinIdMask;
  record[9] |= (Dir() ? 0x01 : 0x00) << 6;
  record[10] = ReceivedDataByteCount();
  record[10] |= (DataLength() & 0x0F) << 4;
  FillDataBytes(record, 11, data_bytes_);// The MLSD storage is fixed in LIN.

  if (mandatory) {
    return;
  }

  record[19] = crc_;
  record[20] = static_cast<int8_t>(ChecksumModel()) & 0x03;
  LittleBuffer sof(sof_);
  std::copy(sof.cbegin(), sof.cend(),
            record.begin() + 21);

  LittleBuffer baudrate(baudrate_);
  std::copy(baudrate.cbegin(), baudrate.cend(),
            record.begin() + 29);

  LittleBuffer response_baudrate(response_baudrate_);
  std::copy(response_baudrate.cbegin(), response_baudrate.cend(),
            record.begin() + 33);

  LittleBuffer break_length(break_length_);
  std::copy(break_length.cbegin(), break_length.cend(),
            record.begin() + 37);

  LittleBuffer delimiter_break_length(delimiter_break_length_);
  std::copy(delimiter_break_length.cbegin(), delimiter_break_length.cend(),
            record.begin() + 41);
}

void LinMessage::MakeWakeUp(SampleRecord& sample) const {
  auto& record = sample.record_buffer;
  constexpr size_t record_size = 21;
  if (record.size() != record_size) {
    record.resize(record_size);
  }
  // First 8 byte is time in seconds (double)
  record[8 + 0] = BusChannel();
  LittleBuffer baudrate(baudrate_);
  std::copy(baudrate.cbegin(), baudrate.cend(), record.begin() + 9);

  LittleBuffer sof(sof_);
  std::copy(sof.cbegin(), sof.cend(), record.begin() + 13);
}

void LinMessage::MakeChecksumError(SampleRecord& sample) const {
  auto& record = sample.record_buffer;
  const bool mandatory = data_group_ != nullptr ?
    data_group_->MandatoryMembersOnly() : false;
  const size_t record_size = mandatory ? 10 : 45;

  if (record.size() != record_size) {
    record.resize(record_size);
  }
  // First 8 bytes are time in seconds (double)
  record[8] = BusChannel();
  record[9] = LinId() & kLinIdMask;
  if (mandatory) {
    return;
  }
  record[9] |= (Dir() ? 0x01 : 0x00) << 6;
  record[10] = ReceivedDataByteCount();
  record[10] |= (DataLength() & 0x0F) << 4;
  FillDataBytes(record, 11, data_bytes_);// The MLSD storage is fixed in LIN.
  record[19] = crc_;
  record[20] = static_cast<int8_t>(ChecksumModel()) & 0x03;
  LittleBuffer sof(sof_);
  std::copy(sof.cbegin(), sof.cend(),
            record.begin() + 21);

  LittleBuffer baudrate(baudrate_);
  std::copy(baudrate.cbegin(), baudrate.cend(),
            record.begin() + 29);

  LittleBuffer response_baudrate(response_baudrate_);
  std::copy(response_baudrate.cbegin(), response_baudrate.cend(),
            record.begin() + 33);

  LittleBuffer break_length(break_length_);
  std::copy(break_length.cbegin(), break_length.cend(),
            record.begin() + 37);

  LittleBuffer delimiter_break_length(delimiter_break_length_);
  std::copy(delimiter_break_length.cbegin(), delimiter_break_length.cend(),
            record.begin() + 41);
}

void LinMessage::MakeTransmissionError(SampleRecord& sample) const {
  auto& record = sample.record_buffer;
  const bool mandatory = data_group_ != nullptr ?
    data_group_->MandatoryMembersOnly() : false;
  const size_t record_size = mandatory ? 10 : 31;
  if (record.size() != record_size) {
    record.resize(record_size);
  }
  // First 8 byte is time in seconds (double)
  record[8] = BusChannel();
  record[9] = lin_id_ & kLinIdMask;

  if (mandatory) {
    return;
  }

  record[10] = ExpectedDataByteCount() & 0x0F;
  record[10] |= (static_cast<int8_t>(ChecksumModel()) & 0x03) << 4;

  LittleBuffer sof(sof_);
  std::copy(sof.cbegin(), sof.cend(), record.begin() + 11);

  LittleBuffer baudrate(baudrate_);
  std::copy(baudrate.cbegin(), baudrate.cend(), record.begin() + 19);

  LittleBuffer break_length(break_length_);
  std::copy(break_length.cbegin(), break_length.cend(),
            record.begin() + 23);

  LittleBuffer delimiter_break_length(delimiter_break_length_);
  std::copy(delimiter_break_length.cbegin(), delimiter_break_length.cend(),
            record.begin() + 27);
}

void LinMessage::MakeSyncError(SampleRecord& sample) const {
  auto& record = sample.record_buffer;
  constexpr size_t record_size = 29;
  if (record.size() != record_size) {
    record.resize(record_size);
  }
  // First 8 byte is time in seconds (double)
  record[8] = BusChannel();

  LittleBuffer sof(sof_);
  std::copy(sof.cbegin(), sof.cend(), record.begin() + 9);

  LittleBuffer baudrate(baudrate_);
  std::copy(baudrate.cbegin(), baudrate.cend(), record.begin() + 17);

  LittleBuffer break_length(break_length_);
  std::copy(break_length.cbegin(), break_length.cend(),
            record.begin() + 21);

  LittleBuffer delimiter_break_length(delimiter_break_length_);
  std::copy(delimiter_break_length.cbegin(), delimiter_break_length.cend(),
            record.begin() + 25);
}

void LinMessage::MakeReceiveError(SampleRecord& sample) const {
  auto& record = sample.record_buffer;
  constexpr size_t record_size = 44;
  if (record.size() != record_size) {
    record.resize(record_size);
  }
  // First 8 byte is time in seconds (double)
  record[8] = BusChannel();
  record[9] = LinId() & kLinIdMask;
  record[10] = ReceivedDataByteCount() & 0x0F;
  record[10] |= (ExpectedDataByteCount() & 0x0F ) << 4;
  record[11] = DataLength() & 0x0F;
  record[11] |= (static_cast<int8_t>(ChecksumModel()) & 0x03) << 4;
  FillDataBytes(record, 12, data_bytes_);// The MLSD storage is fixed in LIN.

  LittleBuffer sof(sof_);
  std::copy(sof.cbegin(), sof.cend(),
            record.begin() + 20);

  LittleBuffer baudrate(baudrate_);
  std::copy(baudrate.cbegin(), baudrate.cend(),
            record.begin() + 28);

  LittleBuffer response_baudrate(response_baudrate_);
  std::copy(response_baudrate.cbegin(), response_baudrate.cend(),
            record.begin() + 32);

  LittleBuffer break_length(break_length_);
  std::copy(break_length.cbegin(), break_length.cend(),
            record.begin() + 36);

  LittleBuffer delimiter_break_length(delimiter_break_length_);
  std::copy(delimiter_break_length.cbegin(), delimiter_break_length.cend(),
            record.begin() + 40);
}

void LinMessage::MakeSpike(SampleRecord& sample) const {
  auto& record = sample.record_buffer;
  constexpr size_t record_size = 21;
  if (record.size() != record_size) {
    record.resize(record_size);
  }
  // First 8 byte is time in seconds (double)
  record[8] = BusChannel();

  LittleBuffer baudrate(baudrate_);
  std::copy(baudrate.cbegin(), baudrate.cend(),
            record.begin() + 9);

  LittleBuffer sof(sof_);
  std::copy(sof.cbegin(), sof.cend(),
            record.begin() + 13);
}

void LinMessage::MakeLongDominantSignal(SampleRecord& sample) const {
  auto& record = sample.record_buffer;
  constexpr size_t record_size = 26;
  if (record.size() != record_size) {
    record.resize(record_size);
  }
  // First 8 byte is time in seconds (double)
  record[8] = BusChannel();

  LittleBuffer baudrate(baudrate_);
  std::copy(baudrate.cbegin(), baudrate.cend(), record.begin() + 9);

  LittleBuffer sof(sof_);
  std::copy(sof.cbegin(), sof.cend(), record.begin() + 13);

  LittleBuffer length(total_signal_length_);
  std::copy(length.cbegin(), length.cend(), record.begin() + 21);

  record[25] = static_cast<uint8_t>(TypeOfLongDominantSignal());
}

void LinMessage::ExpectedDataByteCount(uint8_t nof_bytes) {
  spare_ &= ~kExpectedLength;
  spare_ |= nof_bytes & kExpectedLength;
}

uint8_t LinMessage::ExpectedDataByteCount() const {
  return spare_ & kExpectedLength;
}

void LinMessage::TypeOfLongDominantSignal(LinTypeOfLongDominantSignal type) {
  auto temp = static_cast<uint8_t>(type);
  temp <<= 4;
  spare_ &= ~kDominantTypeMask;
  spare_ |= temp;
}

LinTypeOfLongDominantSignal LinMessage::TypeOfLongDominantSignal() const {
  return static_cast<LinTypeOfLongDominantSignal>((spare_ & kDominantTypeMask) >>4);
}

void LinMessage::DataBytes(std::vector<uint8_t> data_bytes) {
  DataLength(static_cast<uint8_t>(data_bytes.size()));
  data_bytes_ = std::move(data_bytes);
}

}  // namespace mdf