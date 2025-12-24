/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "si4block.h"
namespace {

constexpr size_t kIndexName = 0;
constexpr size_t kIndexPath = 1;
constexpr size_t kIndexMd = 2;

std::string MakeSourceTypeString(uint8_t type) {
  switch (type) {
    case 0:
      return "Other";
    case 1:
      return "ECU";
    case 2:
      return "Bus";
    case 3:
      return "I/O";
    case 4:
      return "Tool";
    case 5:
      return "User";
    default:
      break;
  }
  return "Unknown";
}

std::string MakeBusTypeString(uint8_t type) {
  switch (type) {
    case 0:
      return "";
    case 1:
      return "Other";
    case 2:
      return "CAN";
    case 3:
      return "LIN";
    case 4:
      return "MOST";
    case 5:
      return "FlexRay";
    case 6:
      return "K-Line";
    case 7:
      return "Ethernet";
    case 8:
      return "USB";
    default:
      break;
  }
  return "Unknown";
}

std::string MakeFlagString(uint16_t flag) {
  std::ostringstream s;
  if (flag & 0x01) {
    s << "Simulated";
  }
  return s.str();
}

}  // namespace
namespace mdf::detail {
Si4Block::Si4Block() {
  block_type_ = "##SI";
}

void Si4Block::GetBlockProperty(BlockPropertyList &dest) const {
  MdfBlock::GetBlockProperty(dest);

  dest.emplace_back("Links", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Name TX", ToHexString(Link(kIndexName)), name_,
                    BlockItemType::LinkItem);
  dest.emplace_back("Path TX", ToHexString(Link(kIndexPath)), path_,
                    BlockItemType::LinkItem);
  dest.emplace_back("Comment MD", ToHexString(Link(kIndexMd)), Comment(),
                    BlockItemType::LinkItem);
  dest.emplace_back("", "", "", BlockItemType::BlankItem);

  dest.emplace_back("Information", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Name", name_);
  dest.emplace_back("Path", path_);
  dest.emplace_back("Source Type", MakeSourceTypeString(type_));
  dest.emplace_back("Bus Type", MakeBusTypeString(bus_type_));
  dest.emplace_back("Flags", MakeFlagString(flags_));

  if (md_comment_) {
    md_comment_->GetBlockProperty(dest);
  }
}

uint64_t Si4Block::Read(std::streambuf& buffer) {
  uint64_t bytes = ReadHeader4(buffer);
  bytes += ReadNumber(buffer, type_);
  bytes += ReadNumber(buffer, bus_type_);
  bytes += ReadNumber(buffer, flags_);
  std::vector<uint8_t> reserved;
  bytes += ReadByte(buffer, reserved, 5);
  name_ = ReadTx4(buffer, kIndexName);
  path_ = ReadTx4(buffer, kIndexPath);
  ReadMdComment(buffer, kIndexMd);
  return bytes;
}

uint64_t Si4Block::Write(std::streambuf& buffer) {
  const bool update = FilePosition() > 0;  // True if already written to file
  if (update) {
    return block_length_;
  }

  block_type_ = "##SI";
  block_length_ = 24 + (3 * 8) + 1 + 1 + 1 + 5;
  link_list_.resize(3, 0);
  WriteTx4(buffer, kIndexName, name_);
  WriteTx4(buffer, kIndexPath, path_);
  WriteMdComment(buffer, kIndexMd);

  uint64_t bytes = MdfBlock::Write(buffer);
  bytes += WriteNumber(buffer, type_);
  bytes += WriteNumber(buffer, bus_type_);
  bytes += WriteNumber(buffer, flags_);
  bytes += WriteBytes(buffer, 5);
  UpdateBlockSize(buffer, bytes);
  return bytes;
}

int64_t Si4Block::Index() const { return FilePosition(); }

void Si4Block::Name(const std::string &name) { name_ = name; }

const std::string &Si4Block::Name() const { return name_; }

void Si4Block::Path(const std::string &path) { path_ = path; }

const std::string &Si4Block::Path() const { return path_; }

void Si4Block::Description(const std::string &desc) {
  auto *metadata = CreateMetaData();
  if (metadata != nullptr) {
    metadata->StringProperty("TX", desc);
  }
}

std::string Si4Block::Description() const {
  const auto *metadata = MetaData();
  return metadata != nullptr ? metadata->StringProperty("TX") : std::string();
}

void Si4Block::Type(SourceType type) { type_ = static_cast<uint8_t>(type); }

SourceType Si4Block::Type() const { return static_cast<SourceType>(type_); }

void Si4Block::Bus(BusType type) { bus_type_ = static_cast<uint8_t>(type); }

BusType Si4Block::Bus() const { return static_cast<BusType>(bus_type_); }

void Si4Block::Flags(uint8_t flags) { flags_ = flags; }

uint8_t Si4Block::Flags() const { return flags_; }

IMetaData *Si4Block::CreateMetaData() {
  return MdfBlock::CreateMetaData();
}

IMetaData *Si4Block::MetaData() const {
  return MdfBlock::MetaData();
}


}  // namespace mdf::detail