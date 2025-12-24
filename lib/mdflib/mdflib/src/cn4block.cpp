/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "cn4block.h"

#include <ctime>
#include <string>

#include "bigbuffer.h"
#include "ca4block.h"
#include "cg4block.h"
#include "littlebuffer.h"
#include "sd4block.h"
#include "dz4block.h"
#include "dl4block.h"
#include "hl4block.h"

namespace {

constexpr size_t kIndexNext = 0;
constexpr size_t kIndexCx = 1;
constexpr size_t kIndexName = 2;
constexpr size_t kIndexSi = 3;
constexpr size_t kIndexCc = 4;
constexpr size_t kIndexData = 5;
constexpr size_t kIndexUnit = 6;
constexpr size_t kIndexMd = 7;
constexpr size_t kIndexAt = 8;

constexpr uint32_t kDefaultXFlag =
    0x1000;  ///< Default X-axis defined for this channel

constexpr size_t kMaxDataSize = 4'000'000;
std::string MakeTypeString(uint8_t type) {
  switch (type) {
    case 0:
      return "Fixed";
    case 1:
      return "VLSD";
    case 2:
      return "Master";
    case 3:
      return "Virtual Master";
    case 4:
      return "Synchronization";
    case 5:
      return "MLSD";
    case 6:
      return "Virtual Data";
    default:
      break;
  }
  return "Unknown";
}

std::string MakeSyncString(uint8_t type) {
  switch (type) {
    case 0:
      return "None";
    case 1:
      return "Time";
    case 2:
      return "Angle";
    case 3:
      return "Distance";
    case 4:
      return "Index";
    default:
      break;
  }
  return "Unknown";
}

std::string MakeDataTypeString(uint8_t type) {
  switch (type) {
    case 0:
      return "Unsigned Integer (LE)";
    case 1:
      return "Unsigned Integer (BE)";
    case 2:
      return "Signed Integer (LE)";
    case 3:
      return "Signed Integer (BE)";
    case 4:
      return "Float (LE)";
    case 5:
      return "Float (BE)";
    case 6:
      return "String ASCII";
    case 7:
      return "String UTF-8";
    case 8:
      return "String UTF-16 (LE)";
    case 9:
      return "String UTF-16 (BE)";
    case 10:
      return "Byte Array";
    case 11:
      return "MIME Sample";
    case 12:
      return "MIME Stream";
    case 13:
      return "CAN Open Date";
    case 14:
      return "CAN Open Time";
    default:
      break;
  }
  return "Unknown";
}

std::string MakeFlagString(uint32_t flag) {
  std::ostringstream s;
  if (flag & 0x01) {
    s << "All Invalid";
  }
  if (flag & 0x02) {
    s << (s.str().empty() ? "Invalid" : ",Invalid");
  }
  if (flag & 0x04) {
    s << (s.str().empty() ? "Decimal" : ",Decimal");
  }
  if (flag & 0x08) {
    s << (s.str().empty() ? "Range" : ",Range");
  }
  if (flag & 0x10) {
    s << (s.str().empty() ? "Limit" : ",Limit");
  }
  if (flag & 0x20) {
    s << (s.str().empty() ? "Extended" : ",Extended");
  }
  if (flag & 0x40) {
    s << (s.str().empty() ? "Discrete" : ",Discrete");
  }
  if (flag & 0x80) {
    s << (s.str().empty() ? "Calibration" : ",Calibration");
  }
  if (flag & 0x100) {
    s << (s.str().empty() ? "Calculated" : ",Calculated");
  }
  if (flag & 0x200) {
    s << (s.str().empty() ? "Virtual" : ",Virtual");
  }
  if (flag & 0x400) {
    s << (s.str().empty() ? "Bus Event" : ",Bus Event");
  }
  if (flag & 0x800) {
    s << (s.str().empty() ? "Monotonous" : ",Monotonous");
  }
  if (flag & 0x1000) {
    s << (s.str().empty() ? "Default X" : ",Default X");
  }
  if (flag & 0x2000) {
    s << (s.str().empty() ? "Signal Event" : ",Signal Event");
  }
  if (flag & 0x4000) {
    s << (s.str().empty() ? "VLSD" : ",VLSD");
  }
  return s.str();
}

///< Helper function that recursively copies all data bytes to a
/// destination buffer.
uint64_t CopyDataToBuffer(const mdf::detail::MdfBlock *data, std::streambuf& from_file,
                        std::vector<uint8_t> &buffer, uint64_t &buffer_index) {
  if (data == nullptr) {
    return 0;
  }
  uint64_t count = 0;
  const auto *db = dynamic_cast<const mdf::detail::DataBlock *>(data);
  const auto *dl = dynamic_cast<const mdf::detail::DataListBlock *>(data);
  if (db != nullptr) {
    count += db->CopyDataToBuffer(from_file, buffer, buffer_index);
  } else if (dl != nullptr) {
    for (const auto &block : dl->DataBlockList()) {
      count += CopyDataToBuffer(block.get(), from_file, buffer, buffer_index);
    }
  }
  return count;
}

}  // end namespace

