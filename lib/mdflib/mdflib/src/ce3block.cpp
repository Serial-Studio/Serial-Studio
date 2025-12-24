/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "ce3block.h"

namespace mdf::detail {
uint64_t Ce3Block::Read(std::streambuf& buffer) {
  uint64_t bytes = ReadHeader3(buffer);
  bytes += ReadNumber(buffer, type_);
  switch (type_) {
    case 2:
      bytes += ReadNumber(buffer, nof_module_);
      bytes += ReadNumber(buffer, address_);
      bytes += ReadStr(buffer, description_, 80);
      bytes += ReadStr(buffer, ecu_, 32);
      break;

    case 19:
      bytes += ReadNumber(buffer, message_id_);
      bytes += ReadNumber(buffer, index_);
      bytes += ReadStr(buffer, message_, 36);
      bytes += ReadStr(buffer, sender_, 36);
      break;

    default:
      break;
  }

  return bytes;
}
}  // namespace mdf::detail
