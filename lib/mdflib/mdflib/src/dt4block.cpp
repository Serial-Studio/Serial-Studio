/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "dt4block.h"
#include "mdf/mdflogstream.h"

namespace mdf::detail {

Dt4Block::Dt4Block() {
  block_type_ = "##DT";
}

void Dt4Block::Init(const MdfBlock &id_block) {
  auto* temp = const_cast<MdfBlock*>(&id_block);
  dg_block_ = dynamic_cast<IDataGroup*>(temp);
  MdfBlock::Init(id_block);
}

void Dt4Block::GetBlockProperty(BlockPropertyList &dest) const {
  MdfBlock::GetBlockProperty(dest);
  dest.emplace_back("Information", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Data Size [byte]", std::to_string(DataSize()));
}

uint64_t Dt4Block::Read(std::streambuf& buffer) {
  const uint64_t bytes = ReadHeader4(buffer);
  data_position_ = GetFilePosition(buffer);
  return bytes;
}

uint64_t Dt4Block::DataSize() const {
  return block_length_ > 24 ? block_length_ - 24 : 0;
}

void Dt4Block::UpdateDataSize(std::streambuf& buffer) {
  int64_t last_file_position = GetLastFilePosition(buffer);
  const auto data_size = last_file_position - data_position_;
  if (data_size > 0) {
    block_length_ = 24 + static_cast<uint64_t>(data_size);
  }
  MDF_TRACE() << "Updated the last DT block data size. Size: " << data_size;
}

uint64_t Dt4Block::Write(std::streambuf& buffer) {
  const auto update = FilePosition() > 0;
  if (!update) {
    block_type_ = "##DT";
    block_length_ = 24 + data_.size();  // The block length is updated by
                                        // the MDF4 writer
    link_list_.clear();
  }


  uint64_t bytes = update ? MdfBlock::Update(buffer) : MdfBlock::Write(buffer);

  if (!update) {
    data_position_ = GetFilePosition(buffer);
    bytes += WriteByte(buffer, data_);
  } else {
    bytes = block_length_;
  }
  return bytes;
}


}  // namespace mdf::detail