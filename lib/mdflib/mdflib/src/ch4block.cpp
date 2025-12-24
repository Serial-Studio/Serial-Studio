/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "ch4block.h"

#include <algorithm>
#include "hd4block.h"
namespace {

constexpr size_t kIndexCh = 1;
constexpr size_t kIndexTx = 2;
constexpr size_t kIndexMd = 3;
constexpr size_t kIndexElement = 4;
constexpr size_t kIndexNext = 0;
}  // namespace

namespace mdf::detail {
Ch4Block::Ch4Block() { block_type_ = "##CH"; }

void Ch4Block::GetBlockProperty(BlockPropertyList &dest) const {
  MdfBlock::GetBlockProperty(dest);

  dest.emplace_back("Links", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Next CH", ToHexString(Link(kIndexNext)), "",
                    BlockItemType::LinkItem);
  dest.emplace_back("First CH", ToHexString(Link(kIndexNext)), "",
                    BlockItemType::LinkItem);
  dest.emplace_back("Name TX", ToHexString(Link(kIndexNext)), name_,
                    BlockItemType::LinkItem);
  dest.emplace_back("Comment MD", ToHexString(Link(kIndexMd)), Comment(),
                    BlockItemType::LinkItem);
  for (uint32_t ii = 0; ii < nof_elements_; ++ii) {
    size_t index = kIndexElement + (3 * ii);
    dest.emplace_back("Reference DG", ToHexString(Link(index)), "",
                      BlockItemType::LinkItem);
    dest.emplace_back("Reference CG", ToHexString(Link(index + 1)), "",
                      BlockItemType::LinkItem);
    dest.emplace_back("Reference CN", ToHexString(Link(index + 2)), "",
                      BlockItemType::LinkItem);
  }
  dest.emplace_back("", "", "", BlockItemType::BlankItem);

  dest.emplace_back("Information", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Nof Elements", std::to_string(nof_elements_));
  dest.emplace_back("Type", std::to_string(type_));

  if (md_comment_) {
    md_comment_->GetBlockProperty(dest);
  }
}

uint64_t Ch4Block::Read(std::streambuf& buffer) {  // NOLINT
  uint64_t bytes = ReadHeader4(buffer);
  bytes += ReadNumber(buffer, nof_elements_);
  bytes += ReadNumber(buffer, type_);

  std::vector<uint8_t> reserved;
  bytes += ReadByte(buffer, reserved, 3);

  name_ = ReadTx4(buffer, kIndexTx);
  ReadMdComment(buffer, kIndexMd);
  ReadLink4List(buffer, ch_list_, kIndexCh);
  return bytes;
}

uint64_t Ch4Block::Write(std::streambuf& buffer) {  // NOLINT
  const auto update = FilePosition() > 0;
  if (update) {
    WriteLink4List(buffer, ch_list_, kIndexCh,
                   UpdateOption::DoNotUpdateWrittenBlock);
    return block_length_;
  }
  nof_elements_ = static_cast<uint32_t>(element_list_.size());
  block_type_ = "##CH";
  block_length_ = 24 + ((4 + (nof_elements_ * 3)) * 8) + 8 + 1 + 3;
  link_list_.resize(4 + (nof_elements_ * 3), 0);

  for (size_t index_n = 0; index_n < nof_elements_; ++index_n) {
    const auto index = 4 + (index_n * 3);
    const auto &element = element_list_[index_n];
    link_list_[index] =
        element.data_group != nullptr ? element.data_group->Index() : 0;
    link_list_[index + 1] =
        element.channel_group != nullptr ? element.channel_group->Index() : 0;
    link_list_[index + 2] =
        element.channel != nullptr ? element.channel->Index() : 0;
  }
  WriteTx4(buffer, kIndexTx, name_);
  WriteMdComment(buffer, kIndexMd);
  WriteLink4List(buffer, ch_list_, kIndexCh,
                 UpdateOption::DoNotUpdateWrittenBlock);

  uint64_t bytes = MdfBlock::Write(buffer);
  bytes += WriteNumber(buffer, nof_elements_);
  bytes += WriteNumber(buffer, type_);
  bytes += WriteBytes(buffer, 3);
  UpdateBlockSize(buffer, bytes);

  return bytes;
}

MdfBlock *Ch4Block::Find(int64_t index) const {  // NOLINT
  for (auto &ch : ch_list_) {
    if (!ch) {
      continue;
    }
    auto *p = ch->Find(index);
    if (p != nullptr) {
      return p;
    }
  }
  return MdfBlock::Find(index);
}

int64_t Ch4Block::Index() const { return FilePosition(); }

const std::string &Ch4Block::Name() const { return name_; }

void Ch4Block::Name(const std::string &name) { name_ = name; }

void Ch4Block::Type(ChType type) { type_ = static_cast<uint8_t>(type); }

ChType Ch4Block::Type() const { return static_cast<ChType>(type_); }

void Ch4Block::Description(const std::string &description) {
  md_comment_ = std::make_unique<Md4Block>(description);
}

std::string Ch4Block::Description() const { return MdText(); }

IMetaData *Ch4Block::CreateMetaData() {
  return MdfBlock::CreateMetaData();
}

IMetaData *Ch4Block::MetaData() const {
  return MdfBlock::MetaData();
}

void Ch4Block::AddElementLink(const ElementLink &element) {
  element_list_.push_back(element);
}

const std::vector<ElementLink> &Ch4Block::ElementLinks() const {
  return element_list_;
}

void Ch4Block::FindReferencedBlocks(const Hd4Block &hd4) {  // NOLINT
  element_list_.clear();
  for (uint32_t index_n = 0; index_n < nof_elements_; ++index_n) {
    const auto index = 5 + (index_n * 3);
    if (index < link_list_.size()) {
      const auto *block1 = hd4.Find(Link(index));
      const auto *block2 = hd4.Find(Link(index + 1));
      const auto *block3 = hd4.Find(Link(index + 2));
      ElementLink element;
      if (block1 != nullptr) {
        element.data_group = dynamic_cast<const IDataGroup *>(block1);
      }
      if (block2 != nullptr) {
        element.channel_group = dynamic_cast<const IChannelGroup *>(block2);
      }
      if (block3 != nullptr) {
        element.channel = dynamic_cast<const IChannel *>(block3);
      }
      element_list_.emplace_back(element);
    }
  }
  for (auto &ch4 : ch_list_) {
    if (ch4) {
      ch4->FindReferencedBlocks(hd4);
    }
  }
}

IChannelHierarchy *Ch4Block::CreateChannelHierarchy() {
  auto ch4 = std::make_unique<Ch4Block>();
  ch4->Init(*this);
  ch_list_.push_back(std::move(ch4));
  return ch_list_.empty() ? nullptr : ch_list_.back().get();
}

std::vector<IChannelHierarchy *> Ch4Block::ChannelHierarchies() const {
  std::vector<IChannelHierarchy *> list;
  std::transform(ch_list_.cbegin(), ch_list_.cend(), std::back_inserter(list),
                         [](const auto &ch4) { return ch4.get(); });
  return list;
}

}  // namespace mdf::detail