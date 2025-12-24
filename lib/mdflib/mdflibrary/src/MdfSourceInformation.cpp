/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "MdfSourceInformation.h"
#include "mdflibrary.h"

namespace MdfLibrary {

int64_t MdfSourceInformation::Index::get() {
  return info_ != nullptr ? info_->Index() : 0; 
}

String^ MdfSourceInformation::Name::get() {
  return info_ != nullptr ? MdfLibrary::Utf8Conversion(info_->Name()) : gcnew String("");
}

void MdfSourceInformation::Name::set(String^ name) {
  if (info_ != nullptr) {
    info_->Name(MdfLibrary::Utf8Conversion(name));
  }    
}
String^ MdfSourceInformation::Description::get() {
  return info_ != nullptr ? MdfLibrary::Utf8Conversion(info_->Description()) : gcnew String("");
}

void MdfSourceInformation::Description::set(String^ desc) {
if (info_ != nullptr) {
    info_->Description(MdfLibrary::Utf8Conversion(desc));
  }    
}

String^ MdfSourceInformation::Path::get() {
  return info_ != nullptr ? MdfLibrary::Utf8Conversion(info_->Path()) : gcnew String("");
}

void MdfSourceInformation::Path::set(String^ path) {
if (info_ != nullptr) {
    info_->Path(MdfLibrary::Utf8Conversion(path));
  }    
}

SourceType MdfSourceInformation::Type::get() {
  return info_ != nullptr ?
    static_cast<SourceType>(info_->Type()) : SourceType::Other;
}

void MdfSourceInformation::Type::set(SourceType type) {
  if (info_ != nullptr) {
    info_->Type(static_cast<mdf::SourceType>(type));
  }
}

BusType MdfSourceInformation::Bus::get() {
  return info_ != nullptr ?
    static_cast<BusType>(info_->Bus()) : BusType::None;
}

void MdfSourceInformation::Bus::set(BusType bus) {
  if (info_ != nullptr) {
    info_->Bus(static_cast<mdf::BusType>(bus));
  }
}

uint8_t MdfSourceInformation::Flags::get() {
  return info_ != nullptr ? info_->Flags() : 0; 
}

void MdfSourceInformation::Flags::set(uint8_t flags) {
  if (info_ != nullptr) {
    info_->Flags(flags);
  }
}

MdfMetaData^ MdfSourceInformation::MetaData::get() {
  const auto* temp = info_ != nullptr ? info_->MetaData() : nullptr;
  return temp != nullptr ?
    gcnew MdfMetaData(const_cast<mdf::IMetaData*>(temp)) : nullptr;
}

MdfMetaData^ MdfSourceInformation::CreateMetaData() {
  auto* temp = info_ != nullptr ? info_->CreateMetaData() : nullptr;
  return gcnew MdfMetaData(temp);
}

MdfSourceInformation::MdfSourceInformation(mdf::ISourceInformation* info)
  : info_(info) {
  
}

}
