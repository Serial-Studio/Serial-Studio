/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once

#include "samplequeue.h"

namespace mdf::detail {

class Writer4SampleQueue : public SampleQueue {
 public:
  Writer4SampleQueue() = delete;
  Writer4SampleQueue(MdfWriter& writer,
                     IDataGroup& data_group);
  ~Writer4SampleQueue() override;

 protected:
  uint64_t offset_ = 0;
  void SaveQueue(std::unique_lock<std::mutex>& lock) override;
  void CleanQueue(std::unique_lock<std::mutex>& lock) override;


  /** \brief Calculates number of DZ blocks in the sample queue */
  [[nodiscard]] size_t CalculateNofDzBlocks() const;
  virtual void SaveQueueCompressed(std::unique_lock<std::mutex>& lock);

  /** \brief Save one DZ block from the sample queue. */
  virtual void CleanQueueCompressed(std::unique_lock<std::mutex>& lock, bool finalize);

  void SetLastPosition(std::streambuf& buffer) override;


};

}  // namespace mdf

