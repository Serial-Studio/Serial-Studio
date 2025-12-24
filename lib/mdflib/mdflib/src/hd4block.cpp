/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "hd4block.h"

#include <algorithm>

#include "ixmlfile.h"
#include "dt4block.h"
#include "cn4block.h"
#include "sr4block.h"
#include "mdf/mdflogstream.h"

namespace {
// LINK index
constexpr size_t kIndexDg = 0;
constexpr size_t kIndexFh = 1;
constexpr size_t kIndexCh = 2;
constexpr size_t kIndexAt = 3;
constexpr size_t kIndexEv = 4;
constexpr size_t kIndexMd = 5;
// constexpr size_t kIndexNext = 0;

template <typename T>
T GetCommonProperty(const mdf::detail::Hd4Block& block,
                    const std::string& key) {
  const auto* md4 = block.Md4();
  if (md4 == nullptr || md4->IsTxtBlock()) {
    return {};
  }
  auto xml_file = mdf::CreateXmlFile("Expat");
  xml_file->ParseString(md4->Text());
  const auto* common = xml_file->GetNode("common_properties");
  if (common == nullptr) {
    return {};
  }
  const auto* key_node = common->GetNode("e", "name", key);
  return key_node == nullptr ? T{} : key_node->Value<T>();
}

template <typename T>
void SetCommonProperty(mdf::detail::Hd4Block& block, const std::string& key,
                       const T& value) {
  auto xml_file = mdf::CreateXmlFile("Expat");
  const auto* md4 = block.Md4();
  if (md4 != nullptr) {
    xml_file->ParseString(md4->Text());
  }
  auto& root = xml_file->RootName("HDcomment");
  root.AddUniqueNode("TX");  // Must be first in XML ???
  auto& common = root.AddUniqueNode("common_properties");
  auto& key_node = common.AddUniqueNode("e", "name", key);
  if (typeid(T) == typeid(int64_t)) {
    key_node.SetAttribute<std::string>("type", "integer");
  } else if (typeid(T) == typeid(float) || typeid(T) == typeid(double)) {
    key_node.SetAttribute<std::string>("type", "float");
  } else if (typeid(T) == typeid(bool)) {
    key_node.SetAttribute<std::string>("type", "boolean");
  } else if (typeid(T) == typeid(std::string)) {
    key_node.SetAttribute<std::string>("type", "string");
  }
  key_node.Value(value);
  block.Md4(xml_file->WriteString(true));
}

}  // namespace

