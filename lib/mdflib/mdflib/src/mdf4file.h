/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <cstdio>
#include <memory>

#include "hd4block.h"
#include "idblock.h"
#include "mdf/mdffile.h"

namespace mdf::detail {

class Mdf4File : public MdfFile {
 public:
  Mdf4File();
  explicit Mdf4File(std::unique_ptr<IdBlock> id_block);
  ~Mdf4File() override = default;

  Mdf4File(const Mdf4File&) = delete;
  Mdf4File(Mdf4File&&) = delete;
  Mdf4File& operator=(const Mdf4File&) = delete;
  Mdf4File& operator=(Mdf4File&&) = delete;

  void Attachments(AttachmentList& dest) const override;
  void DataGroups(DataGroupList& dest) const override;
  [[nodiscard]] std::string Version() const override;
  void MinorVersion(int minor) override;

  void ProgramId(const std::string& program_id) override;
  [[nodiscard]] std::string ProgramId() const override;

  [[nodiscard]] IHeader* Header() const override;

  [[nodiscard]] IAttachment* CreateAttachment() override;

  [[nodiscard]] IDataGroup* CreateDataGroup() override;

  [[nodiscard]] MdfBlock* Find(int64_t id) const;

  [[nodiscard]] bool IsMdf4() const override;

  void IsFinalized(bool finalized, std::streambuf& buffer, uint16_t standard_flags,
                   uint16_t custom_flags) override;
  [[nodiscard]] bool IsFinalized(uint16_t& standard_flags,
                                 uint16_t& custom_flags) const override;
  [[nodiscard]] bool IsFinalizedDone() const override;

  void ReadHeader(std::streambuf& buffer) override;
  void ReadMeasurementInfo(std::streambuf& buffer) override;
  void ReadEverythingButData(std::streambuf& buffer) override;

  [[nodiscard]] const IdBlock& Id() const;
  [[nodiscard]] const Hd4Block& Hd() const;

  bool Write(std::streambuf& buffer) override;

  [[nodiscard]] IDataGroup* FindParentDataGroup(
      const IChannel& channel) const  override;

 private:
  std::unique_ptr<IdBlock> id_block_;
  std::unique_ptr<Hd4Block> hd_block_;

  bool finalized_done_ = false;
  bool FinalizeFile(std::streambuf& buffer);
  void FindAllReferences(std::streambuf& buffer);
};
}  // namespace mdf::detail
