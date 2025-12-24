/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "ev4block.h"

#include "hd4block.h"

namespace {
constexpr size_t kIndexNext = 0;
constexpr size_t kIndexParent = 1;
constexpr size_t kIndexRange = 2;
constexpr size_t kIndexName = 3;
constexpr size_t kIndexMd = 4;
constexpr size_t kIndexScope = 5;

std::string MakeTypeString(uint8_t type) {
  switch (type) {
    case 0:
      return "Recording";
    case 1:
      return "Recording Interrupt";
    case 2:
      return "Acquisition Interrupt";
    case 3:
      return "Start Recording";
    case 4:
      return "Stop Recording";
    case 5:
      return "Trigger";
    case 6:
      return "Marker";
    default:
      break;
  }
  return "Unknown";
}

std::string MakeSyncTypeString(uint8_t type) {
  switch (type) {
    case 0:
      return "";
    case 1:
      return "Time Calculated";
    case 2:
      return "Angle Calculated";
    case 3:
      return "Distance Calculated";
    case 4:
      return "Index Calculated";
    default:
      break;
  }
  return "Unknown";
}

std::string MakeRangeTypeString(uint8_t type) {
  switch (type) {
    case 0:
      return "Point";
    case 1:
      return "Begin";
    case 2:
      return "End";
    default:
      break;
  }
  return "Unknown";
}

std::string MakeCauseString(uint8_t cause) {
  switch (cause) {
    case 0:
      return "Other";
    case 1:
      return "Error";
    case 2:
      return "Tool";
    case 3:
      return "Script";
    case 4:
      return "User";
    default:
      break;
  }
  return "Unknown";
}

std::string MakeFlagString(uint8_t flag) {
  std::ostringstream s;
  if (flag & 0x01) {
    s << "Post";
  }
  return s.str();
}
}  // namespace

