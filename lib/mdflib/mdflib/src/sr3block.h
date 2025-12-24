/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <cstdio>
#include <memory>
#include <algorithm>

#include "datalistblock.h"
#include "mdf/isamplereduction.h"

namespace mdf::detail {
class Sr3Block : public DataListBlock , public ISampleReduction {
 public:
  [[nodiscard]] int64_t Index() const override;
  [[nodiscard]] std::string BlockType() const override;

  void NofSamples(uint64_t nof_samples) override;
  [[nodiscard]] uint64_t NofSamples() const override;

  void Interval(double interval) override;
  [[nodiscard]] double Interval() const override;

  [[nodiscard]] const IChannelGroup* ChannelGroup() const override;

  uint64_t Read(std::streambuf& buffer) override;
  uint64_t Write(std::streambuf& buffer) override;

  void ReadData(std::streambuf& buffer) const;
  void ClearData() override;

  void GetChannelValueUint( const IChannel& channel, uint64_t sample,
                            uint64_t array_index, SrValue<uint64_t>& value ) const override;
  void GetChannelValueInt( const IChannel& channel, uint64_t sample,
                           uint64_t array_index, SrValue<int64_t>& value ) const override;
  void GetChannelValueDouble( const IChannel& channel, uint64_t sample,
                              uint64_t array_index, SrValue<double>& value ) const override;
 private:
  uint32_t nof_reduced_samples_ = 0;
  double time_interval_ = 0;
  mutable std::vector<uint8_t> data_list_;

  [[nodiscard]] uint16_t BlockSize() const;

  template <typename T>
  void GetChannelValueT( const IChannel& channel, uint64_t sample,
                         uint64_t array_index, SrValue<T>& value ) const;

};

/** \brief Returns the channel value for one sample.
 *
 * @tparam T Type of channel value
 * @param channel Reference to the channel object
 * @param sample Sample number (0...N)
 * @param array_index Array index if the channel is an array.
 * @param value Reference to value mean, min and max structure.
 */
template<typename T>
void Sr3Block::GetChannelValueT(const IChannel &channel,
                                uint64_t sample,
                                uint64_t array_index,
                                SrValue<T> &value) const {
  const auto block_size = BlockSize();
  if (data_list_.empty() || block_size == 0) {
    return;
  }

  // Set up a temporary record buffer. It will be used as temporary
  // buffer.
  std::vector<uint8_t> record_buffer(block_size, 0);

  // Check if the data_list_ buffer is format as an RD block.
  // The main idea is to copy to the
  // temporary buffer, so it looks like an CG record.

    if (const auto mean_index = static_cast<int>(sample * 3 * block_size);
        static_cast<size_t>(mean_index + block_size) <= data_list_.size()) {
      std::copy_n(data_list_.cbegin() + mean_index, block_size, record_buffer.begin() );
    }
    value.MeanValid = channel.GetChannelValue(record_buffer, value.MeanValue, array_index);

    if (const auto min_index = static_cast<int>((sample * 3 * block_size) + block_size);
        static_cast<size_t>(min_index + block_size) <= data_list_.size()) {
      std::copy_n(data_list_.cbegin() + min_index, block_size, record_buffer.begin() );
    }
    value.MinValid = channel.GetChannelValue(record_buffer, value.MinValue, array_index);

    if (const auto max_index = static_cast<int>((sample * 3 * block_size) + (2 * block_size));
        static_cast<size_t>(max_index + block_size) <= data_list_.size()) {
      std::copy_n(data_list_.cbegin() + max_index, block_size, record_buffer.begin() );
    }
    value.MaxValid = channel.GetChannelValue(record_buffer, value.MaxValue, array_index);

}

}  // namespace mdf::detail
