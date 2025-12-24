/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <cstdio>
#include <string>
#include <vector>

#include "mdfblock.h"

namespace mdf::detail {
class Ce3Block : public MdfBlock {
 public:
  uint64_t Read(std::streambuf& buffer) override;

 private:
  uint16_t type_ = 0;

  // DIM block values
  uint16_t nof_module_ = 0;
  uint32_t address_ = 0;
  std::string description_;
  std::string ecu_;

  // CAN block values
  uint32_t message_id_ = 0;
  uint32_t index_ = 0;
  std::string message_;
  std::string sender_;
};
}  // namespace mdf::detail
