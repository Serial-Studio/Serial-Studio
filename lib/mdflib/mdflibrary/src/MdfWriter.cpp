/*
 * Copyright 2023 Simplxs
 * SPDX-License-Identifier: MIT
 */

#include "MdfWriter.h"
#include "MdfNetHelper.h"
#include "mdflibrary.h"

namespace MdfLibrary {

MdfWriter::MdfWriter(MdfWriterType writer_type) {
  writer_ = mdf::MdfFactory::CreateMdfWriterEx(
        static_cast<mdf::MdfWriterType>(writer_type));
}

MdfWriter::~MdfWriter() { this->!MdfWriter(); }


MdfWriter::!MdfWriter() {
  if (writer_ != nullptr) {
    delete writer_;
    writer_ = nullptr;
  }
}

String^ MdfWriter::Name::get() {
  return writer_ == nullptr ? String::Empty :
    MdfLibrary::Utf8Conversion(writer_->Name());
}

MdfFile^ MdfWriter::File::get() {
  auto* mdf_file = writer_ != nullptr ? writer_->GetFile() : nullptr;
  return mdf_file != nullptr ? gcnew MdfFile(mdf_file) : nullptr;
}

MdfHeader^ MdfWriter::Header::get() {
  auto* header = writer_ != nullptr ? writer_->Header() : nullptr;
  return header != nullptr ? gcnew MdfHeader(header) : nullptr;
}

bool MdfWriter::IsFileNew::get() {
  return writer_  != nullptr ? writer_->IsFileNew() : false;
}

void MdfWriter::CompressData::set(bool compress) {
  if (writer_ != nullptr)  {
    writer_->CompressData(compress);
  }  
}

bool MdfWriter::CompressData::get() {
    return  writer_ != nullptr ? writer_->CompressData() : false;
}

void MdfWriter::PreTrigTime::set(double preTrigTime) {
  if (writer_ != nullptr) {
    writer_->PreTrigTime(preTrigTime);
  }
}

double MdfWriter::PreTrigTime::get() {
  return writer_ != nullptr ? writer_->PreTrigTime() : 0.0;
}

uint64_t MdfWriter::StartTime::get() {
  return writer_ != nullptr ? writer_->StartTime(): 0;
}

uint64_t MdfWriter::StopTime::get() {
  return writer_ != nullptr ? writer_->StopTime(): 0;
}

void MdfWriter::BusType::set(MdfBusType type) {
  if (writer_ != nullptr) {
    writer_->BusType(static_cast<uint16_t>(type));
  }
}

MdfBusType MdfWriter::BusType::get() {
  return writer_ != nullptr ? static_cast<MdfBusType>(writer_->BusType()) :
    MdfBusType::UNKNOWN;
}

void MdfWriter::StorageType::set(MdfStorageType type) {
  if (writer_ != nullptr) {
    writer_->StorageType(static_cast<mdf::MdfStorageType>(type));
  }
}

MdfStorageType MdfWriter::StorageType::get() {
  return writer_ != nullptr ?
    static_cast<MdfStorageType>(writer_->StorageType()) :
    MdfStorageType::FixedLengthStorage;
}

void MdfWriter::MaxLength::set(uint32_t length) {
  if (writer_ != nullptr) {
    writer_->MaxLength(length);
  }
}

uint32_t MdfWriter::MaxLength::get() {
  return writer_ != nullptr ? writer_->MaxLength() : 8;
}

bool MdfWriter::Init(String^ path_name) {
  return writer_ != nullptr ?
    writer_->Init(MdfLibrary::Utf8Conversion(path_name)) : false;
}

MdfDataGroup^ MdfWriter::CreateDataGroup() {
  auto* data_group = writer_->CreateDataGroup();
  return data_group != nullptr ? gcnew MdfDataGroup(data_group) : nullptr;
}

bool MdfWriter::CreateBusLogConfiguration() {
  return  writer_ != nullptr ? writer_->CreateBusLogConfiguration() : false;
}

bool MdfWriter::InitMeasurement() {
  return writer_ != nullptr ?writer_->InitMeasurement() : false;
}

void MdfWriter::SaveSample(MdfChannelGroup^ group, uint64_t time) {
  if (writer_ != nullptr && group != nullptr && group->group_ != nullptr) {
    writer_->SaveSample(*group->group_, time);
  }
}
void MdfWriter::SaveSample(MdfDataGroup^ data_group, 
                           MdfChannelGroup^ channel_group, uint64_t time) {
  if (writer_ != nullptr && data_group != nullptr && channel_group != nullptr 
        && data_group->group_ != nullptr && channel_group->group_ != nullptr) {
    writer_->SaveSample(*data_group->group_, *channel_group->group_, time);
  }
}

void MdfWriter::SaveCanMessage(MdfChannelGroup^ group, uint64_t time,
    const CanMessage^ message) {
  if (writer_ != nullptr && group != nullptr && group->group_ != nullptr 
      && message != nullptr && message->msg_ != nullptr) {
    writer_->SaveCanMessage(*group->group_, time,*message->msg_);
  }
 
}
void MdfWriter::SaveCanMessage(MdfDataGroup^ data_group, 
                               MdfChannelGroup^ channel_group, 
                               uint64_t time,
                               const CanMessage^ message) {
  if (writer_ != nullptr 
      && data_group != nullptr && data_group->group_ != nullptr 
      && channel_group != nullptr && channel_group->group_ != nullptr 
      && message != nullptr && message->msg_ != nullptr) {
    writer_->SaveCanMessage(*data_group->group_, 
                            *channel_group->group_, 
                            time,
                            *message->msg_);
  }
 
}
void MdfWriter::StartMeasurement(uint64_t start_time) {
  if (writer_ != nullptr) {
    writer_->StartMeasurement(start_time);
  }
}

void MdfWriter::StartMeasurement(DateTime start_time) {
  if (writer_ != nullptr) {
    writer_->StartMeasurement(MdfNetHelper::GetUnixNanoTimestamp(start_time));
  }
}

void MdfWriter::StartMeasurement(IMdfTimeStamp^ start_time) {
  if (writer_ != nullptr && start_time != nullptr) {
    writer_->StartMeasurement(*start_time->GetTimestamp());
  }
}

void MdfWriter::StopMeasurement(uint64_t stop_time) {
  if (writer_ != nullptr) {
    writer_->StopMeasurement(stop_time);
  }
}

void MdfWriter::StopMeasurement(DateTime stop_time) {
  if (writer_ != nullptr) {
    writer_->StopMeasurement(MdfNetHelper::GetUnixNanoTimestamp(stop_time));
  }
}

bool MdfWriter::FinalizeMeasurement() {
  return writer_ != nullptr ? writer_->FinalizeMeasurement() : false;
}

}  // namespace MdfLibrary
