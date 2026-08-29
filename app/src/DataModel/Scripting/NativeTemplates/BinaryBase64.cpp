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

#include "DataModel/Scripting/NativeTemplates/BinaryBase64.h"

#include "DataModel/Scripting/NativeTemplates/NativeTemplateSupport.h"
#include "SSAssert.h"

using DataModel::INativeParser;
using DataModel::INativeTemplate;
using DataModel::NativeParamSpec;
using DataModel::NativeParamType;
using namespace DataModel::TemplateSupport;

//--------------------------------------------------------------------------------------------------
// Base64-encoded data
//--------------------------------------------------------------------------------------------------

/**
 * @brief Decodes a Base64 string into one decimal channel per byte.
 */
class Base64Parser final : public INativeParser {
public:
  /**
   * @brief Decodes the Base64 payload and emits one channel per byte.
   */
  [[nodiscard]] QList<QStringList> parseText(const QString& frame) override
  {
    SS_ASSERT(!frame.isEmpty(), return {});

    const auto decoded = QByteArray::fromBase64(frame.trimmed().toLatin1());
    if (decoded.isEmpty())
      return {};

    return byteRowFrame(decoded);
  }

  /**
   * @brief Treats binary frames as the Base64 text itself.
   */
  [[nodiscard]] QList<QStringList> parseBinary(const QByteArray& frame) override
  {
    return parseText(QString::fromLatin1(frame));
  }

  /**
   * @brief UTF-8 text frames are the Base64 bytes themselves (ASCII); skips the round-trip.
   */
  [[nodiscard]] QList<QStringList> parseUtf8(const QByteArray& frame) override
  {
    const auto decoded = QByteArray::fromBase64(frame.trimmed());
    if (decoded.isEmpty())
      return {};

    return byteRowFrame(decoded);
  }
};

/**
 * @brief Descriptor for the Base64 template.
 */
class Base64Template final : public INativeTemplate {
public:
  /**
   * @brief Returns the stable template id.
   */
  [[nodiscard]] QString id() const override { return QStringLiteral("base64_encoded"); }

  /**
   * @brief Returns the translated display name.
   */
  [[nodiscard]] QString name() const override { return trNativeTemplate("Base64-encoded data"); }

  /**
   * @brief Returns the translated one-line description.
   */
  [[nodiscard]] QString description() const override
  {
    return trNativeTemplate("Decodes a Base64 payload into one decimal channel per byte. Use with "
                            "the Base64 decoder.");
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
    return std::make_unique<Base64Parser>();
  }
};

//--------------------------------------------------------------------------------------------------
// Descriptor accessor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the process-wide Base64 template descriptor.
 */
const DataModel::INativeTemplate& DataModel::base64Template()
{
  static const Base64Template s_base64;
  return s_base64;
}
