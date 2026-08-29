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

#include "DataModel/Scripting/NativeTemplates/BinaryUbx.h"

#include "DataModel/Scripting/NativeTemplates/NativeTemplateSupport.h"
#include "SSAssert.h"

using DataModel::INativeParser;
using DataModel::INativeTemplate;
using DataModel::NativeParamSpec;
using DataModel::NativeParamType;
using namespace DataModel::TemplateSupport;

//--------------------------------------------------------------------------------------------------
// UBX protocol (u-blox)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Latching UBX decoder for NAV-PVT, NAV-SAT, NAV-SOL and NAV-POSLLH messages.
 */
class UbxParser final : public DataModel::NativeLatchParser {
public:
  /**
   * @brief Stores the checksum-validation flag.
   */
  explicit UbxParser(bool validateChecksum)
    : NativeLatchParser(kUbxChannels), m_validateChecksum(validateChecksum)
  {
    SS_ASSERT_LOG(latchCount() == kUbxChannels);
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
   * @brief Decodes one sync-stripped UBX message: [class][id][len lo][len hi][payload][ckA][ckB].
   */
  [[nodiscard]] QList<QStringList> parseBinary(const QByteArray& frame) override
  {
    if (frame.size() < 6)
      return latchedFrame();

    const int msg_class = u8At(frame, 0);
    const int msg_id    = u8At(frame, 1);
    const int length    = u16Le(frame, 2);
    if (frame.size() < 4 + length + 2)
      return latchedFrame();

    if (m_validateChecksum && !checksumOk(frame, length))
      return latchedFrame();

    const QByteArray payload = frame.mid(4, length);
    routeMessage(msg_class, msg_id, payload);
    return latchedFrame();
  }

private:
  /**
   * @brief Validates the Fletcher checksum over class, id, length and payload.
   */
  [[nodiscard]] static bool checksumOk(const QByteArray& frame, int length)
  {
    SS_ASSERT(frame.size() >= 4 + length + 2, return false);

    quint8 ck_a = 0;
    quint8 ck_b = 0;
    for (qsizetype i = 0; i < 4 + length; ++i) {
      ck_a = static_cast<quint8>(ck_a + u8At(frame, i));
      ck_b = static_cast<quint8>(ck_b + ck_a);
    }

    return ck_a == u8At(frame, 4 + length) && ck_b == u8At(frame, 4 + length + 1);
  }

  /**
   * @brief Routes a validated message payload into the channel layout.
   */
  void routeMessage(int msgClass, int msgId, const QByteArray& payload)
  {
    if (msgClass != 0x01)
      return;

    if (msgId == 0x07 && payload.size() >= 68)
      return routeNavPvt(payload);

    if (msgId == 0x35 && payload.size() >= 6)
      return routeNavSat(payload);

    if (msgId == 0x06 && payload.size() >= 48)
      return routeNavSol(payload);

    if (msgId == 0x02 && payload.size() >= 20)
      routeNavPosLlh(payload);
  }

  /**
   * @brief NAV-PVT: position, velocity and time into channels 0-10.
   */
  void routeNavPvt(const QByteArray& p)
  {
    SS_ASSERT(p.size() >= 68, return);

    storeAt(0, QString::number(i32Le(p, 28) * 1e-7));
    storeAt(1, QString::number(i32Le(p, 24) * 1e-7));
    storeAt(2, QString::number(i32Le(p, 36) * 0.001));
    storeAt(3, QString::number(i32Le(p, 32) * 0.001));
    storeAt(4, QString::number(i32Le(p, 60) * 0.001));
    storeAt(5, QString::number(i32Le(p, 64) * 1e-5));
    storeAt(6, QString::number(i32Le(p, 48) * 0.001));
    storeAt(7, QString::number(i32Le(p, 52) * 0.001));
    storeAt(8, QString::number(i32Le(p, 56) * 0.001));
    storeAt(9, QString::number(u8At(p, 23)));
    storeAt(10, QString::number(u32Le(p, 0)));
  }

  /**
   * @brief NAV-SAT summary into channels 11-13.
   */
  void routeNavSat(const QByteArray& p)
  {
    SS_ASSERT(p.size() >= 6, return);

    storeAt(11, QString::number(u32Le(p, 0)));
    storeAt(12, QString::number(u8At(p, 4)));
    storeAt(13, QString::number(u8At(p, 5)));
  }

  /**
   * @brief NAV-SOL ECEF solution into channels 14-18.
   */
  void routeNavSol(const QByteArray& p)
  {
    SS_ASSERT(p.size() >= 48, return);

    storeAt(14, QString::number(i32Le(p, 12) * 0.01));
    storeAt(15, QString::number(i32Le(p, 16) * 0.01));
    storeAt(16, QString::number(i32Le(p, 20) * 0.01));
    storeAt(17, QString::number(u32Le(p, 24) * 0.01));
    storeAt(18, QString::number(u8At(p, 47)));
  }

  /**
   * @brief NAV-POSLLH geodetic position into channels 0-3.
   */
  void routeNavPosLlh(const QByteArray& p)
  {
    SS_ASSERT(p.size() >= 20, return);

    storeAt(0, QString::number(i32Le(p, 8) * 1e-7));
    storeAt(1, QString::number(i32Le(p, 4) * 1e-7));
    storeAt(2, QString::number(i32Le(p, 16) * 0.001));
    storeAt(3, QString::number(i32Le(p, 12) * 0.001));
  }

private:
  static constexpr int kUbxChannels = 20;

  bool m_validateChecksum;
};

/**
 * @brief Descriptor for the UBX template.
 */
class UbxTemplate final : public INativeTemplate {
public:
  /**
   * @brief Returns the stable template id.
   */
  [[nodiscard]] QString id() const override { return QStringLiteral("ubx_ublox"); }

  /**
   * @brief Returns the translated display name.
   */
  [[nodiscard]] QString name() const override { return trNativeTemplate("UBX protocol (u-blox)"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trNativeTemplate("Decodes NAV-PVT, NAV-SAT, NAV-SOL and NAV-POSLLH messages into "
                            "navigation channels. Use with the Binary decoder and 0xB5 0x62 "
                            "start delimiter.");
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
                                               "Rejects messages with an invalid Fletcher "
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
    return std::make_unique<UbxParser>(checksum);
  }
};

//--------------------------------------------------------------------------------------------------
// Descriptor accessor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the process-wide UBX (u-blox) template descriptor.
 */
const DataModel::INativeTemplate& DataModel::ubxTemplate()
{
  static const UbxTemplate s_ubx;
  return s_ubx;
}
