/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "sr3block.h"

#include "mdf/ichannelgroup.h"
#include "cg3block.h"

namespace {

constexpr size_t kIndexSr = 0;
constexpr size_t kIndexData = 1;

}  // namespace

namespace mdf::detail {
int64_t Sr3Block::Index() const {
  return MdfBlock::Index();
}

std::string Sr3Block::BlockType() const {
  return MdfBlock::BlockType();
}

void Sr3Block::NofSamples(uint64_t nof_samples) {
  nof_reduced_samples_ = static_cast<uint32_t>(nof_samples);
}

uint64_t Sr3Block::NofSamples() const { return nof_reduced_samples_; }

void Sr3Block::Interval(double interval) {time_interval_ = interval;}

double Sr3Block::Interval() const { return time_interval_; }

const IChannelGroup *Sr3Block::ChannelGroup() const {
  const auto* mdf_block = CgBlock();
  return mdf_block != nullptr ? dynamic_cast<const IChannelGroup*>(mdf_block) : nullptr;
}

uint64_t Sr3Block::Read(std::streambuf& buffer) {
  uint64_t bytes = ReadHeader3(buffer);
  bytes += ReadLinks3(buffer, 2);
  bytes += ReadNumber(buffer, nof_reduced_samples_);
  bytes += ReadNumber(buffer, time_interval_);
  return bytes;
}

uint64_t Sr3Block::Write(std::streambuf& buffer) {
  // SR block must be written with its data once with no updates
  const bool update = FilePosition() > 0;
  if (update) {
    return block_size_;
  }
  block_type_ = "SR";
  block_size_ = (2 + 2) + (2 * 4) + 4 + 8;
  link_list_.resize(2, 0);
  uint64_t bytes = MdfBlock::Write(buffer);
  bytes += WriteNumber(buffer, nof_reduced_samples_);
  bytes += WriteNumber(buffer, time_interval_);

  for (size_t index = 0; index < block_list_.size(); ++index) {
    auto &data = block_list_[index];
    if (!data) {
      continue;
    }
    if (data->FilePosition() > 0) {
      continue;
    }
    data->Write(buffer);
    if (index == 0) {
      UpdateLink(buffer, kIndexData, data->FilePosition());
    }
  }

  return bytes;
}


void Sr3Block::ReadData(std::streambuf& buffer) const {
  uint64_t data_size = DataSize();
  if (data_size == 0) {
    data_list_.clear();
    return;
  }
  try {
    data_list_.resize(static_cast<size_t>(data_size), 0);
  } catch (const std::exception&) {
    data_list_.clear();
    return;
  }

  SetFilePosition(buffer, Link(kIndexData));
  ReadByte(buffer,data_list_, data_size);
}

void Sr3Block::ClearData() {
  DataListBlock::ClearData();
  data_list_.clear();
}

void Sr3Block::GetChannelValueUint( const IChannel& channel, uint64_t sample,
                                   uint64_t array_index, SrValue<uint64_t>& value ) const {
  GetChannelValueT(channel, sample, array_index, value);
}
void Sr3Block::GetChannelValueInt( const IChannel& channel, uint64_t sample,
                                   uint64_t array_index, SrValue<int64_t>& value ) const {
  GetChannelValueT(channel, sample, array_index, value);
}

void Sr3Block::GetChannelValueDouble( const IChannel& channel, uint64_t sample,
                                      uint64_t array_index, SrValue<double>& value ) const {
  GetChannelValueT(channel, sample, array_index, value);
}

uint16_t Sr3Block::BlockSize() const {
  const auto* channel_group = ChannelGroup();
  if (channel_group == nullptr) {
    return 0;
  }

  const auto* cg3 = dynamic_cast<const Cg3Block*>(channel_group);
  if (cg3 == nullptr) {
    return 0;
  }

  return cg3->RecordSize();
}


}  // namespace mdf::detail