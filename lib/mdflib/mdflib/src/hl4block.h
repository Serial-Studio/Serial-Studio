/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include "datalistblock.h"
namespace mdf::detail {

namespace Hl4Flags {

constexpr uint16_t EqualLength = 0x01;
constexpr uint16_t TimeValues = 0x02;
constexpr uint16_t AngleValues = 0x04;
constexpr uint16_t DistanceValues = 0x08;

}

enum class Hl4ZipType : uint8_t {
  Deflate = 0,
  TransposeAndDeflate = 1
};

class Hl4Block : public DataListBlock {
 public:
  void Flags(uint16_t flags) { flags_ = flags; }
  [[nodiscard]] uint16_t Flags() const { return flags_; }

  void Type(Hl4ZipType type) { type_ = static_cast<uint8_t>(type); }
  [[nodiscard]] Hl4ZipType Type() const {
    return static_cast<Hl4ZipType>(type_);
  }

  void GetBlockProperty(BlockPropertyList& dest) const override;
  uint64_t Read(std::streambuf& buffer) override;
  uint64_t Write(std::streambuf& buffer) override;
 private:
  uint16_t flags_ = 0;
  uint8_t type_ = 0; // ZIP types
  /* 5 byte reserved */
  // The block_list_ in the DataListBlock is a list of DL blocks, so
  // it can be treated as a normal link list
};

}  // namespace mdf::detail
