/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once

#include "mdf/iconfigadapter.h"

namespace mdf {

class FlexRayConfigAdapter : public IConfigAdapter {
 public:
  FlexRayConfigAdapter() = delete;
  explicit FlexRayConfigAdapter( const MdfWriter& writer);

  void CreateConfig(IDataGroup& dg_block) override;

 protected:

  /** \brief Creates the FLX_Frame channel group.
  *
  * The FlexRay Frame channel group consists of 2 top-most channels, time
  * and the a message frame channel. The message frame channel have so-called
  * composite channel a.k.a as sub-channels and below is the channel group
  * layout.
  *
  * Note that the VLSD offset must be the last 8 bytes in the record. This
  * is an internal design requirement as this offset is updated when the sample
  * is written to th the file.
  * <table>
  * <caption id="multi_row">FLX_Frame Layout</caption>
  * <tr><th>Byte:Bit Offset</th><th>Channel Description</th></tr>
  * <tr><td>0-7</td><td>Time [s]</td></tr>
  * <tr><td>8</td><td>Bus Channel</td></tr>
  * <tr><td>9-10</td><td>Frame ID</td></tr>
  * <tr><td>11:0-5</td><td>Cycle Count (6-bit)</td></tr>
  * <tr><td>11:6</td><td>FlexRay Channel (enumerate)</td></tr>
  * <tr><td>11:7</td><td>Direction (enumerate)</td></tr>
  * <tr><td>12</td><td>Payload Length (note word not bytes)</td></tr>
  * <tr><td>13</td><td>Data Length</td></tr>
  * <tr><td>14-23</td><td>Offset to VLSD/SD (64-bit). Must be last</td></tr>
  * <tr><td colspan="2">Non-Mandatory Members</td></tr>
  * <tr><td>14-16</td><td>CRC checksum (24-bit)</td></tr>
  * <tr><td>17-18</td><td>Header CRC (16-bit)</td></tr>
  * <tr><td>19:0</td><td>Reserved Bit Flag (boolean)</td></tr>
  * <tr><td>19:1</td><td>Payload Preamble Flag (boolean)</td></tr>
  * <tr><td>19:2</td><td>NULL Frame Flag (enumerate)</td></tr>
  * <tr><td>19:3</td><td>Sync Frame Flag (boolean)</td></tr>
  * <tr><td>19:4</td><td>Startup Frame Flag (boolean)</td></tr>
  * <tr><td>20-23</td><td>Frame Length in ns</td></tr>
  * <tr><td>24-33</td><td>Offset to VLSD/SD (64-bit). Must be last</td></tr>
  * </table>
  * @param data_group Reference to the data group.
   */
   void CreateFlxFrame(IDataGroup& data_group) const;

  /** \brief Creates the FLX_Pdu channel group.
  *
  * The FlexRay PDU channel group consists of 2 top-most channels, time
  * and the a message frame channel. The message frame channel have so-called
  * composite channel a.k.a as sub-channels and below is the channel group
  * layout.
  *
  * Note that the VLSD offset must be the last 8 bytes in the record. This
  * is an internal design requirement as this offset is updated when the sample
  * is written to th the file.
  * <table>
  * <caption id="FlxPduLayout">FLX_Pdu Layout</caption>
  * <tr><th>Byte:Bit Offset</th><th>Channel Description</th></tr>
  * <tr><td>0-7</td><td>Time [s]</td></tr>
  * <tr><td>8</td><td>Bus Channel</td></tr>
  * <tr><td>9-10</td><td>Frame ID</td></tr>
  * <tr><td>11:0-5</td><td>Cycle Count (6-bit)</td></tr>
  * <tr><td>11:6</td><td>FlexRay Channel (enumerate)</td></tr>
  * <tr><td>12</td><td>PDU Length</td></tr>
  * <tr><td>13</td><td>Data Length</td></tr>
  * <tr><td>14-23</td><td>Offset to VLSD/SD (64-bit). Must be last</td></tr>
  * <tr><td colspan="2">Non-Mandatory Members</td></tr>
  * <tr><td>14:0</td><td>Multiplexed Flag (boolean)</td></tr>
  * <tr><td>14:1</td><td>Valid Flag (boolean)</td></tr>
  * <tr><td>15-16</td><td>Switch Value</td></tr>
  * <tr><td>17-18</td><td>Update Bit Position</td></tr>
  * <tr><td>19</td><td>PDU Byte Offset</td></tr>
  * <tr><td>14:2-4</td><td>PDU Bit Offset</td></tr>
  * <tr><td>20-27</td><td>Offset to VLSD/SD (64-bit). Must be last</td></tr>
  * </table>
  * @param data_group Reference to the data group.
   */
  void CreateFlxPdu(IDataGroup& data_group) const;

