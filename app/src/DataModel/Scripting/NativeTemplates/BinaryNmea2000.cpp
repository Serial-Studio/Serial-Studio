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

#include "DataModel/Scripting/NativeTemplates/BinaryNmea2000.h"

#include "DataModel/Scripting/NativeTemplates/NativeTemplateSupport.h"
#include "SSAssert.h"

using DataModel::INativeParser;
using DataModel::INativeTemplate;
using DataModel::NativeParamSpec;
using DataModel::NativeParamType;
using namespace DataModel::TemplateSupport;

//--------------------------------------------------------------------------------------------------
// NMEA 2000 messages
//--------------------------------------------------------------------------------------------------

/**
 * @brief Latching NMEA 2000 decoder routing common PGNs into channels.
 */
class Nmea2000Parser final : public DataModel::NativeLatchParser {
public:
  /**
   * @brief Builds the parser with the fixed channel layout.
   */
  Nmea2000Parser() : NativeLatchParser(kN2kChannels)
  {
    SS_ASSERT_LOG(latchCount() == kN2kChannels);
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
   * @brief Decodes one preprocessed CAN frame: [id u32 le][len][data...].
   */
  [[nodiscard]] QList<QStringList> parseBinary(const QByteArray& frame) override
  {
    if (frame.size() < 6)
      return latchedFrame();

    const quint32 can_id = u32Le(frame, 0);
    const int length     = qMin<int>(u8At(frame, 4), 8);
    if (frame.size() < 5 + length)
      return latchedFrame();

    const QByteArray data = frame.mid(5, length);
    routePgn(extractPgn(can_id), data);
    return latchedFrame();
  }

private:
  /**
   * @brief Extracts the PGN from a 29-bit CAN identifier.
   */
  [[nodiscard]] static quint32 extractPgn(quint32 canId)
  {
    const quint32 pf = (canId >> 16) & 0xFF;
    const quint32 ps = (canId >> 8) & 0xFF;
    const quint32 dp = (canId >> 24) & 0x01;

    if (pf < 240)
      return (dp << 16) | (pf << 8);

    return (dp << 16) | (pf << 8) | ps;
  }

  /**
   * @brief Routes a PGN payload into the channel layout.
   */
  void routePgn(quint32 pgn, const QByteArray& d)
  {
    SS_ASSERT_LOG(d.size() <= 8);

    if (pgn == 127257 && d.size() >= 7)
      return routeAttitude(d);

    if (pgn == 129025 && d.size() >= 8)
      return routePosition(d);

    if (pgn == 129026 && d.size() >= 6)
      return routeCogSog(d);

    if (pgn == 128259 && d.size() >= 5)
      return routeSpeed(d);

    if (pgn == 128267 && d.size() >= 7)
      return routeDepth(d);

    if (pgn == 130310 && d.size() >= 5)
      return routeWaterEnv(d);

    if (pgn == 130311 && d.size() >= 7)
      routeAtmosphere(d);
  }

  /**
   * @brief Attitude (127257): yaw, pitch, roll in degrees into channels 0-2.
   */
  void routeAttitude(const QByteArray& d)
  {
    static constexpr double kRadToDeg = 180.0 / 3.14159265358979323846;

    storeAt(0, QString::number(i16Le(d, 1) * 0.0001 * kRadToDeg));
    storeAt(1, QString::number(i16Le(d, 3) * 0.0001 * kRadToDeg));
    storeAt(2, QString::number(i16Le(d, 5) * 0.0001 * kRadToDeg));
  }

  /**
   * @brief Position rapid update (129025) into channels 3-4.
   */
  void routePosition(const QByteArray& d)
  {
    storeAt(3, QString::number(i32Le(d, 0) * 1e-7));
    storeAt(4, QString::number(i32Le(d, 4) * 1e-7));
  }

  /**
   * @brief COG and SOG rapid update (129026) into channels 5-7.
   */
  void routeCogSog(const QByteArray& d)
  {
    static constexpr double kRadToDeg   = 180.0 / 3.14159265358979323846;
    static constexpr double kMpsToKnots = 1.94384;

    storeAt(5, QString::number(u16Le(d, 2) * 0.0001 * kRadToDeg));
    storeAt(6, QString::number(u16Le(d, 4) * 0.01 * kMpsToKnots));
    storeAt(7, QString::number(u8At(d, 1) & 0x03));
  }

  /**
   * @brief Speed water referenced (128259) into channels 8-9.
   */
  void routeSpeed(const QByteArray& d)
  {
    static constexpr double kMpsToKnots = 1.94384;

    storeAt(8, QString::number(u16Le(d, 1) * 0.01 * kMpsToKnots));
    storeAt(9, QString::number(u16Le(d, 3) * 0.01 * kMpsToKnots));
  }

  /**
   * @brief Water depth (128267) into channels 10-11.
   */
  void routeDepth(const QByteArray& d)
  {
    storeAt(10, QString::number(u32Le(d, 1) * 0.01));
    storeAt(11, QString::number(i16Le(d, 5) * 0.001));
  }

  /**
   * @brief Environmental water parameters (130310) into channels 12-13.
   */
  void routeWaterEnv(const QByteArray& d)
  {
    storeAt(12, QString::number(u16Le(d, 1) * 0.01 - 273.15));
    storeAt(13, QString::number(u16Le(d, 3) * 0.01 - 273.15));
  }

  /**
   * @brief Environmental atmospheric parameters (130311) into channels 14-16.
   */
  void routeAtmosphere(const QByteArray& d)
  {
    storeAt(14, QString::number(u16Le(d, 1)));
    storeAt(15, QString::number(u16Le(d, 3) * 0.01 - 273.15));
    storeAt(16, QString::number(i16Le(d, 5) * 0.004));
  }

private:
  static constexpr int kN2kChannels = 20;
};

/**
 * @brief Descriptor for the NMEA 2000 template.
 */
class Nmea2000Template final : public INativeTemplate {
public:
  /**
   * @brief Returns the stable template id.
   */
  [[nodiscard]] QString id() const override { return QStringLiteral("nmea_2000"); }

  /**
   * @brief Returns the translated display name.
   */
  [[nodiscard]] QString name() const override { return trNativeTemplate("NMEA 2000 messages"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trNativeTemplate("Decodes common marine PGNs (attitude, position, speed, depth, "
                            "environment) into channels. Use with the Binary decoder.");
  }

  /**
   * @brief Returns the parameter schema for the template.
   */
  [[nodiscard]] QList<NativeParamSpec> params() const override { return {}; }

  /**
   * @brief Builds a configured parser instance.
   */
  [[nodiscard]] std::unique_ptr<INativeParser> makeParser(const QJsonObject& params,
                                                          QString& error) const override
  {
    Q_UNUSED(params)
    Q_UNUSED(error)
    return std::make_unique<Nmea2000Parser>();
  }
};

//--------------------------------------------------------------------------------------------------
// Descriptor accessor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the process-wide NMEA 2000 template descriptor.
 */
const DataModel::INativeTemplate& DataModel::nmea2000Template()
{
  static const Nmea2000Template s_nmea2000;
  return s_nmea2000;
}
