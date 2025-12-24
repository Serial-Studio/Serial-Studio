/*
* Copyright 2024 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once

#include "mdf/iconfigadapter.h"

namespace mdf {

class LinConfigAdapter: public IConfigAdapter {
 public:
  LinConfigAdapter() = delete;
  explicit LinConfigAdapter(const MdfWriter& writer);

  void CreateConfig(IDataGroup& dg_block) override;
 protected:

  /** \brief Creates the channels for a LIN data frame.
  *
  * The composition layout is as below.
  *
  * <table>
  * <caption id="LinFrame">LIN_Frame Layout</caption>
  * <tr><th>Byte:Bit Offset</th><th>Channel Description</th></tr>
  * <tr><td>0-7</td><td>Time [s]</td></tr>
  * <tr><td>8</td><td>Bus Channel</td></tr>
  * <tr><td>9:0-5</td><td>Frame ID (6-bit)</td></tr>
  * <tr><td>9:6</td><td>Direction (enumerate)</td></tr>
  * <tr><td>10:0-3</td><td>Received Data Count</td></tr>
  * <tr><td>10:4-7</td><td>Data Length</td></tr>
  * <tr><td>11-18</td><td>Data Bytes</td></tr>
  * <tr><td colspan="2">Non-Mandatory Members</td></tr>
  * <tr><td>19</td><td>Checksum</td></tr>
  * <tr><td>20:0-1:</td><td>Checksum Model (enumerate)</td></tr>
  * <tr><td>21-28</td><td>Start of Frame 64-bit (ns)</td></tr>
  * <tr><td>29-32</td><td>Baud rate (bits/s). Note float 32-bit value</td></tr>
  * <tr><td>33-36</td><td>Response Baud Rate. Note float 32-bit value</td></tr>
  * <tr><td>37-40</td><td>Break Length 32-bit (ns)</td></tr>
  * <tr><td>41-44</td><td>Break Delimiter Length (ns)</td></tr>
  * </table>
  * @param group The The LIN Data Frame channel group object.
   */
  void CreateFrameChannels(IChannelGroup& group) const;

  /** \brief Creates the channels for a LIN Checksum Error message.
  *
  * The composition layout is as below.
  *
  * <table>
  * <caption id="LinChecksumError">LIN_ChecksumError Layout</caption>
  * <tr><th>Byte:Bit Offset</th><th>Channel Description</th></tr>
  * <tr><td>0-7</td><td>Time [s]</td></tr>
  * <tr><td>8</td><td>Bus Channel</td></tr>
  * <tr><td>9:0-5</td><td>Frame ID (6-bit)</td></tr>
  * <tr><td colspan="2">Non-Mandatory Members</td></tr>
  * <tr><td>9:6</td><td>Direction (enumerate)</td></tr>
  * <tr><td>10:0-4</td><td>Received Data Count</td></tr>
  * <tr><td>10:5-7</td><td>Data Length</td></tr>
  * <tr><td>11-18</td><td>Data Bytes</td></tr>
  * <tr><td>19</td><td>Checksum</td></tr>
  * <tr><td>20:0-1:</td><td>Checksum Model (enumerate)</td></tr>
  * <tr><td>21-28</td><td>Start of Frame 64-bit (ns)</td></tr>
  * <tr><td>29-32</td><td>Baud rate (bits/s). Note float 32-bit value</td></tr>
  * <tr><td>33-36</td><td>Response Baud Rate. Note float 32-bit value</td></tr>
  * <tr><td>37-40</td><td>Break Length 32-bit (ns)</td></tr>
  * <tr><td>41-44</td><td>Break Delimiter Length (ns)</td></tr>
  * </table>
  * @param group The The LIN ErrorChecksum channel group object.
   */
  void CreateChecksumErrorChannels(IChannelGroup& group) const;

  /** \brief Creates the channels for a LIN Receive Error message.
  *
  * The composition layout is as below.
  *
  * <table>
  * <caption id="LinReceiveError">LIN_ReceiveError Layout</caption>
  * <tr><th>Byte:Bit Offset</th><th>Channel Description</th></tr>
  * <tr><td>0-7</td><td>Time [s]</td></tr>
  * <tr><td>8</td><td>Bus Channel</td></tr>
  * <tr><td>9:0-5</td><td>Frame ID (6-bit)</td></tr>
  * <tr><td>10:0-3</td><td>Received Data Count</td></tr>
  * <tr><td>10:4-7</td><td>Soecified Data Count</td></tr>
  * <tr><td>11:0-3</td><td>Data Length</td></tr>
  * <tr><td>12-19</td><td>Data Bytes (8 bytes)</td></tr>
  * <tr><td>11:4-5:</td><td>Checksum Model (enumerate)</td></tr>
  * <tr><td>20-27</td><td>Start of Frame 64-bit (ns)</td></tr>
  * <tr><td>28-31</td><td>Baud rate (bits/s). Note float 32-bit value</td></tr>
  * <tr><td>32-35</td><td>Response Baud rate (bits/s). Note float 32-bit value</td></tr>
  * <tr><td>36-39</td><td>Break Length 32-bit (ns)</td></tr>
  * <tr><td>40-43</td><td>Break Delimiter Length (ns)</td></tr>
  * </table>
  * @param group The The LIN Receive Error channel group object.
  */
  void CreateReceiveErrorChannels(IChannelGroup& group) const;

  /** \brief Creates the channels for a LIN Sync Error message.
  *
  * The composition layout is as below.
  *
  * <table>
  * <caption id="LinSyncError">LIN_SyncError Layout</caption>
  * <tr><th>Byte:Bit Offset</th><th>Channel Description</th></tr>
  * <tr><td>0-7</td><td>Time [s]</td></tr>
  * <tr><td>8</td><td>Bus Channel</td></tr>
  * <tr><td>9-16</td><td>Start of Frame 64-bit (ns)</td></tr>
  * <tr><td>17-20</td><td>Baud rate (bits/s). Note float 32-bit value</td></tr>
  * <tr><td>21-24</td><td>Break Length 32-bit (ns)</td></tr>
  * <tr><td>25-28</td><td>Break Delimiter Length (ns)</td></tr>
  * </table>
  * @param group The The LIN SyncError channel group object.
   */
  void CreateSyncChannels(IChannelGroup& group) const;

  /** \brief Creates the channels for a LIN Wake Up message..
  *
  * The composition layout is as below.
  *
  * <table>
  * <caption id="LinWakeUp">LIN_WakeUp Layout</caption>
  * <tr><th>Byte:Bit Offset</th><th>Channel Description</th></tr>
  * <tr><td>0-7</td><td>Time [s]</td></tr>
  * <tr><td>8</td><td>Bus Channel</td></tr>
  * <tr><td>9-12</td><td>Response Baud Rate. Note float 32-bit value</td></tr>
  * <tr><td>13-20</td><td>Start of Frame 64-bit (ns)</td></tr>
  * </table>
  * @param group The The CAN Data Frame channel group object.
   */
  void CreateWakeUpChannels(IChannelGroup& group) const;

  /** \brief Creates the channels for a LIN Transmission Error message.
  *
  * The composition layout is as below.
  *
  * <table>
  * <caption id="LinTransmissionError">LIN_TransmissionError Layout</caption>
  * <tr><th>Byte:Bit Offset</th><th>Channel Description</th></tr>
  * <tr><td>0-7</td><td>Time [s]</td></tr>
  * <tr><td>8</td><td>Bus Channel</td></tr>
  * <tr><td>9:0-5</td><td>Frame ID (6-bit)</td></tr>
  * <tr><td colspan="2">Non-Mandatory Members</td></tr>
  * <tr><td>10:0-4</td><td>Specified Data Count</td></tr>
  * <tr><td>10:4-5:</td><td>Checksum Model (enumerate)</td></tr>
  * <tr><td>11-18</td><td>Start of Frame 64-bit (ns)</td></tr>
  * <tr><td>19-22</td><td>Baud rate (bits/s). Note float 32-bit value</td></tr>
  * <tr><td>23-26</td><td>Break Length 32-bit (ns)</td></tr>
  * <tr><td>27-30</td><td>Break Delimiter Length (ns)</td></tr>
  * </table>
  * @param group The The LIN ErrorChecksum channel group object.
   */
  void CreateTransmissionErrorChannels(IChannelGroup& group) const;

  /** \brief Creates the channels for a LIN Spike message.
 *
 * The composition layout is as below.
 *
 * <table>
 * <caption id="LinSpike">LIN_Spike Layout</caption>
 * <tr><th>Byte:Bit Offset</th><th>Channel Description</th></tr>
 * <tr><td>0-7</td><td>Time [s]</td></tr>
 * <tr><td>8</td><td>Bus Channel</td></tr>
 * <tr><td>9-12</td><td>Baud rate (bits/s). Note float 32-bit value</td></tr>
 * <tr><td>13-20</td><td>Start of Frame 64-bit (ns)</td></tr>
 * </table>
 * @param group The The LIN Spike channel group object.
 */
  void CreateSpikeChannels(IChannelGroup& group) const;

 /** \brief Creates the channels for a LIN Long Dominant Signal message.
 *
 * The composition layout is as below.
 *
 * <table>
 * <caption id="LinLongDom">LIN_LongDom Layout</caption>
 * <tr><th>Byte:Bit Offset</th><th>Channel Description</th></tr>
 * <tr><td>0-7</td><td>Time [s]</td></tr>
 * <tr><td>8</td><td>Bus Channel</td></tr>
 * <tr><td>9-12</td><td>Baud rate (bits/s). Note float 32-bit value</td></tr>
 * <tr><td>13-20</td><td>Start of Frame 64-bit (ns)</td></tr>
 * <tr><td>21-24</td><td>Length (ns)</td></tr>
 * <tr><td>25:0-1</td><td>Type (enumerate)</td></tr>
 * </table>
 * @param group The The LIN Spike channel group object.
 */
  void CreateLongDominantChannels(IChannelGroup& group) const;
};

}  // namespace mdf


