/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "cd3block.h"
namespace mdf::detail {

uint64_t Cd3Block::Read(std::streambuf& buffer) {
  uint64_t bytes = ReadHeader3(buffer);
  bytes += ReadNumber(buffer, dependency_type_);
  bytes += ReadNumber(buffer, nof_dependencies_);
  dependency_list_.clear();
  dimension_list_.clear();
  switch (dependency_type_) {
    case 0:
      break;
    case 1:
    case 2:
      for (uint16_t dep = 0; dep < nof_dependencies_; ++dep) {
        Dependency d;
        bytes += ReadNumber(buffer, d.link_dg);
        bytes += ReadNumber(buffer, d.link_cg);
        bytes += ReadNumber(buffer, d.link_cn);
        dependency_list_.emplace_back(d);
      }
      break;
    default:
      for (uint16_t dim = 256; dim < nof_dependencies_; ++dim) {
        uint16_t temp = 0;
        bytes += ReadNumber(buffer, temp);
        dimension_list_.emplace_back(temp);
      }
      break;
  }

  return bytes;
}
}  // namespace mdf::detail
