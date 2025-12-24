/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "MdfAttachment.h"
#include "mdflibrary.h"

namespace MdfLibrary {

MdfAttachment::MdfAttachment(mdf::IAttachment* attachment) {
  attachment_ = attachment;
}

Int64 MdfAttachment::Index::get() {
  return attachment_ != nullptr ? attachment_->Index() : 0;
}

UInt16 MdfAttachment::CreatorIndex::get() {
  return attachment_ != nullptr ? attachment_->CreatorIndex() : 0;
}

void MdfAttachment::CreatorIndex::set(UInt16 index) {
  if (attachment_ != nullptr) {
    attachment_->CreatorIndex(index);
  }
}

bool MdfAttachment::Embedded::get() {
  return attachment_ != nullptr ? attachment_->IsEmbedded() : false;  
}

void MdfAttachment::Embedded::set(bool embedded) {
  if (attachment_ != nullptr) {
    attachment_->IsEmbedded(embedded);
  }  
}

bool MdfAttachment::Compressed::get() {
   return attachment_ != nullptr ? attachment_->IsCompressed() : false;   
}

void MdfAttachment::Compressed::set(bool compressed) {
  if (attachment_ != nullptr) {
    attachment_->IsCompressed(compressed);
  }  
}

String^ MdfAttachment::Md5::get() {
  if (attachment_ == nullptr) {
    return gcnew String("");
  }

  return attachment_->Md5().has_value()
             ? MdfLibrary::Utf8Conversion(attachment_->Md5().value())
             : gcnew String("");
}

String^ MdfAttachment::FileName::get() {
  return attachment_ != nullptr ? MdfLibrary::Utf8Conversion(
    attachment_->FileName()) : gcnew String("");
}

void MdfAttachment::FileName::set(String^ name) {
  if (attachment_ != nullptr) {
    attachment_->FileName(MdfLibrary::Utf8Conversion(name));
  }
}

String^ MdfAttachment::FileType::get() {
  return attachment_ != nullptr ? MdfLibrary::Utf8Conversion(
  attachment_->FileType()) : gcnew String("");
}

void MdfAttachment::FileType::set(String^ type) {
  if (attachment_ != nullptr) {
    attachment_->FileType(MdfLibrary::Utf8Conversion(type));
  }
}

MdfMetaData^ MdfAttachment::MetaData::get() {
 return attachment_ != nullptr ?
   gcnew MdfMetaData( attachment_->MetaData()) : nullptr;
}

MdfMetaData^ MdfAttachment::CreateMetaData() {
  return gcnew MdfMetaData(attachment_ != nullptr ?
    attachment_->CreateMetaData() : nullptr);
}

}
