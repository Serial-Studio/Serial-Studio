/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include "mdf/idatagroup.h"

#include <algorithm>
#include <execution>

#include "mdf/ichannel.h"
#include "mdf/ichannelgroup.h"
#include "mdf/ichannelobserver.h"


namespace mdf {

void IDataGroup::AttachSampleObserver(ISampleObserver *observer) const {
  observer_list_.emplace_back(observer);
}

void IDataGroup::DetachSampleObserver(const ISampleObserver *observer) const {
  if (observer_list_.empty()) return;
  for (auto itr = observer_list_.begin(); itr != observer_list_.end();
       /* No ++itr here */) {
    if (*itr == observer) {
      itr = observer_list_.erase(itr);
    } else {
      ++itr;
    }
  }
}

void IDataGroup::DetachAllSampleObservers() const {
  std::for_each(observer_list_.begin(), observer_list_.end(),
    [] (auto *observer) {
      if (observer != nullptr) {
        observer->DataGroup(nullptr);
      }
    });
  observer_list_.clear();
}

bool IDataGroup::NotifySampleObservers(uint64_t sample, uint64_t record_id,
    const std::vector<uint8_t> &record) const {
  if (fast_observer_list_.empty()) {
    return false; // No meaning to continue reading
  }

  if ( fast_observer_list_.size() == 1) {
    for (ISampleObserver* observer : observer_list_) {
      if (observer == nullptr) {
        continue;
      }
      const bool continue_reading = observer->OnSample(sample, record_id, record);
      if (!continue_reading) {
        return false;
      }
    }
    return true;
  }

  auto itr = fast_observer_list_.find(record_id);
  if (itr == fast_observer_list_.end()) {
    return true;
  }
  auto& observer_list = itr->second;
  for (ISampleObserver* observer : observer_list ) {
    if (observer != nullptr) {
      const bool continue_reading = observer->OnSample(sample, record_id, record);
      if (!continue_reading) {
        return false;
      }
    }
  };

  return true;
}

void IDataGroup::ClearData() {
  auto list = ChannelGroups();
  std::for_each(list.begin(),list.end(),
                        [](auto *cg) { cg->ClearData(); });
}

IMetaData *IDataGroup::CreateMetaData() { return nullptr; }

IMetaData *IDataGroup::MetaData() const { return nullptr; }

void IDataGroup::Description(const std::string &) {}

std::string IDataGroup::Description() const { return {}; }

void IDataGroup::RecordIdSize(uint8_t id_size) {}

uint8_t IDataGroup::RecordIdSize() const { return 0; }


bool IDataGroup::IsEmpty() const {
  // Check if any samples have been stored.
  const auto cg_list = ChannelGroups();
  return std::all_of(cg_list.cbegin(), cg_list.cend(), [] (const auto* group) {
    return group != nullptr &&  group->NofSamples() == 0;
  });
}

IChannelGroup *IDataGroup::CreateChannelGroup(const std::string_view &name) {
  auto cg_list = ChannelGroups();
  auto itr = std::find_if(cg_list.begin(), cg_list.end(),
                          [&] (const auto* group) {
    return group != nullptr && group->Name() == name && !name.empty();
  });
  if (itr != cg_list.end()) {
    return *itr;
  }
  auto* new_group = CreateChannelGroup();
  if (new_group != nullptr) {
    new_group->Name(name.data());
  }
  return new_group;
}

IChannelGroup *IDataGroup::GetChannelGroup(const std::string_view &name) const {
  if (name.empty()) {
    return nullptr;
  }
  const auto cg_list = ChannelGroups();
  // First search on full name and if this fails, try to find a sub-string;
  auto itr = std::find_if(cg_list.begin(), cg_list.end(),
      [&] (const auto* group) {
    return group != nullptr && group->Name() == name;
  });
  if (itr == cg_list.cend() && name.size() > 3) {
    itr = std::find_if(cg_list.begin(), cg_list.end(),
        [&] (const auto* group) {
             return group != nullptr && group->Name().find(name) != std::string::npos;
        });
  }
  return itr != cg_list.cend() ? *itr : nullptr;
}

IChannelGroup *IDataGroup::GetChannelGroup(uint64_t record_id) const {
  auto cg_list = ChannelGroups();
  auto itr = std::find_if(cg_list.begin(), cg_list.end(),
                [&] (const auto* group) {
                  return group != nullptr && group->RecordId() == record_id;
                });
  return itr != cg_list.end() ? *itr : nullptr;
}

bool IDataGroup::IsSubscribingOnRecord(uint64_t record_id) const {
  return std::any_of(observer_list_.cbegin(), observer_list_.cend(),
                     [&] (const auto* observer) {
    return observer != nullptr && observer->IsRecordIdNeeded(record_id);

  });
}

bool IDataGroup::IsSubscribingOnChannel(const IChannel& channel) const {
  return std::any_of(observer_list_.cbegin(), observer_list_.cend(),
              [&] ( const auto& observer) {
    if (!observer) {
      return false;
    }
    const auto* channel_observer = dynamic_cast<const IChannelObserver*>(observer);
    if (channel_observer == nullptr) {
      // Some type of OnSample observer. Check if the record ID is un use.
      if (IsSubscribingOnRecord(channel.RecordId())) {
        return true;
      }
    } else if (channel_observer->Channel().Index() == channel.Index()) {
      return true;
    }
    return false;
  });
}

bool IDataGroup::IsSubscribingOnChannelVlsd(const IChannel& channel) const {
  return std::any_of(observer_list_.cbegin(), observer_list_.cend(),
   [&] ( const auto& observer) {
     if (!observer) {
       return false;
     }
     const auto* channel_observer = dynamic_cast<const IChannelObserver*>(observer);
     if (channel_observer == nullptr) {
       // Some type of OnSample observer. Check if the record ID is un use.
       if (IsSubscribingOnRecord(channel.RecordId())) {
         return true;
       }
     } else if (channel_observer->Channel().Index() == channel.Index()) {
       return channel_observer->ReadVlsdData();
     }
     return false;
   });
}

void IDataGroup::InitFastObserverList() {
  fast_observer_list_.clear();
  const auto cg_list = ChannelGroups();
  for (const auto* channel_group : cg_list ) {
    if (channel_group == nullptr) {
      continue;
    }
    uint64_t record_id = channel_group->RecordId();
    std::vector<ISampleObserver*> temp_list;
    for (auto* observer : observer_list_) {
      if (observer == nullptr) {
        continue;
      }
      if (observer->IsRecordIdNeeded(record_id)) {
        temp_list.emplace_back(observer);
      }
    }
    if (!temp_list.empty()) {
      fast_observer_list_.emplace(record_id, std::move(temp_list));
    }
  }
}

void IDataGroup::SetDgComment(const DgComment &dg_comment) {
  if (IMetaData* meta_data = CreateMetaData();
      meta_data != nullptr ) {
    meta_data->XmlSnippet(dg_comment.ToXml());
  }
}

void IDataGroup::GetDgComment(DgComment &dg_comment) const {
  if (const IMetaData* meta_data = MetaData();
      meta_data != nullptr) {
    dg_comment.FromXml(meta_data->XmlSnippet());
  }
}

}  // namespace mdf
