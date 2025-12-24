/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <cstdio>
#include <string>
#include <vector>

#include "mdf/ichannelconversion.h"
#include "mdfblock.h"

namespace mdf::detail {



class Cc3Block : public MdfBlock, public IChannelConversion {
 public:
  [[nodiscard]] int64_t Index() const override;
  [[nodiscard]] std::string BlockType() const override {
    return MdfBlock::BlockType();
  }
  void Unit(const std::string& unit) override;
  [[nodiscard]] std::string Unit() const override;

  void Type(ConversionType type) override;
  [[nodiscard]] ConversionType Type() const override;

  [[nodiscard]] bool IsUnitValid() const override;
  [[nodiscard]] bool IsDecimalUsed() const override;

  [[nodiscard]] uint8_t Decimals() const override;

  void GetBlockProperty(BlockPropertyList& dest) const override;
  uint64_t Read(std::streambuf& buffer) override;
  uint64_t Write(std::streambuf& buffer) override;

 protected:
  bool ConvertValueToText(double channel_value,
                          std::string& eng_value) const override;
  bool ConvertValueRangeToText(double channel_value,
                               std::string& eng_value) const override;

 private:
  bool range_valid_ = false;
  double min_ = 0.0;
  double max_ = 0.0;
  std::string unit_;
  uint16_t conversion_type_ = 0xFFFF;


};
}  // namespace mdf::detail
