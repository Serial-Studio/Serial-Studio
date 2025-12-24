/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "dt3block.h"

#include <stdexcept>
#include "mdf/idatagroup.h"
#include "dg3block.h"

namespace mdf::detail {

void Dt3Block::GetBlockProperty(BlockPropertyList &dest) const {
  MdfBlock::GetBlockProperty(dest);
  dest.emplace_back("Information", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Data Size [byte]", std::to_string(DataSize()));
}

uint64_t Dt3Block::Read(std::streambuf& buffer) {
  block_type_ = "DT";
  file_position_ = GetFilePosition(buffer);
  data_position_ = file_position_;
  return 0;
}

uint64_t Dt3Block::Write(std::streambuf& buffer) {
  const bool update = FilePosition() > 0;
  if (update) {
    return block_size_;
  }
  SetLastFilePosition(buffer);
  block_type_ = "DT";
  file_position_ = GetFilePosition(buffer);
  data_position_ = file_position_;
  // ToDo: Fix write of data
  return 0;
}

uint64_t Dt3Block::DataSize() const {
  uint64_t bytes = 0;
  if (dg_block_ == nullptr) {
    return 0;
  }
  auto* dg3_block = dynamic_cast<Dg3Block*>(dg_block_);
  if (dg3_block == nullptr) {
    return 0;
  }
  for (const auto &cg3 : dg3_block->Cg3()) {
    if (!cg3) {
      continue;
    }
    bytes += (dg3_block->NofRecordId() + cg3->RecordSize()) * cg3->NofSamples();
  }
  return bytes;
}

void Dt3Block::Init(const MdfBlock &id_block) {
  auto* temp = const_cast<MdfBlock*>(&id_block);
  dg_block_ = dynamic_cast<IDataGroup*>(temp);
  MdfBlock::Init(id_block);
}

}  // namespace mdf::detail