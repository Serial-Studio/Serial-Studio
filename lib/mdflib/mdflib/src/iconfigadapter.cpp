/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */
#include <utility>
#include <sstream>

#include "mdf/iconfigadapter.h"
#include "mdf/mdfwriter.h"
#include "mdf/isourceinformation.h"
#include "mdf/ichannel.h"
#include "mdf/ichannelgroup.h"
#include "mdf/mdflogstream.h"

namespace mdf {

IConfigAdapter::IConfigAdapter(const MdfWriter& writer)
: writer_(writer) {
  StorageType(writer_.StorageType());
  BusType(writer_.BusType());
  MaxLength(writer_.MaxLength());
  MandatoryMembersOnly(writer_.MandatoryMembersOnly());
}

void IConfigAdapter::BusType(uint16_t bus_type) {
  bus_type_ = bus_type;
}

uint16_t IConfigAdapter::BusType() const {
  return bus_type_;
}

void IConfigAdapter::BusName(std::string bus_name) {
  bus_name_ = std::move(bus_name);
}

const std::string& IConfigAdapter::BusName() const {
  return bus_name_;
}

void IConfigAdapter::Protocol(std::string protocol) {
  protocol_ = std::move(protocol);
}

const std::string& IConfigAdapter::Protocol() const {
  return protocol_;
}

void IConfigAdapter::FilterType(MessageFilter type) {
  filter_type_ = type;
}

MessageFilter IConfigAdapter::FilterType() const {
  return filter_type_;
}

void IConfigAdapter::ChannelFilter(uint8_t channel) {
  bus_channel_ = channel;
}

uint8_t IConfigAdapter::ChannelFilter() const {
  return bus_channel_;
}

void IConfigAdapter::IdFilter(uint64_t message_id) {
  message_id_ = message_id;
}

uint64_t IConfigAdapter::IdFilter() const {
  return message_id_;
}

void IConfigAdapter::MessageName(std::string message_name) {
  message_name_ = std::move(message_name);
}

const std::string& IConfigAdapter::MessageName() const {
  return message_name_;
}

void IConfigAdapter::MessageLength(uint64_t message_length) {
  message_length_ = message_length;
}

uint64_t IConfigAdapter::MessageLength() const {
  return message_length_;
}

void IConfigAdapter::NetworkName(std::string network_name) {
  network_name_ = std::move(network_name);
}

const std::string& IConfigAdapter::NetworkName() const {
  return network_name_;
}

void IConfigAdapter::DeviceName(std::string device_name) {
  device_name_ = std::move(device_name);
}

const std::string& IConfigAdapter::DeviceName() const {
    return device_name_;
}

IChannel* IConfigAdapter::CreateTimeChannel(IChannelGroup& group,
                                        const std::string_view& name) const {
  const auto cn_list = group.Channels();

  // First check if a time channel exist. If so return it unchanged.
  const auto itr = std::find_if(cn_list.begin(), cn_list.end(),
                                [] (const IChannel* channel) -> bool {
                                  return channel != nullptr
                         && channel->Type() == ChannelType::Master
                         && channel->Sync() == ChannelSyncType::Time;
                                });
  if (itr != cn_list.end()) {
    return *itr;
  }

  IChannel* time = group.CreateChannel(name);
  if (time != nullptr ) {
    time->Name(name.data());
    time->Description(group.Name());
    time->Type(ChannelType::Master);
    time->Sync(ChannelSyncType::Time);
    time->DataType(ChannelDataType::FloatLe);
    time->DataBytes(8);
    time->Unit("s");
    return time;
  }
  return time;
}

ISourceInformation* IConfigAdapter::CreateSourceInformation(IChannelGroup& group) const {
  ISourceInformation* info = group.CreateSourceInformation();
  if (info == nullptr) {
    MDF_ERROR() << "Failed to create the SI block. Group: " << group.Name();
    return info;
  }

  if (!network_name_.empty()) {
    info->Name(network_name_);
  } else if (!device_name_.empty()) {
    info->Name(device_name_);
  } else if (!bus_name_.empty()) {
    info->Name(bus_name_);
  } else {
    info->Name(group.Name());
  }

  info->Path(group.Name());
  info->Type(bus_type_ != 0 ? SourceType::Bus : SourceType::Other);

  switch (bus_type_) {
    case MdfBusType::CAN:
      info->Bus(BusType::Can);
      break;

    case MdfBusType::LIN:
      info->Bus(BusType::Lin);
      break;

    case MdfBusType::MOST:
      info->Bus(BusType::Most);
      break;

    case MdfBusType::FlexRay:
      info->Bus(BusType::FlexRay);
      break;

    case MdfBusType::Ethernet:
      info->Bus(BusType::Ethernet);
      break;

    default:
      info->Bus(BusType::Other);
      break;
  }

  if (!protocol_.empty() || message_id_ != 0 || bus_channel_ != 0
      || !message_name_.empty()) {
    SiComment comment("Configurations for the bus");

    if (bus_channel_ != 0) {
      MdProperty channel("ChannelNo", std::to_string(bus_channel_));
      channel.DataType(MdDataType::MdInteger);
      comment.AddProperty(channel);
    }
    if (message_id_ != 0) {
      MdProperty message_id("MessageID", std::to_string(message_id_));
      message_id.DataType(MdDataType::MdInteger);
      comment.AddProperty(message_id);
    }
    if (message_length_ != 0) {
      MdProperty message_length("MessageLength", std::to_string(message_length_));
      message_length.DataType(MdDataType::MdInteger);
      comment.AddProperty(message_length);
    }
    if (!message_name_.empty())  {
      MdProperty message("Message", message_name_);
      comment.AddProperty(message);
    }
    if (!protocol_.empty()) {
      comment.Protocol(MdString(protocol_));
    }
    info->SetSiComment(comment);
  }
  return info;
}

ISourceInformation* IConfigAdapter::CreateSourceInformation(IChannel& channel) const {
  const IChannelGroup* group = channel.ChannelGroup();
  if (group == nullptr) {
    MDF_ERROR() << "Failed to find the CG block. Channel: " << channel.Name();
    return nullptr;
  }

  ISourceInformation* info = channel.CreateSourceInformation();
  if (info == nullptr) {
    MDF_ERROR() << "Failed to create the SI block. Channel: " << channel.Name();
    return nullptr;
  }

  if (!network_name_.empty()) {
    info->Name(network_name_);
  } else {
    info->Name(group->Name());
  }
  if (!device_name_.empty()) {
    info->Path(device_name_);
  } else {
    info->Path(group->Name());
  }

  info->Type(bus_type_ != 0 ? SourceType::Bus : SourceType::Other);

  switch (bus_type_) {
    case MdfBusType::CAN:
      info->Bus(BusType::Can);
      break;

    case MdfBusType::LIN:
      info->Bus(BusType::Lin);
      break;

    case MdfBusType::MOST:
      info->Bus(BusType::Most);
      break;

    case MdfBusType::FlexRay:
      info->Bus(BusType::FlexRay);
      break;

    case MdfBusType::Ethernet:
      info->Bus(BusType::Ethernet);
      break;

    default:
      info->Bus(BusType::Other);
      break;
  }

  if (!protocol_.empty() || message_id_ != 0 || bus_channel_ != 0
      || !message_name_.empty()) {
    SiComment comment("Configurations for the bus");

    if (bus_channel_ != 0) {
      MdProperty channel_no("ChannelNo", std::to_string(bus_channel_));
      channel_no.DataType(MdDataType::MdInteger);
      comment.AddProperty(channel_no);
    }
    if (message_id_ != 0) {
      MdProperty message_id("MessageID", std::to_string(message_id_));
      message_id.DataType(MdDataType::MdInteger);
      comment.AddProperty(message_id);
    }
    if (message_length_ != 0) {
      MdProperty message_length("MessageLength", std::to_string(message_length_));
      message_length.DataType(MdDataType::MdInteger);
      comment.AddProperty(message_length);
    }
    if (!message_name_.empty())  {
      MdProperty message("Message", message_name_);
      comment.AddProperty(message);
    }
    if (!protocol_.empty()) {
      comment.Protocol(MdString(protocol_));
    }
    info->SetSiComment(comment);
  }
  return info;
}

IChannel* IConfigAdapter::CreateBitChannel(IChannel& parent,
                                             const std::string_view& name,
                                             uint32_t byte_offset,
                                             uint16_t bit_offset) const {
  IChannel* frame_bit = parent.CreateChannelComposition(name);
  if (frame_bit != nullptr) {
    frame_bit->Type(ChannelType::FixedLength);
    frame_bit->Sync(ChannelSyncType::None);
    frame_bit->DataType(ChannelDataType::UnsignedIntegerLe);
    frame_bit->Flags(CnFlag::BusEvent);
    frame_bit->ByteOffset(byte_offset);
    frame_bit->BitOffset(bit_offset);
    frame_bit->BitCount(1);
  }
  return frame_bit;
}

IChannel* IConfigAdapter::CreateBitsChannel(IChannel& parent,
                                           const std::string_view& name,
                                           uint32_t byte_offset,
                                           uint16_t bit_offset,
                                           uint32_t nof_bits ) const {
  IChannel* frame_bits = CreateBitChannel(parent, name, byte_offset, bit_offset);
  if (frame_bits != nullptr) {
    frame_bits->BitCount(nof_bits);
  }
  return frame_bits;
}

std::string IConfigAdapter::MakeGroupName(
    const std::string_view& base_name) const {
  std::ostringstream name;
  std::string bus_name = bus_name_;
  if (bus_name == "FlexRay" || bus_name == "FlxRay") {
    bus_name = "FLX";
  } else if (bus_name.find("CAN") != std::string::npos) {
    // Filter out any CANFD bus types
    bus_name = "CAN";
  }
  switch (filter_type_) {
    case MessageFilter::ChannelFilter:
      name << bus_name << bus_channel_ << "_" << base_name;
      break;

    case MessageFilter::MessageIdFilter:
      name << bus_name;
      if (bus_channel_ > 0) {
        name << bus_channel_;
      }
      name << "_" << base_name;
      name << "_" << message_id_;
      break;

    default:
      name << bus_name << "_" << base_name;
      break;
  }
  return name.str();
}

IChannel* IConfigAdapter::CreateBusChannel( IChannel& parent_channel) const {
  std::ostringstream name;
  name << parent_channel.Name() << ".BusChannel";
  const bool fixed_channel = ChannelFilter() > 0
                             && MessageFilter() != MessageFilter::NoFilter;
  IChannel* frame_bus = parent_channel.CreateChannelComposition(name.str());
  if (frame_bus != nullptr) {
    frame_bus->Type(ChannelType::FixedLength);
    frame_bus->Sync(ChannelSyncType::None);
    frame_bus->DataType(ChannelDataType::UnsignedIntegerLe);
    frame_bus->Flags(CnFlag::BusEvent | CnFlag::RangeValid);
    frame_bus->Range(0, 255);
    if (fixed_channel) {
      if (IChannelConversion* fixed_conv = frame_bus->CreateChannelConversion();
          fixed_conv != nullptr) {
        fixed_conv->Name("Fixed Bus Channel");
        fixed_conv->Type(ConversionType::Linear);
        fixed_conv->Parameter(0, ChannelFilter());
        fixed_conv->Parameter(1, 0.0 );
      }
    }
    frame_bus->ByteOffset(8+0); // Always after the time
    frame_bus->BitOffset(0);
    frame_bus->BitCount(8);
  }
  return frame_bus;
}

IChannel* IConfigAdapter::CreateSubChannel(IChannel& parent_channel,
                                                     const std::string_view& sub_name,
                                                     uint32_t byte_offset,
                                                     uint16_t bit_offset,
                                                     uint32_t nof_bits) const {
  std::ostringstream name;
  name << parent_channel.Name() << "." << sub_name;
  IChannel* sub_channel = nullptr;

  switch (nof_bits) {
    case 0:
      break;
    case 1:
      sub_channel = CreateBitChannel(parent_channel, name.str(),
                                     byte_offset,bit_offset);
      break;

    default:
      sub_channel = CreateBitsChannel(parent_channel, name.str(),
                                      byte_offset,bit_offset, nof_bits);
      break;
  }
  if (sub_channel != nullptr ) {
    sub_channel->Flags(CnFlag::BusEvent);
  }
  return sub_channel;
}

}  // namespace mdf