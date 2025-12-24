/*
 * Copyright 2023 Simplxs
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include "MdfMetaData.h"

using namespace MdfLibrary::ExportFunctions;

namespace MdfLibrary {
class MdfSourceInformation {
 private:
  mdf::ISourceInformation* sourceInformation;

 public:
  MdfSourceInformation(mdf::ISourceInformation* sourceInformation)
      : sourceInformation(sourceInformation) {
    if (sourceInformation == nullptr)
      throw std::runtime_error("MdfSourceInformation Init failed");
  }
  MdfSourceInformation(const mdf::ISourceInformation* sourceInformation)
      : MdfSourceInformation(
            const_cast<mdf::ISourceInformation*>(sourceInformation)) {}
  ~MdfSourceInformation() { sourceInformation = nullptr; }
  int64_t GetIndex() const {
    return MdfSourceInformationGetIndex(sourceInformation);
  }
  std::string GetName() const {
    std::string str(MdfSourceInformationGetName(sourceInformation, nullptr) + 1, '\0');
    str.resize(MdfSourceInformationGetName(sourceInformation, str.data()));
    return str;
  }
  void SetName(const char* name) {
    MdfSourceInformationSetName(sourceInformation, name);
  }
  std::string GetDescription() const {
    std::string str(MdfSourceInformationGetDescription(sourceInformation, nullptr) + 1, '\0');
    str.resize(MdfSourceInformationGetDescription(sourceInformation, str.data()));
    return str;
  }
  void SetDescription(const char* desc) {
    MdfSourceInformationSetDescription(sourceInformation, desc);
  }
  std::string GetPath() const {
    std::string str(MdfSourceInformationGetPath(sourceInformation, nullptr) + 1, '\0');
    str.resize(MdfSourceInformationGetPath(sourceInformation, str.data()));
    return str;
  }
  void SetPath(const char* path) {
    MdfSourceInformationSetPath(sourceInformation, path);
  }
  SourceType GetType() const {
    return MdfSourceInformationGetType(sourceInformation);
  }
  void SetType(SourceType type) {
    MdfSourceInformationSetType(sourceInformation, type);
  }
  BusType GetBus() const {
    return MdfSourceInformationGetBus(sourceInformation);
  }
  void SetBus(BusType bus) {
    MdfSourceInformationSetBus(sourceInformation, bus);
  }
  uint8_t GetFlags() const {
    return MdfSourceInformationGetFlags(sourceInformation);
  }
  void SetFlags(uint8_t flags) {
    MdfSourceInformationSetFlags(sourceInformation, flags);
  }
  const MdfMetaData GetMetaData() const {
    return MdfMetaData(MdfSourceInformationGetMetaData(sourceInformation));
  }
  MdfMetaData CreateMetaData() {
    return MdfMetaData(MdfSourceInformationCreateMetaData(sourceInformation));
  }
};
}  // namespace MdfLibrary