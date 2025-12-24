/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "cc4block.h"

#include <sstream>
#include <string>
namespace {
constexpr size_t kIndexName = 0;
constexpr size_t kIndexUnit = 1;
constexpr size_t kIndexMd = 2;
constexpr size_t kIndexInverse = 3;
constexpr size_t kIndexRef = 4;

std::string MakeTypeString(uint8_t type) {
  switch (type) {
    case 0:
      return "1:1";
    case 1:
      return "Linear";
    case 2:
      return "Rational";
    case 3:
      return "Algebraic";
    case 4:
      return "Value to Value, Interpolation";
    case 5:
      return "Value to Value";
    case 6:
      return "Value Range to Value";
    case 7:
      return "Value to Text";
    case 8:
      return "Value Range to Text";
    case 9:
      return "Text to Value";
    case 10:
      return "Text to Text";
    case 11:
      return "Bitfield to Text";
    default:
      break;
  }
  return "Unknown";
}

std::string MakeFlagString(uint16_t flag) {
  std::ostringstream s;
  if (flag & 0x0001) {
    s << "Decimal";
  }
  if (flag & 0x0002) {
    s << (s.str().empty() ? "Range" : ",Range");
  }
  if (flag & 0x0004) {
    s << (s.str().empty() ? "Status" : ",Status");
  }
  return s.str();
}
}  // namespace

