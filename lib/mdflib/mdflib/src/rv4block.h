/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include "datablock.h"

namespace mdf::detail {

class Rv4Block : public DataBlock {
 public:
  uint64_t Read(std::streambuf& buffer) override;

 protected:
  [[nodiscard]] uint64_t DataSize() const override;

 private:
};

}  // namespace mdf::detail
