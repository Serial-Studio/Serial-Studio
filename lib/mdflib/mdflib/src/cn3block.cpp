/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "cn3block.h"

#include <limits>

#include "cg3block.h"
#include "mdf/mdfhelper.h"

namespace {
constexpr uint16_t kBitOffset = 0x2000;
constexpr size_t kIndexNext = 0;
constexpr size_t kIndexCc = 1;
constexpr size_t kIndexCe = 2;
constexpr size_t kIndexCd = 3;
constexpr size_t kIndexTx = 4;
constexpr size_t kIndexTxLong = 5;
constexpr size_t kIndexTxDisplay = 6;

constexpr std::string_view kTypeList[17] = {
    "Unsigned Integer",
    "Signed Integer",
    "Float",
    "Double",
    "VAX F Float",
    "VAX G Float",
    "VAX D Float",
    "String",
    "Byte Array",
    "Unsigned Integer BE",
    "Signed Integer BE",
    "Float BE",
    "Double BE",
    "Unsigned Integer LE",
    "Signed Integer LE",
    "Float LE",
    "Double LE",
};

}  // namespace

namespace mdf::detail {

int64_t Cn3Block::Index() const { return FilePosition(); }

void Cn3Block::Name(const std::string &name) {
  short_name_ = name;
  if (name.size() > 31) {
    short_name_.resize(31);
    long_name_ = name;
  } else {
    long_name_.clear();
    long_name_.shrink_to_fit();
  }
}

std::string Cn3Block::Name() const {
  return long_name_.empty() ? short_name_ : long_name_;
}

IChannelConversion *Cn3Block::ChannelConversion() const {
  return cc_block_.get();
}

ChannelDataType Cn3Block::DataType() const {
  switch (signal_type_) {
    case 0:
      return IsBigEndian() ? ChannelDataType::UnsignedIntegerBe
                           : ChannelDataType::UnsignedIntegerLe;
    case 1:
      return IsBigEndian() ? ChannelDataType::SignedIntegerBe
                           : ChannelDataType::SignedIntegerLe;

    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
      return IsBigEndian() ? ChannelDataType::FloatBe
                           : ChannelDataType::FloatLe;

    case 7:
      return ChannelDataType::StringAscii;

    case 8:
      if (cc_block_ && cc_block_->Type() == ConversionType::DateConversion) {
        return ChannelDataType::CanOpenDate;
      }
      if (cc_block_ && cc_block_->Type() == ConversionType::TimeConversion) {
        return ChannelDataType::CanOpenTime;
      }
      return ChannelDataType::ByteArray;

    case 9:
      return ChannelDataType::UnsignedIntegerBe;

    case 10:
      return ChannelDataType::SignedIntegerBe;

    case 11:
    case 12:
      return ChannelDataType::FloatBe;

    case 13:
      return ChannelDataType::UnsignedIntegerLe;

    case 14:
      return ChannelDataType::SignedIntegerLe;

    case 15:
    case 16:
      return ChannelDataType::FloatLe;

    default:
      break;
  }
  return IsBigEndian() ? ChannelDataType::UnsignedIntegerBe
                       : ChannelDataType::UnsignedIntegerLe;
}

ChannelType Cn3Block::Type() const {
  return channel_type_ == 1 ? ChannelType::Master : ChannelType::FixedLength;
}

uint64_t Cn3Block::DataBytes() const {
  return (static_cast<size_t>(nof_bits_) / 8) + (nof_bits_ % 8 > 0 ? 1 : 0);
}

uint8_t Cn3Block::Decimals() const {
  auto max = static_cast<uint8_t>(
      DataBytes() == 4 ? std::numeric_limits<float>::max_digits10
                       : std::numeric_limits<double>::max_digits10);
  return max;
}

bool Cn3Block::IsDecimalUsed() const { return false; }

bool Cn3Block::IsUnitValid() const {
  const auto *cc = ChannelConversion();
  return cc != nullptr || Type() == ChannelType::Master;
}

std::string Cn3Block::Unit() const {
  const auto *cc = ChannelConversion();
  if (cc != nullptr) {
    return cc->Unit();
  }
  if (Type() == ChannelType::Master) {
    return "s";
  }
  return {};
}

void Cn3Block::GetBlockProperty(BlockPropertyList &dest) const {
  MdfBlock::GetBlockProperty(dest);

  dest.emplace_back("Links", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Next CN", ToHexString(Link(kIndexNext)),
                    "Link to next channel", BlockItemType::LinkItem);
  dest.emplace_back("Conversion CC", ToHexString(Link(kIndexCc)),
                    "Link to channel conversion", BlockItemType::LinkItem);
  dest.emplace_back("Source Dependency CE", ToHexString(Link(kIndexCe)),
                    "Link to source dependency block", BlockItemType::LinkItem);
  dest.emplace_back("Dependency CD", ToHexString(Link(kIndexCd)),
                    "Link to dependency", BlockItemType::LinkItem);
  dest.emplace_back("Comment TX", ToHexString(Link(kIndexTx)), comment_,
                    BlockItemType::LinkItem);
  dest.emplace_back("", "", "", BlockItemType::BlankItem);

  dest.emplace_back("Information", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Channel Type",
                    channel_type_ == 0 ? "Data channel" : "Time channel");
  dest.emplace_back("Short Name", short_name_);
  dest.emplace_back("Description", description_);
  dest.emplace_back("Start Offset [bits]", std::to_string(start_offset_));
  dest.emplace_back("Number of bits", std::to_string(nof_bits_));
  dest.emplace_back("Signal Data Type", std::to_string(signal_type_),
      signal_type_ < 17 ? kTypeList[signal_type_].data() : "Unknown Type");
  dest.emplace_back("Range Valid", range_valid_ ? "True" : "False");
  dest.emplace_back("Min Range", MdfHelper::FormatDouble(min_, 6));
  dest.emplace_back("Max Range", MdfHelper::FormatDouble(max_, 6));
  dest.emplace_back("Sampling Rate [s]",
                    MdfHelper::FormatDouble(sample_rate_, 3));
  dest.emplace_back("Long Nane", long_name_);
  dest.emplace_back("Display Nane", display_name_);
  dest.emplace_back("byte Offset", std::to_string(byte_offset_));
  dest.emplace_back("Byte Offset", std::to_string(ByteOffset()));
}

uint64_t Cn3Block::Read(std::streambuf& buffer) {
  uint64_t bytes = ReadHeader3(buffer);
  bytes += ReadLinks3(buffer, 5);
  bytes += ReadNumber(buffer, channel_type_);
  bytes += ReadStr(buffer, short_name_, 32);
  bytes += ReadStr(buffer, description_, 128);
  bytes += ReadNumber(buffer, start_offset_);
  bytes += ReadNumber(buffer, nof_bits_);
  bytes += ReadNumber(buffer, signal_type_);
  bytes += ReadBool(buffer, range_valid_);
  bytes += ReadNumber(buffer, min_);
  bytes += ReadNumber(buffer, max_);
  bytes += ReadNumber(buffer, sample_rate_);
  // The for loop handle earlier version of the MDF file
  for (int ii = 0; bytes < block_size_; ++ii) {
    switch (ii) {
      case 1:
      case 0: {
        uint32_t link = 0;
        bytes += ReadNumber(buffer, link);
        link_list_.emplace_back(link);
        break;
      }

      case 2:
        bytes += ReadNumber(buffer, byte_offset_);
        break;

      default: {
        uint8_t temp = 0;
        bytes += ReadNumber(buffer, temp);
        break;
      }
    }
  }

  if (Link(kIndexCc) > 0) {
    SetFilePosition(buffer, Link(kIndexCc));
    cc_block_ = std::make_unique<Cc3Block>();
    cc_block_->Init(*this);
    cc_block_->Read(buffer);
  }

  if (Link(kIndexCe) > 0) {
    SetFilePosition(buffer, Link(kIndexCe));
    ce_block_ = std::make_unique<Ce3Block>();
    ce_block_->Init(*this);
    ce_block_->Read(buffer);
  }

  if (Link(kIndexCd) > 0) {
    SetFilePosition(buffer, Link(kIndexCd));
    cd_block_ = std::make_unique<Cd3Block>();
    cd_block_->Init(*this);
    cd_block_->Read(buffer);
  }

  comment_ = ReadTx3(buffer, kIndexTx);
  long_name_ = ReadTx3(buffer, kIndexTxLong);
  display_name_ = ReadTx3(buffer, kIndexTxDisplay);

  return bytes;
}

uint64_t Cn3Block::Write(std::streambuf& buffer) {
  int64_t long_name_link = 0;
  int64_t display_name_link = 0;

  const bool update =
      FilePosition() > 0;  // Write or update the values inside the block
  if (!update) {
    block_type_ = "CN";
    block_size_ =
        (2 + 2) + (5 * 4) + 2 + 32 + 128 + 3 * 2 + 2 + 3 * 8 + 2 * 4 + 2;
    link_list_.resize(5, 0);

    if (!long_name_.empty()) {
      Tx3Block tx(long_name_);
      tx.Init(*this);
      tx.Write(buffer);
      long_name_link = tx.FilePosition();
    }

    if (!display_name_.empty()) {
      Tx3Block tx(display_name_);
      tx.Init(*this);
      tx.Write(buffer);
      display_name_link = tx.FilePosition();
    }
  }

  uint64_t bytes = update ? block_size_ : MdfBlock::Write(buffer);
  if (!update) {
    bytes += WriteNumber(buffer, channel_type_);
    bytes += WriteStr(buffer, short_name_, 32);
    bytes += WriteStr(buffer, description_, 128);
    bytes += WriteNumber(buffer, start_offset_);
    bytes += WriteNumber(buffer, nof_bits_);
    bytes += WriteNumber(buffer, signal_type_);
    bytes += WriteBool(buffer, range_valid_);
    bytes += WriteNumber(buffer, min_);
    bytes += WriteNumber(buffer, max_);
    bytes += WriteNumber(buffer, sample_rate_);
    bytes += WriteNumber(buffer, static_cast<uint32_t>(long_name_link));
    bytes += WriteNumber(buffer, static_cast<uint32_t>(display_name_link));
    bytes += WriteNumber(buffer, byte_offset_);
  }

  if (cc_block_ && Link(kIndexCc) <= 0) {
    cc_block_->Write(buffer);
    UpdateLink(buffer, kIndexCc, cc_block_->FilePosition());
  }

  if (ce_block_ && Link(kIndexCe) <= 0) {
    ce_block_->Write(buffer);
    UpdateLink(buffer, kIndexCe, ce_block_->FilePosition());
  }

  if (cd_block_ && Link(kIndexCd) <= 0) {
    cd_block_->Write(buffer);
    UpdateLink(buffer, kIndexCd, cd_block_->FilePosition());
  }

  if (!comment_.empty() && Link(kIndexTx) <= 0) {
    Tx3Block tx(comment_);
    tx.Init(*this);
    tx.Write(buffer);
    UpdateLink(buffer, kIndexTx, tx.FilePosition());
  }

  return bytes;
}

void Cn3Block::BitCount(uint32_t bits ) {
  nof_bits_ = static_cast<uint16_t>(bits);
}

uint32_t Cn3Block::BitCount() const { return nof_bits_; }


void Cn3Block::BitOffset(uint16_t bits ) {
  start_offset_ = bits; // Should be 0..7

}

uint16_t Cn3Block::BitOffset() const { return start_offset_ % 8; }

uint32_t Cn3Block::ByteOffset() const {
  return static_cast<uint32_t>(start_offset_/ 8) +
    static_cast<uint32_t>(byte_offset_);
}

void Cn3Block::ByteOffset(uint32_t byte_offset) {
  if (byte_offset < kBitOffset) {
    //
    start_offset_ = static_cast<uint16_t>(byte_offset * 8);
    byte_offset_ = 0;
  } else {
    byte_offset_ = (byte_offset / kBitOffset) * kBitOffset;
    start_offset_ = static_cast<uint16_t>((byte_offset % kBitOffset) * 8);
  }
}

std::string Cn3Block::Comment() const { return comment_; }

MdfBlock *Cn3Block::Find(int64_t index) const {
  if (cc_block_) {
    auto *p = cc_block_->Find(index);
    if (p != nullptr) {
      return p;
    }
  }
  return DataListBlock::Find(index);
}

void Cn3Block::DisplayName(const std::string &name) { display_name_ = name; }

std::string Cn3Block::DisplayName() const { return display_name_; }

void Cn3Block::Description(const std::string &description) {
  description_ = description;
}

std::string Cn3Block::Description() const { return description_; }

void Cn3Block::Unit(const std::string &unit) {
  if (!cc_block_) {
    CreateChannelConversion();
    cc_block_->Type(ConversionType::NoConversion);
    cc_block_->Unit(unit);
  } else {
    cc_block_->Unit(unit);
  }
}

void Cn3Block::Type(ChannelType type) {
  channel_type_ = type == ChannelType::Master ? 1 : 0;
}

void Cn3Block::DataType(ChannelDataType type) {
  switch (type) {
    case ChannelDataType::UnsignedIntegerLe:
      signal_type_ = IsBigEndian() ? 13 : 0;
      if (DataBytes() == 0) {
        DataBytes(4);
      }
      break;

    case ChannelDataType::UnsignedIntegerBe:
      signal_type_ = IsBigEndian() ? 0 : 9;
      if (DataBytes() == 0) {
        DataBytes(4);
      }
      break;

    case ChannelDataType::SignedIntegerLe:
      signal_type_ = IsBigEndian() ? 14 : 1;
      if (DataBytes() == 0) {
        DataBytes(4);
      }
      break;

    case ChannelDataType::SignedIntegerBe:
      signal_type_ = IsBigEndian() ? 1 : 10;
      if (DataBytes() == 0) {
        DataBytes(4);
      }
      break;

    case ChannelDataType::FloatLe:
      signal_type_ = IsBigEndian() ? 15 : 2;
      if (DataBytes() == 8) {
        ++signal_type_;
      } else if (DataBytes() == 0) {
        DataBytes(4);
      }
      break;

    case ChannelDataType::FloatBe:
      signal_type_ = IsBigEndian() ? 2 : 11;
      if (DataBytes() == 8) {
        ++signal_type_;
      } else if (DataBytes() == 0) {
        DataBytes(4);
      }
      break;

    case ChannelDataType::StringUTF8:
    case ChannelDataType::StringAscii:
      signal_type_ = 7;
      if (DataBytes() == 0) {
        DataBytes(4);
      }
      break;

    case ChannelDataType::ByteArray:
      signal_type_ = 8;
      if (DataBytes() == 0) {
        DataBytes(6);
      }
      break;

    case ChannelDataType::CanOpenDate:
      signal_type_ = 8;  // Byte Array
      DataBytes(7);
      if (!cc_block_) {
        CreateChannelConversion();
      }
      cc_block_->Type(ConversionType::DateConversion);
      break;

    case ChannelDataType::CanOpenTime:
      signal_type_ = 8;  // Byte Array
      DataBytes(6);
      if (!cc_block_) {
        CreateChannelConversion();
      }
      cc_block_->Type(ConversionType::TimeConversion);
      break;

    default:
      signal_type_ = 0;
      if (DataBytes() == 0) {
        DataBytes(4);
      }
      break;
  }
}

void Cn3Block::DataBytes(uint64_t nof_bytes) {
  nof_bits_ = static_cast<uint16_t>(nof_bytes * 8);
  if (nof_bytes == 8 &&
      (signal_type_ == 2 || signal_type_ == 11 || signal_type_ == 15)) {
    ++signal_type_;  // Making it a double type instead.
  }
}

void Cn3Block::SamplingRate(double sampling_rate) {
  sample_rate_ = sampling_rate;
}

double Cn3Block::SamplingRate() const { return sample_rate_; }

void Cn3Block::AddCc3(std::unique_ptr<Cc3Block> &cc3) {
  cc_block_ = std::move(cc3);
}

std::vector<uint8_t> &Cn3Block::SampleBuffer() const {
  return cg3_block->SampleBuffer();
}

void Cn3Block::Init(const MdfBlock &id_block) {
  MdfBlock::Init(id_block);
  cg3_block = dynamic_cast<const Cg3Block *>(&id_block);
}

IChannelConversion *Cn3Block::CreateChannelConversion() {
  if (!cc_block_) {
    cc_block_ = std::make_unique<Cc3Block>();
    cc_block_->Init(*this);
  }
  return cc_block_.get();
}

IChannel *Cn3Block::CreateChannelComposition() { return nullptr; }
std::vector<IChannel *> Cn3Block::ChannelCompositions() {
  return {};
}

const IChannelGroup* Cn3Block::ChannelGroup() const {
  return dynamic_cast<const IChannelGroup*> (CgBlock());
}

}  // namespace mdf::detail