namespace mdf::detail {

Cc4Block::Cc4Block() { block_type_ = "##CC"; }

int64_t Cc4Block::Index() const { return FilePosition(); }

void Cc4Block::Name(const std::string& name) { name_ = name; }

std::string Cc4Block::Name() const { return name_; }

void Cc4Block::Unit(const std::string& unit) {
  CcUnit cc_unit;
  cc_unit.Comment(MdString(unit));
  unit_ = std::make_unique<Md4Block>(cc_unit.ToXml());
}

std::string Cc4Block::Unit() const {
  if (!unit_) {
    return {};
  }
  if (unit_->IsTxtBlock()) {
    return unit_->Text();
  }
  return unit_->TxComment();
}

void Cc4Block::Description(const std::string& desc) {
  auto* metadata = CreateMetaData();
  if (metadata != nullptr) {
    metadata->StringProperty("TX", desc);
  }
}

std::string Cc4Block::Description() const { return Comment(); }

void Cc4Block::Type(ConversionType type) { type_ = static_cast<uint8_t>(type); }

ConversionType Cc4Block::Type() const {
  return static_cast<ConversionType>(type_);
}

void Cc4Block::Decimals(uint8_t decimals) {
  flags_ |= CcFlag::PrecisionValid;
  precision_ = decimals;
}

uint8_t Cc4Block::Decimals() const {
  const auto max = static_cast<uint8_t>(
                              std::numeric_limits<double>::max_digits10);
  return  flags_ & CcFlag::PrecisionValid ?
                             precision_ : max;
}

bool Cc4Block::IsUnitValid() const { return Link(kIndexUnit) != 0; }

bool Cc4Block::IsDecimalUsed() const {
  return (flags_ & CcFlag::PrecisionValid) != 0;
}

void Cc4Block::Range(double min, double max) {
  if (max > min) {
    flags_ |= CcFlag::RangeValid;
  }
  range_min_ = min;
  range_max_ = max;
}

std::optional<std::pair<double, double>> Cc4Block::Range() const {
  return (flags_ & CcFlag::RangeValid) != 0
             ? std::optional(std::pair(range_min_, range_max_))
             : std::optional<std::pair<double, double>>();
}

uint16_t Cc4Block::Flags() const { return flags_; }
void Cc4Block::Flags(uint16_t flags) { flags_ = flags; }

IChannelConversion* Cc4Block::CreateInverse() {
  cc_block_= std::make_unique<Cc4Block>();
  cc_block_->Init(*this);
  return cc_block_.get();
}

IChannelConversion* Cc4Block::Inverse() const {
  return cc_block_ ? cc_block_.get() : nullptr;
}

void Cc4Block::GetBlockProperty(BlockPropertyList& dest) const {
  MdfBlock::GetBlockProperty(dest);

  dest.emplace_back("Links", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Name TX", ToHexString(Link(kIndexName)), name_,
                    BlockItemType::LinkItem);

  dest.emplace_back("Unit TX/MD", ToHexString(Link(kIndexUnit)),
                    unit_ ? unit_->Comment() : std::string("<Invalid>"),
                    BlockItemType::LinkItem);
  dest.emplace_back("Comment MD", ToHexString(Link(kIndexMd)), Comment(),
                    BlockItemType::LinkItem);
  dest.emplace_back("Inverse CC", ToHexString(Link(kIndexInverse)),
                    "Link to inverse formula", BlockItemType::LinkItem);
  for (const auto& block : ref_list_) {
    if (!block) {

      dest.emplace_back("Reference Link NIL", "0x00", "", BlockItemType::LinkItem);
    } else if (block->BlockType() == "TX") {
      const auto* tx = dynamic_cast<const Tx4Block*>(block.get());
      dest.emplace_back("Reference Link TX", ToHexString(block->FilePosition()),
                        tx != nullptr ? tx->Text() : "",
                        BlockItemType::LinkItem);
    } else if (block->BlockType() == "CC") {
      const auto* cc = dynamic_cast<const Cc4Block*>(block.get());
      dest.emplace_back("Reference Link CC", ToHexString(block->FilePosition()),
                        cc != nullptr ? cc->Name() : "",
                        BlockItemType::LinkItem);
    }
  }
  dest.emplace_back("", "", "", BlockItemType::BlankItem);

  dest.emplace_back("Information", "", "", BlockItemType::HeaderItem);
  if (Link(kIndexName) > 0) {
    dest.emplace_back("Name", name_);
  }
  if (Link(kIndexUnit) > 0) {
    dest.emplace_back("Unit", Unit());
  }
  if (precision_ < 0xFF) {
    dest.emplace_back("Nof Decimals", std::to_string(precision_));
  }
  dest.emplace_back("Conversion Type", MakeTypeString(type_));
  dest.emplace_back("Flags", MakeFlagString(flags_));
  dest.emplace_back("Nof References", std::to_string(nof_references_));
  dest.emplace_back("Nof Values", std::to_string(nof_values_));
  dest.emplace_back("Min Range", ToString(range_min_));
  dest.emplace_back("Max Range", ToString(range_max_));
  for (uint16_t ii = 0; ii < static_cast<uint16_t>(value_list_.size()); ++ii) {
    std::ostringstream label;
    label << "Value " << ii;
    dest.emplace_back(label.str(), ToString(Parameter(ii)) );
  }
  if (md_comment_) {
    md_comment_->GetBlockProperty(dest);
  }
}

uint64_t Cc4Block::Read(std::streambuf& buffer) {  // NOLINT
  uint64_t bytes = ReadHeader4(buffer);
  bytes += ReadNumber(buffer, type_);
  bytes += ReadNumber(buffer, precision_);
  bytes += ReadNumber(buffer, flags_);
  bytes += ReadNumber(buffer, nof_references_);
  bytes += ReadNumber(buffer, nof_values_);
  bytes += ReadNumber(buffer, range_min_);
  bytes += ReadNumber(buffer, range_max_);

  value_list_.clear();
  value_list_.shrink_to_fit();

  for (uint16_t ii = 0; ii < nof_values_; ++ii) {
    if (Type() == ConversionType::BitfieldToText) {
      uint64_t temp = 0;
      bytes += ReadNumber(buffer, temp);
      value_list_.emplace_back(temp);
    } else {
      double temp = 0;
      bytes += ReadNumber(buffer, temp);
      value_list_.emplace_back(temp);
    }
  }

  name_ = ReadTx4(buffer, kIndexName);
  if (Link(kIndexUnit) > 0) {
    SetFilePosition(buffer, Link(kIndexUnit));
    unit_ = std::make_unique<Md4Block>();
    unit_->Init(*this);
    unit_->Read(buffer);
  }
  ReadMdComment(buffer, kIndexMd);

  if (Link(kIndexInverse) > 0) {
    SetFilePosition(buffer, Link(kIndexInverse));
    cc_block_ = std::make_unique<Cc4Block>();
    cc_block_->Init(*this);
    cc_block_->Read(buffer);
  }
  if (ref_list_.empty() && nof_references_ > 0) {
    for (uint16_t ii = 0; ii < nof_references_; ++ii) {
      if (Link(kIndexRef + ii) <= 0) {
        ref_list_.emplace_back(std::unique_ptr<MdfBlock>());
        continue;
      }
      SetFilePosition(buffer, Link(kIndexRef + ii));
      const std::string block_type = ReadBlockType(buffer);

      SetFilePosition(buffer, Link(kIndexRef + ii));
      if (block_type == "TX") {
        auto tx = std::make_unique<Tx4Block>();
        tx->Init(*this);
        tx->Read(buffer);
        ref_list_.emplace_back(std::move(tx));
      } else if (block_type == "CC") {
        auto cc = std::make_unique<Cc4Block>();
        cc->Init(*this);
        cc->ChannelDataType(channel_data_type_);
        cc->Read(buffer);
        ref_list_.emplace_back(std::move(cc));
      } else {
        ref_list_.emplace_back(std::unique_ptr<MdfBlock>());
      }
    }
  }
  return bytes;
}

uint64_t Cc4Block::Write(std::streambuf& buffer) {  // NOLINT
  const bool update = FilePosition() > 0;  // True if already written to file
  if (update) {
    return static_cast<size_t>(block_length_);
  }

  nof_references_ = static_cast<uint16_t>(ref_list_.size());
  nof_values_ = static_cast<uint16_t>(value_list_.size());

  block_type_ = "##CC";
  block_length_ = 24 + (4 * 8) + (nof_references_ * 8) + 1 + 1 + 2 + 2 + 2 + 8 +
                  8 + (8 * nof_values_);
  link_list_.resize(4 + nof_references_, 0);
  WriteTx4(buffer, kIndexName, name_);
  WriteBlock4(buffer, unit_, kIndexUnit);
  WriteMdComment(buffer, kIndexMd);
  WriteBlock4(buffer, cc_block_, kIndexInverse);
  for (uint16_t index_m = 0; index_m < nof_references_; ++index_m) {
    const auto index = kIndexRef + index_m;
    auto& block = ref_list_[index_m];
    WriteBlock4(buffer, block, index);
  }

  uint64_t bytes = MdfBlock::Write(buffer);
  bytes += WriteNumber(buffer, type_);
  bytes += WriteNumber(buffer, precision_);
  bytes += WriteNumber(buffer, flags_);
  bytes += WriteNumber(buffer, nof_references_);
  bytes += WriteNumber(buffer, nof_values_);
  bytes += WriteNumber(buffer, range_min_);
  bytes += WriteNumber(buffer, range_max_);
  for (uint16_t index_n = 0; index_n < nof_values_; ++index_n) {
    const auto& val = value_list_[index_n];
    if (std::holds_alternative<double>(val)) {
      bytes += WriteNumber(buffer, std::get<double>(val));
    } else if (std::holds_alternative<uint64_t>(val)) {
      bytes += WriteNumber(buffer, std::get<uint64_t>(val));
    } else {
      bytes += WriteNumber(buffer, 0.0);
    }
  }
  UpdateBlockSize(buffer, bytes);
  return bytes;
}

MdfBlock* Cc4Block::Find(int64_t index) const {  // NOLINT
  if (cc_block_) {
    auto* p = cc_block_->Find(index);
    if (p != nullptr) {
      return p;
    }
  }
  if (unit_) {
    auto* p = unit_->Find(index);
    if (p != nullptr) {
      return p;
    }
  }
  for (const auto& ref : ref_list_) {
    if (ref) {
      auto* p = ref->Find(index);
      if (p != nullptr) {
        return p;
      }
    }
  }
  return MdfBlock::Find(index);
}

bool Cc4Block::ConvertValueToText(double channel_value,
                                  std::string& eng_value) const {
  if (ref_list_.empty()) {
    return false;
  }
  // First iterate to find the ref
  size_t ref_index = ref_list_.size() - 1;  // Default CC/TX
  for (uint16_t n = 0; n < nof_values_; ++n) {
    if (n >= value_list_.size()) {
      break;
    }
    const auto key = Parameter(n);
    if (channel_value == key) {
      ref_index = n;
      break;
    }
  }
  if (ref_index >= ref_list_.size()) {
    return false;
  }
  const auto& block = ref_list_[ref_index];
  if (!block) {
    eng_value.clear();
    eng_value.shrink_to_fit();
  } else if (block->BlockType() == "TX") {
    const auto* tx = dynamic_cast<const Tx4Block*>(block.get());
    if (tx == nullptr) {
      return false;
    }
    eng_value = tx->Text();
  } else if (block->BlockType() == "CC") {
    const auto* cc = dynamic_cast<const IChannelConversion*>(block.get());
    if (cc == nullptr) {
      return false;
    }
    return cc->Convert(channel_value, eng_value);
  } else {
    return false;
  }
  return true;
}

bool Cc4Block::ConvertValueRangeToText(double channel_value,
                                       std::string& eng_value) const {
  if (ref_list_.empty()) {
    return false;
  }

  // First iterate to find the ref
  size_t ref_index = ref_list_.size() - 1;  // Default CC/TX
  for (uint16_t n = 0; n < nof_values_; ++n) {
    const uint16_t key_min_index = n * 2;
    const uint16_t key_max_index = key_min_index + 1;
    if (key_max_index >= value_list_.size()) {
      break;
    }
    const auto key_min = Parameter(key_min_index);
    const auto key_max = Parameter(key_max_index);
    if (IsChannelInteger() && channel_value >= key_min &&
        channel_value <= key_max) {
      ref_index = n;
      break;
    }

    if (IsChannelFloat() && channel_value >= key_min &&
        channel_value < key_max) {
      ref_index = n;
      break;
    }
  }
  if (ref_index >= ref_list_.size()) {
    return false;
  }
  const auto& block = ref_list_[ref_index];
  if (!block) {
    eng_value.clear();
    eng_value.shrink_to_fit();
  } else if (block->BlockType() == "TX") {
    const auto* tx = dynamic_cast<const Tx4Block*>(block.get());
    if (tx == nullptr) {
      return false;
    }
    eng_value = tx->Text();
  } else if (block->BlockType() == "CC") {
    const auto* cc = dynamic_cast<const IChannelConversion*>(block.get());
    if (cc == nullptr) {
      return false;
    }
    return cc->Convert(channel_value, eng_value);
  } else {
    return false;
  }
  return true;
}

bool Cc4Block::ConvertTextToValue(const std::string& channel_value,
                                  double& eng_value) const {
  if (value_list_.empty()) {
    return false;
  }
  // Initialize to default value index
  auto value_index = static_cast<uint16_t>(value_list_.size() - 1);  
  for (uint16_t n = 0; n < nof_values_; ++n) {
    if (n >= ref_list_.size()) {
      break;
    }
    const auto& block = ref_list_[n];
    if (!block) {
      return false;
    }
    if (block->BlockType() != "TX") {
      return false;
    }
    const auto* tx = dynamic_cast<const Tx4Block*>(block.get());
    if (tx == nullptr) {
      return false;
    }
    if (tx->Text() == channel_value) {
      value_index = n;
      break;
    }
  }
  if (value_index >= value_list_.size()) {
    return false;
  }
  eng_value = Parameter(value_index);
  return true;
}

bool Cc4Block::ConvertTextToTranslation(const std::string& channel_value,
                                        std::string& eng_value) const {
  if (ref_list_.empty()) {
    return false;
  }
  size_t value_index = ref_list_.size() - 1;  // Default value
  for (size_t index = 0; index + 1 < ref_list_.size(); index += 2) {
    const auto& txt = ref_list_[index];
    if (!txt) {
      return false;
    }
    if (txt->BlockType() != "TX") {
      return false;
    }
    const auto* tx4 = dynamic_cast<const Tx4Block*>(txt.get());
    if (tx4 == nullptr) {
      return false;
    }
    if (tx4->Text() == channel_value) {
      value_index = index + 1;
      break;
    }
  }
  {
    if (value_index >= ref_list_.size()) {
      return false;
    }
    const auto& txt = ref_list_[value_index];
    if (!txt) {
      return false;
    }
    if (txt->BlockType() != "TX") {
      return false;
    }
    const auto* tx4 = dynamic_cast<const Tx4Block*>(txt.get());
    if (tx4 == nullptr) {
      return false;
    }
    eng_value = tx4->Text();
  }

  return true;
}

void Cc4Block::Formula(const std::string& formula) {
  // The formula should be place in ref_list[0].
  constexpr size_t index = 0;
  while (index >= ref_list_.size()) {
    auto temp = std::make_unique<Tx4Block>();
    ref_list_.emplace_back(std::move(temp));
  }
  auto* tx4 = dynamic_cast<Tx4Block*>(ref_list_[0].get());
  if (tx4 != nullptr) {
    tx4->Text(formula);
  }
  IChannelConversion::Formula(formula);
}

const std::string& Cc4Block::Formula() const {
  const auto* tx4 = ref_list_.empty() ? nullptr :
                                dynamic_cast<Tx4Block*>(ref_list_[0].get());
  if (tx4 != nullptr) {
    formula_ = tx4->Text();
  } else {
    formula_.clear();
  }
  return IChannelConversion::Formula();
}

[[nodiscard]] uint16_t Cc4Block::NofReferences() const {
  return nof_references_;  
}

void Cc4Block::Reference(uint16_t index, const std::string& text) {
  while (index >= ref_list_.size()) {
    auto temp = std::make_unique<Tx4Block>();
    ref_list_.emplace_back(std::move(temp));
  }
  auto* tx4 = dynamic_cast<Tx4Block*>(ref_list_.back().get());
  if (tx4 != nullptr) {
    tx4->Text(text);
  }
}

std::string Cc4Block::Reference(uint16_t index) const {
  const auto* block = index < ref_list_.size() ?
      ref_list_[index].get() : nullptr;
  if (block == nullptr || block->BlockType() != "TX") {
    return {"Not TX"};
  }
  const auto* tx4 = dynamic_cast<const Tx4Block*> (block);
  if (tx4 != nullptr) {
    return tx4->Text();
  }
  return {"Invalid"};

}

void Cc4Block::SetCcUnit(const CcUnit& unit) {
  unit_ = std::make_unique<Md4Block>(unit.ToXml());
}

void Cc4Block::GetCcUnit(CcUnit& unit) const {
  if (unit_) {
    unit.FromXml(unit_->XmlSnippet());
  }
}

}  // namespace mdf::detail