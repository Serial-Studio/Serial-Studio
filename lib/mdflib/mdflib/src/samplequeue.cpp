/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include "samplequeue.h"

#include "mdf/mdflogstream.h"
#include "mdf/mostmessage.h"
#include "mdf/flexraymessage.h"

#include "dg3block.h"
#include "dg4block.h"
#include "dt4block.h"

namespace mdf::detail {

SampleQueue::SampleQueue(MdfWriter& writer,
                         IDataGroup& data_group)
: writer_(writer),
  data_group_(data_group) {

}

SampleQueue::~SampleQueue() {
  SampleQueue::Reset();
}

void SampleQueue::AddSample(SampleRecord&& record) {
  size_ += record.SampleSize();
  queue_.emplace_back(record);
}

void SampleQueue::PushSample(const SampleRecord& record) {
  size_ += record.SampleSize();
  queue_.push_front(record);
}

SampleRecord SampleQueue::GetSample() {
  SampleRecord record = queue_.front();
  queue_.pop_front();
  size_ -= record.SampleSize();
  return record;
}

void SampleQueue::PopSample() {
  const SampleRecord& record = queue_.front();
  queue_.pop_front();
  size_ -= record.SampleSize();

}
void SampleQueue::Reset() {
  queue_.clear();
  size_ = 0;
}

size_t SampleQueue::QueueSize() const {
  const auto id_size = data_group_.RecordIdSize();
  return size_ + (id_size * queue_.size());
}

void SampleQueue::TrimQueue() {
  // Last Time - First Time <= Pre-trig time. Note that we must include start
  // sample, so ignore the last sample
  const auto start_time = writer_.StartTime();
  const auto pre_trig_time = writer_.PreTrigTimeNs();

  while (queue_.size() > 1) {

    const auto next_time = queue_[1].timestamp;
    const auto last_time = queue_.back().timestamp;
    if (start_time > 0) {
      // Measurement started
      if (next_time >= start_time - pre_trig_time) {
        break;
      }
    } else {
      // Measurement not started. The queue shall be at least the pre-trig time
      const auto buffer_time = last_time - next_time;
      if (buffer_time <= pre_trig_time) {
        break;
      }
    }
    PopSample();
  }
}

void SampleQueue::SaveQueue(std::unique_lock<std::mutex>& lock) {
  // Save uncompressed data in last DG3 block
  // MDF 3 can only store one DG in real-time. By ignoring
  // the SaveQueue() function when more than one DG block, the
  // samples are queued in memory and when CleanQueue is called
  // the samples are stored as normal.
  if (NofDgBlocks() > 1) {
    return;
  }
  CleanQueue(lock);
}

void SampleQueue::CleanQueue(std::unique_lock<std::mutex>& lock) {
  const auto* dg3 = dynamic_cast<detail::Dg3Block*>(&data_group_);
  if (dg3 == nullptr ) {
    return;
  }

  lock.unlock();

  writer_.Open(std::ios_base::in | std::ios_base::out | std::ios_base::binary);
  if (!writer_.IsOpen()) {
    lock.lock();
    return;
  }

  SetLastPosition(*writer_.file_);

  lock.lock();

  // Trim the queue so the start time is included in the first sample
  TrimQueue();
  const auto stop_time = writer_.StopTime();
  // Save the queue onto the file
  while (!IsEmpty()) {
    // Write a sample last to file
    auto sample = GetSample();
    if (stop_time > 0 && sample.timestamp > stop_time) {
      break;  // Skip this sample
    }
    lock.unlock();

    if (dg3->NofRecordId() > 0) {
      const auto id = static_cast<uint8_t>(sample.record_id);
      writer_.file_->sputc(static_cast<char>(id));
    }
    writer_.file_->sputn(reinterpret_cast<const char*>(sample.record_buffer.data()),
                         static_cast<std::streamsize>(sample.record_buffer.size()) );
    if (dg3->NofRecordId() > 1) {
      const auto id = static_cast<uint8_t>(sample.record_id);
      writer_.file_->sputc(static_cast<char>(id));
    }
    IncrementNofSamples(sample.record_id);
    lock.lock();
  }

  // Update channel group headers to reflect the new number of samples
  lock.unlock();
  for (const auto& cg3 : dg3->Cg3()) {
    if (cg3 != nullptr) {
      cg3->Write(*writer_.file_);
    }
  }

  writer_.Close();
  lock.lock();

}

void SampleQueue::IncrementNofSamples(uint64_t record_id) const {
  const auto list = data_group_.ChannelGroups();
  std::for_each(list.cbegin(), list.cend(), [&](IChannelGroup* group) -> void {
    if (group != nullptr && group->RecordId() == record_id) {
      group->IncrementSample();            // Increment internal sample counter
      group->NofSamples(group->Sample());  // Update block counter
    }
  });
}

void SampleQueue::SaveSample(const IChannelGroup& group, uint64_t time) {
  SampleRecord sample = group.GetSampleRecord();
  sample.timestamp = time;
  RecalculateTime(group.RecordId(), sample);
  AddSample(std::move(sample));
}

void SampleQueue::SaveCanMessage(const IChannelGroup& group, uint64_t time,
                               const CanMessage& msg) {
  SampleRecord sample = group.GetSampleRecord();
  sample.timestamp = time;
  RecalculateTime(group.RecordId(),sample);

  // Convert the CAN message to a sample record. Note that depending on the
  // storage type, either the whole message buffer is stored (Max Length) or
  // an index to a SD or a VLSD CG record is stored. The index cannot be
  // calculated at this point as it have to be calculated before saving the
  // recording to disc. Instead, is the buffer temporary hold by the
  // SampleRecord struct and fixed just before saving to disc.
  const bool save_index = group.StorageType() != MdfStorageType::MlsdStorage;
  const uint32_t max_length = group.MaxLength();
  const std::string& group_name = group.Name();
  // Only checking the part of the group name as it may contain channel
  // and message ID/Name as well.
  if (group_name.find("_DataFrame") != std::string::npos)  {
    msg.TypeOfMessage(MessageType::CAN_DataFrame);
    msg.ToRaw(MessageType::CAN_DataFrame, sample, max_length, save_index);
  } else if (group_name.find("_RemoteFrame") != std::string::npos) {
    msg.TypeOfMessage(MessageType::CAN_RemoteFrame);
    msg.ToRaw(MessageType::CAN_RemoteFrame, sample, max_length, save_index);
  } else if (group_name.find("_ErrorFrame") != std::string::npos) {
    msg.TypeOfMessage(MessageType::CAN_ErrorFrame);
    msg.ToRaw(MessageType::CAN_ErrorFrame, sample, max_length, save_index);
  } else if (group_name.find("_OverloadFrame") != std::string::npos) {
    msg.TypeOfMessage(MessageType::CAN_OverloadFrame);
    msg.ToRaw(MessageType::CAN_OverloadFrame, sample,max_length, save_index);
  }
  AddSample(std::move(sample));
}

void SampleQueue::SaveLinMessage(const IChannelGroup& group, uint64_t time,
                               const LinMessage& msg) {
  SampleRecord sample = group.GetSampleRecord();
  sample.timestamp = time;

  RecalculateTime(group.RecordId(), sample);

  // Convert the LIN message to a sample record. Note that LIN always uses the
  // MLSD storage type.

  if (group.Name().find("_Frame") != std::string::npos) {
    msg.MessageType(LinMessageType::LIN_Frame);
    msg.ToRaw(LinMessageType::LIN_Frame, sample);
  } else if (group.Name().find("_WakeUp") != std::string::npos) {
    msg.MessageType(LinMessageType::LIN_WakeUp);
    msg.ToRaw(LinMessageType::LIN_WakeUp, sample);
  } else if (group.Name().find("_ChecksumError") != std::string::npos) {
    msg.MessageType(LinMessageType::LIN_ChecksumError);
    msg.ToRaw(LinMessageType::LIN_ChecksumError, sample);
  } else if (group.Name().find("_TransmissionError") != std::string::npos) {
    msg.MessageType(LinMessageType::LIN_TransmissionError);
    msg.ToRaw(LinMessageType::LIN_TransmissionError, sample);
  } else if (group.Name().find("_SyncError") != std::string::npos) {
    msg.MessageType(LinMessageType::LIN_SyncError);
    msg.ToRaw(LinMessageType::LIN_SyncError, sample);
  } else if (group.Name().find("_ReceiveError") != std::string::npos) {
    msg.MessageType(LinMessageType::LIN_ReceiveError);
    msg.ToRaw(LinMessageType::LIN_ReceiveError, sample);
  } else if (group.Name().find("_Spike") != std::string::npos) {
    msg.MessageType(LinMessageType::LIN_Spike);
    msg.ToRaw(LinMessageType::LIN_Spike, sample);
  } else if (group.Name().find("_LongDom") != std::string::npos) {
    msg.MessageType(LinMessageType::LIN_LongDominantSignal);
    msg.ToRaw(LinMessageType::LIN_LongDominantSignal, sample);
  }

  AddSample(std::move(sample));
}

void SampleQueue::SaveEthMessage(const IChannelGroup& group, uint64_t time,
                               const EthMessage& msg) {
  SampleRecord sample = group.GetSampleRecord();
  sample.timestamp = time;
  RecalculateTime(group.RecordId(), sample);

  // Convert the LIN message to a sample record. Note that LIN always uses the
  // MLSD storage type.

  if (group.Name().find( "_Frame") != std::string::npos) {
    msg.MessageType(EthMessageType::ETH_Frame);
    msg.ToRaw(EthMessageType::ETH_Frame, sample);
  } else if (group.Name().find( "_ChecksumError") != std::string::npos) {
    msg.MessageType(EthMessageType::ETH_ChecksumError);
    msg.ToRaw(EthMessageType::ETH_ChecksumError, sample);
  } else if (group.Name().find( "_LengthError") != std::string::npos) {
    msg.MessageType(EthMessageType::ETH_LengthError);
    msg.ToRaw(EthMessageType::ETH_LengthError, sample);
  } else if (group.Name().find("_ReceiveError") != std::string::npos) {
    msg.MessageType(EthMessageType::ETH_ReceiveError);
    msg.ToRaw(EthMessageType::ETH_ReceiveError, sample);
  }
  AddSample(std::move(sample));
}

void SampleQueue::SaveMostMessage(const IChannelGroup& group, uint64_t time,
                                 const IMostEvent& msg) {
  switch (msg.MessageType()) {
    case MostMessageType::Message:
      if (group.Name().find("_Message") == std::string::npos) {
        return;
      }
      break;

    case MostMessageType::Packet:
      if (group.Name().find("_Packet") == std::string::npos) {
        return;
      }
      break;

    case MostMessageType::EthernetPacket:
       if (group.Name().find("_EthernetPacket") == std::string::npos) {
         return;
       }
       break;

    case MostMessageType::SignalState:
      if (group.Name().find("_SignalState") == std::string::npos) {
        return;
      }
      break;

    case MostMessageType::MaxPosInfo:
      if (group.Name().find("_MaxPosInfo") == std::string::npos) {
        return;
      }
      break;

    case MostMessageType::BoundDesc:
      if (group.Name().find("_BoundDesc") == std::string::npos) {
        return;
      }
      break;

    case MostMessageType::AllocTable:
      if (group.Name().find("_AllocTable") == std::string::npos) {
        return;
      }
      break;

    case MostMessageType::SysLockState:
      if (group.Name().find("_SysLockState") == std::string::npos) {
        return;
      }
      break;

    case MostMessageType::ShutdownFlag:
      if (group.Name().find("_ShutdownFlag") == std::string::npos) {
        return;
      }
      break;

    default:
      return;
  }

  SampleRecord sample = group.GetSampleRecord();
  sample.timestamp = time;
  RecalculateTime(group.RecordId(), sample);
  msg.ToRaw(sample);
  AddSample(std::move(sample));
}

void SampleQueue::SaveFlexRayMessage(const IChannelGroup& group, uint64_t time,
                                  const IFlexRayEvent& msg) {
  switch (msg.MessageType()) {
    case FlexRayMessageType::Frame:
      if (group.Name().find("_Frame") == std::string::npos) {
        return;
      }
      break;

    case FlexRayMessageType::Pdu:
      if (group.Name().find("_Pdu") == std::string::npos) {
        return;
      }
      break;

    case FlexRayMessageType::FrameHeader:
      if (group.Name().find("_FrameHeader") == std::string::npos) {
        return;
      }
      break;

    case FlexRayMessageType::NullFrame:
      if (group.Name().find("_NullFrame") == std::string::npos) {
        return;
      }
      break;

    case FlexRayMessageType::ErrorFrame:
      if (group.Name().find("_ErrorFrame") == std::string::npos) {
        return;
      }
      break;

    case FlexRayMessageType::Symbol:
      if (group.Name().find("_Symbol") == std::string::npos) {
        return;
      }
      break;

    default:
      return;
  }

  SampleRecord sample = group.GetSampleRecord();
  sample.timestamp = time;
  RecalculateTime(group.RecordId(), sample);
  msg.ToRaw(sample);
  AddSample(std::move(sample));
}

void SampleQueue::RecalculateTime(uint64_t record_id, SampleRecord& sample) {
  const auto itr_master = master_channels_.find(record_id);
  const auto* master =  itr_master == master_channels_.cend() ?
                                                                     nullptr : itr_master->second;
  if (master != nullptr && master->CalculateMasterTime()) {
    auto rel_ns = static_cast<int64_t>(sample.timestamp);
    rel_ns -= static_cast<int64_t>(writer_.StartTime());
    const double rel_s = static_cast<double>(rel_ns) / 1'000'000'000.0;
    master->SetTimestamp(rel_s, sample.record_buffer);
  }
}


void SampleQueue::RecalculateTimeMaster() {
  master_channels_.clear();
  const auto cg_list = data_group_.ChannelGroups();
  for (const auto* group : cg_list) {
    if (group == nullptr) {
      continue;
    }
    const auto cn_list = group->Channels();
    for (const auto* channel : cn_list) {
      if (channel == nullptr) {
        continue;
      }
      // Pattern for MDF 4 time master channel
      if (channel->Type() == ChannelType::Master &&
          channel->Sync() == ChannelSyncType::Time) {
        master_channels_.emplace(group->RecordId(), channel);
        break;
      }
      // Patter for MDF 3 (time) master channel
      if (channel->Type() == ChannelType::Master &&
          channel->Sync() == ChannelSyncType::None) {
        master_channels_.emplace(group->RecordId(), channel);
        break;
      }
    }
  }
  if (master_channels_.empty()) {
    return;
  }

  for (auto& sample : queue_) {
    auto itr_ch = master_channels_.find(sample.record_id);
    if (itr_ch == master_channels_.end()) {
      continue;
    }
    const auto* master = itr_ch->second;
    if (master == nullptr || !master->CalculateMasterTime() ) {
      continue;
    }
    auto rel_ns = static_cast<int64_t>(sample.timestamp);
    rel_ns -= static_cast<int64_t>(writer_.StartTime());
    const double rel_s = static_cast<double>(rel_ns) / 1'000'000'000.0;

    master->SetTimestamp(rel_s, sample.record_buffer);
  }
}

void SampleQueue::Open() {
  if (!writer_.IsOpen()) {
    writer_.Open(std::ios_base::in | std::ios_base::out | std::ios_base::binary);
  }
}

bool SampleQueue::IsOpen() const { return writer_.IsOpen(); }
void SampleQueue::Close() {
  writer_.Close();
}
bool SampleQueue::IsEmpty() const { return queue_.empty(); }

void SampleQueue::SetLastPosition(std::streambuf& buffer) {
  buffer.pubseekoff(0, std::ios_base::end);

  auto *dg3 = dynamic_cast<Dg3Block *>(&data_group_);
  if (dg3 == nullptr) {
    return;
  }

  if (dg3->Link(3) > 0) {
    return;
  }

  dg3->SetLastFilePosition(buffer);
  const auto position = detail::GetFilePosition(buffer);
  dg3->UpdateLink(buffer, 3, position);
  dg3->SetLastFilePosition(buffer);
}

void SampleQueue::SetDataPosition(std::streambuf& buffer) {
  if (writer_.CompressData()) {
    return;
  }
  auto* dg4 = dynamic_cast<Dg4Block*>(&data_group_);
  if (dg4 == nullptr) {
    return;
  }
  auto& block_list = dg4->DataBlockList();
  if (block_list.empty()) {
    return;
  }
  auto* dt4 = dynamic_cast<Dt4Block*>(block_list.back().get());
  if (dt4 == nullptr) {
    return;
  }
  SetLastPosition(buffer);
  const int64_t data_position = GetFilePosition(buffer);
  dt4->DataPosition(data_position);
}


}  // namespace mdf