/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "pr3block.h"

#include <cstdio>
#include <sstream>
#include <utility>
namespace mdf::detail {

Pr3Block::Pr3Block(std::string meta_data) : text_(std::move(meta_data)) {}

uint64_t Pr3Block::Read(std::streambuf& buffer) {
  uint64_t bytes = ReadHeader3(buffer);
  bytes += ReadStr(buffer, text_, block_size_ - bytes);
  return bytes;
}

uint64_t Pr3Block::Write(std::streambuf& buffer) {
  block_type_ = "PR";
  block_size_ = static_cast<uint16_t>((2 + 2) + text_.size());
  link_list_.clear();

  uint64_t bytes = MdfBlock::Write(buffer);
  bytes += WriteStr(buffer, text_, text_.size());
  return bytes;
}

}  // namespace mdf::detail
