/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <memory>
#include <string>
#include <vector>
#include <functional>

#include "cg4block.h"
#include "hl4block.h"
#include "dt4block.h"
#include "datalistblock.h"
#include "mdf/idatagroup.h"
#include "dgrange.h"

namespace mdf::detail {
class Dg4Block : public DataListBlock, public IDataGroup {
 public:
  using Cg4List = std::vector<std::unique_ptr<Cg4Block>>;
  Dg4Block();

  [[nodiscard]] int64_t Index() const override;
  [[nodiscard]] std::string BlockType() const override {
    return MdfBlock::BlockType();
  }
  void Description(const std::string& desc) override;
  [[nodiscard]] std::string Description() const override;

  [[nodiscard]] std::vector<IChannelGroup*> ChannelGroups() const override;
  [[nodiscard]] IChannelGroup* CreateChannelGroup() override;

  void AddCg4(std::unique_ptr<Cg4Block>& cg4);
  [[nodiscard]] const Cg4List& Cg4() const { return cg_list_; }

  void GetBlockProperty(BlockPropertyList& dest) const override;
  MdfBlock* Find(int64_t index) const override;

  uint64_t Read(std::streambuf& buffer) override;
  void ReadCgList(std::streambuf& buffer);

  void ReadData(std::streambuf& buffer);
  void ReadRangeData(std::streambuf& buffer, DgRange& range);
  void ReadVlsdData(std::streambuf& buffer,Cn4Block& channel,
                    const std::vector<uint64_t>& offset_list,
                    std::function<void(uint64_t, const std::vector<uint8_t>&)>& callback);

  void ClearData() override;

  IMetaData* CreateMetaData() override;
  IMetaData* MetaData() const override;
  void RecordIdSize(uint8_t id_size) override;
  uint8_t RecordIdSize() const override;
  uint64_t Write(std::streambuf& buffer) override;
  uint64_t DataSize() const override;

  Hl4Block* CreateOrGetHl4();
  Dt4Block* CreateOrGetDt4(std::streambuf& buffer);
  bool FinalizeDtBlocks(std::streambuf& buffer);
  bool FinalizeCgAndVlsdBlocks(std::streambuf& buffer, bool update_cg, bool update_vlsd);

  [[nodiscard]] IChannelGroup *FindParentChannelGroup(
      const IChannel &channel) const override;
  [[nodiscard]] Cg4Block* FindCgRecordId(uint64_t record_id) const;
 private:
  uint8_t rec_id_size_ = 0;
  /* 7 byte reserved */
  Cg4List cg_list_;

  void ParseDataRecords(std::streambuf& buffer, uint64_t nof_data_bytes);
  uint64_t ReadRecordId(std::streambuf& buffer, uint64_t& record_id) const;

};

}  // namespace mdf::detail
