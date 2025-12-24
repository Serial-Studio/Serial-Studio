/*
* Copyright 2024 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include "mdf/linconfigadapter.h"
#include "mdf/ichannelgroup.h"
#include "mdf/idatagroup.h"
#include "mdf/mdflogstream.h"

namespace mdf {
LinConfigAdapter::LinConfigAdapter(const MdfWriter& writer)
    : IConfigAdapter(writer) {
  BusType(MdfBusType::LIN);
  BusName("LIN");
  StorageType(MdfStorageType::MlsdStorage);
  MaxLength(8);
}

void LinConfigAdapter::CreateConfig(IDataGroup& dg_block) {
  dg_block.MandatoryMembersOnly(MandatoryMembersOnly());

  if (StorageType() != MdfStorageType::MlsdStorage) {
    StorageType(MdfStorageType::MlsdStorage);
    MaxLength(8);
  }

  if (IChannelGroup* cg_frame = dg_block.CreateChannelGroup(
          MakeGroupName("Frame"));
    cg_frame != nullptr) {
    cg_frame->PathSeparator('.');
    cg_frame->Flags(CgFlag::PlainBusEvent | CgFlag::BusEvent);
    CreateSourceInformation(*cg_frame);
    CreateTimeChannel(*cg_frame, "t");
    CreateFrameChannels(*cg_frame);
  }

  if (IChannelGroup* cg_checksum_error = dg_block.CreateChannelGroup(
          MakeGroupName("ChecksumError"));
      cg_checksum_error != nullptr ) {
    cg_checksum_error->PathSeparator('.');
    cg_checksum_error->Flags(CgFlag::PlainBusEvent | CgFlag::BusEvent);
    CreateSourceInformation(*cg_checksum_error);
    CreateTimeChannel(*cg_checksum_error, "t");
    CreateChecksumErrorChannels(*cg_checksum_error);
  }

  if (IChannelGroup* cg_receive_error = dg_block.CreateChannelGroup(
          MakeGroupName("ReceiveError"));
      cg_receive_error != nullptr ) {
    cg_receive_error->PathSeparator('.');
    cg_receive_error->Flags(CgFlag::PlainBusEvent | CgFlag::BusEvent);
    CreateSourceInformation(*cg_receive_error);
    CreateTimeChannel(*cg_receive_error, "t");
    CreateReceiveErrorChannels(*cg_receive_error);
  }

  if (IChannelGroup* cg_sync_error = dg_block.CreateChannelGroup(
          MakeGroupName("SyncError"));
      cg_sync_error != nullptr ) {
    cg_sync_error->PathSeparator('.');
    cg_sync_error->Flags(CgFlag::PlainBusEvent | CgFlag::BusEvent);
    CreateSourceInformation(*cg_sync_error);
    CreateTimeChannel(*cg_sync_error, "t");
    CreateSyncChannels(*cg_sync_error);
  }
  if (IChannelGroup* cg_transmit_error = dg_block.CreateChannelGroup(
          MakeGroupName("TransmissionError"));
      cg_transmit_error != nullptr ) {
    cg_transmit_error->PathSeparator('.');
    cg_transmit_error->Flags(CgFlag::PlainBusEvent | CgFlag::BusEvent);
    CreateSourceInformation(*cg_transmit_error);
    CreateTimeChannel(*cg_transmit_error, "t");
    CreateTransmissionErrorChannels(*cg_transmit_error);
  }

   if (IChannelGroup* cg_wakeUp = dg_block.CreateChannelGroup(
          MakeGroupName("WakeUp"));
      cg_wakeUp != nullptr ) {
    cg_wakeUp->PathSeparator('.');
    cg_wakeUp->Flags(CgFlag::PlainBusEvent | CgFlag::BusEvent);
    CreateSourceInformation(*cg_wakeUp);
    CreateTimeChannel(*cg_wakeUp, "t");
    CreateWakeUpChannels(*cg_wakeUp);
  }

  if (IChannelGroup* cg_spike = dg_block.CreateChannelGroup(
          MakeGroupName("Spike"));
      cg_spike != nullptr ) {
    cg_spike->PathSeparator('.');
    cg_spike->Flags(CgFlag::PlainBusEvent | CgFlag::BusEvent);
    CreateSourceInformation(*cg_spike);
    CreateTimeChannel(*cg_spike, "t");
    CreateSpikeChannels(*cg_spike);
  }

  if (IChannelGroup* cg_long_dom = dg_block.CreateChannelGroup(
          MakeGroupName("LongDom"));
      cg_long_dom != nullptr ) {
    cg_long_dom->PathSeparator('.');
    cg_long_dom->Flags(CgFlag::PlainBusEvent | CgFlag::BusEvent);
    CreateSourceInformation(*cg_long_dom);
    CreateTimeChannel(*cg_long_dom, "t");
    CreateLongDominantChannels(*cg_long_dom);
  }
}

void LinConfigAdapter::CreateFrameChannels(IChannelGroup& group) const {
  const bool mandatory_only = MandatoryMembersOnly();
  IChannel* cn_frame = group.CreateChannel("LIN_Frame");
  if (cn_frame == nullptr) {
    MDF_ERROR() << "Failed to create the LIN_Frame channel.";
    return;
  }

  cn_frame->Type(ChannelType::FixedLength);
  cn_frame->Sync(ChannelSyncType::None);
  cn_frame->DataBytes(mandatory_only ? 19 - 8 : 45 - 8);
  cn_frame->Flags(CnFlag::BusEvent);
  cn_frame->DataType(ChannelDataType::ByteArray);
  if (!network_name_.empty()) {
    CreateSourceInformation(*cn_frame);
  }
  CreateBusChannel(*cn_frame);
  CreateBitsChannel(*cn_frame, "LIN_Frame.ID", 9, 0, 6);
  CreateBitChannel(*cn_frame, "LIN_Frame.Dir", 9, 7);

  if (IChannel* count = CreateBitsChannel(*cn_frame,"LIN_Frame.ReceivedDataByteCount", 10, 0, 4);
    count != nullptr) {
    count->Unit("B");
  }

  if (IChannel* length = CreateBitsChannel(*cn_frame,"LIN_Frame.DataLength", 10, 4, 4);
    length != nullptr) {
    length->Unit("B");
  }

  if (IChannel* frame_bytes =
          cn_frame->CreateChannelComposition("LIN_Frame.DataBytes");
      frame_bytes != nullptr) {
    frame_bytes->Type(ChannelType::MaxLength);
    frame_bytes->BitCount(64);
    frame_bytes->Sync(ChannelSyncType::None);
    frame_bytes->DataType(ChannelDataType::ByteArray);
    frame_bytes->Flags(CnFlag::BusEvent);
    frame_bytes->ByteOffset(11);
    frame_bytes->BitOffset(0);
  }

  if (mandatory_only) {
    return;
  }

  CreateBitsChannel(*cn_frame,"LIN_Frame.Checksum", 19, 0, 8);

  if (IChannel* crc_model = CreateBitsChannel(*cn_frame,"LIN_Frame.ChecksumModel", 20, 0, 2);
      crc_model != nullptr) {
    crc_model->DataType(ChannelDataType::SignedIntegerLe);
  }
  if ( IChannel* sof = CreateBitsChannel(*cn_frame, "LIN_Frame.SOF", 21, 0, 64);
      sof != nullptr) {
     sof->Unit("ns");
  }

  if (IChannel* baudrate =
          CreateBitsChannel(*cn_frame, "LIN_Frame.Baudrate", 29, 0, 32);
      baudrate != nullptr) {
    baudrate->DataType(ChannelDataType::FloatLe);
    baudrate->Unit("bits/s");
  }

  if (IChannel* resp =
          CreateBitsChannel(*cn_frame,"LIN_Frame.ResponseBaudrate", 33, 0 , 32);
      resp != nullptr) {
    resp->DataType(ChannelDataType::FloatLe);
    resp->Unit("bits/s");
  }

  if (IChannel* break_length = CreateBitsChannel(*cn_frame, "LIN_Frame.BreakLength", 37, 0, 32);
      break_length != nullptr) {
    break_length->Unit("ns");
  }

  if (IChannel* break_length = CreateBitsChannel(*cn_frame, "LIN_Frame.BreakDelimiterLength", 41, 0, 32);
      break_length != nullptr) {
    break_length->Unit("ns");
  }
}

void LinConfigAdapter::CreateChecksumErrorChannels(mdf::IChannelGroup& group) const {
const bool mandatory_only = MandatoryMembersOnly();
  IChannel* cn_frame = group.CreateChannel("LIN_ChecksumError");
  if (cn_frame == nullptr) {
    MDF_ERROR() << "Failed to create the LIN_ChecksumError channel.";
    return;
  }

  cn_frame->Type(ChannelType::FixedLength);
  cn_frame->Sync(ChannelSyncType::None);
  cn_frame->DataBytes(mandatory_only ? 10 - 8 : 45 - 8);
  cn_frame->Flags(CnFlag::BusEvent);
  cn_frame->DataType(ChannelDataType::ByteArray);
  if (!network_name_.empty()) {
    CreateSourceInformation(*cn_frame);
  }
  CreateBusChannel(*cn_frame);
  CreateBitsChannel(*cn_frame, "LIN_ChecksumError.ID", 9, 0, 6);

  CreateBitChannel(*cn_frame, "LIN_ChecksumError.Dir", 9, 7);

  if (mandatory_only) {
    return;
  }

  if (IChannel* count = CreateBitsChannel(*cn_frame,"LIN_ChecksumError.ReceivedDataByteCount", 10, 0, 4);
    count != nullptr ) {
    count->Unit("B");
  }
  if (IChannel* length = CreateBitsChannel(*cn_frame,"LIN_ChecksumError.DataLength", 10, 4, 4);
    length != nullptr) {
    length->Unit("B");
  }

  if (IChannel* frame_bytes =
          cn_frame->CreateChannelComposition("LIN_ChecksumError..DataBytes");
      frame_bytes != nullptr) {
    frame_bytes->Type(ChannelType::MaxLength);
    frame_bytes->BitCount(64);
    frame_bytes->Sync(ChannelSyncType::None);
    frame_bytes->DataType(ChannelDataType::ByteArray);
    frame_bytes->Flags(CnFlag::BusEvent);
    frame_bytes->ByteOffset(11);
    frame_bytes->BitOffset(0);
  }

  CreateBitsChannel(*cn_frame,"LIN_ChecksumError.Checksum", 19, 0, 8);

  if (IChannel* crc_model = CreateBitsChannel(*cn_frame,"LIN_ChecksumError.ChecksumModel", 20, 0, 2);
      crc_model != nullptr) {
    crc_model->DataType(ChannelDataType::SignedIntegerLe);
  }
  if ( IChannel* sof = CreateBitsChannel(*cn_frame, "LIN_ChecksumError..SOF", 21, 0, 64);
      sof != nullptr) {
     sof->Unit("ns");
  }

  if (IChannel* baudrate =
          CreateBitsChannel(*cn_frame, "LIN_ChecksumError.Baudrate", 29, 0, 32);
      baudrate != nullptr) {
    baudrate->DataType(ChannelDataType::FloatLe);
    baudrate->Unit("bits/s");
  }

  if (IChannel* resp =
          CreateBitsChannel(*cn_frame,"LIN_ChecksumError.ResponseBaudrate", 33, 0 , 32);
      resp != nullptr) {
    resp->DataType(ChannelDataType::FloatLe);
    resp->Unit("bits/s");
  }

  if (IChannel* break_length = CreateBitsChannel(*cn_frame, "LIN_ChecksumError.BreakLength", 37, 0, 32);
      break_length != nullptr) {
    break_length->Unit("ns");
  }

  if (IChannel* break_length = CreateBitsChannel(*cn_frame, "LIN_ChecksumError.BreakDelimiterLength", 41, 0, 32);
      break_length != nullptr) {
    break_length->Unit("ns");
  }

}

void LinConfigAdapter::CreateReceiveErrorChannels(IChannelGroup& group) const {
  IChannel* cn_frame = group.CreateChannel("LIN_ReceiveError");
  if (cn_frame == nullptr) {
    MDF_ERROR() << "Failed to create the LIN_ReceiveError channel.";
    return;
  }

  cn_frame->Type(ChannelType::FixedLength);
  cn_frame->Sync(ChannelSyncType::None);
  cn_frame->DataBytes(44 - 8);
  cn_frame->Flags(CnFlag::BusEvent);
  cn_frame->DataType(ChannelDataType::ByteArray);
  if (!network_name_.empty()) {
    CreateSourceInformation(*cn_frame);
  }

  CreateBusChannel(*cn_frame);
  CreateBitsChannel(*cn_frame,"LIN_ReceiveError.ID", 9, 0, 6);
  if (IChannel* nof_spec = CreateBitsChannel(*cn_frame, "LIN_ReceiveError.SpecifiedDataByteCount",10, 4, 4);
      nof_spec != nullptr) {
    nof_spec->Unit("B");
  }

  if (IChannel* nof_rec = CreateBitsChannel(*cn_frame, "LIN_ReceiveError.ReceivedDataByteCount", 10, 0, 4);
      nof_rec != nullptr) {
    nof_rec->Unit("B");
  }

  if (IChannel* frame_length = CreateBitsChannel(*cn_frame, "LIN_ReceiveError.DataLength",11,0,4);
      frame_length != nullptr) {
    frame_length->Unit("B");
  }

  if (IChannel* frame_bytes =
          cn_frame->CreateChannelComposition("LIN_ReceiveError.DataBytes");
      frame_bytes != nullptr) {
    frame_bytes->Type(ChannelType::MaxLength);
    frame_bytes->BitCount(64);
    frame_bytes->Sync(ChannelSyncType::None);
    frame_bytes->DataType(ChannelDataType::ByteArray);
    frame_bytes->Flags(CnFlag::BusEvent);
    frame_bytes->ByteOffset(12);
    frame_bytes->BitOffset(0);
  }

  if (IChannel* crc_model = CreateBitsChannel(*cn_frame, "LIN_ReceiveError.ChecksumModel",9, 6, 2);
      crc_model != nullptr) {
    crc_model->DataType(ChannelDataType::SignedIntegerLe);
  }

  if (IChannel* sof = CreateBitsChannel(*cn_frame, "LIN_ReceiveError.SOF", 20, 0, 64);
      sof != nullptr) {
    sof->Unit("ns");
  }

  if (IChannel* baudrate = CreateBitsChannel(*cn_frame, "LIN_ReceiveError.Baudrate", 28, 0, 32);
      baudrate != nullptr) {
    baudrate->DataType(ChannelDataType::FloatLe);
    baudrate->Unit("bits/s");
  }

  if (IChannel* resp = CreateBitsChannel(*cn_frame, "LIN_ReceiveError.ResponseBaudrate", 32, 0, 32);
      resp != nullptr) {
    resp->DataType(ChannelDataType::FloatLe);
    resp->Flags(CnFlag::BusEvent);
    resp->Unit("bits/s");
  }

  if (IChannel* break_length = CreateBitsChannel(*cn_frame, "LIN_ReceiveError.BreakLength", 36, 0, 32);
      break_length != nullptr) {
    break_length->Unit("ns");
  }

  if (IChannel* delimiter = CreateBitsChannel(*cn_frame, "LIN_ReceiveError.BreakDelimiterLength", 40, 0, 32);
      delimiter != nullptr) {
    delimiter->Unit("ns");
  }
}

void LinConfigAdapter::CreateSyncChannels(IChannelGroup& group) const {
  IChannel* cn_frame = group.CreateChannel("LIN_SyncError");
  if (cn_frame == nullptr) {
    MDF_ERROR() << "Failed to create the LIN_SyncError channel.";
    return;
  }

  cn_frame->Type(ChannelType::FixedLength);
  cn_frame->Sync(ChannelSyncType::None);
  cn_frame->DataBytes(29-8);
  cn_frame->Flags(CnFlag::BusEvent);
  cn_frame->DataType(ChannelDataType::ByteArray);
  if (!network_name_.empty()) {
    CreateSourceInformation(*cn_frame);
  }
  CreateBusChannel(*cn_frame);
  if (IChannel* sof = CreateBitsChannel(*cn_frame, "LIN_SyncError.SOF", 9, 0, 64);
      sof != nullptr) {
      sof->Unit("ns");
  }
    if (IChannel* baudrate = CreateBitsChannel(*cn_frame, "LIN_SyncError.Baudrate", 17, 0, 32);
      baudrate != nullptr) {
    baudrate->DataType(ChannelDataType::FloatLe);
    baudrate->Unit("bits/s");
  }
  if (IChannel* break_length = CreateBitsChannel(*cn_frame, "LIN_SyncError.BreakLength", 21, 0, 32);
      break_length != nullptr) {
    break_length->Unit("ns");
  }

  if (IChannel* del_break_length = CreateBitsChannel(*cn_frame, "LIN_SyncError.BreakDelimiterLength", 25, 0, 32);
      del_break_length != nullptr) {
    del_break_length->Unit("ns");
  }
}

void LinConfigAdapter::CreateWakeUpChannels(IChannelGroup& group) const {
  IChannel* cn_frame = group.CreateChannel("LIN_WakeUp");
  if (cn_frame == nullptr) {
    MDF_ERROR() << "Failed to create the LIN_WakeUp channel.";
    return;
  }

  cn_frame->Type(ChannelType::FixedLength);
  cn_frame->Sync(ChannelSyncType::None);
  cn_frame->DataBytes(21 - 8);
  cn_frame->Flags(CnFlag::BusEvent);
  cn_frame->DataType(ChannelDataType::ByteArray);
  if (!network_name_.empty()) {
    CreateSourceInformation(*cn_frame);
  }

  CreateBusChannel(*cn_frame);
  if (IChannel* baudrate = CreateBitsChannel(*cn_frame, "LIN_WakeUp.Baudrate", 9, 0, 32);
      baudrate != nullptr) {
    baudrate->DataType(ChannelDataType::FloatLe);
    baudrate->Unit("bits/s");
  }

  if (IChannel* sof = CreateBitsChannel(*cn_frame, "LIN_WakeUp.SOF", 13, 0, 8);
      sof != nullptr) {
    sof->Unit("ns");
  }
}

void LinConfigAdapter::CreateTransmissionErrorChannels(IChannelGroup& group) const {
  const bool mandatory = writer_.MandatoryMembersOnly();
  IChannel* cn_frame = group.CreateChannel("LIN_TransmissionError");
  if (cn_frame == nullptr) {
    MDF_ERROR() << "Failed to create the channel. Name: " << "LIN_TransmissionError";
    return;
  }

  cn_frame->Type(ChannelType::FixedLength);
  cn_frame->Sync(ChannelSyncType::None);
  cn_frame->DataBytes(mandatory ? 10 - 8 : 31 - 8);
  cn_frame->Flags(CnFlag::BusEvent);
  cn_frame->DataType(ChannelDataType::ByteArray);
  if (!network_name_.empty()) {
    CreateSourceInformation(*cn_frame);
  }
  CreateBusChannel(*cn_frame);
  CreateBitsChannel(*cn_frame,"LIN_Transmission.ID", 9, 0, 6);

  if (mandatory) {
    return;
  }

  if (IChannel* specified = CreateBitsChannel(*cn_frame,"LIN_TransmissionError.SpecifiedDataByteCount", 10, 0, 4);
    specified != nullptr) {
    specified->Unit("B");
  }

  if (IChannel* crc_model = CreateBitsChannel(*cn_frame, "LIN_TransmissionError.ChecksumModel", 10, 4, 2);
      crc_model != nullptr) {
    crc_model->DataType(ChannelDataType::SignedIntegerLe);
  }
  if (IChannel* sof = CreateBitsChannel(*cn_frame, "LIN_TransmissionError.SOF", 11, 0, 64);
      sof != nullptr) {
    sof->Unit("ns");
  }

  if (IChannel* baudrate = CreateBitsChannel(*cn_frame, "LIN_TransmissionError.Baudrate", 19, 0, 32);
      baudrate != nullptr) {
    baudrate->DataType(ChannelDataType::FloatLe);
    baudrate->Unit("bits/s");
  }

  if (IChannel* break_length = CreateBitsChannel(*cn_frame, "LIN_TransmissionError.BreakLength", 23, 0, 32);
      break_length != nullptr) {
    break_length->Unit("ns");
  }

  if (IChannel* delimiter = CreateBitsChannel(*cn_frame, "LIN_TransmissionError.BreakDelimiterLength", 27, 0, 32);
      delimiter != nullptr) {
    delimiter->Unit("ns");
  }
}

void LinConfigAdapter::CreateSpikeChannels(IChannelGroup& group) const {
  IChannel* cn_frame = group.CreateChannel("LIN_Spike");
  if (cn_frame == nullptr) {
    MDF_ERROR() << "Failed to create the LIN_Spike channel.";
    return;
  }

  cn_frame->Type(ChannelType::FixedLength);
  cn_frame->Sync(ChannelSyncType::None);
  cn_frame->DataBytes(21 - 8);
  cn_frame->Flags(CnFlag::BusEvent);
  cn_frame->DataType(ChannelDataType::ByteArray);
  if (!network_name_.empty()) {
    CreateSourceInformation(*cn_frame);
  }

  CreateBusChannel(*cn_frame);
  if (IChannel* baudrate = CreateBitsChannel(*cn_frame, "LIN_Spike.Baudrate", 9, 0, 32);
      baudrate != nullptr) {
    baudrate->DataType(ChannelDataType::FloatLe);
    baudrate->Unit("bits/s");
      }

  if (IChannel* sof = CreateBitsChannel(*cn_frame, "LIN_Spike.SOF", 13, 0, 8);
      sof != nullptr) {
    sof->Unit("ns");
      }
}

void LinConfigAdapter::CreateLongDominantChannels(mdf::IChannelGroup& group) const {
  auto* cn_frame = group.CreateChannel("LIN_LongDom");
  if (cn_frame == nullptr) {
    MDF_ERROR() << "Failed to create the channel. Name: " << "LIN_LongDom";
    return;
  }

  cn_frame->Type(ChannelType::FixedLength);
  cn_frame->Sync(ChannelSyncType::None);
  cn_frame->DataBytes(26 - 8);
  cn_frame->Flags(CnFlag::BusEvent);
  cn_frame->DataType(ChannelDataType::ByteArray);
  if (!network_name_.empty()) {
    CreateSourceInformation(*cn_frame);
  }
  CreateBusChannel(*cn_frame);
  if (IChannel* baudrate = CreateBitsChannel(*cn_frame, "LIN_LongDom.Baudrate", 9, 0, 32);
      baudrate != nullptr) {
    baudrate->DataType(ChannelDataType::FloatLe);
    baudrate->Unit("bits/s");
  }

  if (IChannel* sof = CreateBitsChannel(*cn_frame, "LIN_LongDom.SOF", 13, 0, 64);
      sof != nullptr) {
    sof->Unit("ns");
  }

  if (IChannel* length = CreateBitsChannel(*cn_frame, "LIN_LongDom.Length", 21, 0, 32);
      length != nullptr) {
    length->Unit("ns");
  }

  CreateBitsChannel(*cn_frame, "LIN_LongDom.Type", 25, 0, 2);

}


}  // namespace mdf