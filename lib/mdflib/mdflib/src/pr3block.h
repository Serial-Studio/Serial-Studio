/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <cstdio>
#include <string>

#include "mdfblock.h"

namespace mdf::detail {
class Pr3Block : public MdfBlock {
 public:
  Pr3Block() = default;
  explicit Pr3Block(std::string meta_data);

  uint64_t Read(std::streambuf& buffer) override;
  uint64_t Write(std::streambuf& buffer) override;

  [[nodiscard]] std::string Text() const { return text_; }

 private:
  std::string text_;
};
}  // namespace mdf::detail
