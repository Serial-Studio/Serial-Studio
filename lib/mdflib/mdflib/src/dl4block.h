/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include "datalistblock.h"

namespace mdf::detail {
namespace Dl4Flags {
constexpr uint8_t EqualLength = 0x01;
constexpr uint8_t TimeValues = 0x02;
constexpr uint8_t AngleValues = 0x04;
constexpr uint8_t DistanceValues = 0x08;
}

class Dl4Block : public DataListBlock {
 public:
  void GetBlockProperty(BlockPropertyList& dest) const override;
  uint64_t Read(std::streambuf& buffer) override;
  uint64_t Write(std::streambuf& buffer) override;

  void Flags(uint8_t flags) {flags_ = flags;}
  [[nodiscard]] uint8_t Flags() const {return flags_;}

  void EqualLength(uint64_t length);
  [[nodiscard]] uint64_t EqualLength() const;

  void Offset(size_t index, uint64_t offset);
  [[nodiscard]] uint64_t Offset(size_t index) const;

  void TimeValue(size_t index, uint64_t value);
  [[nodiscard]] uint64_t TimeValue(size_t index) const;

  void AngleValue(size_t index, uint64_t value);
  [[nodiscard]] uint64_t AngleValue(size_t index) const;

  void DistanceValue(size_t index, uint64_t value);
  [[nodiscard]] uint64_t DistanceValue(size_t index) const;

 private:
  uint8_t flags_ = 0;
  /* 3 byte reserved */
  uint32_t nof_blocks_ = 0;
  uint64_t equal_length_ = 0;

  std::vector<uint64_t> offset_list_;
  std::vector<uint64_t> time_list_;
  std::vector<uint64_t> angle_list_;
  std::vector<uint64_t> distance_list_;
};



}  // namespace mdf::detail
