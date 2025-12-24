/*
* Copyright 2025 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "mdf/canbusobserver.h"

#include <sstream>

#include "mdf/idatagroup.h"
#include "mdf/ichannelgroup.h"
#include "mdf/mdflogstream.h"

namespace mdf {

CanBusObserver::CanBusObserver(const IDataGroup& data_group,
                               const IChannelGroup& channel_group)
    : ISampleObserver(data_group), channel_group_(channel_group) {
  const std::string group_name = channel_group.Name();
  record_id_list_.clear();
  nof_samples_ = channel_group.NofSamples();
  record_id_list_.insert(channel_group.RecordId());
  FindVLsdRecord(channel_group);

  if (group_name.find("_DataFrame") != std::string::npos) {
    message_type_ = MessageType::CAN_DataFrame;
    base_name_ = "CAN_DataFrame";
  } else if (group_name.find("_RemoteFrame") != std::string::npos) {
    message_type_ = MessageType::CAN_RemoteFrame;
    base_name_ = "CAN_RemoteFrame";
  } else if (group_name.find("_ErrorFrame") != std::string::npos) {
    message_type_ = MessageType::CAN_ErrorFrame;
    base_name_ = "CAN_ErrorFrame";
  } else if (group_name.find("_OverloadFrame") != std::string::npos) {
    message_type_ = MessageType::CAN_OverloadFrame;
    base_name_ = "CAN_OverloadFrame";
  } else {
    // Invalid CAN channel group name.
    // Try to find the message type by channel names.
    for (const auto* channel : channel_group_.Channels()) {
      if (channel == nullptr) {
        continue;
      }
      const std::string channel_name = channel->Name();
      if (channel_name.find("_DataFrame") != std::string::npos) {
        message_type_ = MessageType::CAN_DataFrame;
        base_name_ = "CAN_DataFrame";
        break;
      } else if (channel_name.find("_RemoteFrame") != std::string::npos) {
        message_type_ = MessageType::CAN_RemoteFrame;
        base_name_ = "CAN_RemoteFrame";
        break;
      } else if (channel_name.find("_ErrorFrame") != std::string::npos) {
        message_type_ = MessageType::CAN_ErrorFrame;
        base_name_ = "CAN_ErrorFrame";
        break;
      } else if (channel_name.find("_OverloadFrame") != std::string::npos) {
        message_type_ = MessageType::CAN_OverloadFrame;
        base_name_ = "CAN_OverloadFrame";
        break;
      }
    }
    if (base_name_.empty()) {
      // No payload channel found.
      MDF_TRACE() << "Didn't find the CAN message type. Group: " << group_name;
      record_id_list_.clear();
      nof_samples_ = 0;
    }
  }
  time_channel_ = channel_group.GetMasterChannel();
  bus_channel_ = GetChannel("BusChannel");
  ide_channel_ = GetChannel("IDE");
  id_channel_ = GetChannel("ID");
  dlc_channel_ = GetChannel("DLC");
  length_channel_ = GetChannel("DataLength");
  data_channel_ = GetChannel("DataBytes");
  dir_channel_ = GetChannel("Dir");
  crc_channel_ = GetChannel("CRC");
  srr_channel_ = GetChannel("SRR");
  edl_channel_ = GetChannel("EDL");
  brs_channel_ = GetChannel("BRS");
  esi_channel_ = GetChannel("ESI");
  r0_channel_ = GetChannel("RO");
  r1_channel_ = GetChannel("R1");
  wake_up_channel_ = GetChannel("WakeUp");
  single_wire_channel_ = GetChannel("SingleWire");
  frame_duration_channel_ = GetChannel("FrameDuration");
  bit_position_channel_ = GetChannel("BitPosition");
}

std::string CanBusObserver::Name() const {
  return channel_group_.Name();
}

bool CanBusObserver::OnSample(uint64_t sample, uint64_t record_id,
                              const std::vector<uint8_t>& record) {
  if (DataGroup() == nullptr || nof_samples_ == 0
      || record_id_list_.find(record_id) == record_id_list_.end()) {
    // Not interested in this record.
    return true;
  }

  if (sample >= nof_samples_) {
    MDF_ERROR() << "Overrun of samples detected. Sample/Nof Samples: "
        << sample << "/" << nof_samples_ << ".";
    // Don't continue.
    return false;
  }

  const auto current_sample_index = static_cast<int64_t>(sample);
  if (current_sample_index > current_sample_index_) {
    last_sample_.Reset();
    last_sample_.sample = sample;
    current_sample_index_ = current_sample_index;
    new_sample_ = true;
  } else {
    new_sample_ = false;
  }

  if (OnCanMessage) {
    // The user uses callback. Reset the internal queue
    ClearSampleBuffer();
    HandleCallback(record_id, record);
  } else {
    if (!CheckSampleBufferSize()) {
      return false;
    }
    HandleSample(record_id, record);
  }

  return true;
}

void CanBusObserver::FindVLsdRecord(const IChannelGroup& channel_group) {
  const auto channel_list = channel_group.Channels();
  for (const auto* channel : channel_list) {
    if (channel == nullptr) {
      continue;
    }
    if (channel->VlsdRecordId() > 0) {
      record_id_list_.insert(channel->VlsdRecordId());
    }
  }
}

void CanBusObserver::HandleCallback(uint64_t record_id,
                                const std::vector<uint8_t>& record) {
  if (record_id != channel_group_.RecordId()) {
    last_sample_.vlsd_data = record;
    if (!new_sample_) {
      FireCallback();
    }
  } else {
    last_sample_.record = record;
    switch (channel_group_.StorageType()) {
      case MdfStorageType::VlsdStorage:
        if (!new_sample_) {
          FireCallback();
        }
        break;

      case MdfStorageType::FixedLengthStorage:
      case MdfStorageType::MlsdStorage:
        FireCallback();
        break;

      default:
        break;
    }
  }
}

void CanBusObserver::HandleSample(uint64_t record_id,
                          const std::vector<uint8_t>& record) {
  const uint64_t sample = last_sample_.sample;
  // Update the last sample object
  if (record_id != channel_group_.RecordId()) {
    // VLSD CG data storage.
    last_sample_.vlsd_data = record;
    if (!new_sample_ && sample < sample_buffer_.size()) {
      auto msg = std::make_unique<CanMessage>();
      msg->TypeOfMessage(message_type_);
      ParseCanMessage(*msg);
      sample_buffer_[sample] = std::move(msg);
    }
  } else {
    // Main record
    last_sample_.record = record;
    switch (channel_group_.StorageType()) {

      case MdfStorageType::VlsdStorage:
        // Wait for the VLSD CG record callback.
        // It received in the callback before this callback. Note that
        // this assumes that the logger uses this one-to-one saving
        // policy.
        if (!new_sample_ && sample < sample_buffer_.size()) {
          auto msg = std::make_unique<CanMessage>();
          msg->TypeOfMessage(message_type_);
          ParseCanMessage(*msg);
          sample_buffer_[sample] = std::move(msg);
        }
        break;

      case MdfStorageType::MlsdStorage:
        // The VLSD bytes is stored in the data bytes channel.
        // Its length is stored in another channel (have to be data length).
        // There is some bugs for some loggers to not correctly point out the
        // correct data length channel.
        if (sample < sample_buffer_.size()) {
          auto msg = std::make_unique<CanMessage>();
          msg->TypeOfMessage(message_type_);
          ParseCanMessage(*msg);
          sample_buffer_[sample] = std::move(msg);
        }
        break;

      case MdfStorageType::FixedLengthStorage:
        // The VLSD data is stored in the data byes channel's SD block.
        // The SD block has to be read in ahead of this call and it's
        // consumes primary memory. The data bytes is a 64-bit offset
        // into that SD block.
        if (sample < sample_buffer_.size()) {
          auto msg = std::make_unique<CanMessage>();
          msg->TypeOfMessage(message_type_);
          ParseCanMessage(*msg);
          sample_buffer_[sample] = std::move(msg);
        }
        break;

      default:
        break;
    }

  }
}

bool CanBusObserver::CheckSampleBufferSize() {
  if (sample_buffer_.size() != nof_samples_) {
    // Resize the internal buffer.
    try {
      sample_buffer_.resize(nof_samples_);
      sample_buffer_.shrink_to_fit();
    } catch (const std::exception& err) {
      MDF_ERROR() << "Didn't resize the buffer: " << err.what();
      return false;
    }
  }
  return true;
}

void CanBusObserver::ClearSampleBuffer() {
  if (!sample_buffer_.empty()) {
    sample_buffer_.clear();
    sample_buffer_.shrink_to_fit();
  }
}

bool CanBusObserver::FireCallback() {
  if (!OnCanMessage) {
    return false;
  }
  const uint64_t sample = last_sample_.sample;
  CanMessage msg;
  ParseCanMessage(msg);

  return OnCanMessage(sample, msg);
}

const IChannel* CanBusObserver::GetChannel(
    const std::string_view& sub_name) const {
  if (sub_name.empty()) {
    return nullptr;
  }
  std::ostringstream name;
  name << base_name_ << "." << sub_name;
  return channel_group_.GetChannel(name.str());
}

/** \brief Converts the record and VLSD record to a CAN message.
 *
 * The function convert a CanSample byte buffer, to a CAN message.
 * @param msg Returning bus message
 */
