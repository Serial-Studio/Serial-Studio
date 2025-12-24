/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "cg4block.h"
#include <algorithm>

#include <codecvt>
#include <climits>
#include "cn4block.h"
#include "sr4block.h"
#include "mdf/mdflogstream.h"

namespace {

constexpr size_t kIndexNext = 0;
constexpr size_t kIndexCn = 1;
constexpr size_t kIndexName = 2;
constexpr size_t kIndexSi = 3;
constexpr size_t kIndexSr = 4;
constexpr size_t kIndexMd = 5;
constexpr size_t kIndexMaster = 6;

std::string MakeFlagString(uint16_t flag) {
  std::ostringstream s;
  if (flag & 0x0001) {
    s << "VLSD";
  }
  if (flag & 0x0002) {
    s << (s.str().empty() ? "Bus Event" : ",Bus Event");
  }
  if (flag & 0x0004) {
    s << (s.str().empty() ? "Plain" : ",Plain");
  }
  return s.str();
}

void AddCxChannels(const mdf::detail::Cn4Block &cn_block, // NOLINT
                   std::vector<mdf::IChannel *>& channel_list) {
  auto& cx_list = cn_block.Cx4();
  for (auto& cx: cx_list) {
    if (!cx || cx->BlockType() != "CN") {
      continue;
    }
    auto* cn4_block = dynamic_cast<mdf::detail::Cn4Block*>(cx.get());
    if (cn4_block == nullptr) {
      continue;
    }
    channel_list.push_back(cn4_block);
    // Include any composition channels as well
    AddCxChannels(*cn4_block, channel_list);
  }
}

}  // end namespace

