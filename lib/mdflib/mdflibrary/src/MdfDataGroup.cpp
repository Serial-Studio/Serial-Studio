/*
* Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "MdfDataGroup.h"
#include "mdflibrary.h"

namespace MdfLibrary {

int64_t MdfDataGroup::Index::get() {
  return group_ != nullptr ? group_->Index() : 0; 
}

String^ MdfDataGroup::Description::get() {
  return group_ != nullptr ? MdfLibrary::Utf8Conversion(
  group_->Description()) : gcnew String("");
}

void MdfDataGroup::Description::set(String^ desc) {
  if (group_ != nullptr) {
    group_->Description(MdfLibrary::Utf8Conversion(desc));
  }    
}

uint8_t MdfDataGroup::RecordIdSize::get() {
  return group_ != nullptr ? group_->RecordIdSize() : 0;
}

void MdfDataGroup::RecordIdSize::set(uint8_t id_size) {
  if (group_ != nullptr) {
    group_->RecordIdSize(id_size);
  }
}

array<MdfChannelGroup^>^ MdfDataGroup::ChannelGroups::get() {
  if (group_ == nullptr) {
    return gcnew array<MdfChannelGroup^>(0);
  }
  const auto list = group_->ChannelGroups();
  auto temp = gcnew array<MdfChannelGroup^>(static_cast<int>(list.size()));
  for (size_t index = 0; index < list.size(); ++index) {
    temp[static_cast<int>(index)] = gcnew MdfChannelGroup(list[index]);
  }
  return temp;
}

MdfMetaData^ MdfDataGroup::MetaData::get() {
  auto* temp = group_ != nullptr ? group_->MetaData() : nullptr;
  return temp != nullptr ? gcnew MdfMetaData(temp) : nullptr; 
}

MdfChannelGroup^ MdfDataGroup::CreateChannelGroup() {
  auto* temp = group_ != nullptr ?
    group_->CreateChannelGroup() : nullptr;
  return gcnew MdfChannelGroup(temp);
}

MdfChannelGroup^ MdfDataGroup::GetChannelGroup(String^ groupName) {
  if (groupName == nullptr) {
    return nullptr;
  }
  const auto name = MdfLibrary::Utf8Conversion(groupName);
  auto* channelGroup = group_ != nullptr ?
    group_->GetChannelGroup(name) : nullptr;
  return channelGroup != nullptr ?
    gcnew MdfChannelGroup(channelGroup) : nullptr;
}

MdfMetaData^ MdfDataGroup::CreateMetaData() {
  auto* temp = group_ != nullptr ?
    group_->CreateMetaData() : nullptr;
  return gcnew MdfMetaData(temp);  
}

bool MdfDataGroup::IsRead::get() {
  return group_ != nullptr ? group_->IsRead() : false;
}

MdfChannelGroup^ MdfDataGroup::FindParentChannelGroup(const MdfChannel^ channel) {
  const auto* input =
    channel != nullptr ? channel->channel_ : nullptr;
  if (input == nullptr) {
    return nullptr;
  }
  
  const auto* temp = group_ != nullptr ?
    group_->FindParentChannelGroup(*input) : nullptr;
  return temp != nullptr ? gcnew MdfChannelGroup(const_cast<mdf::IChannelGroup*>(temp)) : nullptr;  
}

MdfDataGroup::MdfDataGroup(mdf::IDataGroup* group)
  : group_(group) {
}

}