namespace mdf::detail {

Hd4Block::Hd4Block() {
  block_type_ = "##HD";
  UtcTimestamp now(MdfHelper::NowNs());
  timestamp_.SetTime(now);
}

MdfBlock* Hd4Block::Find(int64_t index) const {
  if (index <= 0) {
    return nullptr;
  }

  for (auto& dg : dg_list_) {
    if (!dg) {
      continue;
    }
    auto* p = dg->Find(index);
    if (p != nullptr) {
      return p;
    }
  }
  for (auto& fh : fh_list_) {
    if (!fh) {
      continue;
    }
    auto* p = fh->Find(index);
    if (p != nullptr) {
      return p;
    }
  }

  for (auto& ch : ch_list_) {
    if (!ch) {
      continue;
    }
    auto* p = ch->Find(index);
    if (p != nullptr) {
      return p;
    }
  }

  for (auto& at : at_list_) {
    if (!at) {
      continue;
    }
    auto* p = at->Find(index);
    if (p != nullptr) {
      return p;
    }
  }

  for (auto& ev : ev_list_) {
    if (!ev) {
      continue;
    }
    auto* p = ev->Find(index);
    if (p != nullptr) {
      return p;
    }
  }

  return MdfBlock::Find(index);
}

void Hd4Block::GetBlockProperty(BlockPropertyList& dest) const {
  MdfBlock::GetBlockProperty(dest);

  dest.emplace_back("Links", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("First DG", ToHexString(Link(kIndexDg)),
                    "Link to first data group", BlockItemType::LinkItem);
  dest.emplace_back("First FH", ToHexString(Link(kIndexFh)),
                    "Link to first file history", BlockItemType::LinkItem);
  dest.emplace_back("First CH", ToHexString(Link(kIndexCh)),
                    "Link to first channel hierarchy", BlockItemType::LinkItem);
  dest.emplace_back("First AT", ToHexString(Link(kIndexAt)),
                    "Link to first attachment", BlockItemType::LinkItem);
  dest.emplace_back("First EV", ToHexString(Link(kIndexEv)),
                    "Link to first event", BlockItemType::LinkItem);
  dest.emplace_back("Comment MD", ToHexString(Link(kIndexMd)), Comment(),
                    BlockItemType::LinkItem);
  dest.emplace_back("", "", "", BlockItemType::BlankItem);

  dest.emplace_back("Information", "", "", BlockItemType::HeaderItem);
  dest.emplace_back("Description", Description());
  timestamp_.GetBlockProperty(dest);
  switch (static_cast<Hd4TimeClass>(time_class_)) {
    case Hd4TimeClass::LocalPcTime:
      dest.emplace_back("Time Source", "PC Time");
      break;

    case Hd4TimeClass::ExternalTime:
      dest.emplace_back("Time Source", "External");
      break;

    case Hd4TimeClass::ExternalNTP:
      dest.emplace_back("Time Source", "NTP/PTP");
      break;
  }
  dest.emplace_back("Flags", ToHexString(flags_));
  dest.emplace_back("Start Angle [rad]", ToString(start_angle_));
  dest.emplace_back("Start Distance [m]", ToString(start_distance_));

  if (md_comment_) {
    md_comment_->GetBlockProperty(dest);
  }
}

uint64_t Hd4Block::Read(std::streambuf& buffer) {
  uint64_t bytes = ReadHeader4(buffer);

  timestamp_.Init(*this);
  bytes += timestamp_.Read(buffer);

  bytes += ReadNumber(buffer, time_class_);
  bytes += ReadNumber(buffer, flags_);
  std::vector<uint8_t> reserved;
  bytes += ReadByte(buffer, reserved, 1);
  bytes += ReadNumber(buffer, start_angle_);
  bytes += ReadNumber(buffer, start_distance_);

  ReadLink4List(buffer, fh_list_, kIndexFh);
  ReadMdComment(buffer, kIndexMd);

  return bytes;
}

void Hd4Block::ReadMeasurementInfo(std::streambuf& buffer) {
  // We assume that the ID and HD block have been read (see ReadHeader)

  // Special handling of DG blocks.
  ReadLink4List(buffer, dg_list_, kIndexDg);
  for (auto& dg4 : dg_list_) {
    dg4->ReadCgList(buffer);
  }

  // Need to read in any attachments before we read in the channels as
  // they reference the attachments.
  ReadLink4List(buffer, at_list_, kIndexAt);
}

void Hd4Block::ReadEverythingButData(std::streambuf& buffer) {
  // We assume that ReadMeasurementInfo have been called earlier
  for (auto& dg4 : dg_list_) {
    if (!dg4) {
      continue;
    }
    for (auto& cg4 : dg4->Cg4()) {
      if (!cg4) {
        continue;
      }
      cg4->ReadCnList(buffer);
      cg4->ReadSrList(buffer);

      // Update the VLSD record id reference on all channels
      // and fix the MSLD channel as well
      // Update the storage type for the CG block.
      MdfStorageType storage_type = MdfStorageType::FixedLengthStorage;
      uint64_t max_length = 8;
      const auto channel_list = cg4->Channels();
      for (auto *channel : channel_list) {
        const auto *cn4 = dynamic_cast<const Cn4Block *>(channel);
        if (cn4 == nullptr || cn4->DataLink() == 0) {
          // Nothing to more of interest
          continue;
        }
        // Search for the block within the DG block
        const auto *block = Find(cn4->DataLink());
        if (block != nullptr && block->BlockType() == "CG"
            && cn4->Type() == ChannelType::VariableLength) {
          // VLSD CG Storage
          const auto *ref_cg = dynamic_cast<const Cg4Block *>(block);
          cn4->VlsdRecordId(ref_cg == nullptr ? 0 : ref_cg->RecordId());
          if (cn4->VlsdRecordId() > 0) {
            storage_type = MdfStorageType::VlsdStorage;
            max_length = 8;
          }
        } else if (block != nullptr && block->BlockType() == "CN") {
          // Point to the length of byte array channel
          if (const auto *mlsd_channel = dynamic_cast<const Cn4Block *>(block);
              mlsd_channel != nullptr) {
            // Check if the MLSD channel is valid. It must be of data type 0..3
            // and number of data bytes > 0.
            bool mlsd_valid = mlsd_channel->DataBytes() > 0;
            switch (mlsd_channel->DataType()) {
              case ChannelDataType::UnsignedIntegerLe:
              case ChannelDataType::UnsignedIntegerBe:
              case ChannelDataType::SignedIntegerLe:
              case ChannelDataType::SignedIntegerBe:
                break;

              default:
                mlsd_valid = false;
                break;
            }
            if (!mlsd_valid) {

               // Try to recover with data length channel
              if (const auto* data_length_channel = cg4->GetChannel(".DataLength");
                  data_length_channel != nullptr ) {
                mlsd_channel = dynamic_cast<const Cn4Block*>(data_length_channel);
                MDF_TRACE() << "Invalid MLSD data channel referenced. Changed to Data Channel: "
                            << data_length_channel->Name();
              } else {
                MDF_ERROR() << "Invalid MLSD data channel referenced. Data Channel: "
                            << mlsd_channel->Name();
              }
            }
            storage_type = MdfStorageType::MlsdStorage;
            max_length = cn4->DataBytes();
            cn4->MlsdChannel(mlsd_channel);
          }
        }
      }
      cg4->StorageType(storage_type);
      cg4->MaxLength(static_cast<uint32_t>(max_length));
    }

  }
  // Must read in all channels before creating CH block that references the CN
  // blocks
  ReadLink4List(buffer, ch_list_, kIndexCh);
  ReadLink4List(buffer, ev_list_, kIndexEv);
  // Need to scan through the event and hierarchy blocks to find the referenced
  // blocks
  for (auto& ch4 : ch_list_) {
    ch4->FindReferencedBlocks(*this);
  }
  for (auto& ev4 : ev_list_) {
    ev4->FindReferencedBlocks(*this);
  }
}

int64_t Hd4Block::Index() const { return FilePosition(); }

void Hd4Block::Author(const std::string& author) {
  SetCommonProperty(*this, "author", author);
}

std::string Hd4Block::Author() const {
  return GetCommonProperty<std::string>(*this, "author");
}

void Hd4Block::Department(const std::string& department) {
  SetCommonProperty(*this, "department", department);
}

std::string Hd4Block::Department() const {
  return GetCommonProperty<std::string>(*this, "department");
}

void Hd4Block::Project(const std::string& name) {
  SetCommonProperty(*this, "project", name);
}

std::string Hd4Block::Project() const {
  return GetCommonProperty<std::string>(*this, "project");
}

void Hd4Block::Subject(const std::string& subject) {
  SetCommonProperty(*this, "subject", subject);
}

std::string Hd4Block::Subject() const {
  return GetCommonProperty<std::string>(*this, "subject");
}

void Hd4Block::Description(const std::string& description) {
  auto xml_file = CreateXmlFile("Expat");
  const auto* md4 = Md4();
  if (md4 != nullptr) {
    xml_file->ParseString(md4->Text());
  }
  auto& root = xml_file->RootName("HDcomment");
  auto& tx_node = root.AddUniqueNode("TX");
  tx_node.Value(description);
  Md4(xml_file->WriteString(true));
}

std::string Hd4Block::Description() const { return Comment(); }

void Hd4Block::MeasurementId(const std::string& uuid) {
  SetCommonProperty(*this, "Measurement.UUID", uuid);
}

std::string Hd4Block::MeasurementId() const {
  return GetCommonProperty<std::string>(*this, "Measurement.UUID");
}

void Hd4Block::RecorderId(const std::string& uuid) {
  SetCommonProperty(*this, "Recorder.UUID", uuid);
}

std::string Hd4Block::RecorderId() const {
  return GetCommonProperty<std::string>(*this, "Recorder.UUID");
}

void Hd4Block::RecorderIndex(int64_t index) {
  SetCommonProperty(*this, "Recorder.FileIndex", index);
}

int64_t Hd4Block::RecorderIndex() const {
  return GetCommonProperty<int64_t>(*this, "Recorder.FileIndex");
}

void Hd4Block::StartTime(uint64_t ns_since_1970) {
  timestamp_.SetTime(ns_since_1970);
}

void Hd4Block::StartTime(ITimestamp& timestamp) {
  timestamp_.SetTime(timestamp);
}

uint64_t Hd4Block::StartTime() const { return timestamp_.GetTimeNs(); }

IMetaData* Hd4Block::CreateMetaData() {
  return MdfBlock::CreateMetaData();
}

IMetaData* Hd4Block::MetaData() const {
  return MdfBlock::MetaData();
}

IDataGroup* Hd4Block::LastDataGroup() const {
  return dg_list_.empty() ? nullptr : dg_list_.back().get();
}

IAttachment* Hd4Block::CreateAttachment() {
  auto at4 = std::make_unique<At4Block>();
  at4->Init(*this);
  AddAt4(at4);
  return at_list_.empty() ? nullptr : at_list_.back().get();
}

std::vector<IAttachment*> Hd4Block::Attachments() const {
  std::vector<IAttachment*> list;
  std::transform(at_list_.cbegin(), at_list_.cend(), std::back_inserter(list),
                         [](const auto& at4) { return at4.get(); });
  return list;
}

IFileHistory* Hd4Block::CreateFileHistory() {
  auto fh4 = std::make_unique<Fh4Block>();
  AddFh4(fh4);
  return fh_list_.empty() ? nullptr : fh_list_.back().get();
}

std::vector<IFileHistory*> Hd4Block::FileHistories() const {
  std::vector<IFileHistory*> list;
  std::transform(fh_list_.cbegin(), fh_list_.cend(), std::back_inserter(list),
                         [](const auto& fh4) { return fh4.get(); });
  return list;
}

IEvent* Hd4Block::CreateEvent() {
  auto ev4 = std::make_unique<Ev4Block>();
  ev4->Init(*this);
  ev_list_.push_back(std::move(ev4));
  return ev_list_.empty() ? nullptr : ev_list_.back().get();
}

std::vector<IEvent*> Hd4Block::Events() const {
  std::vector<IEvent*> list;
  std::transform(ev_list_.cbegin(),ev_list_.cend(), std::back_inserter(list),
                         [](const auto& ev4) { return ev4.get(); });
  return list;
}

IChannelHierarchy* Hd4Block::CreateChannelHierarchy() {
  auto ch4 = std::make_unique<Ch4Block>();
  ch4->Init(*this);
  ch_list_.push_back(std::move(ch4));
  return ch_list_.empty() ? nullptr : ch_list_.back().get();
}

std::vector<IChannelHierarchy*> Hd4Block::ChannelHierarchies() const {
  std::vector<IChannelHierarchy*> list;
  std::transform(ch_list_.cbegin(), ch_list_.cend(), std::back_inserter(list),
                         [](const auto& ch4) { return ch4.get(); });
  return list;
}

IDataGroup* Hd4Block::CreateDataGroup() {
  auto dg4 = std::make_unique<Dg4Block>();
  dg4->Init(*this);
  dg_list_.push_back(std::move(dg4));
  return dg_list_.empty() ? nullptr : dg_list_.back().get();
}

std::vector<IDataGroup*> Hd4Block::DataGroups() const {
  std::vector<IDataGroup*> list;
  std::transform(dg_list_.cbegin(), dg_list_.cend(), std::back_inserter(list),
                         [](const auto& dg) { return dg.get(); });
  return list;
}

void Hd4Block::AddDg4(std::unique_ptr<Dg4Block>& dg4) {
  if (dg4) {
    dg4->Init(*this);
    dg_list_.push_back(std::move(dg4));
  }
}

void Hd4Block::AddAt4(std::unique_ptr<At4Block>& at4) {
  if (at4) {
    at4->Init(*this);
    at_list_.push_back(std::move(at4));
  }
}

void Hd4Block::AddFh4(std::unique_ptr<Fh4Block>& fh4) {
  if (fh4) {
    fh4->Init(*this);
    fh_list_.push_back(std::move(fh4));
  }
}

void Hd4Block::StartAngle(double angle) {
  flags_ |= Hd4Flags::kStartAngleValid;
  start_angle_ = angle;
}

std::optional<double> Hd4Block::StartAngle() const {
  if ((flags_ & Hd4Flags::kStartAngleValid) != 0) {
    return {start_angle_};
  }
  return {};
}

void Hd4Block::StartDistance(double distance) {
  flags_ |= Hd4Flags::kStartDistanceValid;
  start_distance_ = distance;
}

std::optional<double> Hd4Block::StartDistance() const {
  if ((flags_ & Hd4Flags::kStartDistanceValid) != 0) {
    return {start_distance_};
  }
  return {};
}

uint64_t Hd4Block::Write(std::streambuf& buffer) {
  const bool update =
      FilePosition() > 0;  // Write or update the values inside the block
  if (!update) {
    block_type_ = "##HD";
    block_length_ = 24 + (6 * 8) + 8 + 4 + 4 + 1 + 1 + 1 + 1 + 8 + 8;
    link_list_.resize(6, 0);
  }
  uint64_t bytes = update ? MdfBlock::Update(buffer) : MdfBlock::Write(buffer);

  // These values may change after the initial write
  bytes += timestamp_.Write(buffer);
  bytes += WriteNumber(buffer, time_class_);
  bytes += WriteNumber(buffer, flags_);
  bytes += WriteBytes(buffer, 1);
  bytes += WriteNumber(buffer, start_angle_);
  bytes += WriteNumber(buffer, start_distance_);

  if (!update) {
    UpdateBlockSize(buffer, bytes);
  }
  WriteLink4List(buffer, fh_list_, kIndexFh,
                 UpdateOption::DoNotUpdateWrittenBlock);
  WriteMdComment(buffer, kIndexMd);
  WriteLink4List(buffer, at_list_, kIndexAt,
                 UpdateOption::DoNotUpdateWrittenBlock);
  WriteLink4List(buffer, ch_list_, kIndexCh,
                 UpdateOption::DoNotUpdateWrittenBlock);
  WriteLink4List(buffer, ev_list_, kIndexEv,
                 UpdateOption::DoNotUpdateWrittenBlock);

  // Always write the list of DG last as it may contain a DT block that shall
  // be appended with data bytes. The DT block must be the last written block
  // in these cases.
  WriteLink4List(buffer, dg_list_, kIndexDg,
       UpdateOption::DoUpdateAllBlocks);  // Always rewrite last DG block
  return bytes;
}

bool Hd4Block::FinalizeDtBlocks(std::streambuf& buffer) const {
  if (dg_list_.empty()) {
    return true;
  }
  Dg4Block* last_dg = dg_list_.back().get();
  return last_dg != nullptr && last_dg->FinalizeDtBlocks(buffer);
}

bool Hd4Block::FinalizeCgAndVlsdBlocks(std::streambuf& buffer, bool update_cg,
                                     bool update_vlsd) const {
  if (dg_list_.empty()) {
    return true;
  }
  auto* last_dg = dg_list_.back().get();
  return last_dg != nullptr &&
         last_dg->FinalizeCgAndVlsdBlocks(buffer, update_cg, update_vlsd);
}

const IMdfTimestamp* Hd4Block::StartTimestamp() const {
  return &timestamp_;
}

}  // namespace mdf::detail