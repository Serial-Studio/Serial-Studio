/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once

#include "mdf/iconfigadapter.h"

namespace mdf {


class CanConfigAdapter : public IConfigAdapter {
 public:
  CanConfigAdapter() = delete;
  explicit CanConfigAdapter(const MdfWriter& writer);

  void CreateConfig(IDataGroup& dg_block) override;
 protected:

  /** \brief Creates the composition channels for a data frame
  *
  * The composition layout is as below. The is typical record size is 24
  * bytes
  *
  * <table>
  * <caption id="CanDataFrame">CAN_DataFrame Layout</caption>
  * <tr><th>Byte:Bit Offset</th><th>Channel Description</th></tr>
  * <tr><td>0-7</td><td>Time [s]</td></tr>
  * <tr><td>8</td><td>Bus Channel</td></tr>
  * <tr><td>9-12</td><td>Message ID + IDE (32-bit)</td></tr>
  * <tr><td>13:0-3</td><td>Data Length Code (DLC)</td></tr>
  * <tr><td>14</td><td>Data Length (7-bit)</td></tr>
  * <tr><td>15-22(+ MLSD)</td><td>Offset to VLSD/SD (64-bit) or MLSD data. Must be last</td></tr>
  * <tr><td colspan="2">Non-Mandatory Members</td></tr>
  * <tr><td>13:4</td><td>Direction (enumerate)</td></tr>
  * <tr><td>13:5</td><td>Substitute Remote Request (SRR) Flag</td></tr>
  * <tr><td>13:6</td><td>Extended Data Length (EDL)</td></tr>
  * <tr><td>13:7</td><td>Bit Rate Switch (BRS)</td></tr>
  * <tr><td>15:0</td><td>Error State Indicator (ESI)</td></tr>
  * <tr><td>15:1</td><td>Wake Up Flag</td></tr>
  * <tr><td>15:2</td><td>Single Wire Flag</td></tr>
  * <tr><td>15:3</td><td>R0 Flag</td></tr>
  * <tr><td>15:4</td><td>R1 Flag</td></tr>
  * <tr><td>16-19</td><td>Frame Length in ns (32-bit)</td></tr>
  * <tr><td>20-27(+ MLSD)</td><td>Offset to VLSD/SD (64-bit) or MLSD data. Must be last</td></tr>
  * </table>
  * @param group The The CAN Data Frame channel group object.
   */
  virtual void CreateCanDataFrameChannel(IChannelGroup& group) const;

  /** \brief Create the composition channels for a remote frame
  *
  * The composition layout is as above. Note that the
  * <table>
  * <caption id="CanRemoteFrame">CAN_RemoteFrame Layout</caption>
  * <tr><th>Byte:Bit Offset</th><th>Channel Description</th></tr>
  * <tr><td>0-7</td><td>Time [s]</td></tr>
  * <tr><td>8</td><td>Bus Channel</td></tr>
  * <tr><td>9-12</td><td>Message ID + IDE (29-bit)</td></tr>
  * <tr><td>13:0-3</td><td>Data Length Code (DLC)</td></tr>
  * <tr><td>14</td><td>Data Length (7-bit)</td></tr>
  * <tr><td colspan="2">Non-Mandatory Members</td></tr>
  * <tr><td>13:4</td><td>Direction (enumerate)</td></tr>
  * <tr><td>13:5</td><td>Substitute Remote Request (SRR) Flag</td></tr>
  * <tr><td>13:6</td><td>Wake Up Flag</td></tr>
  * <tr><td>13:7</td><td>Single Wire Flag</td></tr>
  * <tr><td>15:0</td><td>R0 Flag</td></tr>
  * <tr><td>15:1</td><td>R1 Flag</td></tr>
  * <tr><td>16-19</td><td>Frame Length in ns (32-bit)</td></tr>
  * </table>
  * @param group The The CAN Remote Frame channel group object.
   */
  virtual void CreateCanRemoteFrameChannel(IChannelGroup& group) const;

  /** \brief Create the composition channels for an error frame
  *
  * The composition layout is as above.
   * <table>
   * <caption id="CanErrorFrame">CAN_ErrorFrame Layout</caption>
   * <tr><th>Byte:Bit Offset</th><th>Channel Description</th></tr>
   * <tr><td>0-7</td><td>Time [s]</td></tr>
   * <tr><td>8</td><td>Bus Channel</td></tr>
   * <tr><td>9-12</td><td>Message ID (29-bit)</td></tr>
   * <tr><td>12:7</td><td>Identifier Extension Flag (IDE)</td></tr>
   * <tr><td>13:0-3</td><td>Data Length Code (DLC)</td></tr>
   * <tr><td>14</td><td>Data Length (7-bit)</td></tr>
   * <tr><td>13:4</td><td>Direction (enumerate)</td></tr>
   * <tr><td>13:5</td><td>Substitute Remote Request (SRR) Flag</td></tr>
   * <tr><td>13:6</td><td>Extended Data Length (EDL)</td></tr>
   * <tr><td>13:7</td><td>Bit Rate Switch (BRS)</td></tr>
   * <tr><td>15:0</td><td>Error State Indicator (ESI)</td></tr>
   * <tr><td>15:1</td><td>Wake Up Flag</td></tr>
   * <tr><td>15:2</td><td>Single Wire Flag</td></tr>
   * <tr><td>15:3</td><td>R0 Flag</td></tr>
   * <tr><td>15:4</td><td>R1 Flag</td></tr>
   * <tr><td>15:5-7</td><td>Error Type (enumerate)</td></tr>
   * <tr><td>16-19</td><td>Frame Length in ns (32-bit)</td></tr>
   * <tr><td>20-21</td><td>Bit Position (16-bit)</td></tr>
   * <tr><td>22-29(+ MLSD)</td><td>Offset to VLSD/SD (64-bit) or MLSD data. Must be last</td></tr>
   * </table>
  * @param group The The CAN Error Frame channel group object.
   */
  virtual void CreateCanErrorFrameChannel(IChannelGroup& group) const;

  /** \brief Create the composition channels for an error frame
  *
  * The composition layout is as above.
  * <table>
  * <caption id="CanOverloadFrame">CAN_OverloadFrame Layout</caption>
  * <tr><th>Byte:Bit Offset</th><th>Channel Description</th></tr>
  * <tr><td>0-7</td><td>Time [s]</td></tr>
  * <tr><td>8</td><td>Bus Channel</td></tr>
  * <tr><td>9:0</td><td>Direction (enumerate)</td></tr>
  * </table>
  * @param group The The CAN Overload Frame channel group object.
   */
  virtual void CreateCanOverloadFrameChannel(IChannelGroup& group);

 private:
  IChannel* CreateDataBytesChannel(IChannel& parent_channel,
                                   uint16_t byte_offset) const;
  IChannel* CreateDirChannel(IChannel& parent_channel,
                                   uint16_t byte_offset, uint8_t bit_offset) const;
  IChannel* CreateErrorTypeChannel(IChannel& parent_channel,
                             uint16_t byte_offset, uint8_t bit_offset) const;
};

}  // namespace mdf


