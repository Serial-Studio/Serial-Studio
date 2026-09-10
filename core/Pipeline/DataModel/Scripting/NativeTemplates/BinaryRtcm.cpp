/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include "DataModel/Scripting/NativeTemplates/BinaryRtcm.h"

#include <bit>

#include "Core/SSAssert.h"
#include "DataModel/Scripting/NativeTemplates/NativeTemplateSupport.h"

using DataModel::INativeParser;
using DataModel::INativeTemplate;
using DataModel::NativeParamSpec;
using DataModel::NativeParamType;
using namespace DataModel::TemplateSupport;

//--------------------------------------------------------------------------------------------------
// RTCM corrections
//--------------------------------------------------------------------------------------------------

/**
 * @brief Latching RTCM3 decoder for reference-station (1005) and MSM7 (1077, 1087) messages.
 */
class RtcmParser final : public DataModel::NativeLatchParser {
public:
  /**
   * @brief Stores the CRC-validation flag.
   */
  explicit RtcmParser(bool validateCrc)
    : NativeLatchParser(kRtcmChannels), m_validateCrc(validateCrc)
  {
    SS_ASSERT_LOG(latchCount() == kRtcmChannels);
  }

  /**
   * @brief Treats text frames as UTF-8 bytes and reuses the binary path.
   */
  [[nodiscard]] QList<QStringList> parseText(const QString& frame) override
  {
    return parseBinary(frame.toUtf8());
  }

  /**
   * @brief UTF-8 text frames already carry the raw bytes; skips the QString round-trip.
   */
  [[nodiscard]] QList<QStringList> parseUtf8(const QByteArray& frame) override
  {
    return parseBinary(frame);
  }

  /**
   * @brief Decodes one RTCM3 frame: 0xD3, 10-bit length, payload, CRC-24Q.
   */
  [[nodiscard]] QList<QStringList> parseBinary(const QByteArray& frame) override
  {
    if (frame.size() < 6 || u8At(frame, 0) != 0xD3)
      return latchedFrame();

    const int length = ((u8At(frame, 1) & 0x03) << 8) | u8At(frame, 2);
    if (frame.size() < 3 + length + 3)
      return latchedFrame();

    if (m_validateCrc && !crc24Ok(frame, length))
      return latchedFrame();

    const QByteArray payload = frame.mid(3, length);
    const int message_type   = static_cast<int>(readBits(payload, 0, 12));
    routeMessage(message_type, payload);
    return latchedFrame();
  }

private:
  /**
   * @brief Validates the CRC-24Q over header + payload against the trailing 3 bytes.
   */
  [[nodiscard]] static bool crc24Ok(const QByteArray& frame, int length)
  {
    SS_ASSERT(frame.size() >= 3 + length + 3, return false);

    quint32 crc = 0;
    for (qsizetype i = 0; i < 3 + length; ++i) {
      crc ^= static_cast<quint32>(u8At(frame, i)) << 16;
      for (int bit = 0; bit < 8; ++bit) {
        crc <<= 1;
        if (crc & 0x1000000)
          crc ^= 0x1864CFB;
      }
    }

    crc                    &= 0xFFFFFF;
    const quint32 expected  = (static_cast<quint32>(u8At(frame, 3 + length)) << 16)
                           | (static_cast<quint32>(u8At(frame, 3 + length + 1)) << 8)
                           | static_cast<quint32>(u8At(frame, 3 + length + 2));
    return crc == expected;
  }

  /**
   * @brief Reads big-endian bits from the payload with optional sign extension.
   */
  [[nodiscard]] static qint64 readBits(const QByteArray& data,
                                       int bitPos,
                                       int numBits,
                                       bool isSigned = false)
  {
    if (numBits < 1 || numBits > 62)
      return 0;

    quint64 value = 0;
    for (int i = 0; i < numBits; ++i) {
      const int byte_index = (bitPos + i) / 8;
      const int bit_index  = 7 - ((bitPos + i) % 8);
      const int bit = (byte_index < data.size()) ? ((u8At(data, byte_index) >> bit_index) & 1) : 0;
      value         = (value << 1) | static_cast<quint64>(bit);
    }

    if (isSigned && (value & (quint64(1) << (numBits - 1))))
      return static_cast<qint64>(value) - (qint64(1) << numBits);

    return static_cast<qint64>(value);
  }

