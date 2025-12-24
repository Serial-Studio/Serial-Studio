/*
 * Copyright 2023 Simplxs
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include "MdfMetaData.h"

using namespace MdfLibrary::ExportFunctions;

namespace MdfLibrary {
class MdfFileHistory {
 private:
  mdf::IFileHistory* history;

 public:
  MdfFileHistory(mdf::IFileHistory* history) : history(history) {
    if (history == nullptr)
      throw std::runtime_error("MdfFileHistory Init failed");
  }
  MdfFileHistory(const mdf::IFileHistory* history)
      : MdfFileHistory(const_cast<mdf::IFileHistory*>(history)) {}
  ~MdfFileHistory() { history = nullptr; }
  int64_t GetIndex() const { return MdfFileHistoryGetIndex(history); }
  uint64_t GetTime() const { return MdfFileHistoryGetTime(history); }
  void SetTime(uint64_t time) { MdfFileHistorySetTime(history, time); }
  const MdfMetaData GetMetaData() const {
    return MdfFileHistoryGetMetaData(history);
  }
  std::string GetDescription() const {
    std::string str(MdfFileHistoryGetDescription(history, nullptr) + 1, '\0');
    str.resize(MdfFileHistoryGetDescription(history, str.data()));
    return str;
  }
  void SetDescription(const char* desc) {
    MdfFileHistorySetDescription(history, desc);
  }
  std::string GetToolName() const {
    std::string str(MdfFileHistoryGetToolName(history, nullptr) + 1, '\0');
    str.resize(MdfFileHistoryGetToolName(history, str.data()));
    return str;
  }
  void SetToolName(const char* name) {
    MdfFileHistorySetToolName(history, name);
  }
  std::string GetToolVendor() const {
    std::string str(MdfFileHistoryGetToolVendor(history, nullptr) + 1, '\0');
    str.resize(MdfFileHistoryGetToolVendor(history, str.data()));
    return str;
  }
  void SetToolVendor(const char* vendor) {
    MdfFileHistorySetToolVendor(history, vendor);
  }
  std::string GetToolVersion() const {
    std::string str(MdfFileHistoryGetToolVersion(history, nullptr) + 1, '\0');
    str.resize(MdfFileHistoryGetToolVersion(history, str.data()));
    return str;
  }
  void SetToolVersion(const char* version) {
    MdfFileHistorySetToolVersion(history, version);
  }
  std::string GetUserName() const {
    std::string str(MdfFileHistoryGetUserName(history, nullptr) + 1, '\0');
    str.resize(MdfFileHistoryGetUserName(history, str.data()));
    return str;
  }
  void SetUserName(const char* user) {
    MdfFileHistorySetUserName(history, user);
  }
};
}  // namespace MdfLibrary