    /** \brief Creates the FLX_FrameHeader channel group.
    *
    * The FlexRay Frame Header channel group consists of 2 top-most channels, time
    * and the a message frame channel. The message frame channel have so-called
    * composite channel a.k.a as sub-channels and below is the channel group
    * layout.
    *
    * Note that the VLSD offset must be the last 8 bytes in the record. This
    * is an internal design requirement as this offset is updated when the sample
    * is written to th the file.
    * <table>
    * <caption id="multi_row">FLX_FrameHeader Layout</caption>
    * <tr><th>Byte:Bit Offset</th><th>Channel Description</th></tr>
    * <tr><td>0-7</td><td>Time [s]</td></tr>
    * <tr><td>8</td><td>Bus Channel</td></tr>
    * <tr><td>9-10</td><td>Frame ID</td></tr>
    * <tr><td>11:0-5</td><td>Cycle Count (6-bit)</td></tr>
    * <tr><td>11:6</td><td>FlexRay Channel (enumerate)</td></tr>
    * <tr><td>11:7</td><td>Direction (enumerate)</td></tr>
    * <tr><td>12</td><td>Payload Length (note word not bytes)</td></tr>
    * <tr><td colspan="2">Non-Mandatory Members</td></tr>
    * <tr><td>13</td><td>Fill Data Length</td></tr>
    * <tr><td>14-16</td><td>CRC checksum (24-bit)</td></tr>
    * <tr><td>17-18</td><td>Header CRC (16-bit)</td></tr>
    * <tr><td>19:0</td><td>Reserved Bit Flag (boolean)</td></tr>
    * <tr><td>19:1</td><td>Payload Preamble Flag (boolean)</td></tr>
    * <tr><td>19:2</td><td>NULL Frame Flag (enumerate)</td></tr>
    * <tr><td>19:3</td><td>Sync Frame Flag (boolean)</td></tr>
    * <tr><td>19:4</td><td>Startup Frame Flag (boolean)</td></tr>
    * <tr><td>20-23</td><td>Frame Length in ns</td></tr>
    * <tr><td>24-33</td><td>Offset to VLSD/SD (64-bit). Must be last</td></tr>
    * </table>
    * @param data_group Reference to the data group.
     */
    void CreateFlxFrameHeader(IDataGroup& data_group) const;


  /** \brief Creates the FLX_NullFrame channel group.
  *
  * The FlexRay Null Frame channel group consists of 2 top-most channels, time
  * and the a message frame channel. The message frame channel have so-called
  * composite channel a.k.a as sub-channels and below is the channel group
  * layout.
  *
  * Note that the VLSD offset must be the last 8 bytes in the record. This
  * is an internal design requirement as this offset is updated when the sample
  * is written to th the file.
  * <table>
  * <caption id="NullFrameLayout">FLX_NullFrame Layout</caption>
  * <tr><th>Byte:Bit Offset</th><th>Channel Description</th></tr>
  * <tr><td>0-7</td><td>Time [s]</td></tr>
  * <tr><td>8</td><td>Bus Channel</td></tr>
  * <tr><td>9-10</td><td>Frame ID</td></tr>
  * <tr><td>11:0-5</td><td>Cycle Count (6-bit)</td></tr>
  * <tr><td>11:6</td><td>FlexRay Channel (enumerate)</td></tr>
  * <tr><td>11:7</td><td>Direction (enumerate)</td></tr>
  * <tr><td colspan="2">Non-Mandatory Members</td></tr>
  * <tr><td>12</td><td>Payload Length (note word not bytes)</td></tr>
  * <tr><td>13</td><td>Data Length</td></tr>
  * <tr><td>14-16</td><td>CRC checksum (24-bit)</td></tr>
  * <tr><td>17-18</td><td>Header CRC (16-bit)</td></tr>
  * <tr><td>19:0</td><td>Reserved Bit Flag (boolean)</td></tr>
  * <tr><td>19:1</td><td>Payload Preamble Flag (boolean)</td></tr>
  * <tr><td>19:2</td><td>NULL Frame Flag (enumerate)</td></tr>
  * <tr><td>19:3</td><td>Sync Frame Flag (boolean)</td></tr>
  * <tr><td>19:4</td><td>Startup Frame Flag (boolean)</td></tr>
  * <tr><td>20-23</td><td>Frame Length in ns</td></tr>
  * <tr><td>24-33</td><td>Offset to VLSD/SD (64-bit). Must be last</td></tr>
  * </table>
  * @param data_group Reference to the data group.
     */
    void CreateFlxNullFrame(IDataGroup& data_group) const;

