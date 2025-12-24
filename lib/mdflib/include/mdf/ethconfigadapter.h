/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once

#include "mdf/iconfigadapter.h"

namespace mdf {

class EthConfigAdapter : public IConfigAdapter {
 public:
  EthConfigAdapter() = delete;
  explicit EthConfigAdapter(const MdfWriter& writer);

  void CreateConfig(IDataGroup& dg_block) override;
 protected:

  /** \brief Create the composition channels for an Ethernet frame.
  *
  * The composition layout is as above. Note that the
  * <table>
  * <caption id="EthFrame">ETH_Frame Layout</caption>
  * <tr><th>Byte:Bit Offset</th><th>Channel Description</th></tr>
  * <tr><td>0-7</td><td>Time [s]</td></tr>
  * <tr><td>8</td><td>Bus Channel</td></tr>
  * <tr><td>9-14</td><td>Source Address (48-bit)</td></tr>
  * <tr><td>15-20</td><td>Destination Address (48-bit)</td></tr>
  * <tr><td>21-22</td><td>Ethernet Type (16-bit)</td></tr>
  * <tr><td>23-24</td><td>Received Data Count (16-bit)</td></tr>
  * <tr><td>25-26</td><td>Data Length (16-bit)</td></tr>
  * <tr><td>27-34</td><td>Offset to VLSD (54-bit). Must be last</td></tr>
  * <tr><td colspan="2">Non-Mandatory Members</td></tr>
  * <tr><td>27-30</td><td>CRC 32-bit</td></tr>
  * <tr><td>31:0</td><td>Direction (enumerate)</td></tr>
  * <tr><td>32-33</td><td>Padding Bytes</td></tr>
  * <tr><td>34-41</td><td>Offset to VLSD (54-bit). Must be last</td></tr>
  * </table>
  * @param group The The ETH Frame channel group object.
   */
  void CreateFrameChannels(IChannelGroup& group) const;

  /** \brief Create the composition channels for an Ethernet Checksum Error frame.
*
* The composition layout is as above. Note that the
* <table>
* <caption id="EthChecksumError">ETH_ChecksumError Layout</caption>
* <tr><th>Byte:Bit Offset</th><th>Channel Description</th></tr>
* <tr><td>0-7</td><td>Time [s]</td></tr>
* <tr><td>8</td><td>Bus Channel</td></tr>
* <tr><td>9-14</td><td>Source Address (48-bit)</td></tr>
* <tr><td>15-20</td><td>Destination Address (48-bit)</td></tr>
* <tr><td>21-22</td><td>Ethernet Type (16-bit)</td></tr>
* <tr><td>23-24</td><td>Data Length (16-bit)</td></tr>
* <tr><td>25-28</td><td>CRC 32-bit</td></tr>
* <tr><td>29-32</td><td>Expected CRC 32-bit</td></tr>
* <tr><td colspan="2">Non-Mandatory Members</td></tr>
* <tr><td>33-34</td><td>Received Data Count (16-bit)</td></tr>
* <tr><td>35:0</td><td>Direction (enumerate)</td></tr>
* <tr><td>36-37</td><td>Padding Bytes</td></tr>
* <tr><td>38-45</td><td>Offset to VLSD (54-bit). Must be last</td></tr>
* </table>
* @param group The The ETH Checksum Error channel group object.
 */
  void CreateChecksumErrorChannels(IChannelGroup& group) const;

  /** \brief Create the composition channels for an Ethernet Length Error frame.
*
* The composition layout is as above. Note that the
* <table>
* <caption id="EthLength">ETH_LengthError Layout</caption>
* <tr><th>Byte:Bit Offset</th><th>Channel Description</th></tr>
* <tr><td>0-7</td><td>Time [s]</td></tr>
* <tr><td>8</td><td>Bus Channel</td></tr>
* <tr><td>9-14</td><td>Source Address (48-bit)</td></tr>
* <tr><td>15-20</td><td>Destination Address (48-bit)</td></tr>
* <tr><td>21-22</td><td>Ethernet Type (16-bit)</td></tr>
* <tr><td>23-24</td><td>Received Data Count (16-bit)</td></tr>
* <tr><td colspan="2">Non-Mandatory Members</td></tr>
* <tr><td>25-26</td><td>Data Length (16-bit)</td></tr>
* <tr><td>27-30</td><td>CRC 32-bit</td></tr>
* <tr><td>31:0</td><td>Direction (enumerate)</td></tr>
* <tr><td>32-33</td><td>Padding Bytes</td></tr>
* <tr><td>34-41</td><td>Offset to VLSD (54-bit). Must be last</td></tr>
* </table>
* @param group The The ETH Length Error channel group object.
*/
  void CreateLengthErrorChannels(IChannelGroup& group) const;


  /** \brief Create the composition channels for an Ethernet Receive Error frame.
*
* The composition layout is as above. Note that the
* <table>
* <caption id="EthReceiveError">ETH_ReceiveError Layout</caption>
* <tr><th>Byte:Bit Offset</th><th>Channel Description</th></tr>
* <tr><td>0-7</td><td>Time [s]</td></tr>
* <tr><td>8</td><td>Bus Channel</td></tr>
* <tr><td>9-14</td><td>Source Address (48-bit)</td></tr>
* <tr><td>15-20</td><td>Destination Address (48-bit)</td></tr>
* <tr><td>21-22</td><td>Ethernet Type (16-bit)</td></tr>
* <tr><td>23-24</td><td>Received Data Count (16-bit)</td></tr>
* <tr><td colspan="2">Non-Mandatory Members</td></tr>
* <tr><td>25-26</td><td>Data Length (16-bit)</td></tr>
* <tr><td>27-30</td><td>CRC 32-bit</td></tr>
* <tr><td>31:0</td><td>Direction (enumerate)</td></tr>
* <tr><td>31:1-</td><td>Error Type (enumerate) </td></tr>
* <tr><td>32-33</td><td>Padding Bytes</td></tr>
* <tr><td>34-41</td><td>Offset to VLSD (54-bit). Must be last</td></tr>
* </table>
* @param group The The ETH Receive Error channel group object.
*/
  void CreateReceiveErrorChannels(IChannelGroup& group) const;
};

}  // namespace mdf

