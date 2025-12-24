/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "dg4block.h"

#include <stdexcept>
#include <algorithm>
#include <map>

#include "dl4block.h"
#include "dt4block.h"
#include "dz4block.h"
#include "hl4block.h"
#include "cn4block.h"
#include "sr4block.h"
#include "mdf/mdflogstream.h"
#include "readcache.h"

namespace {
constexpr size_t kIndexCg = 1;
constexpr size_t kIndexData = 2;
constexpr size_t kIndexMd = 3;
constexpr size_t kIndexNext = 0;

struct CgCounter {
  mdf::detail::Cg4Block* CgBlock = nullptr;
  uint64_t NofSamples = 0;
  uint64_t NofBytes = 0;
};

///< Helper function that recursively copies all data bytes to a
/// destination file.
uint64_t CopyDataToFile(
    const mdf::detail::DataListBlock::BlockList& block_list,  // NOLINT
    std::streambuf& from_file, std::streambuf& to_file) {
  uint64_t count = 0;
  for (const auto& block : block_list) {
    if (!block) {
      continue;
    }
    const auto* db = dynamic_cast<const mdf::detail::DataBlock*>(block.get());
    const auto* dl =
        dynamic_cast<const mdf::detail::DataListBlock*>(block.get());
    if (db != nullptr) {
      count += db->CopyDataToFile(from_file, to_file);
    } else if (dl != nullptr) {
      count += CopyDataToFile(dl->DataBlockList(), from_file, to_file);
    }
  }
  return count;
}

}  // namespace

