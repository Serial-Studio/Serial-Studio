/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include "datablock.h"

namespace mdf::detail {

class Rd4Block : public DataBlock {
 public:
  uint64_t Read(std::streambuf& buffer) override;

 protected:
  [[nodiscard]] uint64_t DataSize() const override;
};

}  // namespace mdf::detail
