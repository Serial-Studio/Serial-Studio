/*
* Copyright 2025 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <cstdint>
#include <functional>
#include <vector>
#include <string_view>
#include <string>

#include "mdf/canmessage.h"
#include "mdf/isampleobserver.h"

namespace mdf {

class CanBusObserver : public ISampleObserver {
public:
  CanBusObserver() = delete;
  CanBusObserver(const IDataGroup& data_group,
                 const IChannelGroup& channel_group);

  std::string Name() const;

  uint64_t NofSamples() const { return nof_samples_; }

  CanMessage* GetCanMessage(uint64_t sample);

  std::function<bool(uint64_t sample, const CanMessage& msg)> OnCanMessage;
protected:
  bool OnSample(uint64_t sample, uint64_t record_id,
                        const std::vector<uint8_t>& record) override;
private:
  struct CanSample {
    uint64_t sample = 0;
    std::vector<uint8_t> record;
    std::vector<uint8_t> vlsd_data;
    void Reset() {
      sample = 0;
      record.clear();
      record.shrink_to_fit();
      vlsd_data.clear();
      vlsd_data.shrink_to_fit();
    }
  };

  int64_t current_sample_index_ = -1;
  uint64_t nof_samples_ = 0;
  CanSample last_sample_;
  std::vector<std::unique_ptr<CanMessage>> sample_buffer_;
  bool new_sample_ = false;
  std::string base_name_;

  MessageType message_type_ = MessageType::CAN_DataFrame;
  const IChannelGroup& channel_group_;
  const IChannel* time_channel_ = nullptr; ///< Time channel
  const IChannel* bus_channel_ = nullptr; ///< Bus channel number
  const IChannel* ide_channel_ = nullptr; ///< Extended ID bit
  const IChannel* id_channel_ = nullptr; ///< Msg/CAN ID
  const IChannel* dlc_channel_ = nullptr; ///< Data Length Code
  const IChannel* length_channel_ = nullptr; ///< Data Length
  const IChannel* data_channel_ = nullptr; ///< Data bytes
  const IChannel* dir_channel_ = nullptr; ///< Direction (bool/enumerate)
  const IChannel* crc_channel_ = nullptr; ///< CRC (<= 21 bit)
  const IChannel* srr_channel_ = nullptr; ///< SRR bit
  const IChannel* edl_channel_ = nullptr; ///< EDL bit
  const IChannel* brs_channel_ = nullptr; ///< BRS bit
  const IChannel* esi_channel_ = nullptr; ///< ESI bit
  const IChannel* r0_channel_ = nullptr; ///< R0 bit
  const IChannel* r1_channel_ = nullptr; ///< R1 bit
  const IChannel* wake_up_channel_ = nullptr; ///< Wake Up bit
  const IChannel* single_wire_channel_ = nullptr; ///< Single Wire bit
  const IChannel* frame_duration_channel_ = nullptr; ///< Frame duration (ns)
  const IChannel* bit_position_channel_ = nullptr; ///< Bit position (error msg)
  const IChannel* error_type_channel_ = nullptr; ///< Error Type enumeration

  void FindVLsdRecord(const IChannelGroup& channel_group);

  void HandleCallback(uint64_t record_id,
    const std::vector<uint8_t>& record);

  void HandleSample(uint64_t record_id,
    const std::vector<uint8_t>& record);

  bool CheckSampleBufferSize();
  void ClearSampleBuffer();
  bool FireCallback();
  const IChannel* GetChannel(const std::string_view& sub_name) const;
  void ParseCanMessage(CanMessage& msg) const;
};

} // mdf


