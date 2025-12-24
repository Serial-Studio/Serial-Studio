/*
 * Copyright 2023 Simplxs
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include "MdfChannelGroup.h"

using namespace MdfLibrary::ExportFunctions;

namespace MdfLibrary {
class MdfDataGroup {
 private:
  mdf::IDataGroup* group;

 public:
  MdfDataGroup(mdf::IDataGroup* group) : group(group) {
    if (group == nullptr) throw std::runtime_error("MdfDataGroup Init failed");
  }
  MdfDataGroup(const mdf::IDataGroup* group)
      : MdfDataGroup(const_cast<mdf::IDataGroup*>(group)) {}
  ~MdfDataGroup() { group = nullptr; }
  mdf::IDataGroup* GetDataGroup() const { return group; }
  int64_t GetIndex() const { return MdfDataGroupGetIndex(group); }
  std::string GetDescription() const {
    std::string str(MdfDataGroupGetDescription(group, nullptr) + 1, '\0');
    str.resize(MdfDataGroupGetDescription(group, str.data()));
    return str;
  }
  uint8_t GetRecordIdSize() const { return MdfDataGroupGetRecordIdSize(group); }
  const MdfMetaData GetMetaData() const {
    return MdfDataGroupGetMetaData(group);
  }
  MdfChannelGroup GetChannelGroup(const std::string name) const {
    return MdfDataGroupGetChannelGroupByName(group, name.c_str());
  }
  MdfChannelGroup GetChannelGroup(uint64_t record_id) const {
    return MdfDataGroupGetChannelGroupByRecordId(group, record_id);
  }
  std::vector<MdfChannelGroup> GetChannelGroups() const {
    size_t count = MdfDataGroupGetChannelGroups(group, nullptr);
    if (count <= 0) return {};
    auto pGroups = new mdf::IChannelGroup*[count];
    MdfDataGroupGetChannelGroups(group, pGroups);
    std::vector<MdfChannelGroup> groups;
    for (size_t i = 0; i < count; i++)
      groups.push_back(MdfChannelGroup(pGroups[i]));
    delete[] pGroups;
    return groups;
  }
  bool IsRead() { return MdfDataGroupIsRead(group); }
  MdfMetaData CreateMetaData() { return MdfDataGroupCreateMetaData(group); }
  MdfChannelGroup CreateChannelGroup() {
    return MdfDataGroupCreateChannelGroup(group);
  }
  const MdfChannelGroup FindParentChannelGroup(MdfChannel channel) {
    return MdfDataGroupFindParentChannelGroup(group, channel.GetChannel());
  }
};
}  // namespace MdfLibrary