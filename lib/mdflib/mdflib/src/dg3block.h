/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <memory>
#include <string>
#include <vector>

#include "cg3block.h"
#include "datalistblock.h"
#include "dt3block.h"
#include "mdf/idatagroup.h"
#include "tr3block.h"
#include "dgrange.h"

namespace mdf::detail {

class Dg3Block : public DataListBlock, public IDataGroup {
 public:
  using Cg3List = std::vector<std::unique_ptr<Cg3Block>>;

  [[nodiscard]] int64_t Index() const override;
  [[nodiscard]] std::string BlockType() const override {
    return MdfBlock::BlockType();
  }
  [[nodiscard]] std::vector<IChannelGroup*> ChannelGroups() const override;
  [[nodiscard]] IChannelGroup* CreateChannelGroup() override;

  void AddCg3(std::unique_ptr<Cg3Block>& cg3);
  [[nodiscard]] const Cg3List& Cg3() const;
  [[nodiscard]] const Tr3Block* Tr3() const { return tr_block_.get(); }
  uint16_t NofRecordId() const { return nof_record_id_; }
  void GetBlockProperty(BlockPropertyList& dest) const override;
  [[nodiscard]] MdfBlock* Find(int64_t index) const override;
  uint64_t Read(std::streambuf& buffer) override;
  uint64_t Write(std::streambuf& buffer) override;

  void ReadData(std::streambuf& buffer);
  void ReadRangeData(std::streambuf& buffer, DgRange& range);
  void ClearData() override;


  [[nodiscard]] IChannelGroup *FindParentChannelGroup(
      const IChannel &channel) const override;
 private:
  uint16_t nof_cg_blocks_ = 0;
  uint16_t nof_record_id_ = 0;

  std::unique_ptr<Tr3Block> tr_block_;
  Cg3List cg_list_;
  void ParseDataRecords(std::streambuf& buffer, uint64_t nof_data_bytes);
  void ParseRangeDataRecords(std::streambuf& buffer, uint64_t nof_data_bytes,
                             DgRange& range);
  const Cg3Block* FindCgRecordId(const uint64_t record_id) const;
};

}  // namespace mdf::detail
