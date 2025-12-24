/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once

#include "mdf/iconfigadapter.h"

namespace mdf {

class MostConfigAdapter : public IConfigAdapter {
 public:
  MostConfigAdapter() = delete;
  explicit MostConfigAdapter( const MdfWriter& writer);

  void CreateConfig(IDataGroup& dg_block) override;

 protected:

 /** \brief Creates the MOST_Message channel group.
  *
  * The MOST_Message channel group consists of 2 top-most channels, time
  * and the a message frame channel. The message frame channel have so-called
  * composite channel a.k.a as sub-channels and below is the channel group
  * layout.
  *
  * Note that the VLSD offset must be the last 8 bytes in the record. This
  * is an internal design requirement as this offset is updated when the sample
  * is written to th the file.
  * <table>
  * <caption id="multi_row">MOST_Message Layout</caption>
  * <tr><th>Byte:Bit Offset</th><th>Channel Description</th></tr>
  * <tr><td>0-7</td><td>Time [s]</td></tr>
  * <tr><td>8</td><td>Bus Channel</td></tr>
  * <tr><td>9:0-1</td><td>Speed (enumerate)</td></tr>
  * <tr><td>9:2</td><td>Transfer Type (enumerate)</td></tr>
  * <tr><td>9:3</td><td>Direction (enumerate)</td></tr>
  * <tr><td>10-11</td><td>Source Address (16-bit)</td></tr>
  * <tr><td>12-13</td><td>Destination Address (16-bit)</td></tr>
  * <tr><td>14-15</td><td>Specified Number of Data Bytes (16-bit)</td></tr>
  * <tr><td>16-17</td><td>Received Number of Data Bytes (16-bit)</td></tr>
  * <tr><td>18-19</td><td>Number of Data Bytes (16-bit)</td></tr>
  * <tr><td>20-27</td><td>Offset to VLSD/SD (64-bit). Must be last</td></tr>
  * <tr><td colspan="2">Non-Mandatory Members</td></tr>
  * <tr><td>20-23</td><td>CRC checksum (32-bit)</td></tr>
  * <tr><td>24:0-2</td><td>Complete ACK (enumerate)</td></tr>
  * <tr><td>24:3-5</td><td>Pre-emptive ACK (enumerate)</td></tr>
  * <tr><td>24:6</td><td>Tx Failed Flag (boolean)</td></tr>
  * <tr><td>25:0-3</td><td>Tx ACK</td></tr>
  * <tr><td>25:4-7</td><td>Most Control Message Type (enumerate)</td></tr>
  * <tr><td>26</td><td>Packet Index</td></tr>
  * <tr><td>27-34</td><td>Offset to VLSD/SD (64-bit). Must be last</td></tr>
  * </table>
  * @param data_group Reference to the data group.
  */
  void CreateMostMessage(IDataGroup& data_group) const;

  /** \brief Creates the MOST_EthernetPacket channel group.
 *
 * The MOST_EthernetPacket channel group consists of 2 top-most channels, time
 * and the a message frame channel. The message frame channel have so-called
 * composite channel a.k.a as sub-channels and below is the channel group
 * layout.
   *
   * Note that that the VLSD offset must be the last 8 byte in the record
   * buffer. This is an internal requirement.
 * <table>
 * <caption id="multi_row">MOST_EthernetPacket Layout</caption>
 * <tr><th>Byte:Bit Offset</th><th>Channel Description</th></tr>
 * <tr><td>0-7</td><td>Time [s]</td></tr>
 * <tr><td>8</td><td>Bus Channel</td></tr>
 * <tr><td>9:0-1</td><td>Speed (enumerate)</td></tr>
 * <tr><td>9:2</td><td>Transfer Type (enumerate)</td></tr>
 * <tr><td>9:3</td><td>Direction (enumerate)</td></tr>
 * <tr><td>10-15</td><td>Source Address (48-bit)</td></tr>
 * <tr><td>16-21</td><td>Destination Address (48-bit)</td></tr>
 * <tr><td>22-23</td><td>Specified Number of Data Bytes (16-bit)</td></tr>
 * <tr><td>24-25</td><td>Received Number of Data Bytes (16-bit)</td></tr>
 * <tr><td>26-27</td><td>Number of Data Bytes (16-bit)</td></tr>
 * <tr><td>28-35</td><td>Offset to VLSD/SD (64-bit)</td></tr>
 * <tr><td colspan="2">Non-Mandatory Members</td></tr>
 * <tr><td>28-31</td><td>CRC checksum (32-bit)</td></tr>
 * <tr><td>32:0-2</td><td>Complete ACK (enumerate)</td></tr>
 * <tr><td>32:3-5</td><td>Pre-emptive ACK (enumerate)</td></tr>
 * <tr><td>33:0-3</td><td>Tx ACK</td></tr>
 * <tr><td>34-41</td><td>Offset to VLSD/SD (64-bit)</td></tr>
 * </table>
 * @param data_group Reference to the data group.
   */
  void CreateMostEthernetPacket(IDataGroup& data_group) const;