namespace mdf::detail {
Cg4Block::Cg4Block() {
  block_type_ = "##CG";
}

int64_t Cg4Block::Index() const { return FilePosition(); }

void Cg4Block::Name(const std::string &name) { acquisition_name_ = name; }

std::string Cg4Block::Name() const { return acquisition_name_; }

void Cg4Block::Description(const std::string &description) {
  md_comment_ = std::make_unique<Md4Block>(description);
}

std::string Cg4Block::Description() const { return MdText(); }

uint64_t Cg4Block::NofSamples() const { return nof_samples_; }

void Cg4Block::NofSamples(uint64_t nof_samples) { nof_samples_ = nof_samples; }

const IChannel *Cg4Block::GetXChannel(const IChannel &reference) const {
  const auto *cn4 = dynamic_cast<const Cn4Block *>(&reference);
  if (cn4 == nullptr) {
    return nullptr;
  }

  // First check if the channel have a dedicated X channel reference
  const auto x_axis_list = cn4->XAxisLinkList();
  // As we are returning a channel pointer, we must assume that it belongs to
  // this group
  if (x_axis_list.size() == 3 && x_axis_list[1] == Index() && x_axis_list[2]) {
    const auto channel_index = x_axis_list[2];
    const auto find =
      std::find_if(cn_list_.cbegin(), cn_list_.cend(),
                             [&](const auto &p) {
      if (!p) {
        return false;
      }
      return p->Index() == channel_index;
    });

    if (find != cn_list_.cend()) {
      return find->get();
    }
  }

  // Search for the master channel in the group
  const auto master =
    std::find_if(cn_list_.cbegin(), cn_list_.cend(),
                             [&](const auto &x) {
    if (!x) {
      return false;
    }
    return x->Type() == ChannelType::Master ||
           x->Type() == ChannelType::VirtualMaster;
  });

  return master != cn_list_.cend() ? master->get() : nullptr;
}

void Cg4Block::GetBlockProperty(BlockPropertyList &dest) const {
  MdfBlock::GetBlockProperty(dest);

  dest.emplace_back("Links", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Next CG", ToHexString(Link(kIndexNext)),
                    "Link to next channel group", BlockItemType::LinkItem);
  dest.emplace_back("First CN", ToHexString(Link(kIndexCn)),
                    "Link to first channel", BlockItemType::LinkItem);
  dest.emplace_back("Name TX", ToHexString(Link(kIndexName)), acquisition_name_,
                    BlockItemType::LinkItem);
  dest.emplace_back("Source SI", ToHexString(Link(kIndexSi)),
                    "Link to source information", BlockItemType::LinkItem);
  dest.emplace_back("Reduction SR", ToHexString(Link(kIndexSr)),
                    "Link to first sample reduction", BlockItemType::LinkItem);
  dest.emplace_back("Comment MD", ToHexString(Link(kIndexMd)), Comment(),
                    BlockItemType::LinkItem);
  dest.emplace_back("", "", "", BlockItemType::BlankItem);

  dest.emplace_back("Information", "", "", BlockItemType::HeaderItem);

  if (Link(kIndexName) > 0) {
    dest.emplace_back("Name", acquisition_name_);
  }
  dest.emplace_back("Nof Channels", std::to_string(cn_list_.size()));
  dest.emplace_back("Nof SR", std::to_string(sr_list_.size()));
  dest.emplace_back("Record ID", std::to_string(record_id_));
  dest.emplace_back("Nof Samples", std::to_string(nof_samples_));
  dest.emplace_back("Flags", MakeFlagString(flags_));

  const wchar_t path_separator[2] = {static_cast<wchar_t>(path_separator_),0};
  const wchar_t* path_ptr = path_separator;

  std::string utf8_path_separator;
  {
    char mbstr[MB_LEN_MAX * 2] = {0};
    std::mbstate_t state = std::mbstate_t();
    size_t len = std::wcsrtombs(mbstr, 
                                &path_ptr,
                                sizeof(mbstr), 
                                &state);
    if (len != static_cast<size_t>(-1)) {
      utf8_path_separator.assign(mbstr, len);
    }
  }

  dest.emplace_back("Path Separator", utf8_path_separator);

  dest.emplace_back("Data Bytes", std::to_string(nof_data_bytes_));
  dest.emplace_back("Invalid Bytes", std::to_string(nof_invalid_bytes_));
  if (md_comment_) {
    md_comment_->GetBlockProperty(dest);
  }
}

uint64_t Cg4Block::Read(std::streambuf& buffer) {
  uint64_t bytes = ReadHeader4(buffer);
  bytes += ReadNumber(buffer, record_id_);
  bytes += ReadNumber(buffer, nof_samples_);
  bytes += ReadNumber(buffer, flags_);
  bytes += ReadNumber(buffer, path_separator_);
  std::vector<uint8_t> reserved;
  bytes += ReadByte(buffer, reserved, 4);
  bytes += ReadNumber(buffer, nof_data_bytes_);
  bytes += ReadNumber(buffer, nof_invalid_bytes_);

  acquisition_name_ = ReadTx4(buffer, kIndexName);
  if (Link(kIndexSi) > 0) {
    SetFilePosition(buffer, Link(kIndexSi));
    si_block_ = std::make_unique<Si4Block>();
    si_block_->Init(*this);
    si_block_->Read(buffer);
  }
  ReadMdComment(buffer, kIndexMd);
  return bytes;
}

uint64_t Cg4Block::Write(std::streambuf& buffer) {
  const bool update = FilePosition() > 0;  // True if already written to file
  const auto master = (flags_ & CgFlag::RemoteMaster) != 0;
  const auto vlsd = (flags_ & CgFlag::VlsdChannel) != 0;
  if (!update) {
    block_type_ = "##CG";
    block_length_ = 24 + (6 * 8) + 8 + 8 + 2 + 2 + 4 + 4 + 4;
    if (master) {
      block_length_ += 8;  // Add one more link for master
    }
    link_list_.resize(master ? 7 : 6, 0);
  }


  WriteLink4List(buffer, cn_list_, kIndexCn,
                 UpdateOption::DoNotUpdateWrittenBlock);
  WriteTx4(buffer, kIndexName, acquisition_name_);
  WriteBlock4(buffer, si_block_, kIndexSi);
  WriteLink4List(buffer, sr_list_, kIndexSr,
                 UpdateOption::DoNotUpdateWrittenBlock);
  WriteMdComment(buffer, kIndexMd);
  // ToDo: Remote master handling

  uint64_t bytes = update ? MdfBlock::Update(buffer) : MdfBlock::Write(buffer);
  if (update) {
    // Update number of samples
    if (nof_samples_position_ > 0) {
      SetFilePosition(buffer, nof_samples_position_);
      WriteNumber(buffer, nof_samples_);
    }
    // Update VLSD size (which is a 64-bit value, low 32-bit)
    if (nof_data_position_ > 0) {
      SetFilePosition(buffer, nof_data_position_);
      WriteNumber(buffer, nof_data_bytes_);
    }
    // Update VLSD size (which is a 64-bit value, high 32-bit)
    if (nof_invalid_position_ > 0) {
      SetFilePosition(buffer, nof_invalid_position_);
      WriteNumber(buffer, nof_invalid_bytes_);
    }
    bytes = block_length_;
  } else {
    bytes += WriteNumber(buffer, record_id_);
    nof_samples_position_ = GetFilePosition(buffer);
    bytes += WriteNumber(buffer, nof_samples_);
    bytes += WriteNumber(buffer, flags_);
    bytes += WriteNumber(buffer, path_separator_);
    bytes += WriteBytes(buffer, 4);
    // Save the nof data and invalid bytes in case of a VLSD group.
    // Number data bytes is the lower 32-bit and number invalid bytes is the
    // 32-bit higher value.
    if (vlsd) {
      nof_data_position_ = GetFilePosition(buffer);
    }
    bytes += WriteNumber(buffer, nof_data_bytes_);
    if (vlsd) {
      nof_invalid_position_ = GetFilePosition(buffer);
    }
    bytes += WriteNumber(buffer, nof_invalid_bytes_);
    UpdateBlockSize(buffer, bytes);

    // Must scan through the channels and detect if any MLSD channel exist
    // and update its signal index. First need to find the length channel
    // block position. Then set the signal data index to that value
    const auto* data_length = GetChannel(".DataLength");
    if (data_length != nullptr) {
      const auto cn_list = Channels(); // This list include the composite channels
      const auto block_position = data_length->Index();
      for (auto* channel : cn_list) {
        if (channel != nullptr && channel->Type() == ChannelType::MaxLength) {
          dynamic_cast<Cn4Block*>(channel)->UpdateDataLink(buffer, block_position);
        }
      }
    }

  }
  return bytes;
}

void Cg4Block::ReadCnList(std::streambuf& buffer) {
  ReadLink4List(buffer, cn_list_, kIndexCn);
}

void Cg4Block::ReadSrList(std::streambuf& buffer) {
  ReadLink4List(buffer, sr_list_, kIndexSr);
}

MdfBlock *Cg4Block::Find(int64_t index) const {
  if (si_block_) {
    auto *p = si_block_->Find(index);
    if (p != nullptr) {
      return p;
    }
  }
  for (auto &cn : cn_list_) {
    if (!cn) {
      continue;
    }
    auto *p = cn->Find(index);
    if (p != nullptr) {
      return p;
    }
  }

  for (auto &sr : sr_list_) {
    if (!sr) {
      continue;
    }
    auto *p = sr->Find(index);
    if (p != nullptr) {
      return p;
    }
  }

  return MdfBlock::Find(index);
}

uint64_t Cg4Block::ReadDataRecord(std::streambuf& buffer,
                                const IDataGroup &notifier) const {
  uint64_t count = 0;
  if (flags_ & CgFlag::VlsdChannel) {
    // This is normally used for string and the CG block only include one signal
    uint32_t length = 0;
    count += ReadNumber(buffer, length);
    std::vector<uint8_t> record(length, 0);
    if (length > 0) {
      count += buffer.sgetn(
          reinterpret_cast<char*>(record.data()),
           length);
    }
    const uint64_t sample = Sample();
    if (sample < NofSamples()) {
      const bool continue_reading = notifier.NotifySampleObservers(sample,
                                                 RecordId(), record);
      IncrementSample();
      if (!continue_reading) {
        return 0;
      }
    }
  } else {
    // Normal fixed length records
    const size_t record_size = nof_data_bytes_ + nof_invalid_bytes_;
    std::vector<uint8_t> record(record_size, 0);
    count = buffer.sgetn(
        reinterpret_cast<char*>(record.data()),
        static_cast<std::streamsize>(record.size()) );
    const uint64_t sample = Sample();
    if (sample < NofSamples()) {
      const bool continue_reading = notifier.NotifySampleObservers(sample, RecordId(), record);
      IncrementSample();
      if (!continue_reading) {
        return 0;
      }
    }
  }
  return count;
}



std::vector<IChannel *> Cg4Block::Channels() const {
  std::vector<IChannel *> channel_list;
  for (const auto& cn4 : cn_list_) {
    const auto* cn_block = cn4.get();
    if (cn_block == nullptr) {
      continue;
    }
    channel_list.push_back(cn4.get());
    // Include any composition channels as well
    AddCxChannels(*cn_block, channel_list);

  }
  return channel_list;
}

uint64_t Cg4Block::RecordId() const { return record_id_; }

void Cg4Block::RecordId(uint64_t record_id) { record_id_ = record_id; }

void Cg4Block::AddCn4(std::unique_ptr<Cn4Block> &cn4) {
  cn_list_.push_back(std::move(cn4));
}

uint16_t Cg4Block::Flags() const { return flags_; }

void Cg4Block::Flags(uint16_t flags) { flags_ = flags; }

char16_t Cg4Block::PathSeparator() { return path_separator_; }

void Cg4Block::PathSeparator(char16_t path_separator) {
  path_separator_ = path_separator;
}

ISourceInformation *Cg4Block::CreateSourceInformation() {
  if (!si_block_) {
    auto si4 = std::make_unique<Si4Block>();
    si4->Init(*this);
    si_block_ = std::move(si4);
  }
  return si_block_.get();
}

ISourceInformation *Cg4Block::SourceInformation() const {
  return si_block_ ? si_block_.get() : nullptr;
}

void Cg4Block::UpdateCycleCounter(uint64_t nof_samples) {
  nof_samples_ += nof_samples;
}

void Cg4Block::UpdateVlsdSize(uint64_t nof_data_bytes) {
  if (Flags() & CgFlag::VlsdChannel) {
    uint64_t vlsd_size = nof_invalid_bytes_;
    vlsd_size <<= 32;
    vlsd_size += nof_data_bytes_;

    vlsd_size += nof_data_bytes;
    nof_data_bytes_ = static_cast<uint32_t>(vlsd_size & 0xFFFFFFFF);
    nof_invalid_bytes_ = static_cast<uint32_t>(vlsd_size >> 32);
  }
}

uint64_t Cg4Block::StepRecord(std::streambuf& buffer) const {
  const size_t record_size = nof_data_bytes_ + nof_invalid_bytes_;
  return  StepFilePosition(buffer, record_size);
}

IChannel *Cg4Block::CreateChannel() {
  auto cn4 = std::make_unique<detail::Cn4Block>();
  cn4->Init(*this);
  AddCn4(cn4);
  return cn_list_.empty() ? nullptr : cn_list_.back().get();
}

void Cg4Block::PrepareForWriting() {
  nof_data_bytes_ = 0;
  vlsd_index_ = 0;
  if (Flags() & CgFlag::VlsdChannel) {
    // This is a specialized CG (VLSD) group with variable length
    // channel. Some channels in other groups may reference this group.
    // This group does not contain any channels.
    nof_invalid_bytes_ = 0;
    sample_buffer_.clear();
    sample_buffer_.shrink_to_fit();
    return;
  }

  // Calculates number of data bytes. Note that some channels may
  // be a so-called array channel. Little bit complicated but the
  // referenced channel have a channel array (CA) block.
  uint64_t byte_offset = 0;
  uint64_t invalid_bit_offset = 0;
  for (const auto &channel : cn_list_) {
    if (!channel) {
      continue;
    }
    channel->PrepareForWriting(byte_offset);
    const auto value_size = static_cast<uint32_t>(channel->DataBytes());
    const uint64_t array_size = channel->ArraySize();
    if (array_size > 1) {
      nof_data_bytes_ += static_cast<uint32_t>(value_size * array_size);
      byte_offset += static_cast<uint32_t>(value_size * array_size);
    } else {
      // Not an array
      nof_data_bytes_ += static_cast<uint32_t>(value_size);
      byte_offset += static_cast<uint32_t>(value_size);
    }

    if (channel->Flags() & CnFlag::InvalidValid) {
      channel->SetInvalidOffset(invalid_bit_offset);
      if (array_size > 1) {
        invalid_bit_offset += array_size;
      } else {
        ++invalid_bit_offset;
      }
    }
  }

  if (invalid_bit_offset == 0) {
    nof_invalid_bytes_ = 0;
  } else if ((invalid_bit_offset % 8) > 0) {
    nof_invalid_bytes_ = static_cast<uint32_t>((invalid_bit_offset / 8) + 1);
  } else {
    nof_invalid_bytes_ = static_cast<uint32_t>(invalid_bit_offset / 8);
  }

  if (const auto total_size = nof_invalid_bytes_ + nof_data_bytes_;
      total_size > 0) {
    sample_buffer_.resize(total_size,0);
  } else {
    sample_buffer_.clear();
    sample_buffer_.shrink_to_fit();
  }
}

void Cg4Block::WriteSample(std::streambuf& buffer, uint8_t record_id_size,
                                   const std::vector<uint8_t> &source) {
  switch (record_id_size) {
    case 0:
      // No Record ID to store
      break;

    case 1: {
      const auto id = static_cast<uint8_t>(RecordId());
      WriteNumber(buffer, id);
      break;
    }

    case 2: {
      const auto id = static_cast<uint16_t>(RecordId());
      WriteNumber(buffer, id);
      break;
    }

    case 4: {
      const auto id = static_cast<uint32_t>(RecordId());
      WriteNumber(buffer, id);
      break;
    }

    default: {
      const auto id = RecordId();
      WriteNumber(buffer, id);
      break;
    }

  }
  const uint64_t bytes = buffer.sputn(
      reinterpret_cast<const char*>(source.data()),
      static_cast<std::streamsize>(source.size()));
  if (bytes != source.size()) {
    MDF_ERROR() << "Failed to write a sample data. Written : "
                << bytes << "(" << source.size() << ")";
  }
  IncrementSample();            // Increment internal sample counter
  NofSamples(Sample());
}

void Cg4Block::WriteCompressedSample(std::vector<uint8_t>& dest,
                                     uint8_t record_id_size,
                                     const std::vector<uint8_t> &buffer) {
  switch (record_id_size) {
    case 0:
      // No Record ID to store
      break;

    case 1: {
      const auto id = static_cast<uint8_t>(RecordId());
      const LittleBuffer ident(id);
      std::copy(ident.cbegin(), ident.cend(), std::back_inserter(dest));
      break;
    }

    case 2: {
      const auto id = static_cast<uint16_t>(RecordId());
      const LittleBuffer ident(id);
      std::copy(ident.cbegin(), ident.cend(), std::back_inserter(dest));
      break;
    }

    case 4: {
      const auto id = static_cast<uint32_t>(RecordId());
      const LittleBuffer ident(id);
      std::copy(ident.cbegin(), ident.cend(), std::back_inserter(dest));
      break;
    }

    default: {
      const auto id = RecordId();
      const LittleBuffer ident(id);
      std::copy(ident.cbegin(), ident.cend(),
        std::back_inserter(dest));
      break;
    }
  }
  std::copy(buffer.cbegin(), buffer.cend(),
    std::back_inserter(dest));
  IncrementSample();            // Increment internal sample counter
  NofSamples(Sample());
}

uint64_t Cg4Block::WriteVlsdSample(std::streambuf& buffer, uint8_t record_id_size,
                                   const std::vector<uint8_t> &source) {
  const uint64_t vlsd_index = vlsd_index_; // Temporary store the old index
  uint64_t total_count = nof_invalid_bytes_;
  total_count <<= 32;
  total_count += nof_data_bytes_;

  switch (record_id_size) {
    case 0:
      // No Record ID to store
      break;

    case 1: {
      const auto id = static_cast<uint8_t>(RecordId());
      WriteNumber(buffer, id);
      break;
    }

    case 2: {
      const auto id = static_cast<uint16_t>(RecordId());
      WriteNumber(buffer, id);
      break;
    }

    case 4: {
      const auto id = static_cast<uint32_t>(RecordId());
      WriteNumber(buffer, id);
      break;
    }

    default: {
      const auto id = RecordId();
      WriteNumber(buffer, id);
      break;
    }

  }
  const auto length = static_cast<uint32_t>(source.size());
  const LittleBuffer buff(length);
  const auto length_count = buffer.sputn(
      reinterpret_cast<const char*>(buff.data()),
              sizeof(length) );
  // total_count += length_count;
  const auto buffer_count = buffer.sputn(
          reinterpret_cast<const char*>(source.data()),
          static_cast<std::streamsize>(source.size()) );
  total_count += buffer_count;
  IncrementSample();            // Increment internal sample counter
  NofSamples(Sample());

  vlsd_index_ += length_count + buffer_count; // Happens to be the VLSD CG
                                              // length. So update it as well
  nof_data_bytes_ = static_cast<uint32_t>(total_count & 0xFFFFFFFF);
  nof_invalid_bytes_ = static_cast<uint32_t>(total_count >> 32);
  return vlsd_index; // Note it returns the old index
}

uint64_t Cg4Block::WriteCompressedVlsdSample(std::vector<uint8_t>& dest,
                                             uint8_t record_id_size,
                                   const std::vector<uint8_t> &buffer) {
  const uint64_t vlsd_index = vlsd_index_; // Temporary store the old index
  uint64_t total_count = nof_invalid_bytes_;
  total_count <<= 32;
  total_count += nof_data_bytes_;

  switch (record_id_size) {
    case 0:
      // No Record ID to store
      break;

    case 1: {
      const auto id = static_cast<uint8_t>(RecordId());
      const LittleBuffer ident(id);
      std::copy(ident.cbegin(), ident.cend(), std::back_inserter(dest));
      break;
    }

    case 2: {
      const auto id = static_cast<uint16_t>(RecordId());
      const LittleBuffer ident(id);
      std::copy(ident.cbegin(), ident.cend(), std::back_inserter(dest));
      break;
    }

    case 4: {
      const auto id = static_cast<uint32_t>(RecordId());
      const LittleBuffer ident(id);
      std::copy(ident.cbegin(), ident.cend(), std::back_inserter(dest));
      break;
    }

    default: {
      const auto id = RecordId();
      const LittleBuffer ident(id);
      std::copy(ident.cbegin(), ident.cend(), std::back_inserter(dest));
      break;
    }
  }

  const auto length = static_cast<uint32_t>(buffer.size());

  const LittleBuffer buff(length);
  std::copy(buff.cbegin(), buff.cend(), std::back_inserter(dest));

  std::copy(buffer.cbegin(), buffer.cend(), std::back_inserter(dest));
  total_count += buffer.size();
  IncrementSample();            // Increment internal sample counter
  NofSamples(Sample());

  vlsd_index_ += buff.size() + buffer.size();
  nof_data_bytes_ = static_cast<uint32_t>(total_count & 0xFFFFFFFF);
  nof_invalid_bytes_ = static_cast<uint32_t>(total_count >> 32);
  return vlsd_index; // Note it returns the old index
}

Cn4Block *Cg4Block::FindSdChannel() const {
  auto cn_list = Channels();
  const auto itr =
    std::find_if(cn_list.begin(), cn_list.end(),
      [](const auto* channel) {
    return channel != nullptr &&
           channel->Type() == ChannelType::VariableLength &&
           channel->VlsdRecordId() == 0;
  });

  if (itr == cn_list.end()) {
    return nullptr;
  }
  return dynamic_cast<Cn4Block*>(*itr);
}

Cn4Block *Cg4Block::FindVlsdChannel(uint64_t record_id) const {
  auto cn_list = Channels();
  const auto itr =
    std::find_if(cn_list.begin(), cn_list.end(),
                          [&] (const auto* channel) {
                            return channel != nullptr &&
                                   channel->Type() == ChannelType::VariableLength &&
                                   channel->VlsdRecordId() == record_id;
                          });

  if (itr == cn_list.end()) {
    return nullptr;
  }
  return dynamic_cast<Cn4Block*>(*itr);
}

const IDataGroup *Cg4Block::DataGroup() const {
  const auto* mdf_block = DgBlock();
  return mdf_block != nullptr ? dynamic_cast<const IDataGroup*>(mdf_block) : nullptr;
}

}  // namespace mdf::detail