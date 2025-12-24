/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "MdfChannelGroup.h"
#include "mdflibrary.h"

namespace MdfLibrary {
int64_t MdfChannelGroup::Index::get() {
  return group_ != nullptr ? group_->Index() : 0; 
}

uint64_t MdfChannelGroup::RecordId::get() {
  return group_ != nullptr ? group_->RecordId() : 0; 
}

void MdfChannelGroup::RecordId::set(uint64_t record_id) {
  if (group_ != nullptr) {
    group_->RecordId(record_id);
  }    
}

String^ MdfChannelGroup::Name::get() {
  return group_ != nullptr ? MdfLibrary::Utf8Conversion(group_->Name()) : gcnew String("");
}

void MdfChannelGroup::Name::set(String^ name) {
  if (group_ != nullptr) {
    group_->Name(MdfLibrary::Utf8Conversion(name));
  }    
}

String^ MdfChannelGroup::Description::get() {
  return group_ != nullptr ? MdfLibrary::Utf8Conversion(group_->Description()) : gcnew String("");
}

void MdfChannelGroup::Description::set(String^ desc) {
  if (group_ != nullptr) {
    group_->Description(MdfLibrary::Utf8Conversion(desc));
  }    
}

uint64_t MdfChannelGroup::NofSamples::get() {
  return group_ != nullptr ? group_->NofSamples() : 0; 
}

void MdfChannelGroup::NofSamples::set(uint64_t samples) {
  if (group_ != nullptr) {
    group_->NofSamples(samples);
  }    
}

uint16_t MdfChannelGroup::Flags::get() {
  return group_ != nullptr ? group_->Flags() : 0; 
}

void MdfChannelGroup::Flags::set(uint16_t flags) {
  if (group_ != nullptr) {
    group_->Flags(flags);
  }    
}

wchar_t MdfChannelGroup::PathSeparator::get() {
  return group_ != nullptr ? group_->PathSeparator() : L'/';
}

void MdfChannelGroup::PathSeparator::set(wchar_t sep) {
  if (group_ != nullptr) {
    group_->PathSeparator(sep);
  }
}

array<MdfChannel^>^ MdfChannelGroup::Channels::get() {
  array<MdfChannel^>^ temp;
  if (group_ != nullptr) {
    const auto list = group_->Channels();
    temp = gcnew array<MdfChannel^>(static_cast<int>(list.size()));
    for (size_t index = 0; index < list.size(); ++index) {
      temp[static_cast<int>(index)] = gcnew MdfChannel(list[index]);
    }  
  } else {
    temp = gcnew array<MdfChannel^>(0);
  }
  return temp;
}

MdfMetaData^ MdfChannelGroup::MetaData::get() {
  auto* temp = group_ != nullptr ?
    group_->MetaData() : nullptr;
  return temp != nullptr ? gcnew MdfMetaData(temp) : nullptr;
}

MdfSourceInformation^ MdfChannelGroup::SourceInformation::get() {
  auto* temp = group_ != nullptr ?
    group_->SourceInformation() : nullptr;
  return temp != nullptr ? gcnew MdfSourceInformation(temp) : nullptr;
}

MdfChannel^ MdfChannelGroup::GetXChannel(const MdfChannel^ ref_channel) {
  const auto* ref = ref_channel != nullptr ?
      ref_channel->channel_ : nullptr;
  mdf::IChannel *temp = nullptr;
  if (group_ != nullptr && ref != nullptr) {
    temp = const_cast<mdf::IChannel*>(group_->GetXChannel(*ref));
  }
  return temp != nullptr ? gcnew MdfChannel(temp) : nullptr;
}

MdfChannel^ MdfChannelGroup::CreateChannel() {
  auto* temp = group_ != nullptr ? group_->CreateChannel() : nullptr;
  return gcnew MdfChannel(temp);
}

MdfMetaData^ MdfChannelGroup::CreateMetaData() {
  auto* temp = group_ != nullptr ? group_->CreateMetaData() : nullptr;
  return gcnew MdfMetaData(temp);
}

MdfSourceInformation^ MdfChannelGroup::CreateSourceInformation() {
  auto* temp = group_ != nullptr ? group_->CreateSourceInformation() : nullptr;
  return gcnew MdfSourceInformation(temp);
}

MdfChannelGroup::MdfChannelGroup(mdf::IChannelGroup* group)
  : group_(group) {
  
}

}
