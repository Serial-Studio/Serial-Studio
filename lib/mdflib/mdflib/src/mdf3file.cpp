/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "mdf3file.h"

#include <cstdio>
#include <memory>
#include <stdexcept>
#include <algorithm>

#include "mdf/mdflogstream.h"

namespace mdf::detail {

Mdf3File::Mdf3File()
    : id_block_(std::make_unique<IdBlock>()),
      hd_block_(std::make_unique<Hd3Block>()) {
  id_block_->SetDefaultMdf3Values();
  hd_block_->Init(*id_block_);
}

Mdf3File::Mdf3File(std::unique_ptr<IdBlock> id_block)
    : id_block_(std::move(id_block)) {}

void Mdf3File::Attachments(AttachmentList &dest) const {}

void Mdf3File::DataGroups(DataGroupList &dest) const {
  if (!hd_block_) {
    return;
  }
  const auto &dg_list = hd_block_->Dg3();
  for (const auto &dg : dg_list) {
    dest.emplace_back(dg.get());
  }
}

IHeader *Mdf3File::Header() const { return hd_block_.get(); }

void Mdf3File::ReadHeader(std::streambuf& buffer) {
  if (!id_block_) {
    id_block_ = std::make_unique<IdBlock>();
  }
  if (id_block_->FilePosition() < 0) {
    SetFilePosition(buffer, 0);
    id_block_->Read(buffer);
  }

  if (!hd_block_) {
    hd_block_ = std::make_unique<Hd3Block>();
  }
  if (hd_block_->FilePosition() <= 0) {
    hd_block_->Init(*id_block_);
    SetFilePosition(buffer, 64);
    hd_block_->Read(buffer);
  }
}

void Mdf3File::ReadMeasurementInfo(std::streambuf& buffer) {
  ReadHeader(buffer);
  if (hd_block_) {
    hd_block_->ReadMeasurementInfo(buffer);
  }
}
void Mdf3File::ReadEverythingButData(std::streambuf& buffer) {
  ReadHeader(buffer);
  if (hd_block_) {
    hd_block_->ReadMeasurementInfo(buffer);
    hd_block_->ReadEverythingButData(buffer);
  }
}
bool Mdf3File::IsMdf4() const { return false; }

const IdBlock &Mdf3File::Id() const {
  if (!id_block_) {
    throw std::domain_error("ID block not initialized yet");
  }
  return *id_block_;
}

const Hd3Block &Mdf3File::Hd() const {
  if (!hd_block_) {
    throw std::domain_error("HD3 block not initialized yet");
  }
  return *hd_block_;
}

MdfBlock *Mdf3File::Find(int64_t id) const {
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

IDataGroup *Mdf3File::CreateDataGroup() {
  return hd_block_ ? hd_block_->CreateDataGroup() : nullptr;
}

bool Mdf3File::Write(std::streambuf& buffer) {

  if (!id_block_ || !hd_block_) {
    MDF_ERROR() << "No ID or HD block defined. Invalid use of function.";
    return false;
  }
  try {
    id_block_->Write(buffer);
    hd_block_->Write(buffer);
  } catch (const std::exception &err) {
    MDF_ERROR() << "Failed to write MDF3 file. Error: " << err.what();
  }
  return true;
}

std::string Mdf3File::Version() const {
  return !id_block_ ? "" : id_block_->VersionString();
}

void Mdf3File::ProgramId(const std::string &program_id) {
  if (id_block_) {
    id_block_->ProgramId(program_id);
  }
}

std::string Mdf3File::ProgramId() const {
  return !id_block_ ? "" : id_block_->ProgramId();
}

void Mdf3File::MinorVersion(int minor) {
  if (id_block_) {
    id_block_->MinorVersion(minor);
  }
}
void Mdf3File::IsFinalized(bool finalized, std::streambuf& buffer,
                           uint16_t standard_flags, uint16_t custom_flags) {
  if (id_block_) {
    id_block_->IsFinalized(finalized, buffer, standard_flags, custom_flags);
  }
}

bool Mdf3File::IsFinalized(uint16_t &standard_flags,
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

IDataGroup *Mdf3File::FindParentDataGroup(const IChannel &channel) const {
  const auto channel_index = channel.Index();
  if (!hd_block_ || channel_index <= 0) {
    return nullptr;
  }
  auto &dg_list = hd_block_->Dg3();
  const auto itr = std::find_if(dg_list.cbegin(), dg_list.cend(),
                                [&](const auto &dg_block) {
    return dg_block && dg_block->Find(channel_index) != nullptr;
  });

  return itr != dg_list.cend() ? itr->get() : nullptr;
}

}  // namespace mdf::detail