namespace mdf::detail {

Cn4Block::Cn4Block() { block_type_ = "##CN"; }

void Cn4Block::UpdateDataLink(std::streambuf& buffer, int64_t position) {
  UpdateLink(buffer, kIndexData, position);
}

int64_t Cn4Block::DataLink() const { return Link(kIndexData); }


std::vector<int64_t> Cn4Block::AtLinkList() const {
  std::vector<int64_t> link_list;
  link_list.reserve(nof_attachments_);
  for (uint16_t ii = 0; ii < nof_attachments_; ++ii) {
    link_list.push_back(Link(kIndexAt + ii));
  }
  return link_list;
}

std::vector<int64_t> Cn4Block::XAxisLinkList() const {
  std::vector<int64_t> link_list;
  for (uint16_t ii = 0; ii < 3 && (flags_ & kDefaultXFlag) != 0; ++ii) {
    link_list.push_back(Link(kIndexAt + nof_attachments_ + ii));
  }
  return link_list;
}

int64_t Cn4Block::Index() const { return FilePosition(); }

void Cn4Block::Name(const std::string &name) { name_ = name; }

std::string Cn4Block::Name() const { return name_; }

void Cn4Block::DisplayName(const std::string &name) {
  // No support in MDF4
}

std::string Cn4Block::DisplayName() const {
  // No support in MDF4
  return {};
}

void Cn4Block::Description(const std::string &description) {
  if (!md_comment_) {
    if (auto* md4 = CreateMetaData(); md4 != nullptr) {
      md4->StringProperty("TX", description);
    }
  } else if (auto* md4_block = dynamic_cast<Md4Block*>(md_comment_.get());
        md4_block != nullptr) {
    md4_block->TxComment(description);
  }
}

std::string Cn4Block::Description() const {
  return MdText();
}

IChannelConversion *Cn4Block::ChannelConversion() const {
  return cc_block_.get();
}

ChannelDataType Cn4Block::DataType() const {
  return static_cast<ChannelDataType>(data_type_);
}

ChannelType Cn4Block::Type() const { return static_cast<ChannelType>(type_); }

uint64_t Cn4Block::DataBytes() const {
  auto value_size = static_cast<uint64_t>((bit_count_) / 8)
                    + (bit_count_ % 8 > 0 ? 1 : 0);
/*
  auto* channel_array = ChannelArray();
  if (channel_array != nullptr) {
    // Increase the size with number of values. Validate the array by
    // checking its size that must be larger than 1.
    const auto nof_values = channel_array->NofArrayValues();
    if (nof_values > 1) {
      value_size *= channel_array->NofArrayValues();
    }
  }
  */
  return value_size;
}

void Cn4Block::Decimals(uint8_t precision) {
  precision_ = precision;
  flags_ |= CnFlag::PrecisionValid;
}

uint8_t Cn4Block::Decimals() const {
  const auto max = static_cast<uint8_t>(
      DataBytes() == 4 ? std::numeric_limits<float>::max_digits10
                       : std::numeric_limits<double>::max_digits10);
  return flags_ & CnFlag::PrecisionValid ?
         precision_ : max;
}

bool Cn4Block::IsDecimalUsed() const { return flags_ & CnFlag::PrecisionValid; }

bool Cn4Block::IsUnitValid() const {
  const bool invalid = Link(kIndexUnit) == 0 && !unit_;
  return !invalid;
}

std::string Cn4Block::Unit() const {

  if (!unit_) {
    switch (Sync()) {
      case ChannelSyncType::Time:
        return "s";

      case ChannelSyncType::Angle:
        return "rad";

      case ChannelSyncType::Distance:
        return "m";

      default:
        break;
    }

    // Return the CC block unit if it exists.
    return cc_block_ ? cc_block_->Unit() : std::string();
  }

  if (unit_->IsTxtBlock()) {
    return unit_->Text();
  }
  return unit_->TxComment();
}

void Cn4Block::GetBlockProperty(BlockPropertyList &dest) const {
  MdfBlock::GetBlockProperty(dest);

  dest.emplace_back("Links", "", "", BlockItemType::HeaderItem);

  dest.emplace_back("Next CN", ToHexString(Link(kIndexNext)),
                    "Link to next channel", BlockItemType::LinkItem);
  std::ostringstream desc_cx;
  const auto index_cx = Link(kIndexCx);
  const auto* block_cx = Find(index_cx);
  if (block_cx != nullptr) {
    desc_cx << "Link to " << block_cx->BlockType() << " block";
  } else {
    desc_cx << "Link to composition block";
  }
  dest.emplace_back("Composition CA/CN", ToHexString(index_cx),
                    desc_cx.str(), BlockItemType::LinkItem);
  dest.emplace_back("Name TX", ToHexString(Link(kIndexName)), name_,
                    BlockItemType::LinkItem);
  dest.emplace_back("Source SI", ToHexString(Link(kIndexSi)),
                    "Link to source information", BlockItemType::LinkItem);
  dest.emplace_back("Conversion CC", ToHexString(Link(kIndexCc)),
                    "Link to channel conversion", BlockItemType::LinkItem);
  dest.emplace_back("Signal Data", ToHexString(Link(kIndexData)),
                    "Link to signal data", BlockItemType::LinkItem);
  dest.emplace_back("Unit MD", ToHexString(Link(kIndexUnit)), Unit(),
                    BlockItemType::LinkItem);
  dest.emplace_back("Comment MD", ToHexString(Link(kIndexMd)), Comment(),
                    BlockItemType::LinkItem);
  for (size_t at = 0; at < nof_attachments_; ++at) {
    dest.emplace_back("Attachment AT", ToHexString(Link(kIndexAt + at)),
                      "Reference to attachment", BlockItemType::LinkItem);
  }
  if (Link(kIndexAt + nof_attachments_) > 0) {
    dest.emplace_back(
        "Reference DG", ToHexString(Link(kIndexAt + nof_attachments_)),
        "Reference to x-axis data block", BlockItemType::LinkItem);
    dest.emplace_back(
        "Reference CG", ToHexString(Link(kIndexAt + nof_attachments_ + 1)),
        "Reference to x-axis channel group", BlockItemType::LinkItem);
    dest.emplace_back("Reference CN",
                      ToHexString(Link(kIndexAt + nof_attachments_ + 2)),
                      "Reference to x-axis channel", BlockItemType::LinkItem);
  }
  dest.emplace_back("", "", "", BlockItemType::BlankItem);

  dest.emplace_back("Information", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Id", std::to_string(Index()));
  dest.emplace_back("Name", name_);
  dest.emplace_back("Description", Description());
  dest.emplace_back("Unit", Unit());
  dest.emplace_back("Channel Type", MakeTypeString(type_));
  dest.emplace_back("Synchronization Type", MakeSyncString(sync_type_));
  dest.emplace_back("Data Type", MakeDataTypeString(data_type_));
  dest.emplace_back("Bit Offset", std::to_string(bit_offset_));
  dest.emplace_back("Byte Offset", std::to_string(byte_offset_));
  dest.emplace_back("Bit Count", std::to_string(bit_count_));
  dest.emplace_back("Flags", MakeFlagString(flags_));
  dest.emplace_back("Invalid Bit Position", std::to_string(invalid_bit_pos_));
  dest.emplace_back("Decimals", std::to_string(precision_));
  dest.emplace_back("Nof Attachments", std::to_string(nof_attachments_));
  dest.emplace_back("Range Min", ToString(range_min_));
  dest.emplace_back("Range Max", ToString(range_max_));
  dest.emplace_back("Limit Min", ToString(limit_min_));
  dest.emplace_back("Limit Max", ToString(limit_max_));
  dest.emplace_back("Extended Limit Min", ToString(limit_ext_min_));
  dest.emplace_back("Extended Limit Max", ToString(limit_ext_max_));

  if (md_comment_) {
    md_comment_->GetBlockProperty(dest);
  }
}

uint64_t Cn4Block::Read(std::streambuf& buffer) {
  uint64_t bytes = ReadHeader4(buffer);
  bytes += ReadNumber(buffer, type_);
  bytes += ReadNumber(buffer, sync_type_);
  bytes += ReadNumber(buffer, data_type_);
  bytes += ReadNumber(buffer, bit_offset_);
  bytes += ReadNumber(buffer, byte_offset_);
  bytes += ReadNumber(buffer, bit_count_);
  bytes += ReadNumber(buffer, flags_);
  bytes += ReadNumber(buffer, invalid_bit_pos_);
  bytes += ReadNumber(buffer, precision_);
  std::vector<uint8_t> reserved;
  bytes += ReadByte(buffer, reserved, 1);
  bytes += ReadNumber(buffer, nof_attachments_);
  bytes += ReadNumber(buffer, range_min_);
  bytes += ReadNumber(buffer, range_max_);
  bytes += ReadNumber(buffer, limit_min_);
  bytes += ReadNumber(buffer, limit_max_);
  bytes += ReadNumber(buffer, limit_ext_min_);
  bytes += ReadNumber(buffer, limit_ext_max_);

  name_ = ReadTx4(buffer, kIndexName);

  if (Link(kIndexCx) > 0) {
    SetFilePosition(buffer, Link(kIndexCx));
    auto block_type = ReadBlockType(buffer);

    if (cx_list_.empty() && (Link(kIndexCx) > 0)) {
      for (auto link = Link(kIndexCx); link > 0; /* No ++ here*/) {
        if (block_type == "CA") {
          auto ca_block = std::make_unique<Ca4Block>();
          ca_block->Init(*this);
          ca_block->SetParentChannel(this);
          SetFilePosition(buffer, link);
          ca_block->Read(buffer);
          link = ca_block->Link(0);
          cx_list_.emplace_back(std::move(ca_block));
        } else if (block_type == "CN") {
          auto cn_block = std::make_unique<Cn4Block>();
          cn_block->Init(*this);
          SetFilePosition(buffer, link);
          cn_block->Read(buffer);
          link = cn_block->Link(0);
          cx_list_.emplace_back(std::move(cn_block));
        }
      }
    }
  }

  if (Link(kIndexSi) > 0) {
    SetFilePosition(buffer, Link(kIndexSi));
    si_block_ = std::make_unique<Si4Block>();
    si_block_->Init(*this);
    si_block_->Read(buffer);
  }

  if (Link(kIndexCc) > 0) {
    SetFilePosition(buffer, Link(kIndexCc));
    cc_block_ = std::make_unique<Cc4Block>();
    cc_block_->Init(*this);
    cc_block_->ChannelDataType(data_type_);
    cc_block_->Read(buffer);
  }

  // Need ot check if the data block is owned by this CN block or if it is a
  // reference only
  ReadBlockList(buffer, kIndexData);

  if (Link(kIndexUnit) > 0) {
    SetFilePosition(buffer, Link(kIndexUnit));
    unit_ = std::make_unique<Md4Block>();
    unit_->Init(*this);
    unit_->Read(buffer);
  }
  ReadMdComment(buffer, kIndexMd);

  if (nof_attachments_ > 0) {
    // Need the header block to find the AT blocks by its file position.
    const auto *header = HeaderBlock();
    const auto at_list = AtLinkList();

    // Get a list of attachment file positions and convert
    // these pointers to attachment block pointers.
    for (auto index : at_list) {
      if (header == nullptr) {
        continue;
      }
      const auto *block = header->Find(index);
      if (block == nullptr || block->BlockType() != "AT") {
        continue;
      }
      try {
        const auto *attachment = dynamic_cast<const IAttachment *>(block);
        if (attachment != nullptr) {
          attachment_list_.emplace_back(attachment);
        }
      } catch (const std::exception &) {
      }
    }
  }

  return bytes;
}

uint64_t Cn4Block::Write(std::streambuf& buffer) {
  const bool update = FilePosition() > 0;  // True if already written to file
  if (update) {
    return block_length_;
  }
  nof_attachments_ = static_cast<uint16_t>(attachment_list_.size());
  const auto default_x = (flags_ & CnFlag::DefaultX) != 0;

  block_type_ = "##CN";
  block_length_ = static_cast<uint64_t>(24 + 8 * 8);
  block_length_ += static_cast<uint64_t>(nof_attachments_) * 8;
  if (default_x) {
    block_length_ += 3 * 8;
  }
  block_length_ += 1 + 1 + 1 + 1 + 4 + 4 + 4 + 4 + 1 + 1 + 2 + (6 * 8);
  link_list_.resize(8 + nof_attachments_ + (default_x ? 1 : 0), 0);
  WriteLink4List(buffer, cx_list_, kIndexCx,
                 UpdateOption::DoNotUpdateWrittenBlock);
  WriteTx4(buffer, kIndexName, name_);
  WriteBlock4(buffer, si_block_, kIndexSi);
  WriteBlock4(buffer, cc_block_, kIndexCc);
  // The signal data shall not be stored here. Instead, the function
  // WriteSignalData() should be used. It is called by the Mdf(4)Writer class
  // when the measurement is finalized.
  WriteBlock4(buffer, unit_, kIndexUnit);
  WriteMdComment(buffer, kIndexMd);
  for (size_t index_at = 0; index_at < attachment_list_.size(); ++index_at) {
    const auto index = 8 + index_at;
    const auto *at4 = attachment_list_[index_at];
    link_list_[index] = at4 != nullptr ? at4->Index() : 0;
  }
  if (default_x) {
    const auto index = 8 + nof_attachments_;
    const auto *dg4 = default_x_.data_group;
    const auto *cg4 = default_x_.channel_group;
    const auto *cn4 = default_x_.channel;
    link_list_[index] = dg4 != nullptr ? dg4->Index() : 0;
    link_list_[index] = cg4 != nullptr ? cg4->Index() : 0;
    link_list_[index] = cn4 != nullptr ? cn4->Index() : 0;
  }

  uint64_t bytes = MdfBlock::Write(buffer);
  bytes += WriteNumber(buffer, type_);
  bytes += WriteNumber(buffer, sync_type_);
  bytes += WriteNumber(buffer, data_type_);
  bytes += WriteNumber(buffer, bit_offset_);
  bytes += WriteNumber(buffer, byte_offset_);
  bytes += WriteNumber(buffer, bit_count_);
  bytes += WriteNumber(buffer, flags_);
  bytes += WriteNumber(buffer, invalid_bit_pos_);
  bytes += WriteNumber(buffer, precision_);

  bytes += WriteBytes(buffer, 1);
  bytes += WriteNumber(buffer, nof_attachments_);
  bytes += WriteNumber(buffer, range_min_);
  bytes += WriteNumber(buffer, range_max_);
  bytes += WriteNumber(buffer, limit_min_);
  bytes += WriteNumber(buffer, limit_max_);
  bytes += WriteNumber(buffer, limit_ext_min_);
  bytes += WriteNumber(buffer, limit_ext_max_);
  UpdateBlockSize(buffer, bytes);
  return bytes;
}

MdfBlock *Cn4Block::Find(int64_t index) const {
  if (si_block_) {
    auto *p = si_block_->Find(index);
    if (p != nullptr) {
      return p;
    }
  }

  if (cc_block_) {
    auto *p = cc_block_->Find(index);
    if (p != nullptr) {
      return p;
    }
  }

  if (unit_) {
    auto *p = unit_->Find(index);
    if (p != nullptr) {
      return p;
    }
  }

  for (auto& cx : cx_list_) {
    if (!cx) {
      continue;
    }
    auto* p = cx->Find(index);
    if (p != nullptr) {
      return p;
    }
  }

  return DataListBlock::Find(index);
}

void Cn4Block::ReadSignalData(std::streambuf& buffer) const {
  const uint64_t count = DataSize();

  uint64_t index = 0;
  data_list_.resize(static_cast<size_t>(count), 0);
  // The block list should only contain one block.
  // A SD block is uncompressed data block
  for (const auto &block : block_list_) {
    const auto *data_list = dynamic_cast<const DataListBlock *>(block.get());
    const auto *data_block = dynamic_cast<const DataBlock *>(block.get());
    if (data_list != nullptr) {
      data_list->CopyDataToBuffer(buffer, data_list_, index);
    } else if (data_block != nullptr) {
      data_block->CopyDataToBuffer(buffer, data_list_, index);
    }
  }
}

void Cn4Block::BitCount(uint32_t bits) {
  bit_count_ = bits;
}

uint32_t Cn4Block::BitCount() const { return bit_count_; }

void Cn4Block::BitOffset(uint16_t bits) {
  bit_offset_ = static_cast<uint8_t>(bits);
}

uint16_t Cn4Block::BitOffset() const { return bit_offset_; }

void Cn4Block::ByteOffset(uint32_t bytes) {
  byte_offset_ = bytes;
}

uint32_t Cn4Block::ByteOffset() const { return byte_offset_; }

bool Cn4Block::GetTextValue(const std::vector<uint8_t> &record_buffer,
                            std::string &dest) const {
  auto offset = ByteOffset();
  size_t nof_bytes = BitCount() / 8;

  if (Type() == ChannelType::VariableLength && VlsdRecordId() > 0) {
    offset = 0;
    nof_bytes = record_buffer.size();
  }

  std::vector<uint8_t> temp;
  bool valid = true;
  dest.clear();
  if (Type() == ChannelType::VariableLength && VlsdRecordId() == 0) {
    // Index into the local data buffer
    uint64_t index = 0;
    valid = GetUnsignedValue(record_buffer, index);
    if (index + 4 > data_list_.size()) {
      return false;
    }
    const LittleBuffer<uint32_t> length(data_list_, static_cast<size_t>(index) );
    try {
      temp.resize(length.value(), 0);
    } catch (const std::exception&) {
      return false;
    }
    if (index + 4 + length.value() <= data_list_.size()) {
      memcpy(temp.data(), data_list_.data() + index + 4, length.value());
    } else {
      valid = false;
    }

  } else {
    try {
      temp.resize(nof_bytes);
    } catch(const std::exception&) {
      return false;
    }
    if (offset + nof_bytes <= record_buffer.size()) {
      memcpy(temp.data(), record_buffer.data() + offset, nof_bytes);
    } else {
      valid = false;
    }
  }

  switch (DataType()) {
    case ChannelDataType::StringAscii: {
      // Convert ASCII to UTF8
      std::ostringstream s;
      for (unsigned char ii : temp) {
        char in = static_cast<char>(ii);
        if (in == '\0') {
          break;
        }
        s << in;
      }
      try {
        dest = MdfHelper::Latin1ToUtf8(s.str());
      } catch (const std::exception &) {
        valid = false;  // Conversion error
        dest = s.str();
      }
      break;
    }

    case ChannelDataType::StringUTF8: {
      // No conversion needed
      std::ostringstream s;
      for (unsigned char ii : temp) {
        char in = static_cast<char>(ii);
        if (in == '\0') {
          break;
        }
        s << in;
      }
      dest = s.str();
      break;
    }

    case ChannelDataType::StringUTF16Le: {
      std::wostringstream s;
      for (size_t ii = 0; (ii + 2) <= temp.size(); ii += 2) {
        const LittleBuffer<uint16_t> data(temp, ii);

        if (data.value() == 0) {
          break;
        }
        s << static_cast<wchar_t>(data.value());
      }
      try {
        dest = MdfHelper::Utf16ToUtf8(s.str());
      } catch (const std::exception &) {
        valid = false;  // Conversion error
      }
      break;
    }

    case ChannelDataType::StringUTF16Be: {
      std::wostringstream s;
      for (size_t ii = 0; (ii + 2) <= temp.size(); ii += 2) {
        const BigBuffer<uint16_t> data(temp, ii);
        if (data.value() == 0) {
          break;
        }
        s << static_cast<wchar_t>(data.value());
      }
      try {
        dest = MdfHelper::Utf16ToUtf8(s.str());
      } catch (const std::exception &) {
        valid = false;  // Conversion error
      }
      break;
    }
    default:
      break;
  }
  return valid;
}

bool Cn4Block::GetByteArrayValue(const std::vector<uint8_t> &record_buffer,
                            std::vector<uint8_t> &dest) const {
  auto offset = ByteOffset();
  uint64_t nof_bytes = BitCount() / 8;
  bool valid = true;

  if (Type() == ChannelType::VariableLength && VlsdRecordId() > 0) {
    // If this is a VLSD channel and its data link points to a CG block, that
    // CG block holds no signals. The VLSD CG block record are VLSD. If the data
    // link pointed on an SD block, the that block holds the variable length
    // data. The destination
    offset = 0;
    nof_bytes = record_buffer.size();
  } else if (Type() == ChannelType::MaxLength && mlsd_channel_ != nullptr ) {
    // Number of bytes is on another channel
    valid = mlsd_channel_->GetUnsignedValue(record_buffer, nof_bytes);
  }

  if (Type() == ChannelType::VariableLength && VlsdRecordId() == 0) {
    // Index into the local data buffer. Variable length record is stored
    // in a SD block.
    uint64_t index = 0;  // Actual offset into the data block.
    valid = GetUnsignedValue(record_buffer, index);
    if (index + 4 > data_list_.size()) {
      return false;
    }
    const LittleBuffer<uint32_t> length(data_list_, static_cast<size_t>(index) );
    try {
      dest.resize(length.value(), 0);
    } catch (const std::exception&) {
      return false; // Return invalid value
    }
    if (index + 4 + length.value() <= data_list_.size()) {
      memcpy(dest.data(), data_list_.data() + index + 4, length.value());
    } else {
      valid = false;
    }

  } else {
    try {
      dest.resize(static_cast<size_t>(nof_bytes), 0);
    } catch (const std::exception&) {
      return false; // Return invalid value
    }
    if (offset + nof_bytes <= record_buffer.size()) {
      memcpy(dest.data(), record_buffer.data() + offset, static_cast<size_t>(nof_bytes) );
    } else {
      valid = false;
    }
  }

  return valid;
}

void Cn4Block::SetTextValue(const std::string &value, bool valid) {
  if (Type() == ChannelType::VariableLength) {
    SetValid(valid,0);
    // String stored in signal data. Index should be stored in the record
    // and the string in a temporary data block (data_list_). This block is
    // later stored into an SD/DZ block.
    uint64_t index;
    VlsdData temp(value);
    auto itr = data_map_.find(temp);
    if (itr != data_map_.cend()) {
      index = itr->second;
    } else {
      index = static_cast<uint64_t>(data_list_.size());
      LittleBuffer<uint32_t> length(static_cast<uint32_t>(temp.Size()));
      data_list_.insert(data_list_.end(),length.cbegin(), length.cend());
      data_list_.insert(data_list_.end(),temp.Cbegin(), temp.Cend());
    }
    // Store the index in the record
    if (bit_count_ == 0) {
      SetValid(false,0);
    } else {
      SetUnsignedValueLe(index, true,0);
    }
  } else {
    // Fixed length handle by interface class
    IChannel::SetTextValue(value, valid);
  }
}

void Cn4Block::SetByteArray(const std::vector<uint8_t> &value, bool valid) {
  SetValid(valid,0);
  if (Type() == ChannelType::VariableLength) {
    // String stored in signal data. Index should be stored in the record
    // and the string in a temporary data block (data_list_). This block is
    // later stored into an SD/DZ block.
    uint64_t index;
    VlsdData temp(value);
    auto itr = data_map_.find(temp);
    if (itr != data_map_.cend()) {
      index = itr->second;
    } else {
      index = static_cast<uint64_t>(data_list_.size());
      LittleBuffer<uint32_t> length(static_cast<uint32_t>(temp.Size()));
      data_list_.insert(data_list_.end(),length.cbegin(), length.cend());
      data_list_.insert(data_list_.end(),temp.Cbegin(), temp.Cend());
    }
    // Store the index in the record
    if (bit_count_ == 0) {
      SetValid(false,0);
    } else {
      SetUnsignedValueLe(index, true,0);
    }
  } else {
    // Fixed length handle by interface class
    IChannel::SetByteArray(value, valid);
  }
}

void Cn4Block::Unit(const std::string &unit) {
  if (unit.empty()) {
    unit_.reset();
  } else {
    CnUnit cn_unit;
    cn_unit.Comment(MdString(unit));
    unit_ = std::make_unique<Md4Block>(cn_unit.ToXml());
  }
}

void Cn4Block::Type(ChannelType type) {
  type_ = static_cast<uint8_t>(type);
}

void Cn4Block::DataType(ChannelDataType type) {
  data_type_ = static_cast<uint8_t>(type);
  switch(DataType()) {
    case ChannelDataType::UnsignedIntegerLe:
    case ChannelDataType::UnsignedIntegerBe:
    case ChannelDataType::SignedIntegerLe:
    case ChannelDataType::SignedIntegerBe:
    case ChannelDataType::FloatLe:
    case ChannelDataType::FloatBe:
      if (DataBytes() == 0) {
        DataBytes(4);
      }
      break;

    case ChannelDataType::CanOpenDate:
      DataBytes(7);
      break;

    case ChannelDataType::CanOpenTime:
      DataBytes(6);
      break;

    default:
      break;
  }
}

void Cn4Block::DataBytes(uint64_t nof_bytes) {
  bit_count_ = static_cast<uint32_t>(nof_bytes * 8);
}

void Cn4Block::SamplingRate(double sampling_rate) {}
double Cn4Block::SamplingRate() const { return 0; }

std::vector<uint8_t> &Cn4Block::SampleBuffer() const {
  return cg_block_->SampleBuffer();
}
void Cn4Block::Init(const MdfBlock &id_block) {
  MdfBlock::Init(id_block);
  if (id_block.BlockType() == "CG") {
    cg_block_ = dynamic_cast<const Cg4Block *>(&id_block);
  } else if (id_block.BlockType() == "CN") {
    auto* channel = dynamic_cast<const Cn4Block *>(&id_block);
    // Handle composite channels
    cg_block_ = channel != nullptr ? channel->cg_block_ : nullptr;
  }
}

void Cn4Block::AddCc4(std::unique_ptr<Cc4Block> &cc4) {
  cc_block_ = std::move(cc4);
}

void Cn4Block::Sync(ChannelSyncType type) {
  sync_type_ = static_cast<uint8_t>(type);
}

ChannelSyncType Cn4Block::Sync() const {
  return static_cast<ChannelSyncType>(sync_type_);
}

void Cn4Block::Range(double min, double max) {
  range_min_ = min;
  range_max_ = max;
  flags_ |= CnFlag::RangeValid;
}

std::optional<std::pair<double, double>> Cn4Block::Range() const {
  return (flags_ & CnFlag::RangeValid) != 0
             ? std::optional(std::pair(range_min_, range_max_))
             : IChannel::Range();
}

void Cn4Block::Limit(double min, double max) {
  limit_min_ = min;
  limit_max_ = max;
  flags_ |= CnFlag::LimitValid;
}

std::optional<std::pair<double, double>> Cn4Block::Limit() const {
  return (flags_ & CnFlag::LimitValid) != 0
             ? std::optional(std::pair(limit_min_, limit_max_))
             : IChannel::Limit();
}

void Cn4Block::ExtLimit(double min, double max) {
  limit_ext_min_ = min;
  limit_ext_max_ = max;
  flags_ |= CnFlag::ExtendedLimitValid;
}

std::optional<std::pair<double, double>> Cn4Block::ExtLimit() const {
  return (flags_ & CnFlag::ExtendedLimitValid) != 0
             ? std::optional(std::pair(limit_ext_min_, limit_ext_max_))
             : IChannel::Limit();
}

ISourceInformation *Cn4Block::SourceInformation() const {
  return si_block_.get();
}

ISourceInformation *Cn4Block::CreateSourceInformation() {
  if (!si_block_) {
    si_block_ = std::make_unique<Si4Block>();
    si_block_->Init(*this);
  }
  return si_block_.get();
}

IChannelConversion *Cn4Block::CreateChannelConversion() {
  if (!cc_block_) {
    cc_block_ = std::make_unique<Cc4Block>();
    cc_block_->Init(*this);
  }
  cc_block_->ChannelDataType(data_type_);
  return cc_block_.get();
}

IChannel *Cn4Block::CreateChannelComposition() {
  auto block = std::make_unique<Cn4Block>();
  block->Init(*this);
  cx_list_.push_back(std::move(block));
  return dynamic_cast<IChannel*>(cx_list_.back().get());
}

std::vector<IChannel *> Cn4Block::ChannelCompositions() {
  std::vector<IChannel*> list;
  for ( const auto& itr : cx_list_) {
    try {
      // The cx_list may have CA or CN channels, so we need to test for this.
      auto* channel = dynamic_cast<IChannel*>(itr.get());
      if (channel != nullptr) {
        list.emplace_back(channel);
      }
    } catch (const std::exception&) {
    }
  }
  return list;
}

void Cn4Block::Flags(uint32_t flags) {flags_ = flags; }

uint32_t Cn4Block::Flags() const { return flags_; }

void Cn4Block::PrepareForWriting(uint64_t offset) {
  bit_offset_ = 0;
  byte_offset_ = static_cast<uint32_t>(offset);
  data_list_.clear(); // Temporary storage of signal data (SD)
  data_list_.shrink_to_fit();
  data_map_.clear();

  // The bit count may be set by the user.
  switch (Type()) {
    case ChannelType::Master:
      // Master channel cannot have invalid values.
      flags_ &= ~CnFlag::InvalidValid;
      break;

      // Fixed length of the byte array but another channel holds the array size.
      // This a so-called MLSD channel.
    case ChannelType::MaxLength:
    case ChannelType::Sync:
    case ChannelType::FixedLength:
      // Length fixed below or by user
      break;

      // Variable length channels, store its value in an external
      // signal data (SD) block of in the next channel group (VLSD CG).
      // The VLSD CG block should be the next CG block and cannot have
      // any channels. This channel stores a byte offset (index) into
      // the SD or VLSD block.
    case ChannelType::VariableLength:
      // Store unsigned 32/64-bit index to variable block
      if (bit_count_ == 0) {
        bit_count_ = 8 * 8; // Assume 64 bit index
      }
      return;

    case ChannelType::VirtualData:
    case ChannelType::VirtualMaster:
      // Sample index * CC sets the channel value. No valid bits
      bit_count_ = 0;
      flags_ &= ~CnFlag::InvalidValid;
      return;
  }

    // Note that in case the channel is an array, the channel
    // size describe one channel value.
  switch (DataType()) {
    case ChannelDataType::UnsignedIntegerLe:
    case ChannelDataType::UnsignedIntegerBe:
    case ChannelDataType::SignedIntegerLe:
    case ChannelDataType::SignedIntegerBe:
      if (bit_count_ == 0) {
        bit_count_ = 64; // Assume 64-bit size
      } else {
        switch (bit_count_) {
          case 8:
          case 16:
          case 32:
          case 64:
            break;

          default: // No supported sizes
            bit_count_ = 64;
            break;
        }
      }
      break;

    case ChannelDataType::FloatLe:
    case ChannelDataType::FloatBe:
      if (bit_count_ == 0) {
        bit_count_ = 64; // Assume double
      } else {
        switch (bit_count_) {
          case 32:
          case 64:
            break;

          default: // No supported sizes
            bit_count_ = 64;
            break;
        }
      }
      break;

    case ChannelDataType::StringAscii:
    case ChannelDataType::StringUTF8:
    case ChannelDataType::StringUTF16Le:
    case ChannelDataType::StringUTF16Be:
    case ChannelDataType::ByteArray:
    case ChannelDataType::MimeSample:
    case ChannelDataType::MimeStream:
      // Typical is the data stored as an index to a
      // signal data block.
      if (bit_count_ == 0) {
        Type(ChannelType::VariableLength);
        bit_count_ = 8 * 8; // 64-bit index
      }
      break;

    case ChannelDataType::CanOpenDate:
      bit_count_ = 7 * 8;
      break;

    case ChannelDataType::CanOpenTime:
      bit_count_ = 6 * 8;
      break;

    case ChannelDataType::ComplexLe:
    case ChannelDataType::ComplexBe:
      if (bit_count_ == 0) {
        bit_count_ = 128; // Assume 2 double
      } else {
        switch (bit_count_) {
          case 64: // 2 float
          case 128:// 2 double
            break;

          default: // No supported sizes
            bit_count_ = 128; // 2 double
            break;
        }
      }
      break;

    default:
      if (DataBytes() == 0) {
        DataBytes(4);
      }
      break;
  }

  for (auto* array: ChannelArrays()) {
    if (auto* ca4 = dynamic_cast<Ca4Block*>(array); ca4 != nullptr) {
      ca4->PrepareForWriting();
    }
  }
}

void Cn4Block::SetValid(bool valid, uint64_t array_index) {
  if ((Flags() & CnFlag::InvalidValid) != 0 && cg_block_ != nullptr) {
    auto& buffer = SampleBuffer();
    const uint32_t invalid_pos = invalid_bit_pos_ +
                                 static_cast<uint32_t>(array_index);
    const uint32_t byte_offset = cg_block_->NofDataBytes() + (invalid_pos / 8);
    const uint32_t bit_offset = invalid_pos % 8;
    const uint8_t mask = 0x01 << bit_offset;
    if (byte_offset < buffer.size()) {
      if (valid) {
        buffer[byte_offset] &= ~mask;
      } else {
        buffer[byte_offset] |= mask;
      }
    }
  }
}

bool Cn4Block::GetValid(const std::vector<uint8_t> &record_buffer,
                        uint64_t array_index) const {
  bool valid = true;
  if (Flags() & CnFlag::InvalidValid && cg_block_ != nullptr) {
    const auto invalid_pos = invalid_bit_pos_ + array_index;
    const auto byte_offset = static_cast<size_t>( cg_block_->NofDataBytes() + (invalid_pos / 8) );
    const auto bit_offset = invalid_pos % 8;
    const uint8_t mask = 0x01 << bit_offset;
    if (byte_offset < record_buffer.size()) {
      valid = (record_buffer[byte_offset] & mask) == 0;
    }
  }
  return valid;
}

uint64_t Cn4Block::WriteSignalData(std::streambuf& buffer, bool compress) {
  uint64_t bytes = 0;

  if (Type() == ChannelType::MaxLength ||
      (Type() == ChannelType::VariableLength && VlsdRecordId() > 0) ) {
    // Signal data is updated elsewhere
    return 0;
  }

  if (data_list_.empty()) {
    UpdateLink(buffer, kIndexData, 0); // No data link
  } else if (!compress || data_list_.size() <= 100 ){
    // Store as SD block
    auto sd4 = std::make_unique<Sd4Block>();
    sd4->Data(data_list_);
    bytes = sd4->Write(buffer);
    UpdateLink(buffer, kIndexData,sd4->FilePosition());
  } else if (data_list_.size() <= kMaxDataSize) { // Compress data
    // Store DZ (SD) block
    auto dz4 = std::make_unique<Dz4Block>();
    dz4->OrigBlockType("SD");
    dz4->Type(Dz4ZipType::Deflate);
    dz4->Data(data_list_); // Compress and set up the sizes
    bytes = dz4->Write(buffer);
    UpdateLink(buffer, kIndexData,dz4->FilePosition());
  } else {
    // Store HL + DL + DZ blocks
    auto hl4 = std::make_unique<Hl4Block>();
    hl4->Flags(Hl4Flags::EqualLength);
    hl4->Type(Hl4ZipType::Deflate);

    auto dl4 = std::make_unique<Dl4Block>();
    std::vector<uint8_t> data;
    data.reserve(kMaxDataSize);
    dl4->Flags(Dl4Flags::EqualLength);
    dl4->EqualLength(kMaxDataSize);

    for (const auto in_byte : data_list_) {
      data.push_back(in_byte);
      if (data.size() + 1 > kMaxDataSize) {
        auto dz4 = std::make_unique<Dz4Block>();
        dz4->OrigBlockType("SD");
        dz4->Type(Dz4ZipType::Deflate);
        dz4->Data(data); // Compress and set up the sizes

        auto& block_list = dl4->DataBlockList();
        block_list.push_back(std::move(dz4));

        data.clear();
        data.shrink_to_fit();
        data.reserve(kMaxDataSize);
      }
    }
    if (!data.empty()) {
      auto dz4 = std::make_unique<Dz4Block>();
      dz4->OrigBlockType("SD");
      dz4->Type(Dz4ZipType::Deflate);
      dz4->Data(data); // Compress and set up the sizes

      auto& block_list = dl4->DataBlockList();
      block_list.push_back(std::move(dz4));
    }
    auto& top_list = hl4->DataBlockList();
    top_list.push_back(std::move(dl4));

    bytes = hl4->Write(buffer);
    UpdateLink(buffer, kIndexData,hl4->FilePosition());
  }
  return bytes;
}

void Cn4Block::ClearData() {
  data_list_.clear();
  data_list_.shrink_to_fit();
  data_map_.clear();
  DataListBlock::ClearData();
}

uint64_t Cn4Block::WriteSdSample(const std::vector<uint8_t> &buffer) const  {
  // Save old index as this is the index which new data bytes are inserted
  auto index = static_cast<uint64_t>(data_list_.size());

  // 32-bits length
  const LittleBuffer<uint32_t> length(static_cast<uint32_t>(buffer.size()));
  data_list_.insert(data_list_.end(),length.cbegin(), length.cend());
  // Now add data bytes
  data_list_.insert(data_list_.end(),buffer.cbegin(), buffer.cend());
  return index;
}

IChannelArray *Cn4Block::ChannelArray(size_t index) const {
  if (index >= cx_list_.size()) {
    return nullptr;
  }
  if (auto& mdf_block = cx_list_[index]; mdf_block ) {
    return dynamic_cast<IChannelArray*>(mdf_block.get());
  }
  return nullptr;
}

std::vector<IChannelArray*> Cn4Block::ChannelArrays() const {
  std::vector<IChannelArray*> arrays;
  for (const auto& block:  cx_list_) {
    if (block && block->BlockType() == "CA") {
      arrays.push_back(dynamic_cast<IChannelArray*>(block.get()));
    }
  }
  return arrays;
}

IChannelArray *Cn4Block::CreateChannelArray() {
  auto temp = std::make_unique<Ca4Block>();
  temp->SetParentChannel(this);
  cx_list_.emplace_back(std::move(temp));
   return dynamic_cast<IChannelArray*>(cx_list_.back().get());
}

const IChannelGroup* Cn4Block::ChannelGroup() const {
  return dynamic_cast<const IChannelGroup*> (CgBlock());
}

void Cn4Block::AddAttachmentReference(const IAttachment *attachment) {
  if (attachment == nullptr) {
    return;
  }
  const auto exists = std::find_if(
      attachment_list_.cbegin(),attachment_list_.cend(),
      [&] (const IAttachment* attach) {
          return attach != nullptr && attach == attachment;
       });
  if (exists == attachment_list_.cend()) {
    attachment_list_.emplace_back(attachment);
  }
  nof_attachments_ = static_cast<uint16_t>(attachment_list_.size());
}

std::vector<const IAttachment *> Cn4Block::AttachmentList() const {
  return attachment_list_;
}

void Cn4Block::SetCnUnit(const CnUnit &unit) {
  unit_ = std::make_unique<Md4Block>(unit.ToXml());
}

void Cn4Block::GetCnUnit(CnUnit& unit) const {
  if (unit_) {
    if (const std::string xml_snippet = unit_->XmlSnippet();
        !xml_snippet.empty()) {
      unit.FromXml(xml_snippet);
    }
  }
}

}  // namespace mdf::detail