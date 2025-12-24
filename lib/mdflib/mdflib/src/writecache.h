/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once

#include <map>
#include <memory>
#include <thread>
#include <atomic>
#include <condition_variable>

#include "mdf/ichannelgroup.h"
#include "mdf/mdfwriter.h"
#include "samplequeue.h"

namespace mdf::detail {



class WriteCache final {
 public:
  WriteCache() = delete;
  explicit WriteCache(MdfWriter& writer);
  ~WriteCache();

  void Init();
  void Exit();

  void RecalculateTimeMaster();

  void Notify() {
    sample_event_.notify_one();
  }

  void SaveSample(const IChannelGroup& channel_group, uint64_t time);
  void SaveSample(const IDataGroup& data_group,
                  const IChannelGroup& group, uint64_t time);

 void SaveCanMessage(const IChannelGroup& channel_group, uint64_t time,
                                const CanMessage& msg);
 void SaveCanMessage(const IDataGroup& data_group,
                     const IChannelGroup& channel_group, uint64_t time,
                     const CanMessage& msg);

 void SaveLinMessage(const IChannelGroup& channel_group, uint64_t time,
                     const LinMessage& msg);
 void SaveLinMessage(const IDataGroup& data_group,
                     const IChannelGroup& channel_group, uint64_t time,
                     const LinMessage& msg);

 void SaveEthMessage(const IChannelGroup& channel_group, uint64_t time,
                     const EthMessage& msg);
 void SaveEthMessage(const IDataGroup& data_group,
                     const IChannelGroup& channel_group, uint64_t time,
                     const EthMessage& msg);

 void SaveMostMessage(const IDataGroup& data_group,
                     const IChannelGroup& channel_group,
                     uint64_t time,
                     const IMostEvent& msg);
 void SaveFlexRayMessage(const IDataGroup& data_group,
                      const IChannelGroup& channel_group,
                      uint64_t time,
                      const IFlexRayEvent& msg);
 private:
  MdfWriter& writer_;

  std::thread work_thread_; ///< Sample queue thread.
  std::atomic_bool stop_thread_ = false; ///< Set to true to stop the thread.
  std::mutex locker_; ///< Mutex for thread-safe handling of the sample queue.
  std::condition_variable sample_event_; ///< Used internally.

  using SampleQueuePtr = std::unique_ptr<SampleQueue>;
  std::map<const IDataGroup*, SampleQueuePtr> cache_;

  void StopWorkThread(); ///< Stops the worker thread
  void WorkThread(); ///< Worker thread function

  [[nodiscard]] SampleQueue* GetSampleQueue(
      const IDataGroup& data_group) const;
  [[nodiscard]] SampleQueue* GetSampleQueue(
      const IChannelGroup& channel_group) const;
};

}  // namespace mdf


