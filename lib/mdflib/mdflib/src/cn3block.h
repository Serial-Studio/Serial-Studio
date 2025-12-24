/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <memory>
#include <string>

#include "cc3block.h"
#include "cd3block.h"
#include "ce3block.h"
#include "datalistblock.h"
#include "mdf/ichannel.h"
#include "tx3block.h"

namespace mdf::detail {
class Cg3Block;

class Cn3Block : public DataListBlock, public IChannel {
 public:
  [[nodiscard]] int64_t Index() const override;
  [[nodiscard]] std::string BlockType() const override {
    return MdfBlock::BlockType();
  }
  void Name(const std::string& name) override;
  [[nodiscard]] std::string Name() const override;

  void DisplayName(const std::string& name) override;
  [[nodiscard]] std::string DisplayName() const override;

  void Description(const std::string& description) override;
  [[nodiscard]] std::string Description() const override;

  void Unit(const std::string& unit) override;
  [[nodiscard]] std::string Unit() const override;
  [[nodiscard]] bool IsUnitValid() const override;

  void Type(ChannelType type) override;
  [[nodiscard]] ChannelType Type() const override;

  void DataType(ChannelDataType type) override;
  [[nodiscard]] ChannelDataType DataType() const override;

  void DataBytes(uint64_t nof_bytes) override;
  [[nodiscard]] uint64_t DataBytes() const override;

  [[nodiscard]] IChannelConversion* ChannelConversion() const override;
  IChannelConversion *CreateChannelConversion() override;

  void SamplingRate(double sampling_rate) override;
  [[nodiscard]] double SamplingRate() const override;

  [[nodiscard]] uint8_t Decimals() const override;
  [[nodiscard]] bool IsDecimalUsed() const override;

  [[nodiscard]] std::string Comment() const override;
  [[nodiscard]] MdfBlock* Find(int64_t index) const override;
  void GetBlockProperty(BlockPropertyList& dest) const override;
  void Init(const MdfBlock& id_block) override;
  uint64_t Read(std::streambuf& buffer) override;
  uint64_t Write(std::streambuf& buffer) override;

  void AddCc3(std::unique_ptr<Cc3Block>& cc3);

  [[nodiscard]] Cc3Block* Cc3() const { return cc_block_.get(); }

  [[nodiscard]] Cd3Block* Cd3() const { return cd_block_.get(); }

  [[nodiscard]] Ce3Block* Ce3() const { return ce_block_.get(); }

  void BitCount(uint32_t bits) override;
  [[nodiscard]] uint32_t BitCount() const override;

  void BitOffset(uint16_t bits) override;
  [[nodiscard]] uint16_t BitOffset() const override;

  void ByteOffset(uint32_t bytes) override;
  [[nodiscard]] uint32_t ByteOffset() const override;
  
  IChannel* CreateChannelComposition() override;
  std::vector<IChannel*> ChannelCompositions() override;

  const IChannelGroup* ChannelGroup() const override;
 protected:

  [[nodiscard]] std::vector<uint8_t>& SampleBuffer() const override;

 private:
  uint16_t channel_type_ = 0;
  std::string short_name_;
  std::string description_;
  uint16_t start_offset_ = 0;
  uint16_t nof_bits_ = 0;
  uint16_t signal_type_ = 0;
  bool range_valid_ = false;
  double min_ = 0.0;
  double max_ = 0.0;
  double sample_rate_ = 0.0;
  uint16_t byte_offset_ = 0;

  std::string comment_;
  std::string long_name_;
  std::string display_name_;

  std::unique_ptr<Cc3Block> cc_block_;
  std::unique_ptr<Ce3Block> ce_block_;
  std::unique_ptr<Cd3Block> cd_block_;

  const Cg3Block* cg3_block = nullptr;
};
}  // namespace mdf::detail
