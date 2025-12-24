/*
 * Copyright 2023 Simplxs
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include "MdfFile.h"

using namespace MdfLibrary::ExportFunctions;

namespace MdfLibrary {
class MdfReader {
 private:
  mdf::MdfReader* reader;

 public:
  MdfReader(const char* filename) : reader(MdfReaderInit(filename)) {
    if (reader == nullptr) throw std::runtime_error("MdfReader Init failed");
  }
  ~MdfReader() {
    if (reader == nullptr) return;
    MdfReaderUnInit(reader);
    reader = nullptr;
  }
  MdfReader(const MdfReader&) = delete;
  int64_t GetIndex() const { return MdfReaderGetIndex(reader); }
  bool IsOk() { return MdfReaderIsOk(reader); }
  const MdfFile GetFile() const { return MdfFile(MdfReaderGetFile(reader)); }
  MdfHeader GetHeader() const { return MdfHeader(MdfReaderGetHeader(reader)); }
  const MdfDataGroup GetDataGroup(size_t index) {
    return MdfDataGroup(MdfReaderGetDataGroup(reader, index));
  }
  bool Open() { return MdfReaderOpen(reader); }
  void Close() { MdfReaderClose(reader); }
  bool ReadHeader() { return MdfReaderReadHeader(reader); }
  bool ReadMeasurementInfo() { return MdfReaderReadMeasurementInfo(reader); }
  bool ReadEverythingButData() {
    return MdfReaderReadEverythingButData(reader);
  }
  bool ReadData(MdfDataGroup group) {
    return MdfReaderReadData(reader, group.GetDataGroup());
  }
};
}  // namespace MdfLibrary