/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include "mdf/ethconfigadapter.h"

#include <sstream>

#include "mdf/ichannelgroup.h"
#include "mdf/idatagroup.h"
#include "mdf/mdflogstream.h"

namespace {

void CreateSource(mdf::IChannel& cn_frame) {
  std::ostringstream name;
  name << cn_frame.Name() << ".Source";
  if (mdf::IChannel* source = cn_frame.CreateChannelComposition(name.str());
    source != nullptr) {
    source->Type(mdf::ChannelType::FixedLength);
    source->BitCount(48);
    source->Sync(mdf::ChannelSyncType::None);
    source->DataType(mdf::ChannelDataType::ByteArray);
    source->Flags(mdf::CnFlag::BusEvent);
    source->ByteOffset(9);
    source->BitOffset(0);
    }
}

void CreateDestination(mdf::IChannel& cn_frame) {
  std::ostringstream name;
  name << cn_frame.Name() << ".Destination";
  if (mdf::IChannel* source = cn_frame.CreateChannelComposition(name.str());
    source != nullptr) {
    source->Type(mdf::ChannelType::FixedLength);
    source->BitCount(48);
    source->Sync(mdf::ChannelSyncType::None);
    source->DataType(mdf::ChannelDataType::ByteArray);
    source->Flags(mdf::CnFlag::BusEvent);
    source->ByteOffset(15);
    source->BitOffset(0);
    }
}
void CreateDataBytes(mdf::IChannel& cn_frame,uint16_t byte_offset) {
  std::ostringstream name;
  name << cn_frame.Name() << ".DataBytes";
  if (mdf::IChannel* frame_bytes = cn_frame.CreateChannelComposition(name.str());
    frame_bytes != nullptr) {
    frame_bytes->Type(mdf::ChannelType::VariableLength);
    frame_bytes->BitCount(64);
    frame_bytes->Sync(mdf::ChannelSyncType::None);
    frame_bytes->DataType(mdf::ChannelDataType::ByteArray);
    frame_bytes->Flags(mdf::CnFlag::BusEvent);
    frame_bytes->ByteOffset(byte_offset);
    frame_bytes->BitOffset(0);
  }

}

} //end namespace

