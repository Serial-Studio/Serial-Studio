/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <string>
#include <vector>
#include <map>

#include "cc4block.h"
#include "datalistblock.h"
#include "md4block.h"
#include "mdf/iattachment.h"
#include "mdf/ichannel.h"
#include "mdf/ichannelhierarchy.h"
#include "si4block.h"
#include "vlsddata.h"

namespace mdf::detail {
class Cg4Block;

class Cn4Block : public DataListBlock, public IChannel {
 public:
  using Cx4List = std::vector<std::unique_ptr<MdfBlock>>;

  Cn4Block();
  [[nodiscard]] int64_t Index() const override;
  [[nodiscard]] std::string BlockType() const override {
    return MdfBlock::BlockType();
  }

  void Name(const std::string& name) override;
  [[nodiscard]] std::string Name() const override;

  void DisplayName(const std::string& name) override;
  std::string DisplayName() const override;

  void Description(const std::string& description) override;
  std::string Description() const override;

  void Unit(const std::string& unit) override;
  [[nodiscard]] std::string Unit() const override;
  [[nodiscard]] bool IsUnitValid() const override;

  void SetCnUnit(const CnUnit& unit) override;
  void GetCnUnit(CnUnit& unit) const override;

  void Flags(uint32_t flags) override;
  uint32_t Flags() const override;

  [[nodiscard]] IChannelConversion* ChannelConversion() const override;
  [[nodiscard]] IChannelConversion* CreateChannelConversion() override;

  void Type(ChannelType type) override;
  [[nodiscard]] ChannelType Type() const override;

  void DataType(ChannelDataType type) override;
  [[nodiscard]] ChannelDataType DataType() const override;

  void DataBytes(uint64_t nof_bytes) override;
  [[nodiscard]] uint64_t DataBytes() const override;

  void SamplingRate(double sampling_rate) override;
  double SamplingRate() const override;

  void Decimals(uint8_t precision) override;
  [[nodiscard]] uint8_t Decimals() const override;

  [[nodiscard]] bool IsDecimalUsed() const override;

  void GetBlockProperty(BlockPropertyList& dest) const override;
  [[nodiscard]] MdfBlock* Find(int64_t index) const override;
  uint64_t Read(std::streambuf& buffer) override;
  uint64_t Write(std::streambuf& buffer) override;
  void ReadSignalData(std::streambuf& buffer) const;  ///< Reads in (VLSD) channel data (SD)
  uint64_t WriteSignalData(std::streambuf& buffer, bool compress);

  void Init(const MdfBlock& id_block) override;

  [[nodiscard]] const Cx4List& Cx4() const { return cx_list_; }

  [[nodiscard]] const Si4Block* Si() const { return si_block_.get(); }
  void AddCc4(std::unique_ptr<Cc4Block>& cc4);
  [[nodiscard]] const Cc4Block* Cc() const { return cc_block_.get(); }


  void ClearData() const {
    data_list_.clear();
    data_list_.shrink_to_fit();
  }
  const std::vector<uint8_t>& DataList() const { return data_list_; }

  void UpdateDataLink(std::streambuf& buffer, int64_t position);
  [[nodiscard]] int64_t DataLink() const;

  [[nodiscard]] std::vector<int64_t> AtLinkList() const;
  [[nodiscard]] std::vector<int64_t> XAxisLinkList() const;

  void Sync(ChannelSyncType type) override;
  ChannelSyncType Sync() const override;
  void Range(double min, double max) override;
  std::optional<std::pair<double, double>> Range() const override;
  void Limit(double min, double max) override;
  std::optional<std::pair<double, double>> Limit() const override;
  void ExtLimit(double min, double max) override;
  std::optional<std::pair<double, double>> ExtLimit() const override;

  IMetaData *CreateMetaData() override {
    return MdfBlock::CreateMetaData();
  }
  [[nodiscard]] IMetaData *MetaData() const override {
    return MdfBlock::MetaData();
  }

  [[nodiscard]] ISourceInformation *SourceInformation() const override;
  [[nodiscard]] ISourceInformation* CreateSourceInformation() override;

  void PrepareForWriting(uint64_t offset);
  void SetInvalidOffset(uint64_t bit_offset) {
    invalid_bit_pos_ = static_cast<uint32_t>(bit_offset);
  }

  void SetValid(bool valid, uint64_t array_index) override;
  bool GetValid(const std::vector<uint8_t> &record_buffer,
                uint64_t array_index) const override;
  void ClearData() override;

  void BitCount(uint32_t bits) override;
  [[nodiscard]] uint32_t BitCount() const override;
  void BitOffset(uint16_t bits) override;
  [[nodiscard]] uint16_t BitOffset() const override;
  void ByteOffset(uint32_t bytes) override;
  [[nodiscard]] uint32_t ByteOffset() const override;
  IChannel* CreateChannelComposition() override;
  std::vector<IChannel*> ChannelCompositions() override;

  void MlsdChannel(const Cn4Block* channel) const {
    mlsd_channel_ = channel;
  }

  uint64_t WriteSdSample(const std::vector<uint8_t>& buffer) const;
  [[nodiscard]] IChannelArray *ChannelArray(size_t index) const override;
  [[nodiscard]] std::vector<IChannelArray*> ChannelArrays() const override;
  [[nodiscard]] IChannelArray *CreateChannelArray() override;

  const IChannelGroup* ChannelGroup() const override;
  void AddAttachmentReference(const IAttachment* attachment) override;
  std::vector<const IAttachment*> AttachmentList() const override;

 protected:

  bool GetTextValue(const std::vector<uint8_t>& record_buffer,
                    std::string& dest) const override;
  bool GetByteArrayValue(const std::vector<uint8_t>& record_buffer,
                    std::vector<uint8_t>& dest) const override;
  void SetTextValue(const std::string &value, bool valid) override;
  void SetByteArray(const std::vector<uint8_t> &value, bool valid) override;

  std::vector<uint8_t>& SampleBuffer() const override;

 private:
  uint8_t type_ = 0;
  uint8_t sync_type_ = 0; ///< Normal channel type
  uint8_t data_type_ = 0;
  uint8_t bit_offset_ = 0;
  uint32_t byte_offset_ = 0;
  uint32_t bit_count_ = 0;
  uint32_t flags_ = 0;
  uint32_t invalid_bit_pos_ = 0;
  uint8_t precision_ = 0;
  /* 1 byte reserved */
  uint16_t nof_attachments_ = 0;
  double range_min_ = 0;
  double range_max_ = 0;
  double limit_min_ = 0;
  double limit_max_ = 0;
  double limit_ext_min_ = 0;
  double limit_ext_max_ = 0;

  std::string name_;
  std::unique_ptr<Si4Block> si_block_;
  std::unique_ptr<Cc4Block> cc_block_;
  std::unique_ptr<Md4Block> unit_;

  // The block_list_ points to signal data either a SD/DL/DZ block but can also
  // be a reference to an VLSD CG block

  Cx4List cx_list_;
  std::vector<const IAttachment*> attachment_list_;
  ElementLink default_x_;

  // The data_list_ is a temporary buffer that holds
  // uncompressed signal data
  mutable std::vector<uint8_t> data_list_; ///< Typical SD block data
  mutable std::map<VlsdData, uint64_t> data_map_; ///< Data->index map

  const Cg4Block* cg_block_ = nullptr; ///< Pointer to its CG block
  mutable const Cn4Block* mlsd_channel_ = nullptr; ///< Pointer to length channel
};

}  // namespace mdf::detail
