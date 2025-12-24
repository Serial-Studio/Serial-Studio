/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "mdf/mdfreader.h"

#include <cstdio>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "channelobserver.h"
#include "dg4block.h"
#include "idblock.h"
#include "mdf/mdflogstream.h"
#include "mdf3file.h"
#include "mdf4file.h"
#include "platform.h"
#include "cn4block.h"
#include "sr4block.h"
#include "sr3block.h"
#include "dgrange.h"

#if INCLUDE_STD_FILESYSTEM_EXPERIMENTAL
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif

using namespace std::chrono_literals;
using namespace mdf::detail;

namespace mdf {

bool IsMdfFile(const std::string &filename) {
  std::filebuf file;

  try {
    file.open(filename, std::ios_base::in | std::ios_base::binary);
  } catch (const std::exception&) {
    return false;
  }

  if (!file.is_open()) {
    return false;
  }
  const bool is_mdf = IsMdfFile(file);
  file.close();
  return is_mdf;
}

bool IsMdfFile(std::streambuf &buffer) {

  detail::IdBlock oId;
  bool bError = true;
  try {
    uint64_t bytes = oId.Read(buffer);
    if (bytes == 64) {
      bError = false;
    }
  } catch (const std::exception &) {
    bError = true;
  }
  if (bError) {
    return false;
  }
  if (Platform::strnicmp(oId.FileId().c_str(), "MDF", 3) == 0 ||
      Platform::strnicmp(oId.FileId().c_str(), "UnFinMF", 7) == 0) {
    return true;
  }

  return false;
}

/// Creates a channel sample observer. The sample observer creates internal
/// memory for all samples.\n The function also attach the observer to the
/// notifier (see observer pattern). The destructor of the channel observer
/// detach the observer. \param data_group \param group \param channel \return
/// Smart pointer to a channel observer
ChannelObserverPtr CreateChannelObserver(const IDataGroup &data_group,
                                         const IChannelGroup &group,
                                         const IChannel &channel) {
  std::unique_ptr<IChannelObserver> observer;

  // Note, that num,ber of bytes returns number of bytes for
  // all value in the array
  const auto array_size = channel.ArraySize();
  const auto value_size = channel.DataBytes();

  switch (channel.DataType()) {
    case ChannelDataType::UnsignedIntegerLe:
    case ChannelDataType::UnsignedIntegerBe:
      switch (value_size) {
        case 1:
          observer = std::make_unique<detail::ChannelObserver<uint8_t>>(
              data_group, group, channel);
          break;

          case 2:
          observer = std::make_unique<detail::ChannelObserver<uint16_t>>(
              data_group, group, channel);
          break;

          case 4:
          observer = std::make_unique<detail::ChannelObserver<uint32_t>>(
              data_group, group, channel);
          break;

          case 8:
          default:
          observer = std::make_unique<detail::ChannelObserver<uint64_t>>(
              data_group, group, channel);
          break;
      }
      break;

    case ChannelDataType::SignedIntegerLe:
    case ChannelDataType::SignedIntegerBe:
      if (value_size <= 1) {
        observer = std::make_unique<detail::ChannelObserver<int8_t>>(
            data_group, group, channel);
      } else if (value_size <= 2) {
        observer = std::make_unique<detail::ChannelObserver<int16_t>>(
            data_group, group, channel);
      } else if (value_size <= 4) {
        observer = std::make_unique<detail::ChannelObserver<int32_t>>(
            data_group, group, channel);
      } else {
        observer = std::make_unique<detail::ChannelObserver<int64_t>>(
            data_group, group, channel);
      }
      break;

    case ChannelDataType::FloatLe:
    case ChannelDataType::FloatBe:
      if (value_size <= 4) {
        observer = std::make_unique<detail::ChannelObserver<float>>(
            data_group, group, channel);
      } else {
        observer = std::make_unique<detail::ChannelObserver<double>>(
            data_group, group, channel);
      }
      break;

    case ChannelDataType::StringUTF16Le:
    case ChannelDataType::StringUTF16Be:
    case ChannelDataType::StringUTF8:
    case ChannelDataType::StringAscii:
      observer = std::make_unique<detail::ChannelObserver<std::string>>(
          data_group, group, channel);
      break;

    case ChannelDataType::MimeStream:
    case ChannelDataType::MimeSample:
    case ChannelDataType::ByteArray:
      observer =
          std::make_unique<detail::ChannelObserver<std::vector<uint8_t>>>(
              data_group, group, channel);
      break;

    case ChannelDataType::CanOpenDate:  // Convert to ms since 1970
    case ChannelDataType::CanOpenTime:
      observer = std::make_unique<detail::ChannelObserver<uint64_t>>(
          data_group, group, channel);
      break;

    default:
      break;
  }
  return std::move(observer);
}

ChannelObserverPtr CreateChannelObserver(const IDataGroup &dg_group,
                                         const std::string &channel_name) {
  std::unique_ptr<IChannelObserver> observer;

  const IChannelGroup *channel_group = nullptr;
  const IChannel *channel = nullptr;
  uint64_t nof_samples = 0;
  const auto cg_list = dg_group.ChannelGroups();
  for (const auto *cg_group : cg_list) {
    if (cg_group == nullptr) {
      continue;
    }
    const auto cn_list = cg_group->Channels();
    for (const auto *cn_item : cn_list) {
      if (cn_item == nullptr) {
        continue;
      }
      if (Platform::stricmp(channel_name.c_str(), cn_item->Name().c_str()) ==
          0) {
        if (nof_samples <= cg_group->NofSamples()) {
          nof_samples = cg_group->NofSamples();
          channel_group = cg_group;
          channel = cn_item;
        }
      }
    }
  }
  if (channel_group != nullptr && channel != nullptr) {
    observer = CreateChannelObserver(dg_group, *channel_group, *channel);
  }
  return observer;
}

void CreateChannelObserverForChannelGroup(const IDataGroup &data_group,
                                          const IChannelGroup &group,
                                          ChannelObserverList &dest) {
  auto cn_list = group.Channels();
  for (const auto *cn : cn_list) {
    if (cn != nullptr) {
      dest.emplace_back(
          std::move(CreateChannelObserver(data_group, group, *cn)));
    }
  }
}

void CreateChannelObserverForDataGroup(const IDataGroup &data_group,
                                          ChannelObserverList &dest) {
  const auto cg_list = data_group.ChannelGroups();
  for (const IChannelGroup* group : cg_list) {
    if (group == nullptr || group->NofSamples() == 0) {
      continue;
    }

    const auto cn_list = group->Channels();
    for (const auto *channel : cn_list) {
      if (channel == nullptr) {
        continue;
      }
      auto channel_observer = CreateChannelObserver(data_group, *group, *channel);
      dest.emplace_back( std::move(channel_observer));

    }
  }
}

MdfReader::MdfReader(std::string filename) : filename_(std::move(filename)) {
  // Need to create MDF3 of MDF4 file
  bool bExist = false;
  try {
    fs::path p = fs::u8path(filename_);
    if (fs::exists(p)) {
      bExist = true;
    }
  } catch (const std::exception &error) {
    MDF_ERROR() << "File I/O error. Filename: " << filename_
                << ", Error: " << error.what();
    return;
  }

  if (!bExist) {
    MDF_ERROR() << "The file doesn't exist. Filename: " << filename_;
    // No meaning to continue if the file doesn't exist
    return;
  }

  // Create an internal std::filebuf
  auto file_buffer = std::make_shared<std::filebuf>();
  file_ = std::move(file_buffer);


  VerifyMdfFile();
}

MdfReader::MdfReader(const std::shared_ptr<std::streambuf>& buffer)  {
  file_ = buffer;
  VerifyMdfFile();
}

void MdfReader::VerifyMdfFile() {
  if (!file_) {
    MDF_ERROR() << "No stream attached. Invalid use of function";
    return;
  }

  const bool open = Open();
  if (!open) {
    MDF_ERROR()
        << "The file couldn't be opened for reading (locked?). Filename: "
        << filename_;
    // No meaning to continue if the file cannot be opened
    return;
  }

  auto id_block = std::make_unique<detail::IdBlock>();
  id_block->Read(*file_);

  if (Platform::strnicmp(id_block->FileId().c_str(), "MDF", 3) == 0 ||
      Platform::strnicmp(id_block->FileId().c_str(), "UnFinMF", 7) == 0) {
    if (id_block->Version() >= 400) {
      instance_ = std::make_unique<detail::Mdf4File>(std::move(id_block));
      instance_->FileName(filename_);
    } else {
      instance_ = std::make_unique<detail::Mdf3File>(std::move(id_block));
      instance_->FileName(filename_);
    }
    if (!instance_) {
      Close();
      MDF_ERROR() << "MDF version not supported. File: " << filename_;
    }
  } else {
    MDF_ERROR() << "This is not and MDF file. File: " << filename_;
    Close();
  }
}

MdfReader::~MdfReader() { Close(); }

std::string MdfReader::ShortName() const {
  try {
    auto filename = fs::u8path(filename_).stem().u8string();
    return {filename.begin(), filename.end()};
  } catch (const std::exception &) {
  }
  return {};
}

bool MdfReader::Open() {
  if (!file_) {
    MDF_ERROR() <<
        "No stream buffer has been assigned. Invalid use of the function";
    return false;
  }

  // Note that the above function will return true if it isn't a file
  // buffer.
  return detail::OpenMdfFile(*file_, filename_,
           std::ios_base::in | std::ios_base::binary);
}

bool MdfReader::IsOpen() const {
  std::streambuf* buffer = file_.get();
  if (buffer == nullptr) {
    return false;
  }

  try {
    auto* file_buf = dynamic_cast<std::filebuf*>(buffer);
    return file_buf == nullptr ? true : file_buf->is_open();
  } catch (const std::exception& err) {
    MDF_ERROR() << "Dynamic cast of pointer failed. Error: " << err.what();
  }
  return true;
}

void MdfReader::Close() {
  std::streambuf* buffer = file_.get();
  if (buffer == nullptr) {
    return;
  }
  if (filename_.empty()) {
    // Indicate that the stream buffer interface is used.
    // Could be dangerous to close the file here.
    buffer->pubsync();
    return;
  }

  try {
    auto* file_buf = dynamic_cast<std::filebuf*>(buffer);
    if (file_buf != nullptr && file_buf->is_open()) {
      file_buf->close();
    } else {
      buffer->pubsync(); // Should flush output
    }
  } catch (const std::exception& err) {

    MDF_ERROR() << "Dynamic cast of pointer failed. Error: " << err.what();
  }
}

bool MdfReader::ReadHeader() {
  if (!instance_ || !file_) {
    MDF_ERROR() << "No instance created. File: " << filename_;
    return false;
  }
  // If the file is not open, then open and close the file in this call
  bool shall_close = !IsOpen() && Open();
  if (!IsOpen()) {
    MDF_ERROR() << "File is not open. File: " << filename_;
    return false;
  }

  bool no_error = true;
  try {
    instance_->ReadHeader(*file_);
  } catch (const std::exception &error) {
    MDF_ERROR() << "Initialization failed. Error: " << error.what();
    no_error = false;
  }
  if (shall_close) {
    Close();
  }
  return no_error;
}

bool MdfReader::ReadMeasurementInfo() {
  if (!instance_ || !file_) {
    MDF_ERROR() << "No instance created. File: " << filename_;
    return false;
  }
  bool shall_close = !IsOpen() && Open();
  if (!IsOpen()) {
    MDF_ERROR() << "File is not open. File: " << filename_;
    return false;
  }
  bool no_error = true;
  try {
    instance_->ReadMeasurementInfo(*file_);

  } catch (const std::exception &error) {
    MDF_ERROR() << "Failed to read the DG/CG blocks. Error: " << error.what();
    no_error = false;
  }
  if (shall_close) {
    Close();
  }
  return no_error;
}

bool MdfReader::ReadEverythingButData() {
  if (!instance_ || !file_) {
    MDF_ERROR() << "No instance created. File: " << filename_;
    return false;
  }
  bool shall_close = !IsOpen() && Open();
  if (!IsOpen()) {
    MDF_ERROR() << "File is not open. File: " << filename_;
    return false;
  }
  bool no_error = true;
  try {
    instance_->ReadEverythingButData(*file_);

  } catch (const std::exception &error) {
    MDF_ERROR() << "Failed to read the file information blocks. Error: "
                << error.what();
    no_error = false;
  }
  if (shall_close) {
    Close();
  }
  return no_error;
}

bool MdfReader::ExportAttachmentData(const IAttachment &attachment,
                                     const std::string &dest_file) {
  if (!instance_ || !file_) {
    MDF_ERROR() << "No instance created. File: " << filename_;
    return false;
  }

  bool shall_close = !IsOpen() && Open();
  if (!IsOpen()) {
    MDF_ERROR() << "Failed to open file. File: " << filename_;
    return false;
  }

  bool no_error = true;
  try {
    auto &at4 = dynamic_cast<const detail::At4Block &>(attachment);
    at4.ReadData(*file_, dest_file);
  } catch (const std::exception &error) {
    MDF_ERROR() << "Failed to read the file information blocks. Error: "
                << error.what();
    no_error = false;
  }

  if (shall_close) {
    Close();
  }
  return no_error;
}

bool MdfReader::ExportAttachmentData(const IAttachment &attachment,
                                     std::streambuf &dest_buffer) {
  if (!instance_ || !file_) {
    MDF_ERROR() << "No instance created. File: " << filename_;
    return false;
  }

  bool shall_close = !IsOpen() && Open();
  if (!IsOpen()) {
    MDF_ERROR() << "Failed to open file. File: " << filename_;
    return false;
  }

  bool no_error = true;
  try {
    auto &at4 = dynamic_cast<const detail::At4Block &>(attachment);
    at4.ReadData(*file_, dest_buffer);
  } catch (const std::exception &error) {
    MDF_ERROR() << "Failed to read the file information blocks. Error: "
                << error.what();
    no_error = false;
  }

  if (shall_close) {
    Close();
  }
  return no_error;
}

bool MdfReader::ReadData(IDataGroup &data_group) {
  if (!instance_ || !file_) {
    MDF_ERROR() << "No instance created. File: " << filename_;
    return false;
  }

  bool shall_close = !IsOpen() && Open();
  if (!IsOpen()) {
    MDF_ERROR() << "Didn't open the file. File: " << filename_;
    return false;
  }

  bool error = false;
  try {
    if (instance_->IsMdf4()) {
      auto &dg4 = dynamic_cast<detail::Dg4Block &>(data_group);
      dg4.ReadData(*file_);
    } else {
      auto &dg3 = dynamic_cast<detail::Dg3Block &>(data_group);
      dg3.ReadData(*file_);
    }
  } catch (const std::exception &err) {
    MDF_ERROR() << "Didn't read the file information blocks. Error: "
                << err.what() << ", File: " << filename_;
    error = true;
  }

  if (shall_close) {
    Close();
  }
  return !error;
}

bool MdfReader::ReadPartialData(IDataGroup &data_group, size_t min_sample,
                                size_t max_sample) {
  if (!instance_ || !file_) {
    MDF_ERROR() << "No instance created. File: " << filename_;
    return false;
  }
  if (max_sample < min_sample) {
    max_sample = min_sample;
  }
  bool shall_close = !IsOpen() && Open();
  if (!IsOpen()) {
    MDF_ERROR() << "Failed to open file. File: " << filename_;
    return false;
  }

  bool error = false;
  try {
    if (instance_->IsMdf4()) {

      auto &dg4 = dynamic_cast<detail::Dg4Block &>(data_group);
      DgRange range(dg4, min_sample, max_sample);
      dg4.ReadRangeData(*file_, range);
    } else {
      auto &dg3 = dynamic_cast<detail::Dg3Block &>(data_group);
      DgRange range(dg3, min_sample, max_sample);
      dg3.ReadRangeData(*file_, range);
    }
  } catch (const std::exception &err) {
    MDF_ERROR() << "Failed to read the file information blocks. Error: "
                << err.what();
    error = true;
  }

  if (shall_close) {
    Close();
  }
  return !error;
}

bool MdfReader::ReadSrData(ISampleReduction &sr_group) {
  if (!instance_ || !file_) {
    MDF_ERROR() << "No instance created. File: " << filename_;
    return false;
  }

  bool shall_close = !IsOpen() && Open();
  if (!IsOpen()) {
    MDF_ERROR() << "Failed to open file. File: " << filename_;
    return false;
  }

  bool error = false;
  try {
    if (instance_->IsMdf4()) {
      auto &sr4 = dynamic_cast<detail::Sr4Block &>(sr_group);
      sr4.ReadData(*file_);
    } else {
      auto &sr3 = dynamic_cast<detail::Sr3Block &>(sr_group);
      sr3.ReadData(*file_);
    }
  } catch (const std::exception &err) {
    MDF_ERROR() << "Failed to read the file information blocks. Error: "
                << err.what();
    error = true;
  }

  if (shall_close) {
    Close();
  }
  return !error;
}

const IHeader *MdfReader::GetHeader() const {
  const auto *file = GetFile();
  return file != nullptr ? file->Header() : nullptr;
}

IDataGroup *MdfReader::GetDataGroup(size_t order) const {
  const auto *file = GetFile();
  if (file != nullptr) {
    DataGroupList dg_list;
    file->DataGroups(dg_list);
    if (order < dg_list.size()) {
      return dg_list[order];
    }
  }
  return nullptr;
}

bool MdfReader::ReadVlsdData(IDataGroup &data_group,
                             IChannel &vlsd_channel,
                             const std::vector<uint64_t>& offset_list,
                             std::function<void (uint64_t,
                                      const std::vector<uint8_t>&)>& callback)
{
  if (vlsd_channel.Type() != ChannelType::VariableLength) {
    MDF_ERROR() << "The channels type is not variable length type.";
    return false;
  }

  if (!instance_ || !file_ ) {
    MDF_ERROR() << "No instance created. File: " << filename_;
    return false;
  }

  bool shall_close = !IsOpen() && Open();
  if (!IsOpen()) {
    MDF_ERROR() << "Failed to open file. File: " << filename_;
    return false;
  }

  bool error = false;
  try {
    if (instance_->IsMdf4()) {
      auto &dg4 = dynamic_cast<detail::Dg4Block &>(data_group);
      auto &cn4 = dynamic_cast<detail::Cn4Block &>(vlsd_channel);
      dg4.ReadVlsdData(*file_, cn4,offset_list, callback);
    } else {
      MDF_ERROR() << "Function not support for version MDF version 3.";
      error = true;
    }
  } catch (const std::exception &err) {
    MDF_ERROR() << "Failed to read the file information blocks. Error: "
                << err.what();
    error = true;
  }

  if (shall_close) {
    Close();
  }
  return !error;
}

bool MdfReader::IsFinalized() const {
  if (!instance_) {
    return false;
  }
  uint16_t standard_flags = 0;
  uint16_t custom_flags = 0;
  const bool finalized = instance_->IsFinalized(standard_flags, custom_flags);
  return finalized || instance_->IsFinalizedDone();
}

}  // namespace mdf
