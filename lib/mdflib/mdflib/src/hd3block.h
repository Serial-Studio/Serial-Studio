/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <memory>
#include <string>
#include <vector>

#include "dg3block.h"
#include "mdf/iheader.h"
#include "mdf/itimestamp.h"
#include "mdf3timestamp.h"
#include "mdfblock.h"
#include "pr3block.h"

namespace mdf::detail {

class Hd3Block : public MdfBlock, public IHeader {
 public:
  using Dg3List = std::vector<std::unique_ptr<Dg3Block>>;
  Hd3Block();
  ~Hd3Block() override = default;

  [[nodiscard]] int64_t Index() const override;
  [[nodiscard]] std::string BlockType() const override {
    return MdfBlock::BlockType();
  }
  void Author(const std::string &author) override;
  [[nodiscard]] std::string Author() const override;

  void Department(const std::string &department) override;
  [[nodiscard]] std::string Department() const override;

  void Project(const std::string &name) override;
  [[nodiscard]] std::string Project() const override;

  void Subject(const std::string &subject) override;
  [[nodiscard]] std::string Subject() const override;

  void Description(const std::string &description) override;
  [[nodiscard]] std::string Description() const override;

  void StartTime(uint64_t ns_since_1970) override;
  void StartTime(ITimestamp& timestamp) override;
  
  [[nodiscard]] uint64_t StartTime() const override;
  [[nodiscard]] const mdf::IMdfTimestamp * StartTimestamp() const override;

  [[nodiscard]] IDataGroup *CreateDataGroup() override;
  [[nodiscard]] std::vector<IDataGroup *> DataGroups() const override;
  [[nodiscard]] IDataGroup *LastDataGroup() const override;

  void AddDg3(std::unique_ptr<Dg3Block> &dg3);
  [[nodiscard]] const Dg3List &Dg3() const;

  [[nodiscard]] std::string Comment() const override;
  [[nodiscard]] MdfBlock *Find(int64_t index) const override;
  void GetBlockProperty(BlockPropertyList &dest) const override;
  uint64_t Read(std::streambuf& buffer) override;
  uint64_t Write(std::streambuf& buffer) override;

  void ReadMeasurementInfo(std::streambuf& buffer);
  void ReadEverythingButData(std::streambuf& buffer);

 private:
  uint16_t nof_dg_blocks_ = 0;
  Mdf3Timestamp timestamp_;
  std::string author_;
  std::string organisation_;
  std::string project_;
  std::string subject_;

  std::unique_ptr<Pr3Block> pr_block_;
  Dg3List dg_list_;
  std::string comment_;
};
}  // namespace mdf::detail
