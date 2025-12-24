/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "di4block.h"

namespace mdf::detail {

void Di4Block::GetBlockProperty(BlockPropertyList &dest) const {
  MdfBlock::GetBlockProperty(dest);
  dest.emplace_back("Information", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Data Size [byte]", std::to_string(DataSize()));
}

uint64_t Di4Block::Read(std::streambuf& buffer) {
  uint64_t bytes = ReadHeader4(buffer);
  data_position_ = GetFilePosition(buffer);
  return bytes;
}

uint64_t Di4Block::DataSize() const {
  return block_length_ > 24 ? block_length_ - 24 : 0;
}

}  // namespace mdf::detail