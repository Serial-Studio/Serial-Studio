/*
* Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "MdfReader.h"
#include "mdflibrary.h"

namespace MdfLibrary {

MdfReader::MdfReader(String^ filename) {
  reader_ = new mdf::MdfReader(MdfLibrary::Utf8Conversion(filename));
}


MdfReader::~MdfReader() {
  this->!MdfReader();
}

MdfReader::!MdfReader() {
  delete reader_;
  reader_ = nullptr;
}

Int64 MdfReader::Index::get() {
  return reader_ != nullptr ? reader_->Index(): 0;
}

void MdfReader::Index::set(Int64 index) {
  if (reader_ != nullptr) {
    reader_->Index(index);
  }
}

String^ MdfReader::Name::get() {
  return reader_ != nullptr ? MdfLibrary::Utf8Conversion(reader_->ShortName())
                            : gcnew String("");
}

MdfFile^ MdfReader::File::get() {
  auto* mdf_file = reader_ != nullptr ?
    const_cast<mdf::MdfFile*>(reader_->GetFile()) : nullptr;
  return mdf_file != nullptr ? gcnew MdfFile(mdf_file) : nullptr;
}

MdfHeader^ MdfReader::Header::get() {
   auto* header = reader_ != nullptr ?
     const_cast<mdf::IHeader*>(reader_->GetHeader()) : nullptr;
   return header != nullptr ? gcnew MdfHeader(header) : nullptr; 
}

MdfDataGroup^ MdfReader::DataGroup::get(size_t index) {
   auto* data_group = reader_ != nullptr ?
     const_cast<mdf::IDataGroup*>(reader_->GetDataGroup(index)) : nullptr;
   return data_group != nullptr ? gcnew MdfDataGroup(data_group) : nullptr;   
}

bool MdfReader::IsOk::get() {
  return reader_ != nullptr ? reader_->IsOk() : false;
}

bool MdfReader::Open() {
  return reader_ != nullptr ? reader_->Open() : false;
}

void MdfReader::Close() {
   if (reader_ != nullptr ) {
     reader_->Close();
   }
}

bool MdfReader::ReadHeader() {
  return reader_ != nullptr ? reader_->ReadHeader() : false;
}

bool MdfReader::ReadMeasurementInfo() {
  return reader_ != nullptr ? reader_->ReadMeasurementInfo() : false;  
}

bool MdfReader::ReadEverythingButData() {
  return reader_ != nullptr ? reader_->ReadEverythingButData() : false;  
}

bool MdfReader::ReadData(MdfDataGroup^ group) {
  auto* data_group = group->group_;
  return reader_ != nullptr && data_group != nullptr ?
     reader_->ReadData(*data_group) : false;    
}

bool MdfReader::ReadPartialData(MdfDataGroup^ group, 
    UInt64 min_sample,
    UInt64 max_sample) {
  auto* data_group = group->group_;
  return reader_ != nullptr && data_group != nullptr ?
     reader_->ReadPartialData(*data_group,
         static_cast<size_t>(min_sample),
         static_cast<size_t>(max_sample)) : false;    
}

}

