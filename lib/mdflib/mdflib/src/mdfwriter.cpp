/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "mdf/mdfwriter.h"

#include <mdf/idatagroup.h>
#include <mdf/mdflogstream.h>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <fstream>

#include "dg4block.h"
#include "dt4block.h"
#include "mdf/ethconfigadapter.h"
#include "mdf/linconfigadapter.h"
#include "mdf/canconfigadapter.h"
#include "mdf/mostconfigadapter.h"
#include "mdfblock.h"
#include "platform.h"
#include "samplequeue.h"

#if INCLUDE_STD_FILESYSTEM_EXPERIMENTAL
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif

using namespace fs;
using namespace std::chrono_literals;


namespace {
/*
std::string StrErrNo(int error) {
  std::string err_str(200, '\0');
  Platform::strerror(error, err_str.data(), err_str.size());
  return err_str;
}
*/
mdf::IChannel* CreateTimeChannel(mdf::IChannelGroup& group,
                                 const std::string_view& name) {
  auto cn_list = group.Channels();
  // First check if a time channel exist. If so return it unchanged.
  const auto itr = std::find_if(cn_list.begin(),
                                                        cn_list.end(),
                          [] (const auto* channel) {
    return channel != nullptr && channel->Type() == mdf::ChannelType::Master;
  });
  if (itr != cn_list.end()) {
    return *itr;
  }
  auto* time = group.CreateChannel(name.data());
  if (time != nullptr) {
    time->Name(name.data());
    time->Type(mdf::ChannelType::Master);
    time->Sync(mdf::ChannelSyncType::Time);
    time->DataType(mdf::ChannelDataType::FloatLe);
    time->DataBytes(8);
    time->Unit("s");
  }
  return time;
}

mdf::IChannel* CreateBitChannel(mdf::IChannel& parent,
                                const std::string_view& name,
                                uint32_t byte_offset,
                                uint16_t bit_offset) {
  auto* frame_bit = parent.CreateChannelComposition(name);
  if (frame_bit != nullptr) {
    frame_bit->Type(mdf::ChannelType::FixedLength);
    frame_bit->Sync(mdf::ChannelSyncType::None);
    frame_bit->DataType(mdf::ChannelDataType::UnsignedIntegerLe);
    frame_bit->Flags(mdf::CnFlag::BusEvent);
    frame_bit->ByteOffset(byte_offset);
    frame_bit->BitOffset(bit_offset);
    frame_bit->BitCount(1);
  }
  return frame_bit;
}

}  // namespace

