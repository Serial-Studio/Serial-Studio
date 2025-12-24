/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include "mdf/ifilehistory.h"
#include "mdf4timestamp.h"
#include "mdfblock.h"

namespace mdf::detail {
class Fh4Block : public MdfBlock, public IFileHistory {
 public:
  Fh4Block();
  [[nodiscard]] int64_t Index() const override;
  [[nodiscard]] std::string BlockType() const override {
    return MdfBlock::BlockType();
  }
  void Time(uint64_t ns_since_1970) override;
  void Time(ITimestamp& timestamp) override;
  
  [[nodiscard]] uint64_t Time() const override;
  [[nodiscard]] const mdf::IMdfTimestamp * StartTimestamp() const override;

  void GetBlockProperty(BlockPropertyList& dest) const override;
  [[nodiscard]] IMetaData* CreateMetaData() override;
  [[nodiscard]] IMetaData* MetaData() const override;
  uint64_t Read(std::streambuf& buffer) override;
  uint64_t Write(std::streambuf& buffer) override;

 private:
  Mdf4Timestamp timestamp_;
};
}  // namespace mdf::detail
