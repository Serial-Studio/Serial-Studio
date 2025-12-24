/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "hd3block.h"

#include <algorithm>
#include <vector>

#include "mdf/mdfhelper.h"
#include "pr3block.h"

namespace {

constexpr size_t kIndexDg = 0;
constexpr size_t kIndexTx = 1;
constexpr size_t kIndexPr = 2;
constexpr size_t kIndexNext = 0;

}  // namespace

namespace mdf::detail {
Hd3Block::Hd3Block() {
  block_type_ = "HD";
  UtcTimestamp now(MdfHelper::NowNs());
  timestamp_.SetTime(now);
}
const Hd3Block::Dg3List &Hd3Block::Dg3() const { return dg_list_; }

std::string Hd3Block::Comment() const { return comment_; }

MdfBlock *Hd3Block::Find(int64_t index) const {
  if (pr_block_) {
    auto *p = pr_block_->Find(index);
    if (p != nullptr) {
      return p;
    }
  }
  for (auto &dg : dg_list_) {
    if (!dg) {
      continue;
    }
    auto *p = dg->Find(index);
    if (p != nullptr) {
      return p;
    }
  }
  return MdfBlock::Find(index);
}

void Hd3Block::GetBlockProperty(BlockPropertyList &dest) const {
  MdfBlock::GetBlockProperty(dest);

  dest.emplace_back("Links", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("First DG", ToHexString(Link(kIndexDg)),
                    "Link to first data group", BlockItemType::LinkItem);
  dest.emplace_back("Comment TX", ToHexString(Link(kIndexTx)), comment_,
                    BlockItemType::LinkItem);
  if (pr_block_) {
    std::string pr_text = pr_block_->Text();
    if (pr_text.size() > 20) {
      pr_text.resize(20);
      pr_text += "..";
    }
    dest.emplace_back("Program PR", ToHexString(Link(kIndexPr)), pr_text,
                      BlockItemType::LinkItem);
  } else {
    dest.emplace_back("Program PR", ToHexString(Link(kIndexPr)),
                      "Link to program data", BlockItemType::LinkItem);
  }
  dest.emplace_back("", "", "", BlockItemType::BlankItem);

  dest.emplace_back("Information", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Number of DG Blocks", std::to_string(nof_dg_blocks_));
  dest.emplace_back("Date", timestamp_.date_);
  dest.emplace_back("Time", timestamp_.time_);
  dest.emplace_back("Author", author_);
  dest.emplace_back("Project", project_);
  dest.emplace_back("Organization", organisation_);
  dest.emplace_back("Time Stamp [ns]", std::to_string(timestamp_.local_timestamp_));
  dest.emplace_back("UTC Time Offset [h]", std::to_string(timestamp_.utc_offset_));
  switch (timestamp_.time_quality_) {
    case 0:
      dest.emplace_back("Time Source", "PC Time");
      break;

    case 10:
      dest.emplace_back("Time Source", "External");
      break;

    case 16:
      dest.emplace_back("Time Source", "NTP/PTP");
      break;
  }
  dest.emplace_back("Timer ID", timestamp_.timer_id_);
  dest.emplace_back("Comment", comment_);
}

uint64_t Hd3Block::Read(std::streambuf& buffer) {
  uint64_t bytes = ReadHeader3(buffer);
  bytes += ReadLinks3(buffer, 3);
  // The for loop handles earlier versions of the MDF file
  for (int ii = 3; bytes < block_size_; ++ii) {
    switch (ii) {
      case 3:
        bytes += ReadNumber(buffer, nof_dg_blocks_);
        break;
      case 4:
        bytes += ReadStr(buffer, timestamp_.date_, 10);
        break;
      case 5:
        bytes += ReadStr(buffer, timestamp_.time_, 8);
        break;
      case 6:
        bytes += ReadStr(buffer, author_, 32);
        break;
      case 7:
        bytes += ReadStr(buffer, organisation_, 32);
        break;
      case 8:
        bytes += ReadStr(buffer, project_, 32);
        break;
      case 9:
        bytes += ReadStr(buffer, subject_, 32);
        break;
      case 10:
        bytes += ReadNumber(buffer, timestamp_.local_timestamp_);
        break;
      case 11:
        bytes += ReadNumber(buffer, timestamp_.utc_offset_);
        break;
      case 12:
        bytes += ReadNumber(buffer, timestamp_.time_quality_);
        break;
      case 13:
        bytes += ReadStr(buffer, timestamp_.timer_id_, 32);
        break;
      default:
        std::string temp;
        bytes += ReadStr(buffer, temp, 1);
        break;
    }
  }
  comment_ = ReadTx3(buffer, kIndexTx);

  if (Link(kIndexPr) > 0) {
    pr_block_ = std::make_unique<Pr3Block>();
    pr_block_->Init(*this);
    SetFilePosition(buffer, Link(kIndexPr));
    pr_block_->Read(buffer);
  }
  return bytes;
}

uint64_t Hd3Block::Write(std::streambuf& buffer) {
  const bool update =
      FilePosition() > 0;  // Write or update the values inside the block
  if (!update) {
    block_type_ = "HD";
    block_size_ = (2 + 2) + (3 * 4) + 2 + 10 + 8 + 4 * 32 + 3 * 2 + 32;
    link_list_.resize(3, 0);
  }

  uint64_t bytes = update ? MdfBlock::Update(buffer) : MdfBlock::Write(buffer);

  nof_dg_blocks_ = static_cast<uint16_t>(dg_list_.size());
  bytes += WriteNumber(buffer, nof_dg_blocks_);
  bytes += WriteStr(buffer, timestamp_.date_, 10);
  bytes += WriteStr(buffer, timestamp_.time_, 8);
  bytes += WriteStr(buffer, author_, 32);
  bytes += WriteStr(buffer, organisation_, 32);
  bytes += WriteStr(buffer, project_, 32);
  bytes += WriteStr(buffer, subject_, 32);
  bytes += WriteNumber(buffer, timestamp_.local_timestamp_);
  bytes += WriteNumber(buffer, timestamp_.utc_offset_);
  bytes += WriteNumber(buffer, timestamp_.time_quality_);
  bytes += WriteStr(buffer, timestamp_.timer_id_, 32);

  if (!update) {
    UpdateBlockSize(buffer, bytes);
  }

  if (!comment_.empty() && Link(kIndexTx) <= 0) {
    Tx3Block tx(comment_);
    tx.Init(*this);
    tx.Write(buffer);
    UpdateLink(buffer, kIndexTx, tx.FilePosition());
  }

  if (pr_block_ && Link(kIndexPr) <= 0) {
    pr_block_->Init(*this);
    pr_block_->Write(buffer);
    UpdateLink(buffer, kIndexPr, pr_block_->FilePosition());
  }

  for (size_t index = 0; index < dg_list_.size(); ++index) {
    auto &dg3 = dg_list_[index];
    if (!dg3) {
      continue;
    }

    // Only last DG is updated
    const bool last_dg =
        index + 1 >= dg_list_.size();  // Only last DG is updated
    if (!last_dg && dg3->FilePosition() > 0) {
      continue;
    }

    dg3->Write(buffer);
    if (index == 0) {
      UpdateLink(buffer, kIndexDg, dg3->FilePosition());
    } else {
      auto &prev = dg_list_[index - 1];
      if (prev) {
        prev->UpdateLink(buffer, kIndexNext, dg3->FilePosition());
      }
    }
  }
  return bytes;
}

void Hd3Block::ReadMeasurementInfo(std::streambuf& buffer) {
  dg_list_.clear();
  for (auto link = Link(kIndexDg); link > 0; /* No ++ here*/) {
    auto dg = std::make_unique<Dg3Block>();
    dg->Init(*this);
    SetFilePosition(buffer, link);
    dg->Read(buffer);
    link = dg->Link(kIndexNext);
    dg_list_.emplace_back(std::move(dg));
  }
}

void Hd3Block::ReadEverythingButData(std::streambuf& buffer) {
  // We assume that ReadMeasurementInfo have been called earlier
  for (auto &dg : dg_list_) {
    if (!dg) {
      continue;
    }
    for (auto &cg : dg->Cg3()) {
      cg->ReadCnList(buffer);
      cg->ReadSrList(buffer);
    }
  }
}

int64_t Hd3Block::Index() const { return FilePosition(); }

void Hd3Block::Author(const std::string &author) { author_ = author; }

std::string Hd3Block::Author() const { return author_; }

void Hd3Block::Department(const std::string &department) {
  organisation_ = department;
}

std::string Hd3Block::Department() const { return organisation_; }

void Hd3Block::Project(const std::string &name) { project_ = name; }

std::string Hd3Block::Project() const { return project_; }

void Hd3Block::Subject(const std::string &subject) { subject_ = subject; }

std::string Hd3Block::Subject() const { return subject_; }

void Hd3Block::Description(const std::string &description) {
  comment_ = description;
}

std::string Hd3Block::Description() const { return comment_; }

void Hd3Block::StartTime(uint64_t ns_since_1970) {
  timestamp_.SetTime(ns_since_1970);
}

uint64_t Hd3Block::StartTime() const {
  return timestamp_.local_timestamp_;
}

void Hd3Block::StartTime(ITimestamp &timestamp) {
  timestamp_.SetTime(timestamp);
}

IDataGroup *Hd3Block::CreateDataGroup() {
  auto dg3 = std::make_unique<Dg3Block>();
  dg3->Init(*this);
  dg_list_.push_back(std::move(dg3));
  return dg_list_.empty() ? nullptr : dg_list_.back().get();
}

IDataGroup *Hd3Block::LastDataGroup() const {
  return dg_list_.empty() ? nullptr : dg_list_.back().get();
}

std::vector<IDataGroup *> Hd3Block::DataGroups() const {
  std::vector<IDataGroup *> list;
  std::for_each(dg_list_.begin(), dg_list_.end(),
                        [&](const auto &dg3) { list.push_back(dg3.get()); });
  return list;
}

void Hd3Block::AddDg3(std::unique_ptr<Dg3Block> &dg3) {
  dg3->Init(*this);
  dg_list_.push_back(std::move(dg3));
}

const mdf::IMdfTimestamp *Hd3Block::StartTimestamp() const { 
  return &timestamp_;
}


}  // end namespace mdf::detail