/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "fh4block.h"
namespace {
constexpr size_t kIndexNext = 0;
constexpr size_t kIndexMd = 1;
}  // namespace
namespace mdf::detail {

Fh4Block::Fh4Block() {
  block_type_ = "##FH";
  UtcTimestamp now(MdfHelper::NowNs());
  timestamp_.SetTime(now);
}

void Fh4Block::GetBlockProperty(BlockPropertyList &dest) const {
  MdfBlock::GetBlockProperty(dest);

  dest.emplace_back("Links", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Next FH", ToHexString(Link(kIndexNext)), "",
                    BlockItemType::LinkItem);
  dest.emplace_back("Comment MD", ToHexString(Link(kIndexMd)), Comment(),
                    BlockItemType::LinkItem);
  dest.emplace_back("", "", "", BlockItemType::BlankItem);

  dest.emplace_back("FH Info", "", "", BlockItemType::HeaderItem);
  timestamp_.GetBlockProperty(dest);
  if (md_comment_) {
    md_comment_->GetBlockProperty(dest);
  }
}

uint64_t Fh4Block::Read(std::streambuf& buffer) {
  uint64_t bytes = ReadHeader4(buffer);
  timestamp_.Init(*this);
  bytes += timestamp_.Read(buffer);
  ReadMdComment(buffer, kIndexMd);
  return bytes;
}

uint64_t Fh4Block::Write(std::streambuf& buffer) {
  const bool update =
      FilePosition() > 0;  // Write or update the values inside the block
  if (update) {
    return block_length_;
  }
  block_type_ = "##FH";
  block_length_ = 24 + (2 * 8) + 8 + 2 + 2 + 1 + 3;
  link_list_.resize(2, 0);

  uint64_t bytes = MdfBlock::Write(buffer);
  bytes += timestamp_.Write(buffer);
  bytes += WriteBytes(buffer, 3);
  UpdateBlockSize(buffer, bytes);
  WriteMdComment(buffer, kIndexMd);
  return bytes;
}

IMetaData *Fh4Block::CreateMetaData() { return MdfBlock::CreateMetaData(); }

IMetaData *Fh4Block::MetaData() const { return MdfBlock::MetaData(); }

int64_t Fh4Block::Index() const { return FilePosition(); }

void Fh4Block::Time(uint64_t ns_since_1970) {
  timestamp_.SetTime(ns_since_1970);
}

uint64_t Fh4Block::Time() const { return timestamp_.GetTimeNs(); }

void Fh4Block::Time(ITimestamp &timestamp) { timestamp_.SetTime(timestamp); }

const mdf::IMdfTimestamp *Fh4Block::StartTimestamp() const { return &timestamp_; }

}  // namespace mdf::detail