/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <mdf/ichannelgroup.h>
#include "MdfChannel.h"
#include "MdfSourceInformation.h"

using namespace System;
namespace MdfLibrary {

public ref class MdfChannelGroup {
public:
  literal uint16_t VlsdChannel = 0x0001;
  literal uint16_t BusEvent = 0x0002;
  literal uint16_t PlainBusEvent = 0x0004;
  literal uint16_t RemoteMaster = 0x0008;
  literal uint16_t EventSignal = 0x00010;
  
  property int64_t Index { int64_t get(); }
  property uint64_t RecordId { uint64_t get(); void set(uint64_t record_id); }
  property String^ Name { String^ get(); void set(String^ name); }
  property String^ Description { String^ get(); void set(String^ desc); }
  property uint64_t NofSamples { uint64_t get(); void set(uint64_t samples); }
  property uint16_t Flags { uint16_t get(); void set(uint16_t flags); }
  property wchar_t PathSeparator { wchar_t get(); void set(wchar_t sep); }
  property array<MdfChannel^>^ Channels { array<MdfChannel^>^ get(); }
  property MdfMetaData^ MetaData {
    MdfMetaData^ get();
  }
  property MdfSourceInformation^ SourceInformation {
    MdfSourceInformation^ get();
  }
  MdfChannel^ GetXChannel(const MdfChannel^ ref_channel );
   
  MdfMetaData^ CreateMetaData();
  MdfChannel^ CreateChannel();
  MdfSourceInformation^ CreateSourceInformation();
  
 private:
  MdfChannelGroup() {}
internal:
  mdf::IChannelGroup *group_ = nullptr;
  MdfChannelGroup(mdf::IChannelGroup *group);
};

}