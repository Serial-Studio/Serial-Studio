/*
 * Copyright 2023 Simplxs
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include "MdfFile.h"
#include "CanMessage.h"

using namespace MdfLibrary::ExportFunctions;

namespace MdfLibrary {
class MdfWriter {
 private:
  mdf::MdfWriter* writer;

 public:
  MdfWriter(MdfWriterType type, const char* filename)
      : writer(MdfWriterInit(type, filename)) {
    if (writer == nullptr) throw std::runtime_error("MdfWriter Init failed");
  }
  ~MdfWriter() {
    if (writer == nullptr) return;
    MdfWriterUnInit(writer);
    writer = nullptr;
  }
  MdfWriter(const MdfWriter&) = delete;
  MdfFile GetFile() const { return MdfWriterGetFile(writer); }
  MdfHeader GetHeader() const { return MdfWriterGetHeader(writer); }
  bool IsFileNew() const { return MdfWriterIsFileNew(writer); }
  bool GetCompressData() const { return MdfWriterGetCompressData(writer); }
  void SetCompressData(bool compress) {
    MdfWriterSetCompressData(writer, compress);
  }
  double GetPreTrigTime() const { return MdfWriterGetPreTrigTime(writer); }
  void SetPreTrigTime(double pre_trig_time) {
    MdfWriterSetPreTrigTime(writer, pre_trig_time);
  }
  uint64_t GetStartTime() const { return MdfWriterGetStartTime(writer); }
  uint64_t GetStopTime() const { return MdfWriterGetStopTime(writer); }
  MdfBusType GetBusType() const { return MdfWriterGetBusType(writer); }
  void SetBusType(MdfBusType type) { MdfWriterSetBusType(writer, type); }
  MdfStorageType GetStorageType() const {
    return MdfWriterGetStorageType(writer);
  }
  void SetStorageType(MdfStorageType type) {
    MdfWriterSetStorageType(writer, type);
  }
  uint32_t GetMaxLength() const { return MdfWriterGetMaxLength(writer); }
  void SetMaxLength(uint32_t length) { MdfWriterSetMaxLength(writer, length); }
  bool CreateBusLogConfiguration() { return MdfWriterCreateBusLogConfiguration(writer); }
  MdfDataGroup CreateDataGroup() {
    return MdfDataGroup(MdfWriterCreateDataGroup(writer));
  }
  void SaveSample(MdfChannelGroup group, uint64_t time) {
    MdfWriterSaveSample(writer, group.GetChannelGroup(), time);
  }
  void SaveCanMessage(const MdfChannelGroup group, uint64_t time, const CanMessage& msg) {
    MdfWriterSaveCanMessage(writer, group.GetChannelGroup(), time, msg.GetCanMessage());
  }
  bool InitMeasurement() { return MdfWriterInitMeasurement(writer); }
  void StartMeasurement(uint64_t start_time) {
    MdfWriterStartMeasurement(writer, start_time);
  }
  void StopMeasurement(uint64_t stop_time) {
    MdfWriterStopMeasurement(writer, stop_time);
  }
  bool FinalizeMeasurement() { return MdfWriterFinalizeMeasurement(writer); }
};
}  // namespace MdfLibrary