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

#include "DataModel/Scripting/NativeTemplates/TextNmea0183.h"

#include "DataModel/Scripting/NativeTemplates/NativeTemplateSupport.h"
#include "SerialStudio.h"
#include "SSAssert.h"

using DataModel::INativeParser;
using DataModel::INativeTemplate;
using DataModel::NativeParamSpec;
using DataModel::NativeParamType;
using namespace DataModel::TemplateSupport;

//--------------------------------------------------------------------------------------------------
// NMEA 0183
//--------------------------------------------------------------------------------------------------

/**
 * @brief Latching NMEA 0183 sentence extractor (GGA, RMC, GLL, VTG) with optional checksum.
 */
class Nmea0183Parser final : public DataModel::NativeLatchParser {
public:
  /**
   * @brief Stores the talker prefix and checksum-validation flag.
   */
  Nmea0183Parser(const QString& talker, bool validateChecksum)
    : NativeLatchParser(kNmeaChannels), m_talker(talker), m_validateChecksum(validateChecksum)
  {
    SS_ASSERT(!m_talker.isEmpty(), m_talker = QStringLiteral("GP"));
  }

  /**
   * @brief Validates and routes a sentence into the fixed channel layout.
   */
  [[nodiscard]] QList<QStringList> parseText(const QString& frame) override
  {
    const QString sentence = frame.trimmed();
    if (!sentence.startsWith(QLatin1Char('$')))
      return latchedFrame();

    const qsizetype star = sentence.indexOf(QLatin1Char('*'));
    if (m_validateChecksum && star >= 0 && !checksumOk(sentence, star))
      return latchedFrame();

    const QString data = (star >= 0) ? sentence.left(star) : sentence;
    const auto fields  = data.split(QLatin1Char(','));
    const QString id   = fields.constFirst().mid(1);
    if (!id.startsWith(m_talker))
      return latchedFrame();

    routeSentence(id.mid(m_talker.length()), fields);
    return latchedFrame();
  }

  /**
   * @brief Treats binary frames as UTF-8 text and reuses the text path.
   */
  [[nodiscard]] QList<QStringList> parseBinary(const QByteArray& frame) override
  {
    return parseText(QString::fromUtf8(frame));
  }

private:
  /**
   * @brief XOR-validates the checksum between $ and *.
   */
  [[nodiscard]] static bool checksumOk(const QString& sentence, qsizetype star)
  {
    SS_ASSERT(star > 0, return false);
    SS_ASSERT(sentence.startsWith(QLatin1Char('$')), return false);

    bool ok            = false;
    const int expected = QStringView(sentence).mid(star + 1).toInt(&ok, 16);
    if (!ok)
      return false;

    int computed = 0;
    for (qsizetype i = 1; i < star; ++i)
      computed ^= sentence.at(i).toLatin1();

    return computed == expected;
  }

  /**
   * @brief Converts DDMM.MMMM + hemisphere to signed decimal degrees.
   */
  [[nodiscard]] static double parseCoordinate(const QString& text,
                                              const QString& hemisphere,
                                              int degreeDigits)
  {
    SS_ASSERT(degreeDigits == 2 || degreeDigits == 3, degreeDigits = 2);

    if (text.length() < degreeDigits + 2)
      return 0.0;

    static constexpr double kInvMinutesPerDegree = 1.0 / 60.0;

    const double degrees = SerialStudio::toDouble(QStringView(text).left(degreeDigits), nullptr);
    const double minutes = SerialStudio::toDouble(QStringView(text).mid(degreeDigits), nullptr);
    const double decimal = degrees + (minutes * kInvMinutesPerDegree);

    const bool negative = (hemisphere == QStringLiteral("S") || hemisphere == QStringLiteral("W"));
    return negative ? -decimal : decimal;
  }