  /**
   * @brief Counts the satellites flagged in a 64-bit mask.
   */
  [[nodiscard]] static int satelliteCount(qint64 mask)
  {
    return std::popcount(static_cast<quint64>(mask));
  }

  /**
   * @brief Routes a validated message payload into the channel layout.
   */
  void routeMessage(int messageType, const QByteArray& payload)
  {
    if (messageType == 1005)
      routeReferenceStation(payload);
    else if (messageType == 1077)
      routeMsm7(payload, 30, 4);
    else if (messageType == 1087)
      routeMsm7(payload, 27, 8);
  }

  /**
   * @brief Reference station ARP (1005): station id + ECEF into channels 0-3.
   */
  void routeReferenceStation(const QByteArray& p)
  {
    int bits = 12;
    storeAt(0, QString::number(readBits(p, bits, 12)));
    bits += 12 + 6;
    storeAt(1, QString::number(readBits(p, bits, 38, true) * 0.0001));
    bits += 38;
    storeAt(2, QString::number(readBits(p, bits, 38, true) * 0.0001));
    bits += 38;
    storeAt(3, QString::number(readBits(p, bits, 38, true) * 0.0001));
  }

  /**
   * @brief MSM7 header: station id, epoch, sync flag and satellite count.
   */
  void routeMsm7(const QByteArray& p, int epochBits, int channelBase)
  {
    int bits = 12;
    storeAt(channelBase, QString::number(readBits(p, bits, 12)));
    bits += 12;
    storeAt(channelBase + 1, QString::number(readBits(p, bits, epochBits)));
    bits += epochBits;
    storeAt(channelBase + 2, QString::number(readBits(p, bits, 1)));
    bits += 1;
    storeAt(channelBase + 3, QString::number(satelliteCount(readBits(p, bits, 62))));
  }

private:
  static constexpr int kRtcmChannels = 16;

  bool m_validateCrc;
};

/**
 * @brief Descriptor for the RTCM corrections template.
 */
class RtcmTemplate final : public INativeTemplate {
public:
  /**
   * @brief Returns the stable template id.
   */
  [[nodiscard]] QString id() const override { return QStringLiteral("rtcm_corrections"); }

  /**
   * @brief Returns the translated display name.
   */
  [[nodiscard]] QString name() const override { return trNativeTemplate("RTCM corrections"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trNativeTemplate("Decodes RTCM3 reference-station and MSM7 headers (station id, epoch, "
                            "satellite count) into channels. Use with the Binary decoder.");
  }

  /**
   * @brief Returns the parameter schema for the template.
   */
  [[nodiscard]] QList<NativeParamSpec> params() const override
  {
    auto crc = DataModel::nativeParam(
      "validateCrc",
      NativeParamType::Bool,
      QT_TRANSLATE_NOOP("NativeTemplates", "Validate CRC"),
      QT_TRANSLATE_NOOP("NativeTemplates", "Rejects frames with an invalid CRC-24Q checksum."),
      true);

    return {crc};
  }

  /**
   * @brief Builds a configured parser instance.
   */
  [[nodiscard]] std::unique_ptr<INativeParser> makeParser(const QJsonObject& params,
                                                          QString& error) const override
  {
    Q_UNUSED(error)
    const bool crc = DataModel::nativeParamBool(params, QStringLiteral("validateCrc"), true);
    return std::make_unique<RtcmParser>(crc);
  }
};

//--------------------------------------------------------------------------------------------------
// Descriptor accessor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the process-wide RTCM corrections template descriptor.
 */
const DataModel::INativeTemplate& DataModel::rtcmTemplate()
{
  static const RtcmTemplate s_rtcm;
  return s_rtcm;
}
