/*
 * Copyright 2023 Simplxs
 * SPDX-License-Identifier: MIT
 */
#if defined(_WIN32)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <mdf/ichannelgroup.h>
#include <mdf/idatagroup.h>
#include <mdf/ievent.h>
#include <mdf/ifilehistory.h>
#include <mdf/mdffactory.h>
#include <mdf/mdfreader.h>
#include <mdf/mdfwriter.h>

using namespace mdf;

#if defined(_WIN32)
// WINDOWS
#define EXPORT(ReturnType, ClassName, FuncName, ...) \
  __declspec(dllexport) ReturnType ClassName##FuncName(__VA_ARGS__)
#elif defined(__linux__)
// LINUX
#define EXPORT(ReturnType, ClassName, FuncName, ...) \
  ReturnType ClassName##FuncName(__VA_ARGS__)
#elif defined(__CYGWIN__)
// CYGWIN
#define EXPORT(ReturnType, ClassName, FuncName, ...) \
  ReturnType ClassName##FuncName(__VA_ARGS__)
#elif defined(__APPLE__)
// MACOS
#define EXPORT(ReturnType, ClassName, FuncName, ...) \
  ReturnType ClassName##FuncName(__VA_ARGS__)
#else
// UNKNOWN
#pragma warning Unknown dynamic link import / export semantics.
#endif

