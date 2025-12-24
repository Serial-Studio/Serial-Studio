/*
* Copyright 2024 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include "mdf/ethmessage.h"
#include "mdf/mdfwriter.h"
#include "mdf/idatagroup.h"

#include "littlebuffer.h"

namespace mdf {
EthMessage::EthMessage(const MdfWriter& writer)
: EthMessage() {

}



void EthMessage::DataBytes(const std::vector<uint8_t>& data) {
  data_bytes_ = data;
  data_length_ = static_cast<uint16_t>(data_bytes_.size());
}

void EthMessage::Reset() {
  bus_channel_ = 0;
  source_ = {};
  destination_ = {};
  eth_type_ = 0x0800; ///< IPv4 as default
  received_data_byte_count_ = 0;
  data_length_ = 0;
  data_bytes_.clear();
  data_bytes_.shrink_to_fit();
  crc_ = 0;
  expected_crc_ = 0;
  padding_byte_count_ = 0;

}

void EthMessage::ToRaw(EthMessageType msg_type, SampleRecord& sample) const {
  switch (msg_type) {
    case EthMessageType::ETH_Frame:
      MakeDataFrame(sample);
      break;

    case EthMessageType::ETH_ChecksumError:
      MakeChecksumError(sample);
      break;

    case EthMessageType::ETH_LengthError:
      MakeLengthError(sample);
      break;

    case EthMessageType::ETH_ReceiveError:
      MakeReceiveError(sample);
      break;

    default:
      break;
  }

}

void EthMessage::MakeDataFrame(SampleRecord& sample) const {
  auto& record = sample.record_buffer;
  const bool mandatory = data_group_ != nullptr ? data_group_->MandatoryMembersOnly() : false;
  const size_t record_size = mandatory ? 35 : 42;

  if (record.size() != record_size) {
    record.resize(record_size);
  }
  // First 8 byte is time in seconds (double)
  record[8] = BusChannel();
  std::copy(source_.cbegin(), source_.cend(), record.begin() + 9);
  std::copy(destination_.cbegin(), destination_.cend(), record.begin() + 15);

  const LittleBuffer type(eth_type_);
  std::copy(type.cbegin(), type.cend(), record.begin() + 21);

  const LittleBuffer rec(received_data_byte_count_);
  std::copy(rec.cbegin(), rec.cend(), record.begin() + 23);

  const LittleBuffer length(data_length_);
  std::copy(length.cbegin(), length.cend(), record.begin() + 25);

  sample.vlsd_data = true;
  sample.vlsd_buffer = data_bytes_;

  constexpr uint64_t data_index = 0;
  const LittleBuffer index(data_index);
  if (mandatory) {
    // The data index have in reality not been updated at this point, but
    // it will be updated when the sample buffer is written to the disc.
    // We need to save the data bytes to a temp buffer (VLSD data).
    std::copy(index.cbegin(), index.cend(), record.begin() + 27);
    return;
  }

  const LittleBuffer crc(crc_);
  std::copy(crc.cbegin(), crc.cend(), record.begin() + 27);

  record[31] = Dir() ? 0x01 : 0x00;

  const LittleBuffer pad(padding_byte_count_);
  std::copy(pad.cbegin(), pad.cend(), record.begin() + 32);

  // The data index have in reality not been updated at this point, but
  // it will be updated when the sample buffer is written to the disc.
  // We need to save the data bytes to a temp buffer (VLSD data).
  std::copy(index.cbegin(), index.cend(), record.begin() + 34);
}

void EthMessage::MakeChecksumError(SampleRecord& sample) const {
  auto& record = sample.record_buffer;
  const bool mandatory = data_group_ != nullptr ? data_group_->MandatoryMembersOnly() : false;
  const size_t record_size = mandatory ? 33 : 46;

  if (record.size() != record_size) {
    record.resize(record_size);
  }

  // First 8 byte is time in seconds (double)
  record[8] = BusChannel();
  std::copy(source_.cbegin(), source_.cend(), record.begin() + 9);
  std::copy(destination_.cbegin(), destination_.cend(), record.begin() + 15);

  const LittleBuffer type(eth_type_);
  std::copy(type.cbegin(), type.cend(), record.begin() + 21);

  const LittleBuffer length(data_length_);
  std::copy(length.cbegin(), length.cend(), record.begin() + 23);

  const LittleBuffer crc(crc_);
  std::copy(crc.cbegin(), crc.cend(), record.begin() + 25);

  const LittleBuffer expected_crc(expected_crc_);
  std::copy(expected_crc.cbegin(), expected_crc.cend(), record.begin() + 29);

  if (mandatory) {
    sample.vlsd_data = false;
    sample.vlsd_buffer.clear();
    sample.vlsd_buffer.shrink_to_fit();
    return;
  }

  const LittleBuffer rec(received_data_byte_count_);
  std::copy(rec.cbegin(), rec.cend(), record.begin() + 33);

  record[35] = Dir() ? 0x01 : 0x00;

  const LittleBuffer pad(padding_byte_count_);
  std::copy(pad.cbegin(), pad.cend(), record.begin() + 36);
  // The data index have in reality not been updated at this point, but
  // it will be updated when the sample buffer is written to the disc.
  // We need to save the data bytes to a temp buffer (VLSD data).
  constexpr uint64_t data_index = 0;
  const LittleBuffer index(data_index);
  std::copy(index.cbegin(), index.cend(), record.begin() + 38);

  sample.vlsd_data = true;
  sample.vlsd_buffer = data_bytes_;
}

void EthMessage::MakeLengthError(SampleRecord& sample) const {
  auto& record = sample.record_buffer;
  const bool mandatory = data_group_ != nullptr ? data_group_->MandatoryMembersOnly() : false;
  const size_t record_size = mandatory ? 25 : 42;

  if (record.size() != record_size) {
    record.resize(record_size);
  }

  // First 8 byte is time in seconds (double)
  record[8] = BusChannel();
  std::copy(source_.cbegin(), source_.cend(), record.begin() + 9);
  std::copy(destination_.cbegin(), destination_.cend(), record.begin() + 15);

  const LittleBuffer type(eth_type_);
  std::copy(type.cbegin(), type.cend(), record.begin() + 21);

  const LittleBuffer rec(received_data_byte_count_);
  std::copy(rec.cbegin(), rec.cend(), record.begin() + 23);

  if (mandatory) {
    sample.vlsd_data = false;
    sample.vlsd_buffer.clear();
    sample.vlsd_buffer.shrink_to_fit();
    return;
  }

  const LittleBuffer length(data_length_);
  std::copy(length.cbegin(), length.cend(), record.begin() + 25);

  const LittleBuffer crc(crc_);
  std::copy(crc.cbegin(), crc.cend(), record.begin() + 27);

  record[31] = Dir() ? 0x01 : 0x00;

  const LittleBuffer pad(padding_byte_count_);
  std::copy(pad.cbegin(), pad.cend(), record.begin() + 32);
  // The data index have in reality not been updated at this point, but
  // it will be updated when the sample buffer is written to the disc.
  // We need to save the data bytes to a temp buffer (VLSD data).
  constexpr uint64_t data_index = 0;
  const LittleBuffer index(data_index);
  std::copy(index.cbegin(), index.cend(), record.begin() + 34);

  sample.vlsd_data = true;
  sample.vlsd_buffer = data_bytes_;
}

void EthMessage::MakeReceiveError(SampleRecord& sample) const {
  auto& record = sample.record_buffer;
  const bool mandatory = data_group_ != nullptr ? data_group_->MandatoryMembersOnly() : false;
  const size_t record_size = mandatory ? 25 : 42;

  if (record.size() != record_size) {
    record.resize(record_size);
  }

  // First 8 byte is time in seconds (double)
  record[8] = BusChannel();
  std::copy(source_.cbegin(), source_.cend(), record.begin() + 9);
  std::copy(destination_.cbegin(), destination_.cend(), record.begin() + 15);

  const LittleBuffer type(eth_type_);
  std::copy(type.cbegin(), type.cend(), record.begin() + 21);

  const LittleBuffer rec(received_data_byte_count_);
  std::copy(rec.cbegin(), rec.cend(), record.begin() + 23);

  if (mandatory) {
    sample.vlsd_data = false;
    sample.vlsd_buffer.clear();
    sample.vlsd_buffer.shrink_to_fit();
    return;
  }

  const LittleBuffer length(data_length_);
  std::copy(length.cbegin(), length.cend(), record.begin() + 25);

  const LittleBuffer crc(crc_);
  std::copy(crc.cbegin(), crc.cend(), record.begin() + 27);

  record[31] = Dir() ? 0x01 : 0x00;
  record[31] |= static_cast<uint8_t>(ErrorType()) << 1;

  const LittleBuffer pad(padding_byte_count_);
  std::copy(pad.cbegin(), pad.cend(), record.begin() + 32);
  // The data index have in reality not been updated at this point, but
  // it will be updated when the sample buffer is written to the disc.
  // We need to save the data bytes to a temp buffer (VLSD data).
  constexpr uint64_t data_index = 0;
  const LittleBuffer index(data_index);
  std::copy(index.cbegin(), index.cend(), record.begin() + 34);

  sample.vlsd_data = true;
  sample.vlsd_buffer = data_bytes_;
}

}  // namespace mdf