namespace mdf::detail {

Dg4Block::Dg4Block() { block_type_ = "##DG"; }
IChannelGroup* Dg4Block::CreateChannelGroup() {
  auto cg4 = std::make_unique<Cg4Block>();
  cg4->Init(*this);
  AddCg4(cg4);
  return cg_list_.empty() ? nullptr : cg_list_.back().get();
}

MdfBlock* Dg4Block::Find(int64_t index) const {
  for (auto& cg : cg_list_) {
    if (!cg) {
      continue;
    }
    auto* p = cg->Find(index);
    if (p != nullptr) {
      return p;
    }
  }
  return DataListBlock::Find(index);
}

void Dg4Block::GetBlockProperty(BlockPropertyList& dest) const {
  MdfBlock::GetBlockProperty(dest);

  dest.emplace_back("Links", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Next DG", ToHexString(Link(kIndexNext)),
                    "Link to next data group", BlockItemType::LinkItem);
  dest.emplace_back("First CG", ToHexString(Link(kIndexCg)),
                    "Link to first channel group", BlockItemType::LinkItem);
  dest.emplace_back("Link Data", ToHexString(Link(kIndexData)), "Link to Data",
                    BlockItemType::LinkItem);
  dest.emplace_back("Comment MD", ToHexString(Link(kIndexMd)), Comment(),
                    BlockItemType::LinkItem);
  dest.emplace_back("", "", "", BlockItemType::BlankItem);

  dest.emplace_back("Information", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Record ID Size [byte]", std::to_string(rec_id_size_));
  if (md_comment_) {
    md_comment_->GetBlockProperty(dest);
  }
}

uint64_t Dg4Block::Read(std::streambuf& buffer) {
  uint64_t bytes = ReadHeader4(buffer);
  bytes += ReadNumber(buffer, rec_id_size_);
  std::vector<uint8_t> reserved;
  bytes += ReadByte(buffer, reserved, 7);

  ReadMdComment(buffer, kIndexMd);
  ReadBlockList(buffer, kIndexData);

  return bytes;
}

uint64_t Dg4Block::Write(std::streambuf& buffer) {
  const bool update = FilePosition() > 0;  // True if already written to file
  if (!update) {
    block_type_ = "##DG";
    block_length_ = 24 + (4 * 8) + 8;
    link_list_.resize(4, 0);
  }

  WriteLink4List(buffer, cg_list_, kIndexCg,
              UpdateOption::DoUpdateAllBlocks); // Save nof samples in CG block
  WriteMdComment(buffer, kIndexMd);


  uint64_t bytes = update ? MdfBlock::Update(buffer) : MdfBlock::Write(buffer);
  if (update) {
    bytes = block_length_;
  } else {
    bytes += WriteNumber(buffer, rec_id_size_);
    bytes += WriteBytes(buffer, 7);
    UpdateBlockSize(buffer, bytes);
    // Need to update the signal data link for any channels referencing
    // VLSD CG blocks.
    for (auto& cg4 : cg_list_) {
      if (cg4 && (cg4->Flags() & CgFlag::VlsdChannel) ) {
        // The previous CG block has a channel which needs to update its
        // signal data link.
        auto* other_cg4 = FindCgRecordId(cg4->RecordId() -1);
        auto* cn4 = other_cg4 != nullptr ?
                 other_cg4->FindVlsdChannel(cg4->RecordId()) : nullptr;
        if (cn4 != nullptr) {
          cn4->UpdateDataLink(buffer, cg4->FilePosition());
        }
      }
    }
  }
  // Need to write any data block, so it is positioned last in the file
  // as any DT block should be appended with data bytes. The DT block must be
  // last in the file
  WriteLink4List(buffer, block_list_, kIndexData,
                 UpdateOption::DoUpdateOnlyLastBlock); // Update last HL or DT
  return bytes;
}

uint64_t Dg4Block::DataSize() const { return DataListBlock::DataSize(); }

Hl4Block* Dg4Block::CreateOrGetHl4() {
  if (DataBlockList().empty()) {
    auto& hl4_list = DataBlockList();
    // Add a HL4 block
    auto hl_block = std::make_unique<Hl4Block>();
    hl_block->Init(*this);
    hl_block->Flags(0);
    hl_block->Type(Hl4ZipType::Deflate);
    hl4_list.push_back(std::move(hl_block));
  }
  return dynamic_cast<Hl4Block*>(DataBlockList().back().get());
}

Dt4Block *Dg4Block::CreateOrGetDt4(std::streambuf& buffer) {
  // The data block list should include a DT block that we later will append
  // samples (buffers) to.
  auto& block_list = DataBlockList();
  if (block_list.empty()) {
    auto dt4 = std::make_unique<Dt4Block>();
    dt4->Init(*this);
    //buffer.pubseekoff(0, std::ios_base::end);

    //if (Link(2) > 0) {
    //  return nullptr;
    //}
    if (Link(kIndexData) == 0 && FilePosition() > 0) {
      SetLastFilePosition(buffer);
      dt4->Write(buffer);
      UpdateLink(buffer, kIndexData, dt4->FilePosition());
      dt4->DataPosition( 24 + dt4->FilePosition());
    }

/*const auto position = GetFilePosition(buffer);
    SetLastFilePosition(buffer);
    const int64_t data_position = GetFilePosition(buffer);
    dt4->DataPosition(data_position);
*/
    block_list.push_back(std::move(dt4));
  }
  return dynamic_cast<Dt4Block*>(block_list.back().get());

}
void Dg4Block::ReadCgList(std::streambuf& buffer) {
  ReadLink4List(buffer, cg_list_, kIndexCg);
}

void Dg4Block::ReadData(std::streambuf& buffer) {

  const auto& block_list = DataBlockList();
  if (block_list.empty()) {
    return;
  }
  InitFastObserverList();
  // First scan through all CN blocks and set up any VLSD CG or MLSD channel
  // relations.

  for (const auto& cg4 : cg_list_) {
    if (!cg4) {
      continue;
    }

    // Fetch all data from sample reduction blocks (SR).
    for (const auto& sr4 : cg4->Sr4()) {
      if (!sr4) {
        continue;
      }
      sr4->ReadData(buffer);
    }

    // Fetch all signal data (SD)
    const auto channel_list = cg4->Channels();
    for (const auto* channel :channel_list) {
      if (channel == nullptr) {
        continue;
      }
      // Check if the channel is in the subscription
      if (!IsSubscribingOnChannel(*channel) ) {
        continue;
      }
      const auto* cn_block = dynamic_cast<const Cn4Block*>(channel);
      if (cn_block == nullptr) {
        continue;
      }
      // Fetch the channels referenced data block.
      // Note that some types of
      // data blocks are owned by this channel as a SD block, but some are only
      // references to block own by another block. Of interest is VLSD CG block
      // and MLSD channel.
      const auto data_link = cn_block->DataLink();
      if (data_link == 0) {
        continue; // No data to read into the system
      }
      const auto* block = Find(data_link);
      if (block == nullptr) {
        if (const auto* header = HeaderBlock(); header != nullptr) {
          block = header->Find(data_link);
        }
      }

      if (block == nullptr) {
        MDF_DEBUG() << "Missing data block in channel. Channel :"
                    << cn_block->Name()
                    << ", Data Link: " << data_link;

        continue; // Strange that the block doesn't exist
      }

      switch (cn_block->Type()) {
        case ChannelType::VariableLength:
          if (IsSubscribingOnChannelVlsd(*channel)) {
            if (block->BlockType() == "SD") {
              cn_block->ReadSignalData(buffer);
            } else if (block->BlockType() == "DZ") {
              cn_block->ReadSignalData(buffer);
            } else if (block->BlockType() == "DL") {
              cn_block->ReadSignalData(buffer);
            } else if (block->BlockType() == "HL") {
              cn_block->ReadSignalData(buffer);
            }
          }
          break;

        default:
          break;
      }
    }
  }

/*
  // Convert everything to a samples in a file single DT block can be read
  // directly, but the remaining block types are streamed to a temporary file.
  // The
  // main reason is that linked data blocks are not aligned to a record or even
  // worse a channel value bytes. Converting everything to a simple DT block
  // solves that problem.

  bool close_data_file = false; // If a DT block does not close the data file.
  std::FILE* data_file = nullptr;
  size_t data_size = 0;
  if (block_list.size() == 1 && block_list[0] &&
      block_list[0]->BlockType() == "DT") {  // If DT read from file directly
    const auto* dt = dynamic_cast<const Dt4Block*>(block_list[0].get());
    if (dt != nullptr) {
      SetFilePosition(file, dt->DataPosition());
      data_file = file;
      data_size = dt->DataSize();
    }
  } else {
    // If not a DT block, extract the linked data block list into temporary
    // file which is a DT block according to the above DT block.
    close_data_file = true;
    data_file = std::tmpfile();
    data_size = CopyDataToFile(block_list, file, data_file);
    std::rewind(data_file);  // SetFilePosition(data_file,0);
  }

  // Now, it is time to scan through and do a call-back for each record
  // auto pos = GetFilePosition(data_file);
  // Read through all record
  ParseDataRecords(data_file, data_size);
  if (data_file != nullptr && close_data_file) {
    fclose(data_file);
  }
*/

  for (const auto& channel_group : Cg4() ) {
    if (channel_group) {
      channel_group->ResetSampleCounter();
    }
  }
  ReadCache read_cache(this, buffer);
  while (read_cache.ParseRecord()) {
  }

  for (const auto& cg : cg_list_) {
    if (!cg) {
      continue;
    }
    for (const auto& cn : cg->Cn4()) {
      if (!cn) {
        continue;
      }
      cn->ClearData();
    }
  }
}

void Dg4Block::ReadRangeData(std::streambuf& buffer, DgRange& range) {
  const auto& block_list = DataBlockList();
  if (block_list.empty()) {
    return;
  }

  InitFastObserverList();

  // First scan through all CN blocks and set up any VLSD CG or MLSD channel
  // relations.
  for (const auto& cg4 : cg_list_) {
    if (!cg4) {
      continue;
    }
    const uint64_t record_id = cg4->RecordId();
    const bool cg_used = range.IsUsed(record_id);
    // Do not read SD or SD data if the channel group isn't requested.
    if (!cg_used) {
      continue;
    }
    // Fetch all data from sample reduction blocks (SR), but only if the
    // channel group data is subscribed.
    for (const auto& sr4 : cg4->Sr4()) {
      if (!sr4) {
        continue;
      }
      sr4->ReadData(buffer);
    }

    // Fetch all signal data (SD)
    const auto channel_list = cg4->Channels();
    for (const auto* channel :channel_list) {
      if (channel == nullptr) {
        continue;
      }
      // Check if the channel is in the subscription
      if (!IsSubscribingOnChannel(*channel) ) {
        continue;
      }
      const auto* cn_block = dynamic_cast<const Cn4Block*>(channel);
      if (cn_block == nullptr) {
        continue;
      }
      // Fetch the channels referenced data block. Note that some types of
      // data blocks are owned by this channel as an SD block, but some are only
      // references to block own by another block. Of interest is VLSD CG block
      // and MLSD channel.
      const auto data_link = cn_block->DataLink();
      if (data_link == 0) {
        continue; // No data to read into the system
      }
      const auto* block = Find(data_link);
      if (block == nullptr) {
        MDF_DEBUG() << "Missing data block in channel. Channel :"
                    << cn_block->Name()
                    << ", Data Link: " << data_link;

        continue; // Strange that the block doesn't exist
      }

      switch (cn_block->Type()) {
        case ChannelType::VariableLength:
          if (IsSubscribingOnChannelVlsd(*channel)) {
            if (block->BlockType() == "SD") {
              cn_block->ReadSignalData(buffer);
            } else if (block->BlockType() == "DZ") {
              cn_block->ReadSignalData(buffer);
            } else if (block->BlockType() == "DL") {
              cn_block->ReadSignalData(buffer);
            } else if (block->BlockType() == "HL") {
              cn_block->ReadSignalData(buffer);
            }
          }
          break;

        default:
          break;
      }
    }
  }

  // Reset the internal sample counter and initial the read cache
  for (const auto& channel_group : Cg4() ) {
    if (channel_group) {
      channel_group->ResetSampleCounter();
    }
  }
  ReadCache read_cache(this, buffer);
  while (read_cache.ParseRangeRecord(range)) {
  }

  for (const auto& cg : cg_list_) {
    if (!cg) {
      continue;
    }
    for (const auto& cn : cg->Cn4()) {
      if (!cn) {
        continue;
      }
      cn->ClearData();
    }
  }
}

void  Dg4Block::ReadVlsdData(std::streambuf& buffer,Cn4Block& channel,
                  const std::vector<uint64_t>& offset_list,
                  std::function<void(uint64_t,
                              const std::vector<uint8_t>&)>& callback) {

  // Fetch the channels referenced data block. Note that some types of
  // data blocks are owned by this channel as an SD block, but some are only
  // references to block own by another block. Of interest is VLSD CG block
  // and MLSD channel.

  const auto data_link = channel.DataLink();
  if (data_link == 0) {
    return; // No data to read into the system
  }
  InitFastObserverList();
  auto* block = Find(data_link);
  if (block == nullptr) {
    throw std::runtime_error(
        "Missing referenced data block in channel.");
  }
  bool read_signal_data = false;  // Traditional SD storage
  bool read_vlsd_cg_data = false; // Newer CG-VLSD data

  switch (channel.Type()) {
    case ChannelType::VariableLength:
      if (block->BlockType() == "CG") {
        const auto* cg_block = dynamic_cast<const Cg4Block*>(block);
        if (cg_block != nullptr &&
            (cg_block->Flags() & CgFlag::VlsdChannel) != 0 ) {
          read_vlsd_cg_data = true;
        }
      } else if (block->BlockType() == "SD") {
        read_signal_data = true;
      } else if (block->BlockType() == "DZ") {
        read_signal_data = true;
      } else if (block->BlockType() == "DL") {
        read_signal_data = true;
      }
      break;

    default:
      throw std::runtime_error("The channel is not a VLSD type.");
  }

  if (read_signal_data) {
    ReadCache read_cache(&channel, buffer);
    read_cache.SetOffsetFilter(offset_list);
    read_cache.SetCallback(callback);
    const std::set<uint64_t>& offset_filter = read_cache.GetSortedOffsetList();
    if (!offset_filter.empty()) {
      for (uint64_t offset : offset_filter) {
        const bool more = read_cache.ParseSignalDataOffset(offset);
        if (!more) {
          break;
        }
      }
    } else {
      while (read_cache.ParseSignalData()) { }
    }
  } else if (read_vlsd_cg_data) {
    ReadCache read_cache(this, buffer);
    read_cache.SetRecordId(channel.VlsdRecordId());
    read_cache.SetOffsetFilter(offset_list);
    read_cache.SetCallback(callback);

    while (read_cache.ParseVlsdCgData()) {
    }
  }

}

void Dg4Block::ParseDataRecords(std::streambuf& buffer, uint64_t nof_data_bytes) {
  if (nof_data_bytes == 0) {
    return;
  }
  for (const auto& channel_group : Cg4() ) {
    if (channel_group) {
      channel_group->ResetSampleCounter();
    }
  }

  for (uint64_t count = 0; count < nof_data_bytes; /* No ++count here*/) {
    // 1. Read Record ID
    uint64_t record_id = 0;
    count += ReadRecordId(buffer, record_id);

    const auto* cg4 = FindCgRecordId(record_id);
    if (cg4 == nullptr) {
      break;
    }
    const auto read = cg4->ReadDataRecord(buffer, *this);
    if (read == 0) {
      break;
    }

    count += read;
  }
}

uint64_t Dg4Block::ReadRecordId(std::streambuf& buffer, uint64_t& record_id) const {
  record_id = 0;

  uint64_t count = 0;
  switch (rec_id_size_) {
    case 1: {
      uint8_t id = 0;
      count += ReadNumber(buffer, id);
      record_id = id;
      break;
    }

    case 2: {
      uint16_t id = 0;
      count += ReadNumber(buffer, id);
      record_id = id;
      break;
    }

    case 4: {
      uint32_t id = 0;
      count += ReadNumber(buffer, id);
      record_id = id;
      break;
    }

    case 8: {
      uint64_t id = 0;
      count += ReadNumber(buffer, id);
      record_id = id;
      break;
    }
    default:
      break;
  }
  return count;
}

Cg4Block* Dg4Block::FindCgRecordId(uint64_t record_id) const {
  if (cg_list_.size() == 1) {
    return cg_list_[0].get();
  }
  for (const auto& cg : cg_list_) {
    if (!cg) {
      continue;
    }
    if (cg->RecordId() == record_id) {
      return cg.get();
    }
  }
  return nullptr;
}

std::vector<IChannelGroup*> Dg4Block::ChannelGroups() const {
  std::vector<IChannelGroup*> list;
  for (const auto& cg : cg_list_) {
    if (cg) {
      list.emplace_back(cg.get());
    }
  }
  return list;
}

void Dg4Block::AddCg4(std::unique_ptr<Cg4Block>& cg4) {
  cg4->Init(*this);
  cg4->RecordId(0);
  cg_list_.push_back(std::move(cg4));

  if (cg_list_.size() < 2) {
    rec_id_size_ = 0;
  } else if (cg_list_.size() < 0x100) {
    rec_id_size_ = 1;
  } else if (cg_list_.size() < 0x10000) {
    rec_id_size_ = 2;
  } else if (cg_list_.size() < 0x100000000) {
    rec_id_size_ = 4;
  } else {
    rec_id_size_ = 8;
  }

  uint64_t id4 = cg_list_.size() < 2 ? 0 : 1;
  for (auto& group : cg_list_) {
    if (group) {
      group->RecordId(id4++);
    }
  }
}

int64_t Dg4Block::Index() const { return FilePosition(); }

IMetaData* Dg4Block::CreateMetaData() {
  return MdfBlock::CreateMetaData();
}

IMetaData* Dg4Block::MetaData() const {
  return MdfBlock::MetaData();
}

void Dg4Block::Description(const std::string& desc) {
  auto* md4 = CreateMetaData();
  if (md4 != nullptr) {
    md4->StringProperty("TX", desc);
  }
}
std::string Dg4Block::Description() const {
  return Comment();
}

void Dg4Block::RecordIdSize(uint8_t id_size) { rec_id_size_ = id_size; }

uint8_t Dg4Block::RecordIdSize() const { return rec_id_size_; }

bool Dg4Block::FinalizeDtBlocks(std::streambuf& buffer) {
  auto& block_list = DataBlockList();
  if (block_list.empty()) {
    // No data blocks to update
    MDF_DEBUG() << "No last data block to update.";
    return true;
  }
  auto* last_block = block_list.back().get();
  if (last_block == nullptr || last_block->BlockType() != "DT") {
    MDF_DEBUG() << "Last data block is not a DT block.";
    return true;
  }
  auto* dt_block = dynamic_cast<Dt4Block*>(last_block);
  if (dt_block == nullptr) {
    MDF_ERROR() << "Invalid DT block type-cast.";
    return false;
  }
  dt_block->UpdateDataSize(buffer);

  return true;
}

// Update the unfinished payload data (DT) block. This function updates the
// channel group (CG) and a CG-VLSD channel group regarding cycle count
// and offsets (VLSD)
bool Dg4Block::FinalizeCgAndVlsdBlocks(std::streambuf& buffer, bool update_cg,
                                     bool update_vlsd) {
  auto& block_list = DataBlockList();
  if (block_list.empty()) {
    // No data blocks to update
    MDF_DEBUG() << "No last data block to update.";
    return true;
  }
  // It is the last DT block in the list that can be updated. If it isn't a DT
  // block, we cannot finish the file.
  auto* last_block = block_list.back().get();
  if (last_block == nullptr || last_block->BlockType() != "DT") {
    MDF_DEBUG() << "Last data block is not a DT block.";
    return true;
  }
  auto* dt_block = dynamic_cast<Dt4Block*>(last_block);
  if (dt_block == nullptr) {
    MDF_ERROR() << "Invalid DT block type-cast.";
    return false;
  }
    // Create a simple map that speeds up the counting.
  std::map<uint64_t, CgCounter> counter_list;
  for (const auto& cg4 : Cg4()) {
    if (!cg4) {
      continue;
    }
    CgCounter counter;
    counter.CgBlock = cg4.get();
    counter_list.emplace(cg4->RecordId(), counter);
  }
  if (counter_list.empty()) {
    return true; // Nothing to update
  }


  SetFilePosition(buffer, dt_block->DataPosition());
  uint64_t count = 0;
  while (count < dt_block->DataSize()) {
    uint64_t record_id = 0;
    count += ReadRecordId(buffer,record_id);
    auto cg_find = counter_list.size() == 1 ?
           counter_list.begin() : counter_list.find(record_id);
    if (cg_find == counter_list.end()) {
      MDF_DEBUG() << "Failed to find the CG block. Record ID: " << record_id;
      break;
    }
    auto& counter = cg_find->second;
    auto* cg_block = counter.CgBlock;
    uint64_t count_bytes = 0;
    if (cg_block == nullptr) {
      MDF_ERROR() << "CG block pointer is null. Internal error.";
      return false;
    }
    if ((cg_block->Flags() & CgFlag::VlsdChannel) != 0) {
      // This is normally used for string,
      // and the CG block only includes one signal
      uint32_t length = 0;
      count_bytes += ReadNumber(buffer, length);
      //std::vector<uint8_t> record(length, 0);
      if (length > 0) {
        count_bytes += StepFilePosition(buffer, length);
      }
    } else {
      // Normal fixed length records
      const size_t record_size = cg_block->NofDataBytes() + cg_block->NofInvalidBytes();
      count_bytes += StepFilePosition(buffer, record_size);
    }
    ++counter.NofSamples;
    counter.NofBytes += count_bytes;
    count += count_bytes;
  }
  for (const auto& [record_id_1, counter1] : counter_list) {
    Cg4Block* cg_block = counter1.CgBlock;
    if (cg_block == nullptr) {
      continue;
    }
    if (update_cg) {
      cg_block->UpdateCycleCounter(counter1.NofSamples);
    }
    if (update_vlsd) {
      cg_block->UpdateVlsdSize(counter1.NofBytes);
    }
  }
  return true;
}

IChannelGroup* Dg4Block::FindParentChannelGroup(const IChannel&
                                                      channel) const {
  const auto channel_index = channel.Index();
  const auto &cg_list = Cg4();
  const auto itr = std::find_if(cg_list.cbegin(), cg_list.cend(),
                                [&](const auto &cg_block) {
    return cg_block && cg_block->Find(channel_index) != nullptr;
  });
  return itr != cg_list.cend() ? itr->get() : nullptr;
}

void Dg4Block::ClearData() {
  DataListBlock::ClearData();
  IDataGroup::ClearData();
  for (auto& cg4 : Cg4()) {
    if (!cg4) {
      continue;
    }
    for (auto& sr4 : cg4->Sr4()) {
      if (sr4) {
        sr4->ClearData();
      }
    }

    for (auto& cn4 : cg4->Cn4()) {
      if (cn4) {
        cn4->ClearData();
      }
    }
  }
}

}  // namespace mdf::detail