namespace mdf {


void MdfWriter::PreTrigTime(double pre_trig_time) {
  pre_trig_time *= 1'000'000'000;
  pre_trig_time_ = static_cast<uint64_t>(pre_trig_time);
}

double MdfWriter::PreTrigTime() const {
  auto temp = static_cast<double>(pre_trig_time_);
  temp /= 1'000'000'000;
  return temp;
}

uint64_t MdfWriter::PreTrigTimeNs() const {
  return pre_trig_time_;
}

IHeader* MdfWriter::Header() const {
  return mdf_file_ ? mdf_file_->Header() : nullptr;
}

IDataGroup* MdfWriter::CreateDataGroup() {
  return !mdf_file_ ? nullptr : mdf_file_->CreateDataGroup();
}

IChannelGroup* MdfWriter::CreateChannelGroup(IDataGroup* parent) {
  return parent == nullptr ? nullptr : parent->CreateChannelGroup();
}

void MdfWriter::Open(std::ios_base::openmode mode) {
  auto* buffer = file_.get();
  if (buffer == nullptr) {
    MDF_ERROR()
    << "Invalid use of function. Need to init the file or stream buffer first.";
    return;
  }
  detail::OpenMdfFile(*file_, filename_, mode);
}

bool MdfWriter::IsOpen() const {
  const auto* buffer = file_.get();
  if (buffer == nullptr) {
    return false;
  }
  try {
    const auto* file = dynamic_cast<const std::filebuf*>(buffer);
    if (file == nullptr) {
      return true;
    }
    return file->is_open();
  } catch (const std::exception& err) {
    MDF_ERROR() << "Checking if file is opened, failed. Error: " << err.what();
  }
  return false;
}

void MdfWriter::Close() {
  auto* buffer = file_.get();
  if (buffer == nullptr) {
    return;
  }
  if (filename_.empty()) {
    // This indicates that the user used the stream buffer interface.
    // Closing the stream buffer may cause that the stream never is
    // opened again.
    buffer->pubsync();
    return;
  }
  try {
    auto* file = dynamic_cast<std::filebuf*>(buffer);
    if (file != nullptr && file->is_open()) {
      file->close();
    } else if (file == nullptr) {
      buffer->pubsync();
    }

  } catch( const std::exception& err) {
    MDF_ERROR() << "Close of file failed. Error: " << err.what();
  }
}

bool MdfWriter::Init(const std::string& filename) {
  bool init = false;
  CreateMdfFile();
  filename_ = filename;
  if (mdf_file_) {
    mdf_file_->FileName(filename);
  }
  file_ = std::make_shared<std::filebuf>();
  if (!file_) {
    MDF_ERROR() << "Failed to create a file buffer. Internal error.";
    return false;
  }
  try {
    if (fs::exists(filename_)) {
      // Read in existing file so we can append to it

      Open(std::ios_base::in | std::ios_base::binary);
      if (IsOpen()) {
        mdf_file_->ReadEverythingButData(*file_);
        Close();
        write_state_ = WriteState::Finalize;  // Append to the file
        MDF_DEBUG() << "Reading existing file. File: " << filename_;
        init = true;
      } else {
        MDF_ERROR() << "Failed to open the existing MDF file. File: "
                    << filename_;
        write_state_ = WriteState::Create;
      }
    } else {
      // Create a new file
      write_state_ = WriteState::Create;  // Indicate the file shall be opened
                                          // with "wb" option.
      init = true;
    }
  } catch (const std::exception& err) {
    if (IsOpen()) {
      Close();
      write_state_ = WriteState::Finalize;
      MDF_ERROR() << "Failed to read the existing MDF file. Error: "
                  << err.what() << ", File: " << filename_;
    } else {
      write_state_ = WriteState::Create;
      MDF_ERROR() << "Failed to open the existing MDF file. Error: "
                  << err.what() << ", File: " << filename_;
    }
  }
  return init;
}

bool MdfWriter::Init(const std::shared_ptr<std::streambuf>& buffer) {
  bool init = true;
  CreateMdfFile();
  file_ = buffer;
  write_state_ = WriteState::Create;
  return init;
}

bool MdfWriter::InitMeasurement() {
  ExitWriteCache();  // Just in case
  if (!mdf_file_) {
    MDF_ERROR() << "The MDF file is not created. Invalid use of the function.";
    return false;
  }

  // Set up internal sample buffers so the last channel values can be stored
  const bool prep = PrepareForWriting();
  if (!prep) {
    MDF_ERROR() << "Failed to prepare the file for writing. File: "
                << filename_;
    return false;
  }
  // 1: Save ID, HD, DG, AT, CG and CN blocks to the file.

  Open(write_state_ == WriteState::Create ?
             std::ios_base::out | std::ios_base::binary | std::ios_base::trunc:
             std::ios_base::in | std::ios_base::out | std::ios_base::binary);
  if (!IsOpen()) {
    MDF_ERROR() << "Failed to open the file for writing. File: " << filename_;
    return false;
  }

  const bool write = mdf_file_->Write(*file_);

  //SetDataPosition();  // Set up data position to end of file
  Close();
  start_time_ = 0;  // Zero indicate not started
  stop_time_ = 0;   // Zero indicate not stopped
  // Start the working thread that handles the samples
  write_state_ = WriteState::Init;  // Waits for new samples
  InitWriteCache();
  return write;
}

void MdfWriter::StartMeasurement(uint64_t start_time) {
  write_state_ = WriteState::StartMeas;
  start_time_ = start_time;
  stop_time_ = 0;  // Zero indicate not stopped

  // The sample queue actual have absolute time. We have to recalculate the
  // times to relative times by using the start_time_.
  RecalculateTimeMaster();

  NotifySample();

  // Set the time in the header if this is the first DG block in the file.
  // This gives a better start time than when the file was created.
  if ( auto* header = Header();
      header != nullptr && IsFirstMeasurement()) {
    header->StartTime(start_time);
  }

  NotifySample();
}

void MdfWriter::StartMeasurement(ITimestamp &start_time) {
  write_state_ = WriteState::StartMeas;
  start_time_ = start_time.GetUtcTimeNs();
  stop_time_ = 0;  // Zero indicate not stopped
                   
  // The sample queue actual have absolute time. We have to recalculate the
  // times to relative times by using the start_time_.
  RecalculateTimeMaster();

  NotifySample();

  // Set the time in the header if this is the first DG block in the file.
  // This gives a better start time than when the file was created.
  if ( auto* header = Header();
      header != nullptr && IsFirstMeasurement()) {
    header->StartTime(start_time);
  }

  NotifySample();
}

void MdfWriter::StopMeasurement(uint64_t stop_time) {
  ExitWriteCache();
  write_state_ = WriteState::StopMeas;
  stop_time_ = stop_time;
  NotifySample();
}

void MdfWriter::StopMeasurement(ITimestamp& start_time) {
  StopMeasurement(start_time.GetUtcTimeNs());
}

bool MdfWriter::FinalizeMeasurement() {

  // Save outstanding non-written blocks and any block updates as
  // sample counters which changes during DG/DT updates
  if (!mdf_file_) {
    MDF_ERROR() << "The MDF file is not created. Invalid use of the function.";
    return false;
  }

  Open(std::ios_base::in | std::ios_base::out | std::ios_base::binary);
  if (!IsOpen()) {
    MDF_ERROR() << "Failed to open the file for writing. File: " << filename_;
    return false;
  }
  const bool write = mdf_file_ && mdf_file_->Write(*file_);
  const bool signal_data = WriteSignalData(*file_);
  Close();
  write_state_ = WriteState::Finalize;
  return write && signal_data;
}

IChannel* MdfWriter::CreateChannel(IChannelGroup* parent) {
  return parent == nullptr ? nullptr : parent->CreateChannel();
}

bool MdfWriter::WriteSignalData(std::streambuf&) {
  // Only  supported by MDF4
  return true;
}

std::string MdfWriter::Name() const {
  try {
    auto filename = u8path(filename_).stem().u8string();
    return {filename.begin(), filename.end()};
  } catch (...) {
  }
  return {};
}

bool MdfWriter::CreateBusLogConfiguration() {
  auto* header = Header();
  if (header == nullptr) {
    MDF_ERROR()
        << "File/Header haven't been created yet. Improper use of function";
    return false;
  }

  if (bus_type_ == 0) {
    MDF_ERROR() << "There is no bus defined. Invalid use of the function";
    return false;
  }

  // 1. Create new DG block if not already exists
  auto* last_dg = header->LastDataGroup();
  if (last_dg == nullptr || !last_dg->IsEmpty()) {
    last_dg = header->CreateDataGroup();
  }
  if (last_dg == nullptr) {
    MDF_ERROR() << "Failed to create a DG block.";
    return false;
  }


  // 2. Create bus dependent CG and CN blocks
  if ((bus_type_ & MdfBusType::CAN) != 0) {
    CanConfigAdapter configurator(*this);
    configurator.CreateConfig(*last_dg);
  }
  if ((bus_type_ & MdfBusType::LIN) != 0) {
    LinConfigAdapter configurator(*this);
    configurator.CreateConfig(*last_dg);
  }
  if ((bus_type_ & MdfBusType::Ethernet) != 0) {
    EthConfigAdapter configurator(*this);
    configurator.CreateConfig(*last_dg);
  }
  if ((bus_type_ & MdfBusType::MOST) != 0) {
    MostConfigAdapter configurator(*this);
    configurator.CreateConfig(*last_dg);
  }
  return true;
}

std::string MdfWriter::BusTypeAsString() const {
  return MdfBusTypeToString(bus_type_);
}

bool MdfWriter::IsFirstMeasurement() const {
  const auto* header = Header();
  if (header == nullptr) {
    return true;
  }

  for (const auto* data_group : header->DataGroups()) {
    if (data_group == nullptr) {
      continue;
    }
    for (const auto* channel_group : data_group->ChannelGroups()) {
      if (channel_group == nullptr ||
          (channel_group->Flags() & CgFlag::VlsdChannel) != 0) {
        continue;
      }
      if (channel_group->NofSamples() > 0) {
        return false;
      }
    }

  }
  return true;
}
/*
void MdfWriter::SetDataPosition() {
  if (CompressData() || !file_) {
    return;
  }

  const auto* header = Header();
  if (header == nullptr) {
    MDF_ERROR() << "No MDF header. Invalid use of function.";
    return;
  }

  std::vector<IDataGroup*> measurement_list;

  for (IDataGroup* data_group : header->DataGroups()) {
    if (data_group == nullptr) {
      continue;
    }
    for (IChannelGroup* channel_group : data_group->ChannelGroups()) {
      if (channel_group == nullptr ||
          (channel_group->Flags() & CgFlag::VlsdChannel) != 0 ||
          channel_group->NofSamples() > 0) {
        continue;
      }
      measurement_list.push_back(data_group);
      break;
    }
  }

  for (IDataGroup* measurement : measurement_list) {
    if (measurement == nullptr) {
      continue;
    }

    auto* dg4 = dynamic_cast<detail::Dg4Block*>(measurement);
    if (dg4 == nullptr) {
      return;
    }
    auto& block_list = dg4->DataBlockList();
    if (block_list.empty()) {
      return;
    }
    auto* dt4 = dynamic_cast<detail::Dt4Block*>(block_list.back().get());
    if (dt4 == nullptr) {
      return;
    }

    file_->pubseekoff(0, std::ios_base::end);

    if (dg4->Link(2) > 0) {
      return;
    }

    dg4->SetLastFilePosition(*file_);
    const auto position = detail::GetFilePosition(*file_);
    dg4->UpdateLink(*file_, 2, position);
    dg4->SetLastFilePosition(*file_);

    const int64_t data_position = detail::GetFilePosition(*file_);
    dt4->DataPosition(0); //data_position);

  }
}
*/
/** \brief Checks if the compression is required
 *
 * The function checks if the compression is required to be enabled.
 * This is required if it is MDF 4 and more than one DG block is active.
 */


}  // namespace mdf
