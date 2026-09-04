/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>

namespace IO {
namespace Drivers {

/**
 * @brief gs_usb wire vocabulary (candleLight firmware / Linux kernel gs_usb driver),
 *        header-only so the ctest tier exercises it without linking libusb or QtSerialBus.
 */
namespace GsUsb {

// gs_usb control requests (bRequest, Linux kernel enum gs_usb_breq)
inline constexpr std::uint8_t kBreqHostFormat    = 0;
inline constexpr std::uint8_t kBreqBitTiming     = 1;
inline constexpr std::uint8_t kBreqMode          = 2;
inline constexpr std::uint8_t kBreqBtConst       = 4;
inline constexpr std::uint8_t kBreqDataBitTiming = 10;
inline constexpr std::uint8_t kBreqBtConstExt    = 11;

// gs_usb mode values
inline constexpr std::uint32_t kModeReset = 0;
inline constexpr std::uint32_t kModeStart = 1;

// gs_can_mode flags (GS_CAN_MODE_* = BIT(n))
inline constexpr std::uint32_t kModeListenOnly    = 1u << 0;
inline constexpr std::uint32_t kModeLoopBack      = 1u << 1;
inline constexpr std::uint32_t kModeFd            = 1u << 8;
inline constexpr std::uint32_t kModeBerrReporting = 1u << 12;

// gs_device_bt_const::feature bits (GS_CAN_FEATURE_* = BIT(n))
inline constexpr std::uint32_t kFeaturePadPkts    = 1u << 7;
inline constexpr std::uint32_t kFeatureFd         = 1u << 8;
inline constexpr std::uint32_t kFeatureBtConstExt = 1u << 10;

// gs_host_frame::flags bits (GS_CAN_FLAG_* = BIT(n))
inline constexpr std::uint8_t kFrameFlagFd  = 1u << 1;
inline constexpr std::uint8_t kFrameFlagBrs = 1u << 2;
inline constexpr std::uint8_t kFrameFlagEsi = 1u << 3;

// SocketCAN-style CAN ID flags carried in gs_host_frame::can_id
inline constexpr std::uint32_t kCanEffFlag = 0x80000000u;
inline constexpr std::uint32_t kCanRtrFlag = 0x40000000u;
inline constexpr std::uint32_t kCanErrFlag = 0x20000000u;
inline constexpr std::uint32_t kCanEffMask = 0x1fffffffu;
inline constexpr std::uint32_t kCanSffMask = 0x000007ffu;

// Sentinel echo_id marking a received (non-echo) frame
inline constexpr std::uint32_t kHostFrameRx = 0xffffffffu;

// Host byte-order marker requesting little-endian framing from the device
inline constexpr std::uint32_t kHostFormatLE = 0x0000beefu;

// Frame geometry
inline constexpr int kClassicFrameSize = 20;
inline constexpr int kFdFrameSize      = 76;
inline constexpr int kFdPayloadMax     = 64;

// Largest bulk packet a gs_usb device can request TX padding to (USB high-speed max)
inline constexpr int kMaxBulkPacketSize = 512;

// Default FD data-phase bitrate applied when no explicit rate is configured
inline constexpr std::uint32_t kDefaultFdDataBitrate = 2000000;

// BRP search ceiling bounding the solver against hostile brpMax (real silicon stays under 1024)
inline constexpr std::uint32_t kBrpSearchCeiling = 65536;

#pragma pack(push, 1)

/**
 * @brief Host byte-order handshake payload (gs_host_config).
 */
struct GsHostConfig {
  std::uint32_t byteOrder;
};

/**
 * @brief Channel start/reset request with mode flags (gs_device_mode).
 */
struct GsDeviceMode {
  std::uint32_t mode;
  std::uint32_t flags;
};

/**
 * @brief Bit-timing segments programmed into the device (gs_device_bittiming).
 */
struct GsDeviceBitTiming {
  std::uint32_t propSeg;
  std::uint32_t phaseSeg1;
  std::uint32_t phaseSeg2;
  std::uint32_t sjw;
  std::uint32_t brp;
};

/**
 * @brief Classic bit-timing limits reported by the device (gs_device_bt_const).
 */
struct GsDeviceBtConst {
  std::uint32_t feature;
  std::uint32_t fclkCan;
  std::uint32_t tseg1Min;
  std::uint32_t tseg1Max;
  std::uint32_t tseg2Min;
  std::uint32_t tseg2Max;
  std::uint32_t sjwMax;
  std::uint32_t brpMin;
  std::uint32_t brpMax;
  std::uint32_t brpInc;
};

/**
 * @brief Extended limits adding the FD data-phase ranges (gs_device_bt_const_extended).
 */
struct GsDeviceBtConstExt {
  std::uint32_t feature;
  std::uint32_t fclkCan;
  std::uint32_t tseg1Min;
  std::uint32_t tseg1Max;
  std::uint32_t tseg2Min;
  std::uint32_t tseg2Max;
  std::uint32_t sjwMax;
  std::uint32_t brpMin;
  std::uint32_t brpMax;
  std::uint32_t brpInc;
  std::uint32_t dtseg1Min;
  std::uint32_t dtseg1Max;
  std::uint32_t dtseg2Min;
  std::uint32_t dtseg2Max;
  std::uint32_t dsjwMax;
  std::uint32_t dbrpMin;
  std::uint32_t dbrpMax;
  std::uint32_t dbrpInc;
};

/**
 * @brief Classic CAN frame as it crosses the bulk endpoints (gs_host_frame).
 */
struct GsHostFrame {
  std::uint32_t echoId;
  std::uint32_t canId;
  std::uint8_t canDlc;
  std::uint8_t channel;
  std::uint8_t flags;
  std::uint8_t reserved;
  std::uint8_t data[8];
};

/**
 * @brief CAN FD frame as it crosses the bulk endpoints (64-byte payload slot).
 */
struct GsHostFrameFd {
  std::uint32_t echoId;
  std::uint32_t canId;
  std::uint8_t canDlc;
  std::uint8_t channel;
  std::uint8_t flags;
  std::uint8_t reserved;
  std::uint8_t data[64];
};

#pragma pack(pop)

static_assert(sizeof(GsHostFrame) == kClassicFrameSize, "gs_host_frame must be 20 bytes");
static_assert(sizeof(GsHostFrameFd) == kFdFrameSize, "gs_host_frame (FD) must be 76 bytes");
static_assert(sizeof(GsDeviceBtConstExt) == 72, "gs_device_bt_const_extended must be 72 bytes");
static_assert(offsetof(GsHostFrameFd, data) == offsetof(GsHostFrame, data),
              "classic and FD host frames must share their header layout");

/**
 * @brief Maps a CAN FD DLC code (0-15) to its payload length in bytes.
 */
[[nodiscard]] inline int dlc2len(std::uint8_t dlc)
{
  constexpr std::uint8_t kTable[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64};
  return kTable[dlc & 0x0F];
}

/**
 * @brief Maps a payload length (0-64 bytes) to the smallest CAN FD DLC code that holds it.
 */
[[nodiscard]] inline std::uint8_t len2dlc(int length)
{
  constexpr struct {
    int max;
    std::uint8_t dlc;
  } kSteps[] = {
    { 8,  8},
    {12,  9},
    {16, 10},
    {20, 11},
    {24, 12},
    {32, 13},
    {48, 14},
    {64, 15}
  };

  if (length <= 8)
    return static_cast<std::uint8_t>(std::max(length, 0));

  for (const auto& step : kSteps)
    if (length <= step.max)
      return step.dlc;

  return 15;
}

/**
 * @brief Solves gs_usb bit-timing segments for a target bitrate from BT_CONST-style limits;
 *        the same solver serves the arbitration and (with data-phase limits) the FD data phase.
 *        Limits arrive from untrusted device firmware, so the search is bounded: brpMax clamps
 *        to kBrpSearchCeiling and the divisor math runs in 64 bits (no wrap, no infinite loop).
 */
[[nodiscard]] inline bool solveBitTiming(const GsDeviceBtConst& bt,
                                         std::uint32_t bitrate,
                                         GsDeviceBitTiming& out)
{
  if (bitrate == 0 || bt.fclkCan == 0 || bt.brpMin > bt.brpMax)
    return false;

  const std::uint32_t step    = std::max<std::uint32_t>(1, bt.brpInc);
  const std::uint64_t ceiling = std::min<std::uint64_t>(bt.brpMax, kBrpSearchCeiling);
  for (std::uint64_t brp = std::max<std::uint32_t>(1, bt.brpMin); brp <= ceiling; brp += step) {
    const std::uint64_t divisor = static_cast<std::uint64_t>(bitrate) * brp;
    if (divisor > bt.fclkCan)
      break;

    if ((bt.fclkCan % divisor) != 0)
      continue;

    const std::uint32_t total = static_cast<std::uint32_t>(bt.fclkCan / divisor);
    if (total < 1 + bt.tseg1Min + bt.tseg2Min || total > 1 + bt.tseg1Max + bt.tseg2Max)
      continue;

    std::uint32_t tseg2 = static_cast<std::uint32_t>(std::lround(total * 0.125));
    tseg2               = std::clamp(tseg2, std::max<std::uint32_t>(1, bt.tseg2Min), bt.tseg2Max);
    if (total <= 1 + tseg2)
      continue;

    const std::uint32_t tseg1 = total - 1 - tseg2;
    if (tseg1 < bt.tseg1Min || tseg1 > bt.tseg1Max)
      continue;

    out.phaseSeg2 = tseg2;
    out.phaseSeg1 = std::max<std::uint32_t>(1, tseg1 / 2);
    out.propSeg   = tseg1 - out.phaseSeg1;
    if (out.propSeg == 0) {
      out.propSeg   = 1;
      out.phaseSeg1 = tseg1 - 1;
    }

    out.sjw = std::min(bt.sjwMax, tseg2);
    out.brp = static_cast<std::uint32_t>(brp);
    return true;
  }

  return false;
}

/**
 * @brief Extracts the classic-limit view of an extended BT_CONST, or the data-phase view when
 *        @p dataPhase is set (the FD fallback when BT_CONST_EXT is absent reuses classic limits).
 */
[[nodiscard]] inline GsDeviceBtConst timingLimits(const GsDeviceBtConstExt& ext, bool dataPhase)
{
  GsDeviceBtConst bt{};
  bt.feature = ext.feature;
  bt.fclkCan = ext.fclkCan;

  if (dataPhase) {
    bt.tseg1Min = ext.dtseg1Min;
    bt.tseg1Max = ext.dtseg1Max;
    bt.tseg2Min = ext.dtseg2Min;
    bt.tseg2Max = ext.dtseg2Max;
    bt.sjwMax   = ext.dsjwMax;
    bt.brpMin   = ext.dbrpMin;
    bt.brpMax   = ext.dbrpMax;
    bt.brpInc   = ext.dbrpInc;
    return bt;
  }

  bt.tseg1Min = ext.tseg1Min;
  bt.tseg1Max = ext.tseg1Max;
  bt.tseg2Min = ext.tseg2Min;
  bt.tseg2Max = ext.tseg2Max;
  bt.sjwMax   = ext.sjwMax;
  bt.brpMin   = ext.brpMin;
  bt.brpMax   = ext.brpMax;
  bt.brpInc   = ext.brpInc;
  return bt;
}

}  // namespace GsUsb
}  // namespace Drivers
}  // namespace IO
