/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "MdfFile.h"
#include "mdflibrary.h"

namespace MdfLibrary {

array<MdfAttachment^>^ MdfFile::Attachments::get() {
  mdf::AttachmentList list;
  if (mdf_file_ != nullptr) {
    mdf_file_->Attachments(list);
  }
  array<MdfAttachment^>^ temp_list =
    gcnew array<MdfAttachment^>(static_cast<int>(list.size()));
  for (size_t index = 0; index < list.size(); ++index) {
    auto* temp = const_cast<mdf::IAttachment*>(list[index]);
    temp_list[static_cast<int>(index)] = gcnew MdfAttachment(temp);
  }
  return temp_list;
}

array<MdfDataGroup^>^ MdfFile::DataGroups::get() {
   mdf::DataGroupList list;
   if (mdf_file_ != nullptr) {
     mdf_file_->DataGroups(list);
   }
   array<MdfDataGroup^>^ temp_list =
     gcnew array<MdfDataGroup^>(static_cast<int>(list.size()));
   for (size_t index = 0; index < list.size(); ++index) {
     auto* temp = const_cast<mdf::IDataGroup*>(list[index]);
     temp_list[static_cast<int>(index)] = gcnew MdfDataGroup(temp);
   }
   return temp_list;
}

String^ MdfFile::Name::get() {
  return mdf_file_ != nullptr ? MdfLibrary::Utf8Conversion(
    mdf_file_->Name()) : gcnew String("");
}

void MdfFile::Name::set(String^ name) {
  if (mdf_file_ != nullptr) {
     mdf_file_->Name(MdfLibrary::Utf8Conversion(name));
  }
}

String^ MdfFile::FileName::get() {
  return mdf_file_ != nullptr ?
    MdfLibrary::Utf8Conversion(mdf_file_->FileName()) : gcnew String("");
}

void MdfFile::FileName::set(String^ filename) {
  if (mdf_file_ != nullptr) {
     mdf_file_->FileName(MdfLibrary::Utf8Conversion(filename));
  }  
}

String^ MdfFile::Version::get() {
  return mdf_file_ != nullptr ? MdfLibrary::Utf8Conversion(
      mdf_file_->Version()) : gcnew String("");
}

int MdfFile::MainVersion::get() {
  return  mdf_file_ != nullptr ? mdf_file_->MainVersion() : 0;
}

int MdfFile::MinorVersion::get() {
  return  mdf_file_ != nullptr ? mdf_file_->MinorVersion() : 0;  
}
void MdfFile::MinorVersion::set(int minor) {
  if (mdf_file_ != nullptr) {
    mdf_file_->MinorVersion(minor);
  }
}

String^ MdfFile::ProgramId::get() {
  return mdf_file_ != nullptr ? MdfLibrary::Utf8Conversion( 
      mdf_file_->ProgramId()) : gcnew String("");
}

void MdfFile::ProgramId::set(String^ program_id) {
  if (mdf_file_ != nullptr) {
    mdf_file_->ProgramId(MdfLibrary::Utf8Conversion(program_id));
  }  
}

bool MdfFile::Finalized::get() {
  uint16_t standard_flags = 0;
  uint16_t custom_flags = 0;
  return mdf_file_ != nullptr ?
    mdf_file_->IsFinalized(standard_flags, custom_flags) : false;
}

MdfHeader^ MdfFile::Header::get() {
  auto* header = mdf_file_ != nullptr ? mdf_file_->Header() : nullptr;
  return gcnew MdfHeader(header);
}

MdfAttachment^ MdfFile::CreateAttachment() {
  auto* attachment = mdf_file_ != nullptr ?
    mdf_file_->CreateAttachment() : nullptr;
  return gcnew MdfAttachment(attachment);
}

MdfDataGroup^ MdfFile::CreateDataGroup() {
  auto* data_group = mdf_file_ != nullptr ?
    mdf_file_->CreateDataGroup() : nullptr;
  return gcnew MdfDataGroup(data_group);  
}

MdfDataGroup^ MdfFile::FindParentDataGroup(const MdfChannel^ channel) {
  
  const auto* data_group = mdf_file_ != nullptr && channel != nullptr &&
    channel->channel_ != nullptr ?
    mdf_file_->FindParentDataGroup(*channel->channel_) : nullptr;
  return data_group != nullptr ?
    gcnew MdfDataGroup(const_cast<mdf::IDataGroup*>(data_group)) : nullptr; 
}

bool MdfFile::IsMdf4::get() {
  return mdf_file_ != nullptr ? mdf_file_->IsMdf4() : false;
};


MdfFile::MdfFile(mdf::MdfFile* mdf_file)
  : mdf_file_( mdf_file) {;
}
  
} // end namespace