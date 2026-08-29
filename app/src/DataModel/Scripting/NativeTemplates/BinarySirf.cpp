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

#include "DataModel/Scripting/NativeTemplates/BinarySirf.h"

#include "DataModel/Scripting/NativeTemplates/NativeTemplateSupport.h"
#include "SSAssert.h"

using DataModel::INativeParser;
using DataModel::INativeTemplate;
using DataModel::NativeParamSpec;
using DataModel::NativeParamType;
using namespace DataModel::TemplateSupport;

//--------------------------------------------------------------------------------------------------
// SiRF binary protocol
//--------------------------------------------------------------------------------------------------

/**
 * @brief Latching SiRF decoder for messages 41 (geodetic), 2, 4 and 7.
 */
class SirfParser final : public DataModel::NativeLatchParser {
public:
  /**
   * @brief Stores the checksum-validation flag.
   */
  explicit SirfParser(bool validateChecksum)
    : NativeLatchParser(kSirfChannels), m_validateChecksum(validateChecksum)
  {
    SS_ASSERT_LOG(latchCount() == kSirfChannels);
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
   * @brief Decodes one delimiter-stripped SiRF message: [len hi][len lo][payload][chk hi][chk lo].
   */
  [[nodiscard]] QList<QStringList> parseBinary(const QByteArray& frame) override
  {
    if (frame.size() < 5)
      return latchedFrame();

    const int length = ((u8At(frame, 0) & 0x7F) << 8) | u8At(frame, 1);
    if (length < 1 || frame.size() < 2 + length + 2)
      return latchedFrame();

    if (m_validateChecksum && !checksumOk(frame, length))
      return latchedFrame();

    const int message_id  = u8At(frame, 2);
    const QByteArray data = frame.mid(3, length - 1);
    routeMessage(message_id, data);
    return latchedFrame();
  }

private:
  /**
   * @brief Validates the 15-bit additive checksum over the payload.
   */
  [[nodiscard]] static bool checksumOk(const QByteArray& frame, int length)
  {
    SS_ASSERT(frame.size() >= 2 + length + 2, return false);

    quint32 sum = 0;
    for (qsizetype i = 2; i < 2 + length; ++i)
      sum += u8At(frame, i);

    const int expected = ((u8At(frame, 2 + length) & 0x7F) << 8) | u8At(frame, 2 + length + 1);
    return static_cast<int>(sum & 0x7FFF) == expected;
  }

  /**
   * @brief Routes a validated message payload into the channel layout.
   */
  void routeMessage(int messageId, const QByteArray& data)
  {
    if (messageId == 41 && data.size() >= 90)
      return routeGeodetic(data);

    if (messageId == 2 && data.size() >= 29)
      return routeMeasured(data);

    if (messageId == 4 && data.size() >= 8)
      return routeTracker(data);

    if (messageId == 7 && data.size() >= 8)
      routeClock(data);
  }

  /**
   * @brief Geodetic navigation data (id 41) into channels 0-8.
   */
  void routeGeodetic(const QByteArray& d)
  {
    SS_ASSERT(d.size() >= 90, return);

    storeAt(0, QString::number(i32Be(d, 23) * 1e-7));
    storeAt(1, QString::number(i32Be(d, 27) * 1e-7));
    storeAt(2, QString::number(i32Be(d, 35) * 0.01));
    storeAt(3, QString::number(i32Be(d, 31) * 0.01));
    storeAt(4, QString::number(i16Be(d, 40) * 0.01));
    storeAt(5, QString::number(u16Be(d, 42) * 0.01));
    storeAt(6, QString::number(u8At(d, 88)));
    storeAt(7, QString::number(u8At(d, 89) * 0.2));
    storeAt(8, QString::number(u16Be(d, 0)));
  }

  /**
   * @brief Measurement data (id 2) into channels 10-13.
   */
  void routeMeasured(const QByteArray& d)
  {
    SS_ASSERT(d.size() >= 29, return);

    storeAt(10, QString::number(i32Be(d, 1)));
    storeAt(11, QString::number(i32Be(d, 5)));
    storeAt(12, QString::number(i32Be(d, 9)));
    storeAt(13, QString::number(u8At(d, 28)));
  }

  /**
   * @brief Tracker data (id 4) into channels 14-16.
   */
  void routeTracker(const QByteArray& d)
  {
    SS_ASSERT(d.size() >= 8, return);

    storeAt(14, QString::number(u16Be(d, 1)));
    storeAt(15, QString::number(u32Be(d, 3) * 0.01));
    storeAt(16, QString::number(u8At(d, 7)));
  }

  /**
   * @brief Clock status (id 7) into channels 17-19.
   */
  void routeClock(const QByteArray& d)
  {
    SS_ASSERT(d.size() >= 8, return);

    storeAt(17, QString::number(u16Be(d, 1)));
    storeAt(18, QString::number(u32Be(d, 3)));
    storeAt(19, QString::number(u8At(d, 7)));
  }

private:
  static constexpr int kSirfChannels = 20;

  bool m_validateChecksum;
};

/**
 * @brief Descriptor for the SiRF template.
 */
class SirfTemplate final : public INativeTemplate {
public:
  /**
   * @brief Returns the stable template id.
   */
  [[nodiscard]] QString id() const override { return QStringLiteral("sirf_binary"); }

  /**
   * @brief Returns the translated display name.
   */
  [[nodiscard]] QString name() const override { return trNativeTemplate("SiRF binary protocol"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trNativeTemplate("Decodes SiRF navigation messages (geodetic data, measurements, "
                            "clock) into channels. Use with the Binary decoder and 0xA0 0xA2 "
                            "framing.");
  }

  /**
   * @brief Returns the parameter schema for the template.
   */
  [[nodiscard]] QList<NativeParamSpec> params() const override
  {
    auto checksum =
      DataModel::nativeParam("validateChecksum",
                             NativeParamType::Bool,
                             QT_TRANSLATE_NOOP("NativeTemplates", "Validate checksum"),
                             QT_TRANSLATE_NOOP("NativeTemplates",
                                               "Rejects messages with an invalid additive "
                                               "checksum."),
                             true);

    return {checksum};
  }

  /**
   * @brief Builds a configured parser instance.
   */
  [[nodiscard]] std::unique_ptr<INativeParser> makeParser(const QJsonObject& params,
                                                          QString& error) const override
  {
    Q_UNUSED(error)
    const bool checksum =
      DataModel::nativeParamBool(params, QStringLiteral("validateChecksum"), true);
    return std::make_unique<SirfParser>(checksum);
  }
};

//--------------------------------------------------------------------------------------------------
// Descriptor accessor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the process-wide SiRF binary template descriptor.
 */
const DataModel::INativeTemplate& DataModel::sirfTemplate()
{
  static const SirfTemplate s_sirf;
  return s_sirf;
}
