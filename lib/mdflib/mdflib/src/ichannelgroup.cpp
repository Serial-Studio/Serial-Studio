/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "mdf/ichannelgroup.h"

namespace mdf {

SampleRecord IChannelGroup::GetSampleRecord() const {
  SampleRecord record;
  record.timestamp = MdfHelper::NowNs();
  record.record_id = RecordId();
  record.record_buffer = sample_buffer_;
  return record;
}

void IChannelGroup::ClearData() { sample_ = 0; }

void IChannelGroup::IncrementSample() const { ++sample_; }

size_t IChannelGroup::Sample() const { return sample_; }

uint16_t IChannelGroup::Flags() const { return 0; }

void IChannelGroup::Flags(uint16_t flags) {}

char16_t IChannelGroup::PathSeparator() { return u'/'; }

void IChannelGroup::PathSeparator(char16_t path_separator) {}

ISourceInformation *IChannelGroup::CreateSourceInformation() { return nullptr; }

ISourceInformation *IChannelGroup::SourceInformation() const {
  return nullptr;
}
IMetaData *IChannelGroup::CreateMetaData() { return nullptr; }
IMetaData *IChannelGroup::MetaData() const { return nullptr; }

IChannel *IChannelGroup::CreateChannel(const std::string_view &name) {
  auto cn_list = Channels();
  auto itr = std::find_if(cn_list.begin(), cn_list.end(),
                              [&] (const auto* channel) {
    return channel != nullptr &&
           strcmp(channel->Name().c_str(), name.data()) == 0;
                              });
  if (itr != cn_list.end()) {
    return *itr;
  }
  auto* new_channel = CreateChannel();
  if (new_channel != nullptr) {
    new_channel->Name(name.data());
  }
  return new_channel;
}

IChannel *IChannelGroup::GetChannel(const std::string_view &name) const {
  auto cn_list = Channels(); // The list contains the composition channels as
                             // well as the ordinary channels.
  auto itr = std::find_if(cn_list.begin(), cn_list.end(),
                  [&] (const auto* channel) {
                    if (!channel) {
                      return false;
                    }
                    const auto pos = channel->Name().find(name);
                    return pos != std::string::npos;
                  });
  return itr != cn_list.end() ? *itr : nullptr;
}

IChannel *IChannelGroup::GetMasterChannel() const {
  auto cn_list = Channels(); // The list contains the composition channels as
  // well as the ordinary channels.
  auto itr = std::find_if(cn_list.begin(), cn_list.end(),
                  [] (const auto* channel) -> bool {
                    return (channel != nullptr &&
                      (channel->Type() == ChannelType::Master || channel->Type() == ChannelType::VirtualMaster));
              });
  return itr != cn_list.end() ? *itr : nullptr;
}

void IChannelGroup::SetCgComment(const CgComment &cg_comment) {
  if (IMetaData* meta_data = CreateMetaData();
      meta_data != nullptr ) {
    meta_data->XmlSnippet(cg_comment.ToXml());
  }
}

void IChannelGroup::GetCgComment(CgComment &cg_comment) const {
  if (const IMetaData* meta_data = MetaData();
      meta_data != nullptr) {
    cg_comment.FromXml(meta_data->XmlSnippet());
  }
}

BusType IChannelGroup::GetBusType() const {
  // Check if this block is a bus type block.
  if ((Flags() & CgFlag::BusEvent) == 0) {
    return BusType::None;
  }

  // Normally the SI block holds the bus type.
  if (const auto* si_block = SourceInformation(); si_block != nullptr) {
    return si_block->Bus();
  }

  // The CG block name should hold the bus type.
  const std::string group_name = Name();
  if (group_name.size() >= 3) {
    const std::string bus_name = group_name.substr(0, 3);
    if (bus_name == "CAN") {
      return BusType::Can;
    }
    if (bus_name == "LIN") {
      return BusType::Lin;
    }
    if (bus_name == "CAN") {
      return BusType::Can;
    }
    if (bus_name == "Fle") {
      return BusType::FlexRay;
    }
    if (bus_name == "MOS") {
      return BusType::Most;
    }
    if (bus_name == "ETH") {
      return BusType::Ethernet;
    }
  }

  // Some implementation uses unique names on the group name.
  // So better check for the bus type in the channel names.
  for (const IChannel* channel : Channels()) {
    if (channel == nullptr) {
      continue;
    }
    const std::string channel_name = channel->Name();
    if (channel_name.find("_DataFrame") == std::string::npos) {
      continue;
    }

    if (channel_name.size() >= 3) {
      const std::string bus_name = channel_name.substr(0, 3);
      if (bus_name == "CAN") {
        return BusType::Can;
      }
      if (bus_name == "LIN") {
        return BusType::Lin;
      }
      if (bus_name == "CAN") {
        return BusType::Can;
      }
      if (bus_name == "Fle") {
        return BusType::FlexRay;
      }
      if (bus_name == "MOS") {
        return BusType::Most;
      }
      if (bus_name == "ETH") {
        return BusType::Ethernet;
      }
    }
  }

  return BusType::None;
}

}  // namespace mdf
