/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "tx3block.h"

#include <cstdio>
#include <sstream>
#include <utility>

#include "mdf/mdfhelper.h"

namespace mdf::detail {

Tx3Block::Tx3Block(std::string text) : text_(std::move(text)) {}

uint64_t Tx3Block::Read(std::streambuf& buffer) {
  uint64_t bytes = ReadHeader3(buffer);
  bytes += ReadStr(buffer, text_, block_length_ - bytes);
  return bytes;
}

std::string Tx3Block::Text() const {
  std::string temp = text_;
  MdfHelper::Trim(temp);
  return temp;
}

uint64_t Tx3Block::Write(std::streambuf& buffer) {
  block_type_ = "TX";
  block_size_ = static_cast<uint16_t>((2 + 2) + text_.size() + 1);
  link_list_.clear();

  auto bytes = MdfBlock::Write(buffer);
  bytes += buffer.sputn(text_.data(),
                        static_cast<std::streamsize>(text_.size()));
  constexpr char blank = '\0';
  buffer.sputc(blank);
  ++bytes;
  return bytes;
}

}  // namespace mdf::detail