    /** \brief Creates the FLX_ErrorFrame channel group.
  *
  * The FlexRay Error Frame channel group consists of 2 top-most channels, time
  * and the a message frame channel. The message frame channel have so-called
  * composite channel a.k.a as sub-channels and below is the channel group
  * layout.
  *
  * Note that the VLSD offset must be the last 8 bytes in the record. This
  * is an internal design requirement as this offset is updated when the sample
  * is written to th the file.
  * <table>
  * <caption id="ErrorFrameLayout">FLX_ErrorFrame Layout</caption>
  * <tr><th>Byte:Bit Offset</th><th>Channel Description</th></tr>
  * <tr><td>0-7</td><td>Time [s]</td></tr>
  * <tr><td>8</td><td>Bus Channel</td></tr>
  * <tr><td>9-10</td><td>Frame ID</td></tr>
  * <tr><td>11:0-5</td><td>Cycle Count (6-bit)</td></tr>
  * <tr><td>11:6</td><td>FlexRay Channel (enumerate)</td></tr>
  * <tr><td>11:7</td><td>Direction (enumerate)</td></tr>
  * <tr><td colspan="2">Non-Mandatory Members</td></tr>
  * <tr><td>12</td><td>Payload Length (note word not bytes)</td></tr>
  * <tr><td>13</td><td>Data Length</td></tr>
  * <tr><td>14-16</td><td>CRC checksum (24-bit)</td></tr>
  * <tr><td>17-18</td><td>Header CRC (16-bit)</td></tr>
  * <tr><td>19:0</td><td>Reserved Bit Flag (boolean)</td></tr>
  * <tr><td>19:1</td><td>Payload Preamble Flag (boolean)</td></tr>
  * <tr><td>19:2</td><td>NULL Frame Flag (enumerate)</td></tr>
  * <tr><td>19:3</td><td>Sync Frame Flag (boolean)</td></tr>
  * <tr><td>19:4</td><td>Startup Frame Flag (boolean)</td></tr>
  * <tr><td>19:5</td><td>Syntax Error Flag (boolean)</td></tr>
  * <tr><td>19:6</td><td>Content Error Flag (boolean)</td></tr>
  * <tr><td>19:7</td><td>Slot Boundary Violation Flag (boolean)</td></tr>
  * <tr><td>20:0</td><td>TX Conflict Flag (boolean)</td></tr>
  * <tr><td>20:1</td><td>Valid Flag (boolean)</td></tr>
  * <tr><td>21-24</td><td>Frame Length in ns</td></tr>
  * <tr><td>25-34</td><td>Offset to VLSD/SD (64-bit). Must be last</td></tr>
  * </table>
  * @param data_group Reference to the data group.
     */
    void CreateFlxErrorFrame(IDataGroup& data_group) const;

    /** \brief Creates the FLX_Symbol channel group.
  *
  * The FlexRay Symbol channel group consists of 2 top-most channels, time
  * and the a message frame channel. The message frame channel have so-called
  * composite channel a.k.a as sub-channels and below is the channel group
  * layout.
  *
  * Note that the VLSD offset must be the last 8 bytes in the record. This
  * is an internal design requirement as this offset is updated when the sample
  * is written to th the file.
  * <table>
  * <caption id="SymbolLayout">FLX_Symbol Layout</caption>
  * <tr><th>Byte:Bit Offset</th><th>Channel Description</th></tr>
  * <tr><td>0-7</td><td>Time [s]</td></tr>
  * <tr><td>8</td><td>Bus Channel</td></tr>
  * <tr><td>9:0-5</td><td>Cycle Count (6-bit)</td></tr>
  * <tr><td>9:6-7</td><td>FlexRay Channel Mask (enumerate)</td></tr>
  * <tr><td colspan="2">Non-Mandatory Members</td></tr>
  * <tr><td>10</td><td>Symbol Length in micro-ticks (MT)</td></tr>
  * <tr><td>11</td><td>Symbol Types (enumerate)</td></tr>
  * </table>
  * @param data_group Reference to the data group.
     */
    void CreateFlxSymbol(IDataGroup& data_group) const;
 private:
  void CreateFrameChannel(IChannelGroup& data_group) const;
  void CreatePduChannel(IChannelGroup& data_group) const;
  void CreateFrameHeaderChannel(IChannelGroup& data_group) const;
  void CreateNullFrameChannel(IChannelGroup& data_group) const;
  void CreateErrorFrameChannel(IChannelGroup& data_group) const;
  void CreateSymbolChannel(IChannelGroup& data_group) const;

  void CreateFlxChannelChannel(IChannel& parent_channel,
                              uint32_t byte_offset,
                              uint16_t bit_offset) const;
  void CreateFlxChannelMaskChannel(IChannel& parent_channel,
                               uint32_t byte_offset,
                               uint16_t bit_offset) const;
  void CreateSymbolTypeChannel(IChannel& parent_channel,
                                   uint32_t byte_offset,
                                   uint16_t bit_offset) const;
  void CreateDirectionChannel(IChannel& parent_channel,
                              uint32_t byte_offset,
                              uint16_t bit_offset) const;
  void CreateNullFrameChannel(IChannel& parent_channel,
                                uint32_t byte_offset,
                                uint16_t bit_offset) const;
  void CreateDataBytesChannel(IChannel& parent_channel,
                                uint32_t byte_offset) const;
  void CreateFillDataBytesChannel(IChannel& parent_channel,
                                uint32_t byte_offset) const;
};


}  // namespace mdf


