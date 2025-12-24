/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <string>
#include <vector>

#include "mdfblock.h"
#include "tx3block.h"

namespace mdf::detail {
struct Tr3Event {
  double event_time = 0;
  double pre_time = 0;
  double post_time = 0;
};

class Tr3Block : public MdfBlock {
 public:
  [[nodiscard]] std::string Comment() const override;
  uint64_t Read(std::streambuf& buffer) override;
  uint64_t Write(std::streambuf& buffer) override;

 private:
  uint16_t nof_events_ = 0;
  std::vector<Tr3Event> event_list_;
  std::string comment_;
};
}  // namespace mdf::detail
