/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once

#include "writer4samplequeue.h"

namespace mdf::detail {

class ConverterSampleQueue : public Writer4SampleQueue {
 public:
  ConverterSampleQueue() = delete;
  ConverterSampleQueue(MdfWriter& writer,
                     IDataGroup& data_group);
  ~ConverterSampleQueue() override;

 protected:
  void TrimQueue() override; ///< Trims the sample queue.
  void SaveQueue(std::unique_lock<std::mutex>& lock) override;
  void CleanQueue(std::unique_lock<std::mutex>& lock) override;

  void SaveQueueCompressed(std::unique_lock<std::mutex>& lock) override;

  /** \brief Save one DZ block from the sample queue. */
  void CleanQueueCompressed(std::unique_lock<std::mutex>& lock,
                            bool finalize) override;
 private:
  // void ConverterThread()
};

}  // namespace mdf::detail


