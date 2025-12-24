/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "rv4block.h"

namespace mdf::detail {

uint64_t Rv4Block::Read(std::streambuf& buffer) {
  uint64_t bytes = ReadHeader4(buffer);
  data_position_ = GetFilePosition(buffer);
  return bytes;
}

uint64_t Rv4Block::DataSize() const {
  return block_length_ > 24 ? block_length_ - 24 : 0;
}

}  // namespace mdf::detail