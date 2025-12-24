/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "mdf4file.h"
#include <algorithm>
#include "mdf/mdflogstream.h"
#include "cn4block.h"
#include "sr4block.h"
#include "ca4block.h"

namespace mdf::detail {

Mdf4File::Mdf4File()
    : id_block_(std::make_unique<IdBlock>()),
      hd_block_(std::make_unique<Hd4Block>()) {
  hd_block_->Init(*id_block_);
}

Mdf4File::Mdf4File(std::unique_ptr<IdBlock> id_block)
    : id_block_(std::move(id_block)) {}

IHeader *Mdf4File::Header() const { return hd_block_.get(); }

void Mdf4File::ReadHeader(std::streambuf& buffer) {
  if (!id_block_) {
    id_block_ = std::make_unique<IdBlock>();
    SetFilePosition(buffer, 0);
    id_block_->Read(buffer);
  }
  if (!hd_block_) {
    hd_block_ = std::make_unique<Hd4Block>();
    hd_block_->Init(*id_block_);
    SetFilePosition(buffer, 64);
    hd_block_->Read(buffer);
  }


}

void Mdf4File::ReadMeasurementInfo(std::streambuf& buffer) {
  ReadHeader(buffer);
  if (hd_block_) {
    hd_block_->ReadMeasurementInfo(buffer);
  }
}

void Mdf4File::ReadEverythingButData(std::streambuf& buffer) {
  ReadHeader(buffer);
  if (hd_block_) {
    hd_block_->ReadMeasurementInfo(buffer);
    hd_block_->ReadEverythingButData(buffer);
  }
  // Now when all blocks are read in, it is time to read
  // in all referenced blocks.
  // This is mainly done for the channel array blocks.
  FindAllReferences(buffer);

  // Check if the file needs to be finalized
  uint16_t standard_flags = 0;
  uint16_t custom_flags = 0;
  if (!finalized_done_ && id_block_ &&
      !id_block_->IsFinalized(standard_flags, custom_flags)) {
    // Try to finalize the last DG block.
    finalized_done_ = FinalizeFile(buffer);
  }
}

bool Mdf4File::IsMdf4() const { return true; }

const IdBlock &Mdf4File::Id() const {
  if (!id_block_) {
    throw std::domain_error("ID block not initialized yet");
  }
  return *id_block_;
}

const Hd4Block &Mdf4File::Hd() const {
  if (!hd_block_) {
    throw std::domain_error("HD4 block not initialized yet");
  }
  return *hd_block_;
}

MdfBlock *Mdf4File::Find(int64_t id) const {
  if (id_block_) {
    auto *p = id_block_->Find(id);
    if (p != nullptr) {
      return p;
    }
  }
  if (hd_block_) {
    auto *p = hd_block_->Find(id);
    if (p != nullptr) {
      return p;
    }
  }
  return nullptr;
}

void Mdf4File::Attachments(AttachmentList &dest) const {
  if (!hd_block_) {
    return;
  }
  const auto &at4_list = hd_block_->At4();
  for (const auto &at4 : at4_list) {
    dest.emplace_back(at4.get());
  }
}
void Mdf4File::DataGroups(DataGroupList &dest) const {
  if (!hd_block_) {
    return;
  }
  const auto &dg4_list = hd_block_->Dg4();
  for (const auto &dg4 : dg4_list) {
    dest.emplace_back(dg4.get());
  }
}

std::string Mdf4File::Version() const {
  return !id_block_ ? "" : id_block_->VersionString();
}

void Mdf4File::MinorVersion(int minor) {
  if (id_block_) {
    id_block_->MinorVersion(minor);
  }
}

void Mdf4File::ProgramId(const std::string &program_id) {
  if (id_block_) {
    id_block_->ProgramId(program_id);
  }
}
std::string Mdf4File::ProgramId() const {
  return !id_block_ ? "" : id_block_->ProgramId();
}

IAttachment *Mdf4File::CreateAttachment() {
  IAttachment *attachment = nullptr;
  if (hd_block_) {
    auto at4 = std::make_unique<At4Block>();
    at4->Init(*hd_block_);
    hd_block_->AddAt4(at4);
    const auto &at_list = hd_block_->At4();
    attachment = at_list.empty() ? nullptr : at_list.back().get();
  }
  return attachment;
}

IDataGroup *Mdf4File::CreateDataGroup() {
  IDataGroup *dg = nullptr;
  if (hd_block_) {
    auto dg4 = std::make_unique<Dg4Block>();
    dg4->Init(*hd_block_);
    hd_block_->AddDg4(dg4);
    dg = hd_block_->LastDataGroup();
  }
  return dg;
}

bool Mdf4File::Write(std::streambuf& buffer) {

  if (!id_block_ || !hd_block_) {
    MDF_ERROR() << "No ID or HD block defined. Invalid use of function.";
    return false;
  }

  try {
    id_block_->Write(buffer);
    hd_block_->Write(buffer);
  } catch (const std::exception &err) {
    MDF_ERROR() << "Failed to write the MDF4 file. Error: " << err.what();
    return false;
  }
  return true;
}

void Mdf4File::IsFinalized(bool finalized, std::streambuf& buffer,
                           uint16_t standard_flags, uint16_t custom_flags) {
  if (id_block_) {
    id_block_->IsFinalized(finalized, buffer, standard_flags, custom_flags);
  }
}

bool Mdf4File::IsFinalized(uint16_t &standard_flags,
                           uint16_t &custom_flags) const {
  bool finalized = true;
  if (id_block_) {
    finalized = id_block_->IsFinalized(standard_flags, custom_flags);
  } else {
    standard_flags = 0;
    custom_flags = 0;
  }
  return finalized;
}

bool Mdf4File::FinalizeFile(std::streambuf& buffer) {

  uint16_t standard_flags = 0;
  uint16_t custom_flags = 0;
  const auto finalized = IsFinalized(standard_flags, custom_flags);
  if (finalized || standard_flags == 0 || !hd_block_) {
    MDF_DEBUG()
        << "The files standard flags are OK. Cannot finalize the file. File: "
        << Name();
    return false;
  }

  // 1. Update DL block
  // 2. Update DT  block
  // 3. Update RD block
  // 4. Update CG (CA) block
  const bool update_cg_blocks = (standard_flags & 0x01) != 0;
  const bool update_sr_blocks = (standard_flags & 0x02) != 0;
  const bool update_dt_blocks = (standard_flags & 0x04) != 0;
  const bool update_rd_blocks = (standard_flags & 0x08) != 0;
  const bool update_dl_blocks = (standard_flags & 0x10) != 0;
  const bool update_vlsd_bytes = (standard_flags & 0x20) != 0;
  const bool update_vlsd_offset = (standard_flags & 0x40) != 0;
  bool updated = true;
  if (update_dl_blocks) {
    MDF_ERROR() << "Finalizing DL block is not supported. File: " << Name();
    updated = false;
  }
  if (update_dt_blocks) {
    // Update the number of bytes in the DT block.
    const bool update_dt = hd_block_->FinalizeDtBlocks(buffer);
    if (!update_dt) {
      MDF_ERROR() << "Fail to finalize last DT block. File: " << Name();
      updated = false;
    }
  }
  if (update_rd_blocks) {
    MDF_ERROR() << "Finalizing RD block is not supported. File: " << Name();
    updated = false;
  }

  if (update_cg_blocks || update_vlsd_bytes || update_vlsd_offset ) {
    const bool update_nof_samples = hd_block_->FinalizeCgAndVlsdBlocks(
        buffer, update_cg_blocks, update_vlsd_bytes || update_vlsd_offset);
    if (!update_nof_samples) {
      MDF_ERROR() << "Failed to update number of samples and VLSD block sizes. File: " << Name();
      updated = false;
    }
  }
  if (update_sr_blocks) {
    MDF_ERROR() << "Finalizing SR block is not supported. File: " << Name();
    updated = false;
  }
  return updated;
}

IDataGroup *Mdf4File::FindParentDataGroup(const IChannel &channel) const {
  const auto channel_index = channel.Index();
  if (!hd_block_ || channel_index <= 0) {
    return nullptr;
  }
  const auto &dg_list = hd_block_->Dg4();
  const auto itr = std::find_if(dg_list.cbegin(), dg_list.cend(),
                                [&](const auto &dg_block) {
    return dg_block && dg_block->Find(channel_index) != nullptr;
    });

  return itr != dg_list.cend() ? itr->get() : nullptr;
}

void Mdf4File::FindAllReferences(std::streambuf& buffer) {
  // Find all channel arrays
  if (!hd_block_) {
    return;
  }

    // Find all channel array
  for (auto& dg4 : hd_block_->Dg4()) {
    if (!dg4) {
      continue;
    }
    for (auto& cg4 : dg4->Cg4()) {
      if (!cg4) {
        continue;
      }
      for (auto& cn4 : cg4->Cn4()) {
        if (!cn4) {
          continue;
        }
        for (auto& block : cn4->Cx4()) {
          if (!block) {
            continue;
          }

          try {
            auto* ca4 = dynamic_cast<Ca4Block*> (block.get());
            if (ca4 != nullptr) {
              ca4->FindAllReferences(buffer);
            }
          } catch (const std::exception& ) {
          }

        }
      }
    }
  }
}

bool Mdf4File::IsFinalizedDone() const {
  return finalized_done_;
}

}  // namespace mdf::detail