/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include "mdf/isourceinformation.h"
#include "mdfblock.h"
namespace mdf::detail {

class Si4Block : public MdfBlock, public ISourceInformation {
 public:
  Si4Block();

  [[nodiscard]] int64_t Index() const override;
  [[nodiscard]] std::string BlockType() const override {
    return MdfBlock::BlockType();
  }
  void Name(const std::string &name) override;
  [[nodiscard]] const std::string &Name() const override;

  void Path(const std::string &path) override;
  [[nodiscard]] const std::string &Path() const override;

  void Description(const std::string &desc) override;
  [[nodiscard]] std::string Description() const override;

  void Type(SourceType type) override;
  [[nodiscard]] SourceType Type() const override;

  void Bus(BusType type) override;
  [[nodiscard]] BusType Bus() const override;

  void Flags(uint8_t flags) override;
  [[nodiscard]] uint8_t Flags() const override;

  IMetaData *CreateMetaData() override;
  [[nodiscard]] IMetaData *MetaData() const override;

  void GetBlockProperty(BlockPropertyList &dest) const override;
  uint64_t Read(std::streambuf& buffer) override;
  uint64_t Write(std::streambuf& buffer) override;

 private:
  uint8_t type_ = 0;
  uint8_t bus_type_ = 0;
  uint8_t flags_ = 0;
  /* 5 byte reserved */
  std::string name_;
  std::string path_;
};
}  // namespace mdf::detail
