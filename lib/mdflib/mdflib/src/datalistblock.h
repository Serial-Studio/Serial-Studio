/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <cstdio>
#include <memory>
#include <vector>

#include "mdfblock.h"

namespace mdf::detail {
class DataBlock;
class DataListBlock : public MdfBlock {
 public:
  using BlockList = std::vector<std::unique_ptr<MdfBlock>>;
  BlockList& DataBlockList() { return block_list_; }
  [[nodiscard]] const BlockList& DataBlockList() const { return block_list_; }
  [[nodiscard]] virtual uint64_t DataSize() const;
  [[nodiscard]] MdfBlock* Find(int64_t index) const override;
  void ReadBlockList(std::streambuf& buffer, size_t data_index);
  void WriteBlockList(std::streambuf& buffer, size_t data_index);

  void ReadLinkList(std::streambuf& buffer, size_t data_index, uint32_t nof_link);
  virtual void ClearData();
  void CopyDataToBuffer(std::streambuf& buffer, std::vector<uint8_t> &dest,
                                                   uint64_t &buffer_index) const;
  [[nodiscard]] bool IsRdBlock() const;
   void GetDataBlockList(std::vector<DataBlock*>& block_list) const;
 protected:
  BlockList block_list_;
};

}  // namespace mdf::detail
