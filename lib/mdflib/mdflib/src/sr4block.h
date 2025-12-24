/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include "datalistblock.h"
#include "mdf/isamplereduction.h"
#include "mdf/ichannelgroup.h"


namespace mdf::detail {
class Sr4Block : public DataListBlock, public ISampleReduction {
 public:
  Sr4Block();
  [[nodiscard]] int64_t Index() const override;
  [[nodiscard]] std::string BlockType() const override;
  void NofSamples(uint64_t nof_samples) override;
  [[nodiscard]] uint64_t NofSamples() const override;

  void Interval(double interval) override;
  [[nodiscard]] double Interval() const override;

  void SyncType(SrSyncType type) override;
  [[nodiscard]] SrSyncType SyncType() const override;

  void Flags(uint8_t flags) override;
  [[nodiscard]] uint8_t Flags() const override;

  [[nodiscard]] const IChannelGroup* ChannelGroup() const override;

  void GetBlockProperty(BlockPropertyList& dest) const override;
  uint64_t Read(std::streambuf& buffer) override;

  void ReadData(std::streambuf& buffer) const;  ///< Reads in SR/Rx data
  void ClearData() override;

 protected:
  void GetChannelValueUint( const IChannel& channel, uint64_t sample,
                            uint64_t array_index, SrValue<uint64_t>& value ) const override;
  void GetChannelValueInt( const IChannel& channel, uint64_t sample,
                            uint64_t array_index, SrValue<int64_t>& value ) const override;
  void GetChannelValueDouble( const IChannel& channel, uint64_t sample,
                              uint64_t array_index, SrValue<double>& value ) const override;
 private:
  uint64_t nof_samples_ = 0;
  double interval_ = 0;
  uint8_t type_ = 0;
  uint8_t flags_ = 0;
  /* reserved 6 byte */

  mutable std::vector<uint8_t> data_list_; ///< Typical RD block data


  template <typename T>
  void GetChannelValueT( const IChannel& channel, uint64_t sample,
                               uint64_t array_index, SrValue<T>& value ) const;
  /** \brief Returns the block size of the associated CG block.
   *
   * Returns the block size in bytes of the associated CG block. It also
   * hides some include files that we don't want in this include file.
   * @return Number of bytes of one RD record.
   */
  uint64_t BlockSize() const;
  uint64_t BlockDataSize() const;
  uint64_t BlockInvalidSize() const;
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
void Sr4Block::GetChannelValueT(const IChannel &channel,
                                uint64_t sample,
                                uint64_t array_index,
                                SrValue<T> &value) const {
  const auto block_size = BlockSize();
  if (data_list_.empty() || block_size == 0) {
    return;
  }

  // Set up a temporary record buffer. It will be used as temporary
  // buffer. It solves the RD vs RV/RI blocks data storage.
  std::vector<uint8_t> record_buffer(static_cast<size_t>(block_size), 0);

  // Check if the data_list_ buffer is format as an RD block
  // or as RV/RI blocks. The main idea is to copy to the
  // temporary buffer, so it looks like an "RD" stored record.
  if (IsRdBlock()) {
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
  } else {
    const auto block_data_size = BlockDataSize();
    const auto block_invalid_size = BlockInvalidSize();
    const auto invalid_index = NofSamples() * 3 * block_data_size;

    if (const auto mean_data_index = static_cast<int>(sample * 3 * block_data_size);
        static_cast<size_t>(mean_data_index + block_data_size) <= data_list_.size()) {
      std::copy_n(data_list_.cbegin() + mean_data_index, block_data_size,
                  record_buffer.begin() );
    }
    if (const auto mean_invalid_index = static_cast<int>(invalid_index + (sample * 3 * block_invalid_size));
        static_cast<size_t>(mean_invalid_index + block_invalid_size) <= data_list_.size()) {
      std::copy_n(data_list_.cbegin() + mean_invalid_index, block_invalid_size,
                  record_buffer.begin() + static_cast<int>(block_data_size));
    }
    value.MeanValid = channel.GetChannelValue(record_buffer, value.MeanValue, array_index);

    if (const auto min_data_index = static_cast<int>((sample * 3 * block_data_size) + block_data_size);
        static_cast<size_t>(min_data_index + block_data_size) <= data_list_.size()) {
      std::copy_n(data_list_.cbegin() + min_data_index, block_data_size,
                  record_buffer.begin() );
    }
    if (const auto min_invalid_index =
        static_cast<int>(invalid_index + (sample * 3 * block_invalid_size) + block_invalid_size);
        static_cast<size_t>(min_invalid_index + block_invalid_size) <= data_list_.size()) {
      std::copy_n(data_list_.cbegin() + min_invalid_index, block_invalid_size,
                  record_buffer.begin() + static_cast<int>(block_data_size) );
    }
    value.MinValid = channel.GetChannelValue(record_buffer, value.MinValue, array_index);

    if (const auto max_data_index =
        static_cast<int>((sample * 3 * block_data_size) + (2 * block_data_size));
        static_cast<size_t>(max_data_index + block_data_size) <= data_list_.size()) {
      std::copy_n(data_list_.cbegin() + max_data_index, block_data_size,
                  record_buffer.begin() );
    }
    if (const auto max_invalid_index =
        static_cast<int>(invalid_index + (sample * 3 * block_invalid_size) + (2 * block_invalid_size));
        static_cast<size_t>(max_invalid_index + block_invalid_size) <= data_list_.size()) {
      std::copy_n(data_list_.cbegin() + max_invalid_index, block_invalid_size,
                  record_buffer.begin() + static_cast<int>(block_data_size) );
    }
    value.MaxValid = channel.GetChannelValue(record_buffer, value.MaxValue, array_index);
  }
}


}  // namespace mdf::detail
