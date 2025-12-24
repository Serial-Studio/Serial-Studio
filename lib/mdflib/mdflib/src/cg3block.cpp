/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "cg3block.h"

#include <algorithm>
#include <cstdio>

#include "mdf/idatagroup.h"
#include "mdf/mdflogstream.h"

namespace {

constexpr size_t kIndexNext = 0;
constexpr size_t kIndexCn = 1;
constexpr size_t kIndexTx = 2;
constexpr size_t kIndexSr = 3;

}  // namespace

namespace mdf::detail {

int64_t Cg3Block::Index() const { return FilePosition(); }

void Cg3Block::Name(const std::string &name) {}

std::string Cg3Block::Name() const { return {}; }

void Cg3Block::Description(const std::string &description) {
  comment_ = description;
}
std::string Cg3Block::Description() const { return comment_; }

uint64_t Cg3Block::NofSamples() const { return nof_records_; }

void Cg3Block::NofSamples(uint64_t nof_samples) {
  nof_records_ = static_cast<uint32_t>(nof_samples);
}

void Cg3Block::RecordId(uint64_t record_id) {
  record_id_ = static_cast<uint16_t>(record_id);
}

uint64_t Cg3Block::RecordId() const { return record_id_; }

std::vector<IChannel *> Cg3Block::Channels() const {
  std::vector<IChannel *> channel_list;
  std::for_each(cn_list_.begin(), cn_list_.end(),
                [&](const auto &cn) { channel_list.push_back(cn.get()); });
  return channel_list;
}

const IChannel *Cg3Block::GetXChannel(const IChannel &) const {
  // Search for the master channel in the group
  auto master = std::find_if(cn_list_.cbegin(), cn_list_.cend(),
                             [&](const auto &cn3) {
    return cn3 && (cn3->Type() == ChannelType::Master ||
                         cn3->Type() == ChannelType::VirtualMaster);
  });

  return master != cn_list_.cend() ? master->get() : nullptr;
}

void Cg3Block::GetBlockProperty(BlockPropertyList &dest) const {
  MdfBlock::GetBlockProperty(dest);

  dest.emplace_back("Links", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Next CG", ToHexString(Link(kIndexNext)),
                    "Link to next channel group", BlockItemType::LinkItem);
  dest.emplace_back("First CN", ToHexString(Link(kIndexCn)),
                    "Link to first channel", BlockItemType::LinkItem);
  dest.emplace_back("Comment TX", ToHexString(Link(kIndexTx)), comment_,
                    BlockItemType::LinkItem);
  dest.emplace_back("Reduction SR", ToHexString(Link(kIndexSr)),
                    "Link to first sample reduction", BlockItemType::LinkItem);
  dest.emplace_back("", "", "", BlockItemType::BlankItem);

  dest.emplace_back("Information", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Record ID", std::to_string(record_id_));
  dest.emplace_back("Nof Channels", std::to_string(nof_channels_));
  dest.emplace_back("Record Size [bytes]",
                    std::to_string(size_of_data_record_));
  dest.emplace_back("Nof Samples", std::to_string(nof_records_));
  dest.emplace_back("Comment", comment_);
}

uint64_t Cg3Block::Read(std::streambuf& buffer) {
  uint64_t bytes = ReadHeader3(buffer);
  bytes += ReadLinks3(buffer, 3);

  bytes += ReadNumber(buffer, record_id_);
  bytes += ReadNumber(buffer, nof_channels_);
  bytes += ReadNumber(buffer, size_of_data_record_);
  bytes += ReadNumber(buffer, nof_records_);
  if (bytes + 4 < block_size_) {
    uint32_t link = 0;
    bytes += ReadNumber(buffer, link);
    link_list_.emplace_back(link);
  }
  comment_ = ReadTx3(buffer, kIndexTx);
  return bytes;
}

uint64_t Cg3Block::Write(std::streambuf& buffer) {
  const bool update =
      FilePosition() > 0;  // Write or update the values inside the block
  nof_channels_ = static_cast<uint16_t>(cn_list_.size());

  if (!update) {
    block_type_ = "CG";
    block_size_ = (2 + 2) + (3 * 4) + 2 + 2 + 2 + 4 + 4;
    link_list_.resize(
        3, 0);  // Note that an extra link is added at the end of the block.
  }

  int64_t sr_link =
      0;  // This link is added at the end of the block not at the beginning
  for (size_t sr_index = 0; sr_index < sr_list_.size(); ++sr_index) {
    auto &sr3 = sr_list_[sr_index];
    if (!sr3 || sr3->FilePosition() > 0) {
      continue;
    }
    sr3->Write(buffer);
    if (sr_index == 0) {
      sr_link = sr3->FilePosition();
    } else {
      if (auto &prev = sr_list_[sr_index - 1]; prev) {
        prev->UpdateLink(buffer, kIndexNext, sr3->FilePosition());
      }
    }
  }

  uint64_t bytes = update ? MdfBlock::Update(buffer) : MdfBlock::Write(buffer);
  if (!update) {
    bytes += WriteNumber(buffer, record_id_);
    bytes += WriteNumber(buffer, nof_channels_);
    bytes += WriteNumber(buffer, size_of_data_record_);
    bytes += WriteNumber(buffer, nof_records_);
    bytes += WriteNumber(buffer, static_cast<uint32_t>(sr_link));
  } else {
    // Only nof_records and SR blocks may be added
    bytes += StepFilePosition(buffer, sizeof(record_id_) + sizeof(nof_channels_) +
                                        sizeof(size_of_data_record_));
    bytes += WriteNumber(buffer, nof_records_);
    // Check if any SR blocks been added. This typical happens after a
    // measurement.
    if (sr_link > 0) {
      bytes += WriteNumber(buffer, static_cast<uint32_t>(sr_link));
    } else {
      bytes += StepFilePosition(buffer, sizeof(uint32_t));
    }
  }

  for (size_t cn_index = 0; cn_index < cn_list_.size(); ++cn_index) {
    auto &cn3 = cn_list_[cn_index];
    if (!cn3 || cn3->FilePosition() > 0) {
      continue;
    }
    cn3->Write(buffer);
    if (cn_index == 0) {
      UpdateLink(buffer, kIndexCn, cn3->FilePosition());
    } else {
      if (auto &prev = cn_list_[cn_index - 1]; prev) {
        prev->UpdateLink(buffer, kIndexNext, cn3->FilePosition());
      }
    }
  }

  if (!comment_.empty() && Link(kIndexTx) <= 0) {
    Tx3Block tx(comment_);
    tx.Init(*this);
    tx.Write(buffer);
    UpdateLink(buffer, kIndexTx, tx.FilePosition());
  }
  return bytes;
}

void Cg3Block::ReadCnList(std::streambuf& buffer) {
  if (cn_list_.empty() && Link(kIndexCn) > 0) {
    for (auto link = Link(kIndexCn); link > 0; /* No ++ here*/) {
      auto cn = std::make_unique<Cn3Block>();
      cn->Init(*this);
      SetFilePosition(buffer, link);
      cn->Read(buffer);
      link = cn->Link(kIndexNext);
      cn_list_.push_back(std::move(cn));
    }
  }
}

void Cg3Block::ReadSrList(std::streambuf& buffer) {
  if (sr_list_.empty() && Link(kIndexSr) > 0) {
    for (auto link = Link(kIndexSr); link > 0; /* No ++ here*/) {
      auto sr = std::make_unique<Sr3Block>();
      sr->Init(*this);
      SetFilePosition(buffer, link);
      sr->Read(buffer);
      link = sr->Link(kIndexNext);
      sr_list_.push_back(std::move(sr));
    }
  }
}

MdfBlock *Cg3Block::Find(int64_t index) const {
  for (auto &p : cn_list_) {
    if (!p) {
      continue;
    }
    auto *pp = p->Find(index);
    if (pp != nullptr) {
      return pp;
    }
  }
  return MdfBlock::Find(index);
}

std::string Cg3Block::Comment() const { return comment_; }

void Cg3Block::AddCn3(std::unique_ptr<Cn3Block> &cn3) {
  cn_list_.push_back(std::move(cn3));
  nof_channels_ = static_cast<uint16_t>(cn_list_.size());
}

bool Cg3Block::PrepareForWriting() {
  try {
    nof_channels_ = static_cast<uint16_t>(cn_list_.size());
    uint64_t record_size = 0;
    size_of_data_record_ = 0;
    for (auto &cn3 : cn_list_) {
      cn3->ByteOffset(static_cast<uint32_t>(record_size));
      record_size += cn3->DataBytes();
      size_of_data_record_ = static_cast<uint16_t>(record_size);
    }
    if (record_size > 0xFFFF) {
      throw std::runtime_error("The record size is to large. Size; "
        + std::to_string(record_size) + " > " + std::to_string(0xFFFF));
    }
    sample_buffer_.resize(static_cast<size_t>(record_size));

  } catch (const std::exception &err) {
    MDF_ERROR() << "Failed to prepare for writing. Error: " << err.what();
    return false;
  }
  return true;
}

uint64_t Cg3Block::ReadDataRecord(std::streambuf& buffer,
                                const IDataGroup &notifier) const {
  // Normal fixed length records
  std::size_t record_size = size_of_data_record_;
  std::vector<uint8_t> record(record_size, 0);
  uint64_t count =
      buffer.sgetn(reinterpret_cast<char *>(record.data()), record.size());
  uint64_t sample = Sample();
  if (sample < NofSamples()) {
    const bool continue_reading = notifier.NotifySampleObservers(sample, RecordId(), record);
    IncrementSample();
    if (!continue_reading) {
      return 0; // Indicating no more reading
    }
  }

  return count;
}

uint64_t Cg3Block::ReadRangeDataRecord(std::streambuf& buffer,
                                const IDataGroup &notifier,
                                     DgRange& range) const {
  uint64_t count;
  const uint64_t sample = Sample();
  const uint64_t next_sample = sample + 1;
  const size_t record_size = size_of_data_record_;
  CgRange* cg_range = range.GetCgRange(RecordId());
  if (cg_range == nullptr) {
    return 0;
  }

  if (!cg_range->IsUsed() ||
      next_sample < range.MinSample() ||
      next_sample > range.MaxSample()) {
    count = StepFilePosition(buffer, record_size);

    if (sample < NofSamples()) {
      IncrementSample();
    }
    if (next_sample > range.MaxSample()) {
      cg_range->IsUsed(false);
    }
  } else {
    // Normal fixed length records

    std::vector<uint8_t> record(record_size, 0);

    count = buffer.sgetn(
        reinterpret_cast<char*>(record.data()), record.size());

    if (sample < NofSamples()) {
      const bool continue_reading =
          notifier.NotifySampleObservers(sample, RecordId(), record);
      IncrementSample();
      if (!continue_reading) {
        return 0;  // Indicating no more reading
      }
    }
  }

  return count;
}
IChannel *Cg3Block::CreateChannel() {
  auto cn3 = std::make_unique<detail::Cn3Block>();
  cn3->Init(*this);
  AddCn3(cn3);
  return cn_list_.empty() ? nullptr : cn_list_.back().get();
}

void Cg3Block::ReadData(std::streambuf& buffer) const {
  for (const auto& sr3 : Sr3()) {
    if (!sr3) { continue;}
    sr3->ReadData(buffer);
  }
}

void Cg3Block::ClearData() {
  IChannelGroup::ClearData();
  for (const auto& sr3 : Sr3() ) {
    if (sr3) {
      sr3->ClearData();
    }
  }
}

const IDataGroup *Cg3Block::DataGroup() const {
  const auto* mdf_block = DgBlock();
  return mdf_block != nullptr ? dynamic_cast<const IDataGroup*>(mdf_block) : nullptr;
}

}  // namespace mdf::detail
