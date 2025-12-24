/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */
#include <sstream>

#include "mdf/canconfigadapter.h"
#include "mdf/ichannelgroup.h"
#include "mdf/idatagroup.h"
#include "mdf/mdfwriter.h"
#include "mdf/mdflogstream.h"

namespace mdf {

CanConfigAdapter::CanConfigAdapter(const MdfWriter& writer)
    : IConfigAdapter(writer) {
  BusType(MdfBusType::CAN);
  BusName("CAN");
}

void CanConfigAdapter::CreateConfig(IDataGroup& dg_block) {
  const bool mandatory_only = MandatoryMembersOnly();
  dg_block.MandatoryMembersOnly(mandatory_only);

  if (StorageType() == MdfStorageType::MlsdStorage && MaxLength() < 8) {
    MaxLength(8);
  }
  // The cn_data_byte points to the channel that defines the raw data bytes.
  // This channel may be updated later to pint to the CG VLSD group.
  const IChannel* cn_data_byte = nullptr;
  if (IChannelGroup* cg_data_frame = dg_block.CreateChannelGroup(
          MakeGroupName("DataFrame"));
      cg_data_frame != nullptr) {
    cg_data_frame->PathSeparator('.');
    cg_data_frame->Flags(CgFlag::PlainBusEvent | CgFlag::BusEvent);
    CreateTimeChannel(*cg_data_frame,"t");
    CreateCanDataFrameChannel(*cg_data_frame);
    cn_data_byte = cg_data_frame->GetChannel("CAN_DataFrame.DataBytes");
    CreateSourceInformation(*cg_data_frame);
    cg_data_frame->MaxLength(MaxLength());
    cg_data_frame->StorageType(StorageType());
  }

  if (StorageType() == MdfStorageType::VlsdStorage && cn_data_byte != nullptr) {
    // Need to add a special CG group for the data samples.
    // Also need to set the VLSD Record ID. The CreateChannelGroup function
    // creates a valid record ID.
    // The VLSD CG group doesn't have any channels.

    if (IChannelGroup* cg_samples_frame = dg_block.CreateChannelGroup("");
        cg_samples_frame != nullptr) {
      cg_samples_frame->Flags(CgFlag::VlsdChannel);
      cg_samples_frame->MaxLength(MaxLength());
      cg_samples_frame->StorageType(StorageType());
      cn_data_byte->VlsdRecordId(cg_samples_frame->RecordId());
    }
  }

  if (IChannelGroup* cg_remote_frame = dg_block.CreateChannelGroup(
          MakeGroupName("RemoteFrame"));
      cg_remote_frame != nullptr) {
    // The remote frame doesn't store any data bytes.
    cg_remote_frame->PathSeparator('.');
    cg_remote_frame->Flags(CgFlag::PlainBusEvent | CgFlag::BusEvent);
    CreateTimeChannel(*cg_remote_frame,"t");
    CreateCanRemoteFrameChannel(*cg_remote_frame);
    CreateSourceInformation(*cg_remote_frame);
    cg_remote_frame->MaxLength(MaxLength());
    cg_remote_frame->StorageType(StorageType());
  }

  // Similar to the data frame, the error frame may store data bytes.
  const IChannel* cn_error_byte = nullptr;
  if (IChannelGroup* cg_error_frame = dg_block.CreateChannelGroup(
          MakeGroupName("ErrorFrame"));
      cg_error_frame != nullptr) {
    cg_error_frame->PathSeparator('.');
    cg_error_frame->Flags(CgFlag::PlainBusEvent | CgFlag::BusEvent);
    CreateTimeChannel(*cg_error_frame,"t");
    CreateCanErrorFrameChannel(*cg_error_frame);
    cn_error_byte = cg_error_frame->GetChannel("CAN_ErrorFrame.DataBytes");
    CreateSourceInformation(*cg_error_frame);
    cg_error_frame->MaxLength(MaxLength());
    cg_error_frame->StorageType(StorageType());
  }

  if (StorageType() == MdfStorageType::VlsdStorage && cn_error_byte != nullptr) {
    // Need to add a special CG group for the error samples
    if (auto* cg_errors_frame = dg_block.CreateChannelGroup("");
        cg_errors_frame != nullptr) {
      cg_errors_frame->Flags(CgFlag::VlsdChannel);
      cg_errors_frame->MaxLength(MaxLength());
      cg_errors_frame->StorageType(StorageType());
      cn_error_byte->VlsdRecordId(cg_errors_frame->RecordId());
    }
  }

  if (mandatory_only) {
    // Do not include the overload frame as it rarely is used.
    return;
  }

  if (IChannelGroup* cg_overload_frame = dg_block.CreateChannelGroup(
          MakeGroupName("OverloadFrame"));
      cg_overload_frame != nullptr) {
    cg_overload_frame->PathSeparator('.');
    cg_overload_frame->Flags(CgFlag::PlainBusEvent | CgFlag::BusEvent);
    CreateTimeChannel(*cg_overload_frame,"t");
    CreateCanOverloadFrameChannel(*cg_overload_frame);
    CreateSourceInformation(*cg_overload_frame);
    cg_overload_frame->MaxLength(MaxLength());
    cg_overload_frame->StorageType(StorageType());
  }
}

void CanConfigAdapter::CreateCanDataFrameChannel(IChannelGroup& group) const {
  const bool mandatory_only = MandatoryMembersOnly();
  IChannel* cn_data_frame = group.CreateChannel("CAN_DataFrame");
  if (cn_data_frame == nullptr) {
    MDF_ERROR() << "Failed to create the CAN_DataFrame channel.";
    return;
  }

  cn_data_frame->Type(ChannelType::FixedLength);
  cn_data_frame->Sync(ChannelSyncType::None);
  switch (StorageType()) {
    // In reality,  MLSD storage should have max length equal to 8.
    case MdfStorageType::MlsdStorage:
      cn_data_frame->DataBytes(mandatory_only ? (23 - 16) + writer_.MaxLength() :
                                (32 - 16) + writer_.MaxLength());
      break;

    case MdfStorageType::FixedLengthStorage:
    case MdfStorageType::VlsdStorage:
    default:
      cn_data_frame->DataBytes(mandatory_only ? 23 - 8 : 32 - 8); // Index into SD or VLSD
      break;
  }
  cn_data_frame->Flags(CnFlag::BusEvent);
  cn_data_frame->DataType(ChannelDataType::ByteArray);
  if (!network_name_.empty()) {
    CreateSourceInformation(*cn_data_frame);
  }

  CreateBusChannel(*cn_data_frame);
  CreateBitsChannel(*cn_data_frame,"CAN_DataFrame.ID", 9,0, 29);
  CreateBitChannel(*cn_data_frame,"CAN_DataFrame.IDE", 12, 7);
  CreateBitsChannel(*cn_data_frame, "CAN_DataFrame.DLC", 13, 0, 4);
  if (IChannel* length = CreateBitsChannel(*cn_data_frame, "CAN_DataFrame.DataLength", 14, 0, 8);
    length != nullptr) {
    length->Unit("B");
  }
  if (mandatory_only) {
   CreateDataBytesChannel(*cn_data_frame, 15);
   return;
  }

  CreateDirChannel(*cn_data_frame, 13, 4);
  CreateBitChannel(*cn_data_frame,"CAN_DataFrame.SRR", 13, 5);
  CreateBitChannel(*cn_data_frame,"CAN_DataFrame.EDL", 13, 6);
  CreateBitChannel(*cn_data_frame,"CAN_DataFrame.BRS", 13, 7);
  CreateBitChannel(*cn_data_frame,"CAN_DataFrame.ESI", 15, 0);
  CreateBitChannel(*cn_data_frame,"CAN_DataFrame.WakeUp", 15, 1);
  CreateBitChannel(*cn_data_frame,"CAN_DataFrame.SingleWire", 15, 2);
  CreateBitChannel(*cn_data_frame,"CAN_DataFrame.R0", 15, 3);
  CreateBitChannel(*cn_data_frame,"CAN_DataFrame.R1", 15, 4);
  if (IChannel* duration = CreateBitsChannel(*cn_data_frame, "CAN_DataFrame.FrameDuration", 16, 0, 32);
    duration != nullptr) {
    duration->Unit("ns");
  }
  CreateBitsChannel(*cn_data_frame, "CAN_DataFrame.CRC", 20, 0, 32);
  CreateDataBytesChannel(*cn_data_frame, 24 );
}

void CanConfigAdapter::CreateCanRemoteFrameChannel(IChannelGroup& group) const {
  const bool mandatory_only = MandatoryMembersOnly();
  IChannel* cn_remote_frame = group.CreateChannel("CAN_RemoteFrame");
  if (cn_remote_frame == nullptr) {
    MDF_ERROR() << "Failed to create the CAN_RemoteFrame channel.";
    return;
  }
  cn_remote_frame->Type(ChannelType::FixedLength);
  cn_remote_frame->Sync(ChannelSyncType::None);
  cn_remote_frame->DataType(ChannelDataType::ByteArray);
  cn_remote_frame->DataBytes(mandatory_only ? 15 - 8 : 24 - 8);
  cn_remote_frame->Flags(CnFlag::BusEvent);

  if (!network_name_.empty()) {
    CreateSourceInformation(*cn_remote_frame);
  }
  CreateBusChannel(*cn_remote_frame);
  CreateBitsChannel(*cn_remote_frame,"CAN_RemoteFrame.ID", 9,0, 29);
  CreateBitChannel(*cn_remote_frame,"CAN_RemoteFrame.IDE", 12, 7);
  CreateBitsChannel(*cn_remote_frame, "CAN_RemoteFrame.DLC", 13, 0, 4);
  if (IChannel* length = CreateBitsChannel(*cn_remote_frame, "CAN_RemoteFrame.DataLength", 14, 0, 8);
    length != nullptr ) {
    length->Unit("B");
  }

  if (mandatory_only) {
    return;
  }
  CreateDirChannel(*cn_remote_frame, 13,4);
  CreateBitChannel(*cn_remote_frame,"CAN_RemoteFrame.SRR", 13, 5);
  CreateBitChannel(*cn_remote_frame,"CAN_RemoteFrame.WakeUp", 13, 6);
  CreateBitChannel(*cn_remote_frame,"CAN_RemoteFrame.SingleWire", 13, 7);
  CreateBitChannel(*cn_remote_frame,"CAN_RemoteFrame.R0", 15, 0);
  CreateBitChannel(*cn_remote_frame,"CAN_RemoteFrame.R1", 15, 1);
  if (IChannel* duration = CreateBitsChannel(*cn_remote_frame, "CAN_RemoteFrame.FrameDuration", 16, 0, 32);
    duration != nullptr) {
    duration->Unit("ns");
  }
  CreateBitsChannel(*cn_remote_frame, "CAN_RemoteFrame.CRC", 20, 0, 32);
}

void CanConfigAdapter::CreateCanErrorFrameChannel(IChannelGroup& group) const {
  IChannel* cn_error_frame = group.CreateChannel("CAN_ErrorFrame");
  if (cn_error_frame == nullptr) {
    MDF_ERROR() << "Failed to create the CAN_ErrorFrame channel.";
    return;
  }
  cn_error_frame->Type(ChannelType::FixedLength);
  cn_error_frame->Sync(ChannelSyncType::None);
  cn_error_frame->Flags(CnFlag::BusEvent);
  cn_error_frame->DataType(ChannelDataType::ByteArray);

  if (StorageType() == MdfStorageType::MlsdStorage) {
    cn_error_frame->DataBytes((34 - 16) + writer_.MaxLength());
  } else {
    cn_error_frame->DataBytes(34 - 8); // Index into SD or VLSD
  }

  if (!network_name_.empty()) {
    CreateSourceInformation(*cn_error_frame);
  }

  CreateBusChannel(*cn_error_frame);
  CreateBitsChannel(*cn_error_frame,"CAN_ErrorFrame.ID", 9,0, 29);
  CreateBitChannel(*cn_error_frame,"CAN_ErrorFrame.IDE", 12, 7);
  CreateBitsChannel(*cn_error_frame, "CAN_ErrorFrame.DLC", 13, 0, 4);
  if (IChannel* length = CreateBitsChannel(*cn_error_frame, "CAN_ErrorFrame.DataLength", 14, 0, 8);
    length != nullptr) {
    length->Unit("B");
  }
  CreateDirChannel(*cn_error_frame, 13, 4);
  CreateBitChannel(*cn_error_frame,"CAN_ErrorFrame.SRR", 13, 5);
  CreateBitChannel(*cn_error_frame,"CAN_ErrorFrame.EDL", 13, 6);
  CreateBitChannel(*cn_error_frame,"CAN_ErrorFrame.BRS", 13, 7);
  CreateBitChannel(*cn_error_frame,"CAN_ErrorFrame.ESI", 15, 0);
  CreateBitChannel(*cn_error_frame,"CAN_ErrorFrame.WakeUp", 15, 1);
  CreateBitChannel(*cn_error_frame,"CAN_ErrorFrame.SingleWire", 15, 2);
  CreateBitChannel(*cn_error_frame,"CAN_ErrorFrame.R0", 15, 3);
  CreateBitChannel(*cn_error_frame,"CAN_ErrorFrame.R1", 15, 4);
  CreateErrorTypeChannel(*cn_error_frame, 15, 5);
  if (IChannel* duration = CreateBitsChannel(*cn_error_frame, "CAN_ErrorFrame.FrameDuration", 16, 0, 32);
    duration != nullptr ) {
    duration->Unit("ns");
  }
  CreateBitsChannel(*cn_error_frame, "CAN_ErrorFrame.BitPosition", 20, 0, 16);
  CreateBitsChannel(*cn_error_frame, "CAN_ErrorFrame.CRC", 22, 0, 32);
  CreateDataBytesChannel(*cn_error_frame, 26 );
}

void CanConfigAdapter::CreateCanOverloadFrameChannel(IChannelGroup& group) {
  auto* cn_overload_frame = group.CreateChannel("CAN_OverloadFrame");
  if (cn_overload_frame == nullptr) {
    MDF_ERROR() << "Failed to create the CAN_OverloadFrame channel.";
    return;
  }
  cn_overload_frame->Type(ChannelType::FixedLength);
  cn_overload_frame->Sync(ChannelSyncType::None);
  cn_overload_frame->DataType(ChannelDataType::ByteArray);
  cn_overload_frame->DataBytes(2);
  cn_overload_frame->Flags(CnFlag::BusEvent);

  if (!network_name_.empty()) {
    CreateSourceInformation(*cn_overload_frame);
  }
  CreateBusChannel(*cn_overload_frame);
  CreateDirChannel(*cn_overload_frame, 9, 0);
}

IChannel* CanConfigAdapter::CreateDataBytesChannel(IChannel& parent_channel,
                                                   uint16_t byte_offset) const {
  std::ostringstream name;
  name << parent_channel.Name() << ".DataBytes";
  IChannel* frame_bytes = parent_channel.CreateChannelComposition(name.str());
  if (frame_bytes != nullptr) {

    switch(StorageType()) {
      case MdfStorageType::MlsdStorage:
        frame_bytes->Type(ChannelType::MaxLength);
        frame_bytes->BitCount(8 * writer_.MaxLength());
        break;

      case MdfStorageType::VlsdStorage:
      default:
        frame_bytes->Type(ChannelType::VariableLength);
        frame_bytes->BitCount(64); // Index to SD or VLSD CG block
        break;
    }
    frame_bytes->Sync(ChannelSyncType::None);
    frame_bytes->DataType(ChannelDataType::ByteArray);
    frame_bytes->Flags(CnFlag::BusEvent);
    frame_bytes->ByteOffset(byte_offset);
    frame_bytes->BitOffset(0);
  }
  return frame_bytes;
}

IChannel* CanConfigAdapter::CreateDirChannel(IChannel& parent_channel,
                                             uint16_t byte_offset,
                                             uint8_t bit_offset) const {
  std::ostringstream name;
  name << parent_channel.Name() << ".Dir";

  return CreateBitChannel(parent_channel,name.str(),
                                   byte_offset, bit_offset);
}

IChannel* CanConfigAdapter::CreateErrorTypeChannel(IChannel& parent_channel,
                                             uint16_t byte_offset,
                                             uint8_t bit_offset) const {
  std::ostringstream name;
  name << parent_channel.Name() << ".ErrorType";

  IChannel* type = CreateBitsChannel(parent_channel,name.str(),
                                   byte_offset, bit_offset, 3);
  return type;
}
}  // namespace mdf