/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <memory>

#include "hd3block.h"
#include "idblock.h"
#include "mdf/idatagroup.h"
#include "mdf/mdffile.h"


namespace mdf::detail {

class Mdf3File : public MdfFile {
 public:
  Mdf3File();
  explicit Mdf3File(std::unique_ptr<IdBlock> id_block);
  ~Mdf3File() override = default;

  void Attachments(AttachmentList &dest) const override;
  void DataGroups(DataGroupList &dest) const override;
  [[nodiscard]] std::string Version() const override;
  void MinorVersion(int minor) override;

  void ProgramId(const std::string &program_id) override;
  [[nodiscard]] std::string ProgramId() const override;

  [[nodiscard]] IHeader *Header() const override;

  [[nodiscard]] IDataGroup *CreateDataGroup() override;

  [[nodiscard]] MdfBlock *Find(int64_t id) const;


  [[nodiscard]] bool IsMdf4() const override;
  void IsFinalized(bool finalized, std::streambuf& buffer, uint16_t standard_flags,
                   uint16_t custom_flags) override;
  [[nodiscard]] bool IsFinalized(uint16_t &standard_flags,
                                 uint16_t &custom_flags) const override;

  void ReadHeader(std::streambuf& buffer) override;
  void ReadMeasurementInfo(std::streambuf& buffer) override;
  void ReadEverythingButData(std::streambuf& buffer) override;

  [[nodiscard]] const IdBlock &Id() const;
  [[nodiscard]] const Hd3Block &Hd() const;

  bool Write(std::streambuf& buffer) override;

  Mdf3File(const Mdf3File &) = delete;
  Mdf3File(Mdf3File &&) = delete;
  Mdf3File &operator=(const Mdf3File &) = delete;
  Mdf3File &operator=(Mdf3File &&) = delete;

  [[nodiscard]] IDataGroup* FindParentDataGroup(
      const IChannel& channel) const  override;

 private:
  std::unique_ptr<IdBlock> id_block_;
  std::unique_ptr<Hd3Block> hd_block_;
};

}  // namespace mdf::detail
