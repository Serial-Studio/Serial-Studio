/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include "mdfblock.h"

namespace mdf::detail {
class Tx3Block : public MdfBlock {
 public:
  explicit Tx3Block(std::string text);
  Tx3Block() = default;

  uint64_t Read(std::streambuf& buffer) override;
  uint64_t Write(std::streambuf& buffer) override;

  [[nodiscard]] std::string Text() const;

 private:
  std::string text_;
};
}  // namespace mdf::detail
