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

#include "DataModel/Scripting/NativeTemplates/BinaryMavlink.h"

#include "DataModel/Scripting/NativeTemplates/NativeTemplateSupport.h"
#include "SSAssert.h"

using DataModel::INativeParser;
using DataModel::INativeTemplate;
using DataModel::NativeParamSpec;
using DataModel::NativeParamType;
using namespace DataModel::TemplateSupport;

//--------------------------------------------------------------------------------------------------
// MAVLink messages
//--------------------------------------------------------------------------------------------------

/**
 * @brief Latching MAVLink decoder for ATTITUDE, VFR_HUD and GLOBAL_POSITION_INT messages.
 */
class MavlinkParser final : public DataModel::NativeLatchParser {
public:
  /**
   * @brief Stores the expected start marker for the configured protocol version.
   */
  explicit MavlinkParser(int version)
    : NativeLatchParser(kMavlinkChannels), m_marker(version == 2 ? 0xFD : 0xFE)
  {
    SS_ASSERT_LOG(version == 1 || version == 2);
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
   * @brief Decodes one MAVLink frame and updates the latched channels.
   */
  [[nodiscard]] QList<QStringList> parseBinary(const QByteArray& frame) override
  {
    if (frame.size() < 8 || u8At(frame, 0) != m_marker)
      return latchedFrame();

    const int payload_length = u8At(frame, 1);
    const int message_id     = u8At(frame, 5);
    const QByteArray payload = frame.mid(6, payload_length);

    if (message_id == 30 && payload.size() >= 16)
      routeAttitude(payload);
    else if (message_id == 74 && payload.size() >= 12)
      routeVfrHud(payload);
    else if (message_id == 33 && payload.size() >= 16)
      routeGlobalPosition(payload);

    return latchedFrame();
  }

private:
  /**
   * @brief ATTITUDE (id 30): roll, pitch, yaw into channels 0-2.
   */
  void routeAttitude(const QByteArray& p)
  {
    SS_ASSERT(p.size() >= 16, return);

    storeAt(0, QString::number(f32Le(p, 4)));
    storeAt(1, QString::number(f32Le(p, 8)));
    storeAt(2, QString::number(f32Le(p, 12)));
  }

  /**
   * @brief VFR_HUD (id 74): airspeed, groundspeed, heading, throttle into channels 3-6.
   */
  void routeVfrHud(const QByteArray& p)
  {
    SS_ASSERT(p.size() >= 12, return);

    storeAt(3, QString::number(f32Le(p, 0)));
    storeAt(4, QString::number(f32Le(p, 4)));
    storeAt(5, QString::number(i16Le(p, 8)));
    storeAt(6, QString::number(u16Le(p, 10)));
  }

  /**
   * @brief GLOBAL_POSITION_INT (id 33): latitude, longitude, altitude into channels 7-9.
   */
  void routeGlobalPosition(const QByteArray& p)
  {
    SS_ASSERT(p.size() >= 16, return);

    storeAt(7, QString::number(i32Le(p, 4) * 1e-7));
    storeAt(8, QString::number(i32Le(p, 8) * 1e-7));
    storeAt(9, QString::number(i32Le(p, 12) * 0.001));
  }

private:
  static constexpr int kMavlinkChannels = 16;

  int m_marker;
};

/**
 * @brief Descriptor for the MAVLink template.
 */
class MavlinkTemplate final : public INativeTemplate {
public:
  /**
   * @brief Returns the stable template id.
   */
  [[nodiscard]] QString id() const override { return QStringLiteral("mavlink"); }

  /**
   * @brief Returns the translated display name.
   */
  [[nodiscard]] QString name() const override { return trNativeTemplate("MAVLink messages"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trNativeTemplate("Decodes ATTITUDE, VFR_HUD and GLOBAL_POSITION_INT messages into "
                            "channels. Use with the Binary decoder.");
  }

  /**
   * @brief Returns the parameter schema for the template.
   */
  [[nodiscard]] QList<NativeParamSpec> params() const override
  {
    auto version =
      DataModel::nativeParam("version",
                             NativeParamType::Enum,
                             QT_TRANSLATE_NOOP("NativeTemplates", "Protocol version"),
                             QT_TRANSLATE_NOOP("NativeTemplates",
                                               "Selects the expected start marker (0xFE for v1, "
                                               "0xFD for v2)."),
                             QStringLiteral("v1"));
    version.optionValues = {QStringLiteral("v1"), QStringLiteral("v2")};
    version.optionLabels = {trNativeTemplate("MAVLink v1"), trNativeTemplate("MAVLink v2")};

    return {version};
  }

  /**
   * @brief Builds a configured parser instance.
   */
  [[nodiscard]] std::unique_ptr<INativeParser> makeParser(const QJsonObject& params,
                                                          QString& error) const override
  {
    Q_UNUSED(error)
    const QString version =
      DataModel::nativeParamString(params, QStringLiteral("version"), QStringLiteral("v1"));
    return std::make_unique<MavlinkParser>(version == QStringLiteral("v2") ? 2 : 1);
  }
};

//--------------------------------------------------------------------------------------------------
// Descriptor accessor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the process-wide MAVLink template descriptor.
 */
const DataModel::INativeTemplate& DataModel::mavlinkTemplate()
{
  static const MavlinkTemplate s_mavlink;
  return s_mavlink;
}
