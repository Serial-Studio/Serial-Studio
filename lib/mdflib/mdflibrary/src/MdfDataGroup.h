/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <mdf/idatagroup.h>
#include "MdfChannelGroup.h"
#include "MdfMetaData.h"

using namespace System;
namespace MdfLibrary {
public ref class MdfDataGroup {
public:
  property int64_t Index {int64_t get(); }
  property String^ Description { String^ get(); void set(String^ desc); }
  property uint8_t RecordIdSize {uint8_t get(); void set(uint8_t id_size); }

  property MdfMetaData^ MetaData {MdfMetaData^ get(); }
  property array<MdfChannelGroup^>^ ChannelGroups { array<MdfChannelGroup^>^ get(); }

  property bool IsRead { bool get(); }

  MdfMetaData^ CreateMetaData();
  MdfChannelGroup^ CreateChannelGroup();
  MdfChannelGroup^ GetChannelGroup(String^ groupName);

  MdfChannelGroup^ FindParentChannelGroup(const MdfChannel^ channel);
  
private:
  MdfDataGroup() {};
internal:
  mdf::IDataGroup *group_ = nullptr; 
  MdfDataGroup(mdf::IDataGroup* group); 
};
}