void CanBusObserver::ParseCanMessage(CanMessage& msg) const {
  const auto& record = last_sample_.record;
  if (time_channel_ != nullptr) {
    double timestamp = 0.0;
    GetEngValue(*time_channel_, current_sample_index_, record, timestamp);
    msg.Timestamp(timestamp);
  }

  if (bus_channel_ != nullptr) {
    // Sometimes this is a scaled or fixed value
    double bus_channel = 0.0;
    GetEngValue(*bus_channel_, current_sample_index_, record, bus_channel);
    msg.BusChannel(static_cast<uint8_t>(bus_channel));
  }

  if (id_channel_ != nullptr) {
    // This is either CAN ID + IDE flag or the CAN ID.
    // Therefore add the IDE flag after the CAN ID.
    uint32_t msg_id = 0;
    GetChannelValue(*id_channel_, current_sample_index_, record, msg_id);
    msg.MessageId(msg_id);
  }

  if (ide_channel_ != nullptr) {
    bool extended = false;
    GetChannelValue(*ide_channel_, current_sample_index_, record, extended);
    msg.ExtendedId(extended);
  }

  if (dlc_channel_ != nullptr) {
    uint8_t dlc = 0;
    GetChannelValue(*dlc_channel_, current_sample_index_, record, dlc);
    msg.Dlc(dlc);
  }

  if (length_channel_ != nullptr) {
    double length = 0.0;
    GetEngValue(*length_channel_, current_sample_index_, record, length);
    msg.DataLength(static_cast<uint8_t>(length));
  }

  if (data_channel_ != nullptr) {
    switch (channel_group_.StorageType()) {
      case MdfStorageType::VlsdStorage:
        msg.DataBytes(last_sample_.vlsd_data);
        break;

      case MdfStorageType::MlsdStorage: {
        const uint8_t data_length = msg.DataLength();
        std::vector<uint8_t> data;
        GetChannelValue(*data_channel_, current_sample_index_, record, data);
        if (data_length < data.size()) {
          data.resize(data_length);
        }
        msg.DataBytes(data);
        break;
      }

      // SD Storage. This value is the offset into the SD
      case MdfStorageType::FixedLengthStorage: {
        std::vector<uint8_t> data_bytes;
        data_channel_->GetChannelValue(record, data_bytes);
        msg.DataBytes(std::move(data_bytes));
        break;
      }

      default:
        break;
    }
  }

  if (dir_channel_ != nullptr) {
    // Note that eng value is a enumerate string
    bool direction = false;
    GetChannelValue(*dir_channel_, current_sample_index_, record, direction);
    msg.Dir(direction);
  }

  if (crc_channel_ != nullptr) {
    uint32_t crc = 0;
    GetChannelValue(*crc_channel_, current_sample_index_, record, crc);
    msg.Crc(crc);
  }

  if (srr_channel_ != nullptr) {
    bool srr = false;
    GetChannelValue(*srr_channel_, current_sample_index_, record, srr);
    msg.Srr(srr);
  }

  if (edl_channel_ != nullptr) {
    bool edl = false;
    GetChannelValue(*edl_channel_, current_sample_index_, record, edl);
    msg.Edl(edl);
  }

  if (brs_channel_ != nullptr) {
    bool brs = false;
    GetChannelValue(*brs_channel_, current_sample_index_, record, brs);
    msg.Brs(brs);
  }

  if (esi_channel_ != nullptr) {
    bool esi = false;
    GetChannelValue(*esi_channel_, current_sample_index_, record, esi);
    msg.Esi(esi);
  }

  if (r0_channel_ != nullptr) {
    bool r0 = false;
    GetChannelValue(*r0_channel_, current_sample_index_, record, r0);
    msg.R0(r0);
  }

  if (r1_channel_ != nullptr) {
    bool r1 = false;
    GetChannelValue(*r1_channel_, current_sample_index_, record, r1);
    msg.R1(r1);
  }

  if (wake_up_channel_ != nullptr) {
    bool wake_up = false;
    GetChannelValue(*wake_up_channel_, current_sample_index_, record, wake_up);
    msg.WakeUp(wake_up);
  }

  if (single_wire_channel_ != nullptr) {
    bool single_wire = false;
    GetChannelValue(*single_wire_channel_, current_sample_index_, record,
                    single_wire);
    msg.SingleWire(single_wire);
  }

  if (frame_duration_channel_ != nullptr) {
    double duration = 0.0;
    GetEngValue(*frame_duration_channel_, current_sample_index_, record, duration);
    msg.FrameDuration(static_cast<uint32_t>(duration));
  }

  if (bit_position_channel_ != nullptr) {
    double position = 0.0;
    GetEngValue(*bit_position_channel_, current_sample_index_, record, position);
    msg.FrameDuration(static_cast<uint16_t>(position));
   }

   if (error_type_channel_ != nullptr) {
     uint16_t type = 0;
     GetChannelValue(*error_type_channel_, current_sample_index_, record, type);
     msg.ErrorType(static_cast<CanErrorType>(type));
   }
}

CanMessage* CanBusObserver::GetCanMessage(uint64_t sample) {
  if (sample >= sample_buffer_.size() ) {
    return nullptr;
  }
  return sample_buffer_[sample].get();
}

} // mdf