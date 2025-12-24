/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "dg3block.h"

#include <algorithm>


namespace {
constexpr size_t kIndexNext = 0;
constexpr size_t kIndexCg = 1;
constexpr size_t kIndexTr = 2;
constexpr size_t kIndexData = 3;
}  // namespace
namespace mdf::detail {

MdfBlock *Dg3Block::Find(int64_t index) const {
  if (tr_block_) {
    auto *pos = tr_block_->Find(index);
    if (pos != nullptr) {
      return pos;
    }
  }
  for (auto &cg3 : cg_list_) {
    if (!cg3) {
      continue;
    }
    auto *pos = cg3->Find(index);
    if (pos != nullptr) {
      return pos;
    }
  }
  return DataListBlock::Find(index);
}

void Dg3Block::GetBlockProperty(BlockPropertyList &dest) const {
  MdfBlock::GetBlockProperty(dest);

  dest.emplace_back("Links", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Next DG", ToHexString(Link(kIndexNext)),
                    "Link to next data group", BlockItemType::LinkItem);
  dest.emplace_back("First CG", ToHexString(Link(kIndexCg)),
                    "Link to first channel group", BlockItemType::LinkItem);
  dest.emplace_back("Link Data", ToHexString(Link(kIndexTr)),
                    "Link to trigger data", BlockItemType::LinkItem);
  dest.emplace_back("Link Data", ToHexString(Link(kIndexData)), "Link to Data",
                    BlockItemType::LinkItem);
  dest.emplace_back("", "", "", BlockItemType::BlankItem);

  dest.emplace_back("Information", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Channel Groups", std::to_string(nof_cg_blocks_));
  dest.emplace_back("Record ID size [bytes]", std::to_string(nof_record_id_));
}

uint64_t Dg3Block::Read(std::streambuf& buffer) {
  uint64_t bytes = ReadHeader3(buffer);
  bytes += ReadLinks3(buffer, 4);
  bytes += ReadNumber(buffer, nof_cg_blocks_);
  bytes += ReadNumber(buffer, nof_record_id_);

  if (Link(kIndexTr) > 0) {
    tr_block_ = std::make_unique<Tr3Block>();
    tr_block_->Init(*this);
    SetFilePosition(buffer, Link(kIndexTr));
    tr_block_->Read(buffer);
  }

  if (Link(kIndexData) > 0) {
    auto dt3 = std::make_unique<Dt3Block>();
    dt3->Init(*this);
    SetFilePosition(buffer, Link(kIndexData));
    dt3->Read(buffer);
    block_list_.push_back(std::move(dt3));
  }

  cg_list_.clear();
  for (auto link = Link(kIndexCg); link > 0; /* No ++ here*/) {
    auto cg3 = std::make_unique<Cg3Block>();
    cg3->Init(*this);
    SetFilePosition(buffer, link);
    cg3->Read(buffer);
    link = cg3->Link(kIndexNext);
    cg_list_.push_back(std::move(cg3));
  }

  return bytes;
}

uint64_t Dg3Block::Write(std::streambuf& buffer) {
  const bool update =
      FilePosition() > 0;  // Write or update the values inside the block
  nof_cg_blocks_ = static_cast<uint16_t>(cg_list_.size());
  nof_record_id_ = nof_cg_blocks_ > 1 ? 1 : 0;
  if (!update) {
    block_type_ = "DG";
    block_size_ = (2 + 2) + (4 * 4) + 2 + 2 + 4;
    link_list_.resize(4, 0);
  }

  uint64_t bytes = update ? MdfBlock::Update(buffer) : MdfBlock::Write(buffer);
  bytes += WriteNumber(buffer, nof_cg_blocks_);
  bytes += WriteNumber(buffer, nof_record_id_);
  const std::vector<uint8_t> reserved(4, 0);
  bytes += WriteByte(buffer, reserved);

  if (tr_block_ && Link(kIndexTr) <= 0) {
    tr_block_->Write(buffer);
    UpdateLink(buffer, kIndexTr, tr_block_->FilePosition());
  }

  for (size_t cg_index = 0; cg_index < cg_list_.size(); ++cg_index) {
    auto &cg3 = cg_list_[cg_index];
    if (!cg3) {
      continue;
    }
    cg3->Write(buffer);
    if (cg_index == 0) {
      UpdateLink(buffer, kIndexCg, cg3->FilePosition());
    } else {
      auto &prev = cg_list_[cg_index - 1];
      if (prev) {
        prev->UpdateLink(buffer, kIndexNext, cg3->FilePosition());
      }
    }
  }

  for (size_t ii = 0; ii < block_list_.size(); ++ii) {
    auto &data = block_list_[ii];
    if (!data) {
      continue;
    }
    if (data->FilePosition() > 0) {
      continue;
    }
    data->Write(buffer);
    if (ii == 0) {
      UpdateLink(buffer, kIndexData, data->FilePosition());
    }
  }

  return bytes;
}

void Dg3Block::AddCg3(std::unique_ptr<Cg3Block> &cg3) {
  cg3->Init(*this);
  cg3->RecordId(0);
  cg_list_.push_back(std::move(cg3));
  nof_cg_blocks_ = static_cast<uint16_t>(cg_list_.size());
  nof_record_id_ = cg_list_.size() > 1 ? 1 : 0;
  uint8_t id3 = cg_list_.size() < 2 ? 0 : 1;
  std::for_each(cg_list_.begin(), cg_list_.end(),
                [&](auto &group) { group->RecordId(id3++); });
}

const Dg3Block::Cg3List &Dg3Block::Cg3() const { return cg_list_; }

int64_t Dg3Block::Index() const { return FilePosition(); }

std::vector<IChannelGroup *> Dg3Block::ChannelGroups() const {
  std::vector<IChannelGroup *> list;
  for (const auto &cg3 : cg_list_) {
    if (cg3) {
      list.emplace_back(cg3.get());
    }
  }
  return list;
}

IChannelGroup *Dg3Block::CreateChannelGroup() {
  auto cg3 = std::make_unique<detail::Cg3Block>();
  cg3->Init(*this);
  AddCg3(cg3);
  return cg_list_.empty() ? nullptr : cg_list_.back().get();
}

void Dg3Block::ReadData(std::streambuf& buffer) {
  InitFastObserverList();

  uint64_t data_size = DataSize();
  SetFilePosition(buffer, Link(kIndexData));

  // Read through all record
  ParseDataRecords(buffer, data_size);

  // Read in all SR block data
  for (const auto& cg3 :Cg3()) {
    if (!cg3) { continue;}
    cg3->ReadData(buffer);
  }
}

void Dg3Block::ReadRangeData(std::streambuf& buffer, DgRange& range) {

  InitFastObserverList();

  uint64_t data_size = DataSize();
  SetFilePosition(buffer, Link(kIndexData));

  // Read in all SR block data
  for (const auto& cg3 :Cg3()) {
    if (!cg3) {
      continue;
    }
    const auto record_id = cg3->RecordId();
    if (range.IsUsed(record_id)) {
      cg3->ReadData(buffer);
    }
  }

  //  auto pos = GetFilePosition(data_file);
  // Read through all record
  ParseRangeDataRecords(buffer, data_size, range);


}
void Dg3Block::ClearData() {
  DataListBlock::ClearData();
  IDataGroup::ClearData();
}

void Dg3Block::ParseDataRecords(std::streambuf& buffer, uint64_t nof_data_bytes) {
  if (nof_data_bytes == 0) {
    return;
  }
  for (const auto& channel_group : Cg3() ) {
    if (channel_group) {
      channel_group->ResetSampleCounter();
    }
  }

  for (uint64_t count = 0; count < nof_data_bytes; /* No ++count here*/) {
    // 1. Read Record ID
    uint8_t record_id = 0;
    if (nof_record_id_ == 1 || nof_record_id_ == 2) {
      count += ReadNumber(buffer, record_id);
    }
    const auto *cg3 = FindCgRecordId(record_id);
    if (cg3 == nullptr) {
      break;
    }

    const auto read = cg3->ReadDataRecord(buffer, *this);
    if (read == 0) {
      break;
    }
    count += read;
    if (nof_record_id_ == 2) {
      count += ReadNumber(buffer, record_id);
    }
  }
}

void Dg3Block::ParseRangeDataRecords(std::streambuf& buffer, uint64_t nof_data_bytes,
                                     DgRange& range) {
  if (nof_data_bytes == 0) {
    return;
  }
  for (const auto& channel_group : Cg3() ) {
    if (channel_group) {
      channel_group->ResetSampleCounter();
    }
  }

  for (uint64_t count = 0; count < nof_data_bytes; /* No ++count here*/) {
    // 1. Read Record ID
    uint8_t record_id = 0;
    if (nof_record_id_ == 1 || nof_record_id_ == 2) {
      count += ReadNumber(buffer, record_id);
    }
    const auto *cg3 = FindCgRecordId(record_id);
    const auto* cg_range = range.GetCgRange(record_id);
    if (cg3 == nullptr || cg_range == nullptr) {
      break;
    }

    const auto read = cg3->ReadRangeDataRecord(buffer,
                                               *this,
                                               range);
    if (read == 0) {
      break;
    }
    count += read;
    if (nof_record_id_ == 2) {
      count += ReadNumber(buffer, record_id);
    }
    if (range.IsReady()) {
      break;
    }
  }
}

const Cg3Block *Dg3Block::FindCgRecordId(const uint64_t record_id) const {
  if (cg_list_.size() == 1) {
    return cg_list_[0].get();
  }

  for (const auto &cg : cg_list_) {
    if (!cg) {
      continue;
    }
    if (cg->RecordId() == record_id) {
      return cg.get();
    }
  }
  return nullptr;
}

IChannelGroup* Dg3Block::FindParentChannelGroup(const IChannel&
                                                          channel) const {
  const auto channel_index = channel.Index();
  const auto &cg_list = Cg3();
  const auto itr = std::find_if(cg_list.cbegin(), cg_list.cend(),
                                [&](const auto &cg_block) {
    return cg_block && cg_block->Find(channel_index) != nullptr;
  });
  return itr != cg_list.cend() ? itr->get() : nullptr;
}



}  // end namespace mdf::detail