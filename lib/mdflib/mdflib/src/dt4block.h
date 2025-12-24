/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <cstdio>

#include "datablock.h"
namespace mdf::detail {


class Dt4Block : public DataBlock {
 public:
  Dt4Block();
  void Init(const MdfBlock &id_block) override;
  void GetBlockProperty(BlockPropertyList& dest) const override;
  uint64_t Read(std::streambuf& buffer) override;
  uint64_t Write(std::streambuf& buffer) override;
  [[nodiscard]] uint64_t DataSize() const override;
  void UpdateDataSize(std::streambuf& buffer);

};

}  // namespace mdf::detail
