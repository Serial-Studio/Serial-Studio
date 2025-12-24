/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <cstdio>

#include "datablock.h"
namespace mdf::detail {

class Di4Block : public DataBlock {
 public:
  void GetBlockProperty(BlockPropertyList& dest) const override;
  uint64_t Read(std::streambuf& buffer) override;
  [[nodiscard]] uint64_t DataSize() const override;
};

}  // namespace mdf::detail
