/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <limits>
#include <string>

#include <vector>

#include "md4block.h"
#include "mdf/ichannelconversion.h"
#include "mdfblock.h"

namespace mdf::detail {

class Cc4Block : public MdfBlock, public IChannelConversion {
 public:
  using RefList = std::vector<std::unique_ptr<MdfBlock>>;


  Cc4Block();

  [[nodiscard]] int64_t Index() const override;
  [[nodiscard]] std::string BlockType() const override {
    return MdfBlock::BlockType();
  }
  void Name(const std::string& name) override;

  [[nodiscard]] std::string Name() const override;

  void Unit(const std::string& unit) override;
  [[nodiscard]] std::string Unit() const override;
  void SetCcUnit(const CcUnit& unit) override;
  void GetCcUnit(CcUnit& unit) const override;

  void Description(const std::string& desc) override;
  [[nodiscard]] std::string Description() const override;

  void Type(ConversionType type) override;
  [[nodiscard]] ConversionType Type() const override;

  void Decimals(uint8_t decimals) override;
  [[nodiscard]] uint8_t Decimals() const override;

  [[nodiscard]] bool IsUnitValid() const override;
  [[nodiscard]] bool IsDecimalUsed() const override;

  [[nodiscard]] IChannelConversion* CreateInverse() override;
  [[nodiscard]] IChannelConversion* Inverse() const override;

  void Range(double min, double max) override;
  [[nodiscard]] std::optional<std::pair<double, double>> Range() const override;

  void Flags(uint16_t flags) override;
  [[nodiscard]] uint16_t Flags() const override;

  [[nodiscard]] IMetaData* CreateMetaData() override {
    return MdfBlock::CreateMetaData();
  }
  [[nodiscard]] IMetaData* MetaData() const override {
    return MdfBlock::MetaData();
  }

  [[nodiscard]] const Cc4Block* Cc() const { return cc_block_.get(); }

  [[nodiscard]] uint16_t NofReferences() const override;
  void Reference(uint16_t index, const std::string& text) override;
  [[nodiscard]] std::string Reference(uint16_t index) const override;
  
  [[nodiscard]] const RefList& References() const { return ref_list_; }

  [[nodiscard]] MdfBlock* Find(int64_t index) const override;
  void GetBlockProperty(BlockPropertyList& dest) const override;

  uint64_t Read(std::streambuf& buffer) override;
  uint64_t Write(std::streambuf& buffer) override;

  void Formula( const std::string& formula) override;
  [[nodiscard]] const std::string& Formula() const override;


 protected:
  bool ConvertValueToText(double channel_value,
                          std::string& eng_value) const override;
  bool ConvertValueRangeToText(double channel_value,
                               std::string& eng_value) const override;
  bool ConvertTextToValue(const std::string& channel_value,
                          double& eng_value) const override;
  bool ConvertTextToTranslation(const std::string& channel_value,
                                std::string& eng_value) const override;

 private:
  uint8_t type_ = 0;
  uint8_t precision_ = 0;
  uint16_t flags_ = 0;
  uint16_t nof_references_ = 0;

  double range_min_ = 0;
  double range_max_ = 0;

  std::string name_;
  std::unique_ptr<Cc4Block> cc_block_;  ///< Inverse conversion block
  std::unique_ptr<Md4Block> unit_;
  RefList ref_list_;
};

}  // namespace mdf::detail
