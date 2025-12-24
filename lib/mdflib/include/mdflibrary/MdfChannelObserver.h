/*
 * Copyright 2023 Simplxs
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include "MdfDataGroup.h"

using namespace MdfLibrary::ExportFunctions;

namespace MdfLibrary {
class MdfChannelObserver {
 private:
  mdf::IChannelObserver* observer;

 public:
  MdfChannelObserver(mdf::IChannelObserver* observer) : observer(observer) {
    if (observer == nullptr)
      throw std::runtime_error("MdfChannelObserver Init failed");
  }
  MdfChannelObserver(const mdf::IChannelObserver* observer)
      : MdfChannelObserver(const_cast<mdf::IChannelObserver*>(observer)) {}
  MdfChannelObserver(MdfDataGroup data_group, MdfChannelGroup channel_group,
                     MdfChannel channel)
      : MdfChannelObserver(MdfChannelObserverCreate(
            data_group.GetDataGroup(), channel_group.GetChannelGroup(),
            channel.GetChannel())) {}
  MdfChannelObserver(MdfDataGroup data_group, const char* channel_name)
      : MdfChannelObserver(MdfChannelObserverCreateByChannelName(
            data_group.GetDataGroup(), channel_name)) {}
  ~MdfChannelObserver() {
    if (observer == nullptr) return;
    MdfChannelObserverUnInit(observer);
    observer = nullptr;
  }
  MdfChannelObserver(const MdfChannelObserver&) = delete;
  MdfChannelObserver(MdfChannelObserver&& channel_observer) {
    observer = channel_observer.observer;
    channel_observer.observer = nullptr;
  }
  int64_t GetNofSamples() const {
    return MdfChannelObserverGetNofSamples(observer);
  }
  std::string GetName() const {
    std::string str(MdfChannelObserverGetName(observer, nullptr) + 1, '\0');
    str.resize(MdfChannelObserverGetName(observer, str.data()));
    return str;
  }
  std::string GetUnit() const {
    std::string str(MdfChannelObserverGetUnit(observer, nullptr) + 1, '\0');
    str.resize(MdfChannelObserverGetUnit(observer, str.data()));
    return str;
  }
  const MdfChannel GetChannel() const {
    return MdfChannel(MdfChannelObserverGetChannel(observer));
  }
  bool IsMaster() const { return MdfChannelObserverIsMaster(observer); }
  bool GetChannelValue(uint64_t sample, int64_t& value) const {
    return MdfChannelObserverGetChannelValueAsSigned(observer, sample, value);
  }
  bool GetChannelValue(uint64_t sample, uint64_t& value) const {
    return MdfChannelObserverGetChannelValueAsUnSigned(observer, sample, value);
  }
  bool GetChannelValue(uint64_t sample, double& value) const {
    return MdfChannelObserverGetChannelValueAsFloat(observer, sample, value);
  }
  bool GetChannelValue(uint64_t sample, std::string& value) const {
    size_t size;
    MdfChannelObserverGetChannelValueAsString(observer, sample, nullptr, size);
    value.resize(size);
    bool valid =
        MdfChannelObserverGetChannelValueAsString(observer, sample, value.data(), size);
    return valid;
  }
  bool GetChannelValue(uint64_t sample, std::vector<uint8_t>& value) const {
    size_t size;
    MdfChannelObserverGetChannelValueAsArray(observer, sample, nullptr, size);
    value.resize(size);
    return MdfChannelObserverGetChannelValueAsArray(observer, sample,
                                                    value.data(), size);
  }
  bool GetEngValue(uint64_t sample, int64_t& value) const {
    return MdfChannelObserverGetEngValueAsSigned(observer, sample, value);
  }
  bool GetEngValue(uint64_t sample, uint64_t& value) const {
    return MdfChannelObserverGetEngValueAsUnSigned(observer, sample, value);
  }
  bool GetEngValue(uint64_t sample, double& value) const {
    return MdfChannelObserverGetEngValueAsFloat(observer, sample, value);
  }
  bool GetEngValue(uint64_t sample, std::string& value) const {
    size_t size;
    MdfChannelObserverGetEngValueAsString(observer, sample, nullptr, size);
    value.resize(size);
    bool valid =
        MdfChannelObserverGetEngValueAsString(observer, sample, value.data(), size);
    return valid;
  }
  bool GetEngValue(uint64_t sample, std::vector<uint8_t>& value) const {
    size_t size;
    MdfChannelObserverGetEngValueAsArray(observer, sample, nullptr, size);
    value.resize(size);
    return MdfChannelObserverGetEngValueAsArray(observer, sample, value.data(),
                                                size);
  }
};

std::vector<MdfChannelObserver> MdfCreateChannelObserverForChannelGroup(
    MdfDataGroup data_group, MdfChannelGroup channel_group) {
  size_t count =
      MdfChannelGroupGetChannels(channel_group.GetChannelGroup(), nullptr);
  if (count <= 0) return {};
  auto pObservers = new mdf::IChannelObserver*[count];
  MdfChannelObserverCreateForChannelGroup(
      data_group.GetDataGroup(), channel_group.GetChannelGroup(), pObservers);
  std::vector<MdfChannelObserver> observers;
  for (size_t i = 0; i < count; i++) observers.push_back(pObservers[i]);
  delete[] pObservers;
  return observers;
}

}  // namespace MdfLibrary