extern "C" {
namespace MdfLibrary::ExportFunctions {
#pragma region MdfReader
EXPORT(mdf::MdfReader*, MdfReader, Init, const char* filename) {
  return new MdfReader(filename);
}
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...) \
  EXPORT(ReturnType, MdfReader, FuncName, mdf::MdfReader* reader, ##__VA_ARGS__)
EXPORTFEATUREFUNC(void, UnInit) { delete reader; }
EXPORTFEATUREFUNC(int64_t, GetIndex) { return reader->Index(); }
EXPORTFEATUREFUNC(bool, IsOk) { return reader->IsOk(); }
EXPORTFEATUREFUNC(const mdf::MdfFile*, GetFile) { return reader->GetFile(); }
EXPORTFEATUREFUNC(const mdf::IHeader*, GetHeader) {
  return reader->GetHeader();
}
EXPORTFEATUREFUNC(const mdf::IDataGroup*, GetDataGroup, size_t index) {
  return reader->GetDataGroup(index);
}
EXPORTFEATUREFUNC(bool, Open) { return reader->Open(); }
EXPORTFEATUREFUNC(void, Close) { reader->Close(); }
EXPORTFEATUREFUNC(bool, ReadHeader) { return reader->ReadHeader(); }
EXPORTFEATUREFUNC(bool, ReadMeasurementInfo) {
  return reader->ReadMeasurementInfo();
}
EXPORTFEATUREFUNC(bool, ReadEverythingButData) {
  return reader->ReadEverythingButData();
}
EXPORTFEATUREFUNC(bool, ReadData, mdf::IDataGroup* group) {
  return reader->ReadData(*group);
}
#undef EXPORTFEATUREFUNC
#pragma endregion

#pragma region MdfWriter
EXPORT(mdf::MdfWriter*, MdfWriter, Init, MdfWriterType type,
       const char* filename) {
  auto* writer = MdfFactory::CreateMdfWriterEx(type);
  if (!writer) return nullptr;
  writer->Init(filename);
  return writer;
}
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...) \
  EXPORT(ReturnType, MdfWriter, FuncName, mdf::MdfWriter* writer, ##__VA_ARGS__)
EXPORTFEATUREFUNC(void, UnInit) { delete writer; }
EXPORTFEATUREFUNC(mdf::MdfFile*, GetFile) { return writer->GetFile(); }
EXPORTFEATUREFUNC(mdf::IHeader*, GetHeader) { return writer->Header(); }
EXPORTFEATUREFUNC(bool, IsFileNew) { return writer->IsFileNew(); }
EXPORTFEATUREFUNC(bool, GetCompressData) { return writer->CompressData(); }
EXPORTFEATUREFUNC(void, SetCompressData, bool compress) {
  writer->CompressData(compress);
}
EXPORTFEATUREFUNC(double, GetPreTrigTime) { return writer->PreTrigTime(); }
EXPORTFEATUREFUNC(void, SetPreTrigTime, double pre_trig_time) {
  writer->PreTrigTime(pre_trig_time);
}
EXPORTFEATUREFUNC(uint64_t, GetStartTime) { return writer->StartTime(); }
EXPORTFEATUREFUNC(uint64_t, GetStopTime) { return writer->StopTime(); }
EXPORTFEATUREFUNC(uint16_t, GetBusType) { return writer->BusType(); }
EXPORTFEATUREFUNC(void, SetBusType, uint16_t type) { writer->BusType(type); }
EXPORTFEATUREFUNC(MdfStorageType, GetStorageType) {
  return writer->StorageType();
}
EXPORTFEATUREFUNC(void, SetStorageType, MdfStorageType type) {
  writer->StorageType(type);
}
EXPORTFEATUREFUNC(uint32_t, GetMaxLength) { return writer->MaxLength(); }
EXPORTFEATUREFUNC(void, SetMaxLength, uint32_t length) {
  writer->MaxLength(length);
}
EXPORTFEATUREFUNC(bool, CreateBusLogConfiguration) {
  return writer->CreateBusLogConfiguration();
}
EXPORTFEATUREFUNC(mdf::IDataGroup*, CreateDataGroup) {
  return writer->CreateDataGroup();
}
EXPORTFEATUREFUNC(bool, InitMeasurement) { return writer->InitMeasurement(); }
EXPORTFEATUREFUNC(void, SaveSample, mdf::IChannelGroup* group, uint64_t time) {
  writer->SaveSample(*group, time);
}
EXPORTFEATUREFUNC(void, SaveCanMessage, mdf::IChannelGroup* group,
                  uint64_t time, mdf::CanMessage* message) {
  writer->SaveCanMessage(*group, time, *message);
}
EXPORTFEATUREFUNC(void, StartMeasurement, uint64_t start_time) {
  writer->StartMeasurement(start_time);
}
EXPORTFEATUREFUNC(void, StopMeasurement, uint64_t stop_time) {
  writer->StopMeasurement(stop_time);
}
EXPORTFEATUREFUNC(bool, FinalizeMeasurement) {
  return writer->FinalizeMeasurement();
}
#undef EXPORTFEATUREFUNC
#pragma endregion

#pragma region MdfFile
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...) \
  EXPORT(ReturnType, MdfFile, FuncName, mdf::MdfFile* file, ##__VA_ARGS__)
EXPORTFEATUREFUNC(size_t, GetName, char* name) {
  if (name) strcpy(name, file->Name().c_str());
  return file->Name().size();
}
EXPORTFEATUREFUNC(void, SetName, const char* name) { file->Name(name); }
EXPORTFEATUREFUNC(size_t, GetFileName, char* filename) {
  if (filename) strcpy(filename, file->FileName().c_str());
  return file->FileName().size();
}
EXPORTFEATUREFUNC(void, SetFileName, const char* filename) {
  file->FileName(filename);
}
EXPORTFEATUREFUNC(size_t, GetVersion, char* version) {
  if (version) strcpy(version, file->Version().c_str());
  return file->Version().size();
}
EXPORTFEATUREFUNC(int, GetMainVersion) { return file->MainVersion(); }
EXPORTFEATUREFUNC(int, GetMinorVersion) { return file->MinorVersion(); }
EXPORTFEATUREFUNC(void, SetMinorVersion, int minor) {
  file->MinorVersion(minor);
}
EXPORTFEATUREFUNC(size_t, GetProgramId, char* program_id) {
  if (program_id) strcpy(program_id, file->ProgramId().c_str());
  return file->ProgramId().size();
}
EXPORTFEATUREFUNC(void, SetProgramId, const char* program_id) {
  file->ProgramId(program_id);
}
EXPORTFEATUREFUNC(bool, GetFinalized, uint16_t& standard_flags,
                  uint16_t& custom_flags) {
  return file->IsFinalized(standard_flags, custom_flags);
}
EXPORTFEATUREFUNC(const mdf::IHeader*, GetHeader) { return file->Header(); }
EXPORTFEATUREFUNC(bool, GetIsMdf4) { return file->IsMdf4(); }
EXPORTFEATUREFUNC(size_t, GetAttachments,
                  const mdf::IAttachment* pAttachment[]) {
  AttachmentList AttachmentList;
  file->Attachments(AttachmentList);
  if (pAttachment == nullptr) return AttachmentList.size();
  for (size_t i = 0; i < AttachmentList.size(); i++)
    pAttachment[i] = AttachmentList[i];
  return AttachmentList.size();
}
EXPORTFEATUREFUNC(size_t, GetDataGroups, const mdf::IDataGroup* pDataGroup[]) {
  DataGroupList DataGroupList;
  file->DataGroups(DataGroupList);
  if (pDataGroup == nullptr) return DataGroupList.size();
  for (size_t i = 0; i < DataGroupList.size(); i++)
    pDataGroup[i] = DataGroupList[i];
  return DataGroupList.size();
}
EXPORTFEATUREFUNC(mdf::IAttachment*, CreateAttachment) {
  return file->CreateAttachment();
}
EXPORTFEATUREFUNC(mdf::IDataGroup*, CreateDataGroup) {
  return file->CreateDataGroup();
}
#undef EXPORTFEATUREFUNC
#pragma endregion

#pragma region MdfHeader
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...) \
  EXPORT(ReturnType, MdfHeader, FuncName, mdf::IHeader* header, ##__VA_ARGS__)
EXPORTFEATUREFUNC(int64_t, GetIndex) { return header->Index(); }
EXPORTFEATUREFUNC(size_t, GetDescription, char* desc) {
  if (desc) strcpy(desc, header->Description().c_str());
  return header->Description().size();
}
EXPORTFEATUREFUNC(void, SetDescription, const char* desc) {
  header->Description(desc);
}
EXPORTFEATUREFUNC(size_t, GetAuthor, char* author) {
  if (author) strcpy(author, header->Author().c_str());
  return header->Author().size();
}
EXPORTFEATUREFUNC(void, SetAuthor, const char* author) {
  header->Author(author);
}
EXPORTFEATUREFUNC(size_t, GetDepartment, char* department) {
  if (department) strcpy(department, header->Department().c_str());
  return header->Department().size();
}
EXPORTFEATUREFUNC(void, SetDepartment, const char* department) {
  header->Department(department);
}
EXPORTFEATUREFUNC(size_t, GetProject, char* project) {
  if (project) strcpy(project, header->Project().c_str());
  return header->Project().size();
}
EXPORTFEATUREFUNC(void, SetProject, const char* project) {
  header->Project(project);
}
EXPORTFEATUREFUNC(size_t, GetSubject, char* subject) {
  if (subject) strcpy(subject, header->Subject().c_str());
  return header->Subject().size();
}
EXPORTFEATUREFUNC(void, SetSubject, const char* subject) {
  header->Subject(subject);
}
EXPORTFEATUREFUNC(size_t, GetMeasurementId, char* uuid) {
  if (uuid) strcpy(uuid, header->MeasurementId().c_str());
  return header->MeasurementId().size();
}
EXPORTFEATUREFUNC(void, SetMeasurementId, const char* uuid) {
  header->MeasurementId(uuid);
}
EXPORTFEATUREFUNC(size_t, GetRecorderId, char* uuid) {
  if (uuid) strcpy(uuid, header->RecorderId().c_str());
  return header->RecorderId().size();
}
EXPORTFEATUREFUNC(void, SetRecorderId, const char* uuid) {
  header->RecorderId(uuid);
}
EXPORTFEATUREFUNC(int64_t, GetRecorderIndex) { return header->RecorderIndex(); }
EXPORTFEATUREFUNC(void, SetRecorderIndex, int64_t index) {
  header->RecorderIndex(index);
}
EXPORTFEATUREFUNC(uint64_t, GetStartTime) { return header->StartTime(); }
EXPORTFEATUREFUNC(void, SetStartTime, uint64_t time) {
  header->StartTime(time);
}
EXPORTFEATUREFUNC(bool, IsStartAngleUsed) {
  return header->StartAngle().has_value();
}
EXPORTFEATUREFUNC(double, GetStartAngle) {
  return header->StartAngle().value_or(0.0);
}
EXPORTFEATUREFUNC(void, SetStartAngle, double angle) {
  header->StartAngle(angle);
}
EXPORTFEATUREFUNC(bool, IsStartDistanceUsed) {
  return header->StartDistance().has_value();
}
EXPORTFEATUREFUNC(double, GetStartDistance) {
  return header->StartDistance().value_or(0.0);
}
EXPORTFEATUREFUNC(void, SetStartDistance, double distance) {
  header->StartDistance(distance);
}
EXPORTFEATUREFUNC(const mdf::IMetaData*, GetMetaData) {
  return header->MetaData();
}
EXPORTFEATUREFUNC(size_t, GetAttachments, mdf::IAttachment* pAttachments[]) {
  if (pAttachments == nullptr) return header->Attachments().size();
  auto attachments = header->Attachments();
  for (size_t i = 0; i < attachments.size(); ++i)
    pAttachments[i] = attachments[i];
  return attachments.size();
}
EXPORTFEATUREFUNC(size_t, GetFileHistorys, mdf::IFileHistory* pFileHistorys[]) {
  if (pFileHistorys == nullptr) return header->FileHistories().size();
  auto file_historys = header->FileHistories();
  for (size_t i = 0; i < file_historys.size(); ++i)
    pFileHistorys[i] = file_historys[i];
  return file_historys.size();
}
EXPORTFEATUREFUNC(size_t, GetEvents, mdf::IEvent* pEvents[]) {
  if (pEvents == nullptr) return header->Events().size();
  auto events = header->Events();
  for (size_t i = 0; i < events.size(); ++i) pEvents[i] = events[i];
  return events.size();
}
EXPORTFEATUREFUNC(size_t, GetDataGroups, mdf::IDataGroup* pDataGroups[]) {
  if (pDataGroups == nullptr) return header->DataGroups().size();
  auto data_groups = header->DataGroups();
  for (size_t i = 0; i < data_groups.size(); ++i)
    pDataGroups[i] = data_groups[i];
  return data_groups.size();
}
EXPORTFEATUREFUNC(mdf::IDataGroup*, GetLastDataGroup) {
  return header->LastDataGroup();
}
EXPORTFEATUREFUNC(mdf::IMetaData*, CreateMetaData) {
  return header->CreateMetaData();
}
EXPORTFEATUREFUNC(mdf::IAttachment*, CreateAttachment) {
  return header->CreateAttachment();
}
EXPORTFEATUREFUNC(mdf::IFileHistory*, CreateFileHistory) {
  return header->CreateFileHistory();
}
#undef CreateEvent
EXPORTFEATUREFUNC(mdf::IEvent*, CreateEvent) { return header->CreateEvent(); }
EXPORTFEATUREFUNC(mdf::IDataGroup*, CreateDataGroup) {
  return header->CreateDataGroup();
}
#undef EXPORTFEATUREFUNC
#pragma endregion

#pragma region MdfDataGroup
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...)                 \
  EXPORT(ReturnType, MdfDataGroup, FuncName, mdf::IDataGroup* group, \
         ##__VA_ARGS__)
EXPORTFEATUREFUNC(int64_t, GetIndex) { return group->Index(); }
EXPORTFEATUREFUNC(size_t, GetDescription, char* description) {
  if (description) strcpy(description, group->Description().c_str());
  return group->Description().size();
}
EXPORTFEATUREFUNC(uint8_t, GetRecordIdSize) { return group->RecordIdSize(); }
EXPORTFEATUREFUNC(const mdf::IMetaData*, GetMetaData) {
  return group->MetaData();
}
EXPORTFEATUREFUNC(mdf::IChannelGroup*, GetChannelGroupByName,
                  const char* name) {
  return group->GetChannelGroup(name);
}
EXPORTFEATUREFUNC(mdf::IChannelGroup*, GetChannelGroupByRecordId,
                  uint64_t record_id) {
  return group->GetChannelGroup(record_id);
}
EXPORTFEATUREFUNC(size_t, GetChannelGroups,
                  mdf::IChannelGroup* pChannelGroups[]) {
  if (pChannelGroups == nullptr) return group->ChannelGroups().size();
  auto channel_groups = group->ChannelGroups();
  for (size_t i = 0; i < channel_groups.size(); ++i)
    pChannelGroups[i] = channel_groups[i];
  return channel_groups.size();
}
EXPORTFEATUREFUNC(bool, IsRead) { return group->IsRead(); }
EXPORTFEATUREFUNC(mdf::IMetaData*, CreateMetaData) {
  return group->CreateMetaData();
}
EXPORTFEATUREFUNC(mdf::IChannelGroup*, CreateChannelGroup) {
  return group->CreateChannelGroup();
}
EXPORTFEATUREFUNC(const mdf::IChannelGroup*, FindParentChannelGroup,
                  const mdf::IChannel* channel) {
  return group->FindParentChannelGroup(*channel);
}
#undef EXPORTFEATUREFUNC
#pragma endregion

#pragma region MdfChannelGroup
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...)                       \
  EXPORT(ReturnType, MdfChannelGroup, FuncName, mdf::IChannelGroup* group, \
         ##__VA_ARGS__)
EXPORTFEATUREFUNC(int64_t, GetIndex) { return group->Index(); }
EXPORTFEATUREFUNC(uint64_t, GetRecordId) { return group->RecordId(); }
EXPORTFEATUREFUNC(size_t, GetName, char* name) {
  if (name) strcpy(name, group->Name().c_str());
  return group->Name().size();
}
EXPORTFEATUREFUNC(void, SetName, const char* name) { group->Name(name); }
EXPORTFEATUREFUNC(size_t, GetDescription, char* desc) {
  if (desc) strcpy(desc, group->Description().c_str());
  return group->Description().size();
}
EXPORTFEATUREFUNC(void, SetDescription, const char* desc) {
  group->Description(desc);
}
EXPORTFEATUREFUNC(uint64_t, GetNofSamples) { return group->NofSamples(); }
EXPORTFEATUREFUNC(void, SetNofSamples, uint64_t samples) {
  group->NofSamples(samples);
}
EXPORTFEATUREFUNC(uint16_t, GetFlags) { return group->Flags(); }
EXPORTFEATUREFUNC(void, SetFlags, uint16_t flags) { group->Flags(flags); }
EXPORTFEATUREFUNC(wchar_t, GetPathSeparator) { return group->PathSeparator(); }
EXPORTFEATUREFUNC(void, SetPathSeparator, wchar_t sep) {
  group->PathSeparator(sep);
}
EXPORTFEATUREFUNC(size_t, GetChannels, mdf::IChannel* pChannels[]) {
  if (pChannels == nullptr) return group->Channels().size();
  auto channels = group->Channels();
  for (size_t i = 0; i < channels.size(); i++) pChannels[i] = channels[i];
  return channels.size();
}
EXPORTFEATUREFUNC(const mdf::IMetaData*, GetMetaData) {
  return group->MetaData();
}
EXPORTFEATUREFUNC(const mdf::ISourceInformation*, GetSourceInformation) {
  return group->SourceInformation();
}
EXPORTFEATUREFUNC(const mdf::IChannel*, GetXChannel,
                  const mdf::IChannel* ref_channel) {
  return group->GetXChannel(*ref_channel);
}
EXPORTFEATUREFUNC(mdf::IMetaData*, CreateMetaData) {
  return group->CreateMetaData();
}
EXPORTFEATUREFUNC(mdf::IChannel*, CreateChannel) {
  return group->CreateChannel();
}
EXPORTFEATUREFUNC(mdf::ISourceInformation*, CreateSourceInformation) {
  return group->CreateSourceInformation();
}
#undef EXPORTFEATUREFUNC
#pragma endregion

#pragma region MdfChannel
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...)               \
  EXPORT(ReturnType, MdfChannel, FuncName, mdf::IChannel* channel, \
         ##__VA_ARGS__)

EXPORTFEATUREFUNC(int64_t, GetIndex) { return channel->Index(); }
EXPORTFEATUREFUNC(size_t, GetName, char* name) {
  if (name) strcpy(name, channel->Name().c_str());
  return channel->Name().size();
}
EXPORTFEATUREFUNC(void, SetName, const char* name) { channel->Name(name); }
EXPORTFEATUREFUNC(size_t, GetDisplayName, char* name) {
  if (name) strcpy(name, channel->DisplayName().c_str());
  return channel->DisplayName().size();
}
EXPORTFEATUREFUNC(void, SetDisplayName, const char* name) {
  channel->DisplayName(name);
}
EXPORTFEATUREFUNC(size_t, GetDescription, char* desc) {
  if (desc) strcpy(desc, channel->Description().c_str());
  return channel->Description().size();
}
EXPORTFEATUREFUNC(void, SetDescription, const char* desc) {
  channel->Description(desc);
}
EXPORTFEATUREFUNC(bool, IsUnitUsed) { return channel->IsUnitValid(); }
EXPORTFEATUREFUNC(size_t, GetUnit, char* unit) {
  if (unit) strcpy(unit, channel->Unit().c_str());
  return channel->Unit().size();
}
EXPORTFEATUREFUNC(void, SetUnit, const char* unit) { channel->Unit(unit); }
EXPORTFEATUREFUNC(ChannelType, GetType) { return channel->Type(); }
EXPORTFEATUREFUNC(void, SetType, ChannelType type) { channel->Type(type); }
EXPORTFEATUREFUNC(ChannelSyncType, GetSync) { return channel->Sync(); }
EXPORTFEATUREFUNC(void, SetSync, ChannelSyncType type) { channel->Sync(type); }
EXPORTFEATUREFUNC(ChannelDataType, GetDataType) { return channel->DataType(); }
EXPORTFEATUREFUNC(void, SetDataType, ChannelDataType type) {
  channel->DataType(type);
}
EXPORTFEATUREFUNC(uint32_t, GetFlags) { return channel->Flags(); }
EXPORTFEATUREFUNC(void, SetFlags, uint32_t flags) { channel->Flags(flags); }
EXPORTFEATUREFUNC(uint64_t, GetDataBytes) { return channel->DataBytes(); }
EXPORTFEATUREFUNC(void, SetDataBytes, uint64_t bytes) {
  channel->DataBytes(bytes);
}
EXPORTFEATUREFUNC(bool, IsPrecisionUsed) { return channel->IsDecimalUsed(); }
EXPORTFEATUREFUNC(uint8_t, GetPrecision) { return channel->Decimals(); }
EXPORTFEATUREFUNC(bool, IsRangeUsed) { return channel->Range().has_value(); }
EXPORTFEATUREFUNC(double, GetRangeMin) { return channel->Range()->first; }
EXPORTFEATUREFUNC(double, GetRangeMax) { return channel->Range()->second; }
EXPORTFEATUREFUNC(void, SetRange, double min, double max) {
  channel->Range(min, max);
}
EXPORTFEATUREFUNC(bool, IsLimitUsed) { return channel->Limit().has_value(); }
EXPORTFEATUREFUNC(double, GetLimitMin) { return channel->Limit()->first; }
EXPORTFEATUREFUNC(double, GetLimitMax) { return channel->Limit()->second; }
EXPORTFEATUREFUNC(void, SetLimit, double min, double max) {
  channel->Limit(min, max);
}
EXPORTFEATUREFUNC(bool, IsExtLimitUsed) {
  return channel->ExtLimit().has_value();
}
EXPORTFEATUREFUNC(double, GetExtLimitMin) { return channel->ExtLimit()->first; }
EXPORTFEATUREFUNC(double, GetExtLimitMax) {
  return channel->ExtLimit()->second;
}
EXPORTFEATUREFUNC(void, SetExtLimit, double min, double max) {
  channel->ExtLimit(min, max);
}
EXPORTFEATUREFUNC(double, GetSamplingRate) { return channel->SamplingRate(); }
EXPORTFEATUREFUNC(uint64_t, GetVlsdRecordId) { return channel->VlsdRecordId(); }
EXPORTFEATUREFUNC(void, SetVlsdRecordId, uint64_t record_id) {
  channel->VlsdRecordId(record_id);
}
EXPORTFEATUREFUNC(uint32_t, GetBitCount) { return channel->BitCount(); }
EXPORTFEATUREFUNC(void, SetBitCount, uint32_t bits) { channel->BitCount(bits); }
EXPORTFEATUREFUNC(uint16_t, GetBitOffset) { return channel->BitOffset(); }
EXPORTFEATUREFUNC(void, SetBitOffset, uint16_t bits) {
  channel->BitOffset(bits);
}
EXPORTFEATUREFUNC(const mdf::IMetaData*, GetMetaData) {
  return channel->MetaData();
}
EXPORTFEATUREFUNC(const mdf::ISourceInformation*, GetSourceInformation) {
  return channel->SourceInformation();
}
EXPORTFEATUREFUNC(const mdf::IChannelConversion*, GetChannelConversion) {
  return channel->ChannelConversion();
}
EXPORTFEATUREFUNC(const mdf::IChannelArray*, GetChannelArray) {
  return channel->ChannelArray(0);
}
EXPORTFEATUREFUNC(size_t, GetChannelCompositions, mdf::IChannel* pChannels[]) {
  if (pChannels == nullptr) return channel->ChannelCompositions().size();
  auto channels = channel->ChannelCompositions();
  for (size_t i = 0; i < channels.size(); i++) pChannels[i] = channels[i];
  return channels.size();
}
EXPORTFEATUREFUNC(mdf::ISourceInformation*, CreateSourceInformation) {
  return channel->CreateSourceInformation();
}
EXPORTFEATUREFUNC(mdf::IChannelConversion*, CreateChannelConversion) {
  return channel->CreateChannelConversion();
}
EXPORTFEATUREFUNC(mdf::IChannelArray*, CreateChannelArray) {
  return channel->CreateChannelArray();
}
EXPORTFEATUREFUNC(mdf::IChannel*, CreateChannelComposition) {
  return channel->CreateChannelComposition();
}
EXPORTFEATUREFUNC(mdf::IMetaData*, CreateMetaData) {
  return channel->CreateMetaData();
}
EXPORTFEATUREFUNC(void, SetChannelValueAsSigned, const int64_t value,
                  bool valid, uint64_t array_index) {
  channel->SetChannelValue(value, valid, array_index);
}
EXPORTFEATUREFUNC(void, SetChannelValueAsUnSigned, const uint64_t value,
                  bool valid, uint64_t array_index) {
  channel->SetChannelValue(value, valid, array_index);
}
EXPORTFEATUREFUNC(void, SetChannelValueAsFloat, const double value,
                  bool valid, uint64_t array_index) {
  channel->SetChannelValue(value, valid, array_index);
}
EXPORTFEATUREFUNC(void, SetChannelValueAsString, const char* value,
                  bool valid, uint64_t array_index) {
  channel->SetChannelValue(std::string(value), valid, array_index);
}
EXPORTFEATUREFUNC(void, SetChannelValueAsArray, const uint8_t* value,
                  size_t size, bool valid, uint64_t array_index) {
  channel->SetChannelValue(std::vector<uint8_t>(value, value + size), valid, array_index);
}
#undef EXPORTFEATUREFUNC
#pragma endregion

#pragma region MdfChannelArray
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...)                       \
  EXPORT(ReturnType, MdfChannelArray, FuncName, mdf::IChannelArray* array, \
         ##__VA_ARGS__)
EXPORTFEATUREFUNC(int64_t, GetIndex) { return array->Index(); }
EXPORTFEATUREFUNC(ArrayType, GetType) { return array->Type(); }
EXPORTFEATUREFUNC(void, SetType, ArrayType type) { array->Type(type); }
EXPORTFEATUREFUNC(ArrayStorage, GetStorage) { return array->Storage(); }
EXPORTFEATUREFUNC(void, SetStorage, ArrayStorage storage) {
  array->Storage(storage);
}
EXPORTFEATUREFUNC(uint32_t, GetFlags) { return array->Flags(); }
EXPORTFEATUREFUNC(void, SetFlags, uint32_t flags) { array->Flags(flags); }
EXPORTFEATUREFUNC(size_t, GetDimensions) { return array->Dimensions(); }
EXPORTFEATUREFUNC(size_t, GetShape, uint64_t pShape[]) {
  if (pShape == nullptr) return array->Shape().size();
  auto shape = array->Shape();
  for (size_t i = 0; i < shape.size(); i++) pShape[i] = shape[i];
  return shape.size();
}
EXPORTFEATUREFUNC(void, SetShape, uint64_t pShape[], size_t count) {
  std::vector<uint64_t> shape(count);
  for (size_t i = 0; i < count; i++) shape[i] = pShape[i];
  array->Shape(shape);
}
EXPORTFEATUREFUNC(uint64_t, GetDimensionSize, size_t dimension) { return array->DimensionSize(dimension); }
  // /** \brief Returns the fixed axis value list for reading only. */
  // [[nodiscard]] virtual const std::vector<double>&  AxisValues() const = 0;

  // /** \brief Returns the fixed axis value list for write. */
  // [[nodiscard]] virtual std::vector<double>&  AxisValues() = 0;

  // /** \brief Returns a list of cycle counts. */
  // [[nodiscard]] virtual const std::vector<uint64_t>& CycleCounts() const = 0;

  // /** \brief Returns a list of cycle counts. */
  // [[nodiscard]] virtual std::vector<uint64_t>& CycleCounts() = 0;
EXPORTFEATUREFUNC(size_t, GetAxisValues, double pAxisValues[]) {
  if (pAxisValues == nullptr) return array->AxisValues().size();
  const auto axis_values = array->AxisValues();
  for (size_t i = 0; i < axis_values.size(); i++) pAxisValues[i] = axis_values[i];
  return axis_values.size();
}
EXPORTFEATUREFUNC(size_t, GetCycleCounts, uint64_t pCycleCounts[]) {
  if (pCycleCounts == nullptr) return array->CycleCounts().size();
  const auto cycle_counts = array->CycleCounts();
  for (size_t i = 0; i < cycle_counts.size(); i++) pCycleCounts[i] = cycle_counts[i];
  return cycle_counts.size();
}
EXPORTFEATUREFUNC(uint64_t, GetNofArrayValues) { return array->NofArrayValues(); }
EXPORTFEATUREFUNC(size_t, GetDimensionAsString, char* dimension) {
  if (dimension) strcpy(dimension, array->DimensionAsString().c_str());
  return array->DimensionAsString().size();
}
EXPORTFEATUREFUNC(void, ResizeArrays) { return array->ResizeArrays(); }
#undef EXPORTFEATUREFUNC
#pragma endregion

#pragma region MdfChannelConversion
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...) \
  EXPORT(ReturnType, MdfChannelConversion, FuncName, \
         mdf::IChannelConversion* conv, ##__VA_ARGS__)
EXPORTFEATUREFUNC(int64_t, GetIndex) { return conv->Index(); }
EXPORTFEATUREFUNC(size_t, GetName, char* name) {
  if (name) strcpy(name, conv->Name().c_str());
  return conv->Name().size();
}
EXPORTFEATUREFUNC(void, SetName, const char* name) { conv->Name(name); }
EXPORTFEATUREFUNC(size_t, GetDescription, char* desc) {
  if (desc) strcpy(desc, conv->Description().c_str());
  return conv->Description().size();
}
EXPORTFEATUREFUNC(void, SetDescription, const char* desc) {
  conv->Description(desc);
}
EXPORTFEATUREFUNC(size_t, GetUnit, char* unit) {
  if (unit) strcpy(unit, conv->Unit().c_str());
  return conv->Unit().size();
}
EXPORTFEATUREFUNC(void, SetUnit, const char* unit) { conv->Unit(unit); }
EXPORTFEATUREFUNC(ConversionType, GetType) { return conv->Type(); }
EXPORTFEATUREFUNC(void, SetType, ConversionType type) { conv->Type(type); }
EXPORTFEATUREFUNC(bool, IsPrecisionUsed) { return conv->IsDecimalUsed(); }
EXPORTFEATUREFUNC(uint8_t, GetPrecision) { return conv->Decimals(); }
EXPORTFEATUREFUNC(bool, IsRangeUsed) { return conv->Range().has_value(); }
EXPORTFEATUREFUNC(double, GetRangeMin) { return conv->Range()->first; }
EXPORTFEATUREFUNC(double, GetRangeMax) { return conv->Range()->second; }
EXPORTFEATUREFUNC(void, SetRange, double min, double max) {
  conv->Range(min, max);
}
EXPORTFEATUREFUNC(uint16_t, GetFlags) { return conv->Flags(); }
EXPORTFEATUREFUNC(size_t, GetReference, uint16_t index, char* reference) {
  if (reference) strcpy(reference, conv->Formula().c_str());
  return conv->Reference(index).size();
}
EXPORTFEATUREFUNC(void, SetReference, uint16_t index, const char* reference) {
  return conv->Reference(index, reference);
}
EXPORTFEATUREFUNC(const mdf::IChannelConversion*, GetInverse) {
  return conv->Inverse();
}
EXPORTFEATUREFUNC(const mdf::IMetaData*, GetMetaData) {
  return conv->MetaData();
}
EXPORTFEATUREFUNC(size_t, GetFormula, char* formula) {
  if (formula) strcpy(formula, conv->Formula().c_str());
  return conv->Formula().size();
}
EXPORTFEATUREFUNC(void, SetFormula, const char* formula) {
  conv->Formula(formula);
}
EXPORTFEATUREFUNC(double, GetNofParameters, uint16_t index) {
  return conv->NofParameters();
}
EXPORTFEATUREFUNC(double, GetParameterAsDouble, uint16_t index) {
  return conv->Parameter(index);
}
EXPORTFEATUREFUNC(void, SetParameterAsDouble, uint16_t index,
                  double parameter) {
  conv->Parameter(index, parameter);
}
EXPORTFEATUREFUNC(uint64_t, GetParameterAsUInt64, uint16_t index) {
  return conv->ParameterUint(index);
}
EXPORTFEATUREFUNC(void, SetParameterAsUInt64, uint16_t index,
                  uint64_t parameter) {
  conv->ParameterUint(index, parameter);
}
EXPORTFEATUREFUNC(mdf::IChannelConversion*, CreateInverse) {
  return conv->CreateInverse();
}
EXPORTFEATUREFUNC(mdf::IMetaData*, CreateMetaData) {
  return conv->CreateMetaData();
}
#undef EXPORTFEATUREFUNC
#pragma endregion

#pragma region MdfChannelObserver
EXPORT(mdf::IChannelObserver*, MdfChannelObserver, Create,
       mdf::IDataGroup* data_group, mdf::IChannelGroup* channel_group,
       mdf::IChannel* channel) {
  auto observer = CreateChannelObserver(*data_group, *channel_group, *channel);
  return observer.release();
}
EXPORT(mdf::IChannelObserver*, MdfChannelObserver, CreateByChannelName,
       mdf::IDataGroup* data_group, const char* channel_name) {
  auto observer = CreateChannelObserver(*data_group, channel_name);
  return observer.release();
}
EXPORT(size_t, MdfChannelObserver, CreateForChannelGroup,
       mdf::IDataGroup* data_group, mdf::IChannelGroup* channel_group,
       mdf::IChannelObserver* pObservers[]) {
  ChannelObserverList temp;
  CreateChannelObserverForChannelGroup(*data_group, *channel_group, temp);
  if (pObservers != nullptr)
    for (size_t i = 0; i < temp.size(); i++) pObservers[i] = temp[i].release();
  return temp.size();
}
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...) \
  EXPORT(ReturnType, MdfChannelObserver, FuncName,   \
         mdf::IChannelObserver* observer, ##__VA_ARGS__)
EXPORTFEATUREFUNC(void, UnInit) { delete observer; }
EXPORTFEATUREFUNC(int64_t, GetNofSamples) { return observer->NofSamples(); }
EXPORTFEATUREFUNC(size_t, GetName, char* name) {
  if (name) strcpy(name, observer->Name().c_str());
  return observer->Name().size();
}
EXPORTFEATUREFUNC(size_t, GetUnit, char* unit) {
  if (unit) strcpy(unit, observer->Unit().c_str());
  return observer->Unit().size();
}
EXPORTFEATUREFUNC(const mdf::IChannel*, GetChannel) {
  return &(observer->Channel());
}
EXPORTFEATUREFUNC(bool, IsMaster) { return observer->IsMaster(); }
EXPORTFEATUREFUNC(bool, GetChannelValueAsSigned, uint64_t sample,
                  int64_t& value) {
  return observer->GetChannelValue(sample, value);
}
EXPORTFEATUREFUNC(bool, GetChannelValueAsUnSigned, uint64_t sample,
                  uint64_t& value) {
  return observer->GetChannelValue(sample, value);
}
EXPORTFEATUREFUNC(bool, GetChannelValueAsFloat, uint64_t sample,
                  double& value) {
  return observer->GetChannelValue(sample, value);
}
EXPORTFEATUREFUNC(bool, GetChannelValueAsString, uint64_t sample, char* value,
                  size_t& size) {
  std::string str;
  bool valid = observer->GetChannelValue(sample, str);
  if (value != nullptr && size >= str.length() + 1) strcpy(value, str.c_str());
  size = str.length();
  return valid;
}
EXPORTFEATUREFUNC(bool, GetChannelValueAsArray, uint64_t sample,
                  uint8_t value[], size_t& size) {
  std::vector<uint8_t> vec;
  bool valid = observer->GetChannelValue(sample, vec);
  if (value != nullptr && size >= vec.size()) memcpy(value, vec.data(), size);
  size = vec.size();
  return valid;
}
EXPORTFEATUREFUNC(bool, GetEngValueAsSigned, uint64_t sample, int64_t& value) {
  return observer->GetEngValue(sample, value);
}
EXPORTFEATUREFUNC(bool, GetEngValueAsUnSigned, uint64_t sample,
                  uint64_t& value) {
  return observer->GetEngValue(sample, value);
}
EXPORTFEATUREFUNC(bool, GetEngValueAsFloat, uint64_t sample, double& value) {
  return observer->GetEngValue(sample, value);
}
EXPORTFEATUREFUNC(bool, GetEngValueAsString, uint64_t sample, char* value,
                  size_t& size) {
  std::string str;
  bool valid = observer->GetEngValue(sample, str);
  if (value != nullptr && size >= str.length() + 1) strcpy(value, str.c_str());
  size = str.length();
  return valid;
}
EXPORTFEATUREFUNC(bool, GetEngValueAsArray, uint64_t sample, uint8_t value[],
                  size_t& size) {
  // Ref `MdfChannelObserver::GetChannelValueAsArray`
  // "Note that engineering value cannot be byte arrays, so I assume"
  // "that it was the observer value that was requested."
  return MdfChannelObserverGetChannelValueAsArray(observer, sample, value,
                                                  size);
}
#undef EXPORTFEATUREFUNC
#pragma endregion

#pragma region MdfSourceInformation
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...) \
  EXPORT(ReturnType, MdfSourceInformation, FuncName, \
         mdf::ISourceInformation* source_information, ##__VA_ARGS__)
EXPORTFEATUREFUNC(int64_t, GetIndex) { return source_information->Index(); }
EXPORTFEATUREFUNC(size_t, GetName, char* name) {
  if (name) strcpy(name, source_information->Name().c_str());
  return source_information->Name().size();
}
EXPORTFEATUREFUNC(void, SetName, const char* name) {
  source_information->Name(name);
}
EXPORTFEATUREFUNC(size_t, GetDescription, char* desc) {
  if (desc) strcpy(desc, source_information->Description().c_str());
  return source_information->Description().size();
}
EXPORTFEATUREFUNC(void, SetDescription, const char* desc) {
  source_information->Description(desc);
}
EXPORTFEATUREFUNC(size_t, GetPath, char* path) {
  if (path) strcpy(path, source_information->Path().c_str());
  return source_information->Path().size();
}
EXPORTFEATUREFUNC(void, SetPath, const char* path) {
  source_information->Path(path);
}
EXPORTFEATUREFUNC(SourceType, GetType) { return source_information->Type(); }
EXPORTFEATUREFUNC(void, SetType, SourceType type) {
  source_information->Type(type);
}
EXPORTFEATUREFUNC(BusType, GetBus) { return source_information->Bus(); }
EXPORTFEATUREFUNC(void, SetBus, BusType bus) { source_information->Bus(bus); }
EXPORTFEATUREFUNC(uint8_t, GetFlags) { return source_information->Flags(); }
EXPORTFEATUREFUNC(void, SetFlags, uint8_t flags) {
  source_information->Flags(flags);
}
EXPORTFEATUREFUNC(const mdf::IMetaData*, GetMetaData) {
  return source_information->MetaData();
}
EXPORTFEATUREFUNC(mdf::IMetaData*, CreateMetaData) {
  return source_information->CreateMetaData();
}
#undef EXPORTFEATUREFUNC
#pragma endregion

#pragma region MdfAttachment
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...)                        \
  EXPORT(ReturnType, MdfAttachment, FuncName, mdf::IAttachment* attachment, \
         ##__VA_ARGS__)
EXPORTFEATUREFUNC(int64_t, GetIndex) { return attachment->Index(); }
EXPORTFEATUREFUNC(uint16_t, GetCreatorIndex) {
  return static_cast<uint16_t>(attachment->CreatorIndex());
}
EXPORTFEATUREFUNC(void, SetCreatorIndex, uint16_t index) {
  attachment->CreatorIndex(index);
}
EXPORTFEATUREFUNC(bool, GetEmbedded) { return attachment->IsEmbedded(); }
EXPORTFEATUREFUNC(void, SetEmbedded, bool embedded) {
  attachment->IsEmbedded(embedded);
}
EXPORTFEATUREFUNC(bool, GetCompressed) { return attachment->IsCompressed(); }
EXPORTFEATUREFUNC(void, SetCompressed, bool compressed) {
  attachment->IsCompressed(compressed);
}
EXPORTFEATUREFUNC(bool, GetMd5, char* md5) {
  if (md5) strcpy(md5, attachment->Md5().value_or("").c_str());
  return attachment->Md5().has_value();
}
EXPORTFEATUREFUNC(size_t, GetFileName, char* name) {
  if (name) strcpy(name, attachment->FileName().c_str());
  return attachment->FileName().size();
}
EXPORTFEATUREFUNC(void, SetFileName, const char* name) {
  attachment->FileName(name);
}
EXPORTFEATUREFUNC(size_t, GetFileType, char* type) {
  if (type) strcpy(type, attachment->FileType().c_str());
  return attachment->FileType().size();
}
EXPORTFEATUREFUNC(void, SetFileType, const char* type) {
  attachment->FileType(type);
}
EXPORTFEATUREFUNC(const mdf::IMetaData*, GetMetaData) {
  return attachment->MetaData();
}
EXPORTFEATUREFUNC(mdf::IMetaData*, CreateMetaData) {
  return attachment->CreateMetaData();
}
#undef EXPORTFEATUREFUNC
#pragma endregion

#pragma region MdfFileHistory
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...) \
  EXPORT(ReturnType, MdfFileHistory, FuncName,       \
         mdf::IFileHistory* file_history, ##__VA_ARGS__)
EXPORTFEATUREFUNC(int64_t, GetIndex) { return file_history->Index(); }
EXPORTFEATUREFUNC(uint64_t, GetTime) { return file_history->Time(); }
EXPORTFEATUREFUNC(void, SetTime, uint64_t time) { file_history->Time(time); }
EXPORTFEATUREFUNC(const mdf::IMetaData*, GetMetaData) {
  return file_history->MetaData();
}
EXPORTFEATUREFUNC(size_t, GetDescription, char* desc) {
  if (desc) strcpy(desc, file_history->Description().c_str());
  return file_history->Description().size();
}
EXPORTFEATUREFUNC(void, SetDescription, const char* desc) {
  file_history->Description(desc);
}
EXPORTFEATUREFUNC(size_t, GetToolName, char* name) {
  if (name) strcpy(name, file_history->ToolName().c_str());
  return file_history->ToolName().size();
}
EXPORTFEATUREFUNC(void, SetToolName, const char* name) {
  file_history->ToolName(name);
}
EXPORTFEATUREFUNC(size_t, GetToolVendor, char* vendor) {
  if (vendor) strcpy(vendor, file_history->ToolVendor().c_str());
  return file_history->ToolVendor().size();
}
EXPORTFEATUREFUNC(void, SetToolVendor, const char* vendor) {
  file_history->ToolVendor(vendor);
}
EXPORTFEATUREFUNC(size_t, GetToolVersion, char* version) {
  if (version) strcpy(version, file_history->ToolVersion().c_str());
  return file_history->ToolVersion().size();
}
EXPORTFEATUREFUNC(void, SetToolVersion, const char* version) {
  file_history->ToolVersion(version);
}
EXPORTFEATUREFUNC(size_t, GetUserName, char* user) {
  if (user) strcpy(user, file_history->UserName().c_str());
  return file_history->UserName().size();
}
EXPORTFEATUREFUNC(void, SetUserName, const char* user) {
  file_history->UserName(user);
}
#undef EXPORTFEATUREFUNC
#pragma endregion

#pragma region MdfEvent
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...) \
  EXPORT(ReturnType, MdfEvent, FuncName, mdf::IEvent* event, ##__VA_ARGS__)
EXPORTFEATUREFUNC(int64_t, GetIndex) { return event->Index(); }
EXPORTFEATUREFUNC(size_t, GetName, char* name) {
  if (name) strcpy(name, event->Name().c_str());
  return event->Name().size();
}
EXPORTFEATUREFUNC(void, SetName, const char* name) { event->Name(name); }
EXPORTFEATUREFUNC(size_t, GetDescription, char* desc) {
  if (desc) strcpy(desc, event->Description().c_str());
  return event->Description().size();
}
EXPORTFEATUREFUNC(void, SetDescription, const char* desc) {
  event->Description(desc);
}
EXPORTFEATUREFUNC(size_t, GetGroupName, char* group) {
  if (group) strcpy(group, event->GroupName().c_str());
  return event->GroupName().size();
}
EXPORTFEATUREFUNC(void, SetGroupName, const char* group) {
  event->GroupName(group);
}
EXPORTFEATUREFUNC(EventType, GetType) { return event->Type(); }
EXPORTFEATUREFUNC(void, SetType, EventType type) { event->Type(type); }
EXPORTFEATUREFUNC(SyncType, GetSync) { return event->Sync(); }
EXPORTFEATUREFUNC(void, SetSync, SyncType type) { event->Sync(type); }
EXPORTFEATUREFUNC(RangeType, GetRange) { return event->Range(); }
EXPORTFEATUREFUNC(void, SetRange, RangeType type) { event->Range(type); }
EXPORTFEATUREFUNC(EventCause, GetCause) { return event->Cause(); }
EXPORTFEATUREFUNC(void, SetCause, EventCause cause) { event->Cause(cause); }
EXPORTFEATUREFUNC(uint16_t, GetCreatorIndex) { return static_cast<uint16_t>(event->CreatorIndex()); }
EXPORTFEATUREFUNC(void, SetCreatorIndex, uint16_t index) {
  event->CreatorIndex(index);
}
EXPORTFEATUREFUNC(int64_t, GetSyncValue) { return event->SyncValue(); }
EXPORTFEATUREFUNC(void, SetSyncValue, int64_t value) {
  event->SyncValue(value);
}
EXPORTFEATUREFUNC(double, GetSyncFactor) { return event->SyncFactor(); }
EXPORTFEATUREFUNC(void, SetSyncFactor, double factor) {
  event->SyncFactor(factor);
}
EXPORTFEATUREFUNC(const mdf::IEvent*, GetParentEvent) {
  return event->ParentEvent();
}
EXPORTFEATUREFUNC(void, SetParentEvent, const mdf::IEvent* parent) {
  event->ParentEvent(parent);
}
EXPORTFEATUREFUNC(const mdf::IEvent*, GetRangeEvent) {
  return event->RangeEvent();
}
EXPORTFEATUREFUNC(void, SetRangeEvent, const mdf::IEvent* range) {
  event->RangeEvent(range);
}
EXPORTFEATUREFUNC(size_t, GetAttachments,
                  const mdf::IAttachment* pAttachment[]) {
  if (pAttachment == nullptr) return event->Attachments().size();
  auto attachment = event->Attachments();
  for (size_t i = 0; i < attachment.size(); ++i) pAttachment[i] = attachment[i];
  return attachment.size();
}
EXPORTFEATUREFUNC(double, GetPreTrig) { return event->PreTrig(); }
EXPORTFEATUREFUNC(void, SetPreTrig, double time) { event->PreTrig(time); }
EXPORTFEATUREFUNC(double, GetPostTrig) { return event->PostTrig(); }
EXPORTFEATUREFUNC(void, SetPostTrig, double time) { event->PostTrig(time); }
EXPORTFEATUREFUNC(const mdf::IMetaData*, GetMetaData) {
  return event->MetaData();
}
EXPORTFEATUREFUNC(void, AddAttachment, const mdf::IAttachment* attachment) {
  event->AddAttachment(attachment);
}
#undef EXPORTFEATUREFUNC
#pragma endregion

#pragma region MdfETag
EXPORT(mdf::ETag*, MdfETag, Init) { return new mdf::ETag; }
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...) \
  EXPORT(ReturnType, MdfETag, FuncName, mdf::ETag* etag, ##__VA_ARGS__)
EXPORTFEATUREFUNC(void, UnInit) { delete etag; }
EXPORTFEATUREFUNC(size_t, GetName, char* name) {
  if (name) strcpy(name, etag->Name().c_str());
  return etag->Name().size();
}
EXPORTFEATUREFUNC(void, SetName, const char* name) { etag->Name(name); }
EXPORTFEATUREFUNC(size_t, GetDescription, char* desc) {
  if (desc) strcpy(desc, etag->Description().c_str());
  return etag->Description().size();
}
EXPORTFEATUREFUNC(void, SetDescription, const char* desc) {
  etag->Description(desc);
}
EXPORTFEATUREFUNC(size_t, GetUnit, char* unit) {
  if (unit) strcpy(unit, etag->Unit().c_str());
  return etag->Unit().size();
}
EXPORTFEATUREFUNC(void, SetUnit, const char* unit) { etag->Unit(unit); }
EXPORTFEATUREFUNC(size_t, GetUnitRef, char* unit) {
  if (unit) strcpy(unit, etag->UnitRef().c_str());
  return etag->UnitRef().size();
}
EXPORTFEATUREFUNC(void, SetUnitRef, const char* unit) { etag->UnitRef(unit); }
EXPORTFEATUREFUNC(size_t, GetType, char* type) {
  if (type) strcpy(type, etag->Type().c_str());
  return etag->Type().size();
}
EXPORTFEATUREFUNC(void, SetType, const char* type) { etag->Type(type); }
EXPORTFEATUREFUNC(ETagDataType, GetDataType) { return etag->DataType(); }
EXPORTFEATUREFUNC(void, SetDataType, ETagDataType type) {
  etag->DataType(type);
}
EXPORTFEATUREFUNC(size_t, GetLanguage, char* language) {
  if (language) strcpy(language, etag->Language().c_str());
  return etag->Language().size();
}
EXPORTFEATUREFUNC(void, SetLanguage, const char* language) {
  etag->Language(language);
}
EXPORTFEATUREFUNC(bool, GetReadOnly) { return etag->ReadOnly(); }
EXPORTFEATUREFUNC(void, SetReadOnly, const bool read_only) {
  etag->ReadOnly(read_only);
}
EXPORTFEATUREFUNC(size_t, GetValueAsString, char* value) {
  auto str = etag->Value<std::string>();
  if (value) strcpy(value, str.c_str());
  return str.length();
}
EXPORTFEATUREFUNC(void, SetValueAsString, const char* value) {
  etag->Value(std::string(value));
}
EXPORTFEATUREFUNC(double, GetValueAsFloat) { return etag->Value<double>(); }
EXPORTFEATUREFUNC(void, SetValueAsFloat, double value) { etag->Value(value); }
EXPORTFEATUREFUNC(bool, GetValueAsBoolean) { return etag->Value<bool>(); }
EXPORTFEATUREFUNC(void, SetValueAsBoolean, bool value) { etag->Value(value); }
EXPORTFEATUREFUNC(int64_t, GetValueAsSigned) { return etag->Value<int64_t>(); }
EXPORTFEATUREFUNC(void, SetValueAsSigned, int64_t value) { etag->Value(value); }
EXPORTFEATUREFUNC(uint64_t, GetValueAsUnsigned) {
  return etag->Value<uint64_t>();
}
EXPORTFEATUREFUNC(void, SetValueAsUnsigned, uint64_t value) {
  etag->Value(value);
}
#undef EXPORTFEATUREFUNC
#pragma endregion

#pragma region MdfMetaData
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...)                  \
  EXPORT(ReturnType, MdfMetaData, FuncName, mdf::IMetaData* metadata, \
         ##__VA_ARGS__)
EXPORTFEATUREFUNC(size_t, GetPropertyAsString, const char* index, char* prop) {
  auto str = metadata->StringProperty(index);
  if (prop) strcpy(prop, str.c_str());
  return str.length();
}
EXPORTFEATUREFUNC(void, SetPropertyAsString, const char* index,
                  const char* prop) {
  metadata->StringProperty(index, std::string(prop));
}
EXPORTFEATUREFUNC(double, GetPropertyAsFloat, const char* index) {
  return metadata->FloatProperty(index);
}
EXPORTFEATUREFUNC(void, SetPropertyAsFloat, const char* index, double prop) {
  metadata->FloatProperty(index, prop);
}
EXPORTFEATUREFUNC(size_t, GetProperties, mdf::ETag* pProperty[]) {
  if (pProperty == nullptr) return metadata->Properties().size();
  auto properties = metadata->Properties();
  for (size_t i = 0; i < properties.size(); ++i)
    pProperty[i] = new mdf::ETag(std::move(properties[i]));
  return properties.size();
}
EXPORTFEATUREFUNC(size_t, GetCommonProperties, mdf::ETag* pProperty[]) {
  if (pProperty == nullptr) return metadata->CommonProperties().size();
  auto properties = metadata->CommonProperties();
  for (size_t i = 0; i < properties.size(); ++i)
    pProperty[i] = new mdf::ETag(std::move(properties[i]));
  return properties.size();
}
EXPORTFEATUREFUNC(void, SetCommonProperties, const mdf::ETag* pProperty[],
                  size_t count) {
  std::vector<mdf::ETag> properties(count);
  for (size_t i = 0; i < count; ++i) properties[i] = *pProperty[i];
  metadata->CommonProperties(properties);
}
EXPORTFEATUREFUNC(size_t, GetXmlSnippet, char* xml) {
  auto str = metadata->XmlSnippet();
  if (xml) strcpy(xml, str.c_str());
  return str.length();
}
EXPORTFEATUREFUNC(void, SetXmlSnippet, const char* xml) {
  metadata->XmlSnippet(std::string(xml));
}
EXPORTFEATUREFUNC(void, AddCommonProperty, mdf::ETag* tag) {
  metadata->CommonProperty(*tag);
}
#undef EXPORTFEATUREFUNC
#pragma endregion

#pragma region CanMessage
EXPORT(mdf::CanMessage*, CanMessage, Init) { return new mdf::CanMessage; }
#define EXPORTFEATUREFUNC(ReturnType, FuncName, ...) \
  EXPORT(ReturnType, CanMessage, FuncName, mdf::CanMessage* can, ##__VA_ARGS__)
EXPORTFEATUREFUNC(void, UnInit) { delete can; }
EXPORTFEATUREFUNC(uint32_t, GetMessageId) { return can->MessageId(); }
EXPORTFEATUREFUNC(void, SetMessageId, const uint32_t msgId) {
  can->MessageId(msgId);
}
EXPORTFEATUREFUNC(uint32_t, GetCanId) { return can->CanId(); }
EXPORTFEATUREFUNC(bool, GetExtendedId) { return can->ExtendedId(); }
EXPORTFEATUREFUNC(void, SetExtendedId, const bool extendedId) {
  can->ExtendedId(extendedId);
}
EXPORTFEATUREFUNC(uint8_t, GetDlc) { return can->Dlc(); }
EXPORTFEATUREFUNC(void, SetDlc, const uint8_t dlc) { can->Dlc(dlc); }
EXPORTFEATUREFUNC(size_t, GetDataLength) { return can->DataLength(); }
EXPORTFEATUREFUNC(void, SetDataLength, const uint32_t dataLength) {
  can->DataLength(dataLength);
}
EXPORTFEATUREFUNC(size_t, GetDataBytes, uint8_t dataList[]) {
  std::vector<uint8_t> vec = can->DataBytes();
  if (dataList != nullptr) memcpy(dataList, vec.data(), vec.size());
  return vec.size();
}
EXPORTFEATUREFUNC(void, SetDataBytes, const uint8_t* dataList,
                  const size_t size) {
  can->DataBytes(std::vector<uint8_t>(dataList, dataList + size));
}

EXPORTFEATUREFUNC(bool, GetDir) { return can->Dir(); }
EXPORTFEATUREFUNC(void, SetDir, const bool transmit) { can->Dir(transmit); }
EXPORTFEATUREFUNC(bool, GetSrr) { return can->Srr(); }
EXPORTFEATUREFUNC(void, SetSrr, const bool srr) { can->Srr(srr); }
EXPORTFEATUREFUNC(bool, GetEdl) { return can->Edl(); }
EXPORTFEATUREFUNC(void, SetEdl, const bool edl) { can->Edl(edl); }
EXPORTFEATUREFUNC(bool, GetBrs) { return can->Brs(); }
EXPORTFEATUREFUNC(void, SetBrs, const bool brs) { can->Brs(brs); }
EXPORTFEATUREFUNC(bool, GetEsi) { return can->Esi(); }
EXPORTFEATUREFUNC(void, SetEsi, const bool esi) { can->Esi(esi); }
EXPORTFEATUREFUNC(bool, GetRtr) { return can->Rtr(); }
EXPORTFEATUREFUNC(void, SetRtr, const bool rtr) { can->Rtr(rtr); }
EXPORTFEATUREFUNC(bool, GetWakeUp) { return can->WakeUp(); }
EXPORTFEATUREFUNC(void, SetWakeUp, const bool wakeUp) { can->WakeUp(wakeUp); }
EXPORTFEATUREFUNC(bool, GetSingleWire) { return can->SingleWire(); }
EXPORTFEATUREFUNC(void, SetSingleWire, const bool singleWire) {
  can->SingleWire(singleWire);
}
EXPORTFEATUREFUNC(uint8_t, GetBusChannel) { return can->BusChannel(); }
EXPORTFEATUREFUNC(void, SetBusChannel, const uint8_t channel) {
  can->BusChannel(channel);
}
EXPORTFEATUREFUNC(uint8_t, GetBitPosition) { return can->BitPosition(); }
EXPORTFEATUREFUNC(void, SetBitPosition, const uint8_t position) {
  can->BitPosition(position);
}
EXPORTFEATUREFUNC(CanErrorType, GetErrorType) { return can->ErrorType(); }
EXPORTFEATUREFUNC(void, SetErrorType, const CanErrorType type) {
  can->ErrorType(type);
}
#undef EXPORTFEATUREFUNC
#pragma endregion
}  // namespace MdfLibrary::ExportFunctions
}
#undef EXPORT