namespace mdf {
EthConfigAdapter::EthConfigAdapter(const MdfWriter& writer)
    : IConfigAdapter(writer) {
  BusType(MdfBusType::Ethernet);
  BusName("ETH");
}

void EthConfigAdapter::CreateConfig(IDataGroup& dg_block) {
  dg_block.MandatoryMembersOnly(MandatoryMembersOnly());
  const IChannel* cn_data_byte = nullptr; // Need to update the VLSD Record ID
  if (IChannelGroup* cg_frame = dg_block.CreateChannelGroup("ETH_Frame");
      cg_frame != nullptr) {
    cg_frame->PathSeparator('.');
    cg_frame->Flags(CgFlag::PlainBusEvent | CgFlag::BusEvent);
    CreateSourceInformation(*cg_frame);
    CreateTimeChannel(*cg_frame, "t");
    CreateFrameChannels(*cg_frame);
    cn_data_byte = cg_frame->GetChannel("ETH_Frame.DataBytes");
  }

  // Add a CG-VLSD block that stores the signal data.
  // The FixedLengthStorage is used for SD storage
  if ( StorageType() != MdfStorageType::FixedLengthStorage &&
      cn_data_byte != nullptr) {
    // Need to add a special CG group for the data samples
    if (IChannelGroup* cg_samples = dg_block.CreateChannelGroup("");
     cg_samples != nullptr) {
      cg_samples->Flags(CgFlag::VlsdChannel);
      cn_data_byte->VlsdRecordId(cg_samples->RecordId());
    }
  }

  cn_data_byte = nullptr;
  if (IChannelGroup* cg_checksum_error = dg_block.CreateChannelGroup("ETH_ChecksumError");
      cg_checksum_error != nullptr) {
    cg_checksum_error->PathSeparator('.');
    cg_checksum_error->Flags(CgFlag::PlainBusEvent | CgFlag::BusEvent);
    CreateSourceInformation(*cg_checksum_error);
    CreateTimeChannel(*cg_checksum_error, "t");
    CreateChecksumErrorChannels(*cg_checksum_error);
    // Note that the cn_data_byte not is mandatory
    cn_data_byte = cg_checksum_error->GetChannel("ETH_ChecksumError.DataBytes");
  }

  // Add a CG-VLSD block that stores the signal data.
  // The FixedLengthStorage is used for SD storage
  if ( StorageType() != MdfStorageType::FixedLengthStorage &&
      cn_data_byte != nullptr) {
    // Need to add a special CG group for the data samples
    if (IChannelGroup* cg_errors = dg_block.CreateChannelGroup("");
        cg_errors != nullptr) {
      cg_errors->Flags(CgFlag::VlsdChannel);
      cn_data_byte->VlsdRecordId(cg_errors->RecordId());
    }
  }

  cn_data_byte = nullptr;
  if (IChannelGroup* cg_length_error = dg_block.CreateChannelGroup("ETH_LengthError");
     cg_length_error != nullptr) {
   cg_length_error->PathSeparator('.');
   cg_length_error->Flags(CgFlag::PlainBusEvent | CgFlag::BusEvent);
   CreateSourceInformation(*cg_length_error);
   CreateTimeChannel(*cg_length_error, "t");
   CreateLengthErrorChannels(*cg_length_error);
   // Note that the cn_data_byte not is mandatory
   cn_data_byte = cg_length_error->GetChannel("ETH_LengthError.DataBytes");
  }

  // Add a CG-VLSD block that stores the signal data.
  // The FixedLengthStorage is used for SD storage
  if ( StorageType() != MdfStorageType::FixedLengthStorage &&
    cn_data_byte != nullptr) {
    // Need to add a special CG group for the data samples
    if (IChannelGroup* cg_errors = dg_block.CreateChannelGroup("");
       cg_errors != nullptr) {
     cg_errors->Flags(CgFlag::VlsdChannel);
     cn_data_byte->VlsdRecordId(cg_errors->RecordId());
    }
  }

  cn_data_byte = nullptr;
  if (IChannelGroup* cg_receive_error = dg_block.CreateChannelGroup("ETH_ReceiveError");
      cg_receive_error != nullptr) {
    cg_receive_error->PathSeparator('.');
    cg_receive_error->Flags(CgFlag::PlainBusEvent | CgFlag::BusEvent);
    CreateSourceInformation(*cg_receive_error);
    CreateTimeChannel(*cg_receive_error, "t");
    CreateReceiveErrorChannels(*cg_receive_error);
    // Note that the cn_data_byte not is mandatory
    cn_data_byte = cg_receive_error->GetChannel("ETH_ReceiveError.DataBytes");
  }

  // Add a CG-VLSD block that stores the signal data.
  // The FixedLengthStorage is used for SD storage
  if ( StorageType() != MdfStorageType::FixedLengthStorage &&
      cn_data_byte != nullptr) {
    // Need to add a special CG group for the data samples
    if (IChannelGroup* cg_errors = dg_block.CreateChannelGroup("");
        cg_errors != nullptr) {
      cg_errors->Flags(CgFlag::VlsdChannel);
      cn_data_byte->VlsdRecordId(cg_errors->RecordId());
    }
  }

}

void EthConfigAdapter::CreateFrameChannels(IChannelGroup& group) const {
  const bool mandatory = writer_.MandatoryMembersOnly();
  auto* cn_frame = group.CreateChannel("ETH_Frame");
  if (cn_frame == nullptr) {
    MDF_ERROR() << "Failed to create the channel. Name :" << "ETH_Frame";
    return;
  }

  cn_frame->Type(ChannelType::FixedLength);
  cn_frame->Sync(ChannelSyncType::None);
  cn_frame->DataBytes(mandatory ? 35 - 8 : 42 - 8);
  cn_frame->Flags(CnFlag::BusEvent);
  cn_frame->DataType(ChannelDataType::ByteArray);

  CreateBusChannel(*cn_frame);
  CreateSource(*cn_frame);
  CreateDestination(*cn_frame);
  CreateBitsChannel(*cn_frame, "ETH_Frame.EtherType", 21, 0 , 16);
  if (IChannel* nof_rec = CreateBitsChannel(*cn_frame, "ETH_Frame.ReceivedDataByteCount", 23, 0 , 16);
      nof_rec != nullptr) {
      nof_rec->Unit("B");
  }
  if (IChannel* frame_length = CreateBitsChannel(*cn_frame, "ETH_Frame.DataLength",25, 0 , 16 );
    frame_length != nullptr) {
    frame_length->Unit("B");
  }

  if (mandatory) {
    // In reality the VSLD-CG storage is the only possible solution. The
    // SD storage is possible in theory only.
    CreateDataBytes(*cn_frame, 27);
    return;
  }
  CreateBitsChannel(*cn_frame, "ETH_Frame.CRC", 27, 0 , 32);
  CreateBitChannel(*cn_frame, "ETH_Frame.Dir",31 , 0);

  if (IChannel* padding_bytes = CreateBitsChannel(*cn_frame, "ETH_Frame.PadByteCount", 32, 0 , 16);
    padding_bytes != nullptr) {
    padding_bytes->Unit("B");
  }
  CreateDataBytes(*cn_frame, 34);
}

void EthConfigAdapter::CreateChecksumErrorChannels(IChannelGroup& group) const {
  const bool mandatory = writer_.MandatoryMembersOnly();

  IChannel* cn_frame = group.CreateChannel("ETH_ChecksumError");
  if (cn_frame == nullptr) {
    MDF_ERROR() << "Failed to create the channel. Name :" << "ETH_ChecksumError";
    return;
  }

  cn_frame->Type(ChannelType::FixedLength);
  cn_frame->Sync(ChannelSyncType::None);
  cn_frame->DataBytes(mandatory ? 33 - 8 : 46 - 8);
  cn_frame->Flags(CnFlag::BusEvent);
  cn_frame->DataType(ChannelDataType::ByteArray);

  CreateBusChannel(*cn_frame);
  CreateSource(*cn_frame);
  CreateDestination(*cn_frame);
  CreateBitsChannel(*cn_frame, "ETH_ChecksumError.EtherType", 21, 0 , 16);
  if (IChannel* frame_length = CreateBitsChannel(*cn_frame, "ETH_ChecksumError.DataLength", 23, 0 , 16 );
      frame_length != nullptr) {
    frame_length->Unit("B");
  }
  CreateBitsChannel(*cn_frame, "ETH_ChecksumError.ReceivedCRC", 25, 0 , 32);
  CreateBitsChannel(*cn_frame, "ETH_ChecksumError.ExpectedCRC", 29, 0 , 32);

  if (mandatory) {
    return;
  }
  if (IChannel* nof_rec = CreateBitsChannel(*cn_frame, "ETH_ChecksumError.ReceivedDataByteCount", 33, 0 , 16);
      nof_rec != nullptr) {
    nof_rec->Unit("B");
  }
  CreateBitChannel(*cn_frame, "ETH_ChecksumError.Dir", 35, 0);

  if (IChannel* padding_bytes = CreateBitsChannel(*cn_frame, "ETH_ChecksumError.PadByteCount", 36, 0 , 16);
        padding_bytes != nullptr) {
    padding_bytes->Unit("B");
  }
  // In reality the VSLD-CG storage is the only possible solution. The
  // SD storage is possible in theory only.
  CreateDataBytes(*cn_frame, 38);
}

void EthConfigAdapter::CreateLengthErrorChannels(IChannelGroup& group) const {
  const bool mandatory = writer_.MandatoryMembersOnly();
  auto* cn_frame = group.CreateChannel("ETH_LengthError");
  if (cn_frame == nullptr) {
    MDF_ERROR() << "Failed to create the channel. Name :" << "ETH_LengthError";
    return;
  }

  cn_frame->Type(ChannelType::FixedLength);
  cn_frame->Sync(ChannelSyncType::None);
  cn_frame->DataBytes(mandatory ? 25 - 8 : 42 - 8);
  cn_frame->Flags(CnFlag::BusEvent);
  cn_frame->DataType(ChannelDataType::ByteArray);

  CreateBusChannel(*cn_frame);
  CreateSource(*cn_frame);
  CreateDestination(*cn_frame);
  CreateBitsChannel(*cn_frame, "ETH_LengthError.EtherType", 21, 0 , 16);
  if (IChannel* nof_rec = CreateBitsChannel(*cn_frame, "ETH_LengthError.ReceivedDataByteCount", 23, 0 , 16);
      nof_rec != nullptr) {
    nof_rec->Unit("B");
  }

  if (mandatory) {
    return;
  }

  if (IChannel* frame_length = CreateBitsChannel(*cn_frame, "ETH_LengthError.DataLength", 25, 0 , 16 );
      frame_length != nullptr) {
    frame_length->Unit("B");
  }
  CreateBitsChannel(*cn_frame, "ETH_LengthError.CRC", 27, 0 , 32);

  CreateBitChannel(*cn_frame, "ETH_LengthError.Dir", 31, 0);

  if (IChannel* padding_bytes = CreateBitsChannel(*cn_frame, "ETH_LengthError.PadByteCount", 32, 0 , 16);
        padding_bytes != nullptr) {
    padding_bytes->Unit("B");
  }
  CreateDataBytes(*cn_frame, 34);
}

void EthConfigAdapter::CreateReceiveErrorChannels(IChannelGroup& group) const {
  const bool mandatory = writer_.MandatoryMembersOnly();
  auto* cn_frame = group.CreateChannel("ETH_ReceiveError");
  if (cn_frame == nullptr) {
    MDF_ERROR() << "Failed to create the channel. Name :" << "ETH_ReceiveError";
    return;
  }

  cn_frame->Type(ChannelType::FixedLength);
  cn_frame->Sync(ChannelSyncType::None);
  cn_frame->DataBytes(mandatory ? 25 - 8 : 42 - 8);
  cn_frame->Flags(CnFlag::BusEvent);
  cn_frame->DataType(ChannelDataType::ByteArray);

  CreateBusChannel(*cn_frame);
  CreateSource(*cn_frame);
  CreateDestination(*cn_frame);
  CreateBitsChannel(*cn_frame, "ETH_ReceiveError.EtherType", 21, 0 , 16);
  if (IChannel* nof_rec = CreateBitsChannel(*cn_frame, "ETH_ReceiveError.ReceivedDataByteCount", 23, 0 , 16);
      nof_rec != nullptr) {
    nof_rec->Unit("B");
  }

  if (mandatory) {
    return;
  }

  if (IChannel* frame_length = CreateBitsChannel(*cn_frame, "ETH_ReceiveError.DataLength", 25, 0 , 16 );
      frame_length != nullptr) {
    frame_length->Unit("B");
  }
  CreateBitsChannel(*cn_frame, "ETH_ReceiveError.CRC", 27, 0 , 32);

  CreateBitChannel(*cn_frame, "ETH_ReceiveError.Dir", 31, 0);

  CreateBitChannel(*cn_frame, "ETH_ReceiveError.ErrorType", 31, 1);

  if (IChannel* padding_bytes = CreateBitsChannel(*cn_frame, "ETH_ReceiveError.PadByteCount", 32, 0 , 16);
        padding_bytes != nullptr) {
    padding_bytes->Unit("B");
  }
  CreateDataBytes(*cn_frame, 34);
}

}  // namespace mdf