  /** \brief Creates the MOST_Packet channel group.
 *
 * The MOST_Packet channel group consists of 2 top-most channels, time
 * and the a message frame channel. The message frame channel have so-called
 * composite channel a.k.a as sub-channels and below is the channel group
 * layout.
 * <table>
 * <caption id="multi_row">MOST_Packet Layout</caption>
 * <tr><th>Byte:Bit Offset</th><th>Channel Description</th></tr>
 * <tr><td>0-7</td><td>Time [s]</td></tr>
 * <tr><td>8</td><td>Bus Channel</td></tr>
 * <tr><td>9:0-1</td><td>Speed (enumerate)</td></tr>
 * <tr><td>9:2</td><td>Transfer Type (enumerate)</td></tr>
 * <tr><td>9:3</td><td>Direction (enumerate)</td></tr>
 * <tr><td>10-11</td><td>Source Address (16-bit)</td></tr>
 * <tr><td>12-13</td><td>Destination Address (16-bit)</td></tr>
 * <tr><td>14-15</td><td>Specified Number of Data Bytes (16-bit)</td></tr>
 * <tr><td>16-17</td><td>Received Number of Data Bytes (16-bit)</td></tr>
 * <tr><td>18-19</td><td>Number of Data Bytes (16-bit)</td></tr>
 * <tr><td>20-27</td><td>Offset to VLSD/SD (64-bit)</td></tr>
 * <tr><td colspan="2">Non-Mandatory Members</td></tr>
 * <tr><td>20-23</td><td>CRC checksum (32-bit)</td></tr>
 * <tr><td>24:0-2</td><td>Complete ACK (enumerate)</td></tr>
 * <tr><td>24:3-5</td><td>Pre-emptive ACK (enumerate)</td></tr>
 * <tr><td>25:0-3</td><td>Tx ACK</td></tr>
 * <tr><td>26</td><td>Packet Index</td></tr>
 * <tr><td>27-34</td><td>Offset to VLSD/SD (64-bit)</td></tr>
 * </table>
 * @param data_group Reference to the data group.
   */
  void CreateMostPacket(IDataGroup& data_group) const ;

 /** \brief Creates the MOST_SignalState channel group.
 *
 * The MOST_SignalState channel group consists of 2 top-most channels, time
 * and the a message frame channel. The message frame channel have
 * composite channels a.k.a as sub-channels. Below is the channel group
 * layout.
 * <table>
 * <caption id="multi_row">MOST_SignalState Layout</caption>
 * <tr><th>Byte:Bit Offset</th><th>Channel Description</th></tr>
 * <tr><td>0-7</td><td>Time [s]</td></tr>
 * <tr><td>8</td><td>Bus Channel</td></tr>
 * <tr><td>9</td><td>State (enumerate)</td></tr>
 * </table>
 * @param data_group Reference to the data group.
   */
  void CreateMostSignalState(IDataGroup& data_group) const;

  /** \brief Creates the MOST_MaxPosInfo channel group.
 *
 * The MOST_MaxPosInfo channel group consists of 2 top-most channels, time
 * and the a message frame channel. The message frame channel have
 * composite channels a.k.a as sub-channels. Below is the channel group
 * layout.
 * <table>
 * <caption id="multi_row">MOST_MaxPosInfo Layout</caption>
 * <tr><th>Byte:Bit Offset</th><th>Channel Description</th></tr>
 * <tr><td>0-7</td><td>Time [s]</td></tr>
 * <tr><td>8</td><td>Bus Channel</td></tr>
 * <tr><td>9</td><td>DeviceCount</td></tr>
 * </table>
 * @param data_group Reference to the data group.
   */
  void CreateMostMaxPosInfo(IDataGroup& data_group) const;

  /** \brief Creates the MOST_BoundDesc channel group.
 *
 * The MOST_BoundDesc channel group consists of 2 top-most channels, time
 * and the a message frame channel. The message frame channel have
 * composite channels a.k.a as sub-channels. Below is the channel group
 * layout.
 * <table>
 * <caption id="multi_row">MOST_BoundDesc Layout</caption>
 * <tr><th>Byte:Bit Offset</th><th>Channel Description</th></tr>
 * <tr><td>0-7</td><td>Time [s]</td></tr>
 * <tr><td>8</td><td>Bus Channel</td></tr>
 * <tr><td>9-10</td><td>SBC [bytes] (16-bits) </td></tr>
 * </table>
 * @param data_group Reference to the data group.
   */
  void CreateMostBoundDesc(IDataGroup& data_group) const ;

  /** \brief Creates the MOST_AllocTable channel group.
 *
 * The MOST_AllocTable channel group consists of 2 top-most channels, time
 * and the a message frame channel. The message frame channel have
 * composite channels a.k.a as sub-channels. Below is the channel group
 * layout.
 * <table>
 * <caption id="multi_row">MOST_AllocTable Layout</caption>
 * <tr><th>Byte:Bit Offset</th><th>Channel Description</th></tr>
 * <tr><td>0-7</td><td>Time [s]</td></tr>
 * <tr><td>8</td><td>Bus Channel</td></tr>
 * <tr><td>9-10</td><td>Free Byte Count [bytes] (16-bits)</td></tr>
 * <tr><td>11</td><td>Table Layout (Enumerate)</td></tr>
 * <tr><td>12-13</td><td>Table Length [bytes] (16-bits)</td></tr>
 * <tr><td>14-21</td><td>Table Data VLSD Offset (64-bits)</td></tr>
 * </table>
 * @param data_group Reference to the data group.
   */
  void CreateMostAllocTable(IDataGroup& data_group) const ;

    /** \brief Creates the MOST_SysLockState channel group.
   *
   * The MOST_SysLockState channel group consists of 2 top-most channels, time
   * and the a message frame channel. The message frame channel have
   * composite channels a.k.a as sub-channels. Below is the channel group
   * layout.
   * <table>
   * <caption id="multi_row">MOST_SysLockState Layout</caption>
   * <tr><th>Byte:Bit Offset</th><th>Channel Description</th></tr>
   * <tr><td>0-7</td><td>Time [s]</td></tr>
   * <tr><td>8</td><td>Bus Channel</td></tr>
   * <tr><td>9</td><td>SysLockState (enumerate)</td></tr>
   * </table>
   * @param data_group Reference to the data group.
     */
    void CreateMostSysLockState(IDataGroup& data_group) const;