namespace mdf::detail {

Ev4Block::Ev4Block() { block_type_ = "##EV"; }

void Ev4Block::GetBlockProperty(BlockPropertyList &dest) const {
  MdfBlock::GetBlockProperty(dest);

  dest.emplace_back("Links", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Next EV", ToHexString(Link(kIndexNext)),
                    "Link to next event", BlockItemType::LinkItem);
  dest.emplace_back("Parent EV", ToHexString(Link(kIndexParent)),
                    "Reference to parent event", BlockItemType::LinkItem);
  dest.emplace_back("Range EV", ToHexString(Link(kIndexRange)),
                    "Reference to range begin event", BlockItemType::LinkItem);
  dest.emplace_back("Name TX", ToHexString(Link(kIndexName)), name_,
                    BlockItemType::LinkItem);
  dest.emplace_back("Comment MD", ToHexString(Link(kIndexMd)), Comment(),
                    BlockItemType::LinkItem);
  for (size_t m = 0; m < length_m_; ++m) {
    dest.emplace_back("Scope CG/CN", ToHexString(Link(kIndexScope + m)),
                      "Reference to scope ", BlockItemType::LinkItem);
  }
  for (size_t n = 0; n < length_n_; ++n) {
    dest.emplace_back("Attachment AT",
                      ToHexString(Link(kIndexScope + length_m_ + n)),
                      "Reference to attachments ", BlockItemType::LinkItem);
  }
  dest.emplace_back("", "", "", BlockItemType::BlankItem);

  dest.emplace_back("Information", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Name", name_);
  dest.emplace_back("Event Type", MakeTypeString(type_));
  dest.emplace_back("Sync Type", MakeSyncTypeString(sync_type_));
  dest.emplace_back("Range Type", MakeRangeTypeString(range_type_));
  dest.emplace_back("Cause", MakeCauseString(cause_));
  dest.emplace_back("Flags", MakeFlagString(flags_));

  dest.emplace_back("Nof Scopes", std::to_string(length_m_));
  dest.emplace_back("Nof Attachments", std::to_string(length_n_));
  dest.emplace_back("Creator Index", std::to_string(creator_index_));
  dest.emplace_back("Sync Base Value", std::to_string(sync_base_value_));
  dest.emplace_back("Sync Factor", ToString(sync_factor_));
  if (md_comment_) {
    md_comment_->GetBlockProperty(dest);
  }
}

uint64_t Ev4Block::Read(std::streambuf& buffer) {
  uint64_t bytes = ReadHeader4(buffer);
  bytes += ReadNumber(buffer, type_);
  bytes += ReadNumber(buffer, sync_type_);
  bytes += ReadNumber(buffer, range_type_);
  bytes += ReadNumber(buffer, cause_);
  bytes += ReadNumber(buffer, flags_);
  std::vector<uint8_t> reserved;
  bytes += ReadByte(buffer, reserved, 3);
  bytes += ReadNumber(buffer, length_m_);
  bytes += ReadNumber(buffer, length_n_);
  bytes += ReadNumber(buffer, creator_index_);
  bytes += ReadNumber(buffer, sync_base_value_);
  bytes += ReadNumber(buffer, sync_factor_);

  name_ = ReadTx4(buffer, kIndexName);
  const size_t group_index = 5 + length_m_ + length_n_;
  if (group_index < link_list_.size()) {
    group_name_ = ReadTx4(buffer, group_index);
  }

  ReadMdComment(buffer, kIndexMd);
  return bytes;
}

uint64_t Ev4Block::Write(std::streambuf& buffer) {
  const bool update =
      FilePosition() > 0;  // Write or update the values inside the block
  if (update) {
    return block_length_;
  }
  length_m_ = static_cast<uint32_t>(scope_list_.size());
  length_n_ = static_cast<uint16_t>(attachment_list_.size());

  const auto group = !group_name_.empty();
  if (group) {
    flags_ |= 0x02;
  }
  block_type_ = "##EV";
  block_length_ =
      24 + ((5 + length_m_ + length_n_) * 8) + 5 + 3 + 4 + 2 + 2 + 8 + 8;
  if (group) {
    block_size_ += 8;
  }
  link_list_.resize(5 + length_m_ + length_n_ + (group ? 1 : 0), 0);
  link_list_[kIndexParent] =
      parent_event_ != nullptr ? parent_event_->Index() : 0;
  link_list_[kIndexRange] = range_event_ != nullptr ? range_event_->Index() : 0;
  for (size_t index_m = 0; index_m < length_m_; ++index_m) {
    const auto index = 5 + index_m;
    const auto *block = reinterpret_cast<const MdfBlock *>(scope_list_[index_m]);
    link_list_[index] = block != nullptr ? block->FilePosition() : 0;
  }
  for (size_t index_n = 0; index_n < length_n_; ++index_n) {
    const auto index = 5 + length_m_ + index_n;
    const auto *block = attachment_list_[index_n];
    link_list_[index] = block != nullptr ? block->Index() : 0;
  }
  WriteTx4(buffer, kIndexName, name_);
  if (group) {
    WriteTx4(buffer, link_list_.size() - 1, group_name_);
  }
  WriteMdComment(buffer, kIndexMd);

  uint64_t bytes = MdfBlock::Write(buffer);
  bytes += WriteNumber(buffer, type_);
  bytes += WriteNumber(buffer, sync_type_);
  bytes += WriteNumber(buffer, range_type_);
  bytes += WriteNumber(buffer, cause_);
  bytes += WriteNumber(buffer, flags_);
  bytes += WriteBytes(buffer, 3);
  bytes += WriteNumber(buffer, length_m_);
  bytes += WriteNumber(buffer, length_n_);
  bytes += WriteNumber(buffer, creator_index_);
  bytes += WriteNumber(buffer, sync_base_value_);
  bytes += WriteNumber(buffer, sync_factor_);
  UpdateBlockSize(buffer, bytes);

  return bytes;
}

int64_t Ev4Block::Index() const { return FilePosition(); }

void Ev4Block::Name(const std::string &name) { name_ = name; }

const std::string &Ev4Block::Name() const { return name_; }

void Ev4Block::GroupName(const std::string &group_name) {
  group_name_ = group_name;
}

const std::string &Ev4Block::GroupName() const { return group_name_; }

void Ev4Block::Type(EventType event_type) {
  type_ = static_cast<uint8_t>(event_type);
}

EventType Ev4Block::Type() const { return static_cast<EventType>(type_); }

void Ev4Block::Sync(SyncType sync_type) {
  sync_type_ = static_cast<uint8_t>(sync_type);
}

SyncType Ev4Block::Sync() const { return static_cast<SyncType>(sync_type_); }

void Ev4Block::Range(RangeType range_type) {
  range_type_ = static_cast<uint8_t>(range_type);
}

RangeType Ev4Block::Range() const {
  return static_cast<RangeType>(range_type_);
}

void Ev4Block::Cause(EventCause cause) { cause_ = static_cast<uint8_t>(cause); }

EventCause Ev4Block::Cause() const { return static_cast<EventCause>(cause_); }

void Ev4Block::CreatorIndex(size_t index) { creator_index_ = static_cast<uint16_t>(index); }

size_t Ev4Block::CreatorIndex() const { return creator_index_; }

void Ev4Block::SyncValue(int64_t value) { sync_base_value_ = value; }

int64_t Ev4Block::SyncValue() const { return sync_base_value_; }

void Ev4Block::SyncFactor(double factor) { sync_factor_ = factor; }

double Ev4Block::SyncFactor() const { return sync_factor_; }

void Ev4Block::ParentEvent(const IEvent *parent) { parent_event_ = parent; }

const IEvent *Ev4Block::ParentEvent() const { return parent_event_; }

void Ev4Block::RangeEvent(const IEvent *range_event) {
  range_event_ = range_event;
}

const IEvent *Ev4Block::RangeEvent() const { return range_event_; }

void Ev4Block::AddScope(const void *scope) {
  if (scope != nullptr) {
    scope_list_.push_back(scope);
  }
}

const std::vector<const void *> &Ev4Block::Scopes() const {
  return scope_list_;
}

void Ev4Block::AddAttachment(const IAttachment *attachment) {
  if (attachment != nullptr) {
    attachment_list_.push_back(attachment);
  }
}

const std::vector<const IAttachment *> &Ev4Block::Attachments() const {
  return attachment_list_;
}

IMetaData *Ev4Block::CreateMetaData() {
  return MdfBlock::CreateMetaData();
}

IMetaData *Ev4Block::MetaData() const {
  return MdfBlock::MetaData();
}

void Ev4Block::FindReferencedBlocks(const Hd4Block &hd4) {
  parent_event_ = dynamic_cast<const Ev4Block *>(hd4.Find(Link(kIndexParent)));
  range_event_ = dynamic_cast<const Ev4Block *>(hd4.Find(Link(kIndexRange)));
  scope_list_.clear();
  for (size_t index_m = 0; index_m < length_m_; ++index_m) {
    const auto index = 5 + index_m;
    if (index < link_list_.size()) {
      scope_list_.push_back(hd4.Find(Link(index)));
    }
  }

  attachment_list_.clear();
  for (size_t index_n = 0; index_n < length_n_; ++index_n) {
    const auto index = 5 + length_m_ + index_n;
    if (index < link_list_.size()) {
      attachment_list_.push_back(
          dynamic_cast<const At4Block *>(hd4.Find(Link(index))));
    }
  }
}

}  // namespace mdf::detail