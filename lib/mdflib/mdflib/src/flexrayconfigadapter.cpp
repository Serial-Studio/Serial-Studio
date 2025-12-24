/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */
#include <string_view>
#include "mdf/flexrayconfigadapter.h"

#include "mdf/ichannel.h"
#include "mdf/ichannelgroup.h"
#include "mdf/idatagroup.h"
#include "mdf/mdflogstream.h"

namespace mdf {

FlexRayConfigAdapter::FlexRayConfigAdapter(const MdfWriter& writer)
    : IConfigAdapter(writer) {
  BusType(MdfBusType::FlexRay);
  BusName("FlexRay");
}

void FlexRayConfigAdapter::CreateConfig(IDataGroup& dg_block) {
  // Transfer internal properties to the data and channel groups.
  dg_block.MandatoryMembersOnly(MandatoryMembersOnly());
  if (StorageType() == MdfStorageType::MlsdStorage) {
    StorageType(MdfStorageType::VlsdStorage);
    MDF_INFO() << "Storage type MLSD is not supported for FlexRay buses. "
               << "Changing to VLSD storage.";
  }

  CreateFlxFrame(dg_block);
  CreateFlxPdu(dg_block);
  CreateFlxFrameHeader(dg_block);
  CreateFlxNullFrame(dg_block);
  CreateFlxErrorFrame(dg_block);
  CreateFlxSymbol(dg_block);
}

void FlexRayConfigAdapter::CreateFlxFrame(IDataGroup& data_group) const {
  const IChannel* cn_data_byte = nullptr;
  if (IChannelGroup* cg_message = data_group.CreateChannelGroup(
          MakeGroupName("Frame"));
      cg_message != nullptr) {
    cg_message->PathSeparator('.');
    cg_message->Flags(CgFlag::PlainBusEvent | CgFlag::BusEvent);
    CreateTimeChannel(*cg_message,"t");
    CreateFrameChannel(*cg_message);
    cn_data_byte = cg_message->GetChannel("FLX_Frame.DataBytes");
    CreateSourceInformation(*cg_message);
  }
  if (StorageType() == MdfStorageType::VlsdStorage && cn_data_byte != nullptr) {
    // Need to add a special CG group for the data samples.
    // Also need to set the VLSD Record ID. The CreateChannelGroup function
    // creates a valid record ID.
    // The VLSD CG group doesn't have any channels.

    if (auto* cg_samples_frame = data_group.CreateChannelGroup("");
        cg_samples_frame != nullptr) {
      cg_samples_frame->Flags(CgFlag::VlsdChannel);
      cn_data_byte->VlsdRecordId(cg_samples_frame->RecordId());
    }
  }
}

void FlexRayConfigAdapter::CreateFlxPdu(IDataGroup& data_group) const {
  const IChannel* cn_data_byte = nullptr;
  if (IChannelGroup* cg_message = data_group.CreateChannelGroup(
          MakeGroupName("Pdu"));
      cg_message != nullptr) {
    cg_message->PathSeparator('.');
    cg_message->Flags(CgFlag::PlainBusEvent | CgFlag::BusEvent);
    CreateTimeChannel(*cg_message,"t");
    CreatePduChannel(*cg_message);
    cn_data_byte = cg_message->GetChannel("FLX_Pdu.DataBytes");
    CreateSourceInformation(*cg_message);
  }
  if (StorageType() == MdfStorageType::VlsdStorage && cn_data_byte != nullptr) {
    // Need to add a special CG group for the data samples.
    // Also need to set the VLSD Record ID. The CreateChannelGroup function
    // creates a valid record ID.
    // The VLSD CG group doesn't have any channels.

    if (auto* cg_samples_frame = data_group.CreateChannelGroup("");
        cg_samples_frame != nullptr) {
      cg_samples_frame->Flags(CgFlag::VlsdChannel);
      cn_data_byte->VlsdRecordId(cg_samples_frame->RecordId());
    }
  }
}

void FlexRayConfigAdapter::CreateFlxFrameHeader(IDataGroup& data_group) const {
  const IChannel* cn_data_byte = nullptr;
  if (IChannelGroup* cg_message = data_group.CreateChannelGroup(
          MakeGroupName("FrameHeader"));
      cg_message != nullptr) {
    cg_message->PathSeparator('.');
    cg_message->Flags(CgFlag::PlainBusEvent | CgFlag::BusEvent);
    CreateTimeChannel(*cg_message,"t");
    CreateFrameHeaderChannel(*cg_message);
    // Note that the channel may be missing
    cn_data_byte = cg_message->GetChannel("FLX_FrameHeader.FillDataBytes");
    CreateSourceInformation(*cg_message);
  }
  if (StorageType() == MdfStorageType::VlsdStorage && cn_data_byte != nullptr) {
    // Need to add a special CG group for the data samples.
    // Also need to set the VLSD Record ID. The CreateChannelGroup function
    // creates a valid record ID.
    // The VLSD CG group doesn't have any channels.

    if (auto* cg_samples_frame = data_group.CreateChannelGroup("");
        cg_samples_frame != nullptr) {
      cg_samples_frame->Flags(CgFlag::VlsdChannel);
      cn_data_byte->VlsdRecordId(cg_samples_frame->RecordId());
    }
  }
}

void FlexRayConfigAdapter::CreateFlxNullFrame(IDataGroup& data_group) const {
  const IChannel* cn_data_byte = nullptr;
  if (IChannelGroup* cg_message = data_group.CreateChannelGroup(
          MakeGroupName("NullFrame"));
      cg_message != nullptr) {
    cg_message->PathSeparator('.');
    cg_message->Flags(CgFlag::PlainBusEvent | CgFlag::BusEvent);
    CreateTimeChannel(*cg_message,"t");
    CreateNullFrameChannel(*cg_message);
    // Note that the channel may be missing
    cn_data_byte = cg_message->GetChannel("FLX_NullFrame.DataBytes");
    CreateSourceInformation(*cg_message);
  }
  if (StorageType() == MdfStorageType::VlsdStorage && cn_data_byte != nullptr) {
    // Need to add a special CG group for the data samples.
    // Also need to set the VLSD Record ID. The CreateChannelGroup function
    // creates a valid record ID.
    // The VLSD CG group doesn't have any channels.

    if (auto* cg_samples_frame = data_group.CreateChannelGroup("");
        cg_samples_frame != nullptr) {
      cg_samples_frame->Flags(CgFlag::VlsdChannel);
      cn_data_byte->VlsdRecordId(cg_samples_frame->RecordId());
    }
  }
}

void FlexRayConfigAdapter::CreateFlxErrorFrame(IDataGroup& data_group) const {
  const IChannel* cn_data_byte = nullptr;
  if (IChannelGroup* cg_message = data_group.CreateChannelGroup(
          MakeGroupName("ErrorFrame"));
      cg_message != nullptr) {
    cg_message->PathSeparator('.');
    cg_message->Flags(CgFlag::PlainBusEvent | CgFlag::BusEvent);
    CreateTimeChannel(*cg_message,"t");
    CreateErrorFrameChannel(*cg_message);
    // Note that the channel may be missing
    cn_data_byte = cg_message->GetChannel("FLX_ErrorFrame.DataBytes");
    CreateSourceInformation(*cg_message);
  }
  if (StorageType() == MdfStorageType::VlsdStorage && cn_data_byte != nullptr) {
    // Need to add a special CG group for the data samples.
    // Also need to set the VLSD Record ID. The CreateChannelGroup function
    // creates a valid record ID.
    // The VLSD CG group doesn't have any channels.

    if (IChannelGroup* cg_samples_frame = data_group.CreateChannelGroup("");
        cg_samples_frame != nullptr) {
      cg_samples_frame->Flags(CgFlag::VlsdChannel);
      cn_data_byte->VlsdRecordId(cg_samples_frame->RecordId());
    }
  }
}

void FlexRayConfigAdapter::CreateFlxSymbol(IDataGroup& data_group) const{

  if (IChannelGroup* cg_message = data_group.CreateChannelGroup(
          MakeGroupName("Symbol"));
      cg_message != nullptr) {
    cg_message->PathSeparator('.');
    cg_message->Flags(CgFlag::PlainBusEvent | CgFlag::BusEvent);
    CreateTimeChannel(*cg_message,"t");
    CreateSymbolChannel(*cg_message);
    CreateSourceInformation(*cg_message);
  }
}

void FlexRayConfigAdapter::CreateFrameChannel(IChannelGroup& channel_group) const {
  const bool mandatory_members_only = MandatoryMembersOnly();
  IChannel* parent_frame = channel_group.CreateChannel("FLX_Frame");
  if (parent_frame == nullptr) {
    MDF_ERROR() << "Failed to create the FLX_Frame channel.";
    return;
  }

  parent_frame->Type(ChannelType::FixedLength);
  parent_frame->Sync(ChannelSyncType::None);
  if (mandatory_members_only) {
    parent_frame->DataBytes(24-8);
  } else {
    parent_frame->DataBytes(34-8);
  }
  parent_frame->Flags(CnFlag::BusEvent);
  parent_frame->DataType(ChannelDataType::ByteArray);
  if (!network_name_.empty()) {
    CreateSourceInformation(*parent_frame);
  }
  CreateBusChannel(*parent_frame);
  CreateSubChannel(*parent_frame, "ID", 9, 0, 11);
  CreateSubChannel(*parent_frame, "Cycle", 11, 0, 6);
  CreateFlxChannelChannel(*parent_frame, 11, 6);
  CreateDirectionChannel(*parent_frame, 11,7);
  if (IChannel* payload = CreateSubChannel(*parent_frame, "PayloadLength", 12, 0, 8);
      payload != nullptr) {
    CnUnit unit("word");
    payload->SetCnUnit(unit);
  }
  if (IChannel* data_length = CreateSubChannel(*parent_frame,"DataLength", 13, 0, 8);
      data_length != nullptr) {
    CnUnit unit("B");
    data_length->SetCnUnit(unit);
  }
  if (mandatory_members_only) {
    CreateDataBytesChannel(*parent_frame, 14);
    return;
  }
  CreateSubChannel(*parent_frame, "CRC", 14,0, 24);
  CreateSubChannel(*parent_frame, "HeaderCRC", 17, 0, 16);
  CreateSubChannel(*parent_frame, "ReservedBitFlag", 19,0, 1);
  CreateSubChannel(*parent_frame, "PayloadPreambleFlag", 19, 1, 1);
  CreateNullFrameChannel(*parent_frame, 19, 2);
  CreateSubChannel(*parent_frame, "SyncFrameFlag", 19, 3, 1);
  CreateSubChannel(*parent_frame,"StartUpFrameFlag", 19, 4, 1);
  if (IChannel* duration = CreateSubChannel(*parent_frame, "FrameLength", 20, 0, 32);
    duration != nullptr ) {
    duration->Unit("ns");
  }
  CreateDataBytesChannel(*parent_frame, 24);
}

void FlexRayConfigAdapter::CreatePduChannel(IChannelGroup& channel_group) const {
  const bool mandatory_members_only = MandatoryMembersOnly();
  IChannel* parent_frame = channel_group.CreateChannel("FLX_Pdu");
  if (parent_frame == nullptr) {
    MDF_ERROR() << "Failed to create the FLX_Pdu channel.";
    return;
  }

  parent_frame->Type(ChannelType::FixedLength);
  parent_frame->Sync(ChannelSyncType::None);
  if (mandatory_members_only) {
    parent_frame->DataBytes(24-8);
  } else {
    parent_frame->DataBytes(28-8);
  }
  parent_frame->Flags(CnFlag::BusEvent);
  parent_frame->DataType(ChannelDataType::ByteArray);
  if (!network_name_.empty()) {
    CreateSourceInformation(*parent_frame);
  }
  CreateBusChannel(*parent_frame);
  CreateSubChannel(*parent_frame, "ID", 9, 0, 11);
  CreateSubChannel(*parent_frame, "Cycle", 11, 0, 6);
  CreateFlxChannelChannel(*parent_frame, 11, 6);
  if (IChannel* pdu = CreateSubChannel(*parent_frame, "PduLength", 12, 0, 8);
      pdu != nullptr) {
    CnUnit unit("B");
    pdu->SetCnUnit(unit);
  }
  if (IChannel* data_length = CreateSubChannel(*parent_frame,"DataLength", 13, 0, 8);
      data_length != nullptr) {
    CnUnit unit("B");
    data_length->SetCnUnit(unit);
  }
  if (mandatory_members_only) {
    CreateDataBytesChannel(*parent_frame, 14);
    return;
  }
  CreateSubChannel(*parent_frame, "IsMultiplexed", 14, 0, 1);
  CreateSubChannel(*parent_frame, "Switch", 15,0, 16);
  CreateSubChannel(*parent_frame, "ValidFlag", 14, 1, 1);
  CreateSubChannel(*parent_frame, "UpdateBitPos", 17, 0, 12);
  CreateSubChannel(*parent_frame, "PduByteOffset", 19, 0, 8);
  CreateSubChannel(*parent_frame, "PduBitOffset", 14, 2, 3);
  CreateDataBytesChannel(*parent_frame, 20);
}

void FlexRayConfigAdapter::CreateFrameHeaderChannel(IChannelGroup& channel_group) const {
  const bool mandatory_members_only = MandatoryMembersOnly();
  IChannel* parent_frame = channel_group.CreateChannel("FLX_FrameHeader");
  if (parent_frame == nullptr) {
    MDF_ERROR() << "Failed to create the FLX_FrameHeader channel.";
    return;
  }

  parent_frame->Type(ChannelType::FixedLength);
  parent_frame->Sync(ChannelSyncType::None);
  if (mandatory_members_only) {
    parent_frame->DataBytes(13-8);
  } else {
    parent_frame->DataBytes(34-8);
  }
  parent_frame->Flags(CnFlag::BusEvent);
  parent_frame->DataType(ChannelDataType::ByteArray);
  if (!network_name_.empty()) {
    CreateSourceInformation(*parent_frame);
  }
  CreateBusChannel(*parent_frame);
  CreateSubChannel(*parent_frame, "ID", 9, 0, 11);
  CreateSubChannel(*parent_frame, "Cycle", 11, 0, 6);
  CreateFlxChannelChannel(*parent_frame, 11, 6);
  CreateDirectionChannel(*parent_frame, 11,7);
  if (IChannel* payload = CreateSubChannel(*parent_frame, "PayloadLength", 12, 0, 8);
      payload != nullptr) {
    CnUnit unit("word");
    payload->SetCnUnit(unit);
  }
  if (mandatory_members_only) {
    return;
  }
  if (IChannel* data_length = CreateSubChannel(*parent_frame,"FillDataLength", 13, 0, 8);
      data_length != nullptr) {
    CnUnit unit("B");
    data_length->SetCnUnit(unit);
  }

  CreateSubChannel(*parent_frame, "CRC", 14,0, 24);
  CreateSubChannel(*parent_frame, "HeaderCRC", 17, 0, 16);
  CreateSubChannel(*parent_frame, "ReservedBitFlag", 19,0, 1);
  CreateSubChannel(*parent_frame, "PayloadPreambleFlag", 19, 1, 1);
  CreateNullFrameChannel(*parent_frame, 19, 2);
  CreateSubChannel(*parent_frame, "SyncFrameFlag", 19, 3, 1);
  CreateSubChannel(*parent_frame,"StartUpFrameFlag", 19, 4, 1);
  if (IChannel* duration = CreateSubChannel(*parent_frame, "FrameLength", 20, 0, 32);
    duration != nullptr ) {
    duration->Unit("ns");
  }
  CreateFillDataBytesChannel(*parent_frame, 24);
}

void FlexRayConfigAdapter::CreateNullFrameChannel(IChannelGroup& channel_group) const {
  const bool mandatory_members_only = MandatoryMembersOnly();
  IChannel* parent_frame = channel_group.CreateChannel("FLX_NullFrame");
  if (parent_frame == nullptr) {
    MDF_ERROR() << "Failed to create the FLX_NullFrame channel.";
    return;
  }

  parent_frame->Type(ChannelType::FixedLength);
  parent_frame->Sync(ChannelSyncType::None);
  if (mandatory_members_only) {
    parent_frame->DataBytes(12-8);
  } else {
    parent_frame->DataBytes(34-8);
  }
  parent_frame->Flags(CnFlag::BusEvent);
  parent_frame->DataType(ChannelDataType::ByteArray);
  if (!network_name_.empty()) {
    CreateSourceInformation(*parent_frame);
  }
  CreateBusChannel(*parent_frame);
  CreateSubChannel(*parent_frame, "ID", 9, 0, 11);
  CreateSubChannel(*parent_frame, "Cycle", 11, 0, 6);
  CreateFlxChannelChannel(*parent_frame, 11, 6);
  CreateDirectionChannel(*parent_frame, 11,7);
  if (mandatory_members_only) {
    return;
  }
  if (IChannel* payload = CreateSubChannel(*parent_frame, "PayloadLength", 12, 0, 8);
      payload != nullptr) {
    CnUnit unit("word");
    payload->SetCnUnit(unit);
  }
  if (IChannel* data_length = CreateSubChannel(*parent_frame,"DataLength", 13, 0, 8);
      data_length != nullptr) {
    CnUnit unit("B");
    data_length->SetCnUnit(unit);
  }
  CreateSubChannel(*parent_frame, "CRC", 14,0, 24);
  CreateSubChannel(*parent_frame, "HeaderCRC", 17, 0, 16);
  CreateSubChannel(*parent_frame, "ReservedBitFlag", 19,0, 1);
  CreateSubChannel(*parent_frame, "PayloadPreambleFlag", 19, 1, 1);
  CreateNullFrameChannel(*parent_frame, 19, 2);
  CreateSubChannel(*parent_frame, "SyncFrameFlag", 19, 3, 1);
  CreateSubChannel(*parent_frame,"StartUpFrameFlag", 19, 4, 1);
  if (IChannel* duration = CreateSubChannel(*parent_frame, "FrameLength", 20, 0, 32);
    duration != nullptr ) {
    duration->Unit("ns");
  }
  CreateDataBytesChannel(*parent_frame, 24);
}

void FlexRayConfigAdapter::CreateErrorFrameChannel(IChannelGroup& channel_group) const {
  const bool mandatory_members_only = MandatoryMembersOnly();
  IChannel* parent_frame = channel_group.CreateChannel("FLX_ErrorFrame");
  if (parent_frame == nullptr) {
    MDF_ERROR() << "Failed to create the FLX_ErrorFrame channel.";
    return;
  }

  parent_frame->Type(ChannelType::FixedLength);
  parent_frame->Sync(ChannelSyncType::None);
  if (mandatory_members_only) {
    parent_frame->DataBytes(12-8);
  } else {
    parent_frame->DataBytes(35-8);
  }
  parent_frame->Flags(CnFlag::BusEvent);
  parent_frame->DataType(ChannelDataType::ByteArray);
  if (!network_name_.empty()) {
    CreateSourceInformation(*parent_frame);
  }
  CreateBusChannel(*parent_frame);
  CreateSubChannel(*parent_frame, "ID", 9, 0, 11);
  CreateSubChannel(*parent_frame, "Cycle", 11, 0, 6);
  CreateFlxChannelChannel(*parent_frame, 11, 6);
  CreateDirectionChannel(*parent_frame, 11,7);
  if (mandatory_members_only) {
    return;
  }
  if (IChannel* payload = CreateSubChannel(*parent_frame, "PayloadLength", 12, 0, 8);
      payload != nullptr) {
    CnUnit unit("word");
    payload->SetCnUnit(unit);
  }
  if (IChannel* data_length = CreateSubChannel(*parent_frame,"DataLength", 13, 0, 8);
      data_length != nullptr) {
    CnUnit unit("B");
    data_length->SetCnUnit(unit);
  }
  CreateSubChannel(*parent_frame, "CRC", 14,0, 24);
  CreateSubChannel(*parent_frame, "HeaderCRC", 17, 0, 16);
  CreateSubChannel(*parent_frame, "ReservedBitFlag", 19,0, 1);
  CreateSubChannel(*parent_frame, "PayloadPreambleFlag", 19, 1, 1);
  CreateNullFrameChannel(*parent_frame, 19, 2);
  CreateSubChannel(*parent_frame, "SyncFrameFlag", 19, 3, 1);
  CreateSubChannel(*parent_frame,"StartUpFrameFlag", 19, 4, 1);
  CreateSubChannel(*parent_frame,"IsSyntaxError", 19, 5, 1);
  CreateSubChannel(*parent_frame,"IsContentError", 19, 6, 1);
  CreateSubChannel(*parent_frame,"IsSlotBndViolation", 19, 7, 1);
  CreateSubChannel(*parent_frame,"IsTxConflict", 20, 0, 1);
  CreateSubChannel(*parent_frame,"IsValidFrame", 20, 1, 1);
  if (IChannel* duration = CreateSubChannel(*parent_frame, "FrameLength", 21, 0, 32);
    duration != nullptr ) {
    duration->Unit("ns");
  }
  CreateDataBytesChannel(*parent_frame, 25);
}

void FlexRayConfigAdapter::CreateSymbolChannel(IChannelGroup& channel_group) const {
  const bool mandatory_members_only = MandatoryMembersOnly();
  IChannel* parent_frame = channel_group.CreateChannel("FLX_Symbol");
  if (parent_frame == nullptr) {
    MDF_ERROR() << "Failed to create the FLX_Symbol channel.";
    return;
  }

  parent_frame->Type(ChannelType::FixedLength);
  parent_frame->Sync(ChannelSyncType::None);
  if (mandatory_members_only) {
    parent_frame->DataBytes(10-8);
  } else {
    parent_frame->DataBytes(12-8);
  }
  parent_frame->Flags(CnFlag::BusEvent);
  parent_frame->DataType(ChannelDataType::ByteArray);
  if (!network_name_.empty()) {
    CreateSourceInformation(*parent_frame);
  }
  CreateBusChannel(*parent_frame);
  CreateSubChannel(*parent_frame, "Cycle", 9, 0, 6);
  CreateFlxChannelMaskChannel(*parent_frame, 9, 6);
  if (mandatory_members_only) {
    return;
  }
  if (IChannel* length = CreateSubChannel(*parent_frame, "SymbolLength", 10, 0, 8);
      length != nullptr ) {
    CnUnit unit("MT");
    length->SetCnUnit(unit);
  }

  CreateSymbolTypeChannel(*parent_frame, 11, 0);
}

void FlexRayConfigAdapter::CreateFlxChannelChannel(IChannel& parent_channel,
                                                  uint32_t byte_offset,
                                                  uint16_t bit_offset) const {
  std::ostringstream name;
  name << parent_channel.Name() << ".FlxChannel";

  if (IChannel* flx = CreateBitChannel(parent_channel, name.str(),
                                       byte_offset,bit_offset);
      flx != nullptr) {
    flx->Flags(CnFlag::BusEvent);
  }
}

void FlexRayConfigAdapter::CreateFlxChannelMaskChannel(IChannel& parent_channel,
                                                   uint32_t byte_offset,
                                                   uint16_t bit_offset) const {
  std::ostringstream name;
  name << parent_channel.Name() << ".FlxChannelMask";

  if (IChannel* flx = CreateBitsChannel(parent_channel, name.str(),
                                       byte_offset,bit_offset, 2);
      flx != nullptr) {
    flx->Flags(CnFlag::BusEvent);
  }
}

void FlexRayConfigAdapter::CreateSymbolTypeChannel(IChannel& parent_channel,
                                                       uint32_t byte_offset,
                                                       uint16_t bit_offset) const {
  std::ostringstream name;
  name << parent_channel.Name() << ".SymbolType";

  if (IChannel* type = CreateBitsChannel(parent_channel, name.str(),
                                        byte_offset,bit_offset, 2);
      type != nullptr) {
    type->Flags(CnFlag::BusEvent);
  }
}

void FlexRayConfigAdapter::CreateDirectionChannel(IChannel& parent_channel,
                                                  uint32_t byte_offset,
                                                  uint16_t bit_offset) const {
  std::ostringstream name;
  name << parent_channel.Name() << ".Dir";

  if (IChannel* dir = CreateBitChannel(parent_channel, name.str(),
                                       byte_offset,bit_offset);
      dir != nullptr) {
    dir->Flags(CnFlag::BusEvent);
  }
}

void FlexRayConfigAdapter::CreateNullFrameChannel(IChannel& parent_channel,
                                                  uint32_t byte_offset,
                                                  uint16_t bit_offset) const {
  std::ostringstream name;
  name << parent_channel.Name() << ".NullFrameFlag";

  if (IChannel* null_frame = CreateBitChannel(parent_channel, name.str(),
                                       byte_offset,bit_offset);
      null_frame != nullptr) {
    null_frame->Flags(CnFlag::BusEvent);
  }
}

void FlexRayConfigAdapter::CreateDataBytesChannel(IChannel& parent_channel,
                                               uint32_t byte_offset) const {
  std::ostringstream name;
  name << parent_channel.Name() << ".DataBytes";
  if (IChannel* bytes = CreateBitsChannel(parent_channel, name.str(),
                                          byte_offset,0, 64);
      bytes != nullptr ) {
    bytes->Flags(CnFlag::BusEvent);
    bytes->DataType(ChannelDataType::ByteArray);
    bytes->Type(ChannelType::VariableLength);
  }
}

void FlexRayConfigAdapter::CreateFillDataBytesChannel(IChannel& parent_channel,
                                                  uint32_t byte_offset) const {
  std::ostringstream name;
  name << parent_channel.Name() << ".FillDataBytes";
  if (IChannel* bytes = CreateBitsChannel(parent_channel, name.str(),
                                          byte_offset,0, 64);
      bytes != nullptr ) {
    bytes->Flags(CnFlag::BusEvent);
    bytes->DataType(ChannelDataType::ByteArray);
    bytes->Type(ChannelType::VariableLength);
  }
}
}  // namespace mdf