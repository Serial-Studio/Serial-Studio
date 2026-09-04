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

#include "DataModel/Scripting/NativeTemplates/BinaryHex.h"

#include "Core/SSAssert.h"
#include "DataModel/Scripting/NativeTemplates/NativeTemplateSupport.h"

using DataModel::INativeParser;
using DataModel::INativeTemplate;
using DataModel::NativeParamSpec;
using DataModel::NativeParamType;
using namespace DataModel::TemplateSupport;

//--------------------------------------------------------------------------------------------------
// Hexadecimal bytes
//--------------------------------------------------------------------------------------------------

/**
 * @brief Decodes a hex string into grouped numeric channels (requires the Hexadecimal decoder).
 */
class HexBytesParser final : public INativeParser {
public:
  /**
   * @brief Stores the grouping, endianness and sign configuration.
   */
  HexBytesParser(int bytesPerValue, bool bigEndian, bool signedValues)
    : m_bytesPerValue(bytesPerValue), m_bigEndian(bigEndian), m_signedValues(signedValues)
  {
    SS_ASSERT(m_bytesPerValue >= 1, m_bytesPerValue = 1);
    SS_ASSERT(m_bytesPerValue <= 8, m_bytesPerValue = 8);
  }

  /**
   * @brief Parses the hex string into byte groups and emits one channel per group.
   */
  [[nodiscard]] QList<QStringList> parseText(const QString& frame) override
  {
    SS_ASSERT(m_bytesPerValue >= 1, return {});

    QString hex = frame;
    hex.remove(QLatin1Char(' '));
    hex.remove(QLatin1Char('\t'));

    QByteArray bytes = QByteArray::fromHex(hex.toLatin1());
    return parseBinary(bytes);
  }

  /**
   * @brief Byte-level twin of parseText: strips whitespace and hex-decodes without QString.
   */
  [[nodiscard]] QList<QStringList> parseUtf8(const QByteArray& frame) override
  {
    QByteArray hex;
    hex.reserve(frame.size());
    for (const char c : frame)
      if (c != ' ' && c != '\t')
        hex.append(c);

    return parseBinary(QByteArray::fromHex(hex));
  }

  /**
   * @brief Emits grouped values from raw bytes.
   */
  [[nodiscard]] QList<QStringList> parseBinary(const QByteArray& frame) override
  {
    return groupedByteFrame(frame, m_bytesPerValue, m_bigEndian, m_signedValues);
  }

private:
  int m_bytesPerValue;
  bool m_bigEndian;
  bool m_signedValues;
};

/**
 * @brief Descriptor for the hexadecimal bytes template.
 */
class HexBytesTemplate final : public INativeTemplate {
public:
  /**
   * @brief Returns the stable template id.
   */
  [[nodiscard]] QString id() const override { return QStringLiteral("hexadecimal_bytes"); }

  /**
   * @brief Returns the translated display name.
   */
  [[nodiscard]] QString name() const override { return trNativeTemplate("Hexadecimal bytes"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trNativeTemplate("Decodes a hex string into numeric channels, with configurable "
                            "grouping, endianness and sign. Use with the Hexadecimal decoder.");
  }

  /**
   * @brief Returns the parameter schema for the template.
   */
  [[nodiscard]] QList<NativeParamSpec> params() const override
  {
    auto group = DataModel::nativeParam(
      "bytesPerValue",
      NativeParamType::Int,
      QT_TRANSLATE_NOOP("NativeTemplates", "Bytes per value"),
      QT_TRANSLATE_NOOP("NativeTemplates", "Number of bytes combined into each channel value."),
      1);
    group.minValue = 1;
    group.maxValue = 8;

    auto endian = DataModel::nativeParam(
      "endianness",
      NativeParamType::Enum,
      QT_TRANSLATE_NOOP("NativeTemplates", "Endianness"),
      QT_TRANSLATE_NOOP("NativeTemplates", "Byte order used when combining multi-byte values."),
      QStringLiteral("big"));
    endian.optionValues = {QStringLiteral("big"), QStringLiteral("little")};
    endian.optionLabels = {trNativeTemplate("Big endian"), trNativeTemplate("Little endian")};

    auto sign = DataModel::nativeParam(
      "signedValues",
      NativeParamType::Bool,
      QT_TRANSLATE_NOOP("NativeTemplates", "Signed values"),
      QT_TRANSLATE_NOOP("NativeTemplates", "Interprets each value as two's-complement signed."),
      false);

    return {group, endian, sign};
  }

  /**
   * @brief Validates the grouping and builds a configured parser instance.
   */
  [[nodiscard]] std::unique_ptr<INativeParser> makeParser(const QJsonObject& params,
                                                          QString& error) const override
  {
    const int group = DataModel::nativeParamInt(params, QStringLiteral("bytesPerValue"), 1);
    if (group < 1 || group > 8) {
      error = trNativeTemplate("Bytes per value must be between 1 and 8.");
      return nullptr;
    }

    const QString endian =
      DataModel::nativeParamString(params, QStringLiteral("endianness"), QStringLiteral("big"));
    const bool sign = DataModel::nativeParamBool(params, QStringLiteral("signedValues"), false);
    return std::make_unique<HexBytesParser>(group, endian != QStringLiteral("little"), sign);
  }
};

//--------------------------------------------------------------------------------------------------
// Descriptor accessor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the process-wide hexadecimal-bytes template descriptor.
 */
const DataModel::INativeTemplate& DataModel::hexBytesTemplate()
{
  static const HexBytesTemplate s_hexBytes;
  return s_hexBytes;
}
