/*
* Copyright 2024 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once

#include <cstdint>
#include <vector>

#include "mdf/samplerecord.h"

namespace mdf {

class MdfWriter;
class IDataGroup;
class IChannelGroup;

enum class LinMessageType : int {
  LIN_Frame,
  LIN_WakeUp,
  LIN_ChecksumError,
  LIN_TransmissionError,
  LIN_SyncError,
  LIN_ReceiveError,
  LIN_Spike,
  LIN_LongDominantSignal
};

enum class LinChecksumModel : int {
  Unknown = -1,
  Classic = 0,
  Enhanced = 1
};

enum class LinTypeOfLongDominantSignal : int {
  FirstDetection = 0,
  CyclicReport = 1,
  EndOfDetection = 2
};

class LinMessage {
 public:
  LinMessage() = default;

  [[deprecated("Use the default LinMessage() constructor instead.")]]
  explicit LinMessage(const MdfWriter&);

  void MessageType(LinMessageType type) const { message_type_ = type; }
  [[nodiscard]] LinMessageType MessageType() const { return message_type_; }

  void BusChannel(uint8_t channel);
  [[nodiscard]] uint8_t BusChannel() const;

  void LinId(uint8_t id);
  [[nodiscard]] uint8_t LinId() const;

  void Dir(bool transmit);
  [[nodiscard]] bool Dir() const;

  void ReceivedDataByteCount(uint8_t nof_bytes);
  [[nodiscard]] uint8_t ReceivedDataByteCount() const;

  void DataLength(uint8_t nof_bytes);
  [[nodiscard]] uint8_t DataLength() const;

  void DataBytes(std::vector<uint8_t> data_bytes);

  [[nodiscard]] const std::vector<uint8_t>& DataBytes() const {
    return data_bytes_;
  }

  void Checksum(uint8_t crc) { crc_ = crc; }
  [[nodiscard]] uint8_t Checksum() const { return crc_; }

  void ChecksumModel(LinChecksumModel model);
  [[nodiscard]] LinChecksumModel ChecksumModel() const;

  void StartOfFrame(uint64_t ns1970) { sof_ = ns1970; }
  [[nodiscard]] uint64_t StartOfFrame() const { return sof_; }

  void Baudrate(float baudrate) {baudrate_ = baudrate; }
  [[nodiscard]] float Baudrate() const { return baudrate_; }

  void ResponseBaudrate(float baudrate) {response_baudrate_ = baudrate; }
  [[nodiscard]] float ResponseBaudrate() const { return response_baudrate_; }

  void BreakLength(uint32_t break_length) { break_length_ = break_length; }
  [[nodiscard]] uint32_t BreakLength() const { return break_length_; }

  void DelimiterBreakLength(uint32_t break_length) {
    delimiter_break_length_ = break_length;
  }
  [[nodiscard]] uint32_t DelimiterBreakLength() const {
    return delimiter_break_length_;
  }
  void ExpectedDataByteCount(uint8_t nof_bytes);
  [[nodiscard]] uint8_t ExpectedDataByteCount() const;

  void TypeOfLongDominantSignal(LinTypeOfLongDominantSignal type);
  [[nodiscard]] LinTypeOfLongDominantSignal TypeOfLongDominantSignal() const;

  void TotalSignalLength(uint32_t total_signal_length) {
    total_signal_length_ = total_signal_length;
  }
  [[nodiscard]] uint32_t TotalSignalLength() const {
    return total_signal_length_;
  }

  void DataGroup( const IDataGroup* data_group ) const {
    data_group_ = data_group;
  }

  void ChannelGroup( const IChannelGroup* channel_group ) const {
    channel_group_ = channel_group;
  }

  void Reset();

  void ToRaw(LinMessageType msg_type, SampleRecord& sample) const;

 private:

  /** \brief Bus channel and checksum model.
   *
   * The lower 6 bits are the channel while
   * the 2 high bits are used as a 2 bit integer.
   */
  uint8_t bus_channel_ = 0;

  /** \brief LIN ID and direction
   *
   * The LIN ID is the lower 6 bits while bit 7 is set if
   * transmitting (Tx).
   */
  uint8_t lin_id_ = 0;

  /** \brief Various data length
   *
   * The received number of byte (low 4 bits).
   * The data length (high 4 bits)
   */
  uint8_t data_length_ = 0;

  /** \brief Checksum 8 bits.
   *
   * The checksum byte. Note that this byte not is
   * mandatory.
   */
  uint8_t crc_ = 0;

  /** \brief Start of frame (ns since 1970)
   *
   * Start of frame (SOF) as ns since 1970.
   * Note that these 8 bytes are not mandatory..
   */
  uint64_t sof_ = 0;

  LinChecksumModel checksum_model_ = LinChecksumModel::Unknown;

  /** \brief Baud rate (bits/s)
   *
   * 4 byte float baud rate. Note that these bytes are not mandatory.
   */
  float baudrate_ = 0.0;

  /** \brief Response baud rate (bits/s)
   *
   * 4 byte float baud rate. Note that these bytes are not mandatory.
   */
  float response_baudrate_ = 0.0;

  /** \brief Break length in ns
   *
   * Break length in ns. Note these bytes are not mandatory.
   */
  uint32_t break_length_ = 0;

  /** \brief Delimiter break length in ns
   *
   * Delimiter break length in ns. Note these bytes are not mandatory.
   */
  uint32_t delimiter_break_length_ = 0;

  /** \brief Data bytes
   *
   * The data bytes are stored in a byte vector.
   *
   */
  std::vector<uint8_t> data_bytes_;

  /** \brief Expected number of data bytes or type of long dominant signal.
   *
   * Number of expected data bytes is only needed when storing a
   * LIN Receive Error Frame. The Long Dominant Signal Frame is rarely used.
   */
  uint8_t spare_ = 0;

  uint32_t total_signal_length_ = 0;

  mutable LinMessageType message_type_ = LinMessageType::LIN_Frame;
  mutable const IDataGroup* data_group_ = nullptr;
  mutable const IChannelGroup* channel_group_ = nullptr;

  void MakeDataFrame(SampleRecord& sample) const;
  void MakeWakeUp(SampleRecord& sample) const;
  void MakeChecksumError(SampleRecord& sample) const;
  void MakeTransmissionError(SampleRecord& sample) const;
  void MakeSyncError(SampleRecord& sample) const;
  void MakeReceiveError(SampleRecord& sample) const;
  void MakeSpike(SampleRecord& sample) const;
  void MakeLongDominantSignal(SampleRecord& sample) const;
};

}  // namespace mdf