  /**
   * @brief Routes a sentence formatter (GGA/RMC/GLL/VTG) into the channel layout.
   */
  void routeSentence(const QString& formatter, const QStringList& fields)
  {
    const auto field = [&fields](qsizetype i) {
      return (i < fields.size()) ? fields.at(i) : QString();
    };
    const auto numeric = [&field](qsizetype i) {
      return QString::number(SerialStudio::toDouble(QStringView(field(i)), nullptr));
    };

    if (formatter == QStringLiteral("GGA")) {
      storeAt(0, QString::number(parseCoordinate(field(2), field(3), 2)));
      storeAt(1, QString::number(parseCoordinate(field(4), field(5), 3)));
      storeAt(2, numeric(9));
      storeAt(3, numeric(6));
      storeAt(4, numeric(7));
      storeAt(5, numeric(8));
      return;
    }

    if (formatter == QStringLiteral("RMC")) {
      storeAt(0, QString::number(parseCoordinate(field(3), field(4), 2)));
      storeAt(1, QString::number(parseCoordinate(field(5), field(6), 3)));
      storeAt(6, numeric(7));
      storeAt(7, numeric(8));
      return;
    }

    if (formatter == QStringLiteral("GLL")) {
      storeAt(0, QString::number(parseCoordinate(field(1), field(2), 2)));
      storeAt(1, QString::number(parseCoordinate(field(3), field(4), 3)));
      return;
    }

    if (formatter == QStringLiteral("VTG")) {
      storeAt(6, numeric(5));
      storeAt(7, numeric(1));
    }
  }

private:
  static constexpr int kNmeaChannels = 16;

  QString m_talker;
  bool m_validateChecksum;
};

/**
 * @brief Descriptor for the NMEA 0183 template.
 */
class Nmea0183Template final : public INativeTemplate {
public:
  /**
   * @brief Returns the stable template id.
   */
  [[nodiscard]] QString id() const override { return QStringLiteral("nmea_0183"); }

  /**
   * @brief Returns the translated display name.
   */
  [[nodiscard]] QString name() const override { return trNativeTemplate("NMEA 0183 sentences"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trNativeTemplate("Decodes GGA, RMC, GLL and VTG sentences into latitude, longitude, "
                            "altitude, speed and fix-quality channels.");
  }

  /**
   * @brief Returns the parameter schema for the template.
   */
  [[nodiscard]] QList<NativeParamSpec> params() const override
  {
    auto talker =
      DataModel::nativeParam("talker",
                             NativeParamType::String,
                             QT_TRANSLATE_NOOP("NativeTemplates", "Talker prefix"),
                             QT_TRANSLATE_NOOP("NativeTemplates",
                                               "Two-letter talker id, e.g. GP for GPS or GN for "
                                               "multi-constellation receivers."),
                             QStringLiteral("GP"));

    auto checksum =
      DataModel::nativeParam("validateChecksum",
                             NativeParamType::Bool,
                             QT_TRANSLATE_NOOP("NativeTemplates", "Validate checksum"),
                             QT_TRANSLATE_NOOP("NativeTemplates",
                                               "Rejects sentences whose *hh checksum does not "
                                               "match."),
                             true);

    return {talker, checksum};
  }

  /**
   * @brief Validates the talker prefix and builds a configured parser instance.
   */
  [[nodiscard]] std::unique_ptr<INativeParser> makeParser(const QJsonObject& params,
                                                          QString& error) const override
  {
    const QString talker =
      DataModel::nativeParamString(params, QStringLiteral("talker"), QStringLiteral("GP"))
        .trimmed();
    if (talker.isEmpty()) {
      error = trNativeTemplate("The talker prefix must not be empty.");
      return nullptr;
    }

    const bool checksum =
      DataModel::nativeParamBool(params, QStringLiteral("validateChecksum"), true);
    return std::make_unique<Nmea0183Parser>(talker, checksum);
  }
};

//--------------------------------------------------------------------------------------------------
// Descriptor accessor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the process-wide NMEA 0183 template descriptor.
 */
const DataModel::INativeTemplate& DataModel::nmea0183Template()
{
  static const Nmea0183Template s_nmea0183;
  return s_nmea0183;
}
