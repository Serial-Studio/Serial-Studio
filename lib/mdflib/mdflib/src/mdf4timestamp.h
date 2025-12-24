/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <cstdio>

#include "mdf/imdftimestamp.h"
#include "mdf/itimestamp.h"
#include "mdf/mdfhelper.h"
#include "mdfblock.h"

namespace mdf::detail {

namespace TimestampFlag {
constexpr uint8_t kUtcTimestamp = 0x00;
constexpr uint8_t kLocalTimestamp = 0x01;
constexpr uint8_t kTimeOffsetValid = 0x02;
}  // namespace TimestampFlag

class Mdf4Timestamp : public IMdfTimestamp, public detail::MdfBlock {
 public:
  Mdf4Timestamp();
  void GetBlockProperty(detail::BlockPropertyList &dest) const override;
  uint64_t Read(std::streambuf& buffer) override;
  uint64_t Write(std::streambuf& buffer) override;

  void SetTime(uint64_t time) override;
  void SetTime(ITimestamp &timestamp) override;

  [[nodiscard]] std::string GetTimeString() const;
  [[nodiscard]] uint64_t GetTimeNs() const override;
  [[nodiscard]] uint16_t GetTzOffsetMin() const override;
  [[nodiscard]] uint16_t GetDstOffsetMin() const override;
  [[nodiscard]] timetype::MdfTimestampType GetTimeType() const override;

 private:
  uint64_t time_;
  int16_t tz_offset_;
  int16_t dst_offset_;
  uint8_t flags_;
};

}  // Namespace mdf::detail