    /** \brief Creates the MOST_ShutdownFlag channel group.
   *
   * The MOST_ShutdownFlag channel group consists of 2 top-most channels, time
   * and the a message frame channel. The message frame channel have
   * composite channels a.k.a as sub-channels. Below is the channel group
   * layout.
   * <table>
   * <caption id="multi_row">MOST_ShutdownFlag Layout</caption>
   * <tr><th>Byte:Bit Offset</th><th>Channel Description</th></tr>
   * <tr><td>0-7</td><td>Time [s]</td></tr>
   * <tr><td>8</td><td>Bus Channel</td></tr>
   * <tr><td>9</td><td>Shutdown Flag (enumerate)</td></tr>
   * </table>
   * @param data_group Reference to the data group.
     */
    void CreateMostShutdownFlag(IDataGroup& data_group) const;

 private:
  void CreateEthernetPacketChannel(IChannelGroup& channel_group) const;
  void CreatePacketChannel(IChannelGroup& channel_group) const;
  void CreateMessageChannel(IChannelGroup& channel_group) const;
  void CreateSignalStateChannel(IChannelGroup& channel_group) const;
  void CreateMaxPosInfoChannel(IChannelGroup& channel_group) const;
  void CreateBoundDescChannel(IChannelGroup& channel_group) const;
  void CreateAllocTableChannel(IChannelGroup& channel_group) const;
  void CreateSysLockStateChannel(IChannelGroup& channel_group) const;
  void CreateShutdownFlagChannel(IChannelGroup& channel_group) const;
  void CreateMandatoryMembers(IChannel& parent_frame) const;

  void CreateSpeedChannel(IChannel& parent_channel,
                          uint32_t byte_offset) const;
  void CreateStateChannel(IChannel& parent_channel,
                          uint32_t byte_offset) const;
  void CreateDeviceCountChannel(IChannel& parent_channel,
                                uint32_t byte_offset) const;
  void CreateSbcChannel(IChannel& parent_channel,
                        uint32_t byte_offset) const;
  void CreateFreeBytesChannel(IChannel& parent_channel,
                              uint32_t byte_offset) const;
  void CreateTableLayoutChannel(IChannel& parent_channel,
                                uint32_t byte_offset) const;
  void CreateTableLengthChannel(IChannel& parent_channel,
                                uint32_t byte_offset) const;
  void CreateTableDataChannel(IChannel& parent_channel,
                              uint32_t byte_offset) const;
  void CreateLockStateChannel(IChannel& parent_channel,
                              uint32_t byte_offset) const;
  void CreateShutdownChannel(IChannel& parent_channel,
                             uint32_t byte_offset) const;
  void CreateTransferTypeChannel(IChannel& parent_channel,
                                 uint32_t byte_offset) const;
  void CreateDirectionChannel(IChannel& parent_channel,
                              uint32_t byte_offset) const;
  void CreateSourceChannel(IChannel& parent_channel,
                           uint32_t byte_offset, uint32_t nof_bits) const;
  void CreateDestinationChannel(IChannel& parent_channel,
                                uint32_t byte_offset, uint32_t nof_bits) const;
  void CreateSpecifiedBytesChannel(IChannel& parent_channel,
                                   uint32_t byte_offset) const;
  void CreateReceivedBytesChannel(IChannel& parent_channel,
                                  uint32_t byte_offset) const;
  void CreateDataLengthChannel(IChannel& parent_channel,
                               uint32_t byte_offset) const;
  void CreateDataBytesChannel(IChannel& parent_channel,
                              uint32_t byte_offset) const;
  void CreateTxFailedChannel(IChannel& parent_channel,
                             uint32_t byte_offset) const;
  void CreateCrcChannel(IChannel& parent_channel,
                        uint32_t byte_offset, uint32_t nof_bits) const;
  void CreateCompleteAckChannel(IChannel& parent_channel,
                                uint32_t byte_offset) const;
  void CreatePreemptiveAckChannel(IChannel& parent_channel,
                                  uint32_t byte_offset) const;
  void CreateTxAckChannel(IChannel& parent_channel,
                          uint32_t byte_offset) const;
  void CreatePacketIndexChannel(IChannel& parent_channel,
                                uint32_t byte_offset) const;
  void CreatePacketTypeChannel(IChannel& parent_channel,
                               uint32_t byte_offset) const;
};

}  // namespace mdf

