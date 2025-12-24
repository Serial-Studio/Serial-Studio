/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "ca4block.h"
#include <algorithm>

#include "cn4block.h"

namespace {

constexpr size_t kIndexComposition = 0;
constexpr size_t kIndexArray = 1;

std::string MakeTypeString(uint8_t type) {
  switch (type) {
    case 0:
      return "Array";
    case 1:
      return "Scaling Axis";
    case 2:
      return "Look-Up";
    case 3:
      return "Interval Axis";
    case 4:
      return "Classification Result";
    default:
      break;
  }
  return "Unknown";
}

std::string MakeStorageString(uint8_t storage) {
  switch (storage) {
    case 0:
      return "CN Template";
    case 1:
      return "CG Template";
    case 2:
      return "DG Template";
    default:
      break;
  }
  return "Unknown";
}

std::string MakeFlagString(uint32_t flag) {
  std::ostringstream s;
  if (flag & mdf::CaFlag::DynamicSize) {
    s << "Dynamic";
  }
  if (flag & mdf::CaFlag::InputQuantity) {
    s << (s.str().empty() ? "Input" : ",Input");
  }
  if (flag & mdf::CaFlag::OutputQuantity) {
    s << (s.str().empty() ? "Output" : ",Output");
  }
  if (flag & mdf::CaFlag::ComparisonQuantity) {
    s << (s.str().empty() ? "Comparison" : ",Comparison");
  }
  if (flag & mdf::CaFlag::Axis) {
    s << (s.str().empty() ? "Axis" : ",Axis");
  }
  if (flag & mdf::CaFlag::FixedAxis) {
    s << (s.str().empty() ? "Fixed" : ",Fixed");
  }
  if (flag & mdf::CaFlag::InverseLayout) {
    s << (s.str().empty() ? "Inverse" : ",Inverse");
  }
  if (flag & mdf::CaFlag::LeftOpenInterval) {
    s << (s.str().empty() ? "Left-Open" : ",Left-Open");
  }

  return s.str();
}

/** \brief Create a descriptive text from a generic MDF block pointer.
 *
 * @param block Pointer to an MDF block.
 * @return Descriptive text about the block.
 */
std::string BlockDescription(const mdf::detail::MdfBlock* block) {
  std::ostringstream desc;
  if (block == nullptr) {
    return desc.str();
  }

  try {
    if (block->BlockType() == "DG") {
      const auto* dg_group = dynamic_cast<const mdf::IDataGroup*>(block);
      if (dg_group == nullptr) {
        desc << "DG: Invalid link";
      } else if (!dg_group->Description().empty()) {
        desc << "DG: " << dg_group->Description();
      } else {
        desc << "DG: " << block->Comment();
      }
    } else if (block->BlockType() == "CG") {
      const auto* cg_group = dynamic_cast<const mdf::IChannelGroup*>(block);
      if (cg_group == nullptr) {
        desc << "CG: Invalid link";
      } else if (!cg_group->Name().empty()) {
        desc << "CG: " << cg_group->Name();
      } else if (!cg_group->Description().empty()) {
        desc << "CG: " << cg_group->Description();
      } else {
        desc << "CG: " << block->Comment();
      }
    } else if (block->BlockType() == "CN") {
      const auto* cn_group = dynamic_cast<const mdf::IChannel*>(block);
      if (cn_group == nullptr) {
        desc << "CN: Invalid link";
      } else if (!cn_group->Name().empty()) {
        desc << "CN: " << cn_group->Name();
      } else if (!cn_group->Description().empty()) {
        desc << "CN: " << cn_group->Description();
      } else {
        desc << "CN: " << block->Comment();
      }
    } else if (block->BlockType() == "CC") {
      const auto* cc_group = dynamic_cast<const mdf::IChannelConversion*>(block);
      if (cc_group == nullptr) {
        desc << "CC: 1:1";
      } else if (!cc_group->Name().empty()) {
        desc << "CC: " << cc_group->Name();
      } else if (!cc_group->Description().empty()) {
        desc << "CC: " << cc_group->Description();
      } else {
        desc << "CC: " << block->Comment();
      }
    } else if (block->BlockType() == "CA") {
      const auto* ca_group = dynamic_cast<const mdf::IChannelArray*>(block);
      if (ca_group == nullptr) {
        desc << "CA: Invalid link";
      } else {
        desc << "CA: " << ::MakeTypeString(
                              static_cast<uint8_t>(ca_group->Type()));
      }
    } else  {
      desc << block->BlockType() << ": " << block->Comment();
    }
  } catch (const std::exception& err) {
    desc << "Link error. Error: " << err.what();
  }
  return desc.str();
}

}  // namespace
namespace mdf::detail {

void Ca4Block::GetBlockProperty(BlockPropertyList &dest) const {
  MdfBlock::GetBlockProperty(dest);
    // Set up all the link list
  const auto nof_links = link_list_.size();
  const auto nof_data_blocks = Storage() == ArrayStorage::DgTemplate ? cycle_count_list_.size() : 0;
  const auto nof_dynamic_sizes = (Flags() & CaFlag::DynamicSize) != 0 ? 3 * dimensions_ : 0;
  const auto nof_input_quantities = (Flags() & CaFlag::InputQuantity) != 0 ? 3 * dimensions_ : 0;
  const auto nof_output_quantities = (Flags() & CaFlag::OutputQuantity) != 0 ? 3 * dimensions_ : 0;
  const auto nof_comp_quantities = (Flags() & CaFlag::ComparisonQuantity) != 0 ? 3 : 0;
  const auto nof_axis_conversions = (Flags() & CaFlag::Axis) != 0 ? dimensions_ : 0;
  const auto nof_axis = ((Flags() & CaFlag::Axis) != 0 ) && ((Flags() & CaFlag::FixedAxis) == 0) ? 3 * dimensions_ : 0;

  dest.emplace_back("Links", "", "", BlockItemType::HeaderItem);

  size_t max_index;
  const auto* hd_block = HeaderBlock(); // Need header block so the function Find() search from there.

  for (size_t link_index = 0; link_index < nof_links && hd_block != nullptr; ++link_index) {
    std::ostringstream desc;
    const auto index = Link(link_index);
    const auto* block = hd_block->Find(index);
    const std::string comment = BlockDescription(block);

    if (link_index == kIndexComposition) {
      dest.emplace_back("Composition Block", ToHexString(index),
                         comment, BlockItemType::LinkItem);
      continue;
    }

    // DATA LINKS
    max_index = kIndexArray + nof_data_blocks;
    if (link_index < max_index) {
      dest.emplace_back("Data Block", ToHexString(index),
                        comment,
                        BlockItemType::LinkItem);
      continue;
    }

    // DYNAMIC LINKS
    max_index = kIndexArray + nof_data_blocks + nof_dynamic_sizes;
      if (link_index < max_index) {
          dest.emplace_back("Dynamic Size Block ", ToHexString(index),
                            comment, BlockItemType::LinkItem);
          continue;
      }

    // INPUT QUANTITY
    max_index = kIndexArray + nof_data_blocks + nof_dynamic_sizes + nof_input_quantities;
    if (link_index < max_index) {
      dest.emplace_back("InputQuantity Block ", ToHexString(index),
                        comment, BlockItemType::LinkItem);
      continue;
    }

    // OUTPUT QUANTITY
    max_index = kIndexArray + nof_data_blocks + nof_dynamic_sizes + nof_input_quantities
        + nof_output_quantities;
    if (link_index < max_index) {
      dest.emplace_back("Output Quantity Block ", ToHexString(index),
                        comment, BlockItemType::LinkItem);
      continue;
    }

    // COMPARISON QUANTITY
    max_index = kIndexArray + nof_data_blocks + nof_dynamic_sizes + nof_input_quantities
        + nof_output_quantities + nof_comp_quantities;
    if (link_index < max_index) {
      dest.emplace_back("Comparison Quantity Block ", ToHexString(index),
                        comment, BlockItemType::LinkItem);
      continue;
    }

    // AXIS CONVERSION
    max_index = kIndexArray + nof_data_blocks + nof_dynamic_sizes + nof_input_quantities
        + nof_output_quantities + nof_comp_quantities + nof_axis_conversions;
    if (link_index < max_index) {
      dest.emplace_back("Axis Conversion Block ", ToHexString(index),
                        index == 0 ? "1:1" : comment,
                        BlockItemType::LinkItem);
      continue;
    }

    // AXIS
    max_index = kIndexArray + nof_data_blocks + nof_dynamic_sizes + nof_input_quantities
        + nof_output_quantities + nof_comp_quantities + nof_axis_conversions + nof_axis;
    if (link_index < max_index) {
      dest.emplace_back("Axis Block ", ToHexString(index),
                        comment, BlockItemType::LinkItem);
      continue;
    }
  }
  dest.emplace_back("", "", "", BlockItemType::BlankItem);

  dest.emplace_back("Information", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Array Type", MakeTypeString(type_));
  dest.emplace_back("Storage Type", MakeStorageString(storage_));
  dest.emplace_back("Nof Dimensions", std::to_string(dimensions_));
  dest.emplace_back("Flags", MakeFlagString(flags_));
  dest.emplace_back("Byte Offset", std::to_string(byte_offset_base_));
  dest.emplace_back("Invalid Bit Position",
                    std::to_string(invalid_bit_pos_base_));
   if (md_comment_) {
    md_comment_->GetBlockProperty(dest);
  }
  for (size_t dim = 0; dim < dim_size_list_.size(); ++dim) {
    std::ostringstream label;
    label << "Dimension " << dim << " Size:";
    dest.emplace_back(label.str(), std::to_string(dim_size_list_[dim]));
  }
  for (size_t axis = 0; axis < axis_value_list_.size(); ++axis) {
    std::ostringstream label;
    label << "Axis " << axis << ":";
    dest.emplace_back(label.str(), std::to_string(axis_value_list_[axis]));
  }
  for (size_t cycle = 0; cycle < cycle_count_list_.size(); ++cycle) {
    std::ostringstream label;
    label << "Cycle Count " << cycle << ":";
    dest.emplace_back(label.str(), std::to_string(cycle_count_list_[cycle]));
  }
}

uint64_t Ca4Block::Read(std::streambuf& buffer) {
  uint64_t bytes = ReadHeader4(buffer);  // This function also read in all links
  bytes += ReadNumber(buffer, type_);
  bytes += ReadNumber(buffer, storage_);
  bytes += ReadNumber(buffer, dimensions_);
  bytes += ReadNumber(buffer, flags_);
  bytes += ReadNumber(buffer, byte_offset_base_);
  bytes += ReadNumber(buffer, invalid_bit_pos_base_);
  dim_size_list_.resize(dimensions_, 0);  // Resize number values/dimension
  for (uint64_t& dim_size : dim_size_list_ ) {
    bytes += ReadNumber(buffer, dim_size);
  }
  const auto sum_dim = SumOfArray();
  const auto prod_dim = ProductOfArray();
  const auto max_bytes = BlockLength();
  if ((flags_ & CaFlag::FixedAxis) != 0) {
    axis_value_list_.resize(static_cast<size_t>(sum_dim), 0.0);
    for (double& axis_value : axis_value_list_) {
      bytes += ReadNumber(buffer, axis_value);
    }
  }

  switch (Storage()) {
    case ArrayStorage::CgTemplate:
    case ArrayStorage::DgTemplate: {
      cycle_count_list_.resize(static_cast<size_t>(prod_dim), 0);
      for (uint64_t& cycle : cycle_count_list_) {
        if (bytes + 8 <= max_bytes) {
          bytes += ReadNumber(buffer, cycle);
        }
      }
      break;
    }

    default:
      break;
  }

  // Finds all objects. Note that is function is called after all blocks are
  // read. For now, it sizes the lists.
  FindAllReferences(buffer);
  return bytes;
}

void Ca4Block::FindAllReferences(std::streambuf& buffer) {
  const uint64_t prod_dim = ProductOfArray();
  size_t link_index = 1;
  if (Storage() == ArrayStorage::DgTemplate) {
    data_links_.resize(static_cast<size_t>(prod_dim), 0);
    for (int64_t& position : data_links_) {
      position = Link(link_index);
      ++link_index;
    }
  }

  if ((Flags() & CaFlag::DynamicSize) != 0) {
    dynamic_size_list_.resize(dimensions_);
    for (CaTripleReference& ref : dynamic_size_list_) {
      ref = ReadReference(link_index);
      link_index += 3;
    }
  }

  if ((Flags() & CaFlag::InputQuantity) != 0) {
    input_quantity_list_.resize(dimensions_);
    for (CaTripleReference& ref : input_quantity_list_) {
      ref = ReadReference(link_index);
      link_index += 3;
    }
  }

  if ((Flags() & CaFlag::OutputQuantity) != 0) {
    output_quantity_ = ReadReference(link_index);
    link_index += 3;
  }

  if ((Flags() & CaFlag::ComparisonQuantity) != 0) {
    comparison_quantity_ = ReadReference(link_index);
    link_index += 3;
  }

  const auto* header = HeaderBlock();
  if ((Flags() & CaFlag::Axis) != 0) {
    axis_conversion_list_.resize(dimensions_,nullptr);
    for (auto& ref: axis_conversion_list_) {
      const int64_t position = Link(link_index);
      if (position > 0 && header != nullptr) {
        try {
          ref = dynamic_cast<const IChannelConversion*>
              (header->Find(position));
        } catch (const std::exception& ) {
          ref = nullptr;
        }
      } else {
        ref = nullptr;
      }
      ++link_index;
    }
  }

  if ((Flags() & CaFlag::Axis) != 0 && (Flags() & CaFlag::FixedAxis) == 0) {
    axis_list_.resize(dimensions_);
    for (CaTripleReference& ref : axis_list_) {
      ref = ReadReference(link_index);
      link_index += 3;
    }
  }
}

uint64_t Ca4Block::Write(std::streambuf& buffer) {
  const bool update = FilePosition() > 0;  // True if already written to file
  if (update) {
    return static_cast<size_t>(block_length_);
  }

  block_type_ = "##CA";
  // Calculate the block length. It's a bit tricky due to the dynamic number
  // of links.
  block_length_ = static_cast<uint64_t>(24 + 8); // Next CA/CN block required
  size_t nof_links = 1;
  if (Storage() == ArrayStorage::DgTemplate) {
    const size_t nof_data_links =  data_links_.size();
    nof_links += nof_data_links;
    block_length_ += static_cast<uint64_t>(nof_data_links) * 8;
  }

  if ((Flags() & CaFlag::DynamicSize) != 0) {
    const size_t nof_dynamic_links = Dimensions();
    nof_links += nof_dynamic_links * 3;
    block_length_ += static_cast<uint64_t>(nof_dynamic_links) * 8 * 3;
  }

  if ((Flags() & CaFlag::InputQuantity) != 0) {
    const size_t nof_quantity_links = Dimensions();
    nof_links += nof_quantity_links * 3;
    block_length_ += static_cast<uint64_t>(nof_quantity_links) * 8 * 3;
  }
  if ((Flags() & CaFlag::OutputQuantity) != 0) {
    nof_links += 3;
    block_length_ +=  8 * 3; // Only one reference link is possible
  }
  if ((Flags() & CaFlag::ComparisonQuantity) != 0) {
    nof_links += 3;
    block_length_ +=  8 * 3; // Only one reference link is possible
  }
  if ((Flags() & CaFlag::Axis) != 0) {
    const size_t nof_axis_links = Dimensions();
    nof_links += nof_axis_links;
    block_length_ += static_cast<uint64_t>(nof_axis_links) * 8;
  }
  if ((Flags() & CaFlag::Axis) != 0 && (Flags() & CaFlag::FixedAxis) == 0) {
    const size_t nof_axis_links = Dimensions();
    nof_links += nof_axis_links * 3;
    block_length_ += static_cast<uint64_t>(nof_axis_links) * 8 * 3;
  }

  block_length_ += 1 + 1 + 2 + 4 + 4 + 4 + (8 * Dimensions());
  if ((Flags() & CaFlag::FixedAxis) != 0) {
    block_length_ += SumOfArray() * 8;
  }
  switch (Storage() ) {
    case ArrayStorage::DgTemplate:
    case ArrayStorage::CgTemplate:
      block_length_ += ProductOfArray() * 8;
      break;

    default:
      break;
  }

  link_list_.resize(nof_links, 0);

  size_t link_index = kIndexArray;
  if (Storage() == ArrayStorage::DgTemplate) {
    for (size_t index = 0; index < ProductOfArray(); ++index) {
      const int64_t data_position = index < data_links_.size() ?
                                        data_links_[index] : 0;
      UpdateLink(buffer, link_index, data_position);
      ++link_index;
    }
  }
  if ((Flags() & CaFlag::DynamicSize) != 0) {
    link_index += WriteReferences(buffer, dynamic_size_list_,
                                  Dimensions(), link_index);
  }
  if ((Flags() & CaFlag::InputQuantity) != 0) {
    link_index += WriteReferences(buffer, input_quantity_list_,
                                  Dimensions(), link_index);
  }
  if ((Flags() & CaFlag::OutputQuantity) != 0) {
    WriteReference(buffer, output_quantity_, link_index);
    link_index += 3;
  }
  if ((Flags() & CaFlag::ComparisonQuantity) != 0) {
    WriteReference(buffer, comparison_quantity_, link_index);
    link_index += 3;
  }
  if ((Flags() & CaFlag::Axis) != 0) {
    for (size_t index = 0; index < Dimensions(); ++index) {
      if (index < axis_conversion_list_.size()) {
        const auto* cc_block = axis_conversion_list_[index];
        const int64_t axis_position = cc_block != nullptr ? cc_block->Index() : 0;
        UpdateLink(buffer,link_index,axis_position);
      } else {
        UpdateLink(buffer,link_index,0);
      }
      ++link_index;
    }
  }
  if ((Flags() & CaFlag::Axis) != 0 && (Flags() & CaFlag::FixedAxis) == 0) {
    link_index += WriteReferences(buffer, axis_list_,
                                  Dimensions(), link_index);
  }

  uint64_t bytes = MdfBlock::Write(buffer);
  bytes += WriteNumber(buffer, type_);
  bytes += WriteNumber(buffer, storage_);
  bytes += WriteNumber(buffer, dimensions_);
  bytes += WriteNumber(buffer, flags_);
  bytes += WriteNumber(buffer, byte_offset_base_);
  bytes += WriteNumber(buffer, invalid_bit_pos_base_);

  for (size_t dimension = 0; dimension < Dimensions(); ++dimension) {
    const uint64_t dim_size = dimension < dim_size_list_.size() ?
                                   dim_size_list_[dimension] : 0;
    bytes += WriteNumber(buffer, dim_size);
  }
  if ((Flags() & CaFlag::FixedAxis) != 0) {
    for (size_t axis = 0; axis < SumOfArray(); ++axis) {
      const double axis_value = axis < axis_value_list_.size() ?
                                axis_value_list_[axis] : 0.0;
      bytes += WriteNumber(buffer, axis_value);
    }
  }

  switch (Storage() ) {
    case ArrayStorage::DgTemplate:
    case ArrayStorage::CgTemplate: {
      for (size_t cycle = 0; cycle < ProductOfArray(); ++cycle) {
        const uint64_t count =
            cycle < cycle_count_list_.size() ? cycle_count_list_[cycle] : 0;
        bytes += WriteNumber(buffer, count);
      }
      break;
    }

    default:
      break;
  }
  UpdateBlockSize(buffer, bytes);
  return bytes;
}

int64_t Ca4Block::Index() const { return FilePosition(); }

void Ca4Block::Type(ArrayType type) { type_ = static_cast<uint8_t>(type); }

ArrayType Ca4Block::Type() const { return static_cast<ArrayType>(type_); }

std::string Ca4Block::TypeAsString() const {
  return MakeTypeString(type_);
}

void Ca4Block::Storage(ArrayStorage storage) {
  storage_ = static_cast<uint8_t>(storage);
}

ArrayStorage Ca4Block::Storage() const {
  return static_cast<ArrayStorage>(storage_);
}

std::string Ca4Block::StorageAsString() const {
  return MakeStorageString(storage_);
}


void Ca4Block::Flags(uint32_t flags) { flags_ = flags; }

uint32_t Ca4Block::Flags() const { return flags_; }

Ca4Block::Ca4Block() { block_type_ = "##CA"; }

uint64_t Ca4Block::SumOfArray() const {
  uint64_t sum = 0;
  std::for_each(dim_size_list_.cbegin(), dim_size_list_.cend(),
                [&] (auto dimension) -> void {
                  sum += dimension;
                });
  return sum;
}

uint64_t Ca4Block::ProductOfArray() const {
  uint64_t product = 1;
  std::for_each(dim_size_list_.cbegin(), dim_size_list_.cend(),
                [&] (auto dimension) -> void {
                  product *= dimension > 0 ? dimension : 1;
                });
  return product;
}

size_t Ca4Block::Dimensions() const {
  return dimensions_;
}

void Ca4Block::Shape(const std::vector<uint64_t>& dim_sizes) {
  dim_size_list_ = dim_sizes;
  dimensions_ = static_cast<uint16_t>(dim_size_list_.size());
}

const std::vector<uint64_t>& Ca4Block::Shape() const {
  return dim_size_list_;
}

uint64_t Ca4Block::DimensionSize(size_t dimension) const {
  return dimension < dim_size_list_.size() ?
    dim_size_list_[dimension] : 0;
}

const std::vector<double>& Ca4Block::AxisValues() const {
  return axis_value_list_;
}

std::vector<double>& Ca4Block::AxisValues() {
  const uint64_t sum_of_array = SumOfArray();
  if ((Flags() & CaFlag::FixedAxis) != 0 && sum_of_array > 0 &&
      axis_value_list_.size() < sum_of_array) {
    axis_value_list_.resize(static_cast<size_t>(sum_of_array),0.0);
  }
  return axis_value_list_;
}

const std::vector<uint64_t>& Ca4Block::CycleCounts() const {
  return cycle_count_list_;
}

std::vector<uint64_t>& Ca4Block::CycleCounts() {
  switch (Storage()) {
    case ArrayStorage::DgTemplate:
    case ArrayStorage::CgTemplate: {
      const uint64_t prod_of_array = ProductOfArray();
      if ( cycle_count_list_.size() < prod_of_array) {
        cycle_count_list_.resize(static_cast<size_t>(prod_of_array),0);
      }
      break;
    }

    default:
      break;
  }
  return cycle_count_list_;
}

CaTripleReference Ca4Block::ReadReference(size_t index) const {
  CaTripleReference ref;
  const auto* header = HeaderBlock();
  if (header == nullptr) {
    return ref;
  }

  const int64_t dg_position = Link(index);
  const int64_t cg_position = Link(index + 1);
  const int64_t cn_position = Link(index + 2);
  if (dg_position > 0) {
    try {
      ref.DataGroup = dynamic_cast<const IDataGroup*>
          (header->Find(dg_position));
    } catch (const std::exception& ) {
      ref.DataGroup = nullptr;
    }
  }

  if (cg_position > 0) {
    try {
      ref.ChannelGroup = dynamic_cast<const IChannelGroup*>
          (header->Find(cg_position));
    } catch (const std::exception& ) {
      ref.ChannelGroup = nullptr;
    }
  }

  if (cn_position > 0) {
    try {
      ref.Channel = dynamic_cast<const IChannel*>
          (header->Find(cn_position));
    } catch (const std::exception& ) {
      ref.Channel = nullptr;
    }
  }
  return ref;
}

void Ca4Block::WriteReference(std::streambuf& buffer,
                    const CaTripleReference& ref, size_t start_index) {

  UpdateLink(buffer, start_index,
             ref.DataGroup != nullptr ? ref.DataGroup->Index() : 0 );
  UpdateLink(buffer, start_index + 1,
             ref.ChannelGroup != nullptr ? ref.ChannelGroup->Index() : 0 );
  UpdateLink(buffer, start_index + 2,
             ref.Channel != nullptr ? ref.Channel->Index() : 0 );
}

size_t Ca4Block::WriteReferences(std::streambuf& buffer,
                       const std::vector<CaTripleReference>& list,
                       size_t max_fill,
                       size_t start_index) {
  size_t nof_links = 0;
  for (size_t index = 0; index < max_fill; ++index) {
    if (index < list.size()) {
      const auto& ref = list[index];
      WriteReference(buffer, ref, start_index + nof_links);
      UpdateLink(buffer, start_index + nof_links,
                 ref.DataGroup != nullptr ? ref.DataGroup->Index() : 0 );
    } else {
      UpdateLink(buffer, start_index + nof_links,0 );
      UpdateLink(buffer, start_index + nof_links + 1, 0 );
      UpdateLink(buffer, start_index + nof_links + 2, 0 );
    }
    nof_links += 3;
  }
  return nof_links;
}

void Ca4Block::SetParentChannel(const Cn4Block* parent) {
  parent_channel_ = parent;
}

void Ca4Block::PrepareForWriting() {
  if (parent_channel_ != nullptr) {
    uint32_t size_of_value = parent_channel_->BitCount() / 8;
    if ((parent_channel_->BitCount() % 8) > 0) {
      ++size_of_value;
    }
    byte_offset_base_ = static_cast<int32_t>(size_of_value);

    invalid_bit_pos_base_ = 0;
  }
}

}  // namespace mdf::detail