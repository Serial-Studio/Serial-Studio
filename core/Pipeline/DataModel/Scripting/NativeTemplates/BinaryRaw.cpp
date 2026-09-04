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

#include "DataModel/Scripting/NativeTemplates/BinaryRaw.h"

#include "Core/SSAssert.h"
#include "DataModel/Scripting/NativeTemplates/NativeTemplateSupport.h"

using DataModel::INativeParser;
using DataModel::INativeTemplate;
using DataModel::NativeParamSpec;
using DataModel::NativeParamType;
using namespace DataModel::TemplateSupport;

//--------------------------------------------------------------------------------------------------
// Raw bytes
//--------------------------------------------------------------------------------------------------

/**
 * @brief Groups raw frame bytes into numeric channels (requires the Binary decoder).
 */
class RawBytesParser final : public INativeParser {
public:
  /**
   * @brief Stores the grouping, endianness and sign configuration.
   */
  RawBytesParser(int bytesPerValue, bool bigEndian, bool signedValues)
    : m_bytesPerValue(bytesPerValue), m_bigEndian(bigEndian), m_signedValues(signedValues)
  {
    SS_ASSERT(m_bytesPerValue >= 1, m_bytesPerValue = 1);
    SS_ASSERT(m_bytesPerValue <= 8, m_bytesPerValue = 8);
  }

  /**
   * @brief Groups the UTF-8 bytes of a text frame.
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
   * @brief Emits one channel per byte group.
   */
  [[nodiscard]] QList<QStringList> parseBinary(const QByteArray& frame) override
  {
    SS_ASSERT(!frame.isEmpty(), return {});

    return groupedByteFrame(frame, m_bytesPerValue, m_bigEndian, m_signedValues);
  }

private:
  int m_bytesPerValue;
  bool m_bigEndian;
  bool m_signedValues;
};

/**
 * @brief Descriptor for the raw bytes template.
 */
class RawBytesTemplate final : public INativeTemplate {
public:
  /**
   * @brief Returns the stable template id.
   */
  [[nodiscard]] QString id() const override { return QStringLiteral("raw_bytes"); }

  /**
   * @brief Returns the translated display name.
   */
  [[nodiscard]] QString name() const override { return trNativeTemplate("Raw bytes"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trNativeTemplate("Groups raw frame bytes into numeric channels, with configurable "
                            "grouping, endianness and sign. Use with the Binary (Direct) decoder.");
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
    return std::make_unique<RawBytesParser>(group, endian != QStringLiteral("little"), sign);
  }
};

//--------------------------------------------------------------------------------------------------
// Descriptor accessor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the process-wide raw-bytes template descriptor.
 */
const DataModel::INativeTemplate& DataModel::rawBytesTemplate()
{
  static const RawBytesTemplate s_rawBytes;
  return s_rawBytes;
}
