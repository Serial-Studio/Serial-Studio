/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include "writecache.h"

#include <chrono>

#include "mdf/mdflogstream.h"
#include "mdf/mostmessage.h"
#include "mdf/flexraymessage.h"
#include "writer4samplequeue.h"
#include "convertersamplequeue.h"


using namespace std::chrono_literals;

namespace mdf::detail {
WriteCache::WriteCache(MdfWriter& writer)
: writer_(writer) {

}

WriteCache::~WriteCache() {
  StopWorkThread();
  cache_.clear();
}

void WriteCache::Init() {
  StopWorkThread();
  cache_.clear();
  writer_.State( WriteState::Init);

  const auto* header = writer_.Header();
  if (header == nullptr) {
    MDF_ERROR() << "No MDF header. Invalid use of function.";
    return;
  }

  std::vector<const IDataGroup*> active_list;
  for (IDataGroup* data_group : header->DataGroups()) {
    if (data_group == nullptr) {
      continue;
    }
    for (IChannelGroup* channel_group : data_group->ChannelGroups()) {
      if (channel_group == nullptr ||
          (channel_group->Flags() & CgFlag::VlsdChannel) != 0 ||
          channel_group->NofSamples() > 0 ) {
        continue;
      }

      switch( writer_.TypeOfWriter()) {
        case MdfWriterType::MdfConverter:
          cache_.emplace(data_group,
                         std::make_unique<ConverterSampleQueue>(writer_, *data_group));
          break;

        case MdfWriterType::MdfBusLogger:
        case MdfWriterType::Mdf4Basic:
          cache_.emplace(data_group,
                         std::make_unique<Writer4SampleQueue>(writer_, *data_group));
          break;

        case MdfWriterType::Mdf3Basic:
        default:
          cache_.emplace(data_group,
                         std::make_unique<SampleQueue>(writer_, *data_group));
          break;
      }
      active_list.emplace_back();
      break; // No need to check more CG groups
    }
  }

  for (auto& [data_group1, sample_queue1] : cache_) {
    if (sample_queue1) {
      sample_queue1->NofDgBlocks(active_list.size());
    }
  }

  // Start the working thread that handles the samples
  writer_.State(WriteState::Init);  // Waits for new samples
  work_thread_ = std::thread(&WriteCache::WorkThread, this);

}

void WriteCache::Exit() {
  StopWorkThread();
  cache_.clear();
}

void WriteCache::StopWorkThread() {
  stop_thread_ = true;
  if (work_thread_.joinable()) {
    sample_event_.notify_one();
    work_thread_.join();
  }
  stop_thread_ = false;
}

void WriteCache::WorkThread() {
  do {
    std::unique_lock lock(locker_);
    sample_event_.wait_for(lock, 10s,
                           [&]() -> bool { return stop_thread_.load(); });
    if (stop_thread_) {
      break;
    }
    for (auto& [channel_group, sample_queue] : cache_) {
      if (!sample_queue) {
        continue;
      }

      switch (writer_.State()) {
        case WriteState::Init: {
          sample_queue->TrimQueue();  // Purge the queue using pre-trig time
            break;
        }
        case WriteState::StartMeas: {
          if (writer_.IsSavePeriodic()) {
            sample_queue->SaveQueue(lock);  // Save the contents of the queue to file
          }
          break;
        }

        case WriteState::StopMeas: {
          sample_queue->CleanQueue(lock);
          break;
        }

        default:
          sample_queue->Reset();
          break;
      }
    }
  } while (!stop_thread_);
  {
    std::unique_lock lock(locker_);
    for (auto& [channel_group, sample_queue] : cache_) {
      if (sample_queue) {
        sample_queue->CleanQueue(lock);
      }
    }
  }
}

void WriteCache::RecalculateTimeMaster() {
  for (auto& [data_group, sample_queue] : cache_) {
    if (sample_queue) {
      std::lock_guard lock(locker_);
      sample_queue->RecalculateTimeMaster();
    }
  }
}

void WriteCache::SaveSample(const IChannelGroup& channel_group, uint64_t time) {
  if (SampleQueue* queue = GetSampleQueue(channel_group);
      queue != nullptr) {
    std::lock_guard lock(locker_);
    queue->SaveSample(channel_group, time);
  }
}

void WriteCache::SaveSample(const IDataGroup& data_group,
                            const IChannelGroup& channel_group,
                            uint64_t time) {
  if (SampleQueue* queue = GetSampleQueue(data_group);
      queue != nullptr) {
    std::lock_guard lock(locker_);
    queue->SaveSample(channel_group, time);
  }
}

void WriteCache::SaveCanMessage(const IChannelGroup& channel_group, uint64_t time,
                    const CanMessage& msg) {
  msg.DataGroup(channel_group.DataGroup());
  msg.ChannelGroup(&channel_group);
  if (SampleQueue* queue = GetSampleQueue(channel_group);
      queue != nullptr) {
    std::lock_guard lock(locker_);
    queue->SaveCanMessage(channel_group, time, msg);
  }
}

void WriteCache::SaveCanMessage(const IDataGroup& data_group,
                                const IChannelGroup& channel_group, uint64_t time,
                                const CanMessage& msg) {
  msg.DataGroup(&data_group);
  msg.ChannelGroup(&channel_group);
  if (SampleQueue* queue = GetSampleQueue(data_group);
      queue != nullptr) {
    std::lock_guard lock(locker_);
    queue->SaveCanMessage(channel_group, time, msg);
  }
}

void WriteCache::SaveLinMessage(const IChannelGroup& channel_group, uint64_t time,
                                const LinMessage& msg) {
  msg.DataGroup(channel_group.DataGroup());
  msg.ChannelGroup(&channel_group);
  if (SampleQueue* queue = GetSampleQueue(channel_group);
      queue != nullptr) {
    std::lock_guard lock(locker_);
    queue->SaveLinMessage(channel_group, time, msg);
  }
}



void WriteCache::SaveLinMessage(const IDataGroup& data_group,
                                const IChannelGroup& channel_group, uint64_t time,
                                const LinMessage& msg) {
  msg.DataGroup(&data_group);
  msg.ChannelGroup(&channel_group);
  if (SampleQueue* queue = GetSampleQueue(data_group);
      queue != nullptr) {
    std::lock_guard lock(locker_);
    queue->SaveLinMessage(channel_group, time, msg);
  }
}

void WriteCache::SaveEthMessage(const IChannelGroup& channel_group, uint64_t time,
                                const EthMessage& msg) {
  msg.DataGroup(channel_group.DataGroup());
  msg.ChannelGroup(&channel_group);
  if (SampleQueue* queue = GetSampleQueue(channel_group);
      queue != nullptr) {
    std::lock_guard lock(locker_);
    queue->SaveEthMessage(channel_group, time, msg);
  }
}

void WriteCache::SaveEthMessage(const IDataGroup& data_group,
                                const IChannelGroup& channel_group, uint64_t time,
                                const EthMessage& msg) {
  msg.DataGroup(&data_group);
  msg.ChannelGroup(&channel_group);
  if (SampleQueue* queue = GetSampleQueue(data_group);
      queue != nullptr) {
    std::lock_guard lock(locker_);
    queue->SaveEthMessage(channel_group, time, msg);
  }
}

void WriteCache::SaveMostMessage(const IDataGroup& data_group,
                                const IChannelGroup& channel_group, uint64_t time,
                                const IMostEvent& msg) {
  msg.DataGroup( &data_group);
  msg.ChannelGroup(&channel_group);

  if (SampleQueue* queue = GetSampleQueue(data_group);
      queue != nullptr) {
    std::lock_guard lock(locker_);
    queue->SaveMostMessage(channel_group, time, msg);
  }
}

void WriteCache::SaveFlexRayMessage(const IDataGroup& data_group,
                                 const IChannelGroup& channel_group, uint64_t time,
                                 const IFlexRayEvent& msg) {
  msg.DataGroup( &data_group);
  msg.ChannelGroup(&channel_group);

  if (SampleQueue* queue = GetSampleQueue(data_group);
      queue != nullptr) {
    std::lock_guard lock(locker_);
    queue->SaveFlexRayMessage(channel_group, time, msg);
  }
}
SampleQueue* WriteCache::GetSampleQueue(
    const IDataGroup& data_group) const {
  if (auto itr = cache_.find(&data_group);
      itr != cache_.end()) {
    return itr->second.get();
  }
  return nullptr;
}

SampleQueue* WriteCache::GetSampleQueue(
    const IChannelGroup& channel_group) const {
  if (cache_.size() > 1) {
    if (const auto* data_group = channel_group.DataGroup();
        data_group != nullptr ) {
      if (auto itr = cache_.find(data_group);
          itr != cache_.end()) {
        return itr->second.get();
      }
    }
  } else if (cache_.size() == 1) {
    auto itr = cache_.begin();
    return itr->second.get();
  }
  return nullptr;
}
} // namespace